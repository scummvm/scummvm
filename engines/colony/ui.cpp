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

#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/cursorman.h"
#include "graphics/fontman.h"
#include "graphics/fonts/dosfont.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowborder.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/palette.h"
#include "image/pict.h"

#include "colony/colony.h"
#include "colony/renderer.h"
#include "colony/sound.h"

namespace Colony {

enum MacTextPopupStyle {
	kMacTextWindow,
	kMacInformWindow
};

Graphics::ManagedSurface *captureMessageBackground(Renderer *gfx, int width, int height) {
	if (!gfx)
		return nullptr;

	Graphics::Surface *screenshot = gfx->getScreenshot();
	if (!screenshot)
		return nullptr;

	Graphics::ManagedSurface *saved = new Graphics::ManagedSurface();
	saved->create(width, height, screenshot->format);
	saved->b
litFrom(*screenshot, Common::Rect(screenshot->w, screenshot->h), Common::Rect(width, height));
	screenshot->free();
	delete screenshot;
	return saved;
}

void restoreMessageBackground(Renderer *gfx, Graphics::ManagedSurface *saved) {
	if (!saved)
		return;
	if (gfx) {
		gfx->drawSurface(&saved->rawSurface(), 0, 0);
		gfx->copyToScreen();
	}
	saved->free();
	delete saved;
}

void animateMacZoom(Renderer *gfx, OSystem *system, const Common::Rect &from, const Common::Rect &to) {
	if (!gfx || !system || from.isEmpty() || to.isEmpty())
		return;

	const bool cursorWasVisible = CursorMan.isVisible();
	CursorMan.showMouse(false);
	gfx->setXorMode(true);
	const int steps = 8;
	for (int i = 0; i <= steps; ++i) {
		Common::Rect r;
		r.left = (from.left * (steps - i) + to.left * i) / steps;
		r.top = (from.top * (steps - i) + to.top * i) / steps;
		r.right = (from.right * (steps - i) + to.right * i) / steps;
		r.bottom = (from.bottom * (steps - i) + to.bottom * i) / steps;
		gfx->drawRect(r, 0xFFFFFFFF);
		gfx->copyToScreen();
		system->delayMillis(12);
		gfx->drawRect(r, 0xFFFFFFFF);
	}
	gfx->setXorMode(false);
	gfx->copyToScreen();
	CursorMan.showMouse(cursorWasVisible);
}

bool drawMacTextPopup(Graphics::MacWindowManager *wm, Renderer *gfx,
		int screenWidth, int screenHeight, int centerX, int centerY,
		const Common::Array<Common::String> &lines, Graphics::TextAlign align, bool macColor,
		int visibleLineCount = -1, Common::Rect *popupBounds = nullptr,
		bool measureOnly = false, MacTextPopupStyle style = kMacTextWindow) {
	if (!gfx || lines.empty())
		return false;
	uint visibleLines = lines.size();
	if (visibleLineCount >= 0 && (uint)visibleLineCount < visibleLines)
		visibleLines = visibleLineCount;

	Graphics::MacFont systemFont(Graphics::kMacFontSystem, 12);
	const Graphics::Font *font = (wm && wm->_fontMan) ? wm->_fontMan->getFont(systemFont) : nullptr;
	if (!font)
		font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!font)
		font = FontMan.getFontB
yUsage(Graphics::FontManager::kBigGUIFont);
	if (!font)
		return false;

	int textWidth = 0;
	for (uint i = 0; i < lines.size(); ++i)
		textWidth = MAX<int>(textWidth, font->getStringWidth(lines[i]));

	const int fontHeight = MAX<int>(1, font->getFontHeight());
	const int sidePad = 8;
	const int lineStep = align == Graphics::kTextAlignCenter ? 18 : 20;
	const int contentWidth = style == kMacInformWindow ? 370 : textWidth + 16;
	const int contentHeight = style == kMacInformWindow ? 72 : 4 + ((int)lines.size() + 1) * 18;
	Common::Rect contentRect;
	if (style == kMacInformWindow)
		contentRect = Common::Rect(56, 60, 56 + contentWidth, 60 + contentHeight);
	else
		contentRect = Common::Rect(centerX - contentWidth / 2, centerY - contentHeight / 2,
			centerX - contentWidth / 2 + contentWidth, centerY - contentHeight / 2 + contentHeight);

