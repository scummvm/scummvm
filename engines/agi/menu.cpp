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

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/text.h"
#include "agi/keyboard.h"
#include "agi/menu.h"

namespace Agi {

GfxMenu::GfxMenu(AgiEngine *vm, GfxMgr *gfx, PictureMgr *picture, TextMgr *text) {
	_vm = vm;
	_gfx = gfx;
	_picture = picture;
	_text = text;

	_allowed = true;
	_submitted = false;
	_delayedExecute = false;

	_setupMenuColumn = 1;
	_setupMenuItemColumn = 1;

	_selectedMenuNr = 0;
	_selectedMenuHeight = 0;
	_selectedMenuWidth = 0;
	_selectedMenuRow = 0;
	_selectedMenuColumn = 0;
}

GfxMenu::~GfxMenu() {
	for (GuiMenuArray::iterator itemIter = _array.begin(); itemIter != _array.end(); ++itemIter)
		delete *itemIter;
	_array.clear();

	for (GuiMenuItemArray::iterator menuIter = _itemArray.begin(); menuIter != _itemArray.end(); ++menuIter)
		delete *menuIter;
	_itemArray.clear();
}

void GfxMenu::addMenu(const char *menuText) {
	// already submitted? in that case no further changes possible
	if (_submitted)
		return;

	GuiMenuEntry *menuEntry = new GuiMenuEntry();

	menuEntry->text = menuText;
	menuEntry->textLen = menuEntry->text.size();
	menuEntry->row = 0;
	menuEntry->column = _setupMenuColumn;
	menuEntry->itemCount = 0;
	menuEntry->firstItemNr = _itemArray.size();
	menuEntry->selectedItemNr = menuEntry->firstItemNr;
	menuEntry->maxItemTextLen = 0;
	_array.push_back(menuEntry);

	_setupMenuColumn += menuEntry->textLen + 1;
}

void GfxMenu::addMenuItem(const char *menuItemText, uint16 controllerSlot) {
	int16 arrayCount = _array.size();

	// already submitted? in that case no further changes possible
	if (_submitted)
		return;

	if (arrayCount == 0)
		error("tried to add a menu item before adding an actual menu");

	// go to latest menu entry
	GuiMenuEntry *curMenuEntry = _array.back();

	GuiMenuItemEntry *menuItemEntry = new GuiMenuItemEntry();

	menuItemEntry->enabled = true;
	menuItemEntry->text = menuItemText;
	menuItemEntry->textLen = menuItemEntry->text.size();
	menuItemEntry->controllerSlot = controllerSlot;

	// Original interpreter on PC used the length of the first item for drawing
	// At least in KQ2 on Apple IIgs follow-up items are longer, which would result in graphic glitches.
	// That's why we remember the longest item and draw according to that
	if (curMenuEntry->maxItemTextLen < menuItemEntry->textLen) {
		curMenuEntry->maxItemTextLen = menuItemEntry->textLen;
	}

	if (curMenuEntry->itemCount == 0) {
		// for first menu item of menu calculated column
		if (menuItemEntry->textLen + curMenuEntry->column < (FONT_COLUMN_CHARACTERS - 1)) {
			_setupMenuItemColumn = curMenuEntry->column;
		} else {
			_setupMenuItemColumn = ( FONT_COLUMN_CHARACTERS - 1 ) - menuItemEntry->textLen;
		}
	}

	menuItemEntry->row = 2 + curMenuEntry->itemCount;
	menuItemEntry->column = _setupMenuItemColumn;

	_itemArray.push_back(menuItemEntry);

	curMenuEntry->itemCount++;
}

void GfxMenu::submit() {
	GuiMenuEntry *menuEntry = nullptr;
	GuiMenuItemEntry *menuItemEntry = nullptr;
	int16 menuCount = _array.size();
	int16 menuNr = 0;
	int16 menuItemNr = 0;
	int16 menuItemLastNr = 0;

	if ((_array.size() == 0) || (_itemArray.size() == 0))
		return;

	_submitted = true;

	// WORKAROUND: For Apple II gs we try to fix the menu text
	// On this platform it seems a system font was used and the menu was drawn differently (probably system menu?)
	// Still the text was misaligned anyway, but it looks worse in our (the original PC) implementation
	// Atari ST SQ1 had one bad menu entry as well, we fix that too.
	switch (_vm->getPlatform()) {
	case Common::kPlatformApple2GS:
	case Common::kPlatformAtariST:
		// Go through all menus
		for (menuNr = 0; menuNr < menuCount; menuNr++) {
			menuEntry = _array[menuNr];
			menuItemLastNr = menuEntry->firstItemNr + menuEntry->itemCount;

			// Go through all items of current menu
			for (menuItemNr = menuEntry->firstItemNr; menuItemNr < menuItemLastNr; menuItemNr++) {
				menuItemEntry = _itemArray[menuItemNr];

				if (menuItemEntry->textLen < menuEntry->maxItemTextLen) {
					// current item text is shorter than the maximum?
					int16 missingCharCount = menuEntry->maxItemTextLen - menuItemEntry->textLen;

					if (menuItemEntry->text.contains('>')) {
						// text contains '>', we now try to find a '<'
						// and then add spaces in case this item is shorter than the first item
						int16 textPos = menuItemEntry->textLen - 1;

						while (textPos > 0) {
							if (menuItemEntry->text[textPos] == '<')
								break;
							textPos--;
						}

						if (textPos > 0) {
							while (missingCharCount) {
								menuItemEntry->text.insertChar(' ', textPos);
								missingCharCount--;
							}
						}
					} else {
						// Also check if text consists only of '-', which is the separator
						// These were sometimes also too small
						int16 separatorCount = 0;
						int16 charPos = 0;

						while (charPos < menuItemEntry->textLen) {
							if (menuItemEntry->text[charPos] != '-')
								break;
							separatorCount++;
							charPos++;
						}

						if (separatorCount == menuItemEntry->textLen) {
							// Separator detected
							while (missingCharCount) {
								menuItemEntry->text.insertChar('-', 0);
								missingCharCount--;
							}
						} else {
							// Append spaces to the end to fill it up
							int16 textPos = menuItemEntry->textLen;
							while (missingCharCount) {
								menuItemEntry->text.insertChar(' ', textPos);
								textPos++;
								missingCharCount--;
							}
						}
					}

					menuItemEntry->textLen = menuItemEntry->text.size();
				}
			}
		}
		break;
	default:
		break;
	}
}

void GfxMenu::itemEnable(uint16 controllerSlot) {
	itemEnableDisable(controllerSlot, true);
}

void GfxMenu::itemDisable(uint16 controllerSlot) {
	itemEnableDisable(controllerSlot, false);
}

void GfxMenu::itemEnableDisable(uint16 controllerSlot, bool enabled) {
	GuiMenuItemArray::iterator listIterator;
	GuiMenuItemArray::iterator listEnd = _itemArray.end();
	GuiMenuItemEntry *menuItemEntry;

	listIterator = _itemArray.begin();
	while (listIterator != listEnd) {
		menuItemEntry = *listIterator;
		if (menuItemEntry->controllerSlot == controllerSlot) {
			menuItemEntry->enabled = enabled;
		}

		listIterator++;
	}
}

void GfxMenu::itemEnableAll() {
	GuiMenuItemArray::iterator listIterator;
	GuiMenuItemArray::iterator listEnd = _itemArray.end();
	GuiMenuItemEntry *menuItemEntry;

	listIterator = _itemArray.begin();
	while (listIterator != listEnd) {
		menuItemEntry = *listIterator;
		menuItemEntry->enabled = true;

		listIterator++;
	}
}

// return true, in case a menu was actually created and submitted by the scripts
bool GfxMenu::isAvailable() {
	return _submitted;
}

void GfxMenu::accessAllow() {
	_allowed = true;
}

void GfxMenu::accessDeny() {
	_allowed = false;
}

void GfxMenu::delayedExecute() {
	_delayedExecute = true;
}

bool GfxMenu::delayedExecuteActive() {
	return _delayedExecute;
}

void GfxMenu::execute() {
	_delayedExecute = false;

	// got submitted? -> safety check
	if (!_submitted)
		return;

	// access allowed at the moment?
	if (!_allowed)
		return;

	_text->charPos_Push();
	_text->charAttrib_Push();
	_text->clearLine(0, _text->calculateTextBackground(15));

	// Draw all menus
	for (uint16 menuNr = 0; menuNr < _array.size(); menuNr++) {
		drawMenuName(menuNr, false);
	}
	drawActiveMenu();

	// original AGI did not do this, at least when the menu was called by scripts
	_vm->inGameTimerPause();

	_vm->cycleInnerLoopActive(CYCLE_INNERLOOP_MENU);
	do {
		_vm->mainCycle();
	} while (_vm->cycleInnerLoopIsActive() && !(_vm->shouldQuit() || _vm->_restartGame));

	// original AGI did not do this, at least when the menu was called by scripts
	_vm->inGameTimerResume();

	removeActiveMenu();

	_text->charAttrib_Pop();
	_text->charPos_Pop();

	// Restore status line
	if (_text->statusEnabled()) {
		_text->statusDraw();
	} else {
		_text->clearLine(0, 0);
	}
}

void GfxMenu::drawMenuName(int16 menuNr, bool inverted) {
	GuiMenuEntry *menuEntry = _array[menuNr];
	bool disabledLook = false;

	if (!inverted) {
		_text->charAttrib_Set(0, _text->calculateTextBackground(15));
	} else {
		_text->charAttrib_Set(15, _text->calculateTextBackground(0));
	}

	_text->charPos_Set(menuEntry->row, menuEntry->column);

	if (menuEntry->itemCount == 0)
		disabledLook = true;

	_text->displayText(menuEntry->text.c_str(), disabledLook);
}

void GfxMenu::drawItemName(int16 itemNr, bool inverted) {
	GuiMenuItemEntry *itemEntry = _itemArray[itemNr];
	bool disabledLook = false;

	if (!inverted) {
		_text->charAttrib_Set(0, _text->calculateTextBackground(15));
	} else {
		_text->charAttrib_Set(15, _text->calculateTextBackground(0));
	}

	_text->charPos_Set(itemEntry->row, itemEntry->column);

	if (itemEntry->enabled == false)
		disabledLook = true;

	_text->displayText(itemEntry->text.c_str(), disabledLook);
}

void GfxMenu::drawActiveMenu() {
	GuiMenuEntry *menuEntry = _array[_selectedMenuNr];
	GuiMenuItemEntry *itemEntry = _itemArray[menuEntry->firstItemNr];
	int16 itemNr = menuEntry->firstItemNr;
	int16 itemCount = menuEntry->itemCount;
	
	// draw menu name as inverted
	drawMenuName(_selectedMenuNr, true);

	// calculate active menu dimensions
	_selectedMenuHeight = (menuEntry->itemCount + 2) * FONT_VISUAL_HEIGHT;
	_selectedMenuWidth  = (menuEntry->maxItemTextLen * FONT_VISUAL_WIDTH) + 8;
	_selectedMenuRow    = (menuEntry->itemCount + 3 - _text->getWindowRowMin()) * FONT_VISUAL_HEIGHT - 1;
	_selectedMenuColumn = (itemEntry->column - 1) * FONT_VISUAL_WIDTH;

	_gfx->drawBox(_selectedMenuColumn, _selectedMenuRow, _selectedMenuWidth, _selectedMenuHeight, 15, 0);

	while (itemCount) {
		if (itemNr == menuEntry->selectedItemNr) {
			drawItemName(itemNr, true);
		} else {
			drawItemName(itemNr, false);
		}
		itemNr++;
		itemCount--;
	}
}

void GfxMenu::removeActiveMenu() {
	// draw menu name normally again
	drawMenuName(_selectedMenuNr, false);

	// overwrite actual menu items by rendering play screen
	_gfx->render_Block(_selectedMenuColumn, _selectedMenuRow, _selectedMenuWidth, _selectedMenuHeight);
}

void GfxMenu::charPress(int16 newChar) {
	GuiMenuEntry *menuEntry = _array[_selectedMenuNr];
	GuiMenuItemEntry *itemEntry = _itemArray[menuEntry->selectedItemNr];
	int16 newMenuNr = _selectedMenuNr;
	int16 newItemNr = menuEntry->selectedItemNr;

	switch (newChar) {
	case AGI_KEY_ENTER:
		// check, if current item is actually enabled
		if (!itemEntry->enabled)
			return;

		// Trigger controller
		_vm->_game.controllerOccured[itemEntry->controllerSlot] = true;

		_vm->cycleInnerLoopInactive(); // exit execute-loop
		break;
	case AGI_KEY_ESCAPE:
		_vm->cycleInnerLoopInactive(); // exit execute-loop
		break;

	// these here change menu item
	case AGI_KEY_UP:
		newItemNr--;
		break;
	case AGI_KEY_DOWN:
		newItemNr++;
		break;
	case AGI_KEY_PAGE_UP:
		// select first item of current menu
		newItemNr = menuEntry->firstItemNr;
		break;
	case AGI_KEY_PAGE_DOWN:
		// select last item of current menu
		newItemNr = menuEntry->firstItemNr + menuEntry->itemCount - 1;
		break;

	case AGI_KEY_LEFT:
		newMenuNr--;
		break;
	case AGI_KEY_RIGHT:
		newMenuNr++;
		break;
	case AGI_KEY_HOME:
		// select first menu
		newMenuNr = 0;
		break;
	case AGI_KEY_END:
		// select last menu
		newMenuNr = _array.size() - 1;
		break;

	default:
		break;
	}

	if (newMenuNr != _selectedMenuNr) {
		// selected menu was changed
		int16 lastMenuNr = _array.size() - 1;

		if (newMenuNr < 0) {
			newMenuNr = lastMenuNr;
		} else if (newMenuNr > lastMenuNr) {
			newMenuNr = 0;
		}

		if (newMenuNr != _selectedMenuNr) {
			removeActiveMenu();
			_selectedMenuNr = newMenuNr;
			drawActiveMenu();
		}
	}

	if (newItemNr != menuEntry->selectedItemNr) {
		// selected item was changed
		int16 lastItemNr = menuEntry->firstItemNr + menuEntry->itemCount - 1;

		if (newItemNr < menuEntry->firstItemNr) {
			newItemNr = lastItemNr;
		} else if (newItemNr > lastItemNr) {
			newItemNr = menuEntry->firstItemNr;
		}

		if (newItemNr != menuEntry->selectedItemNr) {
			// still changed after clip -> draw changes
			drawItemName(menuEntry->selectedItemNr, false);
			drawItemName(newItemNr, true);
			menuEntry->selectedItemNr = newItemNr;
		}
	}
}

} // End of namespace Agi
