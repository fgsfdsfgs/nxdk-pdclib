/* float strtof( const char *, char * * )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include <stdlib.h>

#ifndef REGTEST

#include <math.h>
#include "pdclib/_PDCLIB_int.h"

float strtof( const char * _PDCLIB_restrict s, char * * _PDCLIB_restrict endptr )
{
  /* FIXME: just converting the strtod result to float causes double rounding */
  const double v = _PDCLIB_fmtstrtod( s, endptr );
  if ( isnan( v ) )
      return nanf( NULL );
  return (float)v;
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
