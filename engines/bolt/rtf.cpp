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

#include "bolt/bolt.h"

#include "common/memstream.h"

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
	g_rtfFileHandle = rtfFile->fileHandle;

	if (animIndex >= rtfFile->entryCount)
		return false;

	uint32 *indexTable = rtfFile->indexTable;
	if (!_xp->setFilePos(g_rtfFileHandle, indexTable[animIndex], 0))
		return false;

	g_rtfChunkRemaining = 0;
	g_rtfMidChunk = false;

	g_ringBufBase = ringBuffer;
	g_ringBufWritePtr = ringBuffer;
	g_ringBufSize = bufferSize;
	g_ringBufFreeSpace = bufferSize;

	if (bufferSize >= 0x96000) {
		g_ringBufLowWater = bufferSize - 0x4B000;
		g_ringBufHighWater = bufferSize - 0x19000;
	} else {
		g_ringBufLowWater = bufferSize - bufferSize / 2;
		g_ringBufHighWater = bufferSize - bufferSize / 4;
	}

	g_ringBufUsed = 0;
	g_rtfSoundActive = true;
	g_rtfPlaybackTime = 0;
	g_rtfCumulativeTime = 0;
	g_rtfPendingFrame = nullptr;
	g_rtfSoundQueueHead = nullptr;
	g_rtfSoundPlayHead = nullptr;
	g_rtfChunkListTail = nullptr;
	g_rtfChunkListHead = nullptr;
	g_rtfChunkCount = 0;
	g_rtfQueuedSoundCount = 0;
	g_rtfSoundTiming = 0;

	g_rtfAnimStartOffset = indexTable[animIndex];
	g_rtfNeedInitialFill = true;

	return true;
}

bool BoltEngine::fillRTFBuffer() {
	if (!g_rtfNeedInitialFill)
		return false;

	if (g_ringBufFreeSpace > g_ringBufLowWater && g_rtfSoundActive) {
		g_rtfSoundActive = readPacket();
		return true;
	}

	g_rtfNeedInitialFill = false;
	return false;
}

void BoltEngine::flushRTFSoundQueue() {
	while (g_rtfQueuedSoundCount < 50 && g_rtfSoundQueueHead) {
		RTFPacket *packet = g_rtfSoundQueueHead;

		if (packet->tag == MKTAG('A', '2', '2', '8')) {
			g_rtfQueuedSoundCount++;
			_xp->playSound(packet->dataPtr, packet->dataSize, 22050);

			if (!g_rtfSoundPlayHead)
				g_rtfSoundPlayHead = g_rtfSoundQueueHead;
		}

		g_rtfSoundQueueHead = packet->next;
	}
}

