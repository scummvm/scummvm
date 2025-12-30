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
	nullptr, _DRAWSkeleton, _DRAWThief, _DRAWRat, _DRAWOrc, _DRAWViper,
	_DRAWCarrion, _DRAWGremlin, _DRAWMimic, _DRAWDaemon, _DRAWBalrog
};
int Monster::xPos = 640;
int Monster::yPos = 512;

#define	END		(-9999.99)					/* End marker */

#define HWColour(IDX)   color = IDX
#define	X(n)			(x1 + w * (n)/10)
#define	Y(n)			(y1 + h * (n)/10)
#define HWLine(X1, Y1, X2, Y2) s->drawLine(X1, Y1, X2, Y2, color)
#define BOX(x1,y1,x2,y2) { HWLine(X(x1),Y(y1),X(x2),Y(y1));HWLine(X(x1),Y(y1),X(x1),Y(y2));HWLine(X(x2),Y(y2),X(x2),Y(y1));HWLine(X(x2),Y(y2),X(x1),Y(y2)); }

void Monster::draw(Graphics::ManagedSurface *s, int x, int y,
		int Monster, double Scale) {
	const auto &player = g_engine->_player;
	const auto &dungeon = g_engine->_dungeon;

	xPos = x; yPos = y;						// Save drawing pos
	if (Monster == MN_MIMIC)
		// Fix for Mimic/Chest
		xPos = xPos - 90;

	// Call appropriate function
	assert(Monster <= MN_BALROG);
	(*DRAW_FUNCTIONS)(s, 0, 0, Scale);
}

void Monster::_HPlot(Graphics::ManagedSurface *s, double x, double y, ...) {
	va_list alist;
	double y1, x1;
	int color;

	// Start reading values
	va_start(alist, y);

	do {
		x1 = va_arg(alist, double);			// Get the next two
		y1 = va_arg(alist, double);
		if (x1 != END && y1 != END)			// If legit, draw the line
			HWLine(xPos + x, yPos - y, xPos + x1, yPos - y1);
		x = x1; y = y1;
	} while (x1 != END && y1 != END);

	va_end(alist);
}

void Monster::_DRAWSkeleton(Graphics::ManagedSurface *s, double x, double y, double d) {
	_HPlot(s, y - 23 / d, x, y - 15 / d, x, y - 15 / d, x - 15 / d, y - 8 / d, x - 30 / d, y + 8 / d, x - 30 / d, y + 15 / d, x - 15 / d, y + 15 / d, x, y + 23 / d, x, END, END);
	_HPlot(s, y, x - 26 / d, y, x - 65 / d, END, END);
	_HPlot(s, y - 2 / d + .5, x - 38 / d, y + 2 / d + .5, x - 38 / d, END, END);
	_HPlot(s, y - 3 / d + .5, x - 45 / d, y + 3 / d + .5, x - 45 / d, END, END);
	_HPlot(s, y - 5 / d + .5, x - 53 / d, y + 5 / d + .5, x - 53 / d, END, END);
	_HPlot(s, y - 23 / d, x - 56 / d, y - 30 / d, x - 53 / d, y - 23 / d, x - 45 / d, y - 23 / d, x - 53 / d, y - 8 / d, x - 38 / d, END, END);
	_HPlot(s, y - 15 / d, x - 45 / d, y - 8 / d, x - 60 / d, y + 8 / d, x - 60 / d, y + 15 / d, x - 45 / d, END, END);
	_HPlot(s, y + 15 / d, x - 42 / d, y + 15 / d, x - 57 / d, END, END);
	_HPlot(s, y + 12 / d, x - 45 / d, y + 20 / d, x - 45 / d, END, END);
	_HPlot(s, y, x - 75 / d, y - 5 / d + .5, x - 80 / d, y - 8 / d, x - 75 / d, y - 5 / d + .5, x - 65 / d, y + 5 / d + .5, x - 65 / d, y + 5 / d + .5, x - 68 / d, y - 5 / d + .5, x - 68 / d, y - 5 / d + .5, x - 65 / d, END, END);
	_HPlot(s, y + 5 / d + .5, x - 65 / d, y + 8 / d, x - 75 / d, y + 5 / d + .5, x - 80 / d, y - 5 / d + .5, x - 80 / d, END, END);
	_HPlot(s, y - 5 / d + .5, x - 72 / d, END, END);
	_HPlot(s, y + 5 / d + .5, x - 72 / d, END, END);
}

