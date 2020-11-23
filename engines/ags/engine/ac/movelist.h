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

#ifndef AGS_ENGINE_AC_MOVE_H
#define AGS_ENGINE_AC_MOVE_H

#include "ags/shared/util/wgt2allg.h" // fixed type
#include "ags/engine/game/savegame.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS
using namespace AGS; // FIXME later

#define MAXNEEDSTAGES 256
#define MAXNEEDSTAGES_LEGACY 40

struct MoveList {
	int   pos[MAXNEEDSTAGES];
	int   numstage;
	fixed xpermove[MAXNEEDSTAGES], ypermove[MAXNEEDSTAGES];
	int   fromx, fromy;
	int   onstage, onpart;
	int   lastx, lasty;
	char  doneflag;
	char  direct;  // MoveCharDirect was used or not

	void ReadFromFile_Legacy(Shared::Stream *in);
	AGS::Engine::HSaveError ReadFromFile(Shared::Stream *in, int32_t cmp_ver);
	void WriteToFile(Shared::Stream *out);
};

} // namespace AGS3

#endif
