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

#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"

namespace Scumm {

/**
 * Logic code for:
 *     Backyard Basketball
 */
class LogicHEbasketball : public LogicHE {
public:
	LogicHEbasketball(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID() override;
	int32 dispatch(int op, int numArgs, int32 *args) override;

private:
	int op_1012();
	int op_1050(int32 *args);
	int op_1053();

	// op_1050 loads court object data
	enum CourtObjectType {
		kObjectTypeBackboard = 1,
		kObjectTypeRim = 2,
		kObjectTypeOther = 3,
		kObjectTypeFloor = 4
	};

	struct CourtObject {
		Common::String name;
		CourtObjectType type;
		uint32 data[10];
	};

	Common::Array<CourtObject> _courtObjects;
	uint32 _backboardObjectLeft, _backboardObjectRight;
};

int LogicHEbasketball::versionID() {
	return 1;
}

int32 LogicHEbasketball::dispatch(int op, int numArgs, int32 *args) {
	int res = 0;

	switch (op) {
	case 1001:
		break;

	case 1006:	// WORLD_TO_SCREEN_TRANSLATION
		break;

	case 1011:	// INIT_SCREEN_TRANSLATIONS
		break;

	case 1012:	// GET_COURT_DIMENSIONS
		res = op_1012();
		break;

	case 1035:	// COMPUTE_POINTS_FOR_PIXELS
		break;

	case 1050:	// INIT_COURT
		res = op_1050(args);
		break;

	case 1051:	// INIT_BALL
		break;

	case 1052:	// INIT_PLAYER
		break;

	case 1053:	// DEINIT_COURT
		res = op_1053();
		break;

	case 1056:	// DETECT_BALL_COLLISION
		break;

	case 1057:	// DETECT_PLAYER_COLLISION
		break;

	case 1058:	// GET_LAST_BALL_COLLISION
		break;

	case 1060:	// INIT_VIRTUAL_BALL
		break;

	case 1064:	// RASIE_SHIELDS
		break;

	case 1067:	// IS_PLAYER_IN_BOUNDS
		break;

	case 1073:	// SET_BALL_LOCATION
		break;

	case 1075:	// SET_PLAYER_LOCATION
		break;

	case 1076:	// GET_PLAYER_LOCATION
		break;

	case 1080:	// IS_PLAYER_IN_GAME
		break;

	case 1081:	// IS_BALL_IN_GAME
		break;

	case 1090:	// UPDATE_CURSOR_POS
		break;

	case 1091:	// MAKE_CURSOR_STICKY
		break;

	case 1513:	// INIT_NETWORK_SYSTEM_KLUDGE
		break;

	default:
		LogicHE::dispatch(op, numArgs, args);
	}

	return res;
}

int LogicHEbasketball::op_1012() {
	writeScummVar(108, 12000);
	writeScummVar(109, 8000);
	writeScummVar(110, 760);
	writeScummVar(111, 4000);
	writeScummVar(112, 1600);
	return 1;
}

int LogicHEbasketball::op_1050(int32 *args) {
	// This function loads the court data
	static const char *const courtNames[] = {
		"Dobbaguchi", "Jocindas", "SandyFlats", "Queens",
		"Park", "Scheffler", "Polk", "McMillan",
		"CrownHill", "Memorial", "TechState", "Garden",
		"Moon", "Barn"
	};

	Common::Path courtFileName = Common::Path(Common::String::format("data/courts/%s.cof", courtNames[args[0] - 1]));

	Common::File file;
	if (!file.open(courtFileName))
		error("Could not open file '%s'", courtFileName.toString(Common::Path::kNativeSeparator).c_str());

	debug(0, "Loading court data from '%s'", courtFileName.toString(Common::Path::kNativeSeparator).c_str());

	// First, read in the header
	file.readUint32LE(); // Header size (?)

	char version[6];
	file.read(version, 5);
	version[5] = 0;

	if (strcmp(version, "01.05"))
		error("Invalid court version field: %s", version);

	uint32 objectCount = file.readUint32LE();

	for (uint32 i = 0; i < objectCount; i++) {
		char nameBuf[100];
		memset(nameBuf, 0, sizeof(nameBuf));

		uint32 nameLength = file.readUint32LE();
		assert(nameLength < sizeof(nameBuf) - 1);
		file.read(nameBuf, nameLength);

		CourtObject object;
		object.name = nameBuf;
		object.type = (CourtObjectType)file.readUint32LE();
		for (uint32 j = 0; j < 10; j++)
			object.data[j] = file.readUint32LE();

		debug(1, "Found court object '%s' - Type %d", nameBuf, object.type);

		// Store backboard object indices for later
		if (object.type == kObjectTypeBackboard) {
			if (object.data[7] + object.data[4] / 2 >= 6000)
				_backboardObjectRight = i;
			else
				_backboardObjectLeft = i;
		}

		_courtObjects.push_back(object);
	}

	// TODO: Some other variables are initialized with constants here

	return 1;
}

int LogicHEbasketball::op_1053() {
	_courtObjects.clear();
	// TODO: This also calls op_1065 with one argument (5)

	return 1;
}

LogicHE *makeLogicHEbasketball(ScummEngine_v90he *vm) {
	return new LogicHEbasketball(vm);
}

} // End of namespace Scumm
