/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "driver96.h"
#include "d_draw.h"
#include "render.h"
#include "menu.h"
#include "../sword2.h"

#define MAX_MOUSE_EVENTS 16
#define MOUSEFLASHFRAME 6

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct _mouseAnim {
	uint8 runTimeComp;	// type of runtime compression used for the
				// frame data
	uint8 noAnimFrames;	// number of frames in the anim
	int8 xHotSpot;		
	int8 yHotSpot;
	uint8 mousew;
	uint8 mouseh;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

int16 mousex;
int16 mousey;

static uint8 mouseBacklog = 0;
static uint8 mouseLogPos = 0;
static uint8 mouseFrame;
static uint8 *mouseSprite = NULL;
static _mouseAnim *mouseAnim = NULL;
static _mouseAnim *luggageAnim = NULL;
static _mouseEvent mouseLog[MAX_MOUSE_EVENTS];
static int32 *mouseOffsets;
static int32 *luggageOffset;

// This is the maximum mouse cursor size in the SDL backend

#define MAX_MOUSE_W 80
#define MAX_MOUSE_H 80

byte _mouseData[MAX_MOUSE_W * MAX_MOUSE_H];

void ResetRenderEngine(void) {
	parallaxScrollx = 0;
	parallaxScrolly = 0;
	scrollx = 0;
	scrolly = 0;
}

// --------------------------------------------------------------------------
// Logs the mouse button event passed in buttons.  The button events are 
// defined as RD_LEFTBUTTONDOWN, RD_LEFTBUTTONUP, RD_RIGHTBUTTONDOWN and
// RD_RIGHTBUTTONUP.
// --------------------------------------------------------------------------

void LogMouseEvent(uint16 buttons) {
	_mouseEvent *me;

	// We need to leave the one, which is the current event, alone!
	if (mouseBacklog == MAX_MOUSE_EVENTS - 1)
		return;
	
	me = &mouseLog[(mouseBacklog + mouseLogPos) % MAX_MOUSE_EVENTS];
	me->buttons = buttons;
	mouseBacklog++;
}

// FIXME: The original code used 0 for transparency, while our backend uses
// 0xFF. That means that parts of the mouse cursor that weren't meant to be
// transparent may be now.

int32 DecompressMouse(uint8 *decomp, uint8 *comp, int width, int height, int pitch, int xOff = 0, int yOff = 0) {
	int32 size = width * height;
	int32 i = 0;
	int x = 0;
	int y = 0;

	while (i < size) {
		if (*comp > 183) {
			decomp[(y + yOff) * pitch + x + xOff] = *comp++;
			if (++x >= width) {
				x = 0;
				y++;
			}
			i++;
		} else {
			x += *comp;
			while (x >= width) {
				y++;
				x -= width;
			}
			i += *comp++;
		}
	}

	return RD_OK;
}

void DrawMouse(void) {
	if (!mouseAnim && !luggageAnim)
		return;

	// When an object is used in the game, the mouse cursor should be a
	// combination of a standard mouse cursor and a luggage cursor.
	//
	// However, judging by the original code luggage cursors can also
	// appear on their own. I have no idea which cases though.

	uint16 mouse_width = 0;
	uint16 mouse_height = 0;
	uint16 hotspot_x = 0;
	uint16 hotspot_y = 0;
	int deltaX = 0;
	int deltaY = 0;

	if (mouseAnim) {
		hotspot_x = mouseAnim->xHotSpot;
		hotspot_y = mouseAnim->yHotSpot;
		mouse_width = mouseAnim->mousew;
		mouse_height = mouseAnim->mouseh;
	}

	if (luggageAnim) {
		if (!mouseAnim) {
			hotspot_x = luggageAnim->xHotSpot;
			hotspot_y = luggageAnim->yHotSpot;
		}
		if (luggageAnim->mousew > mouse_width)
			mouse_width = luggageAnim->mousew;
		if (luggageAnim->mouseh > mouse_height)
			mouse_height = luggageAnim->mouseh;
	}

	if (mouseAnim && luggageAnim) {
		deltaX = mouseAnim->xHotSpot - luggageAnim->xHotSpot;
		deltaY = mouseAnim->yHotSpot - luggageAnim->yHotSpot;
	}

	assert(deltaX >= 0);
	assert(deltaY >= 0);

	// HACK for maximum cursor size. (The SDL backend imposes this
	// restriction)

	if (mouse_width + deltaX > MAX_MOUSE_W)
		deltaX = 80 - mouse_width;
	if (mouse_height + deltaY > MAX_MOUSE_H)
		deltaY = 80 - mouse_height;

	mouse_width += deltaX;
	mouse_height += deltaY;

	if ((uint32) (mouse_width * mouse_height) > sizeof(_mouseData)) {
		warning("Mouse cursor too large");
		return;
	}

	memset(_mouseData, 0xFF, mouse_width * mouse_height);

	if (luggageAnim)
		DecompressMouse(_mouseData, (uint8 *) luggageAnim + READ_LE_UINT32(luggageOffset), luggageAnim->mousew,
				luggageAnim->mouseh, mouse_width, deltaX, deltaY);

	if (mouseAnim)
		DecompressMouse(_mouseData, mouseSprite, mouseAnim->mousew, mouseAnim->mouseh, mouse_width);

	g_system->set_mouse_cursor(_mouseData, mouse_width, mouse_height, hotspot_x, hotspot_y);
}

/**
 * Get the next pending mouse event.
 * @return a pointer to the mouse event, or NULL of there is none
 */

_mouseEvent *MouseEvent(void) {
	_mouseEvent *me;

	if (mouseBacklog) {
		me = &mouseLog[mouseLogPos];
		if (++mouseLogPos == MAX_MOUSE_EVENTS)
			mouseLogPos = 0;

		mouseBacklog--;
		return me;
	}

	return NULL;
}

uint8 CheckForMouseEvents(void) {
	return mouseBacklog;	// return the number of mouse events waiting
}

/**
 * Animates the current mouse pointer
 */

int32 AnimateMouse(void) {
	uint8 prevMouseFrame = mouseFrame;

	if (!mouseAnim)
		return RDERR_UNKNOWN;

	if (++mouseFrame == mouseAnim->noAnimFrames)
		mouseFrame = MOUSEFLASHFRAME;

	mouseSprite = (uint8 *) mouseAnim + READ_LE_UINT32(mouseOffsets + mouseFrame);

	if (mouseFrame != prevMouseFrame)
		DrawMouse();

	return RD_OK;
}

/**
 * Sets the mouse cursor animation.
 * @param ma a pointer to the animation data, or NULL to clear the current one
 * @param size the size of the mouse animation data
 * @param mouseFlash RDMOUSE_FLASH or RDMOUSE_NOFLASH, depending on whether
 * or not there is a lead-in animation
 */

int32 SetMouseAnim(uint8 *ma, int32 size, int32 mouseFlash) {
	if (mouseAnim) {
		free(mouseAnim);
		mouseAnim = NULL;
	}

	if (ma)	{
		if (mouseFlash == RDMOUSE_FLASH)
			mouseFrame = 0;
		else
			mouseFrame = MOUSEFLASHFRAME;

		mouseAnim = (_mouseAnim *) malloc(size);
		if (!mouseAnim)
			return RDERR_OUTOFMEMORY;

		memcpy((uint8 *) mouseAnim, ma, size);
		mouseOffsets = (int32 *) ((uint8 *) mouseAnim + sizeof(_mouseAnim));

		AnimateMouse();
		DrawMouse();

		g_system->show_mouse(true);
	} else {
		if (luggageAnim)
			DrawMouse();
		else
			g_system->show_mouse(false);
	}

	return RD_OK;
}

/**
 * Sets the "luggage" animation to accompany the mouse animation. Luggage
 * sprites are of the same format as mouse sprites.
 * @param ma a pointer to the animation data, or NULL to clear the current one
 * @param size the size of the animation data
 */

int32 SetLuggageAnim(uint8 *ma, int32 size) {
	if (luggageAnim) {
		free(luggageAnim);
		luggageAnim = NULL;
	}

	if (ma)	{
		luggageAnim = (_mouseAnim *) malloc(size);
		if (!luggageAnim)
			return(RDERR_OUTOFMEMORY);

		memcpy((uint8 *) luggageAnim, ma, size);
		luggageOffset = (int32 *) ((uint8 *) luggageAnim + sizeof(_mouseAnim));

		AnimateMouse();
		DrawMouse();

		g_system->show_mouse(true);
	} else {
		if (mouseAnim)
			DrawMouse();
		else
			g_system->show_mouse(false);
	}

	return RD_OK;
}
