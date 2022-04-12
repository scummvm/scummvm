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

#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/imuse_digi/dimuse_internalmixer.h"

namespace Scumm {

IMuseDigiInternalMixer::IMuseDigiInternalMixer(Audio::Mixer *mixer, bool isEarlyDiMUSE) {
	_stream = Audio::makeQueuingAudioStream(DIMUSE_SAMPLERATE, true);
	_mixer = mixer;
	_isEarlyDiMUSE = isEarlyDiMUSE;
	_radioChatter = 0;
	_amp8Table = nullptr;
}

IMuseDigiInternalMixer::~IMuseDigiInternalMixer() {
	free(_amp8Table);
	_amp8Table = nullptr;
}

// Lookup table for a linear volume ramp (0 to 16) accounting for panning (-8 to 8)
static const int8 _stereoVolumeTable[284] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	0,  0,  0,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  1,  1,  1,  1,  2,  2,  2,
	2,  2,  3,  3,  3,  3,  3,  3,  0,  0,  1,  1,  2,  2,  2,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  0,
	0,  1,  1,  2,  2,  3,  3,  4,  4,  4,  4,  5,  5,  5,  5,  5,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,
	5,  5,  6,  6,  6,  6,  6,  0,  1,  1,  2,  3,  3,  4,  4,  5,  5,  6,  6,  6,  7,  7,  7,  7,  0,  1,
	2,  2,  3,  4,  4,  5,  6,  6,  7,  7,  7,  8,  8,  8,  8,  0,  1,  2,  3,  3,  4,  5,  6,  6,  7,  7,
	8,  8,  9,  9,  9,  9,  0,  1,  2,  3,  4,  5,  6,  6,  7,  8,  8,  9,  9,  10, 10, 10, 10, 0,  1,  2,
	3,  4,  5,  6,  7,  8,  9,  9,  10, 10, 11, 11, 11, 11, 0,  1,  2,  3,  5,  6,  7,  8,  8,  9,  10, 11,
	11, 11, 12, 12, 12, 0,  1,  3,  4,  5,  6,  7,  8,  9,  10, 11, 11, 12, 12, 13, 13, 13, 0,  1,  3,  4,
	5,  7,  8,  9,  10, 11, 12, 12, 13, 13, 14, 14, 14, 0,  1,  3,  4,  6,  7,  8,  10, 11, 12, 12, 13, 14,
	14, 15, 15, 15, 0,  2,  3,  5,  6,  8,  9,  10, 11, 12, 13, 14, 15, 15, 16, 16, 16, 0,  0,  0
};

int IMuseDigiInternalMixer::init(int bytesPerSample, int numChannels, uint8 *mixBuf, int mixBufSize, int sizeSampleKB, int mixChannelsNum) {
	int amplitudeValue;
	int waveMixChannelsCount;
	int softLdenominator;
	int softLnumerator;
	int softLcurValue;
	int zeroCenterOffset;

	_outWordSize = bytesPerSample;
	_outChannelCount = numChannels;
	_mixBufSize = mixBufSize;
	_stereoReverseFlag = sizeSampleKB;
	_mixBuf = mixBuf;

	waveMixChannelsCount = mixChannelsNum;

	// We're allocating:
	// - The amplitude quantization table for 8-bit audio ((17 * 256) int16 == 2176 int32)
	// - The amplitude quantization table for 12-bit/16-bit audio ((17 * 4096) int16 == 34816 int32)
	// - The pseudologarithmic curve for volume fade ((8 * 4096) int16 == 16384 int32)
	//
	// For the first two tables 17 represents the volume range (0-16), while
	// 256 and 4096 are respectively the dynamic range for 8-bit and 12-bit audio;
	//
	// Every table is initialized with int16 values, but stored as int32 and manipulated
	// at different levels of granularity depending on the case (as uint32, uint16 or uint8).
	// The latter is populated from the middle, inside-out; hence _softLMID.

	int32 *tableMalloc = (int32 *)malloc(53376 * sizeof(int32));

	_amp8Table = tableMalloc;          // Dim: 2176
	_amp12Table = tableMalloc + 2176;  // Dim: 34816
	_softLTable = tableMalloc + 36992; // Dim: 16384
	_softLMID = tableMalloc + 45184;   // Mid table

	if (tableMalloc) {
		// Any unpopulated cell is expected to be set to 0
		memset(tableMalloc, 0, 53376 * sizeof(int32));
		zeroCenterOffset = 0;
		for (int i = 0; i < 17; i++) {
			amplitudeValue = -2048 * zeroCenterOffset;
			for (int j = 0; j < 256; j++) {
				((int16 *)&_amp8Table[i * 128])[j] = (int16)(amplitudeValue / 127);
				amplitudeValue += 16 * zeroCenterOffset;
			}

			zeroCenterOffset += 8;
			if (zeroCenterOffset == 8)
				zeroCenterOffset = 7;
		}

		zeroCenterOffset = 0;
		for (int i = 0; i < 17; i++) {
			amplitudeValue = -2048 * zeroCenterOffset;
			for (int j = 0; j < 4096; j++) {
				((int16 *)&_amp12Table[i * 2048])[j] = (int16)(amplitudeValue / 127);
				amplitudeValue += zeroCenterOffset;
			}

			zeroCenterOffset += 8;
			if (zeroCenterOffset == 8)
				zeroCenterOffset = 7;
		}

		if (_outWordSize == 8) {
			if (waveMixChannelsCount * 1024 > 0) {
				softLnumerator = 0;
				softLdenominator = 2047 * waveMixChannelsCount;
				for (int i = 0; i < waveMixChannelsCount * 2048; i++) {
					softLcurValue = softLnumerator / softLdenominator + 1;
					softLdenominator += waveMixChannelsCount - 1;
					softLnumerator += 254 * waveMixChannelsCount;
					softLcurValue /= 2;

					((int8 *)_softLMID)[i] = (int8)softLcurValue + 128;
					((int8 *)_softLMID)[-i] = 127 - (int8)softLcurValue;
				}
			}
		} else if (waveMixChannelsCount * 2048 > 0) {
			softLdenominator = 2047 * waveMixChannelsCount;
			softLnumerator = 0;
			for (int i = 0; i < waveMixChannelsCount * 2048; i++) {
				softLcurValue = softLnumerator / softLdenominator + 1;
				softLcurValue /= 2;
				softLdenominator += waveMixChannelsCount - 1;
				softLnumerator += 65534 * waveMixChannelsCount;
				((int16 *)_softLMID)[i] = (int16)softLcurValue;
				((int16 *)_softLMID)[-i - 1] = -1 - (int16)softLcurValue;
			}
		}
		_mixer->playStream(Audio::Mixer::kPlainSoundType, &_channelHandle, _stream, -1, Audio::Mixer::kMaxChannelVolume, false);
		return 0;
	} else {
		debug(5, "DiMUSE_InternalMixer::init(): ERROR: couldn't allocate mixer tables");
		return -1;
	}
}

