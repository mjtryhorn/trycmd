/*
 * trycmd_main.c -- Entry point for try.
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
#include <stdlib.h>  /* EXIT_SUCCESS, EXIT_FAILURE. */
#include <stdio.h>   /* stdout. */

/* Application entry point. */
int trycmd_main(const int argc, char* argv[]) {
    struct trycmd_opts opts;
    int result;

    /* Perform all common application initialization. */
    trycmd_debug_init();
    trycmd_intl_init();

    /* Read arguments. */
    if (trycmd_read_options(argc, argv, &opts) != 0
        || opts.opt_sub_argc == 0
        || opts.opt_help) {
        /* Either: 1. one or more options is invalid, or
         *         2. no subcommand has been given, or
         *         3. the user has explicitly requested help.
         * Show a usage message.
         */
        trycmd_print_usage(stdout);
        result = (opts.opt_help) ? EXIT_SUCCESS   /* Help was requested. */
                                 : EXIT_FAILURE;  /* Help is required. */
    } else {
        /* Prepare and run the subcommand. */
        result = trycmd_run_subcommand(&opts);

        /* Show a result message. */
        result = trycmd_show_exit_status(&opts, result);

        /* Pass the child's result out without modification. */
        trycmd_debug("try: exiting with status %d\n", result);
    }
    return result;
}

/* EOF */
