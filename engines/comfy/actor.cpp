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
	if (!spriteId)
		return;

	soundUnprepareHeader();

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

void ComfyEngine::spriteBlitClipped(int16 spriteId, int16 x, int16 y) {
	SpriteResource *loadedSprite = spriteGetPtr(spriteId);
	if (!loadedSprite)
		return;

	SpriteResource &sprite = *loadedSprite;
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

uint32 ComfyEngine::actorReadU32(Actor &actor, uint offset) {
	if (offset + 4 > COMFY_ACTOR_SIZE)
		return 0;

	return READ_LE_UINT32(actor.raw + offset);
}

uint16 ComfyEngine::actorReadU16(Actor &actor, uint offset) {
	if (offset + 2 > COMFY_ACTOR_SIZE)
		return 0;

	return READ_LE_UINT16(actor.raw + offset);
}

byte ComfyEngine::actorReadU8(Actor &actor, uint offset) {
	if (offset >= COMFY_ACTOR_SIZE)
		return 0;

	return actor.raw[offset];
}

void ComfyEngine::actorWriteU32(Actor &actor, uint offset, uint32 value) {
	if (offset + 4 <= COMFY_ACTOR_SIZE)
		WRITE_LE_UINT32(actor.raw + offset, value);
}

void ComfyEngine::actorWriteU16(Actor &actor, uint offset, uint16 value) {
	if (offset + 2 <= COMFY_ACTOR_SIZE)
		WRITE_LE_UINT16(actor.raw + offset, value);
}

void ComfyEngine::actorWriteU8(Actor &actor, uint offset, byte value) {
	if (offset < COMFY_ACTOR_SIZE)
		actor.raw[offset] = value;
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
		actorWriteU32(*root, kActorSpriteSelector, (uint32)(int32)frame);
}

uint16 ComfyEngine::actorGetFrame() {
	Actor *root = rootActor();
	return root ? (uint16)actorReadU32(*root, kActorSpriteSelector) : 0;
}

ComfyEngine::Actor *ComfyEngine::actorResolve(uint16 sceneOrActor, uint16 fallbackActor) {
	if (!sceneOrActor)
		return actorGetPtr(fallbackActor);

	return actorGetPtr(sceneGetHandle(sceneOrActor));
}

uint16 ComfyEngine::actorAlloc(uint16 sceneSlot) {
	Actor *root = actorGetPtr(0);
	uint16 actorIndex = root ? actorReadU16(*root, kActorNextLink) : 0;
	if (!actorIndex || sceneSlot >= _sceneHandles.size())
		return 0;

	Actor *actor = actorGetPtr(actorIndex);
	actorWriteU16(*root, kActorNextLink, actorReadU16(*actor, kActorNextLink));
	_sceneHandles[sceneSlot] = actorIndex;
	actorWriteU16(*actor, kActorSceneHandle, sceneSlot);
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
	actorWriteU16(*actor, kActorNextLink, root ? actorReadU16(*root, kActorNextLink) : 0);
	actorWriteU16(*actor, kActorSceneHandle, 0);
	if (root)
		actorWriteU16(*root, kActorNextLink, actorIndex);
	_sceneHandles[sceneSlot] = 0;
}

void ComfyEngine::actorInsertChild(uint16 childIndex, uint16 parentIndex) {
	Actor *child = actorGetPtr(childIndex);
	Actor *parent = actorGetPtr(parentIndex);
	if (!child || !parent)
		return;

	uint16 tail = actorReadU16(*parent, kActorChildTail);
	if (tail) {
		actorWriteU16(*actorGetPtr(tail), kActorNextLink, childIndex);
		actorWriteU16(*child, kActorPrevLink, tail);
	} else {
		actorWriteU16(*parent, kActorChildHead, childIndex);
		actorWriteU16(*child, kActorPrevLink, 0);
	}

	actorWriteU16(*child, kActorNextLink, 0);
	actorWriteU16(*parent, kActorChildTail, childIndex);
}

void ComfyEngine::actorInsertSibling(uint16 actorIndex, uint16 ownerIndex) {
	Actor *actor = actorGetPtr(actorIndex);
	Actor *owner = actorGetPtr(ownerIndex);
	if (!actor || !owner)
		return;

	uint16 head = actorReadU16(*owner, kActorSiblingHead);
	if (head)
		actorWriteU16(*actorGetPtr(head), kActorPrevLink, actorIndex);

	actorWriteU16(*actor, kActorNextLink, head);
	actorWriteU16(*actor, kActorPrevLink, 0);
	actorWriteU16(*owner, kActorSiblingHead, actorIndex);
}

void ComfyEngine::actorUnlink(uint16 actorIndex) {
	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return;

	Actor *parent = actorGetPtr(actorReadU16(*actor, kActorParent));
	if (!parent)
		return;

	uint16 next = actorReadU16(*actor, kActorNextLink);
	uint16 previous = actorReadU16(*actor, kActorPrevLink);
	if (next)
		actorWriteU16(*actorGetPtr(next), kActorPrevLink, previous);
	else if (actorReadU16(*parent, kActorChildTail) == actorIndex)
		actorWriteU16(*parent, kActorChildTail, previous);

	if (previous)
		actorWriteU16(*actorGetPtr(previous), kActorNextLink, next);
	else if (actorReadU16(*parent, kActorSiblingHead) == actorIndex)
		actorWriteU16(*parent, kActorSiblingHead, next);
	else
		actorWriteU16(*parent, kActorChildHead, next);
}

uint16 ComfyEngine::actorInit(uint16 sceneSlot, uint16 parentSlot, byte visible, byte active,
		uint32 pc, int16 x, int16 y, int16 sprite, byte insertAsChild) {
	uint16 actorIndex = actorAlloc(sceneSlot);
	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return 8;

	actorWriteU16(*actor, kActorSceneHandle, sceneSlot);
	actorWriteU8(*actor, kActorVisible, visible);
	actorWriteU8(*actor, kActorActive, active);
	actorWriteU32(*actor, kActorCurrentPc, pc);
	actorWriteU32(*actor, kActorCallPc, 0);
	actorWriteU32(*actor, kActorResetPc, pc);
	actorWriteU32(*actor, kActorXFixed, (uint32)((int32)x * 0x1000));
	actorWriteU32(*actor, kActorYFixed, (uint32)((int32)y * 0x1000));
	actorWriteU32(*actor, kActorSpriteSelector, (uint32)(int32)sprite);
	actorWriteU16(*actor, kActorChildTail, 0);
	actorWriteU16(*actor, kActorChildHead, 0);
	actorWriteU16(*actor, kActorSiblingHead, 0);
	actorWriteU16(*actor, kActorMoveTicks, 0);
	actorWriteU16(*actor, kActorTriggerKey, 0);
	actorWriteU16(*actor, kActorWaitTarget, 0);
	actorWriteU16(*actor, kActorWaitAccum, 0);
	actorWriteU8(*actor, kActorDirty, 1);
	if (_videoMode == 2) {
		VideoRectRecord empty;
		actorWriteCachedRect(*actor, empty);
		actorWriteU32(*actor, kActorCachedSprite, 0);
		actorWriteU8(*actor, kActorCachedVisible, 0);
	}

	if (parentSlot) {
		uint16 parentIndex = sceneGetHandle(parentSlot);
		actorWriteU16(*actor, kActorParent, parentIndex);
		if (insertAsChild)
			actorInsertChild(actorIndex, parentIndex);
		else
			actorInsertSibling(actorIndex, parentIndex);
	} else {
		actorWriteU16(*actor, kActorParent, 0);
		actorWriteU16(*actor, kActorNextLink, 0);
		actorWriteU16(*actor, kActorPrevLink, 0);
	}

	return 1;
}

void ComfyEngine::actorSetPc(Actor &actor, uint32 pc) {
	uint32 previous = actorReadU32(actor, kActorCallPc);
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

	actorWriteU32(actor, kActorCallPc, packed);
}

uint32 ComfyEngine::actorPopPc(Actor &actor) {
	uint32 packed = actorReadU32(actor, kActorCallPc);
	byte entry = packed >> 24;
	if (entry) {
		actorWriteU32(actor, kActorCallPc, _actorPcTable[entry]);
		_actorPcTable[entry] = _actorPcTable[0];
		_actorPcTable[0] = (uint32)entry << 24;
		if (_sceneActorPcOffset + sizeof(_actorPcTable) <= _sceneMemoryBlock.size()) {
			WRITE_LE_UINT32(&_sceneMemoryBlock[_sceneActorPcOffset], _actorPcTable[0]);
			WRITE_LE_UINT32(&_sceneMemoryBlock[_sceneActorPcOffset + entry * 4], _actorPcTable[entry]);
		}
	} else {
		actorWriteU32(actor, kActorCallPc, 0);
	}

	return packed & 0x00FFFFFF;
}

void ComfyEngine::actorFreePcChain(Actor &actor) {
	byte first = actorReadU32(actor, kActorCallPc) >> 24;
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

	actorWriteU32(actor, kActorCallPc, 0);
}

void ComfyEngine::actorFreeTreePc(uint16 actorIndex) {
	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return;

	if (_videoMode == 2) {
		VideoRectRecord rect = actorReadCachedRect(*actor);
		videoFindBestMode(rect);
	}

	actorFreePcChain(*actor);

	uint16 child = actorReadU16(*actor, kActorSiblingHead);
	while (child) {
		Actor *childActor = actorGetPtr(child);
		uint16 next = childActor ? actorReadU16(*childActor, kActorNextLink) : 0;
		actorFreeTreePc(child);
		child = next;
	}

	child = actorReadU16(*actor, kActorChildHead);
	while (child) {
		Actor *childActor = actorGetPtr(child);
		uint16 next = childActor ? actorReadU16(*childActor, kActorNextLink) : 0;
		actorFreeTreePc(child);
		child = next;
	}

	if (_usesAnimFile && actorReadU32(*actor, kActorSpriteSelector) == 0x00FFFFFF)
		animFrameShutdown(true);

	uint16 sceneSlot = actorReadU16(*actor, kActorSceneHandle);
	actorFreeSlot(sceneSlot);
}

void ComfyEngine::actorClearDirtyTree(uint16 actorIndex) {
	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return;

	actorWriteU8(*actor, kActorDirty, 0);
	uint16 child = actorReadU16(*actor, kActorSiblingHead);
	while (child) {
		Actor *childActor = actorGetPtr(child);
		uint16 next = childActor ? actorReadU16(*childActor, kActorNextLink) : 0;
		actorClearDirtyTree(child);
		child = next;
	}

	child = actorReadU16(*actor, kActorChildHead);
	while (child) {
		Actor *childActor = actorGetPtr(child);
		uint16 next = childActor ? actorReadU16(*childActor, kActorNextLink) : 0;
		actorClearDirtyTree(child);
		child = next;
	}
}

void ComfyEngine::actorSetAllVisible() {
	for (uint i = 0; i + 1 < _actors.size(); i++)
		actorWriteU8(_actors[i], kActorDirty, 1);
}

bool ComfyEngine::actorRunScript(uint16 actorIndex, bool &descendChildren) {
	Actor *actor = actorGetPtr(actorIndex);
	descendChildren = false;
	if (!actor)
		return false;

	_currentActor = actorIndex;
	_actorDestroyedCurrent = false;
	if (_stringTable.size() > 3) {
		_stringTable[2] = actorReadU16(*actor, kActorStringRef);
		_stringTable[3] = actorReadU16(*actor, kActorStringRef + 2);
	}

	uint16 triggerKey = actorReadU16(*actor, kActorTriggerKey);
	if (triggerKey && keyBitTest(triggerKey)) {
		byte triggerFlags = actorReadU8(*actor, kActorTriggerFlags);
		if (triggerFlags & 1)
			actorSetPc(*actor, actorReadU32(*actor, kActorCurrentPc));

		if (triggerFlags & 4)
			keyBitClear(triggerKey);

		if (triggerFlags & 2)
			actorWriteU16(*actor, kActorTriggerKey, 0);

		actorWriteU16(*actor, kActorWaitTarget, 0);
		actorWriteU16(*actor, kActorWaitAccum, 0);
		actorWriteU32(*actor, kActorCurrentPc, actorReadU32(*actor, kActorTriggerPc));
	}

	bool timerWaitYield = false;
	int16 waitTarget = actorReadU16(*actor, kActorWaitTarget);
	if (waitTarget) {
		if (waitTarget > _timerCurrent) {
			actorWriteU16(*actor, kActorWaitTarget, waitTarget - _timerCurrent);
			timerWaitYield = true;
		} else {
			int16 elapsed = _timerCurrent - waitTarget;
			actorWriteU16(*actor, kActorWaitAccum, actorReadU16(*actor, kActorWaitAccum) + elapsed);
			actorWriteU16(*actor, kActorWaitTarget, 0);
		}
	}

	int16 moveTicks = actorReadU16(*actor, kActorMoveTicks);
	if (moveTicks) {
		int16 stepTicks = (int16)(uint16)(_timerCurrent + _timer0 + _timer1 + _timer2 + 2) / 4;
		if (stepTicks > 20)
			stepTicks = 20;

		if (moveTicks < stepTicks) {
			moveTicks = stepTicks;
			actorWriteU16(*actor, kActorMoveTicks, moveTicks);
		}

		int32 moveDx = actorReadU32(*actor, kActorMoveDx);
		int32 moveDy = actorReadU32(*actor, kActorMoveDy);
		int32 stepDx = ((int32)stepTicks * moveDx) / moveTicks;
		int32 stepDy = ((int32)stepTicks * moveDy) / moveTicks;
		actorWriteU32(*actor, kActorXFixed, actorReadU32(*actor, kActorXFixed) + stepDx);
		actorWriteU32(*actor, kActorYFixed, actorReadU32(*actor, kActorYFixed) + stepDy);
		actorWriteU32(*actor, kActorMoveDx, moveDx - stepDx);
		actorWriteU32(*actor, kActorMoveDy, moveDy - stepDy);
		moveTicks -= stepTicks;
		if (moveTicks < 0)
			moveTicks = 0;

		actorWriteU16(*actor, kActorMoveTicks, moveTicks);
		if (moveTicks && actorReadU8(*actor, kActorBlockingMove)) {
			if (_stringTable.size() > 3) {
				actorWriteU16(*actor, kActorStringRef, _stringTable[2]);
				actorWriteU16(*actor, kActorStringRef + 2, _stringTable[3]);
			}

			descendChildren = true;
			return false;
		}

		if (!moveTicks && actorReadU16(*actor, kActorCompletionKey))
			keyBitSet(actorReadU16(*actor, kActorCompletionKey));
	}

	if (timerWaitYield) {
		if (_stringTable.size() > 3) {
			actorWriteU16(*actor, kActorStringRef, _stringTable[2]);
			actorWriteU16(*actor, kActorStringRef + 2, _stringTable[3]);
		}

		descendChildren = true;
		return false;
	}

	uint32 pc = actorReadU32(*actor, kActorCurrentPc);
	for (;;) {
		ScriptDispatchStatus status = scriptStep(*actor, pc);
		if (status == kScriptContinue)
			continue;

		if (status == kScriptYield) {
			if (_stringTable.size() > 3) {
				actorWriteU16(*actor, kActorStringRef, _stringTable[2]);
				actorWriteU16(*actor, kActorStringRef + 2, _stringTable[3]);
			}

			actorWriteU32(*actor, kActorCurrentPc, pc);
		}

		descendChildren = status == kScriptYield && !_actorDestroyedCurrent;
		return status == kScriptDeactivatedRoot || _actorDestroyedCurrent;
	}
}

bool ComfyEngine::actorTickTree(uint16 actorIndex) {
	if (_pendingScene)
		return false;

	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return false;

	if (!actorReadU8(*actor, kActorActive)) {
		actorClearDirtyTree(actorIndex);
		return false;
	}

	bool descendChildren = true;
	bool stopBranch = actorRunScript(actorIndex, descendChildren);
	actor = actorGetPtr(actorIndex);
	if (stopBranch || !descendChildren || !actor)
		return stopBranch;

	uint16 child = actorReadU16(*actor, kActorSiblingHead);
	while (child) {
		Actor *childActor = actorGetPtr(child);
		uint16 savedNext = childActor ? actorReadU16(*childActor, kActorNextLink) : 0;
		if (actorTickTree(child))
			child = savedNext;
		else {
			childActor = actorGetPtr(child);
			child = childActor ? actorReadU16(*childActor, kActorNextLink) : 0;
		}
	}

	child = actorReadU16(*actor, kActorChildHead);
	while (child) {
		Actor *childActor = actorGetPtr(child);
		uint16 savedNext = childActor ? actorReadU16(*childActor, kActorNextLink) : 0;
		if (actorTickTree(child))
			child = savedNext;
		else {
			childActor = actorGetPtr(child);
			child = childActor ? actorReadU16(*childActor, kActorNextLink) : 0;
		}
	}

	return false;
}

void ComfyEngine::actorDrawList(uint16 actorIndex, int16 x, int16 y) {
	while (actorIndex) {
		actorDraw(actorIndex, x, y);
		Actor *actor = actorGetPtr(actorIndex);
		actorIndex = actor ? actorReadU16(*actor, kActorNextLink) : 0;
	}
}

uint16 ComfyEngine::scriptEvalKeyMask(uint32 pc, uint16 mode, VideoRectRecord &maskRecord,
		VideoRectRecord *rects, int16 baseX, int16 baseY) {
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

		VideoRectRecord *rect = bitIndex < COMFY_RESOLUTION_CHANGE_CAPACITY ? &rects[bitIndex] : nullptr;
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
		if (_keymaskX >= 0x0280 || _keymaskX <= (int16)0xFEC0 ||
				_keymaskY >= 0x0190 || _keymaskY <= (int16)0xFF38)
			continue;

		SpriteResource *sprite = spriteGetPtr((int16)spriteId);
		if (!sprite)
			continue;

		int16 left = _keymaskX - sprite->header.hotspotX;
		int16 top = _keymaskY - sprite->header.hotspotY;
		if (mode == 1)
			spriteBlitClipped((int16)spriteId, left, top);

		if (rect) {
			rect->left = left;
			rect->top = top;
			rect->right = left + sprite->header.width;
			rect->bottom = top + sprite->header.height;
			rect->area = sprite->header.width * sprite->header.height;
		}
	}

	_keymaskLastIndex = bitIndex;
	return bitIndex;
}

