/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <stdafx.h>
#include "rect.h"

Rect::Rect() : _topLeft(0, 0), _bottomRight(0,0) { 
}

Rect::Rect(int x, int y) : _topLeft(0, 0), _bottomRight(x, y) {
	check();
}

Rect::Rect(int x1, int y1, int x2, int y2) : _topLeft(x1, y1), _bottomRight(x2, y2) { 
	check(); 
}

Rect::Rect(const Rect & r) : _topLeft(r._topLeft), _bottomRight(r._bottomRight) {
}

Rect & Rect::operator=(const Rect & r) {
	_topLeft = r._topLeft;
	_bottomRight = r._bottomRight;
	return *this;
}

bool Rect::operator==(const Rect & r) const {
	return _topLeft == r._topLeft && _bottomRight == r._bottomRight;
}

void Rect::check() {
	if ((_topLeft.getX() < 0) || (_bottomRight.getX() < _topLeft.getX())  || (_topLeft.getY() < 0) || (_bottomRight.getY() < _topLeft.getY())) {
		error("Invalid rect");
	}
}

bool Rect::isInside(int x, int y) const {
	return _topLeft.getX() >= x && _bottomRight.getX() < x && _topLeft.getY() >= y && _bottomRight.getY() < y;
}

bool Rect::isInside(const Point & p) const {
	return (left() <= p.getX()) && (right() > p.getX()) && (top() <= p.getY()) && (bottom() > p.getY());
}

