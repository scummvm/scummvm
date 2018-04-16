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

#include "common/file.h"

#include "lab/lab.h"

#include "lab/labsets.h"
#include "lab/resource.h"

namespace Lab {

LargeSet::LargeSet(uint16 last, LabEngine *vm) : _vm(vm) {
	last = (((last + 15) >> 4) << 4);

	_array = new uint16[last >> 3];
	memset(_array, 0, last >> 3);
	_lastElement = last;
}

LargeSet::~LargeSet() {
	delete[] _array;
}

bool LargeSet::in(uint16 element) {
	return ((1 << ((element - 1) % 16)) & (_array[(element - 1) >> 4])) > 0;
}

void LargeSet::inclElement(uint16 element) {
	_array[(element - 1) >> 4] |= 1 << ((element - 1) % 16);
}

void LargeSet::exclElement(uint16 element) {
	_array[(element - 1) >> 4] &= ~(1 << ((element - 1) % 16));
}

bool LargeSet::readInitialConditions(const Common::String fileName) {
	Common::File *file = _vm->_resource->openDataFile(fileName, MKTAG('C', 'O', 'N', '0'));

	uint16 conditions = file->readUint16LE();
	for (int i = 0; i < conditions; i++) {
		inclElement(file->readUint16LE());
	}

	delete file;
	return true;
}


} // End of namespace Lab
