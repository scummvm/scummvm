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
#include "mutationofjb/mutationofjb.h"
#include "mutationofjb/inventory.h"
#include "mutationofjb/room.h"
#include "mutationofjb/util.h"
#include "mutationofjb/widgets/conversationwidget.h"
#include "mutationofjb/widgets/gamewidget.h"
#include "mutationofjb/widgets/imagewidget.h"
#include "mutationofjb/widgets/inventorywidget.h"
#include "mutationofjb/widgets/labelwidget.h"

#include "common/events.h"
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
	CONVERSATION_HEIGHT = 61,
	STATUS_BAR_X = 0,
	STATUS_BAR_Y = 140,
	STATUS_BAR_WIDTH = 320,
	STATUS_BAR_HEIGHT = 8
};


GameScreen::GameScreen(Game &game, Graphics::Screen *screen)
	: GuiScreen(game, screen),
	  _inventoryWidget(nullptr),
	  _conversationWidget(nullptr),
	  _statusBarWidget(nullptr),
	  _currentAction(ActionInfo::Walk) {}

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
		_buttons.push_back(button);
		addWidget(button);
	}

	const Common::Rect statusBarRect(STATUS_BAR_X, STATUS_BAR_Y, STATUS_BAR_X + STATUS_BAR_WIDTH, STATUS_BAR_Y + STATUS_BAR_HEIGHT);
	_statusBarWidget = new LabelWidget(*this, statusBarRect);
	addWidget(_statusBarWidget);

	const Common::Rect conversationRect(CONVERSATION_X, CONVERSATION_Y, CONVERSATION_X + CONVERSATION_WIDTH, CONVERSATION_Y + CONVERSATION_HEIGHT);
	const Graphics::Surface conversationSurface = _hudSurfaces[2].getSubArea(conversationRect);
	_conversationWidget = new ConversationWidget(*this, conversationRect, conversationSurface);
	_conversationWidget->setVisible(false);
	addWidget(_conversationWidget);

	_gameWidget = new GameWidget(*this);
	_gameWidget->setCallback(this);
	addWidget(_gameWidget);

	return true;
}

void GameScreen::handleEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_KEYUP: {
		switch (event.kbd.ascii) {
		case 'g':
			_currentAction = ActionInfo::Walk;
			_currentPickedItem.clear();
			break;
		case 'r':
			_currentAction = ActionInfo::Talk;
			_currentPickedItem.clear();
			break;
		case 's':
			_currentAction = ActionInfo::Look;
			_currentPickedItem.clear();
			break;
		case 'b':
			_currentAction = ActionInfo::Use;
			_currentPickedItem.clear();
			break;
		case 'n':
			_currentAction = ActionInfo::PickUp;
			_currentPickedItem.clear();
			break;
		default:
			break;
		}
		break;
	}
	default:
		break;
	}

	GuiScreen::handleEvent(event);
}

ConversationWidget &GameScreen::getConversationWidget() {
	return *_conversationWidget;
}

void GameScreen::showConversationWidget(bool show) {
	_gameWidget->setEnabled(!show);
	_conversationWidget->setVisible(show);
	_statusBarWidget->setText(Common::String());

	for (Common::Array<ButtonWidget *>::const_iterator it = _buttons.begin(); it != _buttons.end(); ++it) {
		(*it)->setVisible(!show);
	}
	_inventoryWidget->setVisible(!show);
}

void GameScreen::refreshAfterSceneChanged() {
	const Widgets &widgets = getWidgets();

	if (!getGame().isCurrentSceneMap()) {
		_gameWidget->setArea(Common::Rect(GameWidget::GAME_NORMAL_AREA_WIDTH, GameWidget::GAME_NORMAL_AREA_HEIGHT));

		for (Widgets::const_iterator it = widgets.begin(); it != widgets.end(); ++it) {
			if (*it == _gameWidget || *it == _conversationWidget)
				continue;

			(*it)->setVisible(true);
		}
	} else {
		_gameWidget->setArea(Common::Rect(GameWidget::GAME_FULL_AREA_WIDTH, GameWidget::GAME_FULL_AREA_HEIGHT));
		for (Widgets::const_iterator it = widgets.begin(); it != widgets.end(); ++it) {
			if (*it == _gameWidget || *it == _conversationWidget)
				continue;

			(*it)->setVisible(false);
		}
	}

	_gameWidget->clearState();

	// Fake mouse move event to update the cursor.
	Common::Event event;
	event.type = Common::EVENT_MOUSEMOVE;
	event.mouse = _game.getEngine().getEventManager()->getMousePos();
	_gameWidget->handleEvent(event);

	_gameWidget->markDirty(GameWidget::DIRTY_AFTER_SCENE_CHANGE);
	_gameWidget->update(*_screen); // Force immediate update.
}

