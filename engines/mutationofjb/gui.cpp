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

#include "mutationofjb/gui.h"
#include "mutationofjb/animationdecoder.h"
#include "mutationofjb/encryptedfile.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/inventory.h"
#include "mutationofjb/util.h"
#include "common/rect.h"
#include "graphics/screen.h"

namespace MutationOfJB {

enum ButtonType {
	BUTTON_WALK,
	BUTTON_TALK,
	BUTTON_LOOK,
	BUTTON_USE,
	BUTTON_PICKUP,
	BUTTON_SCROLL_LEFT,
	BUTTON_SCROLL_RIGHT,
	BUTTON_SETTINGS
};

enum {
	INVENTORY_START_X = 88,
	INVENTORY_START_Y = 149,
	INVENTORY_ITEM_WIDTH = 34,
	INVENTORY_ITEM_HEIGHT = 33,
	INVENTORY_ITEMS_PER_LINE = 8,
	INVENTORY_ITEMS_LINES = 5
};

static Common::Rect ButtonRects[] = {
	Common::Rect(0, 148, 67, 158), // Walk
	Common::Rect(0, 158, 67, 168), // Talk
	Common::Rect(0, 168, 67, 178), // Look
	Common::Rect(0, 178, 67, 188), // Use
	Common::Rect(0, 188, 67, 198), // PickUp
	Common::Rect(67, 149, 88, 174), // ScrollLeft
	Common::Rect(67, 174, 88, 199), // ScrollRight
	Common::Rect(301, 148, 320, 200) // Settings
};

Gui::Gui(Game &game, Graphics::Screen *screen)
	: _game(game),
	_screen(screen),
	_inventoryDirty(false) {
}

bool Gui::init() {
	const bool result1 = loadInventoryList();
	const bool result2 = loadInventoryGfx();

	_game.getGameData().getInventory().setObserver(this);

	return result1 && result2;
}

void Gui::markInventoryDirty() {
	_inventoryDirty = true;
}

void Gui::update() {
	if (_inventoryDirty) {
		drawInventory();
		_inventoryDirty = false;
	}
}

class InventoryAnimationDecoderCallback : public AnimationDecoderCallback {
public:
	InventoryAnimationDecoderCallback(Gui &gui) : _gui(gui) {}
	virtual void onFrame(int frameNo, Graphics::Surface &surface) override;
	virtual void onPaletteUpdated(byte palette[PALETTE_SIZE]) override;
private:
	Gui &_gui;
};

void InventoryAnimationDecoderCallback::onPaletteUpdated(byte palette[PALETTE_SIZE]) {
	_gui._screen->setPalette(palette + 0xC0 * 3, 0xC0, 0x20); // Load only 0x20 colors.
}

void InventoryAnimationDecoderCallback::onFrame(int frameNo, Graphics::Surface &surface) {
	if (frameNo < 3) {
		Graphics::Surface outSurface;
		outSurface.copyFrom(surface);
		_gui._inventorySurfaces.push_back(outSurface);
	}
}

bool Gui::loadInventoryGfx() {
	AnimationDecoder decoder("icons.dat");
	InventoryAnimationDecoderCallback callback(*this);
	return decoder.decode(&callback);
}

bool Gui::loadInventoryList() {
	EncryptedFile file;
	const char *fileName = "fixitems.dat";
	file.open(fileName);
	if (!file.isOpen()) {
		reportFileMissingError(fileName);
		return false;
	}

	int itemIndex = 0;
	while (!file.eos()) {
		Common::String line = file.readLine();
		if (line.empty() || line.hasPrefix("#")) {
			continue;
		}
		const char *firstSpace = strchr(line.c_str(), ' ');
		if (!firstSpace) {
			continue;
		}
		const int len = firstSpace - line.c_str();
		if (!len) {
			continue;
		}
		Common::String item(line.c_str(), len);
		_inventoryItems[item] = itemIndex;
		itemIndex++;
	}

	return true;
}

void Gui::drawInventoryItem(const Common::String &item, int pos) {
	InventoryMap::iterator it = _inventoryItems.find(item);
	if (it == _inventoryItems.end()) {
		return;
	}

	const int index = it->_value;
	const int surfaceNo = index / (INVENTORY_ITEMS_LINES * INVENTORY_ITEMS_PER_LINE);
	const int indexInSurface = index % (INVENTORY_ITEMS_LINES * INVENTORY_ITEMS_PER_LINE);
	const int itemX = indexInSurface % INVENTORY_ITEMS_PER_LINE;
	const int itemY = indexInSurface / INVENTORY_ITEMS_PER_LINE;

	Common::Point destStartPos(INVENTORY_START_X + pos * INVENTORY_ITEM_WIDTH, INVENTORY_START_Y);
	Common::Rect sourceRect(itemX * INVENTORY_ITEM_WIDTH, itemY * INVENTORY_ITEM_HEIGHT, (itemX + 1) * INVENTORY_ITEM_WIDTH, (itemY + 1) * INVENTORY_ITEM_HEIGHT);
	_screen->blitFrom(_inventorySurfaces[surfaceNo], sourceRect, destStartPos);
}

void Gui::drawInventory() {
	Inventory &inventory = _game.getGameData().getInventory();
	const Inventory::Items &items = inventory.getItems();
	Common::Rect fullRect(INVENTORY_START_X, INVENTORY_START_Y, INVENTORY_START_X + Inventory::VISIBLE_ITEMS * INVENTORY_ITEM_WIDTH, INVENTORY_START_Y + INVENTORY_ITEM_HEIGHT);
	_screen->fillRect(fullRect, 0x00);
	for (int i = 0; i < MIN((int) items.size(), (int) Inventory::VISIBLE_ITEMS); ++i) {
		drawInventoryItem(items[i], i);
	}
}

void Gui::onInventoryChanged() {
	markInventoryDirty();
}

}
