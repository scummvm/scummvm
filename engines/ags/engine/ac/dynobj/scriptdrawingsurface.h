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

#ifndef AGS_ENGINE_AC_DYNOBJ_SCRIPTDRAWINGSURFACE_H
#define AGS_ENGINE_AC_DYNOBJ_SCRIPTDRAWINGSURFACE_H

#include "ags/engine/ac/dynobj/cc_agsdynamicobject.h"
#include "ags/shared/game/roomstruct.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Bitmap;
} // namespace Shared
} // namespace AGS

struct ScriptDrawingSurface final : AGSCCDynamicObject {
	// These numbers and types are used to determine the source of this drawing surface;
	// only one of them can be valid for this surface.
	int roomBackgroundNumber;
	RoomAreaMask roomMaskType;
	int dynamicSpriteNumber;
	int dynamicSurfaceNumber;
	bool isLinkedBitmapOnly;
	Shared::Bitmap *linkedBitmapOnly;
	int currentColour;
	int currentColourScript;
	int highResCoordinates;
	int modified;
	int hasAlphaChannel;
	//Shared::Bitmap* abufBackup;

	int Dispose(const char *address, bool force) override;
	const char *GetType() override;
	int Serialize(const char *address, char *buffer, int bufsize) override;
	void Unserialize(int index, const char *serializedData, int dataSize) override;
	Shared::Bitmap *GetBitmapSurface();
	Shared::Bitmap *StartDrawing();
	void PointToGameResolution(int *xcoord, int *ycoord);
	void SizeToGameResolution(int *width, int *height);
	void SizeToGameResolution(int *adjustValue);
	void SizeToDataResolution(int *adjustValue);
	void FinishedDrawing();
	void FinishedDrawingReadOnly();

	ScriptDrawingSurface();
};

} // namespace AGS3

#endif
