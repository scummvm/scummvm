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

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/gfx/operations.h"
#include "sci/gfx/gfx_state_internal.h"
#include "sci/gui/gui_helpers.h"
#include "sci/gui/gui_gfx.h"
#include "sci/gui/gui_cursor.h"
#include "sci/gui/gui_font.h"
#include "sci/gui/gui_text.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_menu.h"

namespace Sci {

SciGuiMenu::SciGuiMenu(SegManager *segMan, SciGuiGfx *gfx, SciGuiText *text, SciGuiScreen *screen, SciGuiCursor *cursor)
	: _segMan(segMan), _gfx(gfx), _text(text), _screen(screen), _cursor(cursor) {

	_listCount = 0;
	// We actually set active item in here and remember last selection of the user
	//  sierra sci always defaulted to first item every time menu was called via ESC, we dont follow that logic
	_curMenuId = 1;
	_curItemId = 1;

	_menuSaveHandle = NULL_REG;
	_barSaveHandle = NULL_REG;
	_oldPort = NULL;
}

SciGuiMenu::~SciGuiMenu() {
	// TODO: deallocate _list and _itemList
}

void SciGuiMenu::init(GfxState *gfxstate) {
	_gfxstate = gfxstate;
}

void SciGuiMenu::add(Common::String title, Common::String content, reg_t contentVmPtr) {
	GuiMenuEntry *menuEntry;
	uint16 itemCount = 0;
	GuiMenuItemEntry *itemEntry;
	int contentSize = content.size();
	int separatorCount;
	int curPos, beginPos, endPos, tempPos;
	int tagPos, rightAlignedPos, functionPos, altPos, controlPos;

	// Sierra SCI starts with id 1, so we do so as well
	_listCount++;
	menuEntry = new GuiMenuEntry(_listCount);
	menuEntry->text = title;
	_list.push_back(menuEntry);

	curPos = 0;
	do {
		itemCount++;
		itemEntry = new GuiMenuItemEntry(_listCount, itemCount);

		beginPos = curPos;

		// Now go through the content till we find end-marker and collect data about it
		// ':' is an end-marker for each item
		tagPos = 0; rightAlignedPos = 0;
		controlPos = 0; altPos = 0; functionPos = 0;
		while ((curPos < contentSize) && (content[curPos] != ':')) {
			switch (content[curPos]) {
			case '=': // Set tag
				if (tagPos)
					error("multiple tag markers within one menu-item");
				tagPos = curPos;
				break;
			case '`': // Right-aligned
				if (rightAlignedPos)
					error("multiple right-aligned markers within one menu-item");
				rightAlignedPos = curPos;
				break;
			case '^': // Ctrl-prefix
				if (controlPos)
					error("multiple control markers within one menu-item");
				controlPos = curPos;
				break;
			case '@': // Alt-prefix
				if (altPos)
					error("multiple alt markers within one menu-item");
				altPos = curPos;
				break;
			case '#': // Function-prefix
				if (functionPos)
					error("multiple function markers within one menu-item");
				functionPos = curPos;
				break;
			}
			curPos++;
		}
		endPos = curPos;

		// Control/Alt/Function key mapping...
		if (controlPos) {
			content.setChar(SCI_MENU_REPLACE_ONCONTROL, controlPos);
			itemEntry->keyModifier = SCI_EVM_CTRL;
			tempPos = controlPos + 1;
			if (tempPos >= contentSize)
				error("control marker at end of item");
			itemEntry->keyPress = tolower(content[tempPos]);
			content.setChar(toupper(content[tempPos]), tempPos);
		}
		if (altPos) {
			content.setChar(SCI_MENU_REPLACE_ONALT, altPos);
			itemEntry->keyModifier = SCI_EVM_ALT;
			tempPos = altPos + 1;
			if (tempPos >= contentSize)
				error("alt marker at end of item");
			itemEntry->keyPress = tolower(content[tempPos]);
			content.setChar(toupper(content[tempPos]), tempPos);
		}
		if (functionPos) {
			content.setChar(SCI_MENU_REPLACE_ONFUNCTION, functionPos);
			tempPos = functionPos + 1;
			if (tempPos >= contentSize)
				error("function marker at end of item");
			itemEntry->keyPress = content[tempPos];
			switch (content[functionPos + 1]) {
			case '1': itemEntry->keyPress = SCI_K_F1; break;
			case '2': itemEntry->keyPress = SCI_K_F2; break;
			case '3': itemEntry->keyPress = SCI_K_F3; break;
			case '4': itemEntry->keyPress = SCI_K_F4; break;
			case '5': itemEntry->keyPress = SCI_K_F5; break;
			case '6': itemEntry->keyPress = SCI_K_F6; break;
			case '7': itemEntry->keyPress = SCI_K_F7; break;
			case '8': itemEntry->keyPress = SCI_K_F8; break;
			case '9': itemEntry->keyPress = SCI_K_F9; break;
			case '0': itemEntry->keyPress = SCI_K_F10; break;
			default:
				error("illegal function key specified");
			}
		}

		// Now get all strings
		tempPos = endPos;
		if (rightAlignedPos) {
			tempPos = rightAlignedPos;
		} else if (tagPos) {
			tempPos = tagPos;
		}
		curPos = beginPos;
		separatorCount = 0;
		while (curPos < tempPos) {
			switch (content[curPos]) {
			case '!':
			case '-':
			case ' ':
				separatorCount++;
			}
			curPos++;
		}
		if (separatorCount == tempPos - beginPos) {
			itemEntry->separatorLine = true;
		} else {
			itemEntry->text = Common::String(content.c_str() + beginPos, tempPos - beginPos);
		}
		itemEntry->textVmPtr = contentVmPtr;
		itemEntry->textVmPtr.offset += beginPos;

		if (rightAlignedPos) {
			rightAlignedPos++;
			tempPos = endPos;
			if (tagPos)
				tempPos = tagPos;
			itemEntry->textRightAligned = Common::String(content.c_str() + rightAlignedPos, tempPos - rightAlignedPos);
		}

		if (tagPos) {
			tempPos = functionPos + 1;
			if (tempPos >= contentSize)
				error("tag marker at end of item");
			itemEntry->tag = atoi(content.c_str() + tempPos);
		}

		curPos = endPos + 1;

		_itemList.push_back(itemEntry);
	} while (curPos < contentSize);
}

GuiMenuItemEntry *SciGuiMenu::findItem(uint16 menuId, uint16 itemId) {
	GuiMenuItemList::iterator listIterator;
	GuiMenuItemList::iterator listEnd = _itemList.end();
	GuiMenuItemEntry *listEntry;

	listIterator = _itemList.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		if ((listEntry->menuId == menuId) && (listEntry->id == itemId))
			return listEntry;

		listIterator++;
	}
	return NULL;
}

