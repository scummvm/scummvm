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

//=============================================================================
//
// Managed script object wrapping std::map<String, String> and
// unordered_map<String, String>.
//
// TODO: support wrapping non-owned Dictionary, passed by the reference, -
// that would let expose internal engine's dicts using same interface.
// TODO: maybe optimize key lookup operations further by not creating a String
// object from const char*. It seems, C++14 standard allows to use convertible
// types as keys; need to research what perfomance impact that would make.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_DYNOBJ_SCRIPTDICT_H
#define AGS_ENGINE_AC_DYNOBJ_SCRIPTDICT_H

#include <map>
#include <unordered_map>
#include <string.h>
#include "ac/dynobj/cc_agsdynamicobject.h"
#include "util/string.h"
#include "util/string_types.h"

namespace AGS3 {

using namespace AGS::Common;

class ScriptDictBase : public AGSCCDynamicObject {
public:
	int Dispose(const char *address, bool force) override;
	const char *GetType() override;
	int Serialize(const char *address, char *buffer, int bufsize) override;
	void Unserialize(int index, const char *serializedData, int dataSize) override;

	virtual bool IsCaseSensitive() const = 0;
	virtual bool IsSorted() const = 0;

	virtual void Clear() = 0;
	virtual bool Contains(const char *key) = 0;
	virtual const char *Get(const char *key) = 0;
	virtual bool Remove(const char *key) = 0;
	virtual bool Set(const char *key, const char *value) = 0;
	virtual int GetItemCount() = 0;
	virtual void GetKeys(std::vector<const char *> &buf) const = 0;
	virtual void GetValues(std::vector<const char *> &buf) const = 0;

private:
	virtual size_t CalcSerializeSize() = 0;
	virtual void SerializeContainer() = 0;
	virtual void UnserializeContainer(const char *serializedData) = 0;
};

template <typename TDict, bool is_sorted, bool is_casesensitive>
class ScriptDictImpl final : public ScriptDictBase {
public:
	typedef typename TDict::const_iterator ConstIterator;

	ScriptDictImpl() = default;

	bool IsCaseSensitive() const override {
		return is_casesensitive;
	}
	bool IsSorted() const override {
		return is_sorted;
	}

	void Clear() override {
		for (auto it = _dic.begin(); it != _dic.end(); ++it)
			DeleteItem(it);
		_dic.clear();
	}
	bool Contains(const char *key) override {
		return _dic.count(String::Wrapper(key)) != 0;
	}
	const char *Get(const char *key) override {
		auto it = _dic.find(String::Wrapper(key));
		if (it == _dic.end()) return nullptr;
		return it->second.GetNullableCStr();
	}
	bool Remove(const char *key) override {
		auto it = _dic.find(String::Wrapper(key));
		if (it == _dic.end()) return false;
		DeleteItem(it);
		_dic.erase(it);
		return true;
	}
	bool Set(const char *key, const char *value) override {
		if (!key) return false;
		size_t key_len = strlen(key);
		size_t value_len = value ? strlen(value) : 0;
		return TryAddItem(key, key_len, value, value_len);
	}
	int GetItemCount() override {
		return _dic.size();
	}
	void GetKeys(std::vector<const char *> &buf) const override {
		for (auto it = _dic.begin(); it != _dic.end(); ++it)
			buf.push_back(it->first.GetCStr()); // keys cannot be null
	}
	void GetValues(std::vector<const char *> &buf) const override {
		for (auto it = _dic.begin(); it != _dic.end(); ++it)
			buf.push_back(it->second.GetNullableCStr()); // values may be null
	}

private:
	bool TryAddItem(const char *key, size_t key_len, const char *value, size_t value_len) {
		String elem_key(key, key_len);
		String elem_value;
		if (value)
			elem_value.SetString(value, value_len);
		_dic[elem_key] = elem_value;
		return true;
	}
	void DeleteItem(ConstIterator it) {
		/* do nothing */
	}

	size_t CalcSerializeSize() override {
		size_t total_sz = sizeof(int32_t);
		for (auto it = _dic.begin(); it != _dic.end(); ++it) {
			total_sz += sizeof(int32_t) + it->first.GetLength();
			total_sz += sizeof(int32_t) + it->second.GetLength();
		}
		return total_sz;
	}

	void SerializeContainer() override {
		SerializeInt((int)_dic.size());
		for (auto it = _dic.begin(); it != _dic.end(); ++it) {
			SerializeInt((int)it->first.GetLength());
			memcpy(&serbuffer[bytesSoFar], it->first.GetCStr(), it->first.GetLength());
			bytesSoFar += it->first.GetLength();
			if (it->second.GetNullableCStr()) { // values may be null
				SerializeInt((int)it->second.GetLength());
				memcpy(&serbuffer[bytesSoFar], it->second.GetCStr(), it->second.GetLength());
				bytesSoFar += it->second.GetLength();
			} else {
				SerializeInt(-1);
			}
		}
	}

	void UnserializeContainer(const char *serializedData) override {
		size_t item_count = (size_t)UnserializeInt();
		for (size_t i = 0; i < item_count; ++i) {
			size_t key_len = UnserializeInt();
			int key_pos = bytesSoFar;
			bytesSoFar += key_len;
			size_t value_len = UnserializeInt();
			if (value_len == (size_t)-1) {
				TryAddItem(&serializedData[key_pos], key_len, nullptr, 0);
			} else {
				int value_pos = bytesSoFar;
				bytesSoFar += value_len;
				TryAddItem(&serializedData[key_pos], key_len, &serializedData[value_pos], value_len);
			}
		}
	}

	TDict _dic;
};

typedef ScriptDictImpl< std::map<String, String>, true, true > ScriptDict;
typedef ScriptDictImpl< std::map<String, String, StrLessNoCase>, true, false > ScriptDictCI;
typedef ScriptDictImpl< std::unordered_map<String, String>, false, true > ScriptHashDict;
typedef ScriptDictImpl< std::unordered_map<String, String, HashStrNoCase, StrEqNoCase>, false, false > ScriptHashDictCI;

} // namespace AGS3

#endif
