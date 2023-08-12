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

// Atari a.out / ELF symbol handling by Thorsten Otto.
// include in each of the .S files and put every global symbol into
// SYM(<symbol name>).

#ifndef __USER_LABEL_PREFIX__
#define __USER_LABEL_PREFIX__ _
#endif

#ifndef __REGISTER_PREFIX__
#define __REGISTER_PREFIX__
#endif

#ifndef __IMMEDIATE_PREFIX__
#define __IMMEDIATE_PREFIX__ #
#endif

#define CONCAT1(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a ## b

/* Use the right prefix for global labels.  */

#define SYM(x) CONCAT1 (__USER_LABEL_PREFIX__, x)

#ifdef __ELF__
#define FUNC(x) .type SYM(x),function
#else
/* The .proc pseudo-op is accepted, but ignored, by GAS.  We could just	
   define this to the empty string for non-ELF systems, but defining it
   to .proc means that the information is available to the assembler if
   the need arises.  */
#define FUNC(x) .proc
#endif
		
#define REG(x) CONCAT1 (__REGISTER_PREFIX__, x)

#define IMM(x) CONCAT1 (__IMMEDIATE_PREFIX__, x)

#define d0 REG(d0)
#define d1 REG(d1)
#define d2 REG(d2)
#define d3 REG(d3)
#define d4 REG(d4)
#define d5 REG(d5)
#define d6 REG(d6)
#define d7 REG(d7)
#define a0 REG(a0)
#define a1 REG(a1)
#define a2 REG(a2)
#define a3 REG(a3)
#define a4 REG(a4)
#define a5 REG(a5)
#define a6 REG(a6)
#define a7 REG(a7)
#define fp REG(fp)
#define sp REG(sp)
#define pc REG(pc)

#define sr REG(sr)
