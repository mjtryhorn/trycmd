#!/bin/sh

#
# file:      bootstrap.sh
# brief:     Run autoconf tools.
#
# author:    M. J. Tryhorn
# date:      2017-Feb-26
# version:   1.0
# copyright: MIT License (see LICENSE).
#

# Enable option: exit on error.
set -e

# Perform autoconf bootstrap.
echo "Running autoconf tools ..."
aclocal
autoconf
autoheader
automake --add-missing

# Instruct the user on what to do next.
echo "Autoconf tools ran successfully."
echo "To build and install, run: ./configure"
echo "                           make"
echo "                           sudo make install"

## EOF ##
