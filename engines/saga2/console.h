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

    // Input: <None>. Kills the center actor.
	bool cmdKillProtag(int argc, const char **argv);

    // Input: <Actor ID>. Kills an actor.
	bool cmdKill(int argc, const char **argv);

    // Input: <Object ID>. Prints an object's name.
	bool cmdObjName(int argc, const char **argv);

    // Input: <Name Index>. Prints an ObjectID corresponding to the name index.
	bool cmdObjNameIndexToID(int argc, const char **argv);

    // Input: <Object Name>. Prints a list of objects containing the string in their name.
	bool cmdSearchObj(int argc, const char **argv);

    // Input: <Object ID>. Adds the object to the center actor's inventory.
	bool cmdAddObj(int argc, const char **argv);

    // Input: <1/0>. Sets godmode.
	bool cmdGodmode(int argc, const char **argv);

    // Input: <1/0>. Sets whether the position coordinates show.
	bool cmdPosition(int argc, const char **argv);

    // Input: <1/0>. Sets whether an item's stats show when holding it.
	bool cmdStats(int argc, const char **argv);

	// Input: <1/0>. Sets whether the original debug status messages show.
	bool cmdStatusMsg(int argc, const char **argv);

    // Input: <1/0>. Sets whether you can teleport by right clicking on the screen.
	bool cmdTeleportOnClick(int argc, const char **argv);

    // Input: <1/0>. Sets whether you can teleport by clicking on the map.
	bool cmdTeleportOnMap(int argc, const char **argv);

    // Input: <u> <v> <z>. Teleports the center character to the given position.
	bool cmdTeleport(int argc, const char **argv);

    // Input: <Actor ID>. Teleports the character to the npc.
	bool cmdTeleportToNPC(int argc, const char **argv);

    // Input: <Actor ID> <u> <v> <z>. Teleports the npc to the given position.
	bool cmdTeleportNPC(int argc, const char **argv);

    // Input: <Actor ID>. Teleports the npc to the center actor.
	bool cmdTeleportNPCHere(int argc, const char **argv);

    // Input: <None>. Teleport the three brothers to the center actor.
	bool cmdTeleportPartyHere(int argc, const char **argv);

    // Input: <None>. Saves the current location locally.
	bool cmdSaveLoc(int argc, const char **argv);

    // Input: <None>. Teleports the center actor to the location saved with cmdSaveLoc.
	bool cmdLoadLoc(int argc, const char **argv);

    // Input: <Place ID>. Teleports to the given place.
	bool cmdGotoPlace(int argc, const char **argv);

    // Input: <None>. Lists all of the place names along with their IDs.
	bool cmdListPlaces(int argc, const char **argv);

    // Input: <Map Scale Multiplier>. Dumps the map into a png.
	bool cmdDumpMap(int argc, const char **argv);

    // Input: <Music Index>. Plays a music track.
	bool cmdPlayMusic(int argc, const char **argv);

    // Input: <Voice ID>. Plays a voice track.
	bool cmdPlayVoice(int argc, const char **argv);

	// Input: <1/0>. Sets the invisibility effect on the party.
	bool cmdInvisibility(int argc, const char **argv);

	// Input: <1/0>. Sets state of Automap cheat for testing.
	bool cmdMapCheat(int argc, const char **argv);
};

}

#endif
