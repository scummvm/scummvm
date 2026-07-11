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

bool ComfyEngine::spriteLoad(int16 spriteId) {
	if (spriteId <= 0 || uint16(spriteId) >= _spriteResources.size())
		return false;

	SpriteResource &sprite = _spriteResources[spriteId];
	if (sprite.loaded && _objectCacheEntries[spriteId].slotSize != 0xFFFF)
		return true;

	sprite.loaded = false;
	sprite.pixels.clear();

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
	uint32 slotSize = (sprite.pixels.size() + (sprite.pixels.size() & 1) + 0x0C);
	if (slotSize <= _scenePoolData.size()) {
		if (_scenePoolCursor > _scenePoolData.size() - slotSize) {
			_scenePoolCursor = 0;
			_scenePoolEvictCursor = 0;
			for (uint i = 0; i < _objectCacheEntries.size(); i++)
				_objectCacheEntries[i].slotSize = 0xFFFF;

			for (uint i = 0; i < _frameCacheEntries.size(); i++)
				_frameCacheEntries[i].slotSize = 0xFFFF;
		}

		SpriteCacheEntry &entry = _objectCacheEntries[spriteId];
		entry.poolOffset = _scenePoolCursor;
		entry.slotSize = slotSize;
		WRITE_LE_UINT16(&_scenePoolData[_scenePoolCursor], uint16(spriteId));
		WRITE_LE_UINT16(&_scenePoolData[_scenePoolCursor + 2], slotSize);
		memcpy(&_scenePoolData[_scenePoolCursor + 0x0C], &sprite.pixels[0], sprite.pixels.size());
		_scenePoolCursor += slotSize;
	}

	return true;
}

ComfyEngine::SpriteResource *ComfyEngine::spriteGet(int16 spriteId) {
	if (spriteId > 0)
		return spriteLoad(spriteId) ? &_spriteResources[spriteId] : nullptr;

	if (!spriteId)
		return nullptr;

	uint16 tile = uint16(-spriteId) - 1;
	uint32 offset = uint32(tile) * COMFY_TILE_SIZE;
	if (offset >= _picDataSize || offset >= _comfyObjData.size())
		return nullptr;

	uint32 size = MIN<uint32>(COMFY_TILE_SIZE, MIN<uint32>(_picDataSize - offset, _comfyObjData.size() - offset));
	_frameSpriteResource.id = spriteId;
	memset(&_frameSpriteResource.header, 0, sizeof(_frameSpriteResource.header));
	_frameSpriteResource.header.dataSize = size;
	_frameSpriteResource.header.tiledSize = size;
	_frameSpriteResource.pixels.resize(size);
	memcpy(&_frameSpriteResource.pixels[0], &_comfyObjData[offset], size);
	_frameSpriteResource.loaded = true;
	if (uint16(-spriteId) < _frameCacheEntries.size()) {
		SpriteCacheEntry &entry = _frameCacheEntries[-spriteId];
		entry.poolOffset = offset;
		entry.slotSize = uint16(size + (size & 1) + 0x0C);
	}

	return &_frameSpriteResource;
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
	SpriteResource *loadedSprite = spriteGet(spriteId);
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
					int16 screenX = _mirrorMode ? int16(_logicalScreenWidth - 1 - drawX) : drawX;
					if (screenX >= 0 && screenX < int16(_logicalScreenWidth) && drawY >= 0 && drawY < int16(_logicalScreenHeight))
						_framebufPtr[drawY * _logicalScreenWidth + screenX] = value;
				}
			} else if (op & 0x40) {
				drawX += count;
			} else {
				if (packetPos + count > rowPos + rowSize)
					return;

				for (uint i = 0; i < count; i++, drawX++) {
					byte value = sprite.pixels[packetPos++];
					int16 screenX = _mirrorMode ? int16(_logicalScreenWidth - 1 - drawX) : drawX;
					if (screenX >= 0 && screenX < int16(_logicalScreenWidth) && drawY >= 0 && drawY < int16(_logicalScreenHeight))
						_framebufPtr[drawY * _logicalScreenWidth + screenX] = value;
				}
			}
		}

		rowPos += rowSize;
	}

	renderSetDirty();
}

