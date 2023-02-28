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

#include "tetraedge/te/te_variant.h"

namespace Tetraedge {

TeVariant::TeVariant() : _type(TypeNone), _data(0) {
}

TeVariant::TeVariant(bool val) : _type(TypeBoolean), _data(val) {
}

TeVariant::TeVariant(double val) : _type(TypeFloat64) {
	*(double *)(&_data) = val;
}

TeVariant::TeVariant(const Common::String &val) : _type(TypeString), _data(0), _strVal(val) {
}

TeVariant::TeVariant(const char *val) : _type(TypeString), _data(0), _strVal(val) {
}

TeVariant::TeVariant(const TeVariant &other) : _type(other._type), _data(other._data), _strVal(other._strVal) {
}

bool TeVariant::toBoolean(bool *success) const {
	if (_type == TypeBoolean) {
		if (success)
			*success = true;
		return _data != 0;
	} else {
		if (success)
			*success = false;
		return false;
	}
}

float TeVariant::toFloat32(bool *success) const {
	if (_type == TypeFloat32) {
		if (success)
			*success = true;
		return *(const float *)(&_data);
	} else {
		if (success)
			*success = false;
		return false;
	}

}

double TeVariant::toFloat64(bool *success) const {
	if (_type == TypeFloat64) {
		if (success)
			*success = true;
		return *(const double *)(&_data);
	} else {
		if (success)
			*success = false;
		return false;
	}

}

int32 TeVariant::toSigned32(bool *success) const {
	if (_type == TypeInt32) {
		if (success)
			*success = true;
		return *(const int32 *)(&_data);
	} else {
		if (success)
			*success = false;
		return false;
	}

}

int64 TeVariant::toSigned64(bool *success) const {
	if (_type == TypeInt64) {
		if (success)
			*success = true;
		return *(const int64 *)(&_data);
	} else {
		if (success)
			*success = false;
		return false;
	}

}

Common::String TeVariant::toString(bool *success) const {
	if (_type == TypeString) {
		if (success)
			*success = true;
		return _strVal;
	} else {
		if (success)
			*success = false;
		return "";
	}

}

uint32 TeVariant::toUnsigned32(bool *success) const {
	if (_type == TypeUInt32) {
		if (success)
			*success = true;
		return *(const uint32 *)(&_data);
	} else {
		if (success)
			*success = false;
		return false;
	}

}

uint64 TeVariant::toUnsigned64(bool *success) const {
	if (_type == TypeUInt64) {
		if (success)
			*success = true;
		return *(const uint64 *)(&_data);
	} else {
		if (success)
			*success = false;
		return false;
	}

}

Common::String TeVariant::dumpStr() const {
	switch (_type) {
	case TypeInt32:
		return Common::String::format("%d", (int)toSigned32());
	case TypeUInt32:
		return Common::String::format("%u", (unsigned int)toUnsigned32());
	case TypeInt64:
		return Common::String::format("%ld", (long)toSigned64());
	case TypeUInt64:
		return Common::String::format("%lu", (unsigned long)toUnsigned64());
	case TypeFloat32:
		return Common::String::format("%.04f", toFloat32());
	case TypeFloat64:
		return Common::String::format("%.04f", toFloat64());
	case TypeBoolean:
		return Common::String::format("%s", toBoolean() ? "true" : "false");
	case TypeString:
		return toString();
	default:
	case TypeNone:
		return "(none)";
	}
}


} // end namespace Tetraedge
