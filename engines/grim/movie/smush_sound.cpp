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
	_sampleRate = 22050;
	_resetFlag = 1;
	_bytesPerFrame[0] = 1;
	_bytesPerFrame[1] = 2;
	_bytesPerFrame[2] = 2;
	_bytesPerFrame[3] = 4;
	_bytesPerFrame[4] = 2;
	_bytesPerFrame[5] = 4;
	_pauseCount = 0;
	_engineOk = 0;
	_syncMark = 0;
	_syncAccumulator = 0.0;
	_fillCounter = 0;
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

int SmushSound::souEngineStart(const byte *data, int volume, int pan, int soundClass) {
	if (!_engineOk)
		return 0;

	// The only packets which reach the engine are the ones SmushWave builds
	if (READ_BE_UINT32(data) != MKTAG('P', 'S', 'A', 'D'))
		return 0;

	int8 panByte = (int8)data[17];
	int newPan;
	if (panByte == -128)
		newPan = 128;
	else
		newPan = panByte + pan;

	return souEngineStartStream(data + 18, READ_BE_UINT32(data + 4) - 10, (volume * data[16]) >> 7, newPan,
	                            soundClass | READ_LE_INT16(data + 14), READ_LE_INT16(data + 8), READ_LE_INT16(data + 10), READ_LE_INT16(data + 12));
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
	for (int i = 0; i < _channelCount; i++)
		_channels[i].state = 0;
}

int SmushSound::souEngineGetRate() {
	if (_driver)
		return _driver->sndDrivGetRate();

	return -1;
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
			if (elapsed / 5 < SMUSH_TIMER_FREQUENCY) {
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

int SmushSound::souEngineStartStream(const byte *data, int size, uint32 volume, int pan, int soundClass, int tag1, int chunkCounter, int tag2) {
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

	int largest = 0;
	for (int i = _channelCount - 1; i >= 0; i--) {
		if (_channels[i].capacity > largest)
			largest = _channels[i].capacity;
	}

	// Only the music class is started
	if (tag2 > 1)
		souEngineMarkSyncTime();

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

	int copySize = chan->capacity;
	if (size < copySize)
		copySize = size;

	memcpy(chan->dataBuffer, data, copySize);
	chan->scriptPointer = chan->dataBuffer + 16;

	chan->dataAreaSize = chan->capacity - headerSize - 24;
	chan->dataArea = chan->scriptPointer + headerSize + 8;
	chan->writePosition = size - headerSize - 24;
	chan->totalSize = totalSize;
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
	if (formatTag == MKTAG('S', '6', 'S', 'Z'))
		chan->format = 5;
	else if (formatTag == MKTAG('S', '6', 'M', 'Z'))
		chan->format = 4;
	else
		chan->format = 0;

	return formatTag;
}

void SmushSound::souEngineFill(int frameCount, SmushDriver *driver) {
	_fillCounter++;
	souEngineExternalSync((double)frameCount * SMUSH_TIMER_FREQUENCY / (double)_sampleRate);

	if (_pauseCount)
		return;

	if (_resetFlag) {
		_resetFlag = 0;
		for (int i = 0; i < SOU_MAX_CHANNELS; i++)
			_playback[i].tailBytes = 0;
	}

	for (int i = 0; i < _channelCount; i++) {
		SmushSoundChannel *chan = &_channels[i];
		SmushSoundPlayback *rt = &_playback[i];

		int volume = chan->volume;

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

					driver->sndDrivMix(chan->stagingBuffer - rt->tailBytes + 6144, destOffset, chunkFrames, chunkBytes,
					                   rt->previousVolume * rt->tailBytes * SOU_MASTER_VOLUME / 780288, chan->pan, rt->previousFormat);

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

						driver->sndDrivMix(rt->dataStart + ringPosition, destOffset, chunkFrames, chunkBytes,
						                   (rt->gain * volume) >> 4, chan->pan, rt->format);

						rt->remaining -= chunkBytes;
						rt->readPosition += chunkBytes;
						framesLeft -= chunkFrames;
						destOffset += chunkFrames;
					}

					if (rt->remaining <= 0) {
						bool backToMix = false;

						while (framesLeft) {
							byte *op = rt->script;

							// The only script of a movie sets the whole stream up, then ends
							switch (op[0]) {
							case 1: {
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
										framesLeft = 0;
									}
								}
								break;
							}
							default:
								chan->state = 0;
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
}

} // End of namespace Grim
