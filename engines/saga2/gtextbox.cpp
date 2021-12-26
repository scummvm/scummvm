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

#include "common/keyboard.h"

#include "saga2/saga2.h"
#include "saga2/cmisc.h"
#include "saga2/fta.h"
#include "saga2/panel.h"
#include "saga2/gtextbox.h"

namespace Saga2 {

// definitions
StaticRect editBaseRect = {7, 45, 314, 111};

/* ===================================================================== *
   Imports
 * ===================================================================== */

//  These are all things that need to be moved to their proper places.

extern gFont        *mainFont;

//-----------------------------------------------------------------------
// init

/****** gtextbox.cpp/gTextBox [class] ******************************
*
*   NAME
*       gTextBox class -- an editable text string control
*
*   FUNCTION
*       This class manifests as a "string gadget" or editable text
*       box. You supply the buffer of characters to be edited (although
*       it will create an undo buffer).
*
*       gTextBox has the ability to "pass through" some keystrokes
*       to the application (such as ESC and TAB) if desired.
*
*       It does not currently support the concept of cut & paste.
*
*       This class returns true to the "tabSelect" query indicating
*       that it is tab-selectable.
*
*   /h2/NOTIFICATIONS
*       This class sends a gEventNewValue whenever the text box becomes
*       deactivated. If the deactivation occurred because of the user
*       hitting return, then the value of the event will be 1; In all
*       other cases it is zero.
*
*       In addition, a gEventAltValue is sent for each character
*       typed. (Value is always 0). This is to allow other displays
*       to be dynamically updated as the user types.
*
*   SEE ALSO
*       gControl class
*       gTextBox::gTextBox
*       gTextBox::setValue
*
**********************************************************************
*/

/****** gtextbox.cpp/gTextBox::gTextBox ******************************
*
*   NAME
*       gTextBox::gTextBox -- constructor for editable text box
*
*   SYNOPSIS
*       new gTextBox( list, rect, title, buffer, length, flags, id, cmd );
*
*       gTextBox::gTextBox ( gPanelList &, const Rect16 &, char *,
*       /c/ char *, uint16, uint16, uint16, AppFunc * = NULL );
*
*   FUNCTION
*       The constructor for gTextBox initializes the cursor position,
*       scroll position, and text buffers. It also allocates a temporary
*       undo buffer which is deleted by the destructor.
*
*   INPUTS
*       list        Can be either a gWindow or a gPanelList. The newly
*                   constructed control will be placed on the contentsList
*                   of the gWindow or gPanelList. In addition, the panel's
*                   window pointer will be initialized to the gWindow
*                   argument, or to the gPanelList's gWindow pointer if
*                   a gPanelList is passed.
*
*       rect        Defines the rectangular area of the gControl.
*
*       title       The title of the text box.
*
*       buffer      The buffer to place the text in. If the buffer
*                   contains text initially, that text will be
*                   shown when the control is drawn.
*
*       length      The maximum length of the text buffer.
*
*       flag        Various flags which control operation of the
*                   text box:
*
*       /i/         textBoxAlignRight causes the text to
*                   be right-justified within the box.
*
*       /i/         textBoxAlignCenter causes the text to be centered within the box.
*
*       /i/         textBoxNoFilter indicates that non-edit keys
*                   (such as ESC and TAB) should be passed on to the
*                   application's AppFunc.
*
*       /i/         textBoxStayActive tells the text box not to
*                   deactivate when the user hits return.
*
*       id          A 16-bit Application-specific ID for this panel.
*
*       cmd         [Optional Argument] A pointer to the AppFunction
*                   for this panel. AppFunctions are application-defined
*                   callback functions, and should be created using the
*                   APPFUNC() macro.
*
*   RESULT
*
**********************************************************************
*/

gTextBox::gTextBox(
    gPanelList      &list,
    const Rect16    &box,
    gFont           *font,
    int8            height,
    int8            FGColor,
    int8            BGColor,
    int8            HLColor,
    byte            BGHLColor,
    byte            CRColor,
    const char      *title_,
    const char      *buffer,
    char            **stringBufs,
    uint16          length,
    uint16          flg,
    uint16          ident,
    bool            noEditing,
    AppFunc         *cmd,
    AppFunc         *cmdEnter,
    AppFunc         *cmdEscape)
	: gControl(list, box, title_, ident, cmd) {
	int16   i;


	hilit                   = false;
	noUndo                  = false;

	index                   = 0;    // index into string array ( which string )
	maxLen                  = length;
	flags                   = flg;
	currentLen[index]     = buffer ? strlen(buffer) : 0;
	cursorPos               = anchorPos = scrollPixels = 0;
	undoBuffer              = new char[maxLen + 1]();
	textFont                = font;
	oldFont                 = nullptr;
	fontHeight              = height;
	fontOffset              = fontHeight + 2;

	fontColorFore           = FGColor;
	fontColorBack           = BGColor;
	fontColorHilite         = HLColor;
	fontColorBackHilite     = BGHLColor;
	cursorColor             = CRColor;
	linesPerPage            = (box.height / fontOffset);
	endLine                 = clamp(0, (index + linesPerPage), numEditLines);
	oldMark                 = -1;

	displayOnly             = noEditing;
	editing                 = false;
	editRect                = box;
	editRect.height         = fontHeight;
	inDrag                  = false;
	onEnter                 = cmdEnter;
	onEscape                = cmdEscape;
	isActiveCtl             = false;
	selected                = 0;
	parent                  = &list;

	blinkStart = 0;
	blinkX = 0;
	blinkState = 0;

	// set the filedStrings pointer
	fieldStrings = stringBufs;

	// get the size of each string
	for (i = 0; i < numEditLines; i++) {
		exists[i] = ((stringBufs[i][0] & 0x80) == 0);
		stringBufs[i][0] &= 0x7F;
		currentLen[i] = MIN<int>(editLen, strlen(stringBufs[i]));
	}

	internalBuffer = false;
	fullRedraw = true;
	index = 0;
	enSelect(0);
	if (!displayOnly) {
		cursorPos = 0;
		anchorPos = currentLen[index];
	}
	fullRedraw = true;
}

//-----------------------------------------------------------------------

gTextBox::~gTextBox() {
	deSelect();
	selected = 0;
	if (undoBuffer) {
		delete[] undoBuffer;
	}
}

/****** gtextbox.cpp/gTextBox::insertText ****************************
*
*   NAME
*       gTextBox::insertText -- insert text at current cursor position
*
*   SYNOPSIS
*       success = textBox->insertText( newText, length );
*
*       bool gTextBox::insertText( char *, int );
*
*   FUNCTION
*       This function inserts the text string "newText" into the
*       text box at the current cursor position. If a range of
*       characters are selected, they will be deleted and replaced
*       with the new text. The text box will be redrawn in any case.
*
*   INPUTS
*       newText     The text string to insert.
*
*       length      How many characters to insert.
*
*   RESULT
*       true if there was enough room in the buffer to insert the text.
*
**********************************************************************
*/
bool gTextBox::insertText(char *newText, int length) {
	int16       selStart = MIN(cursorPos, anchorPos),
	            selWidth = ABS(cursorPos - anchorPos),
	            selEnd   = selStart + selWidth;

	if (length == -1) length = strlen(newText);

	//  If inserting the text would make the string too long,
	//  then don't insert it.

	if (currentLen[index] - selWidth + length >= maxLen) return false;

	//  Move the text after the selection to where it will be
	//  after the insertion.

	if (selEnd < currentLen[index]) {
		memmove(fieldStrings[index] + selStart + length,
		        fieldStrings[index] + selEnd,
		        currentLen[index] - selEnd);
	}

	//  Move the inserted text, if any, to the opening

	if (length > 0) {
		memmove(fieldStrings[index] + selStart, newText, length);
	}

	//  Set the insertion point to the end of the new text.

	cursorPos = anchorPos = selStart + length;
	currentLen[index] += (length - selWidth);
	fieldStrings[index][currentLen[index]] = '\0';

	return true;
}

/****** gtextbox.cpp/gTextBox::setText *******************************
*
*   NAME
*       gTextBox::setText -- set the text in the text box.
*
*   SYNOPSIS
*       textBox->setText( newText );
*
*       void gTextBox::setText( char * );
*
*   FUNCTION
*       This function replaces the contents of the text buffer with
*       the string "newText" which is assumed to be NULL-terminated.
*       The text box is redrawn.
*
*   INPUTS
*       newText     A null terminated string.
*
*   RESULT
*       none
*
**********************************************************************
*/
void gTextBox::setText(char *newText) {
	int16       len = MIN((int)(strlen(newText)), (int)(maxLen - 1));

	cursorPos = 0;
	anchorPos = currentLen[index];

	insertText(newText, len);
	cursorPos = anchorPos = 0;

	if (window.isOpen()) drawContents();
}

//-----------------------------------------------------------------------

void gTextBox::setEditExtent(const Rect16 &r) {
	editRect.x = r.x;
	editRect.y = r.y;
	editRect.width = r.width;
	editRect.height = r.height;
}


//-----------------------------------------------------------------------

bool gTextBox::activate(gEventType why) {
	if (why == gEventAltValue) {            // momentarily depress
		selected = 1;
		notify(why, 0);                      // notify App of successful hit
		return true;
	}
	isActiveCtl = true;
	if (!selected) {
		enSelect(index);
	}
	selected = 1;
	fullRedraw = true;
	draw();
	if (why == gEventNone)
		return true;
	return gPanel::activate(why);
}

//-----------------------------------------------------------------------

void gTextBox::deactivate() {
	selected = 0;
	isActiveCtl = false;
	draw();
	fullRedraw = true;
	gPanel::deactivate();
}

//-----------------------------------------------------------------------

void gTextBox::prepareEdit(int which) {
	if (!displayOnly) {
		if (undoBuffer) memcpy(undoBuffer, fieldStrings[which], currentLen[which] + 1);
		undoLen = currentLen[which];
	}
}

//-----------------------------------------------------------------------

bool gTextBox::changed() {
	if (undoBuffer && editing) {
		return memcmp(undoBuffer, fieldStrings[index], currentLen[index] + 1);
	}
	return false;
}

//-----------------------------------------------------------------------

void gTextBox::commitEdit() {
	if (undoBuffer && changed()) {
		memcpy(undoBuffer, fieldStrings[index], currentLen[index] + 1);
		undoLen = currentLen[index];
		cursorPos = anchorPos = currentLen[index];
		notify(gEventNewValue, 1);       // tell app about new value
	}
}


//-----------------------------------------------------------------------

void gTextBox::revertEdit() {
	if (undoBuffer && changed()) {
		cursorPos = anchorPos = currentLen[index] = undoLen;
		memcpy(fieldStrings[index], undoBuffer, currentLen[index] + 1);
		notify(gEventNewValue, 0);         // tell app about new value
	}
}

//-----------------------------------------------------------------------
// LINE SELECTION

//-----------------------------------------------------------------------
// Choose the part of the list to display & the specific Item in that list

void gTextBox::scroll(int8 req) {
	int16   indexReq = req;
	int16   oldIndex = index;
	int16   visOld = (oldIndex - (endLine - linesPerPage));
	int16   visBase = endLine;
	int16   visIndex;

	indexReq        = clamp(0, indexReq, numEditLines);
	visIndex = (indexReq - (visBase - linesPerPage));
	if (ABS(oldIndex - indexReq) < 2) {
		if (visIndex < 0) {
			visBase--;
			visIndex++;
		} else if (visIndex >= linesPerPage) {
			visBase++;
			visIndex--;
		}
	} else {
		while (visIndex >= linesPerPage) {
			visBase = clamp(linesPerPage, visBase + linesPerPage, numEditLines);
			visIndex = (indexReq - (visBase - linesPerPage));
		}
		while (visIndex < 0) {
			visBase = clamp(linesPerPage, visBase - linesPerPage, numEditLines);
			visIndex = (indexReq - (visBase - linesPerPage));
		}
	}

	if (endLine != visBase) {
		fullRedraw = true;
	}
	endLine = visBase;

	if (visIndex != visOld) {
		Rect16  textBoxExtent   = editRect;

		// setup the editing extent
		textBoxExtent.y = (fontOffset * visIndex)  + _extent.y;

		setEditExtent(textBoxExtent);
		fullRedraw = true;
	}
}

//-----------------------------------------------------------------------

void gTextBox::deSelect(bool commit) {
	if (index >= 0 && editing) {
		if (commit)
			commitEdit();
		else
			revertEdit();
		editing = false;
		fullRedraw = true;
	}
}

//-----------------------------------------------------------------------

void gTextBox::enSelect(int which) {
	scroll(which);
	index = which;
	if (!displayOnly) {
		prepareEdit(which);
		editing   = true;
		cursorPos = 0;
		anchorPos = currentLen[index];
	} else {
		hilit = true;
	}
}

//-----------------------------------------------------------------------

void gTextBox::reSelect(int which) {
	if (which != index) {
		deSelect(false);
		draw();
		enSelect(which);
		fullRedraw = true;
	}
}

//-----------------------------------------------------------------------
// select a nearby line of text

void gTextBox::selectionMove(int howMany) {
	int8    newIndex;

	newIndex = clamp(0, index + howMany, numEditLines - 1);
#ifndef ALLOW_BAD_LOADS
	if (displayOnly) {
		int i = newIndex;
		if (howMany > 0) {
			while (!exists[i] && i < numEditLines - 1) i++;
			if (!exists[i]) {
				i = newIndex;
				while (!exists[i] && i > 0) i--;
			}
			if (exists[i])
				newIndex = i;
		} else {
			while (!exists[i] && i > 0) i--;
			if (!exists[i]) {
				i = newIndex;
				while (!exists[i] && i < numEditLines - 1) i++;
			}
			if (exists[i])
				newIndex = i;
		}

	}
#endif
	reSelect(newIndex);
	if (!displayOnly) {
		cursorPos = 0;
		anchorPos = currentLen[index];
	}

	draw();

	//activate(gEventAltValue);
}


//-----------------------------------------------------------------------

void gTextBox::scrollUp() {
	selectionUp(linesPerPage - 2);
}

//-----------------------------------------------------------------------

void gTextBox::scrollDown() {
	selectionDown(linesPerPage - 2);
}

//-----------------------------------------------------------------------
// INPUT HANDLING

//-----------------------------------------------------------------------

bool gTextBox::pointerHit(gPanelMessage &msg) {
	Point16 pos             = msg.pickPos;
	int16 newPos;


	if (Rect16(0, 0, _extent.width, _extent.height).ptInside(pos)) {
		int8    newIndex;
		// get the position of the line
		newIndex = clamp(0, pos.y / fontOffset, linesPerPage - 1);
		newIndex = (endLine - (linesPerPage - newIndex));

		if (index != newIndex)
			reSelect(newIndex);
		if (editing) {
			if (textFont) {
				newPos = WhichIChar(textFont, (uint8 *)fieldStrings[index], msg.pickPos.x - 3, currentLen[index]);
			} else {
				newPos = WhichIChar(mainFont, (uint8 *)fieldStrings[index], msg.pickPos.x - 3, currentLen[index]);
			}
			if (msg.leftButton) {
				if (cursorPos != newPos || anchorPos != newPos) {
					anchorPos = newPos;
					cursorPos = newPos;
				}
			}
			draw();
		}
		if (!isActive()) {
			makeActive();
		}
	}
	return true; //gControl::activate( gEventMouseDown );
}



//-----------------------------------------------------------------------

//  Drag-select the text.

void gTextBox::pointerDrag(gPanelMessage &msg) {
	int16 newPos;

	if (msg.leftButton) {
		if (textFont) {
			newPos = WhichIChar(textFont, (uint8 *)fieldStrings[index], msg.pickPos.x - 3, currentLen[index]);
		} else {
			newPos = WhichIChar(mainFont, (uint8 *)fieldStrings[index], msg.pickPos.x - 3, currentLen[index]);
		}
		inDrag = true;
		if (cursorPos != newPos) {
			//if (newPos<cursorPos)
			cursorPos = newPos;
			//else
			//  anchorPos = newPos ;
		}
		draw();
	}

}

//-----------------------------------------------------------------------

//  Mouse release code

void gTextBox::pointerRelease(gPanelMessage &msg) {
	if (!msg.leftButton) {
		inDrag = false;
		draw();
	}
}

//-----------------------------------------------------------------------


bool gTextBox::keyStroke(gPanelMessage &msg) {
	gPort &port = window.windowPort;
	int16 selStart = MIN(cursorPos, anchorPos),
	      selWidth = ABS(cursorPos - anchorPos);
	uint16 key = msg.key;

	//  Process the various keystrokes...
	if (editing && cursorPos > anchorPos) {
		cursorPos = anchorPos;
		anchorPos = cursorPos;
	}

	switch (key) {
	case Common::KEYCODE_UP:
		selectionUp(1);
		return true;

	case Common::KEYCODE_DOWN:
		selectionDown(1);
		return true;

	case Common::KEYCODE_PAGEUP:
		selectionUp(linesPerPage);
		return true;

	case Common::KEYCODE_PAGEDOWN:
		selectionDown(linesPerPage);
		return true;
	}

	if (key == Common::ASCII_RETURN) { // return key
		if (editing) {
			commitEdit();
			if (!(flags & textBoxStayActive))
				deactivate();                       // deactivate the text box
		}

		if (onEnter != nullptr) {
			gEvent ev;
			ev.eventType = gEventKeyDown ;
			ev.value = 1;
			ev.panel = parent;
			(*onEnter)(ev);
		}

		return true;
	} else if (key == Common::ASCII_ESCAPE) {               // escape key
		revertEdit();
		deactivate();                       // deactivate the text box
		if (onEscape != nullptr) {
			gEvent ev;
			ev.eventType = gEventKeyDown ;
			ev.value = 1;
			ev.value = 1;
			ev.panel = this; //parent;
			(*onEscape)(ev);
		}

		if (flags & textBoxNoFilter)
			return false;
		return true;
	} else if (editing) {
		switch (key) {
		case Common::KEYCODE_LEFT:
			if (anchorPos > 0)
				anchorPos--;
			if (!(msg.qualifier & qualifierShift))
				cursorPos = anchorPos;
			break;

		case Common::KEYCODE_RIGHT:
			if (anchorPos < currentLen[index])
				anchorPos++;
			if (!(msg.qualifier & qualifierShift))
				cursorPos = anchorPos;
			break;

		case Common::KEYCODE_HOME:
			cursorPos = 0;
			anchorPos = 0;
			break;

		case Common::KEYCODE_END:
			cursorPos = currentLen[index];
			anchorPos = currentLen[index];
			break;

		case Common::KEYCODE_z: // Alt-Z
			if (msg.qualifier & (qualifierControl | qualifierAlt)) {
				if (undoBuffer) {
					cursorPos = anchorPos = currentLen[index] = undoLen;
					memcpy(fieldStrings[index], undoBuffer, currentLen[index] + 1);
					notify(gEventAltValue, 0);  // tell app about new value
				}
			} else {
				//  Insert text, if it will fit

				if (insertText((char *)&key, 1) == false)
					return false;
				notify(gEventAltValue, 0);       // tell app about new value
			}
			break;

		case Common::ASCII_BACKSPACE:
			if (selWidth == 0) {                // if insertion point
				if (selStart < 1) return false; // if at start, do nothing
				selStart--;                     // if I-bar, backup 1 char
				selWidth = 1;                   // delete 1 char
			}

			//  Delete N chars
			memmove(fieldStrings[index] + selStart,
					fieldStrings[index] + selStart + selWidth,
					currentLen[index] - (selStart + selWidth));
			cursorPos = anchorPos = selStart;   // adjust cursor pos
			currentLen[index] -= selWidth;                // adjust str len
			notify(gEventAltValue, 0);       // tell app about new value
			break;

		case Common::KEYCODE_DELETE:
			if (selWidth == 0) {                // if insertion point
				// don't delete if at end
				if (selStart >= currentLen[index]) return false;
				selWidth = 1;                   // delete 1 char
			}

			//  Delete N chars
			memmove(fieldStrings[index] + selStart,
					fieldStrings[index] + selStart + selWidth,
					currentLen[index] - (selStart + selWidth));
			cursorPos = anchorPos = selStart;   // adjust cursor pos
			currentLen[index] -= selWidth;    // adjust str len
			notify(gEventAltValue, 0);       // tell app about new value
			break;

		case Common::ASCII_TAB:
			return false;

		default:
			if (flags & textBoxNoFilter)
				return false;

			if (key >= Common::KEYCODE_SPACE &&     // 32 (First printable character)
				key <= Common::KEYCODE_KP_EQUALS && // 272 (Last printable character)
				key != Common::KEYCODE_DELETE) {
				//  Insert text, if it will fit

				if (insertText((char *)&key, 1) == false)
					return false;
				notify(gEventAltValue, 0);       // tell app about new value
			}

			break;
		}
	}

	if (editing) {
		fieldStrings[index][currentLen[index]] = '\0';

		//  Now, redraw the contents.

		SAVE_GPORT_STATE(port);                  // save pen color, etc.
		g_vm->_pointer->hide(port, _extent);              // hide mouse pointer

		drawContents();                         // draw the string

		g_vm->_pointer->show(port, _extent);              // show mouse pointer

		return true;
	}
	return false;

}


//-----------------------------------------------------------------------
// PUBLIC INTERFACE

//-----------------------------------------------------------------------

bool gTextBox::tabSelect() {
	return true;
}

//-----------------------------------------------------------------------

char *gTextBox::getLine(int8 stringIndex) {
	// return the save name
	return fieldStrings[stringIndex];
}

//-----------------------------------------------------------------------

char *gTextBox::selectedText(int &length) {
	length = ABS(cursorPos - anchorPos);
	return fieldStrings[index] + MIN(cursorPos, anchorPos);
}

//-----------------------------------------------------------------------
// DRAWING ROUTINES

void gTextBox::timerTick(gPanelMessage &msg) {
	handleTimerTick(gameTime);
}

//-----------------------------------------------------------------------
void gTextBox::handleTimerTick(int32 tick) {
	if (selected && !displayOnly && editing && !inDrag) {
		if (blinkStart == 0) {
			blinkState = 0;
			blinkStart = tick;
			return;
		}
		if (tick - blinkStart > blinkTime) {
			gPort   &port = window.windowPort;
			SAVE_GPORT_STATE(port);                  // save pen color, etc.
			g_vm->_pointer->hide(port, _extent);              // hide mouse pointer

			port.setPenMap(port.penMap);
			port.setStyle(0);
			port.setColor(blinkState ? blinkColor0 : blinkColor1);
			port.fillRect(editRect.x + blinkX - ((blinkWide + 1) / 2), editRect.y + 1, blinkWide, editRect.height - 1);

			g_vm->_pointer->show(port, _extent);              // show mouse pointer

			blinkState = !blinkState;
			blinkStart = tick;
		}
	}
}

//-----------------------------------------------------------------------

void gTextBox::editRectFill(gPort &fillPort, gPen *pen) {
	fillPort.setPenMap(pen);
	fillPort.setStyle(0);
	fillPort.setColor(fontColorBackHilite);
	fillPort.fillRect(0, 0, editRect.width, editRect.height);
}


//-----------------------------------------------------------------------

void gTextBox::drawContents() {
	int16 cPos, aPos;
	assert(textFont);
	assert(fontColorBack != -1);

	gPort           &port = window.windowPort,
	                 tPort;


	cPos = MIN(cursorPos, anchorPos);
	aPos = MAX(cursorPos, anchorPos);

	//  Allocate a temporary pixel map and render into it.
	if (NewTempPort(tPort, editRect.width, editRect.height)) {
		int16       cursorX,
		            anchorX = 0,
		            hiliteX,
		            hiliteWidth,
		            textHeight_;


		textHeight_ = fontHeight;


		if (hilit || editing) {
			// fill in the editing field's background color
			editRectFill(tPort, port.penMap);
		}

		if (selected) {                      // if panel is selected
			//  Determine the pixel position of the cursor and
			//  anchor positions.

			if (!displayOnly) {
				if (cPos == aPos) {
					//  If it's an insertion point, then make the cursor
					//  1 pixel wide. (And blink it...)
					cursorX = TextWidth(textFont, fieldStrings[index], cPos, 0);
					anchorX = cursorX + 1;
				} else {
					if (cPos == 0) {
						cursorX = 0;
					} else {
						cursorX = TextWidth(textFont, fieldStrings[index], cPos, 0) + 1;
					}

					if (aPos == 0) {
						anchorX = 0;
					} else {
						anchorX = TextWidth(textFont, fieldStrings[index], aPos, 0) + 1;
					}
				}

				//  Adjust the scrolling of the text string

				if (scrollPixels > cursorX) {
					scrollPixels = cursorX;
				} else if (scrollPixels + (editRect.width - 1) < cursorX) {
					scrollPixels = cursorX - (editRect.width - 1);
				}

				//  Adjust the cursor positions to match the scroll

				cursorX -= scrollPixels;
				anchorX -= scrollPixels;
#ifdef BADINTERFACE
				//  If it's a selection, then check to see if either
				//  end of the selection is after the last character,
				//  in which case, also include the blank space after
				//  the end in the highlight.

				if (cPos != aPos) {
					if (cPos == currentLen[index]) cursorX = editRect.width;
					if (aPos == currentLen[index]) anchorX = editRect.width;
				}
#endif
				hiliteX = MIN(cursorX, anchorX);
				hiliteWidth = MAX(cursorX, anchorX) - hiliteX;

				tPort.setColor(cursorColor);     // draw the highlight
				tPort.fillRect(hiliteX, 0, hiliteWidth, editRect.height);
			}
		}

		// set up the font
		tPort.setFont(textFont);
		tPort.setColor(fontColorHilite);

		tPort.moveTo(-scrollPixels, (editRect.height - textHeight_ + 1) / 2);
		tPort.drawText(fieldStrings[index], currentLen[index]);

		//  Blit the pixelmap to the main screen

		port.setMode(drawModeMatte);
		port.bltPixels(*tPort.map, 0, 0,
		               editRect.x + 1, editRect.y + 1,
		               editRect.width, editRect.height);
		blinkStart = 0;
		blinkX = anchorX;

		DisposeTempPort(tPort);              // dispose of temporary pixelmap
	}
}

//-----------------------------------------------------------------------

void gTextBox::drawClipped() {
	gPort           &port = window.windowPort;
	Rect16          rect = window.getExtent();

#if 0
	if (!inDrag && cursorPos > anchorPos) {
		int16 t = cursorPos;
		cursorPos = anchorPos;
		anchorPos = cursorPos;
	}
#endif

	WriteStatusF(11, "Entry %d[%d] (%d:%d)", index, currentLen[index], cursorPos, anchorPos);

	SAVE_GPORT_STATE(port);                  // save pen color, etc.
	g_vm->_pointer->hide(port, _extent);              // hide mouse pointer

	if (fullRedraw) {
		drawAll(port, Point16(0, 0), Rect16(0, 0, rect.width, rect.height));
		fullRedraw = false;
	}

	if (editing) {
		drawContents();                         // draw the string
		drawTitle(textPosLeft);                  // draw the title
	} else if (displayOnly && hilit) {
		drawContents();
	} else {
		drawAll(port, Point16(0, 0), Rect16(0, 0, rect.width, rect.height));
	}

	g_vm->_pointer->show(port, _extent);              // show mouse pointer

}


//-----------------------------------------------------------------------

void gTextBox::drawAll(gPort &port,
                       const Point16 &offset,
                       const Rect16 &) {
	assert(textFont);

	gPort   tempPort;
	Rect16  bufRect = Rect16(0, 0, 0, 0);
	int16   i;

	bufRect.width   = editBaseRect.width;
	bufRect.height  = editBaseRect.height;

	//  Allocate a temporary pixel map and render into it.
	if (NewTempPort(tempPort, bufRect.width, bufRect.height)) {
		Rect16  workRect;

		workRect = bufRect;
		workRect.x -= offset.x;
		workRect.y -= offset.y;


		if (endLine != oldMark  || fullRedraw) {
			// setup the tempPort
			tempPort.setMode(drawModeMatte);

			// if the text is going to change
			tempPort.setColor(fontColorBack);
			tempPort.fillRect(workRect);

			// draw as glyph
			tempPort.setMode(drawModeMatte);

			// pen color black
			tempPort.setColor(fontColorFore);

			// font
			oldFont = tempPort.font;

			tempPort.setFont(textFont);


			for (i = (endLine - linesPerPage); i < endLine; i++) {
				assert(i >= 0 && i <= numEditLines);

				// move to new text pos
				tempPort.moveTo(workRect.x, workRect.y);

				// pen color black
				tempPort.setColor(((i != index) && exists[i]) ? fontColorFore : textDisable);

				// draw the text
				tempPort.drawText(fieldStrings[i]);

				//increment the position
				workRect.y += fontOffset;
			}


			oldMark = endLine;

			// reset the old font
			tempPort.setFont(oldFont);

			//  Blit the pixelmap to the main screen

			port.setMode(drawModeMatte);

			port.bltPixels(*tempPort.map, 0, 0,
			               _extent.x + 1, _extent.y + 1,
			               bufRect.width, bufRect.height);

		}

		DisposeTempPort(tempPort);           // dispose of temporary pixelmap
	}
}

} // end of namespace Saga2
