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
 */

#include "startrek/common.h"
#include "startrek/console.h"
#include "startrek/graphics.h"
#include "startrek/resource.h"

#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/rendermode.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

namespace StarTrek {

Graphics::Graphics(StarTrekEngine *vm) : _vm(vm), _egaMode(false) {
	_font = nullptr;
	_egaData = nullptr;
	_lutData = nullptr;

	for (int i = 0; i < 32; ++i) {
		_sprites[i] = nullptr;
		_pushedSprites[i] = nullptr;
	}

	_screenRect = Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT);

	if (ConfMan.hasKey("render_mode"))
		_egaMode = (Common::parseRenderMode(ConfMan.get("render_mode").c_str()) == Common::kRenderEGA) && (_vm->getGameType() != GType_STJR) && !(_vm->getFeatures() & GF_DEMO);

	if (_vm->getGameType() == GType_ST25 && _vm->getPlatform() == Common::kPlatformDOS)
		_font = new Font(_vm);

	_numSprites = 0;
	_pushedNumSprites = -1;

	_palData = new byte[256 * 3];
	_lutData = new byte[256 * 3];

	memset(_palData, 0, 256 * 3);
	memset(_lutData, 0, 256 * 3);

	_paletteFadeLevel = 0;
	_lockedMousePos = Common::Point(-1, -1);
	_backgroundImage = nullptr;
}

Graphics::~Graphics() {
	delete[] _egaData;
	delete[] _palData;
	delete[] _lutData;
	delete _backgroundImage;

	delete _font;
}

void Graphics::setBackgroundImage(Common::String imageName) {
	delete _backgroundImage;
	_backgroundImage = new Bitmap(_vm->_resource->loadBitmapFile(imageName));
}

void Graphics::drawBitmapToBackground(const Common::Rect &origRect, const Common::Rect &drawRect, Bitmap *bitmap) {
	byte *dest = _backgroundImage->pixels + drawRect.top * SCREEN_WIDTH + drawRect.left;
	byte *src = bitmap->pixels + (drawRect.left - origRect.left)
	            + (drawRect.top - origRect.top) * bitmap->width;

	for (int y = drawRect.top; y < drawRect.bottom; y++) {
		for (int x = drawRect.left; x < drawRect.right; x++) {
			byte b = *src;

			if (b != 0)
				*dest = b;

			src++;
			dest++;
		}

		src += bitmap->width - drawRect.width();
		dest += SCREEN_WIDTH - drawRect.width();
	}
}

void Graphics::fillBackgroundRect(const Common::Rect &rect, byte color) {
	byte *dest = _backgroundImage->pixels + rect.top * SCREEN_WIDTH + rect.left;
	for (int y = rect.top; y < rect.bottom; y++) {
		memset(dest, color, rect.width());
		dest += SCREEN_WIDTH;
	}
}

byte *Graphics::getBackgroundPixels() {
	return _backgroundImage->pixels;
}

byte *Graphics::lockScreenPixels() {
	return (byte *)_vm->_system->lockScreen()->getPixels();
}

void Graphics::unlockScreenPixels() {
	_vm->_system->unlockScreen();
}

void Graphics::clearScreenAndPriBuffer() {
	Common::fill(_priData, _priData + sizeof(_priData), 0);

	::Graphics::Surface *surface = _vm->_system->lockScreen();
	surface->fillRect(_screenRect, 0);
	_vm->_system->unlockScreen();
	_vm->_system->updateScreen();
	_vm->_system->delayMillis(10);
}

void Graphics::loadPalette(const Common::String &paletteName) {
	// Set the palette from a PAL file
	Common::String palFile = paletteName + ".PAL";
	Common::String lutFile = paletteName + ".LUT";

	Common::MemoryReadStreamEndian *palStream = _vm->_resource->loadFile(palFile.c_str());
	palStream->read(_palData, 256 * 3);
	delete palStream;

	// Load LUT file
	Common::MemoryReadStreamEndian *lutStream = _vm->_resource->loadFile(lutFile.c_str());
	lutStream->read(_lutData, 256);
	delete lutStream;
}

