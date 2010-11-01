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

#include "lastexpress/game/inventory.h"

#include "lastexpress/data/cursor.h"
#include "lastexpress/data/scene.h"
#include "lastexpress/data/snd.h"

#include "lastexpress/game/logic.h"
#include "lastexpress/game/menu.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/sound.h"
#include "lastexpress/game/state.h"

#include "lastexpress/graphics.h"
#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"


#define drawItem(x, y, index, brightness) { Icon icon((CursorStyle)(index)); icon.setPosition(x, y); icon.setBrightness(brightness); _engine->getGraphicsManager()->draw(&icon, GraphicsManager::kBackgroundInventory); }

namespace LastExpress {

Inventory::Inventory(LastExpressEngine *engine) : _engine(engine), _selectedItem(kItemNone), _highlightedItem(kItemNone), _opened(false), _visible(false),
	_showingHourGlass(false), _blinkingEgg(false), _blinkingTime(0), _blinkingInterval(_defaultBlinkingInterval), _blinkingBrightness(100),
	_flagUseMagnifier(false), _flag1(false), _flag2(false), _flagEggHightlighted(false), _itemScene(NULL) {

	_inventoryRect = Common::Rect(0, 0, 32, 32);
	_menuRect = Common::Rect(608, 448, 640, 480);
	_selectedRect = Common::Rect(44, 0, 76, 32);

	init();
}

Inventory::~Inventory() {
	_itemScene = NULL;

	// Zero passed pointers
	_engine = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Inventory handling
//////////////////////////////////////////////////////////////////////////

// Initialize inventory contents
void Inventory::init() {
	// ID
	_entries[kItemMatchBox].cursor = kCursorMatchBox;
	_entries[kItemTelegram].cursor = kCursorTelegram;
	_entries[kItemPassengerList].cursor = kCursorPassengerList;
	_entries[kItemArticle].cursor = kCursorArticle;
	_entries[kItemScarf].cursor = kCursorScarf;
	_entries[kItemPaper].cursor = kCursorPaper;
	_entries[kItemParchemin].cursor = kCursorParchemin;
	_entries[kItemMatch].cursor = kCursorMatch;
	_entries[kItemWhistle].cursor = kCursorWhistle;
	_entries[kItemKey].cursor = kCursorKey;
	_entries[kItemBomb].cursor = kCursorBomb;
	_entries[kItemFirebird].cursor = kCursorFirebird;
	_entries[kItemBriefcase].cursor = kCursorBriefcase;
	_entries[kItemCorpse].cursor = kCursorCorpse;

	// Selectable
	_entries[kItemMatchBox].isSelectable = true;
	_entries[kItemMatch].isSelectable = true;
	_entries[kItemTelegram].isSelectable = true;
	_entries[kItemWhistle].isSelectable = true;
	_entries[kItemKey].isSelectable = true;
	_entries[kItemFirebird].isSelectable = true;
	_entries[kItemBriefcase].isSelectable = true;
	_entries[kItemCorpse].isSelectable = true;
	_entries[kItemPassengerList].isSelectable = true;

	// Auto selection
	_entries[kItem2].manualSelect = false;
	_entries[kItem3].manualSelect = false;
	_entries[kItem5].manualSelect = false;
	_entries[kItem7].manualSelect = false;
	_entries[kItem9].manualSelect = false;
	_entries[kItem11].manualSelect = false;
	_entries[kItemBeetle].manualSelect = false;
	_entries[kItem17].manualSelect = false;
	_entries[kItemFirebird].manualSelect = false;
	_entries[kItemBriefcase].manualSelect = false;
	_entries[kItemCorpse].manualSelect = false;
	_entries[kItemGreenJacket].manualSelect = false;
	_entries[kItem22].manualSelect = false;

	// Scene
	_entries[kItemMatchBox].scene = kSceneMatchbox;
	_entries[kItemTelegram].scene = kSceneTelegram;
	_entries[kItemPassengerList].scene = kScenePassengerList;
	_entries[kItemScarf].scene = kSceneScarf;
	_entries[kItemParchemin].scene = kSceneParchemin;
	_entries[kItemArticle].scene = kSceneArticle;
	_entries[kItemPaper].scene = kScenePaper;
	_entries[kItemFirebird].scene = kSceneFirebird;
	_entries[kItemBriefcase].scene = kSceneBriefcase;

	// Has item
	_entries[kItemTelegram].isPresent = true;
	_entries[kItemArticle].isPresent = true;

	_selectedItem = kItemNone;
}

// FIXME we need to draw cursors with full background opacity so that whatever is in the background is erased
// this saved us clearing some part of the background when switching between states

// TODO if we draw inventory objects on screen, we need to load a new scene.
// Signal that the inventory has taken over the screen and stop processing mouse events after we have been called
bool Inventory::handleMouseEvent(const Common::Event &ev) {

	// Do not show inventory when on the menu screen
	if (getMenu()->isShown() || !_visible)
		return false;

	// Flag to know whether to restore the current cursor or not
	bool insideInventory = false;

	// Egg (menu)
	if (_menuRect.contains(ev.mouse)) {
		insideInventory = true;
		_engine->getCursor()->setStyle(kCursorNormal);

		// If clicked, show the menu
		if (ev.type == Common::EVENT_LBUTTONUP) {
			getSound()->playSound(kEntityPlayer, "LIB039");
			getMenu()->show(false, kSavegameTypeIndex, 0);

			// TODO can we return directly or do we need to make sure the state will be "valid" when we come back from the menu
			return true;
		} else {
			// Highlight if needed
			if (_highlightedItem != getMenu()->getGameId() + 39) {
				_highlightedItem = (InventoryItem)(getMenu()->getGameId() + 39);
				drawItem(608, 448, _highlightedItem, 100)

				askForRedraw();
			}
		}
	} else {
		// remove highlight if needed
		if (_highlightedItem == getMenu()->getGameId() + 39) {
			drawItem(608, 448, _highlightedItem, 50)
			_highlightedItem = kItemNone;
			askForRedraw();
		}
	}

	// Portrait (inventory)
	if (_inventoryRect.contains(ev.mouse)) {
		insideInventory = true;
		_engine->getCursor()->setStyle(kCursorNormal);

		// If clicked, show pressed state and display inventory
		if (ev.type == Common::EVENT_LBUTTONUP) {
			open();
		} else {
			// Highlight if needed
			if (_highlightedItem != (InventoryItem)getProgress().portrait && !_opened) {
				_highlightedItem = (InventoryItem)getProgress().portrait;
				drawItem(0, 0, getProgress().portrait, 100)

				askForRedraw();
			}
		}
	} else {
		// remove highlight if needed
		if (_highlightedItem == (InventoryItem)getProgress().portrait && !_opened) {
			drawItem(0, 0, getProgress().portrait, 50)
			_highlightedItem = kItemNone;
			askForRedraw();
		}
	}

	// If the inventory is open, check all items rect to see if we need to highlight one / handle click
	if (_opened) {

		// Always show normal cursor when the inventory is opened
		insideInventory = true;
		_engine->getCursor()->setStyle(kCursorNormal);

		bool selected = false;

		// Iterate over items
		int16 y = 44;
		for (int i = 1; i < 32; i++) {
			if (!hasItem((InventoryItem)i))
				continue;

			if (Common::Rect(0, y, 32, 32 + y).contains(ev.mouse)) {

				// If released with an item highlighted, show this item
				if (ev.type == Common::EVENT_LBUTTONUP) {
					if (_entries[i].isSelectable) {
						selected = true;
						_selectedItem = (InventoryItem)i;
						drawItem(44, 0, get(_selectedItem)->cursor, 100)
					}

					examine((InventoryItem)i);
					break;
				} else {
					if (_highlightedItem != i) {
						drawItem(0, y, _entries[i].cursor, 100)
						_highlightedItem = (InventoryItem)i;
						askForRedraw();
					}
				}
			} else {
				// Remove highlight if necessary
				if (_highlightedItem == i) {
					drawItem(0, y, _entries[i].cursor, 50)
					_highlightedItem = kItemNone;
					askForRedraw();
				}
			}

			y += 40;
		}

		// Right button is released: we need to close the inventory
		if (ev.type == Common::EVENT_LBUTTONUP) {

			// Not on a selectable item: unselect the current item
			if (!selected)
				unselectItem();

			close();
		}
	}

	// Selected item
	if (_selectedItem != kItemNone && _selectedRect.contains(ev.mouse)) {
		insideInventory = true;

		// Show magnifier icon
		_engine->getCursor()->setStyle(kCursorMagnifier);

		if (ev.type == Common::EVENT_LBUTTONUP) {
			examine((InventoryItem)_selectedItem);
		}
	}

	// If the egg is blinking, refresh
	if (_blinkingEgg)
		drawEgg();

	// Restore cursor
	//if (!insideInventory)
	//	_engine->getCursor()->setStyle(getLogic()->getCursorStyle());

	return insideInventory;
}

//////////////////////////////////////////////////////////////////////////
// UI
//////////////////////////////////////////////////////////////////////////
void Inventory::show() {
	clearBg(GraphicsManager::kBackgroundInventory);
	askForRedraw();

	// Show portrait (first draw, cannot be highlighted)
	drawItem(0, 0, getProgress().portrait, 50)

	// Show selected item
	if (_selectedItem != kItemNone)
		drawItem(44, 0, _selectedItem, 100)

	drawEgg();
}

void Inventory::setPortrait(InventoryItem item) const {
	getProgress().portrait = item;
	drawItem(0, 0, getProgress().portrait, 50);
}

void Inventory::blinkEgg(bool enabled) {
	_blinkingEgg = enabled;

	// Reset state
	_showingHourGlass = false;

	// Show egg at full brightness for first step if blinking
	if (_blinkingEgg)
		drawItem(608, 448, getMenu()->getGameId() + 39, _blinkingBrightness)
	else {
		// Reset values
		_blinkingBrightness = 100;
		_blinkingInterval = _defaultBlinkingInterval;
		drawItem(608, 448, getMenu()->getGameId() + 39, 50) // normal egg state
	}

	askForRedraw();
}

void Inventory::showHourGlass() const{
	if (!getFlags()->flag_5) {
		drawItem(608, 448, kCursorHourGlass, 100);
	}

	askForRedraw();

	getFlags()->shouldDrawEggOrHourGlass = true;
}

//////////////////////////////////////////////////////////////////////////
// Items
//////////////////////////////////////////////////////////////////////////
Inventory::InventoryEntry *Inventory::get(InventoryItem item) {
	if (item >= kPortraitOriginal)
		error("Inventory::getEntry: Invalid inventory item!");

	return &_entries[item];
}

void Inventory::addItem(InventoryItem item) {
	if (item >= kPortraitOriginal)
		return;

	get(item)->isPresent = true;
	get(item)->location = kObjectLocationNone;

	// Auto-select item if necessary
	if (get(item)->cursor && !get(item)->manualSelect) {
		_selectedItem = (InventoryItem)get(item)->cursor;
		drawItem(44, 0, _selectedItem, 100)
		askForRedraw();
	}
}

void Inventory::removeItem(InventoryItem item, ObjectLocation newLocation) {
	if (item >= kPortraitOriginal)
		return;

	get(item)->isPresent = false;
	get(item)->location = newLocation;

	if (get(item)->cursor == (CursorStyle)_selectedItem) {
		_selectedItem = kItemNone;
		_engine->getGraphicsManager()->clear(GraphicsManager::kBackgroundInventory, Common::Rect(44, 0, 44 + 32, 32));
		askForRedraw();
	}
}

bool Inventory::hasItem(InventoryItem item) {
	if (get(item)->isPresent && item < kPortraitOriginal)
		return true;

	return false;
}

void Inventory::selectItem(InventoryItem item) {
	_selectedItem = item;

	drawItem(44, 0, get(_selectedItem)->cursor, 100)
	askForRedraw();
}

void Inventory::unselectItem() {
	_selectedItem = kItemNone;

	_engine->getGraphicsManager()->clear(GraphicsManager::kBackgroundInventory, Common::Rect(44, 0, 44 + 32, 32));
	askForRedraw();
}

void Inventory::setLocationAndProcess(InventoryItem item, ObjectLocation location) {
	if (item >= kPortraitOriginal)
		return;

	if (get(item)->location == location)
		return;

	get(item)->location = location;

	if (isItemSceneParameter(item) && !getFlags()->flag_0)
		getScenes()->processScene();
}

//////////////////////////////////////////////////////////////////////////
// Serializable
//////////////////////////////////////////////////////////////////////////
void Inventory::saveLoadWithSerializer(Common::Serializer &s) {
	for (uint i = 0; i < ARRAYSIZE(_entries); i++)
		_entries[i].saveLoadWithSerializer(s);
}

void Inventory::saveSelectedItem(Common::Serializer &s) {
	s.syncAsUint32LE(_selectedItem);
}

//////////////////////////////////////////////////////////////////////////
// toString
//////////////////////////////////////////////////////////////////////////
Common::String Inventory::toString() {
	Common::String ret = "";

	for (int i = 0; i < kPortraitOriginal; i++)
		ret += Common::String::format("%d : %s\n", i, _entries[i].toString().c_str());

	return ret;
}

//////////////////////////////////////////////////////////////////////////
// Private methods
//////////////////////////////////////////////////////////////////////////
InventoryItem Inventory::getFirstExaminableItem() const {

	int index = 0;
	InventoryEntry entry = _entries[index];
	while (!entry.isPresent || !entry.cursor || entry.manualSelect) {
		index++;
		entry = _entries[index];

		if (index >= kPortraitOriginal)
			return kItemNone;
	}

	return (InventoryItem)index;
}

bool Inventory::isItemSceneParameter(InventoryItem item) const {
	Scene *scene = getScenes()->get(getState()->scene);

	switch(scene->type) {
	default:
		return false;

	case Scene::kTypeItem:
		if (scene->param1 == item)
			return true;
		break;

	case Scene::kTypeItem2:
		if (scene->param1 == item || scene->param2 == item)
			return true;
		break;

	case Scene::kTypeObjectItem:
		if (scene->param2 == item)
			return true;
		break;

	case Scene::kTypeItem3:
		if (scene->param1 == item || scene->param2 == item || scene->param3 == item)
			return true;
		break;

	case Scene::kTypeCompartmentsItem:
		if (scene->param2 == item)
			return true;
		break;
	}

	return false;
}

// Examine an inventory item
void Inventory::examine(InventoryItem item) {
	SceneIndex index = get(item)->scene;
	if (!index)
		return;

	/*if (!getState()->sceneUseBackup ||
		(getState()->sceneBackup2 && getFirstExaminableItem() == _selectedItem))
		flag = 1;*/

	if (!getState()->sceneUseBackup) {
		getState()->sceneBackup = getState()->scene;
		getState()->sceneUseBackup = true;

		getScenes()->loadScene(index);
	} else {

		if (!getState()->sceneBackup2)
			return;

		if (getFirstExaminableItem() == _selectedItem) {
			index = getState()->sceneBackup2;
			getState()->sceneBackup2 = kSceneNone;
			getScenes()->loadScene(index);
		}
	}
}

// FIXME: see different callers and adjust
// - draw with different brightness if mousing over
void Inventory::drawEgg() const {
	if (!getFlags()->flag_5)
		drawItem(608, 448, getMenu()->getGameId() + 39, 50)

	getFlags()->shouldDrawEggOrHourGlass = false;
}

// Blinking egg: we need to blink the egg for delta time, with the blinking getting faster until it's always lit.
void Inventory::drawBlinkingEgg() {

	warning("Inventory::drawEgg - blinking not implemented!");

	//// TODO show egg (with or without mouseover)

	//// Play timer sound
	//if (getGlobalTimer() < 90) {
	//	if (getGlobalTimer() + ticks >= 90)
	//		getSound()->playSoundWithSubtitles("TIMER.SND", 50331664, kEntityPlayer);

	//	if (getSound()->isBuffered("TIMER"))
	//		setGlobalTimer(0);
	//}

	//// Restore egg to standard brightness
	//if (!getGlobalTimer()) {
	//
	//}


	//drawItem(608, 448, getMenu()->getGameId() + 39, _blinkingBrightness)

	//// TODO if delta time > _blinkingInterval, update egg & ask for redraw then adjust blinking time and remaining time
	//

	//// Reset values and stop blinking
	//if (_blinkingTime == 0)
	//	blinkEgg(false);

	askForRedraw();
}

// Close inventory: clear items and reset icon
void Inventory::open() {
	_opened = true;

	// Show selected state
	drawItem(0, 0, getProgress().portrait + 1, 100)

	int16 y = 44;

	// Iterate over items
	for (uint i = 1; i < 32; i++) {
		if (_entries[i].isPresent) {
			drawItem(0, y, _entries[i].cursor, 50)
			y += 40;
		}
	}

	askForRedraw();
}

// Close inventory: clear items and reset icon
void Inventory::close() {
	_opened = false;

	// Fallback to unselected state
	drawItem(0, 0, getProgress().portrait, 100)

	// Erase rectangle for all inventory items
	int count = 0;
	for (uint i = 1; i < 32; i++) {
		if (_entries[i].isPresent) {
			count++;
		}
	}

	_engine->getGraphicsManager()->clear(GraphicsManager::kBackgroundInventory, Common::Rect(0, 44, 32, (int16)(44 + 44 * count)));

	askForRedraw();
}

Common::Rect Inventory::getItemRect(int16 index) const{
	return Common::Rect(0, (int16)((32 + 12) * (index + 1)), 32, (int16)((32 + 12) * (index + 2))); // space between items = 12px
}

} // End of namespace LastExpress
