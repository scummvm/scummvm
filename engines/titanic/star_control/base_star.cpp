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
#include "titanic/star_control/star_control_sub12.h"
#include "titanic/star_control/star_ref.h"
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
	_value = s.readFloatLE();
	_position._x = s.readUint32LE();
	_position._y = s.readUint32LE();
	_position._z = s.readUint32LE();

	for (int idx = 0; idx < 5; ++idx)
		_data[idx] = s.readUint32LE();
}

bool CBaseStarEntry::operator==(const CBaseStarEntry &s) const {
	return _field0 == s._field0 && _field1 == s._field1
		&& _field2 == s._field2 && _field3 == s._field3
		&& _value == s._value && _position == s._position
		&& _data[0] == s._data[0] && _data[1] == s._data[1]
		&& _data[2] == s._data[2] && _data[3] == s._data[3]
		&& _data[4] == s._data[4];
}

/*------------------------------------------------------------------------*/

CBaseStar::CBaseStar() : _minVal(0.0), _maxVal(1.0), _range(0.0),
		_value1(0.0), _value2(0.0), _value3(0.0), _value4(0.0) {
}

void CBaseStar::clear() {
	_data.clear();
}

void CBaseStar::initialize() {
	_minVal = 9.9999998e10;
	_maxVal = -9.9999998e10;
	_minMax.reset();

	for (uint idx = 0; idx < _data.size(); ++idx) {
		const CBaseStarEntry *entry = getDataPtr(idx);
		_minMax.expand(entry->_position);

		if (entry->_value < _minVal)
			_minVal = entry->_value;
		if (entry->_value > _maxVal)
			_maxVal = entry->_value;
	}

	_range = (_maxVal - _minVal) / 1.0;
}

const CBaseStarEntry *CBaseStar::getDataPtr(int index) const {
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
	entry._position._x = 0;
	entry._position._y = 0;
	entry._position._z = 0;
	for (int idx = 0; idx < 5; ++idx)
		entry._data[idx] = 0;
}

void CBaseStar::draw(CSurfaceArea *surfaceArea, CStarControlSub12 *sub12, CStarControlSub5 *sub5) {
	if (!_data.empty()) {
		switch (sub12->proc27()) {
		case 0:
			switch (surfaceArea->_bpp) {
			case 1:
				draw1(surfaceArea, sub12, sub5);
				break;
			case 2:
				draw2(surfaceArea, sub12, sub5);
				break;
			default:
				break;
			}
			break;

		case 2:
			switch (surfaceArea->_bpp) {
			case 1:
				draw3(surfaceArea, sub12, sub5);
				break;
			case 2:
				draw4(surfaceArea, sub12, sub5);
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
	}
}

void CBaseStar::draw1(CSurfaceArea *surfaceArea, CStarControlSub12 *sub12, CStarControlSub5 *sub5) {
	CStarControlSub6 sub6 = sub12->proc23();
	sub12->proc36(&_value1, &_value2, &_value3, &_value4);

	FPoint centroid = surfaceArea->_centroid - FPoint(0.5, 0.5);
	double v70 = sub12->proc25();
	double minVal = v70 - 9216.0;
	//int width1 = surfaceArea->_width - 1;
	//int height1 = surfaceArea->_height - 1;
	FVector vector;
	double v4;

	for (uint idx = 0; idx < _data.size(); ++idx) {
		CBaseStarEntry &entry = _data[idx];
		vector = entry._position;
		v4 = vector._x * sub6._row1._z + vector._y * sub6._row2._z
			+ vector._z * sub6._row3._z + sub6._vector._z;
		if (v4 <= minVal)
			continue;

		
		// TODO Lots of stuff
		double v17 = 0.0, v98 = 0.0;
		if (v17 >= 1.0e12) {
			// TODO
		} else {
			sub5->proc2(&sub6, &vector, centroid._x, centroid._y, v98, surfaceArea, sub12);
		}
	}
}

void CBaseStar::draw2(CSurfaceArea *surfaceArea, CStarControlSub12 *sub12, CStarControlSub5 *sub5) {
	// TODO
}

void CBaseStar::draw3(CSurfaceArea *surfaceArea, CStarControlSub12 *sub12, CStarControlSub5 *sub5) {
	// TODO
}

void CBaseStar::draw4(CSurfaceArea *surfaceArea, CStarControlSub12 *sub12, CStarControlSub5 *sub5) {
	// TODO
}

int CBaseStar::baseFn1(CSurfaceArea *surfaceArea, CStarControlSub12 *sub12,
		const Common::Point &pt) {
	CStarRef1 ref(this, pt);
	ref.process(surfaceArea, sub12);
	return ref._index;
}

int CBaseStar::baseFn2(CSurfaceArea *surfaceArea, CStarControlSub12 *sub12) {
	CStarRef3 ref(this);
	ref.process(surfaceArea, sub12);
	return ref._index;
}

/*------------------------------------------------------------------------*/

void CStarVector::apply() {
	_owner->addMatrixRow(_vector);
}

} // End of namespace Titanic