void IMuseDigiInternalMixer::setRadioChatter() {
	_radioChatter = 1;
}
void IMuseDigiInternalMixer::clearRadioChatter() {
	_radioChatter = 0;
}

int IMuseDigiInternalMixer::clearMixerBuffer() {
	if (!_mixBuf)
		return -1;

	memset(_mixBuf, 0, _mixBufSize);

	return 0;
}

void IMuseDigiInternalMixer::mix(uint8 *srcBuf, int32 inFrameCount, int wordSize, int channelCount, int feedSize, int32 mixBufStartIndex, int volume, int pan, bool ftIs11025Hz) {
	int32 *ampTable;
	int rightChannelVolume;
	int leftChannelVolume;
	int channelVolume;
	int channelPan;

	if (_mixBuf) {
		if (srcBuf) {
			if (inFrameCount) {
				if (channelCount == 1 && _outChannelCount == 2) {
					channelVolume = volume / 8;
					if (volume)
						++channelVolume;
					if (channelVolume >= 17)
						channelVolume = 16;
					channelPan = (pan / 8) - 8;
					if (pan > 64)
						++channelPan;

					channelPan = channelVolume == 0 ? 0 : channelPan;

					// Linear volume quantization from the lookup table
					rightChannelVolume = _stereoVolumeTable[17 * channelVolume + channelPan];
					leftChannelVolume = _stereoVolumeTable[17 * channelVolume - channelPan];
					if (wordSize == 8) {
						mixBits8ConvertToStereo(
							srcBuf,
							inFrameCount,
							feedSize,
							mixBufStartIndex,
							&_amp8Table[leftChannelVolume * 128],
							&_amp8Table[rightChannelVolume * 128],
							ftIs11025Hz);
					} else if (wordSize == 12) {
						mixBits12ConvertToStereo(
							srcBuf,
							inFrameCount,
							feedSize,
							mixBufStartIndex,
							&_amp12Table[leftChannelVolume * 2048],
							&_amp12Table[rightChannelVolume * 2048]);
					} else {
						mixBits16ConvertToStereo(
							srcBuf,
							inFrameCount,
							feedSize,
							mixBufStartIndex,
							&_amp12Table[leftChannelVolume * 2048],
							&_amp12Table[rightChannelVolume * 2048]);
					}
				} else {
					channelVolume = volume / 8;
					if (volume)
						channelVolume++;
					if (channelVolume >= 17)
						channelVolume = 16;

					if (wordSize == 8)
						ampTable = &_amp8Table[channelVolume * 128];
					else
						ampTable = &_amp12Table[channelVolume * 2048];

					if (_outChannelCount == 1) {
						if (channelCount == 1) {
							if (wordSize == 8) {
								mixBits8Mono(srcBuf, inFrameCount, feedSize, mixBufStartIndex, ampTable, ftIs11025Hz);
							} else if (wordSize == 12) {
								mixBits12Mono(srcBuf, inFrameCount, feedSize, mixBufStartIndex, ampTable);
							} else {
								mixBits16Mono(srcBuf, inFrameCount, feedSize, mixBufStartIndex, ampTable);
							}
						} else if (wordSize == 8) {
							mixBits8ConvertToMono(srcBuf, inFrameCount, feedSize, mixBufStartIndex, ampTable);
						} else if (wordSize == 12) {
							mixBits12ConvertToMono(srcBuf, inFrameCount, feedSize, mixBufStartIndex, ampTable);
						} else {
							mixBits16ConvertToMono(srcBuf, inFrameCount, feedSize, mixBufStartIndex, ampTable);
						}
					} else if (wordSize == 8) {
						mixBits8Stereo(srcBuf, inFrameCount, feedSize, mixBufStartIndex, ampTable);
					} else if (wordSize == 12) {
						mixBits12Stereo(srcBuf, inFrameCount, feedSize, mixBufStartIndex, ampTable);
					} else {
						mixBits16Stereo(srcBuf, inFrameCount, feedSize, mixBufStartIndex, ampTable);
					}
				}
			}
		}
	}
}

int IMuseDigiInternalMixer::loop(uint8 **destBuffer, int len) {
	int16 *mixBuffer = (int16 *)_mixBuf;
	uint8 *destBuffer_tmp = *destBuffer;

	if (!_mixBuf || !destBuffer || !len)
		return -1;

	if (_outChannelCount == 2)
		len *= 2;

	if (!_stereoReverseFlag || _outChannelCount == 1) {
		if (_outWordSize != 16) {
			if (len) {
				for (int i = 0; i < len; i++) {
					destBuffer_tmp[i] = ((uint8 *)_softLMID)[mixBuffer[i]];
				}
			}
		}

		if (len) {
			for (int i = 0; i < len; i++) {
				((uint16 *)destBuffer_tmp)[i] = ((uint16 *)_softLMID)[mixBuffer[i]];
			}
		}
	} else {
		len /= 2;
		if (_outWordSize == 16) {
			if (len) {
				for (int i = 0; i < len; i += 2) {
					((uint16 *)destBuffer_tmp)[i]     = ((uint16 *)_softLMID)[mixBuffer[i + 1]];
					((uint16 *)destBuffer_tmp)[i + 1] = ((uint16 *)_softLMID)[mixBuffer[i]];
				}
			}
		}

		if (len) {
			for (int i = 0; i < len; i += 2) {
				destBuffer_tmp[i] = ((uint8 *)_softLMID)[mixBuffer[i + 1]];
				destBuffer_tmp[i + 1] = ((uint8 *)_softLMID)[mixBuffer[i]];
			}
		}
	}
	return 0;
}

