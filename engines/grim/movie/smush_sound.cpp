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

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"

#include "engines/grim/movie/smush_mixer.h"
#include "engines/grim/movie/smush_sound.h"
#include "engines/grim/movie/smush_timer.h"

namespace Grim {

#define SOU_BUFFER_SLACK 6144

SmushSound::SmushSound(SmushTimer *timer, SmushDriver *driver) {
	_timer = timer;
	_driver = driver;
	memset(_channels, 0, sizeof(_channels));
	memset(_playback, 0, sizeof(_playback));
	_channelCount = 0;
	_masterVolume = 127;
	_sampleRate = 22050;
	_duckFloor = 64;
	_duckRate = 256;
	_duckLevel = 256;
	_resetFlag = 1;
	_bytesPerFrame[0] = 1;
	_bytesPerFrame[1] = 2;
	_bytesPerFrame[2] = 2;
	_bytesPerFrame[3] = 4;
	_bytesPerFrame[4] = 2;
	_bytesPerFrame[5] = 4;
	memset(_groupVolumes, 0, sizeof(_groupVolumes));
	_pauseCount = 0;
	_engineOk = 0;
	_syncMark = 0;
	_syncAccumulator = 0.0;
	_fillCounter = 0;
	_musicParam = 0;
	_voiceParam = 0;
	_normalParam = 0;
	_panInvert = 0;
	_driverRate = 22050;
	_reserveRam = 64000;
	memset(_bufferSizes, 0, sizeof(_bufferSizes));
	_bufferSizes[0] = 500000;
}

SmushSound::~SmushSound() {
	engineTerminate();
}

int SmushSound::moduleStartup() {
	return engineInitialize(_driverRate, _reserveRam, _bufferSizes);
}

int SmushSound::moduleShutdown() {
	return engineTerminate();
}

int SmushSound::engineInitialize(int rate, int reserveRam, const int *bufferSizes) {
	_channelCount = 0;
	_masterVolume = 127;
	memset(_groupVolumes, 0, sizeof(_groupVolumes));

	int size = bufferSizes[0];
	int index = 1;
	while (size) {
		size &= ~3;
		if (engineAllocChannel(size, reserveRam)) {
			engineTerminate();
			return -1;
		}

		size = bufferSizes[index++];
	}

	_engineOk = (_driver->sndDrivInitialize(rate, _channelCount, this) == 0);
	_sampleRate = _driver->sndDrivGetRate();
	_fillCounter = 0;
	_musicParam = 1;
	_voiceParam = 1;
	_normalParam = 1;
	_timer->sysExtSync(0);

	return 0;
}

int SmushSound::engineTerminate() {
	if (_engineOk) {
		_driver->sndDrivTerminate();
		_engineOk = 0;
	}

	for (int i = 0; i < _channelCount; i++) {
		free(_channels[i].dataBuffer);
		_channels[i].dataBuffer = nullptr;
		free(_channels[i].stagingBuffer);
		_channels[i].stagingBuffer = nullptr;
	}

	int result = _fillCounter;
	_channelCount = 0;
	return result;
}

int SmushSound::engineAllocChannel(int size, int reserveRam) {
	SmushSoundChannel *chan = &_channels[_channelCount];

	chan->state = 0;
	chan->consumedPosition = 0;
	chan->soundClass = 0;
	chan->soundId = SOU_FREE_ID;
	chan->dataBuffer = nullptr;
	chan->stagingBuffer = nullptr;
	chan->capacity = size & ~3;

	chan->stagingBuffer = (byte *)calloc(6144, 1);
	if (!chan->stagingBuffer)
		return -1;

	chan->dataBuffer = (byte *)calloc(chan->capacity + SOU_BUFFER_SLACK, 1);
	if (chan->dataBuffer) {
		memset(chan->dataBuffer, 127, chan->capacity);
		_channelCount++;
		return 0;
	}

	if (chan->capacity > reserveRam) {
		do {
			chan->dataBuffer = (byte *)calloc(chan->capacity + SOU_BUFFER_SLACK, 1);
			if (chan->dataBuffer)
				break;

			chan->capacity -= 10000;
		} while (chan->capacity > reserveRam);
	}

	chan->capacity &= ~3;
	if (chan->capacity > reserveRam) {
		free(chan->dataBuffer);
		chan->dataBuffer = nullptr;

		chan->capacity -= reserveRam;
		chan->dataBuffer = (byte *)calloc(chan->capacity + SOU_BUFFER_SLACK, 1);
		if (chan->dataBuffer) {
			memset(chan->dataBuffer, 127, chan->capacity);
			_channelCount++;
			return 0;
		}
	}

	free(chan->stagingBuffer);
	chan->stagingBuffer = nullptr;
	return -2;
}

int SmushSound::souEngineStart(const byte *data, int soundId, int volume, int pan, int soundClass) {
	if (!_engineOk)
		return 0;

	uint32 tag = READ_BE_UINT32(data);
	if (tag == MKTAG('P', 'S', 'A', 'D') || tag == MKTAG('P', 'V', 'O', 'C')) {
		if (data[8] || data[9] || data[12] || data[13] || data[16] || data[17]) {
			int8 panByte = (int8)data[17];
			int newPan;
			if (panByte == -128)
				newPan = 128;
			else
				newPan = panByte + pan;

			return souEngineStartStream(data + 18, READ_BE_UINT32(data + 4) - 10, soundId, (volume * data[16]) >> 7, newPan,
			                            soundClass | READ_LE_INT16(data + 14), READ_LE_INT16(data + 8), READ_LE_INT16(data + 10), READ_LE_INT16(data + 12), 1);
		}

		return souEngineStartStream(data + 20, READ_BE_UINT32(data + 4) - 12, soundId, volume, pan, soundClass,
		                            READ_BE_UINT16(data + 10), READ_BE_UINT16(data + 14), READ_BE_UINT16(data + 18), 1);
	}

	if (tag == MKTAG('C', 'r', 'e', 'a')) {
		int offset = data[20] + (data[21] << 8);
		// The disasm builds the size by repeating a single byte three times
		return souEngineStartCreative(data + offset + 6, 65793 * data[offset + 1], soundId, volume, pan, soundClass);
	}

	if (tag == MKTAG('S', 'A', 'U', 'D'))
		return souEngineStartStream(data, READ_BE_UINT32(data + 4) + 8, soundId, volume, pan, soundClass, 0, 0, 1, 0);

	return 0;
}

void SmushSound::souEngineStop(int soundId) {
	for (int i = 0; i < _channelCount; i++) {
		if (_channels[i].soundId == soundId) {
			_channels[i].state = 0;
			_channels[i].soundId = SOU_FREE_ID;
		}
	}
}

void SmushSound::souEnginePause() {
	_pauseCount++;
}

int SmushSound::souEngineResume() {
	if (_pauseCount)
		return --_pauseCount;

	return _pauseCount;
}

void SmushSound::souEngineStopAll() {
	for (int i = 0; i < _channelCount; i++) {
		_channels[i].state = 0;
		_channels[i].soundId = SOU_FREE_ID;
	}
}

void SmushSound::souEngineSetVol(int soundId, int volume) {
	if (soundId == SOU_FREE_ID) {
		_masterVolume = volume;
		return;
	}

	for (int i = 0; i < _channelCount; i++) {
		if (_channels[i].soundId == soundId)
			_channels[i].volume = volume;
	}
}

int SmushSound::souEngineGetVol(int soundId) {
	if (soundId == SOU_FREE_ID)
		return _masterVolume;

	for (int i = 0; i < _channelCount; i++) {
		if (_channels[i].soundId == soundId)
			return _channels[i].volume;
	}

	return -1;
}

void SmushSound::souEngineSetPan(int soundId, int pan) {
	for (int i = 0; i < _channelCount; i++) {
		if (_channels[i].soundId == soundId)
			_channels[i].pan = pan;
	}
}

int SmushSound::souEngineGetPan(int soundId) {
	for (int i = 0; i < _channelCount; i++) {
		if (_channels[i].soundId == soundId)
			return _channels[i].pan;
	}

	return -1;
}

int SmushSound::souEngineSetHVol(int volume) {
	if (_driver)
		return _driver->sndDrivSetVol(volume);

	return 0;
}

int SmushSound::souEngineGetHVol() {
	if (_driver)
		return _driver->sndDrivGetVol();

	return -1;
}

int SmushSound::souEngineSetRate(int rate) {
	_sampleRate = rate;
	if (_driver)
		return _driver->sndDrivDummy(rate);

	return rate;
}

int SmushSound::souEngineGetRate() {
	if (_driver)
		return _driver->sndDrivGetRate();

	return -1;
}

int SmushSound::souEngineQuery(int soundId) {
	if (soundId == SOU_FREE_ID) {
		for (int i = 0; i < _channelCount; i++) {
			if (_channels[i].state)
				return _channels[i].consumedPosition + 1;
		}

		return 0;
	}

	if (soundId >= 0 || -1 - soundId >= _channelCount) {
		for (int i = 0; i < _channelCount; i++) {
			if (_channels[i].soundId == soundId) {
				if (_channels[i].state)
					return _channels[i].consumedPosition + 1;

				break;
			}
		}
	} else {
		SmushSoundChannel *chan = &_channels[-1 - soundId];
		if (chan->state)
			return chan->consumedPosition + 1;
	}

	return 0;
}

int SmushSound::souEngineQueryTime(int channelIndex) {
	int samples = _channels[channelIndex].consumedPosition;

	switch (_channels[channelIndex].format) {
	case 1:
	case 2:
	case 4:
		samples >>= 1;
		break;
	case 3:
	case 5:
		samples >>= 2;
		break;
	default:
		break;
	}

	return (int)(int64)((double)samples / ((double)souEngineGetRate() * 0.001));
}

int SmushSound::souEngineSetParam(int soundId, int param, int value1, int value2) {
	switch (param) {
	case 1:
		setGroupVolume(value1, value2);
		return 0;
	case 2:
		if (value1)
			_duckFloor = value1;
		if (!value2)
			return 0;

		_duckRate = value2;
		return 0;
	case 3:
		_musicParam = value1;
		return 0;
	case 4:
		_voiceParam = value1;
		return 0;
	case 5:
		_normalParam = value1;
		return 0;
	default:
		return -1;
	}
}

int SmushSound::souEngineGetParam(int soundId, int param, int *value1, int *value2) {
	switch (param) {
	case 1:
		// The disasm passes the group index in the pointer slot
		if (value1)
			*value1 = getGroupVolume((uint8)(intptr)value2);
		return 0;
	case 2:
		if (value1)
			*value1 = _duckFloor;
		if (value2)
			*value2 = _duckRate;
		return 0;
	case 3:
		if (value1)
			*value1 = _musicParam;
		return 0;
	case 4:
		if (value1)
			*value1 = _voiceParam;
		return 0;
	case 5:
		if (value1)
			*value1 = _normalParam;
		return 0;
	default:
		return -1;
	}
}

uint8 SmushSound::getGroupVolume(uint8 index) {
	if (index == 0xFF)
		return _masterVolume;

	return _groupVolumes[index];
}

void SmushSound::setGroupVolume(uint8 index, uint8 volume) {
	if (index == 0xFF)
		_masterVolume = volume;

	_groupVolumes[index] = volume;
}

int SmushSound::souEngineMarkSyncTime() {
	_syncMark = _timer->sysGetTime();
	return _syncMark;
}

int SmushSound::souEngineExternalSync(double ticks) {
	_syncAccumulator += ticks;

	if (_syncMark) {
		int now = _timer->sysGetTime();
		if (now > _syncMark) {
			int elapsed = now - _syncMark;
			if (elapsed / 5 < 1193180) {
				_syncMark = 0;
				_syncAccumulator -= (double)elapsed;
			}
		}
	}

	_timer->sysExtSync((int)(int64)_syncAccumulator);
	int result = (int)(int64)_syncAccumulator;
	_syncAccumulator -= (double)(int)(int64)_syncAccumulator;
	return result;
}

int SmushSound::souEngineStartCreative(const byte *data, int size, int soundId, int volume, int pan, int soundClass) {
	int largest = 0;
	for (int i = _channelCount - 1; i >= 0; i--) {
		if (_channels[i].capacity > largest)
			largest = _channels[i].capacity;
	}

	int bestDistance = 10000000;
	for (int i = _channelCount - 1; i >= 0; i--) {
		int capacity = _channels[i].capacity;
		int distance = capacity - size;
		if (distance < 0)
			distance = size - capacity;

		if (distance < bestDistance && size < capacity) {
			bestDistance = distance;
			largest = capacity;
		}
	}

	int scores[SOU_MAX_CHANNELS] = {};
	for (int i = _channelCount - 1; i >= 0; i--) {
		SmushSoundChannel *chan = &_channels[i];
		scores[i] = 0;

		if (chan->capacity != largest)
			continue;

		if (chan->state && chan->state != 3 && chan->soundClass > soundClass)
			continue;

		if (!chan->state)
			scores[i] = 4096;
		if (chan->soundClass < soundClass)
			scores[i] += 2048;
		if (chan->soundClass == soundClass)
			scores[i] += 1024;
		if (chan->state == 3)
			scores[i] += 512;
		scores[i] += chan->age + 1;
	}

	int best = -1;
	int bestScore = 0;
	for (int i = _channelCount - 1; i >= 0; i--) {
		if (scores[i] > bestScore) {
			best = i;
			bestScore = scores[i];
		}
	}

	if (best < 0)
		return 0;

	for (int i = _channelCount - 1; i >= 0; i--) {
		if (_channels[i].age < 256)
			_channels[i].age++;
	}

	SmushSoundChannel *chan = &_channels[best];
	chan->age = 0;
	chan->state = 0;

	byte *buffer = chan->dataBuffer;
	buffer[0] = 1;
	buffer[1] = 8;
	buffer[2] = 0;
	buffer[3] = 0;
	buffer[4] = 0;
	buffer[5] = 0;
	buffer[6] = size >> 24;
	buffer[7] = size >> 16;
	buffer[8] = size >> 8;
	buffer[9] = size;
	buffer[10] = 0;
	buffer[11] = 0;
	memcpy(buffer + 12, data, size);

	chan->scriptPointer = buffer;
	chan->dataArea = buffer + 12;
	chan->dataAreaSize = chan->capacity - 12;
	chan->writePosition = size;
	chan->soundId = soundId;
	chan->volume = volume;
	chan->pan = pan;
	chan->soundClass = soundClass;
	chan->consumedPosition = 0;
	chan->state = 2;

	return pan;
}

int SmushSound::souEngineStartStream(const byte *data, int size, int soundId, uint32 volume, int pan, int soundClass, int tag1, int chunkCounter, int tag2, int copyFlag) {
	if (chunkCounter) {
		int index = _channelCount - 1;
		if (index < 0)
			return index;

		while (!_channels[index].state || tag1 != _channels[index].tag1 || chunkCounter != _channels[index].chunkCounter + 1 || tag2 != _channels[index].tag2) {
			index--;
			if (index < 0)
				return index;
		}

		SmushSoundChannel *chan = &_channels[index];
		chan->chunkCounter++;

		int position = chan->writePosition % chan->dataAreaSize;
		if (position + size <= chan->dataAreaSize) {
			memcpy(chan->dataArea + position, data, size);
		} else {
			memcpy(chan->dataArea + position, data, chan->dataAreaSize - position);
			memcpy(chan->dataArea, data + (chan->dataAreaSize - position), position + size - chan->dataAreaSize);
		}

		if (volume <= 0x7E)
			chan->volume = volume;
		if (pan > -128 && pan < 128)
			chan->pan = pan;

		chan->writePosition += size;
		return pan;
	}

	int streamSize = chunkCounter;
	if (tag2 != 1 || copyFlag)
		streamSize = READ_BE_UINT32(data + 4);

	int largest = 0;
	for (int i = _channelCount - 1; i >= 0; i--) {
		if (_channels[i].capacity > largest)
			largest = _channels[i].capacity;
	}

	int bestDistance = 10000000;
	if ((soundClass & 0xC0) == 0x40) {
		if (tag2 > 1)
			souEngineMarkSyncTime();
	} else {
		for (int i = _channelCount - 1; i >= 0; i--) {
			int capacity = _channels[i].capacity;
			int distance = capacity - streamSize;
			if (distance < 0)
				distance = streamSize - capacity;

			if (distance < bestDistance && capacity > 2 * streamSize / 3) {
				int needed = (tag2 <= 1) ? size : 2 * size;
				if (capacity > needed) {
					bestDistance = distance;
					largest = capacity;
				}
			}
		}
	}

	int scores[SOU_MAX_CHANNELS] = {};
	for (int i = _channelCount - 1; i >= 0; i--) {
		SmushSoundChannel *chan = &_channels[i];
		scores[i] = 0;

		if (chan->capacity != largest)
			continue;

		if (chan->state && chan->state != 3 && chan->soundClass > soundClass)
			continue;

		if (!chan->state)
			scores[i] = 4096;
		if (chan->soundClass < soundClass)
			scores[i] += 2048;
		if (chan->soundClass == soundClass)
			scores[i] += 1024;
		if (chan->state == 3)
			scores[i] += 512;
		scores[i] += chan->age + 1;
	}

	int best = -1;
	int bestScore = -1;
	for (int i = _channelCount - 1; i >= 0; i--) {
		if (scores[i] && scores[i] > bestScore) {
			best = i;
			bestScore = scores[i];
		}
	}

	if (best < 0)
		return best;

	for (int i = _channelCount - 1; i >= 0; i--) {
		if (_channels[i].age < 255)
			_channels[i].age++;
	}

	SmushSoundChannel *chan = &_channels[best];
	chan->tag1 = tag1;
	chan->tag2 = tag2;
	chan->chunkCounter = 0;
	chan->age = 0;
	chan->state = 0;

	int headerSize = READ_BE_UINT32(data + 12);
	int totalSize = READ_BE_UINT32(data + headerSize + 20);

	int copiedSize;
	int bufferSize;
	if (tag2 != 1 || copyFlag) {
		int copySize = chan->capacity;
		copiedSize = size;
		if (size < copySize)
			copySize = size;

		memcpy(chan->dataBuffer, data, copySize);
		chan->scriptPointer = chan->dataBuffer + 16;
		bufferSize = chan->capacity;
	} else {
		copiedSize = size;
		chan->scriptPointer = (byte *)data + 16;
		bufferSize = size;
	}

	chan->dataAreaSize = bufferSize - headerSize - 24;
	chan->dataArea = chan->scriptPointer + headerSize + 8;
	chan->writePosition = copiedSize - headerSize - 24;
	chan->totalSize = totalSize;
	chan->soundId = soundId;
	chan->volume = 127;
	chan->pan = 0;
	if (volume < 0x80)
		chan->volume = volume;
	if (pan > -128 && pan < 128)
		chan->pan = pan;

	chan->soundClass = soundClass;
	chan->consumedPosition = 0;
	chan->state = 2;

	uint32 formatTag = READ_BE_UINT32(chan->dataArea - 8);
	if (formatTag == MKTAG('S', '6', 'S', 'Z')) {
		chan->format = 5;
	} else if (formatTag == MKTAG('S', '6', 'M', 'Z')) {
		chan->format = 4;
	} else if (formatTag == MKTAG('S', 'D', '2', 'S')) {
		chan->format = 1;
	} else if (formatTag > MKTAG('S', 'D', '2', 'S')) {
		if (formatTag == MKTAG('S', 'D', '6', 'M'))
			chan->format = 2;
		else if (formatTag == MKTAG('S', 'D', '6', 'M') + 6)
			chan->format = 3;
		else if (formatTag == MKTAG('S', 'D', '6', 'M') + 518)
			chan->format = 1;
		else
			chan->format = 0;
	} else {
		chan->format = 0;
	}

	return formatTag;
}

void SmushSound::souEngineFill(int frameCount, SmushDriver *driver) {
	_fillCounter++;
	souEngineExternalSync((double)frameCount * 1193180.0 / (double)_sampleRate);

	if (_pauseCount)
		return;

	if (_resetFlag) {
		_resetFlag = 0;
		for (int i = 0; i < SOU_MAX_CHANNELS; i++) {
			_playback[i].tailBytes = 0;
			_playback[i].loopRemaining = 0;
		}
	}

	int voicePlaying = 0;

	for (int i = 0; i < _channelCount; i++) {
		SmushSoundChannel *chan = &_channels[i];
		SmushSoundPlayback *rt = &_playback[i];

		int soundClass = chan->soundClass & 0xC0;
		bool enabled = (soundClass == 128 && _voiceParam) || (soundClass == 64 && _musicParam) || (!soundClass && _normalParam);
		int scaledVolume = chan->volume * _masterVolume;
		int volume = scaledVolume / 127;
		if (soundClass == 64 && _voiceParam)
			volume = (_duckLevel * (scaledVolume / 127)) >> 8;

		if (chan->state == 2 && rt->previousState == 1) {
			int position = rt->dataSize ? rt->readPosition % rt->dataSize : 0;
			rt->tailBytes = 6144;
			rt->previousVolume = chan->volume;
			rt->previousFormat = chan->format;
			rt->previousRate = rt->rate;
			memset(chan->stagingBuffer, 128, 6144);

			int copySize = rt->dataSize - position;
			if (copySize > 6144)
				copySize = 6144;

			memcpy(chan->stagingBuffer, rt->dataStart + position, copySize);
			rt->gain = 0;
		} else if (chan->state == 1) {
			int writePosition = chan->writePosition;
			if (rt->readPosition < writePosition - chan->dataAreaSize + 15000) {
				int totalSize = chan->totalSize;
				if (writePosition < totalSize) {
					rt->gain = 0;
					int available = writePosition - rt->readPosition - 15000;
					if (available > rt->remaining)
						available = rt->remaining;

					int limit = totalSize - rt->dataSize;
					if (available + rt->readPosition > limit)
						available = limit - rt->readPosition;

					if (available > 0) {
						rt->tailBytes = 6144;
						rt->previousRate = rt->rate;
						memcpy(chan->stagingBuffer, rt->dataStart + (rt->dataSize ? rt->readPosition % rt->dataSize : 0), 6144);
						rt->readPosition += available;
						rt->remaining -= available;
					}
				}
			}
		}

		int tailFrameBytes = _bytesPerFrame[rt->previousFormat];
		if (rt->tailBytes > tailFrameBytes) {
			int destOffset = 0;
			int wanted = rt->previousRate * tailFrameBytes * frameCount / _sampleRate;
			int chunkBytes = rt->tailBytes;
			if (chunkBytes >= wanted)
				chunkBytes = wanted;

			chunkBytes -= chunkBytes % 4;
			int remainingBytes = chunkBytes;
			if (chunkBytes >= 4) {
				while (true) {
					if (chunkBytes > 1024)
						chunkBytes = 1024;

					int chunkFrames;
					if (chunkBytes == wanted)
						chunkFrames = frameCount;
					else
						chunkFrames = _sampleRate * (chunkBytes / _bytesPerFrame[rt->previousFormat]) / rt->previousRate;

					if (enabled) {
						int pan = chan->pan;
						if (_panInvert)
							pan = -pan;

						driver->sndDrivMix(chan->stagingBuffer - rt->tailBytes + 6144, destOffset, chunkFrames, chunkBytes, 1,
						                   rt->previousVolume * rt->tailBytes * _masterVolume / 780288, pan, rt->previousFormat);
					}

					destOffset += chunkFrames;
					bool done = remainingBytes - chunkBytes < 4;
					remainingBytes -= chunkBytes;
					rt->tailBytes -= chunkBytes;
					if (done)
						break;

					chunkBytes = remainingBytes;
				}
			}
		}

		if (chan->state == 2) {
			if (rt->previousState != 1)
				rt->gain = 16;

			rt->script = chan->scriptPointer;
			rt->dataStart = chan->dataArea;
			rt->dataSize = chan->dataAreaSize;
			rt->format = chan->format;
			rt->remaining = 0;
			rt->loopRemaining = 0;
			chan->state = 1;
		}

		if (chan->state) {
			int destOffset = 0;
			int framesLeft = frameCount;

			if (frameCount > 0) {
				while (true) {
					if (rt->remaining > 0) {
						int ringPosition = rt->dataSize ? rt->readPosition % rt->dataSize : 0;
						int wanted = rt->rate * framesLeft * _bytesPerFrame[rt->format] / _sampleRate;
						int chunkBytes = rt->remaining;
						if (chunkBytes >= wanted)
							chunkBytes = wanted;

						if (ringPosition + chunkBytes > rt->dataSize)
							chunkBytes = rt->dataSize - ringPosition;

						int gain = rt->gain;
						if (chunkBytes + rt->readPosition <= chan->writePosition) {
							if (gain < 16)
								rt->gain = gain + 1;
							if (chunkBytes > 1024)
								chunkBytes = 1024;

							chan->state = 1;
							if ((chan->soundClass & 0xC0) == 0x80)
								voicePlaying = 1;
						} else {
							if (gain)
								rt->gain = gain - 1;

							chan->state = 3;
							if (chunkBytes > 1024)
								chunkBytes = 1024;

							rt->readPosition -= chunkBytes;
							rt->remaining += chunkBytes;
							ringPosition = rt->dataSize ? rt->readPosition % rt->dataSize : 0;
						}

						int chunkFrames;
						if (chunkBytes == wanted)
							chunkFrames = framesLeft;
						else
							chunkFrames = _sampleRate * (chunkBytes / _bytesPerFrame[rt->format]) / rt->rate;

						if (enabled) {
							int pan = chan->pan;
							if (_panInvert)
								pan = -pan;

							driver->sndDrivMix(rt->dataStart + ringPosition, destOffset, chunkFrames, chunkBytes, 1,
							                   (rt->gain * volume) >> 4, pan, rt->format);
						}

						rt->remaining -= chunkBytes;
						rt->readPosition += chunkBytes;
						framesLeft -= chunkFrames;
						destOffset += chunkFrames;
					}

					if (rt->remaining <= 0) {
						bool backToMix = false;

						while (framesLeft) {
							byte *op = rt->script;

							switch (op[0]) {
							case 1: {
								rt->loopRemaining = 0;
								int dataAreaSize = chan->dataAreaSize;
								rt->readPosition = READ_BE_UINT32(op + 2);
								rt->remaining = READ_BE_UINT32(op + 6);
								rt->rate = _sampleRate;
								rt->script += op[1] + 2;

								int writePosition = chan->writePosition;
								int slack = (writePosition >= chan->totalSize) ? 0 : 15000;
								if (rt->readPosition < writePosition + slack - dataAreaSize) {
									int skip = writePosition - rt->readPosition - dataAreaSize + 15000;
									if (skip <= rt->remaining) {
										rt->readPosition += skip;
										rt->remaining -= skip;
									} else {
										chan->state = 0;
										chan->soundId = SOU_FREE_ID;
										framesLeft = 0;
									}
								}
								break;
							}
							case 2:
							case 8:
							case 9:
							case 10:
							case 11: {
								uint8 index = op[4];
								uint8 value;
								switch (index) {
								case 0xFF:
									value = _masterVolume;
									break;
								case 0xFE:
									value = chan->volume;
									break;
								case 0xFD:
									value = chan->pan;
									break;
								default:
									value = _groupVolumes[index];
									break;
								}

								switch (op[0]) {
								case 2:
									if (value || !index)
										value = 1;
									break;
								case 8:
									value = op[5] < value;
									break;
								case 9:
									value = value < op[5];
									break;
								case 10:
									value = value == op[5];
									break;
								case 11:
									value = value != op[5];
									break;
								default:
									break;
								}

								if (!value)
									rt->script = op + op[1] + 2;
								else
									rt->script = op + READ_BE_UINT16(op + 2);
								break;
							}
							case 3: {
								int8 variable = op[2];
								if (variable == -1) {
									_masterVolume = op[3];
									rt->script = op + op[1] + 2;
								} else if (variable == -2) {
									int newVolume = (int8)op[3];
									int scaled = newVolume * _masterVolume;
									chan->volume = newVolume;
									volume = scaled / 127;
									if ((chan->soundClass & 0xC0) == 0x40 && _voiceParam)
										volume = (_duckLevel * (scaled / 127)) >> 8;

									rt->script = op + op[1] + 2;
								} else {
									if (variable == -3)
										chan->pan = op[3];
									else
										_groupVolumes[op[2]] = op[3];

									rt->script = op + op[1] + 2;
								}
								break;
							}
							case 4: {
								int8 variable = op[2];
								if (variable == -1)
									_masterVolume += op[3];
								else if (variable == -2)
									chan->volume += op[3];
								else if (variable == -3)
									chan->pan += op[3];
								else
									_groupVolumes[op[2]] += op[3];

								rt->script = op + op[1] + 2;
								break;
							}
							case 6:
								rt->loopRemaining = 0;
								rt->readPosition = READ_BE_UINT32(op + 2);
								rt->remaining = READ_BE_UINT32(op + 6);
								rt->rate = READ_BE_UINT32(op + 10);
								rt->script = op + op[1] + 2;
								break;
							case 7: {
								if (!rt->loopRemaining) {
									rt->loopRemaining = READ_BE_UINT32(op + 6);
									rt->repeatAccumulator = 0;
								}

								rt->readPosition = rt->repeatAccumulator + READ_BE_UINT32(op + 2);
								uint32 segmentSize = READ_BE_UINT32(op + 14);
								if ((uint32)rt->loopRemaining < segmentSize)
									segmentSize = rt->loopRemaining;

								rt->remaining = segmentSize;
								rt->rate = READ_BE_UINT32(op + 10);
								rt->loopRemaining -= rt->remaining;
								rt->repeatAccumulator += rt->remaining;
								if (rt->loopRemaining)
									rt->script = op + op[1] + 2;
								else
									rt->script = op + READ_BE_UINT16(op + 18);

								int writePosition = chan->writePosition;
								int dataAreaSize = chan->dataAreaSize;
								int slack = (writePosition >= chan->totalSize) ? 0 : 15000;
								if (rt->readPosition < writePosition + slack - dataAreaSize) {
									int skip = writePosition - rt->readPosition - dataAreaSize + 15000;
									if (skip <= rt->remaining) {
										rt->readPosition += skip;
										rt->remaining -= skip;
									} else {
										chan->state = 0;
										chan->soundId = SOU_FREE_ID;
										framesLeft = 0;
									}
								}
								break;
							}
							default:
								chan->state = 0;
								chan->soundId = SOU_FREE_ID;
								framesLeft = 0;
								break;
							}

							if (rt->remaining > 0) {
								backToMix = true;
								break;
							}
						}

						if (!backToMix)
							break;
					}

					if (framesLeft <= 0)
						break;
				}
			}
		}

		chan->consumedPosition = rt->readPosition;
		rt->previousState = chan->state;
	}

	if (voicePlaying) {
		if (_duckLevel > _duckFloor) {
			_duckLevel -= (_duckRate * frameCount) >> 13;
			if (_duckLevel < _duckFloor)
				_duckLevel = _duckFloor;
		}
	} else if (_duckLevel < 256) {
		_duckLevel += (_duckRate * frameCount) >> 15;
		if (_duckLevel > 256)
			_duckLevel = 256;
	}
}

} // End of namespace Grim
