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
	_spriteDecompressLastSize = 0;
	_spriteDecompressLastComplete = false;
	if (!source || !sourceSize || !sprite.header.width || !sprite.header.height || !outputSize)
		return false;

	// The original temporarily writes a zero sentinel at source[dataSize].
	Common::Array<byte> sourceWithSentinel;
	sourceWithSentinel.resize(sourceSize + 1);
	memcpy(&sourceWithSentinel[0], source, sourceSize);
	sourceWithSentinel[sourceSize] = 0;
	source = &sourceWithSentinel[0];
	uint32 sourceEnd = sourceSize + 1;

	sprite.pixels.resize(outputSize);
	uint32 sourcePos = 0;
	uint32 outputPos = 0;
	uint16 totalOutput = 0;
	uint16 skipCount = 0;
	uint16 fillCount = 0;
	uint16 literalCount = 0;
	byte fillValue = 0;
	bool literalFromFill = false;

	for (uint row = 0; row < sprite.header.height; row++) {
		if (outputPos + 2 > outputSize)
			break;

		uint32 rowStart = outputPos;
		outputPos += 2;
		uint16 pixelsLeft = sprite.header.width;
		uint32 literalPacketPos = 0;
		uint16 literalPacketLength = 0;

		while (pixelsLeft) {
			if (!skipCount && !fillCount && !literalCount) {
				byte op = sourcePos < sourceEnd ? source[sourcePos++] : 0;
				if (op & 0x40) {
					skipCount = op - 0x40;
					while (sourcePos < sourceEnd && (source[sourcePos] & 0x40))
						skipCount += source[sourcePos++] - 0x40;
				} else if (op & 0x80) {
					fillCount = op - 0x80;
					fillValue = sourcePos < sourceEnd ? source[sourcePos++] : 0;
					while (sourcePos < sourceEnd && (source[sourcePos] & 0xC0) == 0xC0)
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
				if (outputPos + count > outputSize)
					count = outputSize - outputPos;

				if (literalFromFill) {
					memset(&sprite.pixels[outputPos], fillValue, count);
				} else {
					if (count > sourceEnd - sourcePos)
						count = sourceEnd - sourcePos;

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

		uint16 rowSize = outputPos - rowStart;
		WRITE_LE_UINT16(&sprite.pixels[rowStart], rowSize);
		totalOutput += rowSize;
		if (totalOutput > outputSize)
			break;
	}

	_spriteDecompressLastSize = totalOutput;
	_spriteDecompressLastComplete = totalOutput == outputSize;
	return _spriteDecompressLastComplete;
}

void ComfyEngine::spriteInvalidateHostCache(SpriteResource &sprite) {
	sprite.loaded = false;
	sprite.pixels.clear();
}

void ComfyEngine::objHdrReadFromXms(byte *destination, uint32 base, uint16 size, uint16 row) {
	if (!destination)
		return;

	uint32 offset = base + (uint32)size * row;
	if (offset > _headerXmsData.size() || size > _headerXmsData.size() - offset)
		error("Headers move (XMS->memory)");

	memcpy(destination, &_headerXmsData[offset], size);
}

void ComfyEngine::objHdrRead(SpriteObjectHeader &destination, uint16 index) {
	byte raw[0x11];
	memset(raw, 0, sizeof(raw));
	objHdrReadFromXms(raw, _headerXmsObjectTableBase, sizeof(raw), index);
	destination.fileOffset = READ_LE_UINT32(raw);
	destination.dataSize = READ_LE_UINT16(raw + 0x04);
	destination.width = READ_LE_UINT16(raw + 0x06);
	destination.height = READ_LE_UINT16(raw + 0x08);
	destination.hotspotX = (int16)READ_LE_UINT16(raw + 0x0A);
	destination.hotspotY = (int16)READ_LE_UINT16(raw + 0x0C);
	destination.reserved = raw[0x0E];
	destination.tiledSize = READ_LE_UINT16(raw + 0x0F);
	_spriteLastHeader = destination;
}

void ComfyEngine::scenePoolEvict() {
	if (_scenePoolEvictCursor >= _scenePoolData.size())
		return;

	byte *entry = &_scenePoolData[_scenePoolEvictCursor];
	int16 id = (int16)READ_LE_UINT16(entry);
	uint16 slotSize = READ_LE_UINT16(entry + 2);
	if (id > 0) {
		if ((uint16)id < _spriteResources.size())
			spriteInvalidateHostCache(_spriteResources[id]);

		if ((uint16)id < _objectCacheEntries.size())
			_objectCacheEntries[id].slotSize = 0xFFFF;
	} else {
		uint16 frameId = (uint16)-id;
		if (_frameSpriteResource.id == frameId)
			spriteInvalidateHostCache(_frameSpriteResource);

		if (frameId < _frameCacheEntries.size())
			_frameCacheEntries[frameId].slotSize = 0xFFFF;
	}

	_scenePoolEvictCursor += slotSize;
	if (_scenePoolEvictCursor < _scenePoolData.size() &&
			READ_LE_UINT16(&_scenePoolData[_scenePoolEvictCursor]) == 0x8000)
		_scenePoolEvictCursor = 0;
}

void ComfyEngine::scenePoolReserveSlot(uint32 size) {
	if (!_scenePoolSize || size >= _scenePoolSize)
		return;

	for (;;) {
		if (_scenePoolCursor >= _scenePoolEvictCursor) {
			uint32 tailFree = _scenePoolSize - _scenePoolCursor;
			if (tailFree > size)
				return;

			if (!_scenePoolEvictCursor)
				scenePoolEvict();
			else
				_scenePoolCursor = 0;
		} else {
			uint32 gapFree = _scenePoolEvictCursor - _scenePoolCursor;
			if (gapFree > size)
				return;

			scenePoolEvict();
		}
	}
}

void ComfyEngine::spriteCache(int16 spriteId) {
	soundServiceWaveBuffers();

	if (!spriteId)
		return;

	if (spriteId < 0) {
		uint16 frameId = (uint16)-spriteId;
		uint32 fileOffset = (uint32)(frameId - 1) * COMFY_TILE_SIZE;
		if (fileOffset >= _picDataSize || fileOffset >= _comfyObjData.size())
			return;

		uint32 size = MIN<uint32>(COMFY_TILE_SIZE,
			MIN<uint32>(_picDataSize - fileOffset, _comfyObjData.size() - fileOffset));
		uint16 slotSize = (uint16)(size + (size & 1) + 0x0C);
		scenePoolReserveSlot((uint32)slotSize + 2);
		if (_scenePoolCursor > _scenePoolData.size() || slotSize > _scenePoolData.size() - _scenePoolCursor)
			return;

		byte *entryData = &_scenePoolData[_scenePoolCursor];
		WRITE_LE_UINT16(entryData, (uint16)spriteId);
		WRITE_LE_UINT16(entryData + 2, slotSize);
		memcpy(entryData + 0x0C, &_comfyObjData[fileOffset], size);
		_frameSpriteResource.id = frameId;
		_frameSpriteResource.header = SpriteObjectHeader();
		_frameSpriteResource.header.dataSize = size;
		_frameSpriteResource.header.tiledSize = size;
		_frameSpriteResource.pixels.resize(size);
		memcpy(&_frameSpriteResource.pixels[0], entryData + 0x0C, size);
		_frameSpriteResource.loaded = true;
		if (frameId < _frameCacheEntries.size()) {
			SpriteCacheEntry &entry = _frameCacheEntries[frameId];
			entry.poolOffset = _scenePoolCursor;
			entry.slotSize = slotSize;
		}

		_scenePoolCursor += slotSize;
		if (_scenePoolCursor < _scenePoolData.size())
			WRITE_LE_UINT16(&_scenePoolData[_scenePoolCursor], 0x8000);

		return;
	}

	if ((uint16)spriteId >= _spriteResources.size())
		return;

	SpriteResource &sprite = _spriteResources[spriteId];
	if (sprite.loaded && _objectCacheEntries[spriteId].slotSize != 0xFFFF)
		return;

	spriteInvalidateHostCache(sprite);

	SpriteObjectHeader header;
	objHdrRead(header, spriteId);
	sprite.header = header;
	if (!header.dataSize || header.fileOffset > _picFileData.size() ||
			header.dataSize > _picFileData.size() - header.fileOffset)
		return;

	byte *source = &_picFileData[header.fileOffset];
	if (header.width == _logicalScreenWidth && header.height == _logicalScreenHeight) {
		sprite.pixels.resize(header.dataSize);
		memcpy(&sprite.pixels[0], source, header.dataSize);
	} else {
		spriteDecompressTile(sprite, source, header.dataSize);
	}

	uint16 payloadSize = sprite.pixels.size() + (sprite.pixels.size() & 1);
	uint16 slotSize = payloadSize + 0x0C;
	scenePoolReserveSlot((uint32)slotSize + 2);
	if (_scenePoolCursor > _scenePoolData.size() || slotSize > _scenePoolData.size() - _scenePoolCursor)
		return;

	byte *entryData = &_scenePoolData[_scenePoolCursor];
	WRITE_LE_UINT16(entryData, (uint16)spriteId);
	WRITE_LE_UINT16(entryData + 2, slotSize);
	WRITE_LE_UINT16(entryData + 4, header.width);
	WRITE_LE_UINT16(entryData + 6, header.height);
	WRITE_LE_UINT16(entryData + 8, (uint16)header.hotspotX);
	WRITE_LE_UINT16(entryData + 0x0A, (uint16)header.hotspotY);
	memcpy(entryData + 0x0C, &sprite.pixels[0], sprite.pixels.size());
	SpriteCacheEntry &entry = _objectCacheEntries[spriteId];
	entry.poolOffset = _scenePoolCursor;
	entry.slotSize = slotSize;
	_scenePoolCursor += slotSize;
	if (_scenePoolCursor < _scenePoolData.size())
		WRITE_LE_UINT16(&_scenePoolData[_scenePoolCursor], 0x8000);

	sprite.loaded = true;
}

ComfyEngine::SpriteResource *ComfyEngine::spriteLookup(uint16 spriteId, int16 x, int16 y) {
	if (spriteId >= _spriteResources.size())
		return nullptr;

	SpriteResource &sprite = _spriteResources[spriteId];
	SpriteCacheEntry *entry = spriteId < _objectCacheEntries.size() ? &_objectCacheEntries[spriteId] : nullptr;
	if (entry && entry->slotSize == 0xFFFF) {
		SpriteObjectHeader header;
		objHdrRead(header, spriteId);
		sprite.header = header;
	}

	int16 left = x - sprite.header.hotspotX;
	int16 top = y - sprite.header.hotspotY;
	int16 right = left + (int16)sprite.header.width - 1;
	int16 bottom = top + (int16)sprite.header.height - 1;
	bool fullscreen = sprite.header.width == _logicalScreenWidth && sprite.header.height == _logicalScreenHeight;
	bool offscreen = left > (int16)(_logicalScreenWidth - 1) || right < 0 ||
		top > (int16)(_logicalScreenHeight - 1) || bottom < 0;

	if (entry && entry->slotSize == 0xFFFF && !fullscreen && offscreen) {
		_objFileOffset = _spriteLastHeader.fileOffset;
		_objFileStride = _spriteLastHeader.tiledSize;
		return nullptr;
	}

	if ((entry && entry->slotSize == 0xFFFF) || (!entry && !sprite.loaded))
		spriteCache((int16)spriteId);

	return &sprite;
}

ComfyEngine::SpriteResource *ComfyEngine::spriteGetPtr(int16 spriteId) {
	if (spriteId > 0) {
		if ((uint16)spriteId >= _spriteResources.size())
			return nullptr;

		if (_objectCacheEntries[spriteId].slotSize == 0xFFFF)
			spriteCache(spriteId);

		return &_spriteResources[spriteId];
	}

	if (!spriteId)
		return nullptr;

	uint16 frameId = (uint16)-spriteId;
	if (frameId >= _frameCacheEntries.size() || _frameCacheEntries[frameId].slotSize == 0xFFFF ||
			!_frameSpriteResource.loaded || _frameSpriteResource.id != frameId)
		spriteCache(spriteId);

	return _frameSpriteResource.loaded && _frameSpriteResource.id == frameId ? &_frameSpriteResource : nullptr;
}

void ComfyEngine::spriteGetConvPtr(int16 spriteId) {
	soundServiceWaveBuffers();

	if (spriteId <= 0 || (uint16)spriteId >= _objectCacheEntries.size())
		return;

	if (_spriteConversionLoads.count < COMFY_RESOURCE_LIST_CAPACITY)
		_spriteConversionLoads.ids[_spriteConversionLoads.count++] = (uint16)spriteId;

	SpriteCacheEntry &entry = _objectCacheEntries[spriteId];
	byte *payload = nullptr;
	uint16 width = 0;
	if (entry.slotSize != 0xFFFF) {
		if (entry.poolOffset <= _scenePoolData.size() && entry.slotSize >= 0x0C &&
				0x0C <= _scenePoolData.size() - entry.poolOffset) {
			byte *entryData = &_scenePoolData[entry.poolOffset];
			width = READ_LE_UINT16(entryData + 4);
			payload = entryData + 0x0C;
		}
	} else {
		SpriteObjectHeader header;
		objHdrRead(header, (uint16)spriteId);
		width = header.width;
		if (header.fileOffset <= _picFileData.size() &&
				header.dataSize <= _picFileData.size() - header.fileOffset)
			payload = &_picFileData[header.fileOffset];
	}

	if (!payload || width != 0xFFFF)
		return;

	// The original uploads the sound-cache tile named by the first payload
	// word to XMS. ScummVM already keeps the complete source resident.
}

void ComfyEngine::spriteBlitRle(byte *destination, const byte *source, uint32 sourceSize) {
	if (!destination || !source)
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

			memcpy(destination + outputPos, source + sourcePos, count);
			sourcePos += count;
		} else if ((op & 0xC0) == 0xC0) {
			memset(destination + outputPos, previousFill, count);
		} else {
			if (sourcePos >= sourceSize)
				return;

			previousFill = source[sourcePos++];
			memset(destination + outputPos, previousFill, count);
		}

		outputPos += count;
	}
}

void ComfyEngine::backgroundTransitionFrames(uint16 frame, uint16 previousFrame) {
	if (_isPanther) {
		if (frame == previousFrame || !frame || _backgroundFramebuf.empty())
			return;

		SpriteResource *background = spriteGetPtr((int16)frame);
		if (!background)
			return;

		if (background->header.width == _logicalScreenWidth &&
				background->header.height == _logicalScreenHeight && !background->pixels.empty()) {
			spriteBlitRle(&_backgroundFramebuf[0], &background->pixels[0], background->pixels.size());
		} else {
			uint16 color = background->pixels.size() > 3 ? background->pixels[3] : 0;
			framebufClear(&_backgroundFramebuf[0], color);
		}

		renderInvalidateFullFrame();
		return;
	}

	SpriteResource *background = spriteGetPtr((int16)frame);
	if (!background)
		return;

	if (background->header.width == _logicalScreenWidth &&
			background->header.height == _logicalScreenHeight && !background->pixels.empty()) {
		spriteBlitRle(_framebufPtr, &background->pixels[0], background->pixels.size());
	} else {
		uint16 color = background->pixels.size() > 3 ? background->pixels[3] : 0;
		framebufClear(_framebufPtr, color);
	}

	videoSetResolution();
}

void ComfyEngine::backgroundRestoreDirtyRects() {
	if (_backgroundFramebuf.empty() || !_framebufPtr)
		return;

	for (uint i = 0; i < _dirtyRectCount; i++) {
		ComfyRect &rect = _dirtyRects[i];
		int16 width = rect.right - rect.left;
		int16 height = rect.bottom - rect.top;
		if (width <= 0 || height <= 0)
			continue;

		for (int16 y = rect.top; y < rect.bottom; y++) {
			uint32 offset = (uint32)y * _logicalScreenWidth + rect.left;
			memcpy(_framebufPtr + offset, &_backgroundFramebuf[offset], width);
		}
	}
}

void ComfyEngine::spriteBlitClipped(int16 spriteId, int16 x, int16 y) {
	SpriteResource *loadedSprite = spriteGetPtr(spriteId);
	if (!loadedSprite)
		return;

	SpriteResource &sprite = *loadedSprite;
	if (sprite.header.width == _logicalScreenWidth && sprite.header.height == _logicalScreenHeight) {
		spriteBlitRle(_framebufPtr, &sprite.pixels[0], sprite.pixels.size());
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
					if (drawX >= 0 && drawX < (int16)_logicalScreenWidth && drawY >= 0 && drawY < (int16)_logicalScreenHeight)
						_framebufPtr[drawY * _logicalScreenWidth + drawX] = value;
				}
			} else if (op & 0x40) {
				drawX += count;
			} else {
				if (packetPos + count > rowPos + rowSize)
					return;

				for (uint i = 0; i < count; i++, drawX++) {
					byte value = sprite.pixels[packetPos++];
					if (drawX >= 0 && drawX < (int16)_logicalScreenWidth && drawY >= 0 && drawY < (int16)_logicalScreenHeight)
						_framebufPtr[drawY * _logicalScreenWidth + drawX] = value;
				}
			}
		}

		rowPos += rowSize;
	}

	renderSetDirty();
}

