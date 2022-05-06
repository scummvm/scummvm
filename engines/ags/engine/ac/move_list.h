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

#ifndef AGS_ENGINE_AC_MOVE_H
#define AGS_ENGINE_AC_MOVE_H

#include "ags/lib/allegro.h" // fixed math
#include "ags/engine/game/savegame.h"

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

struct MoveList {
	int32_t pos[MAXNEEDSTAGES] = {};
	int   numstage = 0;
	fixed xpermove[MAXNEEDSTAGES] = {}, ypermove[MAXNEEDSTAGES] = {};
	int   fromx = 0, fromy = 0;
	int   onstage = 0, onpart = 0;
	int   lastx = 0, lasty = 0;
	int8  doneflag = 0;
	int8  direct = 0;  // MoveCharDirect was used or not

	void ReadFromFile_Legacy(Shared::Stream *in);
	AGS::Engine::HSaveError ReadFromFile(Shared::Stream *in, int32_t cmp_ver);
	void WriteToFile(Shared::Stream *out);
};

} // namespace AGS3

#endif
