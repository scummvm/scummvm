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
#include "saga2/msgbox.h"

namespace  Saga2 {

const int8 windowColor = 33 + 9;
const int8 buttonColor = 36 + 9;
const int numMessageBtns  = 2;

Rect16  mbWindowRect(70, 170, 500, 140);

const Rect16    mbOkBtnRect(100, 100, 100, 25);
const Rect16    mbCancelBtnRect(300, 100, 100, 25);
const Rect16    mbOneBtnRect(200, 100, 100, 25);
const Rect16    mbButtonRects[ numMessageBtns ] = { { mbOkBtnRect },
	{ mbCancelBtnRect }
};
static gFont    *mbButtonFont = &ThinFix8Font;

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern bool             gameRunning;        // true while game running
extern BackWindow       *mainWindow;

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

APPFUNC(cmdDialogQuit);
int16 MsgBox(char *msg, char *btnMsg1, char *btnMsg2);
void writePlaqText(gPort            &port,
                   const Rect16    &r,
                   gFont           *font,
                   int16           textPos,
                   textPallete     &pal,
                   bool            hiLite,
                   char            *msg, ...);

inline Rect16 butBox(int n, int i) {
	return (n > 1 ? mbButtonRects[i] : mbOneBtnRect);
}

/* ===================================================================== *
   Main message box code
 * ===================================================================== */

bool userDialogAvailable(void);
int16 userDialog(char *title, char *msg, char *btnMsg1,
                 char *btnMsg2,
                 char *btnMsg3);

// ------------------------------------------------------------------------
// Wrapper to avoid errors

int16 FTAMessageBox(char *msg,
                    char *btnMsg1,
                    char *btnMsg2)

{
	int16 rv = -1;
	if (userDialogAvailable()) {
		char *b1 = btnMsg1;
		char *b2 = btnMsg2;
		//if (b1[0]=='_') b1++;
		//if (b2[0]=='_') b2++;

		rv = (0 == userDialog(ERROR_HEADING, msg, b1, b2, NULL));
	} else
		rv = MsgBox(msg, btnMsg1, btnMsg2);
	return rv;
}

// ------------------------------------------------------------------------
// Very primitive message box

int16 MsgBox(char *msg,
             char *btnMsg1,
             char *btnMsg2) {
	ErrorWindow *win = NEW_MSGR ErrorWindow(msg, btnMsg1, btnMsg2);
	int16 res = win->getResult();
	delete win;
	return res;
}

char ErrorWindow::mbChs1Text[8];
char ErrorWindow::mbChs2Text[8];
uint8 ErrorWindow::numBtns = 0;
requestInfo ErrorWindow::rInfo;

APPFUNC(ErrorWindow::cmdMessageWindow) {
	gWindow         *win;
	requestInfo     *ri;

	if (ev.panel && ev.eventType == gEventNewValue && ev.value) {
		win = ev.panel->getWindow();        // get the window pointer
		ri = win ? (requestInfo *)win->userData : NULL;

		if (ri) {
			ri->running = 0;
			ri->result = ev.panel->id;
		}
	}
}


ErrorWindow::ErrorWindow(char *msg,   char *btnMsg1,   char *btnMsg2)
	: SimpleWindow(mbWindowRect, 0, msg, cmdMessageWindow) {
	const int maxBtns = 2;

	numBtns = 0;

	if (btnMsg1) numBtns++;
	if (btnMsg2) numBtns++;

	// requester info struct

	rInfo.result    = -1;
	rInfo.running   = TRUE;

	strcpy(mbChs1Text, "\x13");
	strcpy(mbChs2Text, "\x1B");
	char *eq;
	// button one
	if (btnMsg1) {
		new SimpleButton(*this, butBox(numBtns, 0), btnMsg1, 0, cmdMessageWindow);
		if ((eq = strchr(btnMsg1, '_')) != NULL) {
			eq++;
			if (eq)
				mbChs1Text[strlen(mbChs1Text)] = *eq;
		}
	}

	// button two
	if (btnMsg2) {
		new SimpleButton(*this, butBox(numBtns, 1), btnMsg2, 1, cmdMessageWindow);
		if ((eq = strchr(btnMsg2, '_')) != NULL) {
			eq++;
			if (eq)
				mbChs2Text[strlen(mbChs2Text)] = *eq;
		}
	}

	userData = &rInfo;

}

int16 ErrorWindow::getResult(void) {
	open();
	draw();
	EventLoop(rInfo.running, TRUE);
	return rInfo.result;
}

ErrorWindow::~ErrorWindow() {
	mainWindow->invalidate(mbWindowRect);
}



void ErrorWindow::ErrorModeHandleKey(short key, short) {
	if (strchr(mbChs2Text, tolower(key)) ||
	        strchr(mbChs2Text, toupper(key))) {
		rInfo.result    = 2;
		rInfo.running   = FALSE;
		return;
	}
	if (strchr(mbChs1Text, tolower(key)) ||
	        strchr(mbChs1Text, toupper(key))) {
		rInfo.result    = 1;
		rInfo.running   = FALSE;
		return;
	}
	if (numBtns < 2) {
		rInfo.result    = 1;
		rInfo.running   = FALSE;
		return;
	}
}

GameMode        SimpleMode = {
	NULL,                                   // no previous mode
	FALSE,                                  // mode is not nestable
	ErrorWindow::ErrorModeSetup,
	ErrorWindow::ErrorModeCleanup,
	ErrorWindow::ErrorModeHandleTask,
	ErrorWindow::ErrorModeHandleKey,
};

/* ===================================================================== *
   SimpleWindow
 * ===================================================================== */


SimpleWindow::SimpleWindow(const Rect16 &r,
                           uint16 ident,
                           char *stitle,
                           AppFunc *cmd)
	: gWindow(r, ident, "", cmd) {
	prevModeStackCtr = GameMode::getStack(prevModeStackPtr);
	GameMode::SetStack(&PlayMode, &TileMode, &SimpleMode, End_List);
	title = stitle;
}

SimpleWindow::~SimpleWindow(void) {
	GameMode::SetStack(prevModeStackPtr, prevModeStackCtr);
}

bool SimpleWindow::isModal(void) {
	return TRUE;
}

void SimpleWindow::update(const Rect16 &) {
}

void SimpleWindow::draw(void) {
	pointer.hide(mainPort, extent);              // hide mouse pointer
	drawClipped(mainPort, Point16(0, 0), extent);
	pointer.show(mainPort, extent);              // show mouse pointer
}

void SimpleWindow::drawClipped(
    gPort         &port,
    const Point16 &p,
    const Rect16  &r) {
	Rect16          box = extent;
	//gFont             *buttonFont=&Onyx10Font;
	int16           textPos = textPosHigh;
	//textPallete       pal( 33+9, 36+9, 41+9, 34+9, 40+9, 43+9 );
	textPallete     pal(33 + 9, 33 + 9, 41 + 9, 33 + 9, 33 + 9, 41 + 9);
	bool            selected = FALSE;

	box.x += 10;
	box.y += 10;
	box.width -= 20;
	box.height -= 100;

	SAVE_GPORT_STATE(port);                  // save pen color, etc.
	pointer.hide(port, extent);              // hide mouse pointer

	DrawOutlineFrame(port,  extent, windowColor);
	writeWrappedPlaqText(port, box, mbButtonFont, textPos, pal, selected,  title);

	gWindow::drawClipped(port, p, r);

	pointer.show(port, extent);              // show mouse pointer
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
                                        char            *msg, ...) {
	char            textBuf[ 256 ];
	char            lineBuf[ 128 ];
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
		strncpy(lineBuf, text, i);
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

SimpleButton::SimpleButton(gWindow &win, const Rect16 &box, char *title, uint16 ident,
                           AppFunc *cmd)
	: gControl(win, box, title, ident, cmd) {
	window = &win;
}

void SimpleButton::deactivate(void) {
	selected = 0;
	draw();
	gPanel::deactivate();
}

bool SimpleButton::activate(gEventType why) {
	selected = 1;
	draw();

	if (why == gEventKeyDown) {             // momentarily depress
		//delay( 200 );
		deactivate();
		notify(gEventNewValue, 1);       // notify App of successful hit
	}
	return FALSE;
}

bool SimpleButton::pointerHit(gPanelMessage &) {
	//if (ghosted) return FALSE;

	activate(gEventMouseDown);
	return TRUE;
}

void SimpleButton::pointerRelease(gPanelMessage &) {
	//  We have to test selected first because deactivate clears it.

	if (selected) {
		deactivate();                       // give back input focus
		notify(gEventNewValue, 1);       // notify App of successful hit
	} else deactivate();
}

void SimpleButton::pointerDrag(gPanelMessage &msg) {
	if (selected != msg.inPanel) {
		selected = msg.inPanel;
		draw();
	}
}

void SimpleButton::draw(void) {
	gDisplayPort    &port = window->windowPort;
	Rect16  rect = window->getExtent();

	SAVE_GPORT_STATE(port);                  // save pen color, etc.
	pointer.hide(port, extent);              // hide mouse pointer
	drawClipped(port,
	            Point16(0, 0),
	            Rect16(0, 0, rect.width, rect.height));
	pointer.show(port, extent);              // show mouse pointer
}

void SimpleButton::drawClipped(
    gPort         &port,
    const Point16 &,
    const Rect16 &) {
	Rect16          base = window->getExtent();

	Rect16          box = Rect16(extent.x + 1,
	                             extent.y + 1,
	                             extent.width - 2,
	                             extent.height - 2);
	box.x += base.x;
	box.y += base.y;

	SAVE_GPORT_STATE(port);                  // save pen color, etc.
	pointer.hide(port, extent);              // hide mouse pointer

	SimpleWindow::DrawOutlineFrame(port,                     // draw outer frame
	                               box,
	                               buttonColor);

	drawTitle((enum text_positions)0);
	pointer.show(port, extent);              // show mouse pointer
}

} // end of namespace Saga2
