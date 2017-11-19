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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define _PAGER_
#include "key_egg_conf.h"

#undef max
#define max(x, y) ((x) > (y) ? (x) : (y))

void help(char *me) {
    fprintf(stderr,
            "\n"
            "KEY_EGG v%s UDP Receiver.\n"
            "Compiled at: %s %s\n",
            VERSION, __DATE__, __TIME__);

    fprintf(
        stderr,
        "\n"
        "Usage:\n"
        "  -p <p1,p2,...>  Ports to linsten on, otherwise use default ones\n"
        "  -l <file>       File to log received datagrams. Default "
        "[busted.log]\n"
        "  -a              Toggle alarm beep when receiving datagrams\n"
        "  -f              Filter packets by last received size\n"
        "  -d              Daemonize to background, no stdout/stderr outputs\n"
        "  -v              Be verbose\n"
        "\n"
        "E.g. %s -p 60311,10341,43110 -v -a\n"
        "     %s -v -l black.list -a\n"
        "\n",
        me, me);

    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    int c, x;                           // general purpose variables
    int number_of_ports = 0, *sock_fds; /* total sockets open will be
                                           number_of_ports,
                                           sock_fds[number_of_ports] */
    int ready, max_fd = 0, last_recv_size = 0;
    socklen_t len;
    unsigned short *ports;
    char **argv2, *opt, *optarg; // used for parsing cmd line
    char buffer[1024], file2log[256] = "busted.log";
    int recv_alarm = 0, verbose = 0, filter = 0, daemonize = 0;
    struct sockaddr_in serv_addr, client_addr;
    fd_set rfds;
    FILE *fptr;
    time_t curtime;

    // parse cmdline
    for (argv2 = argv + 1; *argv2; argv2++) {
        if (**argv2 == '-')
            opt = *argv2;
        else
            help(argv[0]);
        optarg = 0;
        if (*(argv2 + 1) && **(argv2 + 1) != '-') optarg = *(++argv2);
        switch (*(++opt)) {
        case 'p':
            // count commas
            if (optarg) {
                number_of_ports = 2;
                c = 0;
                while (optarg[c]) {
                    if (optarg[c] == ',') number_of_ports++;
                    c++;
                }
            }

            // alloc memory for ports array
            if (!number_of_ports) {
                fprintf(stderr,
                        "There is no port to linsten on.\nSee help: %s -h\n",
                        argv[0]);
                exit(1);
            }
            ports = (unsigned short *)malloc(number_of_ports *
                                             sizeof(unsigned short));
            memset(ports, 0, number_of_ports * sizeof(unsigned short));

            // fill array with ports
            c = x = 0;
            while (1) {
                if (*optarg == ',' || *optarg == '\0') {
                    buffer[c] = '\0';
                    ports[x++] = atoi(buffer);
                    c = 0;
                    if (*optarg == '\0') break;
                } else
                    buffer[c++] = *optarg;
                optarg++;
            }
            break;

        case 'l':
            if (optarg)
                strcpy(file2log, optarg);
            else {
                fprintf(stderr, "No file specified.\nSee help: %s -h\n",
                        argv[0]);
                exit(1);
            }
            break;

        case 'f': filter = 1; break;
        case 'a': recv_alarm = 1; break;
        case 'v': verbose = 1; break;
        case 'd': daemonize = 1; break;
        default: help(argv[0]);
        }
    }

    /*
     * print key_egg notify address/es, we no longer need the 'argv2` pointer
     * to parse cmd-line, so we'll use it here
     */
    for (argv2 = pager_ip_list, c = 0; *argv2; argv2++, c++)
        printf(
            "[W] Warning: Reverse %d address for key egg notifications is %s\n",
            c + 1, *argv2);

    if (daemonize) {
        switch (fork()) {
        case 0:
            verbose = 0; // disable verbose just in case, we don't need it
            break;

        case -1: perror("Cannot fork process"); exit(1);

        default: puts("Daemon started..."); exit(0);
        }
    }

    /*
     * if no ports were defined by user,
     * we'll use default ports from key_egg_conf.h
     */
    if (!number_of_ports) {
        number_of_ports = sizeof(pager_port_list) / sizeof(unsigned short);
        ports = pager_port_list;
        if (verbose) fputs("[+] Using default ports to listen", stdout);
    } else if (verbose)
        fputs("[+] Using user defined ports to listen", stdout);
    if (verbose) {
        c = 0;
        fputs(" (", stdout);
        while (ports[c]) printf(" %d", ports[c++]);
        puts(" )");
    }

    if (verbose) printf("[+] Creating/opening log file (%s)...\n", file2log);
    fptr = fopen(file2log, "a");
    if (!fptr) {
        perror("fopen() has failed");
        exit(1);
    }
    if (verbose) {
        puts("[+] Done.");
        puts("[+] Trying to allocate memory for socket array...");
    }

    // create socket array
    sock_fds = (int *)malloc(number_of_ports * sizeof(int));
    if (!sock_fds) {
        perror("malloc() has failed");
        exit(1);
    }
    memset(sock_fds, -1, number_of_ports * sizeof(int));

    // bind all sockets on all ports
    if (verbose) {
        puts("[+] Done.");
        puts("[+] Trying to create sockets and assign them to any address...");
    }

    for (c = 0; c < number_of_ports - 1; c++) {
        sock_fds[c] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_fds[c] == -1) {
            perror("socket() has failed to create an UDP socket");
            exit(1);
        }
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(*(ports + c));

        if (bind(sock_fds[c], (struct sockaddr *)&serv_addr,
                 sizeof(serv_addr)) == -1) {
            fprintf(stderr,
                    "bind() has failed to assing an address to a socket on "
                    "port %u: %s\n",
                    *(ports + c), strerror(errno));
            exit(1);
        }
    }

    if (verbose) {
        printf("[+] Done, %d sockets were created.\n", c);
        puts("[+] Now trying to catch any incoming data received from any "
             "socket/port.");
    }

    // wait for incoming data on all sockets
    while (1) {
        // creating file descriptor sets
        FD_ZERO(&rfds);
        for (c = 0, max_fd = 0; c < number_of_ports; c++) {
            if (sock_fds[c] == -1) continue;
            FD_SET(sock_fds[c], &rfds);
            max_fd = max(max_fd, sock_fds[c]);
        }

        memset(buffer, 0, sizeof(buffer));
        // select() returns when one of all file descriptors passed are ready
        ready = select(max_fd + 1, &rfds, 0, 0, 0);
        if (ready == -1) {
            perror("select() has failed");
            exit(1);
        }

        /*
         * check all sockets and try to find out
         * which socket is ready to read data from
         */
        for (c = 0; c < number_of_ports; c++) {
            if (sock_fds[c] == -1) continue;
            if (FD_ISSET(sock_fds[c], &rfds)) {
                len = sizeof(client_addr);
                x = recvfrom(sock_fds[c], (char *)buffer, sizeof(buffer) - 1, 0,
                             (struct sockaddr *)&client_addr, &len);
                if (x < 1) {
                    perror("recvfrom() has failed");
                    shutdown(sock_fds[c], SHUT_RDWR);
                    close(sock_fds[c]);
                    sock_fds[c] = -1;
                    continue;
                }

                // filter
                if (filter && x == last_recv_size) goto skip_print;

                // get the time
                curtime = time(NULL);
                /*
                 * we no longer need 'optarg' pointer to parse cmd line,
                 * therefore we'll use it here
                 */
                optarg = ctime(&curtime);
                fprintf(
                    fptr,
                    "\n%sIncoming from: %s\n%s\n"
                    "=====================================================\n",
                    optarg, inet_ntoa(client_addr.sin_addr), buffer);

                if (!daemonize)
                    fprintf(stdout,
                            "\n%sIncoming from: %s\n%s\n"
                            "=================================================="
                            "===\n",
                            optarg, inet_ntoa(client_addr.sin_addr),
                            strtok(buffer, "\n"));

                fflush(fptr);
            skip_print:
                if (recv_alarm) putc('\a', stderr);
                last_recv_size = x; // store received size
            }
        }
    }
    return 0;
}
