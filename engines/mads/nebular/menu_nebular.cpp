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
#include "mads/scene.h"
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
	_vm->_screenFade = SCREEN_FADE_FAST;

	scene._spriteSlots.reset(true);
	display();

	events.setEventTarget(this);
	events.hideCursor();

	while (!_breakFlag && !_vm->shouldQuit()) {
		if (_redrawFlag) {
			_vm->_game->_scene.drawElements(_vm->_game->_fx, _vm->_game->_fx);
			_redrawFlag = false;
		}

		_vm->_events->waitForNextFrame();
		_vm->_game->_fx = kTransitionNone;
		doFrame();
	}

	events.setEventTarget(nullptr);
	_vm->_sound->stop();
}

void MenuView::display() {
	_vm->_palette->resetGamePalette(4, 8);

	FullScreenDialog::display();
}

bool MenuView::onEvent(Common::Event &event) {
	if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN) {
		_breakFlag = true;
		_vm->_dialogs->_pendingDialog = DIALOG_MAIN_MENU;
		return true;
	}

	return false;
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
	_selectedIndex = -1;
	_buttonDown = false;
	
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
			pt.y + frame0->h + DIALOG_TOP), LAYER_GUI, CAT_COMMAND, i);
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
		}

		return true;

	default:
		break;
	}
	
	return false;
}

int MainMenu::getHighlightedItem(const Common::Point &pt) {
	return _vm->_game->_screenObjects.scan(pt, LAYER_GUI) - 1;
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
		_vm->_screen);
	_vm->_screen.copyRectToScreen(_vm->_screen.getBounds());
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

char TextView::_resourceName[100];
#define TEXTVIEW_LINE_SPACING 2
#define TEXT_ANIMATION_DELAY 100
#define TV_NUM_FADE_STEPS 40
#define TV_FADE_DELAY_MILLI 50

void TextView::execute(MADSEngine *vm, const Common::String &resName) {
	assert(resName.size() < 100);
	strncpy(_resourceName, resName.c_str(), sizeof(_resourceName));
	vm->_dialogs->_pendingDialog = DIALOG_TEXTVIEW;
}

TextView::TextView(MADSEngine *vm) : MenuView(vm) {
	_animating = false;
	_panSpeed = 0;
	_spareScreen = nullptr;
	_scrollCount = 0;
	_lineY = -1;
	_scrollTimeout = 0;
	_panCountdown = 0;
	_translationX = 0;
	_screenId = -1;

	_font = _vm->_font->getFont(FONT_CONVERSATION);
	_vm->_palette->resetGamePalette(4, 0);
	load();
}

TextView::~TextView() {
}

void TextView::load() {
	Common::String scriptName(_resourceName);
	scriptName += ".txr";

	if (!_script.open(scriptName))
		error("Could not open resource %s", _resourceName);

	processLines();
}

void TextView::processLines() {
	if (_script.eos())
		error("Attempted to read past end of response file");

	while (!_script.eos()) {
		// Read in the next line
		_script.readLine(_currentLine, 79);
		char *p = _currentLine + strlen(_currentLine) - 1;
		if (*p == '\n')
			*p = '\0';

		// Commented out line, so go loop for another
		if (_currentLine[0] == '#')
			continue;

		// Process the line
		char *cStart = strchr(_currentLine, '[');
		if (cStart) {
			while (cStart) {
				// Loop for possible multiple commands on one line
				char *cEnd = strchr(_currentLine, ']');
				if (!cEnd)
					error("Unterminated command '%s' in response file", _currentLine);

				*cEnd = '\0';
				processCommand();

				// Copy rest of line (if any) to start of buffer
				strncpy(_currentLine, cEnd + 1, sizeof(_currentLine));

				cStart = strchr(_currentLine, '[');
			}

			if (_currentLine[0]) {
				processText();
				break;
			}

		} else {
			processText();
			break;
		}
	}
}

