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

/*
******************************************************************************
**                                      **
**           XTOOL  Windows 32 API version 3.0nt            **
**             for Windows-NT, Windows 95               **
**                                      **
**    (C)1993-97 by K-D Lab::KranK, K-D Lab::Steeler, Dr.Tronick        **
**                                      **
******************************************************************************
*/

#ifndef QDENGINE_XUTIL_XUTIL_H
#define QDENGINE_XUTIL_XUTIL_H

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include <memory.h>
#include <cstdlib>
#include "common/textconsole.h"




///////////////////////////////////
//		XBuffer
///////////////////////////////////
#define CONV_BUFFER_LEN 63

#define XB_DEFSIZE  256

#define XB_BEG      0
#define XB_CUR      1
#define XB_END      2

#define XB_FORWARD  0
#define XB_BACKWARD 1
#define XB_GLOBAL   2

#define XB_CASEON   0
#define XB_CASEOFF  1

#define XB_DEFRADIX 10
#define XB_DEFDIGITS    8

namespace QDEngine {


///////////////////////////////////
//		XStream
///////////////////////////////////

#define XS_IN       0x0001
#define XS_OUT      0x0002
#define XS_NOREPLACE    0x0004
#define XS_APPEND   0x0008
#define XS_NOBUFFERING  0x0010
#define XS_NOSHARING    0x0020

#define XS_SHAREREAD    0x0040
#define XS_SHAREWRITE   0x0080

#define XS_BEG      0
#define XS_CUR      1
#define XS_END      2

#define XS_DEFRADIX 10
#define XS_DEFDIGITS    8



///////////////////////////////////
//		ErrH
///////////////////////////////////

#ifndef NULL
#define NULL    0L
#endif

#define X_WINNT         0x0001
#define X_WIN32S        0x0002
#define X_WIN95         0x0003

#define XERR_ALL        0xFFFF
#define XERR_NONE       0x0000

#define XERR_USER       0x0001
#define XERR_CTRLBRK        0x0004
#define XERR_MATH       0x0008
#define XERR_FLOAT      0x0010
#define XERR_CRITICAL       0x0020
#define XERR_SIGNAL     0x0040

#define XERR_ACCESSVIOLATION    0x0080
#define XERR_DEBUG      0x0100
#define XERR_UNKNOWN        0x0200

#define xassert(exp) assert(exp)
#define xassertStr(exp, str)

///////////////////////////////////
//		Utils
///////////////////////////////////

void *xalloc(unsigned sz);
void *xrealloc(void *p, unsigned sz);
void  xfree(void *p);

#define XALLOC(a)   xalloc(a)
#define XREALLOC(a,b)   xrealloc(a,b)
#define XFREE(a)    xfree(a)

unsigned int XRnd(unsigned int m);
void XRndSet(unsigned int m);
unsigned int XRndGet();

char *XFindNext();
char *XFindFirst(char *mask);

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#ifndef __ROUND__
#define __ROUND__

template <class T>
inline T sqr(const T &x) {
	return x*x;
}

template <class T>
inline int SIGN(const T &x) {
	return x ? (x > 0 ? 1 : -1) : 0;
}

#endif // __ROUND__

inline int BitSR(int x) {
	int return_var = 0;

	warning("STUB: BitSR()");

#if 0
	_asm {
		mov eax, x
		cdq
		xor eax, edx
		sub     eax, edx
		bsr     eax, eax
		mov[return_var], eax
	}
#endif
	return return_var;
}


void xtDeleteFile(char *fname);

///////////////////////////////////

const char *check_command_line(const char *switch_str); // 0 ��� ������ ����� ����� ��� �������

template<class T> // ��� ��������� ����������
bool check_command_line_parameter(const char *switch_str, T &parameter) {
	const char *s = check_command_line(switch_str);
	if (s) {
		parameter = atoi(s);
		return true;
	} else return false;
}

// ����� Unicode ������ � �������
//void dcprintfW(wchar_t *format, ...);
// ����� Unicode ������ � Debug Output
//void dprintfW(wchar_t *format, ...);

///////////////////////////////////
//		__XCPUID_H
///////////////////////////////////

#define CPU_INTEL       0x00000001

// Intel features IDs

#define INTEL_MMX       0x00800000

// Intel Family IDs
#define CPU_INTEL_386       3
#define CPU_INTEL_486       4
#define CPU_INTEL_PENTIUM   5
#define CPU_INTEL_PENTIUM_PRO   6

//Intel Pentium model's
#define PENTIUM_P5A     0
#define PENTIUM_P5      1
#define PENTIUM_P54C        2
#define PENTIUM_P54T        3
#define PENTIUM_MMX     4
#define PENTIUM_DX4     5
#define PENTIUM_P5OVER      6
#define PENTIUM_P54CA       7

// Intel Pentium Pro model's
#define PENTIUM_PROA        0
#define PENTIUM_PRO     1
#define PENTIUM_PRO2        3
#define PENTIUM_PRO55CT     4
#define PENTIUM_PROF        5

extern unsigned int xt_processor_id;
extern unsigned int xt_processor_type;
extern unsigned int xt_processor_family;
extern unsigned int xt_processor_model;
extern unsigned int xt_processor_revision;
extern unsigned int xt_processor_features;

unsigned int xt_get_cpuid();
char *xt_getMMXstatus();

extern int xt_mmxUse;


///////////////////////////////////
//		__XZIP_H__
///////////////////////////////////

typedef unsigned long ulong;

ulong ZIP_compress(char *trg, ulong trgsize, char *src, ulong srcsize);
ulong ZIP_GetExpandedSize(char *p);
void ZIP_expand(char *trg, ulong trgsize, char *src, ulong srcsize);


///////////////////////////////////
//		Automatic linking
///////////////////////////////////
#ifndef _XUTIL_NO_AUTOMATIC_LIB

#define _LIB_NAME "XUtil"

#endif // _XUTIL_NO_AUTOMATIC_LIB

} // namespace QDEngine

#endif // QDENGINE_XUTIL_XUTIL_H
