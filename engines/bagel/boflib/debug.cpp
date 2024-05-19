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

#include "common/config-manager.h"
#include "common/debug.h"
#include "bagel/boflib/debug.h"
#include "bagel/bagel.h"

namespace Bagel {

CBofDebugOptions *g_pDebugOptions = nullptr;

CBofDebugOptions::CBofDebugOptions(const char *pszFileName) : CBofOptions(pszFileName) {
	// Add programmer definable debug options here
	ConfMan.registerDefault("AbortsOn", true);
	ConfMan.registerDefault("MessageBoxOn", true);
	ConfMan.registerDefault("RandomOn", true);
	ConfMan.registerDefault("DebugLevel", gDebugLevel);
	ConfMan.registerDefault("ShowIO",false);
	ConfMan.registerDefault("MessageSpy", false);


	readSetting("DebugOptions", "AbortsOn", &_bAbortsOn, ConfMan.getBool("AbortsOn"));
	readSetting("DebugOptions", "MessageBoxOn", &_bMessageBoxOn, ConfMan.getBool("MessageBoxOn"));
	readSetting("DebugOptions", "RandomOn", &_bRandomOn, ConfMan.getBool("RandomOn"));
	readSetting("DebugOptions", "DebugLevel", &_nDebugLevel, ConfMan.getInt("DebugLevel"));
	readSetting("DebugOptions", "ShowIO", &_bShowIO, ConfMan.getBool("ShowIO"));
	readSetting("DebugOptions", "MessageSpy", &_bShowMessages, ConfMan.getBool("MessageSpy"));
}

} // namespace Bagel
