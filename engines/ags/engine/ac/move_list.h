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

#ifndef AGS_ENGINE_AC_MOVE_LIST_H
#define AGS_ENGINE_AC_MOVE_LIST_H

#include "ags/lib/allegro.h" // fixed math
#include "ags/engine/game/savegame.h"
#include "ags/shared/util/geometry.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Stream;
}
}
using namespace AGS; // FIXME later

#define MAXNEEDSTAGES 256
#define MAXNEEDSTAGES_LEGACY 40

enum MoveListDoneFlags {
	kMoveListDone_X = 0x01,
	kMoveListDone_Y = 0x02,
	kMoveListDone_XY = kMoveListDone_X | kMoveListDone_Y
};

struct MoveList {
	int 	numstage = 0;
	// Waypoints, per stage
	Point 	pos[MAXNEEDSTAGES];
	// xpermove and ypermove contain number of pixels done per a single step
	// along x and y axes; i.e. this is a movement vector, per path stage
	fixed 	xpermove[MAXNEEDSTAGES]{};
	fixed 	ypermove[MAXNEEDSTAGES]{};
	int 	onstage = 0; // current path stage
	Point from;  // current stage's starting position
	// Steps made during current stage;
	// distance passed is calculated as xpermove[onstage] * onpart;
	// made a fractional value to let recalculate movelist dynamically
	fixed onpart = 0;
	uint8_t doneflag = 0u;
	uint8_t direct = 0; // MoveCharDirect was used or not

	// Gets a movelist's step length, in coordinate units
	// (normally the coord unit is a game pixel)
	float GetStepLength() const;

	void ReadFromFile_Legacy(Shared::Stream *in);
	AGS::Engine::HSaveError ReadFromFile(Shared::Stream *in, int32_t cmp_ver);
	void WriteToFile(Shared::Stream *out);
};

} // namespace AGS3

#endif
