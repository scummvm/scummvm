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
 * Copyright (c) 2002 - 2023 Magnus Lind.
 *
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */

#ifndef GLK_SCOTT_EXO_UTIL_H
#define GLK_SCOTT_EXO_UTIL_H

#include "glk/scott/types.h"

namespace Glk {
namespace Scott {

struct LoadInfo {
	int _basicTxtStart; /* in */
	int _basicVarStart; /* out */
	int _run;           /* out */
	int _start;         /* out */
	int _end;           /* out */
};

int findSys(const byte *buf, int target);

void loadData(uint8_t *data, size_t dataLength, byte mem[65536], LoadInfo *info);

int strToInt(const char *str, int *value);

bool u32eq(const unsigned char *addr, uint32_t val);
bool u16eq(const unsigned char *addr, uint16_t val);
bool u16gteq(const unsigned char *addr, uint16_t val);
bool u16lteq(const unsigned char *addr, uint16_t val);
bool u16noteq(const unsigned char *addr, uint16_t val);
bool u32eqmasked(const unsigned char *addr, uint32_t mask, uint32_t val);
bool u32eqxored(const unsigned char *addr, uint32_t ormask, uint32_t val);
bool u16eqmasked(const unsigned char *addr, uint16_t mask, uint16_t val);

} // End of namespace Scott
} // End of namespace Glk

#endif
