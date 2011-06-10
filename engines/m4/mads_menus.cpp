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

#include "common/textconsole.h"

#include "m4/mads_menus.h"
#include "m4/m4.h"

namespace M4 {

#define REX_MENUSCREEN 990
#define PHANTOM_MENUSCREEN 920
#define DRAGON_MENUSCREEN 922

#define DRAGON_MENU_BUTTON_W = 45
#define DRAGON_MENU_BUTTON_H = 11

RexMainMenuView::RexMainMenuView(MadsM4Engine *vm):
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
	_bgSurface = new M4Surface();
	_bgSurface->loadBackground(REX_MENUSCREEN, &_bgPalData);
	_vm->_palette->addRange(_bgPalData);
	_bgSurface->translate(_bgPalData);

	int row = (height() - MADS_SURFACE_HEIGHT) / 2;
	_bgSurface->copyTo(this, 0, row);

	// Add in the bounding lines for the background
	setColor(2);
	hLine(0, width() - 1, row - 1);
	hLine(0, width() - 1, height() - row + 1);

	// Set up the menu item pos list
	_menuItemPosList[0] = Common::Point(12, 68);
	_menuItemPosList[1] = Common::Point(12, 87);
	_menuItemPosList[2] = Common::Point(12, 107);
	_menuItemPosList[3] = Common::Point(184, 75);
	_menuItemPosList[4] = Common::Point(245, 75);
	_menuItemPosList[5] = Common::Point(184, 99);
}

RexMainMenuView::~RexMainMenuView() {
	delete _menuItem;

	_vm->_palette->deleteRange(_bgPalData);

	delete _bgPalData;
	delete _bgSurface;

	for (uint i = 0; i < _itemPalData.size(); ++i) {
		_vm->_palette->deleteRange(_itemPalData[i]);
		delete _itemPalData[i];
	}
}

bool RexMainMenuView::onEvent(M4EventType eventType, int32 param, int x, int y, bool &captureEvents) {
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
					const Common::Point &pt = _menuItemPosList[_highlightedIndex];
					spr->copyTo(this, pt.x, row + pt.y, spr->getTransparencyIndex());
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
				spr->copyTo(this, _menuItemPosList[_menuItemIndex - 1].x,
					_menuItemPosList[_menuItemIndex - 1].y + row + (itemSize / 2) - (spr->height() / 2),
					spr->getTransparencyIndex());

				delete _menuItem;
				copyTo(_bgSurface, Common::Rect(0, row, width(), row + MADS_SURFACE_HEIGHT), 0, 0,
					spr->getTransparencyIndex());
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
	spr->copyTo(this, _menuItemPosList[_menuItemIndex - 1].x, _menuItemPosList[_menuItemIndex - 1].y +
		row + (itemSize / 2) - (spr->height() / 2), spr->getTransparencyIndex());
}

int RexMainMenuView::getHighlightedItem(int x, int y) {
	y -= (height() - MADS_SURFACE_HEIGHT) / 2;

	for (int index = 0; index < 6; ++index) {
		const Common::Point &pt = _menuItemPosList[index];
		M4Sprite *spr = _menuItem->getFrame(index);

		if ((x >= pt.x) && (y >= pt.y) && (x < (pt.x + spr->width())) && (y < (pt.y + spr->height())))
			return index;
	}

	return -1;
}

