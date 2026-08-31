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

#include "macs2/amiga_decode.h"

#include "common/compression/powerpacker.h"
#include "common/endian.h"
#include "common/hashmap.h"
#include "common/util.h"

namespace Macs2 {

static const uint16 kPlanes = 6;

bool parseAmigaMxoo(const byte *mxoo, uint32 mxooSize, AmigaMxooInfo &out) {
	out = AmigaMxooInfo();
	if (!mxoo || mxooSize < 24)
		return false;
	if (READ_BE_UINT32(mxoo) != MKTAG('M', 'X', 'O', 'O'))
		return false;

	out.scriptOffset = READ_BE_UINT32(mxoo + 4);
	out.stringOffset = READ_BE_UINT32(mxoo + 8);
	out.bodyOffset = 12;
	if (out.scriptOffset > mxooSize || out.stringOffset > mxooSize)
		return false;
	out.bodySize = out.scriptOffset > out.bodyOffset ? out.scriptOffset - out.bodyOffset : 0;
	if (out.bodySize < 0x66)
		return false;

	const byte *body = mxoo + out.bodyOffset;
	if (READ_BE_UINT16(body + 0x0C) != 0x0101)
		return false;

	for (uint i = 0; i < ARRAYSIZE(out.slotOffsets); i++)
		out.slotOffsets[i] = READ_BE_UINT32(body + 0x0E + i * 4);
	out.extraOffset = READ_BE_UINT32(body + 0x62);
	return true;
}

bool inspectAmigaAnimSlot(const byte *mxoo, uint32 mxooSize, uint32 bodyRelativeOffset, AmigaAnimSlotInfo &out) {
	out = AmigaAnimSlotInfo();
	if (!mxoo || bodyRelativeOffset == 0 || bodyRelativeOffset == 0xFFFFFFFF)
		return false;

	const uint32 absOff = 12 + bodyRelativeOffset;
	if (absOff + 16 > mxooSize)
		return false;

	const byte *p = mxoo + absOff;
	out.headerHint = READ_BE_UINT16(p + 0);
	out.seqPos = READ_BE_UINT16(p + 2);
	out.repeatCounter = READ_BE_UINT16(p + 4);
	out.loopStart = READ_BE_UINT16(p + 6);

	const uint16 seqBytes = (out.headerHint >= 2) ? (uint16)(out.headerHint - 2) : 0;
	const uint16 seqPadded = (seqBytes + 1) & ~1;
	const uint32 metaOff = absOff + 8 + seqPadded;
	if (metaOff + 6 > mxooSize)
		return false;

	out.frameCount = READ_BE_UINT16(mxoo + metaOff + 0);
	out.width = READ_BE_UINT16(mxoo + metaOff + 2);
	out.height = READ_BE_UINT16(mxoo + metaOff + 4);
	out.pixelOffset = metaOff + 6;
	out.headerSize = out.pixelOffset - absOff;

	if (out.frameCount == 0 || out.width == 0 || out.height == 0)
		return false;
	if (out.width > 320 || out.height > 200 || out.frameCount > 256)
		return false;

	const uint32 rowBytes = (out.width + 7) / 8;
	const uint32 planeBytes = rowBytes * out.height;
	const uint32 pixelsNeeded = (uint32)out.frameCount * planeBytes * kPlanes;
	if (out.pixelOffset + pixelsNeeded > mxooSize)
		return false;

	out.valid = true;
	return true;
}

bool decodeAmigaPlanarFrame(const byte *planar, uint16 width, uint16 height, uint16 frameIndex,
							uint16 frameCount, Common::Array<byte> &outPixels) {
	if (!planar || width == 0 || height == 0 || frameIndex >= frameCount)
		return false;

	const uint32 rowBytes = (width + 7) / 8;
	const uint32 planeBytes = rowBytes * height;
	const uint32 frameBytes = planeBytes * kPlanes;
	const byte *frameBase = planar + (uint32)frameIndex * frameBytes;

	outPixels.clear();
	outPixels.resize((uint32)width * height);
	Common::fill(outPixels.begin(), outPixels.end(), 0);

	for (uint16 y = 0; y < height; y++) {
		for (uint16 x = 0; x < width; x++) {
			byte color = 0;
			const uint32 bitIndex = (uint32)x & 7;
			const uint32 byteInRow = (uint32)x >> 3;
			for (uint16 plane = 0; plane < kPlanes; plane++) {
				const byte *planeRow = frameBase + plane * planeBytes + (uint32)y * rowBytes;
				if (planeRow[byteInRow] & (0x80 >> bitIndex))
					color |= (byte)(1 << plane);
			}
			outPixels[(uint32)y * width + x] = color;
		}
	}
	return true;
}

bool convertAmigaAnimSlotToDosBlob(const byte *mxoo, uint32 mxooSize, uint32 bodyRelativeOffset,
								   Common::Array<byte> &outBlob) {
	outBlob.clear();
	AmigaAnimSlotInfo info;
	if (!inspectAmigaAnimSlot(mxoo, mxooSize, bodyRelativeOffset, info) || !info.valid)
		return false;

	const byte *planar = mxoo + info.pixelOffset;

	// DOS AnimBlobView layout
	Common::Array<byte> sequence;
	for (uint16 i = 0; i < info.frameCount; i++)
		sequence.push_back((byte)(10 + i));

	const uint32 seqLenMinusOne = sequence.size();
	const uint32 frameDataOffset = 0x0C + seqLenMinusOne;
	uint32 total = frameDataOffset + 2; // + frameCount word
	for (uint16 f = 0; f < info.frameCount; f++)
		total += 10 + (uint32)info.width * info.height;

	outBlob.resize(total);
	byte *dst = outBlob.data();
	WRITE_LE_UINT16(dst + 0x00, info.headerHint);
	WRITE_LE_UINT16(dst + 0x02, info.seqPos ? info.seqPos : 1);
	WRITE_LE_UINT16(dst + 0x04, info.repeatCounter);
	WRITE_LE_UINT16(dst + 0x06, info.loopStart);
	WRITE_LE_UINT16(dst + 0x08, 0); // delay
	WRITE_LE_UINT16(dst + 0x0A, (uint16)seqLenMinusOne);
	if (!sequence.empty())
		memcpy(dst + 0x0C, sequence.data(), sequence.size());

	WRITE_LE_UINT16(dst + frameDataOffset, info.frameCount);
	uint32 off = frameDataOffset + 2;
	for (uint16 f = 0; f < info.frameCount; f++) {
		Common::Array<byte> pixels;
		if (!decodeAmigaPlanarFrame(planar, info.width, info.height, f, info.frameCount, pixels))
			return false;
		WRITE_LE_UINT16(dst + off + 0, 0); // ox
		WRITE_LE_UINT16(dst + off + 2, 0); // oy
		WRITE_LE_UINT16(dst + off + 4, 0); // unk
		WRITE_LE_UINT16(dst + off + 6, info.width);
		WRITE_LE_UINT16(dst + off + 8, info.height);
		memcpy(dst + off + 10, pixels.data(), pixels.size());
		off += 10 + pixels.size();
	}
	return true;
}

bool extractAmigaScript(const byte *mxoo, uint32 mxooSize, Common::Array<byte> &outScript) {
	outScript.clear();
	AmigaMxooInfo info;
	if (!parseAmigaMxoo(mxoo, mxooSize, info))
		return false;
	if (info.scriptOffset + 4 > mxooSize)
		return false;
	// Script section: u16BE pad, u16BE size, then bytecode
	const uint16 size = READ_BE_UINT16(mxoo + info.scriptOffset + 2);
	const uint32 dataOff = info.scriptOffset + 4;
	if (dataOff + size > mxooSize)
		return false;
	outScript.resize(size);
	if (size)
		memcpy(outScript.data(), mxoo + dataOff, size);
	return true;
}

bool extractAmigaStringBlock(const byte *mxoo, uint32 mxooSize, Common::Array<byte> &outStrings) {
	outStrings.clear();
	AmigaMxooInfo info;
	if (!parseAmigaMxoo(mxoo, mxooSize, info))
		return false;
	if (info.stringOffset + 4 > mxooSize)
		return false;
	const uint16 size = READ_BE_UINT16(mxoo + info.stringOffset + 2);
	const uint32 dataOff = info.stringOffset + 4;
	if (dataOff + size > mxooSize)
		return false;
	outStrings.resize(size);
	if (size)
		memcpy(outStrings.data(), mxoo + dataOff, size);
	return true;
}

bool convertAmigaSimpleSpriteToDosBlob(const byte *mxoo, uint32 mxooSize, Common::Array<byte> &outBlob) {
	outBlob.clear();
	if (!mxoo || mxooSize < 32 || READ_BE_UINT32(mxoo) != MKTAG('M', 'X', 'O', 'O'))
		return false;

	const uint32 scriptOff = READ_BE_UINT32(mxoo + 4);
	if (scriptOff <= 12 || scriptOff > mxooSize)
		return false;

	const byte *body = mxoo + 12;
	const uint32 bodyLen = scriptOff - 12;
	if (bodyLen < 20 || body[10] != 1 || body[11] != 1)
		return false;

	const uint16 frames = READ_BE_UINT16(body + 14);
	const uint16 width = READ_BE_UINT16(body + 16);
	const uint16 height = READ_BE_UINT16(body + 18);
	if (frames == 0 || width == 0 || width > 320 || height == 0 || height > 200)
		return false;

	const uint32 rowBytes = (width + 7) / 8;
	const uint32 planeSize = rowBytes * height;
	const uint32 pixelBytes = bodyLen - 20;
	if (planeSize == 0 || pixelBytes < planeSize * 5)
		return false;
	const uint32 numPlanes = pixelBytes / planeSize;
	if (numPlanes < 5 || numPlanes > 6 || pixelBytes != planeSize * numPlanes)
		return false;

	Common::Array<byte> pixels;
	pixels.resize((uint32)width * height);
	Common::fill(pixels.begin(), pixels.end(), 0);
	const byte *pixData = body + 20;
	for (uint32 plane = 0; plane < 5 && plane < numPlanes; plane++) {
		const uint32 pOff = plane * planeSize;
		for (uint16 y = 0; y < height; y++) {
			for (uint32 bx = 0; bx < rowBytes; bx++) {
				const byte b = pixData[pOff + y * rowBytes + bx];
				for (int bit = 0; bit < 8; bit++) {
					const uint16 x = (uint16)(bx * 8 + bit);
					if (x < width && (b & (0x80 >> bit)))
						pixels[y * width + x] |= (byte)(1 << plane);
				}
			}
		}
	}
	if (numPlanes >= 6) {
		const uint32 pOff = 5 * planeSize;
		for (uint16 y = 0; y < height; y++) {
			for (uint32 bx = 0; bx < rowBytes; bx++) {
				const byte b = pixData[pOff + y * rowBytes + bx];
				for (int bit = 0; bit < 8; bit++) {
					const uint16 x = (uint16)(bx * 8 + bit);
					if (x < width && (b & (0x80 >> bit)) == 0)
						pixels[y * width + x] = 0;
				}
			}
		}
	}

	const uint32 total = 0x0C + 2 + 10 + (uint32)width * height;
	outBlob.resize(total);
	byte *dst = outBlob.data();
	WRITE_LE_UINT16(dst + 0x00, frames);
	WRITE_LE_UINT16(dst + 0x02, 1);
	WRITE_LE_UINT16(dst + 0x04, 0);
	WRITE_LE_UINT16(dst + 0x06, 0);
	WRITE_LE_UINT16(dst + 0x08, 0);
	WRITE_LE_UINT16(dst + 0x0A, 0); // seqLenMinusOne
	WRITE_LE_UINT16(dst + 0x0C, 1); // frameCount
	WRITE_LE_UINT16(dst + 0x0E + 0, 0);
	WRITE_LE_UINT16(dst + 0x0E + 2, 0);
	WRITE_LE_UINT16(dst + 0x0E + 4, 0);
	WRITE_LE_UINT16(dst + 0x0E + 6, width);
	WRITE_LE_UINT16(dst + 0x0E + 8, height);
	memcpy(dst + 0x0E + 10, pixels.data(), pixels.size());
	return true;
}

bool convertAmigaPortraitAtlasToDosBlob(const byte *mxoo, uint32 mxooSize, uint32 bodyRelativeExtraOffset,
										Common::Array<byte> &outBlob) {
	outBlob.clear();
	if (!mxoo || bodyRelativeExtraOffset == 0 || bodyRelativeExtraOffset == 0xFFFFFFFF)
		return false;

	const uint32 absExtra = 12 + bodyRelativeExtraOffset;
	// Atlas is 240x80 (3x80x80 frames). Color from planes 0..4.
	const uint16 atlasW = 240;
	const uint16 atlasH = 80;
	const uint16 frameW = 80;
	const uint16 frameH = 80;
	const uint16 frameCount = 3;
	const uint32 rowBytes = (atlasW + 7) / 8;
	const uint32 planeBytes = rowBytes * atlasH;
	const uint32 portraitBytes = planeBytes * 6; // 14400
	if (absExtra + portraitBytes > mxooSize)
		return false;

	const byte *src = mxoo + absExtra;
	Common::Array<byte> atlas;
	atlas.resize((uint32)atlasW * atlasH);
	Common::fill(atlas.begin(), atlas.end(), 0);
	for (uint16 y = 0; y < atlasH; y++) {
		for (uint16 x = 0; x < atlasW; x++) {
			byte color = 0;
			const uint32 bitIndex = (uint32)x & 7;
			const uint32 byteInRow = (uint32)x >> 3;
			for (uint16 plane = 0; plane < kPlanes; plane++) {
				const byte *planeRow = src + plane * planeBytes + (uint32)y * rowBytes;
				if (planeRow[byteInRow] & (0x80 >> bitIndex))
					color |= (byte)(1 << plane);
			}
			atlas[(uint32)y * atlasW + x] = remapAmigaCopperIndexToStableUi(color);
		}
	}

	// DOS anim blob: seqLenMinusOne = frameCount, sequence at +0x0C, frames follow.
	const uint32 seqLenMinusOne = frameCount;
	const uint32 frameDataOffset = 0x0C + seqLenMinusOne;
	const uint32 frameBytes = 10 + (uint32)frameW * frameH;
	const uint32 total = frameDataOffset + 2 + (uint32)frameCount * frameBytes;
	outBlob.resize(total);
	byte *dst = outBlob.data();
	WRITE_LE_UINT16(dst + 0x00, frameCount);
	WRITE_LE_UINT16(dst + 0x02, 1);
	WRITE_LE_UINT16(dst + 0x04, 0);
	WRITE_LE_UINT16(dst + 0x06, 0);
	WRITE_LE_UINT16(dst + 0x08, 0);
	WRITE_LE_UINT16(dst + 0x0A, (uint16)seqLenMinusOne);
	for (uint16 i = 0; i < frameCount; i++)
		dst[0x0C + i] = (byte)(10 + i);

	WRITE_LE_UINT16(dst + frameDataOffset, frameCount);
	uint32 off = frameDataOffset + 2;
	for (uint16 f = 0; f < frameCount; f++) {
		WRITE_LE_UINT16(dst + off + 0, 0);
		WRITE_LE_UINT16(dst + off + 2, 0);
		WRITE_LE_UINT16(dst + off + 4, 0);
		WRITE_LE_UINT16(dst + off + 6, frameW);
		WRITE_LE_UINT16(dst + off + 8, frameH);
		byte *pixDst = dst + off + 10;
		Common::fill(pixDst, pixDst + (uint32)frameW * frameH, 0);
		for (uint16 y = 0; y < frameH; y++) {
			memcpy(pixDst + (uint32)y * frameW,
				   atlas.data() + (uint32)y * atlasW + (uint32)f * frameW,
				   frameW);
		}
		off += frameBytes;
	}
	return true;
}

static bool decompressPp20ToBuffer(const byte *src, uint32 srcLen, Common::Array<byte> &out) {
	out.clear();
	if (!src)
		return false;

	uint32 outLen = 0;
	byte *unpacked = Common::PowerPackerStream::unpackBuffer(src, srcLen, outLen);
	if (!unpacked || outLen == 0) {
		delete[] unpacked;
		return false;
	}
	out.resize(outLen);
	memcpy(out.data(), unpacked, outLen);
	delete[] unpacked;
	return true;
}

byte remapAmigaCopperIndexToStableUi(byte color) {
	if (color == 0)
		return 0;

	const bool ehb = color >= kAmigaColorRegisterCount;
	const byte base = ehb ? (byte)(color - kAmigaColorRegisterCount) : color;
	byte ui;
	if (base >= 17 && base <= 31)
		ui = (byte)(0xF0 + (base - 16));
	else if (base < 16)
		ui = (byte)(0xF0 + base);
	else
		ui = base;

	if (!ehb)
		return ui;
	if (ui < 0xF0)
		return (byte)(kAmigaColorRegisterCount + base);
	return (byte)(0xE0 + (ui - 0xF0));
}

void amiga12ToVga6(uint16 rgb, byte &r6, byte &g6, byte &b6) {
	const byte r4 = (rgb >> 8) & 0xF;
	const byte g4 = (rgb >> 4) & 0xF;
	const byte b4 = rgb & 0xF;
	r6 = (byte)((r4 * 63) / 15);
	g6 = (byte)((g4 * 63) / 15);
	b6 = (byte)((b4 * 63) / 15);
}

void amiga12ToRgb8(uint16 rgb, byte &r, byte &g, byte &b) {
	const byte r4 = (rgb >> 8) & 0xF;
	const byte g4 = (rgb >> 4) & 0xF;
	const byte b4 = rgb & 0xF;
	r = (byte)(r4 * 17);
	g = (byte)(g4 * 17);
	b = (byte)(b4 * 17);
}

bool decodeAmigaMxmmSceneBackground(const byte *mxmm, uint32 mxmmSize,
									Common::Array<byte> &outPixels,
									Graphics::Palette &outPalette,
									uint &outColorCount,
									Common::Array<byte> &outLineCopperPal) {
	outPixels.clear();
	outLineCopperPal.clear();
	outColorCount = 0;
	outPalette = Graphics::Palette(Graphics::PALETTE_COUNT);
	if (!mxmm || mxmmSize < kAmigaMxmmMinSize)
		return false;
	if (READ_BE_UINT32(mxmm) != MKTAG('M', 'X', 'M', 'M'))
		return false;

	const uint32 chunk0Size = READ_BE_UINT32(mxmm + kAmigaMxmmHeaderSize);
	if (chunk0Size == 0 || kAmigaMxmmMinSize + chunk0Size > mxmmSize)
		return false;

	const byte *chunk0 = mxmm + kAmigaMxmmMinSize;
	Common::Array<byte> screen;
	if (READ_BE_UINT32(chunk0) == MKTAG('P', 'P', '2', '0')) {
		if (!decompressPp20ToBuffer(chunk0, chunk0Size, screen))
			return false;
	} else {
		screen.resize(chunk0Size);
		memcpy(screen.data(), chunk0, chunk0Size);
	}

	if (screen.size() < kAmigaSceneScreenSize)
		return false;

	Common::Array<byte> planarIndex;
	planarIndex.resize((uint)kAmigaSceneWidth * kAmigaSceneHeight);
	Common::fill(planarIndex.begin(), planarIndex.end(), 0);

	const byte *planes = screen.data();
	const uint32 rowBytes = kAmigaSceneWidth / 8; // 40
	for (uint16 plane = 0; plane < kAmigaScenePlanes; plane++) {
		const byte *planeBase = planes + (uint32)plane * kAmigaScenePlaneBytes;
		for (uint16 y = 0; y < kAmigaSceneHeight; y++) {
			for (uint32 bx = 0; bx < rowBytes; bx++) {
				const byte b = planeBase[(uint32)y * rowBytes + bx];
				for (int bit = 0; bit < 8; bit++) {
					const uint16 x = (uint16)(bx * 8 + bit);
					if (b & (0x80 >> bit))
						planarIndex[(uint32)y * kAmigaSceneWidth + x] |= (byte)(1 << plane);
				}
			}
		}
	}

	const byte *copper = screen.data() + kAmigaSceneCopperOffset;
	uint16 base16[kAmigaSceneCopperColorCount];
	for (uint i = 0; i < ARRAYSIZE(base16); i++)
		base16[i] = READ_BE_UINT16(copper + i * 2);
	const byte *lineColors = copper + kAmigaSceneCopperBaseBytes;
	auto buildPal32 = [&](uint16 y, byte pal32[kAmigaColorRegisterCount][3]) {
		amiga12ToRgb8(base16[0], pal32[0][0], pal32[0][1], pal32[0][2]);
		for (uint i = 0; i < kAmigaSceneCopperColorCount; i++) {
			const uint16 c = READ_BE_UINT16(lineColors + (uint32)y * kAmigaSceneCopperLineBytes + i * 2);
			amiga12ToRgb8(c, pal32[1 + i][0], pal32[1 + i][1], pal32[1 + i][2]);
		}
		for (uint i = 1; i < kAmigaSceneCopperColorCount; i++)
			amiga12ToRgb8(base16[i], pal32[kAmigaSceneCopperColorCount + i][0], pal32[kAmigaSceneCopperColorCount + i][1], pal32[kAmigaSceneCopperColorCount + i][2]);
	};

	// Reserve 0..31 for Amiga COLOR registers (sprites) and 32..63 for EHB.
	byte staticPal[kAmigaColorRegisterCount][3];
	buildPal32(0, staticPal);
	for (uint i = 0; i < kAmigaColorRegisterCount; i++)
		outPalette.set(i, staticPal[i][0], staticPal[i][1], staticPal[i][2]);
	for (uint i = 0; i < kAmigaColorRegisterCount; i++)
		outPalette.set(kAmigaColorRegisterCount + i, (byte)(staticPal[i][0] / 2), (byte)(staticPal[i][1] / 2), (byte)(staticPal[i][2] / 2));
	outColorCount = kAmigaEhbPaletteCount;

	Common::HashMap<uint32, byte> colorToIndex;
	for (uint i = 0; i < kAmigaEhbPaletteCount; i++) {
		byte r, g, b;
		outPalette.get(i, r, g, b);
		const uint32 key = ((uint32)r << 16) | ((uint32)g << 8) | b;
		if (!colorToIndex.contains(key))
			colorToIndex[key] = (byte)i;
	}

	outPixels.resize((uint)kAmigaSceneWidth * kAmigaSceneHeight);
	for (uint16 y = 0; y < kAmigaSceneHeight; y++) {
		byte pal32[kAmigaColorRegisterCount][3];
		buildPal32(y, pal32);

		for (uint16 x = 0; x < kAmigaSceneWidth; x++) {
			byte idx = planarIndex[(uint32)y * kAmigaSceneWidth + x];
			byte r, g, b;
			if (idx >= kAmigaColorRegisterCount) {
				const byte base = (byte)(idx - kAmigaColorRegisterCount);
				r = (byte)(pal32[base][0] / 2);
				g = (byte)(pal32[base][1] / 2);
				b = (byte)(pal32[base][2] / 2);
			} else {
				r = pal32[idx][0];
				g = pal32[idx][1];
				b = pal32[idx][2];
			}

			const uint32 key = ((uint32)r << 16) | ((uint32)g << 8) | b;
			byte outIdx;
			if (colorToIndex.contains(key)) {
				outIdx = colorToIndex[key];
			} else if (outColorCount < 0xE0) {
				outIdx = (byte)outColorCount;
				colorToIndex[key] = outIdx;
				outPalette.set(outIdx, r, g, b);
				outColorCount++;
			} else {
				outIdx = idx < kAmigaEhbPaletteCount ? idx : (byte)(idx & (kAmigaColorRegisterCount - 1));
			}
			outPixels[(uint32)y * kAmigaSceneWidth + x] = outIdx;
		}
	}

	auto internRgb = [&](byte r, byte g, byte b, byte fallback) -> byte {
		const uint32 key = ((uint32)r << 16) | ((uint32)g << 8) | b;
		if (colorToIndex.contains(key))
			return colorToIndex[key];
		if (outColorCount < 0xE0) {
			const byte outIdx = (byte)outColorCount;
			colorToIndex[key] = outIdx;
			outPalette.set(outIdx, r, g, b);
			outColorCount++;
			return outIdx;
		}
		return fallback;
	};

	outLineCopperPal.resize((uint)kAmigaSceneHeight * kAmigaEhbPaletteCount);
	for (uint16 y = 0; y < kAmigaSceneHeight; y++) {
		byte pal32[kAmigaColorRegisterCount][3];
		buildPal32(y, pal32);
		for (uint c = 0; c < kAmigaColorRegisterCount; c++) {
			outLineCopperPal[(uint32)y * kAmigaEhbPaletteCount + c] =
				internRgb(pal32[c][0], pal32[c][1], pal32[c][2], (byte)c);
			outLineCopperPal[(uint32)y * kAmigaEhbPaletteCount + kAmigaColorRegisterCount + c] =
				internRgb((byte)(pal32[c][0] / 2), (byte)(pal32[c][1] / 2), (byte)(pal32[c][2] / 2),
						  (byte)(kAmigaColorRegisterCount + c));
		}
	}

	return outColorCount > 0;
}

// load_scene_mxmm sequential slots after the 10-byte MXMM header.
enum AmigaMxmmSlot {
	kAmigaMxmmSlotBg = 0,
	kAmigaMxmmSlotMap0 = 1,
	kAmigaMxmmSlotMap1 = 2,
	kAmigaMxmmSlotMap2 = 3,
	kAmigaMxmmSlotMxaa = 4,
	kAmigaMxmmSlotCount = 5
};

struct AmigaMxmmLayout {
	const byte *slot[kAmigaMxmmSlotCount];
	uint32 slotSize[kAmigaMxmmSlotCount];
	const byte *script;
	uint32 scriptSize;
	const byte *extra;
	uint32 extraSize;
	uint32 tableOff;
};

static bool amigaMxmmReadSizedBlob(const byte *mxmm, uint32 mxmmSize, uint32 &pos,
								   const byte *&outPtr, uint32 &outSize) {
	outPtr = nullptr;
	outSize = 0;
	if (pos + 4 > mxmmSize)
		return false;
	const int32 sz = (int32)READ_BE_UINT32(mxmm + pos);
	pos += 4;
	if (sz < 1)
		return true;
	if (pos + (uint32)sz > mxmmSize)
		return false;
	outPtr = mxmm + pos;
	outSize = (uint32)sz;
	pos += (uint32)sz;
	return true;
}

static bool parseAmigaMxmmLayout(const byte *mxmm, uint32 mxmmSize, AmigaMxmmLayout &out) {
	out = AmigaMxmmLayout();
	if (!mxmm || mxmmSize < kAmigaMxmmMinSize || READ_BE_UINT32(mxmm) != MKTAG('M', 'X', 'M', 'M'))
		return false;

	uint32 pos = kAmigaMxmmHeaderSize;
	for (int i = 0; i < kAmigaMxmmSlotCount; i++) {
		if (!amigaMxmmReadSizedBlob(mxmm, mxmmSize, pos, out.slot[i], out.slotSize[i]))
			return false;
	}
	if (!amigaMxmmReadSizedBlob(mxmm, mxmmSize, pos, out.script, out.scriptSize))
		return false;
	if (!amigaMxmmReadSizedBlob(mxmm, mxmmSize, pos, out.extra, out.extraSize))
		return false;
	out.tableOff = pos;
	return true;
}

static bool decompressAmigaChunkToBuffer(const byte *chunk, uint32 chunkSize, Common::Array<byte> &out) {
	out.clear();
	if (!chunk || chunkSize == 0)
		return false;
	if (READ_BE_UINT32(chunk) == MKTAG('P', 'P', '2', '0'))
		return decompressPp20ToBuffer(chunk, chunkSize, out);
	out.resize(chunkSize);
	memcpy(out.data(), chunk, chunkSize);
	return true;
}

static bool amigaMxmmDecodeLengthPrefixedBlob(const byte *payload, uint32 payloadSize,
											 Common::Array<byte> &out) {
	out.clear();
	if (!payload || payloadSize == 0)
		return true;

	Common::Array<byte> blob;
	if (payloadSize >= 4 && READ_BE_UINT32(payload) == MKTAG('P', 'P', '2', '0')) {
		if (!decompressPp20ToBuffer(payload, payloadSize, blob))
			return false;
	} else {
		blob.resize(payloadSize);
		memcpy(blob.data(), payload, payloadSize);
	}
	if (blob.size() < 4)
		return false;

	const uint32 innerLen = READ_BE_UINT32(blob.data());
	if (innerLen == 0)
		return true;
	if (4 + innerLen > blob.size())
		return false;
	out.resize(innerLen);
	memcpy(out.data(), blob.data() + 4, innerLen);
	return true;
}

static bool amigaMxmmSeekSceneTables(const byte *mxmm, uint32 mxmmSize, uint32 &outPos) {
	outPos = 0;
	AmigaMxmmLayout layout;
	if (!parseAmigaMxmmLayout(mxmm, mxmmSize, layout))
		return false;
	outPos = layout.tableOff;
	return true;
}

bool extractAmigaMxmmSceneScript(const byte *mxmm, uint32 mxmmSize,
								 Common::Array<byte> &outScript,
								 Common::Array<byte> &outStrings) {
	outScript.clear();
	outStrings.clear();

	AmigaMxmmLayout layout;
	if (!parseAmigaMxmmLayout(mxmm, mxmmSize, layout))
		return false;
	if (!amigaMxmmDecodeLengthPrefixedBlob(layout.script, layout.scriptSize, outScript))
		return false;
	if (layout.extraSize != 0 &&
		!amigaMxmmDecodeLengthPrefixedBlob(layout.extra, layout.extraSize, outStrings))
		return false;
	return !outScript.empty();
}

static bool amigaMxmmGetTrailerTableBase(const byte *mxmm, uint32 mxmmSize, uint32 &outPos) {
	outPos = 0;
	if (!amigaMxmmSeekSceneTables(mxmm, mxmmSize, outPos))
		return false;
	if (outPos + kAmigaMxmmTrailerTablesSize > mxmmSize)
		return false;
	outPos += kAmigaMxmmTrailerPreambleSize;
	return true;
}

bool extractAmigaMxmmScenePathfinding(const byte *mxmm, uint32 mxmmSize,
									  uint16 &outNumPoints,
									  Common::Array<AmigaPathfindingNode> &outNodes) {
	outNumPoints = 0;
	outNodes.clear();

	uint32 pos = 0;
	if (!amigaMxmmGetTrailerTableBase(mxmm, mxmmSize, pos))
		return false;

	const byte *table = mxmm + pos;
	outNumPoints = READ_BE_UINT16(table);
	if (outNumPoints > kAmigaMxmmPathfindingNodeCount)
		outNumPoints = kAmigaMxmmPathfindingNodeCount;

	outNodes.resize(kAmigaMxmmPathfindingNodeCount);
	for (uint i = 0; i < kAmigaMxmmPathfindingNodeCount; i++) {
		const byte *node = table + kAmigaMxmmWordSize + i * kAmigaMxmmPathfindingNodeSize;
		AmigaPathfindingNode &dst = outNodes[i];
		dst.x = READ_BE_UINT16(node + kAmigaMxmmPathfindingNodeXOffset);
		dst.y = READ_BE_UINT16(node + kAmigaMxmmPathfindingNodeYOffset);
		for (uint a = 0; a < kAmigaMxmmPathfindingMaxAdj; a++)
			dst.adjacent[a] = node[kAmigaMxmmPathfindingNodeAdjOffset + a];
		dst.numConnections = READ_BE_UINT16(node + kAmigaMxmmPathfindingNodeConnOffset);
		if (dst.numConnections > kAmigaMxmmPathfindingMaxAdj)
			dst.numConnections = kAmigaMxmmPathfindingMaxAdj;
	}
	return true;
}

bool extractAmigaMxmmSceneWalkParams(const byte *mxmm, uint32 mxmmSize,
									 uint16 &outWalkDepthThresholdY,
									 uint16 &outWalkDepthScaleFactor,
									 uint16 &outWalkBaseSpeedPct,
									 uint16 &outScenePaletteMode,
									 uint16 &outPaletteDarkenPercent) {
	outWalkDepthThresholdY = 0;
	outWalkDepthScaleFactor = 0;
	outWalkBaseSpeedPct = 0;
	outScenePaletteMode = 0;
	outPaletteDarkenPercent = 0;

	uint32 pos = 0;
	if (!amigaMxmmGetTrailerTableBase(mxmm, mxmmSize, pos))
		return false;

	pos += kAmigaMxmmPathfindingToWalkOffset;

	outWalkDepthThresholdY = READ_BE_UINT16(mxmm + pos + kAmigaMxmmWalkDepthThresholdYOffset);
	outWalkDepthScaleFactor = READ_BE_UINT16(mxmm + pos + kAmigaMxmmWalkDepthScaleFactorOffset);
	outWalkBaseSpeedPct = READ_BE_UINT16(mxmm + pos + kAmigaMxmmWalkBaseSpeedPctOffset);
	outScenePaletteMode = READ_BE_UINT16(mxmm + pos + kAmigaMxmmWalkScenePaletteModeOffset);
	outPaletteDarkenPercent = READ_BE_UINT16(mxmm + pos + kAmigaMxmmWalkPaletteDarkenPercentOffset);
	return true;
}

bool extractAmigaMxmmSceneMaps(const byte *mxmm, uint32 mxmmSize,
							   Common::Array<byte> &outPathfinding,
							   Common::Array<byte> &outDepth,
							   Common::Array<byte> &outShadow) {
	outPathfinding.clear();
	outDepth.clear();
	outShadow.clear();

	AmigaMxmmLayout layout;
	if (!parseAmigaMxmmLayout(mxmm, mxmmSize, layout))
		return false;

	const uint32 kMapBytes = (uint32)kAmigaSceneWidth * kAmigaSceneHeight; // 64000
	auto decodeMap = [&](int slot, Common::Array<byte> &dest) -> bool {
		if (layout.slotSize[slot] == 0)
			return false;
		Common::Array<byte> decoded;
		if (!decompressAmigaChunkToBuffer(layout.slot[slot], layout.slotSize[slot], decoded))
			return false;
		if (decoded.size() != kMapBytes)
			return false;
		dest = Common::move(decoded);
		return true;
	};

	const bool gotDepth = decodeMap(kAmigaMxmmSlotMap0, outDepth);
	const bool gotPath = decodeMap(kAmigaMxmmSlotMap1, outPathfinding);
	// Map2 is MXCC, not a 64000 shadow surface.
	return gotDepth || gotPath;
}

bool extractAmigaMxmmSceneHotspotColors(const byte *mxmm, uint32 mxmmSize,
										uint16 &outNumHotspots,
										Common::Array<uint16> &outColorTable) {
	outNumHotspots = 0;
	outColorTable.clear();

	uint32 pos = 0;
	if (!amigaMxmmGetTrailerTableBase(mxmm, mxmmSize, pos))
		return false;

	pos += kAmigaMxmmPathfindingToHotspotOffset;
	if (pos + kAmigaMxmmHotspotTableSize > mxmmSize)
		return false;

	outNumHotspots = READ_BE_UINT16(mxmm + pos);
	if (outNumHotspots > kAmigaMxmmPathfindingNodeCount)
		outNumHotspots = kAmigaMxmmPathfindingNodeCount;

	outColorTable.resize(kAmigaMxmmHotspotColorBytes / sizeof(uint16));
	memcpy(outColorTable.data(), mxmm + pos + kAmigaMxmmWordSize, kAmigaMxmmHotspotColorBytes);
	return true;
}

bool extractAmigaMxmmMxccHotspotMap(const byte *mxmm, uint32 mxmmSize,
									Common::Array<byte> &outHotspotMap) {
	outHotspotMap.clear();

	AmigaMxmmLayout layout;
	if (!parseAmigaMxmmLayout(mxmm, mxmmSize, layout))
		return false;

	const byte *mxcc = layout.slot[kAmigaMxmmSlotMap2];
	const uint32 mxccSize = layout.slotSize[kAmigaMxmmSlotMap2];
	if (!mxcc || mxccSize < kAmigaMxccRowDataOffset + kAmigaSceneWidth ||
		READ_BE_UINT32(mxcc) != MKTAG('M', 'X', 'C', 'C'))
		return false;

	if (READ_BE_UINT16(mxcc + kAmigaMxccVersionOffset) != 1)
		return false;

	const byte marker = mxcc[kAmigaMxccMarkerOffset];
	if (kAmigaMxccRowDataOffset > mxccSize)
		return false;

	outHotspotMap.resize((uint)kAmigaSceneWidth * kAmigaSceneHeight);
	Common::fill(outHotspotMap.begin(), outHotspotMap.end(), 0);

	for (uint y = 0; y < kAmigaSceneHeight; y++) {
		uint32 rowOff = 0;
		if (y > 0)
			rowOff = READ_BE_UINT16(mxcc + kAmigaMxccRowTableOffset + (y - 1) * 2);

		uint32 p = kAmigaMxccRowDataOffset + rowOff;
		uint x = 0;
		while (x < kAmigaSceneWidth && p < mxccSize) {
			const byte b = mxcc[p++];
			if (b == marker) {
				if (p + 1 >= mxccSize)
					break;
				const byte color = mxcc[p++];
				const byte run = mxcc[p++];
				for (uint r = 0; r < run && x < kAmigaSceneWidth; r++)
					outHotspotMap[y * kAmigaSceneWidth + x++] = color;
			} else {
				outHotspotMap[y * kAmigaSceneWidth + x++] = b;
			}
		}
	}
	return true;
}

bool amigaMxmmHasMxaaOverlay(const byte *mxmm, uint32 mxmmSize) {
	AmigaMxmmLayout layout;
	if (!parseAmigaMxmmLayout(mxmm, mxmmSize, layout))
		return false;
	if (layout.slotSize[kAmigaMxmmSlotMxaa] == 0)
		return false;
	const byte *blob = layout.slot[kAmigaMxmmSlotMxaa];
	const uint32 mxaaSize = layout.slotSize[kAmigaMxmmSlotMxaa];
	if (mxaaSize >= 4 && READ_BE_UINT32(blob) == MKTAG('M', 'X', 'A', 'A'))
		return true;
	if (mxaaSize >= 4 && READ_BE_UINT32(blob) == MKTAG('P', 'P', '2', '0')) {
		Common::Array<byte> decoded;
		if (decompressAmigaChunkToBuffer(blob, mxaaSize, decoded) &&
			decoded.size() >= 4 && READ_BE_UINT32(decoded.data()) == MKTAG('M', 'X', 'A', 'A'))
			return true;
	}
	return false;
}

bool decodeAmigaMxffFont(const byte *mxff, uint32 mxffSize, Common::Array<AmigaMxffGlyph> &outGlyphs) {
	outGlyphs.clear();
	if (!mxff || mxffSize < 0x10A + 160)
		return false;
	if (READ_BE_UINT32(mxff) != MKTAG('M', 'X', 'F', 'F'))
		return false;
	if (READ_BE_UINT16(mxff + 4) != 1)
		return false;

	const uint16 blitHeight = READ_BE_UINT16(mxff + 6);
	const uint16 atlasRows = READ_BE_UINT16(mxff + 8);
	const uint16 atlasWidthPixels = READ_BE_UINT16(mxff + 0xA);
	const uint16 planes = READ_BE_UINT16(mxff + 0xC);
	if (blitHeight == 0 || atlasRows == 0 || atlasWidthPixels < 8 || planes != kAmigaScenePlanes)
		return false;

	const uint32 rowBytes = (uint32)(atlasWidthPixels >> 3);
	if (rowBytes == 0)
		return false;
	const uint32 planeBytes = rowBytes * atlasRows;
	const uint32 atlasBytes = planeBytes * kAmigaScenePlanes;
	const uint32 atlasOff = 0x10A;
	if (atlasOff + atlasBytes > mxffSize)
		return false;

	const byte *charmap = mxff + 0x0E;
	const byte *widths = mxff + 0x8C;
	const byte *atlas = mxff + atlasOff;
	const uint16 glyphHeight = MIN(blitHeight, atlasRows);
	const uint16 cellWidth = 16;

	for (uint i = 0; i < 0x7E; i++) {
		const byte ascii = (byte)(0x20 + i);
		const byte glyphIndex = charmap[i];
		const byte advance = widths[i];
		// 0x4E in the map is unused/empty
		if (glyphIndex == 0x4E || advance == 0)
			continue;

		const uint32 cellX0 = (uint32)glyphIndex * cellWidth;
		if (cellX0 + advance > atlasWidthPixels)
			continue;

		AmigaMxffGlyph glyph;
		glyph.ascii = (char)ascii;
		glyph.width = advance;
		glyph.height = glyphHeight;
		glyph.pixels.resize((uint32)advance * glyphHeight);
		Common::fill(glyph.pixels.begin(), glyph.pixels.end(), 0);

		for (uint16 y = 0; y < glyphHeight; y++) {
			for (uint16 x = 0; x < advance; x++) {
				const uint32 absX = cellX0 + x;
				byte color = 0;
				const uint32 bit = absX & 7;
				const uint32 byteInRow = absX >> 3;
				for (uint16 plane = 0; plane < kAmigaScenePlanes; plane++) {
					const byte *planeRow = atlas + plane * planeBytes + (uint32)y * rowBytes;
					if (planeRow[byteInRow] & (0x80 >> bit))
						color |= (byte)(1 << plane);
				}
				glyph.pixels[(uint32)y * advance + x] = color;
			}
		}
		outGlyphs.push_back(Common::move(glyph));
	}

	return !outGlyphs.empty();
}

bool extractAmigaMxosPcm(const byte *mxos, uint32 mxosSize, Common::Array<byte> &outPcm, uint16 &outRateHz) {
	outPcm.clear();
	outRateHz = 8000;
	if (!mxos || mxosSize < 0x20)
		return false;
	if (READ_BE_UINT32(mxos) != MKTAG('M', 'X', 'O', 'S'))
		return false;

	const uint16 sampleOff = READ_BE_UINT16(mxos + 0x10);
	if (sampleOff < 0x14 || sampleOff >= mxosSize)
		return false;

	if ((uint32)sampleOff + 4 <= mxosSize) {
		const uint16 period = READ_BE_UINT16(mxos + 0x12);
		if (period >= 0x50 && period <= 0x400) {
			const uint32 rate = 3579545u / (uint32)period;
			if (rate >= 4000 && rate <= 28867)
				outRateHz = (uint16)rate;
		}
	}

	const uint32 pcmBytes = mxosSize - (uint32)sampleOff;
	if (pcmBytes < 16)
		return false;

	outPcm.resize(pcmBytes);
	for (uint32 i = 0; i < pcmBytes; i++)
		outPcm[i] = (byte)((int8)mxos[sampleOff + i] + 128);

	return true;
}

} // End of namespace Macs2
