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

#include "sci/sci.h"
#include "sci/util.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/view.h"

namespace Sci {

GfxView::GfxView(ResourceManager *resMan, GfxScreen *screen, GfxPalette *palette, GuiResourceId resourceId)
	: _resMan(resMan), _screen(screen), _palette(palette), _resourceId(resourceId) {
	assert(resourceId != -1);
	_coordAdjuster = g_sci->_gfxCoordAdjuster;
	initData(resourceId);
}

GfxView::~GfxView() {
	// Iterate through the loops
	for (uint16 loopNum = 0; loopNum < _loopCount; loopNum++) {
		// and through the cells of each loop
		for (uint16 celNum = 0; celNum < _loop[loopNum].celCount; celNum++) {
			delete[] _loop[loopNum].cel[celNum].rawBitmap;
		}
		delete[] _loop[loopNum].cel;
	}
	delete[] _loop;

	_resMan->unlockResource(_resource);
}

static const byte EGAmappingStraight[SCI_VIEW_EGAMAPPING_SIZE] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

void GfxView::initData(GuiResourceId resourceId) {
	_resource = _resMan->findResource(ResourceId(kResourceTypeView, resourceId), true);
	if (!_resource) {
		error("view resource %d not found", resourceId);
	}
	_resourceData = _resource->data;
	_resourceSize = _resource->size;

	byte *celData, *loopData;
	uint16 celOffset;
	CelInfo *cel;
	uint16 celCount = 0;
	uint16 mirrorBits = 0;
	uint32 palOffset = 0;
	uint16 headerSize = 0;
	uint16 loopSize = 0, celSize = 0;
	int loopNo, celNo, EGAmapNr;
	byte seekEntry;
	bool isEGA = false;
	bool isCompressed = true;
	ViewType curViewType = _resMan->getViewType();

	_loopCount = 0;
	_embeddedPal = false;
	_EGAmapping = NULL;
	_isSci2Hires = false;

	// we adjust inside getCelRect for SCI0EARLY (that version didn't have the +1 when calculating bottom)
	adjustForSci0Early = getSciVersion() == SCI_VERSION_0_EARLY ? -1 : 0;

	// If we find an SCI1/SCI1.1 view (not amiga), we switch to that type for
	// EGA. This could get used to make view patches for EGA games, where the
	// new views include more colors. Users could manually adjust old views to
	// make them look better (like removing dithered colors that aren't caught
	// by our undithering or even improve the graphics overall).
	if (curViewType == kViewEga) {
		if (_resourceData[1] == 0x80) {
			curViewType = kViewVga;
		} else {
			if (READ_LE_UINT16(_resourceData + 4) == 1)
				curViewType = kViewVga11;
		}
	}

	switch (curViewType) {
	case kViewEga: // View-format SCI0 (and Amiga 16 colors)
		isEGA = true;
	case kViewAmiga: // View-format Amiga (32 colors)
	case kViewVga: // View-format SCI1
		// LoopCount:WORD MirrorMask:WORD Version:WORD PaletteOffset:WORD LoopOffset0:WORD LoopOffset1:WORD...

		_loopCount = _resourceData[0];
		// bit 0x8000 of _resourceData[1] means palette is set
		if (_resourceData[1] & 0x40)
			isCompressed = false;
		mirrorBits = READ_LE_UINT16(_resourceData + 2);
		palOffset = READ_LE_UINT16(_resourceData + 6);

		if (palOffset && palOffset != 0x100) {
			// Some SCI0/SCI01 games also have an offset set. It seems that it
			// points to a 16-byte mapping table but on those games using that
			// mapping will actually screw things up. On the other side: VGA
			// SCI1 games have this pointing to a VGA palette and EGA SCI1 games
			// have this pointing to a 8x16 byte mapping table that needs to get
			// applied then.
			if (!isEGA) {
				_palette->createFromData(&_resourceData[palOffset], _resourceSize - palOffset, &_viewPalette);
				_embeddedPal = true;
			} else {
				// Only use the EGA-mapping, when being SCI1
				if (getSciVersion() >= SCI_VERSION_1_EGA) {
					_EGAmapping = &_resourceData[palOffset];
					for (EGAmapNr = 0; EGAmapNr < SCI_VIEW_EGAMAPPING_COUNT; EGAmapNr++) {
						if (memcmp(_EGAmapping, EGAmappingStraight, SCI_VIEW_EGAMAPPING_SIZE) != 0)
							break;
						_EGAmapping += SCI_VIEW_EGAMAPPING_SIZE;
					}
					// If all mappings are "straight", then we actually ignore the mapping
					if (EGAmapNr == SCI_VIEW_EGAMAPPING_COUNT)
						_EGAmapping = NULL;
					else
						_EGAmapping = &_resourceData[palOffset];
				}
			}
		}

		_loop = new LoopInfo[_loopCount];
		for (loopNo = 0; loopNo < _loopCount; loopNo++) {
			loopData = _resourceData + READ_LE_UINT16(_resourceData + 8 + loopNo * 2);
			// CelCount:WORD Unknown:WORD CelOffset0:WORD CelOffset1:WORD...

			celCount = READ_LE_UINT16(loopData);
			_loop[loopNo].celCount = celCount;
			_loop[loopNo].mirrorFlag = mirrorBits & 1 ? true : false;
			mirrorBits >>= 1;

			// read cel info
			_loop[loopNo].cel = new CelInfo[celCount];
			for (celNo = 0; celNo < celCount; celNo++) {
				celOffset = READ_LE_UINT16(loopData + 4 + celNo * 2);
				celData = _resourceData + celOffset;

				// For VGA
				// Width:WORD Height:WORD DisplaceX:BYTE DisplaceY:BYTE ClearKey:BYTE Unknown:BYTE RLEData starts now directly
				// For EGA
				// Width:WORD Height:WORD DisplaceX:BYTE DisplaceY:BYTE ClearKey:BYTE EGAData starts now directly
				cel = &_loop[loopNo].cel[celNo];
				cel->scriptWidth = cel->width = READ_LE_UINT16(celData);
				cel->scriptHeight = cel->height = READ_LE_UINT16(celData + 2);
				cel->displaceX = (signed char)celData[4];
				cel->displaceY = celData[5];
				cel->clearKey = celData[6];
				if (isEGA) {
					cel->offsetEGA = celOffset + 7;
					cel->offsetRLE = 0;
					cel->offsetLiteral = 0;
				} else {
					cel->offsetEGA = 0;
					if (isCompressed) {
						cel->offsetRLE = celOffset + 8;
						cel->offsetLiteral = 0;
					} else {
						cel->offsetRLE = 0;
						cel->offsetLiteral = celOffset + 8;
					}
				}
				cel->rawBitmap = 0;
				if (_loop[loopNo].mirrorFlag)
					cel->displaceX = -cel->displaceX;
			}
		}
		break;

	case kViewVga11: // View-format SCI1.1+
		// HeaderSize:WORD LoopCount:BYTE Unknown:BYTE Version:WORD Unknown:WORD PaletteOffset:WORD
		headerSize = READ_SCI11ENDIAN_UINT16(_resourceData + 0) + 2; // headerSize is not part of the header, so it's added
		assert(headerSize >= 16);
		_loopCount = _resourceData[2];
		assert(_loopCount);
		_isSci2Hires = _resourceData[5] == 1 ? true : false;
		palOffset = READ_SCI11ENDIAN_UINT32(_resourceData + 8);
		// FIXME: After LoopCount there is another byte and its set for view 50
		// within Laura Bow 2 CD, check what it means.

		loopData = _resourceData + headerSize;
		loopSize = _resourceData[12];
		assert(loopSize >= 16);
		celSize = _resourceData[13];
		assert(celSize >= 32);

		if (palOffset) {
			_palette->createFromData(&_resourceData[palOffset], _resourceSize - palOffset, &_viewPalette);
			_embeddedPal = true;
		}

		_loop = new LoopInfo[_loopCount];
		for (loopNo = 0; loopNo < _loopCount; loopNo++) {
			loopData = _resourceData + headerSize + (loopNo * loopSize);

			seekEntry = loopData[0];
			if (seekEntry != 255) {
				if (seekEntry >= _loopCount)
					error("Bad loop-pointer in sci 1.1 view");
				_loop[loopNo].mirrorFlag = true;
				loopData = _resourceData + headerSize + (seekEntry * loopSize);
			} else {
				_loop[loopNo].mirrorFlag = false;
			}

			celCount = loopData[2];
			_loop[loopNo].celCount = celCount;

			celData = _resourceData + READ_SCI11ENDIAN_UINT32(loopData + 12);

			// read cel info
			_loop[loopNo].cel = new CelInfo[celCount];
			for (celNo = 0; celNo < celCount; celNo++) {
				cel = &_loop[loopNo].cel[celNo];
				cel->scriptWidth = cel->width = READ_SCI11ENDIAN_UINT16(celData);
				cel->scriptHeight = cel->height = READ_SCI11ENDIAN_UINT16(celData + 2);
				cel->displaceX = READ_SCI11ENDIAN_UINT16(celData + 4);
				cel->displaceY = READ_SCI11ENDIAN_UINT16(celData + 6);

				assert(cel->width && cel->height);

				cel->clearKey = celData[8];
				cel->offsetEGA = 0;
				cel->offsetRLE = READ_SCI11ENDIAN_UINT32(celData + 24);
				cel->offsetLiteral = READ_SCI11ENDIAN_UINT32(celData + 28);
				// GK1-hires content is actually uncompressed, we need to swap both so that we process it as such
				if ((cel->offsetRLE) && (!cel->offsetLiteral))
					SWAP(cel->offsetRLE, cel->offsetLiteral);

				cel->rawBitmap = 0;
				if (_loop[loopNo].mirrorFlag)
					cel->displaceX = -cel->displaceX;

				celData += celSize;
			}
		}
#ifdef ENABLE_SCI32
		// adjust width/height returned to scripts
		switch (getSciVersion()) {
		case SCI_VERSION_2:
			if (_isSci2Hires) {
				for (loopNo = 0; loopNo < _loopCount; loopNo++) {
					for (celNo = 0; celNo < _loop[loopNo].celCount; celNo++) {
						_screen->adjustBackUpscaledCoordinates(_loop[loopNo].cel[celNo].scriptWidth, _loop[loopNo].cel[celNo].scriptHeight);
					}
				}
			}
			break;

		case SCI_VERSION_2_1:
			for (loopNo = 0; loopNo < _loopCount; loopNo++) {
				for (celNo = 0; celNo < _loop[loopNo].celCount; celNo++) {
					_coordAdjuster->fromDisplayToScript(_loop[loopNo].cel[celNo].scriptHeight, _loop[loopNo].cel[celNo].scriptWidth);
				}
			}
		default:
			break;
		}
#endif
		break;

	default:
		error("ViewType was not detected, can't continue");
	}
}

GuiResourceId GfxView::getResourceId() const {
	return _resourceId;
}

int16 GfxView::getWidth(int16 loopNo, int16 celNo) const {
	return _loopCount ? getCelInfo(loopNo, celNo)->width : 0;
}

int16 GfxView::getHeight(int16 loopNo, int16 celNo) const {
	return _loopCount ? getCelInfo(loopNo, celNo)->height : 0;
}

const CelInfo *GfxView::getCelInfo(int16 loopNo, int16 celNo) const {
	assert(_loopCount);
	loopNo = CLIP<int16>(loopNo, 0, _loopCount - 1);
	celNo = CLIP<int16>(celNo, 0, _loop[loopNo].celCount - 1);
	return &_loop[loopNo].cel[celNo];
}

uint16 GfxView::getCelCount(int16 loopNo) const {
	assert(_loopCount);
	loopNo = CLIP<int16>(loopNo, 0, _loopCount - 1);
	return _loop[loopNo].celCount;
}

Palette *GfxView::getPalette() {
	return _embeddedPal ? &_viewPalette : NULL;
}

bool GfxView::isSci2Hires() {
	return _isSci2Hires;
}

void GfxView::getCelRect(int16 loopNo, int16 celNo, int16 x, int16 y, int16 z, Common::Rect &outRect) const {
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);
	outRect.left = x + celInfo->displaceX - (celInfo->width >> 1);
	outRect.right = outRect.left + celInfo->width;
	outRect.bottom = y + celInfo->displaceY - z + 1 + adjustForSci0Early;
	outRect.top = outRect.bottom - celInfo->height;
}

