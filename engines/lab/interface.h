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

#include "lab/vga.h"
#include "common/keyboard.h"

#ifndef LAB_INTEFACE_H
#define LAB_INTEFACE_H

namespace Lab {

struct IntuiMessage {
	uint32 Class;
	uint16 Code, Qualifier, MouseX, MouseY, GadgetID;
	uint32 Seconds, Micros;
};


struct Gadget {
	uint16 x, y, GadgetID;
	uint16 KeyEquiv; // if not zero, a key that activates gadget
	uint32 GadgetFlags;
	Image *Im, *ImAlt;
	Gadget *NextGadget;
};

extern Common::KeyState _keyPressed;

/* Defines for the GadgetFlags portion */

#define GADGETOFF 0x01



/* Defines for the Class variable in IntuiMessage */
#define SIZEVERIFY  0x00000001
#define NEWSIZE     0x00000002
#define REFRESHWINDOW   0x00000004
#define MOUSEBUTTONS    0x00000008
#define MOUSEMOVE   0x00000010
#define GADGETDOWN  0x00000020
#define GADGETUP    0x00000040
#define REQSET      0x00000080
#define MENUPICK    0x00000100
#define CLOSEWINDOW 0x00000200
#define RAWKEY      0x00000400
#define REQVERIFY   0x00000800
#define REQCLEAR    0x00001000
#define MENUVERIFY  0x00002000
#define NEWPREFS    0x00004000
#define DISKINSERTED    0x00008000
#define DISKREMOVED 0x00010000
#define WBENCHMESSAGE   0x00020000  /*  System use only     */
#define ACTIVEWINDOW    0x00040000
#define INACTIVEWINDOW  0x00080000
#define DELTAMOVE   0x00100000
#define VANULLLAKEY  0x00200000


/* Defines for the Qualifier variable in IntuiMessage */
#define IEQUALIFIER_LSHIFT      0x0001
#define IEQUALIFIER_RSHIFT      0x0002
#define IEQUALIFIER_CAPSLOCK        0x0004
#define IEQUALIFIER_CONTROL     0x0008
#define IEQUALIFIER_LALT        0x0010
#define IEQUALIFIER_RALT        0x0020
#define IEQUALIFIER_LCOMMAND        0x0040
#define IEQUALIFIER_RCOMMAND        0x0080
#define IEQUALIFIER_NUMERICPAD      0x0100
#define IEQUALIFIER_REPEAT      0x0200
#define IEQUALIFIER_INTERRUPT       0x0400
#define IEQUALIFIER_MULTIBROADCAST  0x0800
#define IEQUALIFIER_MIDBUTTON       0x1000
#define IEQUALIFIER_RBUTTON     0x2000
#define IEQUALIFIER_LEFTBUTTON      0x4000

#define VKEY_UPARROW    273
#define VKEY_DNARROW    274
#define VKEY_RTARROW    275
#define VKEY_LTARROW    276


/*---------------------------------------------------------------------------*/
/*--------------------------- Function Prototypes ---------------------------*/
/*---------------------------------------------------------------------------*/




Gadget *createButton(uint16 x, uint16 y, uint16 id, uint16 key, Image *im, Image *imalt);

void freeButtonList(void *gptrlist);

void drawGadgetList(Gadget *gadlist);

void ghoastGadget(Gadget *curgad, uint16 pencolor);

void unGhoastGadget(Gadget *curgad);

IntuiMessage *getMsg(void);

void replyMsg(void *Msg);

} // End of namespace Lab

#endif /* LAB_INTEFACE_H */

