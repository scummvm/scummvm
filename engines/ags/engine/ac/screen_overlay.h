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

 // ScreenOverlay is a simple sprite container with no advanced functions.
// May contain owned bitmap or reference persistent sprite's id, similar to how
// other game objects do that.
// May logically exist either on UI or room layer.

#ifndef AGS_ENGINE_AC_SCREEN_OVERLAY_H
#define AGS_ENGINE_AC_SCREEN_OVERLAY_H

#include "common/std/memory.h"
#include "ags/shared/core/types.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Bitmap;
class Stream;
} // namespace Shared
} // namespace AGS

namespace AGS {
namespace Engine {
class IDriverDependantBitmap;
} // namespace Engine
} // namespace AGS

using namespace AGS; // FIXME later

enum OverlayFlags {
	kOver_AlphaChannel = 0x0001,
	kOver_PositionAtRoomXY = 0x0002, // room-relative position, may be in ui
	kOver_RoomLayer = 0x0004,        // work in room layer (as opposed to UI)
	kOver_SpriteReference = 0x0008   // reference persistent sprite
};

struct ScreenOverlay {
	// Texture
	Engine::IDriverDependantBitmap *ddb = nullptr;
	int type = -1, timeout = 0;
	// Note that x,y are overlay's properties, that define its position in script;
	// but real drawn position is x + offsetX, y + offsetY;
	int x = 0, y = 0;
	// Border/padding offset for the tiled text windows
	int offsetX = 0, offsetY = 0;
	// Width and height to stretch the texture to
	int scaleWidth = 0, scaleHeight = 0;
	int bgSpeechForChar = -1;
	int associatedOverlayHandle = 0; // script obj handle
	int zorder = INT_MIN;
	int transparency = 0;

	bool HasAlphaChannel() const {
		return (_flags & kOver_AlphaChannel) != 0;
	}
	bool IsSpriteReference() const {
		return (_flags & kOver_SpriteReference) != 0;
	}
	bool IsRoomRelative() const {
		return (_flags & kOver_PositionAtRoomXY) != 0;
	}
	bool IsRoomLayer() const {
		return (_flags & kOver_RoomLayer) != 0;
	}
	void SetAlphaChannel(bool on) {
		on ? _flags |= kOver_AlphaChannel : _flags &= ~kOver_AlphaChannel;
	}
	void SetRoomRelative(bool on) {
		on ? _flags |= kOver_PositionAtRoomXY : _flags &= ~kOver_PositionAtRoomXY;
	}
	void SetRoomLayer(bool on) {
		on ? _flags |= (kOver_RoomLayer | kOver_PositionAtRoomXY) :
			_flags &= ~(kOver_RoomLayer | kOver_PositionAtRoomXY);
	}
	// Gets actual overlay's image, whether owned by overlay or by a sprite reference
	Shared::Bitmap *GetImage() const;
	// Get sprite reference id, or -1 if none set
	int GetSpriteNum() const {
		return _sprnum;
	}
	void SetImage(Shared::Bitmap *pic, int offx = 0, int offy = 0);
	void SetSpriteNum(int sprnum, int offx = 0, int offy = 0);
	// Tells if Overlay has graphically changed recently
	bool HasChanged() const {
		return _hasChanged;
	}
	// Manually marks GUI as graphically changed
	void MarkChanged() {
		_hasChanged = true;
	}
	// Clears changed flag
	void ClearChanged() {
		_hasChanged = false;
	}

	void ReadFromFile(Shared::Stream *in, bool &has_bitmap, int32_t cmp_ver);
	void WriteToFile(Shared::Stream *out) const;

private:
	int _flags = 0; // OverlayFlags
	bool _hasChanged = false;
	std::shared_ptr<Shared::Bitmap> _pic; // owned bitmap
	int _sprnum = -1; // sprite reference
};

} // namespace AGS3

#endif
