/*
 * FILE: resample.h
 *   BY: Julius Smith (at CCRMA, Stanford U)
 * C BY: translated from SAIL to C by Christopher Lee Fraley
 *          (cf0v@andrew.cmu.edu)
 * DATE: 7-JUN-88
 * VERS: 2.0  (17-JUN-88, 3:00pm)
 */

/*
 * October 29, 1999
 * Various changes, bugfixes(?), increased precision, by Stan Brooks.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/* Conversion constants */
#define Lc        7
#define Nc       (1<<Lc)
#define La        16
#define Na       (1<<La)
#define Lp       (Lc+La)
#define Np       (1<<Lp)
#define Amask    (Na-1)
#define Pmask    (Np-1)

#define MAXNWING  (80<<Lc)
/* Description of constants:
 *
 * Nc - is the number of look-up values available for the lowpass filter
 *    between the beginning of its impulse response and the "cutoff time"
 *    of the filter.  The cutoff time is defined as the reciprocal of the
 *    lowpass-filter cut off frequence in Hz.  For example, if the
 *    lowpass filter were a sinc function, Nc would be the index of the
 *    impulse-response lookup-table corresponding to the first zero-
 *    crossing of the sinc function.  (The inverse first zero-crossing
 *    time of a sinc function equals its nominal cutoff frequency in Hz.)
 *    Nc must be a power of 2 due to the details of the current
 *    implementation. The default value of 128 is sufficiently high that
 *    using linear interpolation to fill in between the table entries
 *    gives approximately 16-bit precision, and quadratic interpolation
 *    gives about 23-bit (float) precision in filter coefficients.
 *
 * Lc - is log base 2 of Nc.
 *
 * La - is the number of bits devoted to linear interpolation of the
 *    filter coefficients.
 *
 * Lp - is La + Lc, the number of bits to the right of the binary point
 *    in the integer "time" variable. To the left of the point, it indexes
 *    the input array (X), and to the right, it is interpreted as a number
 *    between 0 and 1 sample of the input X.  The default value of 23 is
 *    about right.  There is a constraint that the filter window must be
 *    "addressable" in a int32_t, more precisely, if Nmult is the number
 *    of sinc zero-crossings in the right wing of the filter window, then
 *    (Nwing<<Lp) must be expressible in 31 bits.
 *
 */