	Common::Rect bounds(8, 24, screenWidth - 8, screenHeight - 8);
	if (wm) {
		Graphics::MacWindowBorder border;
		border.setWindowManager(wm);
		border.setBorderType(Graphics::kWindowWindow);
		if (border.hasBorder(Graphics::kWindowBorderActive) && border.hasOffsets()) {
			const Graphics::BorderOffsets &offsets = border.getOffset();
			const int popupWidth = MAX<int>(border.getMinWidth(Graphics::kWindowBorderActive),
				contentWidth + offsets.left + offsets.right);
			const int popupHeight = MAX<int>(border.getMinHeight(Graphics::kWindowBorderActive),
				contentHeight + offsets.top + offsets.bottom);
			Common::Rect r(contentRect.left - offsets.left, contentRect.top - offsets.top,
				contentRect.left - offsets.left + popupWidth, contentRect.top - offsets.top + popupHeight);
			if (r.left < bounds.left)
				r.translate(bounds.left - r.left, 0);
			if (r.right > bounds.right)
				r.translate(bounds.right - r.right, 0);
			if (r.top < bounds.top)
				r.translate(0, bounds.top - r.top);
			if (r.bottom > bounds.bottom)
				r.translate(0, bounds.bottom - r.bottom);
			if (popupBounds)
				*popupBounds = r;
			if (measureOnly)
				return t
rue;

			Graphics::ManagedSurface popup;
			popup.create(popupWidth, popupHeight, wm->_pixelformat);
			popup.fillRect(Common::Rect(0, 0, popupWidth, popupHeight), popup.format.ARGBToColor(0, 0, 0, 0));
			Common::Rect inner(offsets.left, offsets.top, popupWidth - offsets.right, popupHeight - offsets.bottom);
			inner.clip(Common::Rect(0, 0, popupWidth, popupHeight));
			if (!inner.isEmpty())
				popup.fillRect(inner, wm->_colorWhite);
			border.blitBorderInto(popup, Graphics::kWindowBorderActive);

			const int textX = inner.left + sidePad;
			const int textY = style == kMacInformWindow ?
				inner.top + (inner.height() - fontHeight) / 2 : inner.top + MAX<int>(0, 14 - fontHeight);
			const int textW = MAX<int>(1, inner.width() - sidePad * 2);
			for (uint i = 0; i < visibleLines; ++i)
				font->drawString(&popup, lines[i], textX, textY + (int)i * lineStep, textW, wm->_colorBlack, align);

			gfx->drawSurface(&popup.rawSurface(), r.left, r.top);
			gfx->copyToScreen();
			popup.free();
			return true;
		}
	}

	Common::Rect r = contentRect;
	if (r.left < bounds.left)
		r.translate(bounds.left - r.left, 0);
	if (r.right > bounds.right)
		r.translate(bounds.right - r.right, 0);
	if (r.top < bounds.top)
		r.translate(0, bounds.top - r.top);
	if (r.bottom > bounds.bottom)
		r.translate(0, bounds.bottom - r.bottom);
	if (popupBounds)
		*popupBounds = r;
	if (measureOnly)
		return true;

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
	const int startY = style == kMa
cInformWindow ?
		r.top + (r.height() - fontHeight) / 2 : r.top + MAX<int>(0, 14 - fontHeight);
	for (uint i = 0; i < visibleLines; ++i) {
		const int y = startY + (int)i * lineStep;
		if (align == Graphics::kTextAlignCenter)
			gfx->drawString(font, lines[i], textCenter, y, colBlack, Graphics::kTextAlignCenter);
		else
			gfx->drawString(font, lines[i], textLeft, y, colBlack, Graphics::kTextAlignLeft);
	}

	gfx->copyToScreen();
	return true;
}

// Load a dashboard PICT resource from the Mac resource fork, returning a new
// RGB surface. The companion Color Colony resource fork contains the complete
// dashboard PICT set, so prefer it when available and fall back to the base app.
// Caller owns the returned surface. Returns nullptr on failure.
Graphics::Surface *ColonyEngine::loadPictSurface(int resID) {
	Common::SeekableReadStream *pictStream = nullptr;

	if (_colorResMan && _colorResMan->hasResFork())
		pictStream = _colorResMan->getResource(MKTAG('P', 'I', 'C', 'T'), (int16)resID);

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
				c
onst Graphics::Palette &checkPal = decoder.getPalette();
				invert1bit = ((int)checkPal.size() <= 2);
			}

			result = new Graphics::Surface();
			result->create(src->w, src->h, _gfx->getPixelFormat());
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

// Draw a PICT surface at a specific destination position. The surface was
// created by loadPictSurface() in the exact ARGB layout the renderer's
// drawSurface() consumes, so this is a single textured-quad blit.
void ColonyEngine::drawPictAt(Graphics::Surface *surf, int destX, int destY) {
	if (!surf)
		return;
	_gfx->drawSurface(surf, destX, destY);
}

