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
	BOXSTARTX = BOXSTARTY = 0;
	BICONSTARTX = BICONSTARTY = 0;
	BOXENDX = BOXENDY = 0;
	BOXPSTARTX = BOXPSTARTY = 0;
	// Unused in AGoE
	for (int i = 0; i < 60; i++)
		_tempListPtr[i] = "";
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
	_vm->_screen->_maxChars = 27;
	placeBubble1(msg);
}

void BubbleBox::placeBubble1(const Common::String &msg) {
	_bubbles.clear();
	_vm->_fonts._charSet._lo = 1;
	_vm->_fonts._charSet._hi = 8;
	_vm->_fonts._charFor._lo = 29;
	_vm->_fonts._charFor._hi = 32;

	calcBubble(msg);

	Common::Rect r = _bubbles[0];
	r.translate(-2, 0);
	_vm->_screen->saveBlock(r);
	printBubble(msg);
}

void BubbleBox::calcBubble(const Common::String &msg) {
	// Save points
	Common::Point printOrg = _vm->_screen->_printOrg;
	Common::Point printStart = _vm->_screen->_printStart;

	// Figure out maximum width allowed
	if (_type == TYPE_4) {
		_vm->_fonts._printMaxX = 110;
	} else {
		_vm->_fonts._printMaxX = _vm->_fonts._font2.stringWidth(_bubbleDisplStr);
	}

	// Start of with a rect with the given starting x and y
	Common::Rect bounds(printOrg.x - 2, printOrg.y - 10, printOrg.x - 2, printOrg.y - 10);

	// Loop through getting lines
	Common::String s = msg;
	Common::String line;
	int width = 0;
	bool lastLine;
	do {
		lastLine = _vm->_fonts._font2.getLine(s, _vm->_screen->_maxChars * 6, line, width);
		_vm->_fonts._printMaxX = MAX(width, _vm->_fonts._printMaxX);

		_vm->_screen->_printOrg.y += 6;
		_vm->_screen->_printOrg.x = _vm->_screen->_printStart.x;
	} while (!lastLine);

	if (_type == TYPE_4)
		++_vm->_screen->_printOrg.y += 6;

	// Determine the width for the area
	width = (((_vm->_fonts._printMaxX >> 4) + 1) << 4) + 5;
	if (width >= 24)
		width += 20 - ((width - 24) % 20);
	bounds.setWidth(width);

	// Determine the height for area
	int y = _vm->_screen->_printOrg.y + 6;
	if (_type == TYPE_4)
		y += 6;
	int height = y - bounds.top;
	bounds.setHeight(height);

	height -= (_type == TYPE_4) ? 30 : 24;
	if (height >= 0)
		bounds.setHeight(bounds.height() + 13 - (height % 13));

	// Add the new bounds to the bubbles list
	_bubbles.push_back(bounds);

	// Restore points
	_vm->_screen->_printOrg = printOrg;
	_vm->_screen->_printStart = printStart;
}

