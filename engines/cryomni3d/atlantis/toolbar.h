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

#ifndef CRYOMNI3D_ATLANTIS_TOOLBAR_H
#define CRYOMNI3D_ATLANTIS_TOOLBAR_H

#include "common/array.h"
#include "common/rect.h"
#include "graphics/managed_surface.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "cryomni3d/objects.h"
#include "cryomni3d/sprites.h"

namespace CryOmni3D {

namespace Atlantis {

class CryOmni3DEngine_Atlantis;

class Atlantis_Toolbar {
public:
	Atlantis_Toolbar();
	~Atlantis_Toolbar();

	void init(const Sprites *sprites, Inventory *inventory, CryOmni3DEngine_Atlantis *engine);
	void addMenuSprite(uint16 w, uint16 h, int16 xoff, int16 yoff, uint16 transpKey,
	                   int tx, int ty, const uint16 *pix, const uint8 *blend);

	bool displayToolbar(const Graphics::Surface *original);
	void inventoryChanged(uint newPosition);
	uint inventoryOffset() const { return _inventoryOffset; }
	void setInventoryOffset(uint offset) { _inventoryOffset = offset; }

private:
	typedef uint(Atlantis_Toolbar::*ZoneCallback)(uint dragStatus);

	struct Zone {
		Common::Rect rect;
		ZoneCallback callback;
		bool secondary;  // true = inactive / grayed
		bool hidden;
	};

	Common::Array<Zone> _zones;
	const Sprites *_sprites;
	Inventory *_inventory;
	CryOmni3DEngine_Atlantis *_engine;

	Graphics::PixelFormat _format;

	uint _inventoryOffset;
	uint _inventoryMaxOffset;
	uint _inventoryHovered;
	uint _inventorySelected;
	bool _inventoryButtonDragging;

	uint _position;

	struct MenuSprData {
		Common::Array<uint16> pixels;
		Common::Array<uint8>  blend;   // per-pixel blend factor; kSprNoBlend = opaque
		uint16 w, h;
		int16  xoff, yoff;
		int    tx, ty;
		uint16 transpKey;
	};
	Common::Array<MenuSprData> _menuSprData;

	Graphics::Surface _bgSurface;
	Graphics::ManagedSurface _destSurface;

	// Zone indices within _zones — must match the order added in init().
	// The original Atlantis toolbar has no on-bar widgets for options /
	// prev / next / view-object: the auxiliary buttons we used to draw were
	// guesses that never matched a real sprite, so they are gone.
	enum ZoneIdx {
		kZoneInventory0 = 0  // inventory slots 0-8 occupy indices 0-8
	};

	// 9 visible inventory slots, one per star-of-David ornament in
	// OBJETS0.SPR (the inventory-bar background).
	static const uint kInventorySlots = 9;

	void addZone(Common::Rect rect, ZoneCallback callback);
	void updateZones();
	Common::Array<Zone>::const_iterator hitTestZones(const Common::Point &mousePos) const;
	uint captureEvent(const Common::Point &mousePos, uint dragStatus);
	void drawBackground(const Graphics::Surface *original);
	void drawToolbar(const Graphics::Surface *original);
	// Runs the open toolbar's event loop.  Returns true when the loop ended
	// because an inventory item was taken (the caller then skips the
	// slide-down so the carried object reaches navigation immediately).
	bool handleToolbarEvents(const Graphics::Surface *original);

	template<uint N>
	uint callbackInventory(uint dragStatus) { return callbackInventory(N, dragStatus); }
	uint callbackInventory(uint invId, uint dragStatus);
};

} // namespace Atlantis
} // namespace CryOmni3D

#endif // CRYOMNI3D_ATLANTIS_TOOLBAR_H