bool ComfyEngine::spriteCoversPoint(int16 spriteId, int16 x, int16 y, int16 pointX, int16 pointY) {
	SpriteResource *loadedSprite = spriteGetPtr(spriteId);
	if (!loadedSprite)
		return false;

	SpriteResource &sprite = *loadedSprite;
	if (pointX < x || pointY < y || pointX >= x + (int16)sprite.header.width ||
			pointY >= y + (int16)sprite.header.height)
		return false;

	if (sprite.header.width == _logicalScreenWidth && sprite.header.height == _logicalScreenHeight)
		return true;

	uint32 rowPos = 0;
	for (uint row = 0; row < sprite.header.height && rowPos + 2 <= sprite.pixels.size(); row++) {
		uint16 rowSize = READ_LE_UINT16(&sprite.pixels[rowPos]);
		if (rowSize < 2 || rowPos + rowSize > sprite.pixels.size())
			return false;

		int16 drawY = y + row;
		uint32 packetPos = rowPos + 2;
		int16 drawX = x;
		while (packetPos < rowPos + rowSize) {
			byte op = sprite.pixels[packetPos++];
			uint16 count = op & 0x3F;
			if (op & 0x80) {
				if (packetPos >= rowPos + rowSize)
					return false;

				packetPos++;
				if (drawY == pointY && pointX >= drawX && pointX < drawX + (int16)count)
					return true;

				drawX += count;
			} else if (op & 0x40) {
				drawX += count;
			} else {
				if (packetPos + count > rowPos + rowSize)
					return false;

				if (drawY == pointY && pointX >= drawX && pointX < drawX + (int16)count)
					return true;

				packetPos += count;
				drawX += count;
			}
		}

		rowPos += rowSize;
	}

	return false;
}

