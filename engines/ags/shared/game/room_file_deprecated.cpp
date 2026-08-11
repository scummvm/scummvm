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

//=============================================================================
//
// Deprecated room stuff. Removed from room class and load routine because this
// data is no longer supported in the engine. Perhaps move to some legacy
// knowledge base; or restore when it's possible to support ancient games.
//
//=============================================================================

#if defined (OBSOLETE)

#include "ags/shared/ac/common.h"
#include "ags/shared/util/stream.h"

using namespace AGS::Shared;

#define AE_WAITFLAG   0x80000000
#define MAXANIMSTAGES 10
struct AnimationStruct {
	int   x, y;
	int   data;
	int   object;
	int   speed;
	int8  action;
	int8  wait;
	AnimationStruct() {
		action = 0;
		object = 0;
		wait = 1;
		speed = 5;
	}
};

struct FullAnimation {
	AnimationStruct stage[MAXANIMSTAGES];
	int             numstages;
	FullAnimation() {
		numstages = 0;
	}
};

#define MAXPOINTS 30
struct PolyPoints {
	int x[MAXPOINTS];
	int y[MAXPOINTS];
	int numpoints;
	void add_point(int x, int y);
	PolyPoints() {
		numpoints = 0;
	}

	void Read(AGS::Shared::Stream *in);
};


#define MAXANIMS 10
// Just a list of cut out data
struct DeprecatedRoomStruct {
	// Full-room animations
	int16_t       numanims;
	FullAnimation anims[MAXANIMS];
	// Polygonal walkable areas (unknown version)
	int32_t       numwalkareas;
	PolyPoints    wallpoints[MAX_WALK_AREAS];
	// Unknown flags
	int16_t       flagstates[MAX_LEGACY_ROOM_FLAGS];
};



void PolyPoints::add_point(int x, int y) {
	x[numpoints] = x;
	y[numpoints] = y;
	numpoints++;

	if (numpoints >= MAXPOINTS)
		quit("too many poly points added");
}

void PolyPoints::Read(Stream *in) {
	in->ReadArrayOfInt32(x, MAXPOINTS);
	in->ReadArrayOfInt32(y, MAXPOINTS);
	numpoints = in->ReadInt32();
}


//
// Pre-2.5 scripts (we don't know how to convert them for the modern engine)
//
#define SCRIPT_CONFIG_VERSION 1
HRoomFileError ReadAncientScriptConfig(Stream *in) {
	int fmt = in->ReadInt32();
	if (fmt != SCRIPT_CONFIG_VERSION)
		return new RoomFileError(kRoomFileErr_FormatNotSupported, String::FromFormat("Invalid script configuration format (in room: %d, expected: %d).", fmt, SCRIPT_CONFIG_VERSION));

	size_t var_count = in->ReadInt32();
	for (size_t i = 0; i < var_count; ++i) {
		size_t len = in->ReadInt8();
		in->Seek(len);
	}
	return HRoomFileError::None();
}

HRoomFileError ReadAncientGraphicalScripts(Stream *in) {
	do {
		int ct = in->ReadInt32();
		if (ct == -1 || in->EOS())
			break;
		size_t len = in->ReadInt32();
		in->Seek(len);
	} while (true);
	return HRoomFileError::None();
}

HRoomFileError ReadPre250Scripts(Stream *in) {
	HRoomFileError err = ReadAncientScriptConfig(in);
	if (err)
		err = ReadAncientGraphicalScripts(in);
	return err;
}

#endif // OBSOLETE
