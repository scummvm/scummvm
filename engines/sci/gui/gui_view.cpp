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

SciGUIview::SciGUIview(OSystem *system, EngineState *state, SciGUIgfx *gfx, SciGUIscreen *screen, GUIResourceId resourceId)
	: _system(system), _s(state), _gfx(gfx), _screen(screen), _resourceId(resourceId) {
	assert(resourceId != -1);
	initData(resourceId);
}

SciGUIview::~SciGUIview() {
}

static const byte EGAMappingDefault[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

void SciGUIview::initData(GUIResourceId resourceId) {
	Resource *viewResource = _s->resMan->findResource(ResourceId(kResourceTypeView, resourceId), false);
	if (!viewResource) {
		error("view resource %d not found", resourceId);
	}
	_resourceData = viewResource->data;

	byte *cellData, *loopData;
	uint16 cellOffset;
	sciViewCellInfo *cell;
	uint16 cellCount = 0;
	uint16 mirrorBits = 0;
	uint16 palOffset = 0;
	uint16 headerSize = 0;
	uint16 loopSize = 0, cellSize = 0;
	int loopNo, cellNo;
	byte seekEntry;
	bool IsEGA = false;

	_embeddedPal = false;
	_EGAMapping = (byte *)&EGAMappingDefault;
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
			// CellCount:WORD Unknown:WORD CellOffset0:WORD CellOffset1:WORD...

			cellCount = READ_LE_UINT16(loopData);
			_loop[loopNo].cellCount = cellCount;
			_loop[loopNo].mirrorFlag = mirrorBits & 1 ? true : false;
			mirrorBits >>= 1;

			// read cel info
			_loop[loopNo].cell = new sciViewCellInfo[cellCount];
			for (cellNo = 0; cellNo < cellCount; cellNo++) {
				cellOffset = READ_LE_UINT16(loopData + 4 + cellNo * 2);
				cellData = _resourceData + cellOffset;

				// For VGA
				// Width:WORD Height:WORD DisplaceX:BYTE DisplaceY:BYTE ClearKey:BYTE Unknown:BYTE RLEData starts now directly
				// For EGA
				// Width:WORD Height:WORD DisplaceX:BYTE DisplaceY:BYTE ClearKey:BYTE EGAData starts now directly
				cell = &_loop[loopNo].cell[cellNo];
				cell->width = READ_LE_UINT16(cellData);
				cell->height = READ_LE_UINT16(cellData + 2);
				cell->displaceX = cellData[4];
				cell->displaceY = cellData[5];
				cell->clearKey = cellData[6];
				if (IsEGA) {
					cell->offsetEGA = cellOffset + 7;
					cell->offsetRLE = 0;
				} else {
					cell->offsetEGA = 0;
					cell->offsetRLE = cellOffset + 8;
				}
				cell->offsetLiteral = 0;
				cell->rawBitmap = 0;
				if (_loop[loopNo].mirrorFlag)
					cell->displaceX = -cell->displaceX;
			}
		}
		break;

	case kViewVga11: // View-format SCI1.1
		// LoopCount:WORD MirrorMask:WORD Version:WORD PaletteOffset:WORD LoopOffset0:WORD LoopOffset1:WORD...
		// HeaderSize:WORD LoopCount:WORD Version:WORD Unknown:WORD PaletteOffset:WORD
		headerSize = READ_LE_UINT16(_resourceData + 0);
		_loopCount = READ_LE_UINT16(_resourceData + 2);
		palOffset = READ_LE_UINT16(_resourceData + 8);

		loopData = _resourceData + headerSize;
		loopSize = _resourceData[12];
		cellSize = _resourceData[13];

		if (palOffset) {
			_gfx->CreatePaletteFromData(&_resourceData[palOffset], &_palette);
			_embeddedPal = true;
		}

		_loop = new sciViewLoopInfo[_loopCount];
		for (loopNo = 0; loopNo < _loopCount; loopNo++) {
			loopData = _resourceData + headerSize + (loopNo * loopSize);
			
			seekEntry = loopData[2];
			if (seekEntry != 255) {
				loopData = _resourceData + headerSize + (seekEntry * loopNo);
			}

			cellCount = loopData[4];
			_loop[loopNo].cellCount = cellCount;
			_loop[loopNo].mirrorFlag = false;

			cellData = _resourceData + READ_LE_UINT16(loopData + 14);

			// read cel info
			_loop[loopNo].cell = new sciViewCellInfo[cellCount];
			for (cellNo = 0; cellNo < cellCount; cellNo++) {
				cell = &_loop[loopNo].cell[cellNo];
				cell->width = READ_LE_UINT16(cellData);
				cell->height = READ_LE_UINT16(cellData + 2);
				cell->displaceX = READ_LE_UINT16(cellData + 4);
				cell->displaceY = READ_LE_UINT16(cellData + 6);
				cell->clearKey = cellData[8];
				cell->offsetEGA = 0;
				cell->offsetRLE = READ_LE_UINT16(cellData + 24);
				cell->offsetLiteral = READ_LE_UINT16(cellData + 28);
				cell->rawBitmap = 0;
				if (_loop[loopNo].mirrorFlag)
					cell->displaceX = -cell->displaceX;

				cellData += cellSize;
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

GUIResourceId SciGUIview::getResourceId() {
	return _resourceId;
}

int16 SciGUIview::getWidth(GUIViewLoopNo loopNo, GUIViewCellNo cellNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount -1);
	if (cellNo >= _loop[loopNo].cellCount)
		cellNo = 0;
	return _loopCount ? _loop[loopNo].cell[cellNo].width : 0;
}

int16 SciGUIview::getHeight(GUIViewLoopNo loopNo, GUIViewCellNo cellNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount -1);
	if (cellNo >= _loop[loopNo].cellCount)
		cellNo = 0;
	return _loopCount ? _loop[loopNo].cell[cellNo].height : 0;
}

sciViewCellInfo *SciGUIview::getCellInfo(GUIViewLoopNo loopNo, GUIViewCellNo cellNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount - 1);
	if (cellNo >= _loop[loopNo].cellCount)
		cellNo = 0;
	return _loopCount ? &_loop[loopNo].cell[cellNo] : NULL;
}