ComfyEngine::Actor *ComfyEngine::actorGetPtr(uint16 actorIndex) {
	return actorIndex < _actors.size() ? &_actors[actorIndex] : nullptr;
}

ComfyEngine::Actor *ComfyEngine::rootActor() {
	return !_actors.empty() ? &_actors[0] : nullptr;
}

void ComfyEngine::actorSetFrame(int16 frame) {
	Actor *root = rootActor();
	if (root)
		root->spriteSelector = (uint32)(int32)frame;
}

uint16 ComfyEngine::actorGetFrame() {
	Actor *root = rootActor();
	return root ? (uint16)root->spriteSelector : 0;
}

void ComfyEngine::actorSetPos(uint16 mode, int16 value) {
	Actor *root = rootActor();
	if (!root)
		return;

	switch (mode) {
	case 1:
		root->xFixed = value;
		root->yFixed = 0;
		break;

	case 2:
		root->yFixed = (int32)(((uint32)root->yFixed + (uint32)(int32)value) & 0xFF);
		break;

	default:
		break;
	}
}

ComfyEngine::Actor *ComfyEngine::actorResolve(uint16 sceneOrActor, uint16 fallbackActor) {
	if (!sceneOrActor)
		return actorGetPtr(fallbackActor);

	return actorGetPtr(sceneGetHandle(sceneOrActor));
}

uint16 ComfyEngine::actorAlloc(uint16 sceneSlot) {
	Actor *root = actorGetPtr(0);
	uint16 actorIndex = root ? root->nextLink : 0;
	if (!actorIndex || sceneSlot >= _sceneHandles.size())
		return 0;

	Actor *actor = actorGetPtr(actorIndex);
	root->nextLink = actor->nextLink;
	_sceneHandles[sceneSlot] = actorIndex;
	actor->sceneHandle = sceneSlot;
	return actorIndex;
}

void ComfyEngine::actorFreeSlot(uint16 sceneSlot) {
	if (sceneSlot >= _sceneHandles.size())
		return;

	uint16 actorIndex = _sceneHandles[sceneSlot];
	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return;

	Actor *root = actorGetPtr(0);
	actor->nextLink = root ? root->nextLink : 0;
	actor->sceneHandle = 0;
	if (root)
		root->nextLink = actorIndex;
	_sceneHandles[sceneSlot] = 0;
}

void ComfyEngine::actorInsertChild(uint16 childIndex, uint16 parentIndex) {
	Actor *child = actorGetPtr(childIndex);
	Actor *parent = actorGetPtr(parentIndex);
	if (!child || !parent)
		return;

	uint16 tail = parent->childTail;
	if (tail) {
		actorGetPtr(tail)->nextLink = childIndex;
		child->prevLink = tail;
	} else {
		parent->childHead = childIndex;
		child->prevLink = 0;
	}

	child->nextLink = 0;
	parent->childTail = childIndex;
}

void ComfyEngine::actorInsertSibling(uint16 actorIndex, uint16 ownerIndex) {
	Actor *actor = actorGetPtr(actorIndex);
	Actor *owner = actorGetPtr(ownerIndex);
	if (!actor || !owner)
		return;

	uint16 head = owner->siblingHead;
	if (head)
		actorGetPtr(head)->prevLink = actorIndex;

	actor->nextLink = head;
	actor->prevLink = 0;
	owner->siblingHead = actorIndex;
}

void ComfyEngine::actorUnlink(uint16 actorIndex) {
	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return;

	Actor *parent = actorGetPtr(actor->parent);
	if (!parent)
		return;

	uint16 next = actor->nextLink;
	uint16 previous = actor->prevLink;
	if (next)
		actorGetPtr(next)->prevLink = previous;
	else if (parent->childTail == actorIndex)
		parent->childTail = previous;

	if (previous)
		actorGetPtr(previous)->nextLink = next;
	else if (parent->siblingHead == actorIndex)
		parent->siblingHead = next;
	else
		parent->childHead = next;
}

