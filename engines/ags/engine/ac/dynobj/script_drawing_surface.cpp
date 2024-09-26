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

#include "ags/engine/ac/dynobj/script_drawing_surface.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/drawing_surface.h"
#include "ags/engine/ac/game_state.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/game/room_struct.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

Bitmap *ScriptDrawingSurface::GetBitmapSurface() {
	// TODO: consider creating weak_ptr here, and store one in the DrawingSurface!
	if (roomBackgroundNumber >= 0)
		return _GP(thisroom).BgFrames[roomBackgroundNumber].Graphic.get();
	else if (dynamicSpriteNumber >= 0)
		return _GP(spriteset)[dynamicSpriteNumber];
	else if (dynamicSurfaceNumber >= 0)
		return _G(dynamicallyCreatedSurfaces)[dynamicSurfaceNumber].get();
	else if (linkedBitmapOnly != nullptr)
		return linkedBitmapOnly;
	else if (roomMaskType > kRoomAreaNone)
		return _GP(thisroom).GetMask(roomMaskType);
	quit("!DrawingSurface: attempted to use surface after Release was called");
	return nullptr;
}

Bitmap *ScriptDrawingSurface::StartDrawing() {
	return this->GetBitmapSurface();
}

void ScriptDrawingSurface::FinishedDrawingReadOnly() {
}

void ScriptDrawingSurface::FinishedDrawing() {
	FinishedDrawingReadOnly();
	modified = 1;
}

int ScriptDrawingSurface::Dispose(void *address, bool force) {

	// dispose the drawing surface
	DrawingSurface_Release(this);
	delete this;
	return 1;
}

const char *ScriptDrawingSurface::GetType() {
	return "DrawingSurface";
}

size_t ScriptDrawingSurface::CalcSerializeSize(const void * /*address*/) {
	return sizeof(int32_t) * 9;
}

void ScriptDrawingSurface::Serialize(const void *address, Stream *out) {
	// pack mask type in the last byte of a negative integer
	// note: (-1) is reserved for "unused", for backward compatibility
	if (roomMaskType > 0)
		out->WriteInt32(0xFFFFFF00 | roomMaskType);
	else
		out->WriteInt32(roomBackgroundNumber);
	out->WriteInt32(dynamicSpriteNumber);
	out->WriteInt32(dynamicSurfaceNumber);
	out->WriteInt32(currentColour);
	out->WriteInt32(currentColourScript);
	out->WriteInt32(highResCoordinates);
	out->WriteInt32(modified);
	out->WriteInt32(hasAlphaChannel);
	out->WriteInt32(isLinkedBitmapOnly ? 1 : 0);
}

void ScriptDrawingSurface::Unserialize(int index, Stream *in, size_t data_sz) {
	int room_ds = in->ReadInt32();
	if (room_ds >= 0)
		roomBackgroundNumber = room_ds;
	// negative value may contain a mask type
	else if ((room_ds & 0xFF) != 0xFF)
		roomMaskType = (RoomAreaMask)(room_ds & 0xFF);
	dynamicSpriteNumber = in->ReadInt32();
	dynamicSurfaceNumber = in->ReadInt32();
	currentColour = in->ReadInt32();
	currentColourScript = in->ReadInt32();
	highResCoordinates = in->ReadInt32();
	modified = in->ReadInt32();
	hasAlphaChannel = in->ReadInt32();
	isLinkedBitmapOnly = (in->ReadInt32() != 0);
	ccRegisterUnserializedObject(index, this, this);
}

ScriptDrawingSurface::ScriptDrawingSurface() {
	roomBackgroundNumber = -1;
	roomMaskType = kRoomAreaNone;
	dynamicSpriteNumber = -1;
	dynamicSurfaceNumber = -1;
	isLinkedBitmapOnly = false;
	linkedBitmapOnly = nullptr;
	currentColour = _GP(play).raw_color;
	currentColourScript = 0;
	modified = 0;
	hasAlphaChannel = 0;
	highResCoordinates = 0;
	// NOTE: Normally in contemporary games coordinates ratio will always be 1:1.
	// But we still support legacy drawing, so have to set this up even for modern games,
	// otherwise we'd have to complicate conversion conditions further.
	if (_GP(game).IsLegacyHiRes() && _GP(game).IsDataInNativeCoordinates()) {
		highResCoordinates = 1;
	}
}

} // namespace AGS3
