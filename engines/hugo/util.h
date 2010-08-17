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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_UTIL_H
#define HUGO_UTIL_H

namespace Hugo {

enum seqTextUtil {
	kTech     = 0,
	kErr1     = 1,
	kErr2     = 2,
	kErr3     = 3,
	kErr4     = 4,
	kErr5     = 5,
	kErr6     = 6,
	kGameOver = 7
//	kObsoleteErr1 = 8,
//	kObsoleteErr2 = 9
};

namespace Utils {
int      firstBit(byte data);
int      lastBit(byte data);
void     reverseByte(byte *data);
void     Warn(bool technote, const char *format, ...);
void     Error(int code, const char *format, ...);
void     gameOverMsg();
//	void     Debug_out(char *format, ...);
char    *Box(box_t, const char *, ...);
}

} // Namespace Hugo
#endif
