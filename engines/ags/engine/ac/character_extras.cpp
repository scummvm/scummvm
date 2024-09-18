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

#include "ags/engine/ac/character_extras.h"
#include "ags/engine/ac/view_frame.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_utils.h"

namespace AGS3 {

using namespace AGS::Shared;

int CharacterExtras::GetEffectiveY(CharacterInfo *chi) const {
	return chi->y - (chi->z * zoom_offs) / 100;
}

int CharacterExtras::GetFrameSoundVolume(CharacterInfo *chi) const {
	return AGS3::CalcFrameSoundVolume(
		anim_volume, cur_anim_volume,
		(chi->flags & CHF_SCALEVOLUME) ? zoom : 100);
}

void CharacterExtras::CheckViewFrame(CharacterInfo *chi) {
	AGS3::CheckViewFrame(chi->view, chi->loop, chi->frame, GetFrameSoundVolume(chi));
}

void CharacterExtras::ReadFromSavegame(Stream *in, CharacterSvgVersion save_ver) {
	in->ReadArrayOfInt16(invorder, MAX_INVORDER);
	invorder_count = in->ReadInt16();
	width = in->ReadInt16();
	height = in->ReadInt16();
	zoom = in->ReadInt16();
	xwas = in->ReadInt16();
	ywas = in->ReadInt16();
	tint_r = in->ReadInt16();
	tint_g = in->ReadInt16();
	tint_b = in->ReadInt16();
	tint_level = in->ReadInt16();
	tint_light = in->ReadInt16();
	process_idle_this_time = in->ReadInt8();
	slow_move_counter = in->ReadInt8();
	animwait = in->ReadInt16();
	if (save_ver >= kCharSvgVersion_36025) {
		anim_volume = static_cast<uint8_t>(in->ReadInt8());
		cur_anim_volume = static_cast<uint8_t>(in->ReadInt8());
		in->ReadInt8(); // reserved to fill int32
		in->ReadInt8();
	}
}

void CharacterExtras::WriteToSavegame(Stream *out) const {
	out->WriteArrayOfInt16(invorder, MAX_INVORDER);
	out->WriteInt16(invorder_count);
	out->WriteInt16(width);
	out->WriteInt16(height);
	out->WriteInt16(zoom);
	out->WriteInt16(xwas);
	out->WriteInt16(ywas);
	out->WriteInt16(tint_r);
	out->WriteInt16(tint_g);
	out->WriteInt16(tint_b);
	out->WriteInt16(tint_level);
	out->WriteInt16(tint_light);
	out->WriteInt8(process_idle_this_time);
	out->WriteInt8(slow_move_counter);
	out->WriteInt16(animwait);
	out->WriteInt8(static_cast<uint8_t>(anim_volume));
	out->WriteInt8(static_cast<uint8_t>(cur_anim_volume));
	out->WriteInt8(0); // reserved to fill int32
	out->WriteInt8(0);
}

} // namespace AGS3