void RexMainMenuView::handleAction(MadsGameAction action) {
	MadsEngine *vm = (MadsEngine *)_vm;
	vm->_mouse->cursorOff();
	vm->_viewManager->deleteView(this);

	switch (action) {
	case START_GAME:
	case RESUME_GAME:
		// Load a sample starting scene - note that, currently, calling loadScene automatically
		// removes this menu screen from being displayed
		vm->_mouse->cursorOn();
		vm->startScene(101);
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
			vm->startScene(altAdvert ? 995 : 996);
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

MadsMainMenuView::MadsMainMenuView(MadsM4Engine *vm):
		View(vm, Common::Rect(0, 0, vm->_screen->width(), vm->_screen->height())) {

}

bool MadsMainMenuView::onEvent(M4EventType eventType, int32 param, int x, int y, bool &captureEvents) {
	return false;
}

void MadsMainMenuView::updateState() {
	// TODO: Implement me
}

//--------------------------------------------------------------------------

DragonMainMenuView::DragonMainMenuView(MadsM4Engine *vm):
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

	// Set up the menu item pos list
	_menuItemPosList[0] = Common::Point(46, 187);
	_menuItemPosList[1] = Common::Point(92, 187);
	_menuItemPosList[2] = Common::Point(138, 187);
	_menuItemPosList[3] = Common::Point(184, 187);
	_menuItemPosList[4] = Common::Point(230, 187);
	_menuItemPosList[5] = Common::Point(276, 187);
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

bool DragonMainMenuView::onEvent(M4EventType eventType, int32 param, int x, int y, bool &captureEvents) {
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
	spr->copyTo(this, spr->xOffset - 140, spr->yOffset - spr->height(), spr->getTransparencyIndex());

	_vm->_mouse->cursorOn();
}

int DragonMainMenuView::getHighlightedItem(int x, int y) {
	y -= (height() - MADS_SURFACE_HEIGHT) / 2;

	for (int index = 0; index < 6; ++index) {
		const Common::Point &pt = _menuItemPosList[index];
		M4Sprite *spr = _menuItem->getFrame(0);

		if ((x >= pt.x - 25) && (y >= pt.y - spr->height()) && (x < (pt.x - 25 + spr->width())) && (y < (pt.y)))  {
			debugCN(kDebugGraphics, "x = %d, y = %d, index = %d\n", x, y, index);
			return index;
		}
	}

	return -1;
}

void DragonMainMenuView::handleAction(MadsGameAction action) {
	MadsM4Engine *vm = _vm;
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

	default:
		break;
	}
}


/*--------------------------------------------------------------------------
 * RexDialogView is the base class for the different full-screen dialogs
 * in at least Rex Nebular
 *--------------------------------------------------------------------------
 */

RexDialogView::RexDialogView(): View(_madsVm, Common::Rect(0, 0, _madsVm->_screen->width(), _madsVm->_screen->height())),
		MadsView(this) {
	_screenType = VIEWID_MENU;

	// Initialize class variables
	_priorSceneId = _madsVm->_scene->getCurrentScene();
	_dialogType = DIALOG_NONE;

	// Load necessary quotes
	_madsVm->globals()->loadQuoteRange(1, 48);

	initializeLines();
	initializeGraphics();
}

void RexDialogView::initializeLines() {
	// Set up a list of blank entries for use in the various dialogs
	for (int i = 0; i < DIALOG_LINES_SIZE; ++i) {
		DialogTextEntry rec;
		rec.in_use = false;
		_dialogText.push_back(rec);
	}
	_totalTextEntries = 0;

	// Set up a default sprite slot entry for a full screen refresh
	_spriteSlots.startIndex = 1;
	_spriteSlots[0].spriteType = FULL_SCREEN_REFRESH;
	_spriteSlots[0].seqIndex = -1;
}

void RexDialogView::initializeGraphics() {
	// Set needed palette entries
	_madsVm->_palette->blockRange(0, 16);
	_madsVm->_palette->setEntry(10, 0, 255, 0);
	_madsVm->_palette->setEntry(11, 0, 180, 0);
	_madsVm->_palette->setEntry(12, 255, 255, 0);
	_madsVm->_palette->setEntry(13, 180, 180, 0);
	_madsVm->_palette->setEntry(14, 255, 255, 180);
	_madsVm->_palette->setEntry(15, 180, 180,  180);

	// Load an appropriate background and menu sprites
	loadBackground();
	loadMenuSprites();

	// Set the current cursor
	_madsVm->_mouse->setCursorNum(CURSOR_ARROW);
}


RexDialogView::~RexDialogView() {
	_madsVm->_palette->deleteRange(_bgPalData);
	delete _bgPalData;
	delete _backgroundSurface;
}

void RexDialogView::loadBackground() {
	int bgIndex = _madsVm->globals()->sceneNumber / 100;
	int screenId = 0;

	switch (bgIndex) {
	case 1:
	case 2:
		screenId = 921;
		break;
	case 3:
	case 4:
		screenId = 922;
		break;
	case 5:
	case 6:
	case 7:
		screenId = 923;
		break;
	case 8:
		screenId = 924;
		break;
	case 9:
		screenId = 920;
		break;
	default:
		error("Unknown scene number");
	}

	_backgroundSurface = new M4Surface(width(), MADS_SURFACE_HEIGHT);
	_backgroundSurface->loadBackground(screenId, &_bgPalData);
	_vm->_palette->addRange(_bgPalData);
	_backgroundSurface->translate(_bgPalData);
}

void RexDialogView::loadMenuSprites() {
	const char *SPRITES_NAME = "*MENU.SS";

	_spriteSlots.addSprites(SPRITES_NAME);
}


void RexDialogView::updateState() {
}

void RexDialogView::onRefresh(RectList *rects, M4Surface *destSurface) {
	// Draw the framed base area
	fillRect(this->bounds(), _madsVm->_palette->BLACK);
	setColor(2);
	hLine(0, width(), MADS_Y_OFFSET - 2);
	hLine(0, width(), MADS_Y_OFFSET + MADS_SURFACE_HEIGHT + 2);

	// Add in the loaded background vertically centered
	_backgroundSurface->copyTo(this, 0, (height() - MADS_SURFACE_HEIGHT) / 2);

	// Check whether any of the dialog text entries need to be refreshed
	refreshText();

	// Handle the drawing of the various Mads elements
	refresh();

	View::onRefresh(rects, destSurface);
}

/**
 * Handles item selection within dialogs
 */
bool RexDialogView::onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents) {
	static bool word_7F28C = false;
	int word_7FED2 = 0;
	int word_8502A = 0;

	// If it's a keypress, handle it immediately
	if (eventType == KEVENT_KEY) {
		switch (param1) {
		case Common::KEYCODE_q | (Common::KBD_CTRL << 24):
		case Common::KEYCODE_q | (Common::KBD_ALT << 24):
			_madsVm->quitGame();
			return true;
		case Common::KEYCODE_RETURN:
			_enterFlag = true;
			_selectedLine = 0;
			break;
		case Common::KEYCODE_ESCAPE:
			_selectedLine = 0;
			break;
		default:
			return false;
		}
	}

	// Mark all the dialog text entries as not being seelcted
	for (uint i = 0; i < _dialogText.size(); ++i)
		_dialogText[i].state = STATE_DESELECTED;

	// Check if the mouse is over a registered screen object
	int idx = _screenObjects.scan(x, y, LAYER_GUI);

	if (word_7F28C) {
		if (y < _screenObjects[2].bounds.top) {
			if (eventType != MEVENT_LEFT_RELEASE)
				_dialogText[1].state = STATE_SELECTED;
			idx = 19;
		}

		if (y > _screenObjects[8].bounds.bottom) {
			if (eventType != MEVENT_LEFT_RELEASE)
				_dialogText[7].state = STATE_SELECTED;
			idx = 20;
		}
	}

	int objIndex = -1;
	if ((idx > 0) && ((eventType == MEVENT_LEFT_HOLD) || (eventType == MEVENT_LEFT_DRAG) ||
					(eventType == MEVENT_LEFT_RELEASE))) {
		objIndex = _screenObjects[idx].index;

		if ((_dialogType == DIALOG_SAVE) || (_dialogType == DIALOG_RESTORE)) {
			if ((objIndex > 7) && (objIndex <= 14))
				_dialogText[objIndex].state = STATE_SELECTED;
		}

		if (word_7FED2)
			word_7F28C = (objIndex > 0) && (objIndex <= 7);

		if (_screenObjects[idx].category == 1)
			_dialogText[objIndex].state = STATE_SELECTED;
	} else {
		idx = -1;
	}

	if (idx == 0)
		idx = -1;

	if (_dialogType == DIALOG_ERROR) {
		if (idx == 1)
			idx = -1;
	}

	if (eventType == MEVENT_LEFT_RELEASE) {
		if (!word_7F28C || (objIndex <= 18))
			_selectedLine = objIndex;

		word_8502A = -1;
	}

	return true;
}

