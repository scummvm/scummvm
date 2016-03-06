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

#include "titanic/screen_manager.h"
#include "titanic/star_control/star_control_sub11.h"

namespace Titanic {

CStarControlSub11::CStarControlSub11() : 
		_sub12(nullptr, nullptr), _sub13(nullptr),
		_field4(0), _field8(0), _field20C(0), _field210(0),
		_field214(0), _field218(0), _field21C(0) {
	_sub12.proc3();
}

void CStarControlSub11::load(SimpleFile *file, int param) {
	if (!param) {
		_sub12.load(file, param);

		int val = file->readNumber();
		if (val)
			_sub13.load(file, 0);

		_field218 = file->readNumber();
		_field21C = file->readNumber();
	}
}

void CStarControlSub11::save(SimpleFile *file, int indent) const {
}

} // End of namespace Titanic
