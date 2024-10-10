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

//=============================================================================
//
// Managed script object wrapping std::map<String, String> and
// unordered_map<String, String>.
//
// TODO: support wrapping non-owned Dictionary, passed by the reference, -
// that would let expose internal engine's dicts using same interface.
// TODO: maybe optimize key lookup operations further by not creating a String
// object from const char*. It seems, C++14 standard allows to use convertible
// types as keys; need to research what performance impact that would make.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_DYNOBJ_SCRIPTDICT_H
#define AGS_ENGINE_AC_DYNOBJ_SCRIPTDICT_H

#include "common/std/map.h"
#include "common/std/map.h"
#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string.h"
#include "ags/shared/util/string_types.h"

namespace AGS3 {

using namespace AGS::Shared;

class ScriptDictBase : public AGSCCDynamicObject {
public:
	int Dispose(void *address, bool force) override;
	const char *GetType() override;
	void Unserialize(int index, AGS::Shared::Stream *in, size_t data_sz) override;

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
protected:
	// Calculate and return required space for serialization, in bytes
	size_t CalcSerializeSize(const void *address) override;
	// Write object data into the provided stream
	void Serialize(const void *address, AGS::Shared::Stream *out) override;

private:
	virtual size_t CalcContainerSize() = 0;
	virtual void SerializeContainer(AGS::Shared::Stream *out) = 0;
	virtual void UnserializeContainer(AGS::Shared::Stream *in) = 0;
};

template <typename TDict, bool is_sorted, bool is_casesensitive>
class ScriptDictImpl final : public ScriptDictBase {
public:
	typedef typename TDict::const_iterator ConstIterator;

	ScriptDictImpl() {}

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
#ifdef AGS_PLATFORM_SCUMMVM
		return _dic.find(String::Wrapper(key)) != _dic.end();
#else
		return _dic.count(String::Wrapper(key)) != 0;
#endif
	}
	const char *Get(const char *key) override {
		auto it = _dic.find(String::Wrapper(key));
		if (it == _dic.end()) return nullptr;
		return it->_value.GetCStr();
	}
	bool Remove(const char *key) override {
		auto it = _dic.find(String::Wrapper(key));
		if (it == _dic.end()) return false;
		DeleteItem(it);
		_dic.erase(it);
		return true;
	}
	bool Set(const char *key, const char *value) override {
		if (!key)
			return false;
		if (!value) {
			// Remove keys with null value
			Remove(key);
			return true;
		}

		return TryAddItem(String(key), String(value));
	}
	int GetItemCount() override {
		return _dic.size();
	}
	void GetKeys(std::vector<const char *> &buf) const override {
		for (auto it = _dic.begin(); it != _dic.end(); ++it)
			buf.push_back(it->_key.GetCStr());
	}
	void GetValues(std::vector<const char *> &buf) const override {
		for (auto it = _dic.begin(); it != _dic.end(); ++it)
			buf.push_back(it->_value.GetCStr());
	}

private:
	bool TryAddItem(const String &key, const String &value) {
		_dic[key] = value;
		return true;
	}
	void DeleteItem(ConstIterator /*it*/) { /* do nothing */ }

	size_t CalcContainerSize() override {
		// 2 class properties + item count
		size_t total_sz = sizeof(int32_t) * 3;
		// (int32 + string buffer) per item
		for (auto it = _dic.begin(); it != _dic.end(); ++it) {
			total_sz += sizeof(int32_t) + it->_key.GetLength();
			total_sz += sizeof(int32_t) + it->_value.GetLength();
		}
		return total_sz;
	}

	void SerializeContainer(AGS::Shared::Stream *out) override
    {
        out->WriteInt32((int)_dic.size());
        for (auto it = _dic.begin(); it != _dic.end(); ++it)
        {
            out->WriteInt32((int)it->_key.GetLength());
            out->Write(it->_key.GetCStr(), it->_key.GetLength());
            out->WriteInt32((int)it->_value.GetLength());
            out->Write(it->_value.GetCStr(), it->_value.GetLength());
        }
    }

	void UnserializeContainer(AGS::Shared::Stream *in) override {
		size_t item_count = in->ReadInt32();
		for (size_t i = 0; i < item_count; ++i) {
			size_t key_len = in->ReadInt32();
			String key = String::FromStreamCount(in, key_len);
			size_t value_len = in->ReadInt32();
			if (value_len != (size_t)-1) // do not restore keys with null value (old format)
			{
				String value = String::FromStreamCount(in, value_len);
				TryAddItem(key, value);
			}
		}
	}

	TDict _dic;
};

typedef ScriptDictImpl< std::map<String, String>, true, true > ScriptDict;
typedef ScriptDictImpl< std::map<String, String, IgnoreCase_LessThan>, true, false > ScriptDictCI;
typedef ScriptDictImpl< std::unordered_map<String, String>, false, true > ScriptHashDict;
typedef ScriptDictImpl< std::unordered_map<String, String, IgnoreCase_Hash, IgnoreCase_EqualTo>, false, false > ScriptHashDictCI;

} // namespace AGS3

#endif
