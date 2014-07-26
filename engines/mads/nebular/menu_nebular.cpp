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
 */

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/mads.h"
#include "mads/resources.h"
#include "mads/screen.h"
#include "mads/nebular/menu_nebular.h"

namespace MADS {

namespace Nebular {

#define NEBULAR_MENUSCREEN 990
#define MADS_MENU_Y ((MADS_SCREEN_HEIGHT - MADS_SCENE_HEIGHT) / 2)
#define MADS_MENU_ANIM_DELAY 70

MenuView::MenuView(MADSEngine *vm) : FullScreenDialog(vm) {
	_breakFlag = false;
	_redrawFlag = true;
	_palFlag = false;
}

void MenuView::show() {
	Scene &scene = _vm->_game->_scene;	
	EventsManager &events = *_vm->_events;
	display();

	events.hideCursor();

	while (!_breakFlag && !_vm->shouldQuit()) {
		handleEvents();

		if (_redrawFlag) {
			scene.drawElements(_vm->_game->_fx, _vm->_game->_fx);
			_redrawFlag = false;
		}

		_vm->_events->waitForNextFrame();
		_vm->_game->_fx = kTransitionNone;
		doFrame();
	}
}

void MenuView::display() {
	_vm->_palette->resetGamePalette(4, 8);

	FullScreenDialog::display();
}

void MenuView::handleEvents() {
	Common::Event event;

	while (g_system->getEventManager()->pollEvent(event))
		onEvent(event);
}

/*------------------------------------------------------------------------*/

MainMenu::MainMenu(MADSEngine *vm): MenuView(vm) {
	_itemPosList[0] = Common::Point(12, 68);
	_itemPosList[1] = Common::Point(12, 87);
	_itemPosList[2] = Common::Point(12, 107);
	_itemPosList[3] = Common::Point(184, 75);
	_itemPosList[4] = Common::Point(245, 75);
	_itemPosList[5] = Common::Point(184, 99);

	_delayTimeout = 0;
	_menuItem = NULL;
	_menuItemIndex = 0;
	_frameIndex = 0;
	_highlightedIndex = -1;
	_skipFlag = false;
}

MainMenu::~MainMenu() {
	if (_menuItem)
		delete _menuItem;
}

bool MainMenu::onEvent(Common::Event &event) {
	// Handle keypresses - these can be done at any time, even when the menu items are being drawn
	if (event.type == Common::EVENT_KEYDOWN) {
		switch (event.kbd.keycode) {
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

		case Common::KEYCODE_s: {
			// Goodness knows why, but Rex has a key to restart the menuitem animations

			// Delete the current menu items
			if (_menuItem)
				delete _menuItem;
			/*
			_vm->_palette->deleteRange(_bgPalData);
			delete _bgPalData;
			for (uint i = 0; i < _itemPalData.size(); ++i) {
				_vm->_palette->deleteRange(_itemPalData[i]);
				delete _itemPalData[i];
			}
			_itemPalData.clear();
			*/
			// Restart the animation
			_menuItemIndex = 0;
			_skipFlag = false;
			_menuItem = NULL;
			_vm->_events->hideCursor();
			break;
		}

		default:
			// Any other key skips the menu animation
			_skipFlag = true;
			return false;
		}

		return true;
	}

	int menuIndex;

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		if (_vm->_events->isCursorVisible()) {
			menuIndex = getHighlightedItem(event.mouse.x, event.mouse.y);

			if (menuIndex != _highlightedIndex) {
//				_bgSurface->copyTo(this, Common::Point(0, MADS_MENU_Y));

				_highlightedIndex = menuIndex;
				if (_highlightedIndex != -1) {
					MSprite *spr = _menuItem->getFrame(_highlightedIndex);
					const Common::Point &pt = _itemPosList[_highlightedIndex];
					spr->copyTo(&_vm->_screen, Common::Point(pt.x, MADS_MENU_Y + pt.y));
				}
			}
		} else {
			// Skip the menu animation
			_skipFlag = true;
		}
		return true;

	case Common::EVENT_LBUTTONUP:
		if (_highlightedIndex != -1)
			handleAction((MADSGameAction)_highlightedIndex);
		return true;

	default:
		break;
	}