uint32 ComfyEngine::actorReadDword(Actor &actor, uint offset) {
	if (offset + 4 > COMFY_ACTOR_SIZE) {
		_scriptFault = true;
		return 0;
	}

	return READ_LE_UINT32(actor.raw + offset);
}

uint16 ComfyEngine::actorReadWord(Actor &actor, uint offset) {
	if (offset + 2 > COMFY_ACTOR_SIZE) {
		_scriptFault = true;
		return 0;
	}

	return READ_LE_UINT16(actor.raw + offset);
}

byte ComfyEngine::actorReadByte(Actor &actor, uint offset) {
	if (offset >= COMFY_ACTOR_SIZE) {
		_scriptFault = true;
		return 0;
	}

	return actor.raw[offset];
}

void ComfyEngine::actorWriteDword(Actor &actor, uint offset, uint32 value) {
	if (offset + 4 <= COMFY_ACTOR_SIZE)
		WRITE_LE_UINT32(actor.raw + offset, value);
	else
		_scriptFault = true;
}

void ComfyEngine::actorWriteWord(Actor &actor, uint offset, uint16 value) {
	if (offset + 2 <= COMFY_ACTOR_SIZE)
		WRITE_LE_UINT16(actor.raw + offset, value);
	else
		_scriptFault = true;
}

void ComfyEngine::actorWriteByte(Actor &actor, uint offset, byte value) {
	if (offset < COMFY_ACTOR_SIZE)
		actor.raw[offset] = value;
	else
		_scriptFault = true;
}

ComfyEngine::Actor *ComfyEngine::actorGet(uint16 actorIndex) {
	return actorIndex < _actors.size() ? &_actors[actorIndex] : nullptr;
}

ComfyEngine::Actor *ComfyEngine::actorResolve(uint16 sceneOrActor, uint16 fallbackActor) {
	if (!sceneOrActor)
		return actorGet(fallbackActor);

	return sceneOrActor < _sceneHandles.size() ? actorGet(_sceneHandles[sceneOrActor]) : nullptr;
}

uint16 ComfyEngine::actorAllocate(uint16 sceneSlot) {
	Actor *root = actorGet(0);
	uint16 actorIndex = root ? actorReadWord(*root, kActorNextLink) : 0;
	if (!actorIndex || sceneSlot >= _sceneHandles.size())
		return 0;

	Actor *actor = actorGet(actorIndex);
	actorWriteWord(*root, kActorNextLink, actorReadWord(*actor, kActorNextLink));
	_sceneHandles[sceneSlot] = actorIndex;
	actorWriteWord(*actor, kActorSceneHandle, sceneSlot);
	return actorIndex;
}

void ComfyEngine::actorFree(uint16 sceneSlot) {
	if (sceneSlot >= _sceneHandles.size())
		return;

	uint16 actorIndex = _sceneHandles[sceneSlot];
	Actor *actor = actorGet(actorIndex);
	if (!actor)
		return;

	Actor *root = actorGet(0);
	actorWriteWord(*actor, kActorNextLink, root ? actorReadWord(*root, kActorNextLink) : 0);
	actorWriteWord(*actor, kActorSceneHandle, 0);
	if (root)
		actorWriteWord(*root, kActorNextLink, actorIndex);
	_sceneHandles[sceneSlot] = 0;
}

void ComfyEngine::actorInsertChild(uint16 childIndex, uint16 parentIndex) {
	Actor *child = actorGet(childIndex);
	Actor *parent = actorGet(parentIndex);
	if (!child || !parent)
		return;

	uint16 tail = actorReadWord(*parent, kActorChildTail);
	if (tail) {
		actorWriteWord(*actorGet(tail), kActorNextLink, childIndex);
		actorWriteWord(*child, kActorPrevLink, tail);
	} else {
		actorWriteWord(*parent, kActorChildHead, childIndex);
		actorWriteWord(*child, kActorPrevLink, 0);
	}

	actorWriteWord(*child, kActorNextLink, 0);
	actorWriteWord(*parent, kActorChildTail, childIndex);
}

