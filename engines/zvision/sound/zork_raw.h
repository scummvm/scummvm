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

#ifndef ZVISION_ZORK_RAW_H
#define ZVISION_ZORK_RAW_H

#include "audio/audiostream.h"

namespace Common {
class SeekableReadStream;
}

namespace ZVision {

class ZVision;

struct SoundParams {
	char identifier;
	uint32 rate;
	bool stereo;
	bool packed;
	bool bits16;
};

/**
 * This is a ADPCM stream-reader, this class holds context for multi-chunk reading and no buffers.
 */
class RawChunkStream {
public:
	RawChunkStream(bool stereo);

	~RawChunkStream() {
	}
private:
	uint _stereo;

	/**
	 * Holds the frequency and index from the last sample
	 * 0 holds the left channel, 1 holds the right channel
	 */
	struct {
		int32 sample;
		int16 index;
	} _lastSample[2];

	static const int16 _stepAdjustmentTable[8];
	static const int32 _amplitudeLookupTable[89];

public:

	struct RawChunk {
		int16 *data;
		uint32 size;
	};

	void init();
	//Read next audio portion in new stream (needed for avi), return structure with buffer
	RawChunk readNextChunk(Common::SeekableReadStream *stream);
	//Read numSamples from stream to buffer
	int readBuffer(int16 *buffer, Common::SeekableReadStream *stream, const int numSamples);
};

/**
 * This is a stream, which allows for playing raw ADPCM data from a stream.
 */
class RawZorkStream : public Audio::RewindableAudioStream {
public:
	RawZorkStream(uint32 rate, bool stereo, DisposeAfterUse::Flag disposeStream, Common::SeekableReadStream *stream);

	~RawZorkStream() override {
	}

public:
	static const SoundParams _zNemSoundParamLookupTable[32];
	static const SoundParams _zgiSoundParamLookupTable[24];

private:
	const int _rate;                                           // Sample rate of stream
	Audio::Timestamp _playtime;                                // Calculated total play time
	Common::DisposablePtr<Common::SeekableReadStream> _stream; // Stream to read data from
	bool _endOfData;                                           // Whether the stream end has been reached
	uint _stereo;

	RawChunkStream _streamReader;

public:
	int readBuffer(int16 *buffer, const int numSamples) override;

	bool isStereo() const override {
		return _stereo;
	}
	bool endOfData() const override {
		return _endOfData;
	}

	int getRate() const override {
		return _rate;
	}
	Audio::Timestamp getLength() const {
		return _playtime;
	}

	bool rewind() override;
};

/**
 * Creates an audio stream, which plays from the given stream.
 *
 * @param stream     Stream object to play from.
 * @param rate       Rate of the sound data.
 * @param dispose    AfterUse Whether to delete the stream after use.
 * @return           The new SeekableAudioStream (or 0 on failure).
 */
Audio::RewindableAudioStream *makeRawZorkStream(Common::SeekableReadStream *stream,
        int rate,
        bool stereo,
        DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

Audio::RewindableAudioStream *makeRawZorkStream(const Common::String &filePath, ZVision *engine);

} // End of namespace ZVision

#endif
