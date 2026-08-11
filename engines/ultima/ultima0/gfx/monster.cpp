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

#include "ultima/ultima0/gfx/monster.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {
namespace Gfx {

Monster::DrawFn Monster::DRAW_FUNCTIONS[] = {
	nullptr, drawSkeleton, drawThief, drawRat, drawOrc, drawViper,
	drawCarrion, drawGremlin, drawMimic, drawDaemon, drawBalrog
};
int Monster::_xPos = 640;
int Monster::_yPos = 512;
constexpr int color = COL_MONSTER;

// End marker
#define	END		(-9999.99)

#define	X(n)			(x1 + w * (n)/10)
#define	Y(n)			(y1 + h * (n)/10)
#define HWLine(X1, Y1, X2, Y2) s->drawLine(X1, Y1, X2, Y2, color)
#define BOX(x1,y1,x2,y2) { HWLine(X(x1),Y(y1),X(x2),Y(y1));HWLine(X(x1),Y(y1),X(x1),Y(y2));HWLine(X(x2),Y(y2),X(x2),Y(y1));HWLine(X(x2),Y(y2),X(x1),Y(y2)); }

void Monster::draw(Graphics::ManagedSurface *s, int x, int y, int monster, double scale) {
	// Save drawing pos
	_xPos = x; _yPos = y;
	if (monster == MN_MIMIC)
		// Fix for Mimic/Chest
		_xPos -= 90;

	// Call appropriate function
	assert(monster > 0 && monster <= MN_BALROG);
	(*DRAW_FUNCTIONS[monster])(s, 0, 0, scale);
}

void Monster::hPlot(Graphics::ManagedSurface *s, double x, double y, ...) {
	va_list alist;
	double y1, x1;

	// Start reading values
	va_start(alist, y);
	bool isPixel = true;

	do {
		x1 = va_arg(alist, double);			// Get the next two
		y1 = va_arg(alist, double);
		if (x1 != END && y1 != END) {
			// If legit, draw the line
			HWLine(_xPos + x, _yPos + y, _xPos + x1, _yPos + y1);
			isPixel = false;
		} else if (isPixel)
			// Single pixel only
			HWLine(_xPos + x, _yPos + y, _xPos + x, _yPos + y);

		x = x1; y = y1;
	} while (x1 != END && y1 != END);

	va_end(alist);
}

void Monster::drawSkeleton(Graphics::ManagedSurface *s, double x, double y, double d) {
	hPlot(s, x - 23 / d, y, x - 15 / d, y, x - 15 / d, y - 15 / d, x - 8 / d, y - 30 / d, x + 8 / d, y - 30 / d, x + 15 / d, y - 15 / d, x + 15 / d, y, x + 23 / d, y, END, END);
	hPlot(s, x, y - 26 / d, x, y - 65 / d, END, END);
	hPlot(s, x - 2 / d + .5, y - 38 / d, x + 2 / d + .5, y - 38 / d, END, END);
	hPlot(s, x - 3 / d + .5, y - 45 / d, x + 3 / d + .5, y - 45 / d, END, END);
	hPlot(s, x - 5 / d + .5, y - 53 / d, x + 5 / d + .5, y - 53 / d, END, END);
	hPlot(s, x - 23 / d, y - 56 / d, x - 30 / d, y - 53 / d, x - 23 / d, y - 45 / d, x - 23 / d, y - 53 / d, x - 8 / d, y - 38 / d, END, END);
	hPlot(s, x - 15 / d, y - 45 / d, x - 8 / d, y - 60 / d, x + 8 / d, y - 60 / d, x + 15 / d, y - 45 / d, END, END);
	hPlot(s, x + 15 / d, y - 42 / d, x + 15 / d, y - 57 / d, END, END);
	hPlot(s, x + 12 / d, y - 45 / d, x + 20 / d, y - 45 / d, END, END);
	hPlot(s, x, y - 75 / d, x - 5 / d + .5, y - 80 / d, x - 8 / d, y - 75 / d, x - 5 / d + .5, y - 65 / d, x + 5 / d + .5, y - 65 / d, x + 5 / d + .5, y - 68 / d, x - 5 / d + .5, y - 68 / d, x - 5 / d + .5, y - 65 / d, END, END);
	hPlot(s, x + 5 / d + .5, y - 65 / d, x + 8 / d, y - 75 / d, x + 5 / d + .5, y - 80 / d, x - 5 / d + .5, y - 80 / d, END, END);
	hPlot(s, x - 5 / d + .5, y - 72 / d, END, END);
	hPlot(s, x + 5 / d + .5, y - 72 / d, END, END);
}

void Monster::drawThief(Graphics::ManagedSurface *s, double x, double y, double d) {
	hPlot(s, x, y - 56 / d, x, y - 8 / d, x + 10 / d, y, x + 30 / d, y, x + 30 / d, y - 45 / d, x + 10 / d, y - 64 / d, x, y - 56 / d, END, END);
	hPlot(s, x - 10 / d, y - 64 / d, x - 30 / d, y - 45 / d, x - 30 / d, y, x - 10 / d, y, x, y - 8 / d, END, END);
	hPlot(s, x - 10 / d, y - 64 / d, x - 10 / d, y - 75 / d, x, y - 83 / d, x + 10 / d, y - 75 / d, x, y - 79 / d, x - 10 / d, y - 75 / d, x, y - 60 / d, x + 10 / d, y - 75 / d, x + 10 / d, y - 64 / d, END, END);
}

void Monster::drawRat(Graphics::ManagedSurface *s, double x, double y, double d) {
	hPlot(s, x + 5 / d, y - 30 / d, x, y - 25 / d, x - 5 / d, y - 30 / d, x - 15 / d, y - 5 / d, x - 10 / d, y, x + 10 / d, y, x + 15 / d, y - 5 / d, END, END);
	hPlot(s, x + 20 / d, y - 5 / d, x + 10 / d, y, x + 15 / d, y - 5 / d, x + 5 / d, y - 30 / d, x + 10 / d, y - 40 / d, x + 3 / d + .5, y - 35 / d, x - 3 / d + .5, y - 35 / d, x - 10 / d, y - 40 / d, x - 5 / d, y - 30 / d, END, END);
	hPlot(s, x - 5 / d, y - 33 / d, x - 3 / d + .5, y - 30 / d, END, END);
	hPlot(s, x + 5 / d, y - 33 / d, x + 3 / d + .5, y - 30 / d, END, END);
	hPlot(s, x - 5 / d, y - 20 / d, x - 5 / d, y - 15 / d, END, END);
	hPlot(s, x + 5 / d, y - 20 / d, x + 5 / d, y - 15 / d, END, END);
	hPlot(s, x - 7 / d, y - 20 / d, x - 7 / d, y - 15 / d, END, END);
	hPlot(s, x + 7 / d, y - 20 / d, x + 7 / d, y - 15 / d, END, END);
}

void Monster::drawOrc(Graphics::ManagedSurface *s, double x, double y, double d) {
	hPlot(s, x, y, x - 15 / d, y, x - 8 / d, y - 8 / d, x - 8 / d, y - 15 / d, x - 15 / d, y - 23 / d, x - 15 / d, y - 15 / d, x - 23 / d, y - 23 / d, END, END);
	hPlot(s, x - 23 / d, y - 23 / d, x - 15 / d, y - 53 / d, x - 8 / d, y - 53 / d, x - 15 / d, y - 68 / d, x - 8 / d, y - 75 / d, x, y - 75 / d, END, END);
	hPlot(s, x, y, x + 15 / d, y, x + 8 / d, y - 8 / d, x + 8 / d, y - 15 / d, x + 15 / d, y - 23 / d, x + 15 / d, y - 15 / d, x + 23 / d, y - 23 / d, END, END);
	hPlot(s, x + 23 / d, y - 23 / d, x + 15 / d, y - 53 / d, x + 8 / d, y - 53 / d, x + 15 / d, y - 68 / d, x + 8 / d, y - 75 / d, x, y - 75 / d, END, END);
	hPlot(s, x - 15 / d, y - 68 / d, x + 15 / d, y - 68 / d, END, END);
	hPlot(s, x - 8 / d, y - 53 / d, x + 8 / d, y - 53 / d, END, END);
	hPlot(s, x - 23 / d, y - 15 / d, x + 8 / d, y - 45 / d, END, END);
	hPlot(s, x - 8 / d, y - 68 / d, x, y - 60 / d, x + 8 / d, y - 68 / d, x + 8 / d, y - 60 / d, x - 8 / d, y - 60 / d, x - 8 / d, y - 68 / d, END, END);
	hPlot(s, x, y - 38 / d, x - 8 / d, y - 38 / d, x + 8 / d, y - 53 / d, x + 8 / d, y - 45 / d, x + 15 / d, y - 45 / d, x, y - 30 / d, x, y - 38 / d, END, END);
}

void Monster::drawViper(Graphics::ManagedSurface *s, double x, double y, double d) {
	hPlot(s, x - 10 / d, y - 15 / d, x - 10 / d, y - 30 / d, x - 15 / d, y - 20 / d, x - 15 / d, y - 15 / d, x - 15 / d, y, x + 15 / d, y, x + 15 / d, y - 15 / d, x - 15 / d, y - 15 / d, END, END);
	hPlot(s, x - 15 / d, y - 10 / d, x + 15 / d, y - 10 / d, END, END);
	hPlot(s, x - 15 / d, y - 5 / d, x + 15 / d, y - 5 / d, END, END);
	hPlot(s, x, y - 15 / d, x - 5 / d, y - 20 / d, x - 5 / d, y - 35 / d, x + 5 / d, y - 35 / d, x + 5 / d, y - 20 / d, x + 10 / d, y - 15 / d, END, END);
	hPlot(s, x - 5 / d, y - 20 / d, x + 5 / d, y - 20 / d, END, END);
	hPlot(s, x - 5 / d, y - 25 / d, x + 5 / d, y - 25 / d, END, END);
	hPlot(s, x - 5 / d, y - 30 / d, x + 5 / d, y - 30 / d, END, END);
	hPlot(s, x - 10 / d, y - 35 / d, x - 10 / d, y - 40 / d, x - 5 / d, y - 45 / d, x + 5 / d, y - 45 / d, x + 10 / d, y - 40 / d, x + 10 / d, y - 35 / d, END, END);
	hPlot(s, x - 10 / d, y - 40 / d, x, y - 45 / d, x + 10 / d, y - 40 / d, END, END);
	hPlot(s, x - 5 / d, y - 40 / d, x + 5 / d, y - 40 / d, x + 15 / d, y - 30 / d, x, y - 40 / d, x - 15 / d, y - 30 / d, x - 5 / d + .5, y - 40 / d, END, END);
}

void Monster::drawCarrion(Graphics::ManagedSurface *s, double x, double y, double d) {
	// 79-dst.recty(d) line here
	hPlot(s, x - 20 / d, y - 79 / d, x - 20 / d, y - 88 / d, x - 10 / d, y - 83 / d, x + 10 / d, y - 83 / d, x + 20 / d, y - 88 / d, x + 20 / d, y - 79 / d, x - 20 / d, y - 79 / d, END, END);
	hPlot(s, x - 20 / d, y - 88 / d, x - 30 / d, y - 83 / d, x - 30 / d, y - 78 / d, END, END);
	hPlot(s, x + 20 / d, y - 88 / d, x + 30 / d, y - 83 / d, x + 40 / d, y - 83 / d, END, END);
	hPlot(s, x - 15 / d, y - 86 / d, x - 20 / d, y - 83 / d, x - 20 / d, y - 78 / d, x - 30 / d, y - 73 / d, x - 30 / d, y - 68 / d, x - 20 / d, y - 63 / d, END, END);
	hPlot(s, x - 10 / d, y - 83 / d, x - 10 / d, y - 58 / d, x, y - 50 / d, END, END);
	hPlot(s, x + 10 / d, y - 83 / d, x + 10 / d, y - 78 / d, x + 20 / d, y - 73 / d, x + 20 / d, y - 40 / d, END, END);
	hPlot(s, x + 15 / d, y - 85 / d, x + 20 / d, y - 78 / d, x + 30 / d, y - 76 / d, x + 30 / d, y - 60 / d, END, END);
	hPlot(s, x, y - 83 / d, x, y - 73 / d, x + 10 / d, y - 68 / d, x + 10 / d, y - 63 / d, x, y - 58 / d, END, END);
}

void Monster::drawGremlin(Graphics::ManagedSurface *s, double x, double y, double d) {
	hPlot(s, x + 5 / d + 1, y - 10 / d, x - 5 / d + 1, y - 10 / d, x, y - 15 / d, x + 10 / d, y - 20 / d, x + 5 / d + 1, y - 15 / d, x + 5 / d + 1, y - 10 / d, END, END);
	hPlot(s, x + 5 / d + 1, y - 10 / d, x + 7 / d + 1, y - 6 / d, x + 5 / d + 1, y - 3 / d, x - 5 / d + 1, y - 3 / d, x - 7 / d + 1, y - 6 / d, x - 5 / d + 1, y - 10 / d, END, END);
	hPlot(s, x + 2 / d + 1, y - 3 / d, x + 5 / d + 1, y, x + 8 / d, y, END, END);
	hPlot(s, x - 2 / d + 1, y - 3 / d, x - 5 / d + 1, y, x - 8 / d, y, END, END);
	hPlot(s, x + 3 / d + 1, y - 8 / d, END, END);
	hPlot(s, x - 3 / d + 1, y - 8 / d, END, END);
	hPlot(s, x + 3 / d + 1, y - 5 / d, x - 3 / d + 1, y - 5 / d, END, END);
}

void Monster::drawMimic(Graphics::ManagedSurface *s, double x, double y, double d) {
	double xx = x;
	hPlot(s, 139 - 10 / d, xx, 139 - 10 / d, xx - 10 / d, 139 + 10 / d, xx - 10 / d, 139 + 10 / d, xx, 139 - 10 / d, xx, END, END);
	hPlot(s, 139 - 10 / d, xx - 10 / d, 139 - 5 / d, xx - 15 / d, 139 + 15 / d, xx - 15 / d, 139 + 15 / d, xx - 5 / d, 139 + 10 / d, xx, END, END);
	hPlot(s, 139 + 10 / d, xx - 10 / d, 139 + 15 / d, xx - 15 / d, END, END);
}

void Monster::drawDaemon(Graphics::ManagedSurface *s, double x, double y, double d) {
	hPlot(s, x - 14 / d, y - 46 / d, x - 12 / d, y - 37 / d, x - 20 / d, y - 32 / d, x - 30 / d, y - 32 / d, x - 22 / d, y - 24 / d, x - 40 / d, y - 17 / d, x - 40 / d, y - 7 / d, x - 38 / d, y - 5 / d, x - 40 / d, y - 3 / d, x - 40 / d, y, END, END);
	hPlot(s, x - 40 / d, y, x - 36 / d, y, x - 34 / d, y - 2 / d, x - 32 / d, y, x - 28 / d, y, x - 28 / d, y - 3 / d, x - 30 / d, y - 5 / d, x - 28 / d, y - 7 / d, x - 28 / d, y - 15 / d, x, y - 27 / d, END, END);
	hPlot(s, x + 14 / d, y - 46 / d, x + 12 / d, y - 37 / d, x + 20 / d, y - 32 / d, x + 30 / d, y - 32 / d, x + 22 / d, y - 24 / d, x + 40 / d, y - 17 / d, x + 40 / d, y - 7 / d, x + 38 / d, y - 5 / d, x + 40 / d, y - 3 / d, x + 40 / d, y, END, END);
	hPlot(s, x + 40 / d, y, x + 36 / d, y, x + 34 / d, y - 2 / d, x + 32 / d, y, x + 28 / d, y, x + 28 / d, y - 3 / d, x + 30 / d, y - 5 / d, x + 28 / d, y - 7 / d, x + 28 / d, y - 15 / d, x, y - 27 / d, END, END);
	hPlot(s, x + 6 / d, y - 48 / d, x + 38 / d, y - 41 / d, x + 40 / d, y - 42 / d, x + 18 / d, y - 56 / d, x + 12 / d, y - 56 / d, x + 10 / d, y - 57 / d, x + 8 / d, y - 56 / d, x - 8 / d, y - 56 / d, x - 10 / d, y - 58 / d, x + 14 / d, y - 58 / d, x + 16 / d, y - 59 / d, END, END);
	hPlot(s, x + 16 / d, y - 59 / d, x + 8 / d, y - 63 / d, x + 6 / d, y - 63 / d, x + 2 / d + .5, y - 70 / d, x + 2 / d + .5, y - 63 / d, x - 2 / d + .5, y - 63 / d, x - 2 / d + .5, y - 70 / d, x - 6 / d, y - 63 / d, x - 8 / d, y - 63 / d, x - 16 / d, y - 59 / d, x - 14 / d, y - 58 / d, x - 10 / d, y - 58 / d, END, END);
	hPlot(s, x - 10 / d, y - 57 / d, x - 12 / d, y - 56 / d, x - 18 / d, y - 56 / d, x - 36 / d, y - 47 / d, x - 36 / d, y - 39 / d, x - 28 / d, y - 41 / d, x - 28 / d, y - 46 / d, x - 20 / d, y - 50 / d, x - 18 / d, y - 50 / d, x - 14 / d, y - 46 / d, END, END);
	hPlot(s, x - 28 / d, y - 41 / d, x + 30 / d, y - 55 / d, END, END);
	hPlot(s, x + 28 / d, y - 58 / d, x + 22 / d, y - 56 / d, x + 22 / d, y - 53 / d, x + 28 / d, y - 52 / d, x + 34 / d, y - 54 / d, END, END);
	hPlot(s, x + 20 / d, y - 50 / d, x + 26 / d, y - 47 / d, END, END);
	hPlot(s, x + 10 / d, y - 58 / d, x + 10 / d, y - 61 / d, x + 4 / d, y - 58 / d, END, END);
	hPlot(s, x - 10 / d, y - 58 / d, x - 10 / d, y - 61 / d, x - 4 / d, y - 58 / d, END, END);
	hPlot(s, x + 40 / d, y - 9 / d, x + 50 / d, y - 12 / d, x + 40 / d, y - 7 / d, END, END);
	hPlot(s, x - 8 / d, y - 25 / d, x + 6 / d, y - 7 / d, x + 28 / d, y - 7 / d, x + 28 / d, y - 9 / d, x + 20 / d, y - 9 / d, x + 6 / d, y - 25 / d, END, END);
}

void Monster::drawBalrog(Graphics::ManagedSurface *s, double x, double y, double d) {
	hPlot(s, x + 6 / d, y - 60 / d, x + 30 / d, y - 90 / d, x + 60 / d, y - 30 / d, x + 60 / d, y - 10 / d, x + 30 / d, y - 40 / d, x + 15 / d, y - 40 / d, END, END);
	hPlot(s, x - 6 / d, y - 60 / d, x - 30 / d, y - 90 / d, x - 60 / d, y - 30 / d, x - 60 / d, y - 10 / d, x - 30 / d, y - 40 / d, x - 15 / d, y - 40 / d, END, END);
	hPlot(s, x, y - 25 / d, x + 6 / d, y - 25 / d, x + 10 / d, y - 20 / d, x + 12 / d, y - 10 / d, x + 10 / d, y - 6 / d, x + 10 / d, y, x + 14 / d, y, x + 15 / d, y - 5 / d, x + 16 / d, y, x + 20 / d, y, END, END);
	hPlot(s, x + 20 / d, y, x + 20 / d, y - 6 / d, x + 18 / d, y - 10 / d, x + 18 / d, y - 20 / d, x + 15 / d, y - 30 / d, x + 15 / d, y - 45 / d, x + 40 / d, y - 60 / d, x + 40 / d, y - 70 / d, END, END);
	hPlot(s, x + 40 / d, y - 70 / d, x + 10 / d, y - 55 / d, x + 6 / d, y - 60 / d, x + 10 / d, y - 74 / d, x + 6 / d, y - 80 / d, x + 4 / d + .5, y - 80 / d, x + 3 / d + .5, y - 82 / d, x + 2 / d + .5, y - 80 / d, x, y - 80 / d, END, END);
	hPlot(s, x, y - 25 / d, x - 6 / d, y - 25 / d, x - 10 / d, y - 20 / d, x - 12 / d, y - 10 / d, x - 10 / d, y - 6 / d, x - 10 / d, y, x - 14 / d, y, x - 15 / d, y - 5 / d, x - 16 / d, y, x - 20 / d, y, END, END);
	hPlot(s, x - 20 / d, y, x - 20 / d, y - 6 / d, x - 18 / d, y - 10 / d, x - 18 / d, y - 20 / d, x - 15 / d, y - 30 / d, x - 15 / d, y - 45 / d, x - 40 / d, y - 60 / d, x - 40 / d, y - 70 / d, END, END); // left wing
	hPlot(s, x - 40 / d, y - 70 / d, x - 10 / d, y - 55 / d, x - 6 / d, y - 60 / d, x - 10 / d, y - 74 / d, x - 6 / d, y - 80 / d, x - 4 / d + .5, y - 80 / d, x - 3 / d + .5, y - 82 / d, x - 2 / d + .5, y - 80 / d, x, y - 80 / d, END, END);
	hPlot(s, x - 6 / d, y - 25 / d, x, y - 6 / d, x + 10 / d, y, x + 4 / d + .5, y - 8 / d, x + 6 / d, y - 25 / d, END, END);
	hPlot(s, x - 40 / d, y - 64 / d, x - 40 / d, y - 90 / d, x - 52 / d, y - 80 / d, x - 52 / d, y - 40 / d, END, END);
	hPlot(s, x + 40 / d, y - 86 / d, x + 38 / d, y - 92 / d, x + 42 / d, y - 92 / d, x + 40 / d, y - 86 / d, x + 40 / d, y - 50 / d, END, END);
	hPlot(s, x + 4 / d + .5, y - 70 / d, x + 6 / d, y - 74 / d, END, END);
	hPlot(s, x - 4 / d + .5, y - 70 / d, x - 6 / d, y - 74 / d, END, END);
	hPlot(s, x, y - 64 / d, x, y - 60 / d, END, END);
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