void Graphics::copyRectBetweenBitmaps(Bitmap *destBitmap, int destX, int destY, Bitmap *srcBitmap, int srcX, int srcY, int width, int height) {
	byte *src = srcBitmap->pixels + srcX + srcY * srcBitmap->width;
	byte *dest = destBitmap->pixels + destX + destY * destBitmap->width;

	for (int y = 0; y < height; y++) {
		memcpy(dest, src, width);
		src += srcBitmap->width;
		dest += destBitmap->width;
	}
}

void Graphics::fadeinScreen() {
	while (_paletteFadeLevel <= 100) {
		TrekEvent event;
		_vm->waitForNextTick();

		setPaletteFadeLevel(_palData, _paletteFadeLevel);
		_paletteFadeLevel += 10;
	}

	_paletteFadeLevel = 100;
}

void Graphics::fadeoutScreen() {
	while (_paletteFadeLevel >= 0) {
		TrekEvent event;
		_vm->waitForNextTick();

		setPaletteFadeLevel(_palData, _paletteFadeLevel);
		_paletteFadeLevel -= 10;
	}

	_paletteFadeLevel = 0;
}

void Graphics::setPaletteFadeLevel(byte *palData, int fadeLevel) {
	byte palBuffer[256 * 3];

	int multiplier = (fadeLevel << 8) / 100;

	for (uint16 i = 0; i < 256 * 3; i++) {
		palBuffer[i] = (palData[i] * multiplier) >> 8;

		// Expand color components
		if (_vm->getPlatform() == Common::kPlatformDOS || _vm->getPlatform() == Common::kPlatformMacintosh)
			palBuffer[i] <<= 2;
	}

	_vm->_system->getPaletteManager()->setPalette(palBuffer, 0, 256);

	// FIXME: this isn't supposed to flush changes to graphics, only palettes.
	// Might not matter...
	_vm->_system->updateScreen();
	_vm->_system->delayMillis(10);
}

void Graphics::incPaletteFadeLevel() {
	if (_paletteFadeLevel < 100) {
		_paletteFadeLevel += 10;
		setPaletteFadeLevel(_palData, _paletteFadeLevel);
	}
}

void Graphics::decPaletteFadeLevel() {
	if (_paletteFadeLevel > 0) {
		_paletteFadeLevel -= 10;
		setPaletteFadeLevel(_palData, _paletteFadeLevel);
	}
}


void Graphics::loadPri(const Common::String &priFile) {
	Common::MemoryReadStream *priStream = _vm->_resource->loadFile(priFile + ".pri");
	priStream->read(_priData, SCREEN_WIDTH * SCREEN_HEIGHT / 2);
	delete priStream;
}

void Graphics::clearPri() {
	memset(_priData, 0, sizeof(_priData));
}

void Graphics::setPri(byte val) {
	memset(_priData, val, sizeof(_priData));
}

byte Graphics::getPriValue(int x, int y) {
	assert(_screenRect.contains(x, y));

	int priOffset = y * SCREEN_WIDTH + x;
	byte b = _priData[priOffset / 2];
	if ((priOffset % 2) == 1)
		return b & 0xf;
	else
		return b >> 4;
}

Common::Point Graphics::getMousePos() {
	return _vm->_system->getEventManager()->getMousePos();
}

void Graphics::setMouseBitmap(Common::String bitmapName) {
	Bitmap *bitmap = new Bitmap(_vm->_resource->loadBitmapFile(bitmapName));

	CursorMan.pushCursor(
		bitmap->pixels,
		bitmap->width,
		bitmap->height,
		bitmap->xoffset,
		bitmap->yoffset,
		0
	);

	delete bitmap;
}

void Graphics::popMouseBitmap() {
	CursorMan.popCursor();
}

