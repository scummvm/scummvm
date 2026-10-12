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

#ifndef BCB_UI_H
#define BCB_UI_H

#include "common/rect.h"

namespace BCB {

struct StaticRect {
	int16 left;
	int16 top;
	int16 right;
	int16 bottom;

	operator Common::Rect() const { return Common::Rect(left, top, right, bottom); }
	bool contains(const Common::Point &point) const { return Common::Rect(left, top, right, bottom).contains(point); }
	int16 width() const { return right - left; }
	int16 height() const { return bottom - top; }
};

extern const StaticRect kMapSubmodeMciControlHitRect;
extern const StaticRect kMapSubmodeMovieBackdropRect;
extern const int kMapSubmodeMovieBackdropSourceX;
extern const int kMapSubmodeMovieBackdropSourceY;

struct ButtonControlRecord {
	int controlId;
	int x;
	int y;
	int width;
	int height;
};

struct StaticHitRect {
	int16 left;
	int16 top;
	int16 right;
	int16 bottom;

	bool contains(const Common::Point &point) const { return Common::Rect(left, top, right, bottom).contains(point); }
};

} // End of namespace BCB

#endif // BCB_UI_H