void GfxView::getCelScaledRect(int16 loopNo, int16 celNo, int16 x, int16 y, int16 z, int16 scaleX, int16 scaleY, Common::Rect &outRect) const {
	int16 scaledDisplaceX, scaledDisplaceY;
	int16 scaledWidth, scaledHeight;
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);

	// Scaling displaceX/Y, Width/Height
	scaledDisplaceX = (celInfo->displaceX * scaleX) >> 7;
	scaledDisplaceY = (celInfo->displaceY * scaleY) >> 7;
	scaledWidth = (celInfo->width * scaleX) >> 7;
	scaledHeight = (celInfo->height * scaleY) >> 7;
	scaledWidth = CLIP<int16>(scaledWidth, 0, _screen->getWidth());
	scaledHeight = CLIP<int16>(scaledHeight, 0, _screen->getHeight());

	outRect.left = x + scaledDisplaceX - (scaledWidth >> 1);
	outRect.right = outRect.left + scaledWidth;
	outRect.bottom = y + scaledDisplaceY - z + 1;
	outRect.top = outRect.bottom - scaledHeight;
}

void GfxView::unpackCel(int16 loopNo, int16 celNo, byte *outPtr, uint32 pixelCount) {
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);
	byte *rlePtr;
	byte *literalPtr;
	uint32 pixelNo = 0, runLength;
	byte pixel;

	if (celInfo->offsetEGA) {
		// decompression for EGA views
		literalPtr = _resourceData + _loop[loopNo].cel[celNo].offsetEGA;
		while (pixelNo < pixelCount) {
			pixel = *literalPtr++;
			runLength = pixel >> 4;
			memset(outPtr + pixelNo, pixel & 0x0F, MIN<uint32>(runLength, pixelCount - pixelNo));
			pixelNo += runLength;
		}
	} else {
		// We fill the buffer with transparent pixels, so that we can later skip
		//  over pixels to automatically have them transparent
		// Also some RLE compressed cels are possibly ending with the last
		// non-transparent pixel (is this even possible with the current code?)
		memset(outPtr, _loop[loopNo].cel[celNo].clearKey, pixelCount);

		rlePtr = _resourceData + celInfo->offsetRLE;
		if (!celInfo->offsetLiteral) { // no additional literal data
			if (_resMan->isAmiga32color()) {
				// decompression for amiga views
				while (pixelNo < pixelCount) {
					pixel = *rlePtr++;
					if (pixel & 0x07) { // fill with color
						runLength = pixel & 0x07;
						pixel = pixel >> 3;
						while (runLength-- && pixelNo < pixelCount) {
							outPtr[pixelNo++] = pixel;
						}
					} else { // fill with transparent
						runLength = pixel >> 3;
						pixelNo += runLength;
					}
				}
			} else {
				// decompression for data that has just one combined stream
				while (pixelNo < pixelCount) {
					pixel = *rlePtr++;
					runLength = pixel & 0x3F;
					switch (pixel & 0xC0) {
					case 0: // copy bytes as-is
						while (runLength-- && pixelNo < pixelCount)
							outPtr[pixelNo++] = *rlePtr++;
						break;
					case 0x80: // fill with color
						memset(outPtr + pixelNo, *rlePtr++, MIN<uint32>(runLength, pixelCount - pixelNo));
						pixelNo += runLength;
						break;
					case 0xC0: // fill with transparent
						pixelNo += runLength;
						break;
					}
				}
			}
		} else {
			literalPtr = _resourceData + celInfo->offsetLiteral;
			if (celInfo->offsetRLE) {
				if (g_sci->getPlatform() == Common::kPlatformMacintosh && getSciVersion() >= SCI_VERSION_1_1) {
					// compression for SCI1.1+ Mac
					while (pixelNo < pixelCount) {
						uint32 pixelLine = pixelNo;
						runLength = *rlePtr++;
						pixelNo += runLength;
						runLength = *rlePtr++;
						while (runLength-- && pixelNo < pixelCount) {
							outPtr[pixelNo] = *literalPtr++;
							if (outPtr[pixelNo] == 255)
								outPtr[pixelNo] = 0;
							pixelNo++;
						}
						pixelNo = pixelLine + celInfo->width;
					}
				} else {
					// decompression for data that has separate rle and literal streams
					while (pixelNo < pixelCount) {
						pixel = *rlePtr++;
						runLength = pixel & 0x3F;
						switch (pixel & 0xC0) {
						case 0: // copy bytes as-is
							while (runLength-- && pixelNo < pixelCount)
								outPtr[pixelNo++] = *literalPtr++;
							break;
						case 0x80: // fill with color
							memset(outPtr + pixelNo, *literalPtr++, MIN<uint32>(runLength, pixelCount - pixelNo));
							pixelNo += runLength;
							break;
						case 0xC0: // fill with transparent
							pixelNo += runLength;
							break;
						}
					}
				}
			} else {
				// literal stream only, so no compression
				memcpy(outPtr, literalPtr, pixelCount);
				pixelNo = pixelCount;
			}
		}
	}
}