void Graphics::toggleMouse(bool visible) {
	CursorMan.showMouse(visible);
}

void Graphics::lockMousePosition(int16 x, int16 y) {
	_lockedMousePos = Common::Point(x, y);
}

void Graphics::unlockMousePosition() {
	_lockedMousePos = Common::Point(-1, -1);
}

void Graphics::warpMouse(int16 x, int16 y) {
	_vm->_system->warpMouse(x, y);
}

void Graphics::drawSprite(const Sprite &sprite, ::Graphics::Surface *surface) {
	int left = sprite.drawX;
	int top = sprite.drawY;
	int right = left + sprite.bitmap->width;
	int bottom = top + sprite.bitmap->height;
	drawSprite(sprite, surface, Common::Rect(left, top, right, bottom));
}

// rect is the portion of the sprite to update. It must be entirely contained within the
// sprite's actual, full rectangle.
void Graphics::drawSprite(const Sprite &sprite, ::Graphics::Surface *surface, const Common::Rect &rect, int rectLeft, int rectTop) {
	Common::Rect spriteRect = Common::Rect(sprite.drawX, sprite.drawY,
	        sprite.drawX + sprite.bitmap->width, sprite.drawY + sprite.bitmap->height);

	assert(_screenRect.contains(rect));
	assert(spriteRect.contains(rect));

	byte *dest = (byte *)surface->getPixels() + (rect.top - rectTop) * SCREEN_WIDTH + (rect.left - rectLeft);

	switch (sprite.drawMode) {
	case 0: { // Normal sprite
		byte *src = sprite.bitmap->pixels + (rect.left - sprite.drawX)
		            + (rect.top - sprite.drawY) * sprite.bitmap->width;

		int priOffset = rect.top * SCREEN_WIDTH + rect.left;

		for (int y = rect.top; y < rect.bottom; y++) {
			for (int x = rect.left; x < rect.right; x++) {
				byte priByte = _priData[priOffset / 2];
				byte bgPriority;
				if ((priOffset % 2) == 1)
					bgPriority = priByte & 0xf;
				else
					bgPriority = priByte >> 4;
				priOffset++;

				byte b = *src++;
				if (b == 0 || sprite.drawPriority < bgPriority) {
					dest++;
					continue;
				}
				*dest++ = b;
			}

			src       += sprite.bitmap->width - rect.width();
			dest      += SCREEN_WIDTH - rect.width();
			priOffset += SCREEN_WIDTH - rect.width();
		}
		break;
	}

	case 1: // Invisible
		break;

	case 2: { // Normal sprite with darkened background for "transparent" pixels (and no priority)
		byte *src = sprite.bitmap->pixels + (rect.left - sprite.drawX)
		            + (rect.top - sprite.drawY) * sprite.bitmap->width;

		for (int y = rect.top; y < rect.bottom; y++) {
			for (int x = rect.left; x < rect.right; x++) {
				byte b = *src;

				if (b == 0) // Transparent (darken the pixel)
					*dest = _lutData[*dest];
				else // Solid color
					*dest = b;

				src++;
				dest++;
			}

			src += sprite.bitmap->width - rect.width();
			dest += SCREEN_WIDTH - rect.width();
		}

		break;
	}

	case 3: { // Text
		// The sprite's "bitmap" is not actually a bitmap, but instead the list of
		// characters to display.

		// Units of this rect are "characters" instead of pixels. This contains all
		// characters to be drawn.
		Common::Rect rectangle1;

		rectangle1.left   = (rect.left   - sprite.drawX) / 8;
		rectangle1.top    = (rect.top    - sprite.drawY) / 8;
		rectangle1.right  = (rect.right  - sprite.drawX + 7) / 8;
		rectangle1.bottom = (rect.bottom - sprite.drawY + 7) / 8;

		int drawWidth = rectangle1.width();
		int drawHeight = rectangle1.height();

		dest = (byte *)surface->getPixels() + sprite.drawY * SCREEN_WIDTH + sprite.drawX
		       + rectangle1.top * 8 * SCREEN_WIDTH + rectangle1.left * 8;

		byte *src = sprite.bitmap->pixels + rectangle1.top * sprite.bitmap->width / 8 + rectangle1.left;

		for (int y = 0; y < drawHeight; y++) {
			for (int x = 0; x < drawWidth; x++) {
				byte c = *src;

				int textColor;
				if (c >= 0x10 && c <= 0x1A) // Border characters
					textColor = 0xb3;
				else
					textColor = sprite.textColor;

				byte *fontData = _font->getCharData(c);

				for (int i = 0; i < 8; i++) {
					for (int j = 0; j < 8; j++) {
						byte b = *fontData;

						if (b == 0) // Transparent: use lookup table to darken this pixel
							*dest = _lutData[*dest];
						else if (b == 0x78) // Inner part of character
							*dest = textColor;
						else // Outline of character
							*dest = b;

						fontData++;
						dest++;
					}
					dest += SCREEN_WIDTH - 8;
				}

				dest -= (SCREEN_WIDTH * 8 - 8);
				src++;
			}

			src += (sprite.bitmap->width / 8) - drawWidth;
			dest += (SCREEN_WIDTH * 8) - drawWidth * 8;

		}

		break;
	}

	default:
		error("drawSprite: draw mode %d invalid", sprite.drawMode);
		break;
	}
}

