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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/endian.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "audio/audiostream.h"

#include "engines/zvision/zork_raw.h"

namespace ZVision {

#pragma mark -
#pragma mark --- RawZorkStream ---
#pragma mark -

/**
 * This is a stream, which allows for playing raw PCM data from a stream.
 */
class RawZorkStream : public Audio::SeekableAudioStream {
public:
	RawZorkStream(int rate, DisposeAfterUse::Flag disposeStream, Common::SeekableReadStream *stream)
		: _rate(rate), _playtime(0, rate), _stream(stream, disposeStream), _endOfData(false), _buffer(0) {
		// Setup our buffer for readBuffer
		_buffer = new byte[kSampleBufferLength];
		assert(_buffer);

		// Calculate the total playtime of the stream
		_playtime = Audio::Timestamp(0, _stream->size() / 2 / 1, rate);
	}

	~RawZorkStream() {
		delete[] _buffer;
	}

	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const  { return true; }
	bool endOfData() const { return _endOfData; }

	int getRate() const         { return _rate; }
	Audio::Timestamp getLength() const { return _playtime; }

	bool seek(const Audio::Timestamp &where);
private:
	const int _rate;                                           ///< Sample rate of stream
	Audio::Timestamp _playtime;                                       ///< Calculated total play time
	Common::DisposablePtr<Common::SeekableReadStream> _stream; ///< Stream to read data from
	bool _endOfData;                                           ///< Whether the stream end has been reached

	byte *_buffer;                                             ///< Buffer used in readBuffer
	enum {
		/**
		 * How many samples we can buffer at once.
		 *
		 * TODO: Check whether this size suffices
		 * for systems with slow disk I/O.
		 */
		kSampleBufferLength = 2048
	};

	/**
	 * Fill the temporary sample buffer used in readBuffer.
	 *
	 * @param maxSamples Maximum samples to read.
	 * @return actual count of samples read.
	 */
	int fillBuffer(int maxSamples);
};

int RawZorkStream::readBuffer(int16 *buffer, const int numSamples) {
	int samplesLeft = numSamples;

	while (samplesLeft > 0) {
		// Try to read up to "samplesLeft" samples.
		int len = fillBuffer(samplesLeft);

		// In case we were not able to read any samples
		// we will stop reading here.
		if (!len)
			break;

		// Adjust the samples left to read.
		samplesLeft -= len;

		// Copy the data to the caller's buffer.
		const byte *src = _buffer;
		while (len-- > 0) {
			if (*src < 128)
				*buffer++ = ((128 - *src) << 8) ^ 0x8000;
			else
				*buffer++ = (*src << 8) ^ 0x8000;
			src++;
		}
	}

	return numSamples - samplesLeft;
}

int RawZorkStream::fillBuffer(int maxSamples) {
	int bufferedSamples = 0;
	byte *dst = _buffer;

	// We can only read up to "kSampleBufferLength" samples
	// so we take this into consideration, when trying to
	// read up to maxSamples.
	maxSamples = MIN<int>(kSampleBufferLength, maxSamples);

	// We will only read up to maxSamples
	while (maxSamples > 0 && !endOfData()) {
		// Try to read all the sample data and update the
		// destination pointer.
		const int bytesRead = _stream->read(dst, maxSamples);
		dst += bytesRead;

		// Calculate how many samples we actually read.
		const int samplesRead = bytesRead;

		// Update all status variables
		bufferedSamples += samplesRead;
		maxSamples -= samplesRead;

		// We stop stream playback, when we reached the end of the data stream.
		// We also stop playback when an error occures.
		if (_stream->pos() == _stream->size() || _stream->err() || _stream->eos())
			_endOfData = true;
	}

	return bufferedSamples;
}

bool RawZorkStream::seek(const Audio::Timestamp &where) {
	_endOfData = true;

	if (where > _playtime)
		return false;

	const uint32 seekSample = convertTimeToStreamPos(where, getRate(), isStereo()).totalNumberOfFrames();
	_stream->seek(seekSample, SEEK_SET);

	// In case of an error we will not continue stream playback.
	if (!_stream->err() && !_stream->eos() && _stream->pos() != _stream->size())
		_endOfData = false;

	return true;
}

#pragma mark -
#pragma mark --- Raw stream factories ---
#pragma mark -

Audio::SeekableAudioStream *makeRawZorkStream(Common::SeekableReadStream *stream,
                                   int rate,
                                   DisposeAfterUse::Flag disposeAfterUse) {
	assert(stream->size() % 2 == 0);
	return new RawZorkStream(rate, disposeAfterUse, stream);
}

Audio::SeekableAudioStream *makeRawZorkStream(const byte *buffer, uint32 size,
                                   int rate,
                                   DisposeAfterUse::Flag disposeAfterUse) {
	return makeRawZorkStream(new Common::MemoryReadStream(buffer, size, disposeAfterUse), rate, DisposeAfterUse::YES);
}

} // End of namespace Audio
