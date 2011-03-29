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
 * $URL$
 * $Id$
 *
 */

#ifndef MOHAWK_GRAPHICS_H
#define MOHAWK_GRAPHICS_H

#include "mohawk/bitmap.h"

#include "common/file.h"
#include "common/hashmap.h"
#include "graphics/pict.h"

namespace Graphics {
	class JPEG;
}

namespace Mohawk {

class MohawkEngine;
class MohawkEngine_LivingBooks;
class MohawkBitmap;

class MohawkSurface {
public:
	MohawkSurface();
	MohawkSurface(Graphics::Surface *surface, byte *palette = NULL, int offsetX = 0, int offsetY = 0);
	~MohawkSurface();

	// getSurface() returns the surface in the current format
	// This will be the initial format unless convertToTrueColor() is called
	Graphics::Surface *getSurface() const { return _surface; }
	byte *getPalette() const { return _palette; }

	// Convert the 8bpp image to the current screen format
	// Does nothing if _surface is already >8bpp
	void convertToTrueColor();

	// Functions for OldMohawkBitmap offsets
	// They both default to 0
	int getOffsetX() const { return _offsetX; }
	int getOffsetY() const { return _offsetY; }
	void setOffsetX(int x) { _offsetX = x; }
	void setOffsetY(int y) { _offsetY = y; }

private:
	Graphics::Surface *_surface;
	byte *_palette;
	int _offsetX, _offsetY;
};

class GraphicsManager {
public:
	GraphicsManager();
	virtual ~GraphicsManager();

	// Free all surfaces in the cache
	void clearCache();

	void preloadImage(uint16 image);
	virtual void setPalette(uint16 id);
	void copyAnimImageToScreen(uint16 image, int left = 0, int top = 0);
	void copyAnimImageSectionToScreen(uint16 image, Common::Rect src, Common::Rect dest);
	void copyAnimSubImageToScreen(uint16 image, uint16 subimage, int left = 0, int top = 0);

	void getSubImageSize(uint16 image, uint16 subimage, uint16 &width, uint16 &height);

protected:
	void copyAnimImageSectionToScreen(MohawkSurface *image, Common::Rect src, Common::Rect dest);

	// findImage will search the cache to find the image.
	// If not found, it will call decodeImage to get a new one.
	MohawkSurface *findImage(uint16 id);

	// decodeImage will always return a new image.
	virtual MohawkSurface *decodeImage(uint16 id) = 0;
	virtual Common::Array<MohawkSurface *> decodeImages(uint16 id);

	virtual MohawkEngine *getVM() = 0;
	void addImageToCache(uint16 id, MohawkSurface *surface);

private:
	// An image cache that stores images until clearCache() is called
	Common::HashMap<uint16, MohawkSurface*> _cache;
	Common::HashMap<uint16, Common::Array<MohawkSurface*> > _subImageCache;
};

#ifdef ENABLE_MYST

class MystBitmap;
class MohawkEngine_Myst;

enum RectState {
	kRectEnabled,
	kRectDisabled,
	kRectUnreachable
};

class MystGraphics : public GraphicsManager {
public:
	MystGraphics(MohawkEngine_Myst*);
	~MystGraphics();

	void loadExternalPictureFile(uint16 stack);
	void copyImageSectionToScreen(uint16 image, Common::Rect src, Common::Rect dest);
	void copyImageSectionToBackBuffer(uint16 image, Common::Rect src, Common::Rect dest);
	void copyImageToScreen(uint16 image, Common::Rect dest);
	void copyImageToBackBuffer(uint16 image, Common::Rect dest);
	void copyBackBufferToScreen(Common::Rect r);
	void runTransition(uint16 type, Common::Rect rect, uint16 steps, uint16 delay);
	void drawRect(Common::Rect rect, RectState state);
	void drawLine(const Common::Point &p1, const Common::Point &p2, uint32 color);

protected:
	MohawkSurface *decodeImage(uint16 id);
	MohawkEngine *getVM() { return (MohawkEngine *)_vm; }

private:
	MohawkEngine_Myst *_vm;
	MystBitmap *_bmpDecoder;
	Graphics::PictDecoder *_pictDecoder;
	Graphics::JPEG *_jpegDecoder;