uint16 ComfyEngine::actorInit(uint16 sceneSlot, uint16 parentSlot, byte visible, byte active,
		uint32 pc, int16 x, int16 y, int16 sprite, byte insertAsChild) {
	uint16 actorIndex = actorAlloc(sceneSlot);
	Actor *actor = actorGetPtr(actorIndex);
	if (!actorIndex || !actor)
		return 8;

	actor->sceneHandle = sceneSlot;
	actor->visible = visible;
	actor->active = active;
	actor->currentPc = pc;
	actor->callPc = 0;
	actor->resetPc = pc;
	actor->xFixed = (int32)x * 0x1000;
	actor->yFixed = (int32)y * 0x1000;
	actor->spriteSelector = (uint32)(int32)sprite;
	actor->childTail = 0;
	actor->childHead = 0;
	actor->siblingHead = 0;
	actor->moveTicks = 0;
	actor->triggerKey = 0;
	actor->waitTarget = 0;
	actor->waitAccum = 0;
	actor->dirty = 1;
	if (_videoMode == 2 || _videoMode == 4) {
		ComfyRect empty;
		actor->cachedRect = empty;
		actor->cachedSprite = 0;
		actor->cachedVisible = 0;
	}

	if (parentSlot) {
		uint16 parentIndex = sceneGetHandle(parentSlot);
		actor->parent = parentIndex;
		if (insertAsChild)
			actorInsertChild(actorIndex, parentIndex);
		else
			actorInsertSibling(actorIndex, parentIndex);
	} else {
		actor->parent = 0;
		actor->nextLink = 0;
		actor->prevLink = 0;
	}

	return 1;
}

void ComfyEngine::actorSetPc(Actor &actor, uint32 pc) {
	uint32 previous = actor.callPc;
	uint32 packed = pc;
	if (previous) {
		byte entry = _actorPcTable[0] >> 24;
		_actorPcTable[0] = _actorPcTable[entry];
		_actorPcTable[entry] = previous;
		if (_sceneActorPcOffset + sizeof(_actorPcTable) <= _sceneMemoryBlock.size()) {
			WRITE_LE_UINT32(&_sceneMemoryBlock[_sceneActorPcOffset], _actorPcTable[0]);
			WRITE_LE_UINT32(&_sceneMemoryBlock[_sceneActorPcOffset + entry * 4], _actorPcTable[entry]);
		}

		packed = ((uint32)entry << 24) + pc;
	}

	actor.callPc = packed;
}

uint32 ComfyEngine::actorPopPc(Actor &actor) {
	uint32 packed = actor.callPc;
	byte entry = packed >> 24;
	if (entry) {
		actor.callPc = _actorPcTable[entry];
		_actorPcTable[entry] = _actorPcTable[0];
		_actorPcTable[0] = (uint32)entry << 24;
		if (_sceneActorPcOffset + sizeof(_actorPcTable) <= _sceneMemoryBlock.size()) {
			WRITE_LE_UINT32(&_sceneMemoryBlock[_sceneActorPcOffset], _actorPcTable[0]);
			WRITE_LE_UINT32(&_sceneMemoryBlock[_sceneActorPcOffset + entry * 4], _actorPcTable[entry]);
		}
	} else {
		actor.callPc = 0;
	}

	return packed & 0x00FFFFFF;
}

void ComfyEngine::actorFreePcChain(Actor &actor) {
	byte first = actor.callPc >> 24;
	byte entry = first;
	while (entry) {
		byte next = _actorPcTable[entry] >> 24;
		if (!next) {
			_actorPcTable[entry] = _actorPcTable[0];
			_actorPcTable[0] = (uint32)first << 24;
			if (_sceneActorPcOffset + sizeof(_actorPcTable) <= _sceneMemoryBlock.size())
				memcpy(&_sceneMemoryBlock[_sceneActorPcOffset], _actorPcTable, sizeof(_actorPcTable));

			break;
		}

		_actorPcTable[entry] &= 0xFF000000;
		entry = next;
	}

	actor.callPc = 0;
}

void ComfyEngine::actorFreeTreePc(uint16 actorIndex) {
	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return;

	if (!_usesAnimFile && _videoMode == 2)
		videoFindBestMode(actor->cachedRect);

	actorFreePcChain(*actor);

	uint16 child = actor->siblingHead;
	while (child) {
		Actor *childActor = actorGetPtr(child);
		uint16 next = childActor ? childActor->nextLink : 0;
		actorFreeTreePc(child);
		child = next;
	}

	child = actor->childHead;
	while (child) {
		Actor *childActor = actorGetPtr(child);
		uint16 next = childActor ? childActor->nextLink : 0;
		actorFreeTreePc(child);
		child = next;
	}

	if (_usesAnimFile) {
		uint32 selector = actor->spriteSelector;
		if (_videoMode == 2 || _videoMode == 4) {
			if (selector & 0xFF000000) {
				renderInvalidateFullFrame();
			} else if (selector != 0x00FFFFFF) {
				videoFindBestMode(actor->cachedRect);
			}
		}

		if (selector == 0x00FFFFFF)
			animFrameShutdown(true);
	}

	uint16 sceneSlot = actor->sceneHandle;
	actorFreeSlot(sceneSlot);
}

void ComfyEngine::actorClearDirtyTree(uint16 actorIndex) {
	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return;

	actor->dirty = 0;
	uint16 child = actor->siblingHead;
	while (child) {
		Actor *childActor = actorGetPtr(child);
		uint16 next = childActor ? childActor->nextLink : 0;
		actorClearDirtyTree(child);
		child = next;
	}

	child = actor->childHead;
	while (child) {
		Actor *childActor = actorGetPtr(child);
		uint16 next = childActor ? childActor->nextLink : 0;
		actorClearDirtyTree(child);
		child = next;
	}
}

void ComfyEngine::actorSetAllVisible() {
	for (uint i = 0; i + 1 < _actors.size(); i++)
		_actors[i].dirty = 1;
}

void ComfyEngine::actorClearFirstAnimFrameSelector() {
	for (uint actorIndex = 1; actorIndex < COMFY_ACTOR_COUNT; actorIndex++) {
		Actor *actor = actorGetPtr(actorIndex);
		if (actor && actor->spriteSelector == 0x00FFFFFF) {
			actor->spriteSelector = 0;
			break;
		}
	}
}

bool ComfyEngine::actorRunScript(uint16 actorIndex, bool &descendChildren) {
	Actor *actor = actorGetPtr(actorIndex);
	descendChildren = false;
	if (!actor)
		return false;

	_currentActor = actorIndex;
	_actorDestroyedCurrent = false;
	if (_stringTable.size() > 3) {
		_stringTable[2] = actor->stringRefs[0];
		_stringTable[3] = actor->stringRefs[1];
	}

	uint16 triggerKey = actor->triggerKey;
	if (triggerKey && keyBitTest(triggerKey)) {
		byte triggerFlags = actor->triggerFlags;
		if (triggerFlags & 1)
			actorSetPc(*actor, actor->currentPc);

		if (triggerFlags & 4)
			keyBitClear(triggerKey);

		if (triggerFlags & 2)
			actor->triggerKey = 0;

		actor->waitTarget = 0;
		actor->waitAccum = 0;
		actor->currentPc = actor->triggerPc;
	}

	bool timerWaitYield = false;
	int16 waitTarget = actor->waitTarget;
	if (waitTarget) {
		if (waitTarget > _timerCurrent) {
			actor->waitTarget = waitTarget - _timerCurrent;
			timerWaitYield = true;
		} else {
			int16 elapsed = _timerCurrent - waitTarget;
			actor->waitAccum += elapsed;
			actor->waitTarget = 0;
		}
	}

	int16 moveTicks = actor->moveTicks;
	if (moveTicks) {
		int16 stepTicks = (int16)(uint16)(_timerCurrent + _timer0 + _timer1 + _timer2 + 2) / 4;
		if (stepTicks > 20)
			stepTicks = 20;

		if (moveTicks < stepTicks) {
			moveTicks = stepTicks;
			actor->moveTicks = moveTicks;
		}

		int32 moveDx = actor->moveDx;
		int32 moveDy = actor->moveDy;
		int32 stepDx = ((int32)stepTicks * moveDx) / moveTicks;
		int32 stepDy = ((int32)stepTicks * moveDy) / moveTicks;
		actor->xFixed = (int32)((uint32)actor->xFixed + (uint32)stepDx);
		actor->yFixed = (int32)((uint32)actor->yFixed + (uint32)stepDy);
		actor->moveDx = moveDx - stepDx;
		actor->moveDy = moveDy - stepDy;
		moveTicks -= stepTicks;
		if (moveTicks < 0)
			moveTicks = 0;

		actor->moveTicks = moveTicks;
		if (moveTicks && actor->blockingMove) {
			descendChildren = true;
			return false;
		}

		if (!moveTicks && actor->completionKey)
			keyBitSet(actor->completionKey);
	}

	if (timerWaitYield) {
		if (_stringTable.size() > 3) {
			actor->stringRefs[0] = _stringTable[2];
			actor->stringRefs[1] = _stringTable[3];
		}

		descendChildren = true;
		return false;
	}

	uint32 pc = actor->currentPc;
	for (;;) {
		ScriptDispatchStatus status = scriptStep(*actor, pc);
		if (status == kScriptContinue)
			continue;

		if (status == kScriptYield) {
			if (_stringTable.size() > 3) {
				actor->stringRefs[0] = _stringTable[2];
				actor->stringRefs[1] = _stringTable[3];
			}

			actor->currentPc = pc;
		}

		descendChildren = status == kScriptYield && !_actorDestroyedCurrent;
		return _actorDestroyedCurrent;
	}
}

