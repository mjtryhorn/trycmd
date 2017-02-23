/**
 * \file      trycmd.c
 * \brief     Run a command, display a standard result
 *            and pass on any exit status.
 * \details   Defines the application's main entry point.
 *
 * \author    M. J. Tryhorn
 * \date      2017-Feb-23
 * \version   1.0
 * \copyright MIT License (see LICENSE).
 */

#include "trycmd_config.h"
#include "trycmd.h"

/* Application entry point. */
int main(int argc, char* argv[]) {
    return trycmd_main(argc, argv);
}

/* EOF */
