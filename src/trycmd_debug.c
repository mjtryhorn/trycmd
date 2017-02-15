/*
 * trycmd_debug.c -- Diagnostic functions.
 *
 * Author:  M. J. Tryhorn
 * Date:    2017-Feb-02
 * Version: 1.0
 *
 * Copyright 2017.
 * All rights reserved.
 */

#include "trycmd_config.h"
#include "trycmd.h"
#include <assert.h>  /* assert. */
#include <stdio.h>   /* vfprintf, stderr. */
#include <stdarg.h>  /* va_list, va_start, va_end. */

int trycmd_debug_enabled = 0;

void trycmd_debug_init(void) {
    /*
     * Enable or disable diagnostic output according to the given environment.
     * If TRY_DEBUG is present and non-zero, then enable diagnostic message
     * output.
     */
    trycmd_debug_enabled = !!trycmd_getenv_i(N_("TRY_DEBUG"));
}

void trycmd_debug(const char* const format, ...) {
    /* Check arguments. */
    assert("Unexpected NULL format" && (format != NULL));

    /* Print the given message if and only if debug is enabled. */
    if (trycmd_debug_enabled) {
        va_list ap;
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        va_end(ap);
    }
}

/* EOF */
