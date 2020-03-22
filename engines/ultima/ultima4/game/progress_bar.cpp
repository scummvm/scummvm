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

#include "ultima/ultima4/game/progress_bar.h"
#include "ultima/ultima4/graphics/image.h"
#include "ultima/ultima4/game/settings.h"

namespace Ultima {
namespace Ultima4 {

ProgressBar::ProgressBar(int xp, int yp, int width, int height, int min, int max) :
	View(xp, yp, width, height),
	_min(min),
	_max(max) {
	_current = min;
}

ProgressBar &ProgressBar::operator++()  {
	_current++;
	draw();
	return *this;
}
ProgressBar &ProgressBar::operator--()  {
	_current--;
	draw();
	return *this;
}
void ProgressBar::draw() {
	Image *bar = Image::create(SCALED(_width), SCALED(_height), false, Image::HARDWARE);
	int pos = static_cast<int>((double(_current - _min) / double(_max - _min)) * (_width - (_bWidth * 2)));

	// border color
	bar->fillRect(0, 0, SCALED(_width), SCALED(_height), _bColor.r, _bColor.g, _bColor.b);

	// color
	bar->fillRect(SCALED(_bWidth), SCALED(_bWidth), SCALED(pos), SCALED(_height - (_bWidth * 2)), _color.r, _color.g, _color.b);

	bar->drawOn(_screen, SCALED(_x), SCALED(_y));
	update();

	delete bar;
}

void ProgressBar::setBorderColor(int r, int g, int b, int a) {
	_bColor.r = r;
	_bColor.g = g;
	_bColor.b = b;
	_bColor.a = a;
}

void ProgressBar::setBorderWidth(unsigned int width) {
	_bWidth = width;
}

void ProgressBar::setColor(int r, int g, int b, int a) {
	_color.r = r;
	_color.g = g;
	_color.b = b;
	_color.a = a;
}

} // End of namespace Ultima4
} // End of namespace Ultima
