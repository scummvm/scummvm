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

bool ComfyEngine::animFileOpen() {
	animFileShutdown(true);
	if (!readAssetFile(Common::Path("ANMFILE.DAT"), _animFileData) ||
			_animFileData.size() < COMFY_ANMFILE_HEADER_BYTES)
		return false;

	uint16 magic = READ_LE_UINT16(&_animFileData[0]);
	if (magic != COMFY_ANMFILE_MAGIC && magic != COMFY_ANMFILE_PANTHER_MAGIC)
		return false;

	uint16 count = READ_LE_UINT16(&_animFileData[2]);
	if (uint32(COMFY_ANMFILE_HEADER_BYTES) + uint32(count) * 4 > _animFileData.size())
		return false;

	_animIndexTable.resize(count);
	for (uint i = 0; i < count; i++)
		_animIndexTable[i] = READ_LE_UINT32(&_animFileData[COMFY_ANMFILE_HEADER_BYTES + i * 4]);

	_animPantherFormat = magic == COMFY_ANMFILE_PANTHER_MAGIC;
	_animIndexLoaded = true;
	return true;
}

void ComfyEngine::animFileShutdown(bool closeFile) {
	if (_animActive) {
		if (_animCurrentFrameKey)
			keyBitSet(_animCurrentFrameKey);

		if (_animCurrentActorSceneHandle < _sceneHandles.size()) {
			Actor *actor = actorGetPtr(sceneGetHandle(_animCurrentActorSceneHandle));
			if (actor)
				actorWriteU32(*actor, kActorSpriteSelector, 0);
		}
	}

	_animActive = false;
	_animCurrentIndex = 0;
	_animCurrentActorSceneHandle = 0;
	_animCurrentFrameKey = 0;
	_animVocClockHz = 0;
	_animVocTargetCounter = 0;
	if (closeFile) {
		_animFileData.clear();
		_animIndexTable.clear();
		_animFrameBuffer.clear();
		_animFrameStorage.clear();
		_animPosition = 0;
		_animIndexLoaded = false;
		_animPantherFormat = false;
	}
}

void ComfyEngine::animFileLoadFrame(uint16 animIndex, uint16 frameKey, uint16 actorSceneHandle) {
	if (!_animIndexLoaded || _animActive || animIndex >= _animIndexTable.size()) {
		_animActive = false;
		return;
	}

	uint32 offset = _animIndexTable[animIndex];
	if (offset > _animFileData.size() || COMFY_ANMFILE_HEADER_BYTES > _animFileData.size() - offset)
		return;

	memcpy(_animFrameHeader, &_animFileData[offset], sizeof(_animFrameHeader));
	uint16 expectedMagic = _animPantherFormat ? COMFY_ANMFRAME_PANTHER_MAGIC : COMFY_ANMFRAME_MAGIC;
	if (READ_LE_UINT16(_animFrameHeader) != expectedMagic)
		return;

	_animFrameBuffer.resize(_animPantherFormat ? framebufferBytes() : COMFY_ANMFRAME_BYTES);
	memset(&_animFrameBuffer[0], 0xFF, _animFrameBuffer.size());
	_animFrameStorage.clear();
	memset(_animPendingDirtyRect, 0, sizeof(_animPendingDirtyRect));
	_animPendingDirtyRectSize = 0;
	_animPosition = offset + COMFY_ANMFILE_HEADER_BYTES;
	_animCurrentIndex = animIndex;
	_animCurrentActorSceneHandle = actorSceneHandle;
	_animCurrentFrameKey = frameKey;
	_animVocCounterMode = 0;
	_animVocClockHz = READ_LE_UINT16(_animFrameHeader + 8);
	_animVocTargetCounter = 0xFFFF;
	_animVocBaseCounter = 0;
	_animVocDeltaA = 0;
	_animActive = true;
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
		if (!control) {
			do {
				if (sourcePosition + 1 >= sourceSize || destinationPosition >= destinationSize)
					return destinationPosition;

				sourcePosition++;
				destination[destinationPosition++] = source[sourcePosition++];
				if (sourcePosition >= sourceSize)
					return destinationPosition;

				control = source[sourcePosition++];
			} while (!control);
		}

		for (;;) {
			if (control == 0x80) {
				if (sourcePosition + 1 >= sourceSize)
					return destinationPosition;

				uint16 word = READ_LE_UINT16(source + sourcePosition);
				sourcePosition += 2;
				if (!word)
					return destinationPosition;

				if (int16(word) > 0) {
					destinationPosition = MIN<uint32>(destinationPosition + word, destinationSize);
					break;
				}

				uint16 count = word - 0x8000;
				if ((count & 0x4000) == 0) {
					count = MIN<uint32>(count, sourceSize - sourcePosition);
					count = MIN<uint32>(count, destinationSize - destinationPosition);
					memcpy(destination + destinationPosition, source + sourcePosition, count);
					sourcePosition += count;
					destinationPosition += count;
					break;
				}

				count -= 0x4000;
				if (sourcePosition >= sourceSize)
					return destinationPosition;

				count = MIN<uint32>(count, destinationSize - destinationPosition);
				memset(destination + destinationPosition, source[sourcePosition++], count);
				destinationPosition += count;
				break;
			}

			if (control & 0x80) {
				destinationPosition = MIN<uint32>(destinationPosition + control - 0x80, destinationSize);
				break;
			}

			if (sourcePosition >= sourceSize || destinationPosition >= destinationSize)
				return destinationPosition;

			destination[destinationPosition++] = source[sourcePosition++];
			if (sourcePosition >= sourceSize)
				return destinationPosition;

			control = source[sourcePosition++];
			if (!control)
				break;
		}
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
				destinationPosition += uint32(uint16(0 - runCount)) * _logicalScreenWidth;
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

			destinationPosition = rowStart + _logicalScreenWidth;
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

		if (width < _logicalScreenWidth)
			destinationPosition += _logicalScreenWidth - width;
	}

	return written;
}

