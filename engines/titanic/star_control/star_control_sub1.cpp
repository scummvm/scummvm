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

#include "titanic/star_control/star_control_sub1.h"

namespace Titanic {

CStarControlSub1::CStarControlSub1() : _val1(0), _val2(0), _val3(0), _val4(1),
	_val5(0), _val6(false) {
}

void CStarControlSub1::load(SimpleFile *file, int param) {
	if (!param) {
		_sub7.load(file);
		_sub8.load(file);
		_val1 = file->readNumber();
		_val2 = file->readNumber();
		_val3 = file->readNumber();
		_val4 = file->readNumber();
		_val6 = file->readNumber();
	}
}

void CStarControlSub1::save(SimpleFile *file, int indent) {
	_sub7.save(file, indent);
	_sub8.save(file, indent);
	file->writeNumberLine(_val1, indent);
	file->writeNumberLine(_val2, indent);
	file->writeNumberLine(_val3, indent);
	file->writeNumberLine(_val4, indent);
	file->writeNumberLine(_val6, indent);
}

bool CStarControlSub1::initDocument() {
	bool valid = setup() && _points1.initialize();
	if (valid)
		valid = _sub5.setup();
	if (valid)
		valid = _points1.initialize();
	if (valid)
		valid = _points2.initialize();

	return valid;
}

} // End of namespace Titanic