bool BoltEngine::maintainRTF(int16 mode, RTFPacket **outFrameData) {
	if (!g_rtfFileHandle)
		return false;

	if (!g_ringBufBase)
		return false;

	if (outFrameData)
		*outFrameData = nullptr;

	// Initial buffer fill phase...
	if (g_rtfNeedInitialFill) {
		if (g_ringBufFreeSpace > g_ringBufLowWater && g_rtfSoundActive) {
			g_rtfSoundActive = readPacket();
			return true;
		}

		g_rtfNeedInitialFill = false;
	}

	// Advance sound play head when a sound chunk finishes (mode != 0)
	if (mode != 0) {
		g_rtfQueuedSoundCount--;
		g_rtfSoundTiming -= g_rtfSoundPlayHead->duration;
		g_rtfPlaybackTime += g_rtfSoundPlayHead->duration;

		// Walk to next A228 sound chunk
		do {
			g_rtfSoundPlayHead = g_rtfSoundPlayHead->next;
			if (!g_rtfSoundPlayHead)
				break;
		} while (g_rtfSoundPlayHead->tag != MKTAG('A', '2', '2', '8'));
	}

	// Queue up to 50 pending sound chunks...
	while (g_rtfQueuedSoundCount < 50 && g_rtfSoundQueueHead) {
		if (g_rtfSoundQueueHead->tag == MKTAG('A', '2', '2', '8')) {
			g_rtfQueuedSoundCount++;
			_xp->playSound(g_rtfSoundQueueHead->dataPtr, g_rtfSoundQueueHead->dataSize, 22050);

			if (!g_rtfSoundPlayHead)
				g_rtfSoundPlayHead = g_rtfSoundQueueHead;
		}

		g_rtfSoundQueueHead = g_rtfSoundQueueHead->next;
	}

	if (g_rtfPendingFrame) {
		freePacket(g_rtfPendingFrame);
		g_rtfPendingFrame = nullptr;
	}

	// Playback complete?
	if (!g_rtfSoundActive && g_rtfChunkCount == 0) {
		resetPlaybackState();
		return false;
	}

	// Fill ring buffer if needed...
	if (g_rtfSoundActive && g_ringBufFreeSpace > g_ringBufLowWater) {
		g_rtfSoundActive = readPacket();
		if (g_ringBufFreeSpace > g_ringBufHighWater)
			return true;
	}

	// Check if next video frame is due...
	if (g_rtfChunkCount != 0 && (uint16)g_rtfChunkListHead->timestamp <= (uint16)g_rtfPlaybackTime) {
		g_rtfPendingFrame = deQueuePacket();

		// If it's a sound chunk, just return...
		if (g_rtfPendingFrame->tag == MKTAG('A', '2', '2', '8'))
			return true;

		// Calculate frame timing: how late are we?
		g_rtfPendingFrame->skipCount = g_rtfPlaybackTime - g_rtfPendingFrame->timestamp;

		// Bump skip count if buffer is low...
		if (g_rtfSoundActive && g_ringBufFreeSpace > g_ringBufLowWater)
			g_rtfPendingFrame->skipCount += 1;

		if (!g_rtfSoundActive)
			g_rtfPendingFrame->frameRate = 50; // fixed rate, no sound
		else
			g_rtfPendingFrame->frameRate = g_rtfSoundTiming;

		// Find next sound chunk in queue for timing info...
		RTFPacket *chunk = g_rtfChunkListHead;
		while (chunk) {
			if (chunk->tag == MKTAG('A', '2', '2', '8'))
				break;

			chunk = chunk->next;
		}

		if (chunk)
			g_rtfPendingFrame->duration = chunk->duration;
		else
			g_rtfPendingFrame->duration = 0;

		if (outFrameData)
			*outFrameData = g_rtfPendingFrame;

		return true;
	}

	if (g_rtfSoundActive)
		g_rtfSoundActive = readPacket();

	return true;
}

bool BoltEngine::isRTFPlaying() {
	if (g_rtfSoundActive)
		return true;

	if (g_rtfSoundQueueHead)
		return true;

	if (g_rtfQueuedSoundCount > 10)
		return true;

	return false;
}

bool BoltEngine::killRTF(uint32 *outFilePos) {
	if (!g_rtfFileHandle)
		return false;

	if (!g_ringBufBase)
		return false;

	_xp->stopSound();
	
	if (outFilePos)
		*outFilePos = g_rtfAnimStartOffset;

	return true;
}

bool BoltEngine::readPacket() {
	uint32 readSize;
	uint32 bytesRead;
	byte padByte;
	byte *destPtr;

	if (!g_rtfMidChunk) {
		// Read 8-byte chunk header...
		bytesRead = 4;
		if (!_xp->readFile(g_rtfFileHandle, &g_rtfChunkTag, &bytesRead) || bytesRead != 4)
			return false;

		bytesRead = 4;
		if (!_xp->readFile(g_rtfFileHandle, &g_rtfChunkSize, &bytesRead) || bytesRead != 4)
			return false;

		g_rtfChunkTag = FROM_BE_32(g_rtfChunkTag);
		g_rtfChunkSize = FROM_BE_32(g_rtfChunkSize);

		// End-of-file marker: 'EOR '
		if (g_rtfChunkTag == MKTAG('E', 'O', 'R',' '))
			return false;

		g_rtfChunkRemaining = g_rtfChunkSize;
		g_rtfMidChunk = true;
		g_rtfCurrentPacket = nullptr;
	}

	if (!g_rtfCurrentPacket) {
		g_rtfCurrentPacket = allocPacket(g_rtfChunkSize);
		if (!g_rtfCurrentPacket)
			return true;

		g_rtfCurrentPacket->tag = g_rtfChunkTag;
	}

	// Read up to 4096 bytes at a time...
	readSize = (g_rtfChunkRemaining > 0x1000) ? 0x1000 : g_rtfChunkRemaining;

	if (readSize != 0) {
		destPtr = &g_rtfCurrentPacket->dataPtr[g_rtfChunkSize - g_rtfChunkRemaining];

		bytesRead = readSize;
		if (!_xp->readFile(g_rtfFileHandle, destPtr, &bytesRead))
			return false;

		g_rtfChunkRemaining -= bytesRead;
	}

	if (g_rtfChunkRemaining != 0)
		return true; // More data to read...

	// Chunk fully read!
	g_rtfMidChunk = false;

	// Skip padding byte for odd-sized chunks...
	if (g_rtfChunkSize & 1) {
		bytesRead = 1;
		if (!_xp->readFile(g_rtfFileHandle, &padByte, &bytesRead))
			return false;
	}

	// Finally update file position tracker...
	g_rtfAnimStartOffset += g_rtfChunkSize + (g_rtfChunkSize & 1) + 8;

	preProcessPacket(g_rtfCurrentPacket);

	return true;
}

