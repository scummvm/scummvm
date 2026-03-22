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
 * Based on the original sources
 *   https://github.com/Croquetx/thecolony
 * Copyright (C) 1988, David A. Smith
 *
 * Distributed under Apache Version 2.0 License
 *
 */

#include "colony/colony.h"
#include "colony/renderer.h"
#include "common/system.h"
#include "common/util.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/events.h"
#include "graphics/palette.h"
#include "graphics/fontman.h"
#include "graphics/fonts/dosfont.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowborder.h"
#include "image/pict.h"
#include <math.h>

namespace Colony {

// Pack RGB into 32-bit ARGB with 0xFF000000 marker for direct RGB rendering.
static uint32 packRGB(byte r, byte g, byte b) {
	return 0xFF000000 | ((uint32)r << 16) | ((uint32)g << 8) | b;
}

// Pack Mac 16-bit RGB into 32-bit ARGB.
static uint32 packMacColorUI(const uint16 rgb[3]) {
	return 0xFF000000 | ((rgb[0] >> 8) << 16) | ((rgb[1] >> 8) << 8) | (rgb[2] >> 8);
}

static bool drawMacTextPopup(Graphics::MacWindowManager *wm, Renderer *gfx,
		int screenWidth, int screenHeight, int centerX, int centerY,
		const Common::Array<Common::String> &lines, Graphics::TextAlign align, bool macColor) {
	if (!gfx || lines.empty())
		return false;

	Graphics::MacFont systemFont(Graphics::kMacFontSystem, 12);
	const Graphics::Font *font = (wm && wm->_fontMan) ? wm->_fontMan->getFont(systemFont) : nullptr;
	if (!font)
		font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!font)
		font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	if (!font)
		return false;

	int textWidth = 0;
	for (uint i = 0; i < lines.size(); ++i)
		textWidth = MAX<int>(textWidth, font->getStringWidth(lines[i]));

	const int fontHeight = MAX<int>(1, font->getFontHeight());
	const int fontLeading = MAX<int>(0, font->getFontLeading());
	const int topPad = 8;
	const int bottomPad = 8;
	const int sidePad = 12;
	const int lineGap = MAX<int>(2, fontLeading);
	const int lineStep = fontHeight + lineGap;
	int popupWidth = CLIP<int>(textWidth + sidePad * 2, 96, MAX<int>(96, screenWidth - 16));
	int popupHeight = topPad + bottomPad + fontHeight +
		MAX<int>(0, (int)lines.size() - 1) * lineStep;
	Common::Rect bounds(8, 24, screenWidth - 8, screenHeight - 8);
	if (wm) {
		Graphics::MacWindowBorder border;
		border.setWindowManager(wm);
		border.setBorderType(Graphics::kWindowWindow);
		if (border.hasBorder(Graphics::kWindowBorderActive) && border.hasOffsets()) {
			const Graphics::BorderOffsets &offsets = border.getOffset();
			popupWidth = MAX<int>(border.getMinWidth(Graphics::kWindowBorderActive),
				textWidth + sidePad * 2 + offsets.left + offsets.right);
			popupHeight = MAX<int>(border.getMinHeight(Graphics::kWindowBorderActive),
				topPad + bottomPad + fontHeight + MAX<int>(0, (int)lines.size() - 1) * lineStep +
				offsets.top + offsets.bottom);

			Common::Rect r(centerX - popupWidth / 2, centerY - popupHeight / 2,
				centerX - popupWidth / 2 + popupWidth, centerY - popupHeight / 2 + popupHeight);
			if (r.left < bounds.left)
				r.translate(bounds.left - r.left, 0);
			if (r.right > bounds.right)
				r.translate(bounds.right - r.right, 0);
			if (r.top < bounds.top)
				r.translate(0, bounds.top - r.top);
			if (r.bottom > bounds.bottom)
				r.translate(0, bounds.bottom - r.bottom);

			Graphics::ManagedSurface popup;
			popup.create(popupWidth, popupHeight, wm->_pixelformat);
			popup.fillRect(Common::Rect(0, 0, popupWidth, popupHeight), popup.format.ARGBToColor(0, 0, 0, 0));
			Common::Rect inner(offsets.left, offsets.top, popupWidth - offsets.right, popupHeight - offsets.bottom);
			inner.clip(Common::Rect(0, 0, popupWidth, popupHeight));
			if (!inner.isEmpty())
				popup.fillRect(inner, wm->_colorWhite);
			border.blitBorderInto(popup, Graphics::kWindowBorderActive);

			const int textX = inner.left + sidePad;
			const int textY = inner.top + topPad;
			const int textW = MAX<int>(1, inner.width() - sidePad * 2);
			for (uint i = 0; i < lines.size(); ++i)
				font->drawString(&popup, lines[i], textX, textY + (int)i * lineStep, textW, wm->_colorBlack, align);

			gfx->drawSurface(&popup.rawSurface(), r.left, r.top);
			gfx->copyToScreen();
			popup.free();
			return true;
		}
	}

	Common::Rect r(centerX - popupWidth / 2, centerY - popupHeight / 2,
		centerX - popupWidth / 2 + popupWidth, centerY - popupHeight / 2 + popupHeight);
	if (r.left < bounds.left)
		r.translate(bounds.left - r.left, 0);
	if (r.right > bounds.right)
		r.translate(bounds.right - r.right, 0);
	if (r.top < bounds.top)
		r.translate(0, bounds.top - r.top);
	if (r.bottom > bounds.bottom)
		r.translate(0, bounds.bottom - r.bottom);

	const uint32 colBlack = macColor ? packRGB(0, 0, 0) : 0;
	const uint32 colWhite = macColor ? packRGB(255, 255, 255) : 15;
	const uint32 colShadow = macColor ? packRGB(96, 96, 96) : 0;

	Common::Rect shadow = r;
	shadow.translate(2, 2);
	gfx->fillRect(shadow, colShadow);
	gfx->fillRect(r, colWhite);
	gfx->drawRect(r, colBlack);
	Common::Rect inner = r;
	inner.grow(-2);
	if (!inner.isEmpty())
		gfx->drawRect(inner, colBlack);

	const int textLeft = r.left + sidePad;
	const int textRight = r.right - sidePad;
	const int textCenter = (textLeft + textRight) / 2;
	const int startY = r.top + topPad;
	for (uint i = 0; i < lines.size(); ++i) {
		const int y = startY + (int)i * lineStep;
		if (align == Graphics::kTextAlignCenter)
			gfx->drawString(font, lines[i], textCenter, y, colBlack, Graphics::kTextAlignCenter);
		else
			gfx->drawString(font, lines[i], textLeft, y, colBlack, Graphics::kTextAlignLeft);
	}

	gfx->copyToScreen();
	return true;
}

