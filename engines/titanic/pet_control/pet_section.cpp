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

#include "common/textconsole.h"
#include "titanic/pet_control/pet_section.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

static const uint ARRAY1[6] = {
	0xA7C0DB, 0x9CFFFE, 0x73AEFF, 0xA7C0DB, 0x9CFFFE, 0
};

static const uint ARRAY2[6] = {
	0x10101, 0x1013C, 0xC80101, 0x10101, 0x800101, 0
};

static const uint ARRAY3[5] = {
	0x10101, 0x1013C, 0xC80101, 0x10101, 0x800101
};

void CPetSection::displayMessage(const CString &msg) {
	error("TODO");
}

void CPetSection::proc25() {
	error("TODO");
}

void CPetSection::proc27() {
	error("TODO");
}

void CPetSection::proc28() {
	error("TODO");
}

void CPetSection::proc29() {
	error("TODO");
}

void CPetSection::proc30() {
	error("TODO");
}

uint CPetSection::getDataIndex(int index) {
	return getDataTable()[index];
}

const uint *CPetSection::getDataTable(int index) {
	if (index == -1) {
		CPetControl *pet = getPetControl();
		index = pet ? pet->getState8() : 3;
	}

	switch (index) {
	case 1: return ARRAY1;
	case 2: return ARRAY2;
	default: return ARRAY3;
	}
}

} // End of namespace Titanic