const byte *GfxView::getBitmap(int16 loopNo, int16 celNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount -1);
	celNo = CLIP<int16>(celNo, 0, _loop[loopNo].celCount - 1);
	if (_loop[loopNo].cel[celNo].rawBitmap)
		return _loop[loopNo].cel[celNo].rawBitmap;

	uint16 width = _loop[loopNo].cel[celNo].width;
	uint16 height = _loop[loopNo].cel[celNo].height;
	// allocating memory to store cel's bitmap
	int pixelCount = width * height;
	_loop[loopNo].cel[celNo].rawBitmap = new byte[pixelCount];
	byte *pBitmap = _loop[loopNo].cel[celNo].rawBitmap;

	// unpack the actual cel bitmap data
	unpackCel(loopNo, celNo, pBitmap, pixelCount);

	if (!_resMan->isVGA()) {
		unditherBitmap(pBitmap, width, height, _loop[loopNo].cel[celNo].clearKey);
	}

	// mirroring the cel if needed
	if (_loop[loopNo].mirrorFlag) {
		for (int i = 0; i < height; i++, pBitmap += width)
			for (int j = 0; j < width / 2; j++)
				SWAP(pBitmap[j], pBitmap[width - j - 1]);
	}
	return _loop[loopNo].cel[celNo].rawBitmap;
}