void ComfyEngine::actorInsertSibling(uint16 actorIndex, uint16 ownerIndex) {
	Actor *actor = actorGet(actorIndex);
	Actor *owner = actorGet(ownerIndex);
	if (!actor || !owner)
		return;

	uint16 head = actorReadWord(*owner, kActorSiblingHead);
	if (head)
		actorWriteWord(*actorGet(head), kActorPrevLink, actorIndex);

	actorWriteWord(*actor, kActorNextLink, head);
	actorWriteWord(*actor, kActorPrevLink, 0);
	actorWriteWord(*owner, kActorSiblingHead, actorIndex);
}

void ComfyEngine::actorUnlink(uint16 actorIndex) {
	Actor *actor = actorGet(actorIndex);
	if (!actor)
		return;

	Actor *parent = actorGet(actorReadWord(*actor, kActorParent));
	if (!parent)
		return;

	uint16 next = actorReadWord(*actor, kActorNextLink);
	uint16 previous = actorReadWord(*actor, kActorPrevLink);
	if (next)
		actorWriteWord(*actorGet(next), kActorPrevLink, previous);
	else if (actorReadWord(*parent, kActorChildTail) == actorIndex)
		actorWriteWord(*parent, kActorChildTail, previous);

	if (previous)
		actorWriteWord(*actorGet(previous), kActorNextLink, next);
	else if (actorReadWord(*parent, kActorSiblingHead) == actorIndex)
		actorWriteWord(*parent, kActorSiblingHead, next);
	else
		actorWriteWord(*parent, kActorChildHead, next);
}

uint16 ComfyEngine::actorInit(uint16 sceneSlot, uint16 parentSlot, byte visible, byte active,
		uint32 pc, int16 x, int16 y, int16 sprite, byte insertAsChild) {
	uint16 actorIndex = actorAllocate(sceneSlot);
	Actor *actor = actorGet(actorIndex);
	if (!actor)
		return 8;

	actorWriteWord(*actor, kActorSceneHandle, sceneSlot);
	actorWriteByte(*actor, kActorVisible, visible);
	actorWriteByte(*actor, kActorActive, active);
	actorWriteDword(*actor, kActorCurrentPc, pc);
	actorWriteDword(*actor, kActorResetPc, pc);
	actorWriteDword(*actor, kActorXFixed, uint32(int32(x) * 0x1000));
	actorWriteDword(*actor, kActorYFixed, uint32(int32(y) * 0x1000));
	actorWriteDword(*actor, kActorSpriteSelector, uint32(int32(sprite)));
	actorWriteByte(*actor, kActorDirty, 1);

	if (parentSlot) {
		uint16 parentIndex = parentSlot < _sceneHandles.size() ? _sceneHandles[parentSlot] : 0;
		actorWriteWord(*actor, kActorParent, parentIndex);
		if (insertAsChild)
			actorInsertChild(actorIndex, parentIndex);
		else
			actorInsertSibling(actorIndex, parentIndex);
	}

	return 1;
}

void ComfyEngine::actorSetPc(Actor &actor, uint32 pc) {
	uint32 previous = actorReadDword(actor, kActorCallPc);
	uint32 packed = pc;
	if (previous) {
		byte entry = _actorPcTable[0] >> 24;
		if (!entry || entry >= COMFY_ACTOR_PC_TABLE_COUNT) {
			_scriptFault = true;
			return;
		}

		_actorPcTable[0] = _actorPcTable[entry];
		_actorPcTable[entry] = previous;
		if (_sceneActorPcOffset + sizeof(_actorPcTable) <= _sceneMemoryBlock.size()) {
			WRITE_LE_UINT32(&_sceneMemoryBlock[_sceneActorPcOffset], _actorPcTable[0]);
			WRITE_LE_UINT32(&_sceneMemoryBlock[_sceneActorPcOffset + entry * 4], _actorPcTable[entry]);
		}

		packed = (uint32(entry) << 24) | (pc & 0x00FFFFFF);
	}

	actorWriteDword(actor, kActorCallPc, packed);
}

