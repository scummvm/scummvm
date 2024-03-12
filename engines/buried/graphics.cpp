/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/str-array.h"
#include "common/system.h"
#include "common/compression/unzip.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "graphics/wincursor.h"
#include "graphics/fonts/ttf.h"
#include "image/bmp.h"

#include "buried/buried.h"
#include "buried/graphics.h"
#include "buried/window.h"

namespace Buried {

GraphicsManager::GraphicsManager(BuriedEngine *vm) : _vm(vm) {
	_curCursor = kCursorNone;
	_mouseMoved = false;
	_needsErase = false;

	setCursor(kCursorArrow);
	CursorMan.showMouse(true);

	_screen = new Graphics::Surface();
	_screen->create(640, 480, g_system->getScreenFormat());

	if (_vm->isTrueColor()) {
		// No palette to deal with
		_palette = nullptr;
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

Graphics::Font *GraphicsManager::createFont(int size, bool bold) const {
	// MS Gothic for the Japanese version
	// Arial or Arial Bold for everything else
	if (_vm->getLanguage() == Common::JA_JPN)
		return createMSGothicFont(size, bold);

	return createArialFont(size, bold);
}

Graphics::Font *GraphicsManager::createArialFont(int size, bool bold) const {
	const char *defaultBaseName = bold ? "arialbd.ttf" : "arial.ttf";

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(defaultBaseName);

	// Map the heights needed to point sizes
	if (bold) {
		if (size != 20)
			error("Unhandled Arial Bold height %d", size);

		size = 12;
	} else {
		switch (size) {
		case 12:
		case 13:
			size = 7;
			break;
		case 14:
			size = 8;
			break;
		default:
			error("Unhandled Arial height %d", size);
		}
	}

	// TODO: Make the monochrome mode optional
	// Win3.1 obviously only had raster fonts, but BIT Win3.1 will render
	// with the TrueType font on Win7/Win8 (at least)
	// FIXME: The font is slightly off from the original... need to check. Sizes are right though!

	Graphics::Font *font;

	if (stream) {
		font = Graphics::loadTTFFont(*stream, size, Graphics::kTTFSizeModeCharacter, 96, 96, _vm->isTrueColor() ? Graphics::kTTFRenderModeLight : Graphics::kTTFRenderModeMonochrome);

		delete stream;
	} else {
		const char *fname;
		if (bold)
			fname = "LiberationSans-Bold.ttf";
		else
			fname = "LiberationSans-Regular.ttf";

		font = Graphics::loadTTFFontFromArchive(fname, size, Graphics::kTTFSizeModeCharacter, 96, 96, _vm->isTrueColor() ? Graphics::kTTFRenderModeLight : Graphics::kTTFRenderModeMonochrome);
	}

	if (!font)
		error("Failed to load Arial%s font", bold ? " Bold" : "");

	return font;
}

void GraphicsManager::toggleCursor(bool show) {
	CursorMan.showMouse(show);
}

Cursor GraphicsManager::setCursor(Cursor newCursor) {
	// Don't set the cursor again
	if (newCursor == _curCursor)
		return _curCursor;

	Cursor oldCursor = _curCursor;
	Graphics::Cursor *cursor = nullptr;
	Graphics::WinCursorGroup *cursorGroup = nullptr;

	if (newCursor == kCursorArrow) {
		cursor = Graphics::makeDefaultWinCursor();
	} else if (newCursor == kCursorWait) {
		cursor = Graphics::makeBusyWinCursor();
	} else {
		cursorGroup = _vm->getCursorGroup(newCursor);

		if (!cursorGroup)
			return kCursorNone;

		cursor = cursorGroup->cursors[0].cursor;
	}

	if (!cursor)
		error("Failed to find cursor %d", newCursor);

	CursorMan.replaceCursor(cursor);

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

Graphics::Surface *GraphicsManager::getBitmap(const Common::Path &fileName, bool required) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(fileName);

	if (!stream) {
		if (required)
			error("Could not find bitmap '%s'", fileName.toString(Common::Path::kNativeSeparator).c_str());
		return nullptr;
	}

	Graphics::Surface *surface = getBitmap(stream);
	if (!surface) {
		if (required)
			error("Failed to decode bitmap '%s'", fileName.toString(Common::Path::kNativeSeparator).c_str());
		return nullptr;
	}

	return surface;
}

Graphics::Surface *GraphicsManager::getBitmap(Common::SeekableReadStream *stream) {
	Image::BitmapDecoder decoder;
	if (!decoder.loadStream(*stream)) {
		delete stream;
		return nullptr;
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

void GraphicsManager::invalidateRect(const Common::Rect &rect, bool erase) {
	if (_dirtyRect.isEmpty())
		_dirtyRect = rect;
	else
		_dirtyRect.extend(rect);

	_needsErase |= erase;
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

	_needsErase = false;
}

void GraphicsManager::blit(const Graphics::Surface *surface, int x, int y) {
	assert(surface->format.bytesPerPixel == _screen->format.bytesPerPixel);

	for (int i = 0; i < surface->h; i++)
		memcpy(_screen->getBasePtr(x, y + i), surface->getBasePtr(0, i), surface->w * surface->format.bytesPerPixel);
}

void GraphicsManager::blit(const Graphics::Surface *surface, int x, int y, uint width, uint height) {
	assert(surface->format.bytesPerPixel == _screen->format.bytesPerPixel);

	for (uint i = 0; i < height; i++)
		memcpy(_screen->getBasePtr(x, y + i), surface->getBasePtr(0, i), width * surface->format.bytesPerPixel);
}

void GraphicsManager::blit(const Graphics::Surface *surface, const Common::Rect &srcRect, const Common::Rect &dstRect) {
	assert(surface->format.bytesPerPixel == _screen->format.bytesPerPixel);

	uint width = MIN(srcRect.width(), dstRect.width());
	uint height = MIN(srcRect.height(), dstRect.height());

	for (uint i = 0; i < height; i++)
		memcpy(_screen->getBasePtr(dstRect.left, dstRect.top + i), surface->getBasePtr(srcRect.left, srcRect.top + i), width * surface->format.bytesPerPixel);
}

void GraphicsManager::fillRect(const Common::Rect &rect, uint32 color) {
	_screen->fillRect(rect, color);
}

void GraphicsManager::keyBlit(Graphics::Surface *dst, int xDst, int yDst, int w, int h, const Graphics::Surface *src, uint xSrc, uint ySrc, uint32 transColor) {
	assert(dst->format.bytesPerPixel == src->format.bytesPerPixel);

	w = MIN<int>(src->w, w);
	h = MIN<int>(src->h, h);

	Common::Rect srcRect(xSrc, ySrc, xSrc + w, ySrc + h);
	Common::Rect dstRect(xDst, yDst, xDst + w, yDst + h);

	if (dst->clip(srcRect, dstRect))
		dst->copyRectToSurfaceWithKey(*src, dstRect.left, dstRect.top, srcRect, transColor);
}

void GraphicsManager::keyBlit(Graphics::Surface *dst, int xDst, int yDst, int w, int h, const Graphics::Surface *src, uint xSrc, uint ySrc, byte rTrans, byte gTrans, byte bTrans) {
	if (_vm->isTrueColor()) {
		keyBlit(dst, xDst, yDst, w, h, src, xSrc, ySrc, getColor(rTrans, gTrans, bTrans));
	} else {
		// Find the palette index of the color
		int paletteIndex = -1;
		for (int i = 0; i < 256; i++) {
			if (_palette[i * 3] == rTrans && _palette[i * 3 + 1] == gTrans && _palette[i * 3 + 2] == bTrans) {
				paletteIndex = i;
				break;
			}
		}

		assert(paletteIndex >= 0);

		keyBlit(dst, xDst, yDst, w, h, src, xSrc, ySrc, paletteIndex);
	}
}

void GraphicsManager::opaqueTransparentBlit(Graphics::Surface *dst, int xDst, int yDst, int w, int h, const Graphics::Surface *src, uint xSrc, uint ySrc, int opacityValue, byte rTrans, byte gTrans, byte bTrans) {
	if (_vm->isTrueColor()) {
		uint32 transColor = getColor(rTrans, gTrans, bTrans);

		for (int y = 0; y < h; y++) {
			if (y + yDst < dst->h && y + yDst >= 0) {
				for (int x = 0; x < w; x++) {
					if (x + xDst < dst->w && x + xDst >= 0) {
						uint32 srcColor;

						if (src->format.bytesPerPixel == 2)
							srcColor = *((const uint16 *)src->getBasePtr(x + xSrc, y + ySrc));
						else
							srcColor = *((const uint32 *)src->getBasePtr(x + xSrc, y + ySrc));

						if (srcColor == transColor)
							continue;

						int srcCycles, dstCycles;
						switch (opacityValue) {
						case 50:
							srcCycles = 1;
							dstCycles = 3;
							break;
						case 85:
							srcCycles = 17;
							dstCycles = 3;
							break;
						default:
							srcCycles = 1;
							dstCycles = 0;
							break;
						}

						byte rSrc, gSrc, bSrc;
						g_system->getScreenFormat().colorToRGB(srcColor, rSrc, gSrc, bSrc);

						uint32 dstColor;
						if (dst->format.bytesPerPixel == 2)
							dstColor = *((uint16 *)dst->getBasePtr(x + xDst, y + yDst));
						else
							dstColor = *((uint32 *)dst->getBasePtr(x + xDst, y + yDst));

						byte rDst, gDst, bDst;
						g_system->getScreenFormat().colorToRGB(dstColor, rDst, gDst, bDst);

						byte r = (((int)rSrc * srcCycles) + ((int)rDst * dstCycles)) / (srcCycles + dstCycles);
						byte g = (((int)gSrc * srcCycles) + ((int)gDst * dstCycles)) / (srcCycles + dstCycles);
						byte b = (((int)bSrc * srcCycles) + ((int)bDst * dstCycles)) / (srcCycles + dstCycles);
						uint32 color = g_system->getScreenFormat().RGBToColor(r, g, b);

						if (dst->format.bytesPerPixel == 2)
							*((uint16 *)dst->getBasePtr(x + xDst, y + yDst)) = color;
						else
							*((uint32 *)dst->getBasePtr(x + xDst, y + yDst)) = color;
					}
				}
			}
		}
	} else {
		keyBlit(dst, xDst, yDst, w, h, src, xSrc, ySrc, rTrans, gTrans, bTrans);
	}
}

bool GraphicsManager::checkPointAgainstMaskedBitmap(const Graphics::Surface *bitmap, int x, int y, const Common::Point &point, byte rTrans, byte gTrans, byte bTrans) {
	if (_vm->isTrueColor()) {
		uint32 transColor = getColor(rTrans, gTrans, bTrans);
		uint32 color;

		if (bitmap->format.bytesPerPixel == 2)
			color = *((const uint16 *)bitmap->getBasePtr(point.x - x, point.y - y));
		else
			color = *((const uint32 *)bitmap->getBasePtr(point.x - x, point.y - y));

		return transColor != color;
	} else {
		// Find the palette index of the color
		int paletteIndex = -1;
		for (int i = 0; i < 256; i++) {
			if (_palette[i * 3] == rTrans && _palette[i * 3 + 1] == gTrans && _palette[i * 3 + 2] == bTrans) {
				paletteIndex = i;
				break;
			}
		}

		assert(paletteIndex >= 0);

		return *((const byte *)bitmap->getBasePtr(point.x - x, point.y - y)) != paletteIndex;
	}
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

	for (int y = 0; y < frame->h; y++) {
		for (int x = 0; x < frame->w; x++)
			*((byte *)convertedSurface->getBasePtr(x, y)) = palMap[*((const byte *)frame->getBasePtr(x, y))];
	}

	return convertedSurface;
}

int GraphicsManager::computeHPushOffset(int speed) {
	switch (speed) {
	case 3:
		return 432;
	case 2:
		return 72;
	case 1:
		return 36;
	case 0:
		return 12;
	}

	return 432;
}

int GraphicsManager::computeVPushOffset(int speed) {
	switch (speed) {
	case 3:
		return 189;
	case 2:
		return 63;
	case 1:
		return 21;
	case 0:
		return 7;
	}

	return 189;
}

void GraphicsManager::crossBlit(Graphics::Surface *dst, int xDst, int yDst, uint w, uint h, const Graphics::Surface *src, uint xSrc, uint ySrc) {
	assert(dst->format.bytesPerPixel == src->format.bytesPerPixel);

	for (uint y = 0; y < h; y++)
		memcpy(dst->getBasePtr(xDst, yDst + y), src->getBasePtr(xSrc, ySrc + y), w * src->format.bytesPerPixel);
}

Graphics::Font *GraphicsManager::createMSGothicFont(int size, bool bold) const {
	switch (size) {
	case 10:
	case 11:
		size = 8;
		break;
	case 12:
		size = 9;
		break;
	case 20:
		size = 16;
		break;
	default:
		error("Unknown MS Gothic font size %d", size);
	}

	Graphics::Font *font;

	// Try to see if the user supplied a font
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember("msgothic.ttc");

	// TODO: Fake a bold version
	if (stream) {
		// Force monochrome, since the original uses the bitmap glyphs in the font
		font = Graphics::loadTTFFont(*stream, size, Graphics::kTTFSizeModeCharacter, 96, 96, Graphics::kTTFRenderModeMonochrome);
	} else {
		font = Graphics::loadTTFFontFromArchive("VL-Gothic-Regular.ttf", size, Graphics::kTTFSizeModeCharacter, 96, 96, Graphics::kTTFRenderModeMonochrome);
	}

	if (!font)
		error("Failed to load MS Gothic font");

	delete stream;
	return font;
}

void GraphicsManager::renderText(Graphics::Surface *dst, Graphics::Font *font, const Common::String &text, int x, int y, int w, int h, uint32 color, int lineHeight, TextAlign textAlign, bool centerVertically) {
	if (text.empty())
		return;

	// Convert to UTF-32 for drawing. Choose the codepage based on the language.
	Common::CodePage srcFormat = (_vm->getLanguage() == Common::JA_JPN) ? Common::kWindows932 : Common::kWindows1252;
	Common::U32String convString = text.decode(srcFormat);

	renderText(dst, font, convString, x, y, w, h, color, lineHeight, textAlign, centerVertically);
}

void GraphicsManager::renderText(Graphics::Surface *dst, Graphics::Font *font, const Common::U32String &text, int x, int y, int w, int h, uint32 color, int lineHeight, TextAlign textAlign, bool centerVertically) {
	if (text.empty())
		return;

	Common::U32StringArray lines;
	font->wordWrapText(text, w, lines);

	Graphics::TextAlign align = Graphics::kTextAlignLeft;
	switch (textAlign) {
	case kTextAlignLeft:
		align = Graphics::kTextAlignLeft;
		break;
	case kTextAlignCenter:
		align = Graphics::kTextAlignCenter;
		break;
	case kTextAlignRight:
		align = Graphics::kTextAlignRight;
		break;
	}

	if (centerVertically)
		y += (h - (lines.size() * lineHeight)) / 2;

	// Why is this needed? Dunno, but I guess Windows adds one row of space on the top
	y++;

	for (uint32 i = 0; i < lines.size(); i++) {
		font->drawString(dst, lines[i], x, y, w, color, align);
		y += lineHeight;
	}
}

void GraphicsManager::drawEllipse(const Common::Rect &rect, uint32 color) {
	// HACK: This just hardcodes the sizes of the rows of the ellipses
	// for the one thing in the game that needs it.

	static const int rows7[7] = { 7, 13, 15, 15, 15, 13, 7 };
	static const int rows10[10] = { 7, 11, 13, 15, 15, 15, 15, 13, 11, 7 };
	static const int rows12[12] = { 7, 11, 13, 13, 15, 15, 15, 15, 13, 13, 11, 7 };
	static const int rows15[15] = { 5, 9, 11, 13, 13, 15, 15, 15, 15, 15, 13, 13, 11, 9, 5 };

	const int *table = nullptr;
	switch (rect.height()) {
	case 7:
		table = rows7;
		break;
	case 10:
		table = rows10;
		break;
	case 12:
		table = rows12;
		break;
	case 15:
		table = rows15;
		break;
	}

	assert(table);

	for (int y = 0; y < rect.height(); y++) {
		int width = table[y];
		int x = rect.left + (rect.width() - width) / 2;
		_screen->hLine(x, y + rect.top, x + width, color);
	}
}

TempCursorChange::TempCursorChange(Cursor cursor) {
	_prevCursor = static_cast<BuriedEngine*>(g_engine)->_gfx->setCursor(cursor);
}

TempCursorChange::~TempCursorChange() {
	static_cast<BuriedEngine*>(g_engine)->_gfx->setCursor(_prevCursor);
}

} // End of namespace Buried
