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

namespace Nebular {

TextDialog::TextDialog(MADSEngine *vm, const Common::String &fontName, 
		const Common::Point &pos, int maxChars) {
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

bool CopyProtectionDialog::show(MADSEngine *vm) {
	CopyProtectionDialog *dlg = new CopyProtectionDialog(vm, false);

	delete dlg;
	return true;
}

CopyProtectionDialog::CopyProtectionDialog(MADSEngine *vm, bool priorAnswerWrong):
		TextDialog(vm, FONT_INTERFACE_MADS, Common::Point(-1, -1), 32) {
	getHogAnusEntry(_hogEntry);

	if (priorAnswerWrong) {
		addLine("ANSWER INCORRECT!", true);
		wordWrap("\n");
		addLine("(But we'll give you another chance!)");
	} else {
		addLine("REX NEBULAR version 8.43", true);
		wordWrap("\n");
		addLine("(Copy Protection, for your convenience)");
	}
	wordWrap("\n");

	wordWrap("Now comes the part that everybody hates.  But if we don't");
	wordWrap("do this, nasty rodent-like people will pirate this game");
	wordWrap("and a whole generation of talented designers, programmers,");
	wordWrap("artists, and playtesters will go hungry, and will wander");
	wordWrap("aimlessly through the land at night searching for peace.");
	wordWrap("So let's grit our teeth and get it over with.  Just get");
	
	Common::String line = "out your copy of ";
	line += _hogEntry._bookId == 103 ? "the GAME MANUAL" : "REX'S LOGBOOK";
	line += ".  See!  That was easy.  ";
	wordWrap(line);

	line = Common::String::format("Next, just turn to page %d. On line %d, find word number %d, ",
		_hogEntry._pageNum, _hogEntry._lineNum, _hogEntry._wordNum);
	wordWrap(line);

	wordWrap("and type it on the line below (we',27h,'ve even given you");
	wordWrap("first letter as a hint).  As soon as you do that, we can get");
	wordWrap("right into this really COOL adventure game!\n");
	wordWrap("\n");
	wordWrap("                    ");
	wordWrap("\n");

}

bool CopyProtectionDialog::getHogAnusEntry(HOGANUS &entry) {
	File f;
	f.open("*HOGANUS.DAT");

	// Read in the total number of entries, and randomly pick an entry to use
	int numEntries = f.readUint16LE();
	int entryIndex = _vm->getRandomNumber(numEntries - 2) + 1;

	// Read in the encrypted entry
	f.seek(28 * entryIndex + 2);
	byte entryData[28];
	f.read(entryData, 28);

	// Decrypt it
	for (int i = 0; i < 28; ++i)
		entryData[i] = ~entryData[i];

	// Fill out the fields
	entry._bookId = entryData[0];
	entry._pageNum = READ_LE_UINT16(&entryData[2]);
	entry._lineNum = READ_LE_UINT16(&entryData[4]);
	entry._wordNum = READ_LE_UINT16(&entryData[6]);
	entry._word = Common::String((char *)&entryData[8]);

	f.close();
	return true;
}


} // End of namespace Nebular

} // End of namespace MADS
