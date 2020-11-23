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

#ifndef AGS_ENGINE_SCRIPT_SYSTEMIMPORTS_H
#define AGS_ENGINE_SCRIPT_SYSTEMIMPORTS_H

//include <map>
#include "ags/shared/script/cc_instance.h"    // ccInstance

namespace AGS3 {

struct ICCDynamicObject;
struct ICCStaticObject;

using AGS::Shared::String;

struct ScriptImport {
	ScriptImport() {
		InstancePtr = nullptr;
	}

	String              Name;           // import's uid
	RuntimeScriptValue  Value;
	ccInstance *InstancePtr;   // script instance
};

struct SystemImports {
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

} // namespace AGS3

#endif
