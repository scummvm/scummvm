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

/**
 * @file
 * Sound decoder used in engines:
 *  - sherlock (3DO version of Serrated Scalpel)
 */

#ifndef AUDIO_3DO_SDX2_H
#define AUDIO_3DO_SDX2_H

#include "common/scummsys.h"
#include "common/types.h"
#include "common/stream.h"

#include "audio/audiostream.h"

namespace Audio {

// amount of bytes to be used within the decoder classes as buffers
#define AUDIO_3DO_CACHE_SIZE 1024

// persistent spaces
struct audio_3DO_ADP4_PersistentSpace {
	int16 lastSample;
	int16 stepIndex;
};

struct audio_3DO_SDX2_PersistentSpace {
	int16 lastSample1;
	int16 lastSample2;
};

class Audio3DO_ADP4_Stream : public RewindableAudioStream {
public:
	Audio3DO_ADP4_Stream(Common::SeekableReadStream *stream, uint16 sampleRate, bool stereo, DisposeAfterUse::Flag disposeAfterUse, audio_3DO_ADP4_PersistentSpace *persistentSpace);

protected:
	const uint16 _sampleRate;
	const bool _stereo;

	Common::DisposablePtr<Common::SeekableReadStream> _stream;
	int32 _streamBytesLeft;

	void reset();
	bool rewind();
	bool endOfData() const { return (_stream->pos() >= _stream->size()); }
	bool isStereo() const { return _stereo; }
	int getRate() const { return _sampleRate; }

	int readBuffer(int16 *buffer, const int numSamples);

	bool _initialRead;
	audio_3DO_ADP4_PersistentSpace *_callerDecoderData;
	audio_3DO_ADP4_PersistentSpace _initialDecoderData;
	audio_3DO_ADP4_PersistentSpace _curDecoderData;

private:
	int16 decodeSample(byte compressedNibble);
};

class Audio3DO_SDX2_Stream : public RewindableAudioStream {
public:
	Audio3DO_SDX2_Stream(Common::SeekableReadStream *stream, uint16 sampleRate, bool stereo, DisposeAfterUse::Flag disposeAfterUse, audio_3DO_SDX2_PersistentSpace *persistentSpacePtr);

protected:
	const uint16 _sampleRate;
	const bool _stereo;

	Common::DisposablePtr<Common::SeekableReadStream> _stream;
	int32 _streamBytesLeft;

	void reset();
	bool rewind();
	bool endOfData() const { return (_stream->pos() >= _stream->size()); }
	bool isStereo() const { return _stereo; }
	int getRate() const { return _sampleRate; }

	int readBuffer(int16 *buffer, const int numSamples);

	bool _initialRead;
	audio_3DO_SDX2_PersistentSpace *_callerDecoderData;
	audio_3DO_SDX2_PersistentSpace _initialDecoderData;
	audio_3DO_SDX2_PersistentSpace _curDecoderData;
};

/**
 * Try to decode 3DO ADP4 data from the given seekable stream and create a SeekableAudioStream
 * from that data.
 *
 * @param stream			the SeekableReadStream from which to read the 3DO SDX2 data
 * @sampleRate				sample rate
 * @stereo					if it's stereo or mono
 * @audioLengthMSecsPtr		pointer to a uint32 variable, that is supposed to get the length of the audio in milliseconds
 * @disposeAfterUse			disposeAfterUse	whether to delete the stream after use
 * @persistentSpacePtr		pointer to the persistent space structure
 * @return					a new SeekableAudioStream, or NULL, if an error occurred
 */
RewindableAudioStream *make3DO_ADP4AudioStream(
	Common::SeekableReadStream *stream,
	uint16 sampleRate,
	bool stereo,
	uint32 *audioLengthMSecsPtr = NULL, // returns the audio length in milliseconds
	DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES,
	audio_3DO_ADP4_PersistentSpace *persistentSpacePtr = NULL
);

/**
 * Try to decode 3DO SDX2 data from the given seekable stream and create a SeekableAudioStream
 * from that data.
 *
 * @param stream			the SeekableReadStream from which to read the 3DO SDX2 data
 * @sampleRate				sample rate
 * @stereo					if it's stereo or mono
 * @audioLengthMSecsPtr		pointer to a uint32 variable, that is supposed to get the length of the audio in milliseconds
 * @disposeAfterUse			disposeAfterUse	whether to delete the stream after use
 * @persistentSpacePtr		pointer to the persistent space structure
 * @return					a new SeekableAudioStream, or NULL, if an error occurred
 */
RewindableAudioStream *make3DO_SDX2AudioStream(
	Common::SeekableReadStream *stream,
	uint16 sampleRate,
	bool stereo,
	uint32 *audioLengthMSecsPtr = NULL, // returns the audio length in milliseconds
	DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES,
	audio_3DO_SDX2_PersistentSpace *persistentSpacePtr = NULL
);

} // End of namespace Audio

#endif
