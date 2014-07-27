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

	events.setEventTarget(this);
	events.hideCursor();

	while (!_breakFlag && !_vm->shouldQuit()) {
		if (_redrawFlag) {
			scene.drawElements(_vm->_game->_fx, _vm->_game->_fx);
			_redrawFlag = false;
		}

		_vm->_events->waitForNextFrame();
		_vm->_game->_fx = kTransitionNone;
		doFrame();
	}

	events.setEventTarget(nullptr);
}

void MenuView::display() {
	_vm->_palette->resetGamePalette(4, 8);

	FullScreenDialog::display();
}

/*------------------------------------------------------------------------*/

MainMenu::MainMenu(MADSEngine *vm): MenuView(vm) {
	Common::fill(&_menuItems[0], &_menuItems[7], (SpriteAsset *)nullptr);
	Common::fill(&_menuItemIndexes[0], &_menuItemIndexes[7], -1);
	_delayTimeout = 0;
	_menuItemIndex = -1;
	_frameIndex = 0;
	_skipFlag = false;
	_highlightedIndex = -1;
}

MainMenu::~MainMenu() {
}

void MainMenu::display() {
	MenuView::display();
	Scene &scene = _vm->_game->_scene;

	// Load each of the menu item assets and add to the scene sprites list
	for (int i = 0; i < 7; ++i) {
		Common::String spritesName = Resources::formatName(NEBULAR_MENUSCREEN,
			'A', i + 1, EXT_SS, "");
		_menuItems[i] = new SpriteAsset(_vm, spritesName, 0);
		_menuItemIndexes[i] = scene._sprites.add(_menuItems[i]);

		// Register the menu item area in the screen objects
		MSprite *frame0 = _menuItems[i]->getFrame(0);
		Common::Point pt(frame0->_offset.x - (frame0->w / 2),
			frame0->_offset.y - (frame0->h / 2));
		_vm->_game->_screenObjects.add(
			Common::Rect(pt.x, pt.y, pt.x + frame0->w, pt.y + frame0->h),
			LAYER_GUI, CAT_COMMAND, i);
	}

	// Set the cursor for when it's shown
	_vm->_events->setCursor(CURSOR_ARROW);
}

void MainMenu::doFrame() {
	Scene &scene = _vm->_game->_scene;

	// Delay between animation frames on the menu
	uint32 currTime = g_system->getMillis();
	if (currTime < _delayTimeout)
		return;
	_delayTimeout = currTime + MADS_MENU_ANIM_DELAY;

	// If we've alerady reached the end of the menuitem animation, exit immediately
	if (_menuItemIndex == 6)
		return;

	// If the user has chosen to skip the animation, show the full menu immediately
	if (_skipFlag && _menuItemIndex >= 0) {
		// Quickly loop through all the menu items to display each's final frame		
		for (; _menuItemIndex < 6; ++_menuItemIndex) {
			// Draw the final frame of the menuitem
			_frameIndex = 0;
			addSpriteSlot();
		}

		_vm->_events->showCursor();
	} else {
		if ((_menuItemIndex == -1) || (_frameIndex == 0)) {
			if (++_menuItemIndex == 6) {
				// Reached end of display animation
				_vm->_events->showCursor();
				return;
			}

			_frameIndex = _menuItems[_menuItemIndex]->getCount() - 1;
		} else {
			--_frameIndex;
		}

		// Move to the next menuitem frame
		addSpriteSlot();
	}
}

void MainMenu::addSpriteSlot() {
	Scene &scene = _vm->_game->_scene;
	SpriteSlots &spriteSlots = scene._spriteSlots;
	spriteSlots.deleteTimer(_menuItemIndex);

	SpriteAsset *menuItem = _menuItems[_menuItemIndex];
	MSprite *spr = menuItem->getFrame(_frameIndex);

	SpriteSlot &slot = spriteSlots[spriteSlots.add()];
	slot._flags = IMG_UPDATE;
	slot._seqIndex = _menuItemIndex;
	slot._spritesIndex = _menuItemIndexes[_menuItemIndex];
	slot._frameNumber = _frameIndex + 1;
	slot._position = spr->_offset;
	slot._depth = 1;
	slot._scale = 100;

	_redrawFlag = true;
}

bool MainMenu::onEvent(Common::Event &event) {
	Scene &scene = _vm->_game->_scene;

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
			// Restart the animation
			_menuItemIndex = -1;
			for (int i = 0; i < 6; ++i)
				scene._spriteSlots.deleteTimer(i);

			_skipFlag = false;
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
	/*
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
	*/
	return false;
}


int MainMenu::getHighlightedItem(int x, int y) {
	/*
	y -= MADS_MENU_Y;

	for (int index = 0; index < 6; ++index) {
		const Common::Point &pt = _itemPosList[index];
		MSprite *spr = _menuItem->getFrame(index);

		if ((x >= pt.x) && (y >= pt.y) && (x < (pt.x + spr->w)) && (y < (pt.y + spr->h)))
			return index;
	}
	*/
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
