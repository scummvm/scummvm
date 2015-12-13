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

Gadget *EventManager::createButton(uint16 x, uint16 y, uint16 id, uint16 key, Image *image, Image *altImage) {
	Gadget *gadget = new Gadget();

	if (gadget) {
		gadget->x = _vm->_utils->vgaScaleX(x);
		gadget->y = y;
		gadget->_gadgetID = id;
		gadget->_keyEquiv = key;
		gadget->_image = image;
		gadget->_altImage = altImage;
		gadget->isEnabled = true;

		return gadget;
	} else
		return nullptr;
}

void EventManager::freeButtonList(GadgetList *gadgetList) {
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
void EventManager::drawGadgetList(GadgetList *gadgetList) {
	for (GadgetList::iterator gadget = gadgetList->begin(); gadget != gadgetList->end(); ++gadget) {
		(*gadget)->_image->drawImage((*gadget)->x, (*gadget)->y);

		if (!(*gadget)->isEnabled)
			disableGadget((*gadget), 1);
	}
}

/**
 * Dims a gadget, and makes it unavailable for using.
 */
void EventManager::disableGadget(Gadget *gadget, uint16 penColor) {
	_vm->_graphics->overlayRect(penColor, gadget->x, gadget->y, gadget->x + gadget->_image->_width - 1, gadget->y + gadget->_image->_height - 1);
	gadget->isEnabled = false;
}

/**
 * Undims a gadget, and makes it available again.
 */
void EventManager::enableGadget(Gadget *gadget) {
	gadget->_image->drawImage(gadget->x, gadget->y);
	gadget->isEnabled = true;
}

/**
 * Make a key press have the right case for a gadget KeyEquiv value.
 */
uint16 EventManager::makeGadgetKeyEquiv(uint16 key) {
	if (Common::isAlnum(key))
		key = tolower(key);

	return key;
}

/**
 * Checks whether or not the coords fall within one of the gadgets in a list
 * of gadgets.
 */
Gadget *EventManager::checkNumGadgetHit(GadgetList *gadgetList, uint16 key) {
	uint16 gkey = key - '0';

	if (!gadgetList)
		return nullptr;

	for (GadgetList::iterator gadgetItr = gadgetList->begin(); gadgetItr != gadgetList->end(); ++gadgetItr) {
		Gadget *gadget = *gadgetItr;
		if ((gkey - 1 == gadget->_gadgetID || (gkey == 0 && gadget->_gadgetID == 9) ||
			  (gadget->_keyEquiv != 0 && makeGadgetKeyEquiv(key) == gadget->_keyEquiv))
			  && gadget->isEnabled) {
			mouseHide();
			gadget->_altImage->drawImage(gadget->x, gadget->y);
			mouseShow();
			g_system->delayMillis(80);
			mouseHide();
			gadget->_image->drawImage(gadget->x, gadget->y);
			mouseShow();

			return gadget;
		}
	}

	return nullptr;
}

IntuiMessage *LabEngine::getMsg() {
	static IntuiMessage message;

	_event->updateMouse();

	int qualifiers = _event->_keyPressed.flags;
	Gadget *curgad  = _event->mouseGadget();

	if (curgad) {
		_event->updateMouse();
		message._msgClass = GADGETUP;
		message._code  = curgad->_gadgetID;
		message._gadgetID = curgad->_gadgetID;
		message._qualifier = qualifiers;
		return &message;
	} else if (_event->mouseButton(&message._mouseX, &message._mouseY, true)) {
		// Left Button
		message._qualifier = IEQUALIFIER_LEFTBUTTON | qualifiers;
		message._msgClass = MOUSEBUTTONS;
		return &message;
	} else if (_event->mouseButton(&message._mouseX, &message._mouseY, false)) {
		// Right Button
		message._qualifier = IEQUALIFIER_RBUTTON | qualifiers;
		message._msgClass = MOUSEBUTTONS;
		return &message;
	} else if (_event->keyPress(&message._code)) {
		// Keyboard key
		curgad = _event->checkNumGadgetHit(_event->_screenGadgetList, message._code);

		if (curgad) {
			message._msgClass = GADGETUP;
			message._code  = curgad->_gadgetID;
			message._gadgetID = curgad->_gadgetID;
		} else
			message._msgClass = RAWKEY;

		message._qualifier = qualifiers;
		return &message;
	} else
		return nullptr;
}

} // End of namespace Lab
