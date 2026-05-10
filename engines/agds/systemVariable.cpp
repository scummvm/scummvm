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

#include "agds/systemVariable.h"
#include "common/array.h"
#include "common/stream.h"
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

void IntegerSystemVariable::read(Common::ReadStream &stream) {
	_value = stream.readSint32LE();
}

void IntegerSystemVariable::write(Common::WriteStream &stream) const {
	stream.writeSint32LE(_value);
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

void StringSystemVariable::read(Common::ReadStream &stream) {
	byte len = stream.readByte();
	if (len == 0)
		error("invalid string var length");
	Common::Array<char> str(len);
	stream.read(str.data(), str.size());
	_value = Common::String(str.data(), len - 1);
}

void StringSystemVariable::write(Common::WriteStream &stream) const {
	uint len = _value.size() + 1;
	if (len > 255)
		error("variable too long, %u", len);
	stream.writeByte(len);
	stream.write(_value.c_str(), len);
}

} // namespace AGDS
