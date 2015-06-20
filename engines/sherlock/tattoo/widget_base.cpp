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

#include "sherlock/tattoo/widget_base.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_talk.h"
#include "sherlock/tattoo/tattoo_user_interface.h"

namespace Sherlock {

namespace Tattoo {

WidgetBase::WidgetBase(SherlockEngine *vm) : _vm(vm) {
	_images = nullptr;
}

void WidgetBase::summonWindow() {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	ui._widget = this;
	_outsideMenu = false;
}

void WidgetBase::banishWindow() {
	// TODO
	_surface.free();
}

Common::String WidgetBase::splitLines(const Common::String &str, Common::StringArray &lines, int maxWidth, uint maxLines) {
	Talk &talk = *_vm->_talk;
	const char *strP = str.c_str();

	// Loop counting up lines
	lines.clear();
	while (lines.size() < maxLines) {
		int width = 0;
		const char *spaceP = nullptr;
		const char *lineStartP = strP;

		// Find how many characters will fit on the next line
		while (width < maxWidth && *strP && ((byte)*strP < talk._opcodes[OP_SWITCH_SPEAKER] || 
				(byte)*strP == talk._opcodes[OP_NULL])) {
			width += _surface.charWidth(*strP);

			// Keep track of the last space
			if (*strP == ' ')
				spaceP = strP;
			++strP;
		}

		// If the line was too wide to fit on a single line, go back to the last space 
		// if there was one, or otherwise simply break the line at this point
		if (width >= maxWidth && spaceP != nullptr)
			strP = spaceP;

		// Add the line to the output array
		lines.push_back(Common::String(lineStartP, strP));

		// Move the string ahead to the next line
		if (*strP == ' ' || *strP == 13)
			++strP;
	} while (*strP && ((byte)*strP < talk._opcodes[OP_SWITCH_SPEAKER] || (byte)*strP == talk._opcodes[OP_NULL]));

	// Return any remaining text left over
	return *strP ? Common::String(strP) : Common::String();
}

void WidgetBase::checkMenuPosition() {
	if (_bounds.left < 0)
		_bounds.moveTo(0, _bounds.top);
	if (_bounds.top < 0)
		_bounds.moveTo(_bounds.left, 0);
	if (_bounds.right > SHERLOCK_SCREEN_WIDTH)
		_bounds.moveTo(SHERLOCK_SCREEN_WIDTH - _bounds.width(), _bounds.top);
	if (_bounds.bottom > SHERLOCK_SCREEN_HEIGHT)
		_bounds.moveTo(_bounds.left, SHERLOCK_SCREEN_HEIGHT - _bounds.height());
}

void WidgetBase::makeInfoArea() {
	// Draw the four corners of the Info Box
	_surface.transBlitFrom((*_images)[0], Common::Point(0, 0));
	_surface.transBlitFrom((*_images)[1], Common::Point(_bounds.width() - (*_images)[1]._width, 0));
	_surface.transBlitFrom((*_images)[2], Common::Point(0, _bounds.height() - (*_images)[2]._height));
	_surface.transBlitFrom((*_images)[3], Common::Point(_bounds.width() - (*_images)[3]._width, _bounds.height()));

	// Draw the top of the Info Box
	_surface.hLine((*_images)[0]._width, 0, _bounds.width() - (*_images)[1]._width, INFO_TOP);
	_surface.hLine((*_images)[0]._width, 1, _bounds.width() - (*_images)[1]._width, INFO_MIDDLE);
	_surface.hLine((*_images)[0]._width, 2, _bounds.width() - (*_images)[1]._width, INFO_BOTTOM);

	// Draw the bottom of the Info Box
	_surface.hLine((*_images)[0]._width, _bounds.height()- 3, _bounds.width() - (*_images)[1]._width, INFO_TOP);
	_surface.hLine((*_images)[0]._width, _bounds.height()- 2, _bounds.width() - (*_images)[1]._width, INFO_MIDDLE);
	_surface.hLine((*_images)[0]._width, _bounds.height()- 1, _bounds.width() - (*_images)[1]._width, INFO_BOTTOM);

	// Draw the left Side of the Info Box
	_surface.vLine(0, (*_images)[0]._height, _bounds.height()- (*_images)[2]._height, INFO_TOP);
	_surface.vLine(1, (*_images)[0]._height, _bounds.height()- (*_images)[2]._height, INFO_MIDDLE);
	_surface.vLine(2, (*_images)[0]._height, _bounds.height()- (*_images)[2]._height, INFO_BOTTOM);

	// Draw the right Side of the Info Box
	_surface.vLine(_bounds.width() - 3, (*_images)[0]._height, _bounds.height()- (*_images)[2]._height, INFO_TOP);
	_surface.vLine(_bounds.width() - 2, (*_images)[0]._height, _bounds.height()- (*_images)[2]._height, INFO_MIDDLE);
	_surface.vLine(_bounds.width() - 1, (*_images)[0]._height, _bounds.height()- (*_images)[2]._height, INFO_BOTTOM);
}

} // End of namespace Tattoo

} // End of namespace Sherlock
