/**
 * \file      trycmd_opts.c
 * \brief     Command-line options parsing.
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
#include <string.h>  /* strcmp. */
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
        { N_("-i, --interactive"), _("Execute the command in an interactive subshell.")            },
        { N_("--color[=WHEN],"),   _("Color the result according to command's exit status.")       },
        { N_("--colour[=WHEN]"),   _("WHEN is 'always' (default if omitted), 'never', or 'auto'.") },
        { N_("-v, --verbose"),     _("Verbose output (echos the command being run).")              },
        { N_("-h, --help"),        _("Show this message.")                                         },
        { N_("--"),                _("End of options.")                                            },
        { N_("COMMAND"),           _("The command to run.")                                        },
        { N_("ARG"),               _("Arguments to the command.")                                  },
    };
    const struct option envopts[] = {
        { N_("TRY_INTERACTIVE=1"),     _("Always execute commands in an interactive subshell.") },
        { N_("TRY_COLOR=WHEN"),        _("Add color to the result (see '--color').") },
        { N_("SHELL=" DEF_SHELL_PATH), _("The shell to use when executing the command.") },
    };
    size_t idx;

    /* Print a standard header. */
    fputs(_("Usage: try [OPTION]... COMMAND [ARG]...\n"
            "Run COMMAND to completion then show its result in a standard and clear form.\n"
            "Example: try -i wget www.ietf.org/rfc/rfc2324.txt  # Download an RFC.\n"), os);

    /* Print all command line options, first to last. */
    fputs(_("\nOptions:\n"), os);
    for (idx = 0; idx < sizeof(cmdopts) / sizeof(cmdopts[0]); ++idx) {
        fprintf(os, _("  %-17s  %s\n"), cmdopts[idx].key, cmdopts[idx].value);
    }

    /* Print all environment options. */
    fputs(_("\nEnvironment:\n"), os);
    for (idx = 0; idx < sizeof(envopts) / sizeof(envopts[0]); ++idx) {
        fprintf(os, _("  %-17s  %s\n"), envopts[idx].key, envopts[idx].value);
    }
    fputc('\n', os);

    /* Complete the message by flushing its content. */
    fflush(os);
}

int trycmd_read_options(const int argc, char* argv[],
                        struct trycmd_opts* const opts_out) {
    const char* const shortopts = N_("+ivh");
    const struct option longopts[] = {
        { N_("interactive"), no_argument,       NULL, 'i' },
        { N_("color"),       optional_argument, NULL, 'C' },
        { N_("colour"),      optional_argument, NULL, 'C' },
        { N_("verbose"),     no_argument,       NULL, 'v' },
        { N_("help"),        no_argument,       NULL, 'h' },
        { NULL,              0,                 NULL, 0   }
    };
    struct trycmd_opts opts_out_tmp = { 0 };
    const char* opt_color_when;
    extern char* optarg;
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
    opts_out_tmp.opt_interactive = !!trycmd_getenv_i(N_("TRY_INTERACTIVE"), 0);
    opts_out_tmp.opt_shell = trycmd_getenv_s(N_("SHELL"), DEF_SHELL_PATH);
    opt_color_when = trycmd_getenv_s(N_("TRY_COLOR"), NULL);

    /* Attempt to parse any TRY_COLOR=WHEN environment setting. */
    if (opt_color_when != NULL) {
        if (trycmd_parse_when(opt_color_when, &opts_out_tmp.opt_color) != 0) {
            /*
             * Parse failure. As this was requested via the environment,
             * report it but continue (to avoid environment key conflicts).
             */
            trycmd_debug("trycmd_read_options: unrecognised"
                         " TRY_COLOR value: \"%s\"\n",
                         opt_color_when);
        }
    }

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
            case 'C':  /* Color[=WHEN]. */
                if (trycmd_parse_when(optarg, &opts_out_tmp.opt_color) != 0) {
                    /*
                     * Parse failure. As this was directly requested on
                     * the command-line, report the error and fail fast.
                     */
                    trycmd_debug("trycmd_read_options: unrecognised"
                                 " --color value: \"%s\"\n",
                                 optarg);
                    return -1;
                }
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

    /* Copy the result and return 0 for success. */
    *opts_out = opts_out_tmp;
    return 0;
}

int trycmd_parse_when(const char* const when, enum trycmd_color* const out) {
    const struct when_opt {
        const char* key;
        enum trycmd_color value;
    } whenopts[] = {
        { N_("auto"),   trycmd_color_auto   },
        { N_("always"), trycmd_color_always },
        { N_("never"),  trycmd_color_never  },
    };
    size_t idx;

    /* Check arguments. */
    assert("Unexpected NULL out" && (out != NULL));

    if (when == NULL) {
        /* Coloured output requested but no WHEN specified. */
        *out = trycmd_color_always;
        return 0;
    } else {
        /* Convert the given WHEN string to an enumeration value. */
        for (idx = 0; idx < sizeof(whenopts) / sizeof(whenopts[0]); ++idx) {
            if (strcmp(when, whenopts[idx].key) == 0) {
                *out = whenopts[idx].value;
                return 0;
            }
        }
    }

    /* Unrecognised WHEN string. */
    return -1;
}

/* EOF */
