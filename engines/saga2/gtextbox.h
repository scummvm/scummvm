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
	//  First 4 flags are the TextPositions flags for label placement
	kTextBoxAlignRight   = (1 << 4),
	kTextBoxAlignCenter  = (1 << 5),
	kTextBoxNoFilter     = (1 << 6),     // let non-edit keys come through
	kTextBoxStayActive   = (1 << 7),
	kTextBoxNoBevel      = (1 << 8)
};

// edit box defines
const   int kEditLen         = 35;
const   int kNumEditLines    = 50;
const   int kTextPen         = 12;
const   int kTextDisable     = 14;
const   int kTextHilite      = 11;
const   int kTextBackground  = 87;
const   int kTextBackHilite  = 211;
const   int kCursorColor     = 174;
const   int kTextHeight      = 10;

const int32 kBlinkTime   = 72 / 6;
const int16 kBlinkColor0 = 137;
const int16 kBlinkColor1 = 232;
const int16 kBlinkWide   = 1;

extern StaticRect editBaseRect;


/* ===================================================================== *
   gTextBox class: Single line text-editing control
 * ===================================================================== */

// displays a text box and allows single line editing
class gTextBox : public gControl {
private:

	char    **_fieldStrings;
	char    *_undoBuffer;                // undo buffer for editing
	bool    _internalBuffer;

	// editor values
	uint16  _maxLen,
	        _currentLen[kNumEditLines],
	        _exists[kNumEditLines],
	        _undoLen,
	        _cursorPos,
	        _anchorPos,
	        _scrollPixels;
	uint16  _flags;

	// text display values
	int8    _fontOffset;
	int8    _linesPerPage;
	int8    _index;
	int8    _endLine;
	int8    _oldMark;

	// font settings
	gFont   *_textFont;
	gFont   *_oldFont;
	int8    _fontHeight;
	int8    _fontColorFore;
	int8    _fontColorBack;
	int8    _fontColorHilite;
	byte    _fontColorBackHilite;
	byte    _cursorColor;
	int32   _blinkStart;
	int16   _blinkX;
	int8    _blinkState;


	// editing switch values
	bool    _displayOnly;
	bool    _editing;
	Rect16  _editRect;
	bool    _hilit;
	bool    _noUndo;
	bool    _fullRedraw;
	bool    _inDrag;
	bool    _isActiveCtl;

	AppFunc         *_onEnter;
	AppFunc         *_onEscape;

	gPanelList  *_parent;            // window

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
		return _index;
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
