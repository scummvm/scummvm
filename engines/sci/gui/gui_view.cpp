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
#include "sci/engine/state.h"
#include "sci/gui/gui_gfx.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_palette.h"
#include "sci/gui/gui_view.h"

namespace Sci {

SciGuiView::SciGuiView(ResourceManager *resMan, SciGuiScreen *screen, SciGuiPalette *palette, GuiResourceId resourceId)
	: _resMan(resMan), _screen(screen), _palette(palette), _resourceId(resourceId) {
	assert(resourceId != -1);
	initData(resourceId);
}

SciGuiView::~SciGuiView() {
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

void SciGuiView::initData(GuiResourceId resourceId) {
	_resource = _resMan->findResource(ResourceId(kResourceTypeView, resourceId), true);
	if (!_resource) {
		error("view resource %d not found", resourceId);
	}
	_resourceData = _resource->data;

	byte *celData, *loopData;
	uint16 celOffset;
	sciViewCelInfo *cel;
	uint16 celCount = 0;
	uint16 mirrorBits = 0;
	uint16 palOffset = 0;
	uint16 headerSize = 0;
	uint16 loopSize = 0, celSize = 0;
	int loopNo, celNo, EGAmapNr;
	byte seekEntry;
	bool IsEGA = false;

	_loopCount = 0;
	_embeddedPal = false;
	_EGAmapping = NULL;

	switch (_resMan->getViewType()) {
	case kViewEga: // View-format SCI0 (and Amiga 16 colors)
		IsEGA = true;
	case kViewAmiga: // View-format Amiga (32 colors)
	case kViewVga: // View-format SCI1
		// LoopCount:WORD MirrorMask:WORD Version:WORD PaletteOffset:WORD LoopOffset0:WORD LoopOffset1:WORD...
		
		// bit 0x8000 of _resourceData[1] means palette is set
		_loopCount = _resourceData[0];
		mirrorBits = READ_LE_UINT16(_resourceData + 2);
		palOffset = READ_LE_UINT16(_resourceData + 6);

		if (palOffset && palOffset != 0x100) {
			// Some SCI0/SCI01 games also have an offset set. It seems that it points to a 16-byte mapping table
			//  but on those games using that mapping will actually screw things up.
			// On the other side: vga sci1 games have this pointing to a VGA palette
			//  and ega sci1 games have this pointing to a 8x16 byte mapping table that needs to get applied then
			if (!IsEGA) {
				_palette->createFromData(&_resourceData[palOffset], &_viewPalette);
				_embeddedPal = true;
			} else {
				// Only use the EGA-mapping, when being SCI1
				if (getSciVersion() >= SCI_VERSION_1_EGA) {
					_EGAmapping = &_resourceData[palOffset];
					for (EGAmapNr = 0; EGAmapNr < SCI_VIEW_EGAMAPPING_COUNT; EGAmapNr++) {
						if (memcmp(_EGAmapping, EGAmappingStraight, SCI_VIEW_EGAMAPPING_SIZE)!=0)
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

		_loop = new sciViewLoopInfo[_loopCount];
		for (loopNo = 0; loopNo < _loopCount; loopNo++) {
			loopData = _resourceData + READ_LE_UINT16(_resourceData + 8 + loopNo * 2);
			// CelCount:WORD Unknown:WORD CelOffset0:WORD CelOffset1:WORD...

			celCount = READ_LE_UINT16(loopData);
			_loop[loopNo].celCount = celCount;
			_loop[loopNo].mirrorFlag = mirrorBits & 1 ? true : false;
			mirrorBits >>= 1;

			// read cel info
			_loop[loopNo].cel = new sciViewCelInfo[celCount];
			for (celNo = 0; celNo < celCount; celNo++) {
				celOffset = READ_LE_UINT16(loopData + 4 + celNo * 2);
				celData = _resourceData + celOffset;

				// For VGA
				// Width:WORD Height:WORD DisplaceX:BYTE DisplaceY:BYTE ClearKey:BYTE Unknown:BYTE RLEData starts now directly
				// For EGA
				// Width:WORD Height:WORD DisplaceX:BYTE DisplaceY:BYTE ClearKey:BYTE EGAData starts now directly
				cel = &_loop[loopNo].cel[celNo];
				cel->width = READ_LE_UINT16(celData);
				cel->height = READ_LE_UINT16(celData + 2);
				cel->displaceX = celData[4];
				cel->displaceY = celData[5];
				cel->clearKey = celData[6];
				if (IsEGA) {
					cel->offsetEGA = celOffset + 7;
					cel->offsetRLE = 0;
				} else {
					cel->offsetEGA = 0;
					cel->offsetRLE = celOffset + 8;
				}
				cel->offsetLiteral = 0;
				cel->rawBitmap = 0;
				if (_loop[loopNo].mirrorFlag)
					cel->displaceX = -cel->displaceX;
			}
		}
		break;

	case kViewVga11: // View-format SCI1.1
		// LoopCount:WORD MirrorMask:WORD Version:WORD PaletteOffset:WORD LoopOffset0:WORD LoopOffset1:WORD...
		// HeaderSize:WORD LoopCount:BYTE Unknown:BYTE Version:WORD Unknown:WORD PaletteOffset:WORD
		headerSize = READ_LE_UINT16(_resourceData + 0);
		_loopCount = _resourceData[2];
		palOffset = READ_LE_UINT16(_resourceData + 8);
		// FIXME: After LoopCount there is another byte and its set for view 50 within Laura Bow 2 CD, check what it means

		loopData = _resourceData + headerSize;
		loopSize = _resourceData[12];
		celSize = _resourceData[13];

		if (palOffset) {
			_palette->createFromData(&_resourceData[palOffset], &_viewPalette);
			_embeddedPal = true;
		}

		_loop = new sciViewLoopInfo[_loopCount];
		for (loopNo = 0; loopNo < _loopCount; loopNo++) {
			loopData = _resourceData + headerSize + (loopNo * loopSize);

			seekEntry = loopData[2];
			if (seekEntry != 255) {
				if (seekEntry >= _loopCount)
					error("Bad loop-pointer in sci 1.1 view");
				_loop[loopNo].mirrorFlag = true;
				loopData = _resourceData + headerSize + (seekEntry * loopSize);
			} else {
				_loop[loopNo].mirrorFlag = false;
			}

			celCount = loopData[4];
			_loop[loopNo].celCount = celCount;

			celData = _resourceData + READ_LE_UINT16(loopData + 14);

			// read cel info
			_loop[loopNo].cel = new sciViewCelInfo[celCount];
			for (celNo = 0; celNo < celCount; celNo++) {
				cel = &_loop[loopNo].cel[celNo];
				cel->width = READ_LE_UINT16(celData);
				cel->height = READ_LE_UINT16(celData + 2);
				cel->displaceX = READ_LE_UINT16(celData + 4);
				cel->displaceY = READ_LE_UINT16(celData + 6);
				cel->clearKey = celData[8];
				cel->offsetEGA = 0;
				cel->offsetRLE = READ_LE_UINT16(celData + 24);
				cel->offsetLiteral = READ_LE_UINT16(celData + 28);
				cel->rawBitmap = 0;
				if (_loop[loopNo].mirrorFlag)
					cel->displaceX = -cel->displaceX;

				celData += celSize;
			}
		}
		break;

	default:
		error("ViewType was not detected, can't continue");
	}
}

GuiResourceId SciGuiView::getResourceId() {
	return _resourceId;
}

int16 SciGuiView::getWidth(GuiViewLoopNo loopNo, GuiViewCelNo celNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount - 1);
	celNo = CLIP<int16>(celNo, 0, _loop[loopNo].celCount - 1);
	return _loopCount ? _loop[loopNo].cel[celNo].width : 0;
}

int16 SciGuiView::getHeight(GuiViewLoopNo loopNo, GuiViewCelNo celNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount -1);
	celNo = CLIP<int16>(celNo, 0, _loop[loopNo].celCount - 1);
	return _loopCount ? _loop[loopNo].cel[celNo].height : 0;
}

sciViewCelInfo *SciGuiView::getCelInfo(GuiViewLoopNo loopNo, GuiViewCelNo celNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount - 1);
	celNo = CLIP<int16>(celNo, 0, _loop[loopNo].celCount - 1);
	return _loopCount ? &_loop[loopNo].cel[celNo] : NULL;
}

sciViewLoopInfo *SciGuiView::getLoopInfo(GuiViewLoopNo loopNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount - 1);
	return _loopCount ? &_loop[loopNo] : NULL;
}

void SciGuiView::getCelRect(GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 x, int16 y, int16 z, Common::Rect *outRect) {
	sciViewCelInfo *celInfo = getCelInfo(loopNo, celNo);
	if (celInfo) {
		outRect->left = x + celInfo->displaceX - (celInfo->width >> 1);
		outRect->right = outRect->left + celInfo->width;
		outRect->bottom = y + celInfo->displaceY - z + 1;
		outRect->top = outRect->bottom - celInfo->height;
	}
}

void SciGuiView::unpackCel(GuiViewLoopNo loopNo, GuiViewCelNo celNo, byte *outPtr, uint16 pixelCount) {
	sciViewCelInfo *celInfo = getCelInfo(loopNo, celNo);
	byte *rlePtr;
	byte *literalPtr;
	uint16 pixelNo = 0, runLength;
	byte pixel;

	if (celInfo->offsetEGA) {
		// decompression for EGA views
		literalPtr = _resourceData + _loop[loopNo].cel[celNo].offsetEGA;
		while (pixelNo < pixelCount) {
			pixel = *literalPtr++;
			runLength = pixel >> 4;
			memset(outPtr + pixelNo, pixel & 0x0F, MIN<uint16>(runLength, pixelCount - pixelNo));
			pixelNo += runLength;
		}
		return;
	}

	rlePtr = _resourceData + celInfo->offsetRLE;
	if (!celInfo->offsetLiteral) { // no additional literal data
		if (_resMan->getViewType() == kViewAmiga) {
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
			return;
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
					memset(outPtr + pixelNo, *rlePtr++, MIN<uint16>(runLength, pixelCount - pixelNo));
					pixelNo += runLength;
					break;
				case 0xC0: // fill with transparent
					pixelNo += runLength;
					break;
				}
			}
			return;
		}
	} else {
		// decompression for data that has separate rle and literal streams
		literalPtr = _resourceData + celInfo->offsetLiteral;
		while (pixelNo < pixelCount) {
			pixel = *rlePtr++;
			runLength = pixel & 0x3F;
			switch (pixel & 0xC0) {
			case 0: // copy bytes as-is
				while (runLength-- && pixelNo < pixelCount)
					outPtr[pixelNo++] = *literalPtr++;
				break;
			case 0x80: // fill with color
				memset(outPtr + pixelNo, *literalPtr++, MIN<uint16>(runLength, pixelCount - pixelNo));
				pixelNo += runLength;
				break;
			case 0xC0: // fill with transparent
				pixelNo += runLength;
				break;
			}
		}
		return;
	}
	error("Unable to decompress view");
}

