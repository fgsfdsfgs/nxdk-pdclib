/* double strtod( const char *, char * * )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include <stdlib.h>

#ifndef REGTEST

#include "pdclib/_PDCLIB_int.h"

double strtod( const char * _PDCLIB_restrict s, char * * _PDCLIB_restrict endptr )
{
    return _PDCLIB_fmtstrtod( s, endptr );
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

#include <errno.h>

int main( void )
{
    /* TODO */
    return TEST_RESULTS;
}

#endif
