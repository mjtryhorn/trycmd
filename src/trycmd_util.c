/*
 * trycmd_util.c -- Various utility functions.
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
#include <stddef.h>  /* size_t. */
#include <stdlib.h>  /* atoi. */

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

char* trycmd_getenv_s(const char* const key) {
    /* Check arguments. */
    assert("Unexpected NULL key" && (key != NULL));

    /* Use the standard getenv function to read the active environment. */
    return getenv(key);
}

int trycmd_getenv_i(const char* const key) {
    const char* const value = trycmd_getenv_s(key);
    return (value != NULL) ? /* key present */ atoi(value)
                           : /* no such key */ 0;
}

/* EOF */
