/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */
/**************************************************************************
 *                                     様様様様様様様様様様様様様様様様様 *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    様様様様様様様様様様様様様様様様様 *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Custom.CPP...........  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c  Desc:    Custom functions.....  *
 *  z$$beu     .ue="  $  "=e..    .zed$$c          .....................  *
 *      "#$e z$*"   .  `.   ^*Nc e$""              .....................  *
 *         "$$".  .r"   ^4.  .^$$"                 .....................  *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************/

#ifndef TONY_CUSTOM_H
#define TONY_CUSTOM_H

#include "common/str.h"
#include "tony/mpal/mpal.h"

namespace Tony {

using namespace MPAL;

/*
 *	Defines
 */

typedef uint32 HWND;

#define INIT_CUSTOM_FUNCTION					MapCustomFunctions
#define INIT_CUSTOM_FUNCTION_STRING		"MapCustomFunctions"

#define DECLARE_CUSTOM_FUNCTION(x)		void x

#define BEGIN_CUSTOM_FUNCTION_MAP()																					\
	static void AssignError(int num)	{ \
		error("Custom function %u has been already assigned!", num);		\
	}																																					\
	void INIT_CUSTOM_FUNCTION(LPCUSTOMFUNCTION *lpMap, Common::String *lpStrMap) \
	{																																				


#define END_CUSTOM_FUNCTION_MAP()		\
	}


#define ASSIGN(num, func)				\
	if (lpMap[num] != NULL)				\
		AssignError(num);				\
	lpMap[num] = func;					\
	lpStrMap[num] = #func;

class RMTony;
class RMPointer;
class RMGameBoxes;
class RMLocation;
class RMInventory;
class RMInput;

void INIT_CUSTOM_FUNCTION(LPCUSTOMFUNCTION *lpMap, Common::String *lpStrMap);
void SetupGlobalVars(RMTony *tony, RMPointer *ptr, RMGameBoxes *box, RMLocation *loc, RMInventory *inv, RMInput *input);

#endif

} // end of namespace Tony
