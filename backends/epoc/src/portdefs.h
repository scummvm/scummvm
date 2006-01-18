/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 Andreas 'Sprawl' Karlsson - Original EPOC port, ESDL
 * Copyright (C) 2003-2005 Lars 'AnotherGuest' Persson - Original EPOC port, Audio System
 * Copyright (C) 2005 Jurgen 'SumthinWicked' Braam - EPOC/CVS maintainer
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 */

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <e32def.h>

#include <e32std.h>
#include <math.h>

// the place in Symbian FS where scummvm.ini & std***.txt are saved
#define SYMBIAN32_DOC_DIR "C:\\documents\\ScummVM\\" // includes final \\!
#define DISABLE_SCALERS // we only need 1x

#if defined(USE_TREMOR) && !defined(USE_VORBIS)
#define USE_VORBIS // make sure this one is defined together with USE_TREMOR!
#endif

// hack in some tricks to work around not having these fcns for Symbian
// and we _really_ don't wanna link with any other windows LIBC library!
#ifdef __GCC32__

	#define snprintf(buf,len,args...)	sprintf(buf,args)
	#define vsnprintf					snprintf

	// taken from public domain http://www.opensource.apple.com/darwinsource/WWDC2004/gcc_legacy-939/gcc/floatlib.c
	#define SIGNBIT		0x80000000
	#define HIDDEN		(1 << 23)
	#define EXCESSD		1022
	#define EXPD(fp)	(((fp.l.upper) >> 20) & 0x7FF)
	#define SIGND(fp)	((fp.l.upper) & SIGNBIT)
	#define HIDDEND_LL	((long long)1 << 52)
	#define MANTD_LL(fp)	((fp.ll & (HIDDEND_LL-1)) | HIDDEND_LL)
	
	union double_long {
	    double d;
	    struct {
	      long upper;
	      unsigned long lower;
	    } l;
	    long long ll;
	};

	/* convert double float to double int (dfdi) */
	long long inline
	scumm_fixdfdi (double a1) { // __fixdfdi (double a1)
	    register union double_long dl1;
	    register int exp;
	    register long long l;
	
	    dl1.d = a1;
	
	    if (!dl1.l.upper && !dl1.l.lower)
			return (0);
	
	    exp = EXPD (dl1) - EXCESSD - 64;
	    l = MANTD_LL(dl1);
	
	    if (exp > 0) {
		l = (long long)1<<63;
		if (!SIGND(dl1))
		    l--;
		return l;
	    }
	
	    /* shift down until exp = 0 or l = 0 */
	    if (exp < 0 && exp > -64 && l)
			l >>= -exp;
	    else
			return (0);
	
	    return (SIGND (dl1) ? -l : l);
	}
	
	/* 	okay, okay: I admit it: I absolutely have _NO_ idea why __fixdfdi does not get linked in by gcc from libgcc.a
		because I know it's in there: I checked with `ar x _fixdfdi.o libgcc.a` and the symbol is in there, so I'm lost
		and had to fix it this way. I tried all gcc and ld options I could find: no hope :( If someone can enlighten me:
		feel free to let me know at sumthinwicked@users.sf.net! Much obliged.
		PS1. I think for __fixunsdfdi they have made a circumvention by having to add STATICLIBRARY EGCC.LIB		
		PS2. http://gcc.gnu.org/ml/gcc-bugs/2004-01/msg01596.html might have found out the same problem there
	*/

#else // WINS

	// let's just blatantly ignore this for now and just get it to work :P but does n't work from the debug function
	int inline scumm_snprintf (char *str, unsigned long /*n*/, char const *fmt, ...) {
		va_list args;
		va_start(args, fmt);
		vsprintf(str, fmt, args);
		va_end(args);
		return strlen(str);
	}

	#define snprintf					scumm_snprintf
	#define vsnprintf					scumm_snprintf

#endif

// somehow nobody has this function...
#define hypot(a, b)					sqrt((a)*(a) + (b)*(b))

// Symbian bsearch implementation is flawed
void inline *scumm_bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
	size_t i;

	for (i=0; i < nmemb; i++)
		if (compar(key, (void *)((size_t)base + size * i)) == 0)
			return (void *)((size_t)base + size * i);
	return NULL;
}
#define bsearch						scumm_bsearch

// make sure SymbianFatalError() @ SymbianOS.cpp is known in error() @ engine.cpp
extern void SymbianFatalError(const char *msg) ;