sciViewLoopInfo *SciGUIview::getLoopInfo(GUIViewLoopNo loopNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount - 1);
	return _loopCount ? &_loop[loopNo] : NULL;
}

void SciGUIview::getCellRect(GUIViewLoopNo loopNo, GUIViewCellNo cellNo, int16 x, int16 y, int16 z, Common::Rect *outRect) {
	sciViewCellInfo *cellInfo = getCellInfo(loopNo, cellNo);
	if (cellInfo) {
		outRect->left = x + cellInfo->displaceX - (cellInfo->width >> 1);
		outRect->right = outRect->left + cellInfo->width;
		outRect->bottom = y + cellInfo->displaceY - z + 1;
		outRect->top = outRect->bottom - cellInfo->height;
	}
}

void SciGUIview::unpackCel(GUIViewLoopNo loopNo, GUIViewCellNo cellNo, byte *outPtr, uint16 pixelCount) {
	sciViewCellInfo *cellInfo = getCellInfo(loopNo, cellNo);
	byte *rlePtr;
	byte *literalPtr;
	uint16 pixelNo = 0, brun;
	byte b;

	if (cellInfo->offsetEGA) { // EGA data
		literalPtr = _resourceData + _loop[loopNo].cell[cellNo].offsetEGA;
		// FIXME: Implement EGA "decompression"
		return;
	}


	rlePtr = _resourceData + cellInfo->offsetRLE;
	if (!cellInfo->offsetLiteral) { // no extra literal data
		while (pixelNo < pixelCount) {
			b = *rlePtr++;
			brun = b & 0x3F; // bytes run length on this step
			switch (b & 0xC0) {
			case 0: // copy bytes as-is
				while (brun-- && pixelNo < pixelCount)
					outPtr[pixelNo++] = *rlePtr++;
				break;
			case 0x80: // fill with color
				memset(outPtr + pixelNo, *rlePtr++, MIN<uint16>(brun, pixelCount - pixelNo));
				pixelNo += brun;
				break;
			case 0xC0: // fill with transparent
				pixelNo += brun;
				break;
			}
		}
	} else {
		literalPtr = _resourceData + cellInfo->offsetLiteral;
		while (pixelNo < pixelCount) {
			b = *rlePtr++;
			brun = b & 0x3F; // bytes run length on this step
			switch (b & 0xC0) {
			case 0: // copy bytes as-is
				while (brun-- && pixelNo < pixelCount)
					outPtr[pixelNo++] = *literalPtr++;
				break;
			case 0x80: // fill with color
				memset(outPtr + pixelNo, *literalPtr++, MIN<uint16>(brun, pixelCount - pixelNo));
				pixelNo += brun;
				break;
			case 0xC0: // fill with transparent
				pixelNo += brun;
				break;
			}
		}
	}
}

