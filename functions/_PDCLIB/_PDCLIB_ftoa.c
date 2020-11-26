/*
 * (c) Marco Paland (info@paland.com)
 * 2014-2019, PALANDesign Hannover, Germany
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

/* float -> string conversion functions from https://github.com/mpaland/printf */
/* used for ftoa() and *printf() */

#include <float.h>
#include "pdclib/_PDCLIB_int.h"

#define _PDCLIB_FTOA_MAXFLOAT 1e9
#define _PDCLIB_FTOA_BUFSIZE 32U
#define _PDCLIB_FTOA_DEFAULT_FLOAT_PRECISION 6U

/* powers of 10 */
static const double pow10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

static unsigned int _out_rev( _PDCLIB_ftoa_out_fn out, struct _PDCLIB_status_t * status, int idx, const char* buf, unsigned int len, unsigned int width, unsigned int flags )
{
    const unsigned int start_idx = idx;
    unsigned int i;

    /* pad spaces up to given width */
    if ( !( flags & _PDCLIB_FTOA_FLAG_LEFT ) && !( flags & _PDCLIB_FTOA_FLAG_ZEROPAD ) )
    {
        for ( i = len; i < width; i++, ++idx )
            out( status, ' ' );
    }

    /* reverse string */
    while ( len )
    {
        out( status, buf[--len] );
        ++idx;
    }

    /* append pad spaces up to given width */
    if ( flags & _PDCLIB_FTOA_FLAG_LEFT )
    {
        while ( idx - start_idx < width )
        {
            out( status, ' ' );
            ++idx;
        }
    }

    return idx;
}

unsigned int _PDCLIB_ftoa( _PDCLIB_ftoa_out_fn out, struct _PDCLIB_status_t * status, int idx, double value, unsigned int prec, unsigned int width, unsigned int flags )
{
    char buf[_PDCLIB_FTOA_BUFSIZE];
    unsigned int len = 0U;
    double diff = 0.0;
    unsigned long frac;
    int whole, negative;
    double tmp;

    /* test for special values */
    if ( value != value )
        return _out_rev( out, status, idx, "nan", 3, width, flags );
    if ( value < -DBL_MAX )
        return _out_rev( out, status, idx, "fni-", 4, width, flags );
    if ( value > DBL_MAX )
        return _out_rev( out, status, idx, ( flags & _PDCLIB_FTOA_FLAG_PLUS ) ? "fni+" : "fni", ( flags & _PDCLIB_FTOA_FLAG_PLUS ) ? 4U : 3U, width, flags );

    // test for very large values
    // standard printf behavior is to print EVERY whole number digit -- which could be 100s of characters overflowing your buffers == bad
    if ( ( value > _PDCLIB_FTOA_MAXFLOAT ) || ( value < -_PDCLIB_FTOA_MAXFLOAT ) )
    {
        return _PDCLIB_etoa(out, status, idx, value, prec, width, flags);
    }

    /* test for negative */
    negative = 0;
    if ( value < 0 )
    {
        negative = 1;
        value = 0 - value;
    }

    /* set default precision, if not set explicitly */
    if ( !( flags & _PDCLIB_FTOA_FLAG_PRECISION ) )
        prec = _PDCLIB_FTOA_DEFAULT_FLOAT_PRECISION;

    /* limit precision to 9, cause a prec >= 10 can lead to overflow errors */
    while ( ( len < _PDCLIB_FTOA_BUFSIZE ) && ( prec > 9U ) )
    {
        buf[len++] = '0';
        prec--;
    }

    whole = (int)value;
    tmp = ( value - whole ) * pow10[prec];
    frac = (unsigned long)tmp;
    diff = tmp - frac;

    if ( diff < 0.5 )
    {
        /* round down */
    }
    else if ( diff > 0.5 )
    {
        /* round up */
        ++frac;
    }
    else
    {
        /* if halfway, round up if odd OR if last digit is 0 */
        if ( prec == 0U )
            whole = (whole + 1) & ~1U; /* round whole to even */
        else
            frac = (frac + 1) & ~1U;   /* round fraction to even */
    }

    /* handle rollover, e.g. case 0.99 with prec 1 is 1.0 */
    if ( frac >= pow10[prec] )
    {
        frac = 0;
        ++whole;
    }

    if ( prec )
    {
        unsigned int count = prec;
        /* now do fractional part, as an unsigned number */
        while ( len < _PDCLIB_FTOA_BUFSIZE )
        {
            --count;
            buf[len++] = (char)( 48U + ( frac % 10U ) );
            if ( !( frac /= 10U ) ) break;
        }
        /* add extra 0s */
        while ( ( len < _PDCLIB_FTOA_BUFSIZE ) && ( count-- > 0U ) )
            buf[len++] = '0';
        if ( len < _PDCLIB_FTOA_BUFSIZE )
            buf[len++] = '.'; /* add decimal */
    }

    /* do whole part, number is reversed */
    while ( len < _PDCLIB_FTOA_BUFSIZE )
    {
        buf[len++] = (char)( 48 + ( whole % 10 ) );
        if ( !( whole /= 10 ) ) break;
    }

    /* pad leading zeros */
    if ( !( flags & _PDCLIB_FTOA_FLAG_LEFT ) && ( flags & _PDCLIB_FTOA_FLAG_ZEROPAD ) )
    {
        if ( width && ( negative || ( flags & ( _PDCLIB_FTOA_FLAG_PLUS | _PDCLIB_FTOA_FLAG_SPACE ) ) ) )
            width--;
        while ( ( len < width ) && ( len < _PDCLIB_FTOA_BUFSIZE ) )
            buf[len++] = '0';
    }

    if ( len < _PDCLIB_FTOA_BUFSIZE )
    {
        if ( negative )
            buf[len++] = '-';
        else if ( flags & _PDCLIB_FTOA_FLAG_PLUS )
            buf[len++] = '+'; /* ignore the space if the '+' exists */
        else if ( flags & _PDCLIB_FTOA_FLAG_SPACE )
            buf[len++] = ' ';
    }

    return _out_rev( out, status, idx, buf, len, width, flags );
}

