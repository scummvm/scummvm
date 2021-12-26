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
#include "saga2/grequest.h"
#include "saga2/modal.h"
#include "saga2/mouseimg.h"

namespace Saga2 {

extern gFont        *mainFont;
extern vDisplayPage *drawPage;

const int           maxLines = 16,
                    maxButtons = 16,
                    maxText = 512,
                    maxButtonText = 128,
                    buttonWidth = 60;

//  This function takes a string which is composed of several
//  substrings, each separated by a delimiter character, and
//  breaks it up into the individual sub-strings.

int16 SplitString(
    char            *text,
    char            *textStart[],
    int16           maxStrings,
    char            delimiter) {
	int16           count;

	for (count = 0; count < maxStrings;) {
		textStart[count++] = text;
		if ((text = strchr(text, delimiter)) == nullptr) break;
		*text++ = '\0';
	}
	return count;
}


static void handleRequestEvent(gEvent &ev) {
	gWindow         *win;
	requestInfo     *ri;

	if (ev.panel && ev.eventType == gEventNewValue && ev.value) {
		win = ev.panel->getWindow();        // get the window pointer
		ri = win ? (requestInfo *)win->userData : nullptr;

		if (ri) {
			ri->running = 0;
			ri->result = ev.panel->id;
		}
	}
}

/* ===================================================================== *
   ModalDialogWindow class: The base class for modal dialog windows
 * ===================================================================== */

class ModalDialogWindow : public ModalWindow {

	int16   titleCount;
	Point16 titlePos[maxLines];
	char    *titleStrings[maxLines];
	char    titleBuf[maxText];

	void positionText(
	    char *windowText,
	    va_list args,
	    const Rect16 &textArea);

public:
	ModalDialogWindow(const Rect16 &r,
	                  uint16 ident,
	                  AppFunc *cmd,
	                  char *windowText,
	                  va_list args);
	ModalDialogWindow(const Rect16 &r,
	                  uint16 ident,
	                  AppFunc *cmd,
	                  char *windowText,
	                  va_list args,
	                  const Rect16 &textArea);

	void drawClipped(
	    gPort         &port,
	    const Point16 &offset,
	    const Rect16  &r) override;
};

void ModalDialogWindow::positionText(
    char *windowText,
    va_list args,
    const Rect16 &textArea) {
	if (windowText) {
		int16   i,
		        yPos,
		        maxY;

		int16   fontHeight = mainFont->height;

		// make a copy of the window text string
		vsprintf(titleBuf, windowText, args);

		//  break up the title text string
		titleCount = SplitString(titleBuf, titleStrings, maxLines, '\n');

		yPos = textArea.y +
		       ((textArea.height - titleCount * fontHeight) >> 1);
		yPos = MAX(yPos, textArea.y);

		maxY = textArea.y + textArea.height - fontHeight;

		for (i = 0; i < titleCount; i++, yPos += fontHeight) {
			if (yPos < maxY) {
				titlePos[i].y = yPos;
				titlePos[i].x =
				    textArea.x +
				    ((textArea.width -
				      TextWidth(mainFont, titleStrings[i], -1, 0))
				     >> 1);
			} else titleCount = i;
		}
	} else titleCount = 0;
}


ModalDialogWindow::ModalDialogWindow(
    const Rect16 &r,
    uint16 ident,
    AppFunc *cmd,
    char *windowText,
    va_list args) :
	ModalWindow(r, ident, cmd) {
	positionText(windowText, args, Rect16(0, 0, r.width, r.height));
}

ModalDialogWindow::ModalDialogWindow(
    const Rect16 &r,
    uint16 ident,
    AppFunc *cmd,
    char *windowText,
    va_list args,
    const Rect16 &textArea) :
	ModalWindow(r, ident, cmd) {
	positionText(windowText, args, textArea);
}

void ModalDialogWindow::drawClipped(
    gPort         &port,
    const Point16 &offset,
    const Rect16  &r) {
	if (!_extent.overlap(r)) return;

	int16   i;
	Point16 origin;
	gFont   *textFont = mainFont;
	Rect16  rect;

	SAVE_GPORT_STATE(port);

	origin.x = _extent.x - offset.x;
	origin.y = _extent.y - offset.y;

	rect.x = origin.x;
	rect.y = origin.y;
	rect.width  = _extent.width;
	rect.height = _extent.height;

	port.setColor(4);
	port.frameRect(rect, 2);
	rect.expand(-2, -2);
	port.setColor(12);
	port.fillRect(rect);

	port.setFont(textFont);
	for (i = 0; i < titleCount; i++) {
		Point16 textPos = origin + titlePos[i];

		port.moveTo(textPos + Point16(-1, -1));
		port.setColor(2);
		port.drawText(titleStrings[i], -1);
		port.moveTo(textPos + Point16(1, 1));
		port.setColor(14);
		port.drawText(titleStrings[i], -1);
		port.moveTo(textPos);
		port.setColor(8);
		port.drawText(titleStrings[i], -1);
	}

	ModalWindow::drawClipped(port, offset, r);
}

/* ===================================================================== *
   ModalRequestWindow class: A modal request dialog box
 * ===================================================================== */

class ModalRequestWindow : public ModalDialogWindow {
	char    buttonBuf[maxButtonText];