byte *SciGUIview::getBitmap(GUIViewLoopNo loopNo, GUIViewCellNo cellNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount -1);
	if (cellNo >= _loop[loopNo].cellCount)
		cellNo = 0;
	if (_loop[loopNo].cell[cellNo].rawBitmap)
		return _loop[loopNo].cell[cellNo].rawBitmap;

	uint16 width = _loop[loopNo].cell[cellNo].width;
	uint16 height = _loop[loopNo].cell[cellNo].height;
	// allocating memory to store cel's bitmap
	assert(width * height <= 64000);
	uint16 pixelCount = width * height;
	_loop[loopNo].cell[cellNo].rawBitmap = new byte[pixelCount];
	byte *pOut = _loop[loopNo].cell[cellNo].rawBitmap;

	// Some RLE compressed cels end with the last non-transparent pixel, thats why we fill it up here
	//  FIXME: change this to fill the remaining bytes within unpackCel()
	memset(pOut, _loop[loopNo].cell[cellNo].clearKey, pixelCount);
	unpackCel(loopNo, cellNo, pOut, pixelCount);

	// mirroring the view if needed
	if (_loop[loopNo].mirrorFlag) {
		for (int i = 0; i < height; i++, pOut += width)
			for (int j = 0; j < width / 2; j++)
				SWAP(pOut[j], pOut[width - j - 1]);
	}
	return _loop[loopNo].cell[cellNo].rawBitmap;
}

void SciGUIview::draw(Common::Rect rect, Common::Rect clipRect, GUIViewLoopNo loopNo, GUIViewCellNo cellNo, byte priority, uint16 paletteNo) {
	GUIPalette *palette = _embeddedPal ? &_palette : &_gfx->_sysPalette;
	sciViewCellInfo *cellInfo = getCellInfo(loopNo, cellNo);
	byte *bitmap = getBitmap(loopNo, cellNo);
	int16 cellHeight = cellInfo->height, cellWidth = cellInfo->width;
	int16 width, height;
	byte clearKey = cellInfo->clearKey;
	byte color;
	byte drawMask = priority == 255 ? SCI_SCREEN_MASK_VISUAL : SCI_SCREEN_MASK_VISUAL|SCI_SCREEN_MASK_PRIORITY;
	int x, y;

	// Merge view palette in...
	if (_embeddedPal)
		_gfx->SetPalette(&_palette, 1);

	width = MIN(clipRect.width(), cellWidth);
	height = MIN(clipRect.height(), cellHeight);

	bitmap += (clipRect.top - rect.top) * cellWidth + (clipRect.left - rect.left);
	_gfx->OffsetRect(clipRect);

	for (y = clipRect.top; y < clipRect.top + height; y++, bitmap += cellWidth) {
		for (x = 0; x < width; x++) {
			color = bitmap[x];
			if (color != clearKey && priority >= _screen->Get_Priority(clipRect.left + x, y))
				_screen->Put_Pixel(clipRect.left + x, y, drawMask, palette->mapping[color], priority, 0);
		}
	}
}

} // End of namespace Sci