// Load a PICT resource from the Mac resource fork, returning a new RGB surface.
// Try Color Colony first (has color dashboard PICTs), then fall back to B&W Colony.
// Caller owns the returned surface. Returns nullptr on failure.
Graphics::Surface *ColonyEngine::loadPictSurface(int resID) {
	Common::SeekableReadStream *pictStream = nullptr;

	// Try Color Colony resource fork first
	if (_colorResMan && _colorResMan->hasResFork()) {
		pictStream = _colorResMan->getResource(MKTAG('P', 'I', 'C', 'T'), (int16)resID);
	}

	// Fall back to B&W Colony resource fork
	if (!pictStream && _resMan && (_resMan->isMacFile() || _resMan->hasResFork())) {
		pictStream = _resMan->getResource(MKTAG('P', 'I', 'C', 'T'), (int16)resID);
	}

	if (!pictStream) {
		warning("loadPictSurface(%d): PICT resource not found", resID);
		return nullptr;
	}

	::Image::PICTDecoder decoder;
	Graphics::Surface *result = nullptr;
	if (decoder.loadStream(*pictStream)) {
		const Graphics::Surface *src = decoder.getSurface();
		if (src) {
			// Convert to a persistent RGB surface (decoder surface is transient)
			// Detect 1-bit B&W PICTs where ScummVM's PICTDecoder inverts
			// the Mac QuickDraw convention. In QuickDraw, 1-bit bitmaps
			// use bit 0 = BackColor (white) and bit 1 = ForeColor (black).
			// The PICTDecoder maps index 0 → black instead of white.
			// We detect this by checking for CLUT8 with a tiny palette
			// (0-2 entries, typical of 1-bit PICTs) and invert.
			bool invert1bit = false;
			if (src->format == Graphics::PixelFormat::createFormatCLUT8()) {
				const Graphics::Palette &checkPal = decoder.getPalette();
				invert1bit = ((int)checkPal.size() <= 2);
			}

			result = new Graphics::Surface();
			result->create(src->w, src->h, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
			for (int y = 0; y < src->h; y++) {
				for (int x = 0; x < src->w; x++) {
					byte r, g, b;
					if (src->format == Graphics::PixelFormat::createFormatCLUT8()) {
						byte idx = *((const byte *)src->getBasePtr(x, y));
						if (invert1bit) {
							// QuickDraw 1-bit: index 0 = white, index 1 = black
							byte lum = (idx == 0) ? 255 : 0;
							r = g = b = lum;
						} else {
							const Graphics::Palette &pal = decoder.getPalette();
							if (idx < (int)pal.size()) {
								pal.get(idx, r, g, b);
							} else {
								r = g = b = 0;
							}
						}
					} else {
						uint32 pixel = src->getPixel(x, y);
						src->format.colorToRGB(pixel, r, g, b);
					}
					result->setPixel(x, y, result->format.ARGBToColor(255, r, g, b));
				}
			}
			warning("loadPictSurface(%d): %dx%d invert1bit=%d", resID, result->w, result->h, invert1bit ? 1 : 0);
		}
	}
	delete pictStream;
	return result;
}

// Draw a PICT surface at a specific destination position using packRGB.
// Matches original DrawPicture(pic, &rect) where rect is positioned at (destX, destY).
void ColonyEngine::drawPictAt(Graphics::Surface *surf, int destX, int destY) {
	if (!surf)
		return;
	for (int y = 0; y < surf->h; y++) {
		int sy = destY + y;
		if (sy < 0 || sy >= _height)
			continue;
		for (int x = 0; x < surf->w; x++) {
			int sx = destX + x;
			if (sx < 0 || sx >= _width)
				continue;
			uint32 pixel = surf->getPixel(x, y);
			byte a, r, g, b;
			surf->format.colorToARGB(pixel, a, r, g, b);
			_gfx->setPixel(sx, sy, packRGB(r, g, b));
		}
	}
}

void ColonyEngine::updateViewportLayout() {
	auto makeSafeRect = [](int left, int top, int right, int bottom) {
		if (right < left)
			right = left;
		if (bottom < top)
			bottom = top;
		return Common::Rect(left, top, right, bottom);
	};

	const bool isMac = (_renderMode == Common::kRenderMacintosh);

	// Original IBM_INIT.C: pix_per_Qinch = pixResX/4, pixResY/4
	// MetaWINDOW EGA 640x350: pixResX=96, pixResY=72 → pQx=24, pQy=18
	_pQx = 24;
	_pQy = 18;

	int dashWidth = 0;
	if (_showDashBoard) {
		if (isMac)
			dashWidth = MIN(96, _width / 2);
		else
			dashWidth = 4 * _pQx + 2; // DASHBOAR.C: DashBoard.right = 4*pix_per_Qinch_x+2
		if (_width - dashWidth < 160)
			dashWidth = 0;
	}

	const int menuTop = _menuBarHeight; // 0 for DOS/EGA, 20 for Mac

	// DOS IBM_INIT.C: screenR.left = sR.left + 4*pix_per_Qinch_x + 4 (2px gap)
	// Mac inits.c: screenR.left=96, screenR.bottom=rScreen.bottom-8 (8px bottom margin)
	const int viewportLeft = isMac ? dashWidth : (dashWidth > 0 ? 4 * _pQx + 4 : 0);
	const int viewportBottom = isMac ? (_height - 8) : _height;
	_screenR = makeSafeRect(viewportLeft, menuTop, _width, viewportBottom);
	_clip = _screenR;
	_centerX = (_screenR.left + _screenR.right) >> 1;
	_centerY = (_screenR.top + _screenR.bottom) >> 1;

	_dashBoardRect = makeSafeRect(0, menuTop, dashWidth, _height);
	if (dashWidth == 0) {
		_compassRect = Common::Rect(0, 0, 0, 0);
		_headsUpRect = Common::Rect(0, 0, 0, 0);
		_powerRect = Common::Rect(0, 0, 0, 0);
		return;
	}

	const int pad = 2;

	if (isMac) {
		// Original Mac layout from inits.c/compass.c/power.c:
		// screenR.left = 96  sidebar is 96px wide.
		// Two floating windows centered in sidebar over gray desktop.
		// moveWindow: compRect = (0,0, 2*CCENTER, 3*CCENTER) = (0,0, 70, 105)
		//   floorRect (minimap) = (8,8)-(62,62)  54x54 inside moveWindow
		//   compass dish below at (19,66)-(51,98), needle center at (35,82)
		// infoWindow: sized from PICT resource, positioned above moveWindow
		const int CCENTER = 35;

		// Load PICT surfaces (cached after first load)
		if (!_pictCompass)
			_pictCompass = loadPictSurface(-32757);
		if (!_pictPower) {
			// power.c DrawInfo(): armor → -32755/-32760; !armor → -32752/-32761
			// In B&W, -32752 doesn't exist. The original GetPicture returns null
			// and DrawPicture is a no-op — the panel stays blank until armor > 0.
			// Only fall back to -32755 when armored.
			int wantID;
			if (_armor > 0)
				wantID = -32755;
			else
				wantID = _hasMacColors ? -32761 : -32752;
			_pictPower = loadPictSurface(wantID);
			if (!_pictPower && _armor > 0 && wantID != -32755)
				_pictPower = loadPictSurface(-32755);
			_pictPowerID = _pictPower ? wantID : 0;
		}

		// moveWindow dimensions from original constants
		const int moveW = 2 * CCENTER; // 70
		const int moveH = 3 * CCENTER; // 105
		const int infoW = _pictPower ? _pictPower->w : moveW;
		const int infoH = _pictPower ? _pictPower->h : moveH;

		// Center panels horizontally in sidebar
		const int centerX = dashWidth / 2;

		// Position moveWindow at the bottom of the sidebar
		const int moveLeft = MAX(0, centerX - moveW / 2);
		const int moveTop = _height - pad - moveH;

		// _headsUpRect = floorRect (8,8)-(62,62) relative to moveWindow
		// This is the minimap clipping area  must NOT overlap compass dish
		_headsUpRect = makeSafeRect(moveLeft + 8, moveTop + 8,
		                            moveLeft + 2 * CCENTER - 8, moveTop + 2 * CCENTER - 8);

		// _compassRect = entire moveWindow (used for compass dish drawing)
		_compassRect = makeSafeRect(moveLeft, moveTop, moveLeft + moveW, moveTop + moveH);

		// Position infoWindow below the Mac menu bar.
		// Original PICT is drawn at (-2,-2) in window-local coords, so offset
		// the panel by 2px to prevent the PICT from overlapping the menu bar.
		const int infoLeft = MAX(0, centerX - infoW / 2);
		const int infoTop = menuTop + pad;
		_powerRect = makeSafeRect(infoLeft, infoTop, infoLeft + infoW, infoTop + infoH);
	} else {
		// DASHBOAR.C RCompass(): compOval before shrink
		// compOval.bottom = r->bottom - (pQy >> 2)
		// compOval.top = r->bottom - 4*pQy
		// compOval.left = 2, compOval.right = 4*pQx
		// Then shrink by 2px each side for EraseOval
		const int compBottom = _height - (_pQy >> 2) - 2;  // after shrink
		const int compTop = _height - 4 * _pQy + 2;        // after shrink
		const int compLeft = 2 + 2;                          // after shrink
		const int compRight = 4 * _pQx - 2;                  // after shrink
		_compassRect = makeSafeRect(compLeft, compTop, compRight, compBottom);

		// DASHBOAR.C RHeadsUp(): floorRect uses compOval.top (after shrink)
		// floorRect.bottom = (compOval.top - 4) - (pQy >> 2)
		// floorRect.top = (compOval.top - 4) - 4*pQy
		// floorRect.left = 2, floorRect.right = r->right - 2
		const int floorBottom = (compTop - 4) - (_pQy >> 2);
		const int floorTop = (compTop - 4) - 4 * _pQy;
		_headsUpRect = makeSafeRect(2, MAX(0, floorTop), dashWidth - 2, MAX(0, floorBottom));

		// DASHBOAR.C RPower(): powerRect layout
		// wd = r->right - r->left = dashWidth
		// twd = wd/3; twd--; twd>>=1; twd<<=1 (round down to even)
		// powerWidth = twd; twd *= 3; l = (wd - twd) >> 1
		int twd = dashWidth / 3;
		twd--;
		twd >>= 1;
		twd <<= 1;
		_powerWidth = twd;
		const int totalBarWidth = twd * 3;
		const int powerLeft = (dashWidth - totalBarWidth) >> 1;

		// powerRect.top = r->top + 8
		// powerRect.bottom = r->bottom - 2*(4*pQy + 8)
		const int pTop = 8;
		const int pBottom = _height - 2 * (4 * _pQy + 8);
		_powerHeight = (pBottom - pTop) / 32;
		_powerHeight = MIN(_powerHeight, 5);
		const int pBottomAdj = pTop + _powerHeight * 32;
		_powerRect = makeSafeRect(powerLeft, pTop, powerLeft + totalBarWidth, pBottomAdj);
	}
}

// DASHBOAR.C qlog(): bit-length of x (equivalent to floor(log2(x))+1)
int ColonyEngine::qlog(int32 x) {
	int i = 0;
	while (x > 0) {
		x >>= 1;
		i++;
	}
	return i;
}

// DASHBOAR.C DrawBarGraph(): draw a single power bar column
void ColonyEngine::drawDOSBarGraph(int x, int y, int height) {
	int color;
	if (height <= 3)
		color = 4; // vRED
	else
		color = 1; // vBLUE

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < _powerHeight - 1; j++) {
			int h = y - (2 + i * _powerHeight + j);
			if (h < _powerRect.top)
				return;
			// First line of each segment is black (separator), rest is color
			_gfx->drawLine(x + 2, h, x + _powerWidth - 2, h, (j == 0) ? 0 : color);
		}
	}
}