void ComfyEngine::actorEvalFrameSelection(uint16 actorIndex, int16 x, int16 y) {
	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return;

	uint32 selector = actorReadU32(*actor, kActorCachedSprite);
	VideoRectRecord rect = actorReadCachedRect(*actor);
	if (selector & 0xFF000000) {
		uint16 last = scriptEvalKeyMask(selector & 0x00FFFFFF, 0, rect, _keymaskInvalidationRects,
			rect.left, rect.top);
		if (last != 0xFFFF && last >= COMFY_RESOLUTION_CHANGE_CAPACITY)
			last = COMFY_RESOLUTION_CHANGE_CAPACITY - 1;

		if (last != 0xFFFF) {
			for (uint i = 0; i <= last; i++)
				videoFindBestMode(_keymaskInvalidationRects[i]);
		}
	} else {
		videoFindBestMode(rect);
	}

	actorWriteU32(*actor, kActorCachedSprite, 0);
	uint16 child = actorReadU16(*actor, kActorSiblingHead);
	while (child) {
		actorEvalFrameSelection(child, x, y);
		Actor *childActor = actorGetPtr(child);
		child = childActor ? actorReadU16(*childActor, kActorNextLink) : 0;
	}

	child = actorReadU16(*actor, kActorChildHead);
	while (child) {
		actorEvalFrameSelection(child, x, y);
		Actor *childActor = actorGetPtr(child);
		child = childActor ? actorReadU16(*childActor, kActorNextLink) : 0;
	}
}

