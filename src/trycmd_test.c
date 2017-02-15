/*
 * trycmd_test.c -- Comprehensive automated test suite for trycmd.
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
#include <limits.h>  /* INT_MAX. */
#include <signal.h>  /* raise, SIGABRT, SIGSEGV. */
#include <stdlib.h>  /* abort, setenv, unsetenv, EXIT_FAILURE, EXIT_SUCCESS. */
#include <stdio.h>   /* fmemopen, fprintf, printf, puts. */
#include <string.h>  /* strcmp. */

/* Standard testing apparatus. */
#define ARGV_LEN(X) (sizeof(X) / sizeof((X)[0]) - 1)

#define TEST_EQUAL_I(X, Y) do {                                           \
    const int x_result = (int)(X);                                        \
    const int y_result = (int)(Y);                                        \
    if (x_result != y_result) {                                           \
        fprintf(stderr,                                                   \
                "Expected %s == %s at line %d (%d != %d)\n",              \
                "" #X "", "" #Y "", __LINE__,                             \
                x_result, y_result);                                      \
        return 1;                                                         \
    }                                                                     \
} while (0)

#define TEST_EQUAL_P(X, Y) do {                                           \
    void* const x_result = (X);                                           \
    void* const y_result = (Y);                                           \
    if (x_result != y_result) {                                           \
        fprintf(stderr,                                                   \
                "Expected %s == %s at line %d (%p != %p)\n",              \
                "" #X "", "" #Y "", __LINE__,                             \
                x_result, y_result);                                      \
        return 1;                                                         \
    }                                                                     \
} while (0)

#define TEST_EQUAL_S(X, Y) do {                                           \
    const char* const x_result = (X);                                     \
    const char* const y_result = (Y);                                     \
    if (x_result != y_result) {                                           \
        if (x_result == NULL || y_result == NULL ||                       \
            strcmp(x_result, y_result) != 0) {                            \
            fprintf(stderr,                                               \
                    "Expected %s == %s at line %d (\"%s\" != \"%s\")\n",  \
                    "" #X "", "" #Y "", __LINE__,                         \
                    x_result, y_result);                                  \
            return 1;                                                     \
        }                                                                 \
    }                                                                     \
} while (0)

/* Test function declarations. */
static int      test_trycmd_make_shell_cmd(void);
static int      test_trycmd_run_subcommand(void);
static int      test_trycmd_show_exit_status(void);
static int      test_trycmd_print_usage(void);
static int      test_trycmd_read_options(void);
static int      test_trycmd_align_sz(void);
static int      test_trycmd_align_ptr(void);
static int      test_trycmd_getenv_s(void);
static int      test_trycmd_getenv_i(void);
static int      test_trycmd_main(void);

struct test_func {
    const char* name;
    int         (*func)(void);
};

static const struct test_func all_tests[] = {
    { "trycmd_make_shell_cmd",   &test_trycmd_make_shell_cmd   },
    { "trycmd_run_subcommand",   &test_trycmd_run_subcommand   },
    { "trycmd_show_exit_status", &test_trycmd_show_exit_status },
    { "trycmd_print_usage",      &test_trycmd_print_usage      },
    { "trycmd_read_options",     &test_trycmd_read_options     },
    { "trycmd_align_sz",         &test_trycmd_align_sz         },
    { "trycmd_align_ptr",        &test_trycmd_align_ptr        },
    { "trycmd_getenv_s",         &test_trycmd_getenv_s         },
    { "trycmd_getenv_i",         &test_trycmd_getenv_i         },
    { "trycmd_main",             &test_trycmd_main             },
};
static const size_t all_tests_len = sizeof(all_tests) / sizeof(all_tests[0]);

static char* trycmd_test_progname = NULL;

/* A result outside the normal 0..125 range. */
static const int trycmd_test_high_exit_status = 129;

static void trycmd_initialise_tests(int argc, char* argv[]) {
    /* We expect the command-line to contain the program name. */
    assert(argc > 0);
    assert(argv[0] != NULL);
    assert(argv[0][0] != '\0');
    
    /*
     * Read and store the program name.
     * This can be used to respawn the program for tests.
     */
    trycmd_test_progname = argv[0];

    /* Reset environment options to an expected, initial state. */
    unsetenv("TRY_INTERACTIVE");
    unsetenv("SHELL");
    unsetenv("TESTKEY_1");
    unsetenv("TESTKEY_2");

    /* Prevent all string translation. */
    unsetenv("LANG");
    unsetenv("LANGUAGE");
}