	return false;
}

void MainMenu::doFrame() {
	int itemSize;

	// Delay between animation frames on the menu
	uint32 currTime = g_system->getMillis();
	if (currTime < _delayTimeout)
		return;
	_delayTimeout = currTime + MADS_MENU_ANIM_DELAY;

	// If we've alerady reached the end of the menuitem animation, exit immediately
	if (_menuItemIndex == 7)
		return;

	// If the user has chosen to skip the animation, show the full menu immediately
	if (_skipFlag && !_vm->_events->isCursorVisible()) {
		// Clear any pending animation
//		_savedSurface.copyTo(&_vm->_screen, Common::Point(0, MADS_MENU_Y));
		
		// Quickly loop through all the menu items to display each's final frame
		while (_menuItemIndex < 7) {
			if (_menuItem) {
				// Draw the final frame of the menuitem
				MSprite *spr = _menuItem->getFrame(0);
				itemSize = _menuItem->getFrame(0)->h;
				spr->copyTo(&_vm->_screen, Common::Point(_itemPosList[_menuItemIndex - 1].x,
					_itemPosList[_menuItemIndex - 1].y + MADS_MENU_Y + (itemSize / 2) - (spr->h / 2)));

				delete _menuItem;
				//copyTo(_bgSurface, Common::Rect(0, row, width(), row + MADS_SCENE_HEIGHT), 0, 0);
			}

			// Get the next sprite set
			Common::String spritesName = Resources::formatName(NEBULAR_MENUSCREEN, 
				'A', ++_menuItemIndex, EXT_SS, "");
			_menuItem = new SpriteAsset(_vm, spritesName, 0);

			// Slot it into available palette space
/*
			RGBList *palData = _menuItem->getRgbList();
			_vm->_palette->addRange(palData);
			_menuItem->translate(palData, true);
			_itemPalData.push_back(palData);
*/
		}

		_vm->_events->showCursor();
		return;
	}

	if ((_menuItemIndex == 0) || (_frameIndex == 0)) {
		// Get the next menu item
		if (_menuItem) {
			delete _menuItem;

			// Copy over the current display surface area to the background, so the final frame 
			// of the previous menuitem should be kept on the screen
//			copyTo(_bgSurface, Common::Rect(0, row, width(), row + MADS_SCENE_HEIGHT), 0, 0);
		}

		// Get the next menuitem resource
		Common::String spritesName = Resources::formatName(NEBULAR_MENUSCREEN,
			'A', ++_menuItemIndex, EXT_SS, "");
		_menuItem = new SpriteAsset(_vm, spritesName, 0);
		_frameIndex = _menuItem->getCount() - 1;

		// If the final resource is now loaded, which contains the highlighted versions of 
		// each menuitem, then the startup animation is complete
		if (_menuItemIndex == 7) {
			_vm->_events->showCursor();
			return;
		}
	} else {
		--_frameIndex;
	}

	// Move to the next menuitem frame

	itemSize = _menuItem->getFrame(0)->h;

	//_bgSurface->copyTo(this, 0, row);
	MSprite *spr = _menuItem->getFrame(_frameIndex);
	
	spr->copyTo(&_vm->_screen, 
		Common::Point(_itemPosList[_menuItemIndex - 1].x, 
			_itemPosList[_menuItemIndex - 1].y + MADS_MENU_Y + 
			(itemSize / 2) - (spr->h / 2)));
}

int MainMenu::getHighlightedItem(int x, int y) {
	y -= MADS_MENU_Y;

	for (int index = 0; index < 6; ++index) {
		const Common::Point &pt = _itemPosList[index];
		MSprite *spr = _menuItem->getFrame(index);

		if ((x >= pt.x) && (y >= pt.y) && (x < (pt.x + spr->w)) && (y < (pt.y + spr->h)))
			return index;
	}

	return -1;
}

void MainMenu::handleAction(MADSGameAction action) {
	_vm->_events->hideCursor();
	/*
	switch (action) {
	case START_GAME:
	case RESUME_GAME:
		// Load a sample starting scene - note that, currently, calling loadScene automatically
		// removes this menu screen from being displayed
		_vm->_mouse->cursorOn();
		_vm->_viewManager->addView(_vm->_scene);
		_vm->_scene->loadScene(101);
		return;

	case SHOW_INTRO:
		_vm->_viewManager->showAnimView("@rexopen");
		break;

	case CREDITS:
		_vm->_viewManager->showTextView("credits");
		return;

	case QUOTES:
		_vm->_viewManager->showTextView("quotes");
		return;

	case EXIT:
	{
					// When the Exit action is done from the menu, show one of two possible advertisements

					// Activate the scene display with the specified scene
					bool altAdvert = _vm->_random->getRandomNumber(1000) >= 500;
					_vm->_scene->loadScene(altAdvert ? 995 : 996);
					_vm->_viewManager->addView(_vm->_scene);

					_vm->_viewManager->refreshAll();
					_vm->delay(10000);

					_vm->_events->quitFlag = true;
					return;
	}
		break;
	default:
		break;
	}
	*/
}

} // End of namespace Nebular

} // End of namespace MADS
