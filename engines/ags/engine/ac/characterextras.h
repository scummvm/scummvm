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

#ifndef AGS_ENGINE_AC_CHARACTEREXTRAS_H
#define AGS_ENGINE_AC_CHARACTEREXTRAS_H

#include "ags/engine/ac/runtime_defines.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

struct CharacterExtras {
	// UGLY UGLY UGLY!! The CharacterInfo struct size is fixed because it's
	// used in the scripts, therefore overflowing stuff has to go here
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
	char  process_idle_this_time;
	char  slow_move_counter;
	short animwait;

	void ReadFromFile(Shared::Stream *in);
	void WriteToFile(Shared::Stream *out);
};

} // namespace AGS3

#endif