void TextView::processCommand() {
	Scene &scene = _vm->_game->_scene;
	Common::String scriptLine(_currentLine + 1);
	scriptLine.toUppercase();
	const char *paramP;
	const char *commandStr = scriptLine.c_str();

	if (!strncmp(commandStr, "BACKGROUND", 10)) {
		// Set the background
		paramP = commandStr + 10;
		resetPalette();
		int screenId = getParameter(&paramP);
		
		SceneInfo *sceneInfo = SceneInfo::init(_vm);
		sceneInfo->load(screenId, 0, "", 0, scene._depthSurface, scene._backgroundSurface);
		scene._spriteSlots.fullRefresh();
		_redrawFlag = true;

	} else if (!strncmp(commandStr, "GO", 2)) {
		_animating = true;

	} else if (!strncmp(commandStr, "PAN", 3)) {
		// Set panning values
		paramP = commandStr + 3;
		int panX = getParameter(&paramP);
		int panY = getParameter(&paramP);
		int panSpeed = getParameter(&paramP);

		if ((panX != 0) || (panY != 0)) {
			_pan = Common::Point(panX, panY);
			_panSpeed = panSpeed;
		}

	} else if (!strncmp(commandStr, "DRIVER", 6)) {
		// Set the driver to use
		paramP = commandStr + 7;

		if (!strncmp(paramP, "#SOUND.00", 9)) {
			int driverNum = paramP[9] - '0';
			_vm->_sound->init(driverNum);
		}
	} else if (!strncmp(commandStr, "SOUND", 5)) {
		// Set sound number
		paramP = commandStr + 5;
		int soundId = getParameter(&paramP);
		_vm->_sound->command(soundId);

	} else if (!strncmp(commandStr, "COLOR", 5) && ((commandStr[5] == '0') || 
			(commandStr[5] == '1'))) {
		// Set the text colors
		int index = commandStr[5] - '0';
		paramP = commandStr + 6;

		byte r = getParameter(&paramP);
		byte g = getParameter(&paramP);
		byte b = getParameter(&paramP);

		_vm->_palette->setEntry(5 + index, r, g, b);

	} else if (!strncmp(commandStr, "SPARE", 5)) {
		// Sets a secondary background number that can be later switched in with a PAGE command
		paramP = commandStr + 6;
		int spareIndex = commandStr[5] - '0';
		assert(spareIndex < 4);
		int screenId = getParameter(&paramP);

		// Load the spare background
		SceneInfo *sceneInfo = SceneInfo::init(_vm);
		sceneInfo->_width = MADS_SCREEN_WIDTH;
		sceneInfo->_height = MADS_SCENE_HEIGHT;
		_spareScreens[spareIndex].setSize(MADS_SCREEN_WIDTH, MADS_SCENE_HEIGHT);
		sceneInfo->loadMadsV1Background(screenId, "", SCENEFLAG_TRANSLATE, 
			_spareScreens[spareIndex]);
		delete sceneInfo;

	} else if (!strncmp(commandStr, "PAGE", 4)) {
		// Signals to change to a previous specified secondary background
		paramP = commandStr + 4;
		int spareIndex = getParameter(&paramP);

		// Only allow background switches if one isn't currently in progress
		if (!_spareScreen && _spareScreens[spareIndex].getPixels() != nullptr) {
			_spareScreen = &_spareScreens[spareIndex];
			_translationX = 0;
		}

	} else {
		error("Unknown response command: '%s'", commandStr);
	}
}

int TextView::getParameter(const char **paramP) {
	if ((**paramP != '=') && (**paramP != ','))
		return 0;

	int result = 0;
	++*paramP;
	while ((**paramP >= '0') && (**paramP <= '9')) {
		result = result * 10 + (**paramP - '0');
		++*paramP;
	}

	return result;
}

