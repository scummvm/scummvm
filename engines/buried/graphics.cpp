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
 */

#include "common/fs.h"
#ifdef MACOSX
#include "common/macresman.h"
#endif
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "graphics/wincursor.h"
#include "graphics/fonts/ttf.h"
#include "image/bmp.h"

#include "buried/buried.h"
#include "buried/database.h"
#include "buried/graphics.h"
#include "buried/window.h"

namespace Buried {

GraphicsManager::GraphicsManager(BuriedEngine *vm) : _vm(vm) {
	_curCursor = kCursorNone;
	_mouseMoved = false;

	setCursor(kCursorArrow);
	CursorMan.showMouse(true);

	_screen = new Graphics::Surface();
	_screen->create(640, 480, g_system->getScreenFormat());

	if (_vm->isTrueColor()) {
		// No palette to deal with
		_palette = 0;
	} else {
		// Grab the palette from our EXE bitmap
		_palette = createDefaultPalette();

		// Then apply it. The only time we'll use this call even.
		g_system->getPaletteManager()->setPalette(_palette, 0, 256);
	}
}

GraphicsManager::~GraphicsManager() {
	_screen->free();
	delete _screen;

	delete[] _palette;
}

#define REQUIRED(x) (((uint32)(x)) | 0x80000000)
#define NOT_REQUIRED(x) (x)

static const uint32 s_codePage1252[256] = {
	NOT_REQUIRED(0x0000), NOT_REQUIRED(0x0001), NOT_REQUIRED(0x0002), NOT_REQUIRED(0x0003),
	NOT_REQUIRED(0x0004), NOT_REQUIRED(0x0005), NOT_REQUIRED(0x0006), NOT_REQUIRED(0x0007),
	NOT_REQUIRED(0x0008), NOT_REQUIRED(0x0009), NOT_REQUIRED(0x000A), NOT_REQUIRED(0x000B),
	NOT_REQUIRED(0x000C), NOT_REQUIRED(0x000D), NOT_REQUIRED(0x000E), NOT_REQUIRED(0x000F),
	NOT_REQUIRED(0x0010), NOT_REQUIRED(0x0011), NOT_REQUIRED(0x0012), NOT_REQUIRED(0x0013),
	NOT_REQUIRED(0x0014), NOT_REQUIRED(0x0015), NOT_REQUIRED(0x0016), NOT_REQUIRED(0x0017),
	NOT_REQUIRED(0x0018), NOT_REQUIRED(0x0019), NOT_REQUIRED(0x001A), NOT_REQUIRED(0x001B),
	NOT_REQUIRED(0x001C), NOT_REQUIRED(0x001D), NOT_REQUIRED(0x001E), NOT_REQUIRED(0x001F),
	    REQUIRED(0x0020),     REQUIRED(0x0021),     REQUIRED(0x0022),     REQUIRED(0x0023),
	    REQUIRED(0x0024),     REQUIRED(0x0025),     REQUIRED(0x0026),     REQUIRED(0x0027),
	    REQUIRED(0x0028),     REQUIRED(0x0029),     REQUIRED(0x002A),     REQUIRED(0x002B),
	    REQUIRED(0x002C),     REQUIRED(0x002D),     REQUIRED(0x002E),     REQUIRED(0x002F),
	    REQUIRED(0x0030),     REQUIRED(0x0031),     REQUIRED(0x0032),     REQUIRED(0x0033),
	    REQUIRED(0x0034),     REQUIRED(0x0035),     REQUIRED(0x0036),     REQUIRED(0x0037),
	    REQUIRED(0x0038),     REQUIRED(0x0039),     REQUIRED(0x003A),     REQUIRED(0x003B),
	    REQUIRED(0x003C),     REQUIRED(0x003D),     REQUIRED(0x003E),     REQUIRED(0x003F),
	    REQUIRED(0x0040),     REQUIRED(0x0041),     REQUIRED(0x0042),     REQUIRED(0x0043),
	    REQUIRED(0x0044),     REQUIRED(0x0045),     REQUIRED(0x0046),     REQUIRED(0x0047),
	    REQUIRED(0x0048),     REQUIRED(0x0049),     REQUIRED(0x004A),     REQUIRED(0x004B),
	    REQUIRED(0x004C),     REQUIRED(0x004D),     REQUIRED(0x004E),     REQUIRED(0x004F),
	    REQUIRED(0x0050),     REQUIRED(0x0051),     REQUIRED(0x0052),     REQUIRED(0x0053),
	    REQUIRED(0x0054),     REQUIRED(0x0055),     REQUIRED(0x0056),     REQUIRED(0x0057),
	    REQUIRED(0x0058),     REQUIRED(0x0059),     REQUIRED(0x005A),     REQUIRED(0x005B),
	    REQUIRED(0x005C),     REQUIRED(0x005D),     REQUIRED(0x005E),     REQUIRED(0x005F),
	    REQUIRED(0x0060),     REQUIRED(0x0061),     REQUIRED(0x0062),     REQUIRED(0x0063),
	    REQUIRED(0x0064),     REQUIRED(0x0065),     REQUIRED(0x0066),     REQUIRED(0x0067),
	    REQUIRED(0x0068),     REQUIRED(0x0069),     REQUIRED(0x006A),     REQUIRED(0x006B),
	    REQUIRED(0x006C),     REQUIRED(0x006D),     REQUIRED(0x006E),     REQUIRED(0x006F),
	    REQUIRED(0x0070),     REQUIRED(0x0071),     REQUIRED(0x0072),     REQUIRED(0x0073),
	    REQUIRED(0x0074),     REQUIRED(0x0075),     REQUIRED(0x0076),     REQUIRED(0x0077),
	    REQUIRED(0x0078),     REQUIRED(0x0079),     REQUIRED(0x007A),     REQUIRED(0x007B),
	    REQUIRED(0x007C),     REQUIRED(0x007D),     REQUIRED(0x007E), NOT_REQUIRED(0x007F),
	NOT_REQUIRED(0xFFFE), NOT_REQUIRED(0xFFFE),     REQUIRED(0x201A),     REQUIRED(0x0192),
	    REQUIRED(0x201E),     REQUIRED(0x2026),     REQUIRED(0x2020),     REQUIRED(0x2021),
	    REQUIRED(0x02C6),     REQUIRED(0x2030),     REQUIRED(0x0160),     REQUIRED(0x2039),
	    REQUIRED(0x0152), NOT_REQUIRED(0xFFFE), NOT_REQUIRED(0xFFFE), NOT_REQUIRED(0xFFFE),
	NOT_REQUIRED(0xFFFE),     REQUIRED(0x2018),     REQUIRED(0x2019),     REQUIRED(0x201C),
	    REQUIRED(0x201D),     REQUIRED(0x2022),     REQUIRED(0x2013),     REQUIRED(0x2014),
	    REQUIRED(0x02DC),     REQUIRED(0x2122),     REQUIRED(0x0161),     REQUIRED(0x203A),
	    REQUIRED(0x0153), NOT_REQUIRED(0xFFFE), NOT_REQUIRED(0xFFFE),     REQUIRED(0x0178),
	    REQUIRED(0x00A0),     REQUIRED(0x00A1),     REQUIRED(0x00A2),     REQUIRED(0x00A3),
	    REQUIRED(0x00A4),     REQUIRED(0x00A5),     REQUIRED(0x00A6),     REQUIRED(0x00A7),
	    REQUIRED(0x00A8),     REQUIRED(0x00A9),     REQUIRED(0x00AA),     REQUIRED(0x00AB),
	    REQUIRED(0x00AC),     REQUIRED(0x00AD),     REQUIRED(0x00AE),     REQUIRED(0x00AF),
	    REQUIRED(0x00B0),     REQUIRED(0x00B1),     REQUIRED(0x00B2),     REQUIRED(0x00B3),
	    REQUIRED(0x00B4),     REQUIRED(0x00B5),     REQUIRED(0x00B6),     REQUIRED(0x00B7),
	    REQUIRED(0x00B8),     REQUIRED(0x00B9),     REQUIRED(0x00BA),     REQUIRED(0x00BB),
	    REQUIRED(0x00BC),     REQUIRED(0x00BD),     REQUIRED(0x00BE),     REQUIRED(0x00BF),
	    REQUIRED(0x00C0),     REQUIRED(0x00C1),     REQUIRED(0x00C2),     REQUIRED(0x00C3),
	    REQUIRED(0x00C4),     REQUIRED(0x00C5),     REQUIRED(0x00C6),     REQUIRED(0x00C7),
	    REQUIRED(0x00C8),     REQUIRED(0x00C9),     REQUIRED(0x00CA),     REQUIRED(0x00CB),
	    REQUIRED(0x00CC),     REQUIRED(0x00CD),     REQUIRED(0x00CE),     REQUIRED(0x00CF),
	    REQUIRED(0x00D0),     REQUIRED(0x00D1),     REQUIRED(0x00D2),     REQUIRED(0x00D3),
	    REQUIRED(0x00D4),     REQUIRED(0x00D5),     REQUIRED(0x00D6),     REQUIRED(0x00D7),
	    REQUIRED(0x00D8),     REQUIRED(0x00D9),     REQUIRED(0x00DA),     REQUIRED(0x00DB),
	    REQUIRED(0x00DC),     REQUIRED(0x00DD),     REQUIRED(0x00DE),     REQUIRED(0x00DF),
	    REQUIRED(0x00E0),     REQUIRED(0x00E1),     REQUIRED(0x00E2),     REQUIRED(0x00E3),
	    REQUIRED(0x00E4),     REQUIRED(0x00E5),     REQUIRED(0x00E6),     REQUIRED(0x00E7),
	    REQUIRED(0x00E8),     REQUIRED(0x00E9),     REQUIRED(0x00EA),     REQUIRED(0x00EB),
	    REQUIRED(0x00EC),     REQUIRED(0x00ED),     REQUIRED(0x00EE),     REQUIRED(0x00EF),
	    REQUIRED(0x00F0),     REQUIRED(0x00F1),     REQUIRED(0x00F2),     REQUIRED(0x00F3),
	    REQUIRED(0x00F4),     REQUIRED(0x00F5),     REQUIRED(0x00F6),     REQUIRED(0x00F7),
	    REQUIRED(0x00F8),     REQUIRED(0x00F9),     REQUIRED(0x00FA),     REQUIRED(0x00FB),
	    REQUIRED(0x00FC),     REQUIRED(0x00FD),     REQUIRED(0x00FE),     REQUIRED(0x00FF)
};

#undef REQUIRED
#undef NOT_REQUIRED

Graphics::Font *GraphicsManager::createFont(int size) const {
	Common::SeekableReadStream *stream = 0;

	// HACK: Try to load the system font
	// TODO: MS Gothic for the Japanese version (please buy for clone2727)
	// Arial for everything else (???)
#if defined(WIN32)
	Common::FSNode fontPath("C:/WINDOWS/Fonts/arial.ttf");

	if (fontPath.exists() && !fontPath.isDirectory() && fontPath.isReadable())
		stream = fontPath.createReadStream();

	if (!stream) {
		Common::FSNode win2kFontPath("C:/WINNT/Fonts/arial.ttf");

		if (win2kFontPath.exists() && !win2kFontPath.isDirectory() && win2kFontPath.isReadable())
			stream = win2kFontPath.createReadStream();
	}
#elif defined(MACOSX)
	// Attempt to load the font from the Arial.ttf font first
	Common::FSNode fontPath("/Library/Fonts/Arial.ttf");

	if (fontPath.exists() && !fontPath.isDirectory() && fontPath.isReadable())
		stream = fontPath.createReadStream();

	if (!stream) {
		// Try the suitcase on the system
		Common::FSNode fontDirectory("/Library/Fonts");
		Common::MacResManager resFork;

		// DOUBLE HACK WARNING: Just assume it's 0x1000
		// (it should always be this, the first font, but parsing the FOND would be better)
		if (fontDirectory.exists() && fontDirectory.isDirectory() && resFork.open(fontPath, "Arial") && resFork.hasResFork())
			stream = resFork.getResource(MKTAG('s', 'f', 'n', 't'), 0x1000);

		// ...and one last try
		if (!stream) {
			Common::FSNode msFontDirectory("/Library/Fonts/Microsoft");
			if (fontDirectory.exists() && fontDirectory.isDirectory() && resFork.open(fontPath, "Arial") && resFork.hasResFork())
				stream = resFork.getResource(MKTAG('s', 'f', 'n', 't'), 0x1000);
		}
	}
#endif

	if (!stream) {
		// TODO: Try to load an equivalent font from the theme
		return 0;
	}

	// TODO: Make the monochrome mode optional
	// Win3.1 obviously only had raster fonts, but BIT Win3.1 will render
	// with the TrueType font on Win7/Win8 (at least)
	// TODO: shift-jis (code page 932) for the Japanese version (again, buy for clone2727)
	Graphics::Font *font = Graphics::loadTTFFont(*stream, size, 0, _vm->isTrueColor() ? Graphics::kTTFRenderModeLight : Graphics::kTTFRenderModeMonochrome, s_codePage1252);
	delete stream;
	return font;
}

Cursor GraphicsManager::setCursor(Cursor newCursor) {
	Cursor oldCursor = _curCursor;
	Graphics::Cursor *cursor = 0;
	Graphics::WinCursorGroup *cursorGroup = 0;

	if (newCursor == kCursorArrow) {
		cursor = Graphics::makeDefaultWinCursor();
	} else if (newCursor == kCursorWait) {
		cursor = Graphics::makeBusyWinCursor();
	} else {
		cursorGroup = _vm->_mainEXE->getCursorGroup(newCursor);

		if (!cursorGroup)
			return kCursorNone;

		cursor = cursorGroup->cursors[0].cursor;
	}

	if (!cursor)
		error("Failed to find cursor %d", newCursor);

	// TODO: Fallback mode for platforms without cursor palettes in 8bpp mode?

	CursorMan.replaceCursor(cursor->getSurface(), cursor->getWidth(), cursor->getHeight(),
			cursor->getHotspotX(), cursor->getHotspotY(), cursor->getKeyColor());
	CursorMan.replaceCursorPalette(cursor->getPalette(), cursor->getPaletteStartIndex(), cursor->getPaletteCount());

	if (cursorGroup)
		delete cursorGroup;
	else
		delete cursor;

	_curCursor = newCursor;
	return oldCursor;
}

Graphics::Surface *GraphicsManager::getBitmap(uint32 bitmapID) {
	Common::SeekableReadStream *stream = _vm->getBitmapStream(bitmapID);

	if (!stream)
		error("Could not find bitmap %d", bitmapID);

	Graphics::Surface *surface = getBitmap(stream);
	if (!surface)
		error("Failed to decode bitmap %d", bitmapID);

	return surface;
}

Graphics::Surface *GraphicsManager::getBitmap(const Common::String &fileName) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(fileName);