	struct PictureFile {
		uint32 pictureCount;
		struct PictureEntry {
			uint32 offset;
			uint32 size;
			uint16 id;
			uint16 type;
			uint16 width;
			uint16 height;
		} *entries;

		Common::File picFile;
	} _pictureFile;

	Graphics::Surface *_backBuffer;
	Graphics::PixelFormat _pixelFormat;
	Common::Rect _viewport;
};

#endif // ENABLE_MYST

#ifdef ENABLE_RIVEN

class MohawkEngine_Riven;

class RivenGraphics : public GraphicsManager {
public:
	RivenGraphics(MohawkEngine_Riven *vm);
	~RivenGraphics();

	void copyImageToScreen(uint16, uint32, uint32, uint32, uint32);
	void updateScreen(Common::Rect updateRect = Common::Rect(0, 0, 608, 392));
	bool _updatesEnabled;
	Common::Array<uint16> _activatedPLSTs;
	void drawPLST(uint16 x);
	void drawRect(Common::Rect rect, bool active);
	void drawImageRect(uint16 id, Common::Rect srcRect, Common::Rect dstRect);
	void drawExtrasImage(uint16 id, Common::Rect dstRect);

	// Water Effect
	void scheduleWaterEffect(uint16);
	void clearWaterEffects();
	bool runScheduledWaterEffects();

	// Transitions
	void scheduleTransition(uint16 id, Common::Rect rect = Common::Rect(0, 0, 608, 392));
	void runScheduledTransition();
	void fadeToBlack();

	// Inventory
	void showInventory();
	void hideInventory();

	// Credits
	void beginCredits();
	void updateCredits();
	uint getCurCreditsImage() { return _creditsImage; }

protected:
	MohawkSurface *decodeImage(uint16 id);
	MohawkEngine *getVM() { return (MohawkEngine *)_vm; }

private:
	MohawkEngine_Riven *_vm;
	MohawkBitmap *_bitmapDecoder;

	// Water Effects
	struct SFXERecord {
		// Record values
		uint16 frameCount;
		Common::Rect rect;
		uint16 speed;
		Common::Array<Common::SeekableReadStream*> frameScripts;

		// Cur frame
		uint16 curFrame;
		uint32 lastFrameTime;
	};
	Common::Array<SFXERecord> _waterEffects;

	// Transitions
	int16 _scheduledTransition;
	Common::Rect _transitionRect;

	// Inventory
	void clearInventoryArea();
	void drawInventoryImage(uint16 id, const Common::Rect *rect);
	bool _inventoryDrawn;

	// Screen Related
	Graphics::Surface *_mainScreen;
	bool _dirtyScreen;
	Graphics::PixelFormat _pixelFormat;
	void clearMainScreen();

	// Credits
	uint _creditsImage, _creditsPos;
};

#endif // ENABLE_RIVEN

class LBGraphics : public GraphicsManager {
public:
	LBGraphics(MohawkEngine_LivingBooks *vm, uint16 width, uint16 height);
	~LBGraphics();

	void setPalette(uint16 id);
	void copyOffsetAnimImageToScreen(uint16 image, int left = 0, int top = 0);
	bool imageIsTransparentAt(uint16 image, bool useOffsets, int x, int y);

protected:
	MohawkSurface *decodeImage(uint16 id);
	MohawkEngine *getVM() { return (MohawkEngine *)_vm; }

private:
	MohawkBitmap *_bmpDecoder;
	MohawkEngine_LivingBooks *_vm;
};

#ifdef ENABLE_CSTIME

class MohawkEngine_CSTime;

class CSTimeGraphics : public GraphicsManager {
public:
	CSTimeGraphics(MohawkEngine_CSTime *vm);
	~CSTimeGraphics();

	void drawRect(Common::Rect rect, byte color);

protected:
	MohawkSurface *decodeImage(uint16 id);
	Common::Array<MohawkSurface *> decodeImages(uint16 id);
	MohawkEngine *getVM() { return (MohawkEngine *)_vm; }

private:
	MohawkBitmap *_bmpDecoder;
	MohawkEngine_CSTime *_vm;
};

#endif

} // End of namespace Mohawk

#endif
