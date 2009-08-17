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

#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/list.h"
#include "common/util.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/mp3.h"
#include "sound/vorbis.h"
#include "sound/flac.h"


// This used to be an inline template function, but
// buggy template function handling in MSVC6 forced
// us to go with the macro approach. So far this is
// the only template function that MSVC6 seemed to
// compile incorrectly. Knock on wood.
#define READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, ptr, isLE) \
	((is16Bit ? (isLE ? READ_LE_UINT16(ptr) : READ_BE_UINT16(ptr)) : (*ptr << 8)) ^ (isUnsigned ? 0x8000 : 0))


namespace Audio {

struct StreamFileFormat {
	/** Decodername */
	const char* decoderName;
	const char* fileExtension;
	/**
	 * Pointer to a function which tries to open a file of type StreamFormat.
	 * Return NULL in case of an error (invalid/nonexisting file).
	 */
	AudioStream* (*openStreamFile)(Common::SeekableReadStream *stream, bool disposeAfterUse,
					uint32 startTime, uint32 duration, uint numLoops);
};

static const StreamFileFormat STREAM_FILEFORMATS[] = {
	/* decoderName,		fileExt, openStreamFuntion */
#ifdef USE_FLAC
	{ "Flac",			".flac", makeFlacStream },
	{ "Flac",			".fla",  makeFlacStream },
#endif
#ifdef USE_VORBIS
	{ "Ogg Vorbis",		".ogg",  makeVorbisStream },
#endif
#ifdef USE_MAD
	{ "MPEG Layer 3",	".mp3",  makeMP3Stream },
#endif

	{ NULL, NULL, NULL } // Terminator
};

AudioStream* AudioStream::openStreamFile(const Common::String &basename, uint32 startTime, uint32 duration, uint numLoops) {
	AudioStream* stream = NULL;
	Common::File *fileHandle = new Common::File();

	for (int i = 0; i < ARRAYSIZE(STREAM_FILEFORMATS)-1 && stream == NULL; ++i) {
		Common::String filename = basename + STREAM_FILEFORMATS[i].fileExtension;
		fileHandle->open(filename);
		if (fileHandle->isOpen()) {
			// Create the stream object
			stream = STREAM_FILEFORMATS[i].openStreamFile(fileHandle, true, startTime, duration, numLoops);
			fileHandle = 0;
			break;
		}
	}

	delete fileHandle;

	if (stream == NULL) {
		debug(1, "AudioStream: Could not open compressed AudioFile %s", basename.c_str());
	}

	return stream;
}

#pragma mark -
#pragma mark --- LinearMemoryStream ---
#pragma mark -

inline int32 calculatePlayTime(int rate, int samples) {
	int32 seconds = samples / rate;
	int32 milliseconds = (1000 * (samples % rate)) / rate;
	return seconds * 1000 + milliseconds;
}

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
class LinearMemoryStream : public AudioStream {
protected:
	const byte *_ptr;
	const byte *_end;
	const byte *_loopPtr;
	const byte *_loopEnd;
	const int _rate;
	const byte *_origPtr;
	const int32 _playtime;

public:
	LinearMemoryStream(int rate, const byte *ptr, uint len, uint loopOffset, uint loopLen, bool autoFreeMemory)
		: _ptr(ptr), _end(ptr+len), _loopPtr(0), _loopEnd(0), _rate(rate), _playtime(calculatePlayTime(rate, len / (is16Bit ? 2 : 1) / (stereo ? 2 : 1))) {

		if (loopLen) {
			_loopPtr = _ptr + loopOffset;
			_loopEnd = _loopPtr + loopLen;
		}

		_origPtr = autoFreeMemory ? ptr : 0;
	}
	virtual ~LinearMemoryStream() {
		free(const_cast<byte *>(_origPtr));
	}
	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const			{ return stereo; }
	bool endOfData() const			{ return _ptr >= _end; }

