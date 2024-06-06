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

#ifndef __SERIALIZATION_TYPES_H_INCLUDED__
#define __SERIALIZATION_TYPES_H_INCLUDED__

#include <string>
#include "Handle.h"
#include "qdengine/xlibs/util/serialization/Factory.h"

namespace QDEngine {

class Archive;

/// Строка с редактируемыми значениями из списка
class ComboListString {
public:
	ComboListString() {}
	ComboListString(const char *comboList, const char *value = "") : comboList_(comboList), value_(value) {}

	ComboListString &operator=(const char *value) {
		value_ = value ? value : "";
		return *this;
	}
	ComboListString &operator=(const std::string &value) {
		value_ = value;
		return *this;
	}

	operator const char *() const {
		return value_.c_str();
	}
	const char *comboList() const {
		return comboList_.c_str();
	}
	void setComboList(const char *comboList) {
		comboList_ = comboList;
	}

	std::string &value() {
		return value_;
	}
	const std::string &value() const {
		return value_;
	}

protected:
	std::string value_;
	std::string comboList_;
};

/// Вектор энумерованных бит
template<class Enum>
class BitVector {
	typedef int Value;
public:
	BitVector(Value value = 0) : value_(value) {}

	operator Value() const {
		return value_;
	}

	BitVector &operator |= (Enum value) {
		value_ |= value;
		return *this;
	}
	BitVector &operator |= (Value value) {
		value_ |= value;
		return *this;
	}
	BitVector &operator &= (Value value) {
		value_ &= value;
		return *this;
	}

private:
	Value value_;
};

/// Обертка для сериализации полиморфных указателей по значению enum'а
template<class Enum, class Type, Enum zeroValue, class TypeFactory = Factory<Enum, Type> >
class EnumToClassSerializer {
public:
	EnumToClassSerializer(Enum key = zeroValue) : type_(0), key_(zeroValue) {
		setKey(key);
	}
	EnumToClassSerializer(Enum key, Type *setType) : type_(setType), key_(zeroValue) {
		xassert(!strcmp(TypeFactory::instance().typeName(key_), typeid(type_).name()));
	}

	void setKey(Enum key) {
		if (key_ != key) {
			type_ = key != zeroValue ? TypeFactory::instance().create(key, true) : 0;
			key_ = key;
		}
	}

	Enum key() const {
		return key_;
	}
	Type *type() const {
		return type_;
	}

	operator Type *() const {
		return type_;
	}
	Type *operator->() const {
		return type_;
	}
	Type &operator*() const {
		return *type_;
	}

private:
	Enum key_;
	ShareHandle<Type> type_;
	friend class Archive;
};

} // namespace QDEngine

#endif // __SERIALIZATION_TYPES_H_INCLUDED__
