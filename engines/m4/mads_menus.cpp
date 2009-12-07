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

#include "m4/mads_menus.h"
#include "m4/m4.h"

namespace M4 {

#define REX_MENUSCREEN 990
#define PHANTOM_MENUSCREEN 920
#define DRAGON_MENUSCREEN 922

static Common::Point rexMenuItemPosList[6] = {
	Common::Point(12, 68), Common::Point(12, 87), Common::Point(12, 107),
	Common::Point(184, 75), Common::Point(245, 75), Common::Point(184, 99)
};

static Common::Point dragonMenuItemPosList[6] = {
	Common::Point(46, 187), Common::Point(92, 187), Common::Point(138, 187),
	Common::Point(184, 187), Common::Point(230, 187), Common::Point(276, 187)
};

#define DRAGON_MENU_BUTTON_W = 45
#define DRAGON_MENU_BUTTON_H = 11

RexMainMenuView::RexMainMenuView(M4Engine *vm):
		View(vm, Common::Rect(0, 0, vm->_screen->width(), vm->_screen->height())) {

	_screenType = VIEWID_MAINMENU;
	_screenFlags.get = SCREVENT_ALL;

	_delayTimeout = 0;
	_menuItem = NULL;
	_menuItemIndex = 0;
	_frameIndex = 0;
	_highlightedIndex = -1;
	_skipFlag = false;

	// Load the background for the Rex Nebular game
	_bgSurface = new M4Surface(width(), MADS_SURFACE_HEIGHT);
	_bgSurface->loadBackground(REX_MENUSCREEN, &_bgPalData);
	_vm->_palette->addRange(_bgPalData);
	_bgSurface->translate(_bgPalData);

	int row = (height() - MADS_SURFACE_HEIGHT) / 2;
	_bgSurface->copyTo(this, 0, row);

	// Add in the bounding lines for the background
	setColor(2);
	hLine(0, width() - 1, row - 1);
	hLine(0, width() - 1, height() - row + 1);
}

RexMainMenuView::~RexMainMenuView() {
	if (_menuItem)
		delete _menuItem;

	_vm->_palette->deleteRange(_bgPalData);

	delete _bgPalData;
	delete _bgSurface;

	for (uint i = 0; i < _itemPalData.size(); ++i) {
		_vm->_palette->deleteRange(_itemPalData[i]);
		delete _itemPalData[i];
	}
}

bool RexMainMenuView::onEvent(M4EventType eventType, int param, int x, int y, bool &captureEvents) {
	// Handle keypresses - these can be done at any time, even when the menu items are being drawn
	if (eventType == KEVENT_KEY) {
		switch (param) {
		case Common::KEYCODE_ESCAPE:
		case Common::KEYCODE_F6:
			handleAction(EXIT);
			break;

		case Common::KEYCODE_F1:
			handleAction(START_GAME);
			break;

		case Common::KEYCODE_F2:
			handleAction(RESUME_GAME);
			break;

		case Common::KEYCODE_F3:
			handleAction(SHOW_INTRO);
			break;

		case Common::KEYCODE_F4:
			handleAction(CREDITS);
			break;

		case Common::KEYCODE_F5:
			handleAction(QUOTES);
			break;

		case Common::KEYCODE_s:
			// Goodness knows why, but Rex has a key to restart the menuitem animations

			// Delete the current menu items
			if (_menuItem)
				delete _menuItem;

			_vm->_palette->deleteRange(_bgPalData);
			delete _bgPalData;
			for (uint i = 0; i < _itemPalData.size(); ++i) {
				_vm->_palette->deleteRange(_itemPalData[i]);
				delete _itemPalData[i];
			}
			_itemPalData.clear();

			// Reload the background surface, and restart the animation
			_bgSurface->loadBackground(REX_MENUSCREEN, &_bgPalData);
			_vm->_palette->addRange(_bgPalData);
			_bgSurface->translate(_bgPalData);

			_menuItemIndex = 0;
			_skipFlag = false;
			_menuItem = NULL;
			_vm->_mouse->cursorOff();
			break;

		default:
			// Any other key skips the menu animation
			_skipFlag = true;
			return false;
		}

		return true;
	}

	int row = (height() - MADS_SURFACE_HEIGHT) / 2;
	int menuIndex;

	switch (eventType) {
	case MEVENT_LEFT_CLICK:
	case MEVENT_LEFT_DRAG:
		if (_vm->_mouse->getCursorOn()) {
			menuIndex = getHighlightedItem(x, y);
			if (menuIndex != _highlightedIndex) {
				_bgSurface->copyTo(this, 0, row);

				_highlightedIndex = menuIndex;
				if (_highlightedIndex != -1) {
					M4Sprite *spr = _menuItem->getFrame(_highlightedIndex);
					const Common::Point &pt = rexMenuItemPosList[_highlightedIndex];
					spr->copyTo(this, pt.x, row + pt.y, 0);
				}
			}
		} else {
			// Skip the menu animation
			_skipFlag = true;
		}
		return true;

	case MEVENT_LEFT_RELEASE:
		if (_highlightedIndex != -1)
			handleAction((MadsGameAction) _highlightedIndex);
		return true;

	default:
		break;
	}

	return false;
}

void RexMainMenuView::updateState() {
	char resName[20];
	Common::SeekableReadStream *data;
	int row = (height() - MADS_SURFACE_HEIGHT) / 2;
	int itemSize;

	uint32 currTime = g_system->getMillis();
	if (currTime < _delayTimeout)
		return;
	_delayTimeout = currTime + MADS_MENU_ANIM_DELAY;

	// Rex Nebular handling to cycle through the animated display of the menu items
	if (_menuItemIndex == 7)
		return;

	// If the user has chosen to skip the menu animation, show the menu immediately
	if (_skipFlag && !_vm->_mouse->getCursorOn()) {
		// Clear any pending animation
		_bgSurface->copyTo(this, 0, row);
		// Quickly loop through all the menuitems to display each's final frame
		while (_menuItemIndex < 7) {

			if (_menuItem) {
				// Draw the final frame of the menuitem
				M4Sprite *spr = _menuItem->getFrame(0);
				itemSize = _menuItem->getFrame(0)->height();
				spr->copyTo(this, rexMenuItemPosList[_menuItemIndex - 1].x,
					rexMenuItemPosList[_menuItemIndex - 1].y + row + (itemSize / 2) - (spr->height() / 2), 0);

				delete _menuItem;
				copyTo(_bgSurface, Common::Rect(0, row, width(), row + MADS_SURFACE_HEIGHT), 0, 0);
			}

			// Get the next sprite set
			sprintf(resName, "RM%dA%d.SS", REX_MENUSCREEN, ++_menuItemIndex);
			data = _vm->res()->get(resName);
			_menuItem = new SpriteAsset(_vm, data, data->size(), resName);
			_vm->res()->toss(resName);

			// Slot it into available palette space
			RGBList *palData = _menuItem->getRgbList();
			_vm->_palette->addRange(palData);
			_menuItem->translate(palData, true);
			_itemPalData.push_back(palData);
		}

		_vm->_mouse->cursorOn();
		return;
	}

	if ((_menuItemIndex == 0) || (_frameIndex == 0)) {
		// Get the next menu item
		if (_menuItem) {
			delete _menuItem;

			// Copy over the current display surface area to the background, so the final frame
			// of the previous menuitem should be kept on the screen
			copyTo(_bgSurface, Common::Rect(0, row, width(), row + MADS_SURFACE_HEIGHT), 0, 0);
		}

		// Get the next menuitem resource
		sprintf(resName, "RM%dA%d.SS", REX_MENUSCREEN, ++_menuItemIndex);
		data = _vm->res()->get(resName);
		_menuItem = new SpriteAsset(_vm, data, data->size(), resName);
		_vm->res()->toss(resName);

		// Slot it into available palette space
		RGBList *palData = _menuItem->getRgbList();
		_vm->_palette->addRange(palData);
		_menuItem->translate(palData, true);
		_itemPalData.push_back(palData);

		_frameIndex = _menuItem->getCount() - 1;

		// If the final resource is now loaded, which contains the highlighted versions of
		// each menuitem, then the startup animation is complete
		if (_menuItemIndex == 7) {
			_vm->_mouse->cursorOn();
			return;
		}
	} else {
		--_frameIndex;
	}

	// Move to the next menuitem frame

	itemSize = _menuItem->getFrame(0)->height();

	_bgSurface->copyTo(this, 0, row);
	M4Sprite *spr = _menuItem->getFrame(_frameIndex);
	spr->copyTo(this, rexMenuItemPosList[_menuItemIndex - 1].x, rexMenuItemPosList[_menuItemIndex - 1].y +
		row + (itemSize / 2) - (spr->height() / 2), 0);
}

int RexMainMenuView::getHighlightedItem(int x, int y) {
	y -= (height() - MADS_SURFACE_HEIGHT) / 2;

	for (int index = 0; index < 6; ++index) {
		const Common::Point &pt = rexMenuItemPosList[index];
		M4Sprite *spr = _menuItem->getFrame(index);

		if ((x >= pt.x) && (y >= pt.y) && (x < (pt.x + spr->width())) && (y < (pt.y + spr->height())))
			return index;
	}

	return -1;
}

void RexMainMenuView::handleAction(MadsGameAction action) {
	M4Engine *vm = _vm;
	vm->_mouse->cursorOff();
	vm->_viewManager->deleteView(this);

	switch (action) {
	case START_GAME:
	case RESUME_GAME:
		// Load a sample starting scene - note that, currently, calling loadScene automatically
		// removes this menu screen from being displayed
		vm->_mouse->cursorOn();
		vm->_scene->show();
		vm->_scene->loadScene(101);
		return;

	case SHOW_INTRO:
		vm->_viewManager->showAnimView("@rexopen");
		break;

	case CREDITS:
		vm->_viewManager->showTextView("credits");
		return;

	case QUOTES:
		vm->_viewManager->showTextView("quotes");
		return;

	case EXIT:
		{
			// When the Exit action is done from the menu, show one of two possible advertisements

			// Activate the scene display with the specified scene
			bool altAdvert = vm->_random->getRandomNumber(1000) >= 500;
			vm->_scene->loadScene(altAdvert ? 995 : 996);
			vm->_viewManager->addView(vm->_scene);

			vm->_viewManager->refreshAll();
			vm->delay(10000);

			vm->_events->quitFlag = true;
			return;
		}
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------------------

MadsMainMenuView::MadsMainMenuView(M4Engine *vm):
		View(vm, Common::Rect(0, 0, vm->_screen->width(), vm->_screen->height())) {

}

bool MadsMainMenuView::onEvent(M4EventType eventType, int param, int x, int y, bool &captureEvents) {
	return false;
}

void MadsMainMenuView::updateState() {
	// TODO: Implement me
}

//--------------------------------------------------------------------------

DragonMainMenuView::DragonMainMenuView(M4Engine *vm):
		View(vm, Common::Rect(0, 0, vm->_screen->width(), vm->_screen->height())) {

	_screenType = VIEWID_MAINMENU;
	_screenFlags.get = SCREVENT_ALL;

	_delayTimeout = 0;
	_menuItem = NULL;
	_menuItemIndex = 0;
	_frameIndex = 0;
	_highlightedIndex = -1;
	_skipFlag = false;

	// Load the background for the Dragonsphere game
	this->loadBackground(942, &_bgPalData);
	_vm->_palette->addRange(_bgPalData);
	this->translate(_bgPalData);
}

DragonMainMenuView::~DragonMainMenuView() {
	//if (_menuItem)
	//	delete _menuItem;

	_vm->_palette->deleteRange(_bgPalData);

	delete _bgPalData;

	for (uint i = 0; i < _itemPalData.size(); ++i) {
		_vm->_palette->deleteRange(_itemPalData[i]);
		delete _itemPalData[i];
	}
}

bool DragonMainMenuView::onEvent(M4EventType eventType, int param, int x, int y, bool &captureEvents) {
	char resName[20];
	Common::SeekableReadStream *data;

	// Handle keypresses - these can be done at any time, even when the menu items are being drawn
	if (eventType == KEVENT_KEY) {
		switch (param) {
		case Common::KEYCODE_ESCAPE:
		case Common::KEYCODE_F6:
			handleAction(EXIT);
			break;

		case Common::KEYCODE_F1:
			handleAction(START_GAME);
			break;

		case Common::KEYCODE_F2:
			handleAction(RESUME_GAME);
			break;

		case Common::KEYCODE_F3:
			handleAction(SHOW_INTRO);
			break;

		case Common::KEYCODE_F4:
			handleAction(CREDITS);
			break;

		default:
			// Any other key skips the menu animation
			_skipFlag = true;
			return false;
		}

		return true;
	}

	int menuIndex;

	switch (eventType) {
	case MEVENT_LEFT_CLICK:
	case MEVENT_LEFT_DRAG:
		if (_vm->_mouse->getCursorOn()) {
			menuIndex = getHighlightedItem(x, y);
			if (menuIndex != _highlightedIndex) {

				_highlightedIndex = menuIndex;
				if (_highlightedIndex != -1) {
					sprintf(resName, "MAIN%d.SS", menuIndex);
					data = _vm->res()->get(resName);
					_menuItem = new SpriteAsset(_vm, data, data->size(), resName);
					_vm->res()->toss(resName);

					M4Sprite *spr = _menuItem->getFrame(1);
					spr->copyTo(this, spr->xOffset - 25, spr->yOffset - spr->height());
				}
			}
		} else {
			// Skip the menu animation
			_skipFlag = true;
		}
		return true;

	case MEVENT_LEFT_RELEASE:
		if (_highlightedIndex != -1)
			handleAction((MadsGameAction) _highlightedIndex);
		return true;

	default:
		break;
	}

	return false;
}

void DragonMainMenuView::updateState() {
	char resName[20];
	Common::SeekableReadStream *data;
	RGBList *palData;
	M4Sprite *spr;

	if (_menuItemIndex == 6)
		return;

	while (_menuItemIndex < 6) {
		sprintf(resName, "MAIN%d.SS", _menuItemIndex);
		data = _vm->res()->get(resName);
		_menuItem = new SpriteAsset(_vm, data, data->size(), resName);
		_vm->res()->toss(resName);

		// Slot it into available palette space
		palData = _menuItem->getRgbList();
		_vm->_palette->addRange(palData);
		_menuItem->translate(palData, true);
		_itemPalData.push_back(palData);

		spr = _menuItem->getFrame(0);
		spr->copyTo(this, spr->xOffset - 25, spr->yOffset - spr->height());

		if (_menuItemIndex != 5)
			delete _menuItem;
		_menuItemIndex++;
	}

	// Sphere
	sprintf(resName, "RM920X0.SS");
	data = _vm->res()->get(resName);
	_menuItem = new SpriteAsset(_vm, data, data->size(), resName);
	_vm->res()->toss(resName);

	// Slot it into available palette space
	palData = _menuItem->getRgbList();
	_vm->_palette->addRange(palData);
	_menuItem->translate(palData, true);
	_itemPalData.push_back(palData);

	spr = _menuItem->getFrame(0);					// empty sphere
	spr->copyTo(this, spr->xOffset - 75, spr->yOffset - spr->height());
	spr = _menuItem->getFrame(1);					// dragon inside sphere
	spr->copyTo(this, spr->xOffset - 75, spr->yOffset - spr->height());

	// Dragonsphere letters
	sprintf(resName, "RM920X3.SS");
	data = _vm->res()->get(resName);
	_menuItem = new SpriteAsset(_vm, data, data->size(), resName);
	_vm->res()->toss(resName);

	// Slot it into available palette space
	palData = _menuItem->getRgbList();
	_vm->_palette->addRange(palData);
	_menuItem->translate(palData, true);
	_itemPalData.push_back(palData);

	spr = _menuItem->getFrame(1);
	// FIXME: We assume that the transparent color is the color of the top left pixel
	byte *transparentColor = spr->getBasePtr(0, 0);
	spr->copyTo(this, spr->xOffset - 140, spr->yOffset - spr->height(), (int)*transparentColor);

	_vm->_mouse->cursorOn();
}

int DragonMainMenuView::getHighlightedItem(int x, int y) {
	y -= (height() - MADS_SURFACE_HEIGHT) / 2;

	for (int index = 0; index < 6; ++index) {
		const Common::Point &pt = dragonMenuItemPosList[index];
		M4Sprite *spr = _menuItem->getFrame(0);

		if ((x >= pt.x - 25) && (y >= pt.y - spr->height()) && (x < (pt.x - 25 + spr->width())) && (y < (pt.y)))  {
			printf("x = %d, y = %d, index = %d\n", x, y, index);
			return index;
		}
	}

	return -1;
}

void DragonMainMenuView::handleAction(MadsGameAction action) {
	M4Engine *vm = _vm;
	vm->_mouse->cursorOff();
	vm->_viewManager->deleteView(this);

	switch (action) {
	case START_GAME:
	case RESUME_GAME:
		// Load a sample starting scene - note that, currently, calling loadScene automatically
		// removes this menu screen from being displayed
		vm->_mouse->cursorOn();
		vm->_viewManager->addView(vm->_scene);
		vm->_scene->loadScene(101);
		return;

	case SHOW_INTRO:
		vm->_viewManager->showAnimView("@dragon");
		break;

	case CREDITS:
		vm->_viewManager->showTextView("credits");
		return;

	case EXIT:
		vm->_events->quitFlag = true;
		return;
		break;
	default:
		break;
	}
}

}
