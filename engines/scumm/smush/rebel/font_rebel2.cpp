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

#include "scumm/smush/rebel/font_rebel2.h"

#include "common/endian.h"
#include "common/util.h"

#include "scumm/file.h"
#include "scumm/nut_renderer.h"
#include "scumm/scumm.h"
#include "scumm/smush/rebel/codec_ra2.h"

namespace Scumm {

enum {
	kRebel2MaxStrings = 80,
	kRebel2MaxSpritePixels = 16 * 1024 * 1024,
	kRebel2MaxDecodedSpriteBytes = 32 * 1024 * 1024
};

struct Rebel2NutFrameInfo {
	Rebel2NutFrameInfo() : codec(0), xoffs(0), yoffs(0), width(0), height(0), effect(0), data(nullptr), dataSize(0) {}

	int codec;
	int16 xoffs;
	int16 yoffs;
	uint16 width;
	uint16 height;
	uint16 effect;
	const byte *data;
	int32 dataSize;
};

static void decodeRebel2RawSprite(byte *dst, const byte *src, int width, int height, int dataSize) {
	const int32 totalSize = width * height;
	const int32 copySize = MIN<int32>(totalSize, dataSize);
	if (copySize > 0)
		memcpy(dst, src, copySize);
}

static void applyRebel2Codec23Effect(byte *buffer, int pitch, int width, int height,
		const Common::Rect &clipRect, int x, int y, int spriteWidth, int spriteHeight,
		const byte *src, int32 srcSize, byte effect, const byte *lookup, int scale) {
	if (!buffer || !src || srcSize <= 0 || spriteWidth <= 0 || spriteHeight <= 0)
		return;

	if (scale < 1)
		scale = 1;

	Common::Rect clipped = clipRect;
	clipped.clip(Common::Rect(0, 0, MIN(width, pitch), height));
	if (clipped.isEmpty())
		return;

	const byte *srcEnd = src + srcSize;
	for (int row = 0; row < spriteHeight && src + 2 <= srcEnd; ++row) {
		const int rowSize = READ_LE_UINT16(src);
		src += 2;
		if (src + rowSize > srcEnd)
			break;

		const int scaledTop = y + row * scale;
		const int scaledBottom = scaledTop + scale;
		if (scaledBottom > clipped.top && scaledTop < clipped.bottom) {
			const byte *rowPtr = src;
			const byte *rowEnd = src + rowSize;
			int dstX = x;

			while (rowPtr + 2 <= rowEnd) {
				dstX += *rowPtr++ * scale;

				int run = *rowPtr++;
				int scaledRun = run * scale;
				int drawLeft = MAX<int>(dstX, clipped.left);
				int drawRight = MIN<int>(dstX + scaledRun, clipped.right);
				if (drawLeft < drawRight) {
					for (int dstY = MAX<int>(scaledTop, clipped.top); dstY < MIN<int>(scaledBottom, clipped.bottom); ++dstY) {
						byte *pixel = buffer + dstY * pitch + drawLeft;
						if (lookup) {
							for (int column = drawLeft; column < drawRight; ++column, ++pixel)
								*pixel = lookup[*pixel];
						} else {
							for (int column = drawLeft; column < drawRight; ++column, ++pixel)
								*pixel = (byte)(*pixel + effect);
						}
					}
				}

				dstX += scaledRun;
				if (dstX >= x + spriteWidth * scale)
					break;
			}
		}

		src += rowSize;
	}
}

class Rebel2NutRenderer : public NutRenderer {
public:
	Rebel2NutRenderer(ScummEngine *vm, const char *filename) :
			NutRenderer(vm, nullptr), _spriteData(nullptr), _spriteFrameCount(0) {
		clearCodec23Table();
		clearCodec45Tables();
		loadRebel2Font(filename);
	}

	Rebel2NutRenderer(ScummEngine *vm, const byte *data, int32 dataSize) :
			NutRenderer(vm, nullptr), _spriteData(nullptr), _spriteFrameCount(0) {
		clearCodec23Table();
		clearCodec45Tables();
		loadRebel2SpriteFromData(data, dataSize);
	}

	~Rebel2NutRenderer() override {
		delete[] _spriteData;
	}

	bool drawCodec45Sprite(byte *buffer, int pitch, int width, int height,
			const Common::Rect &clipRect, int x, int y, int spriteIdx, int scale);
	bool drawCodec23Sprite(byte *buffer, int pitch, int width, int height,
			const Common::Rect &clipRect, int x, int y, int spriteIdx, int scale);

private:
	void codec44(byte *dst, const byte *src, int width, int height, int pitch);
	void loadRebel2Font(const char *filename);
	void loadRebel2SpriteFromData(const byte *data, int32 dataSize);
	void decodeRebel2Frame(byte *dst, const Rebel2NutFrameInfo &frame, byte *codec45Palette, byte *codec45Lookup);
	void clearSpriteData();
	void clearCodec23Table();
	void clearCodec45Tables();

