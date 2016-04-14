/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/textconsole.h"
#include "common/stream.h"
#include "common/util.h"

#include "audio/decoders/3do.h"
#include "audio/decoders/adpcm_intern.h"

namespace Audio {

// Reuses ADPCM table
#define audio_3DO_ADP4_stepSizeTable Ima_ADPCMStream::_imaTable
#define audio_3DO_ADP4_stepSizeIndex ADPCMStream::_stepAdjustTable

RewindableAudioStream *make3DO_ADP4AudioStream(Common::SeekableReadStream *stream, uint16 sampleRate, bool stereo, uint32 *audioLengthMSecsPtr, DisposeAfterUse::Flag disposeAfterUse, audio_3DO_ADP4_PersistentSpace *persistentSpace) {
	if (stereo) {
		warning("make3DO_ADP4Stream(): stereo currently not supported");
		return 0;
	}

	if (audioLengthMSecsPtr) {
		// Caller requires the milliseconds of audio
		uint32 audioLengthMSecs = stream->size() * 2 * 1000 / sampleRate; // 1 byte == 2 16-bit sample
		if (stereo) {
			audioLengthMSecs /= 2;
		}
		*audioLengthMSecsPtr = audioLengthMSecs;
	}

	return new Audio3DO_ADP4_Stream(stream, sampleRate, stereo, disposeAfterUse, persistentSpace);
}

Audio3DO_ADP4_Stream::Audio3DO_ADP4_Stream(Common::SeekableReadStream *stream, uint16 sampleRate, bool stereo, DisposeAfterUse::Flag disposeAfterUse, audio_3DO_ADP4_PersistentSpace *persistentSpace)
	: _sampleRate(sampleRate), _stereo(stereo),
	  _stream(stream, disposeAfterUse) {

	_callerDecoderData = persistentSpace;
	memset(&_initialDecoderData, 0, sizeof(_initialDecoderData));
	_initialRead = true;

	reset();
}

void Audio3DO_ADP4_Stream::reset() {
	memcpy(&_curDecoderData, &_initialDecoderData, sizeof(_curDecoderData));
	_streamBytesLeft = _stream->size();
	_stream->seek(0);
}

bool Audio3DO_ADP4_Stream::rewind() {
	reset();
	return true;
}

int16 Audio3DO_ADP4_Stream::decodeSample(byte compressedNibble) {
	int16 currentStep = audio_3DO_ADP4_stepSizeTable[_curDecoderData.stepIndex];
	int32 decodedSample = _curDecoderData.lastSample;
	int16 delta = currentStep >> 3;

	if (compressedNibble & 1)
		delta += currentStep >> 2;

	if (compressedNibble & 2)
		delta += currentStep >> 1;

	if (compressedNibble & 4)
		delta += currentStep;

	if (compressedNibble & 8) {
		decodedSample -= delta;
	} else {
		decodedSample += delta;
	}

	_curDecoderData.lastSample = CLIP<int32>(decodedSample, -32768, 32767);

	_curDecoderData.stepIndex += audio_3DO_ADP4_stepSizeIndex[compressedNibble & 0x07];
	_curDecoderData.stepIndex = CLIP<int16>(_curDecoderData.stepIndex, 0, ARRAYSIZE(audio_3DO_ADP4_stepSizeTable) - 1);

   return _curDecoderData.lastSample;
}

// Writes the requested amount (or less) of samples into buffer and returns the amount of samples, that got written
int Audio3DO_ADP4_Stream::readBuffer(int16 *buffer, const int numSamples) {
	int8  byteCache[AUDIO_3DO_CACHE_SIZE];
	int8 *byteCachePtr = NULL;
	int   byteCacheSize = 0;
	int   requestedBytesLeft = 0;
	int   decodedSamplesCount = 0;

	int8  compressedByte = 0;

	if (endOfData())
		return 0; // no more bytes left

	if (_callerDecoderData) {
		// copy caller decoder data over
		memcpy(&_curDecoderData, _callerDecoderData, sizeof(_curDecoderData));
		if (_initialRead) {
			_initialRead = false;
			memcpy(&_initialDecoderData, &_curDecoderData, sizeof(_initialDecoderData));
		}
	}

	requestedBytesLeft = numSamples >> 1; // 1 byte for 2 16-bit sample
	if (requestedBytesLeft > _streamBytesLeft)
		requestedBytesLeft = _streamBytesLeft; // not enough bytes left

	// in case caller requests an uneven amount of samples, we will return an even amount

	// buffering, so that direct decoding of files and such runs way faster
	while (requestedBytesLeft) {
		if (requestedBytesLeft > AUDIO_3DO_CACHE_SIZE) {
			byteCacheSize = AUDIO_3DO_CACHE_SIZE;
		} else {
			byteCacheSize = requestedBytesLeft;
		}

		requestedBytesLeft -= byteCacheSize;
		_streamBytesLeft   -= byteCacheSize;

		// Fill our byte cache
		_stream->read(byteCache, byteCacheSize);

		byteCachePtr = byteCache;

		// Mono
		while (byteCacheSize) {
			compressedByte = *byteCachePtr++;
			byteCacheSize--;

			buffer[decodedSamplesCount] = decodeSample(compressedByte >> 4);
			decodedSamplesCount++;
			buffer[decodedSamplesCount] = decodeSample(compressedByte & 0x0f);
			decodedSamplesCount++;
		}
	}

	if (_callerDecoderData) {
		// copy caller decoder data back
		memcpy(_callerDecoderData, &_curDecoderData, sizeof(_curDecoderData));
	}

	return decodedSamplesCount;
}

// ============================================================================
static int16 audio_3DO_SDX2_SquareTable[256] = {
	-32768,-32258,-31752,-31250,-30752,-30258,-29768,-29282,-28800,-28322,
	-27848,-27378,-26912,-26450,-25992,-25538,-25088,-24642,-24200,-23762,
	-23328,-22898,-22472,-22050,-21632,-21218,-20808,-20402,-20000,-19602,
	-19208,-18818,-18432,-18050,-17672,-17298,-16928,-16562,-16200,-15842,
	-15488,-15138,-14792,-14450,-14112,-13778,-13448,-13122,-12800,-12482,
	-12168,-11858,-11552,-11250,-10952,-10658,-10368,-10082, -9800, -9522,
	 -9248, -8978, -8712, -8450, -8192, -7938, -7688, -7442, -7200, -6962,
	 -6728, -6498, -6272, -6050, -5832, -5618, -5408, -5202, -5000, -4802,
	 -4608, -4418, -4232, -4050, -3872, -3698, -3528, -3362, -3200, -3042,
	 -2888, -2738, -2592, -2450, -2312, -2178, -2048, -1922, -1800, -1682,
	 -1568, -1458, -1352, -1250, -1152, -1058,  -968,  -882,  -800,  -722,
	  -648,  -578,  -512,  -450,  -392,  -338,  -288,  -242,  -200,  -162,
	  -128,   -98,   -72,   -50,   -32,   -18,    -8,    -2,     0,     2,
	     8,    18,    32,    50,    72,    98,   128,   162,   200,   242,
	   288,   338,   392,   450,   512,   578,   648,   722,   800,   882,
	   968,  1058,  1152,  1250,  1352,  1458,  1568,  1682,  1800,  1922,
	  2048,  2178,  2312,  2450,  2592,  2738,  2888,  3042,  3200,  3362,
	  3528,  3698,  3872,  4050,  4232,  4418,  4608,  4802,  5000,  5202,
	  5408,  5618,  5832,  6050,  6272,  6498,  6728,  6962,  7200,  7442,
	  7688,  7938,  8192,  8450,  8712,  8978,  9248,  9522,  9800, 10082,
	 10368, 10658, 10952, 11250, 11552, 11858, 12168, 12482, 12800, 13122,
	 13448, 13778, 14112, 14450, 14792, 15138, 15488, 15842, 16200, 16562,
	 16928, 17298, 17672, 18050, 18432, 18818, 19208, 19602, 20000, 20402,
	 20808, 21218, 21632, 22050, 22472, 22898, 23328, 23762, 24200, 24642,
	 25088, 25538, 25992, 26450, 26912, 27378, 27848, 28322, 28800, 29282,
	 29768, 30258, 30752, 31250, 31752, 32258
};

Audio3DO_SDX2_Stream::Audio3DO_SDX2_Stream(Common::SeekableReadStream *stream, uint16 sampleRate, bool stereo, DisposeAfterUse::Flag disposeAfterUse, audio_3DO_SDX2_PersistentSpace *persistentSpace)
	: _sampleRate(sampleRate), _stereo(stereo),
	  _stream(stream, disposeAfterUse) {

	_callerDecoderData = persistentSpace;
	memset(&_initialDecoderData, 0, sizeof(_initialDecoderData));
	_initialRead = true;

	reset();
}

void Audio3DO_SDX2_Stream::reset() {
	memcpy(&_curDecoderData, &_initialDecoderData, sizeof(_curDecoderData));
	_streamBytesLeft = _stream->size();
	_stream->seek(0);
}

bool Audio3DO_SDX2_Stream::rewind() {
	reset();
	return true;
}

// Writes the requested amount (or less) of samples into buffer and returns the amount of samples, that got written
int Audio3DO_SDX2_Stream::readBuffer(int16 *buffer, const int numSamples) {
	int8  byteCache[AUDIO_3DO_CACHE_SIZE];
	int8 *byteCachePtr = NULL;
	int   byteCacheSize = 0;
	int   requestedBytesLeft = numSamples; // 1 byte per 16-bit sample
	int   decodedSamplesCount = 0;

	int8  compressedByte = 0;
	uint8 squareTableOffset = 0;
	int16 decodedSample = 0;

	if (endOfData())
		return 0; // no more bytes left

	if (_stereo) {
		// We expect numSamples to be even in case of Stereo audio
		assert((numSamples & 1) == 0);
	}

	if (_callerDecoderData) {
		// copy caller decoder data over
		memcpy(&_curDecoderData, _callerDecoderData, sizeof(_curDecoderData));
		if (_initialRead) {
			_initialRead = false;
			memcpy(&_initialDecoderData, &_curDecoderData, sizeof(_initialDecoderData));
		}
	}

	requestedBytesLeft = numSamples;
	if (requestedBytesLeft > _streamBytesLeft)
		requestedBytesLeft = _streamBytesLeft; // not enough bytes left

	// buffering, so that direct decoding of files and such runs way faster
	while (requestedBytesLeft) {
		if (requestedBytesLeft > AUDIO_3DO_CACHE_SIZE) {
			byteCacheSize = AUDIO_3DO_CACHE_SIZE;
		} else {
			byteCacheSize = requestedBytesLeft;
		}

		requestedBytesLeft -= byteCacheSize;
		_streamBytesLeft   -= byteCacheSize;

		// Fill our byte cache
		_stream->read(byteCache, byteCacheSize);

		byteCachePtr = byteCache;

		if (!_stereo) {
			// Mono
			while (byteCacheSize) {
				compressedByte = *byteCachePtr++;
				byteCacheSize--;
				squareTableOffset = compressedByte + 128;

				if (!(compressedByte & 1))
					_curDecoderData.lastSample1 = 0;

				decodedSample = _curDecoderData.lastSample1 + audio_3DO_SDX2_SquareTable[squareTableOffset];
				_curDecoderData.lastSample1 = decodedSample;

				buffer[decodedSamplesCount] = decodedSample;
				decodedSamplesCount++;
			}
		} else {
			// Stereo
			while (byteCacheSize) {
				compressedByte = *byteCachePtr++;
				byteCacheSize--;
				squareTableOffset = compressedByte + 128;

				if (!(decodedSamplesCount & 1)) {
					// First channel
					if (!(compressedByte & 1))
						_curDecoderData.lastSample1 = 0;

					decodedSample = _curDecoderData.lastSample1 + audio_3DO_SDX2_SquareTable[squareTableOffset];
					_curDecoderData.lastSample1 = decodedSample;
				} else {
					// Second channel
					if (!(compressedByte & 1))
						_curDecoderData.lastSample2 = 0;

					decodedSample = _curDecoderData.lastSample2 + audio_3DO_SDX2_SquareTable[squareTableOffset];
					_curDecoderData.lastSample2 = decodedSample;
				}

				buffer[decodedSamplesCount] = decodedSample;
				decodedSamplesCount++;
			}
		}
	}

	if (_callerDecoderData) {
		// copy caller decoder data back
		memcpy(_callerDecoderData, &_curDecoderData, sizeof(_curDecoderData));
	}

	return decodedSamplesCount;
}

RewindableAudioStream *make3DO_SDX2AudioStream(Common::SeekableReadStream *stream, uint16 sampleRate, bool stereo, uint32 *audioLengthMSecsPtr, DisposeAfterUse::Flag disposeAfterUse, audio_3DO_SDX2_PersistentSpace *persistentSpace) {
	if (stereo) {
		if (stream->size() & 1) {
			warning("make3DO_SDX2Stream(): stereo data is uneven size");
			return 0;
		}
	}

	if (audioLengthMSecsPtr) {
		// Caller requires the milliseconds of audio
		uint32 audioLengthMSecs = stream->size() * 1000 / sampleRate; // 1 byte == 1 16-bit sample
		if (stereo) {
			audioLengthMSecs /= 2;
		}
		*audioLengthMSecsPtr = audioLengthMSecs;
	}

	return new Audio3DO_SDX2_Stream(stream, sampleRate, stereo, disposeAfterUse, persistentSpace);
}

} // End of namespace Audio