void Monster::_DRAWThief(Graphics::ManagedSurface *s, double x, double y, double d) {
	_HPlot(s, y, x - 56 / d, y, x - 8 / d, y + 10 / d, x, y + 30 / d, x, y + 30 / d, x - 45 / d, y + 10 / d, x - 64 / d, y, x - 56 / d, END, END);
	_HPlot(s, y - 10 / d, x - 64 / d, y - 30 / d, x - 45 / d, y - 30 / d, x, y - 10 / d, x, y, x - 8 / d, END, END);
	_HPlot(s, y - 10 / d, x - 64 / d, y - 10 / d, x - 75 / d, y, x - 83 / d, y + 10 / d, x - 75 / d, y, x - 79 / d, y - 10 / d, x - 75 / d, y, x - 60 / d, y + 10 / d, x - 75 / d, y + 10 / d, x - 64 / d, END, END);
}

void Monster::_DRAWRat(Graphics::ManagedSurface *s, double x, double y, double d) {
	_HPlot(s, y + 5 / d, x - 30 / d, y, x - 25 / d, y - 5 / d, x - 30 / d, y - 15 / d, x - 5 / d, y - 10 / d, x, y + 10 / d, x, y + 15 / d, x - 5 / d, END, END);
	_HPlot(s, y + 20 / d, x - 5 / d, y + 10 / d, x, y + 15 / d, x - 5 / d, y + 5 / d, x - 30 / d, y + 10 / d, x - 40 / d, y + 3 / d + .5, x - 35 / d, y - 3 / d + .5, x - 35 / d, y - 10 / d, x - 40 / d, y - 5 / d, x - 30 / d, END, END);
	_HPlot(s, y - 5 / d, x - 33 / d, y - 3 / d + .5, x - 30 / d, END, END);
	_HPlot(s, y + 5 / d, x - 33 / d, y + 3 / d + .5, x - 30 / d, END, END);
	_HPlot(s, y - 5 / d, x - 20 / d, y - 5 / d, x - 15 / d, END, END);
	_HPlot(s, y + 5 / d, x - 20 / d, y + 5 / d, x - 15 / d, END, END);
	_HPlot(s, y - 7 / d, x - 20 / d, y - 7 / d, x - 15 / d, END, END);
	_HPlot(s, y + 7 / d, x - 20 / d, y + 7 / d, x - 15 / d, END, END);
}

void Monster::_DRAWOrc(Graphics::ManagedSurface *s, double x, double y, double d) {
	_HPlot(s, y, x, y - 15 / d, x, y - 8 / d, x - 8 / d, y - 8 / d, x - 15 / d, y - 15 / d, x - 23 / d, y - 15 / d, x - 15 / d, y - 23 / d, x - 23 / d, END, END);
	_HPlot(s, y - 23 / d, x - 45 / d, y - 15 / d, x - 53 / d, y - 8 / d, x - 53 / d, y - 15 / d, x - 68 / d, y - 8 / d, x - 75 / d, y, x - 75 / d, END, END);
	_HPlot(s, y, x, y + 15 / d, x, y + 8 / d, x - 8 / d, y + 8 / d, x - 15 / d, y + 15 / d, x - 23 / d, y + 15 / d, x - 15 / d, y + 23 / d, x - 23 / d, END, END);
	_HPlot(s, y + 23 / d, x - 45 / d, y + 15 / d, x - 53 / d, y + 8 / d, x - 53 / d, y + 15 / d, x - 68 / d, y + 8 / d, x - 75 / d, y, x - 75 / d, END, END);
	_HPlot(s, y - 15 / d, x - 68 / d, y + 15 / d, x - 68 / d, END, END);
	_HPlot(s, y - 8 / d, x - 53 / d, y + 8 / d, x - 53 / d, END, END);
	_HPlot(s, y - 23 / d, x - 15 / d, y + 8 / d, x - 45 / d, END, END);
	_HPlot(s, y - 8 / d, x - 68 / d, y, x - 60 / d, y + 8 / d, x - 68 / d, y + 8 / d, x - 60 / d, y - 8 / d, x - 60 / d, y - 8 / d, x - 68 / d, END, END);
	_HPlot(s, y, x - 38 / d, y - 8 / d, x - 38 / d, y + 8 / d, x - 53 / d, y + 8 / d, x - 45 / d, y + 15 / d, x - 45 / d, y, x - 30 / d, y, x - 38 / d, END, END);
}