ComfyEngine::VideoRectRecord ComfyEngine::actorReadCachedRect(Actor &actor) {
	VideoRectRecord rect;
	rect.left = actorReadU16(actor, kActorCachedRect);
	rect.top = actorReadU16(actor, kActorCachedRect + 2);
	rect.right = actorReadU16(actor, kActorCachedRect + 4);
	rect.bottom = actorReadU16(actor, kActorCachedRect + 6);
	rect.area = actorReadU16(actor, kActorCachedRect + 8);
	return rect;
}

void ComfyEngine::actorWriteCachedRect(Actor &actor, VideoRectRecord rect) {
	actorWriteU16(actor, kActorCachedRect, rect.left);
	actorWriteU16(actor, kActorCachedRect + 2, rect.top);
	actorWriteU16(actor, kActorCachedRect + 4, rect.right);
	actorWriteU16(actor, kActorCachedRect + 6, rect.bottom);
	actorWriteU16(actor, kActorCachedRect + 8, rect.area);
}

bool ComfyEngine::actorDraw(uint16 actorIndex, int16 x, int16 y) {
	Actor *actor = actorGetPtr(actorIndex);
	if (!actor)
		return true;

	if (!animFrameShouldDraw(1))
		return false;

	int16 drawX = (int16)((int32)actorReadU32(*actor, kActorXFixed) >> 12) + x;
	int16 drawY = (int16)((int32)actorReadU32(*actor, kActorYFixed) >> 12) + y;
	byte oldVisible = actorReadU8(*actor, kActorCachedVisible);
	uint32 oldSelector = actorReadU32(*actor, kActorCachedSprite);
	VideoRectRecord oldRect = actorReadCachedRect(*actor);
	if (!actorReadU8(*actor, kActorVisible)) {
		if (_videoMode == 2 && actorReadU8(*actor, kActorVisible) != oldVisible) {
			actorEvalFrameSelection(actorIndex, drawX, drawY);
			actorWriteU8(*actor, kActorCachedVisible, actorReadU8(*actor, kActorVisible));
		}

		return true;
	}

	animFrameRecordVocCounter(1);
	actorDrawList(actorReadU16(*actor, kActorSiblingHead), drawX, drawY);
	uint32 selector = actorReadU32(*actor, kActorSpriteSelector);
	bool oldScripted = (oldSelector & 0xFF000000) != 0;
	bool newScripted = (selector & 0xFF000000) != 0;
	VideoRectRecord newRect;
	uint16 scriptedLastRect = 0xFFFF;
	if (newScripted) {
		uint32 pc = selector & 0x00FFFFFF;
		if (_videoMode == 2) {
			_keymaskCurrentRecord.left = drawX;
			_keymaskCurrentRecord.top = drawY;
			_keymaskCurrentRecord.right = 0;
			_keymaskCurrentRecord.bottom = 0;
			_keymaskCurrentRecord.area = 0;
			scriptedLastRect = scriptEvalKeyMask(pc, 1, _keymaskCurrentRecord, _keymaskRects, drawX, drawY);
			if (scriptedLastRect != 0xFFFF && scriptedLastRect >= COMFY_RESOLUTION_CHANGE_CAPACITY)
				scriptedLastRect = COMFY_RESOLUTION_CHANGE_CAPACITY - 1;
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

				if (spriteId) {
					SpriteResource *sprite = spriteGetPtr((int16)spriteId);
					if (sprite)
						spriteBlitClipped((int16)spriteId, drawX + dx - sprite->header.hotspotX,
							drawY + dy - sprite->header.hotspotY);
				}
			}
		}

		if (_videoMode == 2 && scriptedLastRect != 0xFFFF) {
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
		animFrameBlitAt(drawX, drawY);
	} else if (selector) {
		SpriteResource *sprite = spriteLookup((uint16)selector, drawX, drawY);
		if (sprite) {
			newRect.left = drawX - sprite->header.hotspotX;
			newRect.top = drawY - sprite->header.hotspotY;
			newRect.right = newRect.left + sprite->header.width;
			newRect.bottom = newRect.top + sprite->header.height;
			newRect.area = sprite->header.width * sprite->header.height;
			spriteBlitClipped((int16)selector, drawX - sprite->header.hotspotX, drawY - sprite->header.hotspotY);
		}
	}
	animFrameRecordVocCounter(2);

	if (_videoMode == 2) {
		byte newVisible = actorReadU8(*actor, kActorVisible);
		bool changed = oldSelector != selector || oldVisible != newVisible;
		if (oldRect.left != newRect.left || oldRect.top != newRect.top ||
				oldRect.right != newRect.right || oldRect.bottom != newRect.bottom || oldRect.area != newRect.area)
			changed = true;

		uint16 oldScriptedLastRect = 0xFFFF;
		if (oldScripted) {
			VideoRectRecord maskRecord = oldRect;
			oldScriptedLastRect = scriptEvalKeyMask(oldSelector & 0x00FFFFFF, 0, maskRecord,
				_keymaskOldRects, oldRect.left, oldRect.top);
			if (oldScriptedLastRect != 0xFFFF && oldScriptedLastRect >= COMFY_RESOLUTION_CHANGE_CAPACITY)
				oldScriptedLastRect = COMFY_RESOLUTION_CHANGE_CAPACITY - 1;
		}

		if (changed) {
			bool comparedScriptRects = false;
			if (newScripted && oldScripted && oldSelector == selector &&
					oldRect.left == _keymaskCurrentRecord.left && oldRect.top == _keymaskCurrentRecord.top &&
					scriptedLastRect != 0xFFFF && oldScriptedLastRect != 0xFFFF) {
				uint16 last = MAX(scriptedLastRect, oldScriptedLastRect);
				for (uint i = 0; i <= last; i++) {
					VideoRectRecord *oldItem = i <= oldScriptedLastRect ? &_keymaskOldRects[i] : nullptr;
					VideoRectRecord *newItem = i <= scriptedLastRect ? &_keymaskRects[i] : nullptr;
					bool equal = oldItem && newItem && oldItem->left == newItem->left && oldItem->top == newItem->top &&
						oldItem->right == newItem->right && oldItem->bottom == newItem->bottom && oldItem->area == newItem->area;
					if (!equal) {
						if (oldItem)
							videoFindBestMode(*oldItem);

						if (newItem)
							videoFindBestMode(*newItem);
					}
				}

				comparedScriptRects = true;
			} else if (newScripted && scriptedLastRect != 0xFFFF) {
				for (uint i = 0; i <= scriptedLastRect; i++)
					videoFindBestMode(_keymaskRects[i]);
			} else if (!newScripted) {
				videoFindBestMode(newRect);
			}

			if (!comparedScriptRects) {
				if (oldScripted && oldScriptedLastRect != 0xFFFF) {
					for (uint i = 0; i <= oldScriptedLastRect; i++)
						videoFindBestMode(_keymaskOldRects[i]);
				} else if (!oldScripted) {
					videoFindBestMode(oldRect);
				}
			}
		}

		actorWriteU8(*actor, kActorCachedVisible, newVisible);
		actorWriteU32(*actor, kActorCachedSprite, selector);
		actorWriteCachedRect(*actor, newRect);
	}

	actorDrawList(actorReadU16(*actor, kActorChildHead), drawX, drawY);
	return animFrameShouldDraw(2);
}


} // End of namespace Comfy