uint32 ComfyEngine::actorPopPc(Actor &actor) {
	uint32 packed = actorReadDword(actor, kActorCallPc);
	byte entry = packed >> 24;
	if (entry && entry < COMFY_ACTOR_PC_TABLE_COUNT) {
		actorWriteDword(actor, kActorCallPc, _actorPcTable[entry]);
		_actorPcTable[entry] = _actorPcTable[0];
		_actorPcTable[0] = uint32(entry) << 24;
		if (_sceneActorPcOffset + sizeof(_actorPcTable) <= _sceneMemoryBlock.size()) {
			WRITE_LE_UINT32(&_sceneMemoryBlock[_sceneActorPcOffset], _actorPcTable[0]);
			WRITE_LE_UINT32(&_sceneMemoryBlock[_sceneActorPcOffset + entry * 4], _actorPcTable[entry]);
		}
	} else {
		actorWriteDword(actor, kActorCallPc, 0);
	}

	return packed & 0x00FFFFFF;
}

void ComfyEngine::actorFreePcChain(Actor &actor) {
	byte first = actorReadDword(actor, kActorCallPc) >> 24;
	byte entry = first;
	while (entry && entry < COMFY_ACTOR_PC_TABLE_COUNT) {
		byte next = _actorPcTable[entry] >> 24;
		if (!next) {
			_actorPcTable[entry] = _actorPcTable[0];
			_actorPcTable[0] = uint32(first) << 24;
			if (_sceneActorPcOffset + sizeof(_actorPcTable) <= _sceneMemoryBlock.size())
				memcpy(&_sceneMemoryBlock[_sceneActorPcOffset], _actorPcTable, sizeof(_actorPcTable));

			break;
		}

		_actorPcTable[entry] &= 0xFF000000;
		entry = next;
	}

	actorWriteDword(actor, kActorCallPc, 0);
}

void ComfyEngine::actorFreeTree(uint16 actorIndex) {
	Actor *actor = actorGet(actorIndex);
	if (!actor)
		return;

	uint16 child = actorReadWord(*actor, kActorSiblingHead);
	while (child) {
		Actor *childActor = actorGet(child);
		uint16 next = childActor ? actorReadWord(*childActor, kActorNextLink) : 0;
		actorFreeTree(child);
		child = next;
	}

	child = actorReadWord(*actor, kActorChildHead);
	while (child) {
		Actor *childActor = actorGet(child);
		uint16 next = childActor ? actorReadWord(*childActor, kActorNextLink) : 0;
		actorFreeTree(child);
		child = next;
	}

	uint16 sceneSlot = actorReadWord(*actor, kActorSceneHandle);
	actorFreePcChain(*actor);
	actorFree(sceneSlot);
}

void ComfyEngine::actorClearDirtyTree(uint16 actorIndex) {
	Actor *actor = actorGet(actorIndex);
	if (!actor)
		return;

	actorWriteByte(*actor, kActorDirty, 0);
	uint16 child = actorReadWord(*actor, kActorSiblingHead);
	while (child) {
		Actor *childActor = actorGet(child);
		uint16 next = childActor ? actorReadWord(*childActor, kActorNextLink) : 0;
		actorClearDirtyTree(child);
		child = next;
	}

	child = actorReadWord(*actor, kActorChildHead);
	while (child) {
		Actor *childActor = actorGet(child);
		uint16 next = childActor ? actorReadWord(*childActor, kActorNextLink) : 0;
		actorClearDirtyTree(child);
		child = next;
	}
}

