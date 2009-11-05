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
	_oldPort = NULL;
}

SciGuiMenu::~SciGuiMenu() {
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
		itemEntry->said = _segMan->getString(value);
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
	_gfx->FillRect(_gfx->_menuRect, 1, _screen->_colorWhite);
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
		while (itemIterator != itemEnd) {
			itemEntry = *itemIterator;
			// TODO: said comparsion
			//  said(_s, itemEntry->said.c_str(), 0) != SAID_NO_MATCH
			// Where is this used anyway, so we can test it out?
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
		_gfx->BitsShow(_gfx->_menuRect);
	}
	if (_oldPort)
		_gfx->SetPort(_oldPort);

	if ((itemEntry) || (forceClaimed))
		PUT_SEL32(_segMan, eventObject, claimed, make_reg(0, 1));
	if (itemEntry)
		return make_reg(0, (itemEntry->menuId << 8) | (itemEntry->id));
	return NULL_REG;
}

GuiMenuItemEntry *SciGuiMenu::interactiveGetItem(uint16 menuId, uint16 itemId) {
	GuiMenuItemList::iterator itemIterator = _itemList.begin();
	GuiMenuItemList::iterator itemEnd = _itemList.end();
	GuiMenuItemEntry *itemEntry;
	GuiMenuItemEntry *lastItemEntry = NULL;

	// Fixup menuId if needed
	menuId = CLIP<uint16>(menuId, 1, _listCount);
	// Fixup itemId as well
	if (itemId == 0)
		itemId = 32678;
	while (itemIterator != itemEnd) {
		itemEntry = *itemIterator;
		if (itemEntry->menuId == menuId) {
			if (itemEntry->id == itemId)
				return itemEntry;
			if ((!lastItemEntry) || (itemEntry->id > lastItemEntry->id))
				lastItemEntry = itemEntry;
		}
		itemIterator++;
	}
	return lastItemEntry;
}

GuiMenuItemEntry *SciGuiMenu::interactiveWithKeyboard() {
	sci_event_t curEvent;
	uint16 newMenuId = _curMenuId;
	uint16 newItemId = _curItemId;
	GuiMenuItemEntry *curItemEntry = findItem(_curMenuId, _curItemId);
	GuiMenuItemEntry *newItemEntry = curItemEntry;

	calculateTextWidth();
	_oldPort = _gfx->SetPort(_gfx->_menuPort);
	_menuSaveHandle = _gfx->BitsSave(_gfx->_menuRect, SCI_SCREEN_MASK_VISUAL);
	drawBar();

	_gfx->BitsShow(_gfx->_menuRect);

	while (true) {
		curEvent = gfxop_get_event(_gfxstate, SCI_EVT_ANY);

		switch (curEvent.type) {
		case SCI_EVT_KEYBOARD:
			switch (curEvent.data) {
			case SCI_K_ESC:
				_curMenuId = curItemEntry->menuId; _curItemId = curItemEntry->id;
				return NULL;
			case SCI_K_ENTER:
				_curMenuId = curItemEntry->menuId; _curItemId = curItemEntry->id;
				return curItemEntry;
			case SCI_K_LEFT:
				newMenuId--;
				break;
			case SCI_K_RIGHT:
				newMenuId++;
				break;
			case SCI_K_UP:
				newItemId--;
				break;
			case SCI_K_DOWN:
				newItemId++;
				break;
			}
			if ((newMenuId != curItemEntry->menuId) || (newItemId != curItemEntry->id)) {
				// Selection changed, fix up new selection if required and paint old and new
				newItemEntry = interactiveGetItem(newMenuId, newItemId);
				newMenuId = newItemEntry->menuId; newItemId = newItemEntry->id;

				if (newMenuId != curItemEntry->menuId) {
					// Menu changed, remove cur menu and paint new menu
				}

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

	return NULL;
}

} // End of namespace Sci
