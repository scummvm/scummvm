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

#include "lab/stddefines.h"
#include "lab/interface.h"
#include "lab/storage.h"
#include "lab/timing.h"
#include "lab/mouse.h"
#include "lab/vga.h"
#include "common/util.h"

namespace Lab {

extern bool IsHiRes;

Common::KeyState _keyPressed;

struct Gadget *createButton(uint16 x, uint16 y, uint16 id, uint16 key, Image *im, Image *imalt) {
	Gadget *gptr;

	if (allocate((void **)&gptr, sizeof(struct Gadget))) {
		gptr->x = x;
		gptr->y = y;
		gptr->GadgetID = id;
#if !defined(DOSCODE)
		gptr->KeyEquiv = key;
#endif
		gptr->Im = im;
		gptr->ImAlt = imalt;
		gptr->NextGadget = NULL;

		return gptr;
	} else
		return NULL;
}




void freeButtonList(Gadget *gptrlist) {
	struct Gadget *gptr, *next = gptrlist;

	while (next) {
		gptr = next;
		next = next->NextGadget;

		deallocate(gptr, sizeof(struct Gadget));
	}
}




/*****************************************************************************/
/* Draws a gadget list to the screen.                                        */
/*****************************************************************************/
void drawGadgetList(struct Gadget *gadlist) {
	while (gadlist) {
		drawImage(gadlist->Im, gadlist->x, gadlist->y);

		if (GADGETOFF & gadlist->GadgetFlags)
			ghoastGadget(gadlist, 1);

		gadlist = gadlist->NextGadget;
	}
}


/*****************************************************************************/
/* Ghoasts a gadget, and makes it unavailable for using.                     */
/*****************************************************************************/
void ghoastGadget(struct Gadget *curgad, uint16 pencolor) {
	ghoastRect(pencolor, curgad->x, curgad->y, curgad->x + curgad->Im->Width - 1, curgad->y + curgad->Im->Height - 1);
	curgad->GadgetFlags |= GADGETOFF;
}



/*****************************************************************************/
/* Unghoasts a gadget, and makes it available again.                         */
/*****************************************************************************/
void unGhoastGadget(struct Gadget *curgad) {
	drawImage(curgad->Im, curgad->x, curgad->y);
	curgad->GadgetFlags &= !(GADGETOFF);
}


/*****************************************************************************/
/* Make a key press have the right case for a gadget KeyEquiv value.         */
/*****************************************************************************/
#if !defined(DOSCODE)
uint16 makeGadgetKeyEquiv(uint16 key) {
	if (Common::isAlnum(key))
		key = tolower(key);

	return key;
}
#endif

/*****************************************************************************/
/* Checks whether or not the cords fall within one of the gadgets in a list  */
/* of gadgets.                                                               */
/*****************************************************************************/
static struct Gadget *checkNumGadgetHit(struct Gadget *gadlist, uint16 key) {
#if !defined(DOSCODE)
	uint16 gkey = key - '0';
#else
	key = key - '0';
#endif

	while (gadlist != NULL) {
#if !defined(DOSCODE)

		if ((gkey - 1 == gadlist->GadgetID || (gkey == 0 && gadlist->GadgetID == 9) ||
		        (gadlist->KeyEquiv != 0 && makeGadgetKeyEquiv(key) == gadlist->KeyEquiv))
		        && !(GADGETOFF & gadlist->GadgetFlags))
#else
		if ((((key - 1) == gadlist->GadgetID) || ((key == 0) && (gadlist->GadgetID == 9))) &&
		        !(GADGETOFF & gadlist->GadgetFlags))
#endif
		{
			mouseHide();
			drawImage(gadlist->ImAlt, gadlist->x, gadlist->y);
			mouseShow();
			g_system->delayMillis(80);
			mouseHide();
			drawImage(gadlist->Im, gadlist->x, gadlist->y);
			mouseShow();

			return gadlist;
		} else {
			gadlist = gadlist->NextGadget;
		}
	}

	return NULL;
}



/*****************************************************************************/
/* Checks whether or not a key has been pressed.                             */
/*****************************************************************************/
static bool keyPress(uint16 *KeyCode) {
	if (WSDL_HasNextChar()) {
        *KeyCode = WSDL_GetNextChar();
		return true;
	}

	return false;
}


struct IntuiMessage IMessage;
extern struct Gadget *ScreenGadgetList;

struct IntuiMessage *getMsg(void) {
	struct Gadget *curgad;
	int Qualifiers;

	updateMouse();
#if !defined(DOSCODE)
	Qualifiers = _keyPressed.flags;
#endif

	if ((curgad = mouseGadget()) != NULL) {
		updateMouse();
		IMessage.Class = GADGETUP;
		IMessage.Code  = curgad->GadgetID;
		IMessage.GadgetID = curgad->GadgetID;
		IMessage.Qualifier = Qualifiers;
		return &IMessage;
	}

	else if (mouseButton(&IMessage.MouseX, &IMessage.MouseY, true)) { /* Left Button */
		IMessage.Qualifier = IEQUALIFIER_LEFTBUTTON | Qualifiers;
		IMessage.Class = MOUSEBUTTONS;
		return &IMessage;
	}

	else if (mouseButton(&IMessage.MouseX, &IMessage.MouseY, false)) { /* Right Button */
		IMessage.Qualifier = IEQUALIFIER_RBUTTON | Qualifiers;
		IMessage.Class = MOUSEBUTTONS;
		return &IMessage;
	}

	else if (keyPress(&IMessage.Code)) { /* Keyboard key */
		curgad = checkNumGadgetHit(ScreenGadgetList, IMessage.Code);

		if (curgad) {
			IMessage.Class = GADGETUP;
			IMessage.Code  = curgad->GadgetID;
			IMessage.GadgetID = curgad->GadgetID;
		} else
			IMessage.Class = RAWKEY;

		IMessage.Qualifier = Qualifiers;
		return &IMessage;
	} else
		return NULL;
}

void replyMsg(void *Msg) {
	return;
}

} // End of namespace Lab
