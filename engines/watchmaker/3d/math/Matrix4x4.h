/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef WATCHMAKER_MATRIX4X4_H
#define WATCHMAKER_MATRIX4X4_H

#include "common/scummsys.h"
#include "common/textconsole.h"

namespace Watchmaker {

struct Matrix4x4 {
	float data[16] = {};

	void setIdentity() {
		setValue(1, 1, 1.0f);
		setValue(2, 2, 1.0f);
		setValue(3, 3, 1.0f);
		setValue(4, 4, 1.0f);
	}

	void setValue(int row, int col, float value) {
		data[(col - 1) * 4 + (row - 1)] = value;
	}
	float getValue(int row, int col) const {
		return data[(col - 1) * 4 + (row - 1)];
	}
	void print() const {
		for (int row = 1; row <= 4; row++) {
			for (int col = 1; col <= 4; col++) {
				warning("%f ", getValue(row, col));
			}
			warning("");
		}
	}
	bool operator==(const Matrix4x4 &rhs) const {
		for (int row = 1; row <= 4; row++) {
			for (int col = 1; col <= 4; col++) {
				if (getValue(row, col) != rhs.getValue(row, col)) {
					return false;
				}
			}
		}
		return true;
	}
};


} // End of namespace Watchmaker

#endif // WATCHMAKER_MATRIX4X4_H
