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

#include "titanic/star_control/base_star.h"
#include "titanic/titanic.h"

namespace Titanic {

CBaseStarEntry::CBaseStarEntry() : _field0(0), _value(0.0) {
	Common::fill(&_data[0], &_data[5], 0);
}

void CBaseStarEntry::load(Common::SeekableReadStream &s) {
	_field0 = s.readByte();
	_field1 = s.readByte();
	_field2 = s.readByte();
	_field3 = s.readByte();
	*((uint32 *)&_value) = s.readUint32LE();		// FIXME
	_val._v1 = s.readUint32LE();
	_val._v2 = s.readUint32LE();
	_val._v3 = s.readUint32LE();

	for (int idx = 0; idx < 5; ++idx)
		_data[idx] = s.readUint32LE();
}

/*------------------------------------------------------------------------*/

CBaseStar::CBaseStar() : _minVal(0.0), _maxVal(1.0), _range(0.0) {
}

void CBaseStar::draw(CSurfaceArea *surfaceArea, CStarControlSub12 *sub12, CStarControlSub5 *sub5) {
	// TODO
}

void CBaseStar::clear() {
	_data.clear();
}

void CBaseStar::initialize() {
	_minVal = 9.9999998e10;
	_maxVal = -9.9999998e10;
	_sub4.initialize();

	for (uint idx = 0; idx < _data.size(); ++idx) {
		const CBaseStarEntry *entry = getDataPtr(idx);
		_sub4.checkEntry(entry->_val);

		if (entry->_value < _minVal)
			_minVal = entry->_value;
		if (entry->_value > _maxVal)
			_maxVal = entry->_value;
	}

	_range = (_maxVal - _minVal) / 1.0;
}

CBaseStarEntry *CBaseStar::getDataPtr(int index) {
	return (index >= 0 && index < (int)_data.size()) ? &_data[index] : nullptr;
}

void CBaseStar::loadData(Common::SeekableReadStream &s) {
	uint headerId = s.readUint32LE();
	uint count = s.readUint32LE();
	if (headerId != 100 || count == 0)
		error("Invalid star data");

	// Initialize the data array
	clear();
	_data.resize(count);

	// Iterate through reading the data for each entry
	for (uint idx = 0; idx < count; ++idx)
		_data[idx].load(s);
}

void CBaseStar::loadData(const CString &resName) {
	// Get a stream to read the data from the DAT file
	Common::SeekableReadStream *stream = g_vm->_filesManager->getResource(resName);
	assert(stream);

	// Load the stream
	loadData(*stream);
	delete stream;
}

void CBaseStar::resetEntry(CBaseStarEntry &entry) {
	entry._field0 = 0xFF;
	entry._field1 = 0xFF;
	entry._field2 = 0xFF;
	entry._field3 = 0;
	entry._val._v1 = 0;
	entry._val._v2 = 0;
	entry._val._v3 = 0;
	for (int idx = 0; idx < 5; ++idx)
		entry._data[idx] = 0;
}

} // End of namespace Titanic