	int getRate() const				{ return _rate; }
	int32 getTotalPlayTime() const	{ return _playtime; }
};

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
int LinearMemoryStream<stereo, is16Bit, isUnsigned, isLE>::readBuffer(int16 *buffer, const int numSamples) {
	int samples = numSamples;
	while (samples > 0 && _ptr < _end) {
		int len = MIN(samples, (int)(_end - _ptr) / (is16Bit ? 2 : 1));
		samples -= len;
		do {
			*buffer++ = READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, _ptr, isLE);
			_ptr += (is16Bit ? 2 : 1);
		} while (--len);
		// Loop, if looping was specified
		if (_loopPtr && _ptr >= _end) {
			_ptr = _loopPtr;
			_end = _loopEnd;
		}
	}
	return numSamples-samples;
}



#pragma mark -
#pragma mark --- LinearDiskStream ---
#pragma mark -



/**
 *  LinearDiskStream.  This can stream linear (PCM) audio from disk.  The
 *  function takes an pointer to an array of LinearDiskStreamAudioBlock which defines the
 *  start position and length of each block of uncompressed audio in the stream.
 */
template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
class LinearDiskStream : public AudioStream {

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

	int32 _playtime;		///< Calculated total play time
	Common::SeekableReadStream *_stream;	///< Stream to read data from
	int32 _filePos;			///< Current position in stream
	int32 _diskLeft;		///< Samples left in stream in current block not yet read to buffer
	int32 _bufferLeft;		///< Samples left in buffer in current block
	bool _disposeAfterUse;		///< If true, delete stream object when LinearDiskStream is destructed

	LinearDiskStreamAudioBlock *_audioBlock;	///< Audio block list
	int _audioBlockCount;		///< Number of blocks in _audioBlock
	int _currentBlock;		///< Current audio block number

	int _beginLoop;			///< Loop parameter, currently not implemented
	int _endLoop;			///< Loop parameter, currently not implemented


public:
	LinearDiskStream(int rate, uint beginLoop, uint endLoop, bool disposeStream, Common::SeekableReadStream *stream, LinearDiskStreamAudioBlock *block, uint numBlocks)
		: _rate(rate), _stream(stream), _beginLoop(beginLoop), _endLoop(endLoop), _disposeAfterUse(disposeStream),
		  _audioBlockCount(numBlocks) {

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
		_audioBlock = new LinearDiskStreamAudioBlock[numBlocks];
		memcpy(_audioBlock, block, numBlocks * sizeof(LinearDiskStreamAudioBlock));

		// Set current buffer state, playing first block
		_currentBlock = 0;
		_filePos = _audioBlock[_currentBlock].pos;
		_diskLeft = _audioBlock[_currentBlock].len;

		// Add up length of all blocks in order to caluclate total play time
		int len = 0;
		for (int r = 0; r < _audioBlockCount; r++) {
			len += _audioBlock[r].len;
		}
		_playtime = calculatePlayTime(rate, len / (is16Bit ? 2 : 1) / (stereo ? 2 : 1));
	}


	virtual ~LinearDiskStream() {
		if (_disposeAfterUse) {
			delete _stream;
		}

		delete[] _audioBlock;
		free(_buffer);
	}
	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const			{ return stereo; }
	bool endOfData() const			{ return (_currentBlock == _audioBlockCount - 1) && (_diskLeft == 0) && (_bufferLeft == 0); }

	int getRate() const			{ return _rate; }
	int32 getTotalPlayTime() const	{ return _playtime; }
};

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
int LinearDiskStream<stereo, is16Bit, isUnsigned, isLE>::readBuffer(int16 *buffer, const int numSamples) {
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
			_filePos += readAmount * (is16Bit? 2: 1);

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

	return numSamples-samples;
}



