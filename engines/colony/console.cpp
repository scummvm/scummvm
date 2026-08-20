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
 * Based on the original sources
 *   https://github.com/Croquetx/thecolony
 * Copyright (C) 1988, David A. Smith
 *
 * Distributed under Apache Version 2.0 License
 *
 */

#include "colony/colony.h"
#include "colony/console.h"

namespace Colony {

int wrapBattleDebugCoord(int coord) {
	coord = (int16)coord;
	if (coord < 0)
		coord += 0x8000;
	return coord;
}

const char *robotTypeName(int type) {
	switch (type) {
	case kRobEye: return "Eye";
	case kRobPyramid: return "Pyramid";
	case kRobCube: return "Cube";
	case kRobUPyramid: return "UPyramid";
	case kRobFEye: return "FEye";
	case kRobFPyramid: return "FPyramid";
	case kRobFCube: return "FCube";
	case kRobFUPyramid: return "FUPyramid";
	case kRobSEye: return "SEye";
	case kRobSPyramid: return "SPyramid";
	case kRobSCube: return "SCube";
	case kRobSUPyramid: return "SUPyramid";
	case kRobMEye: return "MEye";
	case kRobMPyramid: return "MPyramid";
	case kRobMCube: return "MCube";
	case kRobMUPyramid: return "MUPyramid";
	case kRobQueen: return "Queen";
	case kRobDrone: return "Drone";
	case kRobSoldier: return "Soldier";
	case kRobSnoop: return "Snoop";
	default: return "Unknown";
	}
}

const char *featureTypeName(int type) {
	switch (type) {
	case kWallFeatureUpStairs: return "UpStairs";
	case kWallFeatureDnStairs: return "DnStairs";
	case kWallFeatureTunnel: return "Tunnel";
	case kWallFeatureElevator: return "Elevator";
	case kWallFeatureAirlock: return "Airlock";
	default: return nullptr;
	}
}

const char *reactorStateName(int state) {
	switch (state) {
	case 0: return "closed, core installed";
	case 1: return "open, core installed";
	case 2: return "open, core removed";
	default: return "unknown";
	}
}

const char *reactorPowerName(int power) {
	switch (power) {
	case 0: return "off";
	case 1: return "emergency";
	case 2: return "full";
	default: return "unknown";
	}
}

struct NumberedTeleporter {
	int x;
	int y;
	int destination;
};

// MAP.7 contains the four working booths. Their destinations establish the
// original numbering: 1 -> 2 -> 3 -> 4 -> nowhere.
const NumberedTeleporter kNumberedTeleporters[] = {
	{9, 3, 2},
	{3, 3, 3},
	{3, 9, 4},
	{9, 9, 0}
};

Debugger::Debugger(ColonyEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("teleport", WRAP_METHOD(Debugger, cmdTeleport));
	registerCmd("teleporter", WRAP_METHOD(Debugger, cmdTeleporter));
	registerCmd("teleporters", WRAP_METHOD(Debugger, cmdTeleporters));
	registerCmd("pos", WRAP_METHOD(Debugger, cmdPos));
	registerCmd("info", WRAP_METHOD(Debugger, cmdInfo));
	registerCmd("goals", WRAP_METHOD(Debugger, cmdGoals));
	registerCmd("win", WRAP_METHOD(Debugger, cmdWin));
	registerCmd("robots", WRAP_METHOD(Debugger, cmdRobots));
	registerCmd("map", WRAP_METHOD(Debugger, cmdMap));
	registerCmd("give", WRAP_METHOD(Debugger, cmdGive));
	registerCmd("power", WRAP_METHOD(Debugger, cmdPower));
	registerCmd("core", WRAP_METHOD(Debugger, cmdCore));
	registerCmd("battle", WRAP_METHOD(Debugger, cmdBattle));
	registerCmd("colony", WRAP_METHOD(Debugger, cmdColony));
	registerCmd("forklift", WRAP_METHOD(Debugger, cmdForklift));
	registerCmd("spawn", WRAP_METHOD(Debugger, cmdSpawn));
}

void Debugger::postEnter() {
	GUI::Debugger::postEnter();

	if (_pendingEnding == 0)
		return;

	const int ending = _pendingEnding;
	_pendingEnding = 0;
	const bool destroyPlanet = ending <= 3;
	const int savedCryos = (ending == 1 || ending == 4) ? 6 :
		((ending == 2 || ending == 5) ? 1 : 0);
	_vm->gameOver(destroyPlanet, savedCryos);
}

bool Debugger::getTeleporterLocation(int number, int &level, int &x, int &y) const {
	const NumberedTeleporter &teleporter = kNumberedTeleporters[number - 1];
	level = 7;
	x = teleporter.x;
	y = teleporter.y;

	for (uint i = 0; i < _vm->_patches.size(); i++) {
		const PatchEntry &patch = _vm->_patches[i];
		if (patch.type == kObjTeleport && patch.from.level == 7 &&
				patch.from.xindex == teleporter.x && patch.from.yindex == teleporter.y) {
			level = patch.to.level;
			x = patch.to.xindex;
			y = patch.to.yindex;
			return true;
		}
	}
	return false;
}

bool Debugger::cmdTeleporters(int argc, const char **argv) {
	debugPrintf("Numbered teleporter series (1 -> 2 -> 3 -> 4):\n");
	for (uint i = 0; i < ARRAYSIZE(kNumberedTeleporters); i++) {
		int level, x, y;
		getTeleporterLocation(i + 1, level, x, y);
		if (level == 100) {
			debugPrintf("  %u: carried by the forklift", i + 1);
		} else {
			debugPrintf("  %u: level %d at (%d, %d)", i + 1, level, x, y);
		}

		if (kNumberedTeleporters[i].destination != 0)
			debugPrintf(" -> %d\n", kNumberedTeleporters[i].destination);
		else
			debugPrintf(" -> nowhere\n");
	}
	return true;
}

bool Debugger::cmdTeleporter(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: teleporter <number>\n");
		debugPrintf("Recalls working teleporter 1-4 into the cell in front of the player.\n");
		debugPrintf("Use 'teleporters' to list their current locations.\n");
		return true;
	}