bool ComfyEngine::actorTickTree(uint16 actorIndex) {
	if (_pendingScene)
		return false;

	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return false;

	if (!actor->active) {
		actorClearDirtyTree(actorIndex);
		return false;
	}

	bool descendChildren = true;
	bool stopBranch = actorRunScript(actorIndex, descendChildren);
	actor = actorGetPtr(actorIndex);
	if (stopBranch || !descendChildren || !actor)
		return stopBranch;

	uint16 child = actor->siblingHead;
	while (child) {
		Actor *childActor = actorGetPtr(child);
		uint16 savedNext = childActor ? childActor->nextLink : 0;
		if (actorTickTree(child))
			child = savedNext;
		else {
			childActor = actorGetPtr(child);
			child = childActor ? childActor->nextLink : 0;
		}
	}

	child = actor->childHead;
	while (child) {
		Actor *childActor = actorGetPtr(child);
		uint16 savedNext = childActor ? childActor->nextLink : 0;
		if (actorTickTree(child))
			child = savedNext;
		else {
			childActor = actorGetPtr(child);
			child = childActor ? childActor->nextLink : 0;
		}
	}

	return false;
}

void ComfyEngine::actorDrawList(uint16 actorIndex, int16 x, int16 y, bool visible) {
	while (actorIndex) {
		if (_usesAnimFile)
			actorDrawInternal(actorIndex, x, y, visible);
		else
			actorDraw(actorIndex, x, y);

		Actor *actor = actorGetPtr(actorIndex);
		actorIndex = actor ? actor->nextLink : 0;
	}
}

void ComfyEngine::renderQueueDrawCommand(int16 x, int16 y, uint32 selector, byte mode, uint16 actorIndex) {
	if (!selector)
		return;

	if (_drawCommandCount >= COMFY_DRAW_COMMAND_CAPACITY)
		error("Too many actor draw commands");

	DrawCommand &command = _drawCommands[_drawCommandCount++];
	command.x = x;
	command.y = y;
	command.selector = selector;
	command.mode = mode;
	command.actorIndex = actorIndex;

	if (_drawCommandCount >= COMFY_DRAW_COMMAND_CAPACITY)
		error("Too many actor draw commands");
}

void ComfyEngine::renderFlushDrawCommands() {
	if ((_isPanther || _engineVersion == 3) && actorGetFrame())
		backgroundRestoreDirtyRects();

	for (uint i = 0; i < _drawCommandCount; i++) {
		DrawCommand &command = _drawCommands[i];
		if (command.mode == 2) {
			if (_engineVersion == 3) {
				for (uint next = i + 1; next < _drawCommandCount; next++) {
					DrawCommand &nextCommand = _drawCommands[next];
					if (nextCommand.mode != 0)
						continue;

					SpriteResource *nextSprite = spriteGetPtr((int16)nextCommand.selector);
					if (!nextSprite)
						continue;

					ComfyRect dirtyRect;
					dirtyRect.left = nextCommand.x;
					dirtyRect.top = nextCommand.y;
					dirtyRect.right = nextCommand.x + nextSprite->header.width;
					dirtyRect.bottom = nextCommand.y + nextSprite->header.height;
					dirtyRect.area = (uint32)nextSprite->header.width * nextSprite->header.height;
					videoFindBestMode(dirtyRect);
				}
			}

			bool coversMouse = animFrameCoversPoint(command.x, command.y, _mouseX, _mouseY);
			animFrameBlitAt(command.x, command.y);
			if (_engineVersion == 3 && command.actorIndex) {
				Actor *actor = actorGetPtr(command.actorIndex);
				if (actor)
					actor->blitHitMouse = coversMouse ? 1 : 0;
			}

			continue;
		}

		SpriteResource *sprite = spriteGetPtr((int16)command.selector);
		if (!sprite)
			continue;

		bool coversMouse = spriteCoversPoint((int16)command.selector, command.x, command.y, _mouseX, _mouseY);
		spriteBlitClipped((int16)command.selector, command.x, command.y);
		if (_engineVersion == 3 && command.actorIndex) {
			Actor *actor = actorGetPtr(command.actorIndex);
			if (actor)
				actor->blitHitMouse = coversMouse ? 1 : 0;
		}

		if (command.mode == 0) {
			ComfyRect dirtyRect;
			dirtyRect.left = command.x;
			dirtyRect.top = command.y;
			dirtyRect.right = command.x + sprite->header.width;
			dirtyRect.bottom = command.y + sprite->header.height;
			dirtyRect.area = (uint32)sprite->header.width * sprite->header.height;
			videoFindBestMode(dirtyRect);
		}
	}

	_drawCommandCount = 0;
}

void ComfyEngine::renderFlushCachedDirtyRects() {
	for (uint i = 0; i < _animFrameDirtyRectCount; i++)
		videoFindBestMode(_animFrameDirtyRects[i]);
}

