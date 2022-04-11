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

#include "ags/engine/ac/room_object.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/common_defines.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/view_frame.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/main/update.h"
#include "ags/shared/util/math.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_utils.h"

namespace AGS3 {

using namespace AGS::Shared;

RoomObject::RoomObject() {
	x = y = 0;
	transparent = 0;
	tint_r = tint_g = 0;
	tint_b = tint_level = 0;
	tint_light = 0;
	zoom = 0;
	last_width = last_height = 0;
	num = 0;
	baseline = 0;
	view = loop = frame = 0;
	wait = moving = 0;
	cycling = 0;
	overall_speed = 0;
	on = 0;
	flags = 0;
	blocking_width = blocking_height = 0;
}

int RoomObject::get_width() {
	if (last_width == 0)
		return _GP(game).SpriteInfos[num].Width;
	return last_width;
}
int RoomObject::get_height() {
	if (last_height == 0)
		return _GP(game).SpriteInfos[num].Height;
	return last_height;
}
int RoomObject::get_baseline() {
	if (baseline < 1)
		return y;
	return baseline;
}

void RoomObject::UpdateCyclingView(int ref_id) {
	if (on != 1) return;
	if (moving > 0) {
		do_movelist_move(&moving, &x, &y);
	}
	if (cycling == 0) return;
	if (view == (uint16_t)-1) return;
	if (wait > 0) {
		wait--;
		return;
	}

	if (cycling >= ANIM_BACKWARDS) {

		update_cycle_view_backwards();

	} else {  // Animate forwards

		update_cycle_view_forwards();

	}  // end if forwards

	ViewFrame *vfptr = &_GP(views)[view].loops[loop].frames[frame];
	if (vfptr->pic > UINT16_MAX)
		debug_script_warn("Warning: object's (id %d) sprite %d is outside of internal range (%d), reset to 0",
		                  ref_id, vfptr->pic, UINT16_MAX);
	num = Math::InRangeOrDef<uint16_t>(vfptr->pic, 0);

	if (cycling == 0)
		return;

	wait = vfptr->speed + overall_speed;
	CheckViewFrame(view, loop, frame);
}


void RoomObject::update_cycle_view_forwards() {
	frame++;
	if (frame >= _GP(views)[view].loops[loop].numFrames) {
		// go to next loop thing
		if (_GP(views)[view].loops[loop].RunNextLoop()) {
			if (loop + 1 >= _GP(views)[view].numLoops)
				quit("!Last loop in a view requested to move to next loop");
			loop++;
			frame = 0;
		} else if (cycling % ANIM_BACKWARDS == ANIM_ONCE) {
			// leave it on the last frame
			cycling = 0;
			frame--;
		} else {
			if (_GP(play).no_multiloop_repeat == 0) {
				// multi-loop anims, go back to start of it
				while ((loop > 0) &&
				        (_GP(views)[view].loops[loop - 1].RunNextLoop()))
					loop--;
			}
			if (cycling % ANIM_BACKWARDS == ANIM_ONCERESET)
				cycling = 0;
			frame = 0;
		}
	}
}

void RoomObject::update_cycle_view_backwards() {
	// animate backwards
	if (frame > 0) {
		frame--;
	} else {
		if ((loop > 0) &&
		        (_GP(views)[view].loops[loop - 1].RunNextLoop())) {
			// If it's a Go-to-next-loop on the previous one, then go back
			loop--;
			frame = _GP(views)[view].loops[loop].numFrames - 1;
		} else if (cycling % ANIM_BACKWARDS == ANIM_ONCE) {
			// leave it on the first frame
			cycling = 0;
			frame = 0;
		} else { // repeating animation
			frame = _GP(views)[view].loops[loop].numFrames - 1;
		}
	}
}

void RoomObject::ReadFromSavegame(Stream *in, int save_ver) {
	x = in->ReadInt32();
	y = in->ReadInt32();
	transparent = in->ReadInt32();
	tint_r = in->ReadInt16();
	tint_g = in->ReadInt16();
	tint_b = in->ReadInt16();
	tint_level = in->ReadInt16();
	tint_light = in->ReadInt16();
	zoom = in->ReadInt16();
	last_width = in->ReadInt16();
	last_height = in->ReadInt16();
	num = in->ReadInt16();
	baseline = in->ReadInt16();
	view = in->ReadInt16();
	loop = in->ReadInt16();
	frame = in->ReadInt16();
	wait = in->ReadInt16();
	moving = in->ReadInt16();
	cycling = in->ReadInt8();
	overall_speed = in->ReadInt8();
	on = in->ReadInt8();
	flags = in->ReadInt8();
	blocking_width = in->ReadInt16();
	blocking_height = in->ReadInt16();
	if (save_ver > 0) {
		name = StrUtil::ReadString(in);
	}
}

void RoomObject::WriteToSavegame(Stream *out) const {
	out->WriteInt32(x);
	out->WriteInt32(y);
	out->WriteInt32(transparent);
	out->WriteInt16(tint_r);
	out->WriteInt16(tint_g);
	out->WriteInt16(tint_b);
	out->WriteInt16(tint_level);
	out->WriteInt16(tint_light);
	out->WriteInt16(zoom);
	out->WriteInt16(last_width);
	out->WriteInt16(last_height);
	out->WriteInt16(num);
	out->WriteInt16(baseline);
	out->WriteInt16(view);
	out->WriteInt16(loop);
	out->WriteInt16(frame);
	out->WriteInt16(wait);
	out->WriteInt16(moving);
	out->WriteInt8(cycling);
	out->WriteInt8(overall_speed);
	out->WriteInt8(on);
	out->WriteInt8(flags);
	out->WriteInt16(blocking_width);
	out->WriteInt16(blocking_height);
	StrUtil::WriteString(name, out);
}

} // namespace AGS3
