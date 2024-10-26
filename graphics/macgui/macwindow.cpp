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

	_draggable = true;
}

void BaseMacWindow::setVisible(bool visible, bool silent) { _visible = visible; _wm->setFullRefresh(true); }

bool BaseMacWindow::isVisible() { return _visible; }

MacWindow::MacWindow(int id, bool scrollable, bool resizable, bool editable, MacWindowManager *wm) :
		BaseMacWindow(id, editable, wm), _scrollable(scrollable), _resizable(resizable) {
	_borderIsDirty = true;

	_pattern = 0;
	_hasPattern = false;

	_highlightedPart = kBorderNone;

	_beingDragged = false;
	_beingResized = false;

	_draggedX = _draggedY = 0;

	_type = kWindowWindow;

	_closeable = false;
	_isTitleVisible = true;

	_borderType = -1;
	_borderWidth = kBorderWidth;

	_macBorder.setWindow(this);

	_hasScrollBar = false;

	_mode = 0;
}

void MacWindow::disableBorder() {
	_macBorder.disableBorder();
}

const Font *MacWindow::getTitleFont() {
	return _wm->_fontMan->getFont(Graphics::MacFont(kMacFontChicago, 12));
}

void MacWindow::setActive(bool active) {
	MacWidget::setActive(active);

	_borderIsDirty = true;
}

bool MacWindow::isActive() const { return _active; }

void MacWindow::resize(int w, int h) {
	if (_composeSurface->w == w && _composeSurface->h == h)
		return;

	_dims.setWidth(w);
	_dims.setHeight(h);
	updateInnerDims();

	rebuildSurface();
}

void MacWindow::resizeInner(int w, int h) {
	if (_composeSurface->w == w && _composeSurface->h == h)
		return;

	_innerDims.setWidth(w);
	_innerDims.setHeight(h);
	updateOuterDims();

	rebuildSurface();
}

