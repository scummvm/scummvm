/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "audio/audiostream.h"
#include "audio/decoders/xa.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/stream.h"
#include "grim/scx.h"

namespace Grim {

// I've only ever seen up to two
#define MAX_CHANNELS 2

class SCXStream : public Audio::RewindableAudioStream {
public:
	SCXStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse);
	~SCXStream();

	bool isStereo() const { return _channels == 2; }
	bool endOfData() const { return _xaStreams[0]->endOfData(); }
	int getRate() const { return _rate; }
	int readBuffer(int16 *buffer, const int numSamples);

	bool rewind();

private:
	int _channels;
	int _rate;
	uint16 _blockSize;

	Audio::RewindableAudioStream *_xaStreams[MAX_CHANNELS];
};

SCXStream::SCXStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	static const uint32 stereoChannelNames[MAX_CHANNELS] = { MKTAG('L', 'E', 'F', 'T'), MKTAG('R', 'G', 'H', 'T') };

	stream->readUint32BE(); // 'SCRX'
	stream->readUint32LE();

	_blockSize = stream->readUint16LE();
	/* totalBlockSize = */ stream->readUint16LE();

	if (_blockSize & 0xf)
		error("Bad SCX block size %04x", _blockSize);

	// Base our channel count based off the block size
	_channels = (_blockSize == 0) ? 1 : 2;

	stream->skip(12);

	uint32 channelSize[MAX_CHANNELS];
	for (int i = 0; i < _channels; i++) {
		uint32 tag = stream->readUint32BE();

		if (isStereo()) {
			if (tag != stereoChannelNames[i])
				error("Bad stereo channel tag found '%s'", tag2str(tag));
		} else if (tag != MKTAG('M', 'O', 'N', 'O'))
			error("Bad mono channel tag found '%s'", tag2str(tag));

		channelSize[i] = stream->readUint32LE();
	}

	stream->seek(0x80);

	uint32 leftRate = 0, rightRate = 0;
	for (int i = 0; i < _channels; i++) {
		if (stream->readUint32BE() != MKTAG('V', 'A', 'G', 'p'))
			error("Bad VAG header");

		/* uint32 version = */ stream->readUint32BE();
		stream->readUint32BE();
		stream->readUint32BE();

		if (i == 0)
			leftRate = stream->readUint32BE();
		else
			rightRate = stream->readUint32BE();

		stream->skip(12); // skip useless info
		stream->skip(16); // skip name
		stream->skip(16); // skip zeroes
	}

	if (isStereo() && leftRate != rightRate)
		error("Mismatching SCX rates");

	_rate = leftRate;

	if (isStereo()) {
		// TODO: Make XAStream allow for appending data (similar to how ScummVM
		// handles AAC/QDM2. For now, we de-interleave the XA ADPCM data and then
		// re-interleave in readBuffer().
		// Of course, in doing something that does better streaming, it would
		// screw up the XA loop points. So, I'm not really sure what is best atm.
		byte *leftOut = (byte*)malloc(channelSize[0]);
		byte *rightOut = (byte*)malloc(channelSize[1]);
		Common::MemoryWriteStream *leftStream = new Common::MemoryWriteStream(leftOut, channelSize[0]);
		Common::MemoryWriteStream *rightStream = new Common::MemoryWriteStream(rightOut, channelSize[1]);
		byte *buf = new byte[_blockSize];

		while (stream->pos() < stream->size()) {
			stream->read(buf, _blockSize);
			leftStream->write(buf, _blockSize);
			stream->read(buf, _blockSize);
			rightStream->write(buf, _blockSize);
		}

		_xaStreams[0] = Audio::makeXAStream(new Common::MemoryReadStream(leftOut, channelSize[0], DisposeAfterUse::YES), _rate);
		_xaStreams[1] = Audio::makeXAStream(new Common::MemoryReadStream(rightOut, channelSize[1], DisposeAfterUse::YES), _rate);

		delete[] buf;
		delete leftStream;
		delete rightStream;
	} else {
		_xaStreams[0] = Audio::makeXAStream(stream->readStream(channelSize[0]), _rate);
		_xaStreams[1] = 0;
	}

	if (disposeAfterUse == DisposeAfterUse::YES)
		delete stream;
}

SCXStream::~SCXStream() {
	for (int i = 0; i < MAX_CHANNELS; i++)
		delete _xaStreams[i];
}

int SCXStream::readBuffer(int16 *buffer, const int numSamples) {
	if (isStereo()) {
		// Needs to be divisible by the channel count
		assert((numSamples % 2) == 0);

		// TODO: As per above, this probably should do more actual streaming

		// Decode enough data from each channel
		int samplesPerChannel = numSamples / 2;
		int16 *leftSamples = new int16[samplesPerChannel];
		int16 *rightSamples = new int16[samplesPerChannel];

		int samplesDecodedLeft = _xaStreams[0]->readBuffer(leftSamples, samplesPerChannel);
		int samplesDecodedRight = _xaStreams[1]->readBuffer(rightSamples, samplesPerChannel);
		assert(samplesDecodedLeft == samplesDecodedRight);

		// Now re-interleave the data
		int samplesDecoded = 0;
		int16 *leftSrc = leftSamples, *rightSrc = rightSamples;
		for (; samplesDecoded < numSamples; samplesDecoded += 2) {
			*buffer++ = *leftSrc++;
			*buffer++ = *rightSrc++;
		}

		delete[] leftSamples;
		delete[] rightSamples;
		return samplesDecoded;
	}

	// Just read from the stream directly for mono
	return _xaStreams[0]->readBuffer(buffer, numSamples);
}

bool SCXStream::rewind() {
	if (!_xaStreams[0]->rewind())
		return false;

	return !isStereo() || _xaStreams[1]->rewind();
}

Audio::RewindableAudioStream *makeSCXStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	if (stream->readUint32BE() != MKTAG('S', 'C', 'R', 'X')) {
		delete stream;
		return 0;
	}

	stream->seek(0);
	return new SCXStream(stream, disposeAfterUse);
}

} // End of namespace Grim
