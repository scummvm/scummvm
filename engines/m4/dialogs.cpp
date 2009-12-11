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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "m4/dialogs.h"
#include "common/file.h"

namespace M4 {

static void strToUpper(char *s) {
	while (*s) {
		*s = toupper(*s);
		++s;
	}
}

void Dialog::initDialog() {

}

void Dialog::incLine() {
	++_numLines;
	assert(++_numLines < 20);
}

void Dialog::writeChars(const char *line) {

}

void Dialog::addLine(const char *line) {

}

bool Dialog::matchCommand(const char *s1, const char *s2) {
	return strncmp(s1, s2, strlen(s2)) == 0;
}

Dialog::Dialog(M4Engine *vm, const char *msgData): View(vm, Common::Rect(100, 80, 220, 140)) {
	assert(msgData);
	const char *srcP = msgData;
	bool skipLine = false;
	bool initFlag = false;
	bool cmdFlag = false;
	bool crFlag = false;

	_dialogTitleId = 0;
	_numLines = 0;
	_dialogIndex = 0;

	char dialogLine[256];
	char cmdText[80];
	char *lineP = &dialogLine[0];
	char *cmdP = NULL;

	while (*srcP != '\0') {
		if (*srcP == '\n') {
			// Line completed
			*lineP = '\0';
			++srcP;

			if (!initFlag) {
				initDialog();
				incLine();
				initFlag = true;
			} else {
				writeChars(dialogLine);
			}
		} else if (*srcP == '[') {
			// Start of a command sequence
			cmdFlag = true;
			cmdP = &cmdText[0];
			++srcP;
			continue;
		} else if (*srcP == ']') {
			// End of a command sequence
			*cmdP = '\0';
			cmdFlag = false;
			strToUpper(cmdText);

			if (matchCommand(cmdText, "CENTER"))
				// Center command
				skipLine = true;
			else if (matchCommand(cmdText, "TITLE")) {
				// Title command
				int id = atoi(cmdText + 5);
				if (id > 0) 
					_dialogTitleId = id;
			} else if (matchCommand(cmdText, "CR")) {
				// CR command
				if (skipLine)
					crFlag = true;
				else {
					initDialog();
				}
			} else if (matchCommand(cmdText, "ASK")) {
				// doAsk();
			} else if (matchCommand(cmdText, "VERB")) {
				// Verb/vocab retrieval
				/*getVocab(); */
			} else if (matchCommand(cmdText, "INDEX")) {
				// Index command
				_dialogIndex = atoi(cmdText + 5);
			} else if (matchCommand(cmdText, "NOUN")) {
				// Noun command
			} else if (matchCommand(cmdText, "SENTENCE")) {
				// Sentence command
			} else {
				error("Unknown dialog command '%s' encountered", cmdText);
			}
		}

		if (cmdFlag)
			*cmdP++ = *srcP;
		++srcP;
	}

	if (!skipLine)
		incLine();
	draw();
}

void Dialog::draw() {
	// TODO: Implement rendering of dialog correctly
	this->fillRect(Common::Rect(0, 0, width(), height()), 0);
	_vm->_font->setColors(5, 5, 0xff);
	_vm->_font->writeString(this, "This will be a dialog", 10, 10, 0, 0);
}

bool Dialog::onEvent(M4EventType eventType, int param1, int x, int y, bool &captureEvents) {
	if (_vm->_mouse->getCursorNum() != CURSOR_ARROW)
		_vm->_mouse->setCursorNum(CURSOR_ARROW);

	captureEvents = true;

	if (eventType == MEVENT_LEFT_CLICK) {
		captureEvents = false;
		_vm->_viewManager->deleteView(this);
	}

	return true;
}


} // End of namespace M4
