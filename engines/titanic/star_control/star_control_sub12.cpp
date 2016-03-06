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

#include "titanic/star_control/star_control_sub12.h"

namespace Titanic {

CStarControlSub12::CStarControlSub12(void *val1, void *val2) : 
		_field4(-1), _field2C(0), _field108(0),
		_sub13(val1) {
}

void CStarControlSub12::load(SimpleFile *file, int param) {
	_sub13.load(file, param);
}

void CStarControlSub12::save(SimpleFile *file, int indent) const {
	_sub13.save(file, indent);
}

} // End of namespace Titanic
