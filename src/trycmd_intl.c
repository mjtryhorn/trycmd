/**
 * \file      trycmd_intl.c
 * \brief     Internationalization functions.
 *
 * \author    M. J. Tryhorn
 * \date      2017-Feb-23
 * \version   1.0
 * \copyright MIT License (see LICENSE).
 */

#include "trycmd_config.h"
#include "trycmd.h"
#include <assert.h>   /* assert. */
#include <locale.h>   /* setlocale, bindtextdomain, textdomain. */
#include <libintl.h>  /* gettext. */

/* Check for required defined values. */
#if !defined(PACKAGE)
#  error Missing defined PACKAGE.
#endif

#if !defined(LOCALEDIR)
#  error Missing defined LOCALEDIR.
#endif

#if !defined(HAVE_SETLOCALE)
#  error Missing required function 'setlocale'.
#endif

void trycmd_intl_init(void) {
    /*
     * Standard gettext boilerplate.
     * "PACKAGE" and "LOCALEDIR" to be set by external tools.
     * See: https://www.gnu.org/software/gettext/manual/gettext.html#Triggering
     *
     * For a simple example, see:
     * http://stackoverflow.com/questions/1003360/complete-c-i18n-gettext-hello-world-example
     */
    const char* const locale     = setlocale(LC_ALL, "");
    const char* const txt_domain = bindtextdomain(PACKAGE, LOCALEDIR);
    const char* const msg_domain = textdomain(PACKAGE);
    assert("Unexpected result from setlocale" && (locale != NULL));
    assert("Unexpected result from bindtextdomain" && (txt_domain != NULL));
    assert("Unexpected result from textdomain" && (msg_domain != NULL));

    /* All result values are needed, but only in debug mode. */
    (void) locale;
    (void) txt_domain;
    (void) msg_domain;
}

char* _(char* const s) {
    /*
     * Pass the given string onward to gettext for translation.
     */
    return gettext(s);
}

/* EOF */
