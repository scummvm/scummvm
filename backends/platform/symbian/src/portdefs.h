/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SYMBIAN_PORTDEFS_H
#define SYMBIAN_PORTDEFS_H

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

#if (__GNUC__ && __cplusplus)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
#endif
#include <stdlib.h>
#if (__GNUC__ && __cplusplus)
#pragma GCC diagnostic pop
#endif

#if (__GNUC__ && __cplusplus)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-local-addr"
#pragma GCC diagnostic ignored "-Wnarrowing"
#endif
#include <e32def.h>
#if !defined(__IGNORE__E32STD_H__) // TKey type from system header
#include <e32std.h> // doesn't meets with lua ones.
#endif
#if (__GNUC__ && __cplusplus)
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#endif

#include <libc\math.h>

/* define pi */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif  /*  M_PI  */


// Enable Symbians own datatypes
// This is done for two reasons
// a) uint is already defined by Symbians libc component
// b) Symbian is using its "own" datatyping, and the Scummvm port
//    should follow this to ensure the best compability possible.
typedef unsigned char byte;
typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short int uint16;
typedef signed short int int16;
typedef unsigned long int uint32;
typedef signed long int int32;
typedef signed long long int64;
typedef unsigned long long uint64;

#ifdef __cplusplus
namespace std
	{

	using ::size_t;

	} // namespace std
#endif

// Define SCUMMVM_DONT_DEFINE_TYPES to prevent scummsys.h from trying to
// re-define those data types.
#define SCUMMVM_DONT_DEFINE_TYPES

// Hide the macro "remove" defined in unistd.h from anywere except where
// we explicitly require it. This lets us use the name "remove" in engines.
// Must be after including unistd.h .
#ifndef SYMBIAN_USE_SYSTEM_REMOVE
#undef remove
#endif

#if __cplusplus >= 201103L
#define USE_CXX11
#define NO_CXX11_INITIALIZER_LIST
#define NO_CXX11_NULLPTR_T
#endif //USE_CXX11

#define DISABLE_COMMAND_LINE
#define USE_RGB_COLOR
#define USE_TINYGL

#ifndef SYMBIAN_DYNAMIC_PLUGIN
#define DETECTION_STATIC
#endif //DETECTION_STATIC

// hack in some tricks to work around not having these fcns for Symbian
// and we _really_ don't wanna link with any other windows LIBC library!
#if defined(__GCC32__)
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
	    union double_long dl1;
	    int exp;
	    long long l;

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

	/*	okay, okay: I admit it: I absolutely have _NO_ idea why __fixdfdi does not get linked in by gcc from libgcc.a
		because I know it's in there: I checked with `ar x _fixdfdi.o libgcc.a` and the symbol is in there, so I'm lost
		and had to fix it this way. I tried all gcc and ld options I could find: no hope :( If someone can enlighten me:
		feel free to let me know at sumthinwicked@users.sf.net! Much obliged.
		PS1. I think for __fixunsdfdi they have made a circumvention by having to add STATICLIBRARY EGCC.LIB
		PS2. http://gcc.gnu.org/ml/gcc-bugs/2004-01/msg01596.html might have found out the same problem there
	*/

#elif defined(__WINS__) // WINS
	extern "C" int symbian_snprintf(char *text, size_t maxlen, const char *fmt, ...);
	extern "C" int symbian_vsnprintf(char *text, size_t maxlen, const char *fmt, va_list ap);
	#define snprintf(buf,len,args...) symbian_snprintf(buf,len,args)
	#define vsnprintf(buf,len,format,valist) symbian_vsnprintf(buf,len,format,valist)

	void*	symbian_malloc	(size_t _size);

	#define malloc symbian_malloc
#else // GCCE and the rest
	extern "C" int symbian_snprintf(char *text, size_t maxlen, const char *fmt, ...);
	extern "C" int symbian_vsnprintf(char *text, size_t maxlen, const char *fmt, va_list ap);
	#define snprintf(buf,len,args...) symbian_snprintf(buf,len,args)
	#define vsnprintf(buf,len,format,valist) symbian_vsnprintf(buf,len,format,valist)
#endif


#ifndef signbit
#define signbit(x)     \
	((sizeof (x) == sizeof (float)) ? __signbitf(x) \
	: (sizeof (x) == sizeof (double)) ? __signbit(x) \
	: __signbitl(x))
#endif

// Functions from openlibm not declared in Symbian math.h
extern "C"{
	float  roundf (float x);
	double nearbyint(double x);
	double round(double x);
	long   lround(double);
	int  __signbit(double);
	int  __signbitf(float);
	int  __signbitl(long double);
	float  truncf(float);
	float  fminf(float x, float y);
	float  fmaxf(float x, float y);
	double fmax (double x, double y);
	long long int strtoll(const char* start, char** end, int radix);
}


#ifndef __WINS__
// yuv2rgb functions from theorarm
extern "C"{
#ifdef COMMON_INTTYPES_H // That header has own inttypes declaration.
#define HAVE_INTTYPES_H  // So we switch off it to avoid conflict declarations.
#endif
#include <theora/yuv2rgb.h>

#ifndef COMMON_INTTYPES_H // No conflict.
#define COMMON_INTTYPES_H
#endif
}
#define USE_ARM_YUV2RGB_ASM
#define USE_ARM_GFX_ASM
#define USE_ARM_SMUSH_ASM
#define USE_ARM_COSTUME_ASM
#endif

// Symbian bsearch implementation is flawed
void *scumm_bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
#define bsearch	scumm_bsearch
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_fclose
#define FORBIDDEN_SYMBOL_EXCEPTION_fopen
#define FORBIDDEN_SYMBOL_EXCEPTION_unlink
#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd
#define FORBIDDEN_SYMBOL_EXCEPTION_stdout
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr

#if defined(__GNUC__)
# define va_copy(dst, src) __builtin_va_copy(dst, src)
#endif

// we cannot include SymbianOS.h everywhere, but this works too (functions code is in SymbianOS.cpp)
namespace Symbian {
extern char* GetExecutablePath();
}
#endif