/**
 * Called after unpacking an EGA cel, this will try to undither (parts) of the
 * cel if the dithering in here matches dithering used by the current picture.
 */
void GfxView::unditherBitmap(byte *bitmapPtr, int16 width, int16 height, byte clearKey) {
	int16 *unditherMemorial = _screen->unditherGetMemorial();

	// It makes no sense to go further, if no memorial data from current picture
	// is available
	if (!unditherMemorial)
		return;

	// Makes no sense to process bitmaps that are 3 pixels wide or less
	if (width <= 3)
		return;

	// If EGA mapping is used for this view, dont do undithering as well
	if (_EGAmapping)
		return;

	// Walk through the bitmap and remember all combinations of colors
	int16 bitmapMemorial[SCI_SCREEN_UNDITHERMEMORIAL_SIZE];
	byte *curPtr;
	byte color1, color2;
	int16 y, x;

	memset(&bitmapMemorial, 0, sizeof(bitmapMemorial));

	// Count all seemingly dithered pixel-combinations as soon as at least 4
	// pixels are adjacent
	curPtr = bitmapPtr;
	for (y = 0; y < height; y++) {
		color1 = curPtr[0]; color2 = (curPtr[1] << 4) | curPtr[2];
		curPtr += 3;
		for (x = 3; x < width; x++) {
			color1 = (color1 << 4) | (color2 >> 4);
			color2 = (color2 << 4) | *curPtr++;
			if (color1 == color2)
				bitmapMemorial[color1]++;
		}
	}

	// Now compare both memorial tables to find out matching
	// dithering-combinations
	bool unditherTable[SCI_SCREEN_UNDITHERMEMORIAL_SIZE];
	byte color, unditherCount = 0;
	memset(&unditherTable, false, sizeof(unditherTable));
	for (color = 0; color < 255; color++) {
		if ((bitmapMemorial[color] > 5) && (unditherMemorial[color] > 200)) {
			// match found, check if colorKey is contained -> if so, we ignore
			// of course
			color1 = color & 0x0F; color2 = color >> 4;
			if ((color1 != clearKey) && (color2 != clearKey) && (color1 != color2)) {
				// so set this and the reversed color-combination for undithering
				unditherTable[color] = true;
				unditherTable[(color1 << 4) | color2] = true;
				unditherCount++;
			}
		}
	}

	// Nothing found to undither -> exit straight away
	if (!unditherCount)
		return;

	// We now need to replace color-combinations
	curPtr = bitmapPtr;
	for (y = 0; y < height; y++) {
		color = *curPtr;
		for (x = 1; x < width; x++) {
			color = (color << 4) | curPtr[1];
			if (unditherTable[color]) {
				// Some color with black? Turn colors around, otherwise it won't
				// be the right color at all.
				if ((color & 0xF0) == 0)
					color = (color << 4) | (color >> 4);
				curPtr[0] = color; curPtr[1] = color;
			}
			curPtr++;
		}
		curPtr++;
	}
}