void RexDialogView::setFrame(int frameNumber, int depth) {
	int slotIndex = _spriteSlots.getIndex();
	_spriteSlots[slotIndex].spriteType = FOREGROUND_SPRITE;
	_spriteSlots[slotIndex].seqIndex = 1;
	_spriteSlots[slotIndex].spriteListIndex = 0; //_menuSpritesIndex;
	_spriteSlots[slotIndex].frameNumber = frameNumber;

	M4Sprite *spr = _spriteSlots.getSprite(0).getFrame(frameNumber - 1);
	_spriteSlots[slotIndex].xp = spr->x;
	_spriteSlots[slotIndex].yp = spr->y;
	_spriteSlots[slotIndex].depth = depth;
	_spriteSlots[slotIndex].scale = 100;
}

void RexDialogView::initVars() {
	_v8502C = -1;
	_selectedLine = -1;
	_lineIndex = 0;
	_enterFlag = false;
	_textLines.clear();
}

void RexDialogView::addLine(const char *msg_p, Font *font, MadsTextAlignment alignment, int left, int top) {
	DialogTextEntry *rec = NULL;

	if (_lineIndex < _totalTextEntries) {
		if (strcmp(msg_p, _dialogText[_lineIndex].text) == 0)  {
			rec = &_dialogText[_lineIndex];
			if (rec->textDisplay_index != 0) {
				MadsTextDisplayEntry &tdEntry = _textDisplay[rec->textDisplay_index];
				if (tdEntry.active) {
					if (_textLines.size() < 20) {
						// Add entry to line list
						_textLines.push_back(tdEntry.msg);
						tdEntry.msg = _textLines[_textLines.size() - 1].c_str();
					}
				}
			}
		}
	} else {
		if (_lineIndex < DIALOG_LINES_SIZE) {
			rec = &_dialogText[_lineIndex];
			_totalTextEntries = _lineIndex + 1;
		}
	}

	// Handling for if a line needs to be added
	if (rec) {
		strcpy(rec->text, msg_p);
		rec->font = font;
		rec->state = STATE_DESELECTED;
		rec->pos.y = top;
		rec->widthAdjust = -1;
		rec->in_use = true;
		rec->textDisplay_index = -1;

		switch (alignment) {
		case ALIGN_CENTER:
			// Center text
			rec->pos.x = (width() - font->getWidth(rec->text)) / 2 + left;
			break;

		case ALIGN_CHAR_CENTER: {
			// Text is center aligned on the '@' character within the string
			char *p = strchr(rec->text, '@');

			if (p) {
				// '@' string handling
				// Get length of string up to the '@' character
				*p = '\0';
				int strWidth = font->getWidth(rec->text, rec->widthAdjust);
				// Remove the character from the string. strcpy isn't used here because it's unsafe for
				// copying within the same string
				while ((*p = *(p + 1)) != '\0') ++p;

				rec->pos.x = (width() / 2) - strWidth;
			} else {
				rec->pos.x = left;
			}
			break;
		}

		case RIGHT_ALIGN:
			// Right align (moving left from given passed left)
			rec->pos.x = left - font->getWidth(rec->text);
			break;

		default:
			break;
		}
	}

	++_lineIndex;
}

