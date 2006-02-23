/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#include "cine/cine.h"

void flipU16(uint16 * pVar) {
	*pVar = (((*pVar) & 0xFF) << 8) | (((*pVar) & 0xFF00) >> 8);
}

void flipU32(uint32 * pVar) {
	uint16 part1;
	uint16 part2;

	part1 = (uint16) ((*pVar) & 0xFFFF);
	part2 = (uint16) (((*pVar) & 0xFFFF0000) >> 16);

	flipU16(&part1);
	flipU16(&part2);

	*pVar = (part2) | ((uint32) part1 << 16);
}

uint16 readU16LE(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[1] << 8) | b[0];
}

uint16 readU16BE(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[0] << 8) | b[1];
}

uint32 readU32BE(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}
