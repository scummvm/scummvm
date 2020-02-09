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
#include "mutationofjb/script.h"
#include "mutationofjb/guiscreen.h"
#include "mutationofjb/widgets/buttonwidget.h"
#include "mutationofjb/widgets/inventorywidget.h"
#include "mutationofjb/widgets/gamewidget.h"

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
class LabelWidget;
class GameWidget;

class GameScreen : public GuiScreen, public InventoryObserver, public ButtonWidgetCallback, public InventoryWidgetCallback, public GameWidgetCallback {
public:
	friend class InventoryAnimationDecoderCallback;
	friend class HudAnimationDecoderCallback;

	GameScreen(Game &game, Graphics::Screen *screen);
	~GameScreen() override;

	bool init();

	void handleEvent(const Common::Event &event) override;

	void onInventoryChanged() override;
	void onButtonClicked(ButtonWidget *) override;
	void onInventoryItemHovered(InventoryWidget *widget, int posInWidget) override;
	void onInventoryItemClicked(InventoryWidget *widget, int posInWidget) override;
	void onGameDoorClicked(GameWidget *, Door *door) override;
	void onGameStaticClicked(GameWidget *, Static *stat) override;
	void onGameEntityHovered(GameWidget *, const Common::String &entity) override;

	ConversationWidget &getConversationWidget();

	void showConversationWidget(bool show);
	void refreshAfterSceneChanged();

private:
	bool loadInventoryGfx();
	bool loadHudGfx();
	void drawInventoryItem(const Common::String &item, int pos);
	void drawInventory();

	void updateStatusBarText(const Common::String &entity, bool inventory);

	Common::Array<Graphics::Surface> _inventorySurfaces;
	Common::Array<Graphics::Surface> _hudSurfaces;

	Common::Array<ButtonWidget *> _buttons;
	InventoryWidget *_inventoryWidget;
	ConversationWidget *_conversationWidget;
	LabelWidget *_statusBarWidget;
	GameWidget *_gameWidget;

	ActionInfo::Action _currentAction;
	Common::String _currentPickedItem;
};

}

#endif
