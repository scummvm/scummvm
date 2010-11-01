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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef LASTEXPRESS_INVENTORY_H
#define LASTEXPRESS_INVENTORY_H

/*
	Inventory entry (32 entries)
	----------------------------

	    byte {1}        - Item ID (set to 0 for "undefined" items)
	    byte {1}        - Scene ID
	    byte {1}        - ??
	    byte {1}        - Selectable (1 if item is selectable, 0 otherwise)
	    byte {1}        - Is item in inventory (set to 1 for telegram and article)
	    byte {1}        - Auto selection (1 for no auto selection, 0 otherwise)
	    byte {1}        - Location

*/

#include "lastexpress/shared.h"

#include "lastexpress/eventhandler.h"

#include "common/events.h"
#include "common/serializer.h"

namespace LastExpress {

class LastExpressEngine;
class Scene;

class Inventory : Common::Serializable, public EventHandler {
public:

	// Entry
	struct InventoryEntry : Common::Serializable {
		CursorStyle cursor;
		SceneIndex scene;
		byte field_2;
		bool isSelectable;
		bool isPresent;
		bool manualSelect;
		ObjectLocation location;

		InventoryEntry() {
			cursor = kCursorNormal;
			scene = kSceneNone;
			field_2 = 0;
			isSelectable = false;
			isPresent = false;
			manualSelect = true;
			location = kObjectLocationNone;
		}

		Common::String toString() {
			return Common::String::format("{ %d - %d - %d - %d - %d - %d - %d }", cursor, scene, field_2, isSelectable, isPresent, manualSelect, location);
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsByte(cursor);
			s.syncAsByte(scene);
			s.syncAsByte(field_2);
			s.syncAsByte(isSelectable);
			s.syncAsByte(isPresent);
			s.syncAsByte(manualSelect);
			s.syncAsByte(location);
		}
	};

	Inventory(LastExpressEngine *engine);
	~Inventory();

	// Inventory contents
	void addItem(InventoryItem item);
	void removeItem(InventoryItem item, ObjectLocation newLocation = kObjectLocationNone);
	bool hasItem(InventoryItem item);
	void selectItem(InventoryItem item);
	void unselectItem();
	InventoryItem getSelectedItem() { return _selectedItem; }

	InventoryEntry *get(InventoryItem item);
	InventoryEntry *getSelectedEntry() { return get(_selectedItem); }

	InventoryItem getFirstExaminableItem() const;
	void setLocationAndProcess(InventoryItem item, ObjectLocation location);

	// UI Control
	void show();
	void blinkEgg(bool enabled);
	void showHourGlass() const;
	void setPortrait(InventoryItem item) const;
	void drawEgg() const;
	void drawBlinkingEgg();

	// Handle inventory UI events.
	bool handleMouseEvent(const Common::Event &ev);

	// State
	bool isMagnifierInUse() { return _flagUseMagnifier; }
	bool isFlag1() { return _flag1; }
	bool isFlag2() { return _flag2; }
	bool isEggHighlighted() { return _flagEggHightlighted; }

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s);
	void saveSelectedItem(Common::Serializer &s);

	/**
	 * Convert this object into a string representation.
	 *
	 * @return A string representation of this object.
	 */
	Common::String toString();

private:
	static const uint32 _defaultBlinkingInterval = 250; ///< Default blinking interval in ms

	LastExpressEngine *_engine;

	InventoryEntry _entries[32];
	InventoryItem _selectedItem;
	InventoryItem _highlightedItem;
	bool _opened;
	bool _visible;

	bool _showingHourGlass;
	bool _blinkingEgg;
	uint32 _blinkingTime;
	uint32 _blinkingInterval;
	uint32 _blinkingBrightness;

	// Flags
	bool _flagUseMagnifier;
	bool _flag1;
	bool _flag2;
	bool _flagEggHightlighted;

	Scene *_itemScene;

	// Important rects
	Common::Rect _inventoryRect;
	Common::Rect _menuRect;
	Common::Rect _selectedRect;

	void init();

	void open();
	void close();
	void examine(InventoryItem item);
	Common::Rect getItemRect(int16 index) const;

	bool isItemSceneParameter(InventoryItem item) const;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_INVENTORY_H
