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

/****************************
 * Rendering Philips PM5544 test pattern
 *
 * Based on Testcard project (https://sourceforge.net/projects/testcard/)
 *
 * Testpattern is a simple monitor test pattern generator
 * Copyright 2003 Jason Giglio <jgiglio@netmar.com>
 *
 * Licensed under GPLv2+
 ****************************/

#include "base/version.h"

#include "graphics/managed_surface.h"
#include "graphics/fonts/amigafont.h"

namespace Graphics {

enum {
	BLACK = 0, WHITE, RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW,
	COLOR270, COLOR180, COLOR90, COLOR0, COLOR326, COLOR146, RED88,
	GRAY20, GRAY40, GRAY50, GRAY60, GRAY80, GRAY75,
	TRANSCOLOR,
};

const uint32 paletteSrc[] = {
	0x000000, 0xffffff, 0xFF0000, 0x00FF00, 0x0000FF, 0x00FFFF, 0xFF00FF, 0xFFFF00,
	0x00C896, 0x649632, 0xC83264, 0x6464FF, 0x6496FF, 0xC86400, 0xE10000,
	0x333333, 0x666666, 0x7F7F7F, 0x999999, 0xCCCCCC, 0xBFBFBF,
	0xFE00FE,
};

static void boxColor(ManagedSurface *surface, int x1, int y1, int x2, int y2, int color) {
	surface->fillRect(Common::Rect(x1, y1, x2 + 1, y2 + 1), color);
}

static void squaremesh(ManagedSurface *surface, int xres, int yres, int width, int height, int gapsize, int meshcolor, int squarecolor) {
	int centerx = xres / 2;
	int centery = yres / 2;

	boxColor(surface, 0, 0, xres, yres, squarecolor);
	for (int x = centerx % (width + gapsize) + width / 2 - (width + gapsize); x < xres; x += gapsize + width) {
		boxColor(surface, x, 0, x + gapsize - 1, yres - 1, meshcolor);
	}
	for (int y = centery % (height + gapsize) + height / 2 - (height + gapsize); y < yres; y += gapsize + height) {
		boxColor(surface, 0, y, xres - 1, y + gapsize - 1, meshcolor);
	}
}

static void frame(ManagedSurface *surface, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int color) {
	boxColor(surface, x1, y1, x2, y3, color);   //top
	boxColor(surface, x1, y4, x2, y2, color);   //bottom
	boxColor(surface, x1, y1, x3, y2, color);   //left
	boxColor(surface, x4, y1, x2, y2, color);   //right
}

static void drawRow(ManagedSurface *surface, int offsety, int colors[], int numColors, int gap, int vsize, int xres, int offsetx) {
	int boxsize = (xres - (numColors + 1) * gap) / numColors;   //-gaps between boxes
	offsetx += gap + 1;
	for (int x = 0; x < numColors; x++) {
		boxColor(surface, offsetx, offsety, offsetx + boxsize - 1, offsety + vsize - 1, colors[x]);
		offsetx += (boxsize + gap);
	}
}

static void circleColorNoblend(ManagedSurface *surface, double x, double y, double r, int color, bool invert) {
	int sx;

	r += 0.49;
	for (sx = (int)round(x - r); sx < (int)round(x + r) + 1; sx++) {
		double h = sqrt(r * r - pow(sx - x, 2));
		if (invert) {
			surface->vLine(sx, 0, -1 + (int)ceil(y - h), color);
			surface->vLine(sx, 1 + (int)floor(y + h), surface->h, color);
		} else
			surface->vLine(sx, (int)ceil(y - h), (int)floor(y + h), color);
	}
	if (invert && sx < surface->w)
		boxColor(surface, sx, 0, surface->w - 1, surface->h - 1, color);
	if (invert && (int)round(x - r) > 0)
		boxColor(surface, 0, 0, -1 + (int)round(x - r), surface->h - 1, color);
}

static void blacksquareinwhitecircle(ManagedSurface *surface, int x1, int y1, int gapsize, int rwidth, int rheight) {
	double c1y = y1 - (gapsize + 1.0) / 2.0;
	double c1x = x1 - (gapsize + 1.0) / 2.0;
	circleColorNoblend(surface, c1x, c1y, rwidth * 1.5, WHITE, false);
	boxColor(surface, x1 - rwidth - gapsize, y1 - rheight - gapsize, x1 + rwidth - 1, y1 + rheight - 1, BLACK);
	boxColor(surface, x1 - rwidth - gapsize, y1 - gapsize, x1 + rwidth - 1, y1 - 1, WHITE);
	boxColor(surface, x1 - gapsize, y1 - rheight - gapsize, x1 - 1, y1 + rheight - 1, WHITE);
	circleColorNoblend(surface, c1x, c1y, gapsize + 1, WHITE, false);
}

ManagedSurface *renderPM5544(int xres, int yres) {
	byte palette[ARRAYSIZE(paletteSrc) * 3];

	for (int i = 0; i < ARRAYSIZE(paletteSrc); i++) {
		palette[i * 3 + 0] = (paletteSrc[i] >> 16) & 0xFF;
		palette[i * 3 + 1] = (paletteSrc[i] >>  8) & 0xFF;
		palette[i * 3 + 2] = (paletteSrc[i]	     ) & 0xFF;
	}

	ManagedSurface *surface = new ManagedSurface(xres, yres, PixelFormat::createFormatCLUT8());

    surface->setPalette(palette, 0, ARRAYSIZE(palette) / 3);

	int squaresize = 8;
	int gapsize = 2;

	int width = 0, height = 0;
	int rwidth = 0, rheight = 0;
	int xsquares = 0, ysquares = 0;

	int xcenter = xres / 2 - xres % 2;

	bool cont = true;
	while (cont) {

		if (squaresize > 8) {
			if (xsquares < 6 || ysquares < 6) {
				squaresize -= 4;
				cont = false;
			}
		}

		squaresize += 2;

		width = xres % 2 + squaresize;
		height = yres % 2 + squaresize;

		rwidth = width + gapsize;
		rheight = height + gapsize;

		xsquares = (xres - rwidth - gapsize - 16) / rwidth / 2;
		ysquares = (yres - rheight - gapsize - 16) / rheight / 2;

	}

	squaremesh(surface, xres, yres, width, height, gapsize, WHITE, GRAY50);

	int startx = xres / 2 - width / 2 - width % 2 - xsquares * rwidth - gapsize - 1;
	int starty = yres / 2 - height / 2 - height % 2 - ysquares * rheight - gapsize - 1;
	int endx = startx + rwidth * (2 * xsquares + 1) + gapsize + 1;
	int endy = starty + rheight * (2 * ysquares + 1) + gapsize + 1;
	int i;

	frame(surface, 0, 0, xres, yres, 1, 1, xres - 2, yres - 2, WHITE);
	frame(surface, 2, 2, xres - 3, yres - 3, startx, starty, endx, endy, BLACK);

	for (i = 0; i < ysquares * 2 + 1; i++) {
		if (i % 2 == 0) {
			boxColor(surface, 0, starty + gapsize + i * rheight, startx - 2, starty + gapsize + (i + 1) * rheight - 1, WHITE);
			boxColor(surface, endx + 2, starty + gapsize + i * rheight, xres - 1, starty + gapsize + (i + 1) * rheight - 1, WHITE);
		}

	}
	for (i = 0; i < xsquares * 2 + 1; i++) {
		if (i % 2 == 0) {
			boxColor(surface, startx + gapsize + i * rwidth, 0, startx + gapsize + (i + 1) * rwidth - 1, starty - 2, WHITE);
			boxColor(surface, startx + gapsize + i * rwidth, endy + 2, startx + gapsize + (i + 1) * rwidth - 1, yres - 1, WHITE);
		}

	}

	int monosize = 6;
	int x1 = xres / 2 - width / 2 - width % 2 - rwidth * monosize - rwidth; //left
	int y1 = yres / 2 - height / 2 - height % 2 - rheight * (monosize - 1); //up
	int x2 = x1 + rwidth * (monosize * 2 + 2) + width - 1;  //right
	int y2 = y1 + (monosize * 2 - 1) * rheight - gapsize - 1;   //bottom

	if (xsquares - 1 > monosize) {
		boxColor(surface, x1, y1, x1 + width - 1, yres / 2, COLOR270);	//up left
		boxColor(surface, x2 - width + 1, y1, x2, yres / 2, COLOR180);	//up right
		boxColor(surface, x1, yres / 2 + yres % 2, x1 + width - 1, y2, COLOR90); //bottom left
		boxColor(surface, x2 - width + 1, yres / 2 + yres % 2, x2, y2, COLOR0); //bottom right
		if (monosize > 2) {
			boxColor(surface, x1 + width, y1, x1 + rwidth + width - 1, y1 + 2 * rheight - gapsize - 1, COLOR326); //up left small
			boxColor(surface, x1 + width, y2 - rheight * 2 + gapsize + 1, x1 + rwidth + width - 1, y2, COLOR146); //bottom left small
			boxColor(surface, x2 - rwidth - width + 1, y1, x2 - width, y1 + 2 * rheight - gapsize - 1, COLOR326); //up right small
			boxColor(surface, x2 - rwidth - width + 1, y2 - rheight * 2 + gapsize + 1, x2 - width, y2, COLOR146); //bottom right small
		}
	}

	if (xsquares - 4 > monosize) {
		blacksquareinwhitecircle(surface, x1 - rwidth * 2, y1 + rheight, gapsize, rwidth, rheight);
		blacksquareinwhitecircle(surface, x2 + rwidth * 2 + gapsize + 1, y1 + rheight, gapsize, rwidth, rheight);
		blacksquareinwhitecircle(surface, x1 - rwidth * 2, y2 - height + 1, gapsize, rwidth, rheight);
		blacksquareinwhitecircle(surface, x2 + rwidth * 2 + gapsize + 1, y2 - height + 1, gapsize, rwidth, rheight);
	}

	int monoradius = monosize * rwidth + gapsize;
	ManagedSurface *monoscope = new ManagedSurface(xres, yres, PixelFormat::createFormatCLUT8());
	monoscope->setTransparentColor(TRANSCOLOR);

	// two bottom rows
	boxColor(monoscope, x1, y2 - rheight + gapsize / 2 + 1, x2, y2 + rheight + gapsize / 2 + 1, YELLOW);
	boxColor(monoscope, xcenter - rwidth / 2, y2 - rheight + gapsize / 2 + 1, xcenter - rwidth / 2 + rwidth, y2 + rheight + gapsize / 2 + 1, RED88);

	boxColor(monoscope, x1, y2 - 2 * rheight + gapsize / 2 + 1, x2, y2 - rheight + gapsize / 2, WHITE);
	boxColor(monoscope, xres / 2 - rwidth * 3, y2 - 2 * rheight + gapsize / 2 + 1, xres / 2 + rwidth * 3, y2 - rheight + gapsize / 2, BLACK);

	// two top rows
	boxColor(monoscope, x1, y2 - 12 * rheight + gapsize / 2 + 1, x2, y2 - rheight * 10 + gapsize / 2, WHITE);
	boxColor(monoscope, xres / 2 - rwidth * 2, y2 - 11 * rheight + gapsize / 2 + 1, xres / 2 + rwidth * 2, y2 - rheight * 10 + gapsize / 2, BLACK);

	boxColor(monoscope, x1, y2 - 10 * rheight + gapsize / 2 + 1, x2, y2 - rheight * 9 + gapsize / 2, BLACK);
	boxColor(monoscope, xres / 2 - rwidth * 3, y2 - 10 * rheight + gapsize / 2 + 1, xres / 2 + rwidth * 3, y2 - rheight * 9 + gapsize / 2, WHITE);

	// color (gray) bars
	int greyColors[] = { BLACK, GRAY20, GRAY40, GRAY60, GRAY80, WHITE};
	drawRow(monoscope, y2 - 3 * rheight + gapsize / 2 + 1, greyColors, 6, 0, rheight, monosize * (rwidth * 2) + 6, xres / 2 - rwidth * monosize - 4);

	boxColor(monoscope, x1, y2 - 6 * rheight + gapsize / 2 + 1, x2, y2 - 3 * rheight + gapsize / 2, BLACK);

	int topColors[] = { YELLOW, CYAN, GREEN, MAGENTA, RED, BLUE };
	drawRow(monoscope, y2 - 8 * rheight + gapsize / 2 + 1, topColors, 6, 0, 2 * rheight, monosize * (rwidth * 2) + 6, xres / 2 - rwidth * monosize - 4);

	// periodic gray and white
	int periodsize = monosize * (rwidth * 2) * 11 / 16 / 6;
	int blacksize = periodsize * 6 / 11;
	boxColor(monoscope, x1, y2 - 9 * rheight + gapsize / 2 + 1, x2, y2 - 8 * rheight + gapsize / 2, GRAY75);
	for (i = 0; i < 5; i++) {
		boxColor(monoscope, xres / 2 + i * periodsize, y2 - 9 * rheight + gapsize / 2 + 1, xres / 2 + i * periodsize + blacksize - 1, y2 - 8 * rheight + gapsize / 2, BLACK);
		boxColor(monoscope, xres / 2 - (i + 1) * periodsize, y2 - 9 * rheight + gapsize / 2 + 1, xres / 2 - (i + 1) * periodsize + blacksize - 1, y2 - 8 * rheight + gapsize / 2, BLACK);
	}

	// small lines
	int xmin = xcenter - rwidth / 2 - rwidth * 4;
	int xmax = xcenter - rwidth / 2 + rwidth * 5;
	int step;
	int alternatingColors[] = { WHITE, BLACK };
	int blackorwhite = 0;
	for (i = xmin; i < xmax; i++) {
		step = 6 * (xmax - i - 1) / (xmax - xmin) + 1;
		boxColor(monoscope, i, y2 - 5 * rheight + gapsize / 2 + 1, i + step - 1, y2 - 3 * rheight + gapsize / 2, alternatingColors[blackorwhite++ % 2]);
		i += step - 1;
	}

	// horizontal middle part
	boxColor(monoscope, xcenter - rwidth / 2, y2 - 7 * rheight + gapsize / 2 + 1, xres / 2 + xres % 2 + rwidth / 2 - 1, y2 - 4 * rheight + gapsize / 2, BLACK);
	boxColor(monoscope, xres / 2 + xres % 2 - 1, y2 - 7 * rheight + gapsize / 2 + 1, xres / 2, y2 - 4 * rheight + gapsize / 2, WHITE);
	boxColor(monoscope, x1, yres / 2 + yres % 2 - 1, x2, yres / 2, WHITE);

	// vertical lines in the middle
	for (i = x1 - gapsize; i < x2 + gapsize; i += rwidth) {
		boxColor(monoscope, i, y2 - 6 * rheight + gapsize / 2 + 1, i + gapsize - 1, y2 - 5 * rheight + gapsize / 2, WHITE);
	}

	circleColorNoblend(monoscope, (xres - 1.0) / 2.0, (yres - 1.0) / 2.0, 3, WHITE, false);
	circleColorNoblend(monoscope, (xres - 1.0) / 2.0, (yres - 1.0) / 2.0, monoradius, TRANSCOLOR, true);

	surface->simpleBlitFrom(*monoscope);

	delete monoscope;

	Graphics::AmigaFont font;

	y2 += (rheight - font.getFontHeight()) / 2;

	font.drawString(surface, "ScummVM", xres / 2 - rwidth * 2, y2 - 11 * rheight + gapsize / 2 + 1, rwidth * 4, WHITE, Graphics::kTextAlignCenter);
	font.drawString(surface, gScummVMVersion, xres / 2 - rwidth * 3, y2 - 2 * rheight + gapsize / 2 + 1, rwidth * 6, WHITE);

    return surface;
}

}  // End of namespace Graphics
