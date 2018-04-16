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
#include "lab/interface.h"
#include "lab/image.h"
#include "lab/utils.h"

namespace Lab {

#define CRUMBSWIDTH 24
#define CRUMBSHEIGHT 24

Interface::Interface(LabEngine *vm) : _vm(vm) {
	_screenButtonList = nullptr;
	_hitButton = nullptr;
}

Button *Interface::createButton(uint16 x, uint16 y, uint16 id, Common::KeyCode key, Image *image, Image *altImage) {
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

void Interface::freeButtonList(ButtonList *buttonList) {
	for (ButtonList::iterator buttonIter = buttonList->begin(); buttonIter != buttonList->end(); ++buttonIter) {
		Button *button = *buttonIter;
		delete button->_image;
		delete button->_altImage;
		delete button;
	}

	buttonList->clear();
}

void Interface::drawButtonList(ButtonList *buttonList) {
	for (ButtonList::iterator button = buttonList->begin(); button != buttonList->end(); ++button) {
		toggleButton((*button), 1, true);

		if (!(*button)->_isEnabled)
			toggleButton((*button), 1, false);
	}
}

void Interface::toggleButton(Button *button, uint16 disabledPenColor, bool enable) {
	if (!enable)
		_vm->_graphics->checkerBoardEffect(disabledPenColor, button->_x, button->_y, button->_x + button->_image->_width - 1, button->_y + button->_image->_height - 1);
	else
		button->_image->drawImage(button->_x, button->_y);

	button->_isEnabled = enable;
}

Button *Interface::checkNumButtonHit(Common::KeyCode key) {
	uint16 gkey = key - '0';

	if (!_screenButtonList)
		return nullptr;

	for (ButtonList::iterator buttonItr = _screenButtonList->begin(); buttonItr != _screenButtonList->end(); ++buttonItr) {
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

Button *Interface::checkButtonHit(Common::Point pos) {
	if (!_screenButtonList)
		return nullptr;

	for (ButtonList::iterator buttonItr = _screenButtonList->begin(); buttonItr != _screenButtonList->end(); ++buttonItr) {
		Button *button = *buttonItr;
		Common::Rect buttonRect(button->_x, button->_y, button->_x + button->_image->_width - 1, button->_y + button->_image->_height - 1);

		if (buttonRect.contains(pos) && button->_isEnabled) {
			_hitButton = button;
			return button;
		}
	}

	return nullptr;
}

void Interface::handlePressedButton() {
	if (!_hitButton)
		return;

	_hitButton->_altImage->drawImage(_hitButton->_x, _hitButton->_y);
	for (int i = 0; i < 3; i++)
		_vm->waitTOF();
	_hitButton->_image->drawImage(_hitButton->_x, _hitButton->_y);

	_hitButton = nullptr;
	_vm->_graphics->screenUpdate();
}

void Interface::attachButtonList(ButtonList *buttonList) {
	_screenButtonList = buttonList;
}

Button *Interface::getButton(uint16 id) {
	for (ButtonList::iterator buttonItr = _screenButtonList->begin(); buttonItr != _screenButtonList->end(); ++buttonItr) {
		Button *button = *buttonItr;
		if (button->_buttonId == id)
			return button;
	}

	return nullptr;
}

void Interface::mayShowCrumbIndicator() {
	static byte dropCrumbsImageData[CRUMBSWIDTH * CRUMBSHEIGHT] = {
		0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0,
		0, 4, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4, 0,
		4, 7, 7, 3, 4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 7, 7, 4,
		4, 7, 4, 4, 0, 0, 3, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 7, 4,
		4, 7, 4, 0, 0, 0, 3, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 4, 7, 4,
		4, 7, 4, 0, 0, 3, 2, 2, 2, 3, 0, 0, 0, 0, 0, 0, 0, 3, 2, 3, 0, 4, 7, 4,
		4, 7, 4, 0, 0, 0, 3, 3, 3, 4, 4, 4, 4, 4, 4, 0, 0, 3, 2, 3, 0, 4, 7, 4,
		4, 7, 4, 0, 0, 0, 0, 0, 4, 7, 7, 7, 7, 7, 7, 4, 3, 2, 2, 2, 3, 4, 7, 4,
		4, 7, 4, 0, 0, 0, 0, 4, 7, 7, 4, 4, 4, 4, 7, 7, 4, 3, 3, 3, 0, 4, 7, 4,
		4, 7, 4, 0, 0, 0, 0, 4, 7, 4, 4, 0, 0, 4, 4, 7, 4, 0, 0, 0, 0, 4, 7, 4,
		4, 7, 4, 0, 0, 0, 0, 4, 7, 4, 0, 0, 0, 0, 4, 7, 4, 0, 0, 0, 0, 4, 7, 4,
		4, 7, 4, 0, 0, 0, 0, 4, 4, 4, 3, 0, 0, 0, 4, 7, 4, 0, 0, 0, 0, 4, 7, 4,
		4, 7, 4, 0, 0, 0, 0, 0, 4, 3, 2, 3, 0, 0, 4, 7, 4, 0, 0, 0, 0, 4, 7, 4,
		4, 7, 4, 0, 0, 0, 0, 0, 0, 3, 2, 3, 0, 0, 4, 7, 4, 0, 0, 0, 0, 4, 7, 4,
		4, 7, 4, 0, 0, 0, 0, 0, 3, 2, 2, 2, 3, 4, 4, 7, 4, 0, 0, 0, 0, 4, 7, 4,
		4, 7, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7, 7, 4, 0, 0, 0, 0, 4, 7, 4,
		0, 4, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4, 0, 0, 0, 0, 0, 4, 7, 4,
		0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 0, 0, 0, 0, 0, 4, 7, 4,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 3, 0, 0, 0, 0, 4, 7, 4,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 3, 0, 0, 0, 0, 4, 7, 4,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 2, 2, 3, 0, 0, 4, 4, 7, 4,
		0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7, 7, 4,
		0, 0, 4, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4, 0,
		0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0
	};

	if (_vm->getPlatform() != Common::kPlatformWindows)
		return;

	if (_vm->_droppingCrumbs && _vm->isMainDisplay()) {
		static byte *imgData = new byte[CRUMBSWIDTH * CRUMBSHEIGHT];
		memcpy(imgData, dropCrumbsImageData, CRUMBSWIDTH * CRUMBSHEIGHT);
		static Image dropCrumbsImage(CRUMBSWIDTH, CRUMBSHEIGHT, imgData, _vm);

		dropCrumbsImage.drawMaskImage(612, 4);
	}
}

void Interface::mayShowCrumbIndicatorOff() {
	static byte dropCrumbsOffImageData[CRUMBSWIDTH * CRUMBSHEIGHT] = {
		0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0,
		0, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4, 0,
		4, 8, 8, 3, 4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 8, 8, 4,
		4, 8, 4, 4, 0, 0, 3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 8, 4,
		4, 8, 4, 0, 0, 0, 3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 4, 8, 4,
		4, 8, 4, 0, 0, 3, 8, 8, 8, 3, 0, 0, 0, 0, 0, 0, 0, 3, 8, 3, 0, 4, 8, 4,
		4, 8, 4, 0, 0, 0, 3, 3, 3, 4, 4, 4, 4, 4, 4, 0, 0, 3, 8, 3, 0, 4, 8, 4,
		4, 8, 4, 0, 0, 0, 0, 0, 4, 8, 8, 8, 8, 8, 8, 4, 3, 8, 8, 8, 3, 4, 8, 4,
		4, 8, 4, 0, 0, 0, 0, 4, 8, 8, 4, 4, 4, 4, 8, 8, 4, 3, 3, 3, 0, 4, 8, 4,
		4, 8, 4, 0, 0, 0, 0, 4, 8, 4, 4, 0, 0, 4, 4, 8, 4, 0, 0, 0, 0, 4, 8, 4,
		4, 8, 4, 0, 0, 0, 0, 4, 8, 4, 0, 0, 0, 0, 4, 8, 4, 0, 0, 0, 0, 4, 8, 4,
		4, 8, 4, 0, 0, 0, 0, 4, 4, 4, 3, 0, 0, 0, 4, 8, 4, 0, 0, 0, 0, 4, 8, 4,
		4, 8, 4, 0, 0, 0, 0, 0, 4, 3, 8, 3, 0, 0, 4, 8, 4, 0, 0, 0, 0, 4, 8, 4,
		4, 8, 4, 0, 0, 0, 0, 0, 0, 3, 8, 3, 0, 0, 4, 8, 4, 0, 0, 0, 0, 4, 8, 4,
		4, 8, 4, 0, 0, 0, 0, 0, 3, 8, 8, 8, 3, 4, 4, 8, 4, 0, 0, 0, 0, 4, 8, 4,
		4, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 4, 0, 0, 0, 0, 4, 8, 4,
		0, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4, 0, 0, 0, 0, 0, 4, 8, 4,
		0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 0, 0, 0, 0, 0, 4, 8, 4,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 8, 3, 0, 0, 0, 0, 4, 8, 4,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 8, 3, 0, 0, 0, 0, 4, 8, 4,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 8, 8, 8, 3, 0, 0, 4, 4, 8, 4,
		0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 4,
		0, 0, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4, 0,
		0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0
	};

	if (_vm->getPlatform() != Common::kPlatformWindows)
		return;

	if (_vm->isMainDisplay()) {
		static byte *imgData = new byte[CRUMBSWIDTH * CRUMBSHEIGHT];
		memcpy(imgData, dropCrumbsOffImageData, CRUMBSWIDTH * CRUMBSHEIGHT);
		static Image dropCrumbsOffImage(CRUMBSWIDTH, CRUMBSHEIGHT, imgData, _vm);

		dropCrumbsOffImage.drawMaskImage(612, 4);
	}
}

} // End of namespace Lab
