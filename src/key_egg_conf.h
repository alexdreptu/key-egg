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

#ifdef _KEYS_
// RSA Public Keys - delete these RSA keys and replace them with your own
#define RSA_KEYS                                                               \
    "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQCfzrIgiT/B99BxyzC9pvcj5c5FlvDk2Tg6xU\
    fxtFx1pLnhaa/AmhYWKxK1qVowf2GhT9CZ4ICacXUTQTfnE2s1vmK+qm4j3rDJQDviBpCOuFgYo\
    juibQpygA8oxJYZdBMRr0i3cSRzJNmu60IiEO/D6aSXcZGENSmnWD3kW5Vhpc0+LROHmPOuUu52\
    6y/K1SbrPhJWTaEk8lLxgvudfWV7+quXd3zecN1CEbrS8scouuXHzBZD3dstBQYpZP+BP7xbI/W\
    J3mtGpJoGgmeRQTfnpVg0qUqY5f6PbMNefZsLicdRgXS1OWoqb1HQb/S1jmo8lmGTwDM3KqdCPd\
    dWVDcX user@host\n\
    ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQC2hGnlTp50kEhiBMCfY/uivDAUWrByIA8r+Cy\
    9OLzRPLu+WiqqKUCNGpRBoA9mBAORKdMc4kot6HKKJQCQggHe6kWKZFxmSD0JWuOXzaa20sfPey\
    N1Cak0fMpmKn/N7mqqlbqwIMcy2e1gB0zs0C4XcfiZfDUGmQWKcA8SmIqi1Z53Ss0PIn71JArqw\
    elKLdJID8/rmBpPPzg7cOSia5Pk90yZkmvGZuJPTYqZaI++F2Q5vBCYluoYLlSGOJJiJtgLyIjw\
    FKoWAeWj/8+s213eQ6QjAt2eWOJoHFZGGfhvR5j3BkZJbSezBqtWBlA3iYtrkLDPSWMRwqzL2xl\
    4FSA3 user@host"
#endif // _KEYS_

#ifdef _PAGER_
// settings for pager notify
const char *pager_ip_list[] = {
    "host1.net", "host2.net", 0 // don't delete me, but change me
};

const unsigned short pager_port_list[] = {
    28960, 30219, 0 // don't delete me
};
#endif // _PAGER_

#ifdef _MAIL_
// settings for mail notify
#define TIMEOUT 2                   // smtp connect timeout (secs)
#define SENDER "info_bot@gmail.com" // who sends the mails

const char *mail_host_list[] = {
    "127.0.0.1", "0.0.0.0", 0 // don't delete me
};

const unsigned short mail_port_list[] = {
    25, // standard smtp port
    0   // don't delete me
};

const char *mail_rcpts[] = {
    "receiver_address@gmail.com", 0 // don't delete me, but change me
};
#endif // _MAIL