static int trycmd_run_all_tests(void) {
    int failure_count = 0;
    size_t testidx;

    /* Check assumptions. */
    assert(all_tests_len <= INT_MAX);

    /* Execute all tests, in-order. */
    for (testidx = 0; testidx != all_tests_len; ++testidx) {
        printf("TEST: %s ... ", all_tests[testidx].name);
        if (all_tests[testidx].func() == 0) {
            puts("succeeded");
        } else {
            puts("failed!");
            ++failure_count;
        }
    }

    /* Print a result message then return the number of failures. */
    printf("%d tests failed.\n", failure_count);
    return failure_count;
}

/* Test function implementations. */
int test_trycmd_make_shell_cmd(void) {
    char* argv_true[] = { "true", NULL };
    char* argv_echo[] = { "echo", "hello", "this", "is", "a", "test", NULL };
    struct trycmd_opts opts = { 0 };
    unsigned char buffer[256];
    char** argv = NULL;
    size_t sz;

    opts.opt_sub_argc = 1;
    opts.opt_sub_argv = argv_true;
    opts.opt_shell = "/bin/dummy_shell";
    opts.opt_interactive = 0;
    sz = trycmd_align_sz(6 * sizeof(char*) +
        sizeof("/bin/dummy_shell -c -- true \"$@\" "),
        sizeof(char*));
    assert("Buffer too small" && sz < sizeof(buffer));
    memset(buffer, 0xef, sizeof(buffer));
    TEST_EQUAL_I(trycmd_make_shell_cmd(&opts, buffer, 0, &argv), sz);
    TEST_EQUAL_I(buffer[0], 0xef);
    TEST_EQUAL_I(trycmd_make_shell_cmd(&opts, buffer, sz - 1, &argv), sz);
    TEST_EQUAL_I(buffer[0], 0xef);

    TEST_EQUAL_I(trycmd_make_shell_cmd(&opts, buffer, sz, &argv), sz);
    TEST_EQUAL_S(argv[0], "/bin/dummy_shell");
    TEST_EQUAL_S(argv[1], "-c");
    TEST_EQUAL_S(argv[2], "--");
    TEST_EQUAL_S(argv[3], "true \"$@\"");
    TEST_EQUAL_S(argv[4], "true");
    TEST_EQUAL_S(argv[5], NULL);
    TEST_EQUAL_I(buffer[sz], 0xef);

    opts.opt_sub_argc = 1;
    opts.opt_sub_argv = argv_true;
    opts.opt_shell = "/bin/dummy_shell";
    opts.opt_interactive = 1;
    sz = trycmd_align_sz(7 * sizeof(char*) +
        sizeof("/bin/dummy_shell -i -c -- true \"$@\" "),
        sizeof(char*));
    assert("Buffer too small" && sz < sizeof(buffer));
    memset(buffer, 0xef, sizeof(buffer));
    TEST_EQUAL_I(trycmd_make_shell_cmd(&opts, buffer, sizeof(buffer), &argv), sz);
    TEST_EQUAL_S(argv[0], "/bin/dummy_shell");
    TEST_EQUAL_S(argv[1], "-i");
    TEST_EQUAL_S(argv[2], "-c");
    TEST_EQUAL_S(argv[3], "--");
    TEST_EQUAL_S(argv[4], "true \"$@\"");
    TEST_EQUAL_S(argv[5], "true");
    TEST_EQUAL_S(argv[6], NULL);
    TEST_EQUAL_I(buffer[sz], 0xef);

    opts.opt_sub_argc = 6;
    opts.opt_sub_argv = argv_echo;
    opts.opt_shell = "/bin/dummy_shell";
    opts.opt_interactive = 0;
    sz = trycmd_align_sz(11 * sizeof(char*) +
         sizeof("/bin/dummy_shell -c -- echo \"$@\" "), /* 29 */
         sizeof(char*));
    assert("Buffer too small" && sz < sizeof(buffer));
    memset(buffer, 0xef, sizeof(buffer));
    TEST_EQUAL_I(trycmd_make_shell_cmd(&opts, buffer, sizeof(buffer), &argv), sz);
    TEST_EQUAL_S(argv[0], "/bin/dummy_shell");
    TEST_EQUAL_S(argv[1], "-c");
    TEST_EQUAL_S(argv[2], "--");
    TEST_EQUAL_S(argv[3], "echo \"$@\"");
    TEST_EQUAL_S(argv[4], "echo");
    TEST_EQUAL_S(argv[5], "hello");
    TEST_EQUAL_S(argv[6], "this");
    TEST_EQUAL_S(argv[7], "is");
    TEST_EQUAL_S(argv[8], "a");
    TEST_EQUAL_S(argv[9], "test");
    TEST_EQUAL_S(argv[10], NULL);
    TEST_EQUAL_I(buffer[sz], 0xef);
    return 0;
}

