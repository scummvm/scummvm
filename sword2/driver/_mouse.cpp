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

//=============================================================================
//
//	Filename	:	mouse.c
//	Created		:	17th September 1996
//	By			:	P.R.Porter
//
//	Summary		:	This module holds the interface to the mouse..
//
//	Version	Date		By		Description
//	------- ---------	---		-----------------------------------------------
//	1.0		18-Sep-96	PRP		Initial mouse functions.  Simple logging of
//								previous 16 mouse events implemented.  Mouse
//								drawing is currently hard coded, but animations
//								will be definable at a later date.
//
//	1.1		03-Oct-96	PRP		Changed the mouse position so that mouse y of
//								zero is the top left corner of the screen, and
//								not the top left corner of the top menubar.
//								Also, removed the x and y position from the
//								mouse log.  And changed the MouseEvent function
//								so that it returns a pointer to _mouseEvent
//
//	1.2		04-Oct-96	PRP		Put direct path in for ddraw.h
//
//	1.3		31-Oct-96	PRP		Added code to draw the proper type of mouse
//								sprite, which comprises of the internal
//								function DrawMouse and globally available
//								AnimateMouse and SetMouseAnim.
//
//	1.4		15-Nov-96	PRP		Definition of menubar size is now obtained
//								from menu.h
//
//	1.5		18-Nov-96	PRP		Changed the direct draw interface to
//								IDirectDraw2.
//
//	1.6		25-Nov-96	PRP		Added functionality to set the luggage
//								sprite.
//
//	1.7		06-Jan-97	PRP		Changed the width and height of sprites
//								to be signed.
//
//	1.8		14-Jan-97	JEL		Reset mouse frame when new animation starts.
//
//	1.9		27-Jan-97	PRP		Changed the mouse drawing routines to utilize
//								directDraw surfaces and transparency blitting.
//
//	1.10	10-Feb-97	PRP		Changed the directDraw error reporting so that
//								it works properly.  Also, created the mouse
//								sprite depending upon whether the hardware can
//								blt or not.
//
//	1.11	19-Mar-97	PRP		Fixed a bug which was causing the mouse sprite
//								to be freed up each frame and therefore 
//								decompressed and re-loaded each frame.
//
//	1.12	20-Mar-97	PRP		Added a function to reset the render code when
//								the control panel is entered.
//
//	1.13	09-Apr-97	PRP		Made the mouse animation wrap back to the
//								seventh frame.
//
//	1.14	10-Apr-97	PRP		Added parameter to define whether mouse flashes
//								or not.
//
//	1.15	23-Jul-97	JEL		Added CheckForMouseEvents() to return no. of events outstanding
//
//
//	Functions
//	---------
//
//	--------------------------------------------------------------------------
//
//  _mouseEvent *MouseEvent(void)
//
//	The address of a _mouseEvent pointer is passed in.  If there is a mouse 
//	event in the queue, a the value of the mouse event pointer is set to the
//	address of the event, otherwise, the mouse event pointer is set to NULL.
//
//	--------------------------------------------------------------------------
//
//	int32 SetMouseAnim(uint8 *ma, int32 size)
//
//	A pointer to a valid mouse animation is passed in, along with the size of
//	the header plus sprite data.  Remember to check that the function has 
//	successfully completed, as memory allocation is required.
//	Pass NULL in to clear the mouse sprite.
//
//	--------------------------------------------------------------------------
//
//	int32 SetLuggageAnim(uint8 *ma, int32 size)
//
//	A pointer to a valid luggage animation is passed in, along with the size of
//	the header plus sprite data.  Remember to check that the function has 
//	successfully completed, as memory allocation is required.
//	Pass NULL in to clear the luggage sprite.  Luggage sprites are of the same
//	format as mouse sprites.
//
//	--------------------------------------------------------------------------
//
//	int32 AnimateMouse(void)
//
//	This function animates the current mouse pointer.  If no pointer is 
//	currently defined, an error code is returned.
//
//=============================================================================


#define WIN32_LEAN_AND_MEAN

//#include <windows.h>
//#include <windowsx.h>

//#include "ddraw.h"

#include "stdafx.h"
#include "driver96.h"

#include "d_draw.h"
#include "render.h"
#include "menu.h"
#include "../sword2.h"


#define MAX_MOUSE_EVENTS 16
#define MOUSEFLASHFRAME 6