/**
 * Compare 2 sprites for the purpose of sorting them by layer before drawing.
 * FIXME: Original returned an int, not a bool. This may affect the stability of the sort...
 */
bool compareSpritesByLayer(Sprite *s1, Sprite *s2) {
	if (s1->drawPriority != s2->drawPriority)
		return s1->drawPriority < s2->drawPriority;
	if (s1->drawPriority2 != s2->drawPriority2)
		return s1->drawPriority2 < s2->drawPriority2;
	if (s1->pos.y != s2->pos.y)
		return s1->pos.y < s2->pos.y;
	return s1->pos.x < s2->pos.x;
}

void Graphics::drawAllSprites(bool updateScreenFlag) {
	// TODO: different video modes?

	if (_numSprites != 0) {
		// Sort sprites by layer
		Common::sort(_sprites, _sprites + _numSprites, &compareSpritesByLayer);

		// Update sprite rectangles
		for (int i = 0; i < _numSprites; i++) {
			Sprite *spr = _sprites[i];
			Common::Rect rect;

			rect.left   = spr->pos.x - spr->bitmap->xoffset;
			rect.top    = spr->pos.y - spr->bitmap->yoffset;
			rect.right  = rect.left + spr->bitmap->width;
			rect.bottom = rect.top + spr->bitmap->height;

			spr->drawX = rect.left;
			spr->drawY = rect.top;

			spr->drawRect = rect.findIntersectingRect(_screenRect);

			if (!spr->drawRect.isEmpty()) { // At least partly on-screen
				if (spr->lastDrawRect.left < spr->lastDrawRect.right) {
					// If the sprite's position is close to where it was last time it was
					// drawn, combine the two rectangles and redraw that whole section.
					// Otherwise, redraw the old position and current position separately.
					rect = spr->drawRect.findIntersectingRect(spr->lastDrawRect);

					if (rect.isEmpty())
						spr->rect2Valid = 0;
					else {
						spr->rectangle2 = spr->drawRect;
						spr->rectangle2.extend(spr->lastDrawRect);
						spr->rect2Valid = 1;
					}
				} else {
					spr->rectangle2 = spr->drawRect;
					spr->rect2Valid = 1;
				}

				spr->isOnScreen = 1;
			} else { // Off-screen
				spr->rect2Valid = 0;
				spr->isOnScreen = 0;
			}
		}

		// Determine what portions of the screen need to be updated
		Common::Rect dirtyRects[MAX_SPRITES * 2];
		int numDirtyRects = 0;

		for (int i = 0; i < _numSprites; i++) {
			Sprite *spr = _sprites[i];

			if (spr->bitmapChanged) {
				if (spr->isOnScreen) {
					if (spr->rect2Valid) {
						dirtyRects[numDirtyRects++] = spr->rectangle2;
					} else {
						dirtyRects[numDirtyRects++] = spr->drawRect;
						dirtyRects[numDirtyRects++] = spr->lastDrawRect;
					}
				} else {
					dirtyRects[numDirtyRects++] = spr->lastDrawRect;
				}
			}
		}

		// Redraw the background on every dirty rectangle
		const ::Graphics::PixelFormat format = ::Graphics::PixelFormat::createFormatCLUT8();
		::Graphics::Surface surface;
		surface.create(SCREEN_WIDTH, SCREEN_HEIGHT, format);

		for (int i = 0; i < numDirtyRects; i++) {
			Common::Rect &r = dirtyRects[i];
			if (r.width() == 0 || r.height() == 0 || r.left < 0 || r.top < 0)
				continue;

			int offset = r.top * SCREEN_WIDTH + r.left;
			surface.copyRectToSurface(_backgroundImage->pixels + offset, SCREEN_WIDTH, r.left, r.top, r.width(), r.height());
		}

		// For each sprite, merge the rectangles that overlap with it and redraw the sprite.
		for (int i = 0; i < _numSprites; i++) {
			Sprite *spr = _sprites[i];

			if (!spr->field16 && spr->isOnScreen) {
				bool mustRedrawSprite = false;
				Common::Rect rect2;

				for (int j = 0; j < numDirtyRects; j++) {
					Common::Rect rect1 = spr->drawRect.findIntersectingRect(dirtyRects[j]);

					if (rect1.width() != 0 && rect1.height() != 0 && rect1.left >= 0 && rect1.top >= 0) {
						if (mustRedrawSprite)
							rect2.extend(rect1);
						else
							rect2 = rect1;
						mustRedrawSprite = true;
					}
				}

				if (mustRedrawSprite)
					drawSprite(*spr, &surface, rect2);
			}

			spr->field16 = false;
			spr->bitmapChanged = false;
			spr->lastDrawRect = spr->drawRect;
		}

		// Copy dirty rects to screen
		for (int j = 0; j < numDirtyRects; j++) {
			Common::Rect &r = dirtyRects[j];
			if (r.width() == 0 || r.height() == 0 || r.left < 0 || r.top < 0)
				continue;

			int offset = r.left + r.top * SCREEN_WIDTH;
			_vm->_system->copyRectToScreen((byte *)surface.getPixels() + offset, SCREEN_WIDTH, r.left, r.top, r.width(), r.height());
		}

		surface.free();
	}

	if (updateScreenFlag)
		this->updateScreen();
}