void Monster::_DRAWViper(Graphics::ManagedSurface *s, double x, double y, double d) {
	_HPlot(s, y - 10 / d, x - 15 / d, y - 10 / d, x - 30 / d, y - 15 / d, x - 20 / d, y - 15 / d, x - 15 / d, y - 15 / d, x, y + 15 / d, x, y + 15 / d, x - 15 / d, y - 15 / d, x - 15 / d, END, END);
	_HPlot(s, y - 15 / d, x - 10 / d, y + 15 / d, x - 10 / d, END, END);
	_HPlot(s, y - 15 / d, x - 5 / d, y + 15 / d, x - 5 / d, END, END);
	_HPlot(s, y, x - 15 / d, y - 5 / d, x - 20 / d, y - 5 / d, x - 35 / d, y + 5 / d, x - 35 / d, y + 5 / d, x - 20 / d, y + 10 / d, x - 15 / d, END, END);
	_HPlot(s, y - 5 / d, x - 20 / d, y + 5 / d, x - 20 / d, END, END);
	_HPlot(s, y - 5 / d, x - 25 / d, y + 5 / d, x - 25 / d, END, END);
	_HPlot(s, y - 5 / d, x - 30 / d, y + 5 / d, x - 30 / d, END, END);
	_HPlot(s, y - 10 / d, x - 35 / d, y - 10 / d, x - 40 / d, y - 5 / d, x - 45 / d, y + 5 / d, x - 45 / d, y + 10 / d, x - 40 / d, y + 10 / d, x - 35 / d, END, END);
	_HPlot(s, y - 10 / d, x - 40 / d, y, x - 45 / d, y + 10 / d, x - 40 / d, END, END);
	_HPlot(s, y - 5 / d, x - 40 / d, y + 5 / d, x - 40 / d, y + 15 / d, x - 30 / d, y, x - 40 / d, y - 15 / d, x - 30 / d, y - 5 / d + .5, x - 40 / d, END, END);
}

void Monster::_DRAWCarrion(Graphics::ManagedSurface *s, double x, double y, double d) {
	/* 79-dst.recty(d) line here */
	_HPlot(s, y - 20 / d, x - 79 / d, y - 20 / d, x - 88 / d, y - 10 / d, x - 83 / d, y + 10 / d, x - 83 / d, y + 20 / d, x - 88 / d, y + 20 / d, x - 79 / d, y - 20 / d, x - 79 / d, END, END);
	_HPlot(s, y - 20 / d, x - 88 / d, y - 30 / d, x - 83 / d, y - 30 / d, x - 78 / d, END, END);
	_HPlot(s, y + 20 / d, x - 88 / d, y + 30 / d, x - 83 / d, y + 40 / d, x - 83 / d, END, END);
	_HPlot(s, y - 15 / d, x - 86 / d, y - 20 / d, x - 83 / d, y - 20 / d, x - 78 / d, y - 30 / d, x - 73 / d, y - 30 / d, x - 68 / d, y - 20 / d, x - 63 / d, END, END);
	_HPlot(s, y - 10 / d, x - 83 / d, y - 10 / d, x - 58 / d, y, x - 50 / d, END, END);
	_HPlot(s, y + 10 / d, x - 83 / d, y + 10 / d, x - 78 / d, y + 20 / d, x - 73 / d, y + 20 / d, x - 40 / d, END, END);
	_HPlot(s, y + 15 / d, x - 85 / d, y + 20 / d, x - 78 / d, y + 30 / d, x - 76 / d, y + 30 / d, x - 60 / d, END, END);
	_HPlot(s, y, x - 83 / d, y, x - 73 / d, y + 10 / d, x - 68 / d, y + 10 / d, x - 63 / d, y, x - 58 / d, END, END);
}

void Monster::_DRAWGremlin(Graphics::ManagedSurface *s, double x, double y, double d) {
	_HPlot(s, y + 5 / d + .5, x - 10 / d, y - 5 / d + .5, x - 10 / d, y, x - 15 / d, y + 10 / d, x - 20 / d, y + 5 / d + .5, x - 15 / d, y + 5 / d + .5, x - 10 / d, END, END);
	_HPlot(s, y + 7 / d + .5, x - 6 / d, y + 5 / d + .5, x - 3 / d, y - 5 / d + .5, x - 3 / d, y - 7 / d + .5, x - 6 / d, y - 5 / d + .5, x - 10 / d, END, END);
	_HPlot(s, y + 2 / d + .5, x - 3 / d, y + 5 / d + .5, x, y + 8 / d, x, END, END);
	_HPlot(s, y - 2 / d + .5, x - 3 / d, y - 5 / d + .5, x, y - 8 / d, x, END, END);
	_HPlot(s, y + 3 / d + .5, x - 8 / d, END, END);
	_HPlot(s, y - 3 / d + .5, x - 8 / d, END, END);
	_HPlot(s, y + 3 / d + .5, x - 5 / d, y - 3 / d + .5, x - 5 / d, END, END);
}

