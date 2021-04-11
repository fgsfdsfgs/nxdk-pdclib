/* long double strtold( const char *, char * * )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include <stdlib.h>

#ifndef REGTEST

#include "pdclib/_PDCLIB_int.h"

long double strtold( const char * _PDCLIB_restrict s, char * * _PDCLIB_restrict endptr )
{
    /* TODO: proper long double handling */
    return (long double)_PDCLIB_fmtstrtod( s, endptr );
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
