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

#include "ags/engine/ac/screen_overlay.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

using AGS::Shared::Stream;

void ScreenOverlay::ReadFromFile(Stream *in, int32_t cmp_ver) {
	// Skipping bmp and pic pointer values
	// TODO: find out if it's safe to just drop these pointers!! replace with unique_ptr?
	bmp = nullptr;
	pic = nullptr;
	in->ReadInt32(); // bmp
	hasSerializedBitmap = in->ReadInt32() != 0;
	type = in->ReadInt32();
	x = in->ReadInt32();
	y = in->ReadInt32();
	timeout = in->ReadInt32();
	bgSpeechForChar = in->ReadInt32();
	associatedOverlayHandle = in->ReadInt32();
	hasAlphaChannel = in->ReadBool();
	positionRelativeToScreen = in->ReadBool();
	if (cmp_ver >= 1) {
		_offsetX = in->ReadInt32();
		_offsetY = in->ReadInt32();
	}
	if (cmp_ver >= 2) {
		zorder = in->ReadInt32();
		transparency = in->ReadInt32();
		in->ReadInt32(); // reserve 2 ints
		in->ReadInt32();
	}
}

void ScreenOverlay::WriteToFile(Stream *out) const {
	// Writing bitmap "pointers" to correspond to full structure writing
	out->WriteInt32(0); // bmp
	out->WriteInt32(pic ? 1 : 0); // pic
	out->WriteInt32(type);
	out->WriteInt32(x);
	out->WriteInt32(y);
	out->WriteInt32(timeout);
	out->WriteInt32(bgSpeechForChar);
	out->WriteInt32(associatedOverlayHandle);
	out->WriteBool(hasAlphaChannel);
	out->WriteBool(positionRelativeToScreen);
	// since cmp_ver = 1
	out->WriteInt32(_offsetX);
	out->WriteInt32(_offsetY);
	// since cmp_ver = 2
	out->WriteInt32(zorder);
	out->WriteInt32(transparency);
	out->WriteInt32(0); // reserve 2 ints
	out->WriteInt32(0);
}

} // namespace AGS3
