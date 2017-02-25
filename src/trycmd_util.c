/**
 * \file      trycmd_util.c
 * \brief     Various utility functions.
 *
 * \author    M. J. Tryhorn
 * \date      2017-Feb-23
 * \version   1.0
 * \copyright MIT License (see LICENSE).
 */

#include "trycmd_config.h"
#include "trycmd.h"
#include <assert.h>  /* assert. */
#include <stddef.h>  /* size_t. */
#include <stdlib.h>  /* atoi. */
#include <string.h>  /* strchr. */
#include <ctype.h>   /* isalnum. */
#include <stdio.h>   /* fileno, fputc, fputs, fprintf, fwrite. */
#include <unistd.h>  /* isatty. */

size_t trycmd_align_sz(const size_t sz, const size_t alignment) {
    /* Check arguments. */
    assert("Unexpected zero alignment" && (alignment != 0));

    /* Align the given size up, to fall on the next aligned boundary. */
    size_t offs = sz % alignment;
    size_t result;
    if (offs != 0) {
        offs = alignment - offs;
    }
    result = sz + offs;

    /* Print then return. */
    trycmd_debug("trycmd_align_sz(sz=%zu, alignment=%zu) == %zu\n",
                 sz, alignment, result);
    return result;
}

void* trycmd_align_ptr(void* const ptr, const size_t alignment) {
    /* Check arguments. */
    assert("Unexpected zero alignment" && (alignment != 0));

    /* Align the given pointer up, to fall on the next aligned boundary. */
    size_t offs = (size_t)ptr % alignment;
    void* result;
    if (offs != 0) {
        offs = alignment - offs;
    }
    result = (char*)ptr + offs;

    /* Print then return. */
    trycmd_debug("trycmd_align_ptr(ptr=%p, alignment=%zu) == %p\n",
                 ptr, alignment, result);
    return result;
}

char* trycmd_getenv_s(const char* const key, char* const def) {
    char* result;
    
    /* Check arguments. */
    assert("Unexpected NULL key" && (key != NULL));

    /* Use the standard getenv function to read the active environment. */
    result = getenv(key);

    /* If no such key is present, apply the given default (may be NULL). */
    if (result == NULL) {
        result = def;
    }
    return result;
}

int trycmd_getenv_i(const char* const key, const int def) {
    const char* const value = trycmd_getenv_s(key, NULL);
    return (value != NULL) ? /* key present */ atoi(value)
                           : /* no such key */ def;
}

int trycmd_is_color_enabled(const enum trycmd_color c, FILE* const os) {
    /* Check arguments. */
    assert("Unexpected NULL os" && (os != NULL));

    switch (c) {
        case trycmd_color_never:
            /* Never use color in output. */
            return 0;
        case trycmd_color_always:
            /* Always use color in output. */
            return 1;
        case trycmd_color_auto:
            /*
             * Color is enabled if and only if os is both:
             * 1. backed by a file descriptor and,
             * 2. connected to a terminal (TTY).
             */
            return isatty(fileno(os));
        default:
            /* Unrecognised */
            assert("Unrecognised trycmd_color value" && 0);
            return 0;
    }
}

int trycmd_needs_quoting(const char c) {
    switch (c) {
        case '_':
        case '-':
        case '.':
        case '/':
            // Special, allowed chars.
            return 0;
        default:
            // Only needs quoting if outside range [0-9a-zA-Z].
            return !isalnum(c);
    }
}

void trycmd_pretty_print_arg(const char* const arg, FILE* const os) {
    int needs_quoting = 0;
    const char* apos;
    const char* bpos;

    /* Check arguments. */
    assert("Unexpected NULL arg" && (arg != NULL));
    assert("Unexpected NULL os" && (os != NULL));

    /*
     * Search for any non-alphanumeric characters
     * that would necessitate quoting.
     */
    for (apos = arg; *apos && needs_quoting == 0; ++apos) {
        needs_quoting = trycmd_needs_quoting(*apos);
    }

    if (!needs_quoting) {
        /* This argument requires no quoting. */
        fputs(arg, os);
    } else {
        /*
         * This argument is not completely alpha-numeric.
         * Print the argument within single quotes,
         * escaping any internal quotes as found.
         */
        apos = arg;
        bpos = strchr(apos, '\'');
        while (bpos != NULL) {
            /* Print everything before the quote. */
            if (apos != bpos) {
                fputc('\'', os);
                fwrite(apos, sizeof(*apos), bpos - apos, os);
                fputc('\'', os);
            }

            /* Now the quote itself, escaped. */
            fprintf(os, "\\\'");

            /* Search for the next quote. */
            apos = bpos + 1;
            bpos = strchr(apos, '\'');
        }

        /* If any argument text remains, print it within quotes. */
        if (*apos) {
            fprintf(os, "\'%s\'", apos);
        }
    }
}

void trycmd_print_argv(const char* const prefix, char* argv[], FILE* const os) {
    /* Check arguments. */
    assert("Unexpected NULL prefix" && (prefix != NULL));
    assert("Unexpected NULL argv" && (argv != NULL));
    assert("Unexpected NULL os" && (os != NULL));

    /* Print the prefix. */
    fputs(prefix, os);

    /* Print any arguments. */
    for (; *argv != NULL; ++argv) {
        fputc(' ', os);
        trycmd_pretty_print_arg(*argv, os);
    }
    
    /* Finish with a newline. */
    fputc('\n', os);
}

/* EOF */
