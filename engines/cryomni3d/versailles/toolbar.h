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

#ifndef CRYOMNI3D_VERSAILLES_TOOLBAR_H
#define CRYOMNI3D_VERSAILLES_TOOLBAR_H

#include "common/array.h"
#include "common/rect.h"
#include "graphics/managed_surface.h"
#include "graphics/surface.h"

#include "cryomni3d/font_manager.h"
#include "cryomni3d/objects.h"
#include "cryomni3d/sprites.h"

namespace CryOmni3D {
class CryOmni3DEngine;

namespace Versailles {


class Toolbar {
public:
	Toolbar() : _sprites(nullptr), _fontManager(nullptr), _inventory(nullptr),
		_messages(nullptr), _engine(nullptr), _inventoryEnabled(true), _inventoryMaxOffset(0),
		_inventoryOffset(0), _inventoryHovered(uint(-1)), _inventorySelected(uint(-1)), _backupSelectedObject(nullptr),
		_mouseInOptions(false), _mouseInViewObject(false), _inventoryButtonDragging(false), _parentMustRedraw(false),
		_shortExit(false), _position(60) { }
	~Toolbar();

	void init(const Sprites *sprites, FontManager *fontManager,
	          const Common::Array<Common::String> *messages, Inventory *inventory, CryOmni3DEngine *engine);

	Graphics::Surface &getBackgroundSurface() { return _bgSurface; }
	bool displayToolbar(const Graphics::Surface *original);
	void inventoryChanged(uint newPosition);
	uint inventoryOffset() const { return _inventoryOffset; }
	void setInventoryOffset(uint offset) { _inventoryOffset = offset; }
	void setInventoryEnabled(bool enabled) { _inventoryEnabled = enabled; }

private:
	typedef uint(Toolbar::*ZoneCallback)(uint dragStatus);
	struct Zone {
		Common::Rect rect;
		uint16 imageMain;
		uint16 imageSecondary;
		ZoneCallback callback;
		bool secondary;
		bool hidden;
	};
	Common::Array<Zone> _zones;
	const Sprites *_sprites;
	FontManager *_fontManager;
	const Common::Array<Common::String> *_messages;
	Inventory *_inventory;
	CryOmni3DEngine *_engine;

	static const uint kTextOffset = 13;

	void addZone(uint16 cursorMainId, uint16 cursorSecondaryId, Common::Point position,
	             ZoneCallback callback);
	void updateZones();
	Common::Array<Zone>::const_iterator hitTestZones(const Common::Point &mousePos) const;
	uint captureEvent(const Common::Point &mousePos, uint dragStatus);
	void drawToolbar(const Graphics::Surface *original);
	void handleToolbarEvents(const Graphics::Surface *original);

	bool _inventoryEnabled;
	uint _inventoryMaxOffset;
	uint _inventoryOffset;
	uint _inventoryHovered;
	uint _inventorySelected;

	Object *_backupSelectedObject;
	bool _mouseInOptions;
	bool _mouseInViewObject;
	bool _inventoryButtonDragging;

	bool _parentMustRedraw;
	bool _shortExit;
	uint _position;

	Graphics::Surface _bgSurface;
	Graphics::ManagedSurface _destSurface;

	template<uint N>
	uint callbackInventory(uint dragStatus) { return callbackInventory(N, dragStatus); }
	uint callbackInventory(uint invId, uint dragStatus);
	uint callbackInventoryPrev(uint dragStatus);
	uint callbackInventoryNext(uint dragStatus);
	uint callbackViewObject(uint dragStatus);
	uint callbackOptions(uint dragStatus);
	uint callbackDocumentation(uint dragStatus);
};

} // End of namespace Versailles
} // End of namespace CryOmni3D

#endif