int test_trycmd_run_subcommand(void) {
    char* argv_true[]  = { trycmd_test_progname, "T", NULL };
    char* argv_false[] = { trycmd_test_progname, "F", NULL };
    struct trycmd_opts opts = { 0 };

    opts.opt_shell = DEF_SHELL_PATH;
    opts.opt_sub_argc = ARGV_LEN(argv_true);
    assert(ARGV_LEN(argv_true) == ARGV_LEN(argv_false));
    TEST_EQUAL_I((opts.opt_sub_argv = argv_true, trycmd_run_subcommand(&opts)), 0);
    TEST_EQUAL_I((opts.opt_sub_argv = argv_false, trycmd_run_subcommand(&opts)), 1);
    return 0;
}

int test_trycmd_show_exit_status(void) {
    char* argv_empty[] = { NULL };
    struct trycmd_opts opts = { 0 };
    opts.opt_sub_argv = argv_empty;

    TEST_EQUAL_I(trycmd_show_exit_status(&opts, 0), 0);
    TEST_EQUAL_I(trycmd_show_exit_status(&opts, 1), 1);
    TEST_EQUAL_I(trycmd_show_exit_status(&opts, 2), 2);
    TEST_EQUAL_I(trycmd_show_exit_status(&opts, 255), 255);
    return 0;
}

int test_trycmd_print_usage(void) {
    char buffer[768] = { 0 };
    FILE* fout;

    /* Write usage information to a memory stream then check its content. */
    fout = fmemopen(buffer, sizeof(buffer), "w");
    trycmd_print_usage(fout);
    fflush(fout);
    fclose(fout);

    TEST_EQUAL_S(buffer,
        "Usage: try [-ivh] [--] [COMMAND] [ARG_1] ... [ARG_N]\n"
        "Where:       -i --interactive      Execute the command in an interactive subshell.\n"
        "             -v --verbose          Verbose output (echos the command being run).\n"
        "             -h --help             Show this message.\n"
        "             --                    End of options.\n"
        "             COMMAND               The command to run.\n"
        "             ARG_[1..N]            Arguments to the command.\n"
        "\n"
        "Environment: TRY_INTERACTIVE=1     Always execute commands in an interactive subshell.\n"
        "             SHELL=/bin/sh         The shell to use when executing the command.\n"
        "\n");
    return 0;
}

