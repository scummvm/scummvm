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

#include "agds/systemVariable.h"
#include "common/debug.h"
#include "common/textconsole.h"

namespace AGDS {

const Common::String &IntegerSystemVariable::getString() const {
	error("invalid type");
}

int IntegerSystemVariable::getInteger() const {
	return _value;
}

void IntegerSystemVariable::setString(const Common::String &value) {
	error("invalid type");
}

void IntegerSystemVariable::setInteger(int value) {
	_value = value;
}

const Common::String &StringSystemVariable::getString() const {
	return _value;
}

int StringSystemVariable::getInteger() const {
	error("invalid type");
}

void StringSystemVariable::setString(const Common::String &value) {
	_value = value;
}

void StringSystemVariable::setInteger(int value) {
	error("invalid type");
}

} // namespace AGDS
