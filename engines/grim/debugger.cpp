/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "engines/grim/debugger.h"
#include "engines/grim/md5check.h"

namespace Grim {

Debugger::Debugger()
	: GUI::Debugger() {

	DCmd_Register("check_gamedata", WRAP_METHOD(Debugger, cmd_checkFiles));
}

Debugger::~Debugger() {

}

bool Debugger::cmd_checkFiles(int argc, const char **argv) {
	if (MD5Check::checkFiles()) {
		DebugPrintf("All files are ok.\n");
	} else {
		DebugPrintf("Some files are corrupted or missing.\n");
	}

	return true;
}

}
