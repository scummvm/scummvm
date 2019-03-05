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
		_messages(nullptr), _inventoryOffset(0), _engine(nullptr),
		_inventoryHovered(-1), _inventorySelected(-1), _inventoryEnabled(true),
		_position(60) { }
	~Toolbar();

	void init(const Sprites *sprites, FontManager *fontManager,
	          const Common::Array<Common::String> *messages, Inventory *inventory, CryOmni3DEngine *engine);

	Graphics::Surface &getBackgroundSurface() { return _bgSurface; }
	bool displayToolbar(const Graphics::Surface *original);
	void inventoryChanged(unsigned int newPosition);
	unsigned int inventoryOffset() const { return _inventoryOffset; }
	void setInventoryOffset(unsigned int offset) { _inventoryOffset = offset; }
	void setInventoryEnabled(bool enabled) { _inventoryEnabled = enabled; }

private:
	typedef unsigned int (Toolbar::*ZoneCallback)(unsigned int dragStatus);
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

	static const unsigned int kTextOffset = 13;

	void addZone(uint16 cursorMainId, uint16 cursorSecondaryId, Common::Point position,
	             ZoneCallback callback);
	void updateZones();
	Common::Array<Zone>::const_iterator hitTestZones(const Common::Point &mousePos) const;
	unsigned int captureEvent(const Common::Point &mousePos, unsigned int dragStatus);
	void drawToolbar(const Graphics::Surface *original);
	void handleToolbarEvents(const Graphics::Surface *original);

	bool _inventoryEnabled;
	unsigned int _inventoryMaxOffset;
	unsigned int _inventoryOffset;
	unsigned int _inventoryHovered;
	unsigned int _inventorySelected;

	Object *_backup_selected_object;
	bool _mouse_in_options;
	bool _mouse_in_view_object;
	bool _inventory_button_dragging;

	bool _parentMustRedraw;
	bool _shortExit;
	unsigned int _position;

	Graphics::Surface _bgSurface;
	Graphics::ManagedSurface _destSurface;

	template<unsigned int N>
	unsigned int callbackInventory(unsigned int dragStatus) { return callbackInventory(N, dragStatus); }
	unsigned int callbackInventory(unsigned int invId, unsigned int dragStatus);
	unsigned int callbackInventoryPrev(unsigned int dragStatus);
	unsigned int callbackInventoryNext(unsigned int dragStatus);
	unsigned int callbackViewObject(unsigned int dragStatus);
	unsigned int callbackOptions(unsigned int dragStatus);
	unsigned int callbackDocumentation(unsigned int dragStatus);
};

} // End of namespace Versailles
} // End of namespace CryOmni3D

#endif
