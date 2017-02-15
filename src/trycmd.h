#ifndef TRYCMD_H_
#define TRYCMD_H_

/*
 * trycmd.h -- Run a command, display a standard result
 *             and pass on any exit status.
 *
 * Author:  M. J. Tryhorn
 * Date:    2017-Feb-02
 * Version: 1.0
 *
 * Copyright 2017.
 * All rights reserved.
 */

#include <stddef.h>  /* size_t. */
#include <stdio.h>   /* FILE. */

/** \file trycmd.h
    \brief Common header for the Try project.
    All public symbols are declared herein.
*/

/** Constant added to the exit status if a subcommand fails with a signal.
 *  We return 128+n where n is the signal value, to match the behaviour of
 *  Bash.
 */
#define TRYCMD_SIGNAL_BASE (128)

/** Options settable by users via the command-line or environment. */
struct trycmd_opts {
    /** If non-zero, spawn all subcommands within an interactive subshell.
     *  Setting this option may be required if the command is an alias, or
     *  similar. Alias commands will likely only be interpretted as proper
     *  commands within the shell if it is interactive.
     */
    int         opt_interactive;

    /** The path to the shell within which all subcommands are to be spawned.
     *  This shell must support the '-i' interactive, '-c' command string and
     *  '--' end-of-options options in order to function correctly.
     */
    char*       opt_shell;

    /** If non-zero, enables verbose application output.
     *  This will print the command being spawned onto stderr.
     */
    int         opt_verbose;

    /** If non-zero, prints application usage information to stdout.
     *  If help is requested then no subcommand will be spawned.
     */
    int         opt_help;

    /** Subcommand argument count.
     *  The length of opt_sub_argv in elements, less any terminating NULL.
     */
    int         opt_sub_argc;

    /** Subcommand argument list.
     *  This list does not require a NULL terminator. */
    char**      opt_sub_argv;
};

/** If non-zero, enables the printing of application diagnostic output. */
extern int      trycmd_debug_enabled;

/** Initialise the Try diagnostic system.
 *  If TRYCMD_DEBUG is present in the environment, and non-zero,
 *  then diagnostics will be enabled.
 */
extern void     trycmd_debug_init(void);

/** Print a single diagnostic message.
 *  @param format A standard printf format string
 */
extern void     trycmd_debug(const char* format, ...);

/** Initialize application strings internationalization support.
 *  Must be called once, at application startup and before any calls
 *  to the _() or N_() functions.
 */
extern void     trycmd_intl_init(void);

/** Translates the given string, to the user's
 *  own language, using the current LOCALE.
 *  @param  s The string to be translated.
 *  @return The translated string, or s if no more
 *          appropriate translation was available.
 */
extern char*    _(char* s);

/** Performs a null translation upon the given string.
 *  This is sibling to the _() translation function. It intentionally does
 *  nothing to the given string and is used to document that strings are not
 *  to be translated (because they are either non-translatable or critical).
 *  @param  s The string to be translated.
 *  @return s unmodified.
 */
inline char*    N_(char* s) {
    /* Intentionally do nothing but return the given string. */
    return s;
}

/** Construct a shell command for use with the standard exec functions.
 *  Converts the given options - shell and subcommand - to  an argument
 *  list for passing to one of the family of exec functions.
 *  @param  opts     Input options defining the shell and subcommand.
 *  @param  buffer   Generic storage to hold argv_out data.
 *  @param  buflen   Length of buffer, in bytes.
 *  @param  argv_out Output command (stored in buffer).
 *  @return The required or used buflen to store the command, in full.
 *          If buflen is less than this required length, then no output
 *          will be written to either buffer or argv_out.
 */
extern size_t   trycmd_make_shell_cmd(const struct trycmd_opts* opts,
                                      void*  buffer,
                                      size_t buflen,
                                      char** argv_out[]);