void BubbleBox::printBubble(const Common::String &msg) {
	drawBubble(_bubbles.size() - 1);

	// Loop through drawing the lines
	Common::String s = msg;
	Common::String line;
	int width = 0;
	bool lastLine;
	do {
		// Get next line
		Font &font2 = _vm->_fonts._font2;
		lastLine = font2.getLine(s, _vm->_screen->_maxChars * 6, line, width);

		// Set font colors
		font2._fontColors[0] = 0;
		font2._fontColors[1] = 27;
		font2._fontColors[2] = 28;
		font2._fontColors[3] = 29;

		int xp = _vm->_screen->_printOrg.x;
		if (_type == TYPE_4)
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
	doBox(0, 0);
}

void BubbleBox::doBox(int item, int box) {
	FontManager &fonts = _vm->_fonts;
	ASurface &screen = *_vm->_screen;

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

	if (_type == TYPE_4) {
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

	int h = _bounds.height() - (_type == TYPE_4 ? 30 : 24);
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
	yp = screen._orgY2 - (_type == TYPE_4 ? 18 : 12);
	screen.plotImage(icons, (_type == TYPE_4) ? 72 : 22,
		Common::Point(screen._orgX1, yp));
	xp = screen._orgX1 + 12;
	yp += (_type == TYPE_4) ? 4 : 8;

	for (int x = 0; x < xSize; ++x, xp += 20) {
		screen.plotImage(icons, (_type == TYPE_4 ? 62 : 34) + x,
			Common::Point(xp, yp));
	}

	yp = screen._orgY2 - (_type == TYPE_4 ? 18 : 12);
	screen.plotImage(icons, (_type == TYPE_4) ? 73 : 23, Common::Point(xp, yp));

	if (_type == TYPE_4) {
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
	int titleWidth = _vm->_fonts._font2.stringWidth(_bubbleDisplStr);
	Font &font2 = _vm->_fonts._font2;
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

void BubbleBox::displayBoxData() {
	_vm->BOXDATAEND = 0;
	_rowOff = 2;
	_vm->_fonts._charSet._lo = 7;  // 0xF7
	_vm->_fonts._charSet._hi = 15;
	_vm->_fonts._charFor._lo = 15; // 0xFF
	_vm->_fonts._charFor._hi = 15;

	if (_tempListPtr[0].size() == 0)
		return;

	int idx = 0;
	if ((_type == TYPE_1) || (_type == TYPE_3)) {
		_vm->BCNT = 0;

		if (_tempListPtr[idx].size() == 0) {
			_vm->BOXDATAEND = 1;
			return;
		}

		_vm->_events->hideCursor();

		_vm->_screen->_orgX1 = BOXSTARTX;
		_vm->_screen->_orgX2 = BOXENDX;
		_vm->_screen->_orgY1 = BOXSTARTY;
		_vm->_screen->_orgY2 = BOXENDY;
		_vm->_screen->_lColor = 0xFA;
		_vm->_screen->drawRect();
		_vm->_events->showCursor();
	}
	
	_vm->_events->hideCursor();
	int oldPStartY = BOXPSTARTY;
	++BOXPSTARTY;

	for (int i = 0; i < _vm->BOXDATASTART; i++, idx++) {
		while (_tempListPtr[idx].size() != 0)
			++idx;
	}

	while (true) {
		warning("TODO: SETCURSOR");
		warning("TODO: PRINTSTR");
		++idx;
		++BOXPSTARTY;
		++_vm->BCNT;
		if (_tempListPtr[idx].size() == 0) {
			BOXPSTARTY = oldPStartY;
			_vm->_events->showCursor();
			_vm->BOXDATAEND = 1;
			return;
		}

		if (_vm->BCNT == _vm->NUMBLINES) {
			BOXPSTARTY = oldPStartY;
			_vm->_events->showCursor();
			return;
		}
	}
}

void BubbleBox::drawSelectBox() {
	warning("TODO drawSelectBox");
}

int BubbleBox::doBox_v1(int item, int box, int &type) {
	FontManager &fonts = _vm->_fonts;

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
	_vm->BCNT = (_vm->_screen->_orgX2 - _vm->_screen->_orgX1) >> 4;
	int oldX = _vm->_screen->_orgX1;
	for ( ;_vm->BCNT > 0; --_vm->BCNT) {
		_vm->_screen->plotImage(icons, 16, Common::Point(_vm->_screen->_orgX1, _vm->_screen->_orgY1));
		_vm->_screen->_orgX1 += 16;
	}

	_vm->_screen->_orgX1 = oldX;
	int oldY = _vm->_screen->_orgY2;
	_vm->_screen->_orgY2 = _vm->_screen->_orgY1 + 8;
	_vm->_screen->_lColor = 0xF9;

	BOXSTARTY = _vm->_screen->_orgY2 + 1;
	_vm->_screen->_orgY2 = oldY;

	if (_type != TYPE_2) {
		oldY = _vm->_screen->_orgY1;
		--_vm->_screen->_orgY2;
		_vm->_screen->_orgY1 = _vm->_screen->_orgY2 - 8;
		if (_type == TYPE_3)
			_vm->_screen->_orgY1 -= 8;
		_vm->_screen->drawRect();

		int tmpX = BICONSTARTX = _vm->_screen->_orgX1;
		BOXSTARTX = tmpX + 1;
		int tmpY = BOXENDY = _vm->_screen->_orgY1;

		if (_type == TYPE_3)
			BICONSTARTY = tmpY - 7;
		else
			BICONSTARTY = tmpY + 1;

		if (_type == TYPE_3)
			warning("TODO: Implement more of TYPE_3");

		_vm->_screen->_orgY1 = oldY;
	}

	if ((_type == TYPE_0) || (_type == TYPE_3))
		warning("TODO: Implement more of TYPE_0 or TYPE_3");

	int len = _bubbleDisplStr.size();
	int ax = _bounds.top >> 3;
	ax -= len;
	ax /= 2;
	int cx = _bounds.left >> 3;
	BOXPSTARTX = cx;
	ax += cx << 16;

	cx = _bounds.right >> 3;
	int bp = _bounds.right - (cx << 3) + 1;
	if (bp == 8) {
		++cx;
		bp = 0;
	}

	_rowOff = bp;
	BOXPSTARTY = cx;
	ax += cx;

	_vm->_fonts._charFor._lo = -1;
	_vm->_events->setCursor(CURSOR_ARROW);

	_vm->_fonts._font1.drawString(_vm->_screen, _bubbleDisplStr, _vm->_screen->_printOrg);

	if (_type == TYPE_2) {
		_vm->_events->showCursor();
		warning("TODO: pop values");
		_vm->_screen->restoreScreen();
	}

	_vm->_destIn = _vm->_screen;

	// Draw buttons
	int ICON1T = 0;
	int ICON1X = 0;
	int ICON1Y = 0;
	if (_btnId1) {
		ICON1T = _btnId1;
		ICON1X = BICONSTARTX + _btnX1;
		ICON1Y = BICONSTARTY;
		_vm->_screen->plotImage(icons, ICON1T + 10, Common::Point(ICON1X, ICON1Y));

		int ICON2T = 0;
		int ICON2X = 0;
		int ICON2Y = 0;
		if (_btnId2) {
			ICON2T = _btnId2;
			ICON2X = BICONSTARTX + _btnX2;
			ICON2Y = BICONSTARTY;
			_vm->_screen->plotImage(icons, ICON2T + 10, Common::Point(ICON2X, ICON2Y));

			int ICON3T = 0;
			int ICON3X = 0;
			int ICON3Y = 0;
			if (_btnId3) {
				ICON3T = _btnId3;
				ICON3X = BICONSTARTX + _btnX3;
				ICON3Y = BICONSTARTY;
				_vm->_screen->plotImage(icons, ICON3T + 10, Common::Point(ICON3X, ICON3Y));
			}
		}
	}
	
	_vm->_screen->restoreScreen();
	_vm->BOXDATASTART = _startItem;
	_vm->BOXSELECTYOLD = -1;
	_vm->BOXSELECTY = _startBox;

	_vm->NUMBLINES = (_bounds.bottom >> 3) - 2;
	if (_type == TYPE_3)
		--_vm->NUMBLINES;

	_vm->_events->showCursor();
	displayBoxData();
	drawSelectBox();

	while (true) {
		_vm->_events->pollEvents();
		if (_vm->_events->_leftButton)
			continue;

		if ((_type != TYPE_1) && (_vm->_timers[2]._flag == 0)) {
			++_vm->_timers[2]._flag;
			if ((_vm->_events->_mousePos.x >= _vm->_word234F3) && (_vm->_events->_mousePos.x <= _vm->_word234F7)
			&& (_vm->_events->_mousePos.y >= _vm->_word234F5) && (_vm->_events->_mousePos.y < _vm->_word234F9)) {
				if (_vm->BCNT) {
					if (_vm->BOXSELECTY != 0) {
						--_vm->BOXSELECTY;
						drawSelectBox();
					} else if (_vm->BOXDATASTART != 0) {
						--_vm->BOXDATASTART;
						displayBoxData();
						drawSelectBox();
					}
				}
				continue;
			} else if ((_vm->_events->_mousePos.x >= _vm->_word234FB) && (_vm->_events->_mousePos.x <= _vm->_word234FF)
			&& (_vm->_events->_mousePos.y >= _vm->_word234FD) && (_vm->_events->_mousePos.y < _vm->_word23501)) {
				if (_vm->BCNT) {
					if (_vm->BCNT == _vm->NUMBLINES) {
						if (_vm->BCNT != _vm->BOXSELECTY + 1) {
							++_vm->BOXSELECTY;
							drawSelectBox();
						} else if (_vm->BOXDATAEND == 0) {
							++_vm->BOXDATASTART;
							displayBoxData();
							drawSelectBox();
						}
					} else if (_vm->BCNT != _vm->BOXSELECTY + 1) {
						++_vm->BOXSELECTY;
						drawSelectBox();
					}
				}
				continue;
			}
		}
		warning("TODO Case 1");

		displayBoxData();
		drawSelectBox();
	}

	warning("TODO: more dobox_v1");
	return -1;
}

void BubbleBox::getList(const char *data[], int *flags) {
	int srcIdx = 0;
	int destIdx = 0;
	while (data[srcIdx]) {
		if (flags[srcIdx]) {
			_tempListPtr[destIdx] = Common::String(data[srcIdx]);
			++destIdx;
		}
		srcIdx++;
	}
	_tempListPtr[destIdx] = "";
}
} // End of namespace Access