byte *SciGuiView::getBitmap(GuiViewLoopNo loopNo, GuiViewCelNo celNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount -1);
	celNo = CLIP<int16>(celNo, 0, _loop[loopNo].celCount - 1);
	if (_loop[loopNo].cel[celNo].rawBitmap)
		return _loop[loopNo].cel[celNo].rawBitmap;

	uint16 width = _loop[loopNo].cel[celNo].width;
	uint16 height = _loop[loopNo].cel[celNo].height;
	// allocating memory to store cel's bitmap
	assert(width * height <= 64000);
	uint16 pixelCount = width * height;
	_loop[loopNo].cel[celNo].rawBitmap = new byte[pixelCount];
	byte *pBitmap = _loop[loopNo].cel[celNo].rawBitmap;

	// Some RLE compressed cels end with the last non-transparent pixel, thats why we fill it up here
	//  FIXME: change this to fill the remaining bytes within unpackCel()
	memset(pBitmap, _loop[loopNo].cel[celNo].clearKey, pixelCount);
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

// Called after unpacking an EGA cel, this will try to undither (parts) of the cel if the dithering in here
//  matches dithering used by the current picture
void SciGuiView::unditherBitmap(byte *bitmapPtr, int16 width, int16 height, byte clearKey) {
	int16 *unditherMemorial = _screen->unditherGetMemorial();

	// It makes no sense to go further, if no memorial data from current picture is available
	if (!unditherMemorial)
		return;

	// Makes no sense to process bitmaps that are 3 pixels wide or less
	if (width <= 3) return;

	// If EGA mapping is used for this view, dont do undithering as well
	if (_EGAmapping)
		return;

	// Walk through the bitmap and remember all combinations of colors
	int16 bitmapMemorial[SCI_SCREEN_UNDITHERMEMORIAL_SIZE];
	byte *curPtr;
	byte color1, color2;
	int16 y, x;

	memset(&bitmapMemorial, 0, sizeof(bitmapMemorial));

	// Count all seemingly dithered pixel-combinations as soon as at least 4 pixels are adjacent
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

	// Now compare both memorial tables to find out matching dithering-combinations
	bool unditherTable[SCI_SCREEN_UNDITHERMEMORIAL_SIZE];
	byte color, unditherCount = 0;
	memset(&unditherTable, false, sizeof(unditherTable));
	for (color = 0; color < 255; color++) {
		if ((bitmapMemorial[color] > 5) && (unditherMemorial[color] > 200)) {
			// match found, check if colorKey is contained -> if so, we ignore of course
			color1 = color & 0x0F; color2 = color >> 4;
			if ((color1 != clearKey) && (color2 != clearKey) && (color1 != color2)) {
				// so set this and the reversed color-combination for undithering
				unditherTable[color] = true; unditherTable[(color1 << 4) | color2] = true;
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
				// some color with black? turn colors around otherwise it wont be the right color at all
				if ((color & 0xF0)==0)
					color = (color << 4) | (color >> 4);
				curPtr[0] = color; curPtr[1] = color;
			}
			curPtr++;
		}
		curPtr++;
	}
}

