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

#include "mutationofjb/inventory.h"
#include "mutationofjb/widgets/buttonwidget.h"
#include "mutationofjb/widgets/inventorywidget.h"
#include "mutationofjb/guiscreen.h"

#include "common/array.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

#include "graphics/surface.h"

namespace Common {
struct Event;
}

namespace Graphics {
class Screen;
}

namespace MutationOfJB {

class Game;
class Widget;
class InventoryWidget;
class ConversationWidget;

class GameScreen : public GuiScreen, public InventoryObserver, public ButtonWidgetCallback, public InventoryWidgetCallback {
public:
	friend class InventoryAnimationDecoderCallback;
	friend class HudAnimationDecoderCallback;

	GameScreen(Game &game, Graphics::Screen *screen);
	~GameScreen();

	bool init();

	virtual void onInventoryChanged() override;
	virtual void onButtonClicked(ButtonWidget *) override;
	virtual void onInventoryItemHovered(InventoryWidget *widget, int posInWidget) override;
	virtual void onInventoryItemClicked(InventoryWidget *widget, int posInWidget) override;

	ConversationWidget &getConversationWidget();

private:
	bool loadInventoryGfx();
	bool loadHudGfx();
	void drawInventoryItem(const Common::String &item, int pos);
	void drawInventory();

	Common::Array<Graphics::Surface> _inventorySurfaces;
	Common::Array<Graphics::Surface> _hudSurfaces;

	InventoryWidget *_inventoryWidget;
	ConversationWidget *_conversationWidget;
};

}

#endif
