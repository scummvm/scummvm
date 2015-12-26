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

#include "wage/wage.h"
#include "wage/gui.h"

namespace Wage {

Gui::Gui() {
}

Gui::~Gui() {
}

void Gui::drawBox(Graphics::Surface *g, int x, int y, int w, int h) {
	Common::Rect r(x, y, x + w + 1, y + h + 1);

	g->fillRect(r, kColorWhite);
	g->frameRect(r, kColorBlack);
}

void Gui::paintBorder(Graphics::Surface *g, int x, int y, int width, int height) {
	int size = 17;
	drawBox(g, x, y, size, size);
	drawBox(g, x+width-size-1, y, size, size);
	drawBox(g, x+width-size-1, y+height-size-1, size, size);
	drawBox(g, x, y+height-size-1, size, size);
	drawBox(g, x + size, y + 2, width - 2*size - 1, size - 4);
	drawBox(g, x + size, y + height - size + 1, width - 2*size - 1, size - 4);
	drawBox(g, x + 2, y + size, size - 4, height - 2*size - 1);
	drawBox(g, x + width - size + 1, y + size, size - 4, height - 2*size-1);

#if 0
	if (active) {
		g.setColor(Color.BLACK);
		g.fillRect(x + size, y + 5, width - 2*size - 1, 8);
		g.fillRect(x + size, y + height - 13, width - 2*size - 1, 8);
		g.fillRect(x + 5, y + size, 8, height - 2*size - 1);
		if (!scrollable) {
			g.fillRect(x + width - 13, y + size, 8, height - 2*size - 1);
		} else {
			int pixels[][] = new int[][] {
					{0,0,0,0,0,1,1,0,0,0,0,0},
					{0,0,0,0,1,1,1,1,0,0,0,0},
					{0,0,0,1,1,1,1,1,1,0,0,0},
					{0,0,1,1,1,1,1,1,1,1,0,0},
					{0,1,1,1,1,1,1,1,1,1,1,0},
					{1,1,1,1,1,1,1,1,1,1,1,1}};
			final int h = pixels.length;
			final int w = pixels[0].length;
			int x1 = x + width - 15;
			int y1 = y + size + 1;
			for (int yy = 0; yy < h; yy++) {
				for (int xx = 0; xx < w; xx++) {
					if (pixels[yy][xx] != 0) {
						g.drawRect(x1+xx, y1+yy, 0, 0);
					}
				}
			}
			g.fillRect(x + width - 13, y + size + h, 8, height - 2*size - 1 - h*2);
			y1 += height - 2*size - h - 2;
			for (int yy = 0; yy < h; yy++) {
				for (int xx = 0; xx < w; xx++) {
					if (pixels[h-yy-1][xx] != 0) {
						g.drawRect(x1+xx, y1+yy, 0, 0);
					}
				}
			}
		}
		if (closeable) {
			if (closeBoxPressed) {
				g.fillRect(x + 6, y + 6, 6, 6);
			} else {
				drawBox(g, x + 5, y + 5, 7, 7);
			}
		}
	}

	if (title != null) {
		// TODO: This "Chicago" is not faithful to the original one on the Mac.
		Font f = new Font("Chicago", Font.BOLD, 12);
		int w = g.getFontMetrics(f).stringWidth(title) + 6;
		int maxWidth = width - size*2 - 7;
		if (w > maxWidth) {
			w = maxWidth;
		}
		drawBox(g, x + (width - w) / 2, y, w, size);
		g.setFont(f);
		Shape clip = g.getClip();
		g.setClip(x + (width - w) / 2, y, w, size);
		g.drawString(title, x + (width - w) / 2 + 3, y + size - 4);
		g.setClip(clip);
	}
#endif
}


} // End of namespace Wage
