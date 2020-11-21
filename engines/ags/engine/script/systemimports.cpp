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

#include <stdlib.h>
#include <string.h>
#include "script/systemimports.h"


extern void quit(const char *);

SystemImports simp;
SystemImports simp_for_plugin;

int SystemImports::add(const String &name, const RuntimeScriptValue &value, ccInstance *anotherscr)
{
    int ixof;

    if ((ixof = get_index_of(name)) >= 0) {
        // Only allow override if not a script-exported function
        if (anotherscr == nullptr) {
            imports[ixof].Value = value;
            imports[ixof].InstancePtr = anotherscr;
        }
        return 0;
    }

    ixof = imports.size();
    for (size_t i = 0; i < imports.size(); ++i)
    {
        if (imports[i].Name == nullptr)
        {
            ixof = i;
            break;
        }
    }

    btree[name] = ixof;
    if (ixof == imports.size())
        imports.push_back(ScriptImport());
    imports[ixof].Name          = name; // TODO: rather make a string copy here for safety reasons
    imports[ixof].Value         = value;
    imports[ixof].InstancePtr   = anotherscr;
    return 0;
}

void SystemImports::remove(const String &name) {
    int idx = get_index_of(name);
    if (idx < 0)
        return;
    btree.erase(imports[idx].Name);
    imports[idx].Name = nullptr;
    imports[idx].Value.Invalidate();
    imports[idx].InstancePtr = nullptr;
}

const ScriptImport *SystemImports::getByName(const String &name)
{
    int o = get_index_of(name);
    if (o < 0)
        return nullptr;

    return &imports[o];
}

const ScriptImport *SystemImports::getByIndex(int index)
{
    if ((size_t)index >= imports.size())
        return nullptr;

    return &imports[index];
}

int SystemImports::get_index_of(const String &name)
{
    IndexMap::const_iterator it = btree.find(name);
    if (it != btree.end())
        return it->second;

    // CHECKME: what are "mangled names" and where do they come from?
    String mangled_name = String::FromFormat("%s$", name.GetCStr());
    // if it's a function with a mangled name, allow it
    it = btree.lower_bound(mangled_name);
    if (it != btree.end() && it->first.CompareLeft(mangled_name) == 0)
        return it->second;

    if (name.GetLength() > 3)
    {
        size_t c = name.FindCharReverse('^');
        if (c != -1 && (c == name.GetLength() - 2 || c == name.GetLength() - 3))
        {
            // Function with number of prametrs on the end
            // attempt to find it without the param count
            return get_index_of(name.Left(c));
        }
    }
    return -1;
}

void SystemImports::RemoveScriptExports(ccInstance *inst)
{
    if (!inst)
    {
        return;
    }

    for (size_t i = 0; i < imports.size(); ++i)
    {
        if (imports[i].Name == nullptr)
            continue;

        if (imports[i].InstancePtr == inst)
        {
            btree.erase(imports[i].Name);
            imports[i].Name = nullptr;
            imports[i].Value.Invalidate();
            imports[i].InstancePtr = nullptr;
        }
    }
}

void SystemImports::clear()
{
    btree.clear();
    imports.clear();
}