	static Rect16 getTextArea(const Rect16 &r) {
		return Rect16(2, 2, r.width - 4, r.height - mainFont->height - 12);
	}

public:
	ModalRequestWindow(
	    const Rect16 &r,
	    uint16 ident,
	    AppFunc *cmd,
	    char *windowText,
	    char *buttonText,
	    va_list args);
};

ModalRequestWindow::ModalRequestWindow(
    const Rect16 &r,
    uint16 ident,
    AppFunc *cmd,
    char *windowText,
    char *buttonText,
    va_list args) :
	ModalDialogWindow(r, ident, cmd, windowText, args, getTextArea(r)) {
	int16   i;

	int16   xPos;

	int16   extraSpace,
	        intervals;

	int16   buttonCount;
	char    *buttonStrings[maxButtons];

	int16   fontHeight = mainFont->height;

	Common::strlcpy(buttonBuf, (buttonText ? buttonText : "_OK"), sizeof(buttonBuf));

	//  break up the button text string
	buttonCount = SplitString(buttonBuf, buttonStrings, maxButtons, '|');

	extraSpace = r.width - buttonWidth * buttonCount;

	intervals = buttonCount * 2 + 1;
	xPos = 0;

	for (i = 0; i < buttonCount; i++) {
		int16       width,
		            extra;

		width = buttonWidth;

		if (intervals > 0) {
			extra = extraSpace / intervals;

			extraSpace -= extra;
			intervals--;
			xPos += extra;
		}

		if (intervals > 0) {
			extra = extraSpace / intervals;

			extraSpace -= extra;
			intervals--;
		} else extra = 0;

		width += extra;

		new LabeledButton(*this,
		                     Rect16(xPos,
		                            r.height - fontHeight - 8,
		                            width,
		                            fontHeight + 6),
		                     *mouseCursors[kMouseCloseBx2Image],
		                     *mouseCursors[kMouseCloseBx1Image],
		                     buttonStrings[i],
		                     i,
		                     handleRequestEvent);

		xPos += width;
	}
}

/* ===================================================================== *
   ModalDisplayWindow class: A modal text display window
 * ===================================================================== */

class ModalDisplayWindow : public ModalDialogWindow {
public:
	ModalDisplayWindow(
	    const Rect16 &r,
	    uint16 ident,
	    AppFunc *cmd,
	    char *windowText,
	    va_list args) :
		ModalDialogWindow(r, ident, cmd, windowText, args) {
	}

	void pointerRelease(gPanelMessage &) override;
	bool keyStroke(gPanelMessage &) override;
};

void ModalDisplayWindow::pointerRelease(gPanelMessage &) {
	requestInfo     *ri = (requestInfo *)userData;
	if (ri) ri->running = false;
}

bool ModalDisplayWindow::keyStroke(gPanelMessage &) {
	requestInfo     *ri = (requestInfo *)userData;
	if (ri) ri->running = false;
	return true;
}

/* ===================================================================== *
   Modal game dialog functions
 * ===================================================================== */

int16 GameDialogA(
    char            *windowText,
    char            *buttonText,
    int             /*resnum*/,
    va_list         args) {
	requestInfo     rInfo;

	ModalRequestWindow *win;

	rInfo.result = -1;
	rInfo.running = true;

	win = new ModalRequestWindow(
	          Rect16((drawPage->size.x - 200) / 2,
	                 (drawPage->size.y - 100) / 3,
	                 200,
	                 100),
	          0,
	          nullptr,
	          windowText,
	          buttonText,
	          args);

	if (win == nullptr) {
		//  REM: perhaps a memory alert of some sort???
		error("Unable to open requester window.");
	}

	win->userData = &rInfo;
	win->open();

	EventLoop(rInfo.running, false);

	delete win;
	return rInfo.result;
}

int16 GameDialog(
    char            *windowText,
    char            *buttonText,
    int             resNum,
    ...) {
	int16   result;
	va_list argptr;

	va_start(argptr, resNum);
	result = GameDialogA(windowText,
	                     buttonText,
	                     resNum,
	                     argptr);
	va_end(argptr);
	return result;
}

int16 GameDisplayA(
    char            *windowText,
    int             /*resnum*/,
    va_list         args) {
	requestInfo     rInfo;

	ModalDisplayWindow *win;

	rInfo.result = -1;
	rInfo.running = true;

	win = new ModalDisplayWindow(
	          Rect16((drawPage->size.x - 200) / 2,
	                 (drawPage->size.y - 100) / 3,
	                 200,
	                 100),
	          0,
	          nullptr,
	          windowText,
	          args);

	if (win == nullptr) {
		//  REM: perhaps a memory alert of some sort???
		error("Unable to open requester window.");
	}

	win->userData = &rInfo;
	win->open();

	EventLoop(rInfo.running, false);

	delete win;
	return rInfo.result;
}

int16 GameDisplay(
    char            *windowText,
    int             resNum,
    ...) {
	int16   result;
	va_list argptr;

	va_start(argptr, resNum);
	result = GameDisplayA(windowText,
	                      resNum,
	                      argptr);
	va_end(argptr);
	return result;
}

} // end of namespace Saga2
