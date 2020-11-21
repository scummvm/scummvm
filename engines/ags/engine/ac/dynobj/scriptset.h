//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
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
#ifndef __AC_SCRIPTSET_H
#define __AC_SCRIPTSET_H

#include <set>
#include <unordered_set>
#include <string.h>
#include "ac/dynobj/cc_agsdynamicobject.h"
#include "util/string.h"
#include "util/string_types.h"

using namespace AGS::Common;

class ScriptSetBase : public AGSCCDynamicObject
{
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
    virtual void GetItems(std::vector<const char*> &buf) const = 0;

private:
    virtual size_t CalcSerializeSize() = 0;
    virtual void SerializeContainer() = 0;
    virtual void UnserializeContainer(const char *serializedData) = 0;
};

template <typename TSet, bool is_sorted, bool is_casesensitive>
class ScriptSetImpl final : public ScriptSetBase
{
public:
    typedef typename TSet::const_iterator ConstIterator;

    ScriptSetImpl() = default;

    bool IsCaseSensitive() const override { return is_casesensitive; }
    bool IsSorted() const override { return is_sorted; }

    bool Add(const char *item) override
    {
        if (!item) return false;
        size_t len = strlen(item);
        return TryAddItem(item, len);
    }
    void Clear() override
    {
        for (auto it = _set.begin(); it != _set.end(); ++it)
            DeleteItem(it);
        _set.clear();
    }
    bool Contains(const char *item) const override { return _set.count(String::Wrapper(item)) != 0; }
    bool Remove(const char *item) override
    {
        auto it = _set.find(String::Wrapper(item));
        if (it == _set.end()) return false;
        DeleteItem(it);
        _set.erase(it);
        return true;
    }
    int GetItemCount() const override { return _set.size(); }
    void GetItems(std::vector<const char*> &buf) const override
    {
        for (auto it = _set.begin(); it != _set.end(); ++it)
            buf.push_back(it->GetCStr());
    }

private:
    bool TryAddItem(const char *item, size_t len)
    {
        return _set.insert(String(item, len)).second;
    }
    void DeleteItem(ConstIterator it) { /* do nothing */ }

    size_t CalcSerializeSize() override
    {
        size_t total_sz = sizeof(int32_t);
        for (auto it = _set.begin(); it != _set.end(); ++it)
            total_sz += sizeof(int32_t) + it->GetLength();
        return total_sz;
    }

    void SerializeContainer() override
    {
        SerializeInt((int)_set.size());
        for (auto it = _set.begin(); it != _set.end(); ++it)
        {
            SerializeInt((int)it->GetLength());
            memcpy(&serbuffer[bytesSoFar], it->GetCStr(), it->GetLength());
            bytesSoFar += it->GetLength();
        }
    }

    void UnserializeContainer(const char *serializedData) override
    {
        size_t item_count = (size_t)UnserializeInt();
        for (size_t i = 0; i < item_count; ++i)
        {
            size_t len = UnserializeInt();
            TryAddItem(&serializedData[bytesSoFar], len);
            bytesSoFar += len;
        }
    }

    TSet _set;
};

typedef ScriptSetImpl< std::set<String>, true, true > ScriptSet;
typedef ScriptSetImpl< std::set<String, StrLessNoCase>, true, false > ScriptSetCI;
typedef ScriptSetImpl< std::unordered_set<String>, false, true > ScriptHashSet;
typedef ScriptSetImpl< std::unordered_set<String, HashStrNoCase, StrEqNoCase>, false, false > ScriptHashSetCI;

#endif // __AC_SCRIPTSET_H
