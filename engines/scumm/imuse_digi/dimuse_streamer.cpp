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

#include "scumm/imuse_digi/dimuse_engine.h"

namespace Scumm {

int IMuseDigital::streamerInit() {
	for (int l = 0; l < DIMUSE_MAX_STREAMS; l++) {
		_streams[l].soundId = 0;
	}
	_lastStreamLoaded = nullptr;
	return 0;
}

IMuseDigiStream *IMuseDigital::streamerAllocateSound(int soundId, int bufId, int32 maxRead) {
	IMuseDigiSndBuffer *bufInfoPtr = _filesHandler->getBufInfo(bufId);
	if (!bufInfoPtr) {
		debug(5, "IMuseDigital::streamerAlloc(): ERROR: couldn't get buffer info");
		return nullptr;
	}

	if ((bufInfoPtr->bufSize / 4) <= maxRead) {
		debug(5, "IMuseDigital::streamerAlloc(): ERROR: maxRead too big for buffer");
		return nullptr;
	}

	for (int l = 0; l < DIMUSE_MAX_STREAMS; l++) {
		if (_streams[l].soundId && _streams[l].bufId == bufId) {
			debug(5, "IMuseDigital::streamerAlloc(): ERROR: stream bufId %d already in use", bufId);
			return nullptr;
		}
	}

	for (int l = 0; l < DIMUSE_MAX_STREAMS; l++) {
		if (!_streams[l].soundId) {
			_streams[l].endOffset = _filesHandler->seek(soundId, 0, SEEK_END, bufId);
			_streams[l].curOffset = 0;
			_streams[l].soundId = soundId;
			_streams[l].bufId = bufId;
			_streams[l].buf = bufInfoPtr->buffer;
			_streams[l].bufFreeSize = bufInfoPtr->bufSize - maxRead - (_isEarlyDiMUSE ? 0 : 4);
			_streams[l].loadSize = bufInfoPtr->loadSize;
			_streams[l].criticalSize = bufInfoPtr->criticalSize;
			_streams[l].maxRead = maxRead;
			_streams[l].loadIndex = 0;
			_streams[l].readIndex = 0;
			_streams[l].paused = 0;
			_streams[l].vocLoopFlag = 0;
			_streams[l].vocLoopTriggerOffset = 0;
			return &_streams[l];
		}
	}
	debug(5, "IMuseDigital::streamerAlloc(): ERROR: no spare streams");
	return nullptr;
}

int IMuseDigital::streamerClearSoundInStream(IMuseDigiStream *streamPtr) {
	streamPtr->soundId = 0;
	if (_lastStreamLoaded == streamPtr) {
		_lastStreamLoaded = 0;
	}
	return 0;
}

int IMuseDigital::streamerProcessStreams() {
	if (!_isEarlyDiMUSE)
		dispatchPredictFirstStream();

	IMuseDigiStream *stream1 = nullptr;
	IMuseDigiStream *stream2 = nullptr;

	for (int l = 0; l < DIMUSE_MAX_STREAMS; l++) {
		if ((_streams[l].soundId) && (!_streams[l].paused)) {
			if (stream2) {
				if (stream1) {
					debug(5, "IMuseDigital::streamerProcessStreams(): WARNING: three streams in use");
				} else {
					stream1 = &_streams[l];
				}
			} else {
				stream2 = &_streams[l];
			}
		}
	}

	if (!stream1) {
		if (stream2)
			streamerFetchData(stream2);
		return 0;
	}

	if (!stream2) {
		if (stream1)
			streamerFetchData(stream1);
		return 0;
	}

	// Check if we've reached or surpassed criticalSize
	bool critical1 = (streamerGetFreeBufferAmount(stream1) >= stream1->criticalSize);
	bool critical2 = (streamerGetFreeBufferAmount(stream2) >= stream2->criticalSize);

	if (!critical1) {
		if (!critical2) {
			if (stream1 == _lastStreamLoaded) {
				streamerFetchData(stream1);
				streamerFetchData(stream2);
				return 0;
			} else {
				streamerFetchData(stream2);
				streamerFetchData(stream1);
				return 0;
			}
		} else {
			streamerFetchData(stream1);
			return 0;
		}
	}

	if (!critical2) {
		streamerFetchData(stream2);
		return 0;
	} else {
		if (stream1 == _lastStreamLoaded) {
			streamerFetchData(stream1);
		} else {
			streamerFetchData(stream2);
		}
		return 0;
	}
}

uint8 *IMuseDigital::streamerGetStreamBuffer(IMuseDigiStream *streamPtr, int size) {
	if (size > streamerGetFreeBufferAmount(streamPtr) || streamPtr->maxRead < size)
		return 0;

	if (size > streamPtr->bufFreeSize - streamPtr->readIndex) {
		memcpy(&streamPtr->buf[streamPtr->bufFreeSize],
			streamPtr->buf,
			size - streamPtr->bufFreeSize + streamPtr->readIndex + (_isEarlyDiMUSE ? 0 : 4));
	}

	int32 readIndex_tmp = streamPtr->readIndex;
	uint8 *ptr = &streamPtr->buf[readIndex_tmp];
	streamPtr->readIndex = readIndex_tmp + size;
	if (streamPtr->bufFreeSize <= readIndex_tmp + size)
		streamPtr->readIndex = readIndex_tmp + size - streamPtr->bufFreeSize;

	return ptr;
}

uint8 *IMuseDigital::streamerGetStreamBufferAtOffset(IMuseDigiStream *streamPtr, int32 offset, int size) {
	if (offset + size > streamerGetFreeBufferAmount(streamPtr) || streamPtr->maxRead < size)
		return 0;

	int32 offsetReadIndex = offset + streamPtr->readIndex;
	if (offsetReadIndex >= streamPtr->bufFreeSize) {
		offsetReadIndex -= streamPtr->bufFreeSize;
	}

	if (size > streamPtr->bufFreeSize - offsetReadIndex) {
		memcpy(&streamPtr->buf[streamPtr->bufFreeSize], streamPtr->buf, size + offsetReadIndex - streamPtr->bufFreeSize);
	}

	return &streamPtr->buf[offsetReadIndex];
}

int IMuseDigital::streamerSetReadIndex(IMuseDigiStream *streamPtr, int offset) {
	_streamerBailFlag = 1;
	if (offset > streamerGetFreeBufferAmount(streamPtr))
		return -1;

	streamPtr->readIndex += offset;
	if (streamPtr->readIndex >= streamPtr->bufFreeSize) {
		streamPtr->readIndex -= streamPtr->bufFreeSize;
	}
	return 0;
}

int IMuseDigital::streamerSetLoadIndex(IMuseDigiStream *streamPtr, int offset) {
	_streamerBailFlag = 1;
	if (offset > streamerGetFreeBufferAmount(streamPtr))
		return -1;

	streamPtr->loadIndex = streamPtr->readIndex + offset;
	if (streamPtr->loadIndex >= streamPtr->bufFreeSize) {
		streamPtr->loadIndex -= streamPtr->bufFreeSize;
	}
	return 0;
}

int IMuseDigital::streamerGetFreeBufferAmount(IMuseDigiStream *streamPtr) {
	int32 freeBufferSize = streamPtr->loadIndex - streamPtr->readIndex;
	if (freeBufferSize < 0)
		freeBufferSize += streamPtr->bufFreeSize;
	return freeBufferSize;
}

int IMuseDigital::streamerSetSoundToStreamFromOffset(IMuseDigiStream *streamPtr, int soundId, int32 offset) {
	_streamerBailFlag = 1;
	streamPtr->soundId = soundId;
	streamPtr->curOffset = offset;
	streamPtr->endOffset = _isEarlyDiMUSE ? _filesHandler->seek(streamPtr->soundId, 0, SEEK_END, streamPtr->bufId) : 0;
	streamPtr->paused = 0;
	if (_lastStreamLoaded == streamPtr) {
		_lastStreamLoaded = nullptr;
	}
	return 0;
}

int IMuseDigital::streamerQueryStream(IMuseDigiStream *streamPtr, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused) {
	dispatchPredictFirstStream();
	bufSize = streamPtr->bufFreeSize;
	criticalSize = streamPtr->criticalSize;
	freeSpace = streamerGetFreeBufferAmount(streamPtr);
	paused = streamPtr->paused;
	return 0;
}

int IMuseDigital::streamerFeedStream(IMuseDigiStream *streamPtr, uint8 *srcBuf, int32 sizeToFeed, int paused) {
	int32 size = streamPtr->readIndex - streamPtr->loadIndex;
	if (size <= 0)
		size += streamPtr->bufFreeSize;
	if (sizeToFeed > (size - 4)) {
		// Don't worry, judging from the intro of The Dig, this is totally expected
		// to happen, and when it does, the output matches the EXE behavior
		debug(5, "IMuseDigital::streamerFeedStream(): WARNING: buffer overflow");
		_streamerBailFlag = 1;

		int32 newTentativeSize = sizeToFeed - (size - 4) - (sizeToFeed - (size - 4)) % 12 + 12;
		size = streamPtr->loadIndex - streamPtr->readIndex;
		if (size < 0)
			size += streamPtr->bufFreeSize;
		if (size >= newTentativeSize) {
			streamPtr->readIndex = newTentativeSize + streamPtr->readIndex;
			if (streamPtr->bufFreeSize <= streamPtr->readIndex)
				streamPtr->readIndex -= streamPtr->bufFreeSize;
		}
	}

	if (sizeToFeed > 0) {
		do {
			size = streamPtr->bufFreeSize - streamPtr->loadIndex;
			if (size >= sizeToFeed) {
				size = sizeToFeed;
			}
			sizeToFeed -= size;
			memcpy(&streamPtr->buf[streamPtr->loadIndex], srcBuf, size);
			srcBuf += size;

			int val = size + streamPtr->loadIndex;
			streamPtr->curOffset += size;
			streamPtr->loadIndex += size;
			if (val >= streamPtr->bufFreeSize) {
				streamPtr->loadIndex = val - streamPtr->bufFreeSize;
			}
		} while (sizeToFeed > 0);
	}

	streamPtr->paused = paused;
	return 0;
}

int IMuseDigital::streamerFetchData(IMuseDigiStream *streamPtr) {
	if (!_isEarlyDiMUSE && streamPtr->endOffset == 0) {
		streamPtr->endOffset = _filesHandler->seek(streamPtr->soundId, 0, SEEK_END, streamPtr->bufId);
	}

	int32 size = streamPtr->readIndex - streamPtr->loadIndex;
	if (size <= 0)
		size += streamPtr->bufFreeSize;

	int32 loadSize = streamPtr->loadSize;
	int32 remainingSize = streamPtr->endOffset - streamPtr->curOffset;

	if (loadSize >= size - (_isEarlyDiMUSE ? 1 : 4)) {
		loadSize = size - (_isEarlyDiMUSE ? 1 : 4);
	}

	if (loadSize >= remainingSize) {
		loadSize = remainingSize;
	}

	if (remainingSize <= 0) {
		streamPtr->paused = 1;

		if (!_isEarlyDiMUSE) {
			// Pad the buffer
			streamPtr->buf[streamPtr->loadIndex] = 127;
			streamPtr->loadIndex++;
			streamPtr->buf[streamPtr->loadIndex] = 127;
			streamPtr->loadIndex++;
			streamPtr->buf[streamPtr->loadIndex] = 127;
			streamPtr->loadIndex++;
			streamPtr->buf[streamPtr->loadIndex] = 127;
			streamPtr->loadIndex++;
		}
	}

	int32 actualAmount;
	int32 requestedAmount;

	while (1) {
		if (!_isEarlyDiMUSE && loadSize <= 0)
			return 0;

		requestedAmount = streamPtr->bufFreeSize - streamPtr->loadIndex;
		if (requestedAmount >= loadSize) {
			requestedAmount = loadSize;
		}

		if (_filesHandler->seek(streamPtr->soundId, streamPtr->curOffset, SEEK_SET, streamPtr->bufId) != streamPtr->curOffset) {
			debug(5, "IMuseDigital::streamerFetchData(): ERROR: invalid seek in streamer (%d), pausing stream...", streamPtr->curOffset);
			streamPtr->paused = 1;
			return 0;
		}

		_streamerBailFlag = 0;

		Common::StackLock lock(_mutex);
		actualAmount = _filesHandler->read(streamPtr->soundId, &streamPtr->buf[streamPtr->loadIndex], requestedAmount, streamPtr->bufId);
		Common::StackLock unlock(_mutex);

		// FT has no bailFlag
		if (!_isEarlyDiMUSE && _streamerBailFlag)
			return 0;

		loadSize -= actualAmount;
		streamPtr->curOffset += actualAmount;
		_lastStreamLoaded = streamPtr;

		int32 newLoadIndex = actualAmount + streamPtr->loadIndex;
		streamPtr->loadIndex = newLoadIndex;
		if (newLoadIndex >= streamPtr->bufFreeSize) {
			streamPtr->loadIndex = newLoadIndex - streamPtr->bufFreeSize;
		}

		if (_isEarlyDiMUSE && streamPtr->vocLoopFlag) {
			if (streamPtr->vocLoopTriggerOffset <= streamPtr->curOffset) {
				dispatchVOCLoopCallback(streamPtr->soundId);
				streamPtr->vocLoopFlag = 0;
			}
		}

		if (actualAmount != requestedAmount)
			break;

		// FT checks for negative or zero loadSize here
		if (_isEarlyDiMUSE && loadSize <= 0)
			return 0;
	}

	debug(5, "IMuseDigital::streamerFetchData(): ERROR: unable to load the correct amount of data (req=%d, act=%d)", requestedAmount, actualAmount);
	_lastStreamLoaded = nullptr;
	return 0;
}

void IMuseDigital::streamerSetLoopFlag(IMuseDigiStream *streamPtr, int offset) {
	streamPtr->vocLoopFlag = 1;
	streamPtr->vocLoopTriggerOffset = offset;
}

void IMuseDigital::streamerRemoveLoopFlag(IMuseDigiStream *streamPtr) {
	streamPtr->vocLoopFlag = 0;
}

} // End of namespace Scumm
