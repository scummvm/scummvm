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

void SmushMixer::souMixerMix(const uint8 *srcBuf, int32 inFrameCount, int wordSize, int channelCount, uint32 feedSize, int32 mixBufStartIndex, int volume, int pan, int signedFlag, int swapEndiannessFlag) {
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

		if (wordSize == 8) {
			mixMono8(srcBuf, inFrameCount, feedSize, mixBufStartIndex, leftAmp, rightAmp);
		} else if (signedFlag) {
			if (swapEndiannessFlag)
				mixMono16Swap((const uint16 *)srcBuf, inFrameCount, feedSize, mixBufStartIndex, leftAmp, rightAmp);
			else
				mixMono16((const int16 *)srcBuf, inFrameCount, feedSize, mixBufStartIndex, leftAmp, rightAmp);
		} else {
			mixMono16Unsigned((const uint16 *)srcBuf, inFrameCount, feedSize, mixBufStartIndex, leftAmp, rightAmp);
		}
	} else if (wordSize == 8) {
		mixStereo8(srcBuf, inFrameCount, feedSize, mixBufStartIndex, volume + 1);
	} else if (signedFlag) {
		if (swapEndiannessFlag)
			mixStereo16Swap((const uint16 *)srcBuf, inFrameCount, feedSize, mixBufStartIndex, volume + 1);
		else
			mixStereo16((const int16 *)srcBuf, inFrameCount, feedSize, mixBufStartIndex, volume + 1);
	} else {
		mixStereo16Unsigned((const uint16 *)srcBuf, inFrameCount, feedSize, mixBufStartIndex, volume + 1);
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

void SmushMixer::mixStereo8(const uint8 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int volume) {
	int32 *mixBufCurCell = &_mixBuf[2 * mixBufStartIndex];
	uint32 ratio = ((uint32)inFrameCount << 16) / feedSize;
	uint32 position = 0;
	int32 left, right, interpolatedLeft, interpolatedRight;

	switch (ratio) {
	case 0x8000:
		for (int i = 0; i < inFrameCount - 1; i++) {
			left = (srcBuf[0] ^ 0x80) << 8;
			right = (srcBuf[1] ^ 0x80) << 8;
			interpolatedLeft = ((srcBuf[0] + srcBuf[2]) << 7) ^ 0x8000;
			interpolatedRight = ((srcBuf[1] + srcBuf[3]) << 7) ^ 0x8000;

			mixBufCurCell[0] += left * volume;
			mixBufCurCell[1] += right * volume;
			mixBufCurCell[2] += interpolatedLeft * volume;
			mixBufCurCell[3] += interpolatedRight * volume;

			srcBuf += 2;
			mixBufCurCell += 4;
		}

		left = (srcBuf[0] ^ 0x80) << 8;
		right = (srcBuf[1] ^ 0x80) << 8;
		mixBufCurCell[0] += left * volume;
		mixBufCurCell[1] += right * volume;
		mixBufCurCell[2] += left * volume;
		mixBufCurCell[3] += right * volume;
		break;
	case 0x10000:
		for (uint32 i = 0; i < 2 * feedSize; i++) {
			mixBufCurCell[0] += ((srcBuf[0] ^ 0x80) << 8) * volume;

			srcBuf++;
			mixBufCurCell++;
		}
		break;
	case 0x20000:
		for (uint32 i = 0; i < feedSize; i++) {
			mixBufCurCell[0] += ((srcBuf[0] ^ 0x80) << 8) * volume;
			mixBufCurCell[1] += ((srcBuf[1] ^ 0x80) << 8) * volume;

			srcBuf += 4;
			mixBufCurCell += 2;
		}
		break;
	default:
		for (uint32 i = 0; i < feedSize; i++) {
			mixBufCurCell[0] += ((srcBuf[0] ^ 0x80) << 8) * volume;
			mixBufCurCell[1] += ((srcBuf[1] ^ 0x80) << 8) * volume;

			mixBufCurCell += 2;
			position += 2 * ratio;
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

void SmushMixer::mixMono16Unsigned(const uint16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int leftAmp, int rightAmp) {
	int32 *mixBufCurCell = &_mixBuf[2 * mixBufStartIndex];
	uint32 ratio = ((uint32)inFrameCount << 16) / feedSize;
	uint32 position = 0;
	int32 sample, interpolated;

	switch (ratio) {
	case 0x8000:
		for (int i = 0; i < inFrameCount - 1; i++) {
			sample = (int16)(srcBuf[0] ^ 0x8000);
			// The disasm interpolates with the sample two positions ahead
			interpolated = (sample + (int16)(srcBuf[2] ^ 0x8000)) >> 1;

			mixBufCurCell[0] += sample * leftAmp;
			mixBufCurCell[1] += sample * rightAmp;
			mixBufCurCell[2] += interpolated * leftAmp;
			mixBufCurCell[3] += interpolated * rightAmp;

			srcBuf++;
			mixBufCurCell += 4;
		}

		sample = (int16)(srcBuf[0] ^ 0x8000);
		mixBufCurCell[0] += sample * leftAmp;
		mixBufCurCell[1] += sample * rightAmp;
		mixBufCurCell[2] += sample * leftAmp;
		mixBufCurCell[3] += sample * rightAmp;
		break;
	case 0x10000:
		for (uint32 i = 0; i < feedSize; i++) {
			sample = (int16)(srcBuf[0] ^ 0x8000);
			mixBufCurCell[0] += sample * leftAmp;
			mixBufCurCell[1] += sample * rightAmp;

			srcBuf++;
			mixBufCurCell += 2;
		}
		break;
	case 0x20000:
		for (uint32 i = 0; i < feedSize; i++) {
			sample = (int16)(srcBuf[0] ^ 0x8000);
			mixBufCurCell[0] += sample * leftAmp;
			mixBufCurCell[1] += sample * rightAmp;

			srcBuf += 2;
			mixBufCurCell += 2;
		}
		break;
	default:
		for (uint32 i = 0; i < feedSize; i++) {
			sample = (int16)(srcBuf[0] ^ 0x8000);
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

void SmushMixer::mixStereo16Unsigned(const uint16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int volume) {
	int32 *mixBufCurCell = &_mixBuf[2 * mixBufStartIndex];
	uint32 ratio = ((uint32)inFrameCount << 16) / feedSize;
	uint32 position = 0;
	int32 left, right;

	switch (ratio) {
	case 0x8000:
		for (int i = 0; i < inFrameCount - 1; i++) {
			left = (int16)(srcBuf[0] ^ 0x8000);
			right = (int16)(srcBuf[1] ^ 0x8000);

			mixBufCurCell[0] += left * volume;
			mixBufCurCell[1] += right * volume;
			mixBufCurCell[2] += ((left + (int16)(srcBuf[2] ^ 0x8000)) >> 1) * volume;
			mixBufCurCell[3] += ((right + (int16)(srcBuf[3] ^ 0x8000)) >> 1) * volume;

			srcBuf += 2;
			mixBufCurCell += 4;
		}

		left = (int16)(srcBuf[0] ^ 0x8000);
		right = (int16)(srcBuf[1] ^ 0x8000);
		mixBufCurCell[0] += left * volume;
		mixBufCurCell[1] += right * volume;
		mixBufCurCell[2] += left * volume;
		mixBufCurCell[3] += right * volume;
		break;
	case 0x10000:
		for (uint32 i = 0; i < 2 * feedSize; i++) {
			mixBufCurCell[0] += (int16)(srcBuf[0] ^ 0x8000) * volume;

			srcBuf++;
			mixBufCurCell++;
		}
		break;
	case 0x20000:
		for (uint32 i = 0; i < feedSize; i++) {
			mixBufCurCell[0] += (int16)(srcBuf[0] ^ 0x8000) * volume;
			mixBufCurCell[1] += (int16)(srcBuf[1] ^ 0x8000) * volume;

			srcBuf += 4;
			mixBufCurCell += 2;
		}
		break;
	default:
		for (uint32 i = 0; i < feedSize; i++) {
			mixBufCurCell[0] += (int16)(srcBuf[0] ^ 0x8000) * volume;
			mixBufCurCell[1] += (int16)(srcBuf[1] ^ 0x8000) * volume;

			mixBufCurCell += 2;
			position += 2 * ratio;
			srcBuf += position >> 16;
			position &= 0xFFFF;
		}
		break;
	}
}

void SmushMixer::mixMono16Swap(const uint16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int leftAmp, int rightAmp) {
	int32 *mixBufCurCell = &_mixBuf[2 * mixBufStartIndex];
	uint32 ratio = ((uint32)inFrameCount << 16) / feedSize;
	uint32 position = 0;
	int32 sample, interpolated;

	switch (ratio) {
	case 0x8000:
		for (int i = 0; i < inFrameCount - 1; i++) {
			sample = (uint16)SWAP_BYTES_16(srcBuf[0]);
			interpolated = (int16)SWAP_BYTES_16((uint16)(((int16)srcBuf[0] + (int16)srcBuf[2]) >> 1));

			mixBufCurCell[0] += sample * leftAmp;
			mixBufCurCell[1] += sample * rightAmp;
			mixBufCurCell[2] += interpolated * leftAmp;
			mixBufCurCell[3] += interpolated * rightAmp;

			srcBuf++;
			mixBufCurCell += 4;
		}

		sample = (int16)SWAP_BYTES_16(srcBuf[0]);
		mixBufCurCell[0] += sample * leftAmp;
		mixBufCurCell[1] += sample * rightAmp;
		mixBufCurCell[2] += sample * leftAmp;
		mixBufCurCell[3] += sample * rightAmp;
		break;
	case 0x10000:
		for (uint32 i = 0; i < feedSize; i++) {
			sample = (uint16)SWAP_BYTES_16(srcBuf[0]);
			mixBufCurCell[0] += sample * leftAmp;
			mixBufCurCell[1] += sample * rightAmp;

			srcBuf++;
			mixBufCurCell += 2;
		}
		break;
	case 0x20000:
		for (uint32 i = 0; i < feedSize; i++) {
			sample = (uint16)SWAP_BYTES_16(srcBuf[0]);
			mixBufCurCell[0] += sample * leftAmp;
			mixBufCurCell[1] += sample * rightAmp;

			srcBuf += 2;
			mixBufCurCell += 2;
		}
		break;
	default:
		for (uint32 i = 0; i < feedSize; i++) {
			sample = (uint16)SWAP_BYTES_16(srcBuf[0]);
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

void SmushMixer::mixStereo16Swap(const uint16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int volume) {
	int32 *mixBufCurCell = &_mixBuf[2 * mixBufStartIndex];
	uint32 ratio = ((uint32)inFrameCount << 16) / feedSize;
	uint32 position = 0;

	switch (ratio) {
	case 0x8000:
		for (int i = 0; i < inFrameCount - 1; i++) {
			mixBufCurCell[0] += volume * (uint16)SWAP_BYTES_16(srcBuf[0]);
			mixBufCurCell[1] += volume * (uint16)SWAP_BYTES_16(srcBuf[1]);
			mixBufCurCell[2] += volume * (uint16)SWAP_BYTES_16((uint16)(((int16)srcBuf[0] + (int16)srcBuf[2]) >> 1));
			mixBufCurCell[3] += volume * (uint16)SWAP_BYTES_16((uint16)(((int16)srcBuf[1] + (int16)srcBuf[3]) >> 1));

			srcBuf += 2;
			mixBufCurCell += 4;
		}

		mixBufCurCell[0] += volume * (uint16)SWAP_BYTES_16(srcBuf[0]);
		mixBufCurCell[1] += volume * (uint16)SWAP_BYTES_16(srcBuf[1]);
		mixBufCurCell[2] += volume * (uint16)SWAP_BYTES_16(srcBuf[0]);
		mixBufCurCell[3] += volume * (uint16)SWAP_BYTES_16(srcBuf[1]);
		break;
	case 0x10000:
		for (uint32 i = 0; i < 2 * feedSize; i++) {
			mixBufCurCell[0] += volume * (uint16)SWAP_BYTES_16(srcBuf[0]);

			srcBuf++;
			mixBufCurCell++;
		}
		break;
	case 0x20000:
		for (uint32 i = 0; i < feedSize; i++) {
			mixBufCurCell[0] += volume * (uint16)SWAP_BYTES_16(srcBuf[0]);
			mixBufCurCell[1] += volume * (uint16)SWAP_BYTES_16(srcBuf[1]);

			srcBuf += 4;
			mixBufCurCell += 2;
		}
		break;
	default:
		for (uint32 i = 0; i < feedSize; i++) {
			mixBufCurCell[0] += volume * (uint16)SWAP_BYTES_16(srcBuf[0]);
			mixBufCurCell[1] += volume * (uint16)SWAP_BYTES_16(srcBuf[1]);

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

SmushDriver::SmushDriver(SmushTimer *timer) {
	_timer = timer;
	_sound = nullptr;
	_system = g_system->getMixer();
	_ring = nullptr;
	_ringBlockCount = 0;
	_ringPlayPosition = 0;
	_lastBlock = -1;
	_driverRate = 0;
	_driverVolume = 0;
	_fillTimer = 0;
	memset(_volumeTable, 0, sizeof(_volumeTable));
	_volumeTableBuilt = false;
}

SmushDriver::~SmushDriver() {
	sndDrivTerminate();
}

int SmushDriver::sndDrivInitialize(int rate, int channelCount, SmushSound *sound) {
	if (rate < 22050)
		rate = 22050;

	if (sndDrivOpenRing())
		return -1;

	if (sndDrivMixerInitialize(4, 1024))
		return -1;

	_sound = sound;
	_driverRate = rate;
	_fillTimer = _timer->sysAlloc(sndDrivFillThreadCallback, 1193180 / (rate / 1024), SYS_TIMER_ONE_SHOT, this);

	return 0;
}

void SmushDriver::sndDrivTerminate() {
	if (_fillTimer)
		_timer->sysFree(_fillTimer);

	_fillTimer = 0;
	sndDrivMixerTerminate();
	sndDrivCloseRing();
}

int SmushDriver::sndDrivSetVol(int volume) {
	if (!_volumeTableBuilt) {
		_volumeTable[0] = -10000;
		_volumeTable[1] = -7000;
		_volumeTable[2] = -6000;
		_volumeTable[3] = -5200;
		_volumeTable[4] = -5000;
		_volumeTable[5] = -4500;
		_volumeTable[6] = -4200;
		_volumeTable[7] = -4100;
		_volumeTable[8] = -4000;

		int position = 16000;
		for (int i = 8; i < 32; i++) {
			int value = (int)(((uint64)2987803337LL * (uint64)(position - 16000)) >> 32) >> 4;
			position += 2000;
			_volumeTable[i] = value + (value >> 31) - 4000;
		}

		position = 32000;
		for (int i = 32; i < 64; i++) {
			int value = (int)(((uint64)2216757315LL * (uint64)(position - 32000)) >> 32) >> 4;
			position += 1000;
			_volumeTable[i] = value + (value >> 31) - 2000;
		}

		position = 64000;
		for (int i = 64; i < 128; i++) {
			int value = (int)(((uint64)2181570691LL * (uint64)(position - 64000)) >> 32) >> 5;
			position += 1000;
			_volumeTable[i] = value + (value >> 31) - 1000;
		}

		_volumeTableBuilt = true;
	}

	_driverVolume = volume & 0x7F;

	int centibels = _volumeTable[_driverVolume];
	int hostVolume = (int)(Audio::Mixer::kMaxChannelVolume * pow(10.0, centibels / 2000.0) + 0.5);
	_system->setChannelVolume(_soundHandle, hostVolume);

	return 0;
}

int SmushDriver::sndDrivGetVol() {
	return _driverVolume;
}

int SmushDriver::sndDrivGetRate() {
	return _driverRate;
}

int SmushDriver::sndDrivDummy(int rate) {
	return rate;
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

	_timer->sysReset(_fillTimer, 1193180 / (_driverRate / 1024));
}

void SmushDriver::sndDrivMix(const uint8 *srcBuf, int32 mixBufStartIndex, uint32 feedSize, int32 srcByteCount, int reserved, int volume, int pan, int format) {
	int channelCount, wordSize, signedFlag;

	switch (format) {
	case 0:
		channelCount = 1;
		wordSize = 8;
		signedFlag = 1;
		break;
	case 1:
		channelCount = 2;
		wordSize = 8;
		signedFlag = 1;
		break;
	case 2:
		channelCount = 1;
		wordSize = 16;
		signedFlag = 0;
		break;
	case 3:
		channelCount = 2;
		wordSize = 16;
		signedFlag = 0;
		break;
	case 4:
		channelCount = 1;
		wordSize = 16;
		signedFlag = 1;
		break;
	case 5:
		channelCount = 2;
		wordSize = 16;
		signedFlag = 1;
		break;
	default:
		channelCount = format;
		wordSize = format;
		signedFlag = format;
		break;
	}

	_mixer.souMixerMix(srcBuf, srcByteCount / (channelCount * (wordSize >> 3)), wordSize, channelCount, feedSize, mixBufStartIndex, volume, (pan + 127) / 2, signedFlag, 0);
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
	_bufferSize = 5000 * _rate * _blockAlign / 1000;

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
	_threshold = 1000 * _rate * _blockAlign / 1000;
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
			if (_written < played + 500 * _rate * _blockAlign / 1000) {
				Common::StackLock lock(_mutex);
				_playing = false;
				_waiting = 1;
				_threshold = played + 500 * _rate * _blockAlign / 1000;
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
	int available = audio->produced - audio->consumed - 1024;
	int space = playedAbsolute - _written + ringSize - 4096;
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