void GfxView::draw(const Common::Rect &rect, const Common::Rect &clipRect, const Common::Rect &clipRectTranslated,
			int16 loopNo, int16 celNo, byte priority, uint16 EGAmappingNr, bool upscaledHires) {
	const Palette *palette = _embeddedPal ? &_viewPalette : &_palette->_sysPalette;
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);
	const byte *bitmap = getBitmap(loopNo, celNo);
	const int16 celHeight = celInfo->height;
	const int16 celWidth = celInfo->width;
	const byte clearKey = celInfo->clearKey;
	const byte drawMask = (priority == 255) ? GFX_SCREEN_MASK_VISUAL : GFX_SCREEN_MASK_VISUAL|GFX_SCREEN_MASK_PRIORITY;
	int x, y;

	if (_embeddedPal) {
		// Merge view palette in...
		_palette->set(&_viewPalette, false);
	}

	const int16 width = MIN(clipRect.width(), celWidth);
	const int16 height = MIN(clipRect.height(), celHeight);

	bitmap += (clipRect.top - rect.top) * celWidth + (clipRect.left - rect.left);

	if (!_EGAmapping) {
		for (y = 0; y < height; y++, bitmap += celWidth) {
			for (x = 0; x < width; x++) {
				const byte color = bitmap[x];
				if (color != clearKey) {
					const int x2 = clipRectTranslated.left + x;
					const int y2 = clipRectTranslated.top + y;
					if (!upscaledHires) {
						if (priority >= _screen->getPriority(x2, y2))
							_screen->putPixel(x2, y2, drawMask, palette->mapping[color], priority, 0);
					} else {
						// UpscaledHires means view is hires and is supposed to
						// get drawn onto lowres screen.
						// FIXME(?): we can't read priority directly with the
						// hires coordinates. may not be needed at all in kq6
						_screen->putPixelOnDisplay(x2, y2, palette->mapping[color]);
					}
				}
			}
		}
	} else {
		byte *EGAmapping = _EGAmapping + (EGAmappingNr * SCI_VIEW_EGAMAPPING_SIZE);
		for (y = 0; y < height; y++, bitmap += celWidth) {
			for (x = 0; x < width; x++) {
				const byte color = EGAmapping[bitmap[x]];
				const int x2 = clipRectTranslated.left + x;
				const int y2 = clipRectTranslated.top + y;
				if (color != clearKey && priority >= _screen->getPriority(x2, y2))
					_screen->putPixel(x2, y2, drawMask, color, priority, 0);
			}
		}
	}
}

