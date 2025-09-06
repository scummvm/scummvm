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

#ifndef HODJNPODJ_HNPLIBS_STDINC_H
#define HODJNPODJ_HNPLIBS_STDINC_H

#include "bagel/afxwin.h"
#include "common/algorithm.h"

namespace Bagel {
namespace HodjNPodj {

#define BOF_DEBUG 1

/*
* Math Function Macros
*/
#define isodd(x) ((x)&1)                    /* Returns 1 if number is odd  */
#define iseven(x) !((x)&1)                  /* Returns 1 if number is even */
#ifndef min
	#define min(x,y) (x<y?x:y)                  /* returns the min of x and y */
#endif
#ifndef max
	#define max(x,y) (x>y?x:y)                  /* returns the max of x and y */
#endif
#ifndef abs
	#define abs(x) ((x)<0?-(x):(x))             /* returns the absolute value of x */
#endif

/*
* normal types
*/
#define CDECL
#ifndef FAR
	#define FAR
#endif

enum {
	YES = 1,
	NO = 0
};

#ifndef MAX_FNAME
	#define MAX_FNAME 256
#endif

#define MAX_DIRPATH 256

#define Assert(f) assert(f)
#define TRACE(s) debug("%s", (s))

#define WF_80x87  1024
#define WF_CPU186  128
#define WF_CPU286    2
#define WF_CPU386    4
#define WF_CPU486    8
#define WF_STANDARD 16
#define WF_ENHANCED 32

inline unsigned int GetWinFlags() {
	return 0;
}

} // namespace HodjNPodj
} // namespace Bagel

#endif
