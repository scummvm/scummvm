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

#include "common/algorithm.h"
#include "access/bubble_box.h"
#include "access/access.h"

namespace Access {

BubbleBox::BubbleBox(AccessEngine *vm) : Manager(vm) {
	_type = TYPE_2;
	_bounds = Common::Rect(64, 32, 64 + 130, 32 + 122);
	_bubblePtr = nullptr;
	_fieldD = 0;
	_fieldE = 0;
	_fieldF = 0;
	_field10 = 0;
}

void BubbleBox::load(Common::SeekableReadStream *stream) {
	_bubbleTitle.clear();

	byte v;
	while ((v = stream->readByte()) != 0)
		_bubbleTitle += (char)v;

	_bubblePtr = _bubbleTitle.c_str();
}

void BubbleBox::clearBubbles() {
	// Loop through the bubble list to restore the screen areas
	for (uint i = 0; i < _bubbles.size(); ++i) {
		_vm->_screen->_screenYOff = 0;
		Common::Rect r = _bubbles[i];
		r.left -= 2;
		r.right = MIN(r.right, (int16)_vm->_screen->w);

		_vm->_screen->copyBlock(&_vm->_buffer2, r);
	}

	// Clear the list
	_bubbles.clear();
}

void BubbleBox::placeBubble(const Common::String &msg) {
	_vm->_screen->_maxChars = 27;
	placeBubble1(msg);
}

void BubbleBox::placeBubble1(const Common::String &msg) {
	clearBubbles();
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
		_vm->_fonts._printMaxX = _vm->_fonts._font2.stringWidth(_bubblePtr);
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
	int titleWidth = _vm->_fonts._font2.stringWidth(_bubblePtr);
	Font &font2 = _vm->_fonts._font2;
	font2._fontColors[0] = 0;
	font2._fontColors[1] = 3;
	font2._fontColors[2] = 2;
	font2._fontColors[3] = 1;
	font2.drawString(_vm->_screen, _bubblePtr, Common::Point(
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

} // End of namespace Access
