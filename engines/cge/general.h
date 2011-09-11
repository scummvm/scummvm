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
 */

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE_GENERAL_H
#define CGE_GENERAL_H

#include "common/system.h"
#include "common/file.h"
#include "common/random.h"
#include "common/textconsole.h"
#include "common/str.h"

namespace CGE {

#define kCryptSeed  0xA5
#define kMaxFile      128

struct Dac {
	uint8 _r;
	uint8 _g;
	uint8 _b;
};

typedef uint16 Crypt(void *buf, uint16 siz);

uint16  XCrypt(void *buf, uint16 siz);
int     takeEnum(const char **tab, const char *text);
uint16  chkSum(void *m, uint16 n);
char   *mergeExt(char *buf, const char *name, const char *ext);
char   *forceExt(char *buf, const char *name, const char *ext);
int     newRandom(int range);

} // End of namespace CGE

#endif
