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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/stddefines.h"
#include "lab/labfun.h"

namespace Lab {

LabSet::LabSet(uint16 last) {
	last = (((last + 15) >> 4) << 4);

	_array = (uint16 *)calloc(last >> 3);
}

LargeSet::~LargeSet() {
	free(_array);
}

bool LargeSet::in(uint16 element) {
	return ((1 << ((element - 1) % 16)) & (_array[(element - 1) >> 4])) > 0;
}

void LargeSet::inclElement(uint16 element) {
	_array[(element - 1) >> 4]) |= 1 << ((element - 1) % 16);
}

void LargeSet::exclElement(uint16 element) {
	_array[(element - 1) >> 4] &= ~(1 << ((element - 1) % 16));
}

bool LargeSet::readInitialConditions(const char *fileName) {
	byte **file;
	uint16 many, set;
	char temp[5];

	if ((file = g_music->newOpen(fileName)) != NULL) {
		readBlock(temp, 4L, file);
		temp[4] = '\0';

		if (strcmp(temp, "CON0") != 0)
			return false;

		readBlock(&many, 2L, file);
#if !defined(DOSCODE)
		swapUShortPtr(&many, 1);
#endif

		for (int counter = 0; counter < many; counter++) {
			readBlock(&set, 2L, file);
#if !defined(DOSCODE)
			swapUShortPtr(&set, 1);
#endif
			inclElement(set);
		}
	} else
		return false;

	return true;
}


} // End of namespace Lab
