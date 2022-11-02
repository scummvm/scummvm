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

#include "common/stream.h"
#include "common/util.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"
#include "audio/decoders/xan_dpcm.h"


namespace Audio {

/**
 * Implements the Xan DPCM decoder used in Crusader: No Regret and Wing
 * Commander IV movies.  Implementation based on the description on the
 * MultiMedia Wiki:
 * https://wiki.multimedia.cx/index.php/Xan_DPCM
 */
class Xan_DPCMStream : public Audio::AudioStream {
public:
	Xan_DPCMStream(int rate, int channels, Common::SeekableReadStream *data) : _rate(rate), _channels(channels), _data(data) {
		assert(channels == 1 || channels == 2);
		_pred[0] = data->readSint16LE();
		if (channels == 2)
			_pred[1] = data->readSint16LE();
		_shift[0] = 4;
		_shift[1] = 4;
	};

	int readBuffer(int16 *buffer, const int numSamples) override {
		int i = 0;
		for (; i < numSamples; i++) {
			int32 *pshift = ((_channels == 2 && (i % 2)) ? _shift + 1 : _shift);
			int32 *ppred  = ((_channels == 2 && (i % 2)) ? _pred + 1 : _pred);
			const uint8 b = _data->readByte();
			const int diff = static_cast<int8>(b & 0xFC) * 256;
			if ((b & 3) == 3)
				*pshift += 1;
			else
				*pshift -= (2 * (b & 3));
			if (*pshift < 0)
				*pshift = 0;
			if (*pshift > 15) {
				warning("Xan DPCM shift should not go over 15, corrupt data?");
				*pshift = 15;
			}
			*ppred += (diff >> *pshift);
			*ppred = CLIP(*ppred, (int32)-32768, (int32)32767);
			*buffer = *ppred;
			buffer++;
			if (_data->eos())
				break;
		}
		return i;
	}

	bool isStereo() const override {
		return _channels == 2;
	}

	int getRate() const override {
		return _rate;
	}

	bool endOfData() const override {
		return _data->eos();
	}

private:
	int _channels;
	int _rate;
	Common::SeekableReadStream *_data;
	int32 _pred[2];
	int32 _shift[2];
};

XanDPCMStream::XanDPCMStream(int rate, int channels) :
	Audio::StatelessPacketizedAudioStream(rate, channels) {
}

AudioStream *XanDPCMStream::makeStream(Common::SeekableReadStream *data) {
	return new Xan_DPCMStream(getRate(), getChannels(), data);
}

} // End of namespace Audio
