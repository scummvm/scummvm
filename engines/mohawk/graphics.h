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
#include "mohawk/livingbooks.h"
#include "mohawk/myst_jpeg.h"
#include "mohawk/myst_pict.h"

#include "common/file.h"

namespace Mohawk {

class MohawkEngine_Myst;
class MohawkEngine_Riven;
class MohawkBitmap;
class MystBitmap;

enum {
	kRivenMainCursor = 3000,
	kRivenPelletCursor = 5000
};

// 803-805 are animated, one large bmp which is in chunks
// Other cursors (200, 300, 400, 500, 600, 700) are not the same in each stack
enum {
	kDefaultMystCursor = 100,				// The default hand
	kWhitePageCursor = 800,					// Holding a white page
	kRedPageCursor = 801,					// Holding a red page
	kBluePageCursor = 802,					// Holding a blue page
	// kDroppingWhitePageAnimCursor = 803,
	// kDroppingRedPageAnimCursor = 804,
	// kDroppingBluePageAnimCursor = 805,
	kNewMatchCursor = 900,					// Match that has not yet been lit
	kLitMatchCursor = 901,					// Match that's burning
	kDeadMatchCursor = 902,					// Match that's been extinguished
	kKeyCursor = 903, 						// Key in Lighthouse in Stoneship
	kRotateClockwiseCursor = 904, 			// Rotate gear clockwise (boiler on Myst)
	kRotateCounterClockwiseCursor = 905,	// Rotate gear counter clockwise (boiler on Myst)
	kMystZipModeCursor = 999				// Zip Mode cursor
};

// A simple struct to hold necessary image info
class ImageData {
public:
	ImageData() : _surface(0), _palette(0) {}
	ImageData(Graphics::Surface *surface, byte *palette = NULL) : _surface(surface), _palette(palette) {}
	~ImageData() {
		if (_palette)
			free(_palette);
		if (_surface) {
			_surface->free();
			delete _surface;
		}
	}

	// getSurface() will convert to the current screen format, if it's not already
	// in that format. Makes it easy to support both 8bpp and 24bpp images.
	Graphics::Surface *getSurface();

	// These are still public in case the 8bpp surface needs to be accessed
	Graphics::Surface *_surface;
	byte *_palette;
};

class MystGraphics {
public:
	MystGraphics(MohawkEngine_Myst*);
	~MystGraphics();
	
	void loadExternalPictureFile(uint16 stack);
	void copyImageSectionToScreen(uint16 image, Common::Rect src, Common::Rect dest);
	void copyImageToScreen(uint16 image, Common::Rect dest);
	void showCursor(void);
	void hideCursor(void);
	void changeCursor(uint16);
	
	void drawRect(Common::Rect rect, bool active);
private:
	MohawkEngine_Myst *_vm;
	MystBitmap *_bmpDecoder;
	MystPICT *_pictDecoder;
	MystJPEG *_jpegDecoder;
	Graphics::PixelFormat _pixelFormat;
	
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
};

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

class RivenGraphics {
public:
	RivenGraphics(MohawkEngine_Riven *vm);
	~RivenGraphics();
	
	void copyImageToScreen(uint16, uint32, uint32, uint32, uint32);
	void updateScreen();
	bool _updatesEnabled;
	void changeCursor(uint16);
	Common::Array<uint16> _activatedPLSTs;
	void drawPLST(uint16 x);
	void drawRect(Common::Rect rect, bool active);

	// Water Effect
	void scheduleWaterEffect(uint16);
	void clearWaterEffects();
	bool runScheduledWaterEffects();
	
	// Transitions
	void scheduleTransition(uint16 id, Common::Rect rect = Common::Rect(0, 0, 608, 392));
	void runScheduledTransition();
	
	// Inventory
	void showInventory();
	void hideInventory();

private:
	MohawkEngine_Riven *_vm;
	MohawkBitmap *_bitmapDecoder;

	// Water Effects
	Common::Array<SFXERecord> _waterEffects;
	
	// Transitions
	int16 _scheduledTransition;
	Common::Rect _transitionRect;
	
	// Inventory
	void clearInventoryArea();
	void drawInventoryImage(uint16 id, Common::Rect rect);
	bool _inventoryDrawn;
	
	// Screen Related
	Graphics::Surface *_mainScreen;
	bool _dirtyScreen;
	Graphics::PixelFormat _pixelFormat;
	byte findBlackIndex();
};

class LBGraphics {
public:
	LBGraphics(MohawkEngine_LivingBooks *vm);
	~LBGraphics();

	void copyImageToScreen(uint16 image, uint16 left = 0, uint16 top = 0);
	void setPalette(uint16 id);

private:
	MohawkBitmap *_bmpDecoder;
	MohawkEngine_LivingBooks *_vm;
	byte *_palette;
};

} // End of namespace Mohawk

#endif