void ColonyEngine::updateViewportLayout() {
	auto makeSafeRect = [](int left, int top, int right, int bottom) {
		if (right < left)
			right = left;
		if (bottom < top)
			bottom = top;
		return Common::Rect(left, top, right, bottom);
	};

	const bool isMac = isMacRenderMode();

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
		// Two floating windows over gray desktop.
		// moveWindow: compRect = (0,0, 2*CCENTER, 3*CCENTER) = (0,0, 70, 105)
		//   floorRect (minimap) = (8,8)-(62,62)  54x54 inside moveWindow
		//   compass dish below at (19,66)-(51,98), needle center at (35,82)
		// infoWindow: original WIND 10930 rect (9,26)-(79,189).
		const int CCENTER = 35;
		auto loadWindowRect = [this, &makeSafeRect](int resID, const Common::Rect &fallback) {
			Common::SeekableReadStream *windStream = nullptr;
			if (_resMan && (_resMan->isMacFile() || _resMan->hasResFork()))
				windStream = _resMan->getResource(MKTAG('W', 'I', 'N', 'D'), (int16)resID);
			if (!windStream && _colorResMan && _colorResMan->hasResFork())
				windStream = _colorResMan->getResource(MKTAG('W', 'I', 'N', 'D'), (int16)resID);

			if (!windStream)
				return fallback;

			Common::Rect r = fallback;
			if (windStream->size() >= 8) {
				const int top = windStream->r
eadSint16BE();
				const int left = windStream->readSint16BE();
				const int bottom = windStream->readSint16BE();
				const int right = windStream->readSint16BE();
				r = makeSafeRect(left, top, right, bottom);
			}
			delete windStream;
			return r;
		};

		const Common::Rect infoWindow = loadWindowRect(10930, Common::Rect(9, 26, 79, 189));

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
				wantID = isMacColorMode() ? -32761 : -32752;
			_pictPower = loadPictSurface(wantID);
			if (!_pictPower && _armor > 0 && wantID != -32755)
				_pictPower = loadPictSurface(-32755);
			_pictPowerID = _pictPower ? wantID : 0;
		}

		// Keep the ScummVM dashboard's bottom moveWindow placement. The original
		// Mac window resource is for a compact 512x342 desktop and should not
		// move this panel upward on our larger Mac render surface.
		const int moveW = 2 * CCENTER; // 70
		const int moveH = 3 * CCENTER; // 105
		const int infoW = infoWindow.width();
		const int infoH = infoWindow.height();

		const int centerX = dashWidth / 2;

		// Position moveWindow at the bottom of the sidebar.
		const int moveLeft = MAX(0, centerX - moveW / 2);
		const int moveTop = _height - pad - moveH;

		// _headsUpRect = floorRect (8,8)-(62,62) relative to moveWindow
		// This is the minimap clipping area  must NOT overlap compass dish
		_headsUpRect = makeSafeRect(moveLeft + 8, moveTop + 8,
			moveLeft + 2 * CCENTER - 8, moveTop + 2 * CCENTER - 8);

		// _compassRect = entire moveWindow (used for compass dish drawing)
		_compassRect = makeSafeRect(moveLeft, moveTop, moveLeft + moveW, mov
eTop + moveH);

		int infoLeft = infoWindow.left;
		int infoTop = infoWindow.top;
		if (infoLeft + infoW > dashWidth)
			infoLeft = MAX(0, dashWidth - infoW);
		if (infoTop + infoH > _height)
			infoTop = MAX(menuTop, _height - pad - infoH);
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
		const int pBottomAdj = pTop + _powerHeight
 * 32;
		_powerRect = makeSafeRect(powerLeft, pTop, powerLeft + totalBarWidth, pBottomAdj);
	}
}

