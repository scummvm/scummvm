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
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"
#include "common/stream.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/raw.h"

namespace Audio {

// This used to be an inline template function, but
// buggy template function handling in MSVC6 forced
// us to go with the macro approach. So far this is
// the only template function that MSVC6 seemed to
// compile incorrectly. Knock on wood.
#define READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, ptr, isLE) \
	((is16Bit ? (isLE ? READ_LE_UINT16(ptr) : READ_BE_UINT16(ptr)) : (*ptr << 8)) ^ (isUnsigned ? 0x8000 : 0))


// TODO: Get rid of this
uint32 calculateSampleOffset(const Timestamp &where, int rate) {
	return where.convertToFramerate(rate).totalNumberOfFrames();
}



#pragma mark -
#pragma mark --- RawMemoryStream ---
#pragma mark -

/**
 * A simple raw audio stream, purely memory based. It operates on a single
 * block of data, which is passed to it upon creation.
 * Optionally supports looping the sound.
 *
 * Design note: This code tries to be as efficient as possible (without
 * resorting to assembly, that is). To this end, it is written as a template
 * class. This way the compiler can create optimized code for each special
 * case. This results in a total of 12 versions of the code being generated.
 */
template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
class RawMemoryStream : public SeekableAudioStream {
protected:
	const byte *_ptr;
	const byte *_end;
	const int _rate;
	const byte *_origPtr;
	const DisposeAfterUse::Flag _disposeAfterUse;
	const Timestamp _playtime;

public:
	RawMemoryStream(int rate, const byte *ptr, uint len, DisposeAfterUse::Flag autoFreeMemory)
	    : _ptr(ptr), _end(ptr+len), _rate(rate), _origPtr(ptr),
	      _disposeAfterUse(autoFreeMemory),
	      _playtime(0, len / (is16Bit ? 2 : 1) / (stereo ? 2 : 1), rate) {
	}

	virtual ~RawMemoryStream() {
		if (_disposeAfterUse == DisposeAfterUse::YES)
			free(const_cast<byte *>(_origPtr));
	}

	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const			{ return stereo; }
	bool endOfData() const			{ return _ptr >= _end; }

	int getRate() const				{ return _rate; }
	bool seek(const Timestamp &where);
	Timestamp getLength() const { return _playtime; }
};

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
int RawMemoryStream<stereo, is16Bit, isUnsigned, isLE>::readBuffer(int16 *buffer, const int numSamples) {
	int samples = numSamples;
	while (samples > 0 && _ptr < _end) {
		int len = MIN(samples, (int)(_end - _ptr) / (is16Bit ? 2 : 1));
		samples -= len;
		do {
			*buffer++ = READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, _ptr, isLE);
			_ptr += (is16Bit ? 2 : 1);
		} while (--len);
	}
	return numSamples-samples;
}

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
bool RawMemoryStream<stereo, is16Bit, isUnsigned, isLE>::seek(const Timestamp &where) {
	const uint8 *ptr = _origPtr + calculateSampleOffset(where, getRate()) * (is16Bit ? 2 : 1) * (stereo ? 2 : 1);
	if (ptr > _end) {
		_ptr = _end;
		return false;
	} else if (ptr == _end) {
		_ptr = _end;
		return true;
	} else {
		_ptr = ptr;
		return true;
	}
}

#pragma mark -
#pragma mark --- RawDiskStream ---
#pragma mark -



/**
 *  RawDiskStream.  This can stream raw PCM audio data from disk.  The
 *  function takes an pointer to an array of RawDiskStreamAudioBlock which defines the
 *  start position and length of each block of uncompressed audio in the stream.
 */
template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
class RawDiskStream : public SeekableAudioStream {

// Allow backends to override buffer size
#ifdef CUSTOM_AUDIO_BUFFER_SIZE
	static const int32 BUFFER_SIZE = CUSTOM_AUDIO_BUFFER_SIZE;
#else
	static const int32 BUFFER_SIZE = 16384;
#endif

protected:
	byte* _buffer;			///< Streaming buffer
	const byte *_ptr;		///< Pointer to current position in stream buffer
	const int _rate;		///< Sample rate of stream

	Timestamp _playtime;	///< Calculated total play time
	Common::SeekableReadStream *_stream;	///< Stream to read data from
	int32 _filePos;			///< Current position in stream
	int32 _diskLeft;		///< Samples left in stream in current block not yet read to buffer
	int32 _bufferLeft;		///< Samples left in buffer in current block
	const DisposeAfterUse::Flag _disposeAfterUse;		///< Indicates whether the stream object should be deleted when this RawDiskStream is destructed

