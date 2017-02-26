# trycmd
A clear and consistent message to inform you whether a command succeeded or
failed.

Trycmd runs a given command then, on the completion of said command, prints
a clear message stating whether the whether the command succeeded or failed.
This may be useful when running commands that report their success or failure
through status codes, where their on-screen notification (if any) is too
subtle. Trycmd is not subtle.

To create automake build files, either use bootstrap:
- <code>$ ./bootstrap.sh</code>

or perform bootstrap's steps manually:
- <code>$ aclocal</code>
- <code>$ autoconf</code>
- <code>$ autoheader</code>
- <code>$ automake --add-missing</code>

Now you should have a configure and all required makefiles.

To build and install:
- <code>$ ./configure</code>
- <code>$ make</code>
- <code>$ sudo make install</code>

To use:
- <code>$ try true   # success.</code>
- <code>$ try false  # failure.</code>
- <code>$ try wget www.ietf.org/rfc/rfc2324.txt  # a real command.</code>
- <code>$ try --color=auto make  # a colorful software build.</code>

For help:
- <code>$ try -h  # show usage.</code>
- <code>$ man try  # manual page.</code>
