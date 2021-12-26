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
#include "ags/shared/util/stream.h"

namespace AGS3 {

using AGS::Shared::Stream;

void AnimatingGUIButton::ReadFromFile(Stream *in) {
	buttonid = in->ReadInt16();
	ongui = in->ReadInt16();
	onguibut = in->ReadInt16();
	view = in->ReadInt16();
	loop = in->ReadInt16();
	frame = in->ReadInt16();
	speed = in->ReadInt16();
	repeat = in->ReadInt16();
	wait = in->ReadInt16();
}

void AnimatingGUIButton::WriteToFile(Stream *out) {
	out->WriteInt16(buttonid);
	out->WriteInt16(ongui);
	out->WriteInt16(onguibut);
	out->WriteInt16(view);
	out->WriteInt16(loop);
	out->WriteInt16(frame);
	out->WriteInt16(speed);
	out->WriteInt16(repeat);
	out->WriteInt16(wait);
}

} // namespace AGS3
