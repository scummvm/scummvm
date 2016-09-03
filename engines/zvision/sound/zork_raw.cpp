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

#include "common/scummsys.h"
#include "common/file.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "common/bufferedstream.h"
#include "common/util.h"
#include "common/tokenizer.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "zvision/sound/zork_raw.h"
#include "zvision/zvision.h"

namespace ZVision {

const int16 RawChunkStream::_stepAdjustmentTable[8] = { -1, -1, -1, 1, 4, 7, 10, 12};

const int32 RawChunkStream::_amplitudeLookupTable[89] = {0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E,
                                                         0x0010, 0x0011, 0x0013, 0x0015, 0x0017, 0x0019, 0x001C, 0x001F,
                                                         0x0022, 0x0025, 0x0029, 0x002D, 0x0032, 0x0037, 0x003C, 0x0042,
                                                         0x0049, 0x0050, 0x0058, 0x0061, 0x006B, 0x0076, 0x0082, 0x008F,
                                                         0x009D, 0x00AD, 0x00BE, 0x00D1, 0x00E6, 0x00FD, 0x0117, 0x0133,
                                                         0x0151, 0x0173, 0x0198, 0x01C1, 0x01EE, 0x0220, 0x0256, 0x0292,
                                                         0x02D4, 0x031C, 0x036C, 0x03C3, 0x0424, 0x048E, 0x0502, 0x0583,
                                                         0x0610, 0x06AB, 0x0756, 0x0812, 0x08E0, 0x09C3, 0x0ABD, 0x0BD0,
                                                         0x0CFF, 0x0E4C, 0x0FBA, 0x114C, 0x1307, 0x14EE, 0x1706, 0x1954,
                                                         0x1BDC, 0x1EA5, 0x21B6, 0x2515, 0x28CA, 0x2CDF, 0x315B, 0x364B,
                                                         0x3BB9, 0x41B2, 0x4844, 0x4F7E, 0x5771, 0x602F, 0x69CE, 0x7462, 0x7FFF
                                                        };

RawChunkStream::RawChunkStream(bool stereo) {
	if (stereo)
		_stereo = 1;
	else
		_stereo = 0;

	init();
}

void RawChunkStream::init() {
	_lastSample[0].index = 0;
	_lastSample[0].sample = 0;
	_lastSample[1].index = 0;
	_lastSample[1].sample = 0;
}

RawChunkStream::RawChunk RawChunkStream::readNextChunk(Common::SeekableReadStream *stream) {
	RawChunk tmp;
	tmp.size = 0;
	tmp.data = NULL;

	if (!stream || stream->size() == 0 || stream->eos())
		return tmp;

	tmp.size = (stream->size() - stream->pos()) * 2;
	tmp.data = (int16 *)calloc(tmp.size, 1);

	readBuffer(tmp.data, stream, stream->size() - stream->pos());

	return tmp;
}

int RawChunkStream::readBuffer(int16 *buffer, Common::SeekableReadStream *stream, const int numSamples) {
	int32 bytesRead = 0;

	// 0: Left, 1: Right
	uint channel = 0;

	while (bytesRead < numSamples) {
		byte encodedSample = stream->readByte();
		if (stream->eos()) {
			return bytesRead;
		}
		bytesRead++;

		int16 index = _lastSample[channel].index;
		uint32 lookUpSample = _amplitudeLookupTable[index];

		int32 sample = 0;

		if (encodedSample & 0x40)
			sample += lookUpSample;
		if (encodedSample & 0x20)
			sample += lookUpSample >> 1;
		if (encodedSample & 0x10)
			sample += lookUpSample >> 2;
		if (encodedSample & 8)
			sample += lookUpSample >> 3;
		if (encodedSample & 4)
			sample += lookUpSample >> 4;
		if (encodedSample & 2)
			sample += lookUpSample >> 5;
		if (encodedSample & 1)
			sample += lookUpSample >> 6;
		if (encodedSample & 0x80)
			sample = -sample;

		sample += _lastSample[channel].sample;
		sample = CLIP<int32>(sample, -32768, 32767);

		buffer[bytesRead - 1] = (int16)sample;

		index += _stepAdjustmentTable[(encodedSample >> 4) & 7];
		index = CLIP<int16>(index, 0, 88);

		_lastSample[channel].sample = sample;
		_lastSample[channel].index = index;

		// Increment and wrap the channel
		channel = (channel + 1) & _stereo;
	}
	return bytesRead;
}

const SoundParams RawZorkStream::_zNemSoundParamLookupTable[32] = {
	{'0', 0x1F40, false, false, false},
	{'1', 0x1F40, true, false, false},
	{'2', 0x1F40, false, false, true},
	{'3', 0x1F40, true, false, true},
	{'4', 0x2B11, false, false, false},
	{'5', 0x2B11, true, false, false},
	{'6', 0x2B11, false, false, true},
	{'7', 0x2B11, true, false, true},
	{'8', 0x5622, false, false, false},
	{'9', 0x5622, true, false, false},
	{'a', 0x5622, false, false, true},
	{'b', 0x5622, true, false, true},
	{'c', 0xAC44, false, false, false},
	{'d', 0xAC44, true, false, false},
	{'e', 0xAC44, false, false, true},
	{'f', 0xAC44, true, false, true},
	{'g', 0x1F40, false, true, false},
	{'h', 0x1F40, true, true, false},
	{'j', 0x1F40, false, true, true},
	{'k', 0x1F40, true, true, true},
	{'l', 0x2B11, false, true, false},
	{'m', 0x2B11, true, true, false},
	{'n', 0x2B11, false, true, true},
	{'p', 0x2B11, true, true, true},
	{'q', 0x5622, false, true, false},
	{'r', 0x5622, true, true, false},
	{'s', 0x5622, false, true, true},
	{'t', 0x5622, true, true, true},
	{'u', 0xAC44, false, true, false},
	{'v', 0xAC44, true, true, false},
	{'w', 0xAC44, false, true, true},
	{'x', 0xAC44, true, true, true}
};

const SoundParams RawZorkStream::_zgiSoundParamLookupTable[24] = {
	{'4', 0x2B11, false, false, false},
	{'5', 0x2B11, true, false, false},
	{'6', 0x2B11, false, false, true},
	{'7', 0x2B11, true, false, true},
	{'8', 0x5622, false, false, false},
	{'9', 0x5622, true, false, false},
	{'a', 0x5622, false, false, true},
	{'b', 0x5622, true, false, true},
	{'c', 0xAC44, false, false, false},
	{'d', 0xAC44, true, false, false},
	{'e', 0xAC44, false, false, true},
	{'f', 0xAC44, true, false, true},
	{'g', 0x2B11, false, true, false},
	{'h', 0x2B11, true, true, false},
	{'j', 0x2B11, false, true, true},
	{'k', 0x2B11, true, true, true},
	{'m', 0x5622, false, true, false},
	{'n', 0x5622, true, true, false},
	{'p', 0x5622, false, true, true},
	{'q', 0x5622, true, true, true},
	{'r', 0xAC44, false, true, false},
	{'s', 0xAC44, true, true, false},
	{'t', 0xAC44, false, true, true},
	{'u', 0xAC44, true, true, true}
};

RawZorkStream::RawZorkStream(uint32 rate, bool stereo, DisposeAfterUse::Flag disposeStream, Common::SeekableReadStream *stream)
	: _rate(rate),
	  _stereo(0),
	  _stream(stream, disposeStream),
	  _endOfData(false),
	  _streamReader(stereo) {
	if (stereo)
		_stereo = 1;

	// Calculate the total playtime of the stream
	if (stereo)
		_playtime = Audio::Timestamp(0, _stream->size() / 2, rate);
	else
		_playtime = Audio::Timestamp(0, _stream->size(), rate);
}

int RawZorkStream::readBuffer(int16 *buffer, const int numSamples) {
	int32 bytesRead = _streamReader.readBuffer(buffer, _stream.get(), numSamples);

	if (_stream->eos())
		_endOfData = true;

	return bytesRead;
}

bool RawZorkStream::rewind() {
	_stream->seek(0, 0);
	_stream->clearErr();
	_endOfData = false;
	_streamReader.init();

	return true;
}

Audio::RewindableAudioStream *makeRawZorkStream(Common::SeekableReadStream *stream,
        int rate,
        bool stereo,
        DisposeAfterUse::Flag disposeAfterUse) {
	if (stereo)
		assert(stream->size() % 2 == 0);

	return new RawZorkStream(rate, stereo, disposeAfterUse, stream);
}

Audio::RewindableAudioStream *makeRawZorkStream(const Common::String &filePath, ZVision *engine) {
	Common::File *file = new Common::File();
	Common::String actualName = filePath;
	bool found = engine->getSearchManager()->openFile(*file, actualName);
	bool isRaw = actualName.hasSuffix(".raw");

	if ((!found && isRaw) || (found && isRaw && file->size() < 10)) {
		if (found)
			file->close();

		// Check for an audio patch (.src)
		actualName.setChar('s', actualName.size() - 3);
		actualName.setChar('r', actualName.size() - 2);
		actualName.setChar('c', actualName.size() - 1);

		if (!engine->getSearchManager()->openFile(*file, actualName))
			return NULL;
	} else if (!found && !isRaw) {
		return NULL;
	}

	// Get the file name
	Common::StringTokenizer tokenizer(actualName, "/\\");
	Common::String fileName;
	while (!tokenizer.empty()) {
		fileName = tokenizer.nextToken();
	}

	fileName.toLowercase();

	const SoundParams *soundParams = NULL;

	if (engine->getGameId() == GID_NEMESIS) {
		for (int i = 0; i < 32; ++i) {
			if (RawZorkStream::_zNemSoundParamLookupTable[i].identifier == (fileName[6]))
				soundParams = &RawZorkStream::_zNemSoundParamLookupTable[i];
		}
	} else if (engine->getGameId() == GID_GRANDINQUISITOR) {
		for (int i = 0; i < 24; ++i) {
			if (RawZorkStream::_zgiSoundParamLookupTable[i].identifier == (fileName[7]))
				soundParams = &RawZorkStream::_zgiSoundParamLookupTable[i];
		}
	}

	if (soundParams == NULL)
		return NULL;

	if (soundParams->packed) {
		return makeRawZorkStream(wrapBufferedSeekableReadStream(file, 2048, DisposeAfterUse::YES), soundParams->rate, soundParams->stereo, DisposeAfterUse::YES);
	} else {
		byte flags = 0;
		if (soundParams->bits16)
			flags |= Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
		if (soundParams->stereo)
			flags |= Audio::FLAG_STEREO;

		return Audio::makeRawStream(file, soundParams->rate, flags, DisposeAfterUse::YES);
	}
}

} // End of namespace ZVision
