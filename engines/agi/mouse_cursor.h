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

#ifndef AGI_MOUSE_CURSOR_H
#define AGI_MOUSE_CURSOR_H

namespace Agi {

/**
 * RGB-palette for the Amiga-style arrow cursor
 * and the Amiga-style busy cursor.
 */
static const byte MOUSECURSOR_PALETTE[] = {
	0x00, 0x00, 0x00, // Black
	0xFF, 0xFF, 0xFF, // White
	0xDE, 0x20, 0x21, // Red
	0xFF, 0xCF, 0xAD  // Light red
};

/**
 * A black and white SCI-style arrow cursor (11x16).
 * 0 = Transparent.
 * 1 = Black (#000000 in 24-bit RGB).
 * 2 = White (#FFFFFF in 24-bit RGB).
 */
static const byte MOUSECURSOR_SCI[] = {
	1,1,0,0,0,0,0,0,0,0,0,
	1,2,1,0,0,0,0,0,0,0,0,
	1,2,2,1,0,0,0,0,0,0,0,
	1,2,2,2,1,0,0,0,0,0,0,
	1,2,2,2,2,1,0,0,0,0,0,
	1,2,2,2,2,2,1,0,0,0,0,
	1,2,2,2,2,2,2,1,0,0,0,
	1,2,2,2,2,2,2,2,1,0,0,
	1,2,2,2,2,2,2,2,2,1,0,
	1,2,2,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,1,0,0,0,0,
	1,2,1,0,1,2,2,1,0,0,0,
	1,1,0,0,1,2,2,1,0,0,0,
	0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,1,2,2,1,0
};

/**
 * A black and white SCI-style busy cursor (15x16).
 * 0 = Transparent.
 * 1 = Black (#000000 in 24-bit RGB).
 * 2 = White (#FFFFFF in 24-bit RGB).
 */
static const byte MOUSECURSOR_SCI_BUSY[] = {
	0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
	0,0,0,0,1,1,1,2,2,1,1,1,0,0,0,
	0,0,0,1,2,2,1,2,2,1,2,2,1,0,0,
	0,1,1,1,2,2,1,2,2,1,2,2,1,0,0,
	1,2,2,1,2,2,1,2,2,1,2,2,1,0,0,
	1,2,2,1,2,2,1,2,2,1,2,2,1,0,0,
	1,2,2,1,2,2,1,2,2,1,2,1,2,1,0,
	1,2,2,1,2,2,1,2,2,1,1,2,2,1,1,
	1,2,2,1,2,2,1,2,2,1,1,2,2,2,1,
	1,2,2,2,2,2,2,2,2,1,1,2,2,2,1,
	1,2,2,2,2,2,2,2,1,2,2,2,2,1,0,
	1,2,2,2,2,2,2,1,2,2,2,2,2,1,0,
	0,1,2,2,2,2,2,1,2,2,2,2,1,0,0,
	0,1,2,2,2,2,2,2,2,2,2,2,1,0,0,
	0,0,1,2,2,2,2,2,2,2,2,1,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,0,0,0,0
};

/**
 * A black and white Atari ST style arrow cursor (11x16).
 * 0 = Transparent.
 * 1 = Black (#000000 in 24-bit RGB).
 * 2 = White (#FFFFFF in 24-bit RGB).
 */
static const byte MOUSECURSOR_ATARI_ST[] = {
	2,2,0,0,0,0,0,0,0,0,0,
	2,1,2,0,0,0,0,0,0,0,0,
	2,1,1,2,0,0,0,0,0,0,0,
	2,1,1,1,2,0,0,0,0,0,0,
	2,1,1,1,1,2,0,0,0,0,0,
	2,1,1,1,1,1,2,0,0,0,0,
	2,1,1,1,1,1,1,2,0,0,0,
	2,1,1,1,1,1,1,1,2,0,0,
	2,1,1,1,1,1,1,1,1,2,0,
	2,1,1,1,1,1,2,2,2,2,2,
	2,1,1,2,1,1,2,0,0,0,0,
	2,1,2,0,2,1,1,2,0,0,0,
	2,2,0,0,2,1,1,2,0,0,0,
	2,0,0,0,0,2,1,1,2,0,0,
	0,0,0,0,0,2,1,1,2,0,0,
	0,0,0,0,0,0,2,2,2,0,0
};

/**
 * A black and white Apple IIGS style arrow cursor (9x11).
 * 0 = Transparent.
 * 1 = Black (#000000 in 24-bit RGB).
 * 2 = White (#FFFFFF in 24-bit RGB).
 */
static const byte MOUSECURSOR_APPLE_II_GS[] = {
	2,2,0,0,0,0,0,0,0,
	2,1,2,0,0,0,0,0,0,
	2,1,1,2,0,0,0,0,0,
	2,1,1,1,2,0,0,0,0,
	2,1,1,1,1,2,0,0,0,
	2,1,1,1,1,1,2,0,0,
	2,1,1,1,1,1,1,2,0,
	2,1,1,1,1,1,1,1,2,
	2,1,1,2,1,1,2,2,0,
	2,2,2,0,2,1,1,2,0,
	0,0,0,0,0,2,2,2,0
};

/**
 * An Amiga-style arrow cursor (8x11).
 * 0 = Transparent.
 * 1 = Black     (#000000 in 24-bit RGB).
 * 3 = Red       (#DE2021 in 24-bit RGB).
 * 4 = Light red (#FFCFAD in 24-bit RGB).
 */
static const byte MOUSECURSOR_AMIGA[] = {
	3,4,1,0,0,0,0,0,
	3,3,4,1,0,0,0,0,
	3,3,3,4,1,0,0,0,
	3,3,3,3,4,1,0,0,
	3,3,3,3,3,4,1,0,
	3,3,3,3,3,3,4,1,
	3,0,3,3,4,1,0,0,
	0,0,0,3,4,1,0,0,
	0,0,0,3,3,4,1,0,
	0,0,0,0,3,4,1,0,
	0,0,0,0,3,3,4,1
};

/**
 * An Amiga-style busy cursor showing an hourglass (13x16).
 * 0 = Transparent.
 * 1 = Black     (#000000 in 24-bit RGB).
 * 3 = Red       (#DE2021 in 24-bit RGB).
 * 4 = Light red (#FFCFAD in 24-bit RGB).
 */
static const byte MOUSECURSOR_AMIGA_BUSY[] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,3,3,3,3,3,3,3,3,3,3,3,1,
	1,3,3,3,3,3,3,3,3,3,3,3,1,
	0,1,4,4,4,4,4,4,4,4,4,1,0,
	0,0,1,4,4,4,4,4,4,4,1,0,0,
	0,0,0,1,4,4,4,4,4,1,0,0,0,
	0,0,0,0,1,4,4,4,1,0,0,0,0,
	0,0,0,0,0,1,4,1,0,0,0,0,0,
	0,0,0,0,0,1,4,1,0,0,0,0,0,
	0,0,0,0,1,3,4,3,1,0,0,0,0,
	0,0,0,1,3,3,4,3,3,1,0,0,0,
	0,0,1,3,3,3,4,3,3,3,1,0,0,
	0,1,3,3,3,4,4,4,3,3,3,1,0,
	1,4,4,4,4,4,4,4,4,4,4,4,1,
	1,4,4,4,4,4,4,4,4,4,4,4,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1
};

/**
 * A Macintosh-style busy cursor showing an hourglass (10x14).
 * 0 = Transparent.
 * 1 = Black     (#000000 in 24-bit RGB).
 * 2 = White (#FFFFFF in 24-bit RGB).
 */
static const byte MOUSECURSOR_MACINTOSH_BUSY[] = {
	0,0,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,0,0,
	0,1,2,2,2,2,2,2,1,0,
	1,2,2,2,2,1,2,2,2,1,
	1,2,2,2,2,1,2,2,2,1,
	1,2,2,2,2,1,2,2,2,1,
	1,2,2,1,1,1,2,2,2,1,
	1,2,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,2,2,2,1,
	0,1,2,2,2,2,2,2,1,0,
	0,0,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,0,0
};

} // End of namespace Agi

#endif /* AGI_MOUSE_CURSOR_H */
