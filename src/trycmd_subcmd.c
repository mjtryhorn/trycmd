/**
 * \file      trycmd_subcmd.c
 * \brief     Spawn subcommands and clearly show results.
 *
 * \author    M. J. Tryhorn
 * \date      2017-Feb-23
 * \version   1.0
 * \copyright MIT License (see LICENSE).
 */

#include "trycmd_config.h"
#include "trycmd.h"
#include <assert.h>        /* assert. */
#include <ctype.h>         /* isalnum. */
#include <stddef.h>        /* size_t. */
#include <stdio.h>         /* snprintf, fprintf, fputc, fputs, stderr. */
#include <stdlib.h>        /* EXIT_SUCCESS, EXIT_FAILURE, abort. */
#include <string.h>        /* strncpy, strnlen. */
#include <sys/types.h>     /* pid_t. */
#include <sys/wait.h>      /* waitpid. */
#include <linux/limits.h>  /* PATH_MAX. */
#include <unistd.h>        /* fork, execv. */

/* Check for required defined values. */
#if !defined(HAVE_FORK)
#  error Missing required function 'fork'.
#endif

#if !defined(HAVE_STRNLEN)
#  error Missing required function 'strnlen'.
#endif

size_t trycmd_make_shell_cmd(const struct trycmd_opts* const opts,
                             void*        buffer,
                             const size_t buflen,
                             char**       argv_out[]) {
    /* This implementation for Bash requires at most 6 additional argv elements:
     *   1. for the shell's path,
     *   2. '-i' for interactive,
     *   3. '-c' to request the running of a subcommand,
     *   4. '--' to mark the end of options to the shell
     *      (included for command-line safety),
     *   5. for the command to be run followed by the magic shell variable "$@",
     *   6. for a NULL, marking argv's end.
     */
    const char shell_arg0_ext[]        = "\"$@\"";
    const char shell_opt_interactive[] = "-i";
    const char shell_opt_command[]     = "-c";
    const char shell_opts_end[]        = "--";
    const char eol_marker              = (char)0xff;

    const int argc_required      = 5
                                 + !!opts->opt_interactive
                                 + opts->opt_sub_argc;
    const size_t shell_sz        = strnlen(opts->opt_shell, PATH_MAX) + 1;
    const size_t shell_arg0_sz   = strnlen(opts->opt_sub_argv[0], PATH_MAX)
                                 + sizeof(shell_arg0_ext)
                                 + 1;
    const size_t argc_sz         = sizeof(char*) * argc_required;
    const size_t min_buflen      = argc_sz
                                 + shell_sz
                                 + sizeof(shell_opt_interactive) * !!opts->opt_interactive
                                 + sizeof(shell_opt_command)
                                 + sizeof(shell_opts_end)
                                 + shell_arg0_sz
                                 + 1;
    const size_t required_buflen = trycmd_align_sz(min_buflen, sizeof(char*));

    /* If sufficient buffer space is available... */
    trycmd_debug("trycmd_make_shell_cmd: buflen=%zu required_buflen=%zu\n",
                 buflen, required_buflen);
    if (buflen >= required_buflen) {
        int idx;
        int result;
        char* aligned_buf;
        char** argv_pos;
        char* buffer_pos;

        /* Check arguments. */
        assert(buffer != NULL);
        assert(argv_out != NULL);
        assert(min_buflen != 0);

        /* Gain a pointer into the buffer at the first char* aligned boundary. */
        aligned_buf = trycmd_align_ptr(buffer, sizeof(char*));

        /* Place an EOL marker, to search for after building the command. */
        aligned_buf[min_buflen - 2] = eol_marker;
        aligned_buf[min_buflen - 1] = eol_marker;
        buffer_pos = aligned_buf;

        /* Build the command. */
        *argv_out   = (char**)buffer_pos;
        argv_pos    = &(*argv_out)[0];
        buffer_pos += argc_sz;

        /* Shell. */
        *argv_pos++ = buffer_pos;
        strncpy(buffer_pos, opts->opt_shell, shell_sz);
        buffer_pos += shell_sz;

        /* Interactive (-i, optional). */
        if (opts->opt_interactive) {
            *argv_pos++ = buffer_pos;
            strncpy(buffer_pos, shell_opt_interactive, sizeof(shell_opt_interactive));
            buffer_pos += sizeof(shell_opt_interactive);
        }

        /* Command (-c). */
        *argv_pos++ = buffer_pos;
        strncpy(buffer_pos, shell_opt_command, sizeof(shell_opt_command));
        buffer_pos += sizeof(shell_opt_command);

        /* End of options (--). */
        *argv_pos++ = buffer_pos;
        strncpy(buffer_pos, shell_opts_end, sizeof(shell_opts_end));
        buffer_pos += sizeof(shell_opts_end);

        /* Command script. */
        *argv_pos++ = buffer_pos;
        result = snprintf(buffer_pos, shell_arg0_sz, "%s %s",
                          opts->opt_sub_argv[0], shell_arg0_ext);
        assert("Unexpected result from snprintf" && (result > 0));
        assert("Unexpected result from snprintf" &&
               ((size_t)result == shell_arg0_sz - 1));
        buffer_pos[result] = '\0';

        /* Copy all remaining arguments into the shell's positional list. */
        for (idx = 0; idx < opts->opt_sub_argc; ++idx) {
            *argv_pos++ = opts->opt_sub_argv[idx];
        }

        /* End of options. */
        *argv_pos = NULL;

        /* Check that the EOL marker remained intact. */
        assert("Unexpected buffer underrun" &&
               (aligned_buf[min_buflen - 2] == '\0'));
        assert("Unexpected buffer overrun" &&
               (aligned_buf[min_buflen - 1] == eol_marker));
    }

    /* Return the buffer space required or used. */
    return required_buflen;
}

