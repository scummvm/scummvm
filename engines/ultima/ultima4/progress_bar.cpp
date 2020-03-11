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

#include "ultima/ultima4/progress_bar.h"
#include "ultima/ultima4/image.h"
#include "ultima/ultima4/settings.h"

namespace Ultima {
namespace Ultima4 {

ProgressBar::ProgressBar(int x, int y, int width, int height, int _min, int _max) :
    View(x, y, width, height),
    min(_min),
    max(_max) {
    current = min;
}

ProgressBar& ProgressBar::operator++()  { current++; draw(); return *this; }
ProgressBar& ProgressBar::operator--()  { current--; draw(); return *this; }
void ProgressBar::draw() {
    Image *bar = Image::create(SCALED(width), SCALED(height), false, Image::HARDWARE);
    int pos = static_cast<int>((double(current - min) / double(max - min)) * (width - (bwidth * 2)));

    // border color
    bar->fillRect(0, 0, SCALED(width), SCALED(height), bcolor.r, bcolor.g, bcolor.b); 

    // color
    bar->fillRect(SCALED(bwidth), SCALED(bwidth), SCALED(pos), SCALED(height - (bwidth * 2)), color.r, color.g, color.b); 

    bar->drawOn(screen, SCALED(x), SCALED(y));
    update();

    delete bar;
}

void ProgressBar::setBorderColor(int r, int g, int b, int a) {
    bcolor.r = r;
    bcolor.g = g;
    bcolor.b = b;
    bcolor.a = a;
}

void ProgressBar::setBorderWidth(unsigned int width) {
    bwidth = width;
}

void ProgressBar::setColor(int r, int g, int b, int a) {
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
}

} // End of namespace Ultima4
} // End of namespace Ultima