	byte *_spriteData;
	int _spriteFrameCount;
	Rebel2NutFrameInfo _spriteFrames[256];
	byte _codec23Lookup[256];
	byte _codec45Palette[0x300];
	byte _codec45Lookup[0x8000];
};

void Rebel2NutRenderer::clearSpriteData() {
	delete[] _spriteData;
	_spriteData = nullptr;
	_spriteFrameCount = 0;
	for (int i = 0; i < ARRAYSIZE(_spriteFrames); ++i)
		_spriteFrames[i] = Rebel2NutFrameInfo();
}

void Rebel2NutRenderer::clearCodec23Table() {
	for (int i = 0; i < ARRAYSIZE(_codec23Lookup); ++i)
		_codec23Lookup[i] = (byte)i;
}

void Rebel2NutRenderer::clearCodec45Tables() {
	memset(_codec45Palette, 0, sizeof(_codec45Palette));
	memset(_codec45Lookup, 0, sizeof(_codec45Lookup));
}

static int rebel2FloorDiv(int value, int divisor) {
	if (value >= 0)
		return value / divisor;
	return -((-value + divisor - 1) / divisor);
}

bool Rebel2NutRenderer::drawCodec45Sprite(byte *buffer, int pitch, int width, int height,
		const Common::Rect &clipRect, int x, int y, int spriteIdx, int scale) {
	if (!buffer || spriteIdx < 0 || spriteIdx >= _spriteFrameCount)
		return false;

	const Rebel2NutFrameInfo &frame = _spriteFrames[spriteIdx];
	if (frame.codec != 45 || !frame.data || frame.dataSize <= 0)
		return false;

	if (scale < 1)
		scale = 1;

	const int dstWidth = MIN(width, pitch);
	if (dstWidth <= 2 || height <= 2)
		return true;

	Common::Rect clipped = clipRect;
	clipped.clip(Common::Rect(0, 0, dstWidth, height));
	if (clipped.isEmpty())
		return true;

	if (scale == 1) {
		smushDecodeRA2BlurClip(buffer, frame.data, x, y,
			clipped.left, clipped.top, clipped.right, clipped.bottom,
			dstWidth, height, pitch, frame.dataSize,
			_codec45Palette, _codec45Lookup);
		return true;
	}

	const int nativeWidth = (dstWidth + scale - 1) / scale;
	const int nativeHeight = (height + scale - 1) / scale;
	if (nativeWidth <= 2 || nativeHeight <= 2)
		return true;

	const size_t nativeSize = (size_t)nativeWidth * nativeHeight;
	byte *nativeBuffer = new byte[nativeSize];
	for (int ny = 0; ny < nativeHeight; ++ny) {
		const int srcY = MIN(ny * scale, height - 1);
		for (int nx = 0; nx < nativeWidth; ++nx) {
			const int srcX = MIN(nx * scale, dstWidth - 1);
			nativeBuffer[ny * nativeWidth + nx] = buffer[srcY * pitch + srcX];
		}
	}

	const int nativeX = rebel2FloorDiv(x, scale);
	const int nativeY = rebel2FloorDiv(y, scale);
	const int nativeClipLeft = rebel2FloorDiv(clipped.left, scale);
	const int nativeClipTop = rebel2FloorDiv(clipped.top, scale);
	const int nativeClipRight = (clipped.right + scale - 1) / scale;
	const int nativeClipBottom = (clipped.bottom + scale - 1) / scale;
	smushDecodeRA2BlurClip(nativeBuffer, frame.data, nativeX, nativeY,
		nativeClipLeft, nativeClipTop, nativeClipRight, nativeClipBottom,
		nativeWidth, nativeHeight, nativeWidth, frame.dataSize, _codec45Palette, _codec45Lookup);

	const int copyLeft = CLIP<int>(nativeX - 1, 0, nativeWidth);
	const int copyTop = CLIP<int>(nativeY - 1, 0, nativeHeight);
	const int copyRight = CLIP<int>(nativeX + frame.width + 1, 0, nativeWidth);
	const int copyBottom = CLIP<int>(nativeY + frame.height + 1, 0, nativeHeight);
	for (int ny = copyTop; ny < copyBottom; ++ny) {
		for (int sy = 0; sy < scale; ++sy) {
			const int dstY = ny * scale + sy;
			if (dstY < clipped.top || dstY >= clipped.bottom)
				continue;

			byte *dstRow = buffer + dstY * pitch;
			const byte *srcRow = nativeBuffer + ny * nativeWidth;
			for (int nx = copyLeft; nx < copyRight; ++nx) {
				for (int sx = 0; sx < scale; ++sx) {
					const int dstX = nx * scale + sx;
					if (dstX >= clipped.left && dstX < clipped.right)
						dstRow[dstX] = srcRow[nx];
				}
			}
		}
	}

	delete[] nativeBuffer;
	return true;
}

bool Rebel2NutRenderer::drawCodec23Sprite(byte *buffer, int pitch, int width, int height,
		const Common::Rect &clipRect, int x, int y, int spriteIdx, int scale) {
	if (!buffer || spriteIdx < 0 || spriteIdx >= _spriteFrameCount)
		return false;

	const Rebel2NutFrameInfo &frame = _spriteFrames[spriteIdx];
	if (frame.codec != 23 || !frame.data || frame.dataSize <= 0)
		return false;

	const byte *src = frame.data;
	int32 srcSize = frame.dataSize;
	const byte *lookup = nullptr;

	if (frame.effect == 0x100) {
		if (srcSize < 256)
			return true;
		lookup = src;
		src += 256;
		srcSize -= 256;
	} else if (frame.effect > 0xff) {
		lookup = _codec23Lookup;
	}

	applyRebel2Codec23Effect(buffer, pitch, width, height, clipRect, x, y,
		frame.width, frame.height, src, srcSize, (byte)frame.effect, lookup, scale);
	return true;
}

void Rebel2NutRenderer::codec44(byte *dst, const byte *src, int width, int height, int pitch) {
	while (height--) {
		byte *dstPtrNext = dst + pitch;
		const byte *srcPtrNext = src + 2 + READ_LE_UINT16(src);
		src += 2;
		int len = width;
		do {
			int offs = READ_LE_UINT16(src); src += 2;
			dst += offs;
			len -= offs;
			if (len <= 0)
				break;

			int w = READ_LE_UINT16(src) + 1; src += 2;
			len -= w;
			if (len < 0)
				w += len;

			while (w--) {
				byte value = *src++;
				*dst++ = (value == 0xff) ? 0 : value;
			}
		} while (len > 0);
		dst = dstPtrNext;
		src = srcPtrNext;
	}
}

void Rebel2NutRenderer::loadRebel2Font(const char *filename) {
	clearSpriteData();
	clearCodec23Table();
	clearCodec45Tables();

	ScummFile *file = _vm->instantiateScummFile();

	_vm->openFile(*file, filename);
	if (!file->isOpen())
		error("Rebel2NutRenderer::loadRebel2Font() Can't open font file: %s", filename);

	uint32 tag = file->readUint32BE();
	if (tag != MKTAG('A','N','I','M'))
		error("Rebel2NutRenderer::loadRebel2Font() there is no ANIM chunk in font header");

	uint32 length = file->readUint32BE();
	byte *dataSrc = new byte[length];
	file->read(dataSrc, length);
	file->close();
	delete file;

	if (READ_BE_UINT32(dataSrc) != MKTAG('A','H','D','R'))
		error("Rebel2NutRenderer::loadRebel2Font() there is no AHDR chunk in font header");

	_numChars = READ_LE_UINT16(dataSrc + 10);
	if (_numChars > ARRAYSIZE(_chars)) {
		warning("Rebel2NutRenderer::loadRebel2Font(%s) numChars (%d) exceeds max, clamping", filename, _numChars);
		_numChars = ARRAYSIZE(_chars);
	}

	uint32 offset = 0;
	uint32 decodedLength = 0;
	int l;

	for (l = 0; l < _numChars; l++) {
		if (offset + 8 > length) {
			warning("Rebel2NutRenderer::loadRebel2Font(%s) truncated before char %d (offset %x), clamping", filename, l, offset);
			break;
		}
		uint32 chunkSize = READ_BE_UINT32(dataSrc + offset + 4);
		uint64 nextOffset = (uint64)offset + chunkSize + 16 + (chunkSize & 1);
		if (nextOffset + 18 > length) {
			warning("Rebel2NutRenderer::loadRebel2Font(%s) font chunk exceeds file at char %d (offset %x), clamping", filename, l, offset);
			break;
		}
		offset = (uint32)nextOffset;
		int width = READ_LE_UINT16(dataSrc + offset + 14);
		_fontHeight = READ_LE_UINT16(dataSrc + offset + 16);
		decodedLength += width * _fontHeight;
	}

	if (l < _numChars)
		_numChars = l;

	if (_numChars <= 0 || decodedLength == 0)
		error("Rebel2NutRenderer::loadRebel2Font(%s) no decodable characters", filename);

	debugC(DEBUG_SMUSH, "Rebel2NutRenderer::loadRebel2Font('%s') - decodedLength = %d", filename, decodedLength);

	_decodedData = new byte[decodedLength];
	byte *decodedPtr = _decodedData;

	offset = 0;
	for (l = 0; l < _numChars; l++) {
		if (offset + 8 > length) {
			warning("Rebel2NutRenderer::loadRebel2Font(%s) invalid font chunk header %d (offset %x), stopping decode", filename, l, offset);
			break;
		}
		uint32 chunkSize = READ_BE_UINT32(dataSrc + offset + 4);
		uint64 nextOffset = (uint64)offset + chunkSize + 8 + (chunkSize & 1);
		if (nextOffset + 8 > length) {
			warning("Rebel2NutRenderer::loadRebel2Font(%s) FRME chunk exceeds file %d (offset %x), stopping decode", filename, l, offset);
			break;
		}
		offset = (uint32)nextOffset;
		if (READ_BE_UINT32(dataSrc + offset) != MKTAG('F','R','M','E')) {
			warning("Rebel2NutRenderer::loadRebel2Font(%s) no FRME chunk %d (offset %x), stopping decode", filename, l, offset);
			break;
		}
		offset += 8;
		if (offset + 22 > length) {
			warning("Rebel2NutRenderer::loadRebel2Font(%s) FOBJ chunk exceeds file %d (offset %x), stopping decode", filename, l, offset);
			break;
		}
		if (READ_BE_UINT32(dataSrc + offset) != MKTAG('F','O','B','J')) {
			warning("Rebel2NutRenderer::loadRebel2Font(%s) no FOBJ chunk in FRME chunk %d (offset %x), stopping decode", filename, l, offset);
			break;
		}

		int codec = READ_LE_UINT16(dataSrc + offset + 8);
		_chars[l].xoffs = READ_LE_INT16(dataSrc + offset + 10);
		_chars[l].yoffs = READ_LE_INT16(dataSrc + offset + 12);
		_chars[l].width = READ_LE_UINT16(dataSrc + offset + 14);
		_chars[l].height = READ_LE_UINT16(dataSrc + offset + 16);
		_chars[l].src = decodedPtr;

		decodedPtr += (_chars[l].width * _chars[l].height);

		memset(_chars[l].src, kDefaultTransparentColor, _chars[l].width * _chars[l].height);
		_chars[l].transparency = kDefaultTransparentColor;

		const uint8 *fobjptr = dataSrc + offset + 22;
		switch (codec) {
		case 1:
			codec1(_chars[l].src, fobjptr, _chars[l].width, _chars[l].height, _chars[l].width);
			break;
		case 21:
			codec21(_chars[l].src, fobjptr, _chars[l].width, _chars[l].height, _chars[l].width);
			break;
		case 44:
			codec44(_chars[l].src, fobjptr, _chars[l].width, _chars[l].height, _chars[l].width);
			break;
		default:
			error("Rebel2NutRenderer::loadRebel2Font: unknown codec: %d", codec);
		}
	}

	delete[] dataSrc;
}

void Rebel2NutRenderer::decodeRebel2Frame(byte *dst, const Rebel2NutFrameInfo &frame, byte *codec45Palette, byte *codec45Lookup) {
	switch (frame.codec) {
	case 1:
	case 3:
		codec1(dst, frame.data, frame.width, frame.height, frame.width);
		break;
	case 20:
		decodeRebel2RawSprite(dst, frame.data, frame.width, frame.height, frame.dataSize);
		break;
	case 21:
		smushDecodeLineUpdate(dst, frame.data, 0, 0, frame.width, frame.height, frame.width, frame.dataSize);
		break;
	case 23:
		break;
	case 44:
		codec44(dst, frame.data, frame.width, frame.height, frame.width);
		break;
	case 45:
		smushDecodeRA2Blur(dst, frame.data, 0, 0, frame.width, frame.height, frame.width, frame.dataSize,
			codec45Palette, codec45Lookup);
		break;
	default:
		warning("Rebel2NutRenderer::loadRebel2SpriteFromData: unknown codec: %d", frame.codec);
		break;
	}
}

void Rebel2NutRenderer::loadRebel2SpriteFromData(const byte *data, int32 dataSize) {
	clearSpriteData();
	clearCodec23Table();
	clearCodec45Tables();

	if (!data || dataSize < 16) {
		warning("Rebel2NutRenderer::loadRebel2SpriteFromData: data too small (%d bytes)", dataSize);
		return;
	}

	if (READ_BE_UINT32(data) != MKTAG('A','N','I','M')) {
		warning("Rebel2NutRenderer::loadRebel2SpriteFromData: no ANIM chunk");
		return;
	}

	uint32 length = READ_BE_UINT32(data + 4);
	if (length > (uint32)(dataSize - 8)) {
		warning("Rebel2NutRenderer::loadRebel2SpriteFromData: ANIM size (%u) exceeds data size (%d)", length, dataSize);
		length = dataSize - 8;
	}

	const int64 animEnd = 8 + (int64)length;
	if (animEnd < 16 || READ_BE_UINT32(data + 8) != MKTAG('A','H','D','R')) {
		warning("Rebel2NutRenderer::loadRebel2SpriteFromData: no AHDR chunk in font data");
		return;
	}

	int declaredChars = READ_LE_UINT16(data + 18);
	if (declaredChars > (int)ARRAYSIZE(_chars)) {
		warning("Rebel2NutRenderer::loadRebel2SpriteFromData: numChars (%d) exceeds max, clamping", declaredChars);
		declaredChars = ARRAYSIZE(_chars);
	}

	_spriteData = new byte[dataSize];
	memcpy(_spriteData, data, dataSize);
	data = _spriteData;

	Rebel2NutFrameInfo frames[ARRAYSIZE(_chars)];
	uint64 decodedLength = 0;
	int frameCount = 0;

	for (int64 offset = 8; offset + 8 <= animEnd && frameCount < declaredChars;) {
		const uint32 tag = READ_BE_UINT32(data + offset);
		const uint32 chunkSize = READ_BE_UINT32(data + offset + 4);
		const int64 chunkDataStart = offset + 8;
		if ((int64)chunkSize > animEnd - chunkDataStart) {
			warning("Rebel2NutRenderer::loadRebel2SpriteFromData: truncated chunk 0x%08x at offset %llx", tag, (long long)offset);
			break;
		}

		const int64 chunkDataEnd = chunkDataStart + chunkSize;
		int64 nextChunk = chunkDataEnd + (chunkSize & 1);
		if (nextChunk > animEnd)
			nextChunk = chunkDataEnd;

		if (tag == MKTAG('F','R','M','E')) {
			Rebel2NutFrameInfo &frame = frames[frameCount];
			if (chunkSize >= 8 && chunkDataStart + 8 <= chunkDataEnd &&
					READ_BE_UINT32(data + chunkDataStart) == MKTAG('F','O','B','J')) {
				const uint32 fobjSize = READ_BE_UINT32(data + chunkDataStart + 4);
				const int64 fobjDataStart = chunkDataStart + 8;
				const int64 fobjDataEnd = fobjDataStart + fobjSize;

				if (fobjSize >= 14 && fobjDataEnd <= chunkDataEnd) {
					frame.codec = READ_LE_UINT16(data + fobjDataStart);
					frame.xoffs = READ_LE_INT16(data + fobjDataStart + 2);
					frame.yoffs = READ_LE_INT16(data + fobjDataStart + 4);
					frame.width = READ_LE_UINT16(data + fobjDataStart + 6);
					frame.height = READ_LE_UINT16(data + fobjDataStart + 8);
					frame.effect = READ_LE_UINT16(data + fobjDataStart + 12);
					frame.data = data + fobjDataStart + 14;
					frame.dataSize = fobjSize - 14;

					if (frame.codec == 23 && frame.effect == 0x100 && frame.dataSize >= 256) {
						memcpy(_codec23Lookup, frame.data, sizeof(_codec23Lookup));
					}

					const uint64 pixels = (uint64)frame.width * frame.height;
					if (pixels == 0) {
						frame.width = 0;
						frame.height = 0;
						frame.data = nullptr;
						frame.dataSize = 0;
					} else if (pixels > kRebel2MaxSpritePixels || decodedLength + pixels > kRebel2MaxDecodedSpriteBytes) {
						warning("Rebel2NutRenderer::loadRebel2SpriteFromData: invalid sprite dimensions %ux%u at frame %d",
							frame.width, frame.height, frameCount);
						frame.width = 0;
						frame.height = 0;
						frame.data = nullptr;
						frame.dataSize = 0;
					} else {
						decodedLength += pixels;
					}
				}
			}
			frameCount++;
		}

		offset = nextChunk;
	}

	_numChars = frameCount;
	_spriteFrameCount = frameCount;
	if (_numChars <= 0) {
		warning("Rebel2NutRenderer::loadRebel2SpriteFromData: no decodable frames");
		return;
	}

	for (int i = 0; i < _spriteFrameCount; ++i)
		_spriteFrames[i] = frames[i];

	delete[] _decodedData;
	_decodedData = decodedLength ? new byte[(uint32)decodedLength] : nullptr;
	memset(_chars, 0, sizeof(_chars));
	_fontHeight = 0;

	byte *decodedPtr = _decodedData;

	for (int i = 0; i < _numChars; i++) {
		const Rebel2NutFrameInfo &frame = frames[i];
		_chars[i].xoffs = frame.xoffs;
		_chars[i].yoffs = frame.yoffs;
		_chars[i].width = frame.width;
		_chars[i].height = frame.height;
		_chars[i].transparency = kDefaultTransparentColor;

		if (frame.width == 0 || frame.height == 0 || frame.data == nullptr)
			continue;

		const uint32 pixels = frame.width * frame.height;
		_chars[i].src = decodedPtr;
		decodedPtr += pixels;
		_fontHeight = MAX<int>(_fontHeight, frame.height);

		memset(_chars[i].src, kDefaultTransparentColor, pixels);
		decodeRebel2Frame(_chars[i].src, frame, _codec45Palette, _codec45Lookup);
	}

	debugC(DEBUG_SMUSH, "Rebel2NutRenderer::loadRebel2SpriteFromData() - numChars=%d decodedLength=%u", _numChars, (uint32)decodedLength);
}

NutRenderer *makeRebel2Font(ScummEngine *vm, const char *filename) {
	return new Rebel2NutRenderer(vm, filename);
}

NutRenderer *makeRebel2SpriteFromData(ScummEngine *vm, const byte *data, int32 dataSize) {
	Rebel2NutRenderer *renderer = new Rebel2NutRenderer(vm, data, dataSize);
	if (renderer->getNumChars() <= 0) {
		delete renderer;
		return nullptr;
	}

	return renderer;
}

bool drawRebel2Codec45Sprite(NutRenderer *sprite, byte *buffer, int pitch, int width, int height,
		const Common::Rect &clipRect, int x, int y, int spriteIdx, int scale) {
	Rebel2NutRenderer *renderer = dynamic_cast<Rebel2NutRenderer *>(sprite);
	return renderer && renderer->drawCodec45Sprite(buffer, pitch, width, height, clipRect, x, y, spriteIdx, scale);
}

bool drawRebel2Codec23Sprite(NutRenderer *sprite, byte *buffer, int pitch, int width, int height,
		const Common::Rect &clipRect, int x, int y, int spriteIdx, int scale) {
	Rebel2NutRenderer *renderer = dynamic_cast<Rebel2NutRenderer *>(sprite);
	return renderer && renderer->drawCodec23Sprite(buffer, pitch, width, height, clipRect, x, y, spriteIdx, scale);
}

Rebel2FontSet::Rebel2FontSet() : numFonts(0), defaultFont(0) {
	memset(fonts, 0, sizeof(fonts));
}

NutRenderer *Rebel2FontSet::getFont(int id) const {
	if (numFonts <= 0)
		return nullptr;

	if (id < 0 || id >= numFonts || fonts[id] == nullptr)
		id = defaultFont;

	if (id < 0 || id >= numFonts)
		return nullptr;

	return fonts[id] ? fonts[id] : fonts[0];
}

static bool parseRebel2FormatCode(const char *str, uint len, uint &pos, int &fontId, int16 &color) {
	if (pos + 1 >= len || str[pos] != '^')
		return false;

	const char code = str[pos + 1];
	if (code == '^') {
		pos++;
		return false;
	}

	if (code == 'f') {
		pos += 2;
		int value = 0;
		while (pos < len && str[pos] >= '0' && str[pos] <= '9') {
			value = value * 10 + str[pos] - '0';
			pos++;
		}
		fontId = value;
		return true;
	}

	if (code == 'c') {
		pos += 2;
		int value = 0;
		while (pos < len && str[pos] >= '0' && str[pos] <= '9') {
			value = value * 10 + str[pos] - '0';
			pos++;
		}
		color = value;
		return true;
	}

	if (code == 'l') {
		pos += 2;
		return true;
	}

	return false;
}

int drawRebel2Char(NutRenderer *font, byte *buffer, Common::Rect &clipRect, int x, int y,
		int pitch, int16 col, byte chr) {
	if (!font || chr >= font->getNumChars())
		return 0;

	const int charWidth = font->getCharWidth(chr);
	const int charHeight = font->getCharHeight(chr);
	int width = MIN(charWidth, clipRect.right - x);
	int height = MIN(charHeight, clipRect.bottom - y);
	const int minX = x < clipRect.left ? clipRect.left - x : 0;
	const int minY = y < clipRect.top ? clipRect.top - y : 0;

	if (width <= 0 || height <= 0)
		return 0;

	width -= minX;
	height -= minY;
	if (width <= 0 || height <= 0)
		return 0;

	const byte *src = font->getCharData(chr) + minY * charWidth + minX;
	byte *dst = buffer + pitch * (y + minY) + x + minX;
	const int color = (col != -1) ? col : 1;

	for (int row = 0; row < height; row++) {
		for (int column = 0; column < width; column++) {
			const byte value = src[column];
			if (value != 0 && value != 0xff)
				dst[column] = (byte)(value + color - 1);
		}
		src += charWidth;
		dst += pitch;
	}

	return MIN(charWidth, clipRect.right - x);
}

int getRebel2StringWidth(const Rebel2FontSet &fontSet, const char *str, uint len) {
	int width = 0;
	int fontId = fontSet.defaultFont;
	int16 color = 0;

	for (uint pos = 0; pos < len;) {
		if (parseRebel2FormatCode(str, len, pos, fontId, color))
			continue;

		const byte chr = (byte)str[pos++];
		if (chr == '\n' || chr == '\r')
			continue;

		NutRenderer *font = fontSet.getFont(fontId);
		if (font && chr < font->getNumChars())
			width += font->getCharWidth(chr);
	}

	return width;
}

int getRebel2StringHeight(const Rebel2FontSet &fontSet, const char *str, uint len) {
	int height = 0;
	int lineHeight = 0;
	int fontId = fontSet.defaultFont;
	int16 color = 0;

	for (uint pos = 0; pos < len;) {
		if (parseRebel2FormatCode(str, len, pos, fontId, color))
			continue;

		const byte chr = (byte)str[pos++];
		if (chr == '\n') {
			NutRenderer *font = fontSet.getFont(fontId);
			height += lineHeight ? lineHeight : (font ? font->getFontHeight() : 0);
			lineHeight = 0;
		} else if (chr != '\r') {
			NutRenderer *font = fontSet.getFont(fontId);
			if (font && chr < font->getNumChars())
				lineHeight = MAX<int>(lineHeight, font->getCharHeight(chr));
		}
	}

	NutRenderer *font = fontSet.getFont(fontId);
	return height + (lineHeight ? lineHeight : (font ? font->getFontHeight() : 0));
}

static void drawRebel2Substring(const Rebel2FontSet &fontSet, const char *str, uint len,
		byte *buffer, Common::Rect &clipRect, int x, int y, int pitch,
		int &fontId, int16 &color) {
	for (uint pos = 0; pos < len;) {
		if (parseRebel2FormatCode(str, len, pos, fontId, color))
			continue;

		const byte chr = (byte)str[pos++];
		if (chr == '\n' || chr == '\r')
			continue;

		NutRenderer *font = fontSet.getFont(fontId);
		x += drawRebel2Char(font, buffer, clipRect, x, y, pitch, color, chr);
	}
}

void drawRebel2String(const Rebel2FontSet &fontSet, const char *str, uint len, byte *buffer,
		Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags) {
	if (!str || !buffer)
		return;

	int lineStart = 0;
	int maxWidth = 0;
	int yStart = y;
	int fontId = fontSet.defaultFont;
	int16 color = col;

	for (uint pos = 0; pos <= len; pos++) {
		if (pos < len && str[pos] != '\n')
			continue;

		const int lineLen = pos - lineStart;
		const int lineWidth = getRebel2StringWidth(fontSet, str + lineStart, lineLen);
		const int lineHeight = getRebel2StringHeight(fontSet, str + lineStart, lineLen);
		maxWidth = MAX(maxWidth, lineWidth);

		int drawX = x;
		if (flags & kStyleAlignCenter)
			drawX = x - lineWidth / 2;
		else if (flags & kStyleAlignRight)
			drawX = x - lineWidth;

		drawRebel2Substring(fontSet, str + lineStart, lineLen, buffer, clipRect, drawX, y, pitch, fontId, color);
		y += lineHeight;
		lineStart = pos + 1;
	}

	clipRect.left = MAX<int>(0, (flags & kStyleAlignCenter) ? x - maxWidth / 2 : ((flags & kStyleAlignRight) ? x - maxWidth : x));
	clipRect.right = MIN<int>(clipRect.right, clipRect.left + maxWidth);
	clipRect.top = yStart;
	clipRect.bottom = y;
}

void drawRebel2StringWrap(const Rebel2FontSet &fontSet, const char *str, uint len, byte *buffer,
		Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags) {
	if (!str || !buffer)
		return;

	int16 substrByteLength[kRebel2MaxStrings];
	int16 substrWidths[kRebel2MaxStrings];
	int16 substrStart[kRebel2MaxStrings];
	memset(substrByteLength, 0, sizeof(substrByteLength));
	memset(substrWidths, 0, sizeof(substrWidths));
	memset(substrStart, 0, sizeof(substrStart));

	int16 numSubstrings = 0;
	int curWidth = 0;
	int curPos = -1;
	int maxWidth = 0;
	int height = 0;
	int lastSubstrHeight = 0;

	while (curPos < (int)len) {
		int textStart = curPos + 1;
		while (textStart < (int)len && str[textStart] == ' ')
			textStart++;

		const int separatorWidth = curPos > 0 ? getRebel2StringWidth(fontSet, str + curPos, textStart - curPos) : 0;

		int nextSeparatorPos = textStart;
		while (nextSeparatorPos < (int)len && str[nextSeparatorPos] != ' ' && str[nextSeparatorPos] != '\n')
			nextSeparatorPos++;

		const int wordWidth = getRebel2StringWidth(fontSet, str + textStart, nextSeparatorPos - textStart);
		int newWidth = curWidth + separatorWidth + wordWidth;

		if (curWidth && newWidth > clipRect.width()) {
			if (numSubstrings < kRebel2MaxStrings) {
				substrWidths[numSubstrings] = curWidth;
				substrByteLength[numSubstrings] = curPos - substrStart[numSubstrings];
				numSubstrings++;
			}
			newWidth = wordWidth;
			substrStart[numSubstrings] = textStart;
		}
		curWidth = newWidth;

		curPos = nextSeparatorPos;
		if (curPos >= (int)len || str[curPos] == '\n') {
			if (numSubstrings < kRebel2MaxStrings) {
				substrWidths[numSubstrings] = curWidth;
				substrByteLength[numSubstrings] = curPos - substrStart[numSubstrings];
				numSubstrings++;
				if (numSubstrings < kRebel2MaxStrings)
					substrStart[numSubstrings] = curPos + 1;
			}
			curWidth = 0;
		}
	}

	if (curWidth && numSubstrings < kRebel2MaxStrings) {
		substrWidths[numSubstrings] = curWidth;
		substrByteLength[numSubstrings] = curPos - substrStart[numSubstrings];
		numSubstrings++;
	}

	for (int i = 0; i < numSubstrings; i++) {
		maxWidth = MAX<int>(maxWidth, substrWidths[i]);
		lastSubstrHeight = substrByteLength[i] > 0 ? getRebel2StringHeight(fontSet, str + substrStart[i], substrByteLength[i]) : 0;
		height += lastSubstrHeight;
	}

	const int clipHeight = height + lastSubstrHeight / 2;
	if (y > clipRect.bottom - clipHeight)
		y = clipRect.bottom - clipHeight;
	if (y < clipRect.top)
		y = clipRect.top;

	if (flags & kStyleAlignCenter) {
		if (x + (maxWidth >> 1) > clipRect.right)
			x = clipRect.right - (maxWidth >> 1);
		if (x - (maxWidth >> 1) < clipRect.left)
			x = clipRect.left + (maxWidth >> 1);
	} else if (flags & kStyleAlignRight) {
		if (x > clipRect.right)
			x = clipRect.right;
		if (x < clipRect.left + maxWidth)
			x = clipRect.left + maxWidth;
	} else {
		if (x > clipRect.right - maxWidth)
			x = clipRect.right - maxWidth;
		if (x < clipRect.left)
			x = clipRect.left;
	}

	const int yStart = y;
	int fontId = fontSet.defaultFont;
	int16 color = col;

	for (int i = 0; i < numSubstrings; i++) {
		int drawX = x;
		if (flags & kStyleAlignCenter)
			drawX = x - substrWidths[i] / 2;
		else if (flags & kStyleAlignRight)
			drawX = x - substrWidths[i];

		const int lineLen = substrByteLength[i] > 0 ? substrByteLength[i] : 0;
		drawRebel2Substring(fontSet, str + substrStart[i], lineLen, buffer, clipRect, drawX, y, pitch, fontId, color);
		y += getRebel2StringHeight(fontSet, str + substrStart[i], lineLen);
	}

	clipRect.left = MAX<int>(0, (flags & kStyleAlignCenter) ? x - maxWidth / 2 : ((flags & kStyleAlignRight) ? x - maxWidth : x));
	clipRect.right = MIN<int>(clipRect.right, clipRect.left + maxWidth);
	clipRect.top = yStart;
	clipRect.bottom = y;
}

} // End of namespace Scumm