void Monster::_DRAWMimic(Graphics::ManagedSurface *s, double x, double y, double d) {
	double xx = x;
	_HPlot(s, 139 - 10 / d, xx, 139 - 10 / d, xx - 10 / d, 139 + 10 / d, xx - 10 / d, 139 + 10 / d, xx, 139 - 10 / d, xx, END, END);
	_HPlot(s, 139 - 10 / d, xx - 10 / d, 139 - 5 / d, xx - 15 / d, 139 + 15 / d, xx - 15 / d, 139 + 15 / d, xx - 5 / d, 139 + 10 / d, xx, END, END);
	_HPlot(s, 139 + 10 / d, xx - 10 / d, 139 + 15 / d, xx - 15 / d, END, END);
}

void Monster::_DRAWDaemon(Graphics::ManagedSurface *s, double x, double y, double d) {
	_HPlot(s, y - 14 / d, x - 46 / d, y - 12 / d, x - 37 / d, y - 20 / d, x - 32 / d, y - 30 / d, x - 32 / d, y - 22 / d, x - 24 / d, y - 40 / d, x - 17 / d, y - 40 / d, x - 7 / d, y - 38 / d, x - 5 / d, y - 40 / d, x - 3 / d, y - 40 / d, x, END, END);
	_HPlot(s, y - 36 / d, x, y - 34 / d, x - 2 / d, y - 32 / d, x, y - 28 / d, x, y - 28 / d, x - 3 / d, y - 30 / d, x - 5 / d, y - 28 / d, x - 7 / d, y - 28 / d, x - 15 / d, y, x - 27 / d, END, END);
	_HPlot(s, y + 14 / d, x - 46 / d, y + 12 / d, x - 37 / d, y + 20 / d, x - 32 / d, y + 30 / d, x - 32 / d, y + 22 / d, x - 24 / d, y + 40 / d, x - 17 / d, y + 40 / d, x - 7 / d, y + 38 / d, x - 5 / d, y + 40 / d, x - 3 / d, y + 40 / d, x, END, END);
	_HPlot(s, y + 36 / d, x, y + 34 / d, x - 2 / d, y + 32 / d, x, y + 28 / d, x, y + 28 / d, x - 3 / d, y + 30 / d, x - 5 / d, y + 28 / d, x - 7 / d, y + 28 / d, x - 15 / d, y, x - 27 / d, END, END);
	_HPlot(s, y + 6 / d, x - 48 / d, y + 38 / d, x - 41 / d, y + 40 / d, x - 42 / d, y + 18 / d, x - 56 / d, y + 12 / d, x - 56 / d, y + 10 / d, x - 57 / d, y + 8 / d, x - 56 / d, y - 8 / d, x - 56 / d, y - 10 / d, x - 58 / d, y + 14 / d, x - 58 / d, y + 16 / d, x - 59 / d, END, END);
	_HPlot(s, y + 8 / d, x - 63 / d, y + 6 / d, x - 63 / d, y + 2 / d + .5, x - 70 / d, y + 2 / d + .5, x - 63 / d, y - 2 / d + .5, x - 63 / d, y - 2 / d + .5, x - 70 / d, y - 6 / d, x - 63 / d, y - 8 / d, x - 63 / d, y - 16 / d, x - 59 / d, y - 14 / d, x - 58 / d, END, END);
	_HPlot(s, y - 10 / d, x - 57 / d, y - 12 / d, x - 56 / d, y - 18 / d, x - 56 / d, y - 36 / d, x - 47 / d, y - 36 / d, x - 39 / d, y - 28 / d, x - 41 / d, y - 28 / d, x - 46 / d, y - 20 / d, x - 50 / d, y - 18 / d, x - 50 / d, y - 14 / d, x - 46 / d, END, END);
	_HPlot(s, y - 28 / d, x - 41 / d, y + 30 / d, x - 55 / d, END, END);
	_HPlot(s, y + 28 / d, x - 58 / d, y + 22 / d, x - 56 / d, y + 22 / d, x - 53 / d, y + 28 / d, x - 52 / d, y + 34 / d, x - 54 / d, END, END);
	_HPlot(s, y + 20 / d, x - 50 / d, y + 26 / d, x - 47 / d, END, END);
	_HPlot(s, y + 10 / d, x - 58 / d, y + 10 / d, x - 61 / d, y + 4 / d, x - 58 / d, END, END);
	_HPlot(s, y - 10 / d, x - 58 / d, y - 10 / d, x - 61 / d, y - 4 / d, x - 58 / d, END, END);
	_HPlot(s, y + 40 / d, x - 9 / d, y + 50 / d, x - 12 / d, y + 40 / d, x - 7 / d, END, END);
	_HPlot(s, y - 8 / d, x - 25 / d, y + 6 / d, x - 7 / d, y + 28 / d, x - 7 / d, y + 28 / d, x - 9 / d, y + 20 / d, x - 9 / d, y + 6 / d, x - 25 / d, END, END);
}

