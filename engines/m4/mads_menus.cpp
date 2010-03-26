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
	MadsM4Engine *vm = _vm;
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
	spr->copyTo(this, spr->xOffset - 140, spr->yOffset - spr->height(), (int)spr->getTransparentColor());

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

RexDialogView::RexDialogView(): MadsView(_madsVm, Common::Rect(0, 0, _madsVm->_screen->width(), _madsVm->_screen->height())) {
	_screenType = VIEWID_MENU;

	// Store the previously active scene
	_priorSceneId = _madsVm->_scene->getCurrentScene();

	// Load necessary quotes
	_madsVm->globals()->loadQuoteRange(1, 48);

	initialiseLines();
	initialiseGraphics();
}

void RexDialogView::initialiseLines() {
	// Set up a list of blank entries for use in the various dialogs
	for (int i = 0; i < DIALOG_LINES_SIZE; ++i) {
		DialogTextEntry rec;
		rec.in_use = false;
		_dialogText.push_back(rec);
	}
	_totalTextEntries = 0;

	// Set up a default sprite slot entry
	_spriteSlots.startIndex = 1;
	_spriteSlots[0].spriteId = -2;
	_spriteSlots[0].timerIndex = -1;
}

void RexDialogView::initialiseGraphics() {
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
	case 9:
		screenId = 920;
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
	setColour(2);
	hLine(0, width(), MADS_Y_OFFSET - 2);
	hLine(0, width(), MADS_Y_OFFSET + MADS_SURFACE_HEIGHT + 2);

	// Add in the loaded background vertically centred
	_backgroundSurface->copyTo(this, 0, (height() - MADS_SURFACE_HEIGHT) / 2);

	// Check whether any of the dialog text entries need to be refreshed
	refreshText();

	MadsView::onRefresh(rects, destSurface);
}

void RexDialogView::setFrame(int frameNumber, int depth) {
	int slotIndex = _spriteSlots.getIndex();
	_spriteSlots[slotIndex].spriteId = 1;
	_spriteSlots[slotIndex].timerIndex = 1;
	_spriteSlots[slotIndex].spriteListIndex = 0; //_menuSpritesIndex;
	_spriteSlots[slotIndex].frameNumber = frameNumber;

	M4Sprite *spr = _spriteSlots.getSprite(0).getFrame(0);
	_spriteSlots[slotIndex].xp = spr->x;
	_spriteSlots[slotIndex].yp = spr->y;
	_spriteSlots[slotIndex].depth = depth;
	_spriteSlots[slotIndex].scale = 100;
}

void RexDialogView::initVars() {
	_word_8502C = -1;
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
		rec->state = 0;
		rec->pos.y = top;
		rec->widthAdjust = -1;
		rec->in_use = true;
		rec->textDisplay_index = -1;

		switch (alignment) {
		case ALIGN_CENTER:
			// Center text
			rec->pos.x = (width() - font->getWidth(rec->text)) / 2;
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
				while ((*p == *(p + 1)) != '\0') ++p;

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

		// Get the item's colours
		uint colour;
		if (_dialogText[i].state == STATE_DESELECTED)
			colour = 0xB0A;
		else if (_dialogText[i].state == STATE_HIGHLIGHTED)
			colour = 0xD0C;
		else
			colour = 0xF0E;

		// If there's an associated text display entry, check to see if it's colour needs to change
		if (_dialogText[i].textDisplay_index >= 0) {
			MadsTextDisplayEntry &tdEntry = _textDisplay[_dialogText[i].textDisplay_index];

			if ((tdEntry.colour1 == (colour & 0xff)) && (tdEntry.colour2 == (colour >> 8)))
				// It's still the same, so no further action needed
				continue;

			// Flag the currently assigned text display to be expired, so it can be re-created
			_textDisplay.expire(_dialogText[i].textDisplay_index);
			_dialogText[i].textDisplay_index = -1;
		}

		// Create a new text display entry for the dialog text line
		_dialogText[i].textDisplay_index = _textDisplay.add(_dialogText[i].pos.x, _dialogText[i].pos.y,
			colour, _dialogText[i].widthAdjust, _dialogText[i].text, _dialogText[i].font);
	}
}

/*--------------------------------------------------------------------------
 * RexDialogView is the Rex Nebular Game Menu dialog
 *--------------------------------------------------------------------------
 */

RexGameMenuDialog::RexGameMenuDialog(): RexDialogView() {
	setFrame(1, 2);
	initVars();

	_vm->_font->setFont(FONT_CONVERSATION_MADS);
	addLines();
	setClickableLines();
}

void RexGameMenuDialog::addLines() {
	// Add the title
	int top = MADS_Y_OFFSET - 2 - ((((_vm->_font->getHeight() + 2) * 6) >> 1) - 78);
		
	addQuote(_vm->_font, ALIGN_CENTER, 0, top, 10);

	// Loop for adding the option lines of the dialog
	top += 6;
	for (int idx = 0; idx < 5; ++idx) {
		top += _vm->_font->getHeight() + 1;
		addQuote(_vm->_font, ALIGN_CENTER, 0, top, 11 + idx);
	}
}

void RexGameMenuDialog::onRefresh(RectList *rects, M4Surface *destSurface) {
	RexDialogView::onRefresh(rects, destSurface);
}

bool RexGameMenuDialog::onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents) {
	bool handled = false;

	// Handle various event types
	switch (eventType) {
	case MEVENT_LEFT_CLICK:
		// Left mouse click
		// TODO: Check and figure out _selectedLine
		handled = true;
		break;

	case KEVENT_KEY:
		// Handle standard dialog keypresses

		handled = true;
		break;

	default:
		break;
	}

	if (_selectedLine > 0) {
		switch (_selectedLine) {
		case 1:
			_madsVm->globals()->dialogType = DIALOG_SAVE;
		case 2:
			_madsVm->globals()->dialogType = DIALOG_RESTORE;
		case 3:
			_madsVm->globals()->dialogType = DIALOG_OPTIONS;
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


}