// DASHBOAR.C SetPower() display update: recompute epower[] from Me.power[]
void ColonyEngine::updateDOSPowerBars() {
	for (int i = 0; i < 3; i++)
		_epower[i] = qlog(_me.power[i]);
}

void ColonyEngine::drawDashboardStep1() {
	if (_dashBoardRect.width() <= 0 || _dashBoardRect.height() <= 0)
		return;

	const bool isMac = (_renderMode == Common::kRenderMacintosh);

	if (isMac) {
		drawDashboardMac();
		return;
	}

	// --- DOS/EGA path ---
	// Matches original DASHBOAR.C DrawDashBoard() → RConsole + RCompass + RHeadsUp + RPower

	// DrawDashBoard(): separator line at screenR.left-1 in vBLACK
	if (_screenR.left > 0)
		_gfx->drawLine(_screenR.left - 1, _screenR.top, _screenR.left - 1, _screenR.bottom - 1, 0);

	// RConsole(): fill dashboard with pattern, frame in vBLUE
	// FillRect(r, 3) with BackColor(vWHITE) = dither pattern using black+white
	_gfx->fillDitherRect(_dashBoardRect, 0, 7);
	_gfx->drawRect(_dashBoardRect, 1); // vBLUE frame

	// RCompass(): draw compass oval
	// _compassRect stores the post-shrink compOval (inner erasable area)
	if (_compassRect.width() > 2 && _compassRect.height() > 2) {
		// Original draws FillOval on the pre-shrink rect, then EraseOval on the shrunk rect.
		// Pre-shrink = _compassRect expanded by 2 on each side
		const int cx = (_compassRect.left + _compassRect.right) >> 1;
		const int cy = (_compassRect.top + _compassRect.bottom) >> 1;
		const int outerRx = (_compassRect.width() + 4) >> 1;
		const int outerRy = (_compassRect.height() + 4) >> 1;
		const int innerRx = _compassRect.width() >> 1;
		const int innerRy = _compassRect.height() >> 1;

		// FillOval: filled oval (vINTWHITE background)
		_gfx->fillEllipse(cx, cy, outerRx, outerRy, 15);
		// EraseOval: clear interior (also vINTWHITE)
		_gfx->fillEllipse(cx, cy, innerRx, innerRy, 15);
		// Frame the outer oval
		_gfx->drawEllipse(cx, cy, outerRx, outerRy, 0);

		// Compass needle: updateDashBoard() uses Me.ang
		const int ex = cx + ((_cost[_me.ang] * innerRx) >> 8);
		const int ey = cy - ((_sint[_me.ang] * innerRy) >> 8);
		_gfx->drawLine(cx, cy, ex, ey, 0); // vBLACK needle
	}

	// RHeadsUp(): minimap
	if (_headsUpRect.width() > 2 && _headsUpRect.height() > 2) {
		_gfx->fillRect(_headsUpRect, 15); // EraseRect (vINTWHITE)
		_gfx->drawRect(_headsUpRect, 0);  // FrameRect (vBLACK)
		drawMiniMap(0);
	}

	// RPower(): power bars (only when armored or armed)
	if (_powerRect.width() > 2 && _powerRect.height() > 2 && (_armor > 0 || _weapons > 0)) {
		_gfx->fillRect(_powerRect, 15); // EraseRect (vINTWHITE)
		_gfx->drawRect(_powerRect, 0);  // FrameRect (vBLACK)

		// Vertical dividers between 3 columns
		const int pl = _powerRect.left;
		_gfx->drawLine(pl + _powerWidth, _powerRect.bottom - 1, pl + _powerWidth, _powerRect.top, 0);
		_gfx->drawLine(pl + _powerWidth * 2, _powerRect.bottom - 1, pl + _powerWidth * 2, _powerRect.top, 0);

		// Horizontal divider above symbol area
		_gfx->drawLine(pl, _powerRect.bottom - _powerWidth - 1, _powerRect.right - 1, _powerRect.bottom - _powerWidth - 1, 0);

		// Symbol 1: triangle (weapon power)
		_gfx->drawLine(pl + 2, _powerRect.bottom - 2, pl + _powerWidth - 2, _powerRect.bottom - 2, 0);
		_gfx->drawLine(pl + _powerWidth - 2, _powerRect.bottom - 2, pl + (_powerWidth >> 1), _powerRect.bottom - (_powerWidth - 2), 0);
		_gfx->drawLine(pl + (_powerWidth >> 1), _powerRect.bottom - (_powerWidth - 2), pl + 2, _powerRect.bottom - 2, 0);

		// Symbol 2: diamond (life power)
		const int d2l = pl + _powerWidth;
		_gfx->drawLine(d2l + 1, _powerRect.bottom - (_powerWidth >> 1), d2l + (_powerWidth >> 1), _powerRect.bottom - (_powerWidth - 1), 0);
		_gfx->drawLine(d2l + (_powerWidth >> 1), _powerRect.bottom - (_powerWidth - 1), d2l + _powerWidth - 1, _powerRect.bottom - (_powerWidth >> 1), 0);
		_gfx->drawLine(d2l + _powerWidth - 1, _powerRect.bottom - (_powerWidth >> 1), d2l + (_powerWidth >> 1), _powerRect.bottom - 1, 0);
		_gfx->drawLine(d2l + (_powerWidth >> 1), _powerRect.bottom - 1, d2l + 1, _powerRect.bottom - (_powerWidth >> 1), 0);

		// Symbol 3: inverted triangle (shield power)
		const int d3l = pl + 2 * _powerWidth;
		_gfx->drawLine(d3l + 2, _powerRect.bottom - (_powerWidth - 2), d3l + _powerWidth - 2, _powerRect.bottom - (_powerWidth - 2), 0);
		_gfx->drawLine(d3l + _powerWidth - 2, _powerRect.bottom - (_powerWidth - 2), d3l + (_powerWidth >> 1), _powerRect.bottom - 2, 0);
		_gfx->drawLine(d3l + (_powerWidth >> 1), _powerRect.bottom - 2, d3l + 2, _powerRect.bottom - (_powerWidth - 2), 0);

		// Draw power bar graphs
		drawDOSBarGraph(pl, _powerRect.bottom - (_powerWidth + 1), _epower[0]);
		drawDOSBarGraph(pl + _powerWidth, _powerRect.bottom - (_powerWidth + 1), _epower[1]);
		drawDOSBarGraph(pl + _powerWidth * 2, _powerRect.bottom - (_powerWidth + 1), _epower[2]);
	}
}