void Graphics::drawAllSpritesInRectToSurface(const Common::Rect &rect, ::Graphics::Surface *surface) {
	surface->copyFrom(*_vm->_system->lockScreen());
	_vm->_system->unlockScreen();

	for (int i = 0; i < _numSprites; i++) {
		Sprite *sprite = _sprites[i];
		if (!sprite->isOnScreen)
			continue;

		Common::Rect intersect = rect.findIntersectingRect(sprite->drawRect);
		if (!intersect.isEmpty())
			drawSprite(*sprite, surface, intersect);
	}
}

void Graphics::forceDrawAllSprites(bool updateScreenFlag) {
	for (int i = 0; i < _numSprites; i++)
		_sprites[i]->bitmapChanged = true;
	drawAllSprites(updateScreenFlag);
}

void Graphics::updateScreen() {
	if (_lockedMousePos.x != -1)
		_vm->_system->warpMouse(_lockedMousePos.x, _lockedMousePos.y);

	_vm->_system->updateScreen();
	_vm->_system->delayMillis(10);
}

Sprite *Graphics::getSpriteAt(int16 x, int16 y) {
	for (int i = _numSprites - 1; i >= 0; i--) {
		Sprite *sprite = _sprites[i];

		if (sprite->drawMode == 1) // Invisible
			continue;

		if (sprite->drawRect.contains(Common::Point(x, y))) {
			if (sprite->drawMode == 2 || sprite->drawMode == 3) // Button or text
				return sprite;

			// For draw mode 0 only, check that we're not clicking on a transparent part.
			int16 relX = x - sprite->drawX;
			int16 relY = y - sprite->drawY;
			byte pixel = sprite->bitmap->pixels[relY * sprite->bitmap->width + relX];
			if (pixel != 0)
				return sprite;
		}
	}

	return nullptr;
}

