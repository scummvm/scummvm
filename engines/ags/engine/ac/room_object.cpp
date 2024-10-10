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
#include "ags/engine/ac/object.h"
#include "ags/engine/ac/room_status.h"
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

int RoomObject::get_width() const {
	if (last_width == 0)
		return _GP(game).SpriteInfos[num].Width;
	return last_width;
}
int RoomObject::get_height() const {
	if (last_height == 0)
		return _GP(game).SpriteInfos[num].Height;
	return last_height;
}
int RoomObject::get_baseline() const {
	if (baseline < 1)
		return y;
	return baseline;
}

void RoomObject::UpdateCyclingView(int ref_id) {
	if (on != 1) return;
	if (moving > 0) {
		do_movelist_move(moving, x, y);
	}
	if (cycling == 0) return;
	if (view == RoomObject::NoView) return;
	if (wait > 0) {
		wait--; return;
	}

	if (!CycleViewAnim(view, loop, frame, get_anim_forwards(), get_anim_repeat()))
		cycling = 0; // finished animating

	ViewFrame *vfptr = &_GP(views)[view].loops[loop].frames[frame];
	if (vfptr->pic > UINT16_MAX)
		debug_script_warn("Warning: object's (id %d) sprite %d is outside of internal range (%d), reset to 0",
			ref_id, vfptr->pic, UINT16_MAX);
	num = Math::InRangeOrDef<uint16_t>(vfptr->pic, 0);

	if (cycling == 0)
		return;

	wait = vfptr->speed + overall_speed;
	CheckViewFrame();
}

// Calculate wanted frame sound volume based on multiple factors
int RoomObject::GetFrameSoundVolume() const {
	// NOTE: room objects don't have "scale volume" flag at the moment
	return AGS3::CalcFrameSoundVolume(anim_volume, cur_anim_volume);
}

void RoomObject::CheckViewFrame() {
	AGS3::CheckViewFrame(view, loop, frame, GetFrameSoundVolume());
}

void RoomObject::ReadFromSavegame(Stream *in, int save_ver) {
	const bool do_align_pad = save_ver < 0;
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
	if (do_align_pad)
		in->ReadInt16(); // int16 padding to int32 (max)

	if (save_ver >= kRoomStatSvgVersion_36016) {
		name = StrUtil::ReadString(in);
	}
	if (save_ver >= kRoomStatSvgVersion_36025) {
		// anim vols order inverted compared to character, by mistake :(
		cur_anim_volume = static_cast<uint8_t>(in->ReadInt8());
		anim_volume = static_cast<uint8_t>(in->ReadInt8());
		in->ReadInt8(); // reserved to fill int32
		in->ReadInt8();

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
	out->WriteInt8(static_cast<uint8_t>(cur_anim_volume));
	out->WriteInt8(static_cast<uint8_t>(anim_volume));
	out->WriteInt8(0); // reserved to fill int32
	out->WriteInt8(0);
}

} // namespace AGS3
