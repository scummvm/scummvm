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

ImageFile *WidgetBase::_interfaceImages;

void WidgetBase::setInterfaceImages(ImageFile *images) {
	_interfaceImages = images;
}

void WidgetBase::freeInterfaceImages() {
	delete _interfaceImages;
	_interfaceImages = nullptr;
}

WidgetBase::WidgetBase(SherlockEngine *vm) : _vm(vm) {
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

void WidgetBase::erase() {
	Screen &screen = *_vm->_screen;
	const Common::Point &currentScroll = getCurrentScroll();

	if (_oldBounds.width() > 0) {
		// Get the bounds to copy from the back buffers, adjusted for scroll position
		Common::Rect oldBounds = _oldBounds;
		oldBounds.translate(currentScroll.x, currentScroll.y);

		// Restore the affected area from the secondary back buffer into the first one, and then copy to screen
		screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(oldBounds.left, oldBounds.top), oldBounds);
		screen.blitFrom(screen._backBuffer1, Common::Point(_oldBounds.left, _oldBounds.top), oldBounds);

		// Reset the old bounds so 
		_oldBounds = Common::Rect(0, 0, 0, 0);
	}
}

void WidgetBase::draw() {
	Screen &screen = *_vm->_screen;
	const Common::Point &currentScroll = getCurrentScroll();

	// If there was a previously drawn frame in a different position that hasn't yet been erased, then erase it
	if (_oldBounds.width() > 0 && _oldBounds != _bounds)
		erase();

	if (_bounds.width() > 0 && !_surface.empty()) {
		// Get the area to draw, adjusted for scroll position
		Common::Rect bounds = _bounds;
		bounds.translate(currentScroll.x, currentScroll.y);

		// Copy any area to be drawn on from the secondary back buffer, and then draw surface on top
		screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(bounds.left, bounds.top), bounds);
		screen._backBuffer1.transBlitFrom(_surface, Common::Point(bounds.left, bounds.top));
		screen.blitFrom(screen._backBuffer1, Common::Point(_bounds.left, _bounds.top), bounds);

		// Store a copy of the drawn area for later erasing
		_oldBounds = _bounds;
	}
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

void WidgetBase::makeInfoArea(Surface &s) {
	ImageFile &images = *_interfaceImages;

	// Draw the four corners of the Info Box
	s.transBlitFrom(images[0], Common::Point(0, 0));
	s.transBlitFrom(images[1], Common::Point(s.w() - images[1]._width, 0));
	s.transBlitFrom(images[2], Common::Point(0, s.h() - images[2]._height));
	s.transBlitFrom(images[3], Common::Point(s.w() - images[3]._width, s.h()));

	// Draw the top of the Info Box
	s.hLine(images[0]._width, 0, s.w() - images[1]._width, INFO_TOP);
	s.hLine(images[0]._width, 1, s.w() - images[1]._width, INFO_MIDDLE);
	s.hLine(images[0]._width, 2, s.w() - images[1]._width, INFO_BOTTOM);

	// Draw the bottom of the Info Box
	s.hLine(images[0]._width, s.h()- 3, s.w() - images[1]._width, INFO_TOP);
	s.hLine(images[0]._width, s.h()- 2, s.w() - images[1]._width, INFO_MIDDLE);
	s.hLine(images[0]._width, s.h()- 1, s.w() - images[1]._width, INFO_BOTTOM);

	// Draw the left Side of the Info Box
	s.vLine(0, images[0]._height, s.h()- images[2]._height, INFO_TOP);
	s.vLine(1, images[0]._height, s.h()- images[2]._height, INFO_MIDDLE);
	s.vLine(2, images[0]._height, s.h()- images[2]._height, INFO_BOTTOM);

	// Draw the right Side of the Info Box
	s.vLine(s.w() - 3, images[0]._height, s.h()- images[2]._height, INFO_TOP);
	s.vLine(s.w() - 2, images[0]._height, s.h()- images[2]._height, INFO_MIDDLE);
	s.vLine(s.w() - 1, images[0]._height, s.h()- images[2]._height, INFO_BOTTOM);
}

void WidgetBase::makeInfoArea() {
	makeInfoArea(_surface);
}

const Common::Point &WidgetBase::getCurrentScroll() const {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	return ui._currentScroll;
}

void WidgetBase::checkTabbingKeys(int numOptions) {
}

} // End of namespace Tattoo

} // End of namespace Sherlock