void SciGuiMenu::setAttribute(uint16 menuId, uint16 itemId, uint16 attributeId, reg_t value) {
	GuiMenuItemEntry *itemEntry = findItem(menuId, itemId);
	if (!itemEntry)
		error("Tried to setAttribute() on non-existant menu-item %d:%d", menuId, itemId);
	switch (attributeId) {
	case SCI_MENU_ATTRIBUTE_ENABLED:
		itemEntry->enabled = value.isNull() ? false : true;
		break;
	case SCI_MENU_ATTRIBUTE_SAID:
		itemEntry->saidVmPtr = value;
		break;
	case SCI_MENU_ATTRIBUTE_TEXT:
		itemEntry->text = _segMan->getString(value);
		itemEntry->textVmPtr = value;
		// We assume here that no script ever creates a separatorLine dynamically
		break;
	case SCI_MENU_ATTRIBUTE_KEYPRESS:
		itemEntry->keyPress = tolower(value.offset);
		itemEntry->keyModifier = 0;
		// TODO: Find out how modifier is handled
		printf("setAttr keypress %X %X\n", value.segment, value.offset);
		break;
	case SCI_MENU_ATTRIBUTE_TAG:
		itemEntry->tag = value.offset;
		break;
	default:
		error("setAttribute() called with unsupported attributeId %X", attributeId);
	}
}

