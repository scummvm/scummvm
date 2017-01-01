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

#include "titanic/star_control/star_control_sub20.h"
#include "common/textconsole.h"

namespace Titanic {

CStarControlSub20::CStarControlSub20(const CStar20Data *src) {
	_lockCounter = 0;
	_dataP = nullptr;

	if (src) {
		copyFrom(src);
	} else {
		_field0 = 0.0;
		_field4 = 0.0;
		_field8 = 20.0;
		_fieldC = 0.0;
		_field10 = 50000.0;
		_field14 = 1.0;
		_field18 = 1.0;
		_field1C = 0.0;
	}
}

CStarControlSub20::~CStarControlSub20() {
	clear();
}

void CStarControlSub20::copyFrom(const CStar20Data *src) {
	*((CStar20Data *)this) = *src;
}

void CStarControlSub20::copyTo(CStar20Data *dest) {
	*dest = *((CStar20Data *)this);
}

void CStarControlSub20::proc4() {
	if (!isLocked() && _field0 < _field10) {
		_field4 += _field0;
		if (_field8 == _field4)
			_field0 -= _field4;
		else
			_field0 += _field4;
	}
}

void CStarControlSub20::proc5() {
	if (!isLocked()) {
		_field4 -= _field8;
		if (_field4 == _field0)
			_field0 += _field4;
		else
			_field0 -= _field4;

		if (_field4 < 0.0)
			_field4 = 0.0;
	}
}

void CStarControlSub20::proc6() {
	if (!isLocked())
		_field0 = _field10;
}

void CStarControlSub20::proc7() {
	if (!isLocked()) {
		_field0 = 0.0;
		_field4 = 0.0;
	}
}

void CStarControlSub20::proc11(CErrorCode &errorCode, FVector &v, const FMatrix &m) {
	if (_field0 > 0.0) {
		warning("TODO: CStarControlSub20::proc11");
	}
}

void CStarControlSub20::setData(void *data) {
	clear();
	_dataP = (byte *)data;
}

void CStarControlSub20::clear() {
	if (_dataP) {
		delete _dataP;
		_dataP = nullptr;
	}
}

void CStarControlSub20::load(SimpleFile *file, int val) {
	if (!val) {
		_field0 = file->readFloat();
		_field4 = file->readFloat();
		_field8 = file->readFloat();
		_fieldC = file->readFloat();
		_field10 = file->readFloat();
		_field14 = file->readFloat();
		_field18 = file->readFloat();
		_field1C = file->readFloat();
	}
}

void CStarControlSub20::save(SimpleFile *file, int indent) {
	file->writeFloatLine(_field0, indent);
	file->writeFloatLine(_field4, indent);
	file->writeFloatLine(_field8, indent);
	file->writeFloatLine(_fieldC, indent);
	file->writeFloatLine(_field10, indent);
	file->writeFloatLine(_field14, indent);
	file->writeFloatLine(_field18, indent);
	file->writeFloatLine(_field1C, indent);
}

void CStarControlSub20::incLockCount() {
	++_lockCounter;
}

void CStarControlSub20::decLockCount() {
	if (_lockCounter > 0)
		--_lockCounter;
}

} // End of namespace Titanic
