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

#ifndef __CC_SYSTEMIMPORTS_H
#define __CC_SYSTEMIMPORTS_H

#include <map>
#include "script/cc_instance.h"    // ccInstance

struct ICCDynamicObject;
struct ICCStaticObject;

using AGS::Common::String;

struct ScriptImport
{
    ScriptImport()
    {
        InstancePtr = nullptr;
    }

    String              Name;           // import's uid
    RuntimeScriptValue  Value;
    ccInstance          *InstancePtr;   // script instance
};

struct SystemImports
{
private:
    // Note we can't use a hash-map here, because we sometimes need to search
    // by partial keys.
    typedef std::map<String, int> IndexMap;

    std::vector<ScriptImport> imports;
    IndexMap btree;

public:
    int  add(const String &name, const RuntimeScriptValue &value, ccInstance *inst);
    void remove(const String &name);
    const ScriptImport *getByName(const String &name);
    int  get_index_of(const String &name);
    const ScriptImport *getByIndex(int index);
    void RemoveScriptExports(ccInstance *inst);
    void clear();
};

extern SystemImports simp;
// This is to register symbols exclusively for plugins, to allow them
// perform old style unsafe function calls
extern SystemImports simp_for_plugin;

#endif  // __CC_SYSTEMIMPORTS_H