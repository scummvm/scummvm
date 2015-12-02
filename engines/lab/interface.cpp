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
#include "lab/labfun.h"
#include "lab/image.h"
#include "lab/interface.h"
#include "lab/mouse.h"
#include "common/util.h"

namespace Lab {

extern bool IsHiRes;

Common::KeyState _keyPressed;

Gadget *createButton(uint16 x, uint16 y, uint16 id, uint16 key, Image *im, Image *imalt) {
	Gadget *gptr;

	x = VGAScaleX(x);

	if ((gptr = new Gadget())) {
		gptr->x = x;
		gptr->y = y;
		gptr->GadgetID = id;
		gptr->KeyEquiv = key;
		gptr->Im = im;
		gptr->ImAlt = imalt;
		gptr->NextGadget = NULL;

		return gptr;
	} else
		return NULL;
}




void freeButtonList(Gadget *gptrlist) {
	Gadget *next = gptrlist;

	while (next) {
		Gadget *gptr = next;
		next = next->NextGadget;

		free(gptr);
	}
}




/*****************************************************************************/
/* Draws a gadget list to the screen.                                        */
/*****************************************************************************/
void drawGadgetList(Gadget *gadlist) {
	while (gadlist) {
		gadlist->Im->drawImage(gadlist->x, gadlist->y);

		if (GADGETOFF & gadlist->GadgetFlags)
			ghoastGadget(gadlist, 1);

		gadlist = gadlist->NextGadget;
	}
}


/*****************************************************************************/
/* Ghoasts a gadget, and makes it unavailable for using.                     */
/*****************************************************************************/
void ghoastGadget(Gadget *curgad, uint16 pencolor) {
	g_lab->ghoastRect(pencolor, curgad->x, curgad->y, curgad->x + curgad->Im->Width - 1, curgad->y + curgad->Im->Height - 1);
	curgad->GadgetFlags |= GADGETOFF;
}



/*****************************************************************************/
/* Unghoasts a gadget, and makes it available again.                         */
/*****************************************************************************/
void unGhoastGadget(Gadget *curgad) {
	curgad->Im->drawImage(curgad->x, curgad->y);
	curgad->GadgetFlags &= !(GADGETOFF);
}


/*****************************************************************************/
/* Make a key press have the right case for a gadget KeyEquiv value.         */
/*****************************************************************************/
uint16 makeGadgetKeyEquiv(uint16 key) {
	if (Common::isAlnum(key))
		key = tolower(key);

	return key;
}

/*****************************************************************************/
/* Checks whether or not the cords fall within one of the gadgets in a list  */
/* of gadgets.                                                               */
/*****************************************************************************/
Gadget *LabEngine::checkNumGadgetHit(Gadget *gadlist, uint16 key) {
	uint16 gkey = key - '0';

	while (gadlist != NULL) {
		if ((gkey - 1 == gadlist->GadgetID || (gkey == 0 && gadlist->GadgetID == 9) ||
		        (gadlist->KeyEquiv != 0 && makeGadgetKeyEquiv(key) == gadlist->KeyEquiv))
		        && !(GADGETOFF & gadlist->GadgetFlags)) {
			_event->mouseHide();
			gadlist->ImAlt->drawImage(gadlist->x, gadlist->y);
			_event->mouseShow();
			g_system->delayMillis(80);
			_event->mouseHide();
			gadlist->Im->drawImage(gadlist->x, gadlist->y);
			_event->mouseShow();

			return gadlist;
		} else {
			gadlist = gadlist->NextGadget;
		}
	}

	return NULL;
}

IntuiMessage IMessage;

IntuiMessage *LabEngine::getMsg() {
	Gadget *curgad;
	int Qualifiers;

	_event->updateMouse();

	Qualifiers = _keyPressed.flags;

	if ((curgad = _event->mouseGadget()) != NULL) {
		_event->updateMouse();
		IMessage.msgClass = GADGETUP;
		IMessage.code  = curgad->GadgetID;
		IMessage.gadgetID = curgad->GadgetID;
		IMessage.qualifier = Qualifiers;
		return &IMessage;
	} else if (_event->mouseButton(&IMessage.mouseX, &IMessage.mouseY, true)) { /* Left Button */
		IMessage.qualifier = IEQUALIFIER_LEFTBUTTON | Qualifiers;
		IMessage.msgClass = MOUSEBUTTONS;
		return &IMessage;
	} else if (_event->mouseButton(&IMessage.mouseX, &IMessage.mouseY, false)) { /* Right Button */
		IMessage.qualifier = IEQUALIFIER_RBUTTON | Qualifiers;
		IMessage.msgClass = MOUSEBUTTONS;
		return &IMessage;
	} else if (_event->keyPress(&IMessage.code)) { /* Keyboard key */
		curgad = checkNumGadgetHit(_event->_screenGadgetList, IMessage.code);

		if (curgad) {
			IMessage.msgClass = GADGETUP;
			IMessage.code  = curgad->GadgetID;
			IMessage.gadgetID = curgad->GadgetID;
		} else
			IMessage.msgClass = RAWKEY;

		IMessage.qualifier = Qualifiers;
		return &IMessage;
	} else
		return NULL;
}

} // End of namespace Lab