	char *end = nullptr;
	const long number = strtol(argv[1], &end, 10);
	if (!argv[1][0] || *end != '\0' || number < 1 || number > (long)ARRAYSIZE(kNumberedTeleporters)) {
		debugPrintf("Invalid teleporter number '%s' (must be 1-4)\n", argv[1]);
		return true;
	}

	if (_vm->_gameMode != kModeColony || _vm->_level < 1 || _vm->_level > 7) {
		debugPrintf("Must be in colony mode to recall a teleporter\n");
		return true;
	}

	int level, x, y;
	const bool relocated = getTeleporterLocation((int)number, level, x, y);
	if (level != 100 && (level < 1 || level > 7 || x < 0 || x > 31 || y < 0 || y > 31)) {
		debugPrintf("Teleporter %ld has an invalid location: level %d at (%d, %d)\n",
			number, level, x, y);
		return true;
	}

	const int facingX = _vm->_cost[_vm->_me.ang];
	const int facingY = _vm->_sint[_vm->_me.ang];
	const bool horizontal = ABS(facingX) >= ABS(facingY);
	const int stepX = horizontal ? (facingX >= 0 ? 1 : -1) : 0;
	const int stepY = horizontal ? 0 : (facingY >= 0 ? 1 : -1);
	const int targetX = _vm->_me.xindex + stepX;
	const int targetY = _vm->_me.yindex + stepY;
	if (targetX < 1 || targetX > 30 || targetY < 1 || targetY > 30) {
		debugPrintf("Target cell (%d, %d) is out of bounds\n", targetX, targetY);
		return true;
	}

