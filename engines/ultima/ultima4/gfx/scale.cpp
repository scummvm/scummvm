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

#include "ultima/ultima4/gfx/image.h"
#include "ultima/ultima4/gfx/scale.h"
#include "ultima/ultima4/core/utils.h"

namespace Ultima {
namespace Ultima4 {

Image *scalePoint(Image *src, int scale, int n);
Image *scale2xBilinear(Image *src, int scale, int n);
Image *scale2xSaI(Image *src, int scale, int N);
Image *scaleScale2x(Image *src, int scale, int N);

Scaler scalerGet(const Common::String &filter) {
	if (filter == "point")
		return &scalePoint;
	else if (filter == "2xBi")
		return &scale2xBilinear;
	else if (filter == "2xSaI")
		return &scale2xSaI;
	else if (filter == "Scale2x")
		return &scaleScale2x;
	else
		return nullptr;
}

/**
 * Returns true if the given scaler can scale by 3 (as well as by 2).
 */
int scaler3x(const Common::String &filter) {
	return filter == "Scale2x";
}

/**
 * A simple row and column duplicating scaler.
 */
Image *scalePoint(Image *src, int scale, int n) {
	int x, y, i, j;
	Image *dest;

	dest = Image::create(src->width() * scale, src->height() * scale, src->isIndexed(), Image::HARDWARE);
	if (!dest)
		return nullptr;

	if (dest->isIndexed())
		dest->setPaletteFromImage(src);

	for (y = 0; y < src->height(); y++) {
		for (x = 0; x < src->width(); x++) {
			for (i = 0; i < scale; i++) {
				for (j = 0; j < scale; j++) {
					uint index;
					src->getPixelIndex(x, y, index);
					dest->putPixelIndex(x * scale + j, y * scale + i, index);
				}
			}
		}
	}

	return dest;
}

/**
 * A scaler that interpolates each intervening pixel from it's two
 * neighbors.
 */
Image *scale2xBilinear(Image *src, int scale, int n) {
	int i, x, y, xoff, yoff;
	RGBA a, b, c, d;
	Image *dest;

	/* this scaler works only with images scaled by 2x */
	assertMsg(scale == 2, "invalid scale: %d", scale);

	dest = Image::create(src->width() * scale, src->height() * scale, false, Image::HARDWARE);
	if (!dest)
		return nullptr;

	/*
	 * Each pixel in the source image is translated into four in the
	 * destination.  The destination pixels are dependant on the pixel
	 * itself, and the three surrounding pixels (A is the original
	 * pixel):
	 * A B
	 * C D
	 * The four destination pixels mapping to A are calculated as
	 * follows:
	 * [   A   ] [  (A+B)/2  ]
	 * [(A+C)/2] [(A+B+C+D)/4]
	 */

	for (i = 0; i < n; i++) {
		for (y = (src->height() / n) * i; y < (src->height() / n) * (i + 1); y++) {
			if (y == (src->height() / n) * (i + 1) - 1)
				yoff = 0;
			else
				yoff = 1;

			for (x = 0; x < src->width(); x++) {
				if (x == src->width() - 1)
					xoff = 0;
				else
					xoff = 1;

				src->getPixel(x, y, a.r, a.g, a.b, a.a);
				src->getPixel(x + xoff, y, b.r, b.g, b.b, b.a);
				src->getPixel(x, y + yoff, c.r, c.g, c.b, c.a);
				src->getPixel(x + xoff, y + yoff, d.r, d.g, d.b, d.a);

				dest->putPixel(x * 2, y * 2, a.r, a.g, a.b, a.a);
				dest->putPixel(x * 2 + 1, y * 2, (a.r + b.r) >> 1, (a.g + b.g) >> 1, (a.b + b.b) >> 1, (a.a + b.a) >> 1);
				dest->putPixel(x * 2, y * 2 + 1, (a.r + c.r) >> 1, (a.g + c.g) >> 1, (a.b + c.b) >> 1, (a.a + c.a) >> 1);
				dest->putPixel(x * 2 + 1, y * 2 + 1, (a.r + b.r + c.r + d.r) >> 2, (a.g + b.g + c.g + d.g) >> 2, (a.b + b.b + c.b + d.b) >> 2, (a.a + b.a + c.a + d.a) >> 2);
			}
		}
	}

	return dest;
}

int colorEqual(RGBA a, RGBA b) {
	return
	    a.r == b.r &&
	    a.g == b.g &&
	    a.b == b.b &&
	    a.a == b.a;
}

RGBA colorAverage(RGBA a, RGBA b) {
	RGBA result;
	result.r = (a.r + b.r) >> 1;
	result.g = (a.g + b.g) >> 1;
	result.b = (a.b + b.b) >> 1;
	result.a = (a.a + b.a) >> 1;
	return result;
}

int _2xSaI_GetResult1(RGBA a, RGBA b, RGBA c, RGBA d) {
	int x = 0;
	int y = 0;
	int r = 0;
	if (colorEqual(a, c)) x++;
	else if (colorEqual(b, c)) y++;
	if (colorEqual(a, d)) x++;
	else if (colorEqual(b, d)) y++;
	if (x <= 1) r++;
	if (y <= 1) r--;
	return r;
}

int _2xSaI_GetResult2(RGBA a, RGBA b, RGBA c, RGBA d) {
	int x = 0;
	int y = 0;
	int r = 0;
	if (colorEqual(a, c)) x++;
	else if (colorEqual(b, c)) y++;
	if (colorEqual(a, d)) x++;
	else if (colorEqual(b, d)) y++;
	if (x <= 1) r--;
	if (y <= 1) r++;
	return r;
}

/**
 * A more sophisticated scaler that interpolates each new pixel the
 * surrounding pixels.
 */
Image *scale2xSaI(Image *src, int scale, int N) {
	int ii, x, y, xoff0, xoff1, xoff2, yoff0, yoff1, yoff2;
	RGBA a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
	RGBA prod0, prod1, prod2;
	Image *dest;

	/* this scaler works only with images scaled by 2x */
	assertMsg(scale == 2, "invalid scale: %d", scale);

	dest = Image::create(src->width() * scale, src->height() * scale, false, Image::HARDWARE);
	if (!dest)
		return nullptr;

	/*
	 * Each pixel in the source image is translated into four in the
	 * destination.  The destination pixels are dependant on the pixel
	 * itself, and the surrounding pixels as shown below (A is the
	 * original pixel):
	 * I E F J
	 * G A B K
	 * H C D L
	 * M N O P
	 */

	for (ii = 0; ii < N; ii++) {
		for (y = (src->height() / N) * ii; y < (src->height() / N) * (ii + 1); y++) {
			if (y == 0)
				yoff0 = 0;
			else
				yoff0 = -1;
			if (y == (src->height() / N) * (ii + 1) - 1) {
				yoff1 = 0;
				yoff2 = 0;
			} else if (y == (src->height() / N) * (ii + 1) - 2) {
				yoff1 = 1;
				yoff2 = 1;
			} else {
				yoff1 = 1;
				yoff2 = 2;
			}


			for (x = 0; x < src->width(); x++) {
				if (x == 0)
					xoff0 = 0;
				else
					xoff0 = -1;
				if (x == src->width() - 1) {
					xoff1 = 0;
					xoff2 = 0;
				} else if (x == src->width() - 2) {
					xoff1 = 1;
					xoff2 = 1;
				} else {
					xoff1 = 1;
					xoff2 = 2;
				}

				src->getPixel(x, y, a.r, a.g, a.b, a.a);
				src->getPixel(x + xoff1, y, b.r, b.g, b.b, b.a);
				src->getPixel(x, y + yoff1, c.r, c.g, c.b, c.a);
				src->getPixel(x + xoff1, y + yoff1, d.r, d.g, d.b, d.a);

				src->getPixel(x, y + yoff0, e.r, e.g, e.b, e.a);
				src->getPixel(x + xoff1, y + yoff0, f.r, f.g, f.b, f.a);
				src->getPixel(x + xoff0, y, g.r, g.g, g.b, g.a);
				src->getPixel(x + xoff0, y + yoff1, h.r, h.g, h.b, h.a);

				src->getPixel(x + xoff0, y + yoff0, i.r, i.g, i.b, i.a);
				src->getPixel(x + xoff2, y + yoff0, j.r, j.g, j.b, j.a);
				src->getPixel(x + xoff0, y, k.r, k.g, k.b, k.a);
				src->getPixel(x + xoff0, y + yoff1, l.r, l.g, l.b, l.a);

				src->getPixel(x + xoff0, y + yoff2, m.r, m.g, m.b, m.a);
				src->getPixel(x, y + yoff2, n.r, n.g, n.b, n.a);
				src->getPixel(x + xoff1, y + yoff2, o.r, o.g, o.b, o.a);
				src->getPixel(x + xoff2, y + yoff2, p.r, p.g, p.b, p.a);

				if (colorEqual(a, d) && !colorEqual(b, c)) {
					if ((colorEqual(a, e) && colorEqual(b, l)) ||
					        (colorEqual(a, c) && colorEqual(a, f) && !colorEqual(b, e) && colorEqual(b, j)))
						prod0 = a;
					else
						prod0 = colorAverage(a, b);

					if ((colorEqual(a, g) && colorEqual(c, o)) ||
					        (colorEqual(a, b) && colorEqual(a, h) && !colorEqual(g, c) && colorEqual(c, m)))
						prod1 = a;
					else
						prod1 = colorAverage(a, c);

					prod2 = a;
				} else if (colorEqual(b, c) && !colorEqual(a, d)) {
					if ((colorEqual(b, f) && colorEqual(a, h)) ||
					        (colorEqual(b, e) && colorEqual(b, d) && !colorEqual(a, f) && colorEqual(a, i)))
						prod0 = b;
					else
						prod0 = colorAverage(a, b);

					if ((colorEqual(c, h) && colorEqual(a, f)) ||
					        (colorEqual(c, g) && colorEqual(c, d) && !colorEqual(a, h) && colorEqual(a, i)))
						prod1 = c;
					else
						prod1 = colorAverage(a, c);

					prod2 = b;
				} else if (colorEqual(a, d) && colorEqual(b, c)) {
					if (colorEqual(a, b))
						prod0 = prod1 = prod2 = a;
					else {
						int r = 0;
						prod0 = colorAverage(a, b);
						prod1 = colorAverage(a, c);

						r += _2xSaI_GetResult1(a, b, g, e);
						r += _2xSaI_GetResult2(b, a, k, f);
						r += _2xSaI_GetResult2(b, a, h, n);
						r += _2xSaI_GetResult1(a, b, l, o);

						if (r > 0)
							prod2 = a;
						else if (r < 0)
							prod2 = b;
						else {
							prod2.r = (a.r + b.r + c.r + d.r) >> 2;
							prod2.g = (a.g + b.g + c.g + d.g) >> 2;
							prod2.b = (a.b + b.b + c.b + d.b) >> 2;
						}
					}
				} else {
					if (colorEqual(a, c) && colorEqual(a, f) && !colorEqual(b, e) && colorEqual(b, j))
						prod0 = a;
					else if (colorEqual(b, e) && colorEqual(b, d) && !colorEqual(a, f) && colorEqual(a, i))
						prod0 = b;
					else
						prod0 = colorAverage(a, b);

					if (colorEqual(a, b) && colorEqual(a, h) && !colorEqual(g, c) && colorEqual(c, m))
						prod1 = a;
					else if (colorEqual(c, g) && colorEqual(c, d) && !colorEqual(a, h) && colorEqual(a, i))
						prod1 = c;
					else
						prod1 = colorAverage(a, c);

					prod2.r = (a.r + b.r + c.r + d.r) >> 2;
					prod2.g = (a.g + b.g + c.g + d.g) >> 2;
					prod2.b = (a.b + b.b + c.b + d.b) >> 2;
					prod2.a = 255;
				}

				dest->putPixel((x << 1), (y << 1), a.r, a.g, a.b, a.a);
				dest->putPixel((x << 1) + 1, (y << 1), prod0.r, prod0.g, prod0.b, prod0.a);
				dest->putPixel((x << 1), (y << 1) + 1, prod1.r, prod1.g, prod1.b, prod1.a);
				dest->putPixel((x << 1) + 1, (y << 1) + 1, prod2.r, prod2.g, prod2.b, prod2.a);
			}
		}
	}

	return dest;
}

/**
 * A more sophisticated scaler that doesn't interpolate, but avoids
 * the stair step effect by detecting angles.
 */
Image *scaleScale2x(Image *src, int scale, int n) {
	int ii, x, y, xoff0, xoff1, yoff0, yoff1;
	RGBA a, b, c, d, e, f, g, h, i;
	RGBA e0, e1, e2, e3;
	RGBA e4, e5, e6, e7;
	Image *dest;

	/* this scaler works only with images scaled by 2x or 3x */
	assertMsg(scale == 2 || scale == 3, "invalid scale: %d", scale);

	dest = Image::create(src->width() * scale, src->height() * scale, src->isIndexed(), Image::HARDWARE);
	if (!dest)
		return nullptr;

	if (dest->isIndexed())
		dest->setPaletteFromImage(src);

	/*
	 * Each pixel in the source image is translated into four (or
	 * nine) in the destination.  The destination pixels are dependant
	 * on the pixel itself, and the eight surrounding pixels (E is the
	 * original pixel):
	 *
	 * A B C
	 * D E F
	 * G H I
	 */

	for (ii = 0; ii < n; ii++) {
		for (y = (src->height() / n) * ii; y < (src->height() / n) * (ii + 1); y++) {
			if (y == 0)
				yoff0 = 0;
			else
				yoff0 = -1;
			if (y == (src->height() / n) * (ii + 1) - 1)
				yoff1 = 0;
			else
				yoff1 = 1;

			for (x = 0; x < src->width(); x++) {
				if (x == 0)
					xoff0 = 0;
				else
					xoff0 = -1;
				if (x == src->width() - 1)
					xoff1 = 0;
				else
					xoff1 = 1;

				src->getPixel(x + xoff0, y + yoff0, a.r, a.g, a.b, a.a);
				src->getPixel(x, y + yoff0, b.r, b.g, b.b, b.a);
				src->getPixel(x + xoff1, y + yoff0, c.r, c.g, c.b, c.a);

				src->getPixel(x + xoff0, y, d.r, d.g, d.b, d.a);
				src->getPixel(x, y, e.r, e.g, e.b, e.a);
				src->getPixel(x + xoff1, y, f.r, f.g, f.b, f.a);

				src->getPixel(x + xoff0, y + yoff1, g.r, g.g, g.b, g.a);
				src->getPixel(x, y + yoff1, h.r, h.g, h.b, h.a);
				src->getPixel(x + xoff1, y + yoff1, i.r, i.g, i.b, i.a);

				// lissen diagonals (45°,135°,225°,315°)
				// corner : if there is gradient towards a diagonal direction,
				// take the color of surrounding points in this direction
				e0 = colorEqual(d, b) && (!colorEqual(b, f)) && (!colorEqual(d, h)) ? d : e;
				e1 = colorEqual(b, f) && (!colorEqual(b, d)) && (!colorEqual(f, h)) ? f : e;
				e2 = colorEqual(d, h) && (!colorEqual(d, b)) && (!colorEqual(h, f)) ? d : e;
				e3 = colorEqual(h, f) && (!colorEqual(d, h)) && (!colorEqual(b, f)) ? f : e;

				// lissen eight more directions (22° or 67°, 112° or 157°...)
				// middle of side : if there is a gradient towards one of these directions (middle of side direction and of direction of either diagonal around this side),
				// take the color of surrounding points in this direction
				e4 = colorEqual(e0, c) ? e0 : colorEqual(e1, a) ? e1 : e;
				e5 = colorEqual(e2, a) ? e2 : colorEqual(e0, g) ? e0 : e;
				e6 = colorEqual(e1, i) ? e1 : colorEqual(e3, c) ? e3 : e;
				e7 = colorEqual(e3, g) ? e3 : colorEqual(e2, i) ? e2 : e;

				if (scale == 2) {
					dest->putPixel(x * 2, y * 2, e0.r, e0.g, e0.b, e0.a);
					dest->putPixel(x * 2 + 1, y * 2, e1.r, e1.g, e1.b, e1.a);
					dest->putPixel(x * 2, y * 2 + 1, e2.r, e2.g, e2.b, e2.a);
					dest->putPixel(x * 2 + 1, y * 2 + 1, e3.r, e3.g, e3.b, e3.a);
				} else if (scale == 3) {
					dest->putPixel(x * 3, y * 3, e0.r, e0.g, e0.b, e0.a);
					dest->putPixel(x * 3 + 1, y * 3, e4.r, e4.g, e4.b, e4.a);
					dest->putPixel(x * 3 + 2, y * 3, e1.r, e1.g, e1.b, e1.a);
					dest->putPixel(x * 3, y * 3 + 1, e5.r, e5.g, e5.b, e5.a);
					dest->putPixel(x * 3 + 1, y * 3 + 1, e.r, e.g, e.b, e.a);
					dest->putPixel(x * 3 + 2, y * 3 + 1, e6.r, e6.g, e6.b, e6.a);
					dest->putPixel(x * 3, y * 3 + 2, e2.r, e2.g, e2.b, e2.a);
					dest->putPixel(x * 3 + 1, y * 3 + 2, e7.r, e7.g, e7.b, e7.a);
					dest->putPixel(x * 3 + 2, y * 3 + 2, e3.r, e3.g, e3.b, e3.a);

				}
			}
		}
	}

	return dest;
}

} // End of namespace Ultima4
} // End of namespace Ultima
