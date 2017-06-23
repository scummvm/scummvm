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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "mads/game.h"
#include "mads/mads.h"
#include "mads/menu_views.h"
#include "mads/resources.h"
#include "mads/scene.h"
#include "mads/screen.h"
#include "mads/nebular/menu_nebular.h"

namespace MADS {

namespace Nebular {

#define NEBULAR_MENUSCREEN 990
#define MADS_MENU_Y ((MADS_SCREEN_HEIGHT - MADS_SCENE_HEIGHT) / 2)
#define MADS_MENU_ANIM_DELAY 70

MainMenu::MainMenu(MADSEngine *vm): MenuView(vm) {
	Common::fill(&_menuItems[0], &_menuItems[7], (SpriteAsset *)nullptr);
	Common::fill(&_menuItemIndexes[0], &_menuItemIndexes[7], -1);
	_delayTimeout = 0;
	_menuItemIndex = -1;
	_frameIndex = 0;
	_skipFlag = false;
	_highlightedIndex = -1;
	_selectedIndex = -1;
	_buttonDown = false;
	_showEvolve = _showSets = false;

	for (int i = 0; i < 7; ++i)
		_menuItems[i] = nullptr;
}

MainMenu::~MainMenu() {
	Scene &scene = _vm->_game->_scene;
	for (int i = 0; i < 7; ++i) {
		if (_menuItemIndexes[i] != -1)
			scene._sprites.remove(_menuItemIndexes[i]);
	}

	scene._spriteSlots.reset();
}

bool MainMenu::shouldShowQuotes() {
	return ConfMan.hasKey("ShowQuotes") && ConfMan.getBool("ShowQuotes");
}

void MainMenu::display() {
	MenuView::display();
	Scene &scene = _vm->_game->_scene;
	ScreenObjects &screenObjects = _vm->_game->_screenObjects;
	screenObjects.clear();

	// Load each of the menu item assets and add to the scene sprites list
	for (int i = 0; i < 7; ++i) {
		Common::String spritesName = Resources::formatName(NEBULAR_MENUSCREEN,
			'A', i + 1, EXT_SS, "");
		_menuItems[i] = new SpriteAsset(_vm, spritesName, 0);
		_menuItemIndexes[i] = scene._sprites.add(_menuItems[i]);

		// Register the menu item area in the screen objects
		MSprite *frame0 = _menuItems[i]->getFrame(0);
		Common::Point pt(frame0->_offset.x - (frame0->w / 2),
			frame0->_offset.y - frame0->h);
		screenObjects.add(
			Common::Rect(pt.x, pt.y + DIALOG_TOP, pt.x + frame0->w,
			pt.y + frame0->h + DIALOG_TOP), SCREENMODE_VGA, CAT_COMMAND, i);
	}

	// Set the cursor for when it's shown
	_vm->_events->setCursor(CURSOR_ARROW);
}

void MainMenu::doFrame() {
	// Delay between animation frames on the menu
	uint32 currTime = g_system->getMillis();
	if (currTime < _delayTimeout)
		return;
	_delayTimeout = currTime + MADS_MENU_ANIM_DELAY;

	// If an item has already been selected, handle rotating out the other menu items
	if (_selectedIndex != -1) {
		if (_frameIndex == _menuItems[0]->getCount()) {
			handleAction((MADSGameAction)_selectedIndex);
		} else {
			for (_menuItemIndex = 0; _menuItemIndex < 6; ++_menuItemIndex) {
				if (_menuItemIndex == 4 && !shouldShowQuotes())
					continue;

				if (_menuItemIndex != _selectedIndex) {
					addSpriteSlot();
				}
			}

			// Move the menu items to the next frame
			++_frameIndex;
		}
		return;
	}

	// If we've alerady reached the end of the menuitem animation, exit immediately
	if (_menuItemIndex == 6)
		return;

	// If the user has chosen to skip the animation, show the full menu immediately
	if (_skipFlag && _menuItemIndex >= 0) {
		// Quickly loop through all the menu items to display each's final frame
		for (; _menuItemIndex < 6; ++_menuItemIndex) {
			if (_menuItemIndex == 4 && !shouldShowQuotes())
				continue;

			// Draw the final frame of the menuitem
			_frameIndex = 0;
			addSpriteSlot();
		}

		_vm->_events->showCursor();
		showBonusItems();
	} else {
		if ((_menuItemIndex == -1) || (_frameIndex == 0)) {
			if (++_menuItemIndex == 6) {

				// Reached end of display animation
				_vm->_events->showCursor();
				showBonusItems();
				return;
			} else if (_menuItemIndex == 4 && !shouldShowQuotes()) {
				++_menuItemIndex;
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

	int seqIndex = (_menuItemIndex < 6) ? _menuItemIndex : _frameIndex;
	spriteSlots.deleteTimer(seqIndex);

	SpriteAsset *menuItem = _menuItems[_menuItemIndex];
	MSprite *spr = menuItem->getFrame(_frameIndex);

	SpriteSlot &slot = spriteSlots[spriteSlots.add()];
	slot._flags = IMG_UPDATE;
	slot._seqIndex = seqIndex;
	slot._spritesIndex = _menuItemIndexes[_menuItemIndex];
	slot._frameNumber = _frameIndex + 1;
	slot._position = spr->_offset;
	slot._depth = 1;
	slot._scale = 100;

	_redrawFlag = true;
}

void MainMenu::showBonusItems() {
	Scene &scene = _vm->_game->_scene;
	_showEvolve = Common::File::exists("SECTION0.HAG") && Common::File::exists("evolve.res");
	_showSets = Common::File::exists("SECTION0.HAG") && Common::File::exists("sets.res");

	if (_showSets)
		scene._kernelMessages.add(Common::Point(290, 143), 0x4140, 0, 0, 0, "S");
	if (_showEvolve)
		scene._kernelMessages.add(Common::Point(305, 143), 0x4140, 0, 0, 0, "E");
}

bool MainMenu::onEvent(Common::Event &event) {
	Scene &scene = _vm->_game->_scene;
	if (_selectedIndex != -1)
		return false;

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

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		if (_vm->_events->isCursorVisible()) {
			_buttonDown = true;
			int menuIndex = getHighlightedItem(event.mouse);

			if (menuIndex != _highlightedIndex) {
				scene._spriteSlots.deleteTimer(menuIndex);

				_highlightedIndex = menuIndex;
				if (_highlightedIndex != -1) {
					_frameIndex = _highlightedIndex;
					addSpriteSlot();
				}
			}
		} else {
			// Skip the menu animation
			_skipFlag = true;
		}
		return true;

	case Common::EVENT_MOUSEMOVE:
		if (_buttonDown) {
			int menuIndex = getHighlightedItem(event.mouse);
			if (menuIndex != _highlightedIndex) {
				if (_highlightedIndex != -1) {
					// Revert to the unselected menu item
					unhighlightItem();
				}

				if (menuIndex != -1) {
					// Highlight new item
					_highlightedIndex = menuIndex;
					_frameIndex = _highlightedIndex;
					addSpriteSlot();
				}
			}
		}
		break;

	case Common::EVENT_LBUTTONUP:
		_buttonDown = false;
		if (_highlightedIndex != -1) {
			_selectedIndex = _highlightedIndex;
			unhighlightItem();
			_frameIndex = 0;
		} else if (_showSets && Common::Rect(290, 165, 300, 185).contains(event.mouse)) {
			handleAction(SETS);
		} else if (_showEvolve && Common::Rect(305, 165, 315, 185).contains(event.mouse)) {
			handleAction(EVOLVE);
		}

		return true;

	default:
		break;
	}

	return false;
}

int MainMenu::getHighlightedItem(const Common::Point &pt) {
	return _vm->_game->_screenObjects.scan(pt, SCREENMODE_VGA) - 1;
}

void MainMenu::unhighlightItem() {
	// Revert to the unselected menu item
	_vm->_game->_scene._spriteSlots.deleteTimer(_highlightedIndex);
	_menuItemIndex = _highlightedIndex;
	_frameIndex = 0;
	addSpriteSlot();

	_menuItemIndex = 6;
	_highlightedIndex = -1;
}

void MainMenu::handleAction(MADSGameAction action) {
	_vm->_events->hideCursor();
	_breakFlag = true;

	switch (action) {
	case START_GAME:
		// Show the difficulty dialog
		_vm->_dialogs->_pendingDialog = DIALOG_DIFFICULTY;
		break;

	case RESUME_GAME:
		// The original resumed the most recently saved game. Instead,
		// just show the load game scren
		_vm->_dialogs->_pendingDialog = DIALOG_RESTORE;
		return;

	case SHOW_INTRO:
		AnimationView::execute(_vm, "rexopen");
		break;

	case CREDITS:
		TextView::execute(_vm, "credits");
		return;

	case QUOTES:
		TextView::execute(_vm, "quotes");
		return;

	case SETS:
		AnimationView::execute(_vm, "sets");
		break;

	case EVOLVE:
		AnimationView::execute(_vm, "evolve");
		break;

	case EXIT:
		_vm->_dialogs->_pendingDialog = DIALOG_ADVERT;
		break;
	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

AdvertView::AdvertView(MADSEngine *vm): EventTarget(), _vm(vm) {
	_breakFlag = false;
}

void AdvertView::show() {
	bool altAdvert = _vm->getRandomNumber(1000) >= 500;
	int screenId = altAdvert ? 995 : 996;
	uint32 expiryTime = g_system->getMillis() + 10 * 1000;

	_vm->_palette->resetGamePalette(4, 8);

	// Load the advert background onto the screen
	SceneInfo *sceneInfo = SceneInfo::init(_vm);
	sceneInfo->load(screenId, 0, Common::String(), 0, _vm->_game->_scene._depthSurface,
		*_vm->_screen);
	_vm->_screen->markAllDirty();
	_vm->_palette->setFullPalette(_vm->_palette->_mainPalette);

	delete sceneInfo;

	EventsManager &events = *_vm->_events;
	events.setEventTarget(this);
	events.hideCursor();

	while (!_breakFlag && !_vm->shouldQuit()) {
		_vm->_events->waitForNextFrame();
		_vm->_game->_fx = kTransitionNone;

		_breakFlag |= g_system->getMillis() >= expiryTime;
	}

	events.setEventTarget(nullptr);
	_vm->quitGame();
	events.pollEvents();
}

bool AdvertView::onEvent(Common::Event &event) {
	if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN) {
		_breakFlag = true;
		return true;
	}

	return false;
}

/*------------------------------------------------------------------------*/

void RexAnimationView::scriptDone() {
	AnimationView::scriptDone();

	Common::String s = getResourceName();
	if (s == "rexend1") {
		TextView::execute(_vm, "ending1");
	} else if (s == "rexend2") {
		TextView::execute(_vm, "ending2");
	} else if (s == "rexend3") {
		TextView::execute(_vm, "credits");
	}
}

} // End of namespace Nebular

} // End of namespace MADS