	if (level == _vm->_level && x == targetX && y == targetY) {
		debugPrintf("Teleporter %ld is already in front of the player at (%d, %d)\n",
			number, targetX, targetY);
		return false;
	}

	if (_vm->_robotArray[targetX][targetY] != 0) {
		debugPrintf("Target cell (%d, %d) is already occupied\n", targetX, targetY);
		return true;
	}

	bool blocked = false;
	if (stepX > 0)
		blocked = (_vm->_wall[targetX][targetY] & 2) != 0;
	else if (stepX < 0)
		blocked = (_vm->_wall[_vm->_me.xindex][_vm->_me.yindex] & 2) != 0;
	else if (stepY > 0)
		blocked = (_vm->_wall[targetX][targetY] & 1) != 0;
	else
		blocked = (_vm->_wall[_vm->_me.xindex][_vm->_me.yindex] & 1) != 0;
	if (blocked) {
		debugPrintf("A wall blocks target cell (%d, %d)\n", targetX, targetY);
		return true;
	}

	if (!relocated && _vm->_patches.size() >= 100) {
		debugPrintf("Cannot recall teleporter %ld: relocation table is full\n", number);
		return true;
	}

	int oldObject = -1;
	if (level == _vm->_level) {
		for (uint i = 0; i < _vm->_objects.size(); i++) {
			const Thing &obj = _vm->_objects[i];
			if (obj.alive && obj.type == kObjTeleport &&
					obj.where.xindex == x && obj.where.yindex == y) {
				oldObject = i;
				break;
			}
		}
	}

	if (oldObject >= 0) {
		if (_vm->_robotArray[x][y] == oldObject + 1)
			_vm->_robotArray[x][y] = 0;
		_vm->_objects[oldObject].alive = 0;
	}

	const int targetXLoc = (targetX << 8) + 128;
	const int targetYLoc = (targetY << 8) + 128;
	if (!_vm->createObject(kObjTeleport, targetXLoc, targetYLoc, 0)) {
		if (oldObject >= 0) {
			_vm->_objects[oldObject].alive = 1;
			_vm->_robotArray[x][y] = oldObject + 1;
		}
		debugPrintf("Failed to place teleporter %ld at (%d, %d)\n", number, targetX, targetY);
		return true;
	}

	PassPatch from = {};
	from.level = level;
	from.xindex = x;
	from.yindex = y;

	PassPatch to = {};
	to.level = _vm->_level;
	to.xindex = targetX;
	to.yindex = targetY;
	to.xloc = targetXLoc;
	to.yloc = targetYLoc;
	to.ang = 0;

	uint8 mapdata[5] = {6, kObjTeleport - kBaseObject, 0, 0, 0};
	const int destination = kNumberedTeleporters[number - 1].destination;
	if (destination != 0) {
		mapdata[2] = 7;
		mapdata[3] = kNumberedTeleporters[destination - 1].x;
		mapdata[4] = kNumberedTeleporters[destination - 1].y;
	}
	_vm->newPatch(kObjTeleport, from, to, mapdata);

	if (level == 100 && _vm->_carryType == kObjTeleport) {
		_vm->_carryType = 0;
		if (_vm->_fl == 2)
			_vm->_fl = 1;
	}
	_vm->_bumpedObject = 0;
	_vm->setPlayerCellMarker();

	debugPrintf("Recalled teleporter %ld to level %d at (%d, %d)\n",
		number, _vm->_level, targetX, targetY);
	return false;
}