#pragma mark -
#pragma mark --- Input stream factory ---
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
				return new LinearMemoryStream<STEREO, true, UNSIGNED, true>(rate, ptr, len, loopOffset, loopLen, autoFree); \
			else  \
				return new LinearMemoryStream<STEREO, true, UNSIGNED, false>(rate, ptr, len, loopOffset, loopLen, autoFree); \
		} else \
			return new LinearMemoryStream<STEREO, false, UNSIGNED, false>(rate, ptr, len, loopOffset, loopLen, autoFree)

AudioStream *makeLinearInputStream(const byte *ptr, uint32 len, int rate, byte flags, uint loopStart, uint loopEnd) {
	const bool isStereo   = (flags & Audio::Mixer::FLAG_STEREO) != 0;
	const bool is16Bit    = (flags & Audio::Mixer::FLAG_16BITS) != 0;
	const bool isUnsigned = (flags & Audio::Mixer::FLAG_UNSIGNED) != 0;
	const bool isLE       = (flags & Audio::Mixer::FLAG_LITTLE_ENDIAN) != 0;
	const bool autoFree   = (flags & Audio::Mixer::FLAG_AUTOFREE) != 0;


	uint loopOffset = 0, loopLen = 0;
	if (flags & Audio::Mixer::FLAG_LOOP) {
		if (loopEnd == 0)
			loopEnd = len;
		assert(loopStart <= loopEnd);
		assert(loopEnd <= len);

		loopOffset = loopStart;
		loopLen = loopEnd - loopStart;
	}

	// Verify the buffer sizes are sane
	if (is16Bit && isStereo) {
		assert((len & 3) == 0 && (loopLen & 3) == 0);
	} else if (is16Bit || isStereo) {
		assert((len & 1) == 0 && (loopLen & 1) == 0);
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





#define MAKE_LINEAR_DISK(STEREO, UNSIGNED) \
		if (is16Bit) { \
			if (isLE) \
				return new LinearDiskStream<STEREO, true, UNSIGNED, true>(rate, loopStart, loopEnd, takeOwnership, &stream, block, numBlocks); \
			else  \
				return new LinearDiskStream<STEREO, true, UNSIGNED, false>(rate, loopStart, loopEnd, takeOwnership, &stream, block, numBlocks); \
		} else \
			return new LinearDiskStream<STEREO, false, UNSIGNED, false>(rate, loopStart, loopEnd, takeOwnership, &stream, block, numBlocks)


AudioStream *makeLinearDiskStream(Common::SeekableReadStream& stream, LinearDiskStreamAudioBlock* block, int numBlocks, int rate, byte flags, bool takeOwnership, uint loopStart, uint loopEnd) {
	const bool isStereo   = (flags & Audio::Mixer::FLAG_STEREO) != 0;
	const bool is16Bit    = (flags & Audio::Mixer::FLAG_16BITS) != 0;
	const bool isUnsigned = (flags & Audio::Mixer::FLAG_UNSIGNED) != 0;
	const bool isLE       = (flags & Audio::Mixer::FLAG_LITTLE_ENDIAN) != 0;


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




#pragma mark -
#pragma mark --- Appendable audio stream ---
#pragma mark -

struct Buffer {
	byte *start;
	byte *end;
};

/**
 * Wrapped memory stream.
 */
class BaseAppendableMemoryStream : public AppendableAudioStream {
protected:

	// A mutex to avoid access problems (causing e.g. corruption of
	// the linked list) in thread aware environments.
	Common::Mutex _mutex;

	// List of all queued buffers
	Common::List<Buffer> _bufferQueue;

	// Position in the front buffer, if any
	bool _finalized;
	const int _rate;
	byte *_pos;

	inline bool eosIntern() const { return _bufferQueue.empty(); };
public:
	BaseAppendableMemoryStream(int rate);
	~BaseAppendableMemoryStream();

	bool endOfStream() const	{ return _finalized && eosIntern(); }
	bool endOfData() const		{ return eosIntern(); }

	int getRate() const			{ return _rate; }

	void finish()				{ _finalized = true; }

	void queueBuffer(byte *data, uint32 size);
};

/**
 * Wrapped memory stream.
 */
template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
class AppendableMemoryStream : public BaseAppendableMemoryStream {
public:
	AppendableMemoryStream(int rate) : BaseAppendableMemoryStream(rate) {}

	bool isStereo() const		{ return stereo; }

	int readBuffer(int16 *buffer, const int numSamples);
};

BaseAppendableMemoryStream::BaseAppendableMemoryStream(int rate)
 : _finalized(false), _rate(rate), _pos(0) {

}

BaseAppendableMemoryStream::~BaseAppendableMemoryStream() {
	// Clear the queue
	Common::List<Buffer>::iterator iter;
	for (iter = _bufferQueue.begin(); iter != _bufferQueue.end(); ++iter)
		delete[] iter->start;
}

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
int AppendableMemoryStream<stereo, is16Bit, isUnsigned, isLE>::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);
	int samples = numSamples;
	while (samples > 0 && !eosIntern()) {
		Buffer buf = *_bufferQueue.begin();
		if (_pos == 0)
			_pos = buf.start;

		assert(buf.start <= _pos && _pos <= buf.end);
		const int samplesLeftInCurBuffer = buf.end - _pos;
		if (samplesLeftInCurBuffer == 0) {
			delete[] buf.start;
			_bufferQueue.erase(_bufferQueue.begin());
			_pos = 0;
			continue;
		}

		int len = MIN(samples, samplesLeftInCurBuffer / (is16Bit ? 2 : 1));
		samples -= len;
		do {
			*buffer++ = READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, _pos, isLE);
			_pos += (is16Bit ? 2 : 1);
		} while (--len);
	}

	return numSamples - samples;
}

