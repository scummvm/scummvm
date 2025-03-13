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

#include "bagel/hodjnpodj/gfx/scrollbar.h"

namespace Bagel {
namespace HodjNPodj {

void ScrollBar::draw() {
	GfxSurface s = getSurface();

	s.clear(getPaletteIndex(RGB(211, 211, 211)));
	s.frameRect(_bounds, BLACK);

	Common::Rect left(0, 0, s.h, s.h);
	drawSquare(s, left);
	drawArrow(s, left, true);
	Common::Rect right(s.w - s.h, 0, s.w, s.h);
	drawSquare(s, right);
	drawArrow(s, right, false);

	drawSquare(s, getThumbRect());
}

void ScrollBar::drawSquare(GfxSurface &s, const Common::Rect &r) {
	s.frameRect(r, BLACK);
	byte darkGrey = getPaletteIndex(RGB(169, 169, 169));

	s.hLine(r.left + 2, r.bottom - 3, r.right - 2, darkGrey);
	s.hLine(r.left + 1, r.bottom - 2, r.right - 2, darkGrey);
	s.vLine(r.right - 3, r.top + 1, r.bottom - 2, darkGrey);
	s.vLine(r.right - 2, r.top + 1, r.bottom - 2, darkGrey);

	s.hLine(r.left + 1, r.top + 1, r.right - 3, WHITE);
	s.vLine(r.left + 1, r.top + 1, r.bottom - 3, WHITE);
}

void ScrollBar::drawArrow(GfxSurface &s, const Common::Rect &r,
		bool leftArrow) {
	int xCenter = (r.left + r.right) / 2;
	int yCenter = (r.top + r.bottom) / 2;
	int xDelta = leftArrow ? 1 : -1;
	int x = leftArrow ? xCenter - 3 : xCenter + 3;

	for (int xCtr = 0; xCtr < 7; ++xCtr, x += xDelta) {
		int yDiff = (xCtr >= 4) ? 1 : xCtr;
		s.vLine(x, yCenter - yDiff, yCenter + yDiff, BLACK);
	}
}

bool ScrollBar::msgMouseDown(const MouseDownMessage &msg) {
	if (msg._pos.x < (_bounds.left + _bounds.height())) {
		// Left arrow button
		if (_value > _minValue)
			setScrollPos(_value - 1);

	} else if (msg._pos.x >= (_bounds.right - _bounds.height())) {
		// Right arrow button
		if (_value < _maxValue)
			setScrollPos(_value + 1);

	} else {
		// Presume we're dragging the thumb
		setScrollPos(getIndexFromX(msg._pos.x));
		_isDragging = true;
	}

	return true;
}

bool ScrollBar::msgMouseUp(const MouseUpMessage &msg) {
	_isDragging = false;
	return true;
}

bool ScrollBar::msgMouseMove(const MouseMoveMessage &msg) {
	if (_isDragging)
		setScrollPos(getIndexFromX(msg._pos.x));

	return true;
}

bool ScrollBar::msgMouseLeave(const MouseLeaveMessage &msg) {
	_isDragging = false;
	return true;
}

void ScrollBar::setScrollRange(int nMinPos, int nMaxPos,
		bool bRedraw) {
	_minValue = nMinPos;
	_maxValue = nMaxPos;
	_value = CLIP(_value, _minValue, _maxValue);

	if (bRedraw)
		redraw();
}

void ScrollBar::setScrollPos(int value) {
	_value = CLIP(value, _minValue, _maxValue);
	redraw();
	_parent->send(GameMessage("SCROLL", _name, _value));
}

Common::Rect ScrollBar::getThumbRect() const {
	// The thumb will start after the left arrow button,
	// and at most, it will be drawn before the right arrow
	int slideArea = _bounds.width() - (_bounds.height() * 3) + 1;

	int xStart = _bounds.height() +
		slideArea * (_value - _minValue) /
		(_maxValue - _minValue);

	return Common::Rect(xStart, 0,
		xStart + _bounds.height(), _bounds.height());
}

int ScrollBar::getIndexFromX(int xp) const {
	int slideStart = _bounds.left + _bounds.height();
	int slideFinish = _bounds.right - (_bounds.height() * 2) + 1;
	int slideArea = _bounds.width() - (_bounds.height() * 3) + 1;

	if (xp < slideStart)
		return _minValue;
	if (xp >= slideFinish)
		return _maxValue;

	return _minValue + (xp - slideStart)
		* (_maxValue - _minValue) / slideArea;
}

} // namespace HodjNPodj
} // namespace Bagel