// --- Mac Color path ---
// Uses actual PICT resources from the Mac resource fork as panel backgrounds,
// matching the original power.c DrawInfo() and compass.c DrawCompass() exactly.
// Original Mac had two floating windows (infoWindow + moveWindow) over gray desktop.

void ColonyEngine::drawDashboardMac() {
	const bool macColor = _hasMacColors;
	const uint32 colBlack = packRGB(0, 0, 0);
	const uint32 colWhite = packRGB(255, 255, 255);
	const uint32 colWinBg = macColor ? packMacColorUI(_macColors[7].bg) : colWhite;
	// power.c: ForeColor(blueColor)  on 1-bit display, blue maps to black
	const uint32 colBlue = macColor ? packRGB(0, 0, 255) : colBlack;

	// Dashboard background  Mac desktop dither pattern (classic 50% gray checkerboard).
	// Original Mac desktop: alternating black/white pixels between floating windows.
	_gfx->fillDitherRect(_dashBoardRect, colBlack, colWhite);

	// Viewport separator
	if (_screenR.left > 0)
		_gfx->drawLine(_screenR.left - 1, _screenR.top, _screenR.left - 1, _screenR.bottom - 1, colBlack);

	// ===== Power panel (infoWindow) =====
	// power.c DrawInfo(): FillRect(&clr,white) → DrawPicture(PICT) → ForeColor(blue) → bars
	// armor && !trouble → PICT -32755; armor && trouble → PICT -32760;
	// !armor && depth>=8 → PICT -32761; !armor && depth<8 → PICT -32752
	if (_powerRect.width() > 4 && _powerRect.height() > 4) {
		// power.c: FillRect(&clr, white)  white background under PICT
		_gfx->fillRect(_powerRect, colWhite);

		// Select correct PICT based on armor/trouble state
		// power.c: !armor && FindDepth()>=8 → -32761 (color); FindDepth()<8 → -32752 (B&W)
		auto qlog = [](int32 x) -> int {
			int i = 0;
			while (x > 0) {
				x >>= 1;
				i++;
			}
			return i;
		};
		// power.c DrawInfo(): epower[i] = qlog(Me.power[i]) — player suit power, NOT corepower.
		const int ePower[3] = { qlog(_me.power[0]), qlog(_me.power[1]), qlog(_me.power[2]) };
		const bool trouble = (ePower[1] < 6);
		int wantPictID;
		if (_armor > 0)
			wantPictID = trouble ? -32760 : -32755;
		else
			wantPictID = macColor ? -32761 : -32752;

		// Reload PICT if state changed (fall back to -32755 only when armored)
		if (_pictPowerID != wantPictID) {
			if (_pictPower) {
				_pictPower->free();
				delete _pictPower;
				_pictPower = nullptr;
			}
			_pictPower = loadPictSurface(wantPictID);
			if (!_pictPower && _armor > 0 && wantPictID != -32755)
				_pictPower = loadPictSurface(-32755);
			_pictPowerID = wantPictID;
		}

		// power.c: SetRect(&info, -2, -2, xSize-2, ySize-2); DrawPicture(inf, &info)
		// In the original B&W game, GetPicture(-32752) returns null when !armor,
		// so DrawPicture is a no-op — the window just shows white fill.
		if (_pictPower)
			drawPictAt(_pictPower, _powerRect.left - 2, _powerRect.top - 2);

		// Blue bars only when armored (power.c: if(armor) { ... ForeColor(blueColor) ... })
		if (_armor > 0 && _pictPower) {
			// power.c info rect adjustments: left+=3,right+=3,top-=3,bottom-=3, then ++/--
			// Net effect: info is adjusted relative to PICT position
			const int infoLeft = _powerRect.left - 2 + 2;  // -2 (PICT offset) +3-1 = 0, +2 net
			const int infoBottom = _powerRect.top - 2 + _pictPower->h - 2; // PICT bottom adjusted
			const int bot = infoBottom - 27; // power.c: bot = info.bottom - 27

			for (int i = 0; i < 3; i++) {
				// power.c: lft = 3 + info.left + i*23
				const int lft = 3 + infoLeft + i * 23;
				for (int j = 0; j < ePower[i] && j < 20; j++) {
					const int ln = bot - 3 * j;
					if (ln <= _powerRect.top)
						break;
					// power.c: MoveTo(lft+1,ln); LineTo(lft+16,ln);  16px wide, 2 lines
					_gfx->drawLine(lft + 1, ln, lft + 16, ln, colBlue);
					if (ln - 1 > _powerRect.top)
						_gfx->drawLine(lft + 1, ln - 1, lft + 16, ln - 1, colBlue);
				}
			}
		}
	}

	// ===== Compass + Floor map (moveWindow) =====
	// compass.c DrawCompass(): All coordinates relative to moveWindow origin.
	// compRect = (0,0, 2*CCENTER, 3*CCENTER) = (0,0, 70, 105)
	// CCENTER = 35
	if (_compassRect.width() > 4 && _compassRect.height() > 4) {
		const int ox = _compassRect.left; // moveWindow origin X
		const int oy = _compassRect.top;  // moveWindow origin Y

		// compass.c: SetRect(&compRect, -2, -2, xSize-2, ySize-2); DrawPicture(comp, &compRect)
		if (_pictCompass)
			drawPictAt(_pictCompass, ox - 2, oy - 2);
		else
			_gfx->fillRect(_compassRect, colWinBg);

		// Floor map (compass.c lines 72-213):
		// floorRect = (8,8)-(62,62)  clipped to _headsUpRect
		// Eye icon at center (CCENTER,CCENTER) = (35,35)
		drawMiniMap(colBlack);

		// Eye icon (compass.c lines 84-88)
		// FrameOval (CCENTER-10, CCENTER-5)-(CCENTER+10, CCENTER+5) = (25,30)-(45,40)
		// FillOval (CCENTER-5, CCENTER-5)-(CCENTER+5, CCENTER+5) = (30,30)-(40,40)
		_gfx->drawEllipse(ox + 35, oy + 35, 10, 5, colBlack);
		_gfx->fillEllipse(ox + 35, oy + 35, 5, 5, colBlack);

		// Compass dish (compass.c lines 59-70):
		// FillOval (CCENTER-16, 2*CCENTER-4)-(CCENTER+16, 2*CCENTER+28) = (19,66)-(51,98)
		// Needle center at (CCENTER, 2*CCENTER+12) = (35, 82)
		const int dishCX = ox + 35;
		const int dishCY = oy + 82;
		_gfx->fillEllipse(dishCX, dishCY, 16, 16, colBlack);

		// Needle: LineTo(CCENTER+(cost[ang]>>3), (2*CCENTER+12)-(sint[ang]>>3))
		const int ex = dishCX + (_cost[_me.look] >> 3);
		const int ey = dishCY - (_sint[_me.look] >> 3);
		_gfx->drawLine(dishCX, dishCY, ex, ey, colWhite);
	}
}

void ColonyEngine::drawMiniMapMarker(int x, int y, int halfSize, uint32 color, bool isMac, const Common::Rect *clip) {
	const Common::Rect &cr = clip ? *clip : _headsUpRect;
	if (x < cr.left + 1 || x >= cr.right - 1 ||
	    y < cr.top + 1 || y >= cr.bottom - 1)
		return;
	if (isMac) {
		_gfx->drawEllipse(x, y, halfSize, halfSize, color);
	} else {
		const int l = MAX<int>(cr.left + 1, x - halfSize);
		const int t = MAX<int>(cr.top + 1, y - halfSize);
		const int r = MIN<int>(cr.right - 1, x + halfSize + 1);
		const int b = MIN<int>(cr.bottom - 1, y + halfSize + 1);
		if (l >= r || t >= b)
			return;
		_gfx->drawRect(Common::Rect(l, t, r, b), color);
	}
}