// FIXME: Does this struct have to be packed?

typedef struct {
	uint8	runTimeComp;	// type of runtime compression used for the frame data
	uint8	noAnimFrames;	// number of frames in the anim
	int8	xHotSpot;		
	int8	yHotSpot;
	uint8	mousew;
	uint8	mouseh;
} _mouseAnim;

int16				mousex;
int16				mousey;

static	uint8		mouseBacklog = 0;
static	uint8		mouseLogPos = 0;
static	uint8		mouseFrame;
static	uint8		*mouseSprite = NULL;
static	_mouseAnim	*mouseAnim = NULL;
static	_mouseAnim	*luggageAnim = NULL;
static	_mouseEvent	mouseLog[MAX_MOUSE_EVENTS];
static  int32		*mouseOffsets;
static	int32		*luggageOffset;

// FIXME: I have no idea how large the mouse cursor can be. Is this enough?

byte _mouseData[128 * 128];





void ResetRenderEngine(void) {
	parallaxScrollx = 0;
	parallaxScrolly = 0;
	scrollx = 0;
	scrolly = 0;

}




//	--------------------------------------------------------------------------
//	Logs the mouse button event passed in buttons.  The button events are 
//	defined as RD_LEFTBUTTONDOWN, RD_LEFTBUTTONUP, RD_RIGHTBUTTONDOWN and
//	RD_RIGHTBUTTONUP.
//	--------------------------------------------------------------------------
void LogMouseEvent(uint16 buttons)

{

	_mouseEvent *me;


	if (mouseBacklog == MAX_MOUSE_EVENTS-1)			// We need to leave the one which is
	{												// the current event alone!
		return;
	}
	
	me = &mouseLog[(mouseBacklog + mouseLogPos) % MAX_MOUSE_EVENTS];
	me->buttons = buttons;
	mouseBacklog += 1;

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

	// HACK for maximum cursor size
	if (mouse_width + deltaX > 80)
		deltaX = 80 - mouse_width;
	if (mouse_height + deltaY > 80)
		deltaY = 80 - mouse_height;

	mouse_width += deltaX;
	mouse_height += deltaY;

	if ((uint32)(mouse_width * mouse_height) > sizeof(_mouseData)) {
		warning("Mouse cursor too large");
		return;
	}

	memset(_mouseData, 0xFF, mouse_width * mouse_height);

	if (luggageAnim)
		DecompressMouse(_mouseData, (uint8 *) luggageAnim + *luggageOffset, luggageAnim->mousew,
				luggageAnim->mouseh, mouse_width, deltaX, deltaY);

	if (mouseAnim)
		DecompressMouse(_mouseData, mouseSprite, mouseAnim->mousew, mouseAnim->mouseh, mouse_width);

	g_sword2->_system->set_mouse_cursor(_mouseData, mouse_width, mouse_height, hotspot_x, hotspot_y);
}



_mouseEvent *MouseEvent(void)

{
	_mouseEvent *me;

	if (mouseBacklog)
	{
		me = &mouseLog[mouseLogPos];
		if (++mouseLogPos == MAX_MOUSE_EVENTS)
		{
			mouseLogPos = 0;
		}
		mouseBacklog -= 1;
		return(me);
	}

	return(NULL);

}


uint8 CheckForMouseEvents(void)		// (James23july97)
{
	return (mouseBacklog);	// return the number of mouse events waiting	
}


int32 AnimateMouse(void) {
	uint8 prevMouseFrame = mouseFrame;

	if (!mouseAnim)
		return RDERR_UNKNOWN;

	if (++mouseFrame == mouseAnim->noAnimFrames)
		mouseFrame = MOUSEFLASHFRAME;
	mouseSprite = (uint8 *) mouseAnim + *(mouseOffsets + mouseFrame);

	if (mouseFrame != prevMouseFrame)
		DrawMouse();

	return RD_OK;
}



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

		g_sword2->_system->show_mouse(true);
	} else {
		if (luggageAnim)
			DrawMouse();
		else
			g_sword2->_system->show_mouse(false);
	}
	return RD_OK;
}


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

		g_sword2->_system->show_mouse(true);
	} else {
		if (mouseAnim)
			DrawMouse();
		else
			g_sword2->_system->show_mouse(false);
	}
	return RD_OK;
}


