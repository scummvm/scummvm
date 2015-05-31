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
 */

#include "common/scummsys.h"
#include "access/access.h"
#include "access/martian/martian_game.h"
#include "access/martian/martian_resources.h"
#include "access/martian/martian_scripts.h"

namespace Access {

namespace Martian {

MartianScripts::MartianScripts(AccessEngine *vm) : Scripts(vm) {
	_game = (MartianEngine *)_vm;
}

void MartianScripts::executeSpecial(int commandIndex, int param1, int param2) {
}

typedef void(MartianScripts::*MartianScriptMethodPtr)();

void MartianScripts::executeCommand(int commandIndex) {
	Scripts::executeCommand(commandIndex);
}

} // End of namespace Martian

} // End of namespace Access