bool ColonyEngine::hasRobotAt(int x, int y) const {
	if (x < 0 || x >= 32 || y < 0 || y >= 32)
		return false;
	return _robotArray[x][y] != 0;
}

bool ColonyEngine::hasFoodAt(int x, int y) const {
	const int kFWALLType = 48;
	if (x < 0 || x >= 32 || y < 0 || y >= 32)
		return false;
	const uint8 num = _foodArray[x][y];
	if (num == 0)
		return false;
	if (num <= _objects.size())
		return _objects[num - 1].type < kFWALLType;
	return true;
}

// Draws the mini floor map into _headsUpRect (shared by Mac and DOS paths)
void ColonyEngine::drawMiniMap(uint32 lineColor) {
	if (_gameMode != kModeColony)
		return;
	if (_me.xindex < 0 || _me.xindex >= 32 || _me.yindex < 0 || _me.yindex >= 32)
		return;

	const Common::Rect miniMapClip(_headsUpRect.left + 1, _headsUpRect.top + 1, _headsUpRect.right - 1, _headsUpRect.bottom - 1);
	auto drawMiniMapLine = [&](int x1, int y1, int x2, int y2, uint32 color) {
		if (clipLineToRect(x1, y1, x2, y2, miniMapClip))
			_gfx->drawLine(x1, y1, x2, y2, color);
	};

	const bool isMac = (_renderMode == Common::kRenderMacintosh);

	int lExt, sExt, xloc, yloc, ccenterx, ccentery;
	if (isMac) {
		// compass.c: CSIZE=64, CCENTER=35
		// xloc = ((Me.xindex << 8) - Me.xloc) >> 2
		// Center at (CCENTER, CCENTER) relative to moveWindow
		lExt = 64; // CSIZE
		sExt = 32; // CSIZE/2
		xloc = ((_me.xindex << 8) - _me.xloc) >> 2;
		yloc = ((_me.yindex << 8) - _me.yloc) >> 2;
		ccenterx = _compassRect.left + 35; // CCENTER in screen coords
		ccentery = _compassRect.top + 35;
	} else {
		const int lExtBase = _dashBoardRect.width() >> 1;
		lExt = lExtBase + (lExtBase >> 1);
		if (lExt & 1)
			lExt--;
		sExt = lExt >> 1;
		xloc = (lExt * ((_me.xindex << 8) - _me.xloc)) >> 8;
		yloc = (lExt * ((_me.yindex << 8) - _me.yloc)) >> 8;
		ccenterx = (_headsUpRect.left + _headsUpRect.right) >> 1;
		ccentery = (_headsUpRect.top + _headsUpRect.bottom) >> 1;
	}
	const int tsin = _sint[_me.look];
	const int tcos = _cost[_me.look];

	int xcorner[6];
	int ycorner[6];
	xcorner[0] = ccenterx + (((long)xloc * tsin - (long)yloc * tcos) >> 8);
	ycorner[0] = ccentery - (((long)yloc * tsin + (long)xloc * tcos) >> 8);
	xcorner[1] = ccenterx + (((long)(xloc + lExt) * tsin - (long)yloc * tcos) >> 8);
	ycorner[1] = ccentery - (((long)yloc * tsin + (long)(xloc + lExt) * tcos) >> 8);
	xcorner[2] = ccenterx + (((long)(xloc + lExt) * tsin - (long)(yloc + lExt) * tcos) >> 8);
	ycorner[2] = ccentery - (((long)(yloc + lExt) * tsin + (long)(xloc + lExt) * tcos) >> 8);
	xcorner[3] = ccenterx + (((long)xloc * tsin - (long)(yloc + lExt) * tcos) >> 8);
	ycorner[3] = ccentery - (((long)(yloc + lExt) * tsin + (long)xloc * tcos) >> 8);
	xcorner[4] = ccenterx + (((long)(xloc + sExt) * tsin - (long)(yloc + sExt) * tcos) >> 8);
	ycorner[4] = ccentery - (((long)(yloc + sExt) * tsin + (long)(xloc + sExt) * tcos) >> 8);
	xcorner[5] = ccenterx + (((long)(xloc + sExt) * tsin - (long)yloc * tcos) >> 8);
	ycorner[5] = ccentery - (((long)yloc * tsin + (long)(xloc + sExt) * tcos) >> 8);

	const int dx = xcorner[1] - xcorner[0];
	const int dy = ycorner[0] - ycorner[1];
	drawMiniMapLine(xcorner[0] - dx, ycorner[0] + dy, xcorner[1] + dx, ycorner[1] - dy, lineColor);
	drawMiniMapLine(xcorner[1] + dy, ycorner[1] + dx, xcorner[2] - dy, ycorner[2] - dx, lineColor);
	drawMiniMapLine(xcorner[2] + dx, ycorner[2] - dy, xcorner[3] - dx, ycorner[3] + dy, lineColor);
	drawMiniMapLine(xcorner[3] - dy, ycorner[3] - dx, xcorner[0] + dy, ycorner[0] + dx, lineColor);

	// compass.c: food markers use FrameOval ±3px, robot markers ±5px.
	const int foodR = isMac ? 3 : 1;
	const int robotR = isMac ? 5 : 2;

	if (hasFoodAt(_me.xindex, _me.yindex))
		drawMiniMapMarker(xcorner[4], ycorner[4], foodR, lineColor, isMac);

	if (_me.yindex > 0 && !(_wall[_me.xindex][_me.yindex] & 0x01)) {
		if (hasFoodAt(_me.xindex, _me.yindex - 1))
			drawMiniMapMarker(xcorner[4] + dy, ycorner[4] + dx, foodR, lineColor, isMac);
		if (hasRobotAt(_me.xindex, _me.yindex - 1))
			drawMiniMapMarker(xcorner[4] + dy, ycorner[4] + dx, robotR, lineColor, isMac);
	}
	if (_me.xindex > 0 && !(_wall[_me.xindex][_me.yindex] & 0x02)) {
		if (hasFoodAt(_me.xindex - 1, _me.yindex))
			drawMiniMapMarker(xcorner[4] - dx, ycorner[4] + dy, foodR, lineColor, isMac);
		if (hasRobotAt(_me.xindex - 1, _me.yindex))
			drawMiniMapMarker(xcorner[4] - dx, ycorner[4] + dy, robotR, lineColor, isMac);
	}
	if (_me.yindex < 30 && !(_wall[_me.xindex][_me.yindex + 1] & 0x01)) {
		if (hasFoodAt(_me.xindex, _me.yindex + 1))
			drawMiniMapMarker(xcorner[4] - dy, ycorner[4] - dx, foodR, lineColor, isMac);
		if (hasRobotAt(_me.xindex, _me.yindex + 1))
			drawMiniMapMarker(xcorner[4] - dy, ycorner[4] - dx, robotR, lineColor, isMac);
	}
	if (_me.xindex < 30 && !(_wall[_me.xindex + 1][_me.yindex] & 0x02)) {
		if (hasFoodAt(_me.xindex + 1, _me.yindex))
			drawMiniMapMarker(xcorner[4] + dx, ycorner[4] - dy, foodR, lineColor, isMac);
		if (hasRobotAt(_me.xindex + 1, _me.yindex))
			drawMiniMapMarker(xcorner[4] + dx, ycorner[4] - dy, robotR, lineColor, isMac);
	}

	// DASHBOAR.C DrawHeadsUp(): player eye icon at minimap center
	// Outer oval: FrameOval ±(pQx/2, pQy/4) = ±(12, 4) for EGA
	// Inner oval (pupil): FillOval ±(pQx/4, pQy/4) = ±(6, 4) for EGA
	if (!isMac) {
		const int px = _pQx >> 1;  // 12
		const int py = _pQy >> 2;  // 4
		_gfx->drawEllipse(ccenterx, ccentery, px, py, lineColor);
		const int px2 = _pQx >> 2; // 6
		_gfx->fillEllipse(ccenterx, ccentery, px2, py, lineColor);
	}
}