void IMuseDigiInternalMixer::mixBits8Mono(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable, bool ftIs11025Hz) {
	uint16 *mixBufCurCell;
	uint8 *srcBuf_ptr;
	uint8 *ptr;
	int residualLength;
	int value;

	mixBufCurCell = (uint16 *)(&_mixBuf[2 * mixBufStartIndex]);
	srcBuf_ptr = srcBuf;
	if (_isEarlyDiMUSE) {
		if (ftIs11025Hz) {
			if (inFrameCount - 1) {
				for (int i = 0; i < inFrameCount - 1; i++) {
					mixBufCurCell[0] += *((uint16 *)ampTable + srcBuf_ptr[0]);
					mixBufCurCell[1] += (*((int16 *)ampTable + srcBuf_ptr[0]) + *((int16 *)ampTable + srcBuf_ptr[1])) >> 1;
					mixBufCurCell += 2;
					srcBuf_ptr += 1;
				}
			}

			mixBufCurCell[0] += *((uint16 *)ampTable + srcBuf_ptr[0]);
			mixBufCurCell[1] += *((uint16 *)ampTable + srcBuf_ptr[0]);
		} else {
			if (inFrameCount) {
				for (int i = 0; i < inFrameCount; i++) {
					mixBufCurCell[i] += *((int16 *)ampTable + srcBuf_ptr[i]);
				}
			}
		}
	} else {
		if (inFrameCount == feedSize) {
			if (_radioChatter) {
				ptr = srcBuf + 4;
				value = srcBuf[0] - 128 + srcBuf[1] - 128 + srcBuf[2] - 128 + srcBuf[3] - 128;
				if (feedSize) {
					for (int i = 0; i < feedSize; i++) {
						mixBufCurCell[i] += 4 * *((uint16 *)ampTable + (srcBuf_ptr[i] - (value >> 2)));
						value += ptr[i] - srcBuf_ptr[i];
					}
				}
			} else {
				if (feedSize) {
					for (int i = 0; i < feedSize; i++) {
						mixBufCurCell[i] += *((uint16 *)ampTable + srcBuf_ptr[i]);
					}
				}
			}
		} else if (2 * inFrameCount == feedSize) {
			if (inFrameCount - 1 != 0) {
				for (int i = 0; i < inFrameCount - 1; i++) {
					mixBufCurCell[0] += *((uint16 *)ampTable + srcBuf_ptr[0]);
					mixBufCurCell[1] += (*((int16 *)ampTable + srcBuf_ptr[0]) + *((int16 *)ampTable + srcBuf_ptr[1])) >> 1;
					mixBufCurCell += 2;
					srcBuf_ptr += 1;
				}
			}
			mixBufCurCell[0] += *((uint16 *)ampTable + srcBuf_ptr[0]);
			mixBufCurCell[1] += *((uint16 *)ampTable + srcBuf_ptr[0]);
		} else if (2 * feedSize == inFrameCount) {
			if (feedSize) {
				for (int i = 0, j = 0; i < feedSize; i++, j += 2) {
					mixBufCurCell[i] += *((uint16 *)ampTable + srcBuf_ptr[j]);
				}
			}
		} else {
			residualLength = -inFrameCount;
			if (feedSize) {
				for (int i = 0; i < feedSize; i++) {
					mixBufCurCell[i] += *((uint16 *)ampTable + srcBuf_ptr[0]);
					// Seek the next srcBuf element until there's excess length
					for (residualLength += inFrameCount; residualLength >= 0; ++srcBuf_ptr)
						residualLength -= feedSize;
				}
			}
		}
	}
}

void IMuseDigiInternalMixer::mixBits12Mono(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable) {
	uint16 *mixBufCurCell;
	uint8 *srcBuf_ptr;
	int xorFlag;
	int residualLength;
	int term_1;
	int term_2;

	if ((inFrameCount & 1) != 0) {
		inFrameCount &= 0xFFFFFFFE;
		debug(5, "DiMUSE_InternalMixer::mixBits12Mono(): WARNING: odd inFrameCount with 12-bit data");
	}

	mixBufCurCell = (uint16 *)(&_mixBuf[2 * mixBufStartIndex]);
	if (feedSize == inFrameCount) {
		if (inFrameCount / 2) {
			srcBuf_ptr = srcBuf;
			for (int i = 0; i < inFrameCount / 2; i++) {
				mixBufCurCell[0] += *((uint16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)));
				mixBufCurCell[1] += *((uint16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));
				srcBuf_ptr += 3;
				mixBufCurCell += 2;
			}
		}
	} else if (2 * inFrameCount == feedSize) {
		srcBuf_ptr = srcBuf;
		if ((inFrameCount / 2) - 1) {
			for (int i = 0; i < (inFrameCount / 2) - 1; i++) {
				mixBufCurCell[0] += *((uint16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)));

				mixBufCurCell[1] += (*((int16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4))) +
									 *((int16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)))) >> 1;

				mixBufCurCell[2] += *((uint16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));

				mixBufCurCell[3] += (*((int16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4))) +
									 *((int16 *)ampTable + (srcBuf_ptr[3] | ((srcBuf_ptr[4] & 0xF)  << 8)))) >> 1;

				srcBuf_ptr += 3;
				mixBufCurCell += 4;
			}
		}

		mixBufCurCell[0] +=  *((uint16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)));
		mixBufCurCell[1] +=  (*((int16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)))
			                + *((int16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)))) >> 1;
		mixBufCurCell[2] +=  *((uint16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));
		mixBufCurCell[3] +=  *((uint16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));
	} else if (2 * feedSize == inFrameCount) {
		if (feedSize) {
			srcBuf_ptr = srcBuf;
			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[i] += *((uint16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF) << 8)));
				srcBuf_ptr += 3;
			}
		}
	} else {
		xorFlag = 0;
		residualLength = -inFrameCount;
		if (feedSize) {
			srcBuf_ptr = srcBuf;
			for (int i = 0; i < feedSize; i++) {
				if (xorFlag) {
					term_1 = (srcBuf_ptr[1] & 0xF0) << 4;
					term_2 = srcBuf_ptr[2];
				} else {
					term_1 = (srcBuf_ptr[1] & 0xF) << 8;
					term_2 = srcBuf_ptr[0];
				}

				mixBufCurCell[i] += *((uint16 *)ampTable + (term_2 | term_1));
				residualLength += inFrameCount;
				while (residualLength >= 0) {
					residualLength -= feedSize;
					xorFlag ^= 1u;
					if (!xorFlag)
						srcBuf_ptr += 3;
				}
			}
		}
	}
}