void MacWindow::rebuildSurface() {
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

void MacWindow::setInnerDimensions(const Common::Rect &r) {
	resizeInner(r.width(), r.height());
	_innerDims.moveTo(r.left, r.top);
	updateOuterDims();

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

uint32 MacWindow::getBorderFlags() const {
	uint32 flags = 0;
	if (_active)
		flags |= kWindowBorderActive;
	if (!_title.empty() && _borderType != 0x02 && _borderType != 0x03 && _borderType != 0x0a && _borderType != 0x0b)
		flags |= kWindowBorderTitle;
	if (_hasScrollBar)
		flags |= kWindowBorderScrollbar;
	return flags;
}

void MacWindow::center(bool toCenter) {
	if (!_wm)
		return;

	Common::Rect screen = _wm->getScreenBounds();

	uint32 flags = getBorderFlags();
	if (toCenter) {
		move((screen.width() - _dims.width()) / 2, (screen.height() - _dims.height()) / 2);
	} else if (_macBorder.hasBorder(flags) && _macBorder.hasOffsets()) {
		move(_macBorder.getOffset().left, _macBorder.getOffset().top);
	} else {
		move(0, 0);
	}
}

void MacWindow::updateInnerDims() {
	if (_dims.isEmpty())
		return;

	uint32 flags = getBorderFlags();

	if (_macBorder.hasBorder(flags) && _macBorder.hasOffsets()) {
		_innerDims = Common::Rect(
			_dims.left + _macBorder.getOffset().left,
			_dims.top + _macBorder.getOffset().top,
			_dims.right - _macBorder.getOffset().right,
			_dims.bottom - _macBorder.getOffset().bottom);
	} else {
		_innerDims = _dims;
		_innerDims.grow(-kBorderWidth);
	}
	// Prevent negative dimensions
	_innerDims.right = MAX(_innerDims.left, _innerDims.right);
	_innerDims.bottom = MAX(_innerDims.top, _innerDims.bottom);
}

void MacWindow::updateOuterDims() {
	if (_innerDims.isEmpty())
		return;

	uint32 flags = getBorderFlags();

	if (_macBorder.hasBorder(flags) && _macBorder.hasOffsets()) {
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
	resizeBorderSurface();

	_borderIsDirty = false;

	ManagedSurface *g = &_borderSurface;

	uint32 flags = getBorderFlags();

	if (_macBorder.hasBorder(flags)) {
		drawBorderFromSurface(g, flags);
	} else {
		warning("MacWindow: No Border Loaded");
		setBorderType(0xff);
		return;
	}

	if (_highlightedPart == kBorderScrollUp || _highlightedPart == kBorderScrollDown)
		setHighlight(kBorderNone);
}

void MacWindow::drawBorderFromSurface(ManagedSurface *g, uint32 flags) {
	if (_wm->_pixelformat.bytesPerPixel == 1) {
		g->clear(_wm->_colorGreen);
	}

	_macBorder.blitBorderInto(*g, flags, _wm);
}

void MacWindow::setTitle(const Common::String &title) {
	if (!_isTitleVisible) {
		// Title hidden right now, so don't propagate the change but just cache it up for later
		_shadowedTitle = title;
		return;
	}

	_title = title;
	_borderIsDirty = true;
	_macBorder.setTitle(title, _borderSurface.w, _wm);
}

void MacWindow::setTitleVisible(bool visible) {
	if (_isTitleVisible && !visible) {
		_shadowedTitle = _title;
		setTitle("");
		_isTitleVisible = visible;
	} else if (!_isTitleVisible && visible) {
		_title = _shadowedTitle;
		_isTitleVisible = visible;
		setTitle(_title);
	}
}

bool MacWindow::isTitleVisible() {
	return _isTitleVisible;
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
	_macBorder.setScroll(scrollPos, scrollSize);
	_borderIsDirty = true;
}

void MacWindow::loadBorder(Common::SeekableReadStream &file, uint32 flags, int lo, int ro, int to, int bo) {
	_macBorder.loadBorder(file, flags, lo, ro, to, bo);
}

void MacWindow::loadBorder(Common::SeekableReadStream &file, uint32 flags, BorderOffsets offsets) {
	_macBorder.loadBorder(file, flags, offsets);
}

void MacWindow::setBorder(Graphics::ManagedSurface *surface, uint32 flags, BorderOffsets offsets) {
	_macBorder.setBorder(surface, flags, offsets);
}

void MacWindow::resizeBorderSurface() {
	updateOuterDims();

	if (_borderSurface.w != _dims.width() || _borderSurface.h != _dims.height()) {
		_borderSurface.free();
		_borderSurface.create(_dims.width(), _dims.height(), _wm->_pixelformat);
	}
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

WindowClick MacWindow::isInBorder(int x, int y) const {
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

bool MacWindow::isInCloseButton(int x, int y) const {
	int bLeft = kBorderWidth;
	int bTop = kBorderWidth;
	if (_macBorder.hasOffsets()) {
		bLeft = _macBorder.getOffset().left;
		bTop = _macBorder.getOffset().top;
	}
	return (x >= _innerDims.left - bLeft && x < _innerDims.left && y >= _innerDims.top - bTop && y < _innerDims.top);
}

bool MacWindow::isInResizeButton(int x, int y) const {
	int bRight = kBorderWidth;
	int bBottom = kBorderWidth;
	if (_macBorder.hasOffsets()) {
		bRight = _macBorder.getOffset().right;
		bBottom = _macBorder.getOffset().bottom;
	}
	return (x >= _innerDims.right && x < _innerDims.right + bRight && y >= _innerDims.bottom && y < _innerDims.bottom + bBottom);
}

WindowClick MacWindow::isInScroll(int x, int y) const {
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
		if (_wm->_mouseDown && _wm->_hoveredWidget && !_wm->_hoveredWidget->_dims.contains(event.mouse.x - _dims.left, event.mouse.y - _dims.top)) {
			_wm->_hoveredWidget->setActive(false);
			// since we de-active the hoveredWidget, so we need to check whether it's the activeWidget of wm
			if (_wm->getActiveWidget() == _wm->_hoveredWidget)
				_wm->setActiveWidget(nullptr);
			_wm->_hoveredWidget = nullptr;
		}

		if (_beingDragged && _draggable) {
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

	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
		if (_wm->getActiveWidget() && _wm->getActiveWidget()->processEvent(event))
			return true;
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
		_macBorder.disableBorder();
	} else {
		_macBorder.setBorderType(borderType);
	}
}

void MacWindow::loadInternalBorder(uint32 flags) {
	_macBorder.loadInternalBorder(flags);
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
