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

#include "audio/decoders/aac.h"

#ifdef USE_FAAD

#include "common/debug.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "audio/audiostream.h"

#include <neaacdec.h>

namespace Audio {

class AACStream : public AudioStream {
public:
	AACStream(Common::SeekableReadStream *stream,
	          DisposeAfterUse::Flag disposeStream,
	          Common::SeekableReadStream *extraData,
	          DisposeAfterUse::Flag disposeExtraData);
	~AACStream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const { return _inBufferPos >= _inBufferSize && !_remainingSamples; }
	bool isStereo() const { return _channels == 2; }
	int getRate() const { return _rate; }

private:
	NeAACDecHandle _handle;
	byte _channels;
	unsigned long _rate;

	byte *_inBuffer;
	uint32 _inBufferSize;
	uint32 _inBufferPos;

	int16 *_remainingSamples;
	uint32 _remainingSamplesSize;
	uint32 _remainingSamplesPos;

	void init(Common::SeekableReadStream *extraData);
};

AACStream::AACStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeStream,
		Common::SeekableReadStream *extraData, DisposeAfterUse::Flag disposeExtraData) {

	_remainingSamples = 0;
	_inBufferPos = 0;

	init(extraData);

	// Copy all the data to a pointer so it can be passed through
	// (At least MPEG-4 chunks shouldn't be large)
	_inBufferSize = stream->size();
	_inBuffer = new byte[_inBufferSize];
	stream->read(_inBuffer, _inBufferSize);

	if (disposeStream == DisposeAfterUse::YES)
		delete stream;

	if (disposeExtraData == DisposeAfterUse::YES)
		delete extraData;
}

AACStream::~AACStream() {
	NeAACDecClose(_handle);
	delete[] _inBuffer;
	delete[] _remainingSamples;
}

void AACStream::init(Common::SeekableReadStream *extraData) {
	// Open the library
	_handle = NeAACDecOpen();

	// Configure the library to our needs
	NeAACDecConfigurationPtr conf = NeAACDecGetCurrentConfiguration(_handle);
	conf->outputFormat = FAAD_FMT_16BIT; // We only support 16bit audio
	conf->downMatrix = 1;                // Convert from 5.1 to stereo if required
	NeAACDecSetConfiguration(_handle, conf);

	// Copy the extra data to a buffer
	extraData->seek(0);
	byte *extraDataBuf = new byte[extraData->size()];
	extraData->read(extraDataBuf, extraData->size());

	// Initialize with our extra data
	// NOTE: This code assumes the extra data is coming from an MPEG-4 file!
	int err = NeAACDecInit2(_handle, extraDataBuf, extraData->size(), &_rate, &_channels);
	delete[] extraDataBuf;

	if (err < 0)
		error("Could not initialize AAC decoder: %s", NeAACDecGetErrorMessage(err));
}

int AACStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;

	assert((numSamples % _channels) == 0);

	// Dip into our remaining samples pool if it's available
	if (_remainingSamples) {
		samples = MIN<int>(numSamples, _remainingSamplesSize - _remainingSamplesPos);

		memcpy(buffer, _remainingSamples + _remainingSamplesPos, samples * 2);
		_remainingSamplesPos += samples;

		if (_remainingSamplesPos == _remainingSamplesSize) {
			delete[] _remainingSamples;
			_remainingSamples = 0;
		}
	}

	// Decode until we have enough samples (or there's no more left)
	while (samples < numSamples && !endOfData()) {
		NeAACDecFrameInfo frameInfo;
		uint16 *decodedSamples = (uint16 *)NeAACDecDecode(_handle, &frameInfo, _inBuffer + _inBufferPos, _inBufferSize - _inBufferPos);

		if (frameInfo.error != 0)
			error("Failed to decode AAC frame: %s", NeAACDecGetErrorMessage(frameInfo.error));

		int decodedSampleSize = frameInfo.samples;
		int copySamples = (decodedSampleSize > (numSamples - samples)) ? (numSamples - samples) : decodedSampleSize;

		memcpy(buffer + samples, decodedSamples, copySamples * 2);
		samples += copySamples;

		// Copy leftover samples for use in a later readBuffer() call
		if (copySamples != decodedSampleSize) {
			_remainingSamplesSize = decodedSampleSize - copySamples;
			_remainingSamples = new int16[_remainingSamplesSize];
			_remainingSamplesPos = 0;
			memcpy(_remainingSamples, decodedSamples + copySamples, _remainingSamplesSize * 2);
		}

		_inBufferPos += frameInfo.bytesconsumed;
	}

	return samples;
}

// Factory function
AudioStream *makeAACStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeStream,
		Common::SeekableReadStream *extraData, DisposeAfterUse::Flag disposeExtraData) {

	return new AACStream(stream, disposeStream, extraData, disposeExtraData);
}

} // End of namespace Audio

#endif // #ifdef USE_FAAD