	if (!stream)
		error("Could not find bitmap '%s'", fileName.c_str());

	Graphics::Surface *surface = getBitmap(stream);
	if (!surface)
		error("Failed to decode bitmap '%s'", fileName.c_str());

	return surface;
}

Graphics::Surface *GraphicsManager::getBitmap(Common::SeekableReadStream *stream) {
	Image::BitmapDecoder decoder;
	if (!decoder.loadStream(*stream)) {
		delete stream;
		return 0;
	}

	delete stream;

	// Convert to the screen format, if required
	if (decoder.getSurface()->format != g_system->getScreenFormat()) {
		assert(_vm->isTrueColor());
		return decoder.getSurface()->convertTo(g_system->getScreenFormat(), decoder.getPalette());
	}

	// Remap the palette, if required
	if (!_vm->isTrueColor() && memcmp(decoder.getPalette() + 3, getDefaultPalette() + 3, 256 - 6) != 0)
		return remapPalettedFrame(decoder.getSurface(), decoder.getPalette());

	// Just copy the frame
	Graphics::Surface *surface = new Graphics::Surface();
	surface->copyFrom(*decoder.getSurface());
	return surface;
}

uint32 GraphicsManager::getColor(byte r, byte g, byte b) {
	if (_vm->isTrueColor())
		return g_system->getScreenFormat().RGBToColor(r, g, b);

	// Find the best match color
	int diff = 0x7FFFFFFF;
	byte best = 0;

	for (uint i = 0; i < 256 && diff > 0; i++) {
		int rDiff = (int)_palette[i * 3] - (int)r;
		int gDiff = (int)_palette[i * 3 + 1] - (int)g;
		int bDiff = (int)_palette[i * 3 + 2] - (int)b;

		int curDiff = rDiff * rDiff + gDiff * gDiff + bDiff * bDiff;

		if (curDiff < diff) {
			best = i;
			diff = curDiff;
		}
	}

	return best;
}

