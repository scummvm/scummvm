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

namespace Titanic {

CBaseStar::CBaseStar() : _fieldC(1), _minVal(0.0), _maxVal(1.0), _range(0.0) {
}

void CBaseStar::proc2(int v1, int v2, int v3) {
	error("TODO");
}

void CBaseStar::clear() {
	if (!_data.empty()) {
		if (_fieldC)
			_data.clear();
	}
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

} // End of namespace Titanic
