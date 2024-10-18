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
// Contains an id of a sprite, which may be either owned by overlay, or shared
// to whole game similar to how other objects use sprites.
// May logically exist either on UI or room layer.
// TODO: historically overlay objects contained an actual bitmap in them.
// This was remade into having a dynamic sprite allocated exclusively for
// overlay. But sprites do not have any kind of a ref count of their own
// (unless exported into script as DynamicSprite), so we have to keep an
// overlay's flag, which tells that the sprite it references must be deleted
// on overlay's disposal. This should be improved at some point, by devising
// a better kind of a sprite's ownership mechanic.

#ifndef AGS_ENGINE_AC_SCREEN_OVERLAY_H
#define AGS_ENGINE_AC_SCREEN_OVERLAY_H

#include "common/std/memory.h"
#include "ags/shared/core/types.h"
#include "ags/shared/util/geometry.h"

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
	kOver_SpriteShared = 0x0008   // reference shared sprite (as opposed to exclusive)
};

enum OverlaySvgVersion {
	kOverSvgVersion_Initial = 0,
	kOverSvgVersion_35028 	= 1, // offset x,y
	kOverSvgVersion_36008 	= 2, // z, transparency
	kOverSvgVersion_36025 	= 3, // merged options into flags
	kOverSvgVersion_36108 	= 4, // don't save owned sprites (use dynamic sprites)
};

struct ScreenOverlay {
	// Overlay's "type" is a merged special overlay ID and internal container index
	int type = -1;
	// Arbitrary creation order index, meant for resolving equal z-sorting
	int creation_id = 0;
	int timeout = 0;
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

	ScreenOverlay() = default;
	ScreenOverlay(ScreenOverlay &&);
	~ScreenOverlay();
	ScreenOverlay &operator=(ScreenOverlay &&);

	// FIXME: These are private in the upstream repo,
	// but Windows CI complains
	ScreenOverlay(const ScreenOverlay &) = default;
	ScreenOverlay &operator=(const ScreenOverlay &) = default;

	bool HasAlphaChannel() const {
		return (_flags & kOver_AlphaChannel) != 0;
	}
	bool IsSpriteShared() const {
		return (_flags & kOver_SpriteShared) != 0;
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
	// Get sprite reference id, or 0 if none set
	int GetSpriteNum() const {
		return _sprnum;
	}
	Size GetGraphicSize() const;
	// Assigns an exclusive image to this overlay; the image will be stored as a dynamic sprite
    // in a sprite cache, but owned by this overlay and therefore disposed at its disposal
	void SetImage(std::unique_ptr<Shared::Bitmap> pic, bool has_alpha = false, int offx = 0, int offy = 0);
	// Assigns a shared sprite to this overlay
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

	void ReadFromSavegame(Shared::Stream *in, bool &has_bitmap, int32_t cmp_ver);
	void WriteToSavegame(Shared::Stream *out) const;

private:
	void ResetImage();

	int _flags = 0;  // OverlayFlags
	int _sprnum = 0; // sprite id
	bool _hasChanged = false;
};

} // namespace AGS3

#endif
