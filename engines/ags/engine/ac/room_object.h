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
// Runtime room object definition.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_ROOM_OBJECT_H
#define AGS_ENGINE_AC_ROOM_OBJECT_H

#include "ags/shared/core/types.h"
#include "ags/shared/ac/common_defines.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

// RoomObject's internal values, packed in RoomObject::cycling
// Animates once and stops at the *last* frame
#define OBJANIM_ONCE      (ANIM_ONCE + 1)
// Animates infinitely until stopped by command
#define OBJANIM_REPEAT    (ANIM_REPEAT + 1)
// Animates once and stops, resetting to the very first frame
#define OBJANIM_ONCERESET (ANIM_ONCERESET + 1)
// Animates backwards, as opposed to forwards
#define OBJANIM_BACKWARDS 10

// IMPORTANT: exposed to plugin API as AGSObject!
// keep that in mind if extending this struct, and dont change existing fields
// unless you plan on adjusting plugin API as well.
struct RoomObject {
	static const uint16_t NoView = UINT16_MAX;

	int   x, y;
	int   transparent;    // current transparency setting
	short tint_r, tint_g;   // specific object tint
	short tint_b, tint_level;
	short tint_light;
	short zoom;           // zoom level, either manual or from the current area
	short last_width, last_height;   // width/height based on a scaled sprite
	uint16_t num;            // sprite slot number
	short baseline;       // <=0 to use Y co-ordinate; >0 for specific baseline
	uint16_t view, loop, frame; // only used to track animation - 'num' holds the current sprite
	short wait, moving;
	int8  cycling;        // stores OBJANIM_* flags and values
	int8  overall_speed;  // animation delay
	int8  on;
	int8  flags;
	// Down to here is a part of the plugin API
	short blocking_width, blocking_height;
	int	  anim_volume = 100;     // default animation volume (relative factor)
	int   cur_anim_volume = 100; // current animation sound volume (relative factor)
	Shared::String name;

	RoomObject();

	int get_width() const;
	int get_height() const;
	int get_baseline() const;

	inline bool has_explicit_light() const {
		return (flags & OBJF_HASLIGHT) != 0;
	}
	inline bool has_explicit_tint()  const {
		return (flags & OBJF_HASTINT) != 0;
	}

	inline bool is_animating() const {
		return (cycling > 0);
	}
	// repeat may be ANIM_ONCE, ANIM_REPEAT, ANIM_ONCERESET;
	// get_anim_repeat() converts from OBJANIM_* to ANIM_* values
	inline int get_anim_repeat() const {
		return (cycling % OBJANIM_BACKWARDS) - 1;
	}
	inline bool get_anim_forwards() const {
		return (cycling < OBJANIM_BACKWARDS);
	}
	inline int get_anim_delay() const {
		return overall_speed;
	}
	// repeat may be ANIM_ONCE, ANIM_REPEAT, ANIM_ONCERESET
	inline void set_animating(int repeat, bool forwards, int delay) {
		// convert "repeat" to 1-based OBJANIM_* flag
		cycling = (repeat + 1) + (!forwards * OBJANIM_BACKWARDS);
		overall_speed = delay;
	}

	void UpdateCyclingView(int ref_id);
	// Calculate wanted frame sound volume based on multiple factors
	int  GetFrameSoundVolume() const;
	// Process the current animation frame for the room object:
	// play linked sounds, and so forth.
	void CheckViewFrame();

	void ReadFromSavegame(Shared::Stream *in, int save_ver);
	void WriteToSavegame(Shared::Stream *out) const;
};

} // namespace AGS3

#endif
