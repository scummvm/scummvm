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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/vdraw.h"
#include "saga2/gpointer.h"
#include "saga2/input.h"

namespace Saga2 {

/* ======================================================================= *
   gMousePointer class
 * ======================================================================= */

//  The mouse pointer constructor

/********* gpointer.cpp/gMousePointer::gMousePointer *****************
*
*       NAME gMousePointer::gMousePointer
*
*   SYNOPSIS
*
*   FUNCTION
*
*     INPUTS
*
*     RESULT
*
**********************************************************************
*/
gMousePointer::gMousePointer(gDisplayPort &port) {
	hideCount = 0;                          // pointer not hidden

	//  initialize coords
	offsetPosition.x = offsetPosition.y = 0;
	currentPosition.x = currentPosition.y = 0;

	//  a backsave extent of 0 means not saved
	saveExtent.width = saveExtent.height = 0;
	shown = 0;

	//  set up the backsave port
	savePort.setMap(&saveMap);
	savePort.setMode(drawModeReplace);

	videoPort = &port;

	//  no imagery at this time.
	pointerImage = NULL;
}

//  The destructor

/********* gpointer.cpp/gMousePointer::~gMousePointer ****************
*
*       NAME gMousePointer::~gMousePointer
*
*   SYNOPSIS
*
*   FUNCTION
*
*     INPUTS
*
*     RESULT
*
**********************************************************************
*/
gMousePointer::~gMousePointer(void) {
	//  Free memory we got earlier

	if (saveMap.data)     free(saveMap.data);
}

//  Init & status check

/********* gpointer.cpp/gMousePointer::~gMousePointer ****************
*
*       NAME gMousePointer::~gMousePointer
*
*   SYNOPSIS
*
*   FUNCTION
*
*     INPUTS
*
*     RESULT
*
**********************************************************************
*/
bool gMousePointer::init(Point16 pointerLimits) {
#ifndef _WIN32
	return (errNoMouse != InitMouse(pointerLimits.x, pointerLimits.y));
#else
	return TRUE;
#endif
}
//  Private routine to draw the mouse pointer image

/********* gpointer.cpp/gMousePointer::draw **************************
*
*       NAME gMousePointer::draw
*
*   SYNOPSIS
*
*   FUNCTION
*
*     INPUTS
*
*     RESULT
*
**********************************************************************
*/
void gMousePointer::draw(void) {
#if     defined( USEWINDOWS )
	if (useWinCursor)
		return;
#endif

	if (pointerImage && hideCount < 1 && saveMap.data) {
		//  Compute the area actually drawn

		saveExtent =
		    intersect(videoPort->clip,
		              Rect16(currentPosition.x,  currentPosition.y,
		                     pointerImage->size.x, pointerImage->size.y));

		//  Added this to fix a shortcoming of intersect().
		if (saveExtent.width <= 0 || saveExtent.height <= 0) {
			shown = 0;
			return;
		}

		//  blit from the screen to the backsave buffer
		videoPort->displayPage->readPixels(saveExtent,
		                                   saveMap.data,
		                                   saveMap.size.x);

		//  Draw the actual pointer (the color-remapped one)
		enum draw_modes saveMode = videoPort->drawMode;// current drawing mode

		videoPort->setMode(drawModeMatte);
		videoPort->bltPixels(
		    *pointerImage,
		    0, 0,
		    currentPosition.x, currentPosition.y,
		    pointerImage->size.x, pointerImage->size.y);

		//  undo any changes we did to the port
		videoPort->setMode(saveMode);
		shown = 1;
	} else {
		shown = 0;
	}
}

//  Private routine to restore the mouse pointer image

/********* gpointer.cpp/gMousePointer::restore ***********************
*
*       NAME gMousePointer::restore
*
*   SYNOPSIS
*
*   FUNCTION
*
*     INPUTS
*
*     RESULT
*
**********************************************************************
*/
void gMousePointer::restore(void) {
#if     defined( USEWINDOWS )
	if (useWinCursor)
		return;
#endif

	enum draw_modes saveMode = videoPort->drawMode; // current drawing mode

	if (shown) {
		//  blit from the saved map to the current position.

		videoPort->displayPage->writePixels(saveExtent,
		                                    saveMap.data,
		                                    saveMap.size.x);

		//  A height of zero means backsave is invalid

		shown = 0;
	}
}

//  Makes the mouse pointer visible

/********* gpointer.cpp/gMousePointer::show **************************
*
*       NAME gMousePointer::show
*
*   SYNOPSIS
*
*   FUNCTION
*
*     INPUTS
*
*     RESULT
*
**********************************************************************
*/
void gMousePointer::show(void) {
	ASSERT(hideCount > 0);

	if (--hideCount == 0) {
#if     defined( USEWINDOWS )
		WinResumeTransfer();
#endif

		draw();
	}
}

//  Makes the mouse pointer invisible

/********* gpointer.cpp/gMousePointer::hide **************************
*
*       NAME gMousePointer::hide
*
*   SYNOPSIS
*
*   FUNCTION
*
*     INPUTS
*
*     RESULT
*
**********************************************************************
*/
void gMousePointer::hide(void) {
	if (hideCount++ == 0) {
#if     defined( USEWINDOWS )
		WinPauseTransfer();
#endif
		restore();
	}
}

//  Makes the mouse pointer visible

/********* gpointer.cpp/gMousePointer::show **************************
*
*       NAME gMousePointer::show
*
*   SYNOPSIS
*
*   FUNCTION
*
*     INPUTS
*
*     RESULT
*
**********************************************************************
*/
void gMousePointer::show(gPort &port, Rect16 r) {
	Point16         org = port.getOrigin();

	r.x += org.x;
	r.y += org.y;

#if     defined( USEWINDOWS )
	// KMY NOTE:
	// In this case, (special), just in case saveExtent.overlap(r) is not always true in matching
	// pairs, we pause/resume regardless.  Maybe I'm being anal, but it's safer and not really any
	// performance hit.
	WinResumeTransfer();
#endif

	if (saveExtent.overlap(r)) {
		if (--hideCount == 0) draw();
	}
}
//  Makes the mouse pointer visible

/********* gpointer.cpp/gMousePointer::manditoryShow **************************
*
*       NAME gMousePointer::manditoryShow
*
*   SYNOPSIS Forces display of mouse pointer (for error handlers & such)
*
*   FUNCTION
*
*     INPUTS
*
*     RESULT
*
**********************************************************************
*/
int gMousePointer::manditoryShow(void) {
	int rv = 0;
	while (hideCount > 0) {
		show();
		rv++;
	}
	while (hideCount < 0) {
		hide();
		rv--;
	}
	if (!shown) draw();
	return rv;
}


//  Makes the mouse pointer invisible

/********* gpointer.cpp/gMousePointer::hide **************************
*
*       NAME gMousePointer::hide
*
*   SYNOPSIS
*
*   FUNCTION
*
*     INPUTS
*
*     RESULT
*
**********************************************************************
*/
void gMousePointer::hide(gPort &port, Rect16 r) {
	Point16         org = port.getOrigin();

	r.x += org.x;
	r.y += org.y;

#if     defined( USEWINDOWS )
	// KMY NOTE:
	// In this case, (special), just in case saveExtent.overlap(r) is not always true in matching
	// pairs, we pause/resume regardless.  Maybe I'm being anal, but it's safer and not really any
	// performance hit.
	WinPauseTransfer();
#endif

	if (saveExtent.overlap(r)) {
		if (hideCount++ == 0) restore();
	}
}

//  Moves the mouse pointer to a new position

/********* gpointer.cpp/gMousePointer::move **************************
*
*       NAME gMousePointer::move
*
*   SYNOPSIS
*
*   FUNCTION
*
*     INPUTS
*
*     RESULT
*
**********************************************************************
*/
void gMousePointer::move(Point16 pos) {
	Point16         offsetPos = pos + offsetPosition;

	if (offsetPos != currentPosition) {
		restore();
		currentPosition = offsetPos;
		draw();
	}
}

//  Sets the mouse pointer imagery

/********* gpointer.cpp/gMousePointer::setImage **********************
*
*       NAME gMousePointer::setImage
*
*   SYNOPSIS
*
*   FUNCTION
*
*     INPUTS
*
*     RESULT
*
**********************************************************************
*/
void gMousePointer::setImage(
    gPixelMap       &img,
    int             x,
    int             y) {
	Point16         pos = currentPosition - offsetPosition;

	if (pointerImage != &img
	        ||  x != offsetPosition.x
	        ||  y != offsetPosition.y
	        ||  img.size != saveMap.size) {
		offsetPosition.x = x;
		offsetPosition.y = y;

		hide();
		if (saveMap.data) free(saveMap.data);
		saveMap.size = img.size;
		saveMap.data = (uint8 *)malloc(img.bytes());
		pointerImage = &img;
		currentPosition = pos + offsetPosition;
#if     defined( USEWINDOWS )
		if (useWinCursor)
			SetWinCursor(img, x, y);
#endif
		show();
	}
}

} // end of namespace Saga2
