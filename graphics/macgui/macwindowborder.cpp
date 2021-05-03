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
#include "graphics/macgui/macfontmanager.h"

namespace Graphics {

using namespace Graphics::MacGUIConstants;

static const byte noborderData[3][3] = {
	{ 0, 1, 0 },
	{ 1, 0, 1 },
	{ 0, 1, 0 },
};

MacWindowBorder::MacWindowBorder() {

	_border = Common::Array<NinePatchBitmap *>(kWindowBorderMaxFlag);
	_window = nullptr;
	_useInternalBorder = false;

	for (uint32 i = 0; i < kWindowBorderMaxFlag; i++)
		_border[i] = nullptr;

	_borderOffsets.left = -1;
	_borderOffsets.right = -1;
	_borderOffsets.top = -1;
	_borderOffsets.bottom = -1;
	_borderOffsets.titleTop = -1;
	_borderOffsets.titleBottom = -1;
	_borderOffsets.dark = false;
	_borderOffsets.titlePos = 0;
	_borderOffsets.upperScrollHeight = 0;
	_borderOffsets.lowerScrollHeight = 0;

	_borderType = 0;

	_scrollSize = -1;
	_scrollPos = 0;
}

MacWindowBorder::~MacWindowBorder() {
	for (uint32 i = 0; i < kWindowBorderMaxFlag; i++) {
		if (_border[i])
			delete _border[i];
	}
}

bool MacWindowBorder::hasBorder(uint32 flags) {
	if (flags >= kWindowBorderMaxFlag) {
		warning("Accessing non-existed border type, %d", flags);
		return false;
	}
	if (_useInternalBorder && !_border[flags]) {
		loadInternalBorder(flags);
	}
	return _border[flags] != nullptr;
}

void MacWindowBorder::disableBorder() {
	Graphics::TransparentSurface *noborder = new Graphics::TransparentSurface();
	noborder->create(3, 3, noborder->getSupportedPixelFormat());
	uint32 colorBlack = noborder->getSupportedPixelFormat().RGBToColor(0, 0, 0);
	uint32 colorPink = noborder->getSupportedPixelFormat().RGBToColor(255, 0, 255);

	for (int y = 0; y < 3; y++)
		for (int x = 0; x < 3; x++)
			*((uint32 *)noborder->getBasePtr(x, y)) = noborderData[y][x] ? colorBlack : colorPink;

	setBorder(noborder, kWindowBorderActive);

	Graphics::TransparentSurface *noborder2 = new Graphics::TransparentSurface(*noborder, true);
	setBorder(noborder2, 0);
}

void MacWindowBorder::addBorder(TransparentSurface *source, uint32 flags, int titlePos) {
	if (flags >= kWindowBorderMaxFlag) {
		warning("Accessing non-existed border type");
		return;
	}
	if (_border[flags])
		delete _border[flags];

	_border[flags] = new NinePatchBitmap(source, true, titlePos);

	if (_border[flags]->getPadding().isValidRect() && _border[flags]->getPadding().left > -1 && _border[flags]->getPadding().top > -1)
		setOffsets(_border[flags]->getPadding());
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
	for (uint32 i = 0; i < kWindowBorderMaxFlag; i++) {
		if ((_border[i] != nullptr) && (i & kWindowBorderTitle))
			_border[i]->modifyTitleWidth(titleWidth);
	}
}

void MacWindowBorder::drawScrollBar(ManagedSurface *g, MacWindowManager *wm) {
	// here, we first check the _scrollSize, and if it is negative, then we don't draw the scrollBar
	if (_scrollSize < 0)
		return;
	int width = _borderOffsets.right;
	int height = _borderOffsets.upperScrollHeight;
	int rx1 = g->w - width + 2;
	int ry1 = height + _scrollPos;
	int rx2 = rx1 + width - 5;
	int ry2 = ry1 + _scrollSize ;
	Common::Rect rr(rx1, ry1, rx2, ry2);

	MacPlotData pd(g, nullptr,  &wm->getPatterns(), 1, 0, 0, 1, wm->_colorWhite, true);
	Graphics::drawFilledRect(rr, wm->_colorWhite, wm->getDrawInvertPixel(), &pd);

	// after drawing, we set the _scrollSize negative, to indicate no more drawing is needed
	// if win95 mode is enabled, then we keep on drawing the scrollbar
	if (!(wm->_mode & kWMModeWin95))
		_scrollSize = -1;
}

void MacWindowBorder::drawTitle(ManagedSurface *g, MacWindowManager *wm, int titleOffset) {
	const Graphics::Font *font = wm->_fontMan->getFont(Graphics::MacFont(kMacFontChicago, 12));
	int width = g->w;
	int titleColor = getOffset().dark ? wm->_colorWhite: wm->_colorBlack;
	int titleY = getOffset().titleTop;
	int sidesWidth = getOffset().left + getOffset().right;
	int titleWidth = font->getStringWidth(_title) + 10;
	int yOff = wm->_fontMan->hasBuiltInFonts() ? 3 : 1;
	int maxWidth = width - sidesWidth - 7;
	if (titleWidth > maxWidth)
		titleWidth = maxWidth;

	font->drawString(g, _title, titleOffset + 5, titleY + yOff, titleWidth, titleColor);
}

void MacWindowBorder::setBorderType(int type) {
	_useInternalBorder = true;
	_borderType = type;
}

void MacWindowBorder::loadBorder(Common::SeekableReadStream &file, uint32 flags, int lo, int ro, int to, int bo) {
	BorderOffsets offsets;
	offsets.left = lo;
	offsets.right = ro;
	offsets.top = to;
	offsets.bottom = bo;
	offsets.titleTop = -1;
	offsets.titleBottom = -1;
	offsets.titlePos = 0;
	offsets.dark = false;
	offsets.upperScrollHeight = 0;
	offsets.lowerScrollHeight = 0;

	loadBorder(file, flags, offsets);
}

void MacWindowBorder::loadBorder(Common::SeekableReadStream &file, uint32 flags, BorderOffsets offsets) {
	Image::BitmapDecoder bmpDecoder;
	Graphics::Surface *source;
	Graphics::TransparentSurface *surface = new Graphics::TransparentSurface();

	bmpDecoder.loadStream(file);
	source = bmpDecoder.getSurface()->convertTo(surface->getSupportedPixelFormat(), bmpDecoder.getPalette());

	surface->create(source->w, source->h, _window->_wm->_pixelformat);
	surface->copyFrom(*source);

	source->free();
	delete source;

	setBorder(surface, flags, offsets);
}

void MacWindowBorder::setBorder(Graphics::TransparentSurface *surface, uint32 flags, int lo, int ro, int to, int bo) {
	BorderOffsets offsets;
	offsets.left = lo;
	offsets.right = ro;
	offsets.top = to;
	offsets.bottom = bo;
	offsets.titleTop = -1;
	offsets.titleBottom = -1;
	offsets.titlePos = 0;
	offsets.dark = false;
	setBorder(surface, flags, offsets);
}

void MacWindowBorder::setBorder(Graphics::TransparentSurface *surface, uint32 flags, BorderOffsets offsets) {
	surface->applyColorKey(255, 0, 255, false);
	addBorder(surface, flags, offsets.titlePos);

	if ((flags & kWindowBorderActive) && offsets.left + offsets.right + offsets.top + offsets.bottom > -4) { // Checking against default -1
		setOffsets(offsets);
		_window->resizeBorderSurface();
	}

	_window->setBorderDirty(true);
	_window->_wm->setFullRefresh(true);
}

void MacWindowBorder::loadInternalBorder(uint32 flags) {
	if (_borderType < 0) {
		warning("trying to load non-existing internal border type");
		return;
	}
	BorderOffsets offsets = _window->_wm->getBorderOffsets(_borderType);
	Common::SeekableReadStream *file = _window->_wm->getBorderFile(_borderType, flags);
	if (file) {
		loadBorder(*file, flags, offsets);
		delete file;
	}
}

void MacWindowBorder::blitBorderInto(ManagedSurface &destination, uint32 flags, MacWindowManager *wm) {
	if (flags >= kWindowBorderMaxFlag) {
		warning("Accessing non-existed border type");
		return;
	}

	TransparentSurface srf;
	NinePatchBitmap *src = _border[flags];

	if (!src) {
		warning("Attempt to blit uninitialized border");
		return;
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