unsigned int _PDCLIB_etoa( _PDCLIB_ftoa_out_fn out, struct _PDCLIB_status_t * status, int idx, double value, unsigned int prec, unsigned int width, unsigned int flags )
{
    union
    {
        _PDCLIB_uint64_t U;
        double F;
    } conv;
    unsigned int minwidth, fwidth, start_idx;
    int negative, exp2, expval;
    double z, z2;

    /* check for NaN and special values */
    if ( ( value != value ) || ( value > DBL_MAX ) || ( value < -DBL_MAX ) )
        return _PDCLIB_ftoa(out, status, idx, value, prec, width, flags);

    /* determine the sign */
    negative = value < 0;
    if ( negative ) value = -value;

    /* default precision */
    if ( !( flags & _PDCLIB_FTOA_FLAG_PRECISION ) )
        prec = _PDCLIB_FTOA_DEFAULT_FLOAT_PRECISION;

    /* handle zero explicitly */
    if ( value != 0 )
    {
        /* determine the decimal exponent */
        /* based on the algorithm by David Gay (https://www.ampl.com/netlib/fp/dtoa.c) */
        conv.F = value;
        exp2 = (int)( ( conv.U >> 52U ) & 0x07FFU ) - 1023; // effectively log2
        conv.U = ( conv.U & ( ( 1ULL << 52U ) - 1U ) ) | ( 1023ULL << 52U ); // drop the exponent so conv.F is now in [1,2)
        /* now approximate log10 from the log2 integer part and an expansion of ln around 1.5 */
        expval = (int)( 0.1760912590558 + exp2 * 0.301029995663981 + ( conv.F - 1.5 ) * 0.289529654602168 );
        /* now we want to compute 10^expval but we want to be sure it won't overflow */
        exp2 = (int)( expval * 3.321928094887362 + 0.5 );
        z    = expval * 2.302585092994046 - exp2 * 0.6931471805599453;
        z2 = z * z;
        conv.U = (_PDCLIB_uint64_t)( exp2 + 1023 ) << 52U;
        /* compute exp(z) using continued fractions
           see https://en.wikipedia.org/wiki/Exponential_function#Continued_fractions_for_ex
        */
        conv.F *= 1 + 2 * z / ( 2 - z + ( z2 / ( 6 + ( z2 / ( 10 + z2 / 14 ) ) ) ) );
        /* correct for rounding errors */
        if ( value < conv.F )
        {
            expval--;
            conv.F /= 10;
        }
    }
    else
    {
        /* zero is 0e0 */
        expval = 0;
        conv.F = 0;
    }

    /* the exponent format is "%+03d" and largest value is "307", so set aside 4-5 characters */
    minwidth = ( ( expval < 100 ) && ( expval > -100 ) ) ? 4U : 5U;

    /* in "%g" mode, "prec" is the number of *significant figures* not decimals */
    if ( flags & _PDCLIB_FTOA_FLAG_ADAPT_EXP )
    {
        const int user_prec = ( flags & _PDCLIB_FTOA_FLAG_PRECISION ); /* remember if prec was specified */
        flags |= _PDCLIB_FTOA_FLAG_PRECISION; /* make sure _ftoa respects precision */
        /* do we want to fall-back to "%f" mode? */
        if ( ( expval >= -4 ) &&  ( expval < 6 || ( expval < (int)prec && expval <= 9 ) ) )
        {
            if ( (int)prec > expval )
                prec = (unsigned)( (int)prec - expval - 1 );
            else
                prec = 0;
            /* no characters in exponent */
            minwidth = 0U;
            expval = 0;
        }
        else
        {
            /* we use one sigfig for the whole part */
            if ( ( prec > 0 ) )
                --prec;
        }
        /* try to omit trailing zeros on the fractional part if there wasn't any precision specified */
        if ( !user_prec && prec && prec < 10 )
        {
            /* FIXME: this is very bad and grossly incorrect */
            const unsigned long whole = (unsigned long)value;
            unsigned long frac = (unsigned long) ( ( value - whole ) * pow10[prec] );
            while ( prec && ( ( frac % 10UL ) == 0UL ) )
            {
                --prec;
                frac /= 10UL;
            }
        }
    }

    /* will everything fit? */
    fwidth = width;
    if ( width > minwidth )
    {
        /* we didn't fall-back so subtract the characters required for the exponent */
        fwidth -= minwidth;
    }
    else
    {
        /* not enough characters, so go back to default sizing */
        fwidth = 0U;
    }

    if ( ( flags & _PDCLIB_FTOA_FLAG_LEFT ) && minwidth )
        fwidth = 0U; /* if we're padding on the right, DON'T pad the floating part */

    /* rescale the float value */
    if ( expval ) value /= conv.F;

    /* output the floating part */
    start_idx = idx;
    idx = _PDCLIB_ftoa( out, status, idx, negative ? -value : value, prec, fwidth, flags & ~_PDCLIB_FTOA_FLAG_ADAPT_EXP );

    /* output the exponent part */
    if ( minwidth )
    {
        char expbuf[16]; /* can't really have an exponent larger than that */
        const int expneg = ( expval < 0 );
        int expidx = (int)sizeof( expbuf ) - 1;
        if ( expneg ) expval = -expval;
        /* output the exponential symbol */
        out( status, ( flags & _PDCLIB_FTOA_FLAG_UPPER ) ? 'E' : 'e' );
        ++idx;
        /* output the exponent sign */
        out( status, expneg ? '-' : '+' );
        ++idx;
        /* store the exponent value */
        expbuf[expidx--] = 0;
        do
        {
            expbuf[expidx--] = '0' + ( expval % 10 );
            expval /= 10;
        }
        while ( expval && expidx >= 0 );
        /* output it */
        for ( ++expidx; expbuf[expidx]; ++expidx, ++idx )
            out( status, expbuf[expidx] );
        /* might need to right-pad spaces */
        if ( flags & _PDCLIB_FTOA_FLAG_LEFT )
        {
            while ( idx - start_idx < width )
            {
                out( status, ' ' );
                ++idx;
            }
        }
    }

    return idx;
}