/**
 * Adds a line consisting of either a single quote, or the combination of two quote Ids
 */
void RexDialogView::addQuote(Font *font, MadsTextAlignment alignment, int left, int top, int id1, int id2) {
	char buffer[80];

	// Copy the first quote string into the buffer
	const char *quoteStr = _madsVm->globals()->getQuote(id1);
	strcpy(buffer, quoteStr);

	// Handle the optional second quote Id
	if (id2 != 0) {
		quoteStr = _madsVm->globals()->getQuote(id2);
		strcat(buffer, " ");
		strcat(buffer, quoteStr);
	}

	// Add in the generated line
	addLine(buffer, font, alignment, left, top);
}

/**
 * Sets any previously created dialog text entries as clickable items
 */
void RexDialogView::setClickableLines() {
	_screenObjects.clear();

	for (int i = 0; i < DIALOG_LINES_SIZE; ++i) {
		if (_dialogText[i].in_use) {
			// Add an entry for the line
			_screenObjects.add(Common::Rect(_dialogText[i].pos.x, _dialogText[i].pos.y,
				_dialogText[i].pos.x + _dialogText[i].font->getWidth(_dialogText[i].text, _dialogText[i].widthAdjust),
				_dialogText[i].pos.y + _dialogText[i].font->getHeight()), 19, i, 1);
		}
	}

	if ((_madsVm->globals()->dialogType == DIALOG_SAVE) || (_madsVm->globals()->dialogType == DIALOG_RESTORE)) {
		// Extra entries for the scroller areas of the  Save and Restor dialogs
		_screenObjects.add(Common::Rect(293, 26, 312, 75), LAYER_GUI, 50, 2);
		_screenObjects.add(Common::Rect(293, 78, 312, 127), LAYER_GUI, 51, 2);
	}
}