reg_t SciGuiMenu::getAttribute(uint16 menuId, uint16 itemId, uint16 attributeId) {
	GuiMenuItemEntry *itemEntry = findItem(menuId, itemId);
	if (!itemEntry)
		error("Tried to getAttribute() on non-existant menu-item %d:%d", menuId, itemId);
	switch (attributeId) {
	case SCI_MENU_ATTRIBUTE_ENABLED:
		if (itemEntry->enabled)
			return make_reg(0, 1);
		break;
	case SCI_MENU_ATTRIBUTE_SAID:
		return itemEntry->saidVmPtr;
	case SCI_MENU_ATTRIBUTE_TEXT:
		return itemEntry->textVmPtr;
	case SCI_MENU_ATTRIBUTE_KEYPRESS:
		// TODO: Find out how modifier is handled
		return make_reg(0, itemEntry->keyPress);
	case SCI_MENU_ATTRIBUTE_TAG:
		return make_reg(0, itemEntry->tag);
	default:
		error("getAttribute() called with unsupported attributeId %X", attributeId);
	}
	return NULL_REG;
}

void SciGuiMenu::drawBar() {
	GuiMenuEntry *listEntry;
	GuiMenuList::iterator listIterator;
	GuiMenuList::iterator listEnd = _list.end();

	// Hardcoded black on white
	_gfx->FillRect(_gfx->_menuBarRect, 1, _screen->_colorWhite);
	_gfx->PenColor(0);
	_gfx->MoveTo(8, 1);

	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		_text->Draw_String(listEntry->text.c_str());

		listIterator++;
	}
}

// This helper calculates all text widths for all menus/items
void SciGuiMenu::calculateTextWidth() {
	GuiMenuList::iterator menuIterator;
	GuiMenuList::iterator menuEnd = _list.end();
	GuiMenuEntry *menuEntry;
	GuiMenuItemList::iterator itemIterator;
	GuiMenuItemList::iterator itemEnd = _itemList.end();
	GuiMenuItemEntry *itemEntry;
	int16 dummyHeight;

	menuIterator = _list.begin();
	while (menuIterator != menuEnd) {
		menuEntry = *menuIterator;
		_text->StringWidth(menuEntry->text.c_str(), 0, menuEntry->textWidth, dummyHeight);

		menuIterator++;
	}

	itemIterator = _itemList.begin();
	while (itemIterator != itemEnd) {
		itemEntry = *itemIterator;
		_text->StringWidth(itemEntry->text.c_str(), 0, itemEntry->textWidth, dummyHeight);
		_text->StringWidth(itemEntry->textRightAligned.c_str(), 0, itemEntry->textRightAlignedWidth, dummyHeight);

		itemIterator++;
	}
}