bool Debugger::cmdTeleport(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: teleport <level> [<x> <y>]\n");
		debugPrintf("  level: floor number (1-8)\n");
		debugPrintf("  x, y:  cell coordinates (0-31). If omitted, finds an entry point.\n");
		return true;
	}

	int level = atoi(argv[1]);
	if (level < 1 || level > 8) {
		debugPrintf("Invalid level %d (must be 1-8)\n", level);
		return true;
	}

	int targetX = -1, targetY = -1;

	if (argc >= 4) {
		targetX = atoi(argv[2]);
		targetY = atoi(argv[3]);
		if (targetX < 0 || targetX > 31 || targetY < 0 || targetY > 31) {
			debugPrintf("Invalid coordinates (%d, %d) - must be 0-31\n", targetX, targetY);
			return true;
		}
	}

	// Clear player from current robot array position
	if (_vm->_me.xindex >= 0 && _vm->_me.xindex < 32 &&
		_vm->_me.yindex >= 0 && _vm->_me.yindex < 32)
		_vm->_robotArray[_vm->_me.xindex][_vm->_me.yindex] = 0;

	// Load the target level
	if (level != _vm->_level)
		_vm->loadMap(level);

	// If no coordinates given, scan for an entry point (stairs/tunnel/elevator)
	if (targetX < 0) {
		for (int x = 0; x < 31 && targetX < 0; x++) {
			for (int y = 0; y < 31 && targetX < 0; y++) {
				for (int d = 0; d < 5; d++) {
					int feat = _vm->_mapData[x][y][d][0];
					if (feat == kWallFeatureUpStairs || feat == kWallFeatureDnStairs ||
						feat == kWallFeatureTunnel || feat == kWallFeatureElevator) {
						targetX = x;
						targetY = y;
						const char *name = featureTypeName(feat);
						debugPrintf("Found entry point: %s at (%d, %d)\n", name, x, y);
						break;
					}
				}
			}
		}
		if (targetX < 0) {
			// Fallback: center of map
			targetX = 17;
			targetY = 17;
			debugPrintf("No entry point found, using default (%d, %d)\n", targetX, targetY);
		}
	}

	// Place player at target position (center of cell)
	_vm->_me.xindex = targetX;
	_vm->_me.yindex = targetY;
	_vm->_me.xloc = (targetX << 8) + 128;
	_vm->_me.yloc = (targetY << 8) + 128;

	// Register player in robot array
	_vm->_robotArray[targetX][targetY] = kMeNum;

	debugPrintf("Teleported to level %d at (%d, %d)\n", _vm->_level, targetX, targetY);
	return false;
}

bool Debugger::cmdPos(int argc, const char **argv) {
	debugPrintf("Level: %d\n", _vm->_level);
	debugPrintf("Cell:  (%d, %d)\n", _vm->_me.xindex, _vm->_me.yindex);
	debugPrintf("Exact: (%d, %d)\n", _vm->_me.xloc, _vm->_me.yloc);
	debugPrintf("Angle: %d  Look: %d\n", _vm->_me.ang, _vm->_me.look);
	return true;
}

bool Debugger::cmdInfo(int argc, const char **argv) {
	debugPrintf("=== Colony Game State ===\n");
	debugPrintf("Level: %d  Position: (%d, %d)  Angle: %d\n",
		_vm->_level, _vm->_me.xindex, _vm->_me.yindex, _vm->_me.ang);
	debugPrintf("Core index: %d\n", _vm->_coreIndex);
	debugPrintf("Core power:  [0]=%d  [1]=%d  [2]=%d\n",
		_vm->_corePower[0], _vm->_corePower[1], _vm->_corePower[2]);
	debugPrintf("Core state:  [0]=%d  [1]=%d\n",
		_vm->_coreState[0], _vm->_coreState[1]);
	debugPrintf("Core height: [0]=%d  [1]=%d\n",
		_vm->_coreHeight[0], _vm->_coreHeight[1]);
	debugPrintf("Keycard: %s  Unlocked: %s\n",
		_vm->_hasKeycard ? "yes" : "no",
		_vm->_unlocked ? "yes" : "no");
	debugPrintf("Weapons: %d  Armor: %d\n", _vm->_weapons, _vm->_armor);
	debugPrintf("Orbit: %d  Forklift: %d  CarryType: %d\n",
		_vm->_orbit, _vm->_fl, _vm->_carryType);
	debugPrintf("Robots: %d  Speed: %d\n", _vm->_robotNum, _vm->_speedShift);
	return true;
}

