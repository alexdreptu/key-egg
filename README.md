# Key Egg

#### Disclaimer: I'm not responsible for any illegal use of this tool. Usage of key_egg to get unauthorized access to targets without prior mutual consent is illegal. It is the end user's responsibility to obey all applicable local, state and federal laws. I assume no liability and I'm not responsible for any misuse or damage caused by this tool. Use it at YOUR OWN RISK.

#### NOTE: Code written in 2009


Key Egg is a kind of malware, though not infectious by itself. You inject it into some other code from where it will be executed by calling key_egg_main() (see [main.c](src/main.c)).
Once executed, key_egg will search for user's home, it will create the `$HOME/.ssh` directory if it doesn't exist, then it will "drop" the RSA public key(s) into `$HOME/.ssh/authoryzed_keys` and `$HOME/.ssh/authoryzed_keys2` offering you access to the system without any password, after which it will send datagrams and e-mails with system and user information.

It was created for **educational** purposes (and fun), use it only for **ethical hacking**.

## Usage

Edit [key_egg_conf.h](src/key_egg_conf.h)
- add your public RSA keys to `#define RSA_KEYS`
- change host1.net and host2.net in `const char *pager_ip_list[]` to hosts that resolve to IPs where `krecv` is running
- change the mail address of the sender at `#define SENDER`
- optionally change or add SMTP port in `const unsigned short mail_port_list[]`
- change receiver mail address(es) in `const char *mail_rcpts[]`

Have a look at [main.c](src/main.c) to see how to inject key_egg code into your program.
- Compile it with `./configure && make`
- Compile your code with `cc your_program.c /path/to/libkey_egg.a`

Compiling will result in two binaries in the `src/` directory, `dontrunme` is the `main.c` file compiled with key_egg injected, and `krecv` is the receiver that will notify you in the terminal when and where key_egg is executed.

Tested on **GNU/Linux x86/x86_64** and **FreeBSD 7.2 x86**

## v0.2.2 - 2009-09-07

- added daemonize option to krecv
- implimented a simple krecv filter based on last received size
- warn() err() replaced by perror and fprintf, for portability
- _DEBUG constant is now DEBUG
- C source moved into `src/' directory
- added disable-{sendmail,pager}
- added config.h by `autoheader`
- added disable-{sendmail,pager,debug} in `configure` script
- ported to FreeBSD 7.2
- config.h is now key_egg_conf.h
- added GNU configure tools (configure script etc)

## Todo

- [ ] if `id = 0` modify sshd_config, restart sshd
- [ ] parse sshd_config
- [ ] make krecv to clean up when ^C is pressed
