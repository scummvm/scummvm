/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/vbacksav.h"

namespace Saga2 {

gBackSave::gBackSave(const Rect16 &extent) {
	Rect16  displayRect(0, 0, 640, 480);

	//  initialize the rectangle

	savedRegion = intersect(extent, displayRect);    // intersect with display size

	//  Set up the image structure for the video page

	savedPixels.size.x = savedRegion.width;
	savedPixels.size.y = savedRegion.height;
//	savedPixels.data = (uint8 *)malloc( savedPixels.bytes() );
	savedPixels.data = (uint8 *)malloc(savedPixels.bytes());

	//  Initialize the graphics port

	setMap(&savedPixels);
	setMode(drawModeReplace);

	saved = false;
}

/********* vbacksav.cpp/gBackSave::~gBackSave ************************
*
*       NAME gBackSave::~gBackSave
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
gBackSave::~gBackSave() {
	free(savedPixels.data);
}

/********* vbacksav.cpp/gBackSave::save ******************************
*
*       NAME gBackSave::save
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
void gBackSave::save(gDisplayPort &port) {
	if (!saved && savedPixels.data) {
		port.protoPage.readPixels(savedRegion,
		                             savedPixels.data,
		                             savedPixels.size.x);
		saved = true;
	}
}

/********* vbacksav.cpp/gBackSave::restore ***************************
*
*       NAME gBackSave::restore
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
void gBackSave::restore(gDisplayPort &port) {
	if (saved && savedPixels.data) {
		port.protoPage.writePixels(savedRegion,
		                              savedPixels.data,
		                              savedPixels.size.x);
		saved = false;
	}
}

} // end of namespace Saga2
