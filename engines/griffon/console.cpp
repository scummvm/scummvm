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

#include "gui/debugger.h"

#include "griffon/console.h"

namespace Griffon {

Console::Console() {
	_godMode = kGodModeNone;
	 registerCmd("godmode", WRAP_METHOD(Console, Cmd_godMode));
	 registerCmd("nodamage", WRAP_METHOD(Console, Cmd_noDamage));
}

bool Console::Cmd_godMode(int argc, const char** argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		debugPrintf("Enables/Disables invincibility and megadamage\n");
		return true;
	}

	if (_godMode != kGodModeNone)
		_godMode = kGodModeNone;
	else
		_godMode = kGodModeAll;

	debugPrintf("God mode is now %s\n", _godMode == kGodModeAll ? "Invincibility/Damage" : "Disabled");
	return true;
}

bool Console::Cmd_noDamage(int argc, const char** argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		debugPrintf("Enables/Disables invincibility\n");
		return true;
	}

	if (_godMode != kGodModeNone)
		_godMode = kGodModeNone;
	else
		_godMode = kGodModeNoDamage;

	debugPrintf("God mode is now %s\n", _godMode ? "Invincibility" : "Disabled");
	return true;
}

} // End of namespace Griffon