bool Debugger::cmdGoals(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: goals\n");
		return true;
	}

	const int savedCryos = _vm->countSavedCryos();
	debugPrintf("=== Mission Goals ===\n");
	debugPrintf("Cryogenic chambers recovered: %d / 6%s\n",
		savedCryos, savedCryos == 6 ? " (complete)" : "");

	const char *reactorNames[] = {"Ship", "Colony"};
	for (int i = 0; i < 2; i++) {
		debugPrintf("%s reactor: %s; power %s (%d)\n",
			reactorNames[i], reactorStateName(_vm->_coreState[i]),
			reactorPowerName(_vm->_corePower[i]), _vm->_corePower[i]);
	}

	if (_vm->_fl == 2 && _vm->_carryType == kObjReactor) {
		debugPrintf("Reactor core in forklift: yes; power %s (%d)\n",
			reactorPowerName(_vm->_corePower[2]), _vm->_corePower[2]);
	} else {
		debugPrintf("Reactor core in forklift: no\n");
	}
	return true;
}

bool Debugger::cmdWin(int argc, const char **argv) {
	static const char *const endingDescriptions[] = {
		"planet destroyed, all 6 cryos recovered",
		"planet destroyed, some cryos recovered",
		"planet destroyed, no cryos recovered",
		"planet spared, all 6 cryos recovered",
		"planet spared, some cryos recovered",
		"planet spared, no cryos recovered"
	};

	if (argc != 2) {
		debugPrintf("Usage: win <ending>\n");
		for (uint i = 0; i < ARRAYSIZE(endingDescriptions); i++)
			debugPrintf("  %u: %s\n", i + 1, endingDescriptions[i]);
		return true;
	}

	char *end = nullptr;
	const long ending = strtol(argv[1], &end, 10);
	if (!argv[1][0] || *end != '\0' || ending < 1 || ending > (long)ARRAYSIZE(endingDescriptions)) {
		debugPrintf("Invalid ending number '%s' (must be 1-6)\n", argv[1]);
		return true;
	}

	debugPrintf("Playing ending %ld: %s\n", ending, endingDescriptions[ending - 1]);
	_pendingEnding = (int)ending;
	detach();
	return false;
}

bool Debugger::cmdRobots(int argc, const char **argv) {
	int count = 0;
	for (int i = 0; i < (int)_vm->_objects.size(); i++) {
		const Thing &obj = _vm->_objects[i];
		if (obj.alive) {
			debugPrintf("  #%d  type=%d (%s)  pos=(%d,%d)  alive=%d  visible=%d\n",
				i, obj.type, robotTypeName(obj.type),
				obj.where.xindex, obj.where.yindex,
				obj.alive, obj.visible);
			count++;
		}
	}
	if (count == 0)
		debugPrintf("No active robots on level %d\n", _vm->_level);
	else
		debugPrintf("Total: %d active robots\n", count);
	return true;
}

bool Debugger::cmdMap(int argc, const char **argv) {
	int cx = _vm->_me.xindex;
	int cy = _vm->_me.yindex;
	int radius = 5;
	if (argc >= 2)
		radius = atoi(argv[1]);

	debugPrintf("Wall map around (%d,%d), radius %d  (bits: 1=S wall, 2=E wall)\n", cx, cy, radius);
	debugPrintf("    ");
	for (int x = cx - radius; x <= cx + radius; x++)
		debugPrintf("%3d", x);
	debugPrintf("\n");

	for (int y = cy - radius; y <= cy + radius; y++) {
		debugPrintf("%3d ", y);
		for (int x = cx - radius; x <= cx + radius; x++) {
			if (x < 0 || x > 31 || y < 0 || y > 31) {
				debugPrintf("  .");
			} else if (x == cx && y == cy) {
				debugPrintf("  @");
			} else {
				int r = _vm->_robotArray[x][y];
				if (r > 0 && r != kMeNum)
					debugPrintf("  R");
				else
					debugPrintf(" %2d", _vm->_wall[x][y]);
			}
		}
		debugPrintf("\n");
	}
	return true;
}