/**
 * Handles creating text display objects for each dialog line initially, and when the selected state
 * of any entry changes
 */
void RexDialogView::refreshText() {
	for (uint i = 0; i < _dialogText.size(); ++i) {
		if (!_dialogText[i].in_use)
			continue;

		// Get the item's colors
		uint color;
		if (_dialogText[i].state == STATE_DESELECTED)
			color = 0xB0A;
		else if (_dialogText[i].state == STATE_SELECTED)
			color = 0xD0C;
		else
			color = 0xF0E;

		// If there's an associated text display entry, check to see if it's color needs to change
		if (_dialogText[i].textDisplay_index >= 0) {
			MadsTextDisplayEntry &tdEntry = _textDisplay[_dialogText[i].textDisplay_index];

			if ((tdEntry.color1 == (color & 0xff)) && (tdEntry.color2 == (color >> 8)))
				// It's still the same, so no further action needed
				continue;

			// Flag the currently assigned text display to be expired, so it can be re-created
			_textDisplay.expire(_dialogText[i].textDisplay_index);
			_dialogText[i].textDisplay_index = -1;
		}

		// Create a new text display entry for the dialog text line
		_dialogText[i].textDisplay_index = _textDisplay.add(_dialogText[i].pos.x, _dialogText[i].pos.y,
			color, _dialogText[i].widthAdjust, _dialogText[i].text, _dialogText[i].font);
	}
}

/*--------------------------------------------------------------------------
 * RexGameMenuDialog is the main game dialog for the game
 *--------------------------------------------------------------------------
 */

RexGameMenuDialog::RexGameMenuDialog(): RexDialogView() {
	_dialogType = DIALOG_GAME_MENU;
	setFrame(1, 2);
	initVars();

	_vm->_font->setFont(FONT_CONVERSATION_MADS);
	addLines();
	setClickableLines();
}

void RexGameMenuDialog::addLines() {
	// Add the title
	int top = MADS_Y_OFFSET - 2 - ((((_vm->_font->current()->getHeight() + 2) * 6) >> 1) - 78);

	addQuote(_vm->_font->current(), ALIGN_CENTER, 0, top, 10);

	// Loop for adding the option lines of the dialog
	top += 6;
	for (int idx = 0; idx < 5; ++idx) {
		top += _vm->_font->current()->getHeight() + 1;
		addQuote(_vm->_font->current(), ALIGN_CENTER, 0, top, 11 + idx);
	}
}

bool RexGameMenuDialog::onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents) {
	// Call the parent event handler to handle line selection
	bool handled = RexDialogView::onEvent(eventType, param1, x, y, captureEvents);

	if (_selectedLine > 0) {
		switch (_selectedLine) {
		case 1:
			// Save Game
			_madsVm->globals()->dialogType = DIALOG_SAVE;
			break;
		case 2:
			// Restore Game
			_madsVm->globals()->dialogType = DIALOG_RESTORE;
			break;
		case 3:
			// Game Play Options
			_madsVm->globals()->dialogType = DIALOG_OPTIONS;
			break;
		case 4:
			// Resume Current Game
			_madsVm->globals()->dialogType = DIALOG_NONE;
			break;
		case 5:
			// Exit From Game
			_madsVm->quitGame();
			break;
		default:
			// TODO: Extra logic for such as resuming scene if necessary
			_madsVm->globals()->dialogType = DIALOG_NONE;
			break;
		}

		// Close this dialog
		_madsVm->_viewManager->deleteView(this);
	}

	return handled;
}

/*--------------------------------------------------------------------------
 * RexOptionsDialog is the game options dialog for Rex Nebular
 *--------------------------------------------------------------------------
 */

RexOptionsDialog::RexOptionsDialog(): RexDialogView() {
	_dialogType = DIALOG_OPTIONS;
	_tempConfig = _madsVm->globals()->_config;

	setFrame(2, 2);
	initVars();

	_vm->_font->setFont(FONT_CONVERSATION_MADS);
	addLines();
	setClickableLines();
}

