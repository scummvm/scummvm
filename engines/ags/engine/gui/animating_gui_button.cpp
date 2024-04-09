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

#include "ags/engine/gui/animating_gui_button.h"
#include "ags/shared/gui/gui_defines.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

using namespace AGS::Shared;

void AnimatingGUIButton::ReadFromSavegame(Stream *in, int cmp_ver) {
	buttonid = in->ReadInt16();
	ongui = in->ReadInt16();
	onguibut = in->ReadInt16();
	view = in->ReadInt16();
	loop = in->ReadInt16();
	frame = in->ReadInt16();
	speed = in->ReadInt16();
	uint16_t anim_flags = in->ReadInt16(); // was repeat (0,1)
	wait = in->ReadInt16();

	if (cmp_ver < kGuiSvgVersion_36020) anim_flags &= 0x1; // restrict to repeat only
	repeat = (anim_flags & 0x1) ? ANIM_REPEAT : ANIM_ONCE;
	blocking = (anim_flags >> 1) & 0x1;
	direction = (anim_flags >> 2) & 0x1;

	if (cmp_ver >= kGuiSvgVersion_36025) {
		volume = in->ReadInt8();
		in->ReadInt8(); // reserved to fill int32
		in->ReadInt8();
		in->ReadInt8();
	}
}

void AnimatingGUIButton::WriteToSavegame(Stream *out) {
	uint16_t anim_flags =
		(repeat & 0x1) | // either ANIM_ONCE or ANIM_REPEAT
		(blocking & 0x1) << 1 |
		(direction & 0x1) << 2;

	out->WriteInt16(buttonid);
	out->WriteInt16(ongui);
	out->WriteInt16(onguibut);
	out->WriteInt16(view);
	out->WriteInt16(loop);
	out->WriteInt16(frame);
	out->WriteInt16(speed);
	out->WriteInt16(anim_flags); // was repeat (0,1)
	out->WriteInt16(wait);
	out->WriteInt8(static_cast<uint8_t>(volume));
	out->WriteInt8(0); // reserved to fill int32
	out->WriteInt8(0);
	out->WriteInt8(0);
}

} // namespace AGS3
