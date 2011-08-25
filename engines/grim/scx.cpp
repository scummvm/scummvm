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

#include "audio/audiostream.h"
#include "audio/decoders/vag.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/stream.h"
#include "grim/scx.h"

namespace Grim {

// I've only ever seen two (LEFT and RGHT)
#define NUM_CHANNELS 2

class SCXStream : public Audio::RewindableAudioStream {
public:
	SCXStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse);
	~SCXStream();

	bool isStereo() const { return true; }
	bool endOfData() const { return _xaStreams[0]->endOfData(); }
	int getRate() const { return _rate; }
	int readBuffer(int16 *buffer, const int numSamples);

	bool rewind();

private:
	int _rate;
	uint16 _blockSize;

	Audio::RewindableAudioStream *_xaStreams[NUM_CHANNELS];
};

SCXStream::SCXStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	static const uint32 channelNames[NUM_CHANNELS] = { MKTAG('L', 'E', 'F', 'T'), MKTAG('R', 'G', 'H', 'T') };

	stream->readUint32BE(); // 'SCRX'
	stream->readUint32LE();

	_blockSize = stream->readUint16LE();
	/* totalBlockSize = */ stream->readUint16LE();

	if (_blockSize & 0xf)
		error("Bad SCX block size %04x", _blockSize);

	stream->skip(12);

	for (int i = 0; i < NUM_CHANNELS; i++) {
		uint32 tag = stream->readUint32BE();
		if (tag != channelNames[i])
			error("Bad channel tag found '%s'", tag2str(tag));

		/* uint32 channelSize = */ stream->readUint32LE();
	}

	stream->skip(88);

	uint32 leftRate = 0, rightRate = 0;
	for (int i = 0; i < NUM_CHANNELS; i++) {
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

	if (leftRate != rightRate)
		error("Mismatching SCX rates");

	_rate = leftRate;

	// TODO: Make XAStream allow for appending data (similar to how ScummVM
	// handles AAC/QDM2. For now, we de-interleave the XA ADPCM data and then
	// re-interleave in readBuffer().
	Common::MemoryWriteStreamDynamic *leftOut = new Common::MemoryWriteStreamDynamic();
	Common::MemoryWriteStreamDynamic *rightOut = new Common::MemoryWriteStreamDynamic();
	byte *buf = new byte[_blockSize];

	while (stream->pos() < stream->size()) {
		stream->read(buf, _blockSize);
		leftOut->write(buf, _blockSize);
		stream->read(buf, _blockSize);
		rightOut->write(buf, _blockSize);
	}

	_xaStreams[0] = Audio::makeVagStream(new Common::MemoryReadStream(leftOut->getData(), leftOut->size(), DisposeAfterUse::YES), _rate);
	_xaStreams[1] = Audio::makeVagStream(new Common::MemoryReadStream(rightOut->getData(), rightOut->size(), DisposeAfterUse::YES), _rate);

	delete[] buf;
	delete leftOut;
	delete rightOut;

	if (disposeAfterUse == DisposeAfterUse::YES)
		delete stream;
}

SCXStream::~SCXStream() {
	for (int i = 0; i < NUM_CHANNELS; i++)
		delete _xaStreams[i];
}

int SCXStream::readBuffer(int16 *buffer, const int numSamples) {
	// Needs to be divisible by NUM_CHANNELS
	assert((numSamples % NUM_CHANNELS) == 0);

	// TODO: As per above, this should do more actual streaming

	// Decode enough data from each channel
	int samplesPerChannel = numSamples / NUM_CHANNELS;
	int16 *leftSamples = new int16[samplesPerChannel];
	int16 *rightSamples = new int16[samplesPerChannel];

	int samplesDecodedLeft = _xaStreams[0]->readBuffer(leftSamples, samplesPerChannel);
	int samplesDecodedRight = _xaStreams[1]->readBuffer(rightSamples, samplesPerChannel);
	assert(samplesDecodedLeft == samplesDecodedRight);

	// Now re-interleave the data
	int samplesDecoded = 0;
	int16 *leftSrc = leftSamples, *rightSrc = rightSamples;
	for (; samplesDecoded < numSamples; samplesDecoded += NUM_CHANNELS) {
		*buffer++ = *leftSrc++;
		*buffer++ = *rightSrc++;
	}

	delete[] leftSamples;
	delete[] rightSamples;
	return samplesDecoded;
}

bool SCXStream::rewind() {
	return _xaStreams[0]->rewind() && _xaStreams[1]->rewind();
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