int test_trycmd_read_options(void) {
    struct trycmd_opts opts = { 0 };
    char* test_argv_empty[]             = { "try", NULL };
    char* test_argv_interactive_short[] = { "try", "-i", NULL };
    char* test_argv_interactive_long[]  = { "try", "--interactive", NULL };
    char* test_argv_verbose_short[]     = { "try", "-v", NULL };
    char* test_argv_verbose_long[]      = { "try", "--verbose", NULL };
    char* test_argv_help_short[]        = { "try", "-h", NULL };
    char* test_argv_help_long[]         = { "try", "--help", NULL };
    char* test_argv_compound[]          = { "try", "-ivh", NULL };
    char* test_argv_cmd_single[]        = { "try", "test_name", NULL };
    char* test_argv_cmd_double[]        = { "try", "test_name", "test_arg_1", NULL };
    char* test_argv_cmd_triple[]        = { "try", "test_name", "test_arg_1", "test_arg_2", NULL };
    char* test_argv_end_of_args[]       = { "try", "--", "-v", "test_arg_1", NULL };
    char* test_argv_cmd_all[]           = { "try", "-i", "-v", "--help", "--",
                                            "test_name", "test_arg_1", "test_arg_2",
                                            NULL
    };

    /* Empty command, equivalent to "$ try". */
    TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_empty), test_argv_empty, &opts), 0);
    TEST_EQUAL_I(opts.opt_interactive, 0);
    TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
    TEST_EQUAL_I(opts.opt_verbose, 0);
    TEST_EQUAL_I(opts.opt_help, 0);
    TEST_EQUAL_I(opts.opt_sub_argc, 0);
    TEST_EQUAL_S(opts.opt_sub_argv[0], NULL);

    /* Interactive command, equivalent to "$ try -i" or "$ try --interactive". */
    TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_interactive_short), test_argv_interactive_short, &opts), 0);
    TEST_EQUAL_I(opts.opt_interactive, 1);
    TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
    TEST_EQUAL_I(opts.opt_verbose, 0);
    TEST_EQUAL_I(opts.opt_help, 0);
    TEST_EQUAL_I(opts.opt_sub_argc, 0);
    TEST_EQUAL_S(opts.opt_sub_argv[0], NULL);
    TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_interactive_long), test_argv_interactive_long, &opts), 0);
    TEST_EQUAL_I(opts.opt_interactive, 1);
    TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
    TEST_EQUAL_I(opts.opt_verbose, 0);
    TEST_EQUAL_I(opts.opt_help, 0);
    TEST_EQUAL_I(opts.opt_sub_argc, 0);
    TEST_EQUAL_S(opts.opt_sub_argv[0], NULL);

    /* Verbose command, equivalent to "$ try -v" or "$ try --verbose". */
    TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_verbose_short), test_argv_verbose_short, &opts), 0);
    TEST_EQUAL_I(opts.opt_interactive, 0);
    TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
    TEST_EQUAL_I(opts.opt_verbose, 1);
    TEST_EQUAL_I(opts.opt_help, 0);
    TEST_EQUAL_I(opts.opt_sub_argc, 0);
    TEST_EQUAL_S(opts.opt_sub_argv[0], NULL);
    TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_verbose_long), test_argv_verbose_long, &opts), 0);
    TEST_EQUAL_I(opts.opt_interactive, 0);
    TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
    TEST_EQUAL_I(opts.opt_verbose, 1);
    TEST_EQUAL_I(opts.opt_help, 0);
    TEST_EQUAL_I(opts.opt_sub_argc, 0);
    TEST_EQUAL_S(opts.opt_sub_argv[0], NULL);

    /* Help command, equivalent to "$ try -h" or "$ try --help". */
    TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_help_short), test_argv_help_short, &opts), 0);
    TEST_EQUAL_I(opts.opt_interactive, 0);
    TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
    TEST_EQUAL_I(opts.opt_verbose, 0);
    TEST_EQUAL_I(opts.opt_help, 1);
    TEST_EQUAL_I(opts.opt_sub_argc, 0);
    TEST_EQUAL_S(opts.opt_sub_argv[0], NULL);
    TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_help_long), test_argv_help_long, &opts), 0);
    TEST_EQUAL_I(opts.opt_interactive, 0);
    TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
    TEST_EQUAL_I(opts.opt_verbose, 0);
    TEST_EQUAL_I(opts.opt_help, 1);
    TEST_EQUAL_I(opts.opt_sub_argc, 0);
    TEST_EQUAL_S(opts.opt_sub_argv[0], NULL);

    /* Compound command, equivalent to "$ try -ivh". */
    TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_compound), test_argv_compound, &opts), 0);
    TEST_EQUAL_I(opts.opt_interactive, 1);
    TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
    TEST_EQUAL_I(opts.opt_verbose, 1);
    TEST_EQUAL_I(opts.opt_help, 1);
    TEST_EQUAL_I(opts.opt_sub_argc, 0);
    TEST_EQUAL_S(opts.opt_sub_argv[0], NULL);

    /* Single argument command, equivalent to "$ try test_name". */
    TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_cmd_single), test_argv_cmd_single, &opts), 0);
    TEST_EQUAL_I(opts.opt_interactive, 0);
    TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
    TEST_EQUAL_I(opts.opt_verbose, 0);
    TEST_EQUAL_I(opts.opt_help, 0);
    TEST_EQUAL_I(opts.opt_sub_argc, 1);
    TEST_EQUAL_S(opts.opt_sub_argv[0], test_argv_cmd_single[1]);
    TEST_EQUAL_S(opts.opt_sub_argv[1], NULL);

    /* Double argument command, equivalent to "$ try test_name test_arg_1". */
    TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_cmd_double), test_argv_cmd_double, &opts), 0);
    TEST_EQUAL_I(opts.opt_interactive, 0);
    TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
    TEST_EQUAL_I(opts.opt_verbose, 0);
    TEST_EQUAL_I(opts.opt_help, 0);
    TEST_EQUAL_I(opts.opt_sub_argc, 2);
    TEST_EQUAL_S(opts.opt_sub_argv[0], test_argv_cmd_double[1]);
    TEST_EQUAL_S(opts.opt_sub_argv[1], test_argv_cmd_double[2]);
    TEST_EQUAL_S(opts.opt_sub_argv[2], NULL);

    /* Triple argument command, equivalent to "$ try test_name test_arg_1 test_arg_2". */
    TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_cmd_triple), test_argv_cmd_triple, &opts), 0);
    TEST_EQUAL_I(opts.opt_interactive, 0);
    TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
    TEST_EQUAL_I(opts.opt_verbose, 0);
    TEST_EQUAL_I(opts.opt_help, 0);
    TEST_EQUAL_I(opts.opt_sub_argc, 3);
    TEST_EQUAL_S(opts.opt_sub_argv[0], test_argv_cmd_triple[1]);
    TEST_EQUAL_S(opts.opt_sub_argv[1], test_argv_cmd_triple[2]);
    TEST_EQUAL_S(opts.opt_sub_argv[2], test_argv_cmd_triple[3]);
    TEST_EQUAL_S(opts.opt_sub_argv[3], NULL);

    /* Premature end-of-arguments command, equivalent to "$ try -- -v test_arg_1". */
    TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_end_of_args), test_argv_end_of_args, &opts), 0);
    TEST_EQUAL_I(opts.opt_interactive, 0);
    TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
    TEST_EQUAL_I(opts.opt_verbose, 0);
    TEST_EQUAL_I(opts.opt_help, 0);
    TEST_EQUAL_I(opts.opt_sub_argc, 2);
    TEST_EQUAL_S(opts.opt_sub_argv[0], test_argv_end_of_args[2]);
    TEST_EQUAL_S(opts.opt_sub_argv[1], test_argv_end_of_args[3]);
    TEST_EQUAL_S(opts.opt_sub_argv[2], NULL);

    /* All argument command, equivalent to "$ try -i -v --help -- test_name test_arg_1 test_arg_2" */
    TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_cmd_all), test_argv_cmd_all, &opts), 0);
    TEST_EQUAL_I(opts.opt_interactive, 1);
    TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
    TEST_EQUAL_I(opts.opt_verbose, 1);
    TEST_EQUAL_I(opts.opt_help, 1);
    TEST_EQUAL_I(opts.opt_sub_argc, 3);
    TEST_EQUAL_S(opts.opt_sub_argv[0], test_argv_cmd_all[5]);
    TEST_EQUAL_S(opts.opt_sub_argv[1], test_argv_cmd_all[6]);
    TEST_EQUAL_S(opts.opt_sub_argv[2], test_argv_cmd_all[7]);
    TEST_EQUAL_S(opts.opt_sub_argv[3], NULL);

    /* Interactive command controlled by environment string TRY_INTERACTIVE=[01]. */
    setenv("TRY_INTERACTIVE", "1", 0);
        TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_empty), test_argv_empty, &opts), 0);
        TEST_EQUAL_I(opts.opt_interactive, 1);
        TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
        TEST_EQUAL_I(opts.opt_verbose, 0);
        TEST_EQUAL_I(opts.opt_help, 0);
        TEST_EQUAL_I(opts.opt_sub_argc, 0);
        TEST_EQUAL_S(opts.opt_sub_argv[0], NULL);
    setenv("TRY_INTERACTIVE", "0", 1);
        TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_interactive_long), test_argv_interactive_long, &opts), 0);
        TEST_EQUAL_I(opts.opt_interactive, 1);
        TEST_EQUAL_S(opts.opt_shell, DEF_SHELL_PATH);
        TEST_EQUAL_I(opts.opt_verbose, 0);
        TEST_EQUAL_I(opts.opt_help, 0);
        TEST_EQUAL_I(opts.opt_sub_argc, 0);
        TEST_EQUAL_S(opts.opt_sub_argv[0], NULL);
    unsetenv("TRY_INTERACTIVE");

    /* Sub-shell controlled controlled by environment string SHELL. */
    setenv("SHELL", "/bin/dummy_shell", 0);
        TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_empty), test_argv_empty, &opts), 0);
        TEST_EQUAL_I(opts.opt_interactive, 0);
        TEST_EQUAL_S(opts.opt_shell, "/bin/dummy_shell");
        TEST_EQUAL_I(opts.opt_verbose, 0);
        TEST_EQUAL_I(opts.opt_help, 0);
        TEST_EQUAL_I(opts.opt_sub_argc, 0);
        TEST_EQUAL_S(opts.opt_sub_argv[0], NULL);
    unsetenv("SHELL");

    /* All environment settings enabled. */
    setenv("TRY_INTERACTIVE", "2", 0);
    setenv("SHELL", "/bin/dummy_shell", 0);
        TEST_EQUAL_I(trycmd_read_options(ARGV_LEN(test_argv_empty), test_argv_empty, &opts), 0);
        TEST_EQUAL_I(opts.opt_interactive, 1);
        TEST_EQUAL_S(opts.opt_shell, "/bin/dummy_shell");
        TEST_EQUAL_I(opts.opt_verbose, 0);
        TEST_EQUAL_I(opts.opt_help, 0);
        TEST_EQUAL_I(opts.opt_sub_argc, 0);
        TEST_EQUAL_S(opts.opt_sub_argv[0], NULL);
    unsetenv("TRY_INTERACTIVE");
    unsetenv("SHELL");
    return 0;
}