void IMuseDigiInternalMixer::mixBits16Mono(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable) {
	uint16 *mixBufCurCell;
	uint16 *srcBuf_ptr;
	int residualLength;

	mixBufCurCell = (uint16 *)(&_mixBuf[2 * mixBufStartIndex]);
	if (feedSize == inFrameCount) {
		if (feedSize) {
			srcBuf_ptr = (uint16 *)srcBuf;
			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[i] += *(uint16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[i] & (int16)0xFFF7) >> 3) + 4096);
			}
		}
	} else if (2 * inFrameCount == feedSize) {
		srcBuf_ptr = (uint16 *)srcBuf;
		int i = 0;
		if (inFrameCount - 1 != 0) {
			for (i = 0; i < inFrameCount - 1; i++) {
				mixBufCurCell[0] += *(uint16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[i] & (int16)0xFFF7) >> 3) + 4096);
				mixBufCurCell[1] += (*(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[i] & (int16)0xFFF7) >> 3) + 4096)
							       + *(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[i + 1] & (int16) 0xFFF7) >> 3) + 4096)) >> 1;
				mixBufCurCell += 2;
			}
		}
		mixBufCurCell[0] += *(uint16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[i] & (int16)0xFFF7) >> 3) + 4096);
		mixBufCurCell[1] += *(uint16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[i] & (int16)0xFFF7) >> 3) + 4096);
	} else {
		if (2 * feedSize == inFrameCount) {
			if (feedSize) {
				srcBuf_ptr = (uint16 *)srcBuf;
				for (int i = 0; i < feedSize; i++) {
					mixBufCurCell[i] += *(uint16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096);
					srcBuf_ptr += 2;
				}
			}
		} else {
			residualLength = -inFrameCount;

			if (feedSize) {
				srcBuf_ptr = (uint16 *)srcBuf;
				for (int i = 0; i < feedSize; i++) {
					mixBufCurCell[i] += *(uint16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096);

					for (residualLength += inFrameCount; residualLength >= 0; ++srcBuf_ptr)
						residualLength -= feedSize;
				}
			}
		}
	}
}

void IMuseDigiInternalMixer::mixBits8ConvertToMono(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable) {
	uint8 *srcBuf_ptr;
	int residualLength;
	uint16 *mixBufCurCell;

	mixBufCurCell = (uint16 *)(&_mixBuf[2 * mixBufStartIndex]);
	srcBuf_ptr = srcBuf;
	if (inFrameCount == feedSize) {
		if (feedSize) {
			for (int i = 0, j = 0; i < feedSize; i++, j += 2) {
				mixBufCurCell[i] += (2 * *((int16 *)ampTable + srcBuf_ptr[j])) >> 1;
			}
		}
	} else if (2 * inFrameCount == feedSize) {
		if (inFrameCount != 1) {
			for (int i = 0; i < inFrameCount - 1; i++) {
				mixBufCurCell[0] += (*((int16 *)ampTable + srcBuf_ptr[1]) + *((int16 *)ampTable + srcBuf_ptr[0])) >> 1;
				int term_1 = (*((int16 *)ampTable + srcBuf_ptr[1]) + *((int16 *)ampTable + srcBuf_ptr[3])) >> 1;
				int term_2 =  *((int16 *)ampTable + srcBuf_ptr[0]) + *((int16 *)ampTable + srcBuf_ptr[2]);
				mixBufCurCell[1] += ((term_2 >> 1) + term_1) >> 1;

				srcBuf_ptr += 2;
				mixBufCurCell += 2;
			}
		}
		mixBufCurCell[0] += (*((int16 *)ampTable + srcBuf_ptr[1]) + *((int16 *)ampTable + srcBuf_ptr[0])) >> 1;
		mixBufCurCell[1] += (*((int16 *)ampTable + srcBuf_ptr[1]) + *((int16 *)ampTable + srcBuf_ptr[0])) >> 1;
	} else if (2 * feedSize == inFrameCount) {
		if (feedSize) {
			for (int i = 0, j = 0; i < feedSize; i++, j += 4) {
				mixBufCurCell[i] += (2 * *((int16 *)ampTable + srcBuf_ptr[j])) >> 1;
			}
		}
	} else {
		residualLength = -inFrameCount;
		if (feedSize) {
			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[i] += (*((int16 *)ampTable + srcBuf_ptr[0]) + *((int16 *)ampTable + srcBuf_ptr[1])) >> 1;
				// Skip srcBuf elements until there's excess length
				for (residualLength += inFrameCount; residualLength >= 0; srcBuf_ptr += 2)
					residualLength -= feedSize;
			}
		}
	}
}

void IMuseDigiInternalMixer::mixBits12ConvertToMono(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable) {
	uint8 *srcBuf_ptr;
	int residualLength;
	uint16 *mixBufCurCell;
	int term_1;
	int term_2;

	mixBufCurCell = (uint16 *)(&_mixBuf[2 * mixBufStartIndex]);
	if (feedSize == inFrameCount) {
		if (feedSize) {
			srcBuf_ptr = srcBuf;

			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[i] += (*((int16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8))) +
									 *((int16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)))) >> 1;
				srcBuf_ptr += 3;
			}
		}
	} else if (2 * inFrameCount == feedSize) {
		srcBuf_ptr = srcBuf;
		if (inFrameCount - 1 != 0) {
			for (int i = 0; i < inFrameCount - 1; i++) {
				term_1 = *((int16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF0) << 8)));
				term_2 = *((int16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF)  << 4)));

				mixBufCurCell[0] += (term_1 + term_2) >> 1;
				mixBufCurCell[1] += (((term_1 + *((int16 *)ampTable + (srcBuf_ptr[3] | ((srcBuf_ptr[4] & 0xF)  << 8)))) >> 1)
								   + ((term_2 + *((int16 *)ampTable + (srcBuf_ptr[5] | ((srcBuf_ptr[4] & 0xF0) << 4)))) >> 1)) >> 1;

				srcBuf_ptr += 3;
				mixBufCurCell += 2;
			}
		}

		mixBufCurCell[0] += (*((int16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)))
						   + *((int16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)))) >> 1;
		mixBufCurCell[1] += (*((int16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)))
						   + *((int16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)))) >> 1;


	} else if (2 * feedSize == inFrameCount) {
		if (feedSize) {
			srcBuf_ptr = srcBuf;

			for (int i = 0; i < feedSize; i++) {

				mixBufCurCell[i] += (*((int16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)))
								   + *((int16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)))) >> 1;

				srcBuf_ptr += 6;
			}
		}
	} else {
		residualLength = -inFrameCount;
		if (feedSize) {
			srcBuf_ptr = srcBuf;
			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[i] += (*((int16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)))
								   + *((int16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)))) >> 1;

				for (residualLength += inFrameCount; residualLength >= 0; srcBuf_ptr += 3)
					residualLength -= feedSize;
			}
		}
	}
}

