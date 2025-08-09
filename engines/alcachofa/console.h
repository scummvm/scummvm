
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

#ifndef ALCACHOFA_CONSOLE_H
#define ALCACHOFA_CONSOLE_H

#include "gui/debugger.h"

namespace Alcachofa {

enum class DebugMode {
	None,
	ClosestFloorPoint,
	FloorIntersections,
	TeleportCharacter,
	FloorAlpha,
	FloorColor
};

class Console : public GUI::Debugger {
public:
	Console();
	~Console() override;

	inline bool showGraphics() const { return _showGraphics; }
	inline bool showInteractables() const { return _showInteractables; }
	inline bool showCharacters() const { return _showCharacters; }
	inline bool showFloor() const { return _showFloor; }
	inline bool showFloorEdges() const { return _showFloorEdges; }
	inline bool showFloorColor() const { return _showFloorColor; }
	bool isAnyDebugDrawingOn() const;

private:
	bool cmdVar(int argc, const char **args);
	bool cmdProcesses(int argc, const char **args);
	bool cmdRoom(int argc, const char **args);
	bool cmdRooms(int argc, const char **args);
	bool cmdChangeRoom(int argc, const char **args);
	bool cmdDisableDebugDraw(int argc, const char **args);
	bool cmdItem(int argc, const char **args);
	bool cmdDebugMode(int argc, const char **args);
	bool cmdTeleport(int argc, const char **args);
	bool cmdToggleRoomFloor(int argc, const char **args);
	bool cmdPlayVideo(int argc, const char **args);

	bool _showGraphics = false;
	bool _showInteractables = false;
	bool _showCharacters = false;
	bool _showFloor = false;
	bool _showFloorEdges = false;
	bool _showFloorColor = false;
};

} // End of namespace Alcachofa

#endif // ALCACHOFA_CONSOLE_H
