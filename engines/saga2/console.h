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

#ifndef SAGA2_CONSOLE_H
#define SAGA2_CONSOLE_H

#include "gui/debugger.h"

#include "saga2/tcoords.h"

namespace Saga2 {

class Saga2Engine;

class Console : public GUI::Debugger {
public:
	Console(Saga2Engine *vm);
	~Console() override;

private:
	Saga2Engine *_vm;

	TilePoint _savedLoc;

	bool cmdKillProtag(int argc, const char **argv);

	bool cmdKill(int argc, const char **argv);

	bool cmdObjName(int argc, const char **argv);

	bool cmdObjNameIndexToID(int argc, const char **argv);

	bool cmdSearchObj(int argc, const char **argv);

	bool cmdAddObj(int argc, const char **argv);

	bool cmdGodmode(int argc, const char **argv);

	bool cmdPosition(int argc, const char **argv);

	bool cmdStats(int argc, const char **argv);

	bool cmdTeleportOnClick(int argc, const char **argv);

	bool cmdTeleport(int argc, const char **argv);

	bool cmdTeleportToNPC(int argc, const char **argv);

	bool cmdTeleportNPC(int argc, const char **argv);

	bool cmdTeleportNPCHere(int argc, const char **argv);

	bool cmdSaveLoc(int argc, const char **argv);

	bool cmdLoadLoc(int argc, const char **argv);

	bool cmdGotoPlace(int argc, const char **argv);

	bool cmdListPlaces(int argc, const char **argv);

	bool cmdDumpMap(int argc, const char **argv);
};

}

#endif
