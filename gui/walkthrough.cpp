/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"

#include "common/file.h"

#include "graphics/font.h"

#include "gui/walkthrough.h"
#include "gui/ScrollBarWidget.h"

namespace GUI {

extern const Graphics::NewFont g_consolefont;

#define kConsoleCharWidth	(g_consolefont.getMaxCharWidth())
#define kConsoleLineHeight	(g_consolefont.getFontHeight() + 2)



WalkthroughDialog::WalkthroughDialog(float widthPercent, float heightPercent)
	: Dialog(0, 0, 1, 1),
		_initialized(false), _widthPercent(widthPercent), _heightPercent(heightPercent) {
	_gameName[0] = 0;
}

WalkthroughDialog::~WalkthroughDialog() {
	destroy();
}

static int getNextWordLength(byte *src, int maxLength) {
	int l;

	for (l = 0; l < maxLength; l++) {
		byte tmp = src[l];
		if ((tmp >= 0) && (tmp <= ' '))
			break;
	}

	return l;
}

bool WalkthroughDialog::loadWalkthroughText() {
	char filename[260];
	File file;

	sprintf(filename, "%s.wkt", _gameName);
	file.open(filename);
	if (!file.isOpen())
		return false;
	int bufferSize = file.size();
	byte *buffer = (byte *)malloc(bufferSize);
	file.read(buffer, bufferSize);
	file.close();

	_linesArray.clear();

	int currentLinePos = 0;
	byte *currentBuffer = buffer;
	byte *lineBuffer = (byte *)malloc(_lineWidth + 1);
	lineBuffer[0] = 0;

	for (;;) {
		if ((currentBuffer - buffer) >= bufferSize)
			break;
		int wordLength = getNextWordLength(currentBuffer, _lineWidth);
		if (((currentLinePos + wordLength) < _lineWidth) && 
				((*currentBuffer != 0x0a) && (*currentBuffer != 0x0d))) {
			if ((*currentBuffer >= 0) && (*currentBuffer <= ' ')) {
				lineBuffer[currentLinePos++] = ' ';
				currentBuffer++;
			} else {
				memcpy(lineBuffer + currentLinePos, currentBuffer, wordLength);
				currentLinePos += wordLength;
				currentBuffer += wordLength;
				if ((currentLinePos + 1) < _lineWidth) {
					lineBuffer[currentLinePos] = ' ';
					currentLinePos++;
				}
			}
		} else {
			if (*currentBuffer == 0x0d) {
				currentBuffer++;
				if ((*currentBuffer == 0x0a) && ((currentBuffer - buffer) < bufferSize)) {
					currentBuffer++;
				}
			} else if (*currentBuffer == 0x0a) {
				currentBuffer++;
			}
			lineBuffer[currentLinePos] = 0;
			Entry line;
			line.text = String((char *)lineBuffer);
			_linesArray.push_back(line);
			lineBuffer[0] = 0;
			currentLinePos = 0;
		}
	}

	free(buffer);
	free(lineBuffer);

	return true;
}

void WalkthroughDialog::setGameName(const char *gameName) {
	strcpy(_gameName, gameName);
}

void WalkthroughDialog::create() {
	if (_initialized)
		destroy();

	// Setup basic layout/dialog size
	reflowLayout();

	// Add scrollbar
	_scrollBar = new ScrollBarWidget(this, _w - kScrollBarWidth - 1, 0, kScrollBarWidth, _h);
	_scrollBar->setTarget(this);

	_currentPos = 0;
	_scrollLine = _linesPerPage - 1;

	_linesArray.clear();

	loadWalkthroughText();

	_initialized = true;
}

void WalkthroughDialog::destroy() {
	if (!_initialized)
		return;

	_linesArray.clear();

	_initialized = false;
}

void WalkthroughDialog::reflowLayout() {
	// Calculate the real width/height (rounded to char/line multiples)
	_w = (uint16)(_widthPercent * g_system->getOverlayWidth());
	_h = (uint16)((_heightPercent * g_system->getOverlayHeight() - 2) / kConsoleLineHeight);
	_h = _h * kConsoleLineHeight + 2;

	// Calculate depending values
	_lineWidth = (_w - kScrollBarWidth - 2) / kConsoleCharWidth;
	_linesPerPage = (_h - 2) / kConsoleLineHeight;
}

void WalkthroughDialog::open() {
	Dialog::open();
}

void WalkthroughDialog::drawDialog() {
	// Blend over the background
	g_gui.fillRect(_x, _y, _w, _h, g_gui._bgcolor);

	// Draw a border
	g_gui.hLine(_x, _y + _h - 1, _x + _w - 1, g_gui._color);

	// Draw text
	int y = _y + 2;

	for (int line = 0; (line < _linesPerPage) && ((_currentPos + line) < (int)_linesArray.size()); line++) {
		const char *text = _linesArray[line + _currentPos].text.c_str();
		int textLen = strlen(text);
		int x = _x + 1;
		for (int column = 0; (column < _lineWidth) && (column < textLen); column++) {
			byte c = text[column];
			g_gui.drawChar(c, x, y, g_gui._textcolor, &g_consolefont);
			x += kConsoleCharWidth;
		}
		y += kConsoleLineHeight;
	}

	// Draw the scrollbar
	_scrollBar->_numEntries = _linesArray.size();
	_scrollBar->_currentPos = _currentPos;
	_scrollBar->_entriesPerPage = _linesPerPage;
	_scrollBar->recalc();
	_scrollBar->draw();

	// Finally blit it all to the screen
	g_gui.addDirtyRect(_x, _y, _w, _h);
}


void WalkthroughDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kSetPositionCmd:
		_currentPos = _scrollBar->_currentPos;
		drawDialog();
		break;
	default:
		return;
	}
}

void WalkthroughDialog::handleMouseWheel(int x, int y, int direction) {
	_scrollBar->handleMouseWheel(x, y, direction);
}

void WalkthroughDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if ((modifiers == OSystem::KBD_CTRL) && (keycode == 'w')) {
		close();
		return;
	}
	switch (keycode) {
	case 256 + 17:	// up arrow
		_currentPos--;
		break;
	case 256 + 18:	// down arrow
		_currentPos++;
		break;
	case 256 + 22:	// home
		_currentPos = 0;
		break;
	case 256 + 23:	// end
		_currentPos = _linesArray.size() - _linesPerPage;
		break;
	case 256 + 24:	// page up
		_currentPos -= _linesPerPage;
		break;
	case 256 + 25:	// page down
		_currentPos += _linesPerPage;
		break;
	default:
		return;
	}
	if (_currentPos < 0) {
		_currentPos = 0;
	}
	if ((_currentPos + _linesPerPage) >= (int)_linesArray.size()) {
		_currentPos = _linesArray.size() - _linesPerPage;
	}

	drawDialog();
}


} // End of namespace GUI
