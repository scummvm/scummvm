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
#include "common/mutex.h"
#include "common/serializer.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "engines/grim/imuse/dimuse_engine.h"
#include "engines/grim/imuse/dimuse_internalmixer.h"

namespace Grim {

IMuseDigiInternalMixer::IMuseDigiInternalMixer(Audio::Mixer *mixer, int sampleRate, bool lowLatencyMode) {
	_mixer = mixer;
	_sampleRate = sampleRate;
	_lowLatencyMode = lowLatencyMode;

	// Mark all separate streams for low latency mode as available...
	for (int i = 0; i < ARRAYSIZE(_separateStreams); i++) {
		_separateStreams[i] = nullptr;
	}

	if (!_lowLatencyMode)
		_stream = Audio::makeQueuingAudioStream(_sampleRate, _mixer->getOutputStereo());
	else
		_stream = nullptr;
}

IMuseDigiInternalMixer::~IMuseDigiInternalMixer() {
	terminate();
}

int IMuseDigiInternalMixer::terminate() {
	free(_softLTable);
	free(_mixBuf);
	_softLTable = nullptr;
	_mixBuf = nullptr;
	return 0;
}

int IMuseDigiInternalMixer::init(int numChannels, int mixBufSize, int mixChannelsNum) {
	// Pan table: 17 volume levels x 17 pan positions
	for (int i = 0; i < 17; i++) {
		for (int j = 0; j < 17; j++) {
			_mixerTable[17 * i + j] = (uint8)(int)((sin((double)j * 3.1415 * 0.03125) * (double)i + 0.5) * 8.0);
		}
	}

	_outChannelCount = numChannels;
	_mixBufSize = 2 * mixBufSize;
	_mixBuf = (int32 *)malloc(_mixBufSize);

	_softLTable = (int16 *)malloc(mixChannelsNum << 16);
	if (!_softLTable) {
		debug(5, "IMuseDigiInternalMixer::init(): ERROR: couldn't allocate mixer tables");
		return -1;
	}

	for (int i = 0; i < (mixChannelsNum << 15); i++) {
		_softLTable[i] = (int16)(int)(((double)i * 524272.0 / (262136.0 + (double)i * 7.0) + 1.0) * 0.5);
	}

	if (!_lowLatencyMode) {
		_mixer->playStream(
			Audio::Mixer::kPlainSoundType,
			&_channelHandle,
			_stream,
			-1,
			Audio::Mixer::kMaxChannelVolume);
	}

	return 0;
}

int IMuseDigiInternalMixer::clearMixerBuffer() {
	if (!_mixBuf)
		return -1;

	memset(_mixBuf, 0, _mixBufSize);

	return 0;
}

void IMuseDigiInternalMixer::mix16ToStereo(const int16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int leftAmp, int rightAmp) {
	int32 *mixBufCurCell = &_mixBuf[2 * mixBufStartIndex];
	uint32 ratio = ((uint32)inFrameCount << 16) / feedSize;
	uint32 position = 0;
	int32 interpolated;

	switch (ratio) {
	case 0x8000:
		for (int i = 0; i < inFrameCount - 1; i++) {
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

void IMuseDigiInternalMixer::mix16ToStereoSwap(const int16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int leftAmp, int rightAmp) {
	int32 *mixBufCurCell = &_mixBuf[2 * mixBufStartIndex];
	uint32 ratio = ((uint32)inFrameCount << 16) / feedSize;
	uint32 position = 0;
	int32 sample, interpolated;

	switch (ratio) {
	case 0x8000:
		for (int i = 0; i < inFrameCount - 1; i++) {
			sample = (int16)SWAP_BYTES_16(srcBuf[0]);
			interpolated = (sample + (int16)SWAP_BYTES_16(srcBuf[2])) >> 1;

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
			sample = (int16)SWAP_BYTES_16(srcBuf[0]);
			mixBufCurCell[0] += sample * leftAmp;
			mixBufCurCell[1] += sample * rightAmp;

			srcBuf++;
			mixBufCurCell += 2;
		}
		break;
	case 0x20000:
		for (uint32 i = 0; i < feedSize; i++) {
			sample = (int16)SWAP_BYTES_16(srcBuf[0]);
			mixBufCurCell[0] += sample * leftAmp;
			mixBufCurCell[1] += sample * rightAmp;

			srcBuf += 2;
			mixBufCurCell += 2;
		}
		break;
	default:
		for (uint32 i = 0; i < feedSize; i++) {
			sample = (int16)SWAP_BYTES_16(srcBuf[0]);
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

void IMuseDigiInternalMixer::mix16(const int16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int volume) {
	int32 *mixBufCurCell = &_mixBuf[2 * mixBufStartIndex];
	uint32 ratio = ((uint32)inFrameCount << 16) / feedSize;
	uint32 position = 0;

	switch (ratio) {
	case 0x8000:
		for (int i = 0; i < inFrameCount - 1; i++) {
			mixBufCurCell[0] += srcBuf[0] * volume;
			mixBufCurCell[1] += srcBuf[1] * volume;
			mixBufCurCell[2] += volume * ((srcBuf[0] + srcBuf[2]) >> 1);
			mixBufCurCell[3] += volume * ((srcBuf[1] + srcBuf[3]) >> 1);

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
			mixBufCurCell[i] += srcBuf[i] * volume;
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

void IMuseDigiInternalMixer::mix16Swap(const int16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int volume) {
	int32 *mixBufCurCell = &_mixBuf[2 * mixBufStartIndex];
	uint32 ratio = ((uint32)inFrameCount << 16) / feedSize;
	uint32 position = 0;
	int32 left, right;

	switch (ratio) {
	case 0x8000:
		for (int i = 0; i < inFrameCount - 1; i++) {
			left = (int16)SWAP_BYTES_16(srcBuf[0]);
			right = (int16)SWAP_BYTES_16(srcBuf[1]);

			mixBufCurCell[0] += left * volume;
			mixBufCurCell[1] += right * volume;
			mixBufCurCell[2] += volume * ((left + (int16)SWAP_BYTES_16(srcBuf[2])) >> 1);
			mixBufCurCell[3] += volume * ((right + (int16)SWAP_BYTES_16(srcBuf[3])) >> 1);

			srcBuf += 2;
			mixBufCurCell += 4;
		}

		left = (int16)SWAP_BYTES_16(srcBuf[0]);
		right = (int16)SWAP_BYTES_16(srcBuf[1]);
		mixBufCurCell[0] += left * volume;
		mixBufCurCell[1] += right * volume;
		mixBufCurCell[2] += left * volume;
		mixBufCurCell[3] += right * volume;
		break;
	case 0x10000:
		for (uint32 i = 0; i < 2 * feedSize; i++) {
			mixBufCurCell[i] += (int16)SWAP_BYTES_16(srcBuf[i]) * volume;
		}
		break;
	case 0x20000:
		for (uint32 i = 0; i < feedSize; i++) {
			mixBufCurCell[0] += (int16)SWAP_BYTES_16(srcBuf[0]) * volume;
			mixBufCurCell[1] += (int16)SWAP_BYTES_16(srcBuf[1]) * volume;

			srcBuf += 4;
			mixBufCurCell += 2;
		}
		break;
	default:
		for (uint32 i = 0; i < feedSize; i++) {
			mixBufCurCell[0] += (int16)SWAP_BYTES_16(srcBuf[0]) * volume;
			mixBufCurCell[1] += (int16)SWAP_BYTES_16(srcBuf[1]) * volume;

			mixBufCurCell += 2;
			position += 2 * ratio;
			srcBuf += position >> 16;
			position &= 0xFFFF;
		}
		break;
	}
}

void IMuseDigiInternalMixer::mix(uint8 *srcBuf, int32 inFrameCount, int wordSize, int channelCount, int swapEndiannessFlag, int feedSize, int32 mixBufStartIndex, int volume, int pan) {
	int channelVolume, channelPan, leftAmp, rightAmp;

	// The original could divide by zero here, let's avoid that...
	if (!_mixBuf || !srcBuf || !inFrameCount || wordSize != 16 || volume < 1 || feedSize < 1)
		return;

	if (channelCount == 1 && _outChannelCount == 2) {
		channelVolume = (volume >> 3) + 1;
		if (channelVolume >= 17)
			channelVolume = 16;

		channelPan = (pan >> 3) - 8;
		if (pan > 64)
			++channelPan;

		leftAmp = _mixerTable[17 * channelVolume + 8 - channelPan];
		rightAmp = _mixerTable[17 * channelVolume + 8 + channelPan];

		if (swapEndiannessFlag)
			mix16ToStereoSwap((const int16 *)srcBuf, inFrameCount, feedSize, mixBufStartIndex, leftAmp, rightAmp);
		else
			mix16ToStereo((const int16 *)srcBuf, inFrameCount, feedSize, mixBufStartIndex, leftAmp, rightAmp);
	} else if (swapEndiannessFlag) {
		mix16Swap((const int16 *)srcBuf, inFrameCount, feedSize, mixBufStartIndex, volume + 1);
	} else {
		mix16((const int16 *)srcBuf, inFrameCount, feedSize, mixBufStartIndex, volume + 1);
	}
}

int IMuseDigiInternalMixer::loop(uint8 **destBuffer, int len) {
	if (!_mixBuf || !destBuffer || !len)
		return -1;

	int16 *destBuffer_tmp = (int16 *)*destBuffer;
	int32 value;

	if (_outChannelCount == 2)
		len *= 2;

	for (int i = 0; i < len; i++) {
		value = _mixBuf[i] >> 7;
		if (value >= 0)
			destBuffer_tmp[i] = _softLTable[value];
		else
			destBuffer_tmp[i] = (int16)(-(uint16)_softLTable[-value - 1]);
	}

	return 0;
}

void IMuseDigiInternalMixer::endStream(int idx) {
	// We mark the stream as finished and then remove
	// the reference: in this way, the stream will have time
	// to flush the remaining sound data and will be disposed
	// by the mixer, while we freed up a slot for a new stream.
	if (_lowLatencyMode && idx != -1) {
		_separateStreams[idx]->finish();
		_separateStreams[idx] = nullptr;
	}
}

Audio::QueuingAudioStream *IMuseDigiInternalMixer::getStream(int idx) {
	if (!_lowLatencyMode || idx == -1) {
		// Stream for the original mode
		return _stream;
	} else if (!_separateStreams[idx]) {
		// New stream for a new sound
		_separateStreams[idx] = Audio::makeQueuingAudioStream(_sampleRate, _mixer->getOutputStereo());

		_mixer->playStream(
			Audio::Mixer::kPlainSoundType,
			&_separateChannelHandles[idx],
			_separateStreams[idx],
			-1,
			Audio::Mixer::kMaxChannelVolume);

		return _separateStreams[idx];
	} else {
		// On-going stream
		return _separateStreams[idx];
	}
}

} // End of namespace Grim