void RexOptionsDialog::reload() {
	for (int i = 0; i < DIALOG_LINES_SIZE; ++i)
		_dialogText[i].in_use = false;
	_totalTextEntries = 0;
	_textDisplay.clear();
	_screenObjects.clear();

	initVars();

	_vm->_font->setFont(FONT_CONVERSATION_MADS);
	addLines();
	setClickableLines();
}

void RexOptionsDialog::addLines() {
	// Add the title
	int top = MADS_Y_OFFSET - 2 - ((((_vm->_font->current()->getHeight() + 1) * 9 + 12) >> 1) - 78);

	addQuote(_vm->_font->current(), ALIGN_CENTER, 0, top, 16);

	// Music state line
	top += _vm->_font->current()->getHeight() + 1 + 6;
	addQuote(_vm->_font->current(), ALIGN_CHAR_CENTER, 0, top, 17, _tempConfig.musicFlag ? 24 : 25);

	// Sound state line
	top += _vm->_font->current()->getHeight() + 1;
	addQuote(_vm->_font->current(), ALIGN_CHAR_CENTER, 0, top, 18, _tempConfig.soundFlag ? 26 : 27);

	// Interface easy state line
	top += _vm->_font->current()->getHeight() + 1;
	addQuote(_vm->_font->current(), ALIGN_CHAR_CENTER, 0, top, 19, _tempConfig.easyMouse ? 29 : 28);

	// Inventory sppinng state line
	top += _vm->_font->current()->getHeight() + 1;
	addQuote(_vm->_font->current(), ALIGN_CHAR_CENTER, 0, top, 20, _tempConfig.invObjectsStill ? 31 : 30);

	// Text window state line
	top += _vm->_font->current()->getHeight() + 1;
	addQuote(_vm->_font->current(), ALIGN_CHAR_CENTER, 0, top, 21, _tempConfig.textWindowStill ? 33 : 32);

	// Screen fade state line
	top += _vm->_font->current()->getHeight() + 1;
	addQuote(_vm->_font->current(), ALIGN_CHAR_CENTER, 0, top, 22, _tempConfig.screenFades + 34);

	// Storyline mode line
	top += _vm->_font->current()->getHeight() + 1;
	addQuote(_vm->_font->current(), ALIGN_CHAR_CENTER, 0, top, 23, (_tempConfig.storyMode == 1) ? 37 : 38);

	// Add Done and Cancel button texts
	top += _vm->_font->current()->getHeight() + 1 + 6;
	addQuote(_vm->_font->current(), ALIGN_CENTER, -54, top, 1, 0);
	addQuote(_vm->_font->current(), ALIGN_CENTER, 54, top, 2, 0);
}

bool RexOptionsDialog::onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents) {
	// Call the parent event handler to handle line selection
	bool handled = RexDialogView::onEvent(eventType, param1, x, y, captureEvents);

	if (_selectedLine > 0) {
		switch (_selectedLine) {
		case 0:
			// Enter or Escape
			_selectedLine = _enterFlag ? 8 : 9;
			return true;
		case 1:
			// Music line
			_tempConfig.musicFlag = !_tempConfig.musicFlag;
			break;
		case 2:
			// Sound line
			_tempConfig.soundFlag = !_tempConfig.soundFlag;
			break;
		case 3:
			// Interface line
			_tempConfig.easyMouse = !_tempConfig.easyMouse;
			break;
		case 4:
			// Inventory line
			_tempConfig.invObjectsStill = !_tempConfig.invObjectsStill;
			break;
		case 5:
			// Text window line
			_tempConfig.textWindowStill = !_tempConfig.textWindowStill;
			break;
		case 6:
			// Screen fades line
			if (++_tempConfig.screenFades > 2)
				_tempConfig.screenFades = 0;
			break;
		case 7:
			// Story mode line
			if (_tempConfig.storyMode == 2)
				_tempConfig.storyMode = 1;
			else if (_tempConfig.storyMode == 1)
				_tempConfig.storyMode = 2;
			break;
		case 8:
		case 9:
			// Done and Cancel buttons
			// TODO: Proper re-loading of settings if Cancel button clicked
			_madsVm->globals()->_config = _tempConfig;

			// Closing the dialog, so return to the game menu
			_madsVm->globals()->dialogType = DIALOG_GAME_MENU;
			_madsVm->_viewManager->deleteView(this);
			return true;
		}

		// Update the option selections
		reload();
	}

	return handled;
}

}