void IMuseDigiInternalMixer::mixBits16ConvertToMono(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable) {
	uint16 *mixBufCurCell;
	uint16 *srcBuf_ptr;
	int residualLength;

	mixBufCurCell = (uint16 *)(&_mixBuf[2 * mixBufStartIndex]);
	if (feedSize == inFrameCount) {
		if (feedSize) {
			srcBuf_ptr = (uint16 *)srcBuf;
			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[i] += (*(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096)
								   + *(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[1] & (int16)0xFFF7) >> 3) + 4096)) >> 1;
				srcBuf_ptr += 2;
			}
		}
	} else if (2 * inFrameCount == feedSize) {
		srcBuf_ptr = (uint16 *)srcBuf;
		if (inFrameCount - 1 != 0) {
			for (int i = 0; i < inFrameCount - 1; i++) {
				mixBufCurCell[0] += (*(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[1] & (int16)0xFFF7) >> 3) + 4096)
								   + *(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096)) >> 1;

				mixBufCurCell[1] +=  (((*(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096)
									+   *(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[2] & (int16)0xFFF7) >> 3) + 4096)) >> 1)
									+ ((*(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[1] & (int16)0xFFF7) >> 3) + 4096)
									+   *(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[3] & (int16)0xFFF7) >> 3) + 4096)) >> 1)) >> 1;
				mixBufCurCell += 2;
				srcBuf_ptr += 2;
			}
		}

		mixBufCurCell[0] += (*(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[1] & (int16)0xFFF7) >> 3) + 4096)
						   + *(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096)) >> 1;
		mixBufCurCell[1] += (*(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[1] & (int16)0xFFF7) >> 3) + 4096)
						   + *(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096)) >> 1;
	} else if (2 * feedSize == inFrameCount) {
		if (feedSize) {
			srcBuf_ptr = (uint16 *)srcBuf;
			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[i] += (*(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096)
								   + *(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[1] & (int16)0xFFF7) >> 3) + 4096)) >> 1;
				srcBuf_ptr += 4;
			}
		}
	} else {
		residualLength = -inFrameCount;

		if (feedSize) {
			srcBuf_ptr = (uint16 *)srcBuf;
			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[i] += (*(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096)
								   + *(int16 *)((uint8 *)ampTable + (((int16)srcBuf_ptr[1] & (int16)0xFFF7) >> 3) + 4096)) >> 1;

				for (residualLength += inFrameCount; residualLength >= 0; srcBuf_ptr += 2)
					residualLength -= feedSize;
			}
		}
	}
}

void IMuseDigiInternalMixer::mixBits8ConvertToStereo(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *leftAmpTable, int32 *rightAmpTable, bool ftIs11025Hz) {
	uint16 *mixBufCurCell;
	uint8 *srcBuf_ptr;
	uint8 *ptr;
	int value;
	int residualLength;

	mixBufCurCell = (uint16 *)(&_mixBuf[4 * mixBufStartIndex]);
	if (_isEarlyDiMUSE) {
		if (ftIs11025Hz) {
			srcBuf_ptr = srcBuf;
			int i = 0;
			if (inFrameCount - 1 != 0) {
				for (i = 0; i < inFrameCount - 1; i++) {
					mixBufCurCell[0] += *((uint16 *)leftAmpTable  + srcBuf_ptr[i]);
					mixBufCurCell[1] += *((uint16 *)rightAmpTable + srcBuf_ptr[i]);
					mixBufCurCell[2] += (*((int16 *)leftAmpTable  + srcBuf_ptr[i]) + *((int16 *)leftAmpTable  + srcBuf_ptr[i + 1])) >> 1;
					mixBufCurCell[3] += (*((int16 *)rightAmpTable + srcBuf_ptr[i]) + *((int16 *)rightAmpTable + srcBuf_ptr[i + 1])) >> 1;
					mixBufCurCell += 4;
				}
			}
			mixBufCurCell[0] += *((uint16 *)leftAmpTable  + srcBuf_ptr[i]);
			mixBufCurCell[1] += *((uint16 *)rightAmpTable + srcBuf_ptr[i]);
			mixBufCurCell[2] += *((uint16 *)leftAmpTable  + srcBuf_ptr[i]);
			mixBufCurCell[3] += *((uint16 *)rightAmpTable + srcBuf_ptr[i]);
		} else {
			srcBuf_ptr = srcBuf;
			if (inFrameCount) {
				for (int i = 0; i < inFrameCount; i++) {
					mixBufCurCell[0] += *((int16 *)leftAmpTable  + srcBuf_ptr[i]);
					mixBufCurCell[1] += *((int16 *)rightAmpTable + srcBuf_ptr[i]);
					mixBufCurCell += 2;
				}
			}
		}
	} else {
		if (feedSize == inFrameCount) {
			if (_radioChatter) {
				srcBuf_ptr = srcBuf;
				ptr = srcBuf + 4;
				value = srcBuf[0] - 128 + srcBuf[1] - 128 + srcBuf[2] - 128 + srcBuf[3] - 128;
				if (feedSize) {
					for (int i = 0; i < feedSize; i++) {
						mixBufCurCell[0] += 4 * *((uint16 *)leftAmpTable  + (srcBuf_ptr[i] - (value >> 2)));
						mixBufCurCell[1] += 4 * *((uint16 *)rightAmpTable + (srcBuf_ptr[i] - (value >> 2)));
						value += ptr[i] - srcBuf_ptr[i];
						mixBufCurCell += 2;
					}
				}
			} else {
				if (feedSize) {
					srcBuf_ptr = srcBuf;
					for (int i = 0; i < feedSize; i++) {
						mixBufCurCell[0] += *((uint16 *)leftAmpTable  + srcBuf_ptr[i]);
						mixBufCurCell[1] += *((uint16 *)rightAmpTable + srcBuf_ptr[i]);
						mixBufCurCell += 2;
					}
				}
			}
		} else if (2 * inFrameCount == feedSize) {
			srcBuf_ptr = srcBuf;
			int i = 0;
			if (inFrameCount - 1 != 0) {
				for (i = 0; i < inFrameCount - 1; i++) {
					mixBufCurCell[0] += *((uint16 *)leftAmpTable  + srcBuf_ptr[i]);
					mixBufCurCell[1] += *((uint16 *)rightAmpTable + srcBuf_ptr[i]);
					mixBufCurCell[2] += (*((int16 *)leftAmpTable  + srcBuf_ptr[i]) + *((int16 *)leftAmpTable  + srcBuf_ptr[i + 1])) >> 1;
					mixBufCurCell[3] += (*((int16 *)rightAmpTable + srcBuf_ptr[i]) + *((int16 *)rightAmpTable + srcBuf_ptr[i + 1])) >> 1;
					mixBufCurCell += 4;
				}
			}
			mixBufCurCell[0] += *((uint16 *)leftAmpTable  + srcBuf_ptr[i]);
			mixBufCurCell[1] += *((uint16 *)rightAmpTable + srcBuf_ptr[i]);
			mixBufCurCell[2] += *((uint16 *)leftAmpTable  + srcBuf_ptr[i]);
			mixBufCurCell[3] += *((uint16 *)rightAmpTable + srcBuf_ptr[i]);
		} else if (2 * feedSize == inFrameCount) {
			if (feedSize) {
				srcBuf_ptr = srcBuf;
				for (int i = 0; i < feedSize; i++) {
					mixBufCurCell[0] += *((uint16 *)leftAmpTable  + srcBuf_ptr[0]);
					mixBufCurCell[1] += *((uint16 *)rightAmpTable + srcBuf_ptr[0]);
					mixBufCurCell += 2;
					srcBuf_ptr += 2;
				}
			}
		} else {
			residualLength = -inFrameCount;
			if (feedSize) {
				srcBuf_ptr = srcBuf;
				for (int i = 0; i < feedSize; i++) {
					mixBufCurCell[0] += *((uint16 *)leftAmpTable  + srcBuf_ptr[0]);
					mixBufCurCell[1] += *((uint16 *)rightAmpTable + srcBuf_ptr[0]);
					mixBufCurCell += 2;

					for (residualLength += inFrameCount; residualLength > 0; ++srcBuf_ptr)
						residualLength -= feedSize;
				}
			}
		}
	}
}

