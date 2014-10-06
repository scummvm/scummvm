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

#if defined(DOSCODE)
/*****************************************************************************/
/* Standard mouse calling template.                                          */
/*****************************************************************************/
static void mouse(int16 *m1, int16 *m2, int16 *m3, int16 *m4) {
	union REGS reg;

	reg.w.ax = *m1;
	reg.w.bx = *m2;
	reg.w.cx = *m3;
	reg.w.dx = *m4;

	int386(0x33, &reg, &reg);

	*m1 = reg.w.ax;
	*m2 = reg.w.bx;
	*m3 = reg.w.cx;
	*m4 = reg.w.dx;
}
#endif


static bool LeftClick = false;
static uint16 leftx = 0, lefty = 0;
static bool RightClick = false;
static uint16 rightx = 0, righty = 0;

static bool MouseHidden = true, QuitMouseHandler = false;
static int32 NumHidden   = 1;
static uint16 CurMouseX, CurMouseY;
static uint16 MouseImageWidth = 10, MouseImageHeight = 15;
static struct Gadget *LastGadgetHit = NULL;
struct Gadget *ScreenGadgetList = NULL;
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


static struct Image MouseImage, BackImage;
static byte BackImageBuffer[256];
static uint16 backx, backy;

static bool drawmouse = false, gadhit    = false;
static struct Gadget *hitgad = NULL;

void mouseShowXY(uint16 CurMouseX, uint16 CurMouseY);

