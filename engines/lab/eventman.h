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

enum MessageClasses {
	kMessageLeftClick,
	kMessageRightClick,
	kMessageButtonUp,
	kMessageRawKey,
	kMessageDeltaMove
};

#define VKEY_UPARROW    273
#define VKEY_DNARROW    274
#define VKEY_RTARROW    275
#define VKEY_LTARROW    276

struct IntuiMessage {
	uint32 _msgClass;
	uint16 _code, _qualifier, _mouseX, _mouseY;
	uint32 _seconds, _micros;
};


struct Button {
	uint16 _x, _y, _buttonId;
	uint16 _keyEquiv; // if not zero, a key that activates button
	bool _isEnabled;
	Image *_image, *_altImage;
};

typedef Common::List<Button *> ButtonList;

class EventManager {
private:
	LabEngine *_vm;

	bool _leftClick;
	bool _rightClick;
	bool _mouseHidden;

	uint16 _nextKeyIn;
	uint16 _nextKeyOut;
	uint16 _keyBuf[64];

	Button *_hitButton;
	Button *_lastButtonHit;
	ButtonList *_screenButtonList;
	Common::Point _mousePos;
	Common::KeyState _keyPressed;

private:
	Button *checkButtonHit(ButtonList *buttonList, Common::Point pos);
	bool keyPress(uint16 *keyCode);
	bool haveNextChar();
	uint16 getNextChar();
	Button *checkNumButtonHit(ButtonList *buttonList, uint16 key);
	uint16 makeButtonKeyEquiv(uint16 key);

public:
	EventManager (LabEngine *vm);

	void attachButtonList(ButtonList *buttonList);
	Button *createButton(uint16 x, uint16 y, uint16 id, uint16 key, Image *image, Image *altImage);
	void disableButton(Button *button, uint16 penColor);
	void drawButtonList(ButtonList *buttonList);
	void enableButton(Button *button);
	void freeButtonList(ButtonList *buttonList);
	Button *getButton(uint16 id);
	Common::Point getMousePos();
	IntuiMessage *getMsg();
	void initMouse();
	void mouseShow();
	void mouseHide();
	void processInput();
	void setMousePos(Common::Point pos);
	void updateMouse();
	Common::Point updateAndGetMousePos();
};

} // End of namespace Lab

#endif // LAB_EVENTMAN_H
