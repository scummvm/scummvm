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

#include "common/inttypes.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "audio/audiostream.h"
#include "audio/decoders/ac3.h"
#include "audio/decoders/raw.h"

extern "C" {
#include <a52dec/a52.h>
}

namespace Audio {

class AC3Stream : public PacketizedAudioStream {
public:
	AC3Stream(double decibel);
	~AC3Stream();

	bool init(Common::SeekableReadStream &firstPacket);
	void deinit();

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) { return _audStream->readBuffer(buffer, numSamples); }
	bool isStereo() const { return _audStream->isStereo(); }
	int getRate() const { return _audStream->getRate(); }
	bool endOfData() const { return _audStream->endOfData(); }
	bool endOfStream() const { return _audStream->endOfStream(); }

	// PacketizedAudioStream API
	void queuePacket(Common::SeekableReadStream *data);
	void finish() { _audStream->finish(); }

private:
	Common::ScopedPtr<QueuingAudioStream> _audStream;
	a52_state_t *_a52State;
	uint32 _frameSize;
	byte _inBuf[4096];
	byte *_inBufPtr;
	int _flags;
	int _sampleRate;
	double _audioGain;
};

AC3Stream::AC3Stream(double decibel) : _a52State(0), _frameSize(0), _inBufPtr(0), _flags(0), _sampleRate(0) {
	_audioGain = pow(2, decibel / 6);
}

AC3Stream::~AC3Stream() {
	deinit();
}

enum {
	HEADER_SIZE = 7
};

bool AC3Stream::init(Common::SeekableReadStream &firstPacket) {
	deinit();

	// In theory, I should pass mm_accel() to a52_init(), but I don't know
        // where that's supposed to be defined.
	_a52State = a52_init(0);

	// Go through the header to find sync
	byte buf[HEADER_SIZE];
	_sampleRate = -1;

	for (uint i = 0; i < firstPacket.size() - sizeof(buf); i++) {
		int flags, bitRate;
		firstPacket.seek(i);
		firstPacket.read(buf, sizeof(buf));

		if (a52_syncinfo(buf, &flags, &_sampleRate, &bitRate) > 0)
			break;
	}

	// Ensure we have a valid sample rate
	if (_sampleRate <= 0) {
		deinit();
		return false;
	}

	_audStream.reset(makeQueuingAudioStream(_sampleRate, true));
	_inBufPtr = _inBuf;
	_flags = 0;
	_frameSize = 0;
	return true;
}

void AC3Stream::deinit() {
	if (!_a52State)
		return;

	_audStream.reset();
	a52_free(_a52State);
	_a52State = 0;
}

void AC3Stream::queuePacket(Common::SeekableReadStream *data) {
	Common::ScopedPtr<Common::SeekableReadStream> packet(data);

	while (packet->pos() < packet->size()) {
		uint32 leftSize = packet->size() - packet->pos();
		uint32 len = _inBufPtr - _inBuf;
		if (_frameSize == 0) {
			// No header seen: find one
			len = HEADER_SIZE - len;
			if (len > leftSize)
				len = leftSize;
			packet->read(_inBufPtr, len);
			leftSize -= len;
			_inBufPtr += len;
			if ((_inBufPtr - _inBuf) == HEADER_SIZE) {
				int sampleRate, bitRate;
				len = a52_syncinfo(_inBuf, &_flags, &sampleRate, &bitRate);
				if (len == 0) {
					memmove(_inBuf, _inBuf + 1, HEADER_SIZE - 1);
					_inBufPtr--;
				} else {
					_frameSize = len;
				}
			}
		} else if (len < _frameSize) {
			len = _frameSize - len;
			if (len > leftSize)
				len = leftSize;

			assert(len < sizeof(_inBuf) - (_inBufPtr - _inBuf));
			packet->read(_inBufPtr, len);
			leftSize -= len;
			_inBufPtr += len;
		} else {
			// TODO: Eventually support more than just stereo max
			int flags = A52_STEREO | A52_ADJUST_LEVEL;
			sample_t level = 32767 * _audioGain;

			if (a52_frame(_a52State, _inBuf, &flags, &level, 0) != 0)
				error("Frame fail");

			int16 *outputBuffer = (int16 *)malloc(6 * 256 * 2 * 2);
			int16 *outputPtr = outputBuffer;
			int outputLength = 0;
			for (int i = 0; i < 6; i++) {
				if (a52_block(_a52State) == 0) {
					sample_t *samples = a52_samples(_a52State);
					for (int j = 0; j < 256; j++) {
						*outputPtr++ = (int16)CLIP<sample_t>(samples[j], -32768, 32767);
						*outputPtr++ = (int16)CLIP<sample_t>(samples[j + 256], -32768, 32767);
					}

					outputLength += 1024;
				}
			}

			if (outputLength > 0) {
				flags = FLAG_STEREO | FLAG_16BITS;

#ifdef SCUMM_LITTLE_ENDIAN
				flags |= FLAG_LITTLE_ENDIAN;
#endif

				_audStream->queueBuffer((byte *)outputBuffer, outputLength, DisposeAfterUse::YES, flags);
			}

			_inBufPtr = _inBuf;
			_frameSize = 0;
		}
	}
}

PacketizedAudioStream *makeAC3Stream(Common::SeekableReadStream &firstPacket, double decibel) {
	Common::ScopedPtr<AC3Stream> stream(new AC3Stream(decibel));
	if (!stream->init(firstPacket))
		return 0;

	return stream.release();
}

} // End of namespace Audio

