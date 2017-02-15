# what flags you want to pass to the C compiler & linker
AM_CFLAGS = -Wall \
            -Wextra \
            -pedantic \
            -std=c99 \
            -DLOCALEDIR="\"@localedir@\"" \
            -D_POSIX_C_SOURCE=200809L
AM_LDFLAGS =

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
noinst_LIBRARIES = libtrycmd.a
bin_PROGRAMS = try try_test

libtrycmd_a_SOURCES = trycmd_opts.c \
                      trycmd_debug.c \
                      trycmd_intl.c \
                      trycmd_subcmd.c \
                      trycmd_util.c \
                      trycmd_main.c
try_SOURCES = trycmd.c
try_LDADD = libtrycmd.a

try_test_SOURCES = trycmd_test.c
try_test_LDADD = libtrycmd.a
