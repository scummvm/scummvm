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

#include "lab/mouse.h"
#include "lab/vga.h"
#include "lab/stddefines.h"
#include "lab/timing.h"
#include "lab/interface.h"

namespace Lab {

extern bool IsHiRes;
extern uint32 VGAScreenWidth, VGAScreenHeight;

void mouseHideXY(void);

static bool LeftClick = false;
static uint16 leftx = 0, lefty = 0;
static bool RightClick = false;
static uint16 rightx = 0, righty = 0;

static bool MouseHidden = true, QuitMouseHandler = false;
static int32 NumHidden   = 1;
static uint16 CurMouseX, CurMouseY;
static uint16 MouseImageWidth = 10, MouseImageHeight = 15;
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


static bool drawmouse = false, gadhit    = false;
static Gadget *hitgad = NULL;

void mouseShowXY(uint16 CurMouseX, uint16 CurMouseY);

/*****************************************************************************/
/* Checks whether or not the cords fall within one of the gadgets in a list  */
/* of gadgets.                                                               */
/*****************************************************************************/
static Gadget *checkGadgetHit(Gadget *gadlist, uint16 x, uint16 y) {
	uint16 counter;

	while (gadlist != NULL) {
		if ((x >= gadlist->x) && (y >= gadlist->y) &&
		        (x <= (gadlist->x + gadlist->Im->Width)) &&
		        (y <= (gadlist->y + gadlist->Im->Height)) &&
		        !(GADGETOFF & gadlist->GadgetFlags)) {
			if (IsHiRes) {
				gadhit = true;
				hitgad = gadlist;
			} else {
				QuitMouseHandler = true;
				VGAStorePage();
				mouseHideXY();
				drawImage(gadlist->ImAlt, gadlist->x, gadlist->y);
				mouseShowXY(x, y);

				for (counter = 0; counter < 3; counter++)
					waitTOF();

				mouseHideXY();
				drawImage(gadlist->Im, gadlist->x, gadlist->y);
				mouseShowXY(x, y);
				VGARestorePage();
				QuitMouseHandler = false;
			}

			return gadlist;
		} else {
			gadlist = gadlist->NextGadget;
		}
	}

	return NULL;
}



void attachGadgetList(Gadget *GadList) {
	if (ScreenGadgetList != GadList)
		LastGadgetHit = NULL;

	ScreenGadgetList = GadList;
}

static Gadget *TempGad;


void mouse_handler(int32 max, int32 mcx, int32 mdx) {
	if (!IsHiRes)
		mcx /= 2;

	if (max & 0x01) { /* mouse Move */
		if ((CurMouseX != mcx) || (CurMouseY != mdx)) {
			CurMouseX = mcx;
			CurMouseY = mdx;

			if (IsHiRes && !QuitMouseHandler) {
				drawmouse = true;
			}
		}
	}

	if ((max & 0x02) && (NumHidden < 2)) { /* Left mouse button click */
		if (ScreenGadgetList)
			TempGad = checkGadgetHit(ScreenGadgetList, mcx, mdx);
		else
			TempGad = NULL;

		if (TempGad) {
			LastGadgetHit = TempGad;
		} else {
			LeftClick = true;
			leftx     = mcx;
			lefty     = mdx;
		}
	}

	if ((max & 0x08) && (NumHidden < 2)) { /* Right mouse button click */
		RightClick = true;
		rightx     = mcx;
		righty     = mdx;
	}
}




void updateMouse(void) {
	uint16 counter;
	bool doUpdateDisplay = false;

	if (drawmouse && !MouseHidden) {
		QuitMouseHandler = true;
		drawmouse = false;
		QuitMouseHandler = false;
		doUpdateDisplay = true;
	}

	if (gadhit) {
		gadhit = false;
		QuitMouseHandler = true;
		mouseHide();
		drawImage(hitgad->ImAlt, hitgad->x, hitgad->y);
		mouseShow();

		for (counter = 0; counter < 3; counter++)
			waitTOF();

		mouseHide();
		drawImage(hitgad->Im, hitgad->x, hitgad->y);
		mouseShow();
		doUpdateDisplay = true;
		QuitMouseHandler = false;
	}

	if (doUpdateDisplay)
		WSDL_UpdateScreen();
}




/*****************************************************************************/
/* Initializes the mouse.                                                    */
/*****************************************************************************/
bool initMouse(void) {
	g_system->setMouseCursor(MouseData, MouseImageWidth, MouseImageHeight, 0, 0, 0);
	g_system->showMouse(false);

	mouseMove(0, 0);

	return true;
}



/*****************************************************************************/
/* Resets the mouse.                                                         */
/*****************************************************************************/
bool mouseReset(void) {
	return true;
}



/*****************************************************************************/
/* Shows the mouse.                                                          */
/*****************************************************************************/
void mouseShow(void) {
	g_system->showMouse(true);
}





/*****************************************************************************/
/* Shows the mouse.                                                          */
/*****************************************************************************/
void mouseShowXY(uint16 MouseX, uint16 MouseY) {
	QuitMouseHandler = true;

	if (NumHidden)
		NumHidden--;

	if ((NumHidden == 0) && MouseHidden) {
		CurMouseX = MouseX;
		CurMouseY = MouseY;
		MouseHidden = false;
	}

	QuitMouseHandler = false;
}



/*****************************************************************************/
/* Hides the mouse.                                                          */
/*****************************************************************************/
void mouseHide(void) {
	QuitMouseHandler = true;

	NumHidden++;

	if (NumHidden && !MouseHidden) {
		MouseHidden = true;

		g_system->showMouse(false);
	}

	QuitMouseHandler = false;
}




/*****************************************************************************/
/* Hides the mouse.                                                          */
/*****************************************************************************/
void mouseHideXY(void) {
	QuitMouseHandler = true;

	NumHidden++;

	if (NumHidden && !MouseHidden) {
		MouseHidden = true;

		g_system->showMouse(false);
	}

	QuitMouseHandler = false;
}



extern int g_MouseX;
extern int g_MouseY;

/*****************************************************************************/
/* Gets the current mouse co-ordinates.  NOTE: On IBM version, will scale    */
/* from virtual to screen co-ordinates automatically.                        */
/*****************************************************************************/
void mouseXY(uint16 *x, uint16 *y) {
	*x = (uint16)g_MouseX;
	*y = (uint16)g_MouseY;

	if (!IsHiRes)
		(*x) /= 2;
}




/*****************************************************************************/
/* Moves the mouse to new co-ordinates.                                      */
/*****************************************************************************/
void mouseMove(uint16 x, uint16 y) {
	if (!IsHiRes)
		x *= 2;

	g_system->warpMouse(x, y);

	if (!MouseHidden) {
		QuitMouseHandler = true;
		mouseXY(&CurMouseX, &CurMouseY);
		QuitMouseHandler = false;
	}
}




/*****************************************************************************/
/* Checks whether or not the mouse buttons have been pressed, and the last   */
/* co-ordinates of the button press.  leftbutton tells whether to check the  */
/* left or right button.                                                     */
/*****************************************************************************/
bool mouseButton(uint16 *x, uint16 *y, bool leftbutton) {
	if (leftbutton) {
		if (LeftClick) {
			*x = leftx;
			*y = lefty;
			LeftClick = false;
			return true;
		}
	} else {
		if (RightClick) {
			*x = rightx;
			*y = righty;
			RightClick = false;
			return true;
		}
	}

	return false;
}




Gadget *mouseGadget(void) {
	Gadget *Temp = LastGadgetHit;

	LastGadgetHit = NULL;
	return Temp;
}

} // End of namespace Lab