// DASHBOAR.C qlog(): bit-length of x (equivalent to floor(log2(x))+1)
int qlog(int32 x) {
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

	const bool isMac = isMacRenderMode();

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
		// Original DOS draws a solid black outer oval, shrinks the rect by 2px on
		// each side, then erases the inner oval to white, 
leaving a black annulus.
		const int cx = (_compassRect.left + _compassRect.right) >> 1;
		const int cy = (_compassRect.top + _compassRect.bottom) >> 1;
		const int outerRx = (_compassRect.width() + 4) >> 1;
		const int outerRy = (_compassRect.height() + 4) >> 1;
		const int innerRx = _compassRect.width() >> 1;
		const int innerRy = _compassRect.height() >> 1;

		_gfx->fillEllipse(cx, cy, outerRx, outerRy, 0);
		_gfx->fillEllipse(cx, cy, innerRx, innerRy, 15);

		// In the current ScummVM colony controls, the rendered camera follows
		// _me.look. Using _me.ang here leaves the DOS compass static under
		// mouse/camera rotation even though the scene is turning.
		const int ex = cx + ((_cost[_me.look] * _compassRect.width()) >> 8);
		const int ey = cy - ((_sint[_me.look] * _compassRect.height()) >> 8);
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
		_gfx->drawLine(pl + _powerWidth - 2, _powerRect.bottom - 2, pl +
 (_powerWidth >> 1), _powerRect.bottom - (_powerWidth - 2), 0);
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
	const bool macColor = isMacColorMode();
	const uint32 colBlack = packRGB(0, 0, 0);
	const uint32 colWhite = packRGB(255, 255, 255);
	const 
uint32 colWinBg = macColor ? packMacColor(_macColors[7].bg) : colWhite;
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
			_pictPowerID = wa
ntPictID;
		}

		// power.c: SetRect(&info, -2, -2, xSize-2, ySize-2); DrawPicture(inf, &info)
		// — art pixel (2,2) sits at the window origin, so color blits at -2.
		// The B&W path keeps its screenshot-calibrated +1 offset.
		// In the original B&W game, GetPicture(-32752) returns null when !armor,
		// so DrawPicture is a no-op — the window just shows white fill.
		if (_pictPower) {
			if (macColor)
				drawPictAt(_pictPower, _powerRect.left - 2, _powerRect.top - 2);
			else
				drawPictAt(_pictPower, _powerRect.left + 1, _powerRect.top + 1);
		}

		if (!macColor) {
			// Match the B&W Window Manager shadow as pixels. The shadow is
			// staggered over the checkerboard desktop; a solid rect is too wide.
			_gfx->fillRect(Common::Rect(_powerRect.right, _powerRect.top - 1,
				_powerRect.right + 1, _powerRect.bottom + 3), colBlack);
			_gfx->fillRect(Common::Rect(_powerRect.right + 1, _powerRect.top,
				_powerRect.right + 2, _powerRect.bottom + 4), colBlack);
			_gfx->fillRect(Common::Rect(_powerRect.right + 2, _powerRect.top - 1,
				_powerRect.right + 3, _powerRect.top), colBlack);
			_gfx->fillRect(Common::Rect(_powerRect.right + 2, _powerRect.top + 1,
				_powerRect.right + 3, _powerRect.bottom + 3), colBlack);
			_gfx->fillRect(Common::Rect(_powerRect.left - 2, _powerRect.bottom,
				_powerRect.right + 3, _powerRect.bottom + 1), colBlack);
			_gfx->fillRect(Common::Rect(_powerRect.left + 1, _powerRect.bottom + 1,
				_powerRect.right + 4, _powerRect.bottom + 2), colBlack);
			_gfx->fillRect(Common::Rect(_powerRect.left, _powerRect.bottom + 2,
				_powerRect.right + 3, _powerRect.bottom + 3), colBlack);
		}

		// Blue bars only when armored (power.c: if(armor) { ... ForeColor(blueColor) ... })
		if (_armor > 0 && _pictPower) {
			// power.c draws the bars with QuickDraw MoveTo/LineTo after shifting
			// the PICT rect. GL line rasterization does not cover the same pixels,
			// so draw the observed bar strips explicitly.
			if (macColor) {
				// Color power
.c DrawInfo(): lft = 3 + info.left + i*23 with the
				// shifted info rect's local origin at engine x = _powerRect.left
				// (PICT blitted at -2); strips MoveTo(lft+1)..LineTo(lft+16) = 16px.
				const int infoLeft = _powerRect.left;
				const int bot = _powerRect.bottom - 30; // power.c: bot = info.bottom - 27

				for (int i = 0; i < 3; i++) {
					const int lft = 3 + infoLeft + i * 23;
					for (int j = 0; j < ePower[i] && j < 20; j++) {
						const int ln = bot - 3 * j;
						if (ln <= _powerRect.top)
							break;
						_gfx->fillRect(Common::Rect(lft + 1, ln - 1, lft + 17, ln + 1), colBlue);
					}
				}
			} else {
				// The B&W art uses other sizes than the color source. Reference B&W
				// frame: columns start at x 10, 33, 56; bars are inset by 2 pixels
				// on each side: x 12..29, 35..52, 58..75 (18px wide).
				const int infoLeft = _powerRect.left - 1;
				const int bot = _powerRect.bottom - 27; // power.c: bot = info.bottom - 27

				for (int i = 0; i < 3; i++) {
					const int lft = 3 + infoLeft + i * 23;
					for (int j = 0; j < ePower[i] && j < 20; j++) {
						const int ln = bot - 3 * j;
						if (ln <= _powerRect.top)
							break;
						_gfx->fillRect(Common::Rect(lft + 1, ln - 1, lft + 19, ln + 1), colBlue);
					}
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
		// Eye icon at center (CCENTER,
CCENTER) = (35,35)
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

// Draw
s the mini floor map into _headsUpRect (shared by Mac and DOS paths)
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

	const bool isMac = isMacRenderMode();

	int lExt, sExt, xloc, yloc, ccenterx, ccentery;
	if (isMac) {
		// compass.c: CSIZE=64, CCENTER=35
		// xloc = ((Me.xindex << 8) - Me.xloc) >> 2
		// Center at

... [Content truncated]