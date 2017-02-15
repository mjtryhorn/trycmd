/*
 * trycmd.c -- Run a command, display a standard result
 *             and pass on any exit status.
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

/* Application entry point. */
int main(int argc, char* argv[]) {
    return trycmd_main(argc, argv);
}

/* EOF */
