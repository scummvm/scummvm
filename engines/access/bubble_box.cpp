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

Box::Box(AccessEngine *vm) : Manager(vm) {
	_edgeSize = 0;
}

void Box::doBox(int item, int box) {
	error("TODO: doBox");
}

/*------------------------------------------------------------------------*/

BubbleBox::BubbleBox(AccessEngine *vm) : Box(vm) {
	_bubblePtr = nullptr;
	_maxChars = 0;
}

void BubbleBox::load(Common::SeekableReadStream *stream) {
	_bubbleTitle.clear();

	byte v;
	while ((v = stream->readByte()) != 0)
		_bubbleTitle += (char)v;

	_bubblePtr = _bubbleTitle.c_str();
}

void BubbleBox::clearBubbles() {
	_bubbles.clear();
}

void BubbleBox::placeBubble() {
	BubbleBox::_maxChars = 27;
	placeBubble1();
}

void BubbleBox::placeBubble1() {
	BubbleBox::clearBubbles();
	_vm->_fonts._charSet._lo = 1;
	_vm->_fonts._charSet._hi = 8;
	_vm->_fonts._charFor._lo = 29;
	_vm->_fonts._charFor._hi = 32;

	calcBubble();

	Common::Rect r = BubbleBox::_bubbles[0];
	r.translate(-2, 0);
	_vm->_screen->saveBlock(r);
	printBubble();
}

void BubbleBox::calcBubble() {
	// Save points
	Common::Point printOrg = _vm->_fonts._printOrg;
	Common::Point printStart = _vm->_fonts._printStart;

	// Figure out maximum width allowed
	if (_edgeSize == 4) {
		_vm->_fonts._printMaxX = 110;
	} else {
		_vm->_fonts._printMaxX = _vm->_fonts._font2.stringWidth(BubbleBox::_bubblePtr);
	}

	// Start of with a rect with the given starting x and y
	Common::Rect bounds(printOrg.x - 2, printOrg.y, printOrg.x - 2, printOrg.y);

	// Loop through getting lines
	Common::String msg(BubbleBox::_bubblePtr);
	Common::String line;
	int width = 0;
	bool lastLine;
	do {
		lastLine = _vm->_fonts._font2.getLine(msg, _vm->_fonts._printMaxX, line, width);
		width = MIN(width, _vm->_fonts._printMaxX);

		_vm->_fonts._printOrg.y += 6;
		_vm->_fonts._printOrg.x = _vm->_fonts._printStart.x;
	} while (!lastLine);

	if (_edgeSize == 4)
		++_vm->_fonts._printOrg.y += 6;

	// Determine the width for the area
	width = (((_vm->_fonts._printMaxX >> 4) + 1) << 4) + 5;
	if (width >= 24)
		width += 20 - ((width - 24) % 20);
	bounds.setWidth(width);

	// Determine the height for area
	int y = _vm->_fonts._printOrg.y + 6;
	if (_edgeSize == 4)
		y += 6;
	int height = y - bounds.top;
	bounds.setHeight(height);

	height -= (_edgeSize == 4) ? 30 : 24;
	if (height >= 0)
		bounds.setHeight(bounds.height() + 13 - (height % 13));

	// Add the new bounds to the bubbles list
	BubbleBox::_bubbles.push_back(bounds);

	// Restore points
	_vm->_fonts._printOrg = printOrg;
	_vm->_fonts._printStart = printStart;
}

void BubbleBox::printBubble() {
	//drawBubble(BubbleBox::_bubbles.size() - 1);
	error("TODO: printBubble");
}

void BubbleBox::drawBubble(int index) {
	_bounds = BubbleBox::_bubbles[index];
	doBox(0, 0);
}

} // End of namespace Access
