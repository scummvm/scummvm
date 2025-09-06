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

#ifndef BAGEL_METAGAME_BGEN_BGEN_H
#define BAGEL_METAGAME_BGEN_BGEN_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/metagame/bgen/bdbg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

// the following are machine-independent definitions of the
// largest positive signed and and unsigned integers and longs
#define MAXPOSINT ((int)(((unsigned int)-1) >> 1))
#define MAXPOSLONG ((long)(((unsigned long)-1L) >> 1))
#define MAXUINT ((unsigned int)-1)
#define MAXULONG ((unsigned long)-1L)
#define MINNEGINT (-MAXPOSINT-1)
#define MINNEGLONG (-MAXPOSLONG-1)

#define DIMENSION(a) (sizeof(a) / sizeof(a[0]))
#define UPPERBOUND(a) (DIMENSION(a) - 1)

typedef char *XPSTR ;
typedef char *XPSTR;
typedef int *XPINT ;
typedef char *XPCHAR ;
typedef void *XPVOID ;

// RGB colors
#define RGBCOLOR_RED        RGB(255,   0,   0)
#define RGBCOLOR_GREEN      RGB(  0, 255,   0)
#define RGBCOLOR_BLUE       RGB(  0,   0, 255)
#define RGBCOLOR_YELLOW     RGB(255, 255,   0)
#define RGBCOLOR_MAGENTA    RGB(255,   0, 255)
#define RGBCOLOR_CYAN       RGB(  0, 255, 255)
#define RGBCOLOR_BLACK      RGB(  0,   0,   0)
#define RGBCOLOR_GREY       RGB(128, 128, 128)
#define RGBCOLOR_BROWN      RGB(128, 128,   0)
#define RGBCOLOR_PURPLE     RGB(128,   0, 128)
#define RGBCOLOR_AQUA       RGB(  0, 128, 128)
#define RGBCOLOR_WHITE      RGB(255, 255, 255)

// mouse button click types
#define CLICK_UMOVE 1
#define CLICK_LDOWN 2
#define CLICK_LMOVE 3
#define CLICK_LUP 4
#define CLICK_LDOUBLE 5
#define CLICK_RDOWN 6
#define CLICK_RMOVE 7
#define CLICK_RUP 8
#define CLICK_RDOUBLE 9

// ASCII characters
#define CCHBOF 1        /* beginning of file char (control A) */
#define CCHBKS 8        /* backspace */
#define CCHTAB 9        /* tab */
#define CCHLF 10        /* line feed - '\n' - newline */
#define CCHFF 12        /* form feed */
#define CCHCR 13        /* carriage return */
#define CCHLF 10        /* line feed */
#define CCHEOF 26       /* end of file char (control Z) */
#define CCHESC 27       /* escape */
#define CCHBL 32        /* blank */
#define CCHDIG(n) 48+n /* 48-57 */  /* digits 0-9 */
#define CCHTXL 32       /* first text character */
#define CCHTXH 127      /* last text character */

#define CCHUND 95       /* underline */
#define CCHLPR '('      /* left paren */
#define CCHRPR ')'      /* right paren */
#define CCHQUO '"'      /* quote */
#define CCHAPO '\''     /* apostrophe (single quote) */
#define CCHLCB '{'      /* left curly bracket */
#define CCHRCB '}'      /* right curly bracket */
#define CCHCOL ':'
#define CCHDOT '.'

#define CCHCTL(let) (let-'A'+1) /* control chars: A-Z */

/* old scan codes -- add 1000 to scan code from IBM PC keyboard */
#define SCANMIN 1000        /* minimum scan code */

#define SCANF(dig) (1058+dig)   /* function keys: 1059-1068 */
#define SCANSF(dig) (1083+dig)  /* shift function keys: 1084-1093 */
#define SCANCF(dig) (1093+dig)  /* control function keys: 1094-1103 */
#define SCANAF(dig) (1103+dig)  /* alt function keys: 1104-1113 */

// code to be used with Windows virtual key codes
#define CCH_ALTKEY 0x100
#define CCH_CTRLKEY 0x200
#define CCH_SHFTKEY 0x400
#define ALTKEY(ch) ((ch)+CCH_ALTKEY)
#define CTRLKEY(ch) ((ch)+CCH_CTRLKEY)
#define SHFTKEY(ch) ((ch)+CCH_SHFTKEY)

inline bool EQV(bool b1, bool b2) {
	return (b1 != 0) == (b2 != 0);
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
