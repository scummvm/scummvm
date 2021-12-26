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

#include "audio/decoders/g711.h"

#include "audio/audiostream.h"
#include "common/stream.h"
#include "common/util.h"

/* from g711.c by SUN microsystems (unrestricted use) */

#define         SIGN_BIT        (0x80)      /* Sign bit for a A-law byte. */
#define         QUANT_MASK      (0xf)       /* Quantization field mask. */
#define         SEG_SHIFT       (4)         /* Left shift for segment number. */
#define         SEG_MASK        (0x70)      /* Segment field mask. */
#define         BIAS            (0x84)      /* Bias for linear code. */

namespace Audio {

/**
 * Logarithmic PCM (G.711)
 * https://en.wikipedia.org/wiki/G.711
 * https://wiki.multimedia.cx/index.php/PCM#Logarithmic_PCM
 */
class G711AudioStream : public SeekableAudioStream {
	Common::DisposablePtr<Common::SeekableReadStream> _stream;
	const int _rate;
	const int _channels;

protected:
	virtual int16 decodeSample(uint8 val) = 0;

public:
	G711AudioStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate, int channels) :
		_stream(stream, disposeAfterUse),
		_rate(rate),
		_channels(channels) {
	}

	int readBuffer(int16 *buffer, const int numSamples) override {
		int samples;

		for (samples = 0; samples < numSamples; samples++) {
			uint8 val = _stream->readByte();
			if (endOfData())
				break;
			buffer[samples] = decodeSample(val);
		}

		return samples;
	}

	bool isStereo() const override { return (_channels == 2); }
	int getRate() const override { return _rate; }
	bool endOfData() const override { return _stream->eos(); }
	bool seek(const Timestamp &where) override {
		const uint32 seekSample = convertTimeToStreamPos(where, getRate(), isStereo()).totalNumberOfFrames();
		return _stream->seek(seekSample, SEEK_SET);
	}
	Timestamp getLength() const override {
		return Timestamp(0, _stream->size() / _channels, _rate);
	}
};

class G711ALawStream : public G711AudioStream {
	int16 decodeSample(uint8 val) override {
		val ^= 0x55;

		int t = val & QUANT_MASK;
		int seg = ((unsigned)val & SEG_MASK) >> SEG_SHIFT;
		if (seg)
			t = (t + t + 1 + 32) << (seg + 2);
		else
			t = (t + t + 1) << 3;

		return (val & SIGN_BIT) ? t : -t;
	}
	
public:
	G711ALawStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate, int channels) :
		G711AudioStream(stream, disposeAfterUse, rate, channels) {
	}
};

SeekableAudioStream *makeALawStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate, int channels) {
	return new G711ALawStream(stream, disposeAfterUse, rate, channels);
}

class G711MuLawStream : public G711AudioStream {
	int16 decodeSample(uint8 val) override {
		val = ~val;

		int t = ((val & QUANT_MASK) << 3) + BIAS;
		t <<= ((unsigned)val & SEG_MASK) >> SEG_SHIFT;

		return (val & SIGN_BIT) ? (BIAS - t) : (t - BIAS);
	}
	
public:
	G711MuLawStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate, int channels) :
		G711AudioStream(stream, disposeAfterUse, rate, channels) {
	}
};

SeekableAudioStream *makeMuLawStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate, int channels) {
	return new G711MuLawStream(stream, disposeAfterUse, rate, channels);
}

} // End of namespace Audio