void TextView::processText() {
	int lineWidth, xStart;

	if (!strcmp(_currentLine, "***")) {
		// Special signifier for end of script
		_scrollCount = _font->getHeight() * 13;
		_lineY = -1;
		return;
	}

	_lineY = 0;

	// Lines are always centered, except if line contains a '@', in which case the
	// '@' marks the position that must be horizontally centered
	char *centerP = strchr(_currentLine, '@');
	if (centerP) {
		*centerP = '\0';
		xStart = (MADS_SCREEN_WIDTH / 2) - _font->getWidth(_currentLine);

		// Delete the @ character and shift back the remainder of the string
		char *p = centerP + 1;
		if (*p == ' ') ++p;
		strcpy(centerP, p);

	} else {
		lineWidth = _font->getWidth(_currentLine);
		xStart = (MADS_SCREEN_WIDTH - lineWidth) / 2;
	}

	// Add the new line to the list of pending lines
	TextLine tl;
	tl._pos = Common::Point(xStart, MADS_SCENE_HEIGHT);
	tl._line = _currentLine;
	tl._textDisplayIndex = -1;
	_textLines.push_back(tl);
}

void TextView::display() {
	FullScreenDialog::display();
	_sceneChanged = true;
}

void TextView::resetPalette() {
	_vm->_palette->resetGamePalette(8, 8);
	_vm->_palette->setEntry(5, 0, 63, 63);
	_vm->_palette->setEntry(6, 0, 45, 45);
}

void TextView::doFrame() {
	Scene &scene = _vm->_game->_scene;
	if (!_animating)
		return;

	// Only update state if wait period has expired
	uint32 currTime = g_system->getMillis();

	// If a screen transition is in progress and it's time for another column, handle it
	if (_spareScreen) {
		byte *srcP = _spareScreen->getBasePtr(_translationX, 0);
		byte *bgP = scene._backgroundSurface.getBasePtr(_translationX, 0);
		byte *screenP = (byte *)_vm->_screen.getBasePtr(_translationX, 0);

		for (int y = 0; y < MADS_SCENE_HEIGHT; ++y, srcP += MADS_SCREEN_WIDTH,
			bgP += MADS_SCREEN_WIDTH, screenP += MADS_SCREEN_WIDTH) {
			*bgP = *srcP;
			*screenP = *srcP;
		}

		// Flag the column of the screen is modified
		_vm->_screen.copyRectToScreen(Common::Rect(_translationX, 0,
			_translationX + 1, MADS_SCENE_HEIGHT));

		// Keep moving the column to copy to the right
		if (++_translationX == MADS_SCREEN_WIDTH) {
			// Surface transition is complete
			_spareScreen = nullptr;
		}
	}

	// Make sure it's time for an update
	if (currTime < _scrollTimeout)
		return;
	_scrollTimeout = g_system->getMillis() + TEXT_ANIMATION_DELAY;
	_redrawFlag = true;

	// If any panning values are set, pan the background surface
	if ((_pan.x != 0) || (_pan.y != 0)) {
		if (_panCountdown > 0) {
			--_panCountdown;
			return;
		}

		// Handle horizontal panning
		if (_pan.x != 0) {
			byte *lineTemp = new byte[_pan.x];
			for (int y = 0; y < MADS_SCENE_HEIGHT; ++y) {
				byte *pixelsP = (byte *)scene._backgroundSurface.getBasePtr(0, y);

				// Copy the first X pixels into temp buffer, move the rest of the line
				// to the start of the line, and then move temp buffer pixels to end of line
				Common::copy(pixelsP, pixelsP + _pan.x, lineTemp);
				Common::copy(pixelsP + _pan.x, pixelsP + MADS_SCREEN_WIDTH, pixelsP);
				Common::copy(lineTemp, lineTemp + _pan.x, pixelsP + MADS_SCREEN_WIDTH - _pan.x);
			}

			delete[] lineTemp;
		}

		// Handle vertical panning
		if (_pan.y != 0) {
			// Store the bottom Y lines into a temp buffer, move the rest of the lines down,
			// and then copy the stored lines back to the top of the screen
			byte *linesTemp = new byte[_pan.y * MADS_SCREEN_WIDTH];
			byte *pixelsP = (byte *)scene._backgroundSurface.getBasePtr(0, MADS_SCENE_HEIGHT - _pan.y);
			Common::copy(pixelsP, pixelsP + MADS_SCREEN_WIDTH * _pan.y, linesTemp);

			for (int y = MADS_SCENE_HEIGHT - 1; y >= _pan.y; --y) {
				byte *destP = (byte *)scene._backgroundSurface.getBasePtr(0, y);
				byte *srcP = (byte *)scene._backgroundSurface.getBasePtr(0, y - _pan.y);
				Common::copy(srcP, srcP + MADS_SCREEN_WIDTH, destP);
			}

			Common::copy(linesTemp, linesTemp + _pan.y * MADS_SCREEN_WIDTH, 
				(byte *)scene._backgroundSurface.getPixels());
			delete[] linesTemp;
		}

		// Flag for a full screen refresh
		scene._spriteSlots.fullRefresh();
	}

	// Scroll all active text lines up
	for (int i = _textLines.size() - 1; i >= 0; --i) {
		TextLine &tl = _textLines[i];
		if (tl._textDisplayIndex != -1)
			// Expire the text line that's already on-screen
			scene._textDisplay.expire(tl._textDisplayIndex);

		tl._pos.y--;
		if (tl._pos.y < 0) {
			_textLines.remove_at(i);
		} else {
			tl._textDisplayIndex = scene._textDisplay.add(tl._pos.x, tl._pos.y, 
				0x605, -1, tl._line, _font);
		}
	}

	if (_scrollCount > 0) {
		// Handling final scrolling of text off of screen
		if (--_scrollCount == 0) {
			scriptDone();
			return;
		}
	} else {
		// Handling a text row
		if (++_lineY == (_font->getHeight() + TEXTVIEW_LINE_SPACING))
			processLines();
	}
}

