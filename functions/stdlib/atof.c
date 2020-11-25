/* double atof( const char * )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include <stdlib.h>

#ifndef REGTEST

#include <errno.h>
#include "pdclib/_PDCLIB_int.h"

double atof( const char * s )
{
    return _PDCLIB_fmtstrtod( s, NULL );
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
