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

#include "common/system.h"

#include "graphics/macgui/macwindowborder.h"
#include "graphics/macgui/macwindowmanager.h"

namespace Graphics {

using namespace Graphics::MacGUIConstants;

MacWindowBorder::MacWindowBorder() {

	_borderInitialized = Common::Array<bool>(_borderTypeNum);
	_border = Common::Array<NinePatchBitmap *>(_borderTypeNum);
	_lazyTag = Common::Array<bool>(_borderTypeNum);
	_window = nullptr;

	for (uint32 i = 0; i < _borderTypeNum; i++) {
		_border[i] = nullptr;
		_borderInitialized[i] = false;
		_lazyTag[i] = false;
	}

	_borderOffsets.left = -1;
	_borderOffsets.right = -1;
	_borderOffsets.top = -1;
	_borderOffsets.bottom = -1;
	_borderOffsets.titleTop = -1;
	_borderOffsets.titleBottom = -1;
	_borderOffsets.dark = false;
}

MacWindowBorder::~MacWindowBorder() {
	for (uint32 i = 0; i < _borderTypeNum; i++) {
		if (_border[i])
			delete _border[i];
	}
}

bool MacWindowBorder::hasBorder(uint32 flags) {
	if (flags >= _borderTypeNum) {
		warning("Accessing non-existed border type, %d", flags);
		return false;
	}
	if (_lazyTag[flags]) {
		_lazyTag[flags] = false;
		_window->loadLazyBorder(flags);
	}
	return _borderInitialized[flags];
}

void MacWindowBorder::addBorder(TransparentSurface *source, uint32 flags, int titlePos, int titleWidth) {
	if (flags >= _borderTypeNum) {
		warning("Accessing non-existed border type");
		return;
	}
	if (_border[flags])
		delete _border[flags];

	_border[flags] = new NinePatchBitmap(source, true, titlePos, titleWidth);
	_borderInitialized[flags] = true;

	if (_border[flags]->getPadding().isValidRect() && _border[flags]->getPadding().left > -1 && _border[flags]->getPadding().top > -1)
		setOffsets(_border[flags]->getPadding());
}

void MacWindowBorder::modifyTitleWidth(uint32 flags, int titleWidth) {
	if (flags >= _borderTypeNum) {
		warning("Accessing non-existed border type");
		return;
	}
	if (!_borderInitialized[flags]) {
		warning("Trying to modify title width of an uninitialized border");
		return;
	}
	_border[flags]->modifyTitleWidth(titleWidth);
}

bool MacWindowBorder::hasOffsets() {
	return _borderOffsets.left > -1 && _borderOffsets.right > -1
		&& _borderOffsets.top > -1 && _borderOffsets.bottom > -1;
}

void MacWindowBorder::setOffsets(int left, int right, int top, int bottom) {
	_borderOffsets.left = left;
	_borderOffsets.right = right;
	_borderOffsets.top = top;
	_borderOffsets.bottom = bottom;
}

void MacWindowBorder::setOffsets(Common::Rect &rect) {
	_borderOffsets.left = rect.left;
	_borderOffsets.right = rect.right;
	_borderOffsets.top = rect.top;
	_borderOffsets.bottom = rect.bottom;
}

void MacWindowBorder::setOffsets(const BorderOffsets &offsets) {
	_borderOffsets = offsets;
}

BorderOffsets &MacWindowBorder::getOffset() {
	return _borderOffsets;
}

void MacWindowBorder::setTitle(const Common::String& title, int width, MacWindowManager *wm) {
	_title = title;
	const Graphics::Font *font = wm->_fontMan->getFont(Graphics::MacFont(kMacFontChicago, 12));
	int sidesWidth = getOffset().left + getOffset().right;
	int titleWidth = font->getStringWidth(_title) + 10;
	int maxWidth = MAX<int>(width - sidesWidth - 7, 0);
	if (titleWidth > maxWidth)
		titleWidth = maxWidth;

	// if titleWidth is changed, then we modify it
	// here, we change all the border that has title
	for (uint32 i = 0; i < _borderTypeNum; i++) {
		if (_borderInitialized[i] && (i & kWindowBorderTitle))
			_border[i]->modifyTitleWidth(titleWidth);
	}
}

void MacWindowBorder::lazyLoad(uint32 flags) {
	if (flags >= _borderTypeNum) {
		warning("trying to load non-existing border type");
		return;
	}
	_lazyTag[flags] = true;
}

void MacWindowBorder::drawScrollBar(ManagedSurface *g, MacWindowManager *wm) {
	// here, we first check the _scrollSize, and if it is negative, then we don't draw the scrollBar
	if (_scrollSize < 0)
		return;
	int size = kBorderWidth;
	int rx1 = 0 + g->w - size + 2;
	int ry1 = 0 + size + _scrollPos + 1;
	int rx2 = rx1 + size - 6;
	int ry2 = ry1 + _scrollSize ;
	Common::Rect rr(rx1, ry1, rx2, ry2);

	MacPlotData pd(g, nullptr,  &wm->getPatterns(), 1, 0, 0, 1, wm->_colorWhite, true);
	Graphics::drawFilledRect(rr, wm->_colorWhite, wm->getDrawInvertPixel(), &pd);

	// after drawing, we set the _scrollSize negative, to indicate no more drawing is needed
	_scrollSize = -1;
}

void MacWindowBorder::drawTitle(ManagedSurface *g, MacWindowManager *wm, int titleOffset) {
	const Graphics::Font *font = wm->_fontMan->getFont(Graphics::MacFont(kMacFontChicago, 12));
	int width = g->w;
	int titleColor = wm->_colorBlack;
	int titleY = getOffset().titleTop;
	int sidesWidth = getOffset().left + getOffset().right;
	int titleWidth = font->getStringWidth(_title) + 10;
	int yOff = wm->_fontMan->hasBuiltInFonts() ? 3 : 1;
	int maxWidth = width - sidesWidth - 7;
	if (titleWidth > maxWidth)
		titleWidth = maxWidth;

	font->drawString(g, _title, titleOffset + 5, titleY + yOff, titleWidth, titleColor);
}

void MacWindowBorder::blitBorderInto(ManagedSurface &destination, uint32 flags, MacWindowManager *wm) {
	if (flags >= _borderTypeNum) {
		warning("Accessing non-existed border type");
		return;
	}

	TransparentSurface srf;
	NinePatchBitmap *src = _border[flags];

	if (!_borderInitialized[flags]) {
		warning("Attempt to blit uninitialized border");
	}

	if (destination.w == 0 || destination.h == 0) {
		warning("Attempt to draw %d x %d window", destination.w, destination.h);
		return;
	}
	
	// we add a special check here, if we have title but the titleWidth is zero, then we try to recalc it
	if ((flags & kWindowBorderTitle) && _border[flags]->getTitleWidth() == 0) {
		setTitle(_title, destination.w, wm);
	}

	srf.create(destination.w, destination.h, destination.format);
	srf.fillRect(Common::Rect(srf.w, srf.h), wm->_colorGreen2);

	src->blit(srf, 0, 0, srf.w, srf.h, NULL, 0, wm);
	destination.transBlitFrom(srf, wm->_colorGreen2);
	srf.free();

	if (flags & kWindowBorderTitle)
		drawTitle(&destination, wm, src->getTitleOffset());

	if (flags & kWindowBorderScrollbar)
		drawScrollBar(&destination, wm);
}

} // End of namespace Graphics