void ComfyEngine::animFileTickCommands() {
	if (!_usesAnimFile || !_animActive || _animVocCounterMode)
		return;

	_animVocTargetCounter++;
	if (_animVocTargetCounter == 1 && _midiPlyrDriver) {
		_midiPlyrDriver->setVocCounter(0);
		_midiPlyrDriver->setIncreaseVocCounter(1);
	}

	bool ended = false;
	bool frameReady = false;
	while (!ended && !frameReady) {
		uint32 headerSize = _animPantherFormat ? 8 : 4;
		if (_animPosition > _animFileData.size() || headerSize > _animFileData.size() - _animPosition) {
			animFileShutdown(false);
			return;
		}

		byte *header = &_animFileData[_animPosition];
		uint16 command = READ_LE_UINT16(header);
		uint32 commandSize = _animPantherFormat ? READ_LE_UINT32(header + 2) : READ_LE_UINT16(header + 2);
		if (commandSize < headerSize || commandSize > _animFileData.size() - _animPosition) {
			animFileShutdown(false);
			return;
		}

		uint32 payloadSize = commandSize - headerSize;
		byte *payload = header + headerSize;
		_animPosition += commandSize;
		if (command == kAnimCommandEnd) {
			ended = true;
		} else if (command == kAnimCommandStoreFrameBytes) {
			uint32 oldSize = _animFrameStorage.size();
			_animFrameStorage.resize(oldSize + payloadSize);
			if (payloadSize)
				memcpy(&_animFrameStorage[oldSize], payload, payloadSize);
		} else if (command == kAnimCommandDirtyRect) {
			_animPendingDirtyRectSize = payloadSize;
			memset(_animPendingDirtyRect, 0, sizeof(_animPendingDirtyRect));
			memcpy(_animPendingDirtyRect, payload, MIN<uint32>(payloadSize, sizeof(_animPendingDirtyRect)));
		} else if (command == kAnimCommandFrame) {
			if (payloadSize) {
				if (_animPantherFormat)
					animDecodePanther(payload, payloadSize);
				else
					animDecompressRle(payload, payloadSize, &_animFrameBuffer[0], _animFrameBuffer.size());
			}

			if (_framebufPtr && !_animFrameBuffer.empty()) {
				memcpy(_framebufPtr, &_animFrameBuffer[0], MIN<uint32>(framebufferBytes(), _animFrameBuffer.size()));
				renderSetDirty();
			}

			frameReady = true;
		} else {
			animFileShutdown(false);
			return;
		}
	}

	if (ended)
		animFileShutdown(false);
}

bool ComfyEngine::animFrameShouldWaitForVoc() {
	return _animActive && !_animVocCounterMode && _animVocTargetCounter && _animVocClockHz && _midiPlyrDriver;
}

int16 ComfyEngine::animFrameVocCounterDelta() {
	if (!animFrameShouldWaitForVoc())
		return 0;

	int32 expected = (int32(int16(_animVocTargetCounter)) * 100) / int16(_animVocClockHz);
	return int16(uint16(_midiPlyrDriver->getVocCounter()) - uint16(expected));
}

bool ComfyEngine::animFrameShouldDraw(uint16 phase) {
	if (!animFrameShouldWaitForVoc())
		return true;

	int16 baseStep = 100 / int16(_animVocClockHz);
	int16 threshold = phase == 1 ? baseStep / 2 : baseStep;
	int16 delta = animFrameVocCounterDelta();
	int16 adjustedDelta = int16((phase == 1 ? int16(delta + _animVocDeltaA) : delta) + _animVocBaseCounter);
	return adjustedDelta <= threshold;
}

void ComfyEngine::animFrameRecordVocCounter(uint16 phase) {
	if (!animFrameShouldWaitForVoc())
		return;

	uint32 counter = _midiPlyrDriver->getVocCounter();
	if (phase == 1) {
		_animVocCounterStartA = counter;
	} else if (phase == 2) {
		_animVocDeltaA = uint16(counter - _animVocCounterStartA);
	} else if (phase == 3) {
		_animVocCounterStartB = counter;
	} else if (phase == 4) {
		_animVocBaseCounter = uint16(counter - _animVocCounterStartB);
	}
}

void ComfyEngine::animFrameWaitForVocCounter() {
	if (!animFrameShouldWaitForVoc())
		return;

	for (uint guard = 0; guard < 1000; guard++) {
		if (animFrameVocCounterDelta() >= -int16(_animVocBaseCounter))
			break;

		soundAdvanceTick();
		processEvents();
		if (shouldQuit())
			break;

		_system->delayMillis(1);
	}
}

} // End of namespace Comfy
