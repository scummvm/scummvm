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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "common/events.h"

#include "lab/lab.h"

#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/image.h"
#include "lab/utils.h"

namespace Lab {

Button *EventManager::createButton(uint16 x, uint16 y, uint16 id, uint16 key, Image *image, Image *altImage) {
	Button *button = new Button();

	if (button) {
		button->_x = _vm->_utils->vgaScaleX(x);
		button->_y = y;
		button->_buttonId = id;
		button->_keyEquiv = key;
		button->_image = image;
		button->_altImage = altImage;
		button->_isEnabled = true;

		return button;
	} else
		return nullptr;
}

void EventManager::freeButtonList(ButtonList *buttonList) {
	for (ButtonList::iterator buttonIter = buttonList->begin(); buttonIter != buttonList->end(); ++buttonIter) {
		Button *button = *buttonIter;
		delete button->_image;
		delete button->_altImage;
		delete button;
	}

	buttonList->clear();
}

/**
 * Draws a button list to the screen.
 */
void EventManager::drawButtonList(ButtonList *buttonList) {
	for (ButtonList::iterator button = buttonList->begin(); button != buttonList->end(); ++button) {
		(*button)->_image->drawImage((*button)->_x, (*button)->_y);

		if (!(*button)->_isEnabled)
			disableButton((*button), 1);
	}
}

/**
 * Dims a button, and makes it unavailable for using.
 */
void EventManager::disableButton(Button *button, uint16 penColor) {
	_vm->_graphics->overlayRect(penColor, button->_x, button->_y, button->_x + button->_image->_width - 1, button->_y + button->_image->_height - 1);
	button->_isEnabled = false;
}

/**
 * Undims a button, and makes it available again.
 */
void EventManager::enableButton(Button *button) {
	button->_image->drawImage(button->_x, button->_y);
	button->_isEnabled = true;
}

/**
 * Make a key press have the right case for a button KeyEquiv value.
 */
uint16 EventManager::makeButtonKeyEquiv(uint16 key) {
	if (Common::isAlnum(key))
		key = tolower(key);

	return key;
}

/**
 * Checks whether or not the coords fall within one of the buttons in a list
 * of buttons.
 */
Button *EventManager::checkNumButtonHit(ButtonList *buttonList, uint16 key) {
	uint16 gkey = key - '0';

	if (!buttonList)
		return nullptr;

	for (ButtonList::iterator buttonItr = buttonList->begin(); buttonItr != buttonList->end(); ++buttonItr) {
		Button *button = *buttonItr;
		if (((gkey - 1 == button->_buttonId) || ((gkey == 0) && (button->_buttonId == 9))
		 || ((button->_keyEquiv != 0) && (makeButtonKeyEquiv(key) == button->_keyEquiv)))
			  && button->_isEnabled) {
			button->_altImage->drawImage(button->_x, button->_y);
			g_system->delayMillis(80);
			button->_image->drawImage(button->_x, button->_y);
			return button;
		}
	}

	return nullptr;
}

IntuiMessage *EventManager::getMsg() {
	static IntuiMessage message;

	updateMouse();

	if (_lastButtonHit) {
		updateMouse();
		message._msgClass = BUTTONUP;
		message._code = _lastButtonHit->_buttonId;
		message._buttonId = _lastButtonHit->_buttonId;
		message._qualifier = _keyPressed.flags;
		_lastButtonHit = nullptr;
		return &message;
	} else if (_leftClick || _rightClick) {
		message._qualifier = (_leftClick) ? IEQUALIFIER_LEFTBUTTON : IEQUALIFIER_RIGHTBUTTON;
		message._msgClass = MOUSEBUTTONS;
		message._mouseX = (!_vm->_isHiRes) ? (uint16)_mousePos.x / 2 : (uint16)_mousePos.x;
		message._mouseY = (uint16)_mousePos.y;
		_leftClick = _rightClick = false;
		return &message;
	} else if (keyPress(&message._code)) {
		Button *curButton = checkNumButtonHit(_screenButtonList, message._code);

		if (curButton) {
			message._msgClass = BUTTONUP;
			message._code = message._buttonId = curButton->_buttonId;
		} else
			message._msgClass = RAWKEY;

		message._qualifier = _keyPressed.flags;
		return &message;
	} else
		return nullptr;
}

} // End of namespace Lab