void SciGuiView::draw(Common::Rect rect, Common::Rect clipRect, Common::Rect clipRectTranslated, GuiViewLoopNo loopNo, GuiViewCelNo celNo, byte priority, uint16 EGAmappingNr, int16 origHeight) {
	GuiPalette *palette = _embeddedPal ? &_viewPalette : &_palette->_sysPalette;
	sciViewCelInfo *celInfo = getCelInfo(loopNo, celNo);
	byte *bitmap = getBitmap(loopNo, celNo);
	int16 celHeight = celInfo->height, celWidth = celInfo->width;
	int16 width, height;
	byte clearKey = celInfo->clearKey;
	byte color;
	byte drawMask = priority == 255 ? SCI_SCREEN_MASK_VISUAL : SCI_SCREEN_MASK_VISUAL|SCI_SCREEN_MASK_PRIORITY;
	int x, y;

	if (_embeddedPal) {
		// Merge view palette in...
		_palette->set(&_viewPalette, 1);
	}

	width = MIN(clipRect.width(), celWidth);
	height = MIN(clipRect.height(), celHeight);

	bitmap += (clipRect.top - rect.top) * celWidth + (clipRect.left - rect.left);

	if (!_EGAmapping) {
		for (y = clipRectTranslated.top; y < clipRectTranslated.top + height; y++, bitmap += celWidth) {
			for (x = 0; x < width; x++) {
				color = bitmap[x];
				if (color != clearKey && priority >= _screen->getPriority(clipRectTranslated.left + x, y)) {
					if (origHeight == -1)	// HACK: this parameter is passed for already scaled views, but we're not actually using it
						_screen->putPixel(clipRectTranslated.left + x, y, drawMask, palette->mapping[color], priority, 0);
					else
						_screen->putPixelOnDisplay(clipRectTranslated.left + x, y, palette->mapping[color]);
				}
			}
		}
	} else {
		byte *EGAmapping = _EGAmapping + (EGAmappingNr * SCI_VIEW_EGAMAPPING_SIZE);
		for (y = clipRectTranslated.top; y < clipRectTranslated.top + height; y++, bitmap += celWidth) {
			for (x = 0; x < width; x++) {
				color = EGAmapping[bitmap[x]];
				if (color != clearKey && priority >= _screen->getPriority(clipRectTranslated.left + x, y))
					_screen->putPixel(clipRectTranslated.left + x, y, drawMask, color, priority, 0);
			}
		}
	}
}

GuiPalette *SciGuiView::getPalette() {
	return _embeddedPal ? &_viewPalette : &_palette->_sysPalette;
}

} // End of namespace Sci
