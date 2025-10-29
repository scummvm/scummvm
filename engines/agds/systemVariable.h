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

#ifndef AGDS_SYSTEM_VARIABLE_H
#define AGDS_SYSTEM_VARIABLE_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Common {
	class ReadStream;
	class WriteStream;
}

namespace AGDS {

class SystemVariable {
public:
	virtual ~SystemVariable() {}

	virtual const Common::String &getString() const = 0;
	virtual int getInteger() const = 0;
	virtual void setString(const Common::String &value) = 0;
	virtual void setInteger(int value) = 0;
	virtual void reset() = 0;
	virtual void read(Common::ReadStream &stream) = 0;
	virtual void write(Common::WriteStream &stream) const = 0;
};

class IntegerSystemVariable : public SystemVariable {
	int _value;
	int _defaultValue;

public:
	IntegerSystemVariable(int defaultValue = 0):
		_value(defaultValue), _defaultValue(defaultValue) {
	}

	virtual const Common::String &getString() const;
	virtual int getInteger() const;
	virtual void setString(const Common::String &value);
	virtual void setInteger(int value);
	virtual void reset() {
		_value = _defaultValue;
	}
	virtual void read(Common::ReadStream &stream);
	virtual void write(Common::WriteStream &stream) const;
};

class StringSystemVariable : public SystemVariable {
	Common::String	_value;
	Common::String	_defaultValue;

public:
	StringSystemVariable(const Common::String &defaultValue = Common::String()):
		_value(defaultValue), _defaultValue(defaultValue) {
	}
	virtual const Common::String &getString() const;
	virtual int getInteger() const;
	virtual void setString(const Common::String &value);
	virtual void setInteger(int value);
	virtual void reset() {
		_value = _defaultValue;
	}
	virtual void read(Common::ReadStream &stream);
	virtual void write(Common::WriteStream &stream) const;
};

} // End of namespace AGDS

#endif /* AGDS_SYSTEM_VARIABLE_H */
