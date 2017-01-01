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

#ifndef LAB_INTERFACE_H
#define LAB_INTERFACE_H

#include "common/events.h"

namespace Lab {

class LabEngine;
class Image;

struct Button {
	uint16 _x, _y, _buttonId;
	Common::KeyCode _keyEquiv; // the key which activates this button
	bool _isEnabled;
	Image *_image, *_altImage;
};

typedef Common::List<Button *> ButtonList;

class Interface {
private:
	LabEngine *_vm;

	Button *_hitButton;
	ButtonList *_screenButtonList;

public:
	Interface(LabEngine *vm);

	void attachButtonList(ButtonList *buttonList);
	Button *createButton(uint16 x, uint16 y, uint16 id, Common::KeyCode key, Image *image, Image *altImage);
	void toggleButton(Button *button, uint16 penColor, bool enable);

	/**
	 * Draws a button list to the screen.
	 */
	void drawButtonList(ButtonList *buttonList);
	void freeButtonList(ButtonList *buttonList);
	Button *getButton(uint16 id);

	/**
	* Checks whether or not the coords fall within one of the buttons in a list
	* of buttons.
	*/
	Button *checkButtonHit(Common::Point pos);

	/**
	* Checks whether or not the coords fall within one of the buttons in a list
	* of buttons.
	*/
	Button *checkNumButtonHit(Common::KeyCode key);

	void handlePressedButton();

	void mayShowCrumbIndicator();
	void mayShowCrumbIndicatorOff();
};

} // End of namespace Lab

#endif // LAB_INTERFACE_H
