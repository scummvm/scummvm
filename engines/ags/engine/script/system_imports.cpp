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

#include "ags/engine/script/system_imports.h"

namespace AGS3 {

uint32_t SystemImports::add(const String &name, const RuntimeScriptValue &value, ccInstance *anotherscr) {
	uint32_t ixof = get_index_of(name);
	// Check if symbol already exists
	if (ixof != UINT32_MAX) {
		// Only allow override if not a script-exported function
		if (anotherscr == nullptr) {
			imports[ixof].Value = value;
			imports[ixof].InstancePtr = anotherscr;
		}
		return ixof;
	}

	ixof = imports.size();
	for (size_t i = 0; i < imports.size(); ++i) {
		if (imports[i].Name == nullptr) {
			ixof = i;
			break;
		}
	}

	btree[name] = ixof;
	if (ixof == imports.size())
		imports.push_back(ScriptImport());
	imports[ixof].Name = name;
	imports[ixof].Value = value;
	imports[ixof].InstancePtr = anotherscr;
	return ixof;
}

void SystemImports::remove(const String &name) {
	uint32_t idx = get_index_of(name);
	if (idx == UINT32_MAX)
		return;
	btree.erase(imports[idx].Name);
	imports[idx].Name = nullptr;
	imports[idx].Value.Invalidate();
	imports[idx].InstancePtr = nullptr;
}

const ScriptImport *SystemImports::getByName(const String &name) {
	uint32_t o = get_index_of(name);
	if (o == UINT32_MAX)
		return nullptr;

	return &imports[o];
}

const ScriptImport *SystemImports::getByIndex(uint32_t index) {
	if (index >= imports.size())
		return nullptr;

	return &imports[index];
}

uint32_t SystemImports::get_index_of(const String &name) {
	IndexMap::const_iterator it = btree.find(name);
	if (it != btree.end())
		return it->_value;

	// CHECKME: what are "mangled names" and where do they come from?
	String mangled_name = String::FromFormat("%s$", name.GetCStr());
	// if it's a function with a mangled name, allow it
	it = btree.lower_bound(mangled_name);
	if (it != btree.end() && it->_key.CompareLeft(mangled_name) == 0)
		return it->_value;

	if (name.GetLength() > 3) {
		size_t c = name.FindCharReverse('^');
		if (c != String::NoIndex && (c == name.GetLength() - 2 || c == name.GetLength() - 3)) {
			// Function with number of prametrs on the end
			// attempt to find it without the param count
			return get_index_of(name.Left(c));
		}
	}
	return UINT32_MAX;
}

String SystemImports::findName(const RuntimeScriptValue &value) {
	for (const auto &import : imports) {
		if (import.Value == value) {
			return import.Name;
		}
	}
	return String();
}

void SystemImports::RemoveScriptExports(ccInstance *inst) {
	if (!inst) {
		return;
	}

	for (auto &import : imports) {
		if (import.Name == nullptr)
			continue;

		if (import.InstancePtr == inst) {
			btree.erase(import.Name);
			import.Name = nullptr;
			import.Value.Invalidate();
			import.InstancePtr = nullptr;
		}
	}
}

void SystemImports::clear() {
	btree.clear();
	imports.clear();
}

} // namespace AGS3
