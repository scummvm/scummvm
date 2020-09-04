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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "graphics/managed_surface.h"
#include "graphics/primitives.h"
#include "graphics/macgui/macwindowmanager.h"

#include "wage/design.h"

namespace Wage {

struct PlotData {
	Graphics::ManagedSurface *surface;
	Graphics::MacPatterns *patterns;
	uint fillType;
	int thickness;
	Design *design;

	PlotData(Graphics::ManagedSurface *s, Graphics::MacPatterns *p, int f, int t, Design *d) :
		surface(s), patterns(p), fillType(f), thickness(t), design(d) {}
};

void drawPixel(int x, int y, int color, void *data);
void drawPixelPlain(int x, int y, int color, void *data);

Design::Design(Common::SeekableReadStream *data) {
	_len = data->readUint16BE() - 2;
	_data = (byte *)malloc(_len);
	data->read(_data, _len);

	_surface = NULL;
	_bounds = NULL;

	_boundsCalculationMode = false;
}

Design::~Design() {
	free(_data);
	if (_surface)
		_surface->free();
	delete _surface;
}

void Design::paint(Graphics::ManagedSurface *surface, Graphics::MacPatterns &patterns, int x, int y) {
	bool needRender = false;

	if (_surface == NULL) {
		_boundsCalculationMode = true;
		_bounds->debugPrint(4, "Internal bounds:");
		render(patterns);
		_boundsCalculationMode = false;
		if (_bounds->right == -10000) {
			_bounds->left = _bounds->top = _bounds->right = _bounds->bottom = 0;
		}
		_bounds->debugPrint(4, "Calculated bounds:");

		_surface = new Graphics::ManagedSurface;
		_surface->create(_bounds->width(), _bounds->height(), Graphics::PixelFormat::createFormatCLUT8());

		_surface->clear(kColorGreen);

		needRender = true;
	}

	_bounds->debugPrint(4, "Using bounds:");
#if 0
	PlotData pd(_surface, &patterns, 8, 1, this);
	int x1 = 50, y1 = 50, x2 = 200, y2 = 200, borderThickness = 30;
	Common::Rect inn(x1-5, y1-5, x2+5, y2+5);
	drawRoundRect(inn, 6, kColorGray80, false, drawPixelPlain, &pd);

	drawThickLine(x1, y1, x2-borderThickness, y1, borderThickness, kColorBlack, drawPixel, &pd);
	drawThickLine(x2-borderThickness, y1, x2-borderThickness, y2, borderThickness, kColorBlack, drawPixel, &pd);
	drawThickLine(x2-borderThickness, y2-borderThickness, x1, y2-borderThickness, borderThickness, kColorBlack, drawPixel, &pd);
	drawThickLine(x1, y2-borderThickness, x1, y1, borderThickness, kColorBlack, drawPixel, &pd);
	drawThickLine(x2+10, y2+10, x2+100, y2+100, borderThickness, kColorBlack, drawPixel, &pd);

	g_system->copyRectToScreen(_surface->getPixels(), _surface->pitch, 0, 0, _surface->w, _surface->h);

	while (true) {
		((WageEngine *)g_engine)->processEvents();
		g_system->updateScreen();
		g_system->delayMillis(50);
	}
	return;
#endif

	if (needRender)
		render(patterns);

	if (_bounds->width() && _bounds->height()) {
		const int padding = 3;
		Common::Rect from(padding, padding, _bounds->width() - 2 * padding, _bounds->height() - 2 * padding);
		Common::Rect to(from);
		to.moveTo(x, y);
		surface->transBlitFrom(*_surface, from, to, kColorGreen);
	}
}

void Design::render(Graphics::MacPatterns &patterns) {
	Common::MemoryReadStream in(_data, _len);
	bool needRender = true;

	while (needRender) {
		byte fillType = in.readByte();
		byte borderThickness = in.readByte();
		byte borderFillType = in.readByte();
		int type = in.readByte();

		if (in.eos())
			break;

		debug(8, "fill: %d borderFill: %d border: %d type: %d", fillType, borderFillType, borderThickness, type);
		switch (type) {
		case 4:
			drawRect(_surface, in, patterns, fillType, borderThickness, borderFillType);
			break;
		case 8:
			drawRoundRect(_surface, in, patterns, fillType, borderThickness, borderFillType);
			break;
		case 12:
			drawOval(_surface, in, patterns, fillType, borderThickness, borderFillType);
			break;
		case 16:
		case 20:
			drawPolygon(_surface, in, patterns, fillType, borderThickness, borderFillType);
			break;
		case 24:
			drawBitmap(_surface, in);
			break;
		default:
			warning("Unknown type => %d", type);
			break;
		}

		//g_system->copyRectToScreen(_surface->getPixels(), _surface->pitch, 0, 0, _surface->w, _surface->h);
		//((WageEngine *)g_engine)->processEvents();
		//g_system->updateScreen();
		//g_system->delayMillis(500);
	}
}

bool Design::isPointOpaque(int x, int y) {
	if (_surface == NULL)
		error("Surface is null");

	byte pixel = ((byte *)_surface->getBasePtr(x, y))[0];

	return pixel != kColorGreen;
}

void Design::adjustBounds(int16 x, int16 y) {
	_bounds->right  = MAX(x, _bounds->right);
	_bounds->bottom = MAX(y, _bounds->bottom);
}

void drawPixel(int x, int y, int color, void *data) {
	PlotData *p = (PlotData *)data;

	if (p->fillType > p->patterns->size())
		return;

	if (p->design && p->design->isBoundsCalculation()) {
		if (x < 0 || y < 0)
			return;
		if (p->thickness == 1) {
			p->design->adjustBounds(x, y);
		} else {
			int x1 = x;
			int x2 = x1 + p->thickness;
			int y1 = y;
			int y2 = y1 + p->thickness;

			for (y = y1; y < y2; y++)
				for (x = x1; x < x2; x++)
					p->design->adjustBounds(x, y);
		}

		return;
	}

	byte *pat = p->patterns->operator[](p->fillType - 1);

	if (p->thickness == 1) {
		if (x >= 0 && x < p->surface->w && y >= 0 && y < p->surface->h) {
			uint xu = (uint)x; // for letting compiler optimize it
			uint yu = (uint)y;

			*((byte *)p->surface->getBasePtr(xu, yu)) =
				(pat[yu % 8] & (1 << (7 - xu % 8))) ?
					color : kColorWhite;
		}
	} else {
		int x1 = x - p->thickness / 2;
		int x2 = x1 + p->thickness;
		int y1 = y - p->thickness / 2;
		int y2 = y1 + p->thickness;

		for (y = y1; y < y2; y++)
			for (x = x1; x < x2; x++)
				if (x >= 0 && x < p->surface->w && y >= 0 && y < p->surface->h) {
					uint xu = (uint)x; // for letting compiler optimize it
					uint yu = (uint)y;
					*((byte *)p->surface->getBasePtr(xu, yu)) =
						(pat[yu % 8] & (1 << (7 - xu % 8))) ?
							color : kColorWhite;
				}
	}
}

void drawPixelPlain(int x, int y, int color, void *data) {
	PlotData *p = (PlotData *)data;

	if (p->design && p->design->isBoundsCalculation()) {
		p->design->adjustBounds(x, y);
		return;
	}

	if (x >= 0 && x < p->surface->w && y >= 0 && y < p->surface->h)
		*((byte *)p->surface->getBasePtr(x, y)) = (byte)color;
}

void Design::drawRect(Graphics::ManagedSurface *surface, Common::ReadStream &in,
				Graphics::MacPatterns &patterns, byte fillType, byte borderThickness, byte borderFillType) {
	int16 y1 = in.readSint16BE();
	int16 x1 = in.readSint16BE();
	int16 y2 = in.readSint16BE();
	int16 x2 = in.readSint16BE();

	if (x1 > x2)
		SWAP(x1, x2);
	if (y1 > y2)
		SWAP(y1, y2);

	Common::Rect r(x1, y1, x2, y2);
	PlotData pd(surface, &patterns, fillType, 1, this);

	if (fillType <= patterns.size())
		Graphics::drawFilledRect(r, kColorBlack, drawPixel, &pd);

	pd.fillType = borderFillType;
	pd.thickness = borderThickness;

	if (borderThickness > 0 && borderFillType <= patterns.size()) {
		Graphics::drawLine(x1, y1, x2, y1, kColorBlack, drawPixel, &pd);
		Graphics::drawLine(x2, y1, x2, y2, kColorBlack, drawPixel, &pd);
		Graphics::drawLine(x2, y2, x1, y2, kColorBlack, drawPixel, &pd);
		Graphics::drawLine(x1, y2, x1, y1, kColorBlack, drawPixel, &pd);
	}
}

void Design::drawRoundRect(Graphics::ManagedSurface *surface, Common::ReadStream &in,
				Graphics::MacPatterns &patterns, byte fillType, byte borderThickness, byte borderFillType) {
	int16 y1 = in.readSint16BE();
	int16 x1 = in.readSint16BE();
	int16 y2 = in.readSint16BE();
	int16 x2 = in.readSint16BE();
	int16 arc = in.readSint16BE();

	if (x1 > x2)
		SWAP(x1, x2);
	if (y1 > y2)
		SWAP(y1, y2);

	Common::Rect r(x1, y1, x2, y2);
	PlotData pd(surface, &patterns, fillType, 1, this);

	if (fillType <= patterns.size())
		Graphics::drawRoundRect(r, arc / 2, kColorBlack, true, drawPixel, &pd);

	pd.fillType = borderFillType;
	pd.thickness = borderThickness;

	if (borderThickness > 0 && borderFillType <= patterns.size())
		Graphics::drawRoundRect(r, arc / 2, kColorBlack, false, drawPixel, &pd);
}

void Design::drawPolygon(Graphics::ManagedSurface *surface, Common::ReadStream &in,
	Graphics::MacPatterns &patterns, byte fillType, byte borderThickness, byte borderFillType) {

	byte ignored = in.readSint16BE(); // ignored

	if (ignored)
		warning("Ignored: %d", ignored);

	int numBytes = in.readSint16BE(); // #bytes used by polygon data, including the numBytes
	int16 by1 = in.readSint16BE();
	int16 bx1 = in.readSint16BE();
	int16 by2 = in.readSint16BE();
	int16 bx2 = in.readSint16BE();
	Common::Rect bbox(bx1, by1, bx2, by2);

	numBytes -= 8;

	int y1 = in.readSint16BE();
	int x1 = in.readSint16BE();

	Common::Array<int> xcoords;
	Common::Array<int> ycoords;

	numBytes -= 6;

	while (numBytes > 0) {
		int y2 = y1;
		int x2 = x1;
		int b = in.readSByte();
		if (b == -128) {
			y2 = in.readSint16BE();
			numBytes -= 3;
		} else {
			y2 += b;
			numBytes -= 1;
		}
		b = in.readSByte();
		if (b == -128) {
			x2 = in.readSint16BE();
			numBytes -= 3;
		} else {
			x2 += b;
			numBytes -= 1;
		}
		xcoords.push_back(x1);
		ycoords.push_back(y1);
		x1 = x2;
		y1 = y2;
	}
	xcoords.push_back(x1);
	ycoords.push_back(y1);

	int npoints = xcoords.size();
	int *xpoints = (int *)calloc(npoints, sizeof(int));
	int *ypoints = (int *)calloc(npoints, sizeof(int));
	for (int i = 0; i < npoints; i++) {
		xpoints[i] = xcoords[i];
		ypoints[i] = ycoords[i];
	}

	PlotData pd(surface, &patterns, fillType, 1, this);

	if (fillType <= patterns.size()) {
		Graphics::drawPolygonScan(xpoints, ypoints, npoints, bbox, kColorBlack, drawPixel, &pd);
	}

	pd.fillType = borderFillType;
	pd.thickness = borderThickness;
	if (borderThickness > 0 && borderFillType <= patterns.size()) {
		for (int i = 1; i < npoints; i++)
			Graphics::drawLine(xpoints[i-1], ypoints[i-1], xpoints[i], ypoints[i], kColorBlack, drawPixel, &pd);
	}

	free(xpoints);
	free(ypoints);
}

void Design::drawOval(Graphics::ManagedSurface *surface, Common::ReadStream &in,
			Graphics::MacPatterns &patterns, byte fillType, byte borderThickness, byte borderFillType) {
	int16 y1 = in.readSint16BE();
	int16 x1 = in.readSint16BE();
	int16 y2 = in.readSint16BE();
	int16 x2 = in.readSint16BE();
	PlotData pd(surface, &patterns, fillType, 1, this);

	if (fillType <= patterns.size())
		Graphics::drawEllipse(x1, y1, x2-1, y2-1, kColorBlack, true, drawPixel, &pd);

	pd.fillType = borderFillType;
	pd.thickness = borderThickness;

	if (borderThickness > 0 && borderFillType <= patterns.size())
		Graphics::drawEllipse(x1, y1, x2-1, y2-1, kColorBlack, false, drawPixel, &pd);
}

void Design::drawBitmap(Graphics::ManagedSurface *surface, Common::SeekableReadStream &in) {
	int numBytes = in.readSint16BE();
	int y1 = in.readSint16BE();
	int x1 = in.readSint16BE();
	int y2 = in.readSint16BE();
	int x2 = in.readSint16BE();
	int w = x2 - x1;
	int h = y2 - y1;
	Graphics::Surface tmp;

	tmp.create(w, h, Graphics::PixelFormat::createFormatCLUT8());

	numBytes -= 10;

	int x = 0, y = 0;
	while (numBytes > 0 && y < h) {
		int n = in.readSByte();
		int count;
		int b = 0;
		int state = 0;

		numBytes--;

		if ((n >= 0) && (n <= 127)) { // If n is between 0 and 127 inclusive, copy the next n+1 bytes literally.
			count = n + 1;
			state = 1;
		} else if ((n >= -127) && (n <= -1)) { // Else if n is between -127 and -1 inclusive, copy the next byte -n+1 times.
			b = in.readByte();
			numBytes--;
			count = -n + 1;
			state = 2;
		} else { // Else if n is -128, noop.
			count = 0;
		}

		for (int i = 0; i < count && y < h; i++) {
			byte color = 0;
			if (state == 1) {
				color = in.readByte();
				numBytes--;
			} else if (state == 2)
				color = b;

			for (int c = 0; c < 8; c++) {
				if (_boundsCalculationMode) {
					adjustBounds(x1 + x, y1 + y);
				} else if (x1 + x >= 0 && x1 + x < surface->w && y1 + y >= 0 && y1 + y < surface->h)
					*((byte *)tmp.getBasePtr(x, y)) = (color & (1 << (7 - c % 8))) ? kColorBlack : kColorWhite;
				x++;
				if (x == w) {
					y++;
					x = 0;
					break;
				}
			}
		}
	}

	in.skip(numBytes);

	if (!_boundsCalculationMode) {
		Graphics::FloodFill ff(&tmp, kColorWhite, kColorGreen);
		for (int yy = 0; yy < h; yy++) {
			ff.addSeed(0, yy);
			ff.addSeed(w - 1, yy);
		}
		for (int xx = 0; xx < w; xx++) {
			ff.addSeed(xx, 0);
			ff.addSeed(xx, h - 1);
		}
		ff.fill();

		for (y = 0; y < h && y1 + y < surface->h; y++) {
			byte *src = (byte *)tmp.getBasePtr(0, y);
			byte *dst = (byte *)surface->getBasePtr(x1, y1 + y);
			for (x = 0; x < w; x++) {
				if (*src != kColorGreen)
					*dst = *src;
				src++;
				dst++;
			}
		}
	}

	tmp.free();
}

void Design::drawRect(Graphics::ManagedSurface *surface, Common::Rect &rect, int thickness, int color, Graphics::MacPatterns &patterns, byte fillType) {
	drawRect(surface, rect.left, rect.top, rect.right, rect.bottom, thickness, color, patterns, fillType);
}

void Design::drawRect(Graphics::ManagedSurface *surface, int x1, int y1, int x2, int y2, int thickness, int color, Graphics::MacPatterns &patterns, byte fillType) {
	PlotData pd(surface, &patterns, fillType, thickness, nullptr);

	Graphics::drawLine(x1, y1, x2, y1, kColorBlack, drawPixel, &pd);
	Graphics::drawLine(x2, y1, x2, y2, kColorBlack, drawPixel, &pd);
	Graphics::drawLine(x2, y2, x1, y2, kColorBlack, drawPixel, &pd);
	Graphics::drawLine(x1, y2, x1, y1, kColorBlack, drawPixel, &pd);
}


void Design::drawFilledRect(Graphics::ManagedSurface *surface, Common::Rect &rect, int color, Graphics::MacPatterns &patterns, byte fillType) {
	PlotData pd(surface, &patterns, fillType, 1, nullptr);

	for (int y = rect.top; y <= rect.bottom; y++)
		Graphics::drawHLine(rect.left, rect.right, y, color, drawPixel, &pd);
}

void Design::drawFilledRoundRect(Graphics::ManagedSurface *surface, Common::Rect &rect, int arc, int color, Graphics::MacPatterns &patterns, byte fillType) {
	PlotData pd(surface, &patterns, fillType, 1, nullptr);

	Graphics::drawRoundRect(rect, arc, color, true, drawPixel, &pd);
}

void Design::drawHLine(Graphics::ManagedSurface *surface, int x1, int x2, int y, int thickness, int color, Graphics::MacPatterns &patterns, byte fillType) {
	PlotData pd(surface, &patterns, fillType, thickness, nullptr);

	Graphics::drawHLine(x1, x2, y, color, drawPixel, &pd);
}

void Design::drawVLine(Graphics::ManagedSurface *surface, int x, int y1, int y2, int thickness, int color, Graphics::MacPatterns &patterns, byte fillType) {
	PlotData pd(surface, &patterns, fillType, thickness, nullptr);

	Graphics::drawVLine(x, y1, y2, color, drawPixel, &pd);
}

} // End of namespace Wage