void ComfyEngine::actorSetAllDirty() {
	for (uint i = 0; i + 1 < _actors.size(); i++)
		actorWriteByte(_actors[i], kActorDirty, 1);
}

bool ComfyEngine::actorRunScript(uint16 actorIndex, bool &descendChildren) {
	Actor *actor = actorGet(actorIndex);
	descendChildren = false;
	if (!actor)
		return false;

	_currentActor = actorIndex;
	_actorDestroyedCurrent = false;
	if (_stringTable.size() > 3) {
		_stringTable[2] = actorReadWord(*actor, kActorStringRef);
		_stringTable[3] = actorReadWord(*actor, kActorStringRef + 2);
	}

	uint16 triggerKey = actorReadWord(*actor, kActorTriggerKey);
	if (triggerKey && keyBitTest(triggerKey)) {
		byte triggerFlags = actorReadByte(*actor, kActorTriggerFlags);
		if (triggerFlags & 1)
			actorSetPc(*actor, actorReadDword(*actor, kActorCurrentPc));

		if (triggerFlags & 4)
			keyBitClear(triggerKey);

		if (triggerFlags & 2)
			actorWriteWord(*actor, kActorTriggerKey, 0);

		actorWriteWord(*actor, kActorWaitTarget, 0);
		actorWriteWord(*actor, kActorWaitAccum, 0);
		actorWriteDword(*actor, kActorCurrentPc, actorReadDword(*actor, kActorTriggerPc));
	}

	int16 moveTicks = actorReadWord(*actor, kActorMoveTicks);
	if (moveTicks) {
		int16 stepTicks = int16(uint16(_timerCurrent + _timer0 + _timer1 + _timer2 + 2)) / 4;
		if (stepTicks > 20)
			stepTicks = 20;

		if (moveTicks < stepTicks) {
			moveTicks = stepTicks;
			actorWriteWord(*actor, kActorMoveTicks, moveTicks);
		}

		int32 moveDx = actorReadDword(*actor, kActorMoveDx);
		int32 moveDy = actorReadDword(*actor, kActorMoveDy);
		int32 stepDx = (int32(stepTicks) * moveDx) / moveTicks;
		int32 stepDy = (int32(stepTicks) * moveDy) / moveTicks;
		actorWriteDword(*actor, kActorXFixed, actorReadDword(*actor, kActorXFixed) + stepDx);
		actorWriteDword(*actor, kActorYFixed, actorReadDword(*actor, kActorYFixed) + stepDy);
		actorWriteDword(*actor, kActorMoveDx, moveDx - stepDx);
		actorWriteDword(*actor, kActorMoveDy, moveDy - stepDy);
		moveTicks -= stepTicks;
		if (moveTicks < 0)
			moveTicks = 0;

		actorWriteWord(*actor, kActorMoveTicks, moveTicks);
		if (moveTicks && actorReadByte(*actor, kActorBlockingMove)) {
			if (_stringTable.size() > 3) {
				actorWriteWord(*actor, kActorStringRef, _stringTable[2]);
				actorWriteWord(*actor, kActorStringRef + 2, _stringTable[3]);
			}

			descendChildren = true;
			return false;
		}

		if (!moveTicks && actorReadWord(*actor, kActorCompletionKey))
			keyBitSet(actorReadWord(*actor, kActorCompletionKey));
	}

	int16 waitTarget = actorReadWord(*actor, kActorWaitTarget);
	if (waitTarget) {
		if (waitTarget > _timerCurrent) {
			actorWriteWord(*actor, kActorWaitTarget, waitTarget - _timerCurrent);
			if (_stringTable.size() > 3) {
				actorWriteWord(*actor, kActorStringRef, _stringTable[2]);
				actorWriteWord(*actor, kActorStringRef + 2, _stringTable[3]);
			}

			descendChildren = true;
			return false;
		}

		int16 elapsed = _timerCurrent - waitTarget;
		actorWriteWord(*actor, kActorWaitAccum, actorReadWord(*actor, kActorWaitAccum) + elapsed);
		actorWriteWord(*actor, kActorWaitTarget, 0);
	}

	uint32 pc = actorReadDword(*actor, kActorCurrentPc);
	for (;;) {
		ScriptDispatchStatus status = scriptStep(*actor, pc);
		if (status == kScriptContinue) {
			actorWriteDword(*actor, kActorCurrentPc, pc);
			continue;
		}

		if (status == kScriptYield && _stringTable.size() > 3) {
			actorWriteWord(*actor, kActorStringRef, _stringTable[2]);
			actorWriteWord(*actor, kActorStringRef + 2, _stringTable[3]);
		}

		descendChildren = status == kScriptYield && !_actorDestroyedCurrent;
		return status == kScriptDeactivatedRoot || _actorDestroyedCurrent;
	}
}

