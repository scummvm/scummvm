/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/debug.h"
#include "common/memstream.h"
#include "common/scummsys.h"
#include "common/str-enc.h"
#include "common/stream.h"
#include "common/system.h"

#include "fool/detection.h"
#include "graphics/cursor.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/managed_surface.h"
#include "graphics/pixelformat.h"
#include "image/pict.h"

#include "fool/fool.h"
#include "fool/toolbox.h"
#include "fool/utils.h"

namespace Fool {

void Toolbox::ClosePicture() {
	ShowPen();
	if (_port) {
		if (!_port->picSave) {
			warning("Toolbox::ClosePicture: picture not open");
			return;
		}
		_port->picSave = nullptr;
	}
}

Common::Rect readRect(Common::SeekableReadStream &stream) {
	Common::Rect result;
	result.top = stream.readSint16BE();
	result.left = stream.readSint16BE();
	result.bottom = stream.readSint16BE();
	result.right = stream.readSint16BE();
	return result;
}

Common::Point readPoint(Common::SeekableReadStream &stream) {
	Common::Point result;
	result.y = stream.readSint16BE();
	result.x = stream.readSint16BE();
	return result;
}

Polygon readPolygon(Common::SeekableReadStream &stream) {
	Polygon result;
	result.polySize = stream.readUint16BE();
	result.polyBBox = readRect(stream);
	for (int i = 0; i < result.polySize - 10; i++) {
		result.polyPoints.push_back(readPoint(stream));
	}
	return result;
}

Region readRegion(Common::SeekableReadStream &stream) {
	Region region;
	region.rgnSize = stream.readUint16BE();
	if (debugChannelSet(8, kDebugGraphics)) {
		debugC(8, kDebugGraphics, "readRegion: region data");
		stream.hexdump(region.rgnSize - 2);
	}
	if (stream.size() - stream.pos() < region.rgnSize - 2) {
		warning("readRegion: not enough data to match size");
	}
	region.rgnBBox = readRect(stream);
	for (int i = 10; i < region.rgnSize; i+=2) {
		region.rgnData.push_back(stream.readSint16BE());
	}
	return region;
}

Pattern readPattern(Common::SeekableReadStream &stream) {
	Pattern result;
	for (int i = 0; i < 8; i++) {
		result.data[i] = stream.readByte();
	}
	return result;
}

PixMap readPixMap(Common::SeekableReadStream &stream, bool hasBaseAddr) {
	PixMap pixMap;
	if (hasBaseAddr) {
		pixMap.baseAddr = stream.readUint32BE();
	}
	uint16 rowBytes = stream.readUint16BE();
	pixMap.rowBytes = rowBytes & 0x3fff;
	pixMap._isBitMap = !(rowBytes & 0x8000);
	pixMap.bounds = readRect(stream);
	if (!pixMap._isBitMap) {
		pixMap.pmVersion = stream.readUint16BE();
		pixMap.packType = stream.readUint16BE();
		pixMap.packSize = stream.readUint32BE();
		pixMap.hRes = stream.readUint32BE();
		pixMap.vRes = stream.readUint32BE();
		pixMap.pixelType = stream.readUint16BE();
		pixMap.pixelSize = stream.readUint16BE();
		pixMap.cmpCount = stream.readUint16BE();
		pixMap.cmpSize = stream.readUint16BE();
		pixMap.planeBytes = stream.readUint32BE();
		pixMap.pmTable = stream.readUint32BE();
		pixMap.pmReserved = stream.readUint32BE();
	}
	return pixMap;
}

BitMap readBitsRectMono(Common::SeekableReadStream &stream, PixMap &pixMap, bool compressed) {
	BitMap result(new Graphics::ManagedSurface());
	result->create(pixMap.bounds.width(), pixMap.bounds.height(), Graphics::PixelFormat::createFormatCLUT8());

	Common::Rect outputRect = result->getBounds();

	if (!compressed) {
		Common::BitStream8MSB bs(stream);
		// rows are word-aligned
		int overflowBits = (pixMap.bounds.width() % 16 == 0) ? 0 : (16 - (pixMap.bounds.width() % 16));
		debugC(5, kDebugLevelGGraphics, "readBitsRectMono: width %d, overflow %d", pixMap.bounds.width(), overflowBits);

		for (int y = pixMap.bounds.top; y < pixMap.bounds.bottom; y++) {
			int yPos = y - pixMap.bounds.top;

			for (int x = pixMap.bounds.left; x < pixMap.bounds.right; x++) {
				int xPos = x - pixMap.bounds.left;

				uint bit = bs.getBit();
				if (outputRect.contains(xPos, yPos)) {
					result->setPixel(xPos, yPos,		bit);
				}
			}
			bs.skip(overflowBits);
		}

		return result;
	}

	for (int y = pixMap.bounds.top; y < pixMap.bounds.bottom; y++) {
		int yPos = y - pixMap.bounds.top;
		int x = 0;

		byte rowBytes = stream.readByte();
		byte readBytes = 0;

		while (readBytes < rowBytes) {
			byte rowBuf[128];
			byte bufLen;

			byte value = stream.readByte();
			readBytes++;

			if (value >= 128) {
				bufLen = (256 - value) + 1;
				byte repeatValue = stream.readByte();
				memset(rowBuf, repeatValue, bufLen);
				readBytes++;
			} else {
				bufLen = value + 1;
				stream.read(rowBuf, bufLen);
				readBytes += bufLen;
			}

			Common::MemoryReadStream ms(rowBuf, bufLen);
			Common::BitStream8MSB bs(ms);

			for (int i = 0; i < 8 * bufLen; i++) {
				int xPos = x;

				uint bit = bs.getBit();

				if (outputRect.contains(xPos, yPos)) {
					result->setPixel(xPos, yPos, bit);
				}

				x++;
			}
		}
	}
	return result;
}

void Toolbox::_drawPackBitsRect(Common::SeekableReadStream &stream, const Common::Rect &picFrame) {
	PixMap pixMap = readPixMap(stream, false);

	// these rectangles are using the pixmap coordinate system
	Common::Rect srcRect = readRect(stream);
	Common::Rect dstRect = readRect(stream);

	SourceMode mode = (SourceMode)stream.readUint16BE();
	pixMap.bounds.debugPrintC(5, kDebugGraphics, "Toolbox::_drawPackBitsRect: bounds");
	srcRect.debugPrintC(5, kDebugGraphics, "Toolbox::_drawPackBitsRect: srcRect");
	dstRect.debugPrintC(5, kDebugGraphics, "Toolbox::_drawPackBitsRect: dstRect");
	picFrame.debugPrintC(5, kDebugGraphics, "Toolbox::_drawPackBitsRect: picFrame");

	if (pixMap._isBitMap) {
		BitMap result = readBitsRectMono(stream, pixMap, true);

		const BitMap intermediate(createRemappedSurface(result->surfacePtr(), nullptr, 0));
		// source rect needs to be in bitmap coordinates
		srcRect.translate(-pixMap.bounds.left, -pixMap.bounds.top);

		// dest rect needs to be in graphics port coordinates
		dstRect.translate(-picFrame.left, -picFrame.top);
		byte fakePal[768];
		Common::fill(fakePal, fakePal+3, 0xff);
		Common::fill(fakePal+3, fakePal+768, 0x00);
		result->rawSurface().debugPrint(5, 0, 0, 0, 0, -1, 128, fakePal);
		CopyBits(intermediate, _port->portBits, srcRect, dstRect, mode, nullptr);
	} else {
		warning("Toolbox::_drawPackBitsRect: PixMaps unsupported");
	}
}


void Toolbox::DrawPicture(PicHandle &myPicture, const Common::Rect &dstRect) {
	if (!_port) {
		warning("Toolbox::DrawPicture: no graphics port");
		return;
	}

	if (!myPicture) {
		warning("Toolbox::DrawPicture: handle is empty");
		return;
	}

	if ((dstRect.width() != myPicture->picFrame.width()) ||
		(dstRect.height() != myPicture->picFrame.height())) {
		warning("Toolbox::DrawPicture: mismatched dimensions not supported");
	}

	if (myPicture->picData.size() < 2) {
		warning("Toolbox::DrawPicture: no image data!");
		return;
	}

	Common::MemoryReadStream stream(myPicture->picData.data(), myPicture->picData.size(), DisposeAfterUse::NO);

	if (debugChannelSet(8, kDebugGraphics)) {
		debugC(8, kDebugGraphics, "Toolbox::DrawPicture: image contents");
		stream.hexdump(stream.size());
	}


	uint16 version = stream.readUint16BE();
	if ((version != kOpVersion1) && (version != kOpVersion2)) {
		warning("Toolbox::DrawPicture: cannot find PICT version opcode");
		return;
	}

	if (version == kOpVersion2) {
		uint16 header = stream.readUint16BE();
		if (header != kOpHeaderOp) {
			warning("Toolbox::DrawPicture: cannot find PICT header opcode");
			return;
		}
		// we don't use the header yet
		stream.skip(24);
	}

	GrafPtr origState = _port;
	GrafPort temp;
	OpenPort(&temp);
	temp.portBits = origState->portBits;
	temp.portRect = dstRect;

	Common::Point ovSize;
	Common::Rect lastRect;
	int count = 0;

	bool endPic = false;

	while (!stream.eos() && !endPic) {
		size_t pos = stream.pos();
		uint16 op;
		if (version == kOpVersion2)
			op = stream.readUint16BE();
		else
			op = stream.readByte();

		debugC(5, kDebugGraphics, "Toolbox::DrawPicture: [%04x] opcode %04x", (uint32_t)pos, op);

		switch (op) {
		case kOpNOP:
			break;
		case kOpClip:
			warning("STUB: Toolbox::DrawPicture: clip");
			readRegion(stream);
			break;
		case kOpBkPat:
			BackPat(readPattern(stream));
			break;
		case kOpTxFont:
			TextFont(stream.readUint16BE());
			break;
		case kOpTxFace:
			TextFont(stream.readByte());
			break;
		case kOpTxMode:
			TextMode((SourceMode)stream.readUint16BE());
			break;
		case kOpSpExtra:
			warning("STUB: Toolbox::DrawPicture: spExtra");
			stream.skip(4);
			break;
		case kOpPnSize:
			{
				int16 width = stream.readSint16BE();
				int16 height = stream.readSint16BE();
				PenSize(width, height);
			}
			break;
		case kOpPnMode:
			PenMode((PatternMode)stream.readUint16BE());
			break;
		case kOpPnPat:
			PenPat(readPattern(stream));
			break;
		case kOpFillPat:
			_port->fillPat = readPattern(stream);
			break;
		case kOpOvSize:
			ovSize = readPoint(stream);
			break;
		case kOpOrigin:
			{
				Common::Point loc = readPoint(stream);
				SetOrigin(loc.x, loc.y);
			}
			break;
		case kOpTxSize:
			TextSize(stream.readUint16BE());
			break;
		case kOpFgColor:
			_port->fgColor = stream.readUint32BE();
			break;
		case kOpBkColor:
			_port->bkColor = stream.readUint32BE();
			break;
		case kOpTxRatio:
			warning("STUB: Toolbox::DrawPicture: txRatio");
			readPoint(stream);
			readPoint(stream);
			break;
		case kOpVersion2:
			break;
		case kOpBkPixPat:
			warning("STUB: Toolbox::DrawPicture: bkPixPat (struct unsupported)");
			break;
		case kOpPnPixPat:
			warning("STUB: Toolbox::DrawPicture: pnPixPat (struct unsupported)");
			break;
		case kOpFillPixPat:
			warning("STUB: Toolbox::DrawPicture: fillPixPat (struct unsupported)");
			break;
		case kOpPnLocHFrac:
			warning("STUB: Toolbox::DrawPicture: pnLocHFrac");
			stream.skip(2);
			break;
		case kOpChExtra:
			warning("STUB: Toolbox::DrawPicture: chExtra");
			stream.skip(2);
			break;
		case kOpRGBFgCol:
			warning("STUB: Toolbox::DrawPicture: rgbFgCol");
			stream.skip(6);
			break;
		case kOpRGBBkCol:
			warning("STUB: Toolbox::DrawPicture: rgbBkCol");
			stream.skip(6);
			break;
		case kOpHiliteMode:
			warning("STUB: Toolbox::DrawPicture: hiliteMode");
			break;
		case kOpHiliteColor:
			warning("STUB: Toolbox::DrawPicture: hiliteColor");
			stream.skip(6);
			break;
		case kOpDefHilite:
			warning("STUB: Toolbox::DrawPicture: defHilite");
			break;
		case kOpColor:
			warning("STUB: Toolbox::DrawPicture: color");
			stream.skip(6);
			break;
		case kOpLine:
			{
				Common::Point pnLoc = readPoint(stream);
				Common::Point newPt = readPoint(stream);
				MoveTo(pnLoc.x, pnLoc.y);
				LineTo(newPt.x, newPt.y);
			}
			break;
		case kOpLineFrom:
			{
				Common::Point newPt = readPoint(stream);
				LineTo(newPt.x, newPt.y);
			}
			break;
		case kOpShortLine:
			{
				Common::Point pnLoc = readPoint(stream);
				int8 dh = stream.readSByte();
				int8 dv = stream.readSByte();
				MoveTo(pnLoc.x, pnLoc.y);
				Line(dh, dv);
			}
			break;
		case kOpShortLineFrom:
			{
				int8 dh = stream.readSByte();
				int8 dv = stream.readSByte();
				Line(dh, dv);
			}
			break;
		case kOpLongText:
			{
				Common::Point txLoc = readPoint(stream);
				Common::String text = stream.readPascalString(false);
				MoveTo(txLoc.x, txLoc.y);
				DrawString(text.decode(Common::kMacRoman));
			}
			break;
		case kOpDHText:
			{
				Common::Point txLoc = _port->pnLoc;
				txLoc.x += stream.readSByte();
				Common::String text = stream.readPascalString(false);
				MoveTo(txLoc.x, txLoc.y);
				DrawString(text.decode(Common::kMacRoman));
			}
			break;
		case kOpDVText:
			{
				Common::Point txLoc = _port->pnLoc;
				txLoc.y += stream.readSByte();
				Common::String text = stream.readPascalString(false);
				MoveTo(txLoc.x, txLoc.y);
				DrawString(text.decode(Common::kMacRoman));
			}
			break;
		case kOpDHDVText:
			{
				Common::Point txLoc = _port->pnLoc;
				txLoc.x += stream.readSByte();
				txLoc.y += stream.readSByte();
				Common::String text = stream.readPascalString(false);
				MoveTo(txLoc.x, txLoc.y);
				DrawString(text.decode(Common::kMacRoman));
			}
			break;
		case kOpFontName:
			warning("STUB: Toolbox::DrawPicture: fontName");
			stream.skip(5);
			stream.readPascalString(false);
			break;

		case kOpFrameRect:
			lastRect = readRect(stream);
			FrameRect(lastRect);
			break;
		case kOpPaintRect:
			lastRect = readRect(stream);
			PaintRect(lastRect);
			break;
		case kOpEraseRect:
			lastRect = readRect(stream);
			EraseRect(lastRect);
			break;
		case kOpInvertRect:
			lastRect = readRect(stream);
			InvertRect(lastRect);
			break;
		case kOpFillRect:
			lastRect = readRect(stream);
			FillRect(lastRect, _port->fillPat);
			break;
		case kOpFrameSameRect:
			FrameRect(lastRect);
			break;
		case kOpPaintSameRect:
			PaintRect(lastRect);
			break;
		case kOpEraseSameRect:
			EraseRect(lastRect);
			break;
		case kOpInvertSameRect:
			InvertRect(lastRect);
			break;
		case kOpFillSameRect:
			FillRect(lastRect, _port->fillPat);
			break;

		case kOpFrameRRect:
			lastRect = readRect(stream);
			FrameRoundRect(lastRect, ovSize.x, ovSize.y);
			break;
		case kOpPaintRRect:
			lastRect = readRect(stream);
			PaintRoundRect(lastRect, ovSize.x, ovSize.y);
			break;
		case kOpEraseRRect:
			lastRect = readRect(stream);
			EraseRoundRect(lastRect, ovSize.x, ovSize.y);
			break;
		case kOpInvertRRect:
			lastRect = readRect(stream);
			InvertRoundRect(lastRect, ovSize.x, ovSize.y);
			break;
		case kOpFillRRect:
			lastRect = readRect(stream);
			FillRoundRect(lastRect, ovSize.x, ovSize.y, _port->fillPat);
			break;
		case kOpFrameSameRRect:
			FrameRoundRect(lastRect, ovSize.x, ovSize.y);
			break;
		case kOpPaintSameRRect:
			PaintRoundRect(lastRect, ovSize.x, ovSize.y);
			break;
		case kOpEraseSameRRect:
			EraseRoundRect(lastRect, ovSize.x, ovSize.y);
			break;
		case kOpInvertSameRRect:
			InvertRoundRect(lastRect, ovSize.x, ovSize.y);
			break;
		case kOpFillSameRRect:
			FillRoundRect(lastRect, ovSize.x, ovSize.y, _port->fillPat);
			break;

		case kOpBitsRect:
			warning("STUB: Toolbox::DrawPicture: bitsRect, aborting");
			endPic = true;
			break;
		case kOpBitsRgn:
			warning("STUB: Toolbox::DrawPicture: bitsRgn, aborting");
			endPic = true;
			break;

		case kOpPackBitsRect:
			_drawPackBitsRect(stream, myPicture->picFrame);
			break;
		case kOpPackBitsRgn:
			warning("STUB: Toolbox::DrawPicture: packBitsRgn, aborting");
			endPic = true;
			break;


		case kOpShortComment:
			//warning("STUB: Toolbox::DrawPicture: shortComment");
			stream.readUint16BE();
			break;
		case kOpLongComment:
			{
				//warning("STUB: Toolbox::DrawPicture: longComment");
				stream.readUint16BE();
				int size = stream.readUint16BE();
				stream.skip(size);
			}
			break;

		case kOpEndPic:
			endPic = true;
			break;

		default:
			warning("Toolbox::DrawPicture: unsupported opcode %04x", op);
		}
		// if version 2, align to nearest word
		if (version == kOpVersion2)
			stream.skip(stream.pos() % 2);

		count++;
		if (myPicture->_opsPerTick && (count % myPicture->_opsPerTick == 0)) {
			Delay(0);
		}
	}
	_port = origState;
}

PicHandle Toolbox::GetPicture(uint16 picID) {
	Handle handle = this->GetResource(MKTAG('P', 'I', 'C', 'T'), picID);
	PicHandle result;
	if (handle) {
		Common::MemoryReadStream stream(handle->data(), handle->size(), DisposeAfterUse::NO);
		result = PicHandle(new Picture);
		result->picSize = stream.readUint16BE();
		result->picFrame = readRect(stream);
		while (!stream.eos()) {
			result->picData.push_back(stream.readByte());
		}
		_resPicts[result] = handle;
	}

	return result;
}

void Toolbox::KillPicture(PicHandle &myPicture) {
	myPicture = nullptr;
}


PicHandle Toolbox::OpenPicture(const Common::Rect &picFrame) {
	PicHandle result(new Picture);
	result->picFrame = picFrame;
	if (_port) {
		if (_port->picSave) {
			warning("Toolbox::OpenPicture: last picture not closed, overwriting");
		}
		_port->picSave = result;
	}
	HidePen();
	return result;
}



} // namespace Fool
