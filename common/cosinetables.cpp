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

// Based on eos' cosine tables

#include "common/cosinetables.h"
#include "common/scummsys.h"

namespace Common {

CosineTable::CosineTable(int nPoints) {
	assert((nPoints >= 16) && (nPoints <= 65536)); // log2 space is in [4,16]
	assert(nPoints % 4 == 0);

	_nPoints = nPoints;
	_radResolution = 2.0 * M_PI / _nPoints;
	_refSize = _nPoints / 4;
	_tableEOS = new float[_nPoints / 2];
	_table = new float[_nPoints];

	for (int i = 0; i < _nPoints; i++)
		_table[i] = cos(i * _radResolution);

	// Table contains cos(2*pi*i/_nPoints) for 0<=i<=_nPoints/4,
	// followed by 3_nPoints/4<=i<_nPoints
	for (int i = 0; i <= _nPoints / 4; i++)
		_tableEOS[i] = cos(i * _radResolution);

	for (int i = 1; i < _nPoints / 4; i++)
		_tableEOS[_nPoints / 2 - i] = _tableEOS[i];	
}

float CosineTable::at(int index) const {
	assert((index >= 0) && (index < _nPoints));
	return _table[index];
}

float CosineTable::atLegacy(int index) const {
	assert((index >= 0) && (index < _nPoints));
	if (index < _refSize)
		// [0,pi/2)
		return _tableEOS[index];
	if ((index > _refSize) && (index < 2 * _refSize))
		// (pi/2,pi)
		return -_tableEOS[2 * _refSize - index];
	if ((index >= 2 * _refSize) && (index < 3 * _refSize))
		// [pi,3/2pi)
		return -_tableEOS[index - 2 * _refSize];
	if ((index > 3 * _refSize) && (index < _nPoints))
		// (3/2pi,2pi)
		return _tableEOS[_nPoints - index];
	return 0.0f; // cos(pi/2) and cos(3pi/2) = 0
}

CosineTable::~CosineTable() {
	delete[] _tableEOS;
	delete[] _table;
}

} // End of namespace Common
