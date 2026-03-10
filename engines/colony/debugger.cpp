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

#include "colony/debugger.h"
#include "colony/colony.h"

namespace Colony {

static const char *robotTypeName(int type) {
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

static const char *featureTypeName(int type) {
	switch (type) {
	case kWallFeatureUpStairs: return "UpStairs";
	case kWallFeatureDnStairs: return "DnStairs";
	case kWallFeatureTunnel: return "Tunnel";
	case kWallFeatureElevator: return "Elevator";
	case kWallFeatureAirlock: return "Airlock";
	default: return nullptr;
	}
}

Debugger::Debugger(ColonyEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("teleport", WRAP_METHOD(Debugger, cmdTeleport));
	registerCmd("pos", WRAP_METHOD(Debugger, cmdPos));
	registerCmd("info", WRAP_METHOD(Debugger, cmdInfo));
	registerCmd("robots", WRAP_METHOD(Debugger, cmdRobots));
	registerCmd("map", WRAP_METHOD(Debugger, cmdMap));
	registerCmd("give", WRAP_METHOD(Debugger, cmdGive));
	registerCmd("power", WRAP_METHOD(Debugger, cmdPower));
	registerCmd("core", WRAP_METHOD(Debugger, cmdCore));
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
	_vm->_coreIndex = (level == 1) ? 0 : 1;

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
	return true;
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

} // End of namespace Colony
