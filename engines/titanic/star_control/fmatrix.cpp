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

#include "titanic/star_control/fmatrix.h"

namespace Titanic {

FMatrix::FMatrix() :
	_row1(1.0, 0.0, 0.0), _row2(0.0, 1.0, 0.0), _row3(0.0, 0.0, 1.0) {
}

FMatrix::FMatrix(DMatrix *src) {
	copyFrom(src);
}

void FMatrix::copyFrom(const DMatrix *src) {
	// TODO
}

void FMatrix::load(SimpleFile *file, int param) {
	_row1._x = file->readFloat();
	_row1._y = file->readFloat();
	_row1._z = file->readFloat();
	_row2._x = file->readFloat();
	_row2._y = file->readFloat();
	_row2._z = file->readFloat();
	_row3._x = file->readFloat();
	_row3._y = file->readFloat();
	_row3._z = file->readFloat();
}

void FMatrix::save(SimpleFile *file, int indent) {
	file->writeFloatLine(_row1._x, indent);
	file->writeFloatLine(_row1._y, indent);
	file->writeFloatLine(_row1._z, indent);
	file->writeFloatLine(_row2._x, indent);
	file->writeFloatLine(_row2._y, indent);
	file->writeFloatLine(_row2._z, indent);
	file->writeFloatLine(_row3._x, indent);
	file->writeFloatLine(_row3._y, indent);
	file->writeFloatLine(_row3._z, indent);
}

void FMatrix::clear() {
	_row1 = FVector(1.0, 0.0, 0.0);
	_row2 = FVector(0.0, 1.0, 0.0);
	_row3 = FVector(0.0, 0.0, 1.0);
}

void FMatrix::set(FVector *row1, FVector *row2, FVector *row3) {
	_row1 = *row1;
	_row2 = *row2;
	_row3 = *row3;
}

} // End of namespace Titanic
