/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002 Ph0x - GP32 Backend
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
 * Copyright (C) 2005 Won Star - GP32 Backend
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "stdafx.h"
#include "common/scummsys.h"

#include "backends/gp32/gp32std.h"
#include "backends/gp32/gp32std_input.h"

static uint32 buttonState = 0;
static uint16 buttonPressState = 0;

#define GET_BUTTON_PRESSED(a) (buttonPressState & (a))
#define ON_BUTTON_PRESSED(a)  (buttonPressState |= (a))
#define OFF_BUTTON_PRESSED(a) (buttonPressState &= ~(a))

#define GET_BUTTON_DOWN(a) (buttonState & (a) << 12)
#define ON_BUTTON_DOWN(a)  buttonState |= ((a) << 12)
#define OFF_BUTTON_DOWN(a) buttonState &= ~((a) << 12)

#define GET_BUTTON_UP(a)   (buttonState & (a))
#define ON_BUTTON_UP(a)    buttonState |= (a)
#define OFF_BUTTON_UP(a)   buttonState &= ~(a)

#define rKEY_A          0x4000
#define rKEY_B          0x2000
#define rKEY_L          0x1000
#define rKEY_R          0x8000
#define rKEY_UP         0x0800
#define rKEY_DOWN       0x0200
#define rKEY_LEFT       0x0100
#define rKEY_RIGHT      0x0400
#define rKEY_START      0x0040
#define rKEY_SELECT     0x0080
#define rPBDAT          (*(volatile unsigned *)0x1560000c)
#define rPEDAT          (*(volatile unsigned *)0x15600030)

/****************************************************************
    GP32 Input mappings - Returns Button Pressed.
****************************************************************/
int gp_trapKey() {
	int value = 0;

	unsigned long gpb = rPBDAT;	// 0x156
	unsigned long gpe = rPEDAT;

	if ((gpb & rKEY_LEFT) == 0)
		value |= GPC_VK_LEFT;
	if ((gpb & rKEY_RIGHT) == 0)
		value |= GPC_VK_RIGHT;
	if ((gpb & rKEY_UP) == 0)
		value |= GPC_VK_UP;
	if ((gpb & rKEY_DOWN) == 0)
		value |= GPC_VK_DOWN;
	if ((gpb & rKEY_A) == 0)
		value |= GPC_VK_FA;
	if ((gpb & rKEY_B) == 0)
		value |= GPC_VK_FB;
	if ((gpb & rKEY_L) == 0)
		value |= GPC_VK_FL;
	if ((gpb & rKEY_R) == 0)
		value |= GPC_VK_FR;
	if ((gpe & rKEY_SELECT) == 0)
		value |= GPC_VK_SELECT;
	if ((gpe & rKEY_START) == 0)
		value |= GPC_VK_START;

	return value;
}

bool gp_getButtonEvent(uint32 *nKeyUD, uint16 *nKeyP) {
//	int nKey = GpKeyGet();
	int nKey = gp_trapKey();

	const int keyBitMask[10] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200};

	for (int i = 0; i < 10; i++) {
		OFF_BUTTON_DOWN(keyBitMask[i]);
		OFF_BUTTON_UP(keyBitMask[i]);
		if (nKey & keyBitMask[i]) {
			if (!GET_BUTTON_PRESSED(keyBitMask[i])) {
//				GPDEBUG("DOWN %d", i);
				ON_BUTTON_PRESSED(keyBitMask[i]);
				ON_BUTTON_DOWN(keyBitMask[i]);
			}
		} else {
			if (GET_BUTTON_PRESSED(keyBitMask[i])) {
//				GPDEBUG("UP %d", i);
				OFF_BUTTON_PRESSED(keyBitMask[i]);
				ON_BUTTON_UP(keyBitMask[i]);
			}
		}
	}

//	GPDEBUG("%08x %04x", buttonState, buttonPressState);
	*nKeyUD = buttonState;
	*nKeyP = buttonPressState;
	return true;
}

#define MAX_EVENTS 32

struct EventQueue {
protected:
	int front, rear;
	int numElement;
	GP32BtnEvent event[MAX_EVENTS];

public:
	void push(GP32BtnEvent *ev) {
		if ((rear + 1) % MAX_EVENTS == front) {
			GPDEBUG("Queue Overflow!");
			return;
		}
		numElement++;

		event[rear].type = ev->type;
		event[rear].button = ev->button;
		rear = (rear + 1) % MAX_EVENTS;
	}

	bool isEmpty() {
		return (numElement == 0);
	}

	bool isFull() {
		return (numElement == MAX_EVENTS);
	}

	bool get(GP32BtnEvent *ev) { 
		if (front == rear) {
			return false;
		}
		numElement--;
		ev->type = event[front].type;
		ev->button = event[front].button;
		front = (front + 1) % MAX_EVENTS;
		return true;
	}
};

EventQueue eventQueue;

bool gp_pumpButtonEvent() {
	int nKey = gp_trapKey();
	GP32BtnEvent ev;

	const int keyBitMask[10] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200};

	for (int i = 0; i < 10; i++) {
		if (nKey & keyBitMask[i]) {
			if (!GET_BUTTON_PRESSED(keyBitMask[i])) {
				ON_BUTTON_PRESSED(keyBitMask[i]);
				ev.type = BUTTON_DOWN;
				ev.button = keyBitMask[i];
				eventQueue.push(&ev);
				//ON_BUTTON_DOWN(keyBitMask[i]);
			}
		} else {
			if (GET_BUTTON_PRESSED(keyBitMask[i])) {
				OFF_BUTTON_PRESSED(keyBitMask[i]);
				ev.type = BUTTON_UP;
				ev.button = keyBitMask[i];
				eventQueue.push(&ev);
				//ON_BUTTON_UP(keyBitMask[i]);
			}
		}
	}

	return true;
}

bool gp_pollButtonEvent(GP32BtnEvent *ev) {
	gp_pumpButtonEvent();

	if(eventQueue.isEmpty()) {
		return false;
	}

	eventQueue.get(ev);
//	GPDEBUG("Event poll %d %d", ev->type, ev->button);

	return true;
}

bool gp_getButtonPressed(uint16 button)
{
	return buttonPressState & button;
}