	RawDiskStreamAudioBlock *_audioBlock;	///< Audio block list
	const int _audioBlockCount;		///< Number of blocks in _audioBlock
	int _currentBlock;		///< Current audio block number
public:
	RawDiskStream(int rate, DisposeAfterUse::Flag disposeStream, Common::SeekableReadStream *stream, RawDiskStreamAudioBlock *block, uint numBlocks)
		: _rate(rate), _playtime(0, rate), _stream(stream), _disposeAfterUse(disposeStream),
		  _audioBlockCount(numBlocks) {

		assert(numBlocks > 0);

		// Allocate streaming buffer
		if (is16Bit) {
			_buffer = (byte *)malloc(BUFFER_SIZE * sizeof(int16));
		} else {
			_buffer = (byte *)malloc(BUFFER_SIZE * sizeof(byte));
		}

		_ptr = _buffer;
		_bufferLeft = 0;

		// Copy audio block data to our buffer
		// TODO: Replace this with a Common::Array or Common::List to
		// make it a little friendlier.
		_audioBlock = new RawDiskStreamAudioBlock[numBlocks];
		memcpy(_audioBlock, block, numBlocks * sizeof(RawDiskStreamAudioBlock));

		// Set current buffer state, playing first block
		_currentBlock = 0;
		_filePos = _audioBlock[_currentBlock].pos;
		_diskLeft = _audioBlock[_currentBlock].len;

		// Add up length of all blocks in order to caluclate total play time
		int len = 0;
		for (int r = 0; r < _audioBlockCount; r++) {
			len += _audioBlock[r].len;
		}
		_playtime = Timestamp(0, len / (is16Bit ? 2 : 1) / (stereo ? 2 : 1), rate);
	}


	virtual ~RawDiskStream() {
		if (_disposeAfterUse == DisposeAfterUse::YES) {
			delete _stream;
		}

		delete[] _audioBlock;
		free(_buffer);
	}
	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const			{ return stereo; }
	bool endOfData() const			{ return (_currentBlock == _audioBlockCount - 1) && (_diskLeft == 0) && (_bufferLeft == 0); }

	int getRate() const			{ return _rate; }
	Timestamp getLength() const { return _playtime; }

	bool seek(const Timestamp &where);
};

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
int RawDiskStream<stereo, is16Bit, isUnsigned, isLE>::readBuffer(int16 *buffer, const int numSamples) {
	int oldPos = _stream->pos();
	bool restoreFilePosition = false;

	int samples = numSamples;

	while (samples > 0 && ((_diskLeft > 0 || _bufferLeft > 0) || (_currentBlock != _audioBlockCount - 1))  ) {
		// Output samples in the buffer to the output
		int len = MIN<int>(samples, _bufferLeft);
		samples -= len;
		_bufferLeft -= len;

		while (len > 0) {
			*buffer++ = READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, _ptr, isLE);
			_ptr += (is16Bit ? 2 : 1);
			len--;
		}

		// Have we now finished this block?  If so, read the next block
		if ((_bufferLeft == 0) && (_diskLeft == 0) && (_currentBlock != _audioBlockCount - 1)) {
			// Next block
			_currentBlock++;

			_filePos = _audioBlock[_currentBlock].pos;
			_diskLeft = _audioBlock[_currentBlock].len;
		}
			
		// Now read more data from disk if there is more to be read
		if ((_bufferLeft == 0) && (_diskLeft > 0)) {
			int32 readAmount = MIN(_diskLeft, BUFFER_SIZE);

			_stream->seek(_filePos, SEEK_SET);
			_stream->read(_buffer, readAmount * (is16Bit? 2: 1));

			// Amount of data in buffer is now the amount read in, and
			// the amount left to read on disk is decreased by the same amount
			_bufferLeft = readAmount;
			_diskLeft -= readAmount;
			_ptr = (byte *)_buffer;
			_filePos += readAmount * (is16Bit ? 2 : 1);

			// Set this flag now we've used the file, it restores it's
			// original position.
			restoreFilePosition = true;
		}
	}

	// In case calling code relies on the position of this stream staying 
	// constant, I restore the location if I've changed it.  This is probably
	// not necessary.
	if (restoreFilePosition) {
		_stream->seek(oldPos, SEEK_SET);
	}

	return numSamples - samples;
}

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
bool RawDiskStream<stereo, is16Bit, isUnsigned, isLE>::seek(const Timestamp &where) {
	const uint32 seekSample = calculateSampleOffset(where, getRate()) * (stereo ? 2 : 1);
	uint32 curSample = 0;

	// Search for the disk block in which the specific sample is placed
	_currentBlock = 0;
	while (_currentBlock < _audioBlockCount) {
		uint32 nextBlockSample = curSample + _audioBlock[_currentBlock].len;

		if (nextBlockSample > seekSample)
			break;

		curSample = nextBlockSample;
		++_currentBlock;
	}

	_filePos = 0;
	_diskLeft = 0;
	_bufferLeft = 0;

	if (_currentBlock == _audioBlockCount) {
		return ((seekSample - curSample) == (uint32)_audioBlock[_currentBlock - 1].len);
	} else {
		const uint32 offset = seekSample - curSample;

		_filePos = _audioBlock[_currentBlock].pos + offset * (is16Bit ? 2 : 1);
		_diskLeft = _audioBlock[_currentBlock].len - offset;

		return true;
	}
}

