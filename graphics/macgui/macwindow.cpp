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

#include "graphics/font.h"
#include "graphics/primitives.h"
#include "common/events.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macwindow.h"

namespace Graphics {

BaseMacWindow::BaseMacWindow(int id, bool editable, MacWindowManager *wm) :
		_id(id), _editable(editable), _wm(wm) {
	_callback = 0;
	_dataPtr = 0;

	_contentIsDirty = true;

	_type = kWindowUnknown;
}

MacWindow::MacWindow(int id, bool scrollable, bool resizable, bool editable, MacWindowManager *wm) :
		BaseMacWindow(id, editable, wm), _scrollable(scrollable), _resizable(resizable) {
	_active = false;
	_borderIsDirty = true;

	_highlightedPart = kBorderNone;

	_scrollPos = _scrollSize = 0.0;

	_beingDragged = false;
	_beingResized = false;

	_draggedX = _draggedY = 0;

	_type = kWindowWindow;
}

MacWindow::~MacWindow() {
}

const Font *MacWindow::getTitleFont() {
	return _wm->getFont("Chicago-12", FontManager::kBigGUIFont);
}

void MacWindow::setActive(bool active) {
	if (active == _active)
		return;

	_active = active;
	_borderIsDirty = true;
}

void MacWindow::resize(int w, int h) {
	if (_surface.w == w && _surface.h == h)
		return;

	_surface.free();
	_surface.create(w, h, PixelFormat::createFormatCLUT8());
	_borderSurface.free();
	_borderSurface.create(w, h, PixelFormat::createFormatCLUT8());
	_composeSurface.free();
	_composeSurface.create(w, h, PixelFormat::createFormatCLUT8());

	_dims.setWidth(w);
	_dims.setHeight(h);

	updateInnerDims();

	_contentIsDirty = true;
	_borderIsDirty = true;
}

void MacWindow::move(int x, int y) {
	if (_dims.left == x && _dims.top == y)
		return;

	_dims.moveTo(x, y);
	updateInnerDims();

	_contentIsDirty = true;
}

void MacWindow::setDimensions(const Common::Rect &r) {
	resize(r.width(), r.height());
	_dims.moveTo(r.left, r.top);
	updateInnerDims();

	_contentIsDirty = true;
}

bool MacWindow::draw(ManagedSurface *g, bool forceRedraw) {
	if (!_borderIsDirty && !_contentIsDirty && !forceRedraw)
		return false;

	if (_borderIsDirty || forceRedraw)
		drawBorder();

	_contentIsDirty = false;

	// Compose
	_composeSurface.blitFrom(_surface, Common::Rect(0, 0, _surface.w - 2, _surface.h - 2), Common::Point(2, 2));
	_composeSurface.transBlitFrom(_borderSurface, kColorGreen);

	g->transBlitFrom(_composeSurface, _composeSurface.getBounds(), Common::Point(_dims.left - 2, _dims.top - 2), kColorGreen2);

	return true;
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

static void drawPixelInverted(int x, int y, int color, void *data) {
	ManagedSurface *surface = (ManagedSurface *)data;

	if (x >= 0 && x < surface->w && y >= 0 && y < surface->h) {
		byte *p = (byte *)surface->getBasePtr(x, y);

		*p = *p == kColorWhite ? kColorBlack : kColorWhite;
	}
}

void MacWindow::updateInnerDims() {
	_innerDims = _dims;
	_innerDims.grow(-kBorderWidth);
}

void MacWindow::drawBorder() {
	_borderIsDirty = false;

	bool active = _active, scrollable = _scrollable, closeable = _active, drawTitle = !_title.empty();
	const int size = kBorderWidth;
	int x = 0;
	int y = 0;
	int width = _borderSurface.w;
	int height = _borderSurface.h;
	ManagedSurface *g = &_borderSurface;

	// We draw rect with outer kColorGreen2 and inner kColorGreen, so on 2 passes we cut out
	// scene by external shape of the border
	int sz = kBorderWidth / 2;
	g->clear(kColorGreen2);
	g->fillRect(Common::Rect(sz, sz, width - sz, height - sz), kColorGreen);

	drawBox(g, x,                    y,                     size,                 size);
	drawBox(g, x + width - size - 1, y,                     size,                 size);
	drawBox(g, x + width - size - 1, y + height - size - 1, size,                 size);
	drawBox(g, x,                    y + height - size - 1, size,                 size);
	drawBox(g, x + size,             y + 2,                 width - 2 * size - 1, size - 4);
	drawBox(g, x + size,             y + height - size + 1, width - 2 * size - 1, size - 4);
	drawBox(g, x + 2,                y + size,              size - 4,             height - 2 * size - 1);
	drawBox(g, x + width - size + 1, y + size,              size - 4,             height - 2 * size - 1);

	if (active) {
		fillRect(g, x + size, y + 5,           width - 2 * size - 1, 8, kColorBlack);
		fillRect(g, x + size, y + height - 13, width - 2 * size - 1, 8, kColorBlack);
		fillRect(g, x + 5,    y + size,        8,                    height - 2 * size - 1, kColorBlack);
		if (!scrollable) {
			fillRect(g, x + width - 13, y + size, 8, height - 2 * size - 1, kColorBlack);
		} else {
			int x1 = x + width - 15;
			int y1 = y + size + 1;

			for (int yy = 0; yy < ARROW_H; yy++) {
				for (int xx = 0; xx < ARROW_W; xx++)
					g->hLine(x1 + xx, y1 + yy, x1 + xx, (arrowPixels[yy][xx] != 0 ? kColorBlack : kColorWhite));
			}

			fillRect(g, x + width - 13, y + size + ARROW_H, 8, height - 2 * size - 1 - ARROW_H * 2, kColorBlack);

			y1 += height - 2 * size - ARROW_H - 2;
			for (int yy = 0; yy < ARROW_H; yy++) {
				for (int xx = 0; xx < ARROW_W; xx++)
					g->hLine(x1 + xx, y1 + yy, x1 + xx, (arrowPixels[ARROW_H - yy - 1][xx] != 0 ? kColorBlack : kColorWhite));
			}

			if (_highlightedPart == kBorderScrollUp || _highlightedPart == kBorderScrollDown) {
				int rx1 = x + width - kBorderWidth + 2;
				int ry1 = y + size + _dims.height() * _scrollPos;
				int rx2 = rx1 + size - 4;
				int ry2 = ry1 + _dims.height() * _scrollSize;
				Common::Rect rr(rx1, ry1, rx2, ry2);

				drawFilledRect(rr, kColorBlack, drawPixelInverted, g);
			}
		}
		if (closeable) {
			if (_highlightedPart == kBorderCloseButton) {
				fillRect(g, x + 6, y + 6, 6, 6, kColorBlack);
			} else {
				drawBox(g, x + 5, y + 5, 7, 7);
			}
		}
	}

	if (drawTitle) {
		const Font *font = getTitleFont();
		int yOff = _wm->hasBuiltInFonts() ? 3 : 1;

		int w = font->getStringWidth(_title) + 10;
		int maxWidth = width - size * 2 - 7;
		if (w > maxWidth)
			w = maxWidth;
		drawBox(g, x + (width - w) / 2, y, w, size);
		font->drawString(g, _title, x + (width - w) / 2 + 5, y + yOff, w, kColorBlack);
	}
}

void MacWindow::setHighlight(WindowClick highlightedPart) {
	if (_highlightedPart == highlightedPart)
		return;

	_highlightedPart = highlightedPart;
	_borderIsDirty = true;
 }

 void MacWindow::setScroll(float scrollPos, float scrollSize) {
	if (_scrollPos == scrollPos && _scrollSize == scrollSize)
		return;

	_scrollPos = scrollPos;
	_scrollSize = scrollSize;
	_borderIsDirty = true;
 }


void MacWindow::drawBox(ManagedSurface *g, int x, int y, int w, int h) {
	Common::Rect r(x, y, x + w + 1, y + h + 1);

	g->fillRect(r, kColorWhite);
	g->frameRect(r, kColorBlack);
}

void MacWindow::fillRect(ManagedSurface *g, int x, int y, int w, int h, int color) {
	Common::Rect r(x, y, x + w, y + h);

	g->fillRect(r, color);
}

WindowClick MacWindow::isInBorder(int x, int y) {
	if (_innerDims.contains(x, y))
		return kBorderInner;

	if (x >= _innerDims.left - kBorderWidth && x < _innerDims.left && y >= _innerDims.top - kBorderWidth && y < _innerDims.top)
		return kBorderCloseButton;

	if (_resizable)
		if (x >= _innerDims.right && x < _innerDims.right + kBorderWidth && y >= _innerDims.bottom && y < _innerDims.bottom + kBorderWidth)
			return kBorderResizeButton;

	if (_scrollable && x >= _innerDims.right && x < _innerDims.right + kBorderWidth) {
		if (y < _innerDims.top - kBorderWidth)
			return kBorderBorder;

		if (y >= _innerDims.bottom + kBorderWidth)
			return kBorderBorder;

		if (y >= _innerDims.top + _innerDims.height() / 2)
			return kBorderScrollDown;

		return kBorderScrollUp;
	}

	return kBorderBorder;
}

bool MacWindow::processEvent(Common::Event &event) {
	WindowClick click = isInBorder(event.mouse.x, event.mouse.y);

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		if (_beingDragged) {
			_dims.translate(event.mouse.x - _draggedX, event.mouse.y - _draggedY);
			updateInnerDims();

			_draggedX = event.mouse.x;
			_draggedY = event.mouse.y;

			_wm->setFullRefresh(true);
		}

		if (_beingResized) {
			resize(MAX(kBorderWidth * 4, _dims.width()  + event.mouse.x - _draggedX),
				   MAX(kBorderWidth * 4, _dims.height() + event.mouse.y - _draggedY));

			_draggedX = event.mouse.x;
			_draggedY = event.mouse.y;

			_wm->setFullRefresh(true);
			(*_callback)(click, event, _dataPtr);
		}
		break;
	case Common::EVENT_LBUTTONDOWN:
		setHighlight(click);

		if (click == kBorderBorder) {
			_beingDragged = true;

			_draggedX = event.mouse.x;
			_draggedY = event.mouse.y;
		}

		if (click == kBorderResizeButton) {
			_beingResized = true;

			_draggedX = event.mouse.x;
			_draggedY = event.mouse.y;
		}

		break;
	case Common::EVENT_LBUTTONUP:
		_beingDragged = false;
		_beingResized = false;

		setHighlight(kBorderNone);
		break;
	default:
		return false;
	}

	return (*_callback)(click, event, _dataPtr);
}

} // End of namespace Wage