bool ComfyEngine::actorTickTreeInternal(uint16 actorIndex) {
	if (_pendingScene)
		return false;

	Actor *actor = actorGet(actorIndex);
	if (!actor)
		return false;

	if (!actorReadByte(*actor, kActorActive)) {
		actorClearDirtyTree(actorIndex);
		return false;
	}

	bool descendChildren = true;
	bool stopBranch = actorRunScript(actorIndex, descendChildren);
	actor = actorGet(actorIndex);
	if (stopBranch || !descendChildren || !actor)
		return stopBranch;

	uint16 child = actorReadWord(*actor, kActorSiblingHead);
	while (child) {
		Actor *childActor = actorGet(child);
		uint16 savedNext = childActor ? actorReadWord(*childActor, kActorNextLink) : 0;
		if (actorTickTreeInternal(child))
			child = savedNext;
		else {
			childActor = actorGet(child);
			child = childActor ? actorReadWord(*childActor, kActorNextLink) : 0;
		}
	}

	child = actorReadWord(*actor, kActorChildHead);
	while (child) {
		Actor *childActor = actorGet(child);
		uint16 savedNext = childActor ? actorReadWord(*childActor, kActorNextLink) : 0;
		if (actorTickTreeInternal(child))
			child = savedNext;
		else {
			childActor = actorGet(child);
			child = childActor ? actorReadWord(*childActor, kActorNextLink) : 0;
		}
	}

	return false;
}

void ComfyEngine::actorDrawList(uint16 actorIndex, int16 x, int16 y) {
	while (actorIndex) {
		actorDraw(actorIndex, x, y);
		Actor *actor = actorGet(actorIndex);
		actorIndex = actor ? actorReadWord(*actor, kActorNextLink) : 0;
	}
}

void ComfyEngine::actorDrawScripted(uint16 actorIndex, uint32 selector, int16 x, int16 y) {
	uint32 pc = selector & 0x00FFFFFF;
	int16 wordsLeft = int16(scriptReadWord(pc) - 1);
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
			SpriteResource *sprite = spriteGet(int16(spriteId));
			if (sprite)
				spriteBlitClipped(int16(spriteId), x + dx - sprite->header.hotspotX, y + dy - sprite->header.hotspotY);
		}
	}

	(void)actorIndex;
}

ComfyEngine::VideoRectRecord ComfyEngine::actorReadCachedRect(Actor &actor) {
	VideoRectRecord rect;
	rect.left = actorReadWord(actor, kActorCachedRect);
	rect.top = actorReadWord(actor, kActorCachedRect + 2);
	rect.right = actorReadWord(actor, kActorCachedRect + 4);
	rect.bottom = actorReadWord(actor, kActorCachedRect + 6);
	rect.area = actorReadWord(actor, kActorCachedRect + 8);
	return rect;
}

