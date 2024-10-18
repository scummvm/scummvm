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
// types as keys; need to research what performance impact that would make.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_DYNOBJ_SCRIPTSET_H
#define AGS_ENGINE_AC_DYNOBJ_SCRIPTSET_H

#include "common/std/set.h"
#include "common/std/unordered_set.h"
#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string.h"
#include "ags/shared/util/string_types.h"

namespace AGS3 {

using namespace AGS::Shared;

class ScriptSetBase : public AGSCCDynamicObject {
public:
	int Dispose(void *address, bool force) override;
	const char *GetType() override;
	void Unserialize(int index, AGS::Shared::Stream *in, size_t data_sz) override;

	virtual bool IsCaseSensitive() const = 0;
	virtual bool IsSorted() const = 0;

	virtual bool Add(const char *item) = 0;
	virtual void Clear() = 0;
	virtual bool Contains(const char *item) const = 0;
	virtual bool Remove(const char *item) = 0;
	virtual int GetItemCount() const = 0;
	virtual void GetItems(std::vector<const char *> &buf) const = 0;

protected:
	// Calculate and return required space for serialization, in bytes
	virtual size_t CalcSerializeSize(const void *address) override;
	// Write object data into the provided stream
	void Serialize(const void *address, AGS::Shared::Stream *out) override;

private:
	virtual size_t CalcContainerSize() = 0;
	virtual void SerializeContainer(AGS::Shared::Stream *out) = 0;
	virtual void UnserializeContainer(AGS::Shared::Stream *in) = 0;
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
		return TryAddItem(String(item));
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
	bool TryAddItem(const String &s) {
		return _set.insert(s)._value;
	}
	void DeleteItem(ConstIterator /*it*/) { /* do nothing */ }

	size_t CalcContainerSize() override {
		// 2 class properties + item count
		size_t total_sz = sizeof(int32_t) * 3;
		// (int32 + string buffer) per item
		for (auto it = _set.begin(); it != _set.end(); ++it)
			total_sz += sizeof(int32_t) + it->GetLength();
		return total_sz;
	}

	void SerializeContainer(AGS::Shared::Stream *out) override {
		out->WriteInt32((int)_set.size());
		for (auto it = _set.begin(); it != _set.end(); ++it) {
			out->WriteInt32((int)it->GetLength());
			out->Write(it->GetCStr(), it->GetLength());
		}
	}

	void UnserializeContainer(AGS::Shared::Stream *in) override {
		size_t item_count = in->ReadInt32();
		for (size_t i = 0; i < item_count; ++i) {
			size_t len = in->ReadInt32();
			String item = String::FromStreamCount(in, len);
			TryAddItem(item);
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