reg_t SciGuiMenu::select(reg_t eventObject) {
	int16 eventType = GET_SEL32V(_segMan, eventObject, type);
	int16 keyPress, keyModifier;
	Common::Point mousePosition;
	GuiMenuItemList::iterator itemIterator = _itemList.begin();
	GuiMenuItemList::iterator itemEnd = _itemList.end();
	GuiMenuItemEntry *itemEntry = NULL;
	bool forceClaimed = false;
	EngineState *s;
	byte saidSpec[64];

	switch (eventType) {
	case SCI_EVT_KEYBOARD:
		keyPress = GET_SEL32V(_segMan, eventObject, message);
		keyModifier = GET_SEL32V(_segMan, eventObject, modifiers);
		switch (keyPress) {
		case 0:
			break;
		case SCI_K_ESC:
			itemEntry = interactiveWithKeyboard();
			forceClaimed = true;
			break;
		default:
			while (itemIterator != itemEnd) {
				itemEntry = *itemIterator;
				if ((itemEntry->keyPress == keyPress) && (itemEntry->keyModifier == keyModifier))
					break;
				itemIterator++;
			}
			if (itemIterator == itemEnd)
				itemEntry = NULL;
		}
		break;

	case SCI_EVT_SAID:
		// HACK: should be removed as soon as said() is cleaned up
		s = ((SciEngine *)g_engine)->getEngineState();
		while (itemIterator != itemEnd) {
			itemEntry = *itemIterator;

			if (!itemEntry->saidVmPtr.isNull()) {
				// TODO: get a pointer to saidVmPtr or make said() work on VmPtrs
				_segMan->memcpy(saidSpec, itemEntry->saidVmPtr, 64);
				if (said(s, saidSpec, 0) != SAID_NO_MATCH)
					break;
			}
			itemIterator++;
		}
		if (itemIterator == itemEnd)
			itemEntry = NULL;
		break;

	case SCI_EVT_MOUSE_PRESS:
		mousePosition = _cursor->getPosition();
		if (mousePosition.y < 10) {
			itemEntry = interactiveWithMouse();
			forceClaimed = true;
		}
		break;
	}

	if (!_menuSaveHandle.isNull()) {
		_gfx->BitsRestore(_menuSaveHandle);
		_gfx->BitsShow(_menuRect);
		_menuSaveHandle = NULL_REG;
		// TODO: Change to ReAnimate()
	}
	if (!_barSaveHandle.isNull()) {
		_gfx->BitsRestore(_barSaveHandle);
		_gfx->BitsShow(_gfx->_menuBarRect);
		_barSaveHandle = NULL_REG;
	}
	if (_oldPort)
		_gfx->SetPort(_oldPort);

	if ((itemEntry) || (forceClaimed))
		PUT_SEL32(_segMan, eventObject, claimed, make_reg(0, 1));
	if (itemEntry)
		return make_reg(0, (itemEntry->menuId << 8) | (itemEntry->id));
	return NULL_REG;
}

GuiMenuItemEntry *SciGuiMenu::interactiveGetItem(uint16 menuId, uint16 itemId, bool menuChanged) {
	GuiMenuItemList::iterator itemIterator = _itemList.begin();
	GuiMenuItemList::iterator itemEnd = _itemList.end();
	GuiMenuItemEntry *itemEntry;
	GuiMenuItemEntry *firstItemEntry = NULL;
	GuiMenuItemEntry *lastItemEntry = NULL;

	// Fixup menuId if needed
	if (menuId > _listCount)
		menuId = 1;
	if (menuId == 0)
		menuId = _listCount;
	while (itemIterator != itemEnd) {
		itemEntry = *itemIterator;
		if (itemEntry->menuId == menuId) {
			if (itemEntry->id == itemId)
				return itemEntry;
			if (!firstItemEntry)
				firstItemEntry = itemEntry;
			if ((!lastItemEntry) || (itemEntry->id > lastItemEntry->id))
				lastItemEntry = itemEntry;
		}
		itemIterator++;
	}
	if ((itemId == 0) || (menuChanged))
		return lastItemEntry;
	return firstItemEntry;
}

