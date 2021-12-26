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
#include "saga2/videobox.h"
#include "saga2/vpal.h"

namespace Saga2 {

// imports
extern BackWindow   *mainWindow;


/* ===================================================================== *
    CVideoBox: Methods
 * ===================================================================== */

CVideoBox::CVideoBox(const Rect16 &box,
                     uint16 ident,
                     AppFunc *cmd) : ModalWindow(box, ident, cmd) {
	// set the size of the window panes
	vidPanRects[0] =  Rect16(x, y, xBrushSize, yBrushSize);
	vidPanRects[1] =  Rect16(x, y + yBrushSize, xBrushSize, yBrushSize);

	// options dialog window decorations
	vidDec[0].set(vidPanRects[0], vidPan1ResID);
	vidDec[1].set(vidPanRects[1], vidPan2ResID);

	// null out the decRes pointer
	decRes = nullptr;

	rInfo.result = -1;
	rInfo.running = false;
}

CVideoBox::~CVideoBox() {
	// remove the resource handle
	if (decRes)
		resFile->disposeContext(decRes);
	decRes = nullptr;

	// stop video if not done
	g_vm->abortVideo();
}

void CVideoBox::deactivate() {
	selected = 0;
	gPanel::deactivate();
}

bool CVideoBox::activate(gEventType why) {
	if (why == gEventMouseDown) {        // momentarily depress
		selected = 1;
		notify(why, 0);                      // notify App of successful hit
		return true;
	}
	return false;
}

void CVideoBox::pointerMove(gPanelMessage &) {
	notify(gEventMouseMove, 0);
}

bool CVideoBox::pointerHit(gPanelMessage &) {
	// mouse hit detected, close video box
	gWindow         *win;
	requestInfo     *ri;

	win = getWindow();      // get the window pointer
	ri = win ? (requestInfo *)win->userData : nullptr;

	if (ri) {
		ri->running = 0;
		ri->result  = id;
	}

	activate(gEventMouseDown);
	return true;
}

void CVideoBox::pointerDrag(gPanelMessage &) {
	if (selected) {
		notify(gEventMouseDrag, 0);
	}
}

void CVideoBox::pointerRelease(gPanelMessage &) {
	if (selected) notify(gEventMouseUp, 0);    // notify App of successful hit
	deactivate();
}

void CVideoBox::drawClipped(
    gPort         &port,
    const Point16 &offset,
    const Rect16  &clipRect) {
	g_vm->_pointer->hide();
	ModalWindow::drawClipped(port, offset, clipRect);
	g_vm->_pointer->show();
}

void CVideoBox::draw() {         // redraw the window
	// draw the decoration stuff
	drawClipped(g_vm->_mainPort, Point16(0, 0), _extent);
}


void CVideoBox::init() {
	assert(resFile);

	// set the result info to nominal startup values
	rInfo.result    = -1;
	rInfo.running   = true;

	// init the resource context handle
	decRes = resFile->newContext(MKTAG('V', 'I', 'D', 'O'),
	                             "Video border resources");


	// get the decorations for this window
	setDecorations(vidDec,
	               ARRAYSIZE(vidDec),
	               decRes,
	               'V', 'B', 'D');

	// attach the result info struct to this window
	userData = &rInfo;
}

int16 CVideoBox::openVidBox(char *fileName) {
	// initalize the graphics for this window
	init();

	// call the rest of the required functionality
	ModalWindow::open();

	// start the video playback
	g_vm->startVideo(fileName, x + borderWidth, y + borderWidth);

	// run this modal event loop
	//EventLoop( rInfo.running, true );
	rInfo.running = g_vm->checkVideo();
	while (rInfo.running)
		rInfo.running = g_vm->checkVideo();

	// get the result
	return rInfo.result;
}

// this opens a video box for business
int16 openVidBox(char *fileName) {
	// get the area of the vid box
	Rect16 area = CVideoBox::getAreaRect();

	g_vm->_pal->quickSavePalette();
	// create a video box
	CVideoBox videoBox(area, 0, nullptr);

	// open this video box
	int16 result = videoBox.openVidBox(fileName);

	// get rid of the box when done
	videoBox.~CVideoBox();

	g_vm->_pal->quickRestorePalette();
	// replace the damaged area
	mainWindow->invalidate(&area);

	// return the result code
	return result;
}

} // end of namespace Saga2
