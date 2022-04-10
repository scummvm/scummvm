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

#ifndef AGS_ENGINE_AC_CHARACTER_EXTRAS_H
#define AGS_ENGINE_AC_CHARACTER_EXTRAS_H

#include "ags/engine/ac/runtime_defines.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Stream;
}
}
using namespace AGS; // FIXME later

// The CharacterInfo struct size is fixed because it's exposed to script
// and plugin API, therefore new stuff has to go here
struct CharacterExtras {
	short invorder[MAX_INVORDER];
	short invorder_count;
	// TODO: implement full AABB and keep updated, so that engine could rely on these cached values all time;
	// TODO: consider having both fixed AABB and volatile one that changes with animation frame (unless you change how anims work)
	short width;
	short height;
	short zoom;
	short xwas;
	short ywas;
	short tint_r;
	short tint_g;
	short tint_b;
	short tint_level;
	short tint_light;
	int8  process_idle_this_time;
	int8  slow_move_counter;
	short animwait;

	void ReadFromFile(Shared::Stream *in);
	void WriteToFile(Shared::Stream *out);
};

} // namespace AGS3

#endif
