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

#include "colony/colony.h"
#include "colony/gfx.h"
#include "common/system.h"
#include "common/util.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/events.h"
#include "graphics/palette.h"
#include "graphics/fonts/dosfont.h"
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

	if (!pictStream)
		return nullptr;

	::Image::PICTDecoder decoder;
	Graphics::Surface *result = nullptr;
	if (decoder.loadStream(*pictStream)) {
		const Graphics::Surface *src = decoder.getSurface();
		if (src) {
			// Convert to a persistent RGB surface (decoder surface is transient)
			result = new Graphics::Surface();
			result->create(src->w, src->h, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
			for (int y = 0; y < src->h; y++) {
				for (int x = 0; x < src->w; x++) {
					byte r, g, b;
					if (src->format == Graphics::PixelFormat::createFormatCLUT8()) {
						// For CLUT8, use the decoder's palette
						byte idx = *((const byte *)src->getBasePtr(x, y));
						const Graphics::Palette &pal = decoder.getPalette();
						if (idx < (int)pal.size()) {
							pal.get(idx, r, g, b);
						} else {
							r = g = b = 0;
						}
					} else {
						uint32 pixel = src->getPixel(x, y);
						src->format.colorToRGB(pixel, r, g, b);
					}
					result->setPixel(x, y, result->format.ARGBToColor(255, r, g, b));
				}
			}
			debugC(1, kColonyDebugUI, "loadPictSurface(%d): %dx%d", resID, result->w, result->h);
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

	int dashWidth = 0;
	if (_showDashBoard) {
		// Mac mode: original inits.c sets screenR.left=96  fixed 96px sidebar.
		// Two floating windows (infoWindow + moveWindow) centered in sidebar.
		const bool isMac = (_renderMode == Common::kRenderMacintosh);
		if (isMac)
			dashWidth = MIN(96, _width / 2);
		else
			dashWidth = CLIP<int>(_width / 6, 72, 140);
		if (_width - dashWidth < 160)
			dashWidth = 0;
	}

	const int menuTop = _menuBarHeight; // 0 for DOS/EGA, 20 for Mac

	_screenR = makeSafeRect(dashWidth, menuTop, _width, _height);
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

	const bool isMac = (_renderMode == Common::kRenderMacintosh);
	const int pad = 2;
	const int topPad = menuTop + pad;

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
			// power.c: !armor → FindDepth()>=8 ? -32761 (color) : -32752 (B&W)
			int wantID;
			if (_armor > 0)
				wantID = -32755;
			else
				wantID = _hasMacColors ? -32761 : -32752;
			_pictPower = loadPictSurface(wantID);
			if (!_pictPower && wantID != -32755)
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
		const int blockLeft = pad;
		const int blockRight = dashWidth - pad;
		const int unit = MAX(8, (dashWidth - (pad * 2)) / 4);

		const int compassBottom = _height - MAX(2, unit / 4);
		const int compassTop = MAX(topPad, compassBottom - unit * 4);
		_compassRect = makeSafeRect(blockLeft, compassTop, blockRight, compassBottom);

		const int headsUpBottom = _compassRect.top - MAX(2, unit / 4) - 4;
		const int headsUpTop = headsUpBottom - unit * 4;
		_headsUpRect = makeSafeRect(blockLeft, MAX(topPad, headsUpTop), blockRight, MAX(topPad, headsUpBottom));

		_powerRect = makeSafeRect(blockLeft, topPad, blockRight, _headsUpRect.top - 4);
	}
}

void ColonyEngine::drawDashboardStep1() {
	if (_dashBoardRect.width() <= 0 || _dashBoardRect.height() <= 0)
		return;

	const bool isMac = (_renderMode == Common::kRenderMacintosh);

	if (isMac) {
		drawDashboardMac();
		return;
	}

	// --- DOS/EGA path (unchanged) ---
	_gfx->fillRect(_dashBoardRect, 0);
	_gfx->fillDitherRect(_dashBoardRect, 0, 15);
	_gfx->drawRect(_dashBoardRect, 1);

	const int shiftX = MAX(1, _dashBoardRect.width() / 8);
	const int shiftY = MAX(1, _dashBoardRect.width() / 8);
	const Common::Rect r = _dashBoardRect;

	_gfx->drawLine(r.left, r.top, r.left + shiftX, r.top + shiftY, 0);
	_gfx->drawLine(r.right - 1, r.top, r.right - 1 - shiftX, r.top + shiftY, 0);
	_gfx->drawLine(r.left, r.bottom - 1, r.left + shiftX, r.bottom - 1 - shiftY, 0);
	_gfx->drawLine(r.right - 1, r.bottom - 1, r.right - 1 - shiftX, r.bottom - 1 - shiftY, 0);

	const Common::Rect tr(r.left + shiftX, r.top + shiftY, r.right - shiftX, r.bottom - shiftY);
	_gfx->fillDitherRect(tr, 0, 15);
	_gfx->drawRect(tr, 0);

	if (_compassRect.width() > 4 && _compassRect.height() > 4) {
		const int cx = (_compassRect.left + _compassRect.right) >> 1;
		const int cy = (_compassRect.top + _compassRect.bottom) >> 1;
		const int rx = MAX(2, (_compassRect.width() - 6) >> 1);
		const int ry = MAX(2, (_compassRect.height() - 6) >> 1);

		_gfx->fillEllipse(cx, cy, rx, ry, 15);
		_gfx->drawEllipse(cx, cy, rx, ry, 1);

		const int ex = cx + ((_cost[_me.look] * rx) >> 8);
		const int ey = cy - ((_sint[_me.look] * ry) >> 8);
		_gfx->drawLine(cx, cy, ex, ey, 0);
		_gfx->drawLine(cx - 2, cy, cx + 2, cy, 0);
		_gfx->drawLine(cx, cy - 2, cx, cy + 2, 0);
	}

	if (_screenR.left > 0)
		_gfx->drawLine(_screenR.left - 1, _screenR.top, _screenR.left - 1, _screenR.bottom - 1, 15);

	if (_headsUpRect.width() > 4 && _headsUpRect.height() > 4) {
		_gfx->fillRect(_headsUpRect, 15);
		_gfx->drawRect(_headsUpRect, 1);
		drawMiniMap(0);
	}

	if (_powerRect.width() > 4 && _powerRect.height() > 4) {
		_gfx->fillRect(_powerRect, 15);
		_gfx->drawRect(_powerRect, 1);
		const int barY = _powerRect.bottom - MAX(3, _powerRect.width() / 8);
		_gfx->drawLine(_powerRect.left + 1, barY, _powerRect.right - 2, barY, 0);
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

		// Reload PICT if state changed (fall back to -32755 if variant missing)
		if (_pictPowerID != wantPictID) {
			if (_pictPower) {
				_pictPower->free();
				delete _pictPower;
				_pictPower = nullptr;
			}
			_pictPower = loadPictSurface(wantPictID);
			if (!_pictPower && wantPictID != -32755)
				_pictPower = loadPictSurface(-32755);
			_pictPowerID = wantPictID;
		}

		// power.c: SetRect(&info, -2, -2, xSize-2, ySize-2); DrawPicture(inf, &info)
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

// Draws the mini floor map into _headsUpRect (shared by Mac and DOS paths)
void ColonyEngine::drawMiniMap(uint32 lineColor) {
	if (_gameMode != kModeColony)
		return;
	if (_me.xindex < 0 || _me.xindex >= 32 || _me.yindex < 0 || _me.yindex >= 32)
		return;

	const int kFWALLType = 48;
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

	auto drawMarker = [&](int x, int y, int halfSize, uint32 color) {
		if (x < _headsUpRect.left + 1 || x >= _headsUpRect.right - 1 ||
		    y < _headsUpRect.top + 1 || y >= _headsUpRect.bottom - 1)
			return;
		if (isMac) {
			// compass.c: FrameOval  circle outline
			_gfx->drawEllipse(x, y, halfSize, halfSize, color);
		} else {
			const int l = MAX<int>(_headsUpRect.left + 1, x - halfSize);
			const int t = MAX<int>(_headsUpRect.top + 1, y - halfSize);
			const int r = MIN<int>(_headsUpRect.right - 1, x + halfSize + 1);
			const int b = MIN<int>(_headsUpRect.bottom - 1, y + halfSize + 1);
			if (l >= r || t >= b)
				return;
			_gfx->drawRect(Common::Rect(l, t, r, b), color);
		}
	};

	auto hasRobotAt = [&](int x, int y) -> bool {
		if (x < 0 || x >= 32 || y < 0 || y >= 32)
			return false;
		return _robotArray[x][y] != 0;
	};
	auto hasFoodAt = [&](int x, int y) -> bool {
		if (x < 0 || x >= 32 || y < 0 || y >= 32)
			return false;
		const uint8 num = _foodArray[x][y];
		if (num == 0)
			return false;
		if (num <= _objects.size())
			return _objects[num - 1].type < kFWALLType;
		return true;
	};

	if (hasFoodAt(_me.xindex, _me.yindex))
		drawMarker(xcorner[4], ycorner[4], foodR, lineColor);

	if (_me.yindex > 0 && !(_wall[_me.xindex][_me.yindex] & 0x01)) {
		if (hasFoodAt(_me.xindex, _me.yindex - 1))
			drawMarker(xcorner[4] + dy, ycorner[4] + dx, foodR, lineColor);
		if (hasRobotAt(_me.xindex, _me.yindex - 1))
			drawMarker(xcorner[4] + dy, ycorner[4] + dx, robotR, lineColor);
	}
	if (_me.xindex > 0 && !(_wall[_me.xindex][_me.yindex] & 0x02)) {
		if (hasFoodAt(_me.xindex - 1, _me.yindex))
			drawMarker(xcorner[4] - dx, ycorner[4] + dy, foodR, lineColor);
		if (hasRobotAt(_me.xindex - 1, _me.yindex))
			drawMarker(xcorner[4] - dx, ycorner[4] + dy, robotR, lineColor);
	}
	if (_me.yindex < 30 && !(_wall[_me.xindex][_me.yindex + 1] & 0x01)) {
		if (hasFoodAt(_me.xindex, _me.yindex + 1))
			drawMarker(xcorner[4] - dy, ycorner[4] - dx, foodR, lineColor);
		if (hasRobotAt(_me.xindex, _me.yindex + 1))
			drawMarker(xcorner[4] - dy, ycorner[4] - dx, robotR, lineColor);
	}
	if (_me.xindex < 30 && !(_wall[_me.xindex + 1][_me.yindex] & 0x02)) {
		if (hasFoodAt(_me.xindex + 1, _me.yindex))
			drawMarker(xcorner[4] + dx, ycorner[4] - dy, foodR, lineColor);
		if (hasRobotAt(_me.xindex + 1, _me.yindex))
			drawMarker(xcorner[4] + dx, ycorner[4] - dy, robotR, lineColor);
	}
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
		color = (_weapons > 0) ? 15 : 7;
		if (_corePower[_coreIndex] > 0)
			color = 0;
	}

	const int cx = _centerX;
	const int cy = _centerY;
	const int qx = MAX(2, _screenR.width() / 32);
	const int qy = MAX(2, _screenR.height() / 32);
	const int fx = (qx * 3) >> 1;
	const int fy = (qy * 3) >> 1;
	auto drawCrossLine = [&](int x1, int y1, int x2, int y2) {
		if (clipLineToRect(x1, y1, x2, y2, _screenR))
			_gfx->drawLine(x1, y1, x2, y2, color);
	};

	if (_weapons > 0) {
		const int yTop = _insight ? (cy - qy) : (cy - fy);
		const int yBottom = _insight ? (cy + qy) : (cy + fy);

		drawCrossLine(cx - qx, yTop, cx - fx, yTop);
		drawCrossLine(cx - fx, yTop, cx - fx, yBottom);
		drawCrossLine(cx - fx, yBottom, cx - qx, yBottom);
		drawCrossLine(cx + qx, yTop, cx + fx, yTop);
		drawCrossLine(cx + fx, yTop, cx + fx, yBottom);
		drawCrossLine(cx + fx, yBottom, cx + qx, yBottom);
		_insight = false;
	} else {
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

	while (text[numLines] != nullptr) {
		int w = font.getStringWidth(text[numLines]);
		if (w > width)
			width = w;
		numLines++;
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

	byte *page = (byte *)malloc(ch + 1);
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
	int kw = font.getStringWidth(kpress);
	if (kw > width)
		width = kw;
	width += 12;

	int lineheight = 14;
	int maxlines = (_screenR.height() / lineheight) - 2;
	if (maxlines > (int)lineArray.size())
		maxlines = lineArray.size();

	Common::Rect r;
	r.top = _centerY - (((maxlines + 1) * lineheight / 2) + 4);
	r.bottom = _centerY + (((maxlines + 1) * lineheight / 2) + 4);
	r.left = _centerX - (width / 2);
	r.right = _centerX + (width / 2);

	_gfx->fillDitherRect(_screenR, 0, 15);

	// Draw shadow/border (original draws 3 frames total)
	for (int i = 0; i < 2; i++) {
		_gfx->drawRect(r, 0);
		r.translate(-1, -1);
	}
	_gfx->fillRect(r, 15);
	_gfx->drawRect(r, 0);

	for (int i = 0; i < maxlines; i++) {
		_gfx->drawString(&font, lineArray[i], r.left + 3, r.top + 4 + i * lineheight, 0);
		if (center == 2) {
			_sound->play(Sound::kDit);
			_system->delayMillis(20);
		}
	}

	_gfx->drawString(&font, (int)lineArray.size() > maxlines ? "-More-" : kpress, (r.left + r.right) / 2, r.top + 6 + maxlines * lineheight, 0, Graphics::kTextAlignCenter);
	_gfx->copyToScreen();

	// Wait for key
	waitForInput();

	free(page);
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
