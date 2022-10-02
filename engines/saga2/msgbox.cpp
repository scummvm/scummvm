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
#include "saga2/fta.h"
#include "saga2/button.h"
#include "saga2/localize.h"
#include "saga2/msgbox.h"
#include "saga2/floating.h"
#include "saga2/gbevel.h"
#include "saga2/fontlib.h"

namespace  Saga2 {

const int8 windowColor = 33 + 9;
const int8 buttonColor = 36 + 9;
const int numMessageBtns  = 2;

static const StaticRect mbWindowRect = {70, 170, 500, 140};

static const StaticRect mbOkBtnRect = {100, 100, 100, 25};
static const StaticRect mbCancelBtnRect = {300, 100, 100, 25};
static const StaticRect mbOneBtnRect = {200, 100, 100, 25};
static const StaticRect *mbButtonRects[numMessageBtns] = {
	&mbOkBtnRect,
	&mbCancelBtnRect
};
static gFont    *mbButtonFont = &ThinFix8Font;

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern BackWindow       *mainWindow;

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

APPFUNC(cmdDialogQuit);
int16 MsgBox(const char *msg, const char *btnMsg1, const char *btnMsg2);

inline Rect16 butBox(int n, int i) {
	return (n > 1 ? *mbButtonRects[i] : mbOneBtnRect);
}

/* ===================================================================== *
   Main message box code
 * ===================================================================== */

bool userDialogAvailable();
int16 userDialog(const char *title, const char *msg, const char *btnMsg1, const char *btnMsg2, const char *btnMsg3);

// ------------------------------------------------------------------------
// Wrapper to avoid errors

int16 FTAMessageBox(const char *msg, const char *btnMsg1, const char *btnMsg2) {
	int16 rv = -1;
	if (userDialogAvailable()) {
		rv = (0 == userDialog(ERROR_HEADING, msg, btnMsg1, btnMsg2, nullptr));
	} else
		rv = MsgBox(msg, btnMsg1, btnMsg2);
	return rv;
}

// ------------------------------------------------------------------------
// Very primitive message box

int16 MsgBox(const char *msg, const char *btnMsg1, const char *btnMsg2) {
	ErrorWindow *win = new ErrorWindow(msg, btnMsg1, btnMsg2);
	int16 res = win->getResult();
	delete win;
	return res;
}

char ErrorWindow::_mbChs1Text[8];
char ErrorWindow::_mbChs2Text[8];
uint8 ErrorWindow::_numBtns = 0;
requestInfo ErrorWindow::_rInfo;

APPFUNC(ErrorWindow::cmdMessageWindow) {
	gWindow         *win;
	requestInfo     *ri;

	if (ev.panel && ev.eventType == gEventNewValue && ev.value) {
		win = ev.panel->getWindow();        // get the window pointer
		ri = win ? (requestInfo *)win->_userData : nullptr;

		if (ri) {
			ri->running = 0;
			ri->result = ev.panel->_id;
		}
	}
}


ErrorWindow::ErrorWindow(const char *msg, const char *btnMsg1, const char *btnMsg2)
	: SimpleWindow(mbWindowRect, 0, msg, cmdMessageWindow) {
	_numBtns = 0;

	if (btnMsg1) _numBtns++;
	if (btnMsg2) _numBtns++;

	// requester info struct

	_rInfo.result    = -1;
	_rInfo.running   = true;

	Common::strcpy_s(_mbChs1Text, "\x13");
	Common::strcpy_s(_mbChs2Text, "\x1B");
	const char *eq;
	// button one
	if (btnMsg1) {
		new SimpleButton(*this, butBox(_numBtns, 0), btnMsg1, 0, cmdMessageWindow);
		if ((eq = strchr(btnMsg1, '_')) != nullptr) {
			eq++;
			if (eq)
				_mbChs1Text[strlen(_mbChs1Text)] = *eq;
		}
	}

	// button two
	if (btnMsg2) {
		new SimpleButton(*this, butBox(_numBtns, 1), btnMsg2, 1, cmdMessageWindow);
		if ((eq = strchr(btnMsg2, '_')) != nullptr) {
			eq++;
			if (eq)
				_mbChs2Text[strlen(_mbChs2Text)] = *eq;
		}
	}

	_userData = &_rInfo;

}

int16 ErrorWindow::getResult() {
	open();
	draw();
	EventLoop(_rInfo.running, true);
	return _rInfo.result;
}

ErrorWindow::~ErrorWindow() {
	mainWindow->invalidate(&mbWindowRect);
}



void ErrorWindow::ErrorModeHandleKey(short key, short) {
	if (strchr(_mbChs2Text, tolower(key)) ||
	        strchr(_mbChs2Text, toupper(key))) {
		_rInfo.result    = 2;
		_rInfo.running   = false;
		return;
	}
	if (strchr(_mbChs1Text, tolower(key)) ||
	        strchr(_mbChs1Text, toupper(key))) {
		_rInfo.result    = 1;
		_rInfo.running   = false;
		return;
	}
	if (_numBtns < 2) {
		_rInfo.result    = 1;
		_rInfo.running   = false;
		return;
	}
}

GameMode        SimpleMode = {
	nullptr,                                // no previous mode
	false,                                  // mode is not nestable
	ErrorWindow::ErrorModeSetup,
	ErrorWindow::ErrorModeCleanup,
	ErrorWindow::ErrorModeHandleTask,
	ErrorWindow::ErrorModeHandleKey,
	nullptr
};

/* ===================================================================== *
   SimpleWindow
 * ===================================================================== */


SimpleWindow::SimpleWindow(const Rect16 &r,
                           uint16 ident,
                           const char *stitle,
                           AppFunc *cmd)
	: gWindow(r, ident, "", cmd) {
	_prevModeStackCtr = GameMode::getStack(_prevModeStackPtr);

	GameMode *gameModes[] = {&PlayMode, &TileMode, &SimpleMode};
	GameMode::SetStack(gameModes, 3);
	_title = stitle;
}

SimpleWindow::~SimpleWindow() {
	GameMode::SetStack(_prevModeStackPtr, _prevModeStackCtr);
}

bool SimpleWindow::isModal() {
	return true;
}

void SimpleWindow::update(const Rect16 &) {
}

void SimpleWindow::draw() {
	g_vm->_pointer->hide(g_vm->_mainPort, _extent);              // hide mouse pointer
	drawClipped(g_vm->_mainPort, Point16(0, 0), _extent);
	g_vm->_pointer->show(g_vm->_mainPort, _extent);              // show mouse pointer
}

void SimpleWindow::drawClipped(
    gPort         &port,
    const Point16 &p,
    const Rect16  &r) {
	Rect16          box = _extent;
	//gFont             *buttonFont=&Onyx10Font;
	int16           textPos = textPosHigh;
	//textPallete       pal( 33+9, 36+9, 41+9, 34+9, 40+9, 43+9 );
	textPallete     pal(33 + 9, 33 + 9, 41 + 9, 33 + 9, 33 + 9, 41 + 9);

	box.x += 10;
	box.y += 10;
	box.width -= 20;
	box.height -= 100;

	SAVE_GPORT_STATE(port);                  // save pen color, etc.
	g_vm->_pointer->hide(port, _extent);              // hide mouse pointer

	DrawOutlineFrame(port,  _extent, windowColor);
	writeWrappedPlaqText(port, box, mbButtonFont, textPos, pal, false, _title);

	gWindow::drawClipped(port, p, r);

	g_vm->_pointer->show(port, _extent);              // show mouse pointer
}

/* ===================================================================== *
   Display code
 * ===================================================================== */


void SimpleWindow::writeWrappedPlaqText(gPort           &port,
                                        const Rect16    &r,
                                        gFont           *font,
                                        int16           textPos,
                                        textPallete     &pal,
                                        bool            hiLite,
                                        const char      *msg, ...) {
	char            textBuf[256];
	char            lineBuf[128];
	va_list         argptr;
	int16           tPos = 0;
	Rect16          r2 = r;

	va_start(argptr, msg);
	int cnt = vsprintf(textBuf, msg, argptr);
	va_end(argptr);

	char *text = &textBuf[0];

	while (tPos < cnt && strlen(text)) {
		text = &text[tPos];
		int i = strlen(text);
		char *src = strchr(text, '\n');
		if (src)
			i = (src - text);
		tPos += i;
		memset(lineBuf, '\0', 128);
		Common::strlcpy(lineBuf, text, i);
		writePlaqText(port, r2, font, textPos, pal, hiLite, lineBuf);
		r2.y += font->height + 4;
		r2.height -= font->height + 4;
	}
}

// ------------------------------------------------------------------------
// Very primitive box

void SimpleWindow::DrawOutlineFrame(gPort &port, const Rect16 &r, int16 fillColor) {
	gPenState       saveState;

	port.getState(saveState);

	if (r.width > 3 && r.height > 3) {
		int16       bottom = r.y + r.height - 2,
		            right  = r.x + r.width - 2;

		port.setIndirectColor(whitePen);
		port.vLine(r.x + 1, r.y + 1, r.height - 3);
		port.hLine(r.x + 2, r.y + 1, r.width  - 3);

		port.setIndirectColor(blackPen);
		port.frameRect(r, 1);

		port.setIndirectColor(buttonDkPen);
		port.hLine(r.x + 1,   bottom, r.width - 2);
		port.vLine(right, r.y + 1,    r.height - 2);

		port.setIndirectColor(buttonPen);
		port.setPixel(r.x + 1, bottom);
		port.setPixel(right,   r.y + 1);

		if (fillColor >= 0) {
			port.setIndirectColor(fillColor);
			port.fillRect(r.x + 2, r.y + 2, r.width - 4, r.height - 4);
		}
	}

	port.setState(saveState);
}


/* ===================================================================== *
   SimpleButton
 * ===================================================================== */

SimpleButton::SimpleButton(gWindow &win, const Rect16 &box, const char *title_, uint16 ident, AppFunc *cmd_)
	: gControl(win, box, title_, ident, cmd_) {
	_window = &win;
}

void SimpleButton::deactivate() {
	_selected = 0;
	draw();
	gPanel::deactivate();
}

bool SimpleButton::activate(gEventType why) {
	_selected = 1;
	draw();

	if (why == gEventKeyDown) {             // momentarily depress
		//delay( 200 );
		deactivate();
		notify(gEventNewValue, 1);       // notify App of successful hit
	}
	return false;
}

bool SimpleButton::pointerHit(gPanelMessage &) {
	//if (ghosted) return false;

	activate(gEventMouseDown);
	return true;
}

void SimpleButton::pointerRelease(gPanelMessage &) {
	//  We have to test selected first because deactivate clears it.

	if (_selected) {
		deactivate();                       // give back input focus
		notify(gEventNewValue, 1);       // notify App of successful hit
	} else deactivate();
}

void SimpleButton::pointerDrag(gPanelMessage &msg) {
	if (_selected != msg._inPanel) {
		_selected = msg._inPanel;
		draw();
	}
}

void SimpleButton::draw() {
	gDisplayPort    &port = _window->_windowPort;
	Rect16  rect = _window->getExtent();

	SAVE_GPORT_STATE(port);                  // save pen color, etc.
	g_vm->_pointer->hide(port, _extent);              // hide mouse pointer
	drawClipped(port,
	            Point16(0, 0),
	            Rect16(0, 0, rect.width, rect.height));
	g_vm->_pointer->show(port, _extent);              // show mouse pointer
}

void SimpleButton::drawClipped(
    gPort         &port,
    const Point16 &,
    const Rect16 &) {
	Rect16          base = _window->getExtent();

	Rect16          box = Rect16(_extent.x + 1,
	                             _extent.y + 1,
	                             _extent.width - 2,
	                             _extent.height - 2);
	box.x += base.x;
	box.y += base.y;

	SAVE_GPORT_STATE(port);                  // save pen color, etc.
	g_vm->_pointer->hide(port, _extent);              // hide mouse pointer

	SimpleWindow::DrawOutlineFrame(port,                     // draw outer frame
	                               box,
	                               buttonColor);

	drawTitle((enum text_positions)0);
	g_vm->_pointer->show(port, _extent);              // show mouse pointer
}

} // end of namespace Saga2