int trycmd_run_subcommand(const struct trycmd_opts* const opts) {
    /* Build the subcommand. */
    const size_t req_buflen = trycmd_make_shell_cmd(opts, NULL, 0, NULL);
    int result = 255;

    /* Check arguments. */
    assert("Unexpected NULL opts" && (opts != NULL));

    if (req_buflen > 0) {
        pid_t child_pid;
        pid_t wait_result;
        char** argv = NULL;
        char dyn_buffer[req_buflen];
        const size_t dyn_buflen = trycmd_make_shell_cmd(opts,
                                                        dyn_buffer,
                                                        req_buflen,
                                                        &argv);
        assert("Unexpected change in req_buflen" && req_buflen == dyn_buflen);
        assert("Unexpected NULL argv" && (argv != NULL));
        assert("Unexpected NULL argv[0]" && (argv[0] != NULL));
        (void) dyn_buflen;

        /* Print the subcommand if requested. */
        if (opts->opt_verbose || trycmd_debug_enabled) {
            trycmd_print_argv("try:", argv, stderr);
            fputc('\n', stderr);
        }

        /* Spawn the subprocess then wait for it to finish. */
        trycmd_debug("trycmd_run_subcommand: spawning %s\n", argv[0]);
        if ((child_pid = fork()) == 0) {
            /* Child process. */
            execv(argv[0], argv);
            assert("Unexpected return from execv" && 0);
            abort();
        } else {
            /* Parent process. */
            trycmd_debug("trycmd_run_subcommand: waitpid(%d)\n", child_pid);
            wait_result = waitpid(child_pid, &result, 0);
            trycmd_debug("trycmd_run_subcommand: child status is %d\n", result);
            assert("Unexpected result from waitpid" && (wait_result == child_pid));
            (void) wait_result;
        }

        /*
         * Child ends and parent process continues.
         * Convert the result to a value which can be
         * returned from main() without modification.
         */
        if (WIFEXITED(result)) {
            /* Exited normally (via exit(n)). */
            result = WEXITSTATUS(result);
        } else if (WIFSIGNALED(result)) {
            /*
             * Exited due to a signal. Use K+n where K is a constant and
             * n is the signal value, to match the behaviour of Bash.
             */
            result = TRYCMD_SIGNAL_BASE + WTERMSIG(result);
        } else {
            /* Exited for another reason. Use 255 as a catch-all. */
            result = 255;
        }
    }

    /* All done. */
    trycmd_debug("trycmd_run_subcommand: returning %d\n", result);
    return result;
}

int trycmd_show_exit_status(const struct trycmd_opts* const opts,
                            const int exit_status,
                            FILE* os) {
    /* Make a dividing line to separate the result from child messages. */
    const char* const divider_line = _("=========================="
                                       "=========================="
                                       "==========================");
    /* Check arguments. */
    assert("Unexpected NULL opts" && (opts != NULL));
    assert("Unexpected NULL os" && (os != NULL));

    /* Print a prologue. */
    if (exit_status == EXIT_SUCCESS) {
        fprintf(os, _("\033[1;32m%s\nSuccess:\033[0m"), divider_line);
    } else {
        fprintf(os, _("\033[1;31m%s\nFailed (status=%d):\033[0m"),
                divider_line, exit_status);
    }

    /* Print the command itself. */
    trycmd_print_argv(N_(""), opts->opt_sub_argv, os);

    /* Print an epilogue. */
    if (exit_status == EXIT_SUCCESS) {
        fprintf(os, _("\n\033[1;32m%s\033[0m\n"), divider_line);
    } else {
        fprintf(os, _("\n\033[1;31m%s\033[0m\n"), divider_line);
    }
    return exit_status;
}

int trycmd_needs_quoting(const char c) {
    switch (c) {
        case '_':
        case '-':
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
}

/* EOF */
