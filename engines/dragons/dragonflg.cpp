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
#include "dragonflg.h"
#include "bigfile.h"


namespace Dragons {

// Properties

Properties::Properties()
		: _count(0), _properties(0) {
}

void Properties::init(uint count, byte *properties) {
	_count = count;
	_properties = properties;
}

void Properties::clear() {
	uint32 size = getSize();
	for (uint32 i = 0; i < size; ++i) {
		_properties[i] = 0;
	}
}

bool Properties::get(uint32 propertyId) {
	uint index;
	byte mask;
	getProperyPos(propertyId, index, mask);
	return (_properties[index] & mask) != 0;
}

void Properties::set(uint32 propertyId, bool value) {
	uint index;
	byte mask;
	getProperyPos(propertyId, index, mask);
	if (value)
		_properties[index] |= mask;
	else
		_properties[index] &= ~mask;
}

uint32 Properties::getSize() {
	return (_count >> 3) + 1;
}

void Properties::getProperyPos(uint32 propertyId, uint &index, byte &mask) {
	assert(propertyId < _count);
	propertyId &= 0xFFFF;
	index = propertyId >> 3;
	mask = 1 << (propertyId & 7);
}

DragonFLG::DragonFLG(BigfileArchive *bigfileArchive) {
	_data = bigfileArchive->load("dragon.flg", _dataSize);
	properties.init(288, _data);
}

DragonFLG::~DragonFLG() {
	if (_data) {
		delete _data;
	}
}

bool DragonFLG::get(uint32 propertyId) {
	return properties.get(propertyId);
}

void DragonFLG::set(uint32 propertyId, bool value) {
	properties.set(propertyId, value);
}

} // End of namespace Dragons
