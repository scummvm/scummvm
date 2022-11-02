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

/**
 * @file
 * Sound decoder used in engines:
 *  - agos
 *  - chewy (subclass)
 *  - kyra
 *  - saga
 *  - scumm
 *  - touche
 */

#ifndef AUDIO_VOC_H
#define AUDIO_VOC_H

#include "audio/audiostream.h"

#include "common/list.h"
#include "common/scummsys.h"
#include "common/types.h"

namespace Common {
class ReadStream;
class SeekableReadStream;
}

namespace Audio {

class AudioStream;
class SeekableAudioStream;


#include "common/pack-start.h"	// START STRUCT PACKING

struct VocFileHeader {
	uint8 desc[20];
	uint16 datablock_offset;
	uint16 version;
	uint16 id;
} PACKED_STRUCT;

struct VocBlockHeader {
	uint8 blocktype;
	uint8 size[3];
	uint8 sr;
	uint8 pack;
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

class VocStream : public SeekableAudioStream {
public:
	VocStream(Common::SeekableReadStream *stream, bool isUnsigned, DisposeAfterUse::Flag disposeAfterUse);
	virtual ~VocStream();

	int readBuffer(int16 *buffer, const int numSamples) override;

	bool isStereo() const override { return false; }

	int getRate() const override { return _rate; }

	bool endOfData() const override { return (_curBlock == _blocks.end()) && (_blockLeft == 0); }

	bool seek(const Timestamp &where) override;

	Timestamp getLength() const override { return _length; }

protected:
	void preProcess();

	Common::SeekableReadStream *const _stream;
	const DisposeAfterUse::Flag _disposeAfterUse;

	const bool _isUnsigned;

	int _rate;
	Timestamp _length;

	struct Block {
		uint8 code;
		uint32 length;

		union {
			struct {
				uint32 offset;
				int rate;
				int samples;
			} sampleBlock;

			struct {
				int count;
			} loopBlock;
		};
	};

	typedef Common::List<Block> BlockList;
	BlockList _blocks;

	BlockList::const_iterator _curBlock;
	uint32 _blockLeft;

	/**
	 * Advance one block in the stream in case
	 * the current one is empty.
	 */
	void updateBlockIfNeeded();

	// Do some internal buffering for systems with really slow slow disk i/o
	enum {
		/**
		 * How many samples we can buffer at once.
		 *
		 * TODO: Check whether this size suffices
		 * for systems with slow disk I/O.
		 */
		kSampleBufferLength = 2048
	};
	byte _buffer[kSampleBufferLength];

	/**
	 * Fill the temporary sample buffer used in readBuffer.
	 *
	 * @param maxSamples Maximum samples to read.
	 * @return actual count of samples read.
	 */
	int fillBuffer(int maxSamples);
};

/**
 * Take a sample rate parameter as it occurs in a VOC sound header, and
 * return the corresponding sample frequency.
 *
 * This method has special cases for the standard rates of 11025 and 22050 kHz,
 * which due to limitations of the format, cannot be encoded exactly in a VOC
 * file. As a consequence, many game files have sound data sampled with those
 * rates, but the VOC marks them incorrectly as 11111 or 22222 kHz. This code
 * works around that and "unrounds" the sampling rates.
 */
extern int getSampleRateFromVOCRate(int vocSR);

/**
 * Try to load a VOC from the given seekable stream and create an AudioStream
 * from that data. Currently this function only supports uncompressed raw PCM
 * data.
 *
 * This does not use any of the looping features of VOC files!
 */
SeekableAudioStream *makeVOCStream(Common::SeekableReadStream *stream, byte flags, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::NO);

} // End of namespace Audio

#endif
