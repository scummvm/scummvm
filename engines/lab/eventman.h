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

struct IntuiMessage {
	uint32 _msgClass;
	uint16 _code, _qualifier, _mouseX, _mouseY, _gadgetID;
	uint32 _seconds, _micros;
};


struct Gadget {
	uint16 x, y, _gadgetID;
	uint16 _keyEquiv; // if not zero, a key that activates gadget
	bool isEnabled;
	Image *_image, *_altImage;
};

typedef Common::List<Gadget *> GadgetList;

class EventManager {
private:
	LabEngine *_vm;
	bool _leftClick;
	bool _rightClick;

	bool _mouseHidden;
	Gadget *_lastGadgetHit;
	uint16 _nextKeyIn;
	uint16 _nextKeyOut;
	Common::Point _mousePos;
	bool _mouseAtEdge;
	uint16 _keyBuf[64];

public:
	EventManager (LabEngine *vm);

	GadgetList *_screenGadgetList;
	Gadget *_hitGadget;
	Common::KeyState _keyPressed;

	Gadget *checkGadgetHit(GadgetList *gadgetList, Common::Point pos);
	void initMouse();
	void updateMouse();
	void mouseShow();
	void mouseHide();
	Common::Point getMousePos();
	void setMousePos(Common::Point pos);
	bool mouseButton(uint16 *x, uint16 *y, bool leftButton);
	Gadget *mouseGadget();
	void attachGadgetList(GadgetList *gadgetList);
	Gadget *getGadget(uint16 id);
	void mouseHandler(int flag, Common::Point pos);
	bool keyPress(uint16 *keyCode);
	bool haveNextChar();
	void processInput(bool can_delay = false);
	uint16 getNextChar();
	Common::Point updateAndGetMousePos();

	Gadget *checkNumGadgetHit(GadgetList *gadgetList, uint16 key);
	Gadget *createButton(uint16 x, uint16 y, uint16 id, uint16 key, Image *im, Image *imalt);
	void freeButtonList(GadgetList *gadgetList);
	void drawGadgetList(GadgetList *gadgetList);
	void disableGadget(Gadget *curgad, uint16 pencolor);
	void enableGadget(Gadget *curgad);
	IntuiMessage *getMsg();
	uint16 makeGadgetKeyEquiv(uint16 key);
};

} // End of namespace Lab

#endif // LAB_EVENTMAN_H