void Monster::_DRAWBalrog(Graphics::ManagedSurface *s, double x, double y, double d) {
	_HPlot(s, y + 6 / d, x - 60 / d, y + 30 / d, x - 90 / d, y + 60 / d, x - 30 / d, y + 60 / d, x - 10 / d, y + 30 / d, x - 40 / d, y + 15 / d, x - 40 / d, END, END);
	_HPlot(s, y - 6 / d, x - 60 / d, y - 30 / d, x - 90 / d, y - 60 / d, x - 30 / d, y - 60 / d, x - 10 / d, y - 30 / d, x - 40 / d, y - 15 / d, x - 40 / d, END, END);
	_HPlot(s, y, x - 25 / d, y + 6 / d, x - 25 / d, y + 10 / d, x - 20 / d, y + 12 / d, x - 10 / d, y + 10 / d, x - 6 / d, y + 10 / d, x, y + 14 / d, x, y + 15 / d, x - 5 / d, y + 16 / d, x, y + 20 / d, x, END, END);
	_HPlot(s, y + 20 / d, x - 6 / d, y + 18 / d, x - 10 / d, y + 18 / d, x - 20 / d, y + 15 / d, x - 30 / d, y + 15 / d, x - 45 / d, y + 40 / d, x - 60 / d, y + 40 / d, x - 70 / d, END, END);
	_HPlot(s, y + 10 / d, x - 55 / d, y + 6 / d, x - 60 / d, y + 10 / d, x - 74 / d, y + 6 / d, x - 80 / d, y + 4 / d + .5, x - 80 / d, y + 3 / d + .5, x - 82 / d, y + 2 / d + .5, x - 80 / d, y, x - 80 / d, END, END);
	_HPlot(s, y, x - 25 / d, y - 6 / d, x - 25 / d, y - 10 / d, x - 20 / d, y - 12 / d, x - 10 / d, y - 10 / d, x - 6 / d, y - 10 / d, x, y - 14 / d, x, y - 15 / d, x - 5 / d, y - 16 / d, x, y - 20 / d, x, END, END);
	_HPlot(s, y - 20 / d, x - 6 / d, y - 18 / d, x - 10 / d, y - 18 / d, x - 20 / d, y - 15 / d, x - 30 / d, y - 15 / d, x - 45 / d, y - 40 / d, x - 60 / d, y - 40 / d, x - 70 / d, END, END);
	_HPlot(s, y - 10 / d, x - 55 / d, y - 6 / d, x - 60 / d, y - 10 / d, x - 74 / d, y - 6 / d, x - 80 / d, y - 4 / d + .5, x - 80 / d, y - 3 / d + .5, x - 82 / d, y - 2 / d + .5, x - 80 / d, y, x - 80 / d, END, END);
	_HPlot(s, y - 6 / d, x - 25 / d, y, x - 6 / d, y + 10 / d, x, y + 4 / d + .5, x - 8 / d, y + 6 / d, x - 25 / d, END, END);
	_HPlot(s, y - 40 / d, x - 64 / d, y - 40 / d, x - 90 / d, y - 52 / d, x - 80 / d, y - 52 / d, x - 40 / d, END, END);
	_HPlot(s, y + 40 / d, x - 86 / d, y + 38 / d, x - 92 / d, y + 42 / d, x - 92 / d, y + 40 / d, x - 86 / d, y + 40 / d, x - 50 / d, END, END);
	_HPlot(s, y + 4 / d + .5, x - 70 / d, y + 6 / d, x - 74 / d, END, END);
	_HPlot(s, y - 4 / d + .5, x - 70 / d, y - 6 / d, x - 74 / d, END, END);
	_HPlot(s, y, x - 64 / d, y, x - 60 / d, END, END);
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