/** Construct and run a shell command from the given options.
 *  Upon completion of the subcommand (if any), this function will return
 *  its exit status as-if it had been run from a terminal directly. An exit
 *  status of zero indicates success, any other value indicates failure. For
 *  more information on specific error codes, refer to the Bash shell's
 *  documentation on exit status codes, behavior this function duplicates.
 *  @param  opts Options describing the shell and subcommand.
 *  @return The subcommand's result value as-if it had been run from a
 *          terminal, directly.
 */
extern int      trycmd_run_subcommand(const struct trycmd_opts* opts);

/** Print a colorful message for the given subcommand exit status.
 *  If exit_status is zero, this will be interpretted as success.
 *  A non-zero exit_status will be interpretted as failure.
 *  @param  opts Options describing the subcommand.
 *  @param  exit_status The exit status to illustrate.
 */
extern int      trycmd_show_exit_status(const struct trycmd_opts* opts,
                                        int exit_status);

/** Print a given command-line argument with quoting as necessary.
 *  @param  arg The argument to be printed as null-terminated string.
 *  @param  os  The destination stream (stdout, stderr).
 */
extern void     trycmd_pretty_print_arg(const char* arg, FILE* os);

/** Print a given argument list to standard output.
 *  @param prefix A printed label for the argument list, preceding it.
 *  @param argv   The argument list to be printed, terminated by NULL.
 */
extern void     trycmd_print_argv(const char* prefix, char* argv[]);

/** Print this application's usage information to the given stream.
 *  This is given in the form of a human-readable message.
 *  @param  os The destination stream (stdout, stderr).
 */
extern void     trycmd_print_usage(FILE* os);

/** Read and parse the given option strings into a trycmd_opts structure.
 *  Command-line options:
 *    1. \-i \-\-interactive
 *       Force the subshell to behave as-if it was an interactive session.
 *       This option may be necessary if your command is, or relies upon,
 *       aliases.
 *    2. \-v \-\-verbose
 *       Enable verbose output.
 *    3. \-h \-\-help
 *       Display a usage message on stdout and exit successfully.
 *
 *  Environment options:
 *    1. TRY_INTERACTIVE=[01]
 *       Set to 1 to always run commands in an interactive sub-shell.
 *
 *  @param  argc     The length of argv in elements.
 *  @param  argv     The command-line arguments, in the order presented
 *                   by the user (need not be NULL terminated).
 *  @param  opts_out The options translated into a trycmd_opts structure.
 *  @return 0 on success, -1 on failure.
 */
extern int      trycmd_read_options(int argc, char* argv[],
                                    struct trycmd_opts* opts_out);

/** Align the given size up, to fall on the next aligned boundary.
 *  If sz is already aligned, then its value will not be changed.
 *  @param  sz        The size value to be aligned.
 *  @param  alignment The alignment.
 *  @return sz rounded-up to the next multiple of alignment.
 */
extern size_t   trycmd_align_sz(size_t sz, size_t alignment);

/** Align the given pointer up, to fall on the next aligned boundary.
 *  If ptr is already aligned, then its value will not be changed.
 *  @param  ptr       The pointer to be aligned.
 *  @param  alignment The alignment.
 *  @return ptr rounded-up to the next multiple of alignment.
 */
extern void*    trycmd_align_ptr(void* ptr, size_t alignment);

/** Find and return a value, from the current environment, for a specific key.
 *  @param  key The key string to find.
 *  @return The found value or NULL if no such key is present.
 */
extern char*    trycmd_getenv_s(const char* key);

/** Find and return a value, from the current environment, for a specific key.
 *  Before returning, this value is converted to an integer using atoi().
 *  @param  key The key string to find.
 *  @return The found value as an integer or 0 if no such key is present.
 */
extern int      trycmd_getenv_i(const char* key);

/** Application entry point. Runs 'try' for the given command-line options.
 *  @param  argc The length of argv in elements.
 *  @param  argv     The command-line arguments, in the order presented
 *                   by the user (need not be NULL terminated).
 *  @return 0 on success, non-zero on failure. If a subcommand was run,
 *          then the result is the exit status of this subcommand.
 */
extern int      trycmd_main(int argc, char* argv[]);

#endif /* TRYCMD_H_ */
