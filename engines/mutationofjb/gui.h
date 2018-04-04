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

#ifndef MUTATIONOFJB_GUI_H
#define MUTATIONOFJB_GUI_H

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "graphics/surface.h"
#include "mutationofjb/inventory.h"

namespace Graphics {
class Screen;
}

namespace MutationOfJB {

class Game;

class Gui : public InventoryObserver {
public:
	friend class InventoryAnimationDecoderCallback;
	Gui(Game &game, Graphics::Screen *screen);
	bool init();
	void update();

	void markInventoryDirty();

	virtual void onInventoryChanged() override;

private:
	bool loadInventoryGfx();
	bool loadInventoryList();
	void drawInventoryItem(const Common::String &item, int pos);
	void drawInventory();

	typedef Common::HashMap<Common::String, int> InventoryMap;

	Game &_game;
	Graphics::Screen *_screen;
	InventoryMap _inventoryItems;
	Common::Array<Graphics::Surface> _inventorySurfaces;
	bool _inventoryDirty;
};

}

#endif