void ComfyEngine::actorWriteCachedRect(Actor &actor, VideoRectRecord rect) {
	actorWriteWord(actor, kActorCachedRect, rect.left);
	actorWriteWord(actor, kActorCachedRect + 2, rect.top);
	actorWriteWord(actor, kActorCachedRect + 4, rect.right);
	actorWriteWord(actor, kActorCachedRect + 6, rect.bottom);
	actorWriteWord(actor, kActorCachedRect + 8, rect.area);
}

void ComfyEngine::actorInvalidateDrawTree(uint16 actorIndex) {
	Actor *actor = actorGet(actorIndex);
	if (!actor)
		return;

	VideoRectRecord rect = actorReadCachedRect(*actor);
	if (rect.area)
		videoFindBestMode(rect);

	actorWriteByte(*actor, kActorCachedVisible, 0);
	actorWriteDword(*actor, kActorCachedSprite, 0);
	VideoRectRecord empty = {0, 0, 0, 0, 0};
	actorWriteCachedRect(*actor, empty);
	uint16 child = actorReadWord(*actor, kActorSiblingHead);
	while (child) {
		Actor *childActor = actorGet(child);
		uint16 next = childActor ? actorReadWord(*childActor, kActorNextLink) : 0;
		actorInvalidateDrawTree(child);
		child = next;
	}

	child = actorReadWord(*actor, kActorChildHead);
	while (child) {
		Actor *childActor = actorGet(child);
		uint16 next = childActor ? actorReadWord(*childActor, kActorNextLink) : 0;
		actorInvalidateDrawTree(child);
		child = next;
	}
}

bool ComfyEngine::actorDraw(uint16 actorIndex, int16 x, int16 y) {
	Actor *actor = actorGet(actorIndex);
	if (!actor)
		return true;

	int16 drawX = int16(int32(actorReadDword(*actor, kActorXFixed)) >> 12) + x;
	int16 drawY = int16(int32(actorReadDword(*actor, kActorYFixed)) >> 12) + y;
	byte oldVisible = actorReadByte(*actor, kActorCachedVisible);
	uint32 oldSelector = actorReadDword(*actor, kActorCachedSprite);
	VideoRectRecord oldRect = actorReadCachedRect(*actor);
	if (!actorReadByte(*actor, kActorVisible)) {
		if (oldVisible)
			actorInvalidateDrawTree(actorIndex);

		return true;
	}

	actorDrawList(actorReadWord(*actor, kActorSiblingHead), drawX, drawY);
	uint32 selector = actorReadDword(*actor, kActorSpriteSelector);
	VideoRectRecord newRect = {0, 0, 0, 0, 0};
	if (selector & 0xFF000000) {
		actorDrawScripted(actorIndex, selector, drawX, drawY);
	} else if (selector) {
		SpriteResource *sprite = spriteGet(int16(selector));
		if (sprite) {
			newRect.left = drawX - sprite->header.hotspotX;
			newRect.top = drawY - sprite->header.hotspotY;
			newRect.right = newRect.left + sprite->header.width;
			newRect.bottom = newRect.top + sprite->header.height;
			newRect.area = sprite->header.width * sprite->header.height;
			spriteBlitClipped(int16(selector), drawX - sprite->header.hotspotX, drawY - sprite->header.hotspotY);
		}
	}

	bool rectChanged = oldRect.left != newRect.left || oldRect.top != newRect.top ||
		oldRect.right != newRect.right || oldRect.bottom != newRect.bottom || oldRect.area != newRect.area;
	if (oldSelector != selector || oldVisible != actorReadByte(*actor, kActorVisible) || rectChanged) {
		if (oldRect.area)
			videoFindBestMode(oldRect);

		if (newRect.area)
			videoFindBestMode(newRect);
	}

	actorWriteByte(*actor, kActorCachedVisible, actorReadByte(*actor, kActorVisible));
	actorWriteDword(*actor, kActorCachedSprite, selector);
	actorWriteCachedRect(*actor, newRect);
	actorDrawList(actorReadWord(*actor, kActorChildHead), drawX, drawY);
	return true;
}


} // End of namespace Comfy
