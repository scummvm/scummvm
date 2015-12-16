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

	Graphics::Surface screen;
	screen.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	Common::Rect r(0, 0, 320, 200);
	setBounds(&r);
	paint(&screen, 0, true);
}

Design::~Design() {
	free(_data);
}

void Design::paint(Graphics::Surface *canvas, TexturePaint *patterns, bool mask) {
	Common::MemoryReadStream in(_data, _len);

	if (mask) {
		//canvas.setColor(Color.WHITE);
		canvas->fillRect(Common::Rect(0, 0, _bounds->width(), _bounds->height()), kColorWhite);
		//canvas.setColor(Color.BLACK);
	}

	while (!in.eos()) {
		byte fillType = in.readByte();
		byte borderThickness = in.readByte();
		byte borderFillType = in.readByte();
		int type = in.readByte();
		switch (type) {
		case 4:
			drawRect(canvas, in, mask, patterns, fillType, borderThickness, borderFillType);
			break;
			/*
		case 8:
			drawRoundRect(canvas, in, mask, patterns, fillType, borderThickness, borderFillType);
			break;
		case 12:
			drawOval(canvas, in, mask, patterns, fillType, borderThickness, borderFillType);
			break;
*/
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
	TexturePaint *patterns, byte fillType, byte borderThickness, byte borderFillType) {
	int16 y = in.readSint16BE();
	int16 x = in.readSint16BE();
	int16 height = in.readSint16BE();
	int16 width = in.readSint16BE();
	Common::Rect outer(x, y, width, height);

	if (mask) {
		surface->fillRect(outer, kColorBlack);
		return;
	}
	//Shape inner = new Rectangle(x+borderThickness, y+borderThickness, width-2*borderThickness, height-2*borderThickness);
	//paintShape(g2d, patterns, outer, inner, borderFillType, fillType);
	surface->frameRect(outer, kColorBlack);
}

void Design::drawPolygon(Graphics::Surface *surface, Common::ReadStream &in, bool mask,
	TexturePaint *patterns, byte fillType, byte borderThickness, byte borderFillType) {
	//surface->setColor(Color.BLACK);
	//in.readUint16BE();
	warning("ignored => %d", in.readSint16BE());
	int numBytes = in.readSint16BE(); // #bytes used by polygon data, including the numBytes
	warning("Num bytes is %d", numBytes);
	// Ignoring these values works!!!
	//in.readUint16BE(); in.readUint16BE(); in.readUint16BE(); in.readUint16BE();
	warning("Ignoring: %d", in.readSint16BE());
	warning("Ignoring: %d", in.readSint16BE());
	warning("Ignoring: %d", in.readSint16BE());
	warning("Ignoring: %d", in.readSint16BE());

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
		warning("%d %d %d %d", x1, y1, x2, y2);
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
		for (int i = 1; i < npoints; i++)
			surface->drawLine(xpoints[i-1], ypoints[i-1], xpoints[i], ypoints[i], kColorBlack);
//		surface->setStroke(oldStroke);
//	}

	free(xpoints);
	free(ypoints);
}


} // End of namespace Wage