void Graphics::addSprite(Sprite *sprite) {
	if (_numSprites >= MAX_SPRITES)
		error("addSprite: too many sprites");

	// Initialize some fields
	sprite->drawMode = 0;
	sprite->field8 = "";
	sprite->field16 = false;
	sprite->bitmapChanged = true; // FIXME (delete this later?)

	sprite->lastDrawRect.top = -1;
	sprite->lastDrawRect.left = -1;
	sprite->lastDrawRect.bottom = -2;
	sprite->lastDrawRect.right = -2;

	_sprites[_numSprites++] = sprite;
}

void Graphics::delSprite(Sprite *sprite) {
	for (int i = 0; i < _numSprites; i++) {
		if (sprite != _sprites[i])
			continue;

		_numSprites--;
		_sprites[i] = _sprites[_numSprites];
		return;
	}

	error("delSprite: sprite not in list");
}

void Graphics::pushSprites() {
	if (_pushedNumSprites != -1)
		error("Tried to push sprites more than once");
	_pushedNumSprites = _numSprites;
	memcpy(_pushedSprites, _sprites, sizeof(_sprites));

	_numSprites = 0;
}

void Graphics::popSprites() {
	if (_pushedNumSprites == -1)
		error("Tried to pop sprites without a prior push");
	_numSprites = _pushedNumSprites;
	memcpy(_sprites, _pushedSprites, sizeof(_sprites));

	_pushedNumSprites = -1;
}

byte *Graphics::getFontGfx(char c) {
	return _font->getCharData(c & 0xff);
}

void Graphics::copyBackgroundScreen() {
	_vm->_system->copyRectToScreen(
		_backgroundImage->pixels,
		_backgroundImage->width,
		_backgroundImage->xoffset,
		_backgroundImage->yoffset,
		_backgroundImage->width,
		_backgroundImage->height
	);
}

void Graphics::loadEGAData(const char *filename) {
	// Load EGA palette data
	if (!_egaMode)
		return;

	if (!_egaData)
		_egaData = new byte[256];

	Common::MemoryReadStreamEndian *egaStream = _vm->_resource->loadFile(filename);
	egaStream->read(_egaData, 256);
	delete egaStream;
}

void Graphics::drawBackgroundImage(const char *filename) {
	// Draw an stjr BGD image (palette built-in)

	Common::MemoryReadStreamEndian *imageStream = _vm->_resource->loadFile(filename);
	byte *palette = new byte[256 * 3];
	imageStream->read(palette, 256 * 3);

	// Expand color components
	for (uint16 i = 0; i < 256 * 3; i++)
		palette[i] <<= 2;

	uint16 xoffset = imageStream->readUint16();
	uint16 yoffset = imageStream->readUint16();
	uint16 width = imageStream->readUint16();
	uint16 height = imageStream->readUint16();

	byte *pixels = new byte[width * height];
	imageStream->read(pixels, width * height);
	delete imageStream;

	_vm->_system->getPaletteManager()->setPalette(palette, 0, 256);
	_vm->_system->copyRectToScreen(pixels, width, xoffset, yoffset, width, height);
	//_vm->_system->updateScreen();
	//_vm->_system->delayMillis(10);

	delete[] pixels;
	delete[] palette;
}

} // End of namespace StarTrek
