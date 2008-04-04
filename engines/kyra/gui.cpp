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

#include "kyra/gui.h"

#include "kyra/screen.h"
#include "kyra/text.h"

#include "common/savefile.h"

namespace Kyra {

GUI::GUI(KyraEngine *kyra)
	: _vm(kyra), _screen(kyra->screen()), _text(kyra->text()) {
	_menuButtonList = 0;
	_haveScrollButtons = false;

	_redrawButtonFunctor = BUTTON_FUNCTOR(GUI, this, &GUI::redrawButtonCallback);
	_redrawShadedButtonFunctor = BUTTON_FUNCTOR(GUI, this, &GUI::redrawShadedButtonCallback);
}

Button *GUI::addButtonToList(Button *list, Button *newButton) {
	if (!newButton)
		return list;

	newButton->nextButton = 0;

	if (list) {
		Button *cur = list;
		while (cur->nextButton)
			cur = cur->nextButton;
		cur->nextButton = newButton;
	} else {
		list = newButton;
	}

	return list;
}

void GUI::initMenuLayout(Menu &menu) {
	if (menu.x == -1)
		menu.x = (320 - menu.width) >> 1;
	if (menu.y == -1)
		menu.y = (200 - menu.height) >> 1;

	for (int i = 0; i < menu.numberOfItems; ++i) {
		if (menu.item[i].x == -1)
			menu.item[i].x = (menu.width - menu.item[i].width) >> 1;
	}
}

void GUI::initMenu(Menu &menu) {
	_menuButtonList = 0;

	_screen->hideMouse();

	int textX;
	int textY;

	int menu_x2 = menu.width  + menu.x - 1;
	int menu_y2 = menu.height + menu.y - 1;

	_screen->fillRect(menu.x + 2, menu.y + 2, menu_x2 - 2, menu_y2 - 2, menu.bkgdColor);
	_screen->drawShadedBox(menu.x, menu.y, menu_x2, menu_y2, menu.color1, menu.color2);

	if (menu.titleX != -1)
		textX = menu.titleX;
	else
		textX = _text->getCenterStringX(getMenuTitle(menu), menu.x, menu_x2);

	textY = menu.y + menu.titleY;

	_text->printText(getMenuTitle(menu), textX - 1, textY + 1, defaultColor1(), defaultColor2(), 0);
	_text->printText(getMenuTitle(menu), textX, textY, menu.textColor, 0, 0);

	int x1, y1, x2, y2;
	for (int i = 0; i < menu.numberOfItems; ++i) {
		if (!menu.item[i].enabled)
			continue;

		x1 = menu.x + menu.item[i].x;
		y1 = menu.y + menu.item[i].y;

		x2 = x1 + menu.item[i].width - 1;
		y2 = y1 + menu.item[i].height - 1;

		if (i < 7) {
			Button *menuButtonData = getButtonListData() + i;
			menuButtonData->nextButton = 0;
			menuButtonData->x = x1;
			menuButtonData->y = y1;
			menuButtonData->width  = menu.item[i].width - 1;
			menuButtonData->height = menu.item[i].height - 1;
			menuButtonData->buttonCallback = menu.item[i].callback;
			menuButtonData->unk6 = menu.item[i].unk1F;
			menuButtonData->unk8 = 0;

			_menuButtonList = addButtonToList(_menuButtonList, menuButtonData);
		}

		_screen->fillRect(x1, y1, x2, y2, menu.item[i].bkgdColor);
		_screen->drawShadedBox(x1, y1, x2, y2, menu.item[i].color1, menu.item[i].color2);

		if (getMenuItemTitle(menu.item[i])) {
			if (menu.item[i].titleX != -1)
				textX = x1 + menu.item[i].titleX + 3;
			else
				textX = _text->getCenterStringX(getMenuItemTitle(menu.item[i]), x1, x2);

			textY = y1 + 2;
			_text->printText(getMenuItemTitle(menu.item[i]), textX - 1, textY + 1, defaultColor1(), 0, 0);

			if (i == menu.highlightedItem)
				_text->printText(getMenuItemTitle(menu.item[i]), textX, textY, menu.item[i].highlightColor, 0, 0);
			else
				_text->printText(getMenuItemTitle(menu.item[i]), textX, textY, menu.item[i].textColor, 0, 0);

			if (getMenuItemLabel(menu.item[i])) {
				_text->printText(getMenuItemLabel(menu.item[i]), menu.x + menu.item[i].labelX - 1, menu.y + menu.item[i].labelY + 1, defaultColor1(), 0, 0);
				_text->printText(getMenuItemLabel(menu.item[i]), menu.x + menu.item[i].labelX, menu.y + menu.item[i].labelY, menu.item[i].textColor, 0, 0);
			}
		}
	}

	if (menu.scrollUpButtonX != -1) {
		_haveScrollButtons = true;

		Button *scrollUpButton = getScrollUpButton();
		scrollUpButton->x = menu.scrollUpButtonX + menu.x;
		scrollUpButton->y = menu.scrollUpButtonY + menu.y;
		scrollUpButton->buttonCallback = getScrollUpButtonHandler();
		scrollUpButton->nextButton = 0;
		
		_menuButtonList = addButtonToList(_menuButtonList, scrollUpButton);
		updateMenuButton(scrollUpButton);

		Button *scrollDownButton = getScrollDownButton();
		scrollDownButton->x = menu.scrollDownButtonX + menu.x;
		scrollDownButton->y = menu.scrollDownButtonY + menu.y;
		scrollDownButton->buttonCallback = getScrollDownButtonHandler();
		scrollDownButton->nextButton = 0;

		_menuButtonList = addButtonToList(_menuButtonList, scrollDownButton);
		updateMenuButton(scrollDownButton);
	} else {
		_haveScrollButtons = false;
	}

	_screen->showMouse();
	_screen->updateScreen();
}

void GUI::processHighlights(Menu &menu, int mouseX, int mouseY) {
	int x1, y1, x2, y2;

	for (int i = 0; i < menu.numberOfItems; ++i) {
		if (!menu.item[i].enabled)
			continue;

		x1 = menu.x + menu.item[i].x;
		y1 = menu.y + menu.item[i].y;

		x2 = x1 + menu.item[i].width;
		y2 = y1 + menu.item[i].height;

		if (mouseX > x1 && mouseX < x2 &&
			mouseY > y1 && mouseY < y2) {

			if (menu.highlightedItem != i) {
				if (menu.item[menu.highlightedItem].enabled)
					redrawText(menu);

				menu.highlightedItem = i;
				redrawHighlight(menu);
				_screen->updateScreen();
			}
		}
	}
}

void GUI::redrawText(const Menu &menu) {
	int textX;
	int i = menu.highlightedItem;

	int x1 = menu.x + menu.item[i].x;
	int y1 = menu.y + menu.item[i].y;

	int x2 = x1 + menu.item[i].width - 1;

	if (menu.item[i].titleX >= 0)
		textX = x1 + menu.item[i].titleX + 3;
	else
		textX = _text->getCenterStringX(getMenuItemTitle(menu.item[i]), x1, x2);

	int textY = y1 + 2;
	_text->printText(getMenuItemTitle(menu.item[i]), textX - 1, textY + 1, defaultColor1(), 0, 0);
	_text->printText(getMenuItemTitle(menu.item[i]), textX, textY, menu.item[i].textColor, 0, 0);
}

void GUI::redrawHighlight(const Menu &menu) {
	int textX;
	int i = menu.highlightedItem;

	int x1 = menu.x + menu.item[i].x;
	int y1 = menu.y + menu.item[i].y;

	int x2 = x1 + menu.item[i].width - 1;

	if (menu.item[i].titleX != -1)
		textX = x1 + menu.item[i].titleX + 3;
	else
		textX = _text->getCenterStringX(getMenuItemTitle(menu.item[i]), x1, x2);

	int textY = y1 + 2;
	_text->printText(getMenuItemTitle(menu.item[i]), textX - 1, textY + 1, defaultColor1(), 0, 0);
	_text->printText(getMenuItemTitle(menu.item[i]), textX, textY, menu.item[i].highlightColor, 0, 0);
}

void GUI::updateAllMenuButtons() {
	for (Button *cur = _menuButtonList; cur; cur = cur->nextButton)
		updateMenuButton(cur);
}

void GUI::updateMenuButton(Button *button) {
	if (!_displayMenu)
		return;

	_screen->hideMouse();
	updateButton(button);
	_screen->updateScreen();
	_screen->showMouse();
}

void GUI::updateButton(Button *button) {
	if (!button || (button->flags & 8))
		return;

	if (button->flags2 & 1)
		button->flags2 &= 0xFFF7;
	else
		button->flags2 |= 8;

	button->flags2 &= 0xFFFC;

	if (button->flags2 & 4)
		button->flags2 |= 0x10;
	else
		button->flags2 &= 0xEEEF;

	button->flags2 &= 0xFFFB;

	processButton(button);
}

int GUI::redrawButtonCallback(Button *button) {
	if (!_displayMenu)
		return 0;

	_screen->hideMouse();
	_screen->drawBox(button->x + 1, button->y + 1, button->x + button->width - 1, button->y + button->height - 1, 0xF8);
	_screen->showMouse();

	return 0;
}

int GUI::redrawShadedButtonCallback(Button *button) {
	if (!_displayMenu)
		return 0;

	_screen->hideMouse();
	_screen->drawShadedBox(button->x, button->y, button->x + button->width, button->y + button->height, 0xF9, 0xFA);
	_screen->showMouse();

	return 0;
}

int GUI::getNextSavegameSlot() {
	Common::InSaveFile *in;

	for (int i = 1; i < 1000; i++) {
		if ((in = _vm->_saveFileMan->openForLoading(_vm->getSavegameFilename(i))))
			delete in;
		else
			return i;
	}
	warning("Didn't save: Ran out of saveGame filenames");
	return 0;
}

} // end of namespace Kyra

