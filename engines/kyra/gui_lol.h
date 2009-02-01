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

#ifndef KYRA_GUI_LOL_H
#define KYRA_GUI_LOL_H

#include "kyra/gui.h"

namespace Kyra {

class LoLEngine;
class Screen_LoL;

class GUI_LoL : public GUI {
	friend class LoLEngine;
public:
	GUI_LoL(LoLEngine *vm);

	void initStaticData();

	// button specific
	void processButton(Button *button) {}
	int processButtonList(Button *buttonList, uint16 inputFlags, int8 mouseWheel);

	// utilities for thumbnail creation
	void createScreenThumbnail(Graphics::Surface &dst) {}

	// tim player specific
	void drawDialogueBox(int numStr, const char *s1, const char *s2, const char *s3);
	uint16 processDialogue();
	void update();
	char *getTableString(int id);

private:
	LoLEngine *_vm;
	Screen_LoL *_screen;

	bool _pressFlag;

	int scrollUp(Button *button) { return 0; }
	int scrollDown(Button *button) { return 0; }

	Button *getButtonListData() { return 0; }
	Button *getScrollUpButton() { return 0; }
	Button *getScrollDownButton() { return 0; }

	Button::Callback _scrollUpFunctor;
	Button::Callback _scrollDownFunctor;
	Button::Callback getScrollUpButtonHandler() const { return _scrollUpFunctor; }
	Button::Callback getScrollDownButtonHandler() const { return _scrollDownFunctor; }

	uint8 defaultColor1() const { return 0; }
	uint8 defaultColor2() const { return 0; }

	const char *getMenuTitle(const Menu &menu) { return 0; }
	const char *getMenuItemTitle(const MenuItem &menuItem) { return 0; }
	const char *getMenuItemLabel(const MenuItem &menuItem) { return 0; }

	void drawDialogueButtons();

	const char *_dialogueButtonString[3];
	uint16 _dialogueButtonPosX;
	uint16 _dialogueButtonPosY;
	int _dialogueNumButtons;
	uint16 _dialogueButtonXoffs;
	int _dialogueHighlightedButton;
};

} // end of namespace Kyra

#endif

