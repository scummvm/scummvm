/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 Andreas 'Sprawl' Karlsson - Original EPOC port, ESDL
 * Copyright (C) 2003-2005 Lars 'AnotherGuest' Persson - Original EPOC port, Audio System
 * Copyright (C) 2005 Jurgen 'SumthinWicked' Braam - EPOC/CVS maintainer
 * Copyright (C) 2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
#define SYMBIAN32_DOC_DIR "C:\\documents\\EScummVM\\" // includes final \\!
#define DISABLE_SCALERS // we only need 1x

// hack in some tricks to work around not having these fcns for Symbian
// and we _really_ don't wanna link with any other windows LIBC library!
#ifdef __GCC32__

	#define snprintf(buf,len,args...)	sprintf(buf,args)
	#define vsnprintf					snprintf

#else // WINS

	// let's just blatantly ignore this for now and just get it to work :P but does n't work from the debug function
	int inline scumm_snprintf (char *str, unsigned long /*n*/, char const *fmt, ...)
	{
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

	for (i=0; i<nmemb; i++)
		if (compar(key, (void*)((size_t)base + size * i)) == 0)
			return (void*)((size_t)base + size * i);
	return NULL;
}
#define bsearch						scumm_bsearch

