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

#ifndef AGS_ENGINE_AC_DYNOBJ_SCRIPT_DRAWING_SURFACE_H
#define AGS_ENGINE_AC_DYNOBJ_SCRIPT_DRAWING_SURFACE_H

#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"
#include "ags/shared/game/room_struct.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

struct ScriptDrawingSurface final : AGSCCDynamicObject {
	// These numbers and types are used to determine the source of this drawing surface;
	// only one of them can be valid for this surface.
	int roomBackgroundNumber;
	RoomAreaMask roomMaskType;
	int dynamicSpriteNumber;
	int dynamicSurfaceNumber;
	bool isLinkedBitmapOnly;
	AGS::Shared::Bitmap *linkedBitmapOnly;
	int currentColour;
	int currentColourScript;
	int highResCoordinates;
	int modified;
	int hasAlphaChannel;
	//Shared::Bitmap* abufBackup;

	int Dispose(void *address, bool force) override;
	const char *GetType() override;
	void Unserialize(int index, AGS::Shared::Stream *in, size_t data_sz) override;
	AGS::Shared::Bitmap *GetBitmapSurface();
	AGS::Shared::Bitmap *StartDrawing();
	void PointToGameResolution(int *xcoord, int *ycoord);
	void SizeToGameResolution(int *width, int *height);
	void SizeToGameResolution(int *adjustValue);
	void SizeToDataResolution(int *adjustValue);
	void FinishedDrawing();
	void FinishedDrawingReadOnly();

	ScriptDrawingSurface();

protected:
	// Calculate and return required space for serialization, in bytes
	size_t CalcSerializeSize(const void *address) override;
	// Write object data into the provided stream
	void Serialize(const void *address, AGS::Shared::Stream *out) override;
};

} // namespace AGS3

#endif
