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
 */

#include "m4/dialogs.h"
#include "common/file.h"
#include "common/textconsole.h"

namespace M4 {

static void strToUpper(char *s) {
	while (*s) {
		*s = toupper(*s);
		++s;
	}
}

static void strToLower(char *s) {
	while (*s) {
		*s = tolower(*s);
		++s;
	}
}

const RGB8 DIALOG_PALETTE[8] = {
	{0x80, 0x80, 0x80}, {0x90, 0x90, 0x90}, {0x70, 0x70, 0x70}, {0x9c, 0x9c, 0x9c},
	{0x80, 0x80, 0x80}, {0x90, 0x90, 0x90}, {0xDC, 0xDC, 0xDC}, {0x00, 0x00, 0x00}
};

#define ROR16(v,amt) (((uint16)(v) >> amt) | ((uint16)(v) << (16 - amt)))

const int DIALOG_SPACING = 1;

/**
 * Handles any dialog initialisation
 */
void Dialog::initDialog() {
	incLine();
}

/**
 * Adds a new line to the dialog output
 */
void Dialog::incLine() {
	_lineX = 0;
	_widthX = 0;

	_lines.push_back(*new DialogLine());
	assert(_lines.size() <= 20);
}

/**
 * Writes some text to the dialog output, taking care of word wrapping if the text size
 * exceeds the dialog's width
 */
void Dialog::writeChars(const char *srcLine) {
	char wordStr[80];
	char line[80];
	int lineLen, lineWidth;
	const char *srcP = srcLine;

	while (*srcP) {
		bool wordEndedP = false, newlineP = false;
		char *destP = &wordStr[0];
		Common::set_to(&wordStr[0], &wordStr[80], 0);

		// Try and get the next word
		for (;;) {
			char v = *srcP;
			*destP++ = v;

			if (v == '\0') break;
			if (v == '\n') {
				newlineP = true;
				++srcP;
				--destP;
				break;
			}

			if (v == ' ') {
				// Word separator
				++srcP;
				--destP;
				wordEndedP = true;
			} else {
				// Standard character
				if (!wordEndedP)
					// Still in the initial word
					++srcP;
				else {
					// First character of next word, so time to break
					--destP;
					break;
				}
			}
		}

		if (destP < &wordStr[0])
			destP = &wordStr[0];
		*destP = '\0';

		lineLen = strlen(wordStr);

		strcpy(line, "");
		if (_lineX > 0)
			strcat(line, " ");
		strcat(line, wordStr);

		lineLen = strlen(line);
		lineWidth = _vm->_font->current()->getWidth(line, DIALOG_SPACING);

		if (((_lineX + lineLen) > _widthChars) || ((_widthX + lineWidth) > _dialogWidth)) {
			incLine();
			appendText(wordStr);
		} else {
			appendText(line);
		}

		if (newlineP)
			incLine();
	}
}

/**
 * Appends some text to the current dialog line
 */
void Dialog::appendText(const char *line) {
	_lineX += strlen(line);
	_widthX += _vm->_font->current()->getWidth(line, DIALOG_SPACING);

	strcat(_lines[_lines.size() - 1].data, line);
}

/**
 * Adds a line of text to the dialog lines list
 */
void Dialog::addLine(const char *line, bool underlineP) {
	if ((_widthX > 0) || (_lineX > 0))
		incLine();

	int lineWidth = _vm->_font->current()->getWidth(line, DIALOG_SPACING);
	int lineLen = strlen(line);

	if ((lineWidth > _dialogWidth) || (lineLen >= _widthChars))
		writeChars(line);
	else {
		_lines[_lines.size() - 1].xp = (_dialogWidth - 10 - lineWidth) / 2;
		strcpy(_lines[_lines.size() - 1].data, line);
	}

	if (underlineP)
		_lines[_lines.size() - 1].underline = true;

	incLine();
}

/**
 * Adds a bar separation line to the dialog lines list
 */
void Dialog::addBarLine() {
	if ((_widthX > 0) || (_lineX > 0))
		incLine();

	// Flag the line as being a bar separator
	_lines[_lines.size() - 1].barLine = true;
	incLine();
}

/**
 * Retrieves a specified vocab entry
 */
void Dialog::getVocab(int vocabId, char **line) {
	assert(vocabId > 0);
	const char *vocabStr = _madsVm->globals()->getVocab(vocabId);
	strcpy(*line, vocabStr);

	if (_commandCase)
		strToUpper(*line);
	else
		strToLower(*line);

	// Move the string pointer to after the added string
	while (!**line)
		++*line;
}

bool Dialog::handleNounSuffix(char *destP, int nounNum, const char *srcP) {
	char srcLine[40];

	// The next source character must be a colon in front of the first verb
	if (*srcP != ':')
		return false;

	// Copy the remainder of the line into a temporary buffer to get the seperate verbs
	strcpy(srcLine, ++srcP);
	char *altP = strchr(srcLine, ':');
	if (altP)
		*altP = '\0';

	if (*srcP != '\0') {
		while (*srcP != ':') {
			++srcP;
			if (!*srcP) break;
		}
	}

	if (*srcP != '\0')
		++srcP;

	//
	char var_FC[40];
	char tempLine[40];
	strcpy(var_FC, srcP);
	char *tmpP = &tempLine[0];
	char *tmp2P = tmpP;

	uint16 _vocabIds[2] = {1, 1}; // FIXME/TODO: Proper vocab ids
	getVocab(_vocabIds[nounNum], &tmpP);

	if ((*(tmpP - 1) != 'S') && (*(tmpP - 1) != 's')) {
		// Singular object
		tmpP = &var_FC[0];
	} else if (!strcmp(tempLine, "a ")) {
		// Pontially plural
		char ch = tolower(*tmp2P);

		if (!((ch > 'U') || ((ch != 'A') && (ch != 'E') && (ch != 'I') && (ch != 'O'))))
			strcpy(tempLine, "an ");
	}

	strcpy(destP, tmpP);
	return true;
}

/**
 * Sets up an area within the dialog for textual input
 */
void Dialog::setupInputArea() {
	_askPosition.x = _lineX + 1;
	_askPosition.y = _lines.size();

	incLine();
}

/**
 * Checks whether the start of an extracted command matches a specified given command constant
 */
bool Dialog::matchCommand(const char *s1, const char *s2) {
	bool result = scumm_strnicmp(s1, s2, strlen(s2)) == 0;
	_commandCase = isupper(static_cast<unsigned char>(*s1));
	return result;
}

Dialog::Dialog(MadsM4Engine *vm, const char *msgData, const char *title): View(vm, Common::Rect(0, 0, 0, 0)) {
	assert(msgData);
	_vm->_font->setFont(FONT_INTERFACE_MADS);

	const char *srcP = msgData;
	bool skipLine = false;
	bool initFlag = false;
	bool cmdFlag = false;
	bool crFlag = false;
	bool underline = false;

	_screenType = LAYER_DIALOG;
	_widthChars = 0;
	_dialogIndex = 0;
	_askPosition.x = 0;
	_askPosition.y = 0;
	_lineX = 0;
	_widthX = 0;
	_dialogWidth = 0;
	_commandCase = false;

	char dialogLine[256];
	char cmdText[80];
	char *lineP = &dialogLine[0];
	char *cmdP = NULL;

	while (srcP && *(srcP - 1) != '\0') {
		if ((*srcP == '\n') || (*srcP == '\0')) {
			// Line completed
			*lineP = '\0';
			++srcP;

			if (!initFlag) {
				initDialog();
				initFlag = true;
			}

			if (!skipLine)
				writeChars(dialogLine);
			else {
				addLine(dialogLine, underline);

				if (crFlag)
					incLine();
			}

			// Clear the current line contents
			dialogLine[0] = '\0';
			lineP = &dialogLine[0];
			skipLine = crFlag = underline = false;
			continue;

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

			if (matchCommand(cmdText, "ASK")) {
				setupInputArea();

			} else if (matchCommand(cmdText, "BAR")) {
				// Adds a full-width line instead of normal text
				addBarLine();

			} else if (matchCommand(cmdText, "CENTER")) {
				// Center command
				skipLine = true;

			} else if (matchCommand(cmdText, "CR")) {
				// CR command
				if (skipLine)
					crFlag = true;
				else if (!initFlag) {
					initDialog();
					initFlag = true;
				}

			} else if (matchCommand(cmdText, "NOUN1")) {
				// Noun command 1
				handleNounSuffix(lineP, 1, cmdText + 5);

			} else if (matchCommand(cmdText, "NOUN2")) {
				// Noun command 2
				handleNounSuffix(lineP, 2, cmdText + 5);

			} else if (matchCommand(cmdText, "SENTENCE")) {
				// Sentence command - loads the title into the line buffer
				strcpy(dialogLine, title);
				strToUpper(dialogLine);
				lineP += strlen(dialogLine) + 1;

			} else if (matchCommand(cmdText, "TAB")) {
				// Specifies the X offset for the current line
				_lines[_lines.size() - 1].xp = atoi(cmdText + 3);

			} else if (matchCommand(cmdText, "TITLE")) {
				// Title command - specifies the dialog width in number of characters
				skipLine = true;
				crFlag = true;
				underline = true;

				int id = atoi(cmdText + 5);
				if (id > 0) {
					// Suffix provided - specifies the dialog width in number of chars
					_widthChars = id * 2;
					_dialogWidth = id * (_vm->_font->current()->getMaxWidth() + DIALOG_SPACING) + 10;
				}

			} else if (matchCommand(cmdText, "UNDER")) {
				// Underline command
				underline = true;

			} else if (matchCommand(cmdText, "VERB")) {
				// Verb/vocab retrieval
				int verbId = 1; // TODO: Get correct vocab
				getVocab(verbId, &lineP);


			} else if (matchCommand(cmdText, "INDEX")) {
				// Index command
				_dialogIndex = atoi(cmdText + 5);
			} else {
				error("Unknown dialog command '%s' encountered", cmdText);
			}
		}

		*lineP++ = *srcP;
		if (cmdFlag)
			*cmdP++ = *srcP;
		++srcP;
	}

	draw();
}

Dialog::Dialog(MadsM4Engine *vm, int widthChars): View(vm, Common::Rect(0, 0, 0, 0)) {
	_vm->_font->setFont(FONT_INTERFACE_MADS);
	_widthChars = widthChars * 2;
	_dialogWidth = widthChars * (_vm->_font->current()->getMaxWidth() + DIALOG_SPACING) + 10;
	_screenType = LAYER_DIALOG;
	_lineX = 0;
	_widthX = 0;
	_askPosition.x = 0;
	_askPosition.y = 0;
}

Dialog::~Dialog() {
	_vm->_palette->deleteRange(_palette);
	delete _palette;
}

void Dialog::draw() {
	assert(_widthChars != 0);

	// Set up the palette for this view
	_palette = new RGBList(8, NULL);
	_palette->setRange(0, 8, DIALOG_PALETTE);
	_vm->_palette->addRange(_palette);

	// Calculate bounds
	int dlgWidth = _dialogWidth;
	int dlgHeight = _lines.size() * (_vm->_font->current()->getHeight() + 1) + 10;
	int dialogX = (_vm->_screen->width() - dlgWidth) / 2;
	int dialogY = (_vm->_screen->height() - dlgHeight) / 2;

	// Create the surface for the dialog
	create(dlgWidth, dlgHeight, Graphics::PixelFormat::createFormatCLUT8());
	_coords.left = dialogX;
	_coords.top = dialogY;
	_coords.right = dialogX + dlgWidth + 1;
	_coords.bottom = dialogY + dlgHeight + 1;

	// Set up the dialog
	fillRect(Common::Rect(0, 0, width(), height()), 3);
	setColor(2);
	hLine(1, width() - 1, height() - 2);	// Bottom edge
	hLine(0, width(), height() - 1);
	vLine(width() - 2, 2, height());		// Right edge
	vLine(width() - 1, 1, height());

	// Render dialog interior
	uint16 seed = 0xb78e;
	for (int yp = 2; yp < (height() - 2); ++yp) {
		byte *destP = this->getBasePtr(2, yp);

		for (int xp = 2; xp < (width() - 2); ++xp) {
			// Adjust the random seed
			uint16 v = seed;
			seed += 0x181D;
			v = ROR16(v, 9);
			seed = (seed ^ v) + ROR16(v, 3);

			*destP++ = ((seed & 0x10) != 0) ? 1 : 0;
		}
	}

	// If an ask position is set, create the input area frame
	if (_askPosition.y > 0) {

	}

	// Handle drawing the text contents
	_vm->_font->current()->setColors(7, 7, 7);
	setColor(7);

	for (uint lineCtr = 0, yp = 5; lineCtr < _lines.size(); ++lineCtr, yp += _vm->_font->current()->getHeight() + 1) {

		if (_lines[lineCtr].barLine) {
			// Bar separation line
			hLine(5, width() - 6, ((_vm->_font->current()->getHeight() + 1) >> 1) + yp);
		} else {
			// Standard line
			Common::Point pt(_lines[lineCtr].xp + 5, yp);
			if (_lines[lineCtr].xp & 0x40)
				++pt.y;

			_vm->_font->current()->writeString(this, _lines[lineCtr].data, pt.x, pt.y, 0, DIALOG_SPACING);

			if (_lines[lineCtr].underline)
				// Underline needed
				hLine(pt.x, pt.x + _vm->_font->current()->getWidth(_lines[lineCtr].data, DIALOG_SPACING),
					pt.y + _vm->_font->current()->getHeight());
		}
	}

	// Do final translation of the dialog to game palette
	this->translate(_palette);
}

bool Dialog::onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents) {
	if (_vm->_mouse->getCursorNum() != CURSOR_ARROW)
		_vm->_mouse->setCursorNum(CURSOR_ARROW);

	captureEvents = true;

	if (eventType == MEVENT_LEFT_CLICK) {
		captureEvents = false;
		_vm->_viewManager->deleteView(this);
	}

	return true;
}

void Dialog::display(MadsM4Engine *vm, int widthChars, const char **descEntries) {
	Dialog *dlg = new Dialog(vm, widthChars);

	while (*descEntries != NULL) {
		dlg->incLine();
		dlg->writeChars(*descEntries);

		int lineWidth = vm->_font->current()->getWidth(*descEntries, DIALOG_SPACING);
		dlg->_lines[dlg->_lines.size() - 1].xp = (dlg->_dialogWidth - 10 - lineWidth) / 2;
		++descEntries;
	}

	dlg->_lines[0].underline = true;

	dlg->draw();
	vm->_viewManager->addView(dlg);
	vm->_viewManager->moveToFront(dlg);
}

void Dialog::getValue(MadsM4Engine *vm, const char *title, const char *text, int numChars, int currentValue) {
	int titleLen = strlen(title);
	Dialog *dlg = new Dialog(vm, titleLen + 4);

	dlg->addLine(title, true);
	dlg->writeChars("\n");

	dlg->writeChars(text);
	dlg->setupInputArea();
	dlg->writeChars("\n");

	dlg->draw();
	vm->_viewManager->addView(dlg);
	vm->_viewManager->moveToFront(dlg);

	// TODO: How to wait until the dialog is closed

}

} // End of namespace M4
