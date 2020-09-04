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
#include "common/debug.h"
#include "dragons/dragonflg.h"
#include "dragons/bigfile.h"


namespace Dragons {

// Properties

Properties::Properties(uint count) : _count(count) {
	_properties = (byte *)malloc(getSize());
	if (!_properties) {
		error("Failed to allocate mem for properties");
	}
	memset(_properties, 0, getSize());
}

Properties::~Properties() {
	free(_properties);
}

void Properties::init(uint count, byte *properties) {
	assert(count <= getSize());
	memcpy(_properties, properties, count);
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
	index = propertyId / 8;
	mask = 1 << (propertyId % 8);
}

void Properties::save(uint numberToWrite, Common::WriteStream *out) {
	assert(numberToWrite % 8 == 0);
	assert(numberToWrite <= _count);
	out->write(_properties, numberToWrite / 8);
}

void Properties::print(char *prefix) {
	char *str = new char[_count + 1];
	uint i = 0;
	for (; i < _count; i++) {
		str[i] = get(i) ? '1' : '0';
	}
	str[i] = 0;
	debug(3, "%s: props = %s", prefix, str);

	delete[] str;
}

DragonFLG::DragonFLG(BigfileArchive *bigfileArchive) {
	_data = bigfileArchive->load("dragon.flg", _dataSize);
	_properties = new Properties(288);
	_properties->init(_dataSize, _data);
}

DragonFLG::~DragonFLG() {
	delete _data;
	delete _properties;
}

bool DragonFLG::get(uint32 propertyId) {
	return _properties->get(propertyId);
}

void DragonFLG::set(uint32 propertyId, bool value) {
	_properties->set(propertyId, value);
}

void DragonFLG::saveState(Common::WriteStream *out) {
	//properties->print("save");
	_properties->save(128, out); // save first 80 flags.
}

void DragonFLG::loadState(Common::ReadStream *in) {
	byte savedState[0x10];

	_properties->init(_dataSize, _data);

	in->read(savedState, 0x10);
	_properties->init(0x10, savedState);
	//properties->print("load");
}

} // End of namespace Dragons