void GraphicsManager::invalidateRect(const Common::Rect &rect) {
	if (_dirtyRect.isEmpty())
		_dirtyRect = rect;
	else
		_dirtyRect.extend(rect);
}

void GraphicsManager::updateScreen(bool drawWindows) {
	bool shouldUpdateScreen = _mouseMoved;
	_mouseMoved = false;

	if (!_dirtyRect.isEmpty()) {
		// Draw the main window, which will draw its children
		if (drawWindows)
			_vm->_mainWindow->updateWindow();

		// Copy just that rect
		g_system->copyRectToScreen(_screen->getBasePtr(_dirtyRect.left, _dirtyRect.top), _screen->pitch, _dirtyRect.left, _dirtyRect.top, _dirtyRect.width(), _dirtyRect.height());

		// Empty out the dirty rect
		_dirtyRect = Common::Rect();

		// Definitely update
		shouldUpdateScreen = true;
	}

	if (shouldUpdateScreen)
		g_system->updateScreen();
}

void GraphicsManager::blit(const Graphics::Surface *surface, int x, int y) {
	assert(surface->format.bytesPerPixel == _screen->format.bytesPerPixel);

	for (int i = 0; i < surface->h; i++)
		memcpy(_screen->getBasePtr(x, y + i), surface->getBasePtr(0, i), surface->w * surface->format.bytesPerPixel);
}

