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


	_hilit                   = false;
	_noUndo                  = false;

	_index                   = 0;    // index into string array ( which string )
	_maxLen                  = length;
	_flags                   = flg;
	_currentLen[_index]     = buffer ? strlen(buffer) : 0;
	_cursorPos               = _anchorPos = _scrollPixels = 0;
	_undoBuffer              = new char[_maxLen + 1]();
	_textFont                = font;
	_oldFont                 = nullptr;
	_fontHeight              = height;
	_fontOffset              = _fontHeight + 2;

	_fontColorFore           = FGColor;
	_fontColorBack           = BGColor;
	_fontColorHilite         = HLColor;
	_fontColorBackHilite     = BGHLColor;
	_cursorColor             = CRColor;
	_linesPerPage            = (box.height / _fontOffset);
	_endLine                 = clamp(0, (_index + _linesPerPage), numEditLines);
	_oldMark                 = -1;

	_displayOnly             = noEditing;
	_editing                 = false;
	_editRect                = box;
	_editRect.height         = _fontHeight;
	_inDrag                  = false;
	_onEnter                 = cmdEnter;
	_onEscape                = cmdEscape;
	_isActiveCtl             = false;
	_selected                = 0;
	_parent                  = &list;

	_blinkStart = 0;
	_blinkX = 0;
	_blinkState = 0;

	// set the filedStrings pointer
	_fieldStrings = stringBufs;

	// get the size of each string
	for (i = 0; i < numEditLines; i++) {
		_exists[i] = ((stringBufs[i][0] & 0x80) == 0);
		stringBufs[i][0] &= 0x7F;
		_currentLen[i] = MIN<int>(editLen, strlen(stringBufs[i]));
	}

	_internalBuffer = false;
	_fullRedraw = true;
	_index = 0;
	enSelect(0);
	if (!_displayOnly) {
		_cursorPos = 0;
		_anchorPos = _currentLen[_index];
	}
	_fullRedraw = true;
}

//-----------------------------------------------------------------------

