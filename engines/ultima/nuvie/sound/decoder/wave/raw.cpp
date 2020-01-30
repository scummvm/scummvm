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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/sound/decoders/raw.cpp $
 * $Id: raw.cpp 54385 2010-11-19 17:03:07Z fingolfin $
 *
 */

//#include <stdlib.h>
//#include <string.h>
#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/sound/mixer/types.h"
#include "decoder/wave/endian.h"
#include "decoder/wave/memstream.h"

#include "audiostream.h"
#include "audio/mixer.h"
#include "decoder/wave/raw.h"

#ifdef MIN
#undef MIN
#endif

template<typename T> inline T MIN(T a, T b)    {
	return (a < b) ? a : b;
}

namespace Audio {

// This used to be an inline template function, but
// buggy template function handling in MSVC6 forced
// us to go with the macro approach. So far this is
// the only template function that MSVC6 seemed to
// compile incorrectly. Knock on wood.
#define READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, ptr, isLE) \
	((is16Bit ? (isLE ? READ_LE_UINT16(ptr) : READ_BE_UINT16(ptr)) : (*ptr << 8)) ^ (isUnsigned ? 0x8000 : 0))


#pragma mark -
#pragma mark --- RawStream ---
#pragma mark -

/**
 * This is a stream, which allows for playing raw PCM data from a stream.
 * It also features playback of multiple blocks from a given stream.
 */
template<bool is16Bit, bool isUnsigned, bool isLE>
class RawStream : public SeekableAudioStream {
public:
	RawStream(int rate, bool stereo, DisposeAfterUse::Flag disposeStream, Common::SeekableReadStream *stream, const RawStreamBlockList &blocks)
		: _rate(rate), _isStereo(stereo), _playtime(0, rate), _stream(stream), _disposeAfterUse(disposeStream), _blocks(blocks), _curBlock(_blocks.begin()), _blockLeft(0), _buffer(0) {

		assert(_blocks.size() > 0);

		// Setup our buffer for readBuffer
		_buffer = new uint8[kSampleBufferLength * (is16Bit ? 2 : 1)];
		assert(_buffer);

		// Set current buffer state, playing first block
		_stream->seek(_curBlock->pos, SEEK_SET);

		// In case of an error we will stop (or rather
		// not start) stream playback.
		if (_stream->err()) {
			_blockLeft = 0;
			_curBlock = _blocks.end();
		} else {
			_blockLeft = _curBlock->len;
		}

		// Add up length of all blocks in order to caluclate total play time
		sint32 len = 0;
		for (RawStreamBlockList::const_iterator i = _blocks.begin(); i != _blocks.end(); ++i) {
			assert(i->len % (_isStereo ? 2 : 1) == 0);
			len += i->len;
		}

		_playtime = Timestamp(0, len / (_isStereo ? 2 : 1), rate);
	}

	~RawStream() {
		if (_disposeAfterUse == DisposeAfterUse::YES)
			delete _stream;

		delete[] _buffer;
	}

	int readBuffer(sint16 *buffer, const int numSamples);

	bool isStereo() const           {
		return _isStereo;
	}
	bool endOfData() const          {
		return (_curBlock == _blocks.end()) && (_blockLeft == 0);
	}

	int getRate() const         {
		return _rate;
	}
	Timestamp getLength() const {
		return _playtime;
	}

	bool seek(const Timestamp &where);
private:
	const int _rate;                               ///< Sample rate of stream
	const bool _isStereo;                          ///< Whether this is an stereo stream
	Timestamp _playtime;                           ///< Calculated total play time
	Common::SeekableReadStream *_stream;           ///< Stream to read data from
	const DisposeAfterUse::Flag _disposeAfterUse;  ///< Indicates whether the stream object should be deleted when this RawStream is destructed
	const RawStreamBlockList _blocks;              ///< Audio block list

	RawStreamBlockList::const_iterator _curBlock;  ///< Current audio block number
	sint32 _blockLeft;                              ///< How many bytes are still left in the current block

	/**
	 * Advance one block in the stream in case
	 * the current one is empty.
	 */
	void updateBlockIfNeeded();

	uint8 *_buffer;                                 ///< Buffer used in readBuffer
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

template<bool is16Bit, bool isUnsigned, bool isLE>
int RawStream<is16Bit, isUnsigned, isLE>::readBuffer(sint16 *buffer, const int numSamples) {
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
		const uint8 *src = _buffer;
		while (len-- > 0) {
			*buffer++ = READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, src, isLE);
			src += (is16Bit ? 2 : 1);
		}
	}

	return numSamples - samplesLeft;
}

template<bool is16Bit, bool isUnsigned, bool isLE>
int RawStream<is16Bit, isUnsigned, isLE>::fillBuffer(int maxSamples) {
	int bufferedSamples = 0;
	uint8 *dst = _buffer;

	// We can only read up to "kSampleBufferLength" samples
	// so we take this into consideration, when trying to
	// read up to maxSamples.
	maxSamples = MIN<int>(kSampleBufferLength, maxSamples);

	// We will only read up to maxSamples
	while (maxSamples > 0 && !endOfData()) {
		// Calculate how many samples we can safely read
		// from the current block.
		const int len = MIN<int>(maxSamples, _blockLeft);

		// Try to read all the sample data and update the
		// destination pointer.
		const int bytesRead = _stream->read(dst, len * (is16Bit ? 2 : 1));
		dst += bytesRead;

		// Calculate how many samples we actually read.
		const int samplesRead = bytesRead / (is16Bit ? 2 : 1);

		// Update all status variables
		bufferedSamples += samplesRead;
		maxSamples -= samplesRead;
		_blockLeft -= samplesRead;

		// In case of an error we will stop
		// stream playback.
		if (_stream->err()) {
			_blockLeft = 0;
			_curBlock = _blocks.end();
		}

		// Advance to the next block in case the current
		// one is already finished.
		updateBlockIfNeeded();
	}

	return bufferedSamples;
}