void TextView::scriptDone() {
	_breakFlag = true;
	_vm->_dialogs->_pendingDialog = DIALOG_MAIN_MENU;
}

/*------------------------------------------------------------------------*/

char AnimationView::_resourceName[100];

void AnimationView::execute(MADSEngine *vm, const Common::String &resName) {
	assert(resName.size() < 100);
	strncpy(_resourceName, resName.c_str(), sizeof(_resourceName));
	vm->_dialogs->_pendingDialog = DIALOG_ANIMVIEW;
}

AnimationView::AnimationView(MADSEngine *vm) : MenuView(vm) {
	_soundDriverLoaded = false;
	_previousUpdate = 0;
	_screenId = -1;
	_showWhiteBars = true;
	_resetPalette = false;
	_resyncMode = NEVER;

	load();
}

void AnimationView::load() {
	Common::String resName(_resourceName);
	if (!resName.hasSuffix("."))
		resName += ".res";

	if (!_script.open(resName))
		error("Could not open resource %s", resName.c_str());

	processLines();
}

bool AnimationView::onEvent(Common::Event &event) {
	// Wait for the Escape key or a mouse press
	if (((event.type == Common::EVENT_KEYDOWN) && (event.kbd.keycode == Common::KEYCODE_ESCAPE)) ||
			(event.type == Common::EVENT_RBUTTONUP)) {
		scriptDone();
		return true;
	}

	return false;
}