void GraphicsManager::blit(const Graphics::Surface *surface, const Common::Rect &srcRect, const Common::Rect &dstRect) {
	assert(surface->format.bytesPerPixel == _screen->format.bytesPerPixel);

	int width = MIN(srcRect.width(), dstRect.width());
	int height = MIN(srcRect.height(), dstRect.height());

	for (int i = 0; i < height; i++)
		memcpy(_screen->getBasePtr(dstRect.left, dstRect.top + i), surface->getBasePtr(srcRect.left, srcRect.top + i), width * surface->format.bytesPerPixel);
}

void GraphicsManager::fillRect(const Common::Rect &rect, uint32 color) {
	_screen->fillRect(rect, color);
}

byte *GraphicsManager::createDefaultPalette() const {
	Common::SeekableReadStream *stream = _vm->getBitmapStream(700);

	if (!stream)
		error("Couldn't find bitmap 700");

	stream->skip(14);

	if (stream->readUint16LE() != 8)
		error("Trying to load palette from non-8bpp image 700");

	stream->skip(16);

	uint32 colorsUsed = stream->readUint32LE();

	if (colorsUsed != 0 && colorsUsed != 256)
		error("Bitmap 700 is missing a full palette");

	stream->skip(4);
	byte *palette = new byte[256 * 3];
	byte *ptr = palette;

	for (uint32 i = 0; i < 256; i++) {
		ptr[2] = stream->readByte();
		ptr[1] = stream->readByte();
		ptr[0] = stream->readByte();
		stream->readByte();
		ptr += 3;
	}

	delete stream;

	// Make sure the first entry is black and the last is white
	palette[0 * 3]   = palette[0 * 3 + 1]   = palette[0 * 3 + 2]   = 0x00;
	palette[255 * 3] = palette[255 * 3 + 1] = palette[255 * 3 + 2] = 0xFF;

	return palette;
}