void IMuseDigiInternalMixer::mixBits12ConvertToStereo(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *leftAmpTable, int32 *rightAmpTable) {
	uint16 *mixBufCurCell;
	uint8 *srcBuf_ptr;

	int xorFlag;
	int residualLength;

	int term_1;
	int term_2;
	int term_3;
	int term_4;

	mixBufCurCell = (uint16 *)(&_mixBuf[4 * mixBufStartIndex]);
	if (feedSize == inFrameCount) {
		if (inFrameCount / 2) {
			srcBuf_ptr = srcBuf;
			for (int i = 0; i < (inFrameCount / 2); i++) {
				mixBufCurCell[0] += *((uint16 *)leftAmpTable  + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)));
				mixBufCurCell[1] += *((uint16 *)rightAmpTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)));
				mixBufCurCell[2] += *((uint16 *)leftAmpTable  + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));
				mixBufCurCell[3] += *((uint16 *)rightAmpTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));
				srcBuf_ptr += 3;
				mixBufCurCell += 4;
			}
		}
	} else if (2 * inFrameCount == feedSize) {
		srcBuf_ptr = srcBuf;
		if ((inFrameCount / 2) - 1 != 0) {
			for (int i = 0; i < (inFrameCount / 2) - 1; i++) {
				mixBufCurCell[0] += *((uint16 *)leftAmpTable  + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF) << 8)));
				mixBufCurCell[1] += *((uint16 *)rightAmpTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF) << 8)));

				mixBufCurCell[2] += (*((int16 *)leftAmpTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)))
								   + *((int16 *)leftAmpTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)))) >> 1;

				mixBufCurCell[3] += (*((int16 *)rightAmpTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)))
								   + *((int16 *)rightAmpTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)))) >> 1;

				mixBufCurCell[4] += *((uint16 *)leftAmpTable  + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));
				mixBufCurCell[5] += *((uint16 *)rightAmpTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));

				mixBufCurCell[6] += (*((int16 *)leftAmpTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4))) +
									 *((int16 *)leftAmpTable + (srcBuf_ptr[3] | ((srcBuf_ptr[4] & 0xF)  << 8)))) >> 1;

				mixBufCurCell[7] += (*((int16 *)rightAmpTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4))) +
									 *((int16 *)rightAmpTable + (srcBuf_ptr[3] | ((srcBuf_ptr[4] & 0xF)  << 8)))) >> 1;

				srcBuf_ptr += 3;
				mixBufCurCell += 8;
			}
		}
		mixBufCurCell[0] += *((uint16 *)leftAmpTable  + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF) << 8)));
		mixBufCurCell[1] += *((uint16 *)rightAmpTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF) << 8)));

		mixBufCurCell[2] += (*((int16 *)leftAmpTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)))
						   + *((int16 *)leftAmpTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)))) >> 1;

		mixBufCurCell[3] += (*((int16 *)rightAmpTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)))
						   + *((int16 *)rightAmpTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)))) >> 1;

		mixBufCurCell[4] += *((uint16 *)leftAmpTable  + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));
		mixBufCurCell[5] += *((uint16 *)rightAmpTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));
		mixBufCurCell[6] += *((uint16 *)leftAmpTable  + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));
		mixBufCurCell[7] += *((uint16 *)rightAmpTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));
	} else if (2 * feedSize == inFrameCount) {
		if (feedSize) {
			srcBuf_ptr = srcBuf;
			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[0] += *((uint16 *)leftAmpTable  + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF) << 8)));
				mixBufCurCell[1] += *((uint16 *)rightAmpTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF) << 8)));
				srcBuf_ptr += 3;
				mixBufCurCell += 2;
			}
		}
	} else {
		xorFlag = 0;
		residualLength = -inFrameCount;
		if (feedSize) {
			srcBuf_ptr = srcBuf;
			for (int i = 0; i < feedSize; i++) {
				if (xorFlag) {
					term_2 = (srcBuf_ptr[1] & 0xF0) << 4;
					term_1 = srcBuf_ptr[2];
				} else {
					term_2 = (srcBuf_ptr[1] & 0xF) << 8;
					term_1 = srcBuf_ptr[0];
				}
				mixBufCurCell[0] += *((uint16 *)leftAmpTable + (term_1 | term_2));

				if (xorFlag) {
					term_4 = (srcBuf_ptr[1] & 0xF0) << 4;
					term_3 = srcBuf_ptr[2];
				} else {
					term_4 = (srcBuf_ptr[1] & 0xF) << 8;
					term_3 = srcBuf_ptr[0];
				}
				mixBufCurCell[1] += *((uint16 *)rightAmpTable + (term_3 | term_4));

				residualLength += inFrameCount;
				while (residualLength > 0) {
					residualLength -= feedSize;
					xorFlag ^= 1u;
					if (!xorFlag)
						srcBuf_ptr += 3;
				}

				mixBufCurCell += 2;
			}
		}
	}
}

