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

#include "common/file.h"

#include "asylum/encounters.h"

namespace Asylum {

Encounter::Encounter() {
	Common::File file;

	// TODO error checks
	file.open("sntrm.dat");

	uint16 _count = file.readUint16LE();

	_variables = (uint16*)malloc(_count);
	file.read(_variables, _count);

	file.seek(2 + _count * 2, SEEK_SET);

	// TODO assert if true
	_anvilStyleFlag = file.readUint16LE();

	uint16 _dataCount = file.readUint16LE();

	for (uint8 i = 0; i < _dataCount; i++) {
		EncounterItem item;
		memset(&item, 0, sizeof(EncounterItem));

		item.keywordIndex = file.readUint32LE();
		item.field2       = file.readUint32LE();
		item.scriptResId  = file.readUint32LE();
		for (uint8 j = 0; j < 50; j++) {
			item.array[j] = file.readUint32LE();
		}
		item.value = file.readUint16LE();

		_items.push_back(item);
	}

	file.close();
}

Encounter::~Encounter() {
	// TODO Auto-generated destructor stub
}

}