void ColonyEngine::markVisited() {
	if (_level < 1 || _level > 8)
		return;
	const int lv = _level - 1;
	const int cx = _me.xindex;
	const int cy = _me.yindex;
	if (cx < 0 || cx >= 32 || cy < 0 || cy >= 32)
		return;

	// Always mark the player's own cell
	_visited[lv][cx][cy] = true;

	// Check cardinal neighbors: blocked by wall bit OR door/airlock feature
	bool canN = false, canS = false, canE = false, canW = false;

	// North: wall at south edge of cell (cx, cy+1)
	if (cy + 1 < 32) {
		canN = !(_wall[cx][cy + 1] & 0x01);
		if (canN && cx < 31 && cy < 31 &&
		    (_mapData[cx][cy][kDirNorth][0] == kWallFeatureDoor || _mapData[cx][cy][kDirNorth][0] == kWallFeatureAirlock))
			canN = false;
	}
	// South: wall at south edge of cell (cx, cy)
	if (cy - 1 >= 0) {
		canS = !(_wall[cx][cy] & 0x01);
		if (canS && cx < 31 && cy < 31 &&
		    (_mapData[cx][cy][kDirSouth][0] == kWallFeatureDoor || _mapData[cx][cy][kDirSouth][0] == kWallFeatureAirlock))
			canS = false;
	}
	// East: wall at west edge of cell (cx+1, cy)
	if (cx + 1 < 32) {
		canE = !(_wall[cx + 1][cy] & 0x02);
		if (canE && cx < 31 && cy < 31 &&
		    (_mapData[cx][cy][kDirEast][0] == kWallFeatureDoor || _mapData[cx][cy][kDirEast][0] == kWallFeatureAirlock))
			canE = false;
	}
	// West: wall at west edge of cell (cx, cy)
	if (cx - 1 >= 0) {
		canW = !(_wall[cx][cy] & 0x02);
		if (canW && cx < 31 && cy < 31 &&
		    (_mapData[cx][cy][kDirWest][0] == kWallFeatureDoor || _mapData[cx][cy][kDirWest][0] == kWallFeatureAirlock))
			canW = false;
	}

	if (canN) _visited[lv][cx][cy + 1] = true;
	if (canS) _visited[lv][cx][cy - 1] = true;
	if (canE) _visited[lv][cx + 1][cy] = true;
	if (canW) _visited[lv][cx - 1][cy] = true;

	// Diagonal neighbors: visible only if both adjacent cardinal directions are open
	if (canN && canE && cx + 1 < 32 && cy + 1 < 32) _visited[lv][cx + 1][cy + 1] = true;
	if (canN && canW && cx - 1 >= 0 && cy + 1 < 32) _visited[lv][cx - 1][cy + 1] = true;
	if (canS && canE && cx + 1 < 32 && cy - 1 >= 0) _visited[lv][cx + 1][cy - 1] = true;
	if (canS && canW && cx - 1 >= 0 && cy - 1 >= 0) _visited[lv][cx - 1][cy - 1] = true;
}

static bool isPassableFeature(int feat) {
	return feat == kWallFeatureDoor || feat == kWallFeatureAirlock ||
	       feat == kWallFeatureUpStairs || feat == kWallFeatureDnStairs ||
	       feat == kWallFeatureTunnel || feat == kWallFeatureElevator;
}

void ColonyEngine::automapCellCorner(int dx, int dy, int xloc, int yloc, int lExt, int tsin, int tcos, int ccx, int ccy, int &sx, int &sy) {
	const long ox = xloc + (long)dx * lExt;
	const long oy = yloc + (long)dy * lExt;
	sx = ccx + (int)((ox * tsin - oy * tcos) >> 8);
	sy = ccy - (int)((oy * tsin + ox * tcos) >> 8);
}

void ColonyEngine::automapDrawWall(const Common::Rect &vp, int x1, int y1, int x2, int y2, uint32 color) {
	if (clipLineToRect(x1, y1, x2, y2, vp)) {
		_gfx->drawLine(x1, y1, x2, y2, color);
		// Thicker wall: offset by 1 pixel perpendicular
		int ox = y2 - y1, oy = x1 - x2;
		int len = (int)sqrtf((float)(ox * ox + oy * oy));
		if (len > 0) {
			ox = ox / len;
			oy = oy / len;
			int ax1 = x1 + ox, ay1 = y1 + oy, ax2 = x2 + ox, ay2 = y2 + oy;
			if (clipLineToRect(ax1, ay1, ax2, ay2, vp))
				_gfx->drawLine(ax1, ay1, ax2, ay2, color);
		}
	}
}

int ColonyEngine::automapWallFeature(int fx, int fy, int dir) {
	if (fx >= 0 && fx < 31 && fy >= 0 && fy < 31) {
		int feat = _mapData[fx][fy][dir][0];
		if (isPassableFeature(feat))
			return feat;
	}
	int nx = fx, ny = fy, opp = -1;
	switch (dir) {
	case kDirNorth: ny = fy + 1; opp = kDirSouth; break;
	case kDirSouth: ny = fy - 1; opp = kDirNorth; break;
	case kDirEast:  nx = fx + 1; opp = kDirWest;  break;
	case kDirWest:  nx = fx - 1; opp = kDirEast;  break;
	default: return 0;
	}
	if (nx >= 0 && nx < 31 && ny >= 0 && ny < 31) {
		int feat = _mapData[nx][ny][opp][0];
		if (isPassableFeature(feat))
			return feat;
	}
	return 0;
}

void ColonyEngine::automapDrawWallWithFeature(const Common::Rect &vp, int wx1, int wy1, int wx2, int wy2, int feat, int lExt, uint32 color) {
	automapDrawWall(vp, wx1, wy1, wx2, wy2, color);

	if (isPassableFeature(feat)) {
		const int ppx = (wy2 - wy1);
		const int ppy = (wx1 - wx2);
		const int tickLen = MAX(2, lExt / 8);
		const int plen = (int)sqrtf((float)(ppx * ppx + ppy * ppy));
		if (plen > 0) {
			const int tx = (ppx * tickLen) / plen;
			const int ty = (ppy * tickLen) / plen;
			const int ax = wx1 + (wx2 - wx1) / 4;
			const int ay = wy1 + (wy2 - wy1) / 4;
			const int bx = wx1 + 3 * (wx2 - wx1) / 4;
			const int by = wy1 + 3 * (wy2 - wy1) / 4;
			automapDrawWall(vp, ax - tx, ay - ty, ax + tx, ay + ty, color);
			automapDrawWall(vp, bx - tx, by - ty, bx + tx, by + ty, color);
		}
	}
}

