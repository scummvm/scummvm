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

#ifndef SAGA2_VIDEOBOX_H
#define SAGA2_VIDEOBOX_H

#include "saga2/modal.h"
#include "saga2/grequest.h"

namespace Saga2 {

// scroll class
class CVideoBox : public ModalWindow {
public:
	enum {
		kVBxSize   = 340,
		kVBySize   = 220,
		kVBx       = (640 - kVBxSize) / 2,
		kVBy       = (480 - kVBySize) / 3
	};

private:
	enum brush {
		kVBxBrushSize  = 340,  // size of each brush 'chunk'.
		kVByBrushSize  = 110,
		kVBnumBrushes  = 2     // number of chunks
	};

	enum borderWidth {
		kVBborderWidth = 6
	};

	enum {
		kVBvidPan1ResID = 0,
		kVBvidPan2ResID
	};

public:

	// resource handle
	hResContext     *_decRes;

	// requester info struct
	requestInfo     _rInfo;

	// rect for the window
	Rect16  _vidBoxRect;

	// rect for the window panes
	Rect16  _vidPanRects[kVBnumBrushes];

public:
	// decoration declarations
	WindowDecoration _vidDec[kVBnumBrushes];


protected:
	void drawClipped(
	    gPort         &port,
	    const Point16 &offset,
	    const Rect16  &clipRect);

	void draw();             // redraw the window

private:
	bool activate(gEventType why);       // activate the control
	void deactivate();
	void pointerMove(gPanelMessage &msg);
	bool pointerHit(gPanelMessage &msg);
	void pointerDrag(gPanelMessage &msg);
	void pointerRelease(gPanelMessage &msg);

public:
	CVideoBox(const Rect16 &box,
	          uint16,
	          AppFunc *cmd = NULL);

	~CVideoBox();

	// returns the active area of this video box
	static Rect16 getAreaRect() {
		return Rect16(kVBx, kVBy, kVBxSize, kVBySize);
	}

	// initializes the resources for this object
	void init();

	// opens and initalizes vid box
	int16 openVidBox(char *fileName);
};

// function prototypes
int16 openVidBox(char *fileName);

} // end of namespace Saga2

#endif
