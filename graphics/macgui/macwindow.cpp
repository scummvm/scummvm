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
#include "graphics/macgui/macwidget.h"
#include "image/bmp.h"

namespace Graphics {

BaseMacWindow::BaseMacWindow(int id, bool editable, MacWindowManager *wm) :
	MacWidget(nullptr, 0, 0, 0, 0, wm, true), _id(id), _editable(editable) {
	_callback = 0;
	_dataPtr = 0;

	_contentIsDirty = true;

	_type = kWindowUnknown;

	_visible = true;
}

void BaseMacWindow::setVisible(bool visible, bool silent) { _visible = visible; _wm->setFullRefresh(true); }

bool BaseMacWindow::isVisible() { return _visible; }

MacWindow::MacWindow(int id, bool scrollable, bool resizable, bool editable, MacWindowManager *wm) :
		BaseMacWindow(id, editable, wm), _scrollable(scrollable), _resizable(resizable) {
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

	_borderType = -1;
	_borderWidth = kBorderWidth;
}

static const byte noborderData[3][3] = {
	{ 0, 1, 0 },
	{ 1, 0, 1 },
	{ 0, 1, 0 },
};

void MacWindow::disableBorder() {
	Graphics::TransparentSurface *noborder = new Graphics::TransparentSurface();
	noborder->create(3, 3, noborder->getSupportedPixelFormat());
	uint32 colorBlack = noborder->getSupportedPixelFormat().RGBToColor(0, 0, 0);
	uint32 colorPink = noborder->getSupportedPixelFormat().RGBToColor(255, 0, 255);

	for (int y = 0; y < 3; y++)
		for (int x = 0; x < 3; x++)
			*((uint32 *)noborder->getBasePtr(x, y)) = noborderData[y][x] ? colorBlack : colorPink;

	setBorder(noborder, true);

	Graphics::TransparentSurface *noborder2 = new Graphics::TransparentSurface(*noborder, true);
	setBorder(noborder2, false);
}

const Font *MacWindow::getTitleFont() {
	return _wm->_fontMan->getFont(Graphics::MacFont(kMacFontChicago, 12));
}

void MacWindow::setActive(bool active) {
	MacWidget::setActive(active);

	_borderIsDirty = true;
}

bool MacWindow::isActive() { return _active; }

void MacWindow::resize(int w, int h, bool inner) {
	if (_composeSurface->w == w && _composeSurface->h == h)
		return;

	if (inner) {
		_innerDims.setWidth(w);
		_innerDims.setHeight(h);
		updateOuterDims();
	} else {
		_dims.setWidth(w);
		_dims.setHeight(h);
		updateInnerDims();
	}

	_composeSurface->free();
	_composeSurface->create(_innerDims.width(), _innerDims.height(), _wm->_pixelformat);

	if (_hasPattern)
		drawPattern();

	_borderSurface.free();
	_borderSurface.create(_dims.width(), _dims.height(), _wm->_pixelformat);

	_contentIsDirty = true;
	_borderIsDirty = true;
	_wm->setFullRefresh(true);
}

void MacWindow::move(int x, int y) {
	if (_dims.left == x && _dims.top == y)
		return;

	_dims.moveTo(x, y);
	updateInnerDims();

	_contentIsDirty = true;
	_wm->setFullRefresh(true);
}

void MacWindow::setDimensions(const Common::Rect &r) {
	resize(r.width(), r.height());
	_dims.moveTo(r.left, r.top);
	updateInnerDims();

	_contentIsDirty = true;
	_wm->setFullRefresh(true);
}

void MacWindow::setBackgroundPattern(int pattern) {
	_pattern = pattern;
	_hasPattern = true;
	drawPattern();
	_contentIsDirty = true;
}

bool MacWindow::draw(bool forceRedraw) {
	if (!_borderIsDirty && !_contentIsDirty && !forceRedraw)
		return false;

	if (_borderIsDirty || forceRedraw)
		drawBorder();

	_contentIsDirty = false;

	return true;
}

bool MacWindow::draw(ManagedSurface *g, bool forceRedraw) {
	if (!draw(forceRedraw))
		return false;

	g->blitFrom(*_composeSurface, Common::Rect(0, 0, _composeSurface->w, _composeSurface->h), Common::Point(_innerDims.left, _innerDims.top));

	uint32 transcolor = (_wm->_pixelformat.bytesPerPixel == 1) ? _wm->_colorGreen : 0;

	g->transBlitFrom(_borderSurface, Common::Rect(0, 0, _borderSurface.w, _borderSurface.h), Common::Point(_dims.left, _dims.top), transcolor);

	return true;
}

void MacWindow::blit(ManagedSurface *g, Common::Rect &dest) {
	// Only the inner surface is blitted here
	uint32 transcolor = (_wm->_pixelformat.bytesPerPixel == 1) ? _wm->_colorGreen2 : 0;

	g->transBlitFrom(*_composeSurface, _composeSurface->getBounds(), dest, transcolor);
}

void MacWindow::center(bool toCenter) {
	if (!_wm)
		return;

	Common::Rect screen = _wm->getScreenBounds();

	if (toCenter) {
		move((screen.width() - _dims.width()) / 2, (screen.height() - _dims.height()) / 2);
	} else if (_macBorder.hasBorder(_active) && _macBorder.hasOffsets()) {
		move(_macBorder.getOffset().left, _macBorder.getOffset().top);
	} else {
		move(0, 0);
	}
}

void MacWindow::updateInnerDims() {
	if (_dims.isEmpty())
		return;

	if (_macBorder.hasBorder(_active) && _macBorder.hasOffsets()) {
		_innerDims = Common::Rect(
			_dims.left + _macBorder.getOffset().left,
			_dims.top + _macBorder.getOffset().top,
			_dims.right - _macBorder.getOffset().right,
			_dims.bottom - _macBorder.getOffset().bottom);
	} else {
		_innerDims = _dims;
		_innerDims.grow(-kBorderWidth);
	}
}

void MacWindow::updateOuterDims() {
	if (_innerDims.isEmpty())
		return;

	if (_macBorder.hasBorder(_active) && _macBorder.hasOffsets()) {
		_dims = Common::Rect(
			_innerDims.left - _macBorder.getOffset().left,
			_innerDims.top - _macBorder.getOffset().top,
			_innerDims.right + _macBorder.getOffset().right,
			_innerDims.bottom + _macBorder.getOffset().bottom);
	} else {
		_dims = _innerDims;
		_dims.grow(kBorderWidth);
	}
}

void MacWindow::drawBorder() {
	_borderIsDirty = false;

	ManagedSurface *g = &_borderSurface;
	int width = _borderSurface.w;

	if (_macBorder.hasBorder(_active)) {

		if (_title.empty())
			drawBorderFromSurface(g);
		else {
			const Graphics::Font *font = getTitleFont();
			int titleColor = _wm->_colorBlack;
			int titleY = _macBorder.getOffset().titleTop;
			int sidesWidth = _macBorder.getOffset().left + _macBorder.getOffset().right;
			int titleWidth = font->getStringWidth(_title) + 10;
			int yOff = _wm->_fontMan->hasBuiltInFonts() ? 3 : 1;
			int maxWidth = width - sidesWidth - 7;
			if (titleWidth > maxWidth)
				titleWidth = maxWidth;

			// if titleWidth is changed, then we modify it
			if (titleWidth != _macBorder.getTitleWidth())
				_macBorder.modifyTitleWidth(titleWidth);

			drawBorderFromSurface(g);
			font->drawString(g, _title, (width - titleWidth) / 2 + 5, titleY + yOff, titleWidth, titleColor);
		}
	} else {
		warning("MacWindow: No Border Loaded");
		setBorderType(0xff);
		return;
	}

	// draw highlight scroll bar
	if (_highlightedPart == kBorderScrollUp || _highlightedPart == kBorderScrollDown) {
		int size = _borderWidth;
		int rx1 = 0 + width - size + 2;
		int ry1 = 0 + size + _scrollPos + 1;
		int rx2 = rx1 + size - 6;
		int ry2 = ry1 + _scrollSize ;
		Common::Rect rr(rx1, ry1, rx2, ry2);

		MacPlotData pd(g, nullptr,  &_wm->getPatterns(), 1, 0, 0, 1, _wm->_colorWhite, true);
		Graphics::drawFilledRect(rr, _wm->_colorWhite, _wm->getDrawInvertPixel(), &pd);
		setHighlight(kBorderNone);
	}
}

void MacWindow::drawBorderFromSurface(ManagedSurface *g) {
	if (_wm->_pixelformat.bytesPerPixel == 1) {
		g->clear(_wm->_colorGreen);
	}

	_macBorder.blitBorderInto(*g, _active, _wm);
}


void MacWindow::drawPattern() {
	byte *pat = _wm->getPatterns()[_pattern - 1];
	for (int y = 0; y < _composeSurface->h; y++) {
		for (int x = 0; x < _composeSurface->w; x++) {
			if (_wm->_pixelformat.bytesPerPixel == 1) {
				byte *dst = (byte *)_composeSurface->getBasePtr(x, y);
				if (pat[y % 8] & (1 << (7 - (x % 8))))
					*dst = _wm->_colorBlack;
				else
					*dst = _wm->_colorWhite;
			} else {
				uint32 *dst = (uint32 *)_composeSurface->getBasePtr(x, y);
				if (pat[y % 8] & (1 << (7 - (x % 8))))
					*dst = _wm->_colorBlack;
				else
					*dst = _wm->_colorWhite;
			}
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
	BorderOffsets offsets;
	offsets.left = lo;
	offsets.right = ro;
	offsets.top = to;
	offsets.bottom = bo;
	offsets.titleTop = -1;
	offsets.titleBottom = -1;
	offsets.dark = false;
	loadBorder(file, active, offsets);
}

void MacWindow::loadBorder(Common::SeekableReadStream &file, bool active, BorderOffsets offsets, int titlePos, int titleWidth) {
	Image::BitmapDecoder bmpDecoder;
	Graphics::Surface *source;
	Graphics::TransparentSurface *surface = new Graphics::TransparentSurface();

	bmpDecoder.loadStream(file);
	source = bmpDecoder.getSurface()->convertTo(surface->getSupportedPixelFormat(), bmpDecoder.getPalette());

	surface->create(source->w, source->h, _wm->_pixelformat);
	surface->copyFrom(*source);

	source->free();
	delete source;

	setBorder(surface, active, offsets, titlePos, titleWidth);
}

void MacWindow::setBorder(Graphics::TransparentSurface *surface, bool active, int lo, int ro, int to, int bo) {
	BorderOffsets offsets;
	offsets.left = lo;
	offsets.right = ro;
	offsets.top = to;
	offsets.bottom = bo;
	offsets.titleTop = -1;
	offsets.titleBottom = -1;
	offsets.dark = false;
	setBorder(surface, active, offsets);
}

void MacWindow::setBorder(Graphics::TransparentSurface *surface, bool active, BorderOffsets offsets, int titlePos, int titleWidth) {
	surface->applyColorKey(255, 0, 255, false);

	if (active)
		_macBorder.addActiveBorder(surface, titlePos, titleWidth);
	else
		_macBorder.addInactiveBorder(surface, titlePos, titleWidth);


	if (active && offsets.left + offsets.right + offsets.top + offsets.bottom > -4) { // Checking against default -1
		_macBorder.setOffsets(offsets);
		updateOuterDims();
		_borderSurface.free();
		_borderSurface.create(_dims.width(), _dims.height(), _wm->_pixelformat);
	}

	_borderIsDirty = true;
	_wm->setFullRefresh(true);
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
		bLeft = _macBorder.getOffset().left;
		bTop = _macBorder.getOffset().top;
	}
	return (x >= _innerDims.left - bLeft && x < _innerDims.left && y >= _innerDims.top - bTop && y < _innerDims.top);
}

bool MacWindow::isInResizeButton(int x, int y) {
	int bRight = kBorderWidth;
	int bBottom = kBorderWidth;
	if (_macBorder.hasOffsets()) {
		bRight = _macBorder.getOffset().right;
		bBottom = _macBorder.getOffset().bottom;
	}
	return (x >= _innerDims.right && x < _innerDims.right + bRight && y >= _innerDims.bottom && y < _innerDims.bottom + bBottom);
}

WindowClick MacWindow::isInScroll(int x, int y) {
	int bTop = kBorderWidth;
	int bRight = kBorderWidth;
	int bBottom = kBorderWidth;
	if (_macBorder.hasOffsets()) {
		bTop = _macBorder.getOffset().top;
		bRight = _macBorder.getOffset().right;
		bBottom = _macBorder.getOffset().bottom;
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
		if (_wm->_mouseDown && _wm->_hoveredWidget && !_wm->_hoveredWidget->_dims.contains(_dims.left - event.mouse.x, _dims.top - event.mouse.y)) {
			_wm->_hoveredWidget->setActive(false);
			_wm->_hoveredWidget = nullptr;
		}

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

	case Common::EVENT_KEYDOWN:
		if (!_editable && !(_wm->getActiveWidget() && _wm->getActiveWidget()->isEditable()))
			return false;

		if (_wm->getActiveWidget())
			return _wm->getActiveWidget()->processEvent(event);

		return false;

	default:
		return false;
	}

	event.mouse.x -= _dims.left;
	event.mouse.y -= _dims.top;

	MacWidget *w = findEventHandler(event, -_dims.left + _innerDims.left, -_dims.top + _innerDims.top);
	if (w && w != this) {
		_wm->_hoveredWidget = w;

		if (w->processEvent(event))
			return true;
	}

	if (_callback)
		return (*_callback)(click, event, _dataPtr);
	else
		return false;
}

void MacWindow::setBorderType(int borderType) {
	_borderType = borderType;
	if (borderType < 0) {
		disableBorder();
	} else {
		BorderOffsets offsets = _wm->getBorderOffsets(borderType);

		Common::SeekableReadStream *activeFile = _wm->getBorderFile(borderType, true);
		if (activeFile) {
			loadBorder(*activeFile, true, offsets);
			delete activeFile;
		}

		Common::SeekableReadStream *inactiveFile = _wm->getBorderFile(borderType, false);
		if (inactiveFile) {
			loadBorder(*inactiveFile, false, offsets);
			delete inactiveFile;
		}
	}
}

void MacWindow::addDirtyRect(const Common::Rect &r) {
	if (!r.isValidRect())
		return;

	Common::Rect bounds = r;
	bounds.clip(Common::Rect(_innerDims.width(), _innerDims.height()));

	if (bounds.width() > 0 && bounds.height() > 0)
		_dirtyRects.push_back(bounds);
}

void MacWindow::markAllDirty() {
	_dirtyRects.clear();
	_dirtyRects.push_back(Common::Rect(_composeSurface->w, _composeSurface->h));
}

void MacWindow::mergeDirtyRects() {
	Common::List<Common::Rect>::iterator rOuter, rInner;

	// Process the dirty rect list to find any rects to merge
	for (rOuter = _dirtyRects.begin(); rOuter != _dirtyRects.end(); ++rOuter) {
		rInner = rOuter;
		while (++rInner != _dirtyRects.end()) {

			if ((*rOuter).intersects(*rInner)) {
				// These two rectangles overlap, so merge them
				rOuter->extend(*rInner);

				// remove the inner rect from the list
				_dirtyRects.erase(rInner);

				// move back to beginning of list
				rInner = rOuter;
			}
		}
	}
}

} // End of namespace Graphics