void BaseAppendableMemoryStream::queueBuffer(byte *data, uint32 size) {
	Common::StackLock lock(_mutex);

/*
	// Verify the buffer size is sane
	if (is16Bit && stereo) {
		assert((size & 3) == 0);
	} else if (is16Bit || stereo) {
		assert((size & 1) == 0);
	}
*/
	// Verify that the stream has not yet been finalized (by a call to finish())
	assert(!_finalized);

	// Queue the buffer
	Buffer buf = {data, data+size};
	_bufferQueue.push_back(buf);


#if 0
	// Output some stats
	uint totalSize = 0;
	Common::List<Buffer>::iterator iter;
	for (iter = _bufferQueue.begin(); iter != _bufferQueue.end(); ++iter)
		totalSize += iter->end - iter->start;
	printf("AppendableMemoryStream::queueBuffer: added a %d byte buf, a total of %d bytes are queued\n",
				size, totalSize);
#endif
}


#define MAKE_WRAPPED(STEREO, UNSIGNED) \
		if (is16Bit) { \
			if (isLE) \
				return new AppendableMemoryStream<STEREO, true, UNSIGNED, true>(rate); \
			else  \
				return new AppendableMemoryStream<STEREO, true, UNSIGNED, false>(rate); \
		} else \
			return new AppendableMemoryStream<STEREO, false, UNSIGNED, false>(rate)

AppendableAudioStream *makeAppendableAudioStream(int rate, byte _flags) {
	const bool isStereo = (_flags & Audio::Mixer::FLAG_STEREO) != 0;
	const bool is16Bit = (_flags & Audio::Mixer::FLAG_16BITS) != 0;
	const bool isUnsigned = (_flags & Audio::Mixer::FLAG_UNSIGNED) != 0;
	const bool isLE       = (_flags & Audio::Mixer::FLAG_LITTLE_ENDIAN) != 0;

	if (isStereo) {
		if (isUnsigned) {
			MAKE_WRAPPED(true, true);
		} else {
			MAKE_WRAPPED(true, false);
		}
	} else {
		if (isUnsigned) {
			MAKE_WRAPPED(false, true);
		} else {
			MAKE_WRAPPED(false, false);
		}
	}
}


} // End of namespace Audio