bool Debugger::cmdGive(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: give <item>\n");
		debugPrintf("  Items: keycard, weapons, armor, all\n");
		return true;
	}

	Common::String item(argv[1]);
	if (item == "keycard") {
		_vm->_hasKeycard = true;
		debugPrintf("Granted keycard\n");
	} else if (item == "weapons") {
		_vm->_weapons = 3;
		debugPrintf("Granted full weapons (3)\n");
	} else if (item == "armor") {
		_vm->_armor = 3;
		debugPrintf("Granted full armor (3)\n");
	} else if (item == "all") {
		_vm->_hasKeycard = true;
		_vm->_weapons = 3;
		_vm->_armor = 3;
		debugPrintf("Granted keycard, full weapons, full armor\n");
	} else {
		debugPrintf("Unknown item '%s'. Valid: keycard, weapons, armor, all\n", argv[1]);
	}
	return true;
}

bool Debugger::cmdPower(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Core power: [0]=%d  [1]=%d  [2]=%d\n",
			_vm->_corePower[0], _vm->_corePower[1], _vm->_corePower[2]);
		debugPrintf("Usage: power <core 0-2> <level 0-2>\n");
		debugPrintf("  0=off, 1=emergency, 2=full\n");
		return true;
	}

	if (argc >= 3) {
		int core = atoi(argv[1]);
		int level = atoi(argv[2]);
		if (core < 0 || core > 2) {
			debugPrintf("Invalid core %d (must be 0-2)\n", core);
			return true;
		}
		if (level < 0 || level > 2) {
			debugPrintf("Invalid power level %d (must be 0-2)\n", level);
			return true;
		}
		_vm->_corePower[core] = level;
		debugPrintf("Core %d power set to %d\n", core, level);
	} else {
		debugPrintf("Core power: [0]=%d  [1]=%d  [2]=%d\n",
			_vm->_corePower[0], _vm->_corePower[1], _vm->_corePower[2]);
	}
	return true;
}

bool Debugger::cmdCore(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Core state:  [0]=%d  [1]=%d\n", _vm->_coreState[0], _vm->_coreState[1]);
		debugPrintf("Core height: [0]=%d  [1]=%d\n", _vm->_coreHeight[0], _vm->_coreHeight[1]);
		debugPrintf("Core index: %d\n", _vm->_coreIndex);
		debugPrintf("Usage: core <index 0-1> <state>\n");
		return true;
	}

	if (argc >= 3) {
		int index = atoi(argv[1]);
		int state = atoi(argv[2]);
		if (index < 0 || index > 1) {
			debugPrintf("Invalid core index %d (must be 0-1)\n", index);
			return true;
		}
		_vm->_coreState[index] = state;
		debugPrintf("Core %d state set to %d\n", index, state);
	} else {
		debugPrintf("Core state:  [0]=%d  [1]=%d\n", _vm->_coreState[0], _vm->_coreState[1]);
		debugPrintf("Core height: [0]=%d  [1]=%d\n", _vm->_coreHeight[0], _vm->_coreHeight[1]);
	}
	return true;
}

