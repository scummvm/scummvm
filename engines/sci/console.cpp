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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Console module

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/scicore/resource.h"
#include "sci/scicore/versions.h"

namespace Sci {

Console::Console(SciEngine *vm) : GUI::Debugger() {
	_vm = vm;

	DCmd_Register("version",			WRAP_METHOD(Console, cmdGetVersion));
	DCmd_Register("man",				WRAP_METHOD(Console, cmdMan));
}

Console::~Console() {
}

bool Console::cmdGetVersion(int argc, const char **argv) {
	int ver = _vm->getVersion();

	DebugPrintf("Resource file version:        %s\n", sci_version_types[_vm->getResMgr()->sci_version]);

	DebugPrintf("Emulated interpreter version: %d.%03d.%03d\n",
		SCI_VERSION_MAJOR(ver), SCI_VERSION_MINOR(ver), SCI_VERSION_PATCHLEVEL(ver));

	return true;
}

bool Console::cmdMan(int argc, const char **argv) {
#if 0
	int section = 0;
	unsigned int i;
	char *name = cmd_params[0].str;
	char *c = strchr(name, '.');
	cmd_mm_entry_t *entry = 0;

	if (c) {
		*c = 0;
		section = atoi(c + 1);
	}

	if (section < 0 || section >= CMD_MM_ENTRIES) {
		DebugPrintf("Invalid section %d\n", section);
		return true;
	}

	DebugPrintf("section:%d\n", section);
	if (section)
		entry = cmd_mm_find(name, section - 1);
	else
		for (i = 0; i < CMD_MM_ENTRIES && !section; i++) {
			if ((entry = cmd_mm_find(name, i)))
				section = i + 1;
		}

	if (!entry) {
		DebugPrintf("No manual entry\n");
		return true;
	}

	DebugPrintf("-- %s: %s.%d\n", cmd_mm[section - 1].name, name, section);
	cmd_mm[section - 1].print(entry, 1);

#endif
	return true;
}

} // End of namespace Sci