void ColonyEngine::drawAutomap() {
	if (_level < 1 || _level > 8)
		return;

	const int lv = _level - 1;
	const bool isMac = (_renderMode == Common::kRenderMacintosh);

	const Common::Rect vp(0, _menuBarHeight, _width, _height);
	const int vpW = vp.width();
	const int vpH = vp.height();
	if (vpW <= 0 || vpH <= 0)
		return;

	_gfx->fillRect(vp, isMac ? 0xFFA0D0FF : 15);
	_gfx->drawRect(vp, 0);

	const int lExt = MIN(vpW, vpH) / 12;
	if (lExt < 8)
		return;

	const int xloc = (lExt * ((_me.xindex << 8) - _me.xloc)) >> 8;
	const int yloc = (lExt * ((_me.yindex << 8) - _me.yloc)) >> 8;
	const int ccx = (vp.left + vp.right) >> 1;
	const int ccy = (vp.top + vp.bottom) >> 1;
	const int tsin = _sint[_me.look];
	const int tcos = _cost[_me.look];
	const uint32 lineColor = 0;

	const int radius = (int)(sqrtf((float)(vpW * vpW + vpH * vpH)) / (2.0f * lExt)) + 2;
	const int px = _me.xindex;
	const int py = _me.yindex;
	const int markerR = isMac ? 5 : 3;
	const int foodR = isMac ? 3 : 2;

	for (int dy = -radius; dy <= radius; dy++) {
		for (int dx = -radius; dx <= radius; dx++) {
			const int cx = px + dx;
			const int cy = py + dy;
			if (cx < 0 || cx >= 32 || cy < 0 || cy >= 32)
				continue;
			if (!_visited[lv][cx][cy])
				continue;

			int x0, y0, x1, y1, x2, y2, x3, y3;
			automapCellCorner(dx, dy, xloc, yloc, lExt, tsin, tcos, ccx, ccy, x0, y0);
			automapCellCorner(dx + 1, dy, xloc, yloc, lExt, tsin, tcos, ccx, ccy, x1, y1);
			automapCellCorner(dx + 1, dy + 1, xloc, yloc, lExt, tsin, tcos, ccx, ccy, x2, y2);
			automapCellCorner(dx, dy + 1, xloc, yloc, lExt, tsin, tcos, ccx, ccy, x3, y3);

			if (_wall[cx][cy] & 0x01)
				automapDrawWallWithFeature(vp, x0, y0, x1, y1, automapWallFeature(cx, cy, kDirSouth), lExt, lineColor);
			if (_wall[cx][cy] & 0x02)
				automapDrawWallWithFeature(vp, x0, y0, x3, y3, automapWallFeature(cx, cy, kDirWest), lExt, lineColor);
			if (cy + 1 < 32 && (_wall[cx][cy + 1] & 0x01))
				automapDrawWallWithFeature(vp, x3, y3, x2, y2, automapWallFeature(cx, cy, kDirNorth), lExt, lineColor);
			if (cx + 1 < 32 && (_wall[cx + 1][cy] & 0x02))
				automapDrawWallWithFeature(vp, x1, y1, x2, y2, automapWallFeature(cx, cy, kDirEast), lExt, lineColor);

			if (ABS(dx) <= 6 && ABS(dy) <= 6) {
				int mx, my;
				automapCellCorner(dx, dy, xloc, yloc, lExt, tsin, tcos, ccx, ccy, mx, my);
				int mx2, my2;
				automapCellCorner(dx + 1, dy + 1, xloc, yloc, lExt, tsin, tcos, ccx, ccy, mx2, my2);
				mx = (mx + mx2) >> 1;
				my = (my + my2) >> 1;

				const uint8 rnum = _robotArray[cx][cy];
				if (rnum > 0 && rnum != kMeNum && rnum <= _objects.size() && _objects[rnum - 1].alive)
					drawMiniMapMarker(mx, my, markerR, lineColor, isMac, &vp);
				if (_foodArray[cx][cy] > 0)
					drawMiniMapMarker(mx, my, foodR, lineColor, isMac, &vp);
			}
		}
	}

	// Player eye icon at center
	const int eyeRx = lExt >> 2;
	const int eyeRy = lExt >> 3;
	_gfx->drawEllipse(ccx, ccy, eyeRx, eyeRy, lineColor);
	_gfx->fillEllipse(ccx, ccy, eyeRx >> 1, eyeRy, lineColor);
}
void ColonyEngine::drawForkliftOverlay() {
	if (_fl <= 0 || _screenR.width() <= 0 || _screenR.height() <= 0)
		return;

	// Original display.c: two diagonal fork arm lines when fl > 0.
	// Left arm:  (centerX/4, 0) to (centerX/2, Height)
	// Right arm: (Width - centerX/4, 0) to (Width - centerX/2, Height)
	// Drawn with PenSize(2,2) in black (white in battle mode).
	const int left = _screenR.left;
	const int top = _screenR.top;
	const int w = _screenR.width();
	const int h = _screenR.height();
	const int cx = w / 2;
	const uint32 color = (_renderMode == Common::kRenderMacintosh) ? packRGB(0, 0, 0) : 0;

	const int tx2 = cx >> 2;  // centerX/4
	const int tx1 = cx >> 1;  // centerX/2

	// Left fork arm
	_gfx->drawLine(left + tx2, top, left + tx1, top + h - 1, color);
	_gfx->drawLine(left + tx2 + 1, top, left + tx1 + 1, top + h - 1, color);
	// Right fork arm
	_gfx->drawLine(left + w - tx2, top, left + w - tx1, top + h - 1, color);
	_gfx->drawLine(left + w - tx2 - 1, top, left + w - tx1 - 1, top + h - 1, color);
}

void ColonyEngine::drawCrosshair() {
	if (!_crosshair || _screenR.width() <= 0 || _screenR.height() <= 0)
		return;

	const bool isMac = (_renderMode == Common::kRenderMacintosh);
	if (isMac && _cursorShoot && !_mouseLocked && _weapons > 0)
		return;

	uint32 color;
	if (isMac) {
		// Mac: black when powered, gray when no weapons, white when armed but no power
		// B&W: no gray, so powered=black, else white
		if (_hasMacColors)
			color = (_corePower[_coreIndex] > 0) ? packRGB(0, 0, 0)
				: (_weapons > 0) ? packRGB(255, 255, 255) : packRGB(128, 128, 128);
		else
			color = (_corePower[_coreIndex] > 0) ? packRGB(0, 0, 0) : packRGB(255, 255, 255);
	} else {
		// IBM_DISP.C: powered → color_cursor=realcolor[vBLACK]=0
		//             unpowered → color_cursor=realcolor[vWHITE]=15
		color = (_corePower[_coreIndex] > 0) ? 0 : 15;
	}

	const int cx = _centerX;
	const int cy = _centerY;

	// Mac display.c: fixed ±20/±30 pixel crosshair.
	// DOS IBM_DISP.C: uses pix_per_Qinch (24/18) and pix_per_Finch (36/27).
	const int qx = isMac ? 20 : _pQx;
	const int qy = isMac ? 20 : _pQy;
	const int fx = isMac ? 30 : ((_pQx * 3) >> 1);
	const int fy = isMac ? 30 : ((_pQy * 3) >> 1);
	auto drawCrossLine = [&](int x1, int y1, int x2, int y2) {
		if (clipLineToRect(x1, y1, x2, y2, _screenR))
			_gfx->drawLine(x1, y1, x2, y2, color);
	};

	if (_weapons > 0) {
		// Original IBM_DISP.C: two bracket shapes (left + right)
		// insight: inner edge at pQx, outer edge at pFinch_x, height pQy..pFinch_y
		// normal: inner edge at pQx, outer edge at pFinch_x, height pFinch_y
		const int yTop = _insight ? (cy - qy) : (cy - fy);
		const int yBottom = _insight ? (cy + qy) : (cy + fy);

		// Left bracket: top-left corner down to bottom-left
		drawCrossLine(cx - qx, yTop, cx - fx, yTop);
		drawCrossLine(cx - fx, yTop, cx - fx, yBottom);
		drawCrossLine(cx - fx, yBottom, cx - qx, yBottom);
		// Right bracket: top-right corner down to bottom-right
		drawCrossLine(cx + qx, yTop, cx + fx, yTop);
		drawCrossLine(cx + fx, yTop, cx + fx, yBottom);
		drawCrossLine(cx + fx, yBottom, cx + qx, yBottom);
		_insight = false;
	} else {
		// Original IBM_DISP.C: simple cross ±pix_per_Qinch from center
		drawCrossLine(cx - qx, cy, cx + qx, cy);
		drawCrossLine(cx, cy - qy, cx, cy + qy);
	}
}

void ColonyEngine::inform(const char *text, bool hold) {
	const char *msg[3];
	msg[0] = text;
	msg[1] = hold ? "-Press Any Key to Continue-" : nullptr;
	msg[2] = nullptr;
	printMessage(msg, hold);
}

