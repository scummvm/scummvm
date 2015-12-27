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

Button *EventManager::createButton(uint16 x, uint16 y, uint16 id, Common::KeyCode key, Image *image, Image *altImage) {
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

void EventManager::drawButtonList(ButtonList *buttonList) {
	for (ButtonList::iterator button = buttonList->begin(); button != buttonList->end(); ++button) {
		toggleButton((*button), 1, true);

		if (!(*button)->_isEnabled)
			toggleButton((*button), 1, false);
	}
}

void EventManager::toggleButton(Button *button, uint16 disabledPenColor, bool enable) {
	if (!enable)
		_vm->_graphics->checkerBoardEffect(disabledPenColor, button->_x, button->_y, button->_x + button->_image->_width - 1, button->_y + button->_image->_height - 1);
	else
		button->_image->drawImage(button->_x, button->_y);

	button->_isEnabled = enable;
}

Button *EventManager::checkNumButtonHit(ButtonList *buttonList, Common::KeyCode key) {
	uint16 gkey = key - '0';

	if (!buttonList)
		return nullptr;

	for (ButtonList::iterator buttonItr = buttonList->begin(); buttonItr != buttonList->end(); ++buttonItr) {
		Button *button = *buttonItr;
		if (!button->_isEnabled)
			continue;

		if ((gkey - 1 == button->_buttonId) || (gkey == 0 && button->_buttonId == 9) || (button->_keyEquiv != Common::KEYCODE_INVALID && key == button->_keyEquiv)) {
			button->_altImage->drawImage(button->_x, button->_y);
			_vm->_system->delayMillis(80);
			button->_image->drawImage(button->_x, button->_y);
			return button;
		}
	}

	return nullptr;
}

IntuiMessage *EventManager::getMsg() {
	static IntuiMessage message;

	updateMouse();
	processInput();

	if (_lastButtonHit) {
		updateMouse();
		message._msgClass = kMessageButtonUp;
		message._code = _lastButtonHit->_buttonId;
		message._qualifier = _keyPressed.flags;
		_lastButtonHit = nullptr;
		return &message;
	} else if (_leftClick || _rightClick) {
		message._msgClass = (_leftClick) ? kMessageLeftClick : kMessageRightClick;
		message._qualifier = 0;
		message._mouse = _mousePos;
		if (!_vm->_isHiRes)
			message._mouse.x /= 2;
		_leftClick = _rightClick = false;
		return &message;
	} else if (_keyPressed.keycode != Common::KEYCODE_INVALID) {
		Button *curButton = checkNumButtonHit(_screenButtonList, _keyPressed.keycode);

		if (curButton) {
			message._msgClass = kMessageButtonUp;
			message._code = curButton->_buttonId;
		} else {
			message._msgClass = kMessageRawKey;
			message._code = _keyPressed.keycode;
		}

		message._qualifier = _keyPressed.flags;
		message._mouse = _mousePos;

		_keyPressed.keycode = Common::KEYCODE_INVALID;

		return &message;
	} else
		return nullptr;
}

} // End of namespace Lab
