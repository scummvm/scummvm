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
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_talk.h"
#include "sherlock/tattoo/tattoo_user_interface.h"

namespace Sherlock {

namespace Tattoo {

WidgetBase::WidgetBase(SherlockEngine *vm) : _vm(vm) {
	_scroll = false;
	_dialogTimer = 0;
	_outsideMenu = false;
}

void WidgetBase::summonWindow() {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	// Double-check that the same widget isn't added twice
	if (ui._widgets.contains(this))
		error("Tried to add a widget multiple times");

	// Add widget to the screen
	if (!ui._fixedWidgets.contains(this))
		ui._widgets.push_back(this);
	ui._windowOpen = true;

	_outsideMenu = false;

	draw();
}

void WidgetBase::banishWindow() {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	erase();
	_surface.free();
	ui._widgets.remove(this);
	ui._windowOpen = false;
}

void WidgetBase::close() {
	Events &events = *_vm->_events;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	banishWindow();
	ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;
	events.clearEvents();
}

bool WidgetBase::active() const {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	for (Common::List<WidgetBase *>::iterator i = ui._widgets.begin(); i != ui._widgets.end(); ++i) {
		if ((*i) == this)
			return true;
	}

	return false;
}


void WidgetBase::erase() {
	Screen &screen = *_vm->_screen;

	if (_oldBounds.width() > 0) {
		// Restore the affected area from the secondary back buffer into the first one, and then copy to screen
		screen._backBuffer1.SHblitFrom(screen._backBuffer2, Common::Point(_oldBounds.left, _oldBounds.top), _oldBounds);
		screen.slamRect(_oldBounds);

		// Reset the old bounds so it won't be erased again
		_oldBounds = Common::Rect(0, 0, 0, 0);
	}
}

void WidgetBase::draw() {
	Screen &screen = *_vm->_screen;

	// If there was a previously drawn frame in a different position that hasn't yet been erased, then erase it
	if (_oldBounds.width() > 0 && _oldBounds != _bounds)
		erase();

	if (_bounds.width() > 0 && !_surface.empty()) {
		// Get the area to draw, adjusted for scroll position
		restrictToScreen();

		// Draw the background for the widget
		drawBackground();

		// Draw the widget onto the back buffer and then slam it to the screen
		screen._backBuffer1.SHtransBlitFrom(_surface, Common::Point(_bounds.left, _bounds.top));
		screen.slamRect(_bounds);

		// Store a copy of the drawn area for later erasing
		_oldBounds = _bounds;
	}
}

void WidgetBase::drawBackground() {
	TattooEngine &vm = *(TattooEngine *)_vm;
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	Common::Rect bounds = _bounds;

	if (vm._transparentMenus) {
		ui.makeBGArea(bounds);
	} else {
		bounds.grow(-3);
		screen._backBuffer1.fillRect(bounds, MENU_BACKGROUND);
	}
}

Common::String WidgetBase::splitLines(const Common::String &str, Common::StringArray &lines, int maxWidth, uint maxLines) {
	Talk &talk = *_vm->_talk;
	const char *strP = str.c_str();

	// Loop counting up lines
	lines.clear();
	do {
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
	} while (*strP && (lines.size() < maxLines) && ((byte)*strP < talk._opcodes[OP_SWITCH_SPEAKER]
			|| (byte)*strP == talk._opcodes[OP_NULL]));

	// Return any remaining text left over
	return *strP ? Common::String(strP) : Common::String();
}

void WidgetBase::restrictToScreen() {
	Screen &screen = *_vm->_screen;

	if (_bounds.left < screen._currentScroll.x)
		_bounds.moveTo(screen._currentScroll.x, _bounds.top);
	if (_bounds.top < 0)
		_bounds.moveTo(_bounds.left, 0);
	if (_bounds.right > (screen._currentScroll.x + SHERLOCK_SCREEN_WIDTH))
		_bounds.moveTo(screen._currentScroll.x + SHERLOCK_SCREEN_WIDTH - _bounds.width(), _bounds.top);
	if (_bounds.bottom > screen._backBuffer1.height())
		_bounds.moveTo(_bounds.left, screen._backBuffer1.height() - _bounds.height());
}

void WidgetBase::makeInfoArea(Surface &s) {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	ImageFile &images = *ui._interfaceImages;

	// Draw the four corners of the Info Box
	s.SHtransBlitFrom(images[0], Common::Point(0, 0));
	s.SHtransBlitFrom(images[1], Common::Point(s.width() - images[1]._width, 0));
	s.SHtransBlitFrom(images[2], Common::Point(0, s.height() - images[2]._height));
	s.SHtransBlitFrom(images[3], Common::Point(s.width() - images[3]._width, s.height()));

	// Draw the top of the Info Box
	s.hLine(images[0]._width, 0, s.width() - images[1]._width, INFO_TOP);
	s.hLine(images[0]._width, 1, s.width() - images[1]._width, INFO_MIDDLE);
	s.hLine(images[0]._width, 2, s.width() - images[1]._width, INFO_BOTTOM);

	// Draw the bottom of the Info Box
	s.hLine(images[0]._width, s.height()- 3, s.width() - images[1]._width, INFO_TOP);
	s.hLine(images[0]._width, s.height()- 2, s.width() - images[1]._width, INFO_MIDDLE);
	s.hLine(images[0]._width, s.height()- 1, s.width() - images[1]._width, INFO_BOTTOM);

	// Draw the left Side of the Info Box
	s.vLine(0, images[0]._height, s.height()- images[2]._height, INFO_TOP);
	s.vLine(1, images[0]._height, s.height()- images[2]._height, INFO_MIDDLE);
	s.vLine(2, images[0]._height, s.height()- images[2]._height, INFO_BOTTOM);

	// Draw the right Side of the Info Box
	s.vLine(s.width() - 3, images[0]._height, s.height()- images[2]._height, INFO_TOP);
	s.vLine(s.width() - 2, images[0]._height, s.height()- images[2]._height, INFO_MIDDLE);
	s.vLine(s.width() - 1, images[0]._height, s.height()- images[2]._height, INFO_BOTTOM);
}

void WidgetBase::makeInfoArea() {
	makeInfoArea(_surface);
}

void WidgetBase::drawDialogRect(const Common::Rect &r, bool raised) {
	static_cast<TattooUserInterface *>(_vm->_ui)->drawDialogRect(_surface, r, raised);
}

void WidgetBase::checkTabbingKeys(int numOptions) {
}

Common::Rect WidgetBase::getScrollBarBounds() const {
	Common::Rect r(BUTTON_SIZE, _bounds.height() - 6);
	r.moveTo(_bounds.width() - BUTTON_SIZE - 3, 3);
	return r;
}

void WidgetBase::drawScrollBar(int index, int pageSize, int count) {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	// Fill the area with transparency
	Common::Rect r = getScrollBarBounds();
	_surface.fillRect(r, TRANSPARENCY);

	bool raised = ui._scrollHighlight != 1;
	_surface.fillRect(Common::Rect(r.left + 2, r.top + 2, r.right - 2, r.top + BUTTON_SIZE - 2), INFO_MIDDLE);
	ui.drawDialogRect(_surface, Common::Rect(r.left, r.top, r.left + BUTTON_SIZE, r.top + BUTTON_SIZE), raised);

	raised = ui._scrollHighlight != 5;
	_surface.fillRect(Common::Rect(r.left + 2, r.bottom - BUTTON_SIZE + 2, r.right - 2, r.bottom - 2), INFO_MIDDLE);
	ui.drawDialogRect(_surface, Common::Rect(r.left, r.bottom - BUTTON_SIZE, r.right, r.bottom), raised);

	// Draw the arrows on the scroll buttons
	byte color = index ? INFO_BOTTOM + 2 : INFO_BOTTOM;
	_surface.hLine(r.left + r.width() / 2, r.top - 2 + BUTTON_SIZE / 2, r.left + r.width() / 2, color);
	_surface.hLine(r.left + r.width() / 2 - 1, r.top - 1 + BUTTON_SIZE / 2, r.left + r.width() / 2 + 1, color);
	_surface.hLine(r.left + r.width() / 2 - 2, r.top + BUTTON_SIZE / 2, r.left + r.width() / 2 + 2, color);
	_surface.hLine(r.left + r.width() / 2 - 3, r.top + 1 + BUTTON_SIZE / 2, r.left + r.width() / 2 + 3, color);

	color = (index + pageSize) < count ? INFO_BOTTOM + 2 : INFO_BOTTOM;
	_surface.hLine(r.left + r.width() / 2 - 3, r.bottom - 1 - BUTTON_SIZE + BUTTON_SIZE / 2, r.left + r.width() / 2 + 3, color);
	_surface.hLine(r.left + r.width() / 2 - 2, r.bottom - 1 - BUTTON_SIZE + 1 + BUTTON_SIZE / 2, r.left + r.width() / 2 + 2, color);
	_surface.hLine(r.left + r.width() / 2 - 1, r.bottom - 1 - BUTTON_SIZE + 2 + BUTTON_SIZE / 2, r.left + r.width() / 2 + 1, color);
	_surface.hLine(r.left + r.width() / 2, r.bottom - 1 - BUTTON_SIZE + 3 + BUTTON_SIZE / 2, r.left + r.width() / 2, color);

	// Draw the scroll position bar
	int barHeight = (r.height() - BUTTON_SIZE * 2) * pageSize / count;
	barHeight = CLIP(barHeight, BUTTON_SIZE, r.height() - BUTTON_SIZE * 2);
	int barY = (count <= pageSize) ? r.top + BUTTON_SIZE : r.top + BUTTON_SIZE +
		(r.height() - BUTTON_SIZE * 2 - barHeight) * index / (count - pageSize);

	_surface.fillRect(Common::Rect(r.left + 2, barY + 2, r.right - 2, barY + barHeight - 3), INFO_MIDDLE);
	ui.drawDialogRect(_surface, Common::Rect(r.left, barY, r.right, barY + barHeight), true);
}

void WidgetBase::handleScrollbarEvents(int index, int pageSize, int count) {
	Events &events = *_vm->_events;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();

	// If they're dragging the scrollbar thumb, keep it selected whilst the button is being held
	if ((events._pressed || events._released) && ui._scrollHighlight == SH_THUMBNAIL)
		return;

	ui._scrollHighlight = SH_NONE;

	if ((!events._pressed && !events._rightReleased) || !_scroll)
		return;

	Common::Rect r = getScrollBarBounds();
	r.translate(_bounds.left, _bounds.top);

	// Calculate the Scroll Position bar
	int barHeight = (r.height() - BUTTON_SIZE * 2) * pageSize / count;
	barHeight = CLIP(barHeight, BUTTON_SIZE, r.height() - BUTTON_SIZE * 2);
	int barY = (count <= pageSize) ? r.top + BUTTON_SIZE : r.top + BUTTON_SIZE +
		(r.height() - BUTTON_SIZE * 2 - barHeight) * index / (count - pageSize);

	if (Common::Rect(r.left, r.top, r.right, r.top + BUTTON_SIZE).contains(mousePos))
		// Mouse on scroll up button
		ui._scrollHighlight = SH_SCROLL_UP;
	else if (Common::Rect(r.left, r.top + BUTTON_SIZE, r.right, barY).contains(mousePos))
		// Mouse on paging up area (the area of the vertical bar above the thumbnail)
		ui._scrollHighlight = SH_PAGE_UP;
	else if (Common::Rect(r.left, barY, r.right, barY + barHeight).contains(mousePos))
		// Mouse on scrollbar thumb
		ui._scrollHighlight = SH_THUMBNAIL;
	else if (Common::Rect(r.left, barY + barHeight, r.right, r.bottom - BUTTON_SIZE).contains(mousePos))
		// Mouse on paging down area (the area of the vertical bar below the thumbnail)
		ui._scrollHighlight = SH_PAGE_DOWN;
	else if (Common::Rect(r.left, r.bottom - BUTTON_SIZE, r.right, r.bottom).contains(mousePos))
		// Mouse on scroll down button
		ui._scrollHighlight = SH_SCROLL_DOWN;
}

void WidgetBase::handleScrolling(int &scrollIndex, int pageSize, int max) {
	Events &events = *_vm->_events;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::KeyCode keycode = ui._keyState.keycode;
	Common::Point mousePos = events.mousePos();

	Common::Rect r = getScrollBarBounds();
	r.translate(_bounds.left, _bounds.top);

	if (ui._scrollHighlight != SH_NONE || keycode == Common::KEYCODE_HOME || keycode == Common::KEYCODE_END
		|| keycode == Common::KEYCODE_PAGEUP || keycode == Common::KEYCODE_PAGEDOWN
		|| keycode == Common::KEYCODE_UP || keycode == Common::KEYCODE_DOWN) {
		// Check for the scrollbar
		if (ui._scrollHighlight == SH_THUMBNAIL) {
			int yp = mousePos.y;
			yp = CLIP(yp, r.top + BUTTON_SIZE + 3, r.bottom - BUTTON_SIZE - 3);

			// Calculate the line number that corresponds to the position that the mouse is on the scrollbar
			int lineNum = (yp - r.top - BUTTON_SIZE - 3) * (max - pageSize) / (r.height() - BUTTON_SIZE * 2 - 6);
			scrollIndex = CLIP(lineNum, 0, max - pageSize);
		}

		// Get the current frame so we can check the scroll timer against it
		uint32 frameNum = events.getFrameCounter();

		if (frameNum > _dialogTimer) {
			// Set the timeout for the next scroll if the mouse button remains held down
			_dialogTimer = (_dialogTimer == 0) ? frameNum + pageSize : frameNum + 1;

			// Check for Scroll Up
			if ((ui._scrollHighlight == SH_SCROLL_UP || keycode == Common::KEYCODE_UP) && scrollIndex)
				--scrollIndex;

			// Check for Page Up
			else if ((ui._scrollHighlight == SH_PAGE_UP || keycode == Common::KEYCODE_PAGEUP) && scrollIndex)
				scrollIndex -= pageSize;

			// Check for Page Down
			else if ((ui._scrollHighlight == SH_PAGE_DOWN || keycode == Common::KEYCODE_PAGEDOWN)
				&& (scrollIndex + pageSize < max)) {
				scrollIndex += pageSize;
				if (scrollIndex + pageSize >max)
					scrollIndex = max - pageSize;
			}

			// Check for Scroll Down
			else if ((ui._scrollHighlight == SH_SCROLL_DOWN || keycode == Common::KEYCODE_DOWN) && (scrollIndex + pageSize < max))
				++scrollIndex;
		}

		if (keycode == Common::KEYCODE_END)
			scrollIndex = max - pageSize;

		if (scrollIndex < 0 || keycode == Common::KEYCODE_HOME)
			scrollIndex = 0;
	}
}

} // End of namespace Tattoo

} // End of namespace Sherlock