int test_trycmd_align_sz(void) {
    TEST_EQUAL_I(trycmd_align_sz(0, 1), 0);
    TEST_EQUAL_I(trycmd_align_sz(0, 2), 0);
    TEST_EQUAL_I(trycmd_align_sz(0, 3), 0);
    TEST_EQUAL_I(trycmd_align_sz(0, 4), 0);
    TEST_EQUAL_I(trycmd_align_sz(1, 1), 1);
    TEST_EQUAL_I(trycmd_align_sz(1, 2), 2);
    TEST_EQUAL_I(trycmd_align_sz(1, 3), 3);
    TEST_EQUAL_I(trycmd_align_sz(1, 4), 4);
    TEST_EQUAL_I(trycmd_align_sz(2, 1), 2);
    TEST_EQUAL_I(trycmd_align_sz(2, 2), 2);
    TEST_EQUAL_I(trycmd_align_sz(2, 3), 3);
    TEST_EQUAL_I(trycmd_align_sz(2, 4), 4);
    TEST_EQUAL_I(trycmd_align_sz(3, 1), 3);
    TEST_EQUAL_I(trycmd_align_sz(3, 2), 4);
    TEST_EQUAL_I(trycmd_align_sz(3, 3), 3);
    TEST_EQUAL_I(trycmd_align_sz(3, 4), 4);
    TEST_EQUAL_I(trycmd_align_sz(128, 4), 128);
    TEST_EQUAL_I(trycmd_align_sz(128, 8), 128);
    TEST_EQUAL_I(trycmd_align_sz(128, 16), 128);
    TEST_EQUAL_I(trycmd_align_sz(128, 32), 128);
    TEST_EQUAL_I(trycmd_align_sz(129, 4), 132);
    TEST_EQUAL_I(trycmd_align_sz(129, 8), 136);
    TEST_EQUAL_I(trycmd_align_sz(129, 16), 144);
    TEST_EQUAL_I(trycmd_align_sz(129, 32), 160);
    return 0;
}