void IMuseDigiInternalMixer::mixBits16ConvertToStereo(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *leftAmpTable, int32 *rightAmpTable) {
	uint16 *mixBufCurCell;
	uint16 *srcBuf_tmp;
	int residualLength;

	mixBufCurCell = (uint16 *)(&_mixBuf[2 * mixBufStartIndex]);

	if (feedSize == inFrameCount) {
		if (feedSize) {
			srcBuf_tmp = (uint16 *)srcBuf;
			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[0] += *(uint16 *)((uint8 *)leftAmpTable  + (((int16)srcBuf_tmp[i] & (int16)0xFFF7) >> 3) + 4096);
				mixBufCurCell[1] += *(uint16 *)((uint8 *)rightAmpTable + (((int16)srcBuf_tmp[i] & (int16)0xFFF7) >> 3) + 4096);
				mixBufCurCell += 2;
			}
		}
	} else if (2 * inFrameCount == feedSize) {
		srcBuf_tmp = (uint16 *)srcBuf;
		int i = 0;
		if (inFrameCount - 1 != 0) {
			for (i = 0; i < inFrameCount - 1; i++) {
				mixBufCurCell[0] += *(uint16 *)((uint8 *)leftAmpTable  + (((int16)srcBuf_tmp[i] & (int16)0xFFF7) >> 3) + 4096);
				mixBufCurCell[1] += *(uint16 *)((uint8 *)rightAmpTable + (((int16)srcBuf_tmp[i] & (int16)0xFFF7) >> 3) + 4096);

				mixBufCurCell[2] += (*(int16 *)((uint8 *)leftAmpTable + (((int16)srcBuf_tmp[i]     & (int16)0xFFF7) >> 3) + 4096)
								   + *(int16 *)((uint8 *)leftAmpTable + (((int16)srcBuf_tmp[i + 1] & (int16)0xFFF7) >> 3) + 4096)) >> 1;

				mixBufCurCell[3] += (*(int16 *)((uint8 *)rightAmpTable + (((int16)srcBuf_tmp[i]     & (int16)0xFFF7) >> 3) + 4096)
								   + *(int16 *)((uint8 *)rightAmpTable + (((int16)srcBuf_tmp[i + 1] & (int16)0xFFF7) >> 3) + 4096)) >> 1;
				mixBufCurCell += 4;
			}
		}
		mixBufCurCell[0] += *(uint16 *)((uint8 *)leftAmpTable  + (((int16)srcBuf_tmp[i] & (int16)0xFFF7) >> 3) + 4096);
		mixBufCurCell[1] += *(uint16 *)((uint8 *)rightAmpTable + (((int16)srcBuf_tmp[i] & (int16)0xFFF7) >> 3) + 4096);
		mixBufCurCell[2] += *(uint16 *)((uint8 *)leftAmpTable  + (((int16)srcBuf_tmp[i] & (int16)0xFFF7) >> 3) + 4096);
		mixBufCurCell[3] += *(uint16 *)((uint8 *)rightAmpTable + (((int16)srcBuf_tmp[i] & (int16)0xFFF7) >> 3) + 4096);
	} else if (2 * feedSize == inFrameCount) {
		if (feedSize) {
			srcBuf_tmp = (uint16 *)srcBuf;
			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[0] += *(uint16 *)((uint8 *)leftAmpTable  + (((int16)srcBuf_tmp[0] & (int16)0xFFF7) >> 3) + 4096);
				mixBufCurCell[1] += *(uint16 *)((uint8 *)rightAmpTable + (((int16)srcBuf_tmp[0] & (int16)0xFFF7) >> 3) + 4096);
				srcBuf_tmp += 2;
				mixBufCurCell += 2;
			}
		}
	} else {
		residualLength = -inFrameCount;
		if (feedSize) {
			srcBuf_tmp = (uint16 *)srcBuf;
			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[0] += *(uint16 *)((uint8 *)leftAmpTable  + (((int16)srcBuf_tmp[0] & (int16)0xFFF7) >> 3) + 4096);
				mixBufCurCell[1] += *(uint16 *)((uint8 *)rightAmpTable + (((int16)srcBuf_tmp[0] & (int16)0xFFF7) >> 3) + 4096);

				for (residualLength += inFrameCount; residualLength > 0; ++srcBuf_tmp)
					residualLength -= feedSize;

				mixBufCurCell += 2;
			}
		}
	}
}

void IMuseDigiInternalMixer::mixBits8Stereo(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable) {
	uint16 *mixBufCurCell;
	uint8 *srcBuf_ptr;
	int residualLength;

	mixBufCurCell = (uint16 *)(&_mixBuf[4 * mixBufStartIndex]);
	if (feedSize == inFrameCount) {
		if (feedSize) {
			srcBuf_ptr = srcBuf;
			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[0] += *((uint16 *)ampTable + srcBuf_ptr[0]);
				mixBufCurCell[1] += *((uint16 *)ampTable + srcBuf_ptr[1]);
				srcBuf_ptr += 2;
				mixBufCurCell += 2;
			}
		}
	} else if (2 * inFrameCount == feedSize) {
		srcBuf_ptr = srcBuf;
		if (inFrameCount - 1 != 0) {
			for (int i = 0; i < inFrameCount - 1; i++) {
				mixBufCurCell[0] += *((uint16 *)ampTable + srcBuf_ptr[0]);
				mixBufCurCell[1] += *((uint16 *)ampTable + srcBuf_ptr[1]);
				mixBufCurCell[2] += (*((int16 *)ampTable + srcBuf_ptr[0]) + *((int16 *)ampTable + srcBuf_ptr[2])) >> 1;
				mixBufCurCell[3] += (*((int16 *)ampTable + srcBuf_ptr[1]) + *((int16 *)ampTable + srcBuf_ptr[3])) >> 1;
				mixBufCurCell += 4;
				srcBuf_ptr += 2;
			}
		}
		mixBufCurCell[0] += *((uint16 *)ampTable + srcBuf_ptr[0]);
		mixBufCurCell[1] += *((uint16 *)ampTable + srcBuf_ptr[1]);
		mixBufCurCell[2] += *((uint16 *)ampTable + srcBuf_ptr[0]);
		mixBufCurCell[3] += *((uint16 *)ampTable + srcBuf_ptr[1]);
	} else if (2 * feedSize == inFrameCount) {
		if (feedSize) {
			srcBuf_ptr = srcBuf;
			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[0] += *((uint16 *)ampTable + srcBuf_ptr[0]);
				mixBufCurCell[1] += *((uint16 *)ampTable + srcBuf_ptr[1]);
				srcBuf_ptr += 4;
				mixBufCurCell += 2;
			}
		}
	} else {
		residualLength = -inFrameCount;
		if (feedSize) {
			srcBuf_ptr = srcBuf;
			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[0] += *((uint16 *)ampTable + srcBuf_ptr[0]);
				mixBufCurCell[1] += *((uint16 *)ampTable + srcBuf_ptr[1]);
				mixBufCurCell += 2;
				for (residualLength += inFrameCount; residualLength >= 0; srcBuf_ptr += 2)
					residualLength -= feedSize;
			}
		}
	}
}