bool Debugger::cmdBattle(int argc, const char **argv) {
	auto prepareBattleDebugState = [&](int xloc, int yloc, int ang) {
		_vm->battleInit();
		_vm->battleSet();

		_vm->_me.xloc = xloc;
		_vm->_me.yloc = yloc;
		_vm->_me.xindex = wrapBattleDebugCoord(xloc) >> 8;
		_vm->_me.yindex = wrapBattleDebugCoord(yloc) >> 8;
		_vm->_me.ang = ang;
		_vm->_me.look = ang;
		_vm->_me.lookY = 0;

		_vm->_me.power[0] = 256;
		_vm->_me.power[1] = 256;
		_vm->_me.power[2] = 256;
		_vm->_weapons = 3;
		_vm->_armor = 3;
		_vm->_hasKeycard = true;
		_vm->_orbit = 0;
		_vm->_projon = false;
		_vm->_pcount = 0;

		_vm->_gameMode = kModeBattle;
		_vm->_level = 0;
	};

	// Place player just outside the ship entrance (Ship is at 0, 0).
	// Original BattleCommand ship check: x in [Ship.xloc-2*BSIZE, Ship.xloc).
	prepareBattleDebugState(-500, 0, 96);

	debugPrintf("Entered battle mode outside the ship at (%d, %d) ang=%d\n",
		_vm->_me.xloc, _vm->_me.yloc, _vm->_me.ang);
	debugPrintf("Suit: power=[%d,%d,%d] weapons=%d armor=%d\n",
		(int)_vm->_me.power[0], (int)_vm->_me.power[1],
		(int)_vm->_me.power[2], _vm->_weapons, _vm->_armor);
	return false;
}

bool Debugger::cmdColony(int argc, const char **argv) {
	auto prepareBattleDebugState = [&](int xloc, int yloc, int ang) {
		_vm->battleInit();
		_vm->battleSet();

		_vm->_me.xloc = xloc;
		_vm->_me.yloc = yloc;
		_vm->_me.xindex = wrapBattleDebugCoord(xloc) >> 8;
		_vm->_me.yindex = wrapBattleDebugCoord(yloc) >> 8;
		_vm->_me.ang = ang;
		_vm->_me.look = ang;
		_vm->_me.lookY = 0;

		_vm->_me.power[0] = 256;
		_vm->_me.power[1] = 256;
		_vm->_me.power[2] = 256;
		_vm->_weapons = 3;
		_vm->_armor = 3;
		_vm->_hasKeycard = true;
		_vm->_unlocked = true;
		_vm->_orbit = 0;
		_vm->_projon = false;
		_vm->_pcount = 0;

		_vm->_gameMode = kModeBattle;
		_vm->_level = 0;
	};

	// Place player just outside the colony entrance (Enter is at 16000, 16000).
	// Original BattleCommand entrance check: x in [Enter.xloc-2*BSIZE, Enter.xloc).
	prepareBattleDebugState(16000 - 500, 16000, 96);

	debugPrintf("Entered battle mode outside the colony at (%d, %d) ang=%d\n",
		_vm->_me.xloc, _vm->_me.yloc, _vm->_me.ang);
	debugPrintf("Suit: power=[%d,%d,%d] weapons=%d armor=%d\n",
		(int)_vm->_me.power[0], (int)_vm->_me.power[1],
		(int)_vm->_me.power[2], _vm->_weapons, _vm->_armor);
	debugPrintf("Security state forced to unlocked for colony debug entry\n");
	return false;
}

bool Debugger::cmdForklift(int argc, const char **argv) {
	if (_vm->_gameMode != kModeColony) {
		debugPrintf("Must be in colony mode\n");
		return true;
	}

	if (argc >= 2) {
		int state = atoi(argv[1]);
		if (state < 0 || state > 2) {
			debugPrintf("Invalid state %d (must be 0-2)\n", state);
			return true;
		}
		_vm->_fl = state;
		if (state > 0)
			_vm->_me.lookY = 0; // reset vertical look
		if (state == 0) {
			_vm->_carryType = 0;
			debugPrintf("Exited forklift (fl=0)\n");
		} else if (state == 1) {
			_vm->_carryType = 0;
			debugPrintf("Entered empty forklift (fl=1)\n");
		} else {
			if (_vm->_carryType == 0)
				_vm->_carryType = kObjBox1;
			debugPrintf("In forklift carrying type %d (fl=2)\n", _vm->_carryType);
		}
	} else {
		debugPrintf("Usage: forklift <state>\n");
		debugPrintf("  0 = exit forklift\n");
		debugPrintf("  1 = enter empty forklift\n");
		debugPrintf("  2 = enter forklift carrying object\n");
		debugPrintf("Current: fl=%d carryType=%d\n", _vm->_fl, _vm->_carryType);
	}
	return true;
}