uint16 ComfyEngine::scriptEvalKeyMask(uint32 pc, uint16 mode, ComfyRect &maskRecord,
		ComfyRect *rects, int16 baseX, int16 baseY) {
	if (mode == 1) {
		_keymaskBits = 0;
		maskRecord.right = 0;
		maskRecord.bottom = 0;
	} else {
		_keymaskBits = (uint16)maskRecord.right | ((uint32)(uint16)maskRecord.bottom << 16);
	}

	int16 wordsLeft = (int16)(scriptReadWord(pc) - 1);
	pc += 2;
	uint16 bitIndex = 0xFFFF;
	_keymaskCount = wordsLeft;
	while (wordsLeft > 0 && !_scriptFault) {
		bitIndex++;
		uint16 spriteId = scriptReadWord(pc);
		uint16 dx = scriptReadWord(pc + 2);
		uint16 dy = scriptReadWord(pc + 4);
		pc += 6;
		wordsLeft -= 3;
		_keymaskCount = wordsLeft;
		_keymaskSpriteWord = spriteId;
		_keymaskArg0 = dx;
		_keymaskArg1 = dy;

		if (spriteId & 0x8000) {
			wordsLeft -= 2;
			_keymaskCount = wordsLeft;
			uint32 mask = (uint32)1 << (bitIndex & 31);
			if (mode == 1) {
				uint16 key = scriptReadWord(pc);
				_keymaskResult = keyBitTest(key) ? 1 : 0;
				if (_keymaskResult) {
					_keymaskBits |= mask;
					maskRecord.right = (uint16)_keymaskBits;
					maskRecord.bottom = (uint16)(_keymaskBits >> 16);
				}

				pc += 2;
				if (_keymaskResult)
					spriteId &= 0x7FFF;
				else
					spriteId = scriptReadWord(pc);
			} else {
				_keymaskResult = (_keymaskBits & mask) ? 1 : 0;
				pc += 2;
				if (_keymaskResult)
					spriteId &= 0x7FFF;
				else
					spriteId = scriptReadWord(pc);
			}

			pc += 2;
			_keymaskSpriteWord = spriteId;
		}

		ComfyRect *rect = bitIndex < COMFY_KEYMASK_RECT_CAPACITY ? &rects[bitIndex] : nullptr;
		if (rect) {
			rect->left = 0;
			rect->top = 0;
			rect->right = 0;
			rect->bottom = 0;
			rect->area = 0;
		}

		if (!spriteId)
			continue;

		_keymaskX = baseX + (int16)dx;
		_keymaskY = baseY + (int16)dy;
		if (_engineVersion <= 2 && (_keymaskX >= 0x0280 || _keymaskX <= (int16)0xFEC0 ||
				_keymaskY >= 0x0190 || _keymaskY <= (int16)0xFF38))
			continue;

		SpriteResource *sprite = _engineVersion <= 2 ?
			spriteGetPtr((int16)spriteId) :
			spriteLookup(spriteId, _keymaskX, _keymaskY);
		if (!sprite)
			continue;

		int16 left = _keymaskX - sprite->header.hotspotX;
		int16 top = _keymaskY - sprite->header.hotspotY;
		if (mode == 1 && _engineVersion <= 2 && !_isPanther)
			spriteBlitClipped((int16)spriteId, left, top);

		if (rect) {
			rect->left = left;
			rect->top = top;
			rect->right = left + sprite->header.width;
			rect->bottom = top + sprite->header.height;
			if (_isPanther || _engineVersion == 3) {
				rect->area = spriteId;
			} else {
				rect->area = (uint32)sprite->header.width * sprite->header.height;
				if (_engineVersion == 1)
					rect->area = (uint16)rect->area;
			}
		}
	}

	_keymaskLastIndex = bitIndex;
	return bitIndex;
}

void ComfyEngine::actorEvalFrameSelection(uint16 actorIndex, int16 x, int16 y) {
	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return;

	uint32 selector = actor->cachedSprite;
	ComfyRect rect = actor->cachedRect;
	if (selector == 0x00FFFFFF) {
		animFrameInvalidateRects(rect.left, rect.top, 0);
	} else if (selector & 0xFF000000) {
		uint16 last = scriptEvalKeyMask(selector & 0x00FFFFFF, 0, rect, _keymaskInvalidationRects,
			rect.left, rect.top);
		if (last != 0xFFFF && last >= COMFY_KEYMASK_RECT_CAPACITY)
			last = COMFY_KEYMASK_RECT_CAPACITY - 1;

		if (last != 0xFFFF) {
			for (uint i = 0; i <= last; i++) {
				ComfyRect dirtyRect = _keymaskInvalidationRects[i];
				dirtyRect.area = (uint32)(dirtyRect.right - dirtyRect.left) *
					(dirtyRect.bottom - dirtyRect.top);
				if (_engineVersion == 1)
					dirtyRect.area = (uint16)dirtyRect.area;

				videoFindBestMode(dirtyRect);
			}
		}
	} else {
		videoFindBestMode(rect);
	}

	actor->cachedSprite = 0;
	uint16 child = actor->siblingHead;
	while (child) {
		actorEvalFrameSelection(child, x, y);
		Actor *childActor = actorGetPtr(child);
		child = childActor ? childActor->nextLink : 0;
	}

	child = actor->childHead;
	while (child) {
		actorEvalFrameSelection(child, x, y);
		Actor *childActor = actorGetPtr(child);
		child = childActor ? childActor->nextLink : 0;
	}
}

bool ComfyEngine::actorDraw(uint16 actorIndex, int16 x, int16 y) {
	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return true;

	if (!animFrameShouldDraw(1)) {
		if (_usesAnimFile)
			animFrameInvalidateRects(x, y, 0);

		return false;
	}

	if (!_usesAnimFile && !actor->visible) {
		if ((_videoMode == 2 || _videoMode == 4) &&
				actor->visible != actor->cachedVisible) {
			int16 drawX = (int16)(actor->xFixed >> 12) + x;
			int16 drawY = (int16)(actor->yFixed >> 12) + y;
			actorEvalFrameSelection(actorIndex, drawX, drawY);
			actor->cachedVisible = actor->visible;
		}

		return true;
	}

	animFrameRecordVocCounter(1);
	bool queuedActorDraw = _usesAnimFile && (_isPanther || _engineVersion == 3) && (_videoMode == 2 || _videoMode == 4);
	if ((_isPanther || _engineVersion == 3) && !queuedActorDraw)
		actorDrawLegacyInternal(actorIndex, x, y);
	else
		actorDrawInternal(actorIndex, x, y, true);

	if (_usesAnimFile) {
		if (queuedActorDraw)
			renderFlushDrawCommands();

		animFrameRecordVocCounter(2);
	}

	return animFrameShouldDraw(2);
}

bool ComfyEngine::actorDrawLegacyInternal(uint16 actorIndex, int16 x, int16 y) {
	Actor *actor = actorGetPtr(actorIndex);
	if (!actor || !actor->visible)
		return true;

	int16 drawX = (int16)(actor->xFixed >> 12) + x;
	int16 drawY = (int16)(actor->yFixed >> 12) + y;
	bool drawResult = true;
	uint16 linkedActor = actor->siblingHead;
	while (linkedActor && drawResult) {
		drawResult = actorDrawLegacyInternal(linkedActor, drawX, drawY);
		Actor *linked = actorGetPtr(linkedActor);
		linkedActor = linked ? linked->nextLink : 0;
	}

	if (!drawResult)
		return false;

	uint32 selector = actor->spriteSelector;
	if (selector == 0x00FFFFFF) {
		animFrameInvalidateRects(drawX, drawY, 1);
		drawResult = animFrameBlitAt(drawX, drawY);
	} else if (selector & 0xFF000000) {
		uint32 pc = selector & 0x00FFFFFF;
		int16 wordsLeft = (int16)(scriptReadWord(pc) - 1);
		pc += 2;
		while (wordsLeft > 0 && !_scriptFault) {
			uint16 spriteId = scriptReadWord(pc);
			int16 dx = scriptReadWord(pc + 2);
			int16 dy = scriptReadWord(pc + 4);
			pc += 6;
			wordsLeft -= 3;
			if (spriteId & 0x8000) {
				wordsLeft -= 2;
				uint16 key = scriptReadWord(pc);
				pc += 2;
				if (keyBitTest(key))
					spriteId &= 0x7FFF;
				else
					spriteId = scriptReadWord(pc);

				pc += 2;
			}

			if (spriteId) {
				int16 spriteX = drawX + dx;
				int16 spriteY = drawY + dy;
				SpriteResource *sprite = spriteLookup(spriteId, spriteX, spriteY);
				if (sprite)
					spriteBlitClipped((int16)spriteId, spriteX - sprite->header.hotspotX,
						spriteY - sprite->header.hotspotY);
			}
		}
	} else if (selector) {
		SpriteResource *sprite = spriteLookup((uint16)selector, drawX, drawY);
		if (sprite)
			spriteBlitClipped((int16)selector, drawX - sprite->header.hotspotX,
				drawY - sprite->header.hotspotY);
	}

	if (!drawResult)
		return false;

	linkedActor = actor->childHead;
	while (linkedActor && drawResult) {
		drawResult = actorDrawLegacyInternal(linkedActor, drawX, drawY);
		Actor *linked = actorGetPtr(linkedActor);
		linkedActor = linked ? linked->nextLink : 0;
	}

	return drawResult;
}