void IMuseDigiInternalMixer::mixBits12Stereo(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable) {
	uint16 *mixBufCurCell;
	uint8 *srcBuf_ptr;
	int residualLength;

	mixBufCurCell = (uint16 *)(&_mixBuf[4 * mixBufStartIndex]);
	if (feedSize == inFrameCount) {
		if (feedSize) {
			srcBuf_ptr = srcBuf;

			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[0] += *((uint16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)));
				mixBufCurCell[1] += *((uint16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));

				srcBuf_ptr += 3;
				mixBufCurCell += 2;
			}
		}
	} else if (2 * inFrameCount == feedSize) {
		srcBuf_ptr = srcBuf;
		if (inFrameCount - 1 != 0) {
			for (int i = 0; i < inFrameCount - 1; i++) {
				mixBufCurCell[0] += *((uint16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)));
				mixBufCurCell[1] += *((uint16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));
				mixBufCurCell[2] += (*((int16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)))
								   + *((int16 *)ampTable + (srcBuf_ptr[3] | ((srcBuf_ptr[4] & 0xF)  << 8)))) >> 1;
				mixBufCurCell[3] += (*((int16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)))
								   + *((int16 *)ampTable + (srcBuf_ptr[5] | ((srcBuf_ptr[4] & 0xF0) << 4)))) >> 1;

				srcBuf_ptr += 3;
				mixBufCurCell += 4;
			}
		}
		mixBufCurCell[0] += *((uint16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)));
		mixBufCurCell[1] += *((uint16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));
		mixBufCurCell[2] += *((uint16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)));
		mixBufCurCell[3] += *((uint16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));
	} else if (2 * feedSize == inFrameCount) {
		if (feedSize) {
			srcBuf_ptr = srcBuf;

			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[0] += *((uint16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)));
				mixBufCurCell[1] += *((uint16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));

				srcBuf_ptr += 6;
				mixBufCurCell += 2;
			}
		}
	} else {
		residualLength = -inFrameCount;
		if (feedSize) {
			srcBuf_ptr = srcBuf;

			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[0] += *((uint16 *)ampTable + (srcBuf_ptr[0] | ((srcBuf_ptr[1] & 0xF)  << 8)));
				mixBufCurCell[1] += *((uint16 *)ampTable + (srcBuf_ptr[2] | ((srcBuf_ptr[1] & 0xF0) << 4)));

				mixBufCurCell += 2;
				for (residualLength += inFrameCount; residualLength >= 0; srcBuf_ptr += 3)
					residualLength -= feedSize;
			}
		}
	}
}

void IMuseDigiInternalMixer::mixBits16Stereo(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable) {
	uint16 *mixBufCurCell;
	uint16 *srcBuf_ptr;
	int residualLength;

	mixBufCurCell = (uint16 *)(&_mixBuf[4 * mixBufStartIndex]);
	if (feedSize == inFrameCount) {
		if (feedSize) {
			srcBuf_ptr = (uint16 *)srcBuf;

			for (int i = 0; i < feedSize; i++) {
				mixBufCurCell[0] += *(uint16 *)((int8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096);
				mixBufCurCell[1] += *(uint16 *)((int8 *)ampTable + (((int16)srcBuf_ptr[1] & (int16)0xFFF7) >> 3) + 4096);
				srcBuf_ptr += 2;
				mixBufCurCell += 2;
			}
		}
	} else if (2 * inFrameCount == feedSize) {
		srcBuf_ptr = (uint16 *)srcBuf;

		if (inFrameCount - 1 != 0) {
			for (int i = 0; i < inFrameCount - 1; i++) {
				mixBufCurCell[0] += *(uint16 *)((int8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096);
				mixBufCurCell[1] += *(uint16 *)((int8 *)ampTable + (((int16)srcBuf_ptr[1] & (int16)0xFFF7) >> 3) + 4096);

				mixBufCurCell[2] += (*(int16 *)((int8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096)
								   + *(int16 *)((int8 *)ampTable + (((int16)srcBuf_ptr[2] & (int16)0xFFF7) >> 3) + 4096)) >> 1;

				mixBufCurCell[4] += (*(int16 *)((int8 *)ampTable + (((int16)(((uint8 *)srcBuf_ptr)[2] | (((uint8 *)srcBuf_ptr)[3] << 8)) & (int16)0xFFF7) >> 3) + 4096)
							       + *(int16 *)((int8 *)ampTable + (((int16)srcBuf_ptr[3] & (int16)0xFFF7) >> 3) + 4096)) >> 1;
				mixBufCurCell += 4;
				srcBuf_ptr += 2;
			}
		}
		mixBufCurCell[0] += *(int16 *)((int8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096);
		mixBufCurCell[1] += *(int16 *)((int8 *)ampTable + (((int16)srcBuf_ptr[1] & (int16)0xFFF7) >> 3) + 4096);
		mixBufCurCell[2] += *(int16 *)((int8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096);
		mixBufCurCell[3] += *(int16 *)((int8 *)ampTable + (((int16)srcBuf_ptr[1] & (int16)0xFFF7) >> 3) + 4096);
	} else {
		if (2 * feedSize == inFrameCount) {
			if (feedSize) {
				srcBuf_ptr = (uint16 *)srcBuf;
				for (int i = 0; i < feedSize; i++) {
					mixBufCurCell[0] += *(uint16 *)((int8 *)ampTable + ((((int16)srcBuf_ptr[0] & (int16)0xFFF7)) >> 3) + 4096);
					mixBufCurCell[1] += *(uint16 *)((int8 *)ampTable + ((((int16)srcBuf_ptr[1] & (int16)0xFFF7)) >> 3) + 4096);
					mixBufCurCell += 2;
					srcBuf_ptr += 4;
				}
			}
		} else {
			residualLength = -inFrameCount;
			if (feedSize) {
				srcBuf_ptr = (uint16 *)srcBuf;
				for (int i = 0; i < feedSize; i++) {
					mixBufCurCell[0] += *(uint16 *)((int8 *)ampTable + (((int16)srcBuf_ptr[0] & (int16)0xFFF7) >> 3) + 4096);
					mixBufCurCell[1] += *(uint16 *)((int8 *)ampTable + (((int16)srcBuf_ptr[1] & (int16)0xFFF7) >> 3) + 4096);

					for (residualLength += inFrameCount; residualLength >= 0; srcBuf_ptr += 2)
						residualLength -= feedSize;
				}
			}
		}
	}
}

} // End of namespace Scumm
