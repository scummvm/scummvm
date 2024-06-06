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

#pragma once

#include "qdengine/xlibs/util/xtl/StaticMap.h"
#include "qdengine/xlibs/util/xtl/StaticString.h"
#include "ComboStrings.h"


namespace QDEngine {
using namespace std;
/////////////////////////////////////////////////
/// Map: enum <-> name, nameAlt
class EnumDescriptor {
public:
	EnumDescriptor(const char *typeName, const char *typeNameAlt) : typeName_(typeName), typeNameAlt_(typeNameAlt) {
		ignoreErrors_ = false;
		descriptors()[typeName] = this;
	}

	const char *name(int key) const;
	const char *nameAlt(int key) const;
	int keyByName(const char *name) const;
	int keyByNameAlt(const char *nameAlt) const;

	bool nameExists(const char *name) const;
	bool nameAltExists(const char *name) const;

	string nameCombination(int bitVector, const char *separator = " | ") const;
	string nameAltCombination(int bitVector, const char *separator = "|") const;

	typedef vector<const char *> NameCombination;
	void nameCombinationStrings(int bitVector, NameCombination &strings) const;

	const char *comboList() const {
		return comboList_.c_str();
	}
	const char *comboListAlt() const {
		return comboListAlt_.c_str();
	}

	const ComboStrings &comboStrings() const {
		return comboStrings_;
	}
	const ComboStrings &comboStringsAlt() const {
		return comboStringsAlt_;
	}

	const char *typeName() const {
		return typeName_;
	}
	const char *typeNameAlt() const {
		return typeNameAlt_;
	}
	int defaultValue() const;

	void ignoreErrors(bool v) {
		ignoreErrors_ = v;
	}

	int size() const {
		return int(nameToKey_.size());
	}

	static const EnumDescriptor *descriptorByTypeName(const char *enumTypeName);

protected:
	void add(int key, const char *name, const char *nameAlt);
	void clear();

private:
	typedef StaticMap<StaticString, EnumDescriptor *> NameToEnumDescriptor;
	static NameToEnumDescriptor &descriptors();
	class Key {
	public:
		Key(int value);
		bool operator<(const Key &rhs) const;
		operator int() const {
			return value_;
		}

	private:
		int value_;
		unsigned int bitsNumber_;
	};

	typedef StaticMap<Key, StaticString> KeyToName;

	KeyToName keyToName_;
	KeyToName keyToNameAlt_;

	typedef StaticMap<StaticString, int> NameToKey;

	NameToKey nameToKey_;
	NameToKey nameAltToKey_;

	string comboList_;
	string comboListAlt_;

	ComboStrings comboStrings_;
	ComboStrings comboStringsAlt_;

	const char *typeName_;
	const char *typeNameAlt_;

	bool ignoreErrors_;
};

/////////////////////////////////////////////////
//		Регистрация enums
/////////////////////////////////////////////////
template<class Enum>
const EnumDescriptor &getEnumDescriptor(const Enum &key);

#define BEGIN_ENUM_DESCRIPTOR(enumType, enumName)   \
    struct Enum##enumType : EnumDescriptor { Enum##enumType(); }; \
    Enum##enumType::Enum##enumType() : EnumDescriptor(typeid(enumType).name(), enumName) {

#define REGISTER_ENUM(enumKey, enumNameAlt) \
    add(enumKey, #enumKey, enumNameAlt);

#define ENUM_DESCRIPTOR_IGNORE_ERRORS   ignoreErrors(true);

#define END_ENUM_DESCRIPTOR(enumType)   \
    }  \
    const EnumDescriptor& getEnumDescriptor(const enumType& key){   \
        static Enum##enumType descriptor;   \
        return descriptor;  \
    }

// Для enums, закрытых классами
#define BEGIN_ENUM_DESCRIPTOR_ENCLOSED(nameSpace, enumType, enumName)   \
    struct Enum##nameSpace##enumType : EnumDescriptor { Enum##nameSpace##enumType(); }; \
    Enum##nameSpace##enumType::Enum##nameSpace##enumType() : EnumDescriptor(typeid(nameSpace::enumType).name(), enumName) {

#define REGISTER_ENUM_ENCLOSED(nameSpace, enumKey, enumNameAlt) \
    add(nameSpace::enumKey, #enumKey, enumNameAlt);

#define END_ENUM_DESCRIPTOR_ENCLOSED(nameSpace, enumType)   \
    }  \
    const EnumDescriptor& getEnumDescriptor(const nameSpace::enumType& key){    \
        static Enum##nameSpace##enumType descriptor;    \
        return descriptor;  \
    }


/////////////////////////////////////////////////
//	Вспомогательные функции для отображения
/////////////////////////////////////////////////
template<class Enum>
const char *getEnumName(const Enum &key) {
	return getEnumDescriptor(Enum()).name(key);
}

template<class Enum>
const char *getEnumNameAlt(const Enum &key) {
	return getEnumDescriptor(Enum()).nameAlt(key);
}

} // namespace QDEngine
