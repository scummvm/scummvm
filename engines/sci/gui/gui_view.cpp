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
#include "sci/tools.h"
#include "sci/gui/gui_gfx.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_view.h"

namespace Sci {

SciGuiView::SciGuiView(OSystem *system, EngineState *state, SciGuiGfx *gfx, SciGuiScreen *screen, GuiResourceId resourceId)
	: _system(system), _s(state), _gfx(gfx), _screen(screen), _resourceId(resourceId) {
	assert(resourceId != -1);
	initData(resourceId);
}

SciGuiView::~SciGuiView() {
}

static const byte EGAMappingDefault[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

void SciGuiView::initData(GuiResourceId resourceId) {
	Resource *viewResource = _s->resMan->findResource(ResourceId(kResourceTypeView, resourceId), false);
	if (!viewResource) {
		error("view resource %d not found", resourceId);
	}
	_resourceData = viewResource->data;

	byte *celData, *loopData;
	uint16 celOffset;
	sciViewCelInfo *cel;
	uint16 celCount = 0;
	uint16 mirrorBits = 0;
	uint16 palOffset = 0;
	uint16 headerSize = 0;
	uint16 loopSize = 0, celSize = 0;
	int loopNo, celNo;
	byte seekEntry;
	bool IsEGA = false;

	_embeddedPal = false;
	_EGAMapping = EGAMappingDefault;
	_loopCount = 0;

	switch (_s->resMan->getViewType()) {
	case kViewEga: // View-format SCI0
		IsEGA = true;
	case kViewVga: // View-format SCI1
		// LoopCount:WORD MirrorMask:WORD Version:WORD PaletteOffset:WORD LoopOffset0:WORD LoopOffset1:WORD...
		
		// bit 0x8000 of _resourceData[1] means palette is set
		_loopCount = _resourceData[0];
		mirrorBits = READ_LE_UINT16(_resourceData + 2);
		palOffset = READ_LE_UINT16(_resourceData + 6);

		if (palOffset && palOffset != 0x100) {
			if (IsEGA) { // simple mapping for 16 colors
				_EGAMapping = _resourceData + palOffset;
			} else {
				_gfx->CreatePaletteFromData(&_resourceData[palOffset], &_palette);
				_embeddedPal = true;
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
			_gfx->CreatePaletteFromData(&_resourceData[palOffset], &_palette);
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

	case kViewAmiga: // View-format on amiga
		// FIXME
		error("ViewType Amiga is currently unsupported");
		break;

	default:
		error("ViewType was not detected, can't continue");
	}
}

GuiResourceId SciGuiView::getResourceId() {
	return _resourceId;
}

int16 SciGuiView::getWidth(GuiViewLoopNo loopNo, GuiViewCelNo celNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount -1);
	if (celNo >= _loop[loopNo].celCount)
		celNo = 0;
	return _loopCount ? _loop[loopNo].cel[celNo].width : 0;
}

int16 SciGuiView::getHeight(GuiViewLoopNo loopNo, GuiViewCelNo celNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount -1);
	if (celNo >= _loop[loopNo].celCount)
		celNo = 0;
	return _loopCount ? _loop[loopNo].cel[celNo].height : 0;
}

sciViewCelInfo *SciGuiView::getCelInfo(GuiViewLoopNo loopNo, GuiViewCelNo celNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount - 1);
	if (celNo >= _loop[loopNo].celCount)
		celNo = 0;
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
	byte byte;

	if (celInfo->offsetEGA) { // EGA data
		literalPtr = _resourceData + _loop[loopNo].cel[celNo].offsetEGA;
		while (pixelNo < pixelCount) {
			byte = *literalPtr++;
			runLength = byte >> 4;
			byte = _EGAMapping[byte & 0x0F];
			memset(outPtr + pixelNo, byte, MIN<uint16>(runLength, pixelCount - pixelNo));
			pixelNo += runLength;
		}
		return;
	}


	rlePtr = _resourceData + celInfo->offsetRLE;
	if (!celInfo->offsetLiteral) { // no extra literal data
		while (pixelNo < pixelCount) {
			byte = *rlePtr++;
			runLength = byte & 0x3F;
			switch (byte & 0xC0) {
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
	} else {
		literalPtr = _resourceData + celInfo->offsetLiteral;
		while (pixelNo < pixelCount) {
			byte = *rlePtr++;
			runLength = byte & 0x3F;
			switch (byte & 0xC0) {
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
	}
}

byte *SciGuiView::getBitmap(GuiViewLoopNo loopNo, GuiViewCelNo celNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount -1);
	if (celNo >= _loop[loopNo].celCount)
		celNo = 0;
	if (_loop[loopNo].cel[celNo].rawBitmap)
		return _loop[loopNo].cel[celNo].rawBitmap;

	uint16 width = _loop[loopNo].cel[celNo].width;
	uint16 height = _loop[loopNo].cel[celNo].height;
	// allocating memory to store cel's bitmap
	assert(width * height <= 64000);
	uint16 pixelCount = width * height;
	_loop[loopNo].cel[celNo].rawBitmap = new byte[pixelCount];
	byte *pOut = _loop[loopNo].cel[celNo].rawBitmap;

	// Some RLE compressed cels end with the last non-transparent pixel, thats why we fill it up here
	//  FIXME: change this to fill the remaining bytes within unpackCel()
	memset(pOut, _loop[loopNo].cel[celNo].clearKey, pixelCount);
	unpackCel(loopNo, celNo, pOut, pixelCount);

	// mirroring the cel if needed
	if (_loop[loopNo].mirrorFlag) {
		for (int i = 0; i < height; i++, pOut += width)
			for (int j = 0; j < width / 2; j++)
				SWAP(pOut[j], pOut[width - j - 1]);
	}
	return _loop[loopNo].cel[celNo].rawBitmap;
}

void SciGuiView::draw(Common::Rect rect, Common::Rect clipRect, GuiViewLoopNo loopNo, GuiViewCelNo celNo, byte priority, uint16 paletteNo) {
	GuiPalette *palette = _embeddedPal ? &_palette : &_gfx->_sysPalette;
	sciViewCelInfo *celInfo = getCelInfo(loopNo, celNo);
	byte *bitmap = getBitmap(loopNo, celNo);
	int16 celHeight = celInfo->height, celWidth = celInfo->width;
	int16 width, height;
	byte clearKey = celInfo->clearKey;
	byte color;
	byte drawMask = priority == 255 ? SCI_SCREEN_MASK_VISUAL : SCI_SCREEN_MASK_VISUAL|SCI_SCREEN_MASK_PRIORITY;
	int x, y;

	// Merge view palette in...
	if (_embeddedPal)
		_gfx->SetPalette(&_palette, 1);

	width = MIN(clipRect.width(), celWidth);
	height = MIN(clipRect.height(), celHeight);

	bitmap += (clipRect.top - rect.top) * celWidth + (clipRect.left - rect.left);
	_gfx->OffsetRect(clipRect);

	for (y = clipRect.top; y < clipRect.top + height; y++, bitmap += celWidth) {
		for (x = 0; x < width; x++) {
			color = bitmap[x];
			if (color != clearKey && priority >= _screen->getPriority(clipRect.left + x, y))
				_screen->putPixel(clipRect.left + x, y, drawMask, palette->mapping[color], priority, 0);
		}
	}
}

} // End of namespace Sci