Graphics::Surface *GraphicsManager::remapPalettedFrame(const Graphics::Surface *frame, const byte *palette) {
	// This is pretty much the same as the Cinepak one
	// It seems to work for the one video I know that needs it (SWLOGO.BTV)
	// TODO: Merge some of this with getColor()

	byte palMap[256];
	const byte *screenPal = getDefaultPalette();

	for (int i = 0; i < 256; i++) {
		int r = palette[i * 3];
		int g = palette[i * 3 + 1];
		int b = palette[i * 3 + 2];

		int diff = 0x7FFFFFFF;
		byte result = 0;

		for (int j = 0; j < 256; j++) {
			int bDiff = b - (int)screenPal[j * 3 + 2];
			int curDiffB = diff - (bDiff * bDiff);

			if (curDiffB > 0) {
				int gDiff = g - (int)screenPal[j * 3 + 1];
				int curDiffG = curDiffB - (gDiff * gDiff);

				if (curDiffG > 0) {
					int rDiff = r - (int)screenPal[j * 3];
					int curDiffR = curDiffG - (rDiff * rDiff);

					if (curDiffR > 0) {
						diff -= curDiffR;
						result = j;

						if (diff == 0)
							break;
					}
				}
			}
		}

		palMap[i] = result;
	}

	Graphics::Surface *convertedSurface = new Graphics::Surface();
	convertedSurface->create(frame->w, frame->h, frame->format);

	for (int y = 0; y < frame->h; y++)
		for (int x = 0; x < frame->w; x++)
			*((byte *)convertedSurface->getBasePtr(x, y)) = palMap[*((byte *)frame->getBasePtr(x, y))];

	return convertedSurface;
}

} // End of namespace Buried
