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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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

#include "common/system.h"
#include "wage/wage.h"
#include "wage/design.h"

namespace Wage {

Design::Design(Common::SeekableReadStream *data) {
	_len = data->readUint16BE() - 2;
	_data = (byte *)malloc(_len);
	data->read(_data, _len);
}

Design::~Design() {
	free(_data);
}

void Design::paint(Graphics::Surface *canvas, Patterns &patterns, bool mask) {
	Common::MemoryReadStream in(_data, _len);

	if (mask || 1) {
		//canvas.setColor(Color.WHITE);
		canvas->fillRect(Common::Rect(0, 0, _bounds->width(), _bounds->height()), kColorWhite);
		//canvas.setColor(Color.BLACK);
	}

	while (!in.eos()) {
		byte fillType = in.readByte();
		byte borderThickness = in.readByte();
		byte borderFillType = in.readByte();
		warning("fill: %d border: %d borderFill: %d", fillType, borderThickness, borderFillType);
		int type = in.readByte();
		switch (type) {
		case 4:
			drawRect(canvas, in, mask, patterns, fillType, borderThickness, borderFillType);
			break;
			/*
		case 8:
			drawRoundRect(canvas, in, mask, patterns, fillType, borderThickness, borderFillType);
			break;
			*/
		case 12:
			drawOval(canvas, in, mask, patterns, fillType, borderThickness, borderFillType);
			break;
		case 16:
		case 20:
			drawPolygon(canvas, in, mask, patterns, fillType, borderThickness, borderFillType);
			break;
/*
		case 24:
			drawBitmap(canvas, in, mask);
			break;
*/
		default:
			warning("Unknown type => %d", type);
			while (true) {
				((WageEngine *)g_engine)->processEvents();
				g_system->updateScreen();
			}
			return;
		}

		g_system->copyRectToScreen(canvas->getPixels(), canvas->pitch, 0, 0, canvas->w, canvas->h);
		g_system->updateScreen();

	}
}

void Design::drawRect(Graphics::Surface *surface, Common::ReadStream &in, bool mask,
	Patterns &patterns, byte fillType, byte borderThickness, byte borderFillType) {
	int16 y1 = in.readSint16BE();
	int16 x1 = in.readSint16BE();
	int16 y2 = in.readSint16BE();
	int16 x2 = in.readSint16BE();
	Common::Rect outer(x1, y1, x2, y2);

	if (mask) {
		surface->fillRect(outer, kColorBlack);
		return;
	}
	fillType = 7;
	Common::Rect inner(x1 + borderThickness, y1 + borderThickness, x2 - borderThickness, y2 - borderThickness);
	patternThickRect(surface, patterns, outer, inner, borderFillType, fillType);
}

void Design::drawPolygon(Graphics::Surface *surface, Common::ReadStream &in, bool mask,
	Patterns &patterns, byte fillType, byte borderThickness, byte borderFillType) {
	//surface->setColor(Color.BLACK);
	//in.readUint16BE();
	warning("ignored => %d", in.readSint16BE());
	int numBytes = in.readSint16BE(); // #bytes used by polygon data, including the numBytes
	warning("Num bytes is %d", numBytes);
	// Ignoring these values works!!!
	//in.readUint16BE(); in.readUint16BE(); in.readUint16BE(); in.readUint16BE();
	int16 by1 = in.readSint16BE();
	int16 bx1 = in.readSint16BE();
	int16 by2 = in.readSint16BE();
	int16 bx2 = in.readSint16BE();
	Common::Rect bbox(bx1, by1, bx2, by2);
	warning("Bbox: %d, %d, %d, %d", bx1, by1, bx2, by2);

	numBytes -= 8;

	int y1 = in.readSint16BE();
	int x1 = in.readSint16BE();

	Common::Array<int> xcoords;
	Common::Array<int> ycoords;

	warning("Start point is (%d,%d)", x1, y1);
	numBytes -= 6;

	while (numBytes > 0) {
		int y2 = y1;
		int x2 = x1;
		int b = in.readSByte();
		//warning("YB = %x", b);
		if (b == (byte)0x80) {
			y2 = in.readSint16BE();
			numBytes -= 3;
		} else {
			//warning("Y");
			y2 += b;
			numBytes -= 1;
		}
		b = in.readSByte();
		//warning("XB = %x", b);
		if (b == (byte) 0x80) {
			x2 = in.readSint16BE();
			numBytes -= 3;
		} else {
			//warning("X");
			x2 += b;
			numBytes -= 1;
		}
		//surface->setColor(colors[c++]);
		//surface->setColor(Color.black);
		xcoords.push_back(x1);
		ycoords.push_back(y1);
		debug(8, "%d %d %d %d", x1, y1, x2, y2);
		//surface->drawLine(x1, y1, x2, y2);
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
	//	warning(fillType);
/*
	if (mask) {
		surface->fillPolygon(xpoints, ypoints, npoints);
		if (borderThickness > 0) {
			Stroke oldStroke = surface->getStroke();
			surface->setStroke(new BasicStroke(borderThickness - 0.5f, BasicStroke.CAP_SQUARE, BasicStroke.JOIN_BEVEL));
			for (int i = 1; i < npoints; i++)
				surface->drawLine(xpoints[i-1], ypoints[i-1], xpoints[i], ypoints[i]);
			surface->setStroke(oldStroke);
		}
		return;
	}
	if (setPattern(g2d, patterns, fillType - 1)) {
		surface->fillPolygon(xpoints, ypoints, npoints);
	}
	//	warning(borderFillType);
	//surface->setColor(Color.black);
	//if (1==0)
	if (borderThickness > 0 && setPattern(g2d, patterns, borderFillType - 1)) {
		Stroke oldStroke = surface->getStroke();
		//if (borderThickness != 1)
		surface->setStroke(new BasicStroke(borderThickness - 0.5f, BasicStroke.CAP_SQUARE, BasicStroke.JOIN_BEVEL));
*/
	patternThickPolygon(surface, patterns, xpoints, ypoints, npoints, bbox, borderFillType, fillType);
	//	surface->setStroke(oldStroke);
//	}

	free(xpoints);
	free(ypoints);
}

void Design::patternThickRect(Graphics::Surface *surface, Patterns &patterns, Common::Rect &outer,
	Common::Rect &inner, byte borderFillType, byte fillType) {
	patternRect(surface, patterns, outer, borderFillType);
	patternRect(surface, patterns, inner, fillType);
}

void Design::patternRect(Graphics::Surface *surface, Patterns &patterns, Common::Rect &rect, byte fillType) {
	for (int y = rect.top; y < rect.bottom; y++)
		patternHLine(surface, patterns, fillType, rect.left, rect.right, y, rect.left, rect.top);
}


// Based on public-domain code by Darel Rex Finley, 2007
// http://alienryderflex.com/polygon_fill/
void Design::patternThickPolygon(Graphics::Surface *surface, Patterns &patterns, int *polyX,
	int *polyY, int npoints, Common::Rect &bbox, byte borderFillType, byte fillType) {
	int *nodeX = (int *)calloc(npoints, sizeof(int));
	int i, j;

	//  Loop through the rows of the image.
	for (int pixelY = bbox.top; pixelY < bbox.bottom; pixelY++) {
		//  Build a list of nodes.
		int nodes = 0;
		j = npoints - 1;

		for (i = 0; i < npoints; i++) {
			if ((polyY[i] < pixelY && polyY[j] >= pixelY) || (polyY[j] < pixelY && polyY[i] >= pixelY)) {
				nodeX[nodes++] = (int)(polyX[i] + (double)(pixelY - polyY[i]) / (double)(polyY[j]-polyY[i]) *
														(double)(polyX[j] - polyX[i]) + 0.5);
			}
			j = i;
		}

		//  Sort the nodes, via a simple “Bubble” sort.
		i = 0;
		while (i < nodes - 1) {
			if (nodeX[i] > nodeX[i + 1]) {
				SWAP(nodeX[i], nodeX[i + 1]);
				if (i)
					i--;
			} else {
				i++;
			}
		}

		//  Fill the pixels between node pairs.
		for (i = 0; i < nodes; i += 2) {
			if (nodeX[i  ] >= bbox.right)
				break;
			if (nodeX[i + 1] > bbox.left) {
				nodeX[i] = MAX<int16>(nodeX[i], bbox.left);
				nodeX[i + 1] = MIN<int16>(nodeX[i + 1], bbox.right);

				patternHLine(surface, patterns, fillType, nodeX[i], nodeX[i + 1], pixelY, bbox.left, bbox.top);
			}
		}
	}

	free(nodeX);

	for (i = 1; i < npoints; i++)
			surface->drawLine(polyX[i-1], polyY[i-1], polyX[i], polyY[i], kColorBlack);
}

void Design::drawOval(Graphics::Surface *surface, Common::ReadStream &in, bool mask,
	Patterns &patterns, byte fillType, byte borderThickness, byte borderFillType) {
	int16 y1 = in.readSint16BE();
	int16 x1 = in.readSint16BE();
	int16 y2 = in.readSint16BE();
	int16 x2 = in.readSint16BE();

	plotEllipseRect(surface, patterns, x1, y1, x2, y2, borderFillType);
	plotEllipseRect(surface, patterns, x1+borderThickness, y1+borderThickness, x2-2*borderThickness, y2-2*borderThickness, fillType);
}


void Design::plotEllipseRect(Graphics::Surface *surface, Patterns &patterns,
			int x0, int y0, int x1, int y1, byte fillType) {
	int xO = x0, yO = y0;
	int a = abs(x1-x0), b = abs(y1-y0), b1 = b&1; /* values of diameter */
	long dx = 4*(1-a)*b*b, dy = 4*(b1+1)*a*a; /* error increment */
	long err = dx+dy+b1*a*a, e2; /* error of 1.step */

	if (x0 > x1) { x0 = x1; x1 += a; } /* if called with swapped points */
	if (y0 > y1) y0 = y1; /* .. exchange them */
	y0 += (b+1)/2; y1 = y0-b1;   /* starting pixel */
	a *= 8*a; b1 = 8*b*b;

	do {
		patternHLine(surface, patterns, fillType, x0, x1 + 1, y0, xO, yO);
		patternHLine(surface, patterns, fillType, x0, x1 + 1, y1, xO, yO);
		e2 = 2*err;
		if (e2 <= dy) { y0++; y1--; err += dy += a; }  /* y step */
		if (e2 >= dx || 2*err > dy) { x0++; x1--; err += dx += b1; } /* x step */
	} while (x0 <= x1);

	while (y0-y1 < b) {  /* too early stop of flat ellipses a=1 */
		patternHLine(surface, patterns, fillType, x0-1, x0-1, y0, xO, yO); /* -> finish tip of ellipse */
		patternHLine(surface, patterns, fillType, x1+1, x1+1, y0, xO, yO);
		y0++;
		patternHLine(surface, patterns, fillType, x0-1, x0-1, y1, xO, yO);
		patternHLine(surface, patterns, fillType, x1+1, x1+1, y1, xO, yO);
		y1--;
	}
}

void Design::patternHLine(Graphics::Surface *surface, Patterns &patterns, byte fillType, int x1, int x2, int y, int x0, int y0) {
	if (x1 > x2)
		SWAP(x1, x2);

	if (fillType > patterns.size())
		return;

	for (int x = x1; x < x2; x++)
		if (x >= 0 && x < surface->w && y >= 0 && y < surface->h)
			*((byte *)surface->getBasePtr(x, y)) =
				(patterns[fillType - 1][(y - y0) % 8] & (1 << (7 - (x - x0) % 8))) ?
					kColorBlack : kColorWhite;
}


} // End of namespace Wage
