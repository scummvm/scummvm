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

#include "graphics/font.h"
#include "graphics/primitives.h"
#include "common/events.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macwindow.h"
#include "image/bmp.h"

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

	_pattern = 0;
	_hasPattern = false;

	_highlightedPart = kBorderNone;

	_scrollPos = _scrollSize = 0.0;

	_beingDragged = false;
	_beingResized = false;

	_draggedX = _draggedY = 0;

	_type = kWindowWindow;

	_closeable = false;

	_borderWidth = kBorderWidth;
}

MacWindow::~MacWindow() {
}

const Font *MacWindow::getTitleFont() {
	return _wm->_fontMan->getFont(Graphics::MacFont(kMacFontChicago, 12));
}

void MacWindow::setActive(bool active) {
	if (active == _active)
		return;

	_active = active;
	_borderIsDirty = true;
}

bool MacWindow::isActive() { return _active; }

void MacWindow::resize(int w, int h) {
	if (_surface.w == w && _surface.h == h)
		return;

	_surface.free();
	_surface.create(w, h, PixelFormat::createFormatCLUT8());

	if (_hasPattern)
		drawPattern();

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

void MacWindow::setBackgroundPattern(int pattern) {
	_pattern = pattern;
	_hasPattern = true;
	drawPattern();
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

int localColorWhite, localColorBlack;

static void drawPixelInverted(int x, int y, int color, void *data) {
	ManagedSurface *surface = (ManagedSurface *)data;

	if (x >= 0 && x < surface->w && y >= 0 && y < surface->h) {
		byte *p = (byte *)surface->getBasePtr(x, y);

		*p = *p == localColorWhite ? localColorBlack : localColorWhite;
	}
}

void MacWindow::updateInnerDims() {
	if (_macBorder.hasBorder(_active) && _macBorder.hasOffsets()) {
		_innerDims = Common::Rect(
			_dims.left + _macBorder.getOffset(kBorderOffsetLeft),
			_dims.top + _macBorder.getOffset(kBorderOffsetTop),
			_dims.right - _macBorder.getOffset(kBorderOffsetRight),
			_dims.bottom - _macBorder.getOffset(kBorderOffsetBottom));
	} else {
		_innerDims = _dims;
		_innerDims.grow(-kBorderWidth);
	}
}

void MacWindow::drawBorder() {
	_borderIsDirty = false;

	ManagedSurface *g = &_borderSurface;

	if (_macBorder.hasBorder(_active)) {
		drawBorderFromSurface(g);
	} else {
		drawSimpleBorder(g);
	}
}

void MacWindow::prepareBorderSurface(ManagedSurface *g) {
	// We draw rect with outer kColorGreen2 and inner kColorGreen, so on 2 passes we cut out
	// scene by external shape of the border
	int sz = kBorderWidth / 2;
	int width = g->w;
	int height = g->h;
	g->clear(kColorGreen2);
	g->fillRect(Common::Rect(sz, sz, width - sz, height - sz), kColorGreen);
}

void MacWindow::drawBorderFromSurface(ManagedSurface *g) {
	g->clear(kColorGreen2);
	Common::Rect inside = _innerDims;
	inside.moveTo(_macBorder.getOffset(kBorderOffsetLeft), _macBorder.getOffset(kBorderOffsetTop));
	g->fillRect(inside, kColorGreen);

	_macBorder.blitBorderInto(_borderSurface, _active);
}

void MacWindow::drawSimpleBorder(ManagedSurface *g) {

	bool active = _active, scrollable = _scrollable, closeable = _active, drawTitle = !_title.empty();
	const int size = kBorderWidth;
	int x = 0;
	int y = 0;
	int width = _borderSurface.w;
	int height = _borderSurface.h;

	prepareBorderSurface(g);

	drawBox(g, x,                    y,                     size,                 size);
	drawBox(g, x + width - size - 1, y,                     size,                 size);
	drawBox(g, x + width - size - 1, y + height - size - 1, size,                 size);
	drawBox(g, x,                    y + height - size - 1, size,                 size);
	drawBox(g, x + size,             y + 2,                 width - 2 * size - 1, size - 4);
	drawBox(g, x + size,             y + height - size + 1, width - 2 * size - 1, size - 4);
	drawBox(g, x + 2,                y + size,              size - 4,             height - 2 * size - 1);
	drawBox(g, x + width - size + 1, y + size,              size - 4,             height - 2 * size - 1);

	if (active) {
		fillRect(g, x + size, y + 5,           width - 2 * size - 1, 8, _wm->_colorBlack);
		fillRect(g, x + size, y + height - 13, width - 2 * size - 1, 8, _wm->_colorBlack);
		fillRect(g, x + 5,    y + size,        8,                    height - 2 * size - 1, _wm->_colorBlack);
		if (!scrollable) {
			fillRect(g, x + width - 13, y + size, 8, height - 2 * size - 1, _wm->_colorBlack);
		} else {
			int x1 = x + width - 15;
			int y1 = y + size + 1;

			for (int yy = 0; yy < ARROW_H; yy++) {
				for (int xx = 0; xx < ARROW_W; xx++)
					g->hLine(x1 + xx, y1 + yy, x1 + xx, (arrowPixels[yy][xx] != 0 ? _wm->_colorBlack : _wm->_colorWhite));
			}

			fillRect(g, x + width - 13, y + size + ARROW_H, 8, height - 2 * size - 1 - ARROW_H * 2, _wm->_colorBlack);

			y1 += height - 2 * size - ARROW_H - 2;
			for (int yy = 0; yy < ARROW_H; yy++) {
				for (int xx = 0; xx < ARROW_W; xx++)
					g->hLine(x1 + xx, y1 + yy, x1 + xx, (arrowPixels[ARROW_H - yy - 1][xx] != 0 ? _wm->_colorBlack : _wm->_colorWhite));
			}

			if (_highlightedPart == kBorderScrollUp || _highlightedPart == kBorderScrollDown) {
				int rx1 = x + width - kBorderWidth + 2;
				int ry1 = y + size + _dims.height() * _scrollPos;
				int rx2 = rx1 + size - 4;
				int ry2 = ry1 + _dims.height() * _scrollSize;
				Common::Rect rr(rx1, ry1, rx2, ry2);

				localColorWhite = _wm->_colorWhite;
				localColorBlack = _wm->_colorBlack;

				Graphics::drawFilledRect(rr, _wm->_colorBlack, drawPixelInverted, g);
			}
		}
		if (closeable) {
			if (_highlightedPart == kBorderCloseButton) {
				fillRect(g, x + 6, y + 6, 6, 6, _wm->_colorBlack);
			} else {
				drawBox(g, x + 5, y + 5, 7, 7);
			}
		}
	}

	if (drawTitle) {
		const Graphics::Font *font = getTitleFont();
		int yOff = _wm->_fontMan->hasBuiltInFonts() ? 3 : 1;

		int w = font->getStringWidth(_title) + 10;
		int maxWidth = width - size * 2 - 7;
		if (w > maxWidth)
			w = maxWidth;
		drawBox(g, x + (width - w) / 2, y, w, size);
		font->drawString(g, _title, x + (width - w) / 2 + 5, y + yOff, w, _wm->_colorBlack);
	}
}

void MacWindow::drawPattern() {
	byte *pat = _wm->getPatterns()[_pattern - 1];
	for (int y = 0; y < _surface.h; y++) {
		for (int x = 0; x < _surface.w; x++) {
			byte *dst = (byte *)_surface.getBasePtr(x, y);
			if (pat[y % 8] & (1 << (7 - (x % 8))))
				*dst = _wm->_colorBlack;
			else
				*dst = _wm->_colorWhite;
		}
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

void MacWindow::loadBorder(Common::SeekableReadStream &file, bool active, int lo, int ro, int to, int bo) {
	Image::BitmapDecoder bmpDecoder;
	Graphics::Surface *source;
	Graphics::TransparentSurface *surface = new Graphics::TransparentSurface();

	bmpDecoder.loadStream(file);
	source = bmpDecoder.getSurface()->convertTo(surface->getSupportedPixelFormat(), bmpDecoder.getPalette());

	surface->create(source->w, source->h, surface->getSupportedPixelFormat());
	surface->copyFrom(*source);
	surface->applyColorKey(255, 0, 255, false);

	if (active)
		_macBorder.addActiveBorder(surface);
	else
		_macBorder.addInactiveBorder(surface);

	if (!_macBorder.hasOffsets())
		_macBorder.setOffsets(lo, ro, to, bo);

	updateInnerDims();
	source->free();
	delete source;
}

void MacWindow::setCloseable(bool closeable) {
	_closeable = closeable;
}

void MacWindow::drawBox(ManagedSurface *g, int x, int y, int w, int h) {
	Common::Rect r(x, y, x + w + 1, y + h + 1);

	g->fillRect(r, _wm->_colorWhite);
	g->frameRect(r, _wm->_colorBlack);
}

void MacWindow::fillRect(ManagedSurface *g, int x, int y, int w, int h, int color) {
	Common::Rect r(x, y, x + w, y + h);

	g->fillRect(r, color);
}

WindowClick MacWindow::isInBorder(int x, int y) {
	if (_innerDims.contains(x, y))
		return kBorderInner;

	if (isInCloseButton(x, y))
		return kBorderCloseButton;

	if (_resizable)
		if (isInResizeButton(x, y))
			return kBorderResizeButton;

	if (_scrollable)
	 	return isInScroll(x, y);

	return kBorderBorder;
}

bool MacWindow::isInCloseButton(int x, int y) {
	int bLeft = kBorderWidth;
	int bTop = kBorderWidth;
	if (_macBorder.hasOffsets()) {
		bLeft = _macBorder.getOffset(kBorderOffsetLeft);
		bTop = _macBorder.getOffset(kBorderOffsetTop);
	}
	return (x >= _innerDims.left - bLeft && x < _innerDims.left && y >= _innerDims.top - bTop && y < _innerDims.top);
}

bool MacWindow::isInResizeButton(int x, int y) {
	int bRight = kBorderWidth;
	int bBottom = kBorderWidth;
	if (_macBorder.hasOffsets()) {
		bRight = _macBorder.getOffset(kBorderOffsetRight);
		bBottom = _macBorder.getOffset(kBorderOffsetBottom);
	}
	return (x >= _innerDims.right && x < _innerDims.right + bRight && y >= _innerDims.bottom && y < _innerDims.bottom + bBottom);
}

WindowClick MacWindow::isInScroll(int x, int y) {
	int bTop = kBorderWidth;
	int bRight = kBorderWidth;
	int bBottom = kBorderWidth;
	if (_macBorder.hasOffsets()) {
		bTop = _macBorder.getOffset(kBorderOffsetTop);
		bRight = _macBorder.getOffset(kBorderOffsetRight);
		bBottom = _macBorder.getOffset(kBorderOffsetBottom);
	}

	if (x >= _innerDims.right && x < _innerDims.right + bRight) {
		if (y < _innerDims.top - bTop)
			return kBorderBorder;

		if (y >= _innerDims.bottom + bBottom)
			return kBorderBorder;

		if (y >= _innerDims.top + _innerDims.height() / 2)
			return kBorderScrollDown;

		return kBorderScrollUp;
	}

	if (y >= _innerDims.bottom && y < _innerDims.bottom + bBottom) {
		if (x < _innerDims.left - bTop)
			return kBorderBorder;

		if (x >= _innerDims.right + bRight)
			return kBorderBorder;

		if (x >= _innerDims.left + _innerDims.width() / 2)
			return kBorderScrollRight;

		return kBorderScrollLeft;
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
			resize(MAX(_borderWidth * 4, _dims.width()  + event.mouse.x - _draggedX),
				   MAX(_borderWidth * 4, _dims.height() + event.mouse.y - _draggedY));

			_draggedX = event.mouse.x;
			_draggedY = event.mouse.y;

			_wm->setFullRefresh(true);

			if (_callback)
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

		if (click == kBorderCloseButton && _closeable) {
			_wm->removeWindow(this);
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

	if (_callback)
		return (*_callback)(click, event, _dataPtr);
	else
		return false;
}

} // End of namespace Wage