bool Debugger::cmdSpawn(int argc, const char **argv) {
	if (_vm->_gameMode != kModeColony) {
		debugPrintf("Must be in colony mode\n");
		return true;
	}

	const struct { const char *name; int type; } spawnTypes[] = {
		{"eye",       kRobEye},
		{"pyramid",   kRobPyramid},
		{"cube",      kRobCube},
		{"upyramid",  kRobUPyramid},
		{"queen",     kRobQueen},
		{"drone",     kRobDrone},
		{"soldier",   kRobSoldier},
		{"snoop",     kRobSnoop},
		{"feye",      kRobFEye},
		{"fpyramid",  kRobFPyramid},
		{"fcube",     kRobFCube},
		{"fupyramid", kRobFUPyramid},
		{"seye",      kRobSEye},
		{"spyramid",  kRobSPyramid},
		{"scube",     kRobSCube},
		{"supyramid", kRobSUPyramid},
		{"meye",      kRobMEye},
		{"mpyramid",  kRobMPyramid},
		{"mcube",     kRobMCube},
		{"mupyramid", kRobMUPyramid},
	};

	if (argc < 2) {
		debugPrintf("Usage: spawn <type>\n");
		debugPrintf("Spawns an enemy in the cell in front of the player.\n\n");
		debugPrintf("Full robots:\n");
		debugPrintf("  eye, pyramid, cube, upyramid\n");
		debugPrintf("  queen, drone, soldier, snoop\n");
		debugPrintf("Egg stages (floating/small/mini):\n");
		debugPrintf("  feye, fpyramid, fcube, fupyramid\n");
		debugPrintf("  seye, spyramid, scube, supyramid\n");
		debugPrintf("  meye, mpyramid, mcube, mupyramid\n");
		return true;
	}

	Common::String name(argv[1]);
	name.toLowercase();

	int type = -1;
	for (uint i = 0; i < ARRAYSIZE(spawnTypes); i++) {
		if (name == spawnTypes[i].name) {
			type = spawnTypes[i].type;
			break;
		}
	}

	if (type < 0) {
		debugPrintf("Unknown enemy type '%s'. Use 'spawn' with no arguments to see valid types.\n", argv[1]);
		return true;
	}

	// Compute the cell in front of the player
	const int dx = _vm->_cost[_vm->_me.ang] >> 2;
	const int dy = _vm->_sint[_vm->_me.ang] >> 2;
	int targetX = _vm->_me.xindex + (dx > 0 ? 1 : (dx < 0 ? -1 : 0));
	int targetY = _vm->_me.yindex + (dy > 0 ? 1 : (dy < 0 ? -1 : 0));

	if (targetX < 1 || targetX > 30 || targetY < 1 || targetY > 30) {
		debugPrintf("Target cell (%d,%d) is out of bounds\n", targetX, targetY);
		return true;
	}

	if (_vm->_robotArray[targetX][targetY] != 0) {
		debugPrintf("Target cell (%d,%d) is already occupied\n", targetX, targetY);
		return true;
	}

	int xloc = (targetX << 8) + 128;
	int yloc = (targetY << 8) + 128;
	uint8 ang = objAngFromPlayer((uint8)(_vm->_me.ang + 128)); // face the player

	if (!_vm->createObject(type, xloc, yloc, ang)) {
		debugPrintf("Failed to create object (no free slot?)\n");
		return true;
	}

	debugPrintf("Spawned %s (type %d) at cell (%d,%d) facing player\n",
		robotTypeName(type), type, targetX, targetY);
	return false;
}

} // End of namespace Colony