void AnimationView::doFrame() {
	Scene &scene = _vm->_game->_scene;
	int bgNumber = 0;

	// Only update state if wait period has expired
	if (_previousUpdate > 0) {
		if (g_system->getMillis() - _previousUpdate < 3000) {
			return;
		} else {
			// time for an update
			_previousUpdate = g_system->getMillis();
		}
	} else {
		_previousUpdate = g_system->getMillis();
		return;
	}
	/*
	char bgFile[10];
	strncpy(bgFile, _currentFile, 5);
	bgFile[0] = bgFile[2];
	bgFile[1] = bgFile[3];
	bgFile[2] = bgFile[4];
	bgFile[3] = '\0';
	bgNumber = atoi(bgFile);
	sprintf(bgFile, "rm%i.art", bgNumber);

	// Not all scenes have a background. If there is one, refresh it
	if (Common::File::exists(bgFile)) {
		_vm->_palette->resetGamePalette(4, 8);
		SceneInfo *sceneInfo = SceneInfo::init(_vm);
		sceneInfo->load(bgNumber, 0, Common::String(), 0, scene._depthSurface,
			scene._backgroundSurface);
	}
	*/
}

void AnimationView::scriptDone() {
	_breakFlag = true;
	_vm->_dialogs->_pendingDialog = DIALOG_MAIN_MENU;
}

void AnimationView::processLines() {
	if (_script.eos()) {
		// end of script, end animation
		scriptDone();
		return;
	}

	char c;
	while (!_script.eos()) {
		// Get in next line
		_currentLine.empty();
		while (!_script.eos() && (c = _script.readByte()) != '\n') {
			if (c != '\r')
				_currentLine += c;
		}
		
		// Process the line
		while (!_currentLine.empty()) {
			if (_currentLine.hasPrefix("-")) {
				_currentLine.deleteChar(0);

				processCommand();
			} else {
				// Get resource name
				Common::String resName;
				while (!_currentLine.empty() && (c = _currentLine[0]) != ' ') {
					_currentLine.deleteChar(0);
					resName += c;
				}

				_resources.push_back(ResourceEntry(resName, _sfx));
				_sfx = 0;
			}

			// Skip any spaces
			while (_currentLine.hasPrefix(" "))
				_currentLine.deleteChar(0);
		}
	}
}

void AnimationView::processCommand() {
	// Get the command character
	char commandChar = toupper(_currentLine[0]);
	_currentLine.deleteChar(0);

	// Handle the command
	switch (commandChar) {
	case 'H':
		// -h[:ex]  Disable EMS / XMS high memory support
		if (_currentLine.hasPrefix(":"))
			_currentLine.deleteChar(0);
		while (_currentLine.hasPrefix("e") || _currentLine.hasPrefix("x"))
			_currentLine.deleteChar(0);
		break;
	case 'O':
		// -o:xxx  Specify opening special effect
		assert(_currentLine[0] == ':');
		_currentLine.deleteChar(0);
		_sfx = getParameter();
		break;
	case 'P':
		// Switch to CONCAT mode, which is ignored anyway
		break;
	case 'R': {
		// Resynch timer (always, beginning, never)
		assert(_currentLine[0] == ':');
		_currentLine.deleteChar(0);

		char v = toupper(_currentLine[0]);
		_currentLine.deleteChar(0);
		if (v == 'N')
			_resyncMode = NEVER;
		else if (v == 'A')
			_resyncMode = ALWAYS;
		else if (v == 'B')
			_resyncMode = BEGINNING;
		else
			error("Unknown parameter");
		break;
	}
	case 'W':
		// Switch white bars being visible
		_showWhiteBars = !_showWhiteBars;
		break;
	case 'X':
		// Exit after animation finishes. Ignore
		break;
	case 'Y':
		// Reset palette on startup
		_resetPalette = true;
		break;
	default:
		error("Unknown command char: '%c'", commandChar);
	}
}

int AnimationView::getParameter() {
	int result = 0;

	while (!_currentLine.empty()) {
		char c = _currentLine[0];
		
		if (c >= '0' && c <= '9') {
			_currentLine.deleteChar(0);
			result = result * 10 + (c - '0');
		} else {
			break;
		}
	}

	return result;
}


} // End of namespace Nebular

} // End of namespace MADS
