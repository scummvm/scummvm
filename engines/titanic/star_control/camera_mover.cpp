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

#include "titanic/star_control/camera_mover.h"
#include "common/textconsole.h"

namespace Titanic {

CCameraMover::CCameraMover(const CNavigationInfo *src) {
	_lockCounter = 0;
	_starVector = nullptr;

	if (src) {
		copyFrom(src);
	} else {
		_size = 0.0;
		_field4 = 0.0;
		_field8 = 20.0;
		_fieldC = 0.0;
		_field10 = 50000.0;
		_field14 = 1.0;
		_field18 = 1.0;
		_field1C = 0.0;
	}
}

CCameraMover::~CCameraMover() {
	clear();
}

void CCameraMover::copyFrom(const CNavigationInfo *src) {
	*((CNavigationInfo *)this) = *src;
}

void CCameraMover::copyTo(CNavigationInfo *dest) {
	*dest = *((CNavigationInfo *)this);
}

void CCameraMover::proc4() {
	if (!isLocked() && _size < _field10) {
		_field4 += _size;
		if (_field8 == _field4)
			_size -= _field4;
		else
			_size += _field4;
	}
}

void CCameraMover::proc5() {
	if (!isLocked()) {
		_field4 -= _field8;
		if (_field4 == _size)
			_size += _field4;
		else
			_size -= _field4;

		if (_field4 < 0.0)
			_field4 = 0.0;
	}
}

void CCameraMover::proc6() {
	if (!isLocked())
		_size = _field10;
}

void CCameraMover::proc7() {
	if (!isLocked()) {
		_size = 0.0;
		_field4 = 0.0;
	}
}

void CCameraMover::updatePosition(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) {
	if (_size > 0.0) {
		pos._x += orientation._row3._x * _size;
		pos._y += orientation._row3._y * _size;
		pos._z += orientation._row3._z * _size;

		errorCode.set();
	}
}

void CCameraMover::setVector(CStarVector *sv) {
	clear();
	_starVector = sv;
}

void CCameraMover::clear() {
	if (_starVector) {
		delete _starVector;
		_starVector = nullptr;
	}
}

void CCameraMover::load(SimpleFile *file, int val) {
	if (!val) {
		_size = file->readFloat();
		_field4 = file->readFloat();
		_field8 = file->readFloat();
		_fieldC = file->readFloat();
		_field10 = file->readFloat();
		_field14 = file->readFloat();
		_field18 = file->readFloat();
		_field1C = file->readFloat();
	}
}

void CCameraMover::save(SimpleFile *file, int indent) {
	file->writeFloatLine(_size, indent);
	file->writeFloatLine(_field4, indent);
	file->writeFloatLine(_field8, indent);
	file->writeFloatLine(_fieldC, indent);
	file->writeFloatLine(_field10, indent);
	file->writeFloatLine(_field14, indent);
	file->writeFloatLine(_field18, indent);
	file->writeFloatLine(_field1C, indent);
}

void CCameraMover::incLockCount() {
	++_lockCounter;
}

void CCameraMover::decLockCount() {
	if (_lockCounter > 0)
		--_lockCounter;
}

} // End of namespace Titanic
