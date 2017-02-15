/*
 * trycmd_opts.c -- Options parsing.
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
#include <stdio.h>   /* fprintf, fputs, fputc, fflush. */
#include <getopt.h>  /* struct option. */
#include <unistd.h>  /* getopt_long. */

void trycmd_print_usage(FILE* const os) {
    /* Statically declare all command line and environment options. */
    struct option {
        const char* key;
        const char* value;
    };
    const struct option cmdopts[] = {
        { N_("-i --interactive"), _("Execute the command in an interactive subshell.") },
        { N_("-v --verbose"),     _("Verbose output (echos the command being run).")   },
        { N_("-h --help"),        _("Show this message.")                              },
        { N_("--"),               _("End of options.")                                 },
        { N_("COMMAND"),          _("The command to run.")                             },
        { N_("ARG_[1..N]"),       _("Arguments to the command.")                       }
    };
    const struct option envopts[] = {
        { N_("TRY_INTERACTIVE=1"),     _("Always execute commands in an interactive subshell.") },
        { N_("SHELL=" DEF_SHELL_PATH), _("The shell to use when executing the command.") },
    };
    size_t idx;

    /* Print all command line options, first to last. */
    fputs(_("Usage: try [-ivh] [--] [COMMAND] [ARG_1] ... [ARG_N]\n"), os);
    fprintf(os, _("Where:       %-20s  %s\n"), cmdopts[0].key, cmdopts[0].value);
    for (idx = 1; idx < sizeof(cmdopts) / sizeof(cmdopts[0]); ++idx) {
        fprintf(os, _("             %-20s  %s\n"), cmdopts[idx].key, cmdopts[idx].value);
    }
    fputc('\n', os);

    /* Print all environment options. */
    fprintf(os, _("Environment: %-20s  %s\n"), envopts[0].key, envopts[0].value);
    for (idx = 1; idx < sizeof(envopts) / sizeof(envopts[0]); ++idx) {
        fprintf(os, _("             %-20s  %s\n"), envopts[idx].key, envopts[idx].value);
    }
    fputc('\n', os);

    /* Complete the message by flushing its content. */
    fflush(os);
}

int trycmd_read_options(const int argc, char* argv[],
                        struct trycmd_opts* const opts_out) {
    const char shortopts[] = "+ivh";
    const struct option longopts[] = {
        { N_("interactive"), 0, NULL, 'i' },
        { N_("verbose"),     0, NULL, 'v' },
        { N_("help"),        0, NULL, 'h' },
        { NULL,              0, NULL, 0   }
    };
    struct trycmd_opts opts_out_tmp = { 0 };
    extern int optind;
    int opt;

    /* Check arguments. */
    assert("Unexpected negative argc" && (argc >= 0));
    assert("Unexpected NULL argv" && (argv != NULL));
    assert("Unexpected NULL opts_out" && (opts_out != NULL));

    /*
     * Reset the option parser's state.
     */
    optind = 1;

    /*
     * Read all environment options first.
     * These are lower precedence than options
     * specified upon the command line directly.
     */
    opts_out_tmp.opt_interactive = !!trycmd_getenv_i(N_("TRY_INTERACTIVE"));
    opts_out_tmp.opt_shell       = trycmd_getenv_s(N_("SHELL"));

    /*
     * Read all standard "-X" and "--X" options.
     * These are higher precedence than options
     * specified within the environment.
     */
    while ((opt = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
        switch (opt) {
            case 'i':  /* Interactive. */
                opts_out_tmp.opt_interactive = 1;
                break;
            case 'v':  /* Verbose. */
                opts_out_tmp.opt_verbose = 1;
                break;
            case 'h':  /* Help me! */
                opts_out_tmp.opt_help = 1;
                break;

            default:   /* Unexpected option. */
                assert("Unexpected option" && 0);
            case '?':  /* Invalid option. */
                return -1;
        }
    }

    /* All remaining options form the subcommand. */
    opts_out_tmp.opt_sub_argc = argc - optind;
    opts_out_tmp.opt_sub_argv = &argv[optind];

    /* Apply any defaults. */
    if (opts_out_tmp.opt_shell == NULL || opts_out_tmp.opt_shell[0] == '\0') {
        opts_out_tmp.opt_shell = DEF_SHELL_PATH;
    }

    /* Copy the result and return 0 for success. */
    *opts_out = opts_out_tmp;
    return 0;
}

/* EOF */
