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
// Managed script object wrapping std::set<String> and unordered_set<String>.
//
// TODO: support wrapping non-owned Set, passed by the reference, -
// that would let expose internal engine's sets using same interface.
// TODO: maybe optimize key lookup operations further by not creating a String
// object from const char*. It seems, C++14 standard allows to use convertible
// types as keys; need to research what perfomance impact that would make.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_DYNOBJ_SCRIPTSET_H
#define AGS_ENGINE_AC_DYNOBJ_SCRIPTSET_H

#include "ags/lib/std/set.h"
#include "ags/lib/std/unordered_set.h"
#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"
#include "ags/shared/util/string.h"
#include "ags/shared/util/string_types.h"

namespace AGS3 {

using namespace AGS::Shared;

class ScriptSetBase : public AGSCCDynamicObject {
public:
	int Dispose(const char *address, bool force) override;
	const char *GetType() override;
	int Serialize(const char *address, char *buffer, int bufsize) override;
	void Unserialize(int index, const char *serializedData, int dataSize) override;

	virtual bool IsCaseSensitive() const = 0;
	virtual bool IsSorted() const = 0;

	virtual bool Add(const char *item) = 0;
	virtual void Clear() = 0;
	virtual bool Contains(const char *item) const = 0;
	virtual bool Remove(const char *item) = 0;
	virtual int GetItemCount() const = 0;
	virtual void GetItems(std::vector<const char *> &buf) const = 0;

private:
	virtual size_t CalcSerializeSize() = 0;
	virtual void SerializeContainer() = 0;
	virtual void UnserializeContainer(const char *serializedData) = 0;
};

template <typename TSet, bool is_sorted, bool is_casesensitive>
class ScriptSetImpl final : public ScriptSetBase {
public:
	typedef typename TSet::const_iterator ConstIterator;

	ScriptSetImpl() {}

	bool IsCaseSensitive() const override {
		return is_casesensitive;
	}
	bool IsSorted() const override {
		return is_sorted;
	}

	bool Add(const char *item) override {
		if (!item) return false;
		size_t len = strlen(item);
		return TryAddItem(item, len);
	}
	void Clear() override {
		for (auto it = _set.begin(); it != _set.end(); ++it)
			DeleteItem(it);
		_set.clear();
	}
	bool Contains(const char *item) const override {
		return _set.count(String::Wrapper(item)) != 0;
	}
	bool Remove(const char *item) override {
		auto it = _set.find(String::Wrapper(item));
		if (it == _set.end()) return false;
		DeleteItem(it);
		_set.erase(it);
		return true;
	}
	int GetItemCount() const override {
		return _set.size();
	}
	void GetItems(std::vector<const char *> &buf) const override {
		for (auto it = _set.begin(); it != _set.end(); ++it)
			buf.push_back(it->GetCStr());
	}

private:
	bool TryAddItem(const char *item, size_t len) {
		return _set.insert(String(item, len))._value;
	}
	void DeleteItem(ConstIterator it) { /* do nothing */
	}

	size_t CalcSerializeSize() override {
		size_t total_sz = sizeof(int32_t);
		for (auto it = _set.begin(); it != _set.end(); ++it)
			total_sz += sizeof(int32_t) + it->GetLength();
		return total_sz;
	}

	void SerializeContainer() override {
		SerializeInt((int)_set.size());
		for (auto it = _set.begin(); it != _set.end(); ++it) {
			SerializeInt((int)it->GetLength());
			memcpy(&serbuffer[bytesSoFar], it->GetCStr(), it->GetLength());
			bytesSoFar += it->GetLength();
		}
	}

	void UnserializeContainer(const char *serializedData) override {
		size_t item_count = (size_t)UnserializeInt();
		for (size_t i = 0; i < item_count; ++i) {
			size_t len = UnserializeInt();
			TryAddItem(&serializedData[bytesSoFar], len);
			bytesSoFar += len;
		}
	}

	TSet _set;
};

typedef ScriptSetImpl< std::set<String>, true, true > ScriptSet;
typedef ScriptSetImpl< std::set<String, IgnoreCase_LessThan>, true, false > ScriptSetCI;
typedef ScriptSetImpl< std::unordered_set<String>, false, true > ScriptHashSet;
typedef ScriptSetImpl< std::unordered_set<String, IgnoreCase_Hash, IgnoreCase_EqualTo>, false, false > ScriptHashSetCI;

} // namespace AGS3

#endif
