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

#ifndef SAGA2_MSGBOX_H
#define SAGA2_MSGBOX_H

#include "saga2/grequest.h"

namespace Saga2 {

struct textPallete;

//Modal Mode GameMode Object

extern GameMode     ModalMode;

/* ===================================================================== *
   ModalWindow --
 * ===================================================================== */

class SimpleWindow : public gWindow {

	GameMode    *prevModeStackPtr[Max_Modes];
	int         prevModeStackCtr;

public:

	SimpleWindow(const Rect16 &r,
	             uint16 ident,
	             const char *title,
	             AppFunc *cmd);
	~SimpleWindow();

	bool isModal();
	void update(const Rect16 &);
	void draw();                         // redraw the panel.
	void drawClipped(gPort &port, const Point16 &offset, const Rect16  &r);
	static void DrawOutlineFrame(gPort &port, const Rect16 &r, int16 fillColor);
	static void writeWrappedPlaqText(gPort          &port,
	                                 const Rect16    &r,
	                                 gFont           *font,
	                                 int16           textPos,
	                                 textPallete     &pal,
	                                 bool            hiLite,
	                                 const char      *msg, ...);

};

class SimpleButton : public gControl {
	gWindow *window;
public:
	SimpleButton(gWindow &, const Rect16 &, const char *, uint16, AppFunc *cmd = NULL);

	void draw();                         // redraw the panel.
	void drawClipped(gPort &port, const Point16 &offset, const Rect16  &r);

private:
	bool activate(gEventType why);       // activate the control
	void deactivate();
	bool pointerHit(gPanelMessage &msg);
	void pointerDrag(gPanelMessage &msg);
	void pointerRelease(gPanelMessage &msg);
};

class ErrorWindow : public SimpleWindow {
	static char mbChs1Text[8];
	static char mbChs2Text[8];
	static uint8    numBtns;
public:

	static requestInfo      rInfo;
	ErrorWindow(const char *msg, const char *btnMsg1, const char *btnMsg2);
	~ErrorWindow();
	int16 getResult();
	static APPFUNC(cmdMessageWindow);
	static void ErrorModeSetup() {}
	static void ErrorModeCleanup() {}
	static void ErrorModeHandleTask() {}
	static void ErrorModeHandleKey(short key, short);

};

int16 FTAMessageBox(const char *msg, const char *btnMsg1, const char *btnMsg2);

} // end of namespace Saga2

#endif
