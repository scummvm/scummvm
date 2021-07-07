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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/algorithm.h"
#include "access/bubble_box.h"
#include "access/access.h"

namespace Access {

BubbleBox::BubbleBox(AccessEngine *vm, Access::BoxType type, int x, int y, int w, int h, int val1, int val2, int val3, int val4, Common::String title) : Manager(vm) {
	_type = type;
	_bounds = Common::Rect(x, y, x + w, y + h);
	_bubbleDisplStr = title;
	_btnId1 = val1;
	_btnX1 = val2;
	_btnId2 = val3;
	_btnX2 = val4;
	_btnId3 = _btnX3 = 0; // Unused in MM and Amazon?
	_boxStartX = _boxStartY = 0;
	_bIconStartX = _bIconStartY = 0;
	_boxEndX = _boxEndY = 0;
	_boxPStartX = _boxPStartY = 0;
	// Unused in AGoE
	for (int i = 0; i < 60; i++) {
		_tempList[i] = "";
		_tempListIdx[i] = 0;
	}
	_btnUpPos = Common::Rect(0, 0, 0, 0);
	_btnDownPos = Common::Rect(0, 0, 0, 0);
	_startItem = _startBox = 0;
	_charCol = 0;
	_rowOff = 0;
}

void BubbleBox::load(Common::SeekableReadStream *stream) {
	_bubbleTitle.clear();

	byte v;
	while ((v = stream->readByte()) != 0)
		_bubbleTitle += (char)v;

	_bubbleDisplStr = _bubbleTitle;
}

void BubbleBox::clearBubbles() {
	// Loop through the bubble list to restore the screen areas
	for (uint i = 0; i < _bubbles.size(); ++i) {
		_vm->_screen->_screenYOff = 0;
		Common::Rect r = _bubbles[i];
		r.left -= 2;
		r.right = MIN(r.right, (int16)_vm->_screen->w);

		_vm->_screen->copyBlock(&_vm->_buffer1, r);
	}

	// Clear the list
	_bubbles.clear();
}

void BubbleBox::placeBubble(const Common::String &msg) {
	_vm->_screen->_maxChars = (_vm->getGameID() == GType_MartianMemorandum) ? 30 : 27;
	placeBubble1(msg);
}

void BubbleBox::placeBubble1(const Common::String &msg) {
	_bubbles.clear();
	_vm->_fonts._charSet._lo = 1;
	_vm->_fonts._charSet._hi = 8;
	_vm->_fonts._charFor._lo = (_vm->getGameID() == GType_MartianMemorandum) ? 247 : 29;
	_vm->_fonts._charFor._hi = (_vm->getGameID() == GType_MartianMemorandum) ? 255 : 32;

	calcBubble(msg);

	Common::Rect r = _bubbles[0];
	r.translate(-2, 0);
	_vm->_screen->saveBlock(r);
	printBubble(msg);
}

void BubbleBox::calcBubble(const Common::String &msg) {
	// Save points
	Screen &screen = *_vm->_screen;
	Common::Point printOrg = screen._printOrg;
	Common::Point printStart = screen._printStart;

	// Figure out maximum width allowed
	if (_type == kBoxTypeFileDialog) {
		_vm->_fonts._printMaxX = 110;
	} else {
		_vm->_fonts._printMaxX = _vm->_fonts._font2->stringWidth(_bubbleDisplStr);
	}

	// Start of with a rect with the given starting x and y
	Common::Rect bounds(printOrg.x - 2, printOrg.y - 10, printOrg.x - 2, printOrg.y - 10);

	// Loop through getting lines
	Common::String s = msg;
	Common::String line;
	int width = 0;
	bool lastLine;
	do {
		lastLine = _vm->_fonts._font2->getLine(s, screen._maxChars * 6, line, width);
		_vm->_fonts._printMaxX = MAX(width, _vm->_fonts._printMaxX);

		screen._printOrg.y += 6;
		screen._printOrg.x = screen._printStart.x;
	} while (!lastLine);

	if (_type == kBoxTypeFileDialog)
		++screen._printOrg.y += 6;

	// Determine the width for the area
	width = (((_vm->_fonts._printMaxX >> 4) + 1) << 4) + 5;
	if (width >= 24)
		width += 20 - ((width - 24) % 20);
	bounds.setWidth(width);

	// Determine the height for area
	int y = screen._printOrg.y + 6;
	if (_type == kBoxTypeFileDialog)
		y += 6;
	int height = y - bounds.top;
	bounds.setHeight(height);

	height -= (_type == kBoxTypeFileDialog) ? 30 : 24;
	if (height >= 0)
		bounds.setHeight(bounds.height() + 13 - (height % 13));

	if (bounds.bottom > screen.h)
		bounds.translate(0, screen.h - bounds.bottom);

	// Add the new bounds to the bubbles list
	_bubbles.push_back(bounds);

	// Restore points
	_vm->_screen->_printOrg = printOrg;
	_vm->_screen->_printStart = printStart;
}

void BubbleBox::printBubble(const Common::String &msg) {
	if (_vm->getGameID() == GType_MartianMemorandum)
		printBubble_v1(msg);
	else
		printBubble_v2(msg);
}

void BubbleBox::printBubble_v1(const Common::String &msg) {
	drawBubble(_bubbles.size() - 1);

	// Loop through drawing the lines
	Common::String s = msg;
	Common::String line;
	int width = 0;
	bool lastLine;
	do {
		// Get next line
		Font &font2 = *_vm->_fonts._font2;
		lastLine = font2.getLine(s, _vm->_screen->_maxChars * 6, line, width);
		// Draw the text
		printString(line);

		// Move print position
		_vm->_screen->_printOrg.y += 6;
		_vm->_screen->_printOrg.x = _vm->_screen->_printStart.x;
	} while (!lastLine);

}

void BubbleBox::printBubble_v2(const Common::String &msg) {
	drawBubble(_bubbles.size() - 1);

	// Loop through drawing the lines
	Common::String s = msg;
	Common::String line;
	int width = 0;
	bool lastLine;
	do {
		// Get next line
		Font &font2 = *_vm->_fonts._font2;
		lastLine = font2.getLine(s, _vm->_screen->_maxChars * 6, line, width);

		// Set font colors
		font2._fontColors[0] = 0;
		font2._fontColors[1] = 27;
		font2._fontColors[2] = 28;
		font2._fontColors[3] = 29;

		int xp = _vm->_screen->_printOrg.x;
		if (_type == kBoxTypeFileDialog)
			xp = (_bounds.width() - width) / 2 + _bounds.left - 4;

		// Draw the text
		font2.drawString(_vm->_screen, line, Common::Point(xp, _vm->_screen->_printOrg.y));

		// Move print position
		_vm->_screen->_printOrg.y += 6;
		_vm->_screen->_printOrg.x = _vm->_screen->_printStart.x;
	} while (!lastLine);
}

void BubbleBox::drawBubble(int index) {
	_bounds = _bubbles[index];
	if (_vm->getGameID() == GType_MartianMemorandum) {
		int btnSelected = 0;
		doBox_v1(0, 0, btnSelected);
	} else
		doBox(0, 0);
}

void BubbleBox::doBox(int item, int box) {
	FontManager &fonts = _vm->_fonts;
	Screen &screen = *_vm->_screen;

	_startItem = item;
	_startBox = box;

	// Save state information
	FontVal charSet = fonts._charSet;
	FontVal charFor = fonts._charFor;
	Common::Point printOrg = screen._printOrg;
	Common::Point printStart = screen._printStart;
	int charCol = _charCol;
	int rowOff = _rowOff;

	_vm->_screen->saveScreen();
	_vm->_screen->setDisplayScan();
	fonts._charFor._hi = 0xff;
	fonts._charSet._lo = 1;
	fonts._charSet._hi = 0;

	if (_type == kBoxTypeFileDialog) {
		fonts._charFor._lo = 0xFF;
		error("TODO: filename listing");
		return;
	}

	// Get icons data
	Resource *iconData = _vm->_files->loadFile("ICONS.LZ");
	SpriteResource *icons = new SpriteResource(_vm, iconData);
	delete iconData;

	// Set the up boundaries and color to use for the box background
	_vm->_screen->_orgX1 = _bounds.left - 2;
	_vm->_screen->_orgY1 = _bounds.top;
	_vm->_screen->_orgX2 = _bounds.right - 2;
	_vm->_screen->_orgY2 = _bounds.bottom;
	_vm->_screen->_lColor = 1;

	int h = _bounds.height() - (_type == kBoxTypeFileDialog ? 30 : 24);
	int ySize = (h < 0) ? 0 : (h + 12) / 13;
	int w = _bounds.width() - 24;
	int xSize = (w < 0) ? 0 : (w + 19) / 20;

	// Draw a background for the entire area
	screen.drawRect();

	// Draw images to form the top border
	int xp, yp;
	screen.plotImage(icons, 20, Common::Point(screen._orgX1, screen._orgY1));
	xp = screen._orgX1 + 12;
	for (int x = 0; x < xSize; ++x, xp += 20)
		screen.plotImage(icons, 24 + x, Common::Point(xp, screen._orgY1));
	screen.plotImage(icons, 21, Common::Point(xp, screen._orgY1));

	// Draw images to form the bottom border
	yp = screen._orgY2 - (_type == kBoxTypeFileDialog ? 18 : 12);
	screen.plotImage(icons, (_type == kBoxTypeFileDialog) ? 72 : 22,
		Common::Point(screen._orgX1, yp));
	xp = screen._orgX1 + 12;
	yp += (_type == kBoxTypeFileDialog) ? 4 : 8;

	for (int x = 0; x < xSize; ++x, xp += 20) {
		screen.plotImage(icons, (_type == kBoxTypeFileDialog ? 62 : 34) + x,
			Common::Point(xp, yp));
	}

	yp = screen._orgY2 - (_type == kBoxTypeFileDialog ? 18 : 12);
	screen.plotImage(icons, (_type == kBoxTypeFileDialog) ? 73 : 23, Common::Point(xp, yp));

	if (_type == kBoxTypeFileDialog) {
		// Further stuff for filename dialog
		error("TODO: Box type 4");
	}

	// Draw images to form the sides
	yp = screen._orgY1 + 12;
	for (int y = 0; y < ySize; ++y, yp += 13) {
		screen.plotImage(icons, 44 + y, Common::Point(screen._orgX1, yp));
		screen.plotImage(icons, 53 + y, Common::Point(screen._orgX2 - 4, yp));
	}

	// Handle drawing title
	int titleWidth = _vm->_fonts._font2->stringWidth(_bubbleDisplStr);
	Font &font2 = *_vm->_fonts._font2;
	font2._fontColors[0] = 0;
	font2._fontColors[1] = 3;
	font2._fontColors[2] = 2;
	font2._fontColors[3] = 1;
	font2.drawString(_vm->_screen, _bubbleDisplStr, Common::Point(
		_bounds.left + (_bounds.width() / 2) - (titleWidth / 2), _bounds.top + 1));

	// Restore positional state
	fonts._charSet = charSet;
	fonts._charFor = charFor;
	screen._printOrg = printOrg;
	screen._printStart = printStart;
	_charCol = charCol;
	_rowOff = rowOff;
	_vm->_screen->restoreScreen();

	// Free icons data
	delete icons;
}

void BubbleBox::setCursorPos(int posX, int posY) {
	_vm->_screen->_printStart = _vm->_screen->_printOrg = Common::Point((posX << 3) + _rowOff, posY << 3);
	warning("Missing call to setCursorPos");
}

void BubbleBox::printString(Common::String msg) {
	warning("TODO: Proper implementation of printString");
	_vm->_fonts._font1->drawString(_vm->_screen, msg, _vm->_screen->_printOrg);
}

void BubbleBox::displayBoxData() {
	_vm->_boxDataEnd = false;
	_rowOff = 2;
	_vm->_fonts._charFor._lo = 0xF7;
	_vm->_fonts._charFor._hi = 0xFF;

	if (_tempList[0].size() == 0)
		return;

	int idx = 0;
	if ((_type == TYPE_1) || (_type == TYPE_3)) {
		_vm->_bcnt = 0;

		if (_tempList[idx].size() == 0) {
			_vm->_boxDataEnd = true;
			return;
		}

		_vm->_events->hideCursor();

		_vm->_screen->_orgX1 = _boxStartX;
		_vm->_screen->_orgX2 = _boxEndX;
		_vm->_screen->_orgY1 = _boxStartY;
		_vm->_screen->_orgY2 = _boxEndY;
		_vm->_screen->_lColor = 0xFA;
		_vm->_screen->drawRect();
		_vm->_events->showCursor();
	}

	_vm->_events->hideCursor();
	int oldPStartY = _boxPStartY;
	++_boxPStartY;

	idx += _vm->_boxDataStart;

	while (true) {
		setCursorPos(_boxPStartX, _boxPStartY);
		printString(_tempList[idx]);

		++idx;
		++_boxPStartY;
		++_vm->_bcnt;
		if (_tempList[idx].size() == 0) {
			_boxPStartY = oldPStartY;
			_vm->_events->showCursor();
			_vm->_boxDataEnd = true;
			return;
		}

		if (_vm->_bcnt == _vm->_numLines) {
			_boxPStartY = oldPStartY;
			_vm->_events->showCursor();
			return;
		}
	}
}

void BubbleBox::drawSelectBox() {
	if (_tempList[0].size() == 0)
		return;

	if (((_type != TYPE_1) && (_type != TYPE_3)) || !_vm->_bcnt)
		return;

	if (_vm->_boxSelectYOld != -1) {
		_vm->_events->hideCursor();
		_vm->_screen->_lColor = 0xFA;

		int val = _vm->_boxSelectYOld + _boxPStartY + 1;
		_vm->_screen->_orgY1 = (val << 3) + 2;
		_vm->_screen->_orgY2 = _vm->_screen->_orgY1 + 7;
		_vm->_screen->_orgX1 = _boxStartX;
		_vm->_screen->_orgX2 = _boxEndX;
		_vm->_screen->drawBox();
		_vm->_events->showCursor();
	}

	_vm->_events->hideCursor();
	_vm->_boxSelectYOld = _vm->_boxSelectY;
	int val = _boxPStartY + _vm->_boxSelectY + 1;
	_vm->_screen->_orgY1 = (val << 3) + 2;
	_vm->_screen->_orgY2 = _vm->_screen->_orgY1 + 7;
	_vm->_screen->_orgX1 = _boxStartX;
	_vm->_screen->_orgX2 = _boxEndX;
	_vm->_screen->_lColor = 0xFE;
	_vm->_screen->drawBox();
	_vm->_events->showCursor();

	if (_type == TYPE_3)
		warning("TODO: List filenames");
}

int BubbleBox::doBox_v1(int item, int box, int &btnSelected) {
	static const int ICONW[] = { 0, 11, 28, 19, 19, 15 };

	FontManager &fonts = _vm->_fonts;
	int retval_ = -1;

	_startItem = item;
	_startBox = box;

	// Save state information
	_vm->_screen->saveScreen();
	_vm->_screen->setDisplayScan();

	fonts._charFor._hi = 0xff;
	fonts._charSet._lo = 1;
	fonts._charSet._hi = 0;

	_vm->_destIn = _vm->_screen;	// TODO: Redundant

	if (_type != TYPE_2) {
		Common::Rect r = _bounds;
		r.left -= 2;
		_vm->_screen->saveBlock(r);
	}

	// Set the up boundaries and color to use for the box background
	_vm->_screen->_orgX1 = _bounds.left - 2;
	_vm->_screen->_orgY1 = _bounds.top;
	_vm->_screen->_orgX2 = _bounds.right - 2;
	_vm->_screen->_orgY2 = _bounds.bottom;
	_vm->_screen->_lColor = 0xFB;

	// Draw a background for the entire area
	_vm->_screen->drawRect();

	// Draw the inner box;
	++_vm->_screen->_orgX1;
	++_vm->_screen->_orgY1;
	--_vm->_screen->_orgX2;
	--_vm->_screen->_orgY2;
	_vm->_screen->_lColor = 0xF9;

	// Draw the inner border
	_vm->_screen->drawBox();

	// Get icons data
	Resource *iconData = _vm->_files->loadFile("ICONS.LZ");
	SpriteResource *icons = new SpriteResource(_vm, iconData);
	delete iconData;

	// Draw upper border
	_vm->_bcnt = (_vm->_screen->_orgX2 - _vm->_screen->_orgX1) >> 4;
	int oldX = _vm->_screen->_orgX1;
	for ( ;_vm->_bcnt > 0; --_vm->_bcnt) {
		_vm->_screen->plotImage(icons, 16, Common::Point(_vm->_screen->_orgX1, _vm->_screen->_orgY1));
		_vm->_screen->_orgX1 += 16;
	}

	_vm->_screen->_orgX1 = oldX;
	int oldY = _vm->_screen->_orgY2;
	_vm->_screen->_orgY2 = _vm->_screen->_orgY1 + 8;
	_vm->_screen->_lColor = 0xF9;

	_boxStartY = _vm->_screen->_orgY2 + 1;
	_vm->_screen->_orgY2 = oldY;

	int tmpX = 0;
	int tmpY = 0;
	if (_type != TYPE_2) {
		oldY = _vm->_screen->_orgY1;
		--_vm->_screen->_orgY2;
		_vm->_screen->_orgY1 = _vm->_screen->_orgY2 - 8;
		if (_type == TYPE_3)
			_vm->_screen->_orgY1 -= 8;
		_vm->_screen->drawRect();
		tmpX = _bIconStartX = _vm->_screen->_orgX1;

		_boxStartX = tmpX + 1;
		tmpY = _boxEndY = _vm->_screen->_orgY1;

		if (_type == TYPE_3)
			_bIconStartY = tmpY + 9;
		else
			_bIconStartY = tmpY + 1;

		if (_type == TYPE_3) {
			_fileStart = Common::Point((tmpX + 2) >> 3, (tmpY + 2) >> 3);
			int rowOff = tmpY - (_fileStart.y << 3) + 1;
			if (rowOff == 8) {
				rowOff = 0;
				++_fileStart.y;
			}
			_fileOff.y = _rowOff = rowOff;
			setCursorPos(_fileStart.x, _fileStart.y);
			_vm->_fonts._charFor._lo = 0xF7;
			_vm->_fonts._charFor._hi = 0;
			printString("FILE:           ");
			_vm->_fonts._charFor._hi = 0xFF;
		}
		_vm->_screen->_orgY1 = oldY;
	}

	if ((_type != TYPE_0) && (_type != TYPE_2)) {
		_vm->_screen->_orgY1 += 8;
		if (_type == TYPE_3)
			_vm->_screen->_orgY2 -= 8;

		_vm->_screen->_orgY2 -= 8;
		_btnUpPos.right = _btnDownPos.right = _vm->_screen->_orgX2;
		_btnUpPos.left = _btnDownPos.left = _vm->_screen->_orgX1 = _vm->_screen->_orgX2 - 8;
		_boxEndX = _vm->_screen->_orgX1 - 1;
		_vm->_screen->drawBox();

		_vm->_screen->_orgY1 += 6;
		_vm->_screen->_orgY2 -= 6;
		_vm->_screen->drawBox();

		_btnUpPos.bottom = _vm->_screen->_orgY1 + 1;
		_btnUpPos.top = _btnUpPos.bottom - 5;
		_btnDownPos.top = _vm->_screen->_orgY2 + 1;
		_btnDownPos.bottom = _btnDownPos.top + 6;

		_vm->_screen->_orgX1 += 4;
		_vm->_screen->_orgX2 = _vm->_screen->_orgX1;
		_vm->_screen->_orgY1 -= 4;
		_vm->_screen->_orgY2 += 2;
		_vm->_screen->drawLine();

		++_vm->_screen->_orgY1;
		--_vm->_screen->_orgX1;
		++_vm->_screen->_orgX2;
		_vm->_screen->drawLine();

		++_vm->_screen->_orgY1;
		--_vm->_screen->_orgX1;
		++_vm->_screen->_orgX2;
		_vm->_screen->drawLine();

		_vm->_screen->_orgY1 = _vm->_screen->_orgY2;
		_vm->_screen->drawLine();

		++_vm->_screen->_orgX1;
		--_vm->_screen->_orgX2;
		++_vm->_screen->_orgY1;
		_vm->_screen->drawLine();

		++_vm->_screen->_orgX1;
		--_vm->_screen->_orgX2;
		++_vm->_screen->_orgY1;
		_vm->_screen->drawLine();
	}

	int len = _bubbleDisplStr.size();
	int newX = _bounds.top >> 3;
	newX = (len - newX) / 2;

	_boxPStartX = _bounds.left >> 3;
	newX += _boxPStartX;

	int newY = _bounds.top >> 3;
	int bp = _bounds.top - (newY << 3) + 1;
	if (bp == 8) {
		++newY;
		bp = 0;
	}

	_rowOff = bp;
	retval_ = _boxPStartY = newY;

	setCursorPos(newX, newY);

	_vm->_fonts._charFor._lo = 0xFF;
	_vm->_fonts._font1->drawString(_vm->_screen, _bubbleDisplStr, _vm->_screen->_printOrg);

	if (_type == TYPE_2) {
		_vm->_events->showCursor();
		warning("TODO: pop values");
		_vm->_screen->restoreScreen();
		delete icons;
		return retval_;
	}

	_vm->_destIn = _vm->_screen;

	// Draw buttons
	int ICON1T = 0;
	int ICON1X = 0;
	int ICON1Y = 0;
	int ICON2T = 0;
	int ICON2X = 0;
	int ICON3T = 0;
	int ICON3X = 0;
	if (_btnId1) {
		ICON1T = _btnId1;
		ICON1X = _bIconStartX + _btnX1;
		ICON1Y = _bIconStartY;
		_vm->_screen->plotImage(icons, ICON1T + 10, Common::Point(ICON1X, ICON1Y));

		if (_btnId2) {
			ICON2T = _btnId2;
			ICON2X = _bIconStartX + _btnX2;
			_vm->_screen->plotImage(icons, ICON2T + 10, Common::Point(ICON2X, _bIconStartY));

			if (_btnId3) {
				ICON3T = _btnId3;
				ICON3X = _bIconStartX + _btnX3;
				_vm->_screen->plotImage(icons, ICON3T + 10, Common::Point(ICON3X, _bIconStartY));
			}
		}
	}

	delete icons;

	_vm->_screen->restoreScreen();
	_vm->_boxDataStart = _startItem;
	_vm->_boxSelectYOld = -1;
	_vm->_boxSelectY = _startBox;

	_vm->_numLines = (_bounds.bottom >> 3) - 2;
	if (_type == TYPE_3)
		--_vm->_numLines;

	_vm->_events->showCursor();
	displayBoxData();
	drawSelectBox();

	while (!_vm->shouldQuit()) {
		_vm->_events->pollEvents();
		if (!_vm->_events->_leftButton)
			continue;

		if (((_type == TYPE_1) || (_type != TYPE_3)) && (_vm->_timers[2]._flag == 0)) {
			++_vm->_timers[2]._flag;
			if (_btnUpPos.contains(_vm->_events->_mousePos)) {
				if (_vm->_bcnt) {
					if (_vm->_boxSelectY != 0) {
						--_vm->_boxSelectY;
						drawSelectBox();
					} else if (_vm->_boxDataStart != 0) {
						--_vm->_boxDataStart;
						displayBoxData();
						drawSelectBox();
					}
				}
				continue;
			} else if (_btnDownPos.contains(_vm->_events->_mousePos)) {
				if (_vm->_bcnt) {
					if (_vm->_bcnt == _vm->_numLines) {
						if (_vm->_bcnt != _vm->_boxSelectY + 1) {
							++_vm->_boxSelectY;
							drawSelectBox();
						} else if (!_vm->_boxDataEnd) {
							++_vm->_boxDataStart;
							displayBoxData();
							drawSelectBox();
						}
					} else if (_vm->_bcnt != _vm->_boxSelectY + 1) {
						++_vm->_boxSelectY;
						drawSelectBox();
					}
				}
				continue;
			}
		}

		if ((_vm->_events->_mousePos.x >= _boxStartX) && (_vm->_events->_mousePos.x <= _boxEndX)
		&&  (_vm->_events->_mousePos.y >= _boxStartY) && (_vm->_events->_mousePos.y <= _boxEndY)) {
			int val = (_vm->_events->_mousePos.x >> 3) - _boxPStartY;
			if (val > _vm->_bcnt)
				continue;
			--val;
			if (_type == TYPE_3)
				_vm->_boxSelect = val;
			else {
				btnSelected = 1;
				if (_vm->_boxSelectY == val)
					break;
				_vm->_boxSelectY = val;
				_vm->_events->debounceLeft();
				drawSelectBox();
				continue;
			}
		}

		if ((_vm->_events->_mousePos.y >= ICON1Y) && (_vm->_events->_mousePos.y <= ICON1Y + 8)
		&&  (_vm->_events->_mousePos.x >= ICON1X)) {
			btnSelected = 1;
			if (_vm->_events->_mousePos.x < ICON1X + ICONW[ICON1T])
				break;

			if ((_vm->_events->_mousePos.x >= ICON2X) && (_vm->_events->_mousePos.x < ICON2X + ICONW[ICON2T])) {
				btnSelected = 2;
				break;
			}

			if ((_vm->_events->_mousePos.x >= ICON3X) && (_vm->_events->_mousePos.x < ICON3X + ICONW[ICON3T])) {
				btnSelected = 3;
				break;
			}

			if (_type != TYPE_3)
				continue;

			if ((_vm->_events->_mousePos.x < tmpX) || (_vm->_events->_mousePos.x > tmpX + 144))
				continue;

			if ((_vm->_events->_mousePos.y < tmpY) || (_vm->_events->_mousePos.y > tmpY + 8))
				continue;

			warning("TODO: sub175B5 - List of files");
		}
	}

	_vm->_events->hideCursor();
	_vm->_screen->restoreBlock();
	_vm->_events->showCursor();
	_vm->_events->debounceLeft();
	if (_vm->_bcnt == 0)
		retval_ = -1;
	else
		retval_ = _vm->_boxDataStart + _vm->_boxSelectY;
	return retval_;
}

void BubbleBox::getList(const char *const data[], int *flags) {
	int srcIdx = 0;
	int destIdx = 0;
	while (data[srcIdx]) {
		if (flags[srcIdx]) {
			_tempList[destIdx] = Common::String(data[srcIdx]);
			_tempListIdx[destIdx] = srcIdx;
			++destIdx;
		}
		srcIdx++;
	}
	_tempList[destIdx] = "";
}
} // End of namespace Access
