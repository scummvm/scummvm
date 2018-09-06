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

#include "mutationofjb/gamescreen.h"

#include "mutationofjb/animationdecoder.h"
#include "mutationofjb/encryptedfile.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/inventory.h"
#include "mutationofjb/util.h"
#include "mutationofjb/widgets/widget.h"
#include "mutationofjb/widgets/inventorywidget.h"
#include "mutationofjb/widgets/imagewidget.h"
#include "mutationofjb/widgets/conversationwidget.h"

#include "common/rect.h"

#include "graphics/screen.h"

namespace MutationOfJB {

enum ButtonType {
	BUTTON_WALK = 0,
	BUTTON_TALK,
	BUTTON_LOOK,
	BUTTON_USE,
	BUTTON_PICKUP,
	BUTTON_SCROLL_LEFT,
	BUTTON_SCROLL_RIGHT,
	BUTTON_SETTINGS,
	NUM_BUTTONS
};

enum {
	INVENTORY_START_X = 88,
	INVENTORY_START_Y = 149,
	INVENTORY_ITEM_WIDTH = 34,
	INVENTORY_ITEM_HEIGHT = 33,
	INVENTORY_ITEMS_PER_LINE = 8,
	INVENTORY_ITEMS_LINES = 5,
	CONVERSATION_X = 0,
	CONVERSATION_Y = 139,
	CONVERSATION_WIDTH = 320,
	CONVERSATION_HEIGHT = 61
};


GameScreen::GameScreen(Game &game, Graphics::Screen *screen)
	: GuiScreen(game, screen),
	  _inventoryWidget(nullptr),
	  _conversationWidget(nullptr) {}

GameScreen::~GameScreen() {}

bool GameScreen::init() {
	if (!loadInventoryGfx()) {
		return false;
	}

	if (!loadHudGfx()) {
		return false;
	}

	_game.getGameData().getInventory().setObserver(this);

	// Init widgets.

	const Common::Rect backgroundRect(CONVERSATION_X, CONVERSATION_Y, CONVERSATION_X + CONVERSATION_WIDTH, CONVERSATION_Y + CONVERSATION_HEIGHT);
	const Graphics::Surface backgroundSurface = _hudSurfaces[0].getSubArea(backgroundRect);
	ImageWidget *image = new ImageWidget(*this, backgroundRect, backgroundSurface);
	addWidget(image);

	_inventoryWidget = new InventoryWidget(*this, _inventorySurfaces);
	_inventoryWidget->setCallback(this);
	addWidget(_inventoryWidget);

	const Common::Rect ButtonRects[] = {
		Common::Rect(0, 148, 67, 158), // Walk
		Common::Rect(0, 158, 67, 168), // Talk
		Common::Rect(0, 168, 67, 178), // Look
		Common::Rect(0, 178, 67, 188), // Use
		Common::Rect(0, 188, 67, 198), // PickUp
		Common::Rect(67, 149, 88, 174), // ScrollLeft
		Common::Rect(67, 174, 88, 199), // ScrollRight
		Common::Rect(301, 148, 320, 200) // Settings
	};

	for (int i = 0; i < NUM_BUTTONS; ++i) {
		const Graphics::Surface normalSurface = _hudSurfaces[0].getSubArea(ButtonRects[i]);
		const Graphics::Surface pressedSurface = _hudSurfaces[1].getSubArea(ButtonRects[i]);
		ButtonWidget *button = new ButtonWidget(*this, ButtonRects[i], normalSurface, pressedSurface);
		button->setId(i);
		button->setCallback(this);
		addWidget(button);
	}

	const Common::Rect conversationRect(CONVERSATION_X, CONVERSATION_Y, CONVERSATION_X + CONVERSATION_WIDTH, CONVERSATION_Y + CONVERSATION_HEIGHT);
	const Graphics::Surface conversationSurface = _hudSurfaces[2].getSubArea(conversationRect);
	_conversationWidget = new ConversationWidget(*this, conversationRect, conversationSurface);
	_conversationWidget->setVisible(false);
	addWidget(_conversationWidget);

	return true;
}

ConversationWidget &GameScreen::getConversationWidget() {
	return *_conversationWidget;
}

class InventoryAnimationDecoderCallback : public AnimationDecoderCallback {
public:
	InventoryAnimationDecoderCallback(GameScreen &gui) : _gui(gui) {}
	virtual void onFrame(int frameNo, Graphics::Surface &surface) override;
	virtual void onPaletteUpdated(byte palette[PALETTE_SIZE]) override;
private:
	GameScreen &_gui;
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

bool GameScreen::loadInventoryGfx() {
	AnimationDecoder decoder("icons.dat");
	InventoryAnimationDecoderCallback callback(*this);
	return decoder.decode(&callback);
}

class HudAnimationDecoderCallback : public AnimationDecoderCallback {
public:
	HudAnimationDecoderCallback(GameScreen &gui) : _gui(gui) {}
	virtual void onFrame(int frameNo, Graphics::Surface &surface) override;
	virtual void onPaletteUpdated(byte palette[PALETTE_SIZE]) override;
private:
	GameScreen &_gui;
};

void HudAnimationDecoderCallback::onPaletteUpdated(byte [PALETTE_SIZE]) {
}

void HudAnimationDecoderCallback::onFrame(int frameNo, Graphics::Surface &surface) {
	if (frameNo == 0 || frameNo == 1 || frameNo == 4) {
		Graphics::Surface outSurface;
		outSurface.copyFrom(surface);
		_gui._hudSurfaces.push_back(outSurface);
	}
}

bool GameScreen::loadHudGfx() {
	AnimationDecoder decoder("room0.dat");
	HudAnimationDecoderCallback callback(*this);
	return decoder.decode(&callback);
}

void GameScreen::onInventoryChanged() {
	_inventoryWidget->markDirty();
}

void GameScreen::onButtonClicked(ButtonWidget *button) {
	const int buttonId = button->getId();
	if (buttonId <= BUTTON_PICKUP) {
		const ActionInfo::Action actions[] = {ActionInfo::Walk, ActionInfo::Talk, ActionInfo::Look, ActionInfo::Use, ActionInfo::PickUp};
		_game.setCurrentAction(actions[buttonId]);
	} else if (buttonId == BUTTON_SCROLL_LEFT) {
		_game.getGameData().getInventory().scrollLeft();
	} else if (buttonId == BUTTON_SCROLL_RIGHT) {
		_game.getGameData().getInventory().scrollRight();
	}
}

void GameScreen::onInventoryItemHovered(InventoryWidget *widget, int posInWidget) {
	// TODO
}

void GameScreen::onInventoryItemClicked(InventoryWidget *widget, int posInWidget) {
	// Position in widget should match the position in inventory.
	const Common::String &item = getGame().getGameData().getInventory().getItems()[posInWidget];

	if (_game.getCurrentAction() == ActionInfo::Use) {
		// TODO
	} else {
		_game.startActionSection(ActionInfo::Look, item);
	}
}

}