void ColonyEngine::printMessage(const char *text[], bool hold) {
	int numLines = 0;
	int width = 0;
	Graphics::DosFont font;
	Common::Array<Common::String> lines;

	while (text[numLines] != nullptr) {
		lines.push_back(text[numLines]);
		int w = font.getStringWidth(text[numLines]);
		if (w > width)
			width = w;
		numLines++;
	}

	if (_renderMode == Common::kRenderMacintosh && drawMacTextPopup(_wm, _gfx,
			_width, _height, _centerX, _centerY, lines, Graphics::kTextAlignCenter, _hasMacColors)) {
		if (hold)
			waitForInput();
		return;
	}

	int pxPerInchX = 72;
	int pxPerInchY = 72;

	Common::Rect rr;
	rr.top = _centerY - (numLines + 1) * (pxPerInchY / 4);
	rr.bottom = _centerY + (numLines + 1) * (pxPerInchY / 4);
	rr.left = _centerX - width / 2 - (pxPerInchX / 2);
	rr.right = _centerX + width / 2 + (pxPerInchX / 2);

	_gfx->fillDitherRect(_screenR, 0, 15);
	makeMessageRect(rr);

	int start;
	int step;
	if (numLines > 1) {
		start = rr.top + (pxPerInchY / 4) * 2;
		step = (rr.height() - (pxPerInchY / 4) * 4) / (numLines - 1);
	} else {
		start = (rr.top + rr.bottom) / 2;
		step = 0;
	}

	for (int i = 0; i < numLines; i++) {
		_gfx->drawString(&font, text[i], (rr.left + rr.right) / 2, start + i * step, 0, Graphics::kTextAlignCenter);
	}

	_gfx->copyToScreen();

	if (hold)
		waitForInput();
}

void ColonyEngine::makeMessageRect(Common::Rect &rr) {
	_gfx->fillRect(rr, 15);
	_gfx->drawRect(rr, 0);
	Common::Rect inner = rr;
	inner.grow(-2);
	_gfx->drawRect(inner, 0);
}

void ColonyEngine::doText(int entry, int center) {
	Common::SeekableReadStream *file = Common::MacResManager::openFileOrDataFork(Common::Path("T.DAT"));
	if (!file) {
		file = Common::MacResManager::openFileOrDataFork(Common::Path("CData/Tdata"));
		if (!file) {
			warning("doText: Could not open text file");
			return;
		}
	}

	uint32 entries = file->readUint32BE();
	if (entry < 0 || (uint32)entry >= entries) {
		warning("doText: Entry %d out of range (max %d)", entry, entries);
		delete file;
		return;
	}

	file->seek(4 + entry * 8);
	uint32 offset = file->readUint32BE();
	uint16 ch = file->readUint16BE();
	file->readUint16BE(); // lines (unused)

	if (ch == 0) {
		delete file;
		return;
	}

	byte *page = new byte[ch + 1];
	file->seek(offset);
	file->read(page, ch);
	delete file;
	page[ch] = 0;

	// Decode: Chain XOR starting from end with '\'
	page[ch - 1] ^= '\\';
	for (int n = ch - 2; n >= 0; n--)
		page[n] ^= page[n + 1];

	Common::Array<Common::String> lineArray;
	char *p = (char *)page;
	int start = 0;
	for (int i = 0; i < ch; i++) {
		if (p[i] == '\r' || p[i] == '\n') {
			p[i] = 0;
			if (p[start])
				lineArray.push_back(&p[start]);
			start = i + 1;
		}
	}
	if (start < ch && p[start])
		lineArray.push_back(&p[start]);

	Graphics::DosFont font;
	int width = 0;
	for (uint i = 0; i < lineArray.size(); i++) {
		int w = font.getStringWidth(lineArray[i]);
		if (w > width)
			width = w;
	}
	const char *kpress = "-Press Any Key to Continue-";
	const char *kmore = "-More-";
	int kw = font.getStringWidth(kpress);
	if (kw > width)
		width = kw;
	width += 12;

	int lineheight = 14;
	int maxlines = (_screenR.height() / lineheight) - 2;
	if (maxlines > (int)lineArray.size())
		maxlines = lineArray.size();

	if (_renderMode == Common::kRenderMacintosh) {
		Common::Array<Common::String> popupLines;
		for (int i = 0; i < maxlines; ++i)
			popupLines.push_back(lineArray[i]);
		popupLines.push_back((int)lineArray.size() > maxlines ? kmore : kpress);
		if (drawMacTextPopup(_wm, _gfx, _width, _height, _centerX, _centerY, popupLines,
				center == 1 ? Graphics::kTextAlignCenter : Graphics::kTextAlignLeft, _hasMacColors)) {
			waitForInput();
			delete[] page;
			return;
		}
	}

	// DOS DOTEXT.C: r positioned at (cX ± wdth, cY ± ((maxlines+1)*7 + 4))
	// then offset by (+3,+3) for shadow. 3 nested FrameRects shrinking by 1.
	const int halfH = ((maxlines + 1) * (lineheight / 2)) + 4;
	Common::Rect r;
	r.left = _centerX - (width / 2) + 3;
	r.right = _centerX + (width / 2) + 3;
	r.top = _centerY - halfH + 3;
	r.bottom = _centerY + halfH + 3;

	// DoGray(): dither the viewport background
	_gfx->fillDitherRect(_screenR, 0, 15);

	// 3 nested FrameRects (shadow), then erase interior + final frame
	for (int i = 0; i < 3; i++) {
		_gfx->drawRect(r, 0);
		r.translate(-1, -1);
	}
	_gfx->fillRect(r, 15);
	_gfx->drawRect(r, 0);

	// Draw first page of text
	for (int i = 0; i < maxlines; i++) {
		_gfx->drawString(&font, lineArray[i], r.left + 3, r.top + 4 + i * lineheight, 0);
		if (center == 2) {
			_sound->play(Sound::kDit);
			_system->delayMillis(20);
		}
	}

	// Show "More" or "Press Any Key" prompt
	const bool hasMore = ((int)lineArray.size() > maxlines);
	_gfx->drawString(&font, hasMore ? "-Press Any Key For More...-" : kpress,
		(r.left + r.right) / 2, r.top + 6 + maxlines * lineheight, 0, Graphics::kTextAlignCenter);
	_gfx->copyToScreen();
	waitForInput();

	// Second page: if text was truncated, show remainder
	// DOS DOTEXT.C: starts from maxlines-1 (repeats last line of page 1 for context)
	if (hasMore) {
		_gfx->fillRect(r, 15);
		_gfx->drawRect(r, 0);
		int pageStart = maxlines - 1;
		for (int i = pageStart; i < (int)lineArray.size() && (i - pageStart) < maxlines; i++) {
			_gfx->drawString(&font, lineArray[i], r.left + 3,
				r.top + 6 + (1 + i - pageStart) * lineheight, 0);
			if (center == 2) {
				_sound->play(Sound::kDit);
				_system->delayMillis(20);
			}
		}
		_gfx->drawString(&font, kpress,
			(r.left + r.right) / 2, r.top + 6 + maxlines * lineheight, 0, Graphics::kTextAlignCenter);
		_gfx->copyToScreen();
		waitForInput();
	}

	delete[] page;
}

bool ColonyEngine::clipLineToRect(int &x1, int &y1, int &x2, int &y2, const Common::Rect &clip) const {
	if (clip.left >= clip.right || clip.top >= clip.bottom)
		return false;
	const int l = clip.left;
	const int r = clip.right - 1;
	const int t = clip.top;
	const int b = clip.bottom - 1;
	auto outCode = [&](int x, int y) {
		int code = 0;
		if (x < l)
			code |= 1;
		else if (x > r)
			code |= 2;
		if (y < t)
			code |= 4;
		else if (y > b)
			code |= 8;
		return code;
	};

	int c1 = outCode(x1, y1);
	int c2 = outCode(x2, y2);
	while (true) {
		if ((c1 | c2) == 0)
			return true;
		if (c1 & c2)
			return false;

		const int cOut = c1 ? c1 : c2;
		int x = 0;
		int y = 0;
		if (cOut & 8) {
			if (y2 == y1)
				return false;
			x = x1 + (x2 - x1) * (b - y1) / (y2 - y1);
			y = b;
		} else if (cOut & 4) {
			if (y2 == y1)
				return false;
			x = x1 + (x2 - x1) * (t - y1) / (y2 - y1);
			y = t;
		} else if (cOut & 2) {
			if (x2 == x1)
				return false;
			y = y1 + (y2 - y1) * (r - x1) / (x2 - x1);
			x = r;
		} else {
			if (x2 == x1)
				return false;
			y = y1 + (y2 - y1) * (l - x1) / (x2 - x1);
			x = l;
		}

		if (cOut == c1) {
			x1 = x;
			y1 = y;
			c1 = outCode(x1, y1);
		} else {
			x2 = x;
			y2 = y;
			c2 = outCode(x2, y2);
		}
	}
}

} // End of namespace Colony