int test_trycmd_align_ptr(void) {
    char* const buf = 0;  /* Fixed and aligned memory location. */
    TEST_EQUAL_P(trycmd_align_ptr(&buf[0], 1), &buf[0]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[0], 2), &buf[0]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[0], 3), &buf[0]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[0], 4), &buf[0]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[1], 1), &buf[1]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[1], 2), &buf[2]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[1], 3), &buf[3]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[1], 4), &buf[4]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[2], 1), &buf[2]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[2], 2), &buf[2]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[2], 3), &buf[3]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[2], 4), &buf[4]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[3], 1), &buf[3]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[3], 2), &buf[4]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[3], 3), &buf[3]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[3], 4), &buf[4]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[128], 4), &buf[128]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[128], 8), &buf[128]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[128], 16), &buf[128]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[128], 32), &buf[128]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[129], 4), &buf[132]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[129], 8), &buf[136]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[129], 16), &buf[144]);
    TEST_EQUAL_P(trycmd_align_ptr(&buf[129], 32), &buf[160]);
    return 0;
}

int test_trycmd_getenv_s(void) {
    setenv("TESTKEY_1", "testval_1", 0);
    setenv("TESTKEY_2", "testval_2", 0);
        TEST_EQUAL_S(trycmd_getenv_s("TESTKEY_1"), "testval_1");
        TEST_EQUAL_S(trycmd_getenv_s("TESTKEY_2"), "testval_2");
        TEST_EQUAL_S(trycmd_getenv_s("XX_BAD_KEY_XX"), NULL);
    unsetenv("TESTKEY_1");
    unsetenv("TESTKEY_2");
    return 0;
}