#pragma mark -
#pragma mark --- Raw stream factories ---
#pragma mark -

/* In the following, we use preprocessor / macro tricks to simplify the code
 * which instantiates the input streams. We used to use template functions for
 * this, but MSVC6 / EVC 3-4 (used for WinCE builds) are extremely buggy when it
 * comes to this feature of C++... so as a compromise we use macros to cut down
 * on the (source) code duplication a bit.
 * So while normally macro tricks are said to make maintenance harder, in this
 * particular case it should actually help it :-)
 */

#define MAKE_LINEAR(STEREO, UNSIGNED) \
		if (is16Bit) { \
			if (isLE) \
				return new RawMemoryStream<STEREO, true, UNSIGNED, true>(rate, ptr, len, autoFree); \
			else  \
				return new RawMemoryStream<STEREO, true, UNSIGNED, false>(rate, ptr, len, autoFree); \
		} else \
			return new RawMemoryStream<STEREO, false, UNSIGNED, false>(rate, ptr, len, autoFree)

SeekableAudioStream *makeRawMemoryStream(const byte *ptr, uint32 len,
		DisposeAfterUse::Flag autoFree,
		int rate, byte flags) {
	const bool isStereo   = (flags & Audio::FLAG_STEREO) != 0;
	const bool is16Bit    = (flags & Audio::FLAG_16BITS) != 0;
	const bool isUnsigned = (flags & Audio::FLAG_UNSIGNED) != 0;
	const bool isLE       = (flags & Audio::FLAG_LITTLE_ENDIAN) != 0;

	// Verify the buffer sizes are sane
	if (is16Bit && isStereo) {
		assert((len & 3) == 0);
	} else if (is16Bit || isStereo) {
		assert((len & 1) == 0);
	}

	if (isStereo) {
		if (isUnsigned) {
			MAKE_LINEAR(true, true);
		} else {
			MAKE_LINEAR(true, false);
		}
	} else {
		if (isUnsigned) {
			MAKE_LINEAR(false, true);
		} else {
			MAKE_LINEAR(false, false);
		}
	}
}


AudioStream *makeRawMemoryStream_OLD(const byte *ptr, uint32 len,
		DisposeAfterUse::Flag autoFree,
		int rate, byte flags,
		uint loopStart, uint loopEnd) {
	SeekableAudioStream *s = makeRawMemoryStream(ptr, len, autoFree, rate, flags);

	if ((flags & Audio::FLAG_LOOP) != 0) {
		const bool isStereo   = (flags & Audio::FLAG_STEREO) != 0;
		const bool is16Bit    = (flags & Audio::FLAG_16BITS) != 0;

		if (loopEnd == 0)
			loopEnd = len;
		assert(loopStart <= loopEnd);
		assert(loopEnd <= len);

		// Verify the buffer sizes are sane
		if (is16Bit && isStereo)
			assert((loopStart & 3) == 0 && (loopEnd & 3) == 0);
		else if (is16Bit || isStereo)
			assert((loopStart & 1) == 0 && (loopEnd & 1) == 0);

		const uint32 extRate = s->getRate() * (is16Bit ? 2 : 1) * (isStereo ? 2 : 1);

		return new SubLoopingAudioStream(s, 0, Timestamp(0, loopStart, extRate), Timestamp(0, loopEnd, extRate));
	} else {
		return s;
	}
}



#define MAKE_LINEAR_DISK(STEREO, UNSIGNED) \
		if (is16Bit) { \
			if (isLE) \
				return new RawDiskStream<STEREO, true, UNSIGNED, true>(rate, disposeStream, stream, block, numBlocks); \
			else  \
				return new RawDiskStream<STEREO, true, UNSIGNED, false>(rate, disposeStream, stream, block, numBlocks); \
		} else \
			return new RawDiskStream<STEREO, false, UNSIGNED, false>(rate, disposeStream, stream, block, numBlocks)


SeekableAudioStream *makeRawDiskStream(Common::SeekableReadStream *stream, RawDiskStreamAudioBlock *block, int numBlocks,
					int rate, byte flags, DisposeAfterUse::Flag disposeStream) {
	const bool isStereo   = (flags & Audio::FLAG_STEREO) != 0;
	const bool is16Bit    = (flags & Audio::FLAG_16BITS) != 0;
	const bool isUnsigned = (flags & Audio::FLAG_UNSIGNED) != 0;
	const bool isLE       = (flags & Audio::FLAG_LITTLE_ENDIAN) != 0;

	if (isStereo) {
		if (isUnsigned) {
			MAKE_LINEAR_DISK(true, true);
		} else {
			MAKE_LINEAR_DISK(true, false);
		}
	} else {
		if (isUnsigned) {
			MAKE_LINEAR_DISK(false, true);
		} else {
			MAKE_LINEAR_DISK(false, false);
		}
	}
}

} // End of namespace Audio