/*****************************************************************************/
/* Checks whether or not the cords fall within one of the gadgets in a list  */
/* of gadgets.                                                               */
/*****************************************************************************/
static Gadget *checkGadgetHit(struct Gadget *gadlist, uint16 x, uint16 y) {
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



void attachGadgetList(struct Gadget *GadList) {
	if (ScreenGadgetList != GadList)
		LastGadgetHit = NULL;

	ScreenGadgetList = GadList;
}




static void drawMouse(void) {
	if (BackImage.ImageData) {
#if !defined(DOSCODE)

		if (backx <= 640 - BackImage.Width && backy <= 480 - BackImage.Height)
#endif
			drawMaskImage(&MouseImage, backx, backy);
	} else {
#if !defined(DOSCODE)

		if (CurMouseX <= 640 - MouseImage.Width && CurMouseY <= 480 - MouseImage.Height)
#endif
			drawMaskImage(&MouseImage, CurMouseX, CurMouseY);
	}
}



static void getBackMouse(void) {
	BackImage.Width = MouseImage.Width;
	BackImage.Height = MouseImage.Height;
	BackImage.ImageData = BackImageBuffer;

	backx = CurMouseX;
	backy = CurMouseY;

#if !defined(DOSCODE)

	if (/* backx >= 0 && backy >= 0 && */ backx <= 640 - BackImage.Width && backy <= 480 - BackImage.Height)
#endif
		readScreenImage(&BackImage, backx, backy);
}

static void restoreBackMouse(void) {
	if (BackImage.ImageData) {
#if !defined(DOSCODE)

		if (/* backx >= 0 && backy >= 0 && */ backx <= 640 - BackImage.Width && backy <= 480 - BackImage.Height)
#endif
			drawImage(&BackImage, backx, backy);

		BackImage.ImageData = NULL;
	}
}


static struct Gadget *TempGad;


#if defined(DOSCODE)
#pragma off (check_stack)
void _loadds far mouse_handler(int32 max, int32 mcx, int32 mdx) {
#pragma aux mouse_handler parm [EAX] [ECX] [EDX]
#else
void mouse_handler(int32 max, int32 mcx, int32 mdx) {
#endif

	if (!IsHiRes)
		mcx /= 2;

	if (max & 0x01) { /* mouse Move */
		if ((CurMouseX != mcx) || (CurMouseY != mdx)) {
			CurMouseX = mcx;
			CurMouseY = mdx;

			if (IsHiRes && !QuitMouseHandler) {
				drawmouse = true;
			} else if (!MouseHidden && !QuitMouseHandler) {
				VGAStorePage();
				restoreBackMouse();
				getBackMouse();
				drawMouse();
				VGARestorePage();
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
#if !defined(DOSCODE)
	bool doUpdateDisplay = false;
#endif

	if (drawmouse && !MouseHidden) {
		QuitMouseHandler = true;
		drawmouse = false;
		restoreBackMouse();
		getBackMouse();
		drawMouse();
		QuitMouseHandler = false;
#if !defined(DOSCODE)
		doUpdateDisplay = true;
#endif
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
#if !defined(DOSCODE)
		doUpdateDisplay = true;
#endif
		QuitMouseHandler = false;
	}

#if !defined(DOSCODE)

	if (doUpdateDisplay)
		g_system->updateScreen();

#endif
}




/*****************************************************************************/
/* Initializes the mouse.                                                    */
/*****************************************************************************/
bool initMouse(void) {
#if defined(DOSCODE)
	void (interrupt far * int_handler)();
	int32 vector;
	byte firstbyte;
	struct SREGS sregs;
	union REGS inregs, outregs;
	int (far * function_ptr)();
	int16 m1, m2, m3, m4;

	segread(&sregs);

	/* Determine mouse-driver interrupt address */
	int_handler = _dos_getvect(0x33);        /* Get interrupt vector       */
	firstbyte = *(byte far *) int_handler;  /* Get first instruction of interrupt */
	vector = (int32) int_handler;

	if ((vector == 0L) || (firstbyte == 0xcf)) { /* Vector should not be zero            */
		/* First instruction should not be iret */
		return false;
	}

	m1 = 0;
	mouse(&m1, &m2, &m3, &m4);

	if (m1 != -1)
		return false;

	m1 = 0x0f;
	m3 = 3;
	m4 = 10;
	mouse(&m1, &m2, &m3, &m4);

	m1 = 0x07;
	m3 = 0;
	m4 = VGAScreenWidth - MouseImageWidth;

	if (!IsHiRes) m4 *= 2;

	mouse(&m1, &m2, &m3, &m4);

	m1 = 0x08;
	m3 = 0;
	m4 = VGAScreenHeight - MouseImageHeight;
	mouse(&m1, &m2, &m3, &m4);
#endif

	BackImage.ImageData = NULL;
	MouseImage.ImageData = MouseData;
	MouseImage.Width = MouseImageWidth;
	MouseImage.Height = MouseImageHeight;

	mouseMove(0, 0);

#if defined(DOSCODE)

	if (IsHiRes) {
		m1 = 0x0f;
		m3 = 0x03;
		m4 = 0x04;
		mouse(&m1, &m2, &m3, &m4);
	}

	inregs.w.ax = 0xc;
	inregs.w.cx = 0x01 + 0x02 + 0x08;  /* mouse move, left and right mouse clicks */
	function_ptr = mouse_handler;
	inregs.x.edx = FP_OFF(function_ptr);
	sregs.es     = FP_SEG(function_ptr);
	int386x(0x33, &inregs, &outregs, &sregs);

	/* mouse reset and status */
	return mouseReset();
#endif

	return true;
}



/*****************************************************************************/
/* Resets the mouse.                                                         */
/*****************************************************************************/
bool mouseReset(void) {
#if defined(DOSCODE)
	int16 m1 = 0, dum;

	mouse(&m1, &dum, &dum, &dum);
	return (m1 == -1);
#else
	return true;
#endif
}



/*****************************************************************************/
/* Shows the mouse.                                                          */
/*****************************************************************************/
void mouseShow(void) {
	QuitMouseHandler = true;
	VGAStorePage();
	mouseShowXY(CurMouseX, CurMouseY);
	VGARestorePage();
	QuitMouseHandler = false;
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
		getBackMouse();
		drawMouse();
#if !defined(DOSCODE)
		g_system->updateScreen();
#endif
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
		VGAStorePage();
		restoreBackMouse();
		VGARestorePage();
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
		restoreBackMouse();
	}

	QuitMouseHandler = false;
}





/*****************************************************************************/
/* Gets the current mouse co-ordinates.  NOTE: On IBM version, will scale    */
/* from virtual to screen co-ordinates automatically.                        */
/*****************************************************************************/
void mouseXY(uint16 *x, uint16 *y) {
	int xx = 0, yy = 0;
	//SDL_GetMousePos(&xx, &yy);
	warning("STUB: mouseXY");
	*x = (uint16)xx;
	*y = (uint16)yy;

	if (!IsHiRes)
		(*x) /= 2;
}




/*****************************************************************************/
/* Moves the mouse to new co-ordinates.                                      */
/*****************************************************************************/
void mouseMove(uint16 x, uint16 y) {
#if defined(DOSCODE)
	int16 m1 = 4, dum;
#endif

	if (!IsHiRes)
		x *= 2;

	g_system->warpMouse(x, y);

	if (!MouseHidden) {
		QuitMouseHandler = true;
		mouseXY(&CurMouseX, &CurMouseY);
		VGAStorePage();
		restoreBackMouse();
		getBackMouse();
		drawMouse();
		VGARestorePage();
#if !defined(DOSCODE)
		g_system->updateScreen();
#endif
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