void BoltEngine::preProcessPacket(RTFPacket *packet) {
	if (packet->tag == MKTAG('A', '2', '2', '8')) {
		// Sound chunk: attempt to play it immediately!
		if (g_rtfQueuedSoundCount < 50 && !g_rtfNeedInitialFill) {
			g_rtfQueuedSoundCount++;
			_xp->playSound(packet->dataPtr, packet->dataSize, 22050);

			if (!g_rtfSoundPlayHead)
				g_rtfSoundPlayHead = packet;
		} else {
			// Can't play yet, mark queue head...
			if (!g_rtfSoundQueueHead)
				g_rtfSoundQueueHead = packet;
		}

		// Duration in hundredths of a second: dataSize / 2205 (22050 Hz / 10)
		packet->duration = packet->dataSize / 2205;
		if (packet->duration == 0)
			packet->duration = 1;

		g_rtfCumulativeTime += packet->duration;
		g_rtfSoundTiming += packet->duration;
	}

	// Stamp packet with current timeline position...
	packet->timestamp = g_rtfCumulativeTime;

	queuePacket(packet);
}

void BoltEngine::queuePacket(RTFPacket *packet) {
	debug(5, "BoltEngine::queuePacket(): Queueing packet of type: %c%c%c%c",
		  (packet->tag >> 24) & 0xFF,
		  (packet->tag >> 16) & 0xFF,
		  (packet->tag >> 8) & 0xFF,
		  packet->tag & 0xFF);

	if (g_rtfChunkListTail) {
		g_rtfChunkListTail->next = packet;
	} else {
		g_rtfChunkListHead = packet;
	}

	g_rtfChunkListTail = packet;
	packet->next = nullptr;
	g_rtfChunkCount++;
}

RTFPacket *BoltEngine::deQueuePacket() {
	if (g_rtfChunkListHead)
		debug(5, "BoltEngine::deQueuePacket(): Dequeuing packet of type: %c%c%c%c",
			  (g_rtfChunkListHead->tag >> 24) & 0xFF,
			  (g_rtfChunkListHead->tag >> 16) & 0xFF,
			  (g_rtfChunkListHead->tag >> 8) & 0xFF,
			  g_rtfChunkListHead->tag & 0xFF);

	RTFPacket *packet = g_rtfChunkListHead;

	if (g_rtfChunkListHead) {
		g_rtfChunkListHead = packet->next;

		if (!g_rtfChunkListHead)
			g_rtfChunkListTail = nullptr;

		packet->next = nullptr;
	}

	g_rtfChunkCount--;
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
	available = (uint32)(g_ringBufBase - g_ringBufWritePtr) + g_ringBufSize;

	if (available >= g_ringBufFreeSpace) {
		available = g_ringBufFreeSpace;
		freeAfterWrap = 0;
	} else {
		freeAfterWrap = g_ringBufFreeSpace - available;
	}

	if (available >= totalSize) {
		allocPtr = g_ringBufWritePtr;
	} else {
		// Not enough space before the end: try wrapping to buffer start...
		if (freeAfterWrap < totalSize)
			return nullptr;

		allocPtr = g_ringBufBase;
		g_ringBufWritePtr = g_ringBufBase;
		g_ringBufUsed = available;
		g_ringBufFreeSpace -= available;
	}

	g_ringBufWritePtr += totalSize;
	g_ringBufFreeSpace -= totalSize;

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
	g_ringBufFreeSpace += packet->allocSize;

	// Calculate distance from packet to buffer end...
	uint32 distToEnd = g_ringBufSize - (packet->ringBufPtr - g_ringBufBase) - (packet->allocSize);

	// If this was the last packet before the wrap point, reclaim the gap!
	if (distToEnd == g_ringBufUsed) {
		g_ringBufFreeSpace += g_ringBufUsed;
		g_ringBufUsed = 0;
	}

	// If buffer is completely empty, reset write pointer!
	if (g_ringBufFreeSpace == g_ringBufSize)
		g_ringBufWritePtr = g_ringBufBase;

	delete packet;
}

void BoltEngine::resetPlaybackState() {
	g_rtfFileHandle = nullptr;
	g_ringBufBase = nullptr;
	g_rtfSoundActive = false;
}

} // End of namespace Bolt
