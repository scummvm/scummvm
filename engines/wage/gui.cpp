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
#include "wage/entities.h"
#include "wage/gui.h"

namespace Wage {

enum {
	kMenuHeight = 19,
	kMenuPadding = 6,
	kMenuItemHeight = 19
};

Gui::Gui() {
	_scene = NULL;
	_sceneDirty = true;
	_screen.create(g_system->getWidth(), g_system->getHeight(), Graphics::PixelFormat::createFormatCLUT8());
}

Gui::~Gui() {
}

void Gui::setScene(Scene *scene) {
	if (_scene != scene)
		_sceneDirty = true;

	_scene = scene;
}

void Gui::draw() {
	if (_scene != NULL && _sceneDirty) {
		_scene->paint(&_screen, 0, kMenuHeight);
		paintBorder(&_screen, 0, kMenuHeight, _scene->_design->getBounds()->width(), _scene->_design->getBounds()->height(),
				true, true, true, false);

		_sceneDirty = false;
	}

	g_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0, _screen.w, _screen.h);
}

void Gui::drawBox(Graphics::Surface *g, int x, int y, int w, int h) {
	Common::Rect r(x, y, x + w + 1, y + h + 1);

	g->fillRect(r, kColorWhite);
	g->frameRect(r, kColorBlack);
}

void Gui::fillRect(Graphics::Surface *g, int x, int y, int w, int h) {
	Common::Rect r(x, y, x + w + 1, y + h + 1);

	g->fillRect(r, kColorBlack);
}

#define ARROW_W 12
#define ARROW_H 6
const int arrowPixels[ARROW_H][ARROW_W] = {
		{0,0,0,0,0,1,1,0,0,0,0,0},
		{0,0,0,0,1,1,1,1,0,0,0,0},
		{0,0,0,1,1,1,1,1,1,0,0,0},
		{0,0,1,1,1,1,1,1,1,1,0,0},
		{0,1,1,1,1,1,1,1,1,1,1,0},
		{1,1,1,1,1,1,1,1,1,1,1,1}};


void Gui::paintBorder(Graphics::Surface *g, int x, int y, int width, int height,
			bool active, bool scrollable, bool closeable, bool closeBoxPressed) {
	int size = 17;
	drawBox(g, x, y, size, size);
	drawBox(g, x+width-size-1, y, size, size);
	drawBox(g, x+width-size-1, y+height-size-1, size, size);
	drawBox(g, x, y+height-size-1, size, size);
	drawBox(g, x + size, y + 2, width - 2*size - 1, size - 4);
	drawBox(g, x + size, y + height - size + 1, width - 2*size - 1, size - 4);
	drawBox(g, x + 2, y + size, size - 4, height - 2*size - 1);
	drawBox(g, x + width - size + 1, y + size, size - 4, height - 2*size-1);

	if (active) {
		fillRect(g, x + size, y + 5, width - 2*size - 1, 8);
		fillRect(g, x + size, y + height - 13, width - 2*size - 1, 8);
		fillRect(g, x + 5, y + size, 8, height - 2*size - 1);
		if (!scrollable) {
			fillRect(g, x + width - 13, y + size, 8, height - 2*size - 1);
		} else {
			int x1 = x + width - 15;
			int y1 = y + size + 1;
			for (int yy = 0; yy < ARROW_H; yy++) {
				for (int xx = 0; xx < ARROW_W; xx++) {
					if (arrowPixels[yy][xx] != 0) {
						g->hLine(x1+xx, y1+yy, x1+xx+1, kColorBlack);
					} else {
						g->hLine(x1+xx, y1+yy, x1+xx+1, kColorWhite);
					}
				}
			}
			fillRect(g, x + width - 13, y + size + ARROW_H, 8, height - 2*size - 1 - ARROW_H*2);
			y1 += height - 2*size - ARROW_H - 2;
			for (int yy = 0; yy < ARROW_H; yy++) {
				for (int xx = 0; xx < ARROW_W; xx++) {
					if (arrowPixels[ARROW_H-yy-1][xx] != 0) {
						g->hLine(x1+xx, y1+yy, x1+xx+1, kColorBlack);
					} else {
						g->hLine(x1+xx, y1+yy, x1+xx+1, kColorWhite);
					}
				}
			}
		}
		if (closeable) {
			if (closeBoxPressed) {
				fillRect(g, x + 6, y + 6, 6, 6);
			} else {
				drawBox(g, x + 5, y + 5, 7, 7);
			}
		}
	}

#if 0
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
