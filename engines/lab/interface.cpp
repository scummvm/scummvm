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
#include "lab/interface.h"
#include "lab/utils.h"

namespace Lab {

Common::KeyState _keyPressed;

Gadget *createButton(uint16 x, uint16 y, uint16 id, uint16 key, Image *im, Image *imalt) {
	Gadget *gptr = new Gadget();

	if (gptr) {
		gptr->x = g_lab->_utils->vgaScaleX(x);
		gptr->y = y;
		gptr->_gadgetID = id;
		gptr->_keyEquiv = key;
		gptr->_image = im;
		gptr->_altImage = imalt;
		gptr->isEnabled = true;

		return gptr;
	} else
		return nullptr;
}

void freeButtonList(GadgetList *gadgetList) {
	for (GadgetList::iterator gadgetIter = gadgetList->begin(); gadgetIter != gadgetList->end(); ++gadgetIter) {
		Gadget *gadget = *gadgetIter;
		delete gadget->_image;
		delete gadget->_altImage;
		delete gadget;
	}

	gadgetList->clear();
}

/**
 * Draws a gadget list to the screen.
 */
void drawGadgetList(GadgetList *gadgetList) {
	for (GadgetList::iterator gadget = gadgetList->begin(); gadget != gadgetList->end(); ++gadget) {
		(*gadget)->_image->drawImage((*gadget)->x, (*gadget)->y);

		if (!(*gadget)->isEnabled)
			disableGadget((*gadget), 1);
	}
}

/**
 * Dims a gadget, and makes it unavailable for using.
 */
void disableGadget(Gadget *curgad, uint16 pencolor) {
	g_lab->_graphics->overlayRect(pencolor, curgad->x, curgad->y, curgad->x + curgad->_image->_width - 1, curgad->y + curgad->_image->_height - 1);
	curgad->isEnabled = false;
}

/**
 * Undims a gadget, and makes it available again.
 */
void enableGadget(Gadget *curgad) {
	curgad->_image->drawImage(curgad->x, curgad->y);
	curgad->isEnabled = true;
}

/**
 * Make a key press have the right case for a gadget KeyEquiv value.
 */
uint16 makeGadgetKeyEquiv(uint16 key) {
	if (Common::isAlnum(key))
		key = tolower(key);

	return key;
}

/**
 * Checks whether or not the coords fall within one of the gadgets in a list
 * of gadgets.
 */
Gadget *LabEngine::checkNumGadgetHit(GadgetList *gadgetList, uint16 key) {
	uint16 gkey = key - '0';

	if (!gadgetList)
		return nullptr;

	for (GadgetList::iterator gadgetItr = gadgetList->begin(); gadgetItr != gadgetList->end(); ++gadgetItr) {
		Gadget *gadget = *gadgetItr;
		if ((gkey - 1 == gadget->_gadgetID || (gkey == 0 && gadget->_gadgetID == 9) ||
			  (gadget->_keyEquiv != 0 && makeGadgetKeyEquiv(key) == gadget->_keyEquiv))
			  && gadget->isEnabled) {
			_event->mouseHide();
			gadget->_altImage->drawImage(gadget->x, gadget->y);
			_event->mouseShow();
			g_system->delayMillis(80);
			_event->mouseHide();
			gadget->_image->drawImage(gadget->x, gadget->y);
			_event->mouseShow();

			return gadget;
		}
	}

	return NULL;
}

IntuiMessage IMessage;

IntuiMessage *LabEngine::getMsg() {
	_event->updateMouse();

	int qualifiers = _keyPressed.flags;
	Gadget *curgad  = _event->mouseGadget();

	if (curgad) {
		_event->updateMouse();
		IMessage._msgClass = GADGETUP;
		IMessage._code  = curgad->_gadgetID;
		IMessage._gadgetID = curgad->_gadgetID;
		IMessage._qualifier = qualifiers;
		return &IMessage;
	} else if (_event->mouseButton(&IMessage._mouseX, &IMessage._mouseY, true)) {
		// Left Button
		IMessage._qualifier = IEQUALIFIER_LEFTBUTTON | qualifiers;
		IMessage._msgClass = MOUSEBUTTONS;
		return &IMessage;
	} else if (_event->mouseButton(&IMessage._mouseX, &IMessage._mouseY, false)) {
		// Right Button
		IMessage._qualifier = IEQUALIFIER_RBUTTON | qualifiers;
		IMessage._msgClass = MOUSEBUTTONS;
		return &IMessage;
	} else if (_event->keyPress(&IMessage._code)) {
		// Keyboard key
		curgad = checkNumGadgetHit(_event->_screenGadgetList, IMessage._code);

		if (curgad) {
			IMessage._msgClass = GADGETUP;
			IMessage._code  = curgad->_gadgetID;
			IMessage._gadgetID = curgad->_gadgetID;
		} else
			IMessage._msgClass = RAWKEY;

		IMessage._qualifier = qualifiers;
		return &IMessage;
	} else
		return nullptr;
}

} // End of namespace Lab