class InventoryAnimationDecoderCallback : public AnimationDecoderCallback {
public:
	InventoryAnimationDecoderCallback(GameScreen &gui) : _gui(gui) {}
	void onFrame(int frameNo, Graphics::Surface &surface) override;
	void onPaletteUpdated(byte palette[PALETTE_SIZE]) override;
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
	void onFrame(int frameNo, Graphics::Surface &surface) override;
	void onPaletteUpdated(byte palette[PALETTE_SIZE]) override;
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

void GameScreen::updateStatusBarText(const Common::String &entity, bool inventory) {
	const bool hasPrevPickedItem = !_currentPickedItem.empty();
	const bool hasCurrentItem = !entity.empty();

	if (!hasPrevPickedItem && !hasCurrentItem) {
		_statusBarWidget->setText(Common::String());
		return;
	}

	HardcodedStrings::StringType actionStringType = HardcodedStrings::LOOK;

	if (inventory) {
		switch (_currentAction) {
		case ActionInfo::Use:
			actionStringType = HardcodedStrings::USE;
			break;
		case ActionInfo::Look:
		default:
			actionStringType = HardcodedStrings::LOOK;
			break;
		}
	} else {
		switch (_currentAction) {
		case ActionInfo::Look:
		default:
			actionStringType = HardcodedStrings::LOOK;
			break;
		case ActionInfo::Walk:
			actionStringType = HardcodedStrings::WALK;
			break;
		case ActionInfo::Talk:
			actionStringType = HardcodedStrings::TALK;
			break;
		case ActionInfo::Use:
			actionStringType = HardcodedStrings::USE;
			break;
		case ActionInfo::PickUp:
			actionStringType = HardcodedStrings::PICKUP;
			break;
		}
	}

	Common::String text = _game.getAssets().getHardcodedStrings().getString(actionStringType);

	if (hasPrevPickedItem)
		text += " " + _currentPickedItem;
	if (hasCurrentItem)
		text += " " + entity;

	_statusBarWidget->setText(text);
}

void GameScreen::onInventoryChanged() {
	_inventoryWidget->markDirty();
}

void GameScreen::onButtonClicked(ButtonWidget *button) {
	const int buttonId = button->getId();
	if (buttonId <= BUTTON_PICKUP) {
		const ActionInfo::Action actions[] = {ActionInfo::Walk, ActionInfo::Talk, ActionInfo::Look, ActionInfo::Use, ActionInfo::PickUp};
		_currentAction = actions[buttonId];
		_currentPickedItem.clear();
	} else if (buttonId == BUTTON_SCROLL_LEFT) {
		_game.getGameData().getInventory().scrollLeft();
	} else if (buttonId == BUTTON_SCROLL_RIGHT) {
		_game.getGameData().getInventory().scrollRight();
	}
}

void GameScreen::onInventoryItemHovered(InventoryWidget *, int posInWidget) {
	if (posInWidget == -1) {
		updateStatusBarText(Common::String(), true);
	} else {
		const Common::String &item = _game.getGameData().getInventory().getItems()[posInWidget];
		updateStatusBarText(item, true);
	}
}

void GameScreen::onInventoryItemClicked(InventoryWidget *, int posInWidget) {
	// Position in widget should match the position in inventory.
	const Common::String &item = _game.getGameData().getInventory().getItems()[posInWidget];

	if (_currentAction == ActionInfo::Use) {
		if (_currentPickedItem.empty()) {
			// Inventory items ending with '[' aren't supposed to be combined (e.g. Fisher's mask).
			if (item.lastChar() == '[')
				_game.startActionSection(ActionInfo::Use, item);
			else
				_currentPickedItem = item;
		} else {
			_game.startActionSection(ActionInfo::Use, _currentPickedItem, item);
			_currentPickedItem.clear();
		}
	} else {
		_game.startActionSection(ActionInfo::Look, item);
	}
}

void GameScreen::onGameDoorClicked(GameWidget *, Door *door) {
	if (!_currentPickedItem.empty()) {
		_game.startActionSection(_currentAction, _currentPickedItem, door->_name);
		_currentPickedItem.clear();
		return;
	}

	if (!_game.startActionSection(_currentAction, door->_name) && _currentAction == ActionInfo::Walk && door->_destSceneId != 0) {
		if (door->allowsImplicitSceneChange())
			_game.changeScene(door->_destSceneId, _game.getGameData()._partB);
	}
}

void GameScreen::onGameStaticClicked(GameWidget *, Static *stat) {
	if (_currentAction == ActionInfo::Use) {
		if (_currentPickedItem.empty()) {
			if (stat->isCombinable())
				_currentPickedItem = stat->_name;
			else
				_game.startActionSection(ActionInfo::Use, stat->_name);
		} else {
			_game.startActionSection(_currentAction, _currentPickedItem, stat->_name);
			_currentPickedItem.clear();
		}
	} else {
		if (!_game.startActionSection(_currentAction, stat->_name)) {
			if (_currentAction == ActionInfo::PickUp && stat->allowsImplicitPickup()) {
				Common::String inventoryName(stat->_name);
				inventoryName.setChar('`', 0);

				_game.getGameData().getInventory().addItem(inventoryName);
				stat->_active = 0;
				_game.getRoom().drawStatic(stat);
			}
		}
	}
}

void GameScreen::onGameEntityHovered(GameWidget *, const Common::String &entity) {
	updateStatusBarText(entity, false);
}

}
