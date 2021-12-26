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

#ifndef SAGA2_GTEXTBOX_H
#define SAGA2_GTEXTBOX_H

namespace Saga2 {

enum textBoxFlags {
	//  First 4 flags are the text_positions flags for label placement
	textBoxAlignRight   = (1 << 4),
	textBoxAlignCenter  = (1 << 5),
	textBoxNoFilter     = (1 << 6),     // let non-edit keys come through
	textBoxStayActive   = (1 << 7),
	textBoxNoBevel      = (1 << 8)
};

// edit box defines
const   int editLen         = 35;
const   int numEditLines    = 50;
const   int textPen         = 12;
const   int textDisable     = 14;
const   int textHilite      = 11;
const   int textBackground  = 87;
const   int textBackHilite  = 211;
const   int cursorColor     = 174;
const   int textHeight      = 10;

const int32 blinkTime   = 72 / 6;
const int16 blinkColor0 = 137;
const int16 blinkColor1 = 232;
const int16 blinkWide   = 1;

extern StaticRect editBaseRect;


/* ===================================================================== *
   gTextBox class: Single line text-editing control
 * ===================================================================== */

// displays a text box and allows single line editing
class gTextBox : public gControl {
private:

	char    **fieldStrings;
	char    *undoBuffer;                // undo buffer for editing
	bool    internalBuffer;

	// editor values
	uint16  maxLen,
	        currentLen[numEditLines],
	        exists[numEditLines],
	        undoLen,
	        cursorPos,
	        anchorPos,
	        scrollPixels;
	uint16  flags;

	// text display values
	int8    fontOffset;
	int8    linesPerPage;
	int8    index;
	int8    endLine;
	int8    oldMark;

	// font settings
	gFont   *textFont;
	gFont   *oldFont;
	int8    fontHeight;
	int8    fontColorFore;
	int8    fontColorBack;
	int8    fontColorHilite;
	byte    fontColorBackHilite;
	byte    cursorColor;
	int32   blinkStart;
	int16   blinkX;
	int8    blinkState;


	// editing switch values
	bool    displayOnly;
	bool    editing;
	Rect16  editRect;
	bool    hilit;
	bool    noUndo;
	bool    fullRedraw;
	bool    inDrag;
	bool    isActiveCtl;

	AppFunc         *onEnter;
	AppFunc         *onEscape;

	gPanelList  *parent;            // window

protected:

	void prepareEdit(int which);
	void revertEdit();
	void commitEdit();
	bool changed();

	void scroll(int8);

	void enSelect(int which);
	void deSelect(bool commit = false);
	void reSelect(int which);

	bool activate(gEventType why);       // activate the control
	void deactivate();

	bool pointerHit(gPanelMessage &msg);
	void pointerDrag(gPanelMessage &msg);
	void pointerRelease(gPanelMessage &msg);
	bool keyStroke(gPanelMessage &msg);

	void selectionMove(int howMany = 1);


	bool insertText(char *text, int length);
	void setText(char *text);
	void setEditExtent(const Rect16 &r);
	void selectionUp(int howMany = 1) {
		selectionMove(-howMany);
	}
	void selectionDown(int howMany = 1) {
		selectionMove(howMany);
	}
	void drawAll(gPort &, const Point16 &, const Rect16 &);

	// Returns a pointer to the currently selected text of length
	char *selectedText(int &length);
	void handleTimerTick(int32 tick);

	void editRectFill(gPort &fillPort, gPen *pen);
	void drawContents();

public:

	gTextBox(gPanelList   &list,            // window
	         const Rect16 &box,          // rectangle
	         gFont       *font,          // font specified for this box
	         int8        height,         // height of the font
	         int8        FGColor,        // foreground color of font
	         int8        BGColor,        // background color of font
	         int8        HLColor,        // hilite color of font
	         byte        BGHLColor,      // back ground hilite color
	         byte        CRColor,        // cusor color
	         const char  *title,         // title bar
	         const char  *buffer,        // buffer to edit
	         char        **stringBufs,
	         uint16      length,         // length of buffer
	         uint16      flg,            // various flags
	         uint16      ident,          // control ID
	         bool        noEditing,      // display text or allow edit
	         AppFunc     *cmd = NULL,    // application command func
	         AppFunc     *cmdEnter = NULL, // Appfunc for enter key
	         AppFunc     *cmdEscape = NULL); // Appfunc for escape key
	~gTextBox();


	void drawClipped(gPort &, const Point16 &, const Rect16 &) {
		drawClipped();
	}
	void drawClipped();
	void draw() {
		drawClipped();    // redraw the panel.
	}

	bool tabSelect();

	virtual void timerTick(gPanelMessage &msg);

	void scrollUp();
	void scrollDown();

	char *getLine(int8);
	int8 getIndex() {
		return index;
	}

	void killChanges() {
		revertEdit();
	}
	void keepChanges() {
		commitEdit();
	}

	void choose(int which = 0) {
		reSelect(which);
		if (!isActive()) makeActive();
	}

};

} // end of namespace Saga2

#endif
