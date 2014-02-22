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
#include "mads/graphics.h"
#include "mads/msurface.h"
#include "mads/nebular/dialogs_nebular.h"

namespace MADS {

Dialog::Dialog(MADSEngine *vm): _vm(vm), _savedSurface(nullptr),
		_position(Common::Point(-1, -1)), _width(0), _height(0) {
}

Dialog::~Dialog() {
	delete _savedSurface;
}


void Dialog::save(MSurface *s) {
	_savedSurface = MSurface::init(_width, _height);
	s->copyTo(_savedSurface, 
		Common::Rect(_position.x, _position.y, _position.x + _width, _position.y + _height),
		Common::Point());
}

void Dialog::restore(MSurface *s) {
	_savedSurface->copyTo(s, _position);
	delete _savedSurface;
	_savedSurface = nullptr;
}

/*------------------------------------------------------------------------*/

TextDialog::TextDialog(MADSEngine *vm, const Common::String &fontName, 
		const Common::Point &pos, int maxChars):
		Dialog(vm) {
	_vm = vm;
	_fontName = fontName;
	_position = pos;
	
	_vm->_font->setFont(FONT_INTERFACE);
	_vm->_font->setColors(TEXTDIALOG_FONT, TEXTDIALOG_FONT, TEXTDIALOG_FONT, TEXTDIALOG_FONT);

	_innerWidth = (_vm->_font->maxWidth() + 1) * maxChars;
	_width = _innerWidth + 10;
	_lineSize = maxChars * 2;
	_lineWidth = 0;
	_currentX = 0;
	_numLines = 0;
	Common::fill(&_lineXp[0], &_lineXp[TEXT_DIALOG_MAX_LINES], 0);
	
	Common::copy(&_vm->_palette->_mainPalette[TEXTDIALOG_F8 * 3], 
		&_vm->_palette->_mainPalette[TEXTDIALOG_F8 * 3 + 8 * 3], 
		&_savedPalette[0]);
	Palette::setGradient(_vm->_palette->_mainPalette, TEXTDIALOG_F8, 2, 0x24, 0x20);
	Palette::setGradient(_vm->_palette->_mainPalette, TEXTDIALOG_FA, 2, 0x27, 0x1C);
	Palette::setGradient(_vm->_palette->_mainPalette, TEXTDIALOG_FC, 2, 0x24, 0x20);
	Palette::setGradient(_vm->_palette->_mainPalette, TEXTDIALOG_FE, 1, 0x37, 0x37);

	_vm->_palette->setPalette(_vm->_palette->_mainPalette + (TEXTDIALOG_F8 * 3),
		TEXTDIALOG_F8, 8);
}

TextDialog::~TextDialog() {
}

void TextDialog::addLine(const Common::String &line, bool underline) {
	if (_lineWidth > 0 || _currentX > 0)
		incNumLines();

	int stringWidth = _vm->_font->getWidth(line);
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

	// TODO
}

} // End of namespace MADS
