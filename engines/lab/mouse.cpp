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

#include "lab/lab.h"
#include "lab/image.h"
#include "lab/mouse.h"
#include "lab/interface.h"

namespace Lab {

static bool LeftClick = false;
static bool RightClick = false;

static bool MouseHidden = true;
static int32 NumHidden   = 1;
static Gadget *LastGadgetHit = NULL;
Gadget *ScreenGadgetList = NULL;
static byte MouseData[] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
						   1, 7, 1, 0, 0, 0, 0, 0, 0, 0,
						   1, 7, 7, 1, 0, 0, 0, 0, 0, 0,
						   1, 7, 7, 7, 1, 0, 0, 0, 0, 0,
						   1, 7, 7, 7, 7, 1, 0, 0, 0, 0,
						   1, 7, 7, 7, 7, 7, 1, 0, 0, 0,
						   1, 7, 7, 7, 7, 7, 7, 1, 0, 0,
						   1, 7, 7, 7, 7, 7, 7, 7, 1, 0,
						   1, 7, 7, 7, 7, 7, 1, 1, 1, 1,
						   1, 7, 7, 1, 7, 7, 1, 0, 0, 0,
						   1, 7, 1, 0, 1, 7, 7, 1, 0, 0,
						   1, 1, 0, 0, 1, 7, 7, 1, 0, 0,
						   0, 0, 0, 0, 0, 1, 7, 7, 1, 0,
						   0, 0, 0, 0, 0, 1, 7, 7, 1, 0,
						   0, 0, 0, 0, 0, 0, 1, 1, 0, 0};

#define MOUSE_WIDTH 10
#define MOUSE_HEIGHT 15

static Gadget *hitgad = NULL;

/*****************************************************************************/
/* Checks whether or not the cords fall within one of the gadgets in a list  */
/* of gadgets.                                                               */
/*****************************************************************************/
Gadget *EventManager::checkGadgetHit(Gadget *gadlist, Common::Point pos) {
	while (gadlist != NULL) {
		if ((pos.x >= gadlist->x) && (pos.y >= gadlist->y) &&
		    (pos.x <= (gadlist->x + gadlist->Im->Width)) &&
		    (pos.y <= (gadlist->y + gadlist->Im->Height)) &&
		     !(GADGETOFF & gadlist->GadgetFlags)) {
			if (_vm->_isHiRes) {
				hitgad = gadlist;
			} else {
				mouseHide();
				gadlist->ImAlt->drawImage(gadlist->x, gadlist->y);
				mouseShow();

				for (uint16 i = 0; i < 3; i++)
					_vm->waitTOF();

				mouseHide();
				gadlist->Im->drawImage(gadlist->x, gadlist->y);
				mouseShow();
			}

			return gadlist;
		} else {
			gadlist = gadlist->NextGadget;
		}
	}

	return NULL;
}



void EventManager::attachGadgetList(Gadget *GadList) {
	if (ScreenGadgetList != GadList)
		LastGadgetHit = NULL;

	ScreenGadgetList = GadList;
}

EventManager::EventManager(LabEngine *vm) : _vm(vm) {
}

void EventManager::mouseHandler(int flag, Common::Point pos) {
	if (NumHidden >= 2)
		return;

	if (flag & 0x02) { /* Left mouse button click */
		Gadget *tmp = NULL;
		if (ScreenGadgetList)
			tmp = checkGadgetHit(ScreenGadgetList, _vm->_isHiRes ? pos : Common::Point(pos.x / 2, pos.y));

		if (tmp)
			LastGadgetHit = tmp;
		else
			LeftClick = true;
	}

	if (flag & 0x08) /* Right mouse button click */
		RightClick = true;
}

void EventManager::updateMouse() {
	bool doUpdateDisplay = false;

	if (!MouseHidden)
		doUpdateDisplay = true;

	if (hitgad) {
		mouseHide();
		hitgad->ImAlt->drawImage(hitgad->x, hitgad->y);
		mouseShow();

		for (uint16 i = 0; i < 3; i++)
			_vm->waitTOF();

		mouseHide();
		hitgad->Im->drawImage(hitgad->x, hitgad->y);
		mouseShow();
		doUpdateDisplay = true;
		hitgad = NULL;
	}

	if (doUpdateDisplay)
		_vm->screenUpdate();
}


/*****************************************************************************/
/* Initializes the mouse.                                                    */
/*****************************************************************************/
void EventManager::initMouse() {
	g_system->setMouseCursor(MouseData, MOUSE_WIDTH, MOUSE_HEIGHT, 0, 0, 0);
	g_system->showMouse(false);

	setMousePos(Common::Point(0, 0));
}


/*****************************************************************************/
/* Shows the mouse.                                                          */
/*****************************************************************************/
void EventManager::mouseShow() {
	if (NumHidden)
		NumHidden--;

	if ((NumHidden == 0) && MouseHidden) {
		_vm->processInput();
		MouseHidden = false;
	}

	g_system->showMouse(true);
}

/*****************************************************************************/
/* Hides the mouse.                                                          */
/*****************************************************************************/
void EventManager::mouseHide() {
	NumHidden++;

	if (NumHidden && !MouseHidden) {
		MouseHidden = true;

		g_system->showMouse(false);
	}
}

/*****************************************************************************/
/* Gets the current mouse co-ordinates.  NOTE: On IBM version, will scale    */
/* from virtual to screen co-ordinates automatically.                        */
/*****************************************************************************/
Common::Point EventManager::getMousePos() {
	if (_vm->_isHiRes)
		return _vm->_mousePos;
	else
		return Common::Point(_vm->_mousePos.x / 2, _vm->_mousePos.y);
}


/*****************************************************************************/
/* Moves the mouse to new co-ordinates.                                      */
/*****************************************************************************/
void EventManager::setMousePos(Common::Point pos) {
	if (_vm->_isHiRes)
		g_system->warpMouse(pos.x, pos.y);
	else
		g_system->warpMouse(pos.x * 2, pos.y);

	if (!MouseHidden)
		_vm->processInput();
}


/*****************************************************************************/
/* Checks whether or not the mouse buttons have been pressed, and the last   */
/* co-ordinates of the button press.  leftbutton tells whether to check the  */
/* left or right button.                                                     */
/*****************************************************************************/
bool EventManager::mouseButton(uint16 *x, uint16 *y, bool leftbutton) {
	if (leftbutton) {
		if (LeftClick) {
			*x = (!_vm->_isHiRes) ? (uint16)_vm->_mousePos.x / 2 : (uint16)_vm->_mousePos.x;
			*y = (uint16)_vm->_mousePos.y;
			LeftClick = false;
			return true;
		}
	} else {
		if (RightClick) {
			*x = (!_vm->_isHiRes) ? (uint16)_vm->_mousePos.x / 2 : (uint16)_vm->_mousePos.x;
			*y = (uint16)_vm->_mousePos.y;
			RightClick = false;
			return true;
		}
	}

	return false;
}

Gadget *EventManager::mouseGadget() {
	Gadget *Temp = LastGadgetHit;

	LastGadgetHit = nullptr;
	return Temp;
}

} // End of namespace Lab
