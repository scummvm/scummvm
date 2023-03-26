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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"
#include "common/tokenizer.h"
#include "common/translation.h"

#include "graphics/font.h"
#include "graphics/fontman.h"

#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/textviewer.h"
#include "gui/widgets/scrollbar.h"

namespace GUI {

#define kDialogWidthPercent 0.8f
#define kDialogHeightPercent 0.8f

#define kPadX 0.05
#define kPadY 0.05

TextViewerDialog::TextViewerDialog(Common::String fname)
	: Dialog(0, 0, 1, 1), _fname(fname) {
	_font = &g_gui.getFont(ThemeEngine::kFontStyleConsole);
	_charWidth = _font->getMaxCharWidth();
	_lineHeight = _font->getFontHeight() + 2;

	// Add scrollbar
	_scrollBar = new ScrollBarWidget(this, 0, 0, 1, 1);
	_scrollBar->setTarget(this);

	// I18N: Close dialog button
	_closeButton = new ButtonWidget(this, 0, 0, 1, 1, _("Close"), Common::U32String(), kCloseCmd);

	_currentPos = 0;
	_scrollLine = _linesPerPage - 1;

	reflowLayout();

	loadFile(fname);
}

TextViewerDialog::~TextViewerDialog() {
	destroy();
}

bool TextViewerDialog::loadFile(Common::String &fname) {
	Common::FSNode file(fname);

	if (!file.exists()) {
		warning("TextViewerDialog::loadFile(): Cannot open file %s", fname.c_str());

		return false;
	}

	Common::SeekableReadStream *stream = file.createReadStream();

	if (!stream) {
		warning("TextViewerDialog::loadFile(): Cannot load file %s", fname.c_str());

		return false;
	}

	warning("TextViewerDialog::loadFile(): File size is: %ld", stream->size());

	_linesArray.clear();

	while (!stream->eos()) {
		Common::String line = stream->readString('\n');

		line.wordWrap(_lineWidth);

		Common::StringTokenizer lines(line, "\n");
		while (!lines.empty())
			_linesArray.push_back(lines.nextToken());
	}

	delete stream;

	return true;
}

void TextViewerDialog::destroy() {
	_linesArray.clear();
}

void TextViewerDialog::reflowLayout() {
	// Calculate the real width/height (rounded to char/line multiples)
	_w = (uint16)(kDialogWidthPercent * g_system->getOverlayWidth());
	_h = (uint16)((kDialogHeightPercent * g_system->getOverlayHeight() - 2) / _lineHeight);
	_h = _h * _lineHeight + 2;

	_x = (g_system->getOverlayWidth() - _w) / 2;
	_y = (g_system->getOverlayHeight() - _h) / 2;

	_padX = _w * kPadX;
	_padY = _h * kPadY;

	int16 bW = g_gui.xmlEval()->getVar("Globals.Button.Width", 0);
	int16 bH = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);
	int16 padR = g_gui.xmlEval()->getVar("Globals.Padding.Right", 5);
	int16 padB = g_gui.xmlEval()->getVar("Globals.Padding.Bottom", 5);
	int16 scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);

	int16 buttonOffset = bH + padB;

	_closeButton->setPos(_w - bW - padR, _h - buttonOffset);
	_closeButton->setSize(bW, bH);

	// Calculate depending values
	_lineWidth = (_w - scrollbarWidth - _padX * 2) / _charWidth;
	_linesPerPage = (_h - _padY * 2 - buttonOffset) / _lineHeight;

	warning("Lines: %d", _linesPerPage);

	_scrollBar->setPos(_w - scrollbarWidth - 1, 0);
	_scrollBar->setSize(scrollbarWidth, _h - buttonOffset);
}

void TextViewerDialog::open() {
	Dialog::open();
}

void TextViewerDialog::drawDialog(DrawLayer layerToDraw) {
	Dialog::drawDialog(layerToDraw);

	setTextDrawableArea(Common::Rect(_x, _y, _x + _w, _y + _h));

	// Draw a border
	//g_gui.hLine(_x, _y + _h - 1, _x + _w - 1, g_gui._color);

	// Draw text
	int y = _y + _padY;

	for (int line = 0; (line < _linesPerPage) && ((_currentPos + line) < (int)_linesArray.size()); line++) {
		int x = _x + _padX;
		const char *text = _linesArray[line + _currentPos].c_str();
		int w = MIN<int>(_lineWidth,  _linesArray[line + _currentPos].size());

		for (int column = 0; column < w; column++) {
			byte c = text[column];
			g_gui.theme()->drawChar(Common::Rect(x, y, x + _charWidth, y + _lineHeight), c, _font);
			x += _charWidth;
		}

		y += _lineHeight;
	}

	// Draw the scrollbar
	_scrollBar->_numEntries = _linesArray.size();
	_scrollBar->_currentPos = _currentPos;
	_scrollBar->_entriesPerPage = _linesPerPage;
	_scrollBar->recalc();
	_scrollBar->draw();
}


void TextViewerDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kSetPositionCmd:
		_currentPos = _scrollBar->_currentPos;

		if (_currentPos < 0) {
			_currentPos = 0;
		}
		if ((_currentPos + _linesPerPage) >= (int)_linesArray.size()) {
			_currentPos = MAX<int32>(_linesArray.size() - _linesPerPage, 0);
		}

		drawDialog(kDrawLayerForeground);
		break;
	case kCloseCmd:
		close();
		break;
	default:
		return;
	}
}

void TextViewerDialog::handleMouseWheel(int x, int y, int direction) {
	_scrollBar->handleMouseWheel(x, y, direction);
}

void TextViewerDialog::handleKeyDown(Common::KeyState state) {
	switch (state.keycode) {
	case Common::KEYCODE_ESCAPE:
		close();
		break;
	case Common::KEYCODE_UP:
		_currentPos--;
		break;
	case Common::KEYCODE_DOWN:
		_currentPos++;
		break;
	case Common::KEYCODE_HOME:
		_currentPos = 0;
		break;
	case Common::KEYCODE_END:
		_currentPos = _linesArray.size() - _linesPerPage;
		break;
	case Common::KEYCODE_PAGEUP:
		_currentPos -= _linesPerPage;
		break;
	case Common::KEYCODE_PAGEDOWN:
		_currentPos += _linesPerPage;
		break;
	default:
		return;
	}

	if (_currentPos < 0) {
		_currentPos = 0;
	}
	if ((_currentPos + _linesPerPage) >= (int)_linesArray.size()) {
		_currentPos = MAX<int32>(_linesArray.size() - _linesPerPage, 0);
	}

	drawDialog(kDrawLayerForeground);
}


} // End of namespace GUI
