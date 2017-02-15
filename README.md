# trycmd
A colorful message to inform you whether a command succeeded or failed.

Trycmd runs a given command then, on the completion of said command, prints
a colorful message stating whether the whether the command succeeded or
failed. This may be useful when running commands that report their success
or failure through status codes, where their on-screen notification (if any)
is too subtle. Trycmd is not subtle.

To create automake build files:
- $ aclocal
- $ autoconf
- $ autoheader
- $ automake --add-missing
Now you should have a configure and all required makefiles.

To build and install:
- $ ./configure
- $ make
- $ sudo make install

To use:
- $ try true   # success.
- $ try false  # failure.
- $ try wget www.ietf.org/rfc/rfc2324.txt  # a real command.
