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
#include "mutationofjb/widgets/widget.h"
#include "mutationofjb/widgets/inventorywidget.h"
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
	INVENTORY_ITEMS_LINES = 5
};


Gui::Gui(Game &game, Graphics::Screen *screen)
	: _game(game),
	_screen(screen) {}

Gui::~Gui() {
	for (Common::Array<Widget *>::iterator it = _widgets.begin(); it != _widgets.end(); ++it) {
		delete *it;
	}
}

Game &Gui::getGame() {
	return _game;
}

bool Gui::init() {
	if (!loadInventoryList()) {
		return false;
	}

	if (!loadInventoryGfx()) {
		return false;
	}

	if (!loadHudGfx()) {
		return false;
	}

	_game.getGameData().getInventory().setObserver(this);

	// Init widgets.
	_inventoryWidget = new InventoryWidget(*this, _inventoryItems, _inventorySurfaces);
	_widgets.push_back(_inventoryWidget);

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
		_widgets.push_back(button);
	}

	return true;
}

void Gui::markDirty() {
	for (Common::Array<Widget *>::iterator it = _widgets.begin(); it != _widgets.end(); ++it) {
		(*it)->markDirty();
	}
}

void Gui::handleEvent(const Common::Event &event) {
	for (Common::Array<Widget *>::iterator it = _widgets.begin(); it != _widgets.end(); ++it) {
		(*it)->handleEvent(event);
	}
}

void Gui::update() {
	for (Common::Array<Widget *>::iterator it = _widgets.begin(); it != _widgets.end(); ++it) {
		(*it)->update(*_screen);
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

class HudAnimationDecoderCallback : public AnimationDecoderCallback {
public:
	HudAnimationDecoderCallback(Gui &gui) : _gui(gui) {}
	virtual void onFrame(int frameNo, Graphics::Surface &surface) override;
	virtual void onPaletteUpdated(byte palette[PALETTE_SIZE]) override;
private:
	Gui &_gui;
};

void HudAnimationDecoderCallback::onPaletteUpdated(byte [PALETTE_SIZE]) {
}

void HudAnimationDecoderCallback::onFrame(int frameNo, Graphics::Surface &surface) {
	if (frameNo == 0 || frameNo == 1 || frameNo == 3) {
		Graphics::Surface outSurface;
		outSurface.copyFrom(surface);
		_gui._hudSurfaces.push_back(outSurface);
	}
}

bool Gui::loadHudGfx() {
	AnimationDecoder decoder("room0.dat");
	HudAnimationDecoderCallback callback(*this);
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

void Gui::onInventoryChanged() {
	_inventoryWidget->markDirty();
}

void Gui::onButtonClicked(ButtonWidget *button) {
	const int buttonId = button->getId();
	if (buttonId <= BUTTON_PICKUP) {
		const ActionInfo::Action actions[] = {ActionInfo::Walk, ActionInfo::Talk, ActionInfo::Look, ActionInfo::Use, ActionInfo::PickUp};
		_game.setCurrentAction(actions[buttonId]);
	}
}

}
