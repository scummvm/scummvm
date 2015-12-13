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

#ifndef LAB_EVENTMAN_H
#define LAB_EVENTMAN_H

#include "common/events.h"

namespace Lab {

class LabEngine;
class Image;

// Defines for the Class variable in IntuiMessage
#define MOUSEBUTTONS    0x00000008
#define BUTTONUP		0x00000040
#define RAWKEY			0x00000400
#define DELTAMOVE		0x00100000

// Defines for the Qualifier variable in IntuiMessage
#define IEQUALIFIER_RIGHTBUTTON     0x2000
#define IEQUALIFIER_LEFTBUTTON      0x4000

#define VKEY_UPARROW    273
#define VKEY_DNARROW    274
#define VKEY_RTARROW    275
#define VKEY_LTARROW    276

struct IntuiMessage {
	uint32 _msgClass;
	uint16 _code, _qualifier, _mouseX, _mouseY, _buttonID;
	uint32 _seconds, _micros;
};


struct Button {
	uint16 x, y, _buttonID;
	uint16 _keyEquiv; // if not zero, a key that activates button
	bool isEnabled;
	Image *_image, *_altImage;
};

typedef Common::List<Button *> ButtonList;

class EventManager {
private:
	LabEngine *_vm;
	bool _leftClick;
	bool _rightClick;

	bool _mouseHidden;
	Button *_lastButtonHit;
	uint16 _nextKeyIn;
	uint16 _nextKeyOut;
	Common::Point _mousePos;
	bool _mouseAtEdge;
	uint16 _keyBuf[64];

public:
	EventManager (LabEngine *vm);

	ButtonList *_screenButtonList;
	Button *_hitButton;
	Common::KeyState _keyPressed;

	Button *checkButtonHit(ButtonList *buttonList, Common::Point pos);
	void initMouse();
	void updateMouse();
	void mouseShow();
	void mouseHide();
	Common::Point getMousePos();
	void setMousePos(Common::Point pos);
	bool mouseButton(uint16 *x, uint16 *y, bool leftButton);
	Button *mouseButton();
	void attachButtonList(ButtonList *buttonList);
	Button *getButton(uint16 id);
	void mouseHandler(int flag, Common::Point pos);
	bool keyPress(uint16 *keyCode);
	bool haveNextChar();
	void processInput(bool can_delay = false);
	uint16 getNextChar();
	Common::Point updateAndGetMousePos();

	Button *checkNumButtonHit(ButtonList *buttonList, uint16 key);
	Button *createButton(uint16 x, uint16 y, uint16 id, uint16 key, Image *im, Image *imalt);
	void freeButtonList(ButtonList *buttonList);
	void drawButtonList(ButtonList *buttonList);
	void disableButton(Button *curgad, uint16 pencolor);
	void enableButton(Button *curgad);
	IntuiMessage *getMsg();
	uint16 makeButtonKeyEquiv(uint16 key);
};

} // End of namespace Lab

#endif // LAB_EVENTMAN_H