int test_trycmd_getenv_i(void) {
    setenv("TESTKEY_1", "99", 0);
    setenv("TESTKEY_2", "100", 0);
        TEST_EQUAL_I(trycmd_getenv_i("TESTKEY_1"), 99);
        TEST_EQUAL_I(trycmd_getenv_i("TESTKEY_2"), 100);
        TEST_EQUAL_I(trycmd_getenv_i("XX_BAD_KEY_XX"), 0);
    unsetenv("TESTKEY_1");
    unsetenv("TESTKEY_2");
    return 0;
}

int test_trycmd_main(void) {
    char* argv_echo[]         = { "try", "echo", "hello", "this", "is", "a", "test", NULL };
    char* argv_true[]         = { "try", trycmd_test_progname, "T", NULL };
    char* argv_false[]        = { "try", trycmd_test_progname, "F", NULL };
    char* argv_abort[]        = { "try", trycmd_test_progname, "A", NULL };
    char* argv_segflt[]       = { "try", trycmd_test_progname, "S", NULL };
    char* argv_exit_status[]  = { "try", trycmd_test_progname, "X", NULL };
    char* argv_non_existent[] = { "try", "XX_this_should_not_exist_XX", NULL };

    TEST_EQUAL_I(trycmd_main(ARGV_LEN(argv_echo), argv_echo), EXIT_SUCCESS);
    TEST_EQUAL_I(trycmd_main(ARGV_LEN(argv_true), argv_true), EXIT_SUCCESS);
    TEST_EQUAL_I(trycmd_main(ARGV_LEN(argv_false), argv_false), EXIT_FAILURE);
    TEST_EQUAL_I(trycmd_main(ARGV_LEN(argv_abort), argv_abort), TRYCMD_SIGNAL_BASE + SIGABRT);
    TEST_EQUAL_I(trycmd_main(ARGV_LEN(argv_segflt), argv_segflt), TRYCMD_SIGNAL_BASE + SIGSEGV);
    TEST_EQUAL_I(trycmd_main(ARGV_LEN(argv_exit_status), argv_exit_status), trycmd_test_high_exit_status);
    TEST_EQUAL_I(trycmd_main(ARGV_LEN(argv_non_existent), argv_non_existent), 127);  /* To match bash. */
    return 0;
}

/* Application entry point. */
int main(int argc, char* argv[]) {
    char mode = 'R';  /* Default mode. */
    int result = EXIT_SUCCESS;

    /* Always initialise the test suite. */
    trycmd_initialise_tests(argc, argv);

    /* Check for a single-character mode argument. */
    if (argc == 2 && argv[1][0] != '\0' && argv[1][1] == '\0') {
        mode = argv[1][0];
    }

    /* Perform an action according to the requested mode (if any). */
    switch (mode) {
        case 'A':  /* 'A'bort. */
            printf("try_test: Aborting\n");
            result = EXIT_FAILURE;
            abort();
            break;
        case 'S':  /* 'S'egfault. */
            printf("try_test: Raising SIGSEGV\n");
            result = EXIT_FAILURE;
            raise(SIGSEGV);
            break;
        case 'T':  /* 'T'rue. */
            printf("try_test: Returning %d\n", EXIT_SUCCESS);
            result = EXIT_SUCCESS;
            break;
        case 'F':  /* 'F'alse. */
            printf("try_test: Returning %d\n", EXIT_FAILURE);
            result = EXIT_FAILURE;
            break;
        case 'X':  /* E'X'it status. */
            printf("try_test: Returning %d\n", trycmd_test_high_exit_status);
            result = trycmd_test_high_exit_status;
            break;
        case 'R':  /* 'R'un test suite. */
            printf("try_test: Running all tests...\n");
            if (trycmd_run_all_tests() != 0) {
                result = EXIT_FAILURE;
            }
            break;
        default:
            fprintf(stderr, "try_test: Unrecognised mode '%c'.\n", mode);
            result = EXIT_FAILURE;
    }
    return result;
}

/* EOF */
