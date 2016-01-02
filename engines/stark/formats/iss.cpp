/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/stark/formats/iss.h"

#include "audio/decoders/adpcm_intern.h"
#include "audio/decoders/raw.h"
#include "common/substream.h"

namespace Stark {
namespace Formats {

/**
 * ADPCM decoder for the .iss files
 */
class ISSADPCMStream : public Audio::Ima_ADPCMStream {
public:
	ISSADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Ima_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {}

protected:
	int readBuffer(int16 *buffer, const int numSamples) {
		// Similar to MS IMA, but without the four-bytes-per-channel requirement
		int samples;

		assert(numSamples % 2 == 0);

		for (samples = 0; samples < numSamples && !endOfData(); samples += 2) {
			if (_blockPos[0] == _blockAlign) {
				// read block header
				for (byte i = 0; i < _channels; i++) {
					_status.ima_ch[i].last = _stream->readSint16LE();
					_status.ima_ch[i].stepIndex = _stream->readSint16LE();
				}
				_blockPos[0] = 4 * _channels;
			}

			byte data = _stream->readByte();
			buffer[samples + (isStereo() ? 1 : 0)] = decodeIMA(data & 0x0f, isStereo() ? 1 : 0);
			buffer[samples + (isStereo() ? 0 : 1)] = decodeIMA((data >> 4) & 0x0f);
			_blockPos[0]++;
		}

		return samples;
	}
};

static Common::String readString(Common::SeekableReadStream *stream) {
	Common::String ret = "";
	byte ch;
	while ((ch = stream->readByte()) != 0x20)
		ret += ch;

	return ret;
}

Audio::RewindableAudioStream *makeISSStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	Common::String codec;
	uint16 blockSize, channels, freq;
	uint32 size;
	byte flags;

	codec = readString(stream);

	if (codec.equals("IMA_ADPCM_Sound")) {

		codec = readString(stream);
		blockSize = (uint16)strtol(codec.c_str(), 0, 10);

		readString(stream);
		// name ?

		readString(stream);
		// ?

		codec = readString(stream);
		channels = (uint16)strtol(codec.c_str(), 0, 10) + 1;

		readString(stream);
		// ?

		codec = readString(stream);
		freq = 44100 / (uint16)strtol(codec.c_str(), 0, 10);

		readString(stream);

		readString(stream);

		codec = readString(stream);
		size = (uint32)strtol(codec.c_str(), 0, 10);

		return new ISSADPCMStream(stream, DisposeAfterUse::YES, size, freq, channels, blockSize);
	} else if (codec.equals("Sound")) {

		readString(stream);
		// name ?

		codec = readString(stream);
		// sample count ?

		codec = readString(stream);
		channels = (uint16)strtol(codec.c_str(), 0, 10) + 1;

		readString(stream);
		// ?

		codec = readString(stream);
		freq = 44100 / (uint16)strtol(codec.c_str(), 0, 10);

		readString(stream);

		readString(stream);

		flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
		if (channels == 2)
			flags |= Audio::FLAG_STEREO;
		return Audio::makeRawStream(new Common::SeekableSubReadStream(stream, stream->pos(), stream->size(), DisposeAfterUse::YES), freq, flags, DisposeAfterUse::YES);
	} else {
		error("Unknown ISS codec '%s'", codec.c_str());
	}
}

} // End of namespace Formats
} // End of namespace Stark