template<bool is16Bit, bool isUnsigned, bool isLE>
void RawStream<is16Bit, isUnsigned, isLE>::updateBlockIfNeeded() {
	// Have we now finished this block? If so, read the next block
	if (_blockLeft == 0 && _curBlock != _blocks.end()) {
		// Next block
		++_curBlock;

		// Check whether we reached the end of the stream
		// yet. In case we did not do this, we will just
		// setup the next block as new block.
		if (_curBlock != _blocks.end()) {
			_stream->seek(_curBlock->pos, SEEK_SET);

			// In case of an error we will stop
			// stream playback.
			if (_stream->err()) {
				_blockLeft = 0;
				_curBlock = _blocks.end();
			} else {
				_blockLeft = _curBlock->len;
			}
		}
	}
}

template<bool is16Bit, bool isUnsigned, bool isLE>
bool RawStream<is16Bit, isUnsigned, isLE>::seek(const Timestamp &where) {
	_blockLeft = 0;
	_curBlock = _blocks.end();

	if (where > _playtime)
		return false;

	const uint32 seekSample = convertTimeToStreamPos(where, getRate(), isStereo()).totalNumberOfFrames();
	uint32 curSample = 0;

	// Search for the disk block in which the specific sample is placed
	for (_curBlock = _blocks.begin(); _curBlock != _blocks.end(); ++_curBlock) {
		uint32 nextBlockSample = curSample + _curBlock->len;

		if (nextBlockSample > seekSample)
			break;

		curSample = nextBlockSample;
	}

	if (_curBlock == _blocks.end()) {
		return ((seekSample - curSample) == 0);
	} else {
		const uint32 offset = seekSample - curSample;

		_stream->seek(_curBlock->pos + offset * (is16Bit ? 2 : 1), SEEK_SET);

		// In case of an error we will stop
		// stream playback.
		if (_stream->err()) {
			_blockLeft = 0;
			_curBlock = _blocks.end();
		} else {
			_blockLeft = _curBlock->len - offset;
		}

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

#define MAKE_RAW_STREAM(UNSIGNED) \
	if (is16Bit) { \
		if (isLE) \
			return new RawStream<true, UNSIGNED, true>(rate, isStereo, disposeAfterUse, stream, blockList); \
		else  \
			return new RawStream<true, UNSIGNED, false>(rate, isStereo, disposeAfterUse, stream, blockList); \
	} else \
		return new RawStream<false, UNSIGNED, false>(rate, isStereo, disposeAfterUse, stream, blockList)

SeekableAudioStream *makeRawStream(Common::SeekableReadStream *stream,
                                   const RawStreamBlockList &blockList,
                                   int rate,
                                   uint8 flags,
                                   DisposeAfterUse::Flag disposeAfterUse) {
	const bool isStereo   = (flags & Audio::FLAG_STEREO) != 0;
	const bool is16Bit    = (flags & Audio::FLAG_16BITS) != 0;
	const bool isUnsigned = (flags & Audio::FLAG_UNSIGNED) != 0;
	const bool isLE       = (flags & Audio::FLAG_LITTLE_ENDIAN) != 0;

	if (blockList.empty()) {
		DEBUG(0, LEVEL_WARNING, "Empty block list passed to makeRawStream");
		if (disposeAfterUse == DisposeAfterUse::YES)
			delete stream;
		return 0;
	}

	if (isUnsigned) {
		MAKE_RAW_STREAM(true);
	} else {
		MAKE_RAW_STREAM(false);
	}
}

SeekableAudioStream *makeRawStream(Common::SeekableReadStream *stream,
                                   int rate, uint8 flags,
                                   DisposeAfterUse::Flag disposeAfterUse) {
	RawStreamBlockList blocks;
	RawStreamBlock block;
	block.pos = 0;

	const bool isStereo   = (flags & Audio::FLAG_STEREO) != 0;
	const bool is16Bit    = (flags & Audio::FLAG_16BITS) != 0;

	assert(stream->size() % ((is16Bit ? 2 : 1) * (isStereo ? 2 : 1)) == 0);

	block.len = stream->size() / (is16Bit ? 2 : 1);
	blocks.push_back(block);

	return makeRawStream(stream, blocks, rate, flags, disposeAfterUse);
}


SeekableAudioStream *makeRawStream(const uint8 *buffer, uint32 size,
                                   int rate, uint8 flags,
                                   DisposeAfterUse::Flag disposeAfterUse) {
	return makeRawStream(new Common::MemoryReadStream(buffer, size, disposeAfterUse), rate, flags, DisposeAfterUse::YES);
}

SeekableAudioStream *makeRawDiskStream_OLD(Common::SeekableReadStream *stream, RawStreamBlock *block, int numBlocks,
        int rate, uint8 flags, DisposeAfterUse::Flag disposeStream) {
	assert(numBlocks > 0);
	RawStreamBlockList blocks;
	for (int i = 0; i < numBlocks; ++i)
		blocks.push_back(block[i]);

	return makeRawStream(stream, blocks, rate, flags, disposeStream);
}

} // End of namespace Audio