gTextBox::~gTextBox() {
	deSelect();
	_selected = 0;
	if (_undoBuffer) {
		delete[] _undoBuffer;
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
	int16       selStart = MIN(_cursorPos, _anchorPos),
	            selWidth = ABS(_cursorPos - _anchorPos),
	            selEnd   = selStart + selWidth;

	if (length == -1) length = strlen(newText);

	//  If inserting the text would make the string too long,
	//  then don't insert it.

	if (_currentLen[_index] - selWidth + length >= _maxLen) return false;

	//  Move the text after the selection to where it will be
	//  after the insertion.

	if (selEnd < _currentLen[_index]) {
		memmove(_fieldStrings[_index] + selStart + length,
		        _fieldStrings[_index] + selEnd,
		        _currentLen[_index] - selEnd);
	}

	//  Move the inserted text, if any, to the opening

	if (length > 0) {
		memmove(_fieldStrings[_index] + selStart, newText, length);
	}

	//  Set the insertion point to the end of the new text.

	_cursorPos = _anchorPos = selStart + length;
	_currentLen[_index] += (length - selWidth);
	_fieldStrings[_index][_currentLen[_index]] = '\0';

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
	int16       len = MIN((int)(strlen(newText)), (int)(_maxLen - 1));

	_cursorPos = 0;
	_anchorPos = _currentLen[_index];

	insertText(newText, len);
	_cursorPos = _anchorPos = 0;

	if (_window.isOpen()) drawContents();
}

//-----------------------------------------------------------------------

void gTextBox::setEditExtent(const Rect16 &r) {
	_editRect.x = r.x;
	_editRect.y = r.y;
	_editRect.width = r.width;
	_editRect.height = r.height;
}


//-----------------------------------------------------------------------

bool gTextBox::activate(gEventType why) {
	if (why == gEventAltValue) {            // momentarily depress
		_selected = 1;
		notify(why, 0);                      // notify App of successful hit
		return true;
	}
	_isActiveCtl = true;
	if (!_selected) {
		enSelect(_index);
	}
	_selected = 1;
	_fullRedraw = true;
	draw();
	if (why == gEventNone)
		return true;
	return gPanel::activate(why);
}

//-----------------------------------------------------------------------

void gTextBox::deactivate() {
	_selected = 0;
	_isActiveCtl = false;
	draw();
	_fullRedraw = true;
	gPanel::deactivate();
}

//-----------------------------------------------------------------------

void gTextBox::prepareEdit(int which) {
	if (!_displayOnly) {
		if (_undoBuffer) memcpy(_undoBuffer, _fieldStrings[which], _currentLen[which] + 1);
		_undoLen = _currentLen[which];
	}
}

//-----------------------------------------------------------------------

bool gTextBox::changed() {
	if (_undoBuffer && _editing) {
		return memcmp(_undoBuffer, _fieldStrings[_index], _currentLen[_index] + 1);
	}
	return false;
}

//-----------------------------------------------------------------------

void gTextBox::commitEdit() {
	if (_undoBuffer && changed()) {
		memcpy(_undoBuffer, _fieldStrings[_index], _currentLen[_index] + 1);
		_undoLen = _currentLen[_index];
		_cursorPos = _anchorPos = _currentLen[_index];
		notify(gEventNewValue, 1);       // tell app about new value
	}
}


//-----------------------------------------------------------------------

void gTextBox::revertEdit() {
	if (_undoBuffer && changed()) {
		_cursorPos = _anchorPos = _currentLen[_index] = _undoLen;
		memcpy(_fieldStrings[_index], _undoBuffer, _currentLen[_index] + 1);
		notify(gEventNewValue, 0);         // tell app about new value
	}
}

//-----------------------------------------------------------------------
// LINE SELECTION

//-----------------------------------------------------------------------
// Choose the part of the list to display & the specific Item in that list

void gTextBox::scroll(int8 req) {
	int16   indexReq = req;
	int16   oldIndex = _index;
	int16   visOld = (oldIndex - (_endLine - _linesPerPage));
	int16   visBase = _endLine;
	int16   visIndex;

	indexReq        = clamp(0, indexReq, numEditLines);
	visIndex = (indexReq - (visBase - _linesPerPage));
	if (ABS(oldIndex - indexReq) < 2) {
		if (visIndex < 0) {
			visBase--;
			visIndex++;
		} else if (visIndex >= _linesPerPage) {
			visBase++;
			visIndex--;
		}
	} else {
		while (visIndex >= _linesPerPage) {
			visBase = clamp(_linesPerPage, visBase + _linesPerPage, numEditLines);
			visIndex = (indexReq - (visBase - _linesPerPage));
		}
		while (visIndex < 0) {
			visBase = clamp(_linesPerPage, visBase - _linesPerPage, numEditLines);
			visIndex = (indexReq - (visBase - _linesPerPage));
		}
	}

	if (_endLine != visBase) {
		_fullRedraw = true;
	}
	_endLine = visBase;

	if (visIndex != visOld) {
		Rect16  textBoxExtent   = _editRect;

		// setup the _editing extent
		textBoxExtent.y = (_fontOffset * visIndex)  + _extent.y;

		setEditExtent(textBoxExtent);
		_fullRedraw = true;
	}
}

//-----------------------------------------------------------------------

void gTextBox::deSelect(bool commit) {
	if (_index >= 0 && _editing) {
		if (commit)
			commitEdit();
		else
			revertEdit();
		_editing = false;
		_fullRedraw = true;
	}
}

//-----------------------------------------------------------------------

void gTextBox::enSelect(int which) {
	scroll(which);
	_index = which;
	if (!_displayOnly) {
		prepareEdit(which);
		_editing   = true;
		_cursorPos = 0;
		_anchorPos = _currentLen[_index];
	} else {
		_hilit = true;
	}
}

//-----------------------------------------------------------------------

void gTextBox::reSelect(int which) {
	if (which != _index) {
		deSelect(false);
		draw();
		enSelect(which);
		_fullRedraw = true;
	}
}

//-----------------------------------------------------------------------
// select a nearby line of text

void gTextBox::selectionMove(int howMany) {
	int8    newIndex;

	newIndex = clamp(0, _index + howMany, numEditLines - 1);
#ifndef ALLOW_BAD_LOADS
	if (_displayOnly) {
		int i = newIndex;
		if (howMany > 0) {
			while (!_exists[i] && i < numEditLines - 1) i++;
			if (!_exists[i]) {
				i = newIndex;
				while (!_exists[i] && i > 0) i--;
			}
			if (_exists[i])
				newIndex = i;
		} else {
			while (!_exists[i] && i > 0) i--;
			if (!_exists[i]) {
				i = newIndex;
				while (!_exists[i] && i < numEditLines - 1) i++;
			}
			if (_exists[i])
				newIndex = i;
		}

	}
#endif
	reSelect(newIndex);
	if (!_displayOnly) {
		_cursorPos = 0;
		_anchorPos = _currentLen[_index];
	}

	draw();

	//activate(gEventAltValue);
}


//-----------------------------------------------------------------------

void gTextBox::scrollUp() {
	selectionUp(_linesPerPage - 2);
}

//-----------------------------------------------------------------------

void gTextBox::scrollDown() {
	selectionDown(_linesPerPage - 2);
}

//-----------------------------------------------------------------------
// INPUT HANDLING

//-----------------------------------------------------------------------

bool gTextBox::pointerHit(gPanelMessage &msg) {
	Point16 pos             = msg._pickPos;
	int16 newPos;


	if (Rect16(0, 0, _extent.width, _extent.height).ptInside(pos)) {
		int8    newIndex;
		// get the position of the line
		newIndex = clamp(0, pos.y / _fontOffset, _linesPerPage - 1);
		newIndex = (_endLine - (_linesPerPage - newIndex));

		if (_index != newIndex)
			reSelect(newIndex);
		if (_editing) {
			if (_textFont) {
				newPos = WhichIChar(_textFont, (uint8 *)_fieldStrings[_index], msg._pickPos.x - 3, _currentLen[_index]);
			} else {
				newPos = WhichIChar(mainFont, (uint8 *)_fieldStrings[_index], msg._pickPos.x - 3, _currentLen[_index]);
			}
			if (msg._leftButton) {
				if (_cursorPos != newPos || _anchorPos != newPos) {
					_anchorPos = newPos;
					_cursorPos = newPos;
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

	if (msg._leftButton) {
		if (_textFont) {
			newPos = WhichIChar(_textFont, (uint8 *)_fieldStrings[_index], msg._pickPos.x - 3, _currentLen[_index]);
		} else {
			newPos = WhichIChar(mainFont, (uint8 *)_fieldStrings[_index], msg._pickPos.x - 3, _currentLen[_index]);
		}
		_inDrag = true;
		if (_cursorPos != newPos) {
			//if (newPos<_cursorPos)
			_cursorPos = newPos;
			//else
			//  _anchorPos = newPos ;
		}
		draw();
	}

}

//-----------------------------------------------------------------------

//  Mouse release code

void gTextBox::pointerRelease(gPanelMessage &msg) {
	if (!msg._leftButton) {
		_inDrag = false;
		draw();
	}
}

//-----------------------------------------------------------------------


bool gTextBox::keyStroke(gPanelMessage &msg) {
	gPort &port = _window._windowPort;
	int16 selStart = MIN(_cursorPos, _anchorPos),
	      selWidth = ABS(_cursorPos - _anchorPos);
	uint16 key = msg._key;

	//  Process the various keystrokes...
	if (_editing && _cursorPos > _anchorPos) {
		_cursorPos = _anchorPos;
		_anchorPos = _cursorPos;
	}

	switch (key) {
	case Common::KEYCODE_UP:
		selectionUp(1);
		return true;

	case Common::KEYCODE_DOWN:
		selectionDown(1);
		return true;

	case Common::KEYCODE_PAGEUP:
		selectionUp(_linesPerPage);
		return true;

	case Common::KEYCODE_PAGEDOWN:
		selectionDown(_linesPerPage);
		return true;
	}

	if (key == Common::ASCII_RETURN) { // return key
		if (_editing) {
			commitEdit();
			if (!(_flags & textBoxStayActive))
				deactivate();                       // deactivate the text box
		}

		if (_onEnter != nullptr) {
			gEvent ev;
			ev.eventType = gEventKeyDown ;
			ev.value = 1;
			ev.panel = _parent;
			(*_onEnter)(ev);
		}

		return true;
	} else if (key == Common::ASCII_ESCAPE) {               // escape key
		revertEdit();
		deactivate();                       // deactivate the text box
		if (_onEscape != nullptr) {
			gEvent ev;
			ev.eventType = gEventKeyDown ;
			ev.value = 1;
			ev.value = 1;
			ev.panel = this; //_parent;
			(*_onEscape)(ev);
		}

		if (_flags & textBoxNoFilter)
			return false;
		return true;
	} else if (_editing) {
		switch (key) {
		case Common::KEYCODE_LEFT:
			if (_anchorPos > 0)
				_anchorPos--;
			if (!(msg._qualifier & qualifierShift))
				_cursorPos = _anchorPos;
			break;

		case Common::KEYCODE_RIGHT:
			if (_anchorPos < _currentLen[_index])
				_anchorPos++;
			if (!(msg._qualifier & qualifierShift))
				_cursorPos = _anchorPos;
			break;

		case Common::KEYCODE_HOME:
			_cursorPos = 0;
			_anchorPos = 0;
			break;

		case Common::KEYCODE_END:
			_cursorPos = _currentLen[_index];
			_anchorPos = _currentLen[_index];
			break;

		case Common::KEYCODE_z: // Alt-Z
			if (msg._qualifier & (qualifierControl | qualifierAlt)) {
				if (_undoBuffer) {
					_cursorPos = _anchorPos = _currentLen[_index] = _undoLen;
					memcpy(_fieldStrings[_index], _undoBuffer, _currentLen[_index] + 1);
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
			memmove(_fieldStrings[_index] + selStart,
					_fieldStrings[_index] + selStart + selWidth,
					_currentLen[_index] - (selStart + selWidth));
			_cursorPos = _anchorPos = selStart;   // adjust cursor pos
			_currentLen[_index] -= selWidth;                // adjust str len
			notify(gEventAltValue, 0);       // tell app about new value
			break;

		case Common::KEYCODE_DELETE:
			if (selWidth == 0) {                // if insertion point
				// don't delete if at end
				if (selStart >= _currentLen[_index]) return false;
				selWidth = 1;                   // delete 1 char
			}

			//  Delete N chars
			memmove(_fieldStrings[_index] + selStart,
					_fieldStrings[_index] + selStart + selWidth,
					_currentLen[_index] - (selStart + selWidth));
			_cursorPos = _anchorPos = selStart;   // adjust cursor pos
			_currentLen[_index] -= selWidth;    // adjust str len
			notify(gEventAltValue, 0);       // tell app about new value
			break;

		case Common::ASCII_TAB:
			return false;

		default:
			if (_flags & textBoxNoFilter)
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

	if (_editing) {
		_fieldStrings[_index][_currentLen[_index]] = '\0';

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
	return _fieldStrings[stringIndex];
}

//-----------------------------------------------------------------------

char *gTextBox::selectedText(int &length) {
	length = ABS(_cursorPos - _anchorPos);
	return _fieldStrings[_index] + MIN(_cursorPos, _anchorPos);
}

//-----------------------------------------------------------------------
// DRAWING ROUTINES

void gTextBox::timerTick(gPanelMessage &msg) {
	handleTimerTick(gameTime);
}

//-----------------------------------------------------------------------
void gTextBox::handleTimerTick(int32 tick) {
	if (_selected && !_displayOnly && _editing && !_inDrag) {
		if (_blinkStart == 0) {
			_blinkState = 0;
			_blinkStart = tick;
			return;
		}
		if (tick - _blinkStart > blinkTime) {
			gPort   &port = _window._windowPort;
			SAVE_GPORT_STATE(port);                  // save pen color, etc.
			g_vm->_pointer->hide(port, _extent);              // hide mouse pointer

			port.setPenMap(port._penMap);
			port.setStyle(0);
			port.setColor(_blinkState ? blinkColor0 : blinkColor1);
			port.fillRect(_editRect.x + _blinkX - ((blinkWide + 1) / 2), _editRect.y + 1, blinkWide, _editRect.height - 1);

			g_vm->_pointer->show(port, _extent);              // show mouse pointer

			_blinkState = !_blinkState;
			_blinkStart = tick;
		}
	}
}

//-----------------------------------------------------------------------

void gTextBox::editRectFill(gPort &fillPort, gPen *pen) {
	fillPort.setPenMap(pen);
	fillPort.setStyle(0);
	fillPort.setColor(_fontColorBackHilite);
	fillPort.fillRect(0, 0, _editRect.width, _editRect.height);
}


//-----------------------------------------------------------------------

void gTextBox::drawContents() {
	int16 cPos, aPos;
	assert(_textFont);
	assert(_fontColorBack != -1);

	gPort           &port = _window._windowPort,
	                 tPort;


	cPos = MIN(_cursorPos, _anchorPos);
	aPos = MAX(_cursorPos, _anchorPos);

	//  Allocate a temporary pixel map and render into it.
	if (NewTempPort(tPort, _editRect.width, _editRect.height)) {
		int16       cursorX,
		            anchorX = 0,
		            _hiliteX,
		            _hiliteWidth,
		            textHeight_;


		textHeight_ = _fontHeight;


		if (_hilit || _editing) {
			// fill in the _editing field's background color
			editRectFill(tPort, port._penMap);
		}

		if (_selected) {                      // if panel is selected
			//  Determine the pixel position of the cursor and
			//  anchor positions.

			if (!_displayOnly) {
				if (cPos == aPos) {
					//  If it's an insertion point, then make the cursor
					//  1 pixel wide. (And blink it...)
					cursorX = TextWidth(_textFont, _fieldStrings[_index], cPos, 0);
					anchorX = cursorX + 1;
				} else {
					if (cPos == 0) {
						cursorX = 0;
					} else {
						cursorX = TextWidth(_textFont, _fieldStrings[_index], cPos, 0) + 1;
					}

					if (aPos == 0) {
						anchorX = 0;
					} else {
						anchorX = TextWidth(_textFont, _fieldStrings[_index], aPos, 0) + 1;
					}
				}

				//  Adjust the scrolling of the text string

				if (_scrollPixels > cursorX) {
					_scrollPixels = cursorX;
				} else if (_scrollPixels + (_editRect.width - 1) < cursorX) {
					_scrollPixels = cursorX - (_editRect.width - 1);
				}

				//  Adjust the cursor positions to match the scroll

				cursorX -= _scrollPixels;
				anchorX -= _scrollPixels;
#ifdef BADINTERFACE
				//  If it's a selection, then check to see if either
				//  end of the selection is after the last character,
				//  in which case, also include the blank space after
				//  the end in the highlight.

				if (cPos != aPos) {
					if (cPos == _currentLen[_index]) cursorX = _editRect.width;
					if (aPos == _currentLen[_index]) anchorX = _editRect.width;
				}
#endif
				_hiliteX = MIN(cursorX, anchorX);
				_hiliteWidth = MAX(cursorX, anchorX) - _hiliteX;

				tPort.setColor(_cursorColor);     // draw the highlight
				tPort.fillRect(_hiliteX, 0, _hiliteWidth, _editRect.height);
			}
		}

		// set up the font
		tPort.setFont(_textFont);
		tPort.setColor(_fontColorHilite);

		tPort.moveTo(-_scrollPixels, (_editRect.height - textHeight_ + 1) / 2);
		tPort.drawText(_fieldStrings[_index], _currentLen[_index]);

		//  Blit the pixelmap to the main screen

		port.setMode(drawModeMatte);
		port.bltPixels(*tPort._map, 0, 0,
		               _editRect.x + 1, _editRect.y + 1,
		               _editRect.width, _editRect.height);
		_blinkStart = 0;
		_blinkX = anchorX;

		DisposeTempPort(tPort);              // dispose of temporary pixelmap
	}
}

//-----------------------------------------------------------------------

void gTextBox::drawClipped() {
	gPort           &port = _window._windowPort;
	Rect16          rect = _window.getExtent();

#if 0
	if (!_inDrag && _cursorPos > _anchorPos) {
		int16 t = _cursorPos;
		_cursorPos = _anchorPos;
		_anchorPos = _cursorPos;
	}
#endif

	WriteStatusF(11, "Entry %d[%d] (%d:%d)", index, _currentLen[_index], _cursorPos, _anchorPos);

	SAVE_GPORT_STATE(port);                  // save pen color, etc.
	g_vm->_pointer->hide(port, _extent);              // hide mouse pointer

	if (_fullRedraw) {
		drawAll(port, Point16(0, 0), Rect16(0, 0, rect.width, rect.height));
		_fullRedraw = false;
	}

	if (_editing) {
		drawContents();                         // draw the string
		drawTitle(textPosLeft);                  // draw the title
	} else if (_displayOnly && _hilit) {
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
	assert(_textFont);

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


		if (_endLine != _oldMark  || _fullRedraw) {
			// setup the tempPort
			tempPort.setMode(drawModeMatte);

			// if the text is going to change
			tempPort.setColor(_fontColorBack);
			tempPort.fillRect(workRect);

			// draw as glyph
			tempPort.setMode(drawModeMatte);

			// pen color black
			tempPort.setColor(_fontColorFore);

			// font
			_oldFont = tempPort._font;

			tempPort.setFont(_textFont);


			for (i = (_endLine - _linesPerPage); i < _endLine; i++) {
				assert(i >= 0 && i <= numEditLines);

				// move to new text pos
				tempPort.moveTo(workRect.x, workRect.y);

				// pen color black
				tempPort.setColor(((i != _index) && _exists[i]) ? _fontColorFore : textDisable);

				// draw the text
				tempPort.drawText(_fieldStrings[i]);

				//increment the position
				workRect.y += _fontOffset;
			}


			_oldMark = _endLine;

			// reset the old font
			tempPort.setFont(_oldFont);

			//  Blit the pixelmap to the main screen

			port.setMode(drawModeMatte);

			port.bltPixels(*tempPort._map, 0, 0,
			               _extent.x + 1, _extent.y + 1,
			               bufRect.width, bufRect.height);

		}

		DisposeTempPort(tempPort);           // dispose of temporary pixelmap
	}
}

} // end of namespace Saga2