/**
 * We don't fully follow sierra sci here, I did the scaling algo myself and it
 * is definitely not pixel-perfect with the one sierra is using. It shouldn't
 * matter because the scaled cel rect is definitely the same as in sierra sci.
 */
void GfxView::drawScaled(const Common::Rect &rect, const Common::Rect &clipRect, const Common::Rect &clipRectTranslated,
			int16 loopNo, int16 celNo, byte priority, int16 scaleX, int16 scaleY) {
	const Palette *palette = _embeddedPal ? &_viewPalette : &_palette->_sysPalette;
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);
	const byte *bitmap = getBitmap(loopNo, celNo);
	const int16 celHeight = celInfo->height;
	const int16 celWidth = celInfo->width;
	const byte clearKey = celInfo->clearKey;
	const byte drawMask = (priority == 255) ? GFX_SCREEN_MASK_VISUAL : GFX_SCREEN_MASK_VISUAL|GFX_SCREEN_MASK_PRIORITY;
	uint16 scalingX[640];
	uint16 scalingY[480];
	int16 scaledWidth, scaledHeight;
	int pixelNo, scaledPixel, scaledPixelNo, prevScaledPixelNo;

	if (_embeddedPal) {
		// Merge view palette in...
		_palette->set(&_viewPalette, false);
	}

	scaledWidth = (celInfo->width * scaleX) >> 7;
	scaledHeight = (celInfo->height * scaleY) >> 7;
	scaledWidth = CLIP<int16>(scaledWidth, 0, _screen->getWidth());
	scaledHeight = CLIP<int16>(scaledHeight, 0, _screen->getHeight());

	// Do we really need to do this?!
	//memset(scalingX, 0, sizeof(scalingX));
	//memset(scalingY, 0, sizeof(scalingY));

	// Create height scaling table
	pixelNo = 0;
	scaledPixel = scaledPixelNo = prevScaledPixelNo = 0;
	while (pixelNo < celHeight) {
		scaledPixelNo = scaledPixel >> 7;
		assert(scaledPixelNo < ARRAYSIZE(scalingY));
		for (; prevScaledPixelNo <= scaledPixelNo; prevScaledPixelNo++)
			scalingY[prevScaledPixelNo] = pixelNo;
		pixelNo++;
		scaledPixel += scaleY;
	}
	pixelNo--;
	scaledPixelNo++;
	for (; scaledPixelNo < scaledHeight; scaledPixelNo++)
		scalingY[scaledPixelNo] = pixelNo;

	// Create width scaling table
	pixelNo = 0;
	scaledPixel = scaledPixelNo = prevScaledPixelNo = 0;
	while (pixelNo < celWidth) {
		scaledPixelNo = scaledPixel >> 7;
		assert(scaledPixelNo < ARRAYSIZE(scalingX));
		for (; prevScaledPixelNo <= scaledPixelNo; prevScaledPixelNo++)
			scalingX[prevScaledPixelNo] = pixelNo;
		pixelNo++;
		scaledPixel += scaleX;
	}
	pixelNo--;
	scaledPixelNo++;
	for (; scaledPixelNo < scaledWidth; scaledPixelNo++)
		scalingX[scaledPixelNo] = pixelNo;

	scaledWidth = MIN(clipRect.width(), scaledWidth);
	scaledHeight = MIN(clipRect.height(), scaledHeight);

	const int16 offsetY = clipRect.top - rect.top;
	const int16 offsetX = clipRect.left - rect.left;

	// Happens in SQ6, first room
	if (offsetX < 0 || offsetY < 0)
		return;

	assert(scaledHeight + offsetY <= ARRAYSIZE(scalingY));
	assert(scaledWidth + offsetX <= ARRAYSIZE(scalingX));
	for (int y = 0; y < scaledHeight; y++) {
		for (int x = 0; x < scaledWidth; x++) {
			const byte color = bitmap[scalingY[y + offsetY] * celWidth + scalingX[x + offsetX]];
			const int x2 = clipRectTranslated.left + x;
			const int y2 = clipRectTranslated.top + y;
			if (color != clearKey && priority >= _screen->getPriority(x2, y2)) {
				_screen->putPixel(x2, y2, drawMask, palette->mapping[color], priority, 0);
			}
		}
	}
}

} // End of namespace Sci