void SciGuiMenu::drawMenu(uint16 oldMenuId, uint16 newMenuId) {
	GuiMenuEntry *listEntry;
	GuiMenuList::iterator listIterator;
	GuiMenuList::iterator listEnd = _list.end();
	GuiMenuItemEntry *listItemEntry;
	GuiMenuItemList::iterator listItemIterator;
	GuiMenuItemList::iterator listItemEnd = _itemList.end();
	Common::Rect menuTextRect;
	uint16 listNr = 0;
	int16 maxTextWidth = 0, maxTextRightAlignedWidth = 0;
	int16 topPos;
	Common::Point pixelPos;

	// Remove menu, if one is displayed
	if (!_menuSaveHandle.isNull()) {
		_gfx->BitsRestore(_menuSaveHandle);
		_gfx->BitsShow(_menuRect);
		// TODO: Change to ReAnimate()
	}

	// First calculate rect of menu and also invert old and new menu text
	_menuRect.top = _gfx->_menuBarRect.bottom;
	menuTextRect.top = _gfx->_menuBarRect.top;
	menuTextRect.bottom = _gfx->_menuBarRect.bottom;
	menuTextRect.left = menuTextRect.right = 7;
	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		listNr++;
		menuTextRect.left = menuTextRect.right;
		menuTextRect.right += listEntry->textWidth;
		if (listNr == newMenuId)
			_menuRect.left = menuTextRect.left;
		if ((listNr == newMenuId) || (listNr == oldMenuId)) {
			menuTextRect.translate(1, 0);
			_gfx->InvertRect(menuTextRect);
			menuTextRect.translate(-1, 0);
		}

		listIterator++;
	}
	if (oldMenuId != 0)
		_gfx->BitsShow(_gfx->_menuBarRect);

	_menuRect.bottom = _menuRect.top + 2;
	listItemIterator = _itemList.begin();
	while (listItemIterator != listItemEnd) {
		listItemEntry = *listItemIterator;
		if (listItemEntry->menuId == newMenuId) {
			_menuRect.bottom += _gfx->_curPort->fontHeight;
			maxTextWidth = MAX<int16>(maxTextWidth, listItemEntry->textWidth);
			maxTextRightAlignedWidth = MAX<int16>(maxTextRightAlignedWidth, listItemEntry->textRightAlignedWidth);
		}
		listItemIterator++;
	}
	_menuRect.right = _menuRect.left + 16 + 4 + 2;
	_menuRect.right += maxTextWidth + maxTextRightAlignedWidth;
	if (!maxTextRightAlignedWidth)
		_menuRect.right -= 5;

	// Save background
	_menuSaveHandle = _gfx->BitsSave(_menuRect, SCI_SCREEN_MASK_VISUAL);

	// Do the drawing
	_gfx->FillRect(_menuRect, SCI_SCREEN_MASK_VISUAL, 0);
	_menuRect.left++; _menuRect.right--; _menuRect.bottom--;
	_gfx->FillRect(_menuRect, SCI_SCREEN_MASK_VISUAL, _screen->_colorWhite);

	_menuRect.left += 8;
	topPos = _menuRect.top + 1;
	listItemIterator = _itemList.begin();
	while (listItemIterator != listItemEnd) {
		listItemEntry = *listItemIterator;
		if (listItemEntry->menuId == newMenuId) {
			if (!listItemEntry->separatorLine) {
				_gfx->TextGreyedOutput(listItemEntry->enabled ? false : true);
				_gfx->MoveTo(_menuRect.left, topPos);
				_text->Draw_String(listItemEntry->text.c_str());
				_gfx->MoveTo(_menuRect.right - listItemEntry->textRightAlignedWidth - 5, topPos);
				_text->Draw_String(listItemEntry->textRightAligned.c_str());
			} else {
				// We dont 100% follow sierra here, we draw the line from left to right. Looks better
				// BTW. SCI1.1 seems to put 2 pixels and then skip one, we don't do this at all (lsl6)
				pixelPos.y = topPos + (_gfx->_curPort->fontHeight >> 1) - 1;
				pixelPos.x = _menuRect.left - 7;
				while (pixelPos.x < (_menuRect.right - 1)) {
					_screen->putPixel(pixelPos.x, pixelPos.y, SCI_SCREEN_MASK_VISUAL, 0, 0, 0);
					pixelPos.x += 2;
				}
			}
			topPos += _gfx->_curPort->fontHeight;
		}
		listItemIterator++;
	}
	_gfx->TextGreyedOutput(false);


	_menuRect.left -= 8;
	_menuRect.left--; _menuRect.right++; _menuRect.bottom++;
	_gfx->BitsShow(_menuRect);
}

