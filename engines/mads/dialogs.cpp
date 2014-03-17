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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "mads/mads.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/nebular/dialogs_nebular.h"

namespace MADS {

Dialog::Dialog(MADSEngine *vm): _vm(vm), _savedSurface(nullptr),
		_position(Common::Point(-1, -1)), _width(0), _height(0) {
}

Dialog::~Dialog() {
	restore(&_vm->_screen);
}


void Dialog::save(MSurface *s) {
	_savedSurface = new MSurface(_width, _height);
	s->copyTo(_savedSurface, 
		Common::Rect(_position.x, _position.y, _position.x + _width, _position.y + _height),
		Common::Point());
}

void Dialog::restore(MSurface *s) {
	if (_savedSurface) {
		_savedSurface->copyTo(s, _position);
		delete _savedSurface;
		_savedSurface = nullptr;
	}
}

void Dialog::draw() {
	// Save the screen portion the dialog will overlap
	save(&_vm->_screen);

	// Draw the dialog
	// Fill entire content of dialog
	_vm->_screen.fillRect(Common::Rect(_position.x, _position.y,
		_position.x + _width, _position.y + _height), TEXTDIALOG_BACKGROUND);

	// Draw the outer edge lines
	_vm->_screen.hLine(_position.x + 1, _position.y + _height - 2,
		_position.x + _width - 2, TEXTDIALOG_EDGE);
	_vm->_screen.hLine(_position.x, _position.y + _height - 1,
		_position.x + _width - 1, TEXTDIALOG_EDGE);
	_vm->_screen.vLine(_position.x + _width - 2, _position.y + 2,
		_position.y + _height - 2, TEXTDIALOG_EDGE);
	_vm->_screen.vLine(_position.x + _width - 1, _position.y + 1,
		_position.y + _height - 1, TEXTDIALOG_EDGE);

	// Draw the gravelly dialog content
	drawContent(Common::Rect(_position.x + 2, _position.y + 2,
		_position.x + _width - 2, _position.y + _height - 2), 0,
		TEXTDIALOG_CONTENT1, TEXTDIALOG_CONTENT2);
}

void Dialog::drawContent(const Common::Rect &r, int seed, byte color1, byte color2) {
	uint16 currSeed = seed ? seed : 0xB78E;

	for (int yp = 0; yp < r.height(); ++yp) {
		byte *destP = _vm->_screen.getBasePtr(r.left, r.top + yp);
		
		for (int xp = 0; xp < r.width(); ++xp) {
			uint16 seedAdjust = currSeed;
			currSeed += 0x181D;
			seedAdjust = (seedAdjust >> 9) | ((seedAdjust & 0x1ff) << 7);
			currSeed ^= seedAdjust;
			seedAdjust = (seedAdjust >> 3) | ((seedAdjust & 7) << 13);
			currSeed += seedAdjust;

			*destP++ = (currSeed & 0x10) ? color2 : color1;
		}
	}
}

/*------------------------------------------------------------------------*/

TextDialog::TextDialog(MADSEngine *vm, const Common::String &fontName, 
		const Common::Point &pos, int maxChars):
		Dialog(vm) {
	_vm = vm;
	_fontName = fontName;
	_position = pos;
	
	_vm->_font->setFont(FONT_INTERFACE);
	_vm->_font->setColors(TEXTDIALOG_BLACK, TEXTDIALOG_BLACK, TEXTDIALOG_BLACK, TEXTDIALOG_BLACK);

	_innerWidth = (_vm->_font->maxWidth() + 1) * maxChars;
	_width = _innerWidth + 10;
	_lineSize = maxChars * 2;
	_lineWidth = 0;
	_currentX = 0;
	_numLines = 0;
	Common::fill(&_lineXp[0], &_lineXp[TEXT_DIALOG_MAX_LINES], 0);
	_askLineNum = -1;
	_askXp = 0;

	// Save the high end of the palette, and set up the entries for dialog display
	Common::copy(&_vm->_palette->_mainPalette[TEXTDIALOG_CONTENT1 * 3], 
		&_vm->_palette->_mainPalette[TEXTDIALOG_CONTENT1 * 3 + 8 * 3], 
		&_savedPalette[0]);
	Palette::setGradient(_vm->_palette->_mainPalette, TEXTDIALOG_CONTENT1, 2, 0x90, 0x80);
	Palette::setGradient(_vm->_palette->_mainPalette, TEXTDIALOG_EDGE, 2, 0x9C, 0x70);
	Palette::setGradient(_vm->_palette->_mainPalette, TEXTDIALOG_FC, 2, 0x90, 0x80);
	Palette::setGradient(_vm->_palette->_mainPalette, TEXTDIALOG_FE, 1, 0xDC, 0xDC);

	_vm->_palette->setPalette(_vm->_palette->_mainPalette + (TEXTDIALOG_CONTENT1 * 3),
		TEXTDIALOG_CONTENT1, 8);
}

TextDialog::~TextDialog() {
	restorePalette();
}

void TextDialog::addLine(const Common::String &line, bool underline) {
	if (_lineWidth > 0 || _currentX > 0)
		incNumLines();

	int stringWidth = _vm->_font->getWidth(line, 1);
	if (stringWidth >= _innerWidth || (int)line.size() >= _lineSize) {
		wordWrap(line);
	} else {
		_lineXp[_numLines] = (_innerWidth / 2) - (stringWidth / 2);
		_lines[_numLines] = line;

		if (underline)
			underlineLine();
	}

	incNumLines();
}

void TextDialog::underlineLine() {
	_lineXp[_numLines] |= 0x80;
}

void TextDialog::incNumLines() {
	_lineWidth = 0;
	_currentX = 0;
	if (++_numLines == TEXT_DIALOG_MAX_LINES)
		error("Exceeded text dialog line max");
}

void TextDialog::wordWrap(const Common::String &line) {
	Common::String tempLine;

	if (!line.empty()) {
		const char *srcP = line.c_str();
		
		do {
			tempLine = "";
			bool endWord = false;
			bool newLine = false;
			bool continueFlag = true;

			do {
				if (!*srcP) {
					continueFlag = false;
				} else {
					tempLine += *srcP;

					if (*srcP == 10) {
						continueFlag = false;
						newLine = true;
						++srcP;
						tempLine.deleteLastChar();
					} else if (*srcP == ' ') {
						++srcP;
						endWord = true;
					} else if (!endWord) {
						++srcP;
					} else {
						tempLine.deleteLastChar();
						continueFlag = false;
					}
				}
			} while (continueFlag);

			if (tempLine.hasSuffix(" "))
				tempLine.deleteLastChar();

			Common::String tempLine2;
			if (_currentX > 0)
				tempLine2 += ' ';
			tempLine2 += tempLine;

			int lineWidth = _vm->_font->getWidth(tempLine2, 1);
			if (((_currentX + (int)tempLine2.size()) > _lineSize) ||
					((_lineWidth + lineWidth) > _innerWidth)) {
				incNumLines();
				appendLine(tempLine);
			} else {
				appendLine(tempLine2);
			}

			if (newLine)
				incNumLines();
		} while (*srcP);
	}
}

void TextDialog::appendLine(const Common::String &line) {
	_currentX += line.size();
	_lineWidth += _vm->_font->getWidth(line, 1);
	_lines[_numLines] += line;
}

void TextDialog::addInput() {
	_askXp = _currentX + 1;
	_askLineNum = _numLines;
	incNumLines();
}

void TextDialog::draw() {
	if (!_lineWidth)
		--_numLines;

	// Figure out the size and position for the dialog
	_height = (_vm->_font->getHeight() + 1) * (_numLines + 1) + 10;
	if (_position.x == -1)
		_position.x = 160 - (_width / 2);
	if (_position.y == -1)
		_position.y = 100 - (_height / 2);

	if ((_position.x + _width) > _vm->_screen.getWidth())
		_position.x = _vm->_screen.getWidth() - (_position.x + _width);
	if ((_position.y + _height) > _vm->_screen.getHeight())
		_position.y = _vm->_screen.getHeight() - (_position.y + _height);

	// Draw the underlying dialog
	Dialog::draw();

	// Draw the text lines
	int lineYp = _position.y + 5; 
	for (int lineNum = 0; lineNum < _numLines; ++lineNum) {
		if (_lineXp[lineNum] == -1) {
			// Draw a line across the entire dialog
			_vm->_screen.hLine(_position.x + 2, 
				lineYp + (_vm->_font->getHeight() + 1)  / 2,
				_position.x + _width - 4, TEXTDIALOG_BLACK);
		} else {
			// Draw a text line
			int xp = (_lineXp[lineNum] & 0x7F) + _position.x + 5;
			int yp = lineYp;
			if (_lineXp[lineNum] & 0x40)
				++yp;

			_vm->_font->writeString(&_vm->_screen, _lines[lineNum], 
				Common::Point(xp, yp), 1);

			if (_lineXp[lineNum] & 0x80) {
				// Draw an underline under the text
				int lineWidth = _vm->_font->getWidth(_lines[lineNum], 1);
				_vm->_screen.hLine(xp, yp + _vm->_font->getHeight(), xp + lineWidth,
					TEXTDIALOG_BLACK);
			}
		}

		lineYp += _vm->_font->getHeight() + 1;
	}
}

void TextDialog::drawWithInput() {
	//int innerWidth = _innerWidth;
	//int lineHeight = _vm->_font->getHeight() + 1;
	//int xp = _position.x + 5;

	// Draw the content of the dialog
	drawContent(Common::Rect(_position.x + 2, _position.y + 2,
		_position.x + _width - 2, _position.y + _height - 2), 0,
		TEXTDIALOG_CONTENT1, TEXTDIALOG_CONTENT2);
	
	error("TODO: drawWithInput");
}

void TextDialog::restorePalette() {
	Common::copy(&_savedPalette[0], &_savedPalette[8 * 3],
		&_vm->_palette->_mainPalette[248 * 3]);
	_vm->_palette->setPalette(_vm->_palette->_mainPalette, 248, 8);
}

/*------------------------------------------------------------------------*/

MessageDialog::MessageDialog(MADSEngine *vm, int maxChars, ...): 
		TextDialog(vm, FONT_INTERFACE, Common::Point(-1, -1), maxChars) {
	// Add in passed line list
	va_list va;
	va_start(va, maxChars);

	const char *line = va_arg(va, const char *);
	while (line) {
		addLine(line);
		line = va_arg(va, const char *);
	}
	va_end(va);
}

void MessageDialog::show() {
	draw();
	_vm->_events->showCursor();

	while (!_vm->shouldQuit() && !_vm->_events->_keyPressed &&
			!_vm->_events->_mouseClicked) {
		_vm->_events->delay(1);
	}
}

/*------------------------------------------------------------------------*/

Dialogs *Dialogs::init(MADSEngine *vm) {
	if (vm->getGameID() == GType_RexNebular)
		return new Nebular::DialogsNebular(vm);

	error("Unknown game");
}

Dialogs::Dialogs(MADSEngine *vm): _vm(vm) {
}

} // End of namespace MADS