void ComfyEngine::actorDrawInternal(uint16 actorIndex, int16 x, int16 y, bool visible) {
	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return;

	int16 drawX = (int16)(actor->xFixed >> 12) + x;
	int16 drawY = (int16)(actor->yFixed >> 12) + y;
	visible = visible && actor->visible;
	byte oldVisible = actor->cachedVisible;
	uint32 oldSelector = actor->cachedSprite;
	ComfyRect oldRect = actor->cachedRect;
	actorDrawList(actor->siblingHead, drawX, drawY, visible);
	uint32 selector = actor->spriteSelector;
	bool useDrawQueue = _usesAnimFile && (_isPanther || _engineVersion == 3) && (_videoMode == 2 || _videoMode == 4);
	if (_isPanther || _engineVersion == 3) {
		ComfyRect newRect;
		if (selector == 0x00FFFFFF) {
			bool changed = false;
			if (oldVisible != (visible ? 1 : 0) && actor->visible && oldVisible == 1)
				changed = true;

			if (oldRect.left != drawX || oldRect.top != drawY)
				changed = true;

			if (changed) {
				renderFlushCachedDirtyRects();
				animFrameInvalidateRects(drawX, drawY, 1);
				newRect.left = drawX;
				newRect.top = drawY;
				actor->cachedVisible = visible ? 1 : 0;
				actor->cachedRect = newRect;

				renderQueueDrawCommand(drawX, drawY, selector, 2, actorIndex);
			} else if (visible && actor->visible) {
				animFrameInvalidateRects(drawX, drawY, 0);
				renderQueueDrawCommand(drawX, drawY, selector, 2, actorIndex);
			}

			actorDrawList(actor->childHead, drawX, drawY, visible);
			return;
		}

		bool scripted = (selector & 0xFF000000) != 0;
		uint16 currentLastRect = 0xFFFF;
		if (scripted) {
			_keymaskCurrentRecord.left = drawX;
			_keymaskCurrentRecord.top = drawY;
			_keymaskCurrentRecord.right = 0;
			_keymaskCurrentRecord.bottom = 0;
			_keymaskCurrentRecord.area = 0;
			currentLastRect = scriptEvalKeyMask(selector & 0x00FFFFFF, 1,
				_keymaskCurrentRecord, _keymaskRects, drawX, drawY);
			if (currentLastRect != 0xFFFF && currentLastRect >= COMFY_KEYMASK_RECT_CAPACITY)
				currentLastRect = COMFY_KEYMASK_RECT_CAPACITY - 1;

			newRect = _keymaskCurrentRecord;
		} else if (selector) {
			SpriteResource *sprite = spriteLookup((uint16)selector, drawX, drawY);
			if (sprite) {
				newRect.left = drawX - sprite->header.hotspotX;
				newRect.top = drawY - sprite->header.hotspotY;
				newRect.right = newRect.left + sprite->header.width;
				newRect.bottom = newRect.top + sprite->header.height;
				newRect.area = selector;
			}
		}

		if (scripted && oldSelector == selector && oldRect.left == newRect.left &&
				oldRect.top == newRect.top && oldVisible == 1 && visible) {
			ComfyRect oldMaskRecord = oldRect;
			uint16 oldLastRect = scriptEvalKeyMask(oldSelector & 0x00FFFFFF, 0,
				oldMaskRecord, _keymaskOldRects, oldRect.left, oldRect.top);
			if (oldLastRect != 0xFFFF && oldLastRect >= COMFY_KEYMASK_RECT_CAPACITY)
				oldLastRect = COMFY_KEYMASK_RECT_CAPACITY - 1;

			if (oldLastRect != 0xFFFF) {
				for (uint i = 0; i <= oldLastRect; i++) {
					ComfyRect &currentRect = _keymaskRects[i];
					ComfyRect &previousRect = _keymaskOldRects[i];
					bool equal = currentRect.left == previousRect.left &&
						currentRect.top == previousRect.top &&
						currentRect.right == previousRect.right &&
						currentRect.bottom == previousRect.bottom &&
						currentRect.area == previousRect.area;
					if (!equal)
						videoFindBestMode(previousRect);

					renderQueueDrawCommand(currentRect.left, currentRect.top,
						currentRect.area, equal ? 1 : 0, actorIndex);
				}
			}

			actor->cachedRect = newRect;

			actorDrawList(actor->childHead, drawX, drawY, visible);
			return;
		}

		bool changed = oldSelector != selector || oldRect.left != newRect.left ||
			oldRect.top != newRect.top || oldRect.right != newRect.right ||
			oldRect.bottom != newRect.bottom || oldRect.area != newRect.area ||
			oldVisible != (visible ? 1 : 0);
		if (changed) {
			if (visible && selector) {
				if (scripted && currentLastRect != 0xFFFF) {
					for (uint i = 0; i <= currentLastRect; i++) {
						ComfyRect &rect = _keymaskRects[i];
						renderQueueDrawCommand(rect.left, rect.top, rect.area, 0, actorIndex);
					}
				} else if (!scripted) {
					renderQueueDrawCommand(newRect.left, newRect.top, newRect.area, 0, actorIndex);
				}
			}

			if (oldVisible) {
				if (oldSelector == 0x00FFFFFF) {
					renderFlushCachedDirtyRects();
				} else if (oldSelector & 0xFF000000) {
					ComfyRect oldMaskRecord = oldRect;
					uint16 oldLastRect = scriptEvalKeyMask(oldSelector & 0x00FFFFFF, 0,
						oldMaskRecord, _keymaskOldRects, oldRect.left, oldRect.top);
					if (oldLastRect != 0xFFFF && oldLastRect >= COMFY_KEYMASK_RECT_CAPACITY)
						oldLastRect = COMFY_KEYMASK_RECT_CAPACITY - 1;

					if (oldLastRect != 0xFFFF) {
						for (uint i = 0; i <= oldLastRect; i++)
							videoFindBestMode(_keymaskOldRects[i]);
					}
				} else {
					videoFindBestMode(oldRect);
				}
			}

			actor->cachedVisible = visible ? 1 : 0;
			actor->cachedRect = newRect;

			actor->cachedSprite = selector;
		} else if (visible) {
			if (scripted && currentLastRect != 0xFFFF) {
				for (uint i = 0; i <= currentLastRect; i++) {
					ComfyRect &rect = _keymaskRects[i];
					renderQueueDrawCommand(rect.left, rect.top, rect.area, 1, actorIndex);
				}
			} else if (!scripted) {
				renderQueueDrawCommand(newRect.left, newRect.top, newRect.area, 1, actorIndex);
			}
		}

		actorDrawList(actor->childHead, drawX, drawY, visible);
		return;
	}

	bool oldScripted = (oldSelector & 0xFF000000) != 0;
	bool newScripted = (selector & 0xFF000000) != 0;
	ComfyRect newRect;
	uint16 scriptedLastRect = 0xFFFF;
	if (newScripted) {
		uint32 pc = selector & 0x00FFFFFF;
		if (_videoMode == 2 || _videoMode == 4) {
			_keymaskCurrentRecord.left = drawX;
			_keymaskCurrentRecord.top = drawY;
			_keymaskCurrentRecord.right = 0;
			_keymaskCurrentRecord.bottom = 0;
			_keymaskCurrentRecord.area = 0;
			scriptedLastRect = scriptEvalKeyMask(pc, 1, _keymaskCurrentRecord, _keymaskRects, drawX, drawY);
			if (scriptedLastRect != 0xFFFF && scriptedLastRect >= COMFY_KEYMASK_RECT_CAPACITY)
				scriptedLastRect = COMFY_KEYMASK_RECT_CAPACITY - 1;

			if (_usesAnimFile && visible && scriptedLastRect != 0xFFFF) {
				for (uint i = 0; i <= scriptedLastRect; i++) {
					ComfyRect &drawRecord = _keymaskRects[i];
					if (drawRecord.area) {
						if (useDrawQueue)
							renderQueueDrawCommand(drawRecord.left, drawRecord.top, drawRecord.area, 1, actorIndex);
						else
							spriteBlitClipped((int16)drawRecord.area, drawRecord.left, drawRecord.top);
					}
				}
			}
		} else {
			int16 wordsLeft = (int16)(scriptReadWord(pc) - 1);
			pc += 2;
			while (wordsLeft > 0 && !_scriptFault) {
				uint16 spriteId = scriptReadWord(pc);
				int16 dx = scriptReadWord(pc + 2);
				int16 dy = scriptReadWord(pc + 4);
				pc += 6;
				wordsLeft -= 3;
				if (spriteId & 0x8000) {
					wordsLeft -= 2;
					uint16 key = scriptReadWord(pc);
					pc += 2;
					if (keyBitTest(key))
						spriteId &= 0x7FFF;
					else
						spriteId = scriptReadWord(pc);

					pc += 2;
				}

				if (visible && spriteId) {
					SpriteResource *sprite = spriteGetPtr((int16)spriteId);
					if (sprite) {
						int16 spriteX = drawX + dx - sprite->header.hotspotX;
						int16 spriteY = drawY + dy - sprite->header.hotspotY;
						if (useDrawQueue)
							renderQueueDrawCommand(spriteX, spriteY, spriteId, 1, actorIndex);
						else
							spriteBlitClipped((int16)spriteId, spriteX, spriteY);
					}
				}
			}
		}

		if ((_videoMode == 2 || _videoMode == 4) && scriptedLastRect != 0xFFFF) {
			newRect = _keymaskCurrentRecord;
		}
	} else if (selector == 0x00FFFFFF) {
		uint16 frameWidth = 0;
		uint16 frameHeight = 0;
		animFrameGetDimensions(frameWidth, frameHeight);
		newRect.left = drawX;
		newRect.top = drawY;
		newRect.right = drawX + frameWidth;
		newRect.bottom = drawY + frameHeight;
		newRect.area = frameWidth * frameHeight;
		if (_engineVersion == 1)
			newRect.area = (uint16)newRect.area;

		if (visible) {
			if (useDrawQueue) {
				renderQueueDrawCommand(drawX, drawY, selector, 2, actorIndex);
			} else {
				animFrameInvalidateRects(drawX, drawY, 1);
				animFrameBlitAt(drawX, drawY);
			}
		}
	} else if (selector) {
		SpriteResource *sprite = spriteLookup((uint16)selector, drawX, drawY);
		if (sprite) {
			newRect.left = drawX - sprite->header.hotspotX;
			newRect.top = drawY - sprite->header.hotspotY;
			newRect.right = newRect.left + sprite->header.width;
			newRect.bottom = newRect.top + sprite->header.height;
			newRect.area = sprite->header.width * sprite->header.height;
			if (_engineVersion == 1)
				newRect.area = (uint16)newRect.area;

			if (visible) {
				int16 spriteX = drawX - sprite->header.hotspotX;
				int16 spriteY = drawY - sprite->header.hotspotY;
				if (useDrawQueue)
					renderQueueDrawCommand(spriteX, spriteY, selector, 1, actorIndex);
				else
					spriteBlitClipped((int16)selector, spriteX, spriteY);
			}
		}
	}
	if (_videoMode == 2 || _videoMode == 4) {
		byte newVisible = visible ? 1 : 0;
		bool changed = oldSelector != selector || oldVisible != newVisible;
		if (oldRect.left != newRect.left || oldRect.top != newRect.top ||
				oldRect.right != newRect.right || oldRect.bottom != newRect.bottom || oldRect.area != newRect.area)
			changed = true;

		uint16 oldScriptedLastRect = 0xFFFF;
		if (oldScripted) {
			ComfyRect maskRecord = oldRect;
			oldScriptedLastRect = scriptEvalKeyMask(oldSelector & 0x00FFFFFF, 0, maskRecord,
				_keymaskOldRects, oldRect.left, oldRect.top);
			if (oldScriptedLastRect != 0xFFFF && oldScriptedLastRect >= COMFY_KEYMASK_RECT_CAPACITY)
				oldScriptedLastRect = COMFY_KEYMASK_RECT_CAPACITY - 1;
		}

		if (changed) {
			bool comparedScriptRects = false;
			if (newScripted && oldScripted && oldSelector == selector &&
					oldRect.left == _keymaskCurrentRecord.left && oldRect.top == _keymaskCurrentRecord.top &&
					scriptedLastRect != 0xFFFF && oldScriptedLastRect != 0xFFFF) {
				uint16 last = MAX(scriptedLastRect, oldScriptedLastRect);
				for (uint i = 0; i <= last; i++) {
					ComfyRect *oldItem = i <= oldScriptedLastRect ? &_keymaskOldRects[i] : nullptr;
					ComfyRect *newItem = i <= scriptedLastRect ? &_keymaskRects[i] : nullptr;
					bool equal = oldItem && newItem && oldItem->left == newItem->left && oldItem->top == newItem->top &&
						oldItem->right == newItem->right && oldItem->bottom == newItem->bottom && oldItem->area == newItem->area;
					if (!equal) {
						if (oldItem) {
							ComfyRect dirtyRect = *oldItem;
							dirtyRect.area = (uint32)(dirtyRect.right - dirtyRect.left) *
								(dirtyRect.bottom - dirtyRect.top);
							if (_engineVersion == 1)
								dirtyRect.area = (uint16)dirtyRect.area;

							videoFindBestMode(dirtyRect);
						}

						if (newItem) {
							ComfyRect dirtyRect = *newItem;
							dirtyRect.area = (uint32)(dirtyRect.right - dirtyRect.left) *
								(dirtyRect.bottom - dirtyRect.top);
							if (_engineVersion == 1)
								dirtyRect.area = (uint16)dirtyRect.area;

							videoFindBestMode(dirtyRect);
						}
					}
				}

				comparedScriptRects = true;
			} else if (newScripted && scriptedLastRect != 0xFFFF) {
				for (uint i = 0; i <= scriptedLastRect; i++) {
					ComfyRect dirtyRect = _keymaskRects[i];
					dirtyRect.area = (uint32)(dirtyRect.right - dirtyRect.left) *
						(dirtyRect.bottom - dirtyRect.top);
					if (_engineVersion == 1)
						dirtyRect.area = (uint16)dirtyRect.area;

					videoFindBestMode(dirtyRect);
				}
			} else if (!newScripted && selector != 0x00FFFFFF) {
				videoFindBestMode(newRect);
			}

			if (!comparedScriptRects) {
				if (oldScripted && oldScriptedLastRect != 0xFFFF) {
					for (uint i = 0; i <= oldScriptedLastRect; i++) {
						ComfyRect dirtyRect = _keymaskOldRects[i];
						dirtyRect.area = (uint32)(dirtyRect.right - dirtyRect.left) *
							(dirtyRect.bottom - dirtyRect.top);
						if (_engineVersion == 1)
							dirtyRect.area = (uint16)dirtyRect.area;

						videoFindBestMode(dirtyRect);
					}
				} else if (oldSelector == 0x00FFFFFF) {
					animFrameInvalidateRects(oldRect.left, oldRect.top, 0);
				} else if (!oldScripted) {
					videoFindBestMode(oldRect);
				}
			}
		}

		actor->cachedVisible = newVisible;
		actor->cachedSprite = selector;
		actor->cachedRect = newRect;
		if (_engineVersion == 1)
			actor->cachedRect.area &= 0xFFFF;
	}

	if (!_usesAnimFile)
		animFrameRecordVocCounter(2);

	actorDrawList(actor->childHead, drawX, drawY, visible);
}


} // End of namespace Comfy
