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

#ifndef BAGEL_HODJNPODJ_LIBS_DIALOG_UNIT_H
#define BAGEL_HODJNPODJ_LIBS_DIALOG_UNIT_H

#include "common/rect.h"
#include "bagel/hodjnpodj/gfx/gfx_surface.h"

namespace Bagel {
namespace HodjNPodj {

constexpr double X_FACTOR = 4.6;

struct DialogPoint : public Common::Point {
public:
	DialogPoint(int xx, int yy) {
		GfxSurface s;
		s.setFontSize(8);
		this->x = (xx * s.getStringWidth("X")) / X_FACTOR;
		this->y = (yy * s.getStringHeight()) / 8;
	}
	DialogPoint(int fontSize, int xx, int yy) {
		GfxSurface s;
		s.setFontSize(fontSize);
		this->x = (xx * s.getStringWidth("X")) / X_FACTOR;
		this->y = (yy * s.getStringHeight()) / 8;
	}
};

struct DialogRect : public Common::Rect {
public:
	DialogRect(int x, int y, int w, int h) {
		GfxSurface s;
		s.setFontSize(8);
		left = (x * s.getStringWidth("X")) / X_FACTOR;
		top = (y * s.getStringHeight()) / 8;
		right = left + (w * s.getStringWidth("X")) / X_FACTOR;
		bottom = top + (h * s.getStringHeight()) / 8;
	}
	DialogRect(int fontSize, int x, int y, int w, int h) {
		GfxSurface s;
		s.setFontSize(fontSize);
		left = (x * s.getStringWidth("X")) / X_FACTOR;
		top = (y * s.getStringHeight()) / 8;
		right = left + (w * s.getStringWidth("X")) / X_FACTOR;
		bottom = top + (h * s.getStringHeight()) / 8;
	}
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
