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

#include "comfy/comfy.h"

#include "common/endian.h"

namespace Comfy {

bool ComfyEngine::spriteDecompressTile(SpriteResource &sprite, const byte *source, uint32 sourceSize) {
	uint32 outputSize = sprite.header.tiledSize;
	if (!source || !sourceSize || !sprite.header.width || !sprite.header.height || !outputSize)
		return false;

	sprite.pixels.resize(outputSize);
	uint32 sourcePos = 0;
	uint32 outputPos = 0;
	uint16 skipCount = 0;
	uint16 fillCount = 0;
	uint16 literalCount = 0;
	byte fillValue = 0;
	bool literalFromFill = false;

	for (uint row = 0; row < sprite.header.height; row++) {
		if (outputPos + 2 > outputSize)
			return false;

		uint32 rowStart = outputPos;
		outputPos += 2;
		uint16 pixelsLeft = sprite.header.width;
		uint32 literalPacketPos = 0;
		uint16 literalPacketLength = 0;

		while (pixelsLeft) {
			if (!skipCount && !fillCount && !literalCount) {
				if (sourcePos >= sourceSize)
					return false;

				byte op = source[sourcePos++];
				if (op & 0x40) {
					skipCount = op - 0x40;
					while (sourcePos < sourceSize && (source[sourcePos] & 0x40))
						skipCount += source[sourcePos++] - 0x40;
				} else if (op & 0x80) {
					fillCount = op - 0x80;
					if (sourcePos >= sourceSize)
						return false;

					fillValue = source[sourcePos++];
					while (sourcePos < sourceSize && (source[sourcePos] & 0xC0) == 0xC0)
						fillCount += source[sourcePos++] - 0xC0;
				} else {
					literalCount = op;
					literalFromFill = false;
				}

				continue;
			}

			if (literalCount) {
				if (!literalPacketLength || literalPacketLength == 0x3F) {
					if (outputPos >= outputSize)
						return false;

					literalPacketPos = outputPos++;
					literalPacketLength = 0;
				}

				uint16 count = MIN<uint16>(pixelsLeft, literalCount);
				count = MIN<uint16>(count, 0x3F - literalPacketLength);
				if (outputPos + count > outputSize || (!literalFromFill && sourcePos + count > sourceSize))
					return false;

				if (literalFromFill)
					memset(&sprite.pixels[outputPos], fillValue, count);
				else {
					memcpy(&sprite.pixels[outputPos], source + sourcePos, count);
					sourcePos += count;
				}

				outputPos += count;
				pixelsLeft -= count;
				literalCount -= count;
				literalPacketLength += count;
				sprite.pixels[literalPacketPos] = literalPacketLength;
			} else if (skipCount) {
				uint16 count = MIN<uint16>(pixelsLeft, skipCount);
				count = MIN<uint16>(count, 0x3F);
				if (outputPos >= outputSize)
					return false;

				sprite.pixels[outputPos++] = count + 0x40;
				pixelsLeft -= count;
				skipCount -= count;
				literalPacketLength = 0;
			} else {
				uint16 count = MIN<uint16>(pixelsLeft, fillCount);
				count = MIN<uint16>(count, 0x3F);
				fillCount -= count;
				if (count < 8) {
					literalCount = count;
					literalFromFill = true;
					continue;
				}

				if (outputPos + 2 > outputSize)
					return false;

				sprite.pixels[outputPos++] = count + 0x80;
				sprite.pixels[outputPos++] = fillValue;
				pixelsLeft -= count;
				literalPacketLength = 0;
			}
		}

		WRITE_LE_UINT16(&sprite.pixels[rowStart], outputPos - rowStart);
	}

	return outputPos == outputSize;
}

bool ComfyEngine::spriteLoad(uint16 spriteId) {
	if (spriteId >= _spriteResources.size())
		return false;

	SpriteResource &sprite = _spriteResources[spriteId];
	if (sprite.loaded)
		return true;

	SpriteObjectHeader &header = sprite.header;
	if (!header.dataSize || header.fileOffset > _picFileData.size() ||
			header.dataSize > _picFileData.size() - header.fileOffset)
		return false;

	byte *source = &_picFileData[header.fileOffset];
	if (header.width == _logicalScreenWidth && header.height == _logicalScreenHeight) {
		sprite.pixels.resize(header.dataSize);
		memcpy(&sprite.pixels[0], source, header.dataSize);
	} else if (!spriteDecompressTile(sprite, source, header.dataSize)) {
		sprite.pixels.clear();
		return false;
	}

	sprite.loaded = true;
	return true;
}

void ComfyEngine::spriteBlitRle(const byte *source, uint32 sourceSize) {
	if (!_framebufPtr || !source)
		return;

	uint32 sourcePos = 0;
	uint32 outputPos = 0;
	uint32 outputSize = framebufferBytes();
	byte previousFill = 0;
	while (outputPos < outputSize && sourcePos < sourceSize) {
		byte op = source[sourcePos++];
		uint32 count = MIN<uint32>(op & 0x3F, outputSize - outputPos);
		if ((op & 0xC0) == 0) {
			if (sourcePos + count > sourceSize)
				return;

			memcpy(_framebufPtr + outputPos, source + sourcePos, count);
			sourcePos += count;
		} else if ((op & 0xC0) == 0xC0) {
			memset(_framebufPtr + outputPos, previousFill, count);
		} else {
			if (sourcePos >= sourceSize)
				return;

			previousFill = source[sourcePos++];
			memset(_framebufPtr + outputPos, previousFill, count);
		}

		outputPos += count;
	}
}

void ComfyEngine::spriteBlitClipped(uint16 spriteId, int16 x, int16 y) {
	if (!spriteLoad(spriteId))
		return;

	SpriteResource &sprite = _spriteResources[spriteId];
	if (sprite.header.width == _logicalScreenWidth && sprite.header.height == _logicalScreenHeight) {
		spriteBlitRle(&sprite.pixels[0], sprite.pixels.size());
		renderSetDirty();
		return;
	}

	uint32 rowPos = 0;
	for (uint row = 0; row < sprite.header.height && rowPos + 2 <= sprite.pixels.size(); row++) {
		uint16 rowSize = READ_LE_UINT16(&sprite.pixels[rowPos]);
		if (rowSize < 2 || rowPos + rowSize > sprite.pixels.size())
			return;

		int16 drawY = y + row;
		uint32 packetPos = rowPos + 2;
		int16 drawX = x;
		while (packetPos < rowPos + rowSize) {
			byte op = sprite.pixels[packetPos++];
			uint16 count = op & 0x3F;
			if (op & 0x80) {
				if (packetPos >= rowPos + rowSize)
					return;

				byte value = sprite.pixels[packetPos++];
				for (uint i = 0; i < count; i++, drawX++) {
					if (drawX >= 0 && drawX < int16(_logicalScreenWidth) && drawY >= 0 && drawY < int16(_logicalScreenHeight))
						_framebufPtr[drawY * _logicalScreenWidth + drawX] = value;
				}
			} else if (op & 0x40) {
				drawX += count;
			} else {
				if (packetPos + count > rowPos + rowSize)
					return;

				for (uint i = 0; i < count; i++, drawX++) {
					byte value = sprite.pixels[packetPos++];
					if (drawX >= 0 && drawX < int16(_logicalScreenWidth) && drawY >= 0 && drawY < int16(_logicalScreenHeight))
						_framebufPtr[drawY * _logicalScreenWidth + drawX] = value;
				}
			}
		}

		rowPos += rowSize;
	}

	renderSetDirty();
}


} // End of namespace Comfy
