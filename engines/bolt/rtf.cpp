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

#include "common/memstream.h"

#include "bolt/bolt.h"

namespace Bolt {

RTFResource *BoltEngine::openRTF(const char *fileName) {
	uint32 headerTag = 0;
	uint32 dataSize = 0;
	uint32 bytesRead = 0;
	RTFResource *rtf = nullptr;

	Common::File *fileHandle = _xp->openFile(fileName, 1);
	if (!fileHandle)
		return nullptr;

	// Read 8-byte header: tag + size
	bytesRead = 4;
	if (_xp->readFile(fileHandle, &headerTag, &bytesRead) && bytesRead == 4) {
		bytesRead = 4;
		if (_xp->readFile(fileHandle, &dataSize, &bytesRead) && bytesRead == 4) {
			headerTag = FROM_BE_32(headerTag);
			dataSize = FROM_BE_32(dataSize);

			if (headerTag == MKTAG('M', 'A', 'P', ' ')) {
				if ((dataSize & 3) == 0) {
					// Allocate: 10-byte header + index data
					rtf = new RTFResource();
					rtf->indexTableRawData = (byte *)_xp->allocMem(dataSize);
					if (rtf && rtf->indexTableRawData) {
						// RTF layout:
						//   +0x00: file handle (uint32)
						//   +0x04: pointer to index table (far ptr)
						//   +0x08: entry count (word)
						//   +0x0A: index table data begins here
						rtf->fileHandle = fileHandle;
						rtf->indexTablePtr = rtf->indexTable;
						rtf->entryCount = (int16)(dataSize / sizeof(uint32));
						rtf->reserveTableSize(rtf->entryCount);

						// Read offset table of available animations...
						bytesRead = dataSize;
						if (_xp->readFile(fileHandle, rtf->indexTableRawData, &bytesRead)) {
							Common::SeekableReadStream *indexDataStream = new Common::MemoryReadStream((byte *)rtf->indexTableRawData, bytesRead, DisposeAfterUse::NO);

							for (int16 i = 0; i < rtf->entryCount; i++) {
								rtf->indexTable[i] = indexDataStream->readUint32BE();
							}

							delete indexDataStream;

							return rtf;
						}
					}
				}
			}
		}
	}

	if (rtf && rtf->indexTableRawData)
		_xp->freeMem(rtf->indexTableRawData);

	if (rtf)
		delete rtf;

	if (fileHandle)
		_xp->closeFile(fileHandle);

	return nullptr;
}

void BoltEngine::closeRTF(RTFResource *rtf) {
	_xp->closeFile(rtf->fileHandle);
	_xp->freeMem(rtf->indexTableRawData);

	delete rtf;
	rtf = nullptr;
}

bool BoltEngine::playRTF(RTFResource *rtfFile, int16 animIndex, byte *ringBuffer, int32 bufferSize) {
	_rtfFileHandle = rtfFile->fileHandle;

	if (animIndex >= rtfFile->entryCount)
		return false;

	uint32 *indexTable = rtfFile->indexTable;
	if (!_xp->setFilePos(_rtfFileHandle, indexTable[animIndex], 0))
		return false;

	_rtfChunkRemaining = 0;
	_rtfMidChunk = false;

	_ringBufBase = ringBuffer;
	_ringBufWritePtr = ringBuffer;
	_ringBufSize = bufferSize;
	_ringBufFreeSpace = bufferSize;

	if (bufferSize >= 0x96000) {
		_ringBufLowWater = bufferSize - 0x4B000;
		_ringBufHighWater = bufferSize - 0x19000;
	} else {
		_ringBufLowWater = bufferSize - bufferSize / 2;
		_ringBufHighWater = bufferSize - bufferSize / 4;
	}

	_ringBufUsed = 0;
	_rtfSoundActive = true;
	_rtfPlaybackTime = 0;
	_rtfCumulativeTime = 0;
	_rtfPendingFrame = nullptr;
	_rtfSoundQueueHead = nullptr;
	_rtfSoundPlayHead = nullptr;
	_rtfChunkListTail = nullptr;
	_rtfChunkListHead = nullptr;
	_rtfChunkCount = 0;
	_rtfQueuedSoundCount = 0;
	_rtfSoundTiming = 0;

	_rtfAnimStartOffset = indexTable[animIndex];
	_rtfNeedInitialFill = true;

	return true;
}

bool BoltEngine::fillRTFBuffer() {
	if (!_rtfNeedInitialFill)
		return false;

	if (_ringBufFreeSpace > _ringBufLowWater && _rtfSoundActive) {
		_rtfSoundActive = readPacket();
		return true;
	}

	_rtfNeedInitialFill = false;
	return false;
}

void BoltEngine::flushRTFSoundQueue() {
	while (_rtfQueuedSoundCount < 50 && _rtfSoundQueueHead) {
		RTFPacket *packet = _rtfSoundQueueHead;

		if (packet->tag == MKTAG('A', '2', '2', '8')) {
			_rtfQueuedSoundCount++;
			_xp->playSound(packet->dataPtr, packet->dataSize, 22050);

			if (!_rtfSoundPlayHead)
				_rtfSoundPlayHead = _rtfSoundQueueHead;
		}

		_rtfSoundQueueHead = packet->next;
	}
}

bool BoltEngine::maintainRTF(int16 mode, RTFPacket **outFrameData) {
	if (!_rtfFileHandle)
		return false;

	if (!_ringBufBase)
		return false;

	if (outFrameData)
		*outFrameData = nullptr;

	// Initial buffer fill phase...
	if (_rtfNeedInitialFill) {
		if (_ringBufFreeSpace > _ringBufLowWater && _rtfSoundActive) {
			_rtfSoundActive = readPacket();
			return true;
		}

		_rtfNeedInitialFill = false;
	}

	// Advance sound play head when a sound chunk finishes (mode != 0)
	if (mode != 0) {
		_rtfQueuedSoundCount--;
		_rtfSoundTiming -= _rtfSoundPlayHead->duration;
		_rtfPlaybackTime += _rtfSoundPlayHead->duration;

		// Walk to next A228 sound chunk
		do {
			_rtfSoundPlayHead = _rtfSoundPlayHead->next;
			if (!_rtfSoundPlayHead)
				break;
		} while (_rtfSoundPlayHead->tag != MKTAG('A', '2', '2', '8'));
	}

	// Queue up to 50 pending sound chunks...
	while (_rtfQueuedSoundCount < 50 && _rtfSoundQueueHead) {
		if (_rtfSoundQueueHead->tag == MKTAG('A', '2', '2', '8')) {
			_rtfQueuedSoundCount++;
			_xp->playSound(_rtfSoundQueueHead->dataPtr, _rtfSoundQueueHead->dataSize, 22050);

			if (!_rtfSoundPlayHead)
				_rtfSoundPlayHead = _rtfSoundQueueHead;
		}

		_rtfSoundQueueHead = _rtfSoundQueueHead->next;
	}

	if (_rtfPendingFrame) {
		freePacket(_rtfPendingFrame);
		_rtfPendingFrame = nullptr;
	}

	// Playback complete?
	if (!_rtfSoundActive && _rtfChunkCount == 0) {
		resetPlaybackState();
		return false;
	}

	// Fill ring buffer if needed...
	if (_rtfSoundActive && _ringBufFreeSpace > _ringBufLowWater) {
		_rtfSoundActive = readPacket();
		if (_ringBufFreeSpace > _ringBufHighWater)
			return true;
	}

	// Check if next video frame is due...
	if (_rtfChunkCount != 0 && (uint16)_rtfChunkListHead->timestamp <= (uint16)_rtfPlaybackTime) {
		_rtfPendingFrame = deQueuePacket();

		// If it's a sound chunk, just return...
		if (_rtfPendingFrame->tag == MKTAG('A', '2', '2', '8'))
			return true;

		// Calculate frame timing: how late are we?
		_rtfPendingFrame->skipCount = _rtfPlaybackTime - _rtfPendingFrame->timestamp;

		// Bump skip count if buffer is low...
		if (_rtfSoundActive && _ringBufFreeSpace > _ringBufLowWater)
			_rtfPendingFrame->skipCount += 1;

		if (!_rtfSoundActive)
			_rtfPendingFrame->frameRate = 50; // fixed rate, no sound
		else
			_rtfPendingFrame->frameRate = _rtfSoundTiming;

		// Find next sound chunk in queue for timing info...
		RTFPacket *chunk = _rtfChunkListHead;
		while (chunk) {
			if (chunk->tag == MKTAG('A', '2', '2', '8'))
				break;

			chunk = chunk->next;
		}

		if (chunk)
			_rtfPendingFrame->duration = chunk->duration;
		else
			_rtfPendingFrame->duration = 0;

		if (outFrameData)
			*outFrameData = _rtfPendingFrame;

		return true;
	}

	if (_rtfSoundActive)
		_rtfSoundActive = readPacket();

	return true;
}

bool BoltEngine::isRTFPlaying() {
	if (_rtfSoundActive)
		return true;

	if (_rtfSoundQueueHead)
		return true;

	if (_rtfQueuedSoundCount > 10)
		return true;

	return false;
}

bool BoltEngine::killRTF(uint32 *outFilePos) {
	if (!_rtfFileHandle)
		return false;

	if (!_ringBufBase)
		return false;

	_xp->stopSound();
	
	if (outFilePos)
		*outFilePos = _rtfAnimStartOffset;

	return true;
}

bool BoltEngine::readPacket() {
	uint32 readSize;
	uint32 bytesRead;
	byte padByte;
	byte *destPtr;

	if (!_rtfMidChunk) {
		// Read 8-byte chunk header...
		bytesRead = 4;
		if (!_xp->readFile(_rtfFileHandle, &_rtfChunkTag, &bytesRead) || bytesRead != 4)
			return false;

		bytesRead = 4;
		if (!_xp->readFile(_rtfFileHandle, &_rtfChunkSize, &bytesRead) || bytesRead != 4)
			return false;

		_rtfChunkTag = FROM_BE_32(_rtfChunkTag);
		_rtfChunkSize = FROM_BE_32(_rtfChunkSize);

		// End-of-file marker: 'EOR '
		if (_rtfChunkTag == MKTAG('E', 'O', 'R',' '))
			return false;

		_rtfChunkRemaining = _rtfChunkSize;
		_rtfMidChunk = true;
		_rtfCurrentPacket = nullptr;
	}

	if (!_rtfCurrentPacket) {
		_rtfCurrentPacket = allocPacket(_rtfChunkSize);
		if (!_rtfCurrentPacket)
			return true;

		_rtfCurrentPacket->tag = _rtfChunkTag;
	}

	// Read up to 4096 bytes at a time...
	readSize = (_rtfChunkRemaining > 0x1000) ? 0x1000 : _rtfChunkRemaining;

	if (readSize != 0) {
		destPtr = &_rtfCurrentPacket->dataPtr[_rtfChunkSize - _rtfChunkRemaining];

		bytesRead = readSize;
		if (!_xp->readFile(_rtfFileHandle, destPtr, &bytesRead))
			return false;

		_rtfChunkRemaining -= bytesRead;
	}

	if (_rtfChunkRemaining != 0)
		return true; // More data to read...

	// Chunk fully read!
	_rtfMidChunk = false;

	// Skip padding byte for odd-sized chunks...
	if (_rtfChunkSize & 1) {
		bytesRead = 1;
		if (!_xp->readFile(_rtfFileHandle, &padByte, &bytesRead))
			return false;
	}

	// Finally update file position tracker...
	_rtfAnimStartOffset += _rtfChunkSize + (_rtfChunkSize & 1) + 8;

	preProcessPacket(_rtfCurrentPacket);

	return true;
}

void BoltEngine::preProcessPacket(RTFPacket *packet) {
	if (packet->tag == MKTAG('A', '2', '2', '8')) {
		// Sound chunk: attempt to play it immediately!
		if (_rtfQueuedSoundCount < 50 && !_rtfNeedInitialFill) {
			_rtfQueuedSoundCount++;
			_xp->playSound(packet->dataPtr, packet->dataSize, 22050);

			if (!_rtfSoundPlayHead)
				_rtfSoundPlayHead = packet;
		} else {
			// Can't play yet, mark queue head...
			if (!_rtfSoundQueueHead)
				_rtfSoundQueueHead = packet;
		}

		// Duration in hundredths of a second: dataSize / 2205 (22050 Hz / 10)
		packet->duration = packet->dataSize / 2205;
		if (packet->duration == 0)
			packet->duration = 1;

		_rtfCumulativeTime += packet->duration;
		_rtfSoundTiming += packet->duration;
	}

	// Stamp packet with current timeline position...
	packet->timestamp = _rtfCumulativeTime;

	queuePacket(packet);
}

void BoltEngine::queuePacket(RTFPacket *packet) {
	debug(5, "BoltEngine::queuePacket(): Queueing packet of type: %c%c%c%c",
		  (packet->tag >> 24) & 0xFF,
		  (packet->tag >> 16) & 0xFF,
		  (packet->tag >> 8) & 0xFF,
		  packet->tag & 0xFF);

	if (_rtfChunkListTail) {
		_rtfChunkListTail->next = packet;
	} else {
		_rtfChunkListHead = packet;
	}

	_rtfChunkListTail = packet;
	packet->next = nullptr;
	_rtfChunkCount++;
}

RTFPacket *BoltEngine::deQueuePacket() {
	if (_rtfChunkListHead)
		debug(5, "BoltEngine::deQueuePacket(): Dequeuing packet of type: %c%c%c%c",
			  (_rtfChunkListHead->tag >> 24) & 0xFF,
			  (_rtfChunkListHead->tag >> 16) & 0xFF,
			  (_rtfChunkListHead->tag >> 8) & 0xFF,
			  _rtfChunkListHead->tag & 0xFF);

	RTFPacket *packet = _rtfChunkListHead;

	if (_rtfChunkListHead) {
		_rtfChunkListHead = packet->next;

		if (!_rtfChunkListHead)
			_rtfChunkListTail = nullptr;

		packet->next = nullptr;
	}

	_rtfChunkCount--;
	return packet;
}

RTFPacket *BoltEngine::allocPacket(uint32 dataSize) {
	uint32 totalSize;
	uint32 available;
	uint32 freeAfterWrap;
	byte *allocPtr = nullptr;

	// Header (0x1C) + data, 4-byte aligned
	totalSize = dataSize + 0x1C;
	if (totalSize & 3)
		totalSize = (totalSize + 4) & ~3;

	// How much contiguous space from write pointer to end of buffer?
	available = (uint32)(_ringBufBase - _ringBufWritePtr) + _ringBufSize;

	if (available >= _ringBufFreeSpace) {
		available = _ringBufFreeSpace;
		freeAfterWrap = 0;
	} else {
		freeAfterWrap = _ringBufFreeSpace - available;
	}

	if (available >= totalSize) {
		allocPtr = _ringBufWritePtr;
	} else {
		// Not enough space before the end: try wrapping to buffer start...
		if (freeAfterWrap < totalSize)
			return nullptr;

		allocPtr = _ringBufBase;
		_ringBufWritePtr = _ringBufBase;
		_ringBufUsed = available;
		_ringBufFreeSpace -= available;
	}

	_ringBufWritePtr += totalSize;
	_ringBufFreeSpace -= totalSize;

	// Init packet header and return...
	RTFPacket *outPacket = new RTFPacket();
	outPacket->dataPtr = &allocPtr[0x1C];
	outPacket->dataSize = dataSize;
	outPacket->allocSize = totalSize;
	outPacket->ringBufPtr = allocPtr;

	return outPacket;
}

void BoltEngine::freePacket(RTFPacket *packet) {
	// Return allocated size to free space...
	_ringBufFreeSpace += packet->allocSize;

	// Calculate distance from packet to buffer end...
	uint32 distToEnd = _ringBufSize - (packet->ringBufPtr - _ringBufBase) - (packet->allocSize);

	// If this was the last packet before the wrap point, reclaim the gap!
	if (distToEnd == _ringBufUsed) {
		_ringBufFreeSpace += _ringBufUsed;
		_ringBufUsed = 0;
	}

	// If buffer is completely empty, reset write pointer!
	if (_ringBufFreeSpace == _ringBufSize)
		_ringBufWritePtr = _ringBufBase;

	delete packet;
}

void BoltEngine::resetPlaybackState() {
	_rtfFileHandle = nullptr;
	_ringBufBase = nullptr;
	_rtfSoundActive = false;
}

} // End of namespace Bolt
