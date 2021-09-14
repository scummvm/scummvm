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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef CHAMBER_COMMON_H
#define CHAMBER_COMMON_H

namespace Chamber {

#define BE(x) (((x) >> 8) | ((x) << 8))
#define LE16(x) (x)

#define TODO(s) \
	{   \
		printf(s);  \
		PromptWait();   \
		for(;;) ;   \
	}

typedef struct rect_t {
	unsigned char sx;
	unsigned char ex;
	unsigned char sy;
	unsigned char ey;
} rect_t;

#if sizeof(rect_t) != 4
#error "rect_t must be 4 bytes long"
#endif

#if 0
#define DEBUG
#endif

#if 1
#define DEBUG_ZONE
#endif

#if 0
/*Rope -> De Profundis*/
#define DEBUG_QUEST 0x00
#endif

#if 0
/*Knife -> The Wall*/
#define DEBUG_QUEST 0x40
#endif

#if 1
/*Goblet -> The Twins*/
#define DEBUG_QUEST 0x80
#endif

#if 0
/*Fly -> Scorpion's*/
#define DEBUG_QUEST 0xC0
#endif

#if 0
#define CHEAT
#endif

} // End of namespace Chamber

#endif
