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

#ifndef AGS_ENGINE_AC_ROOMOBJECT_H
#define AGS_ENGINE_AC_ROOMOBJECT_H

#include "ac/common_defines.h"

namespace AGS3 {

namespace AGS {
namespace Common {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

// IMPORTANT: this struct is restricted by plugin API!
struct RoomObject {
	int   x, y;
	int   transparent;    // current transparency setting
	short tint_r, tint_g;   // specific object tint
	short tint_b, tint_level;
	short tint_light;
	short zoom;           // zoom level, either manual or from the current area
	short last_width, last_height;   // width/height last time drawn
	short num;            // sprite slot number
	short baseline;       // <=0 to use Y co-ordinate; >0 for specific baseline
	short view, loop, frame; // only used to track animation - 'num' holds the current sprite
	short wait, moving;
	char  cycling;        // is it currently animating?
	char  overall_speed;
	char  on;
	char  flags;
	short blocking_width, blocking_height;

	RoomObject();

	int get_width();
	int get_height();
	int get_baseline();

	inline bool has_explicit_light() const {
		return (flags & OBJF_HASLIGHT) != 0;
	}
	inline bool has_explicit_tint()  const {
		return (flags & OBJF_HASTINT) != 0;
	}

	void UpdateCyclingView();
	void update_cycle_view_forwards();
	void update_cycle_view_backwards();

	void ReadFromFile(Common::Stream *in);
	void WriteToFile(Common::Stream *out) const;
};

} // namespace AGS3

#endif