void SciGuiMenu::invertMenuSelection(uint16 itemId) {
	Common::Rect itemRect = _menuRect;

	itemRect.top += (itemId - 1) * _gfx->_curPort->fontHeight;
	itemRect.bottom = itemRect.top + _gfx->_curPort->fontHeight + 1;
	itemRect.left++; itemRect.right--;

	_gfx->InvertRect(itemRect);
	_gfx->BitsShow(itemRect);
}

GuiMenuItemEntry *SciGuiMenu::interactiveWithKeyboard() {
	sci_event_t curEvent;
	uint16 newMenuId = _curMenuId;
	uint16 newItemId = _curItemId;
	GuiMenuItemEntry *curItemEntry = findItem(_curMenuId, _curItemId);
	GuiMenuItemEntry *newItemEntry = curItemEntry;

	// We don't 100% follow sierra here: we select last item instead of selecting first item of first menu everytime

	calculateTextWidth();
	_oldPort = _gfx->SetPort(_gfx->_menuPort);
	_barSaveHandle = _gfx->BitsSave(_gfx->_menuBarRect, SCI_SCREEN_MASK_VISUAL);

	_gfx->PenColor(0);
	_gfx->BackColor(_screen->_colorWhite);

	drawBar();
	drawMenu(0, curItemEntry->menuId);
	invertMenuSelection(curItemEntry->id);
	_gfx->BitsShow(_gfx->_menuBarRect);
	_gfx->BitsShow(_menuRect);

	while (true) {
		curEvent = gfxop_get_event(_gfxstate, SCI_EVT_ANY);

		switch (curEvent.type) {
		case SCI_EVT_KEYBOARD:
			// We don't 100% follow sierra here: - sierra didn't wrap around when changing item id
			//									 - sierra allowed item id to be 0, which didnt make any sense
			do {
				switch (curEvent.data) {
				case SCI_K_ESC:
					_curMenuId = curItemEntry->menuId; _curItemId = curItemEntry->id;
					return NULL;
				case SCI_K_ENTER:
					if (curItemEntry->enabled)  {
						_curMenuId = curItemEntry->menuId; _curItemId = curItemEntry->id;
						return curItemEntry;
					}
					break;
				case SCI_K_LEFT:
					newMenuId--; newItemId = 1;
					break;
				case SCI_K_RIGHT:
					newMenuId++; newItemId = 1;
					break;
				case SCI_K_UP:
					newItemId--;
					break;
				case SCI_K_DOWN:
					newItemId++;
					break;
				}
				if ((newMenuId != curItemEntry->menuId) || (newItemId != curItemEntry->id)) {
					// Selection changed, fix up new selection if required
					newItemEntry = interactiveGetItem(newMenuId, newItemId, newMenuId != curItemEntry->menuId);
					newMenuId = newItemEntry->menuId; newItemId = newItemEntry->id;

					// if we do this step again because of a separator line -> don't repeat left/right, but go down
					switch (curEvent.data) {
					case SCI_K_LEFT:
					case SCI_K_RIGHT:
						curEvent.data = SCI_K_DOWN;
					}
				}
			} while (newItemEntry->separatorLine);
			if ((newMenuId != curItemEntry->menuId) || (newItemId != curItemEntry->id)) {
				// paint old and new
				if (newMenuId != curItemEntry->menuId) {
					// Menu changed, remove cur menu and paint new menu
					drawMenu(curItemEntry->menuId, newMenuId);
				} else {
					invertMenuSelection(curItemEntry->id);
				}
				invertMenuSelection(newItemId);

				curItemEntry = newItemEntry;
			}
			break;

		case SCI_EVT_NONE:
			gfxop_sleep(_gfxstate, 2500 / 1000);
			break;
		}
	}
}

GuiMenuItemEntry *SciGuiMenu::interactiveWithMouse() {
	calculateTextWidth();

	// TODO

	return NULL;
}

} // End of namespace Sci
