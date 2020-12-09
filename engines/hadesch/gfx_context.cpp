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
 * Copyright 2020 Google
 *
 */
#include "hadesch/gfx_context.h"
#include "hadesch/video.h"
#include "hadesch/baptr.h"
#include "common/system.h"
#include "graphics/palette.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "hadesch/hadesch.h"

namespace Hadesch {

void blendVideo8To8(byte *targetPixels, int targetPitch, int targetW, int targetH,
		    byte *sourcePixels, int sourceW, int sourceH, Common::Point offset) {
	int ymin = MAX(0, -offset.y);
	int ymax = MIN(sourceH, targetH - offset.y);
	for (int y = ymin; y < ymax; y++) {
		int xmin = MAX(0, -offset.x);
		int xmax = MIN(sourceW, targetW - offset.x);
		const byte *inptr = sourcePixels + sourceW * y + xmin;
		byte *outptr = targetPixels + targetPitch * (y + offset.y) + offset.x + xmin;
		for (int x = xmin; x < xmax; x++, inptr++, outptr++) {
			if (*inptr)
				*outptr = *inptr;
		}
	}
}

void GfxContext8Bit::blitPodImage(byte *sourcePixels, int sourcePitch, int sourceW, int sourceH,
				  byte *sourcePalette, size_t ncolours, Common::Point offset) {
	
	blendVideo8To8((byte *) surf.getPixels(), surf.pitch,
		       surf.w, surf.h, sourcePixels, sourceW, sourceH,
		       offset);
	for (unsigned i = 0; i < ncolours; i++) {
		int col = sourcePalette[4 * i] & 0xff;
		
		_palette[3 * col    ] = sourcePalette[4 * i + 1];
		_palette[3 * col + 1] = sourcePalette[4 * i + 2];
		_palette[3 * col + 2] = sourcePalette[4 * i + 3];
		_paletteUsed[col] = true;
	}
}

void GfxContext8Bit::blitVideo(byte *sourcePixels, int sourcePitch, int sourceW, int sourceH,
			       byte *sourcePalette, Common::Point offset) {
	blendVideo8To8((byte *) surf.getPixels(), surf.pitch, surf.w, surf.h, sourcePixels, sourceW, sourceH, offset);
	for (int i = 0; i < 256; i++)
		if (!_paletteUsed[i]) {
			_palette[3 * i] = sourcePalette[3 * i];
			_palette[3 * i + 1] = sourcePalette[3 * i + 1];
			_palette[3 * i + 2] = sourcePalette[3 * i + 2];
		}
}

void GfxContext8Bit::fade(int val) {
	if (val == 0x100)
		return;
	for (int i = 0; i < 256 * 3; i++) {
		_palette[i] = ((_palette[i] & 0xff) * val) >> 8;
	}
}

void GfxContext8Bit::clear() {
	surf.clearPalette();
	surf.clear();
	memset(_palette, 0, sizeof(_palette));
	memset(_paletteUsed, 0, sizeof(_paletteUsed));
}

GfxContext8Bit::GfxContext8Bit(int canvasW, int canvasH) : surf(canvasW, canvasH, Graphics::PixelFormat::createFormatCLUT8()) {
	clear();
}

void GfxContext8Bit::renderToScreen(Common::Point viewPoint) {
	if (_palette) {
		g_system->getPaletteManager()->setPalette(_palette, 0, 256);
	}

	g_system->copyRectToScreen(surf.getBasePtr(viewPoint.x, viewPoint.y), surf.w, 0, 0,
				   kVideoWidth, kVideoHeight);
}

byte GfxContext8Bit::findColor(byte r, byte g, byte b) {
	for (uint i = 1; i < 256; i++)
		if (_paletteUsed[i] && _palette[3 * i] == r && _palette[3 * i + 1] == g && _palette[3 * i + 2] == b) {
			return i;
		}
	for (uint i = 1; i < 256; i++)
		if (!_paletteUsed[i]) {
			_palette[3 * i] = r;
			_palette[3 * i + 1] = g;
			_palette[3 * i + 2] = b;
			_paletteUsed[i] = true;
			return i;
		}
	int diff = 0x40000; int c = 0;
	for (uint i = 1; i < 256; i++) {
		int cDiff = (_palette[3 * i] - r) * (_palette[3 * i] - r) + (_palette[3 * i + 1] - g) * (_palette[3 * i + 1] - g)
			+ (_palette[3 * i + 2] - b) * (_palette[3 * i + 2] - b);
		if (cDiff < diff) {
			diff = cDiff;
			c = i;
		}
	}

	return c;
}

void GfxContext8Bit::renderSubtitle(Common::U32String const& line, Common::Point viewPoint) {
	int fgColor = findColor(0xff, 0xff, 0xff);
	int bgColor = findColor(0, 0, 0);
	const Graphics::Font &font(*FontMan.getFontByUsage(Graphics::FontManager::kLocalizedFont));
	Common::Rect rect(70 + viewPoint.x, 420 + viewPoint.y, 570 + viewPoint.x, 420 + viewPoint.y + font.getFontHeight());
	surf.fillRect(rect, bgColor);
	font.drawString(&surf, line, rect.left, rect.top, rect.width() - 10,
			fgColor, Graphics::kTextAlignCenter);
}

void HadeschEngine::wrapSubtitles(const Common::U32String &str, Common::Array<Common::U32String> &lines) {
	const Graphics::Font &font(*FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont));
	font.wordWrapText(str, 500, lines);
}
}
