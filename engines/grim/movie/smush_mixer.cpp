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
#include "common/system.h"
#include "common/util.h"

#include "engines/grim/movie/smush_mixer.h"
#include "engines/grim/movie/smush_sound.h"
#include "engines/grim/movie/smush_timer.h"

namespace Grim {

SmushMixer::SmushMixer() {
	_mixBuf = nullptr;
	_mixBufSize = 0;
	_outChannelCount = 0;
	memset(_mixerTable, 0, sizeof(_mixerTable));
}

SmushMixer::~SmushMixer() {
	souMixerTerminate();
}

int SmushMixer::souMixerInitialize(int outChannelCount, int mixBufSize) {
	for (int i = 0; i < 17; i++) {
		for (int j = 0; j < 17; j++)
			_mixerTable[17 * i + j] = (uint8)(int)((sin((double)j * 3.1415 * 0.03125) * (double)i + 0.5) * 8.0);
	}

	_outChannelCount = outChannelCount;
	_mixBufSize = 4 * mixBufSize;
	_mixBuf = (int32 *)malloc(_mixBufSize);
	if (!_mixBuf)
		return -1;

	return 0;
}

int SmushMixer::souMixerTerminate() {
	free(_mixBuf);
	_mixBuf = nullptr;
	return 0;
}

int SmushMixer::souMixerStart() {
	memset(_mixBuf, 0, _mixBufSize);
	return 0;
}

void SmushMixer::souMixerMix(const uint8 *srcBuf, int32 inFrameCount, int wordSize, int channelCount, uint32 feedSize, int32 mixBufStartIndex, int volume, int pan) {
	// The original could divide by zero here, let's avoid that...
	if (!_mixBuf || !srcBuf || !inFrameCount || !feedSize)
		return;

	if (channelCount == 1) {
		uint32 volumeIndex = volume >> 3;
		if (volume)
			volumeIndex++;
		if (volumeIndex >= 17)
			volumeIndex = 16;

		int panOffset = (pan >> 3) - 8;
		if (pan > 64)
			panOffset++;

		int leftAmp = _mixerTable[17 * volumeIndex + 8 - panOffset];
		int rightAmp = _mixerTable[17 * volumeIndex + 8 + panOffset];

		if (wordSize == 8)
			mixMono8(srcBuf, inFrameCount, feedSize, mixBufStartIndex, leftAmp, rightAmp);
		else
			mixMono16((const int16 *)srcBuf, inFrameCount, feedSize, mixBufStartIndex, leftAmp, rightAmp);
	} else {
		mixStereo16((const int16 *)srcBuf, inFrameCount, feedSize, mixBufStartIndex, volume + 1);
	}
}

int SmushMixer::souMixerEnd(int16 *destBuf, int frameCount) {
	if (!_mixBuf || !destBuf || !frameCount)
		return -1;

	int sampleCount = frameCount;
	if (_outChannelCount == 2)
		sampleCount = 2 * frameCount;

	for (int i = 0; i < sampleCount; i++) {
		int32 sample = _mixBuf[i] >> 7;
		if (sample < -32768)
			sample = -32768;
		if (sample > 32767)
			sample = 32767;

		destBuf[i] = (int16)sample;
	}

	return 0;
}

void SmushMixer::mixMono8(const uint8 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int leftAmp, int rightAmp) {
	int32 *mixBufCurCell = &_mixBuf[2 * mixBufStartIndex];
	uint32 ratio = ((uint32)inFrameCount << 16) / feedSize;
	uint32 position = 0;
	int32 sample, interpolated;

	switch (ratio) {
	case 0x8000:
		for (int i = 0; i < inFrameCount - 1; i++) {
			sample = (srcBuf[0] ^ 0x80) << 8;
			// The disasm interpolates with the sample two positions ahead, without sign extension
			interpolated = ((srcBuf[0] + srcBuf[2]) << 7) ^ 0x8000;

			mixBufCurCell[0] += sample * leftAmp;
			mixBufCurCell[1] += sample * rightAmp;
			mixBufCurCell[2] += interpolated * leftAmp;
			mixBufCurCell[3] += interpolated * rightAmp;

			srcBuf++;
			mixBufCurCell += 4;
		}

		sample = (srcBuf[0] ^ 0x80) << 8;
		mixBufCurCell[0] += sample * leftAmp;
		mixBufCurCell[1] += sample * rightAmp;
		mixBufCurCell[2] += sample * leftAmp;
		mixBufCurCell[3] += sample * rightAmp;
		break;
	case 0x10000:
		for (uint32 i = 0; i < feedSize; i++) {
			sample = (srcBuf[0] ^ 0x80) << 8;
			mixBufCurCell[0] += sample * leftAmp;
			mixBufCurCell[1] += sample * rightAmp;

			srcBuf++;
			mixBufCurCell += 2;
		}
		break;
	case 0x20000:
		for (uint32 i = 0; i < feedSize; i++) {
			sample = (srcBuf[0] ^ 0x80) << 8;
			mixBufCurCell[0] += sample * leftAmp;
			mixBufCurCell[1] += sample * rightAmp;

			srcBuf += 2;
			mixBufCurCell += 2;
		}
		break;
	default:
		for (uint32 i = 0; i < feedSize; i++) {
			sample = (srcBuf[0] ^ 0x80) << 8;
			mixBufCurCell[0] += sample * leftAmp;
			mixBufCurCell[1] += sample * rightAmp;

			mixBufCurCell += 2;
			position += ratio;
			srcBuf += position >> 16;
			position &= 0xFFFF;
		}
		break;
	}
}

void SmushMixer::mixMono16(const int16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int leftAmp, int rightAmp) {
	int32 *mixBufCurCell = &_mixBuf[2 * mixBufStartIndex];
	uint32 ratio = ((uint32)inFrameCount << 16) / feedSize;
	uint32 position = 0;
	int32 interpolated;

	switch (ratio) {
	case 0x8000:
		for (int i = 0; i < inFrameCount - 1; i++) {
			// The disasm interpolates with the sample two positions ahead
			interpolated = (srcBuf[0] + srcBuf[2]) >> 1;

			mixBufCurCell[0] += srcBuf[0] * leftAmp;
			mixBufCurCell[1] += srcBuf[0] * rightAmp;
			mixBufCurCell[2] += interpolated * leftAmp;
			mixBufCurCell[3] += interpolated * rightAmp;

			srcBuf++;
			mixBufCurCell += 4;
		}

		mixBufCurCell[0] += srcBuf[0] * leftAmp;
		mixBufCurCell[1] += srcBuf[0] * rightAmp;
		mixBufCurCell[2] += srcBuf[0] * leftAmp;
		mixBufCurCell[3] += srcBuf[0] * rightAmp;
		break;
	case 0x10000:
		for (uint32 i = 0; i < feedSize; i++) {
			mixBufCurCell[0] += srcBuf[0] * leftAmp;
			mixBufCurCell[1] += srcBuf[0] * rightAmp;

			srcBuf++;
			mixBufCurCell += 2;
		}
		break;
	case 0x20000:
		for (uint32 i = 0; i < feedSize; i++) {
			mixBufCurCell[0] += srcBuf[0] * leftAmp;
			mixBufCurCell[1] += srcBuf[0] * rightAmp;

			srcBuf += 2;
			mixBufCurCell += 2;
		}
		break;
	default:
		for (uint32 i = 0; i < feedSize; i++) {
			mixBufCurCell[0] += srcBuf[0] * leftAmp;
			mixBufCurCell[1] += srcBuf[0] * rightAmp;

			mixBufCurCell += 2;
			position += ratio;
			srcBuf += position >> 16;
			position &= 0xFFFF;
		}
		break;
	}
}

void SmushMixer::mixStereo16(const int16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int volume) {
	int32 *mixBufCurCell = &_mixBuf[2 * mixBufStartIndex];
	uint32 ratio = ((uint32)inFrameCount << 16) / feedSize;
	uint32 position = 0;

	switch (ratio) {
	case 0x8000:
		for (int i = 0; i < inFrameCount - 1; i++) {
			mixBufCurCell[0] += srcBuf[0] * volume;
			mixBufCurCell[1] += srcBuf[1] * volume;
			mixBufCurCell[2] += ((srcBuf[0] + srcBuf[2]) >> 1) * volume;
			mixBufCurCell[3] += ((srcBuf[1] + srcBuf[3]) >> 1) * volume;

			srcBuf += 2;
			mixBufCurCell += 4;
		}

		mixBufCurCell[0] += srcBuf[0] * volume;
		mixBufCurCell[1] += srcBuf[1] * volume;
		mixBufCurCell[2] += srcBuf[0] * volume;
		mixBufCurCell[3] += srcBuf[1] * volume;
		break;
	case 0x10000:
		for (uint32 i = 0; i < 2 * feedSize; i++) {
			mixBufCurCell[0] += srcBuf[0] * volume;

			srcBuf++;
			mixBufCurCell++;
		}
		break;
	case 0x20000:
		for (uint32 i = 0; i < feedSize; i++) {
			mixBufCurCell[0] += srcBuf[0] * volume;
			mixBufCurCell[1] += srcBuf[1] * volume;

			srcBuf += 4;
			mixBufCurCell += 2;
		}
		break;
	default:
		for (uint32 i = 0; i < feedSize; i++) {
			mixBufCurCell[0] += srcBuf[0] * volume;
			mixBufCurCell[1] += srcBuf[1] * volume;

			mixBufCurCell += 2;
			position += 2 * ratio;
			srcBuf += position >> 16;
			position &= 0xFFFF;
		}
		break;
	}
}

SmushDriverStream::SmushDriverStream(SmushDriver *driver) {
	_driver = driver;
}

int SmushDriverStream::readBuffer(int16 *buffer, const int numSamples) {
	return _driver->ringRead(buffer, numSamples);
}

int SmushDriverStream::getRate() const {
	return _driver->sndDrivGetRate();
}

SmushDriver::SmushDriver(SmushTimer *timer, bool remastered) {
	_timer = timer;
	_remastered = remastered;
	_sound = nullptr;
	_system = g_system->getMixer();
	_ring = nullptr;
	_ringBlockCount = 0;
	_ringPlayPosition = 0;
	_lastBlock = -1;
	_driverRate = 0;
	_fillTimer = 0;
}

SmushDriver::~SmushDriver() {
	sndDrivTerminate();
}

int SmushDriver::sndDrivInitialize(int rate, int channelCount, SmushSound *sound) {
	// The disasm of the remastered ignores the requested rate and always runs at 24000
	if (_remastered)
		rate = 24000;
	else if (rate < 22050)
		rate = 22050;

	// The stream reads the rate as soon as it is opened
	_driverRate = rate;

	if (sndDrivOpenRing())
		return -1;

	if (sndDrivMixerInitialize(4, 1024))
		return -1;

	_sound = sound;
	_fillTimer = _timer->sysAlloc(sndDrivFillThreadCallback, SMUSH_TIMER_FREQUENCY / (rate / 1024), SYS_TIMER_ONE_SHOT, this);

	return 0;
}

void SmushDriver::sndDrivTerminate() {
	if (_fillTimer)
		_timer->sysFree(_fillTimer);

	_fillTimer = 0;
	sndDrivMixerTerminate();
	sndDrivCloseRing();
}

int SmushDriver::sndDrivGetRate() {
	return _driverRate;
}

void SmushDriver::sndDrivFillThread() {
	int playBlock = sndDrivGetPlayBlock();
	if (playBlock != -1) {
		int lastBlock = _lastBlock;
		if (playBlock != (lastBlock + 1) % _ringBlockCount) {
			if (lastBlock < 0)
				lastBlock = playBlock;

			_lastBlock = (lastBlock + 1) % _ringBlockCount;

			int16 blockBuf[SMUSH_RING_BLOCK_SIZE / 2];
			int frameCount = SMUSH_RING_BLOCK_SIZE / 4;

			_mixer.souMixerStart();
			_sound->souEngineFill(frameCount, this);
			sndDrivMixerEnd(blockBuf, frameCount);

			Common::StackLock lock(_ringMutex);
			memcpy(_ring + _lastBlock * SMUSH_RING_BLOCK_SIZE, blockBuf, SMUSH_RING_BLOCK_SIZE);
		}
	}

	_timer->sysReset(_fillTimer, SMUSH_TIMER_FREQUENCY / (_driverRate / 1024));
}

void SmushDriver::sndDrivMix(const uint8 *srcBuf, int32 mixBufStartIndex, uint32 feedSize, int32 srcByteCount, int volume, int pan, int format) {
	int channelCount;
	int wordSize;

	// The disasm mixes the tail of a first stream, which has no format yet, as 8 bit mono
	if (format == 0) {
		channelCount = 1;
		wordSize = 8;
	} else {
		// Movies only have signed 16 bit audio: format 4 is mono, format 5 is stereo
		channelCount = (format == 5) ? 2 : 1;
		wordSize = 16;
	}

	_mixer.souMixerMix(srcBuf, srcByteCount / (channelCount * (wordSize >> 3)), wordSize, channelCount, feedSize, mixBufStartIndex, volume, (pan + 127) / 2);
}

void SmushDriver::sndDrivFillThreadCallback(int timer, void *context) {
	((SmushDriver *)context)->sndDrivFillThread();
}

int SmushDriver::sndDrivOpenRing() {
	_ringBlockCount = 3;
	_ringPlayPosition = 0;
	_ring = (byte *)calloc(_ringBlockCount * SMUSH_RING_BLOCK_SIZE, 1);
	if (!_ring)
		return -1;

	_system->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, new SmushDriverStream(this), -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
	return 0;
}

void SmushDriver::sndDrivCloseRing() {
	_system->stopHandle(_soundHandle);

	Common::StackLock lock(_ringMutex);
	free(_ring);
	_ring = nullptr;
}

int SmushDriver::sndDrivGetPlayBlock() {
	Common::StackLock lock(_ringMutex);
	if (!_ring)
		return -1;

	return _ringPlayPosition / SMUSH_RING_BLOCK_SIZE % _ringBlockCount;
}

int SmushDriver::sndDrivMixerInitialize(int arg, int size) {
	return _mixer.souMixerInitialize(2, 4 * size);
}

void SmushDriver::sndDrivMixerTerminate() {
	_mixer.souMixerTerminate();
}

int SmushDriver::sndDrivMixerEnd(int16 *destBuf, int frameCount) {
	return _mixer.souMixerEnd(destBuf, frameCount);
}

int SmushDriver::ringRead(int16 *buffer, int numSamples) {
	Common::StackLock lock(_ringMutex);
	if (!_ring) {
		memset(buffer, 0, numSamples * 2);
		return numSamples;
	}

	int ringSize = _ringBlockCount * SMUSH_RING_BLOCK_SIZE;
	byte *dest = (byte *)buffer;
	int remaining = numSamples * 2;

	while (remaining > 0) {
		int chunk = MIN(remaining, ringSize - _ringPlayPosition);
		memcpy(dest, _ring + _ringPlayPosition, chunk);

		dest += chunk;
		remaining -= chunk;
		_ringPlayPosition = (_ringPlayPosition + chunk) % ringSize;
	}

	return numSamples;
}

SmushDemoDriverStream::SmushDemoDriverStream(SmushDemoDriver *driver) {
	_driver = driver;
}

int SmushDemoDriverStream::readBuffer(int16 *buffer, const int numSamples) {
	return _driver->ringRead(buffer, numSamples);
}

SmushDemoDriver::SmushDemoDriver() {
	_system = g_system->getMixer();
	_buffer = nullptr;
	_bufferSize = 0;
	_playPosition = 0;
	_playing = false;
	_created = false;
	_rate = 0;
	_blockAlign = 0;
	_written = 0;
	_wrapCount = 0;
	_lastPosition = 0;
	_lastTime = 0;
	_lastMeasure = 0;
	_lastReturned = 0;
	_waiting = 0;
	_paused = 0;
	_threshold = 0;
	_msPerByte = 0.0;
}

SmushDemoDriver::~SmushDemoDriver() {
	_system->stopHandle(_soundHandle);

	Common::StackLock lock(_mutex);
	delete[] _buffer;
	_buffer = nullptr;
}

void SmushDemoDriver::sndDrivCreate() {
	_created = true;
}

void SmushDemoDriver::sndDrivRelease() {
	_created = false;
}

int SmushDemoDriver::sndDrivOpen(int rate, int channels, int bits, SmushDemoAudio *audio) {
	sndDrivCreate();

	_rate = rate;
	audio->data = new byte[audio->size]();

	_blockAlign = (bits >> 3) * channels;
	_bufferSize = SMUSH_DEMO_DRIVER_BUFFER_MS * _rate * _blockAlign / 1000;

	{
		Common::StackLock lock(_mutex);
		_buffer = new byte[_bufferSize]();
		_playPosition = 0;
		_playing = false;
	}

	_system->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, new SmushDemoDriverStream(this), -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
	audio->opened = true;

	_written = 0;
	_wrapCount = 0;
	_lastPosition = 0;
	_lastMeasure = g_system->getMillis();
	_lastTime = 0;
	_lastReturned = 0;
	_waiting = 1;
	_paused = 0;
	_msPerByte = 1000.0 / (double)(_rate * _blockAlign);
	_threshold = SMUSH_DEMO_DRIVER_START_MS * _rate * _blockAlign / 1000;
	return 0;
}

void SmushDemoDriver::sndDrivClose(SmushDemoAudio *audio) {
	if (!audio->opened)
		return;

	_system->stopHandle(_soundHandle);
	{
		Common::StackLock lock(_mutex);
		_playing = false;
		delete[] _buffer;
		_buffer = nullptr;
	}

	audio->opened = false;
	if (audio->data) {
		delete[] audio->data;
		audio->data = nullptr;
	}

	sndDrivRelease();
}

int SmushDemoDriver::sndDrivGetTime(SmushDemoAudio *audio) {
	int position;
	{
		Common::StackLock lock(_mutex);
		position = _playPosition;
	}

	uint32 time = g_system->getMillis();
	int result;

	if (position != _lastPosition) {
		if (position < _lastPosition)
			_wrapCount++;

		int played = position + _wrapCount * _bufferSize;
		_lastTime = (int)((double)played * _msPerByte);
		_lastMeasure = time;
		_lastPosition = position;

		if (!_waiting) {
			if (_written < played + SMUSH_DEMO_DRIVER_LOW_MS * _rate * _blockAlign / 1000) {
				Common::StackLock lock(_mutex);
				_playing = false;
				_waiting = 1;
				_threshold = played + SMUSH_DEMO_DRIVER_LOW_MS * _rate * _blockAlign / 1000;
			}
		}
	}

	if (_waiting || _paused)
		result = _lastTime;
	else
		result = _lastTime + (int)(time - _lastMeasure);

	if (result <= _lastReturned)
		result = _lastReturned;

	_lastReturned = result;
	return result;
}

int SmushDemoDriver::sndDrivPause(SmushDemoAudio *audio) {
	{
		Common::StackLock lock(_mutex);
		_playing = false;
	}

	_paused = 1;
	return 0;
}

int SmushDemoDriver::sndDrivResume(SmushDemoAudio *audio) {
	_paused = 0;
	_lastMeasure = g_system->getMillis();

	Common::StackLock lock(_mutex);
	_playing = true;
	_playPosition = _lastPosition;
	return 0;
}

void SmushDemoDriver::sndDrivFill(SmushDemoAudio *audio) {
	sndDrivGetTime(audio);

	int ringSize = _bufferSize;
	int playedAbsolute = _lastPosition + ringSize * _wrapCount;
	// The disasm never reads the last 1024 bytes of the produced audio
	// and keeps a block of the ring free
	int available = audio->produced - audio->consumed - 1024;
	int space = playedAbsolute - _written + ringSize - SMUSH_DEMO_AUDIO_BLOCK_SIZE;
	if (space >= available)
		space = available;

	if (space <= 0)
		return;

	int offset = _written % ringSize;
	int firstLength = MIN(space, ringSize - offset);
	int secondLength = space - firstLength;
	int remaining = space;
	int segmentRemaining = firstLength;
	int destination = offset;

	{
		Common::StackLock lock(_mutex);

		while (remaining > 0) {
			int source = audio->consumed % audio->size;
			int toEnd = audio->size - source;
			int chunk = MIN(remaining, toEnd);
			chunk = MIN(chunk, segmentRemaining);

			memcpy(_buffer + destination, audio->data + source, chunk);
			remaining -= chunk;
			segmentRemaining -= chunk;
			audio->consumed += chunk;

			if (segmentRemaining == 0) {
				segmentRemaining = secondLength;
				destination = 0;
			} else {
				destination += chunk;
			}
		}
	}

	_written += firstLength + secondLength;

	if (_waiting) {
		if (_written > _threshold) {
			{
				Common::StackLock lock(_mutex);
				_playing = true;
				_playPosition = _lastPosition;
			}

			_waiting = 0;
			_lastMeasure = g_system->getMillis();
		}
	}
}

int SmushDemoDriver::ringRead(int16 *buffer, int numSamples) {
	Common::StackLock lock(_mutex);
	if (!_buffer || !_playing) {
		memset(buffer, 0, numSamples * 2);
		return numSamples;
	}

	byte *dest = (byte *)buffer;
	int remaining = numSamples * 2;

	while (remaining > 0) {
		int chunk = MIN(remaining, _bufferSize - _playPosition);
		memcpy(dest, _buffer + _playPosition, chunk);

		dest += chunk;
		remaining -= chunk;
		_playPosition = (_playPosition + chunk) % _bufferSize;
	}

	return numSamples;
}

} // End of namespace Grim
