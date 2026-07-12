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
#include "comfy/midiplyr/midiplyr.h"

#include "common/endian.h"

namespace Comfy {

enum AnimFileCommand {
	kAnimCommandEnd = 0x2145,
	kAnimCommandStoreFrameBytes = 0x4356,
	kAnimCommandDirtyRect = 0x5842,
	kAnimCommandFrame = 0x5246
};

bool ComfyEngine::animFrameIsActive() {
	return _animActive;
}

bool ComfyEngine::animFrameIsReady() {
	return _animActive && _animFrameReady;
}

void ComfyEngine::animFrameReadStorage(byte *destination, uint32 offset, uint16 size) {
	if (offset > _animFrameStorage.size() || size > _animFrameStorage.size() - offset)
		error("Invalid animation storage read");

	if (size)
		memcpy(destination, &_animFrameStorage[offset], size);
}

void ComfyEngine::animFilePushStorageChunk(uint32 fileOffset, uint32 storageOffset, uint16 size) {
	for (int i = 5; i > 0; i--) {
		_animStorageChunkFileOffsets[i] = _animStorageChunkFileOffsets[i - 1];
		_animStorageChunkOffsets[i] = _animStorageChunkOffsets[i - 1];
		_animStorageChunkSizes[i] = _animStorageChunkSizes[i - 1];
	}

	_animStorageChunkFileOffsets[0] = fileOffset;
	_animStorageChunkOffsets[0] = storageOffset;
	_animStorageChunkSizes[0] = size;
}

void ComfyEngine::animFrameSetVocCounter(uint32 counter) {
	if (!_midiPlyrDriver)
		return;

	// The original uses a wave callback counter when the ANM header requests it.
	// ScummVM uses the MidiPlyr timer as the common host-side VOC clock.
	_midiPlyrDriver->setIncreaseVocCounter(1);
	_midiPlyrDriver->setVocCounter(counter);
}

bool ComfyEngine::animFileOpen() {
	if (!readAssetFile(Common::Path("ANMFILE.DAT"), _animFileData) ||
			_animFileData.size() < COMFY_ANMFILE_HEADER_BYTES)
		return false;

	uint16 magic = READ_LE_UINT16(&_animFileData[0]);
	if (magic != COMFY_ANMFILE_MAGIC && magic != COMFY_ANMFILE_PANTHER_MAGIC)
		return false;

	uint16 count = READ_LE_UINT16(&_animFileData[2]);
	if ((uint32)COMFY_ANMFILE_HEADER_BYTES + (uint32)count * 4 > _animFileData.size())
		return false;

	_animIndexTable.resize(count);
	for (uint i = 0; i < count; i++)
		_animIndexTable[i] = READ_LE_UINT32(&_animFileData[COMFY_ANMFILE_HEADER_BYTES + i * 4]);

	_animPantherFormat = magic == COMFY_ANMFILE_PANTHER_MAGIC;
	_animIndexLoaded = true;
	_animActive = false;
	_animUsesWaveVocCounter = false;
	return true;
}

void ComfyEngine::animFrameShutdown(bool freeBuffers) {
	if (_animActive) {
		if (_stringTable.size() > 14)
			_stringTable[14] = 0;

		if (_animCurrentFrameKey)
			keyBitSet(_animCurrentFrameKey);

		_animCurrentFrameKey = 0;
		_animFrameStorage.clear();
		_animActive = false;
	}

	if (freeBuffers) {
		_animFrameBuffer.clear();
		if (_animCurrentActorSceneHandle < _sceneHandles.size()) {
			Actor *actor = actorGetPtr(sceneGetHandle(_animCurrentActorSceneHandle));
			if (actor)
				actorWriteU32(*actor, kActorSpriteSelector, 0);
		}

		_animCurrentActorSceneHandle = 0;
	}
}

void ComfyEngine::animFileShutdown() {
	animFrameShutdown(true);
	_animFileData.clear();
	_animIndexTable.clear();
	_animIndexLoaded = false;
}

void ComfyEngine::animFileLoadFrame(uint16 animIndex, uint16 frameKey, uint16 actorSceneHandle) {
	if (!_animIndexLoaded || _animActive || animIndex >= _animIndexTable.size())
		return;

	animFrameShutdown(true);

	uint32 offset = _animIndexTable[animIndex];
	if (offset > _animFileData.size() || COMFY_ANMFILE_HEADER_BYTES > _animFileData.size() - offset)
		error("ANMFILE.DAT is truncated while loading animation %u", (uint)animIndex);

	memcpy(_animFrameHeader, &_animFileData[offset], sizeof(_animFrameHeader));
	uint16 magic = READ_LE_UINT16(_animFrameHeader);
	if (magic != COMFY_ANMFRAME_MAGIC && magic != COMFY_ANMFRAME_PANTHER_MAGIC)
		error("Invalid frame header in ANMFILE.DAT for animation %u", (uint)animIndex);

	_animFrameBuffer.resize(COMFY_ANMFRAME_BYTES);
	memset(&_animFrameBuffer[0], 0xFF, _animFrameBuffer.size());
	_animFrameStorage.clear();
	memset(_animStorageChunkFileOffsets, 0, sizeof(_animStorageChunkFileOffsets));
	memset(_animStorageChunkOffsets, 0, sizeof(_animStorageChunkOffsets));
	memset(_animStorageChunkSizes, 0, sizeof(_animStorageChunkSizes));
	memset(_animFrameCommandData, 0, sizeof(_animFrameCommandData));
	_animFrameCommandDataSize = 0;
	_animFrameCommandArgument = 0;
	_animFrameCommandFlag = false;
	_animPosition = offset + COMFY_ANMFILE_HEADER_BYTES;
	_animCurrentIndex = animIndex;
	_animCurrentActorSceneHandle = actorSceneHandle;
	_animCurrentFrameKey = frameKey;
	_animUsesWaveVocCounter = READ_LE_UINT32(_animFrameHeader + 4) != 0;
	_animVocClockHz = READ_LE_UINT16(_animFrameHeader + 8);
	_animVocTargetCounter = 0xFFFF;
	_animVocDeltaB = 0;
	_animVocDeltaA = 0;
	_animActive = true;
	_animFrameReady = false;

	if (actorSceneHandle < _sceneHandles.size()) {
		Actor *actor = actorGetPtr(sceneGetHandle(actorSceneHandle));
		if (actor)
			actorWriteU32(*actor, kActorSpriteSelector, 0x00FFFFFF);
	}

	animFileTickCommands();
}

uint32 ComfyEngine::animDecompressRle(const byte *source, uint32 sourceSize, byte *destination, uint32 destinationSize) {
	uint32 sourcePosition = 0;
	uint32 destinationPosition = 0;
	while (sourcePosition < sourceSize) {
		byte control = source[sourcePosition++];
		if (control == 0) {
			if (sourcePosition + 1 >= sourceSize)
				return destinationPosition;

			uint32 count = source[sourcePosition++];
			byte value = source[sourcePosition++];
			count = MIN<uint32>(count, destinationSize - destinationPosition);
			memset(destination + destinationPosition, value, count);
			destinationPosition += count;
			continue;
		}

		if (control == 0x80) {
			if (sourcePosition + 1 >= sourceSize)
				return destinationPosition;

			uint16 word = READ_LE_UINT16(source + sourcePosition);
			sourcePosition += 2;
			if (word == 0)
				return destinationPosition;

			if ((int16)word > 0) {
				destinationPosition = MIN<uint32>(destinationPosition + word, destinationSize);
				continue;
			}

			uint32 count = word - 0x8000;
			if ((count & 0x4000) == 0) {
				count = MIN<uint32>(count, sourceSize - sourcePosition);
				count = MIN<uint32>(count, destinationSize - destinationPosition);
				memcpy(destination + destinationPosition, source + sourcePosition, count);
				sourcePosition += count;
				destinationPosition += count;
				continue;
			}

			count -= 0x4000;
			if (sourcePosition >= sourceSize)
				return destinationPosition;

			byte value = source[sourcePosition++];
			count = MIN<uint32>(count, destinationSize - destinationPosition);
			memset(destination + destinationPosition, value, count);
			destinationPosition += count;
			continue;
		}

		if (control & 0x80) {
			destinationPosition = MIN<uint32>(destinationPosition + control - 0x80, destinationSize);
			continue;
		}

		uint32 count = MIN<uint32>(control, sourceSize - sourcePosition);
		count = MIN<uint32>(count, destinationSize - destinationPosition);
		memcpy(destination + destinationPosition, source + sourcePosition, count);
		sourcePosition += count;
		destinationPosition += count;
	}

	return destinationPosition;
}

uint32 ComfyEngine::animDecodePanther(const byte *source, uint32 sourceSize) {
	if (sourceSize < 6 || _animFrameBuffer.empty())
		return 0;

	uint16 kind = READ_LE_UINT16(source + 4);
	uint32 sourcePosition = 6;
	uint32 destinationPosition = 0;
	uint32 written = 0;
	if (kind == 7) {
		if (sourcePosition + 2 > sourceSize)
			return 0;

		uint16 rows = READ_LE_UINT16(source + sourcePosition);
		sourcePosition += 2;
		while (rows && sourcePosition + 2 <= sourceSize && destinationPosition < _animFrameBuffer.size()) {
			uint16 runCount = READ_LE_UINT16(source + sourcePosition);
			sourcePosition += 2;
			if (runCount & 0x4000) {
				destinationPosition += (uint32)(uint16)(0 - runCount) * COMFY_SCREEN_WIDTH;
				continue;
			}

			uint32 rowStart = destinationPosition;
			while (runCount && sourcePosition < sourceSize && destinationPosition < _animFrameBuffer.size()) {
				destinationPosition += source[sourcePosition++];
				if (sourcePosition >= sourceSize || destinationPosition >= _animFrameBuffer.size())
					return written;

				byte control = source[sourcePosition++];
				if (control & 0x80) {
					uint16 count = 0x100 - control;
					if (sourcePosition + 2 > sourceSize)
						return written;

					byte low = source[sourcePosition++];
					byte high = source[sourcePosition++];
					while (count-- && destinationPosition + 1 < _animFrameBuffer.size()) {
						_animFrameBuffer[destinationPosition++] = low;
						_animFrameBuffer[destinationPosition++] = high;
						written += 2;
					}
				} else {
					uint16 count = MIN<uint32>(control, sourceSize - sourcePosition);
					count = MIN<uint32>(count, _animFrameBuffer.size() - destinationPosition);
					memcpy(&_animFrameBuffer[destinationPosition], source + sourcePosition, count);
					sourcePosition += count;
					destinationPosition += count;
					written += count;
				}

				runCount--;
			}

			destinationPosition = rowStart + COMFY_SCREEN_WIDTH;
			rows--;
		}

		return written;
	}

	if (kind != 0x0F)
		return 0;

	uint16 width = READ_LE_UINT16(_animFrameHeader + 10);
	uint16 height = READ_LE_UINT16(_animFrameHeader + 12);
	for (uint16 row = 0; row < height && sourcePosition < sourceSize &&
			destinationPosition < _animFrameBuffer.size(); row++) {
		sourcePosition++;
		uint16 remaining = width;
		while (remaining && sourcePosition < sourceSize && destinationPosition < _animFrameBuffer.size()) {
			byte control = source[sourcePosition++];
			uint16 count = control;
			if (control & 0x80) {
				count = 0x100 - control;
				count = MIN(count, remaining);
				count = MIN<uint32>(count, sourceSize - sourcePosition);
				count = MIN<uint32>(count, _animFrameBuffer.size() - destinationPosition);
				memcpy(&_animFrameBuffer[destinationPosition], source + sourcePosition, count);
				sourcePosition += count;
			} else {
				if (sourcePosition >= sourceSize)
					return written;

				count = MIN(count, remaining);
				count = MIN<uint32>(count, _animFrameBuffer.size() - destinationPosition);
				memset(&_animFrameBuffer[destinationPosition], source[sourcePosition++], count);
			}

			destinationPosition += count;
			written += count;
			remaining -= count;
		}

		if (width < COMFY_SCREEN_WIDTH)
			destinationPosition += COMFY_SCREEN_WIDTH - width;
	}

	return written;
}

void ComfyEngine::animFileTickCommands() {
	if (!_usesAnimFile)
		return;

	if (!_animActive) {
		if (_stringTable.size() > 14)
			_stringTable[14] = 0;

		return;
	}

	if (_animFrameReady)
		return;

	_animVocTargetCounter++;
	if (_animVocTargetCounter == 1)
		animFrameSetVocCounter(0);

	if (_stringTable.size() > 14)
		_stringTable[14] = _animVocTargetCounter + 1;

	bool ended = false;
	bool frameReady = false;
	while (!ended && !frameReady) {
		uint32 headerSize = _animPantherFormat ? 8 : 4;
		if (_animPosition > _animFileData.size() || headerSize > _animFileData.size() - _animPosition)
			error("ANMFILE.DAT is truncated while reading an animation command");

		byte *header = &_animFileData[_animPosition];
		uint16 command = READ_LE_UINT16(header);
		uint32 rawCommandSize = _animPantherFormat ? READ_LE_UINT32(header + 2) : READ_LE_UINT16(header + 2);
		uint32 commandSize = _animPantherFormat ? rawCommandSize & 0xFFFF : rawCommandSize;
		if (commandSize < headerSize || commandSize > _animFileData.size() - _animPosition)
			error("Invalid animation command size in ANMFILE.DAT");

		uint32 payloadSize = commandSize - headerSize;
		byte *payload = header + headerSize;
		_animPosition += commandSize;
		if (command == kAnimCommandEnd) {
			ended = true;
		} else if (command == kAnimCommandStoreFrameBytes) {
			uint32 oldSize = _animFrameStorage.size();
			uint32 frameStart = _animIndexTable[_animCurrentIndex] + COMFY_ANMFILE_HEADER_BYTES;
			animFilePushStorageChunk((uint32)(payload - &_animFileData[frameStart]), oldSize, (uint16)payloadSize);
			_animFrameStorage.resize(oldSize + payloadSize);
			if (payloadSize)
				memcpy(&_animFrameStorage[oldSize], payload, payloadSize);
		} else if (command == kAnimCommandDirtyRect) {
			_animFrameCommandDataSize = payloadSize;
			_animFrameCommandFlag = _animPantherFormat && (rawCommandSize & 0x80000000) != 0;
			_animFrameCommandArgument = _animPantherFormat ? (rawCommandSize >> 16) & 0x7FF : payloadSize;
			memset(_animFrameCommandData, 0, sizeof(_animFrameCommandData));
			memcpy(_animFrameCommandData, payload, MIN<uint32>(payloadSize, sizeof(_animFrameCommandData)));
		} else if (command == kAnimCommandFrame) {
			if (commandSize > 8) {
				if (_animPantherFormat)
					animDecodePanther(payload, payloadSize);
				else
					animDecompressRle(payload, payloadSize, &_animFrameBuffer[0], _animFrameBuffer.size());
			}

			frameReady = true;
		} else {
			error("Unknown animation command 0x%04X in ANMFILE.DAT", command);
		}
	}

	if (ended)
		animFrameShutdown(false);
}

bool ComfyEngine::animFrameIsPending() {
	return _animActive && !_animFrameReady && _animVocTargetCounter != 0;
}

int16 ComfyEngine::animFrameVocCounterDelta() {
	if (!_midiPlyrDriver)
		return 0;

	uint32 counter = _midiPlyrDriver->getVocCounter();
	if (counter == 0)
		return 0;

	if (_animVocClockHz == 0)
		error("Invalid VOC clock frequency in ANMFILE.DAT");

	int32 expected = ((int32)(int16)_animVocTargetCounter * 100) / (int16)_animVocClockHz;
	return (int16)((uint16)counter - (uint16)expected);
}

bool ComfyEngine::animFrameShouldDraw(uint16 phase) {
	if (!animFrameIsPending())
		return true;

	int16 baseStep = 100 / (int16)_animVocClockHz;
	int16 threshold = phase == 1 ? baseStep / 2 : baseStep;
	int16 delta = animFrameVocCounterDelta();
	int16 adjustedDelta = (int16)((phase == 1 ? (int16)(delta + _animVocDeltaA) : delta) + _animVocDeltaB);
	bool shouldDraw = adjustedDelta <= threshold;
	if (!shouldDraw) {
		if ((int16)_animVocDeltaA > 0)
			_animVocDeltaA--;

		if ((int16)_animVocDeltaB > 0)
			_animVocDeltaB--;
	}

	return shouldDraw;
}

void ComfyEngine::animFrameRecordVocCounter(uint16 phase) {
	if (!animFrameIsPending() || !_midiPlyrDriver)
		return;

	uint32 counter = _midiPlyrDriver->getVocCounter();
	if (phase == 1) {
		_animVocCounterStartA = counter;
	} else if (phase == 2) {
		_animVocCounterEndA = counter;
		_animVocDeltaA = (uint16)(_animVocCounterEndA - _animVocCounterStartA);
	} else if (phase == 3) {
		_animVocCounterStartB = counter;
	} else if (phase == 4) {
		_animVocCounterEndB = counter;
		_animVocDeltaB = (uint16)(_animVocCounterEndB - _animVocCounterStartB);
	}
}

void ComfyEngine::animFrameWaitForVocCounter() {
	if (!animFrameIsPending())
		return;

	while (animFrameVocCounterDelta() < -(int16)(_animVocDeltaB + 3)) {
		processEvents();
		if (shouldQuit())
			break;
	}
}

void ComfyEngine::animFrameSetReady(bool ready) {
	if (!_animActive || ready == _animFrameReady)
		return;

	if (_midiPlyrDriver)
		_midiPlyrDriver->setIncreaseVocCounter(!ready);

	_animFrameReady = ready;
}

bool ComfyEngine::animFrameBlitAt(int16 x, int16 y) {
	if (_animFrameBuffer.empty() || !_framebufPtr)
		return false;

	uint16 rectCount = MIN<uint16>(_animFrameCommandDataSize / 4,
		sizeof(_animFrameCommandData) / 4);
	for (uint i = 0; i < rectCount; i++) {
		uint16 start = READ_LE_UINT16(_animFrameCommandData + i * 4);
		uint16 end = READ_LE_UINT16(_animFrameCommandData + i * 4 + 2);
		int16 left = x + (start % (COMFY_SCREEN_WIDTH / 4)) * 4;
		int16 top = y + (start / (COMFY_SCREEN_WIDTH / 4)) * 4;
		int16 right = x + ((end % (COMFY_SCREEN_WIDTH / 4)) + 1) * 4;
		int16 bottom = y + ((end / (COMFY_SCREEN_WIDTH / 4)) + 1) * 4;

		int16 clippedLeft = MAX<int16>(left, 0);
		int16 clippedTop = MAX<int16>(top, 0);
		int16 clippedRight = MIN<int16>(right, _logicalScreenWidth);
		int16 clippedBottom = MIN<int16>(bottom, _logicalScreenHeight);
		if (clippedLeft >= clippedRight || clippedTop >= clippedBottom)
			continue;

		for (int16 row = clippedTop; row < clippedBottom; row++) {
			uint32 sourceOffset = (uint32)(row - y) * COMFY_SCREEN_WIDTH + clippedLeft - x;
			uint32 destinationOffset = (uint32)row * _logicalScreenWidth + clippedLeft;
			uint32 width = clippedRight - clippedLeft;
			if (sourceOffset >= _animFrameBuffer.size())
				break;

			width = MIN<uint32>(width, _animFrameBuffer.size() - sourceOffset);
			memcpy(_framebufPtr + destinationOffset, &_animFrameBuffer[sourceOffset], width);
		}
	}

	renderSetDirty();
	return true;
}

void ComfyEngine::animFrameInvalidateRects(int16, int16, byte) {
	if (_animFrameBuffer.empty())
		return;

	// The original feeds these rectangles to its partial-update compositor.
	// ScummVM presents the engine framebuffer as one dirty surface.
	renderSetDirty();
}

bool ComfyEngine::animFrameGetDimensions(uint16 &width, uint16 &height) {
	if (!_animIndexLoaded || _animCurrentActorSceneHandle == 0)
		return false;

	if (_animPantherFormat) {
		width = READ_LE_UINT16(_animFrameHeader + 10);
		height = READ_LE_UINT16(_animFrameHeader + 12);
	} else {
		width = COMFY_SCREEN_WIDTH;
		height = COMFY_SCREEN_HEIGHT;
	}

	return true;
}

void ComfyEngine::animFrameClear() {
	if (!_animFrameBuffer.empty())
		memset(&_animFrameBuffer[0], 0xFF, COMFY_ANMFRAME_BYTES);
}

void ComfyEngine::animFilePackState(byte *state) {
	memset(state, 0, COMFY_ANM_STATE_BYTES);
	memcpy(state, _animFrameHeader, sizeof(_animFrameHeader));
	WRITE_LE_UINT16(state + 0x10, _animCurrentIndex);
	WRITE_LE_UINT16(state + 0x12, _animVocTargetCounter);
	state[0x14] = _animActive;
	state[0x15] = !_animFrameBuffer.empty();
	state[0x16] = _animFrameReady;
	WRITE_LE_UINT16(state + 0x17, _animCurrentFrameKey);
	state[0x19] = _animUsesWaveVocCounter;
	WRITE_LE_UINT16(state + 0x1A, _animCurrentActorSceneHandle);
	WRITE_LE_UINT16(state + 0x1C, _animVocDeltaA);
	WRITE_LE_UINT16(state + 0x1E, _animVocDeltaB);
	WRITE_LE_UINT32(state + 0x20, _midiPlyrDriver ? _midiPlyrDriver->getVocCounter() : 0);
	WRITE_LE_UINT32(state + 0x24, _animFrameStorage.size());

	uint32 streamPosition = 0;
	if (_animCurrentIndex < _animIndexTable.size()) {
		uint32 frameStart = _animIndexTable[_animCurrentIndex] + COMFY_ANMFILE_HEADER_BYTES;
		if (_animPosition >= frameStart)
			streamPosition = _animPosition - frameStart;
	}

	WRITE_LE_UINT32(state + 0x28, streamPosition);
	for (uint i = 0; i < 6; i++) {
		byte *chunk = state + 0x2C + i * 0x0A;
		WRITE_LE_UINT32(chunk, _animStorageChunkFileOffsets[i]);
		WRITE_LE_UINT32(chunk + 4, _animStorageChunkOffsets[i]);
		WRITE_LE_UINT16(chunk + 8, _animStorageChunkSizes[i]);
	}
}

void ComfyEngine::animFileRestoreStreamPosition(const byte *state) {
	if (_animCurrentIndex >= _animIndexTable.size())
		return;

	uint32 frameStart = _animIndexTable[_animCurrentIndex] + COMFY_ANMFILE_HEADER_BYTES;
	uint32 streamPosition = READ_LE_UINT32(state + 0x28);
	if (frameStart > _animFileData.size() || streamPosition > _animFileData.size() - frameStart)
		error("Invalid restored stream position in ANMFILE.DAT");

	_animPosition = frameStart + streamPosition;
}

void ComfyEngine::animFileRebuildStorage(uint32 targetSize) {
	_animFrameStorage.clear();
	memset(_animFrameCommandData, 0, sizeof(_animFrameCommandData));
	_animFrameCommandDataSize = 0;
	_animFrameCommandArgument = 0;
	_animFrameCommandFlag = false;
	memset(_animStorageChunkFileOffsets, 0, sizeof(_animStorageChunkFileOffsets));
	memset(_animStorageChunkOffsets, 0, sizeof(_animStorageChunkOffsets));
	memset(_animStorageChunkSizes, 0, sizeof(_animStorageChunkSizes));
	if (_animCurrentIndex >= _animIndexTable.size())
		return;

	uint32 position = _animIndexTable[_animCurrentIndex] + COMFY_ANMFILE_HEADER_BYTES;
	while (position < _animPosition && position < _animFileData.size()) {
		uint32 headerSize = _animPantherFormat ? 8 : 4;
		if (headerSize > _animFileData.size() - position)
			error("ANMFILE.DAT is truncated while rebuilding animation storage");

		byte *header = &_animFileData[position];
		uint16 command = READ_LE_UINT16(header);
		uint32 rawCommandSize = _animPantherFormat ? READ_LE_UINT32(header + 2) : READ_LE_UINT16(header + 2);
		uint32 commandSize = _animPantherFormat ? rawCommandSize & 0xFFFF : rawCommandSize;
		if (commandSize < headerSize || commandSize > _animFileData.size() - position)
			error("Invalid command while rebuilding animation storage");

		uint32 payloadSize = commandSize - headerSize;
		byte *payload = header + headerSize;
		if (command == kAnimCommandStoreFrameBytes && _animFrameStorage.size() < targetSize) {
			uint32 copySize = MIN<uint32>(payloadSize, targetSize - _animFrameStorage.size());
			uint32 oldSize = _animFrameStorage.size();
			uint32 frameStart = _animIndexTable[_animCurrentIndex] + COMFY_ANMFILE_HEADER_BYTES;
			animFilePushStorageChunk((uint32)(payload - &_animFileData[frameStart]), oldSize, (uint16)copySize);
			_animFrameStorage.resize(oldSize + copySize);
			if (copySize)
				memcpy(&_animFrameStorage[oldSize], payload, copySize);
		} else if (command == kAnimCommandDirtyRect) {
			_animFrameCommandDataSize = payloadSize;
			_animFrameCommandFlag = _animPantherFormat && (rawCommandSize & 0x80000000) != 0;
			_animFrameCommandArgument = _animPantherFormat ? (rawCommandSize >> 16) & 0x7FF : payloadSize;
			memset(_animFrameCommandData, 0, sizeof(_animFrameCommandData));
			memcpy(_animFrameCommandData, payload, MIN<uint32>(payloadSize, sizeof(_animFrameCommandData)));
		}

		position += commandSize;
	}
}

void ComfyEngine::animFileUnpackState(const byte *state) {
	animFrameShutdown(true);
	memcpy(_animFrameHeader, state, sizeof(_animFrameHeader));
	_animCurrentIndex = READ_LE_UINT16(state + 0x10);
	_animVocTargetCounter = READ_LE_UINT16(state + 0x12);
	_animActive = state[0x14] != 0;
	_animFrameReady = state[0x16] != 0;
	_animCurrentFrameKey = READ_LE_UINT16(state + 0x17);
	_animUsesWaveVocCounter = state[0x19] != 0;
	_animCurrentActorSceneHandle = READ_LE_UINT16(state + 0x1A);
	_animVocDeltaA = READ_LE_UINT16(state + 0x1C);
	_animVocDeltaB = READ_LE_UINT16(state + 0x1E);
	for (uint i = 0; i < 6; i++) {
		_animStorageChunkFileOffsets[i] = READ_LE_UINT32(state + 0x2C + i * 0x0A);
		_animStorageChunkOffsets[i] = READ_LE_UINT32(state + 0x30 + i * 0x0A);
		_animStorageChunkSizes[i] = READ_LE_UINT16(state + 0x34 + i * 0x0A);
	}

	if (_animCurrentActorSceneHandle == 0)
		return;

	if (_animCurrentActorSceneHandle < _sceneHandles.size()) {
		Actor *actor = actorGetPtr(sceneGetHandle(_animCurrentActorSceneHandle));
		if (actor)
			actorWriteU32(*actor, kActorSpriteSelector, 0x00FFFFFF);
	}

	_animFrameBuffer.resize(COMFY_ANMFRAME_BYTES);
	animFrameClear();
	if (!_animActive)
		return;

	animFileRestoreStreamPosition(state);
	animFileRebuildStorage(READ_LE_UINT32(state + 0x24));
	animFrameSetVocCounter(READ_LE_UINT32(state + 0x20));
}

void ComfyEngine::animFrameInvalidateActorRect() {
	if (_animCurrentActorSceneHandle >= _sceneHandles.size())
		return;

	Actor *actor = actorGetPtr(sceneGetHandle(_animCurrentActorSceneHandle));
	if (!actor)
		return;

	actorWriteU16(*actor, kActorCachedRect, 0x270F);
	actorWriteU16(*actor, kActorCachedRect + 4, 0x270F);
}

} // End of namespace Comfy
