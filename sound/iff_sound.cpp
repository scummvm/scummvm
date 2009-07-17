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

#include "sound/iff_sound.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "common/func.h"

namespace Audio {

void Voice8Header::load(Common::ReadStream &stream) {
	stream.read(this, sizeof(Voice8Header));
	oneShotHiSamples = FROM_BE_32(oneShotHiSamples);
	repeatHiSamples = FROM_BE_32(repeatHiSamples);
	samplesPerHiCycle = FROM_BE_32(samplesPerHiCycle);
	samplesPerSec = FROM_BE_16(samplesPerSec);
	volume = FROM_BE_32(volume);
}



struct A8SVXLoader {
	Voice8Header _header;
	int8 *_data;
	uint32 _dataSize;

	void load(Common::ReadStream &input) {
		Common::IFFParser parser(&input);
		Common::Functor1Mem< Common::IFFChunk&, bool, A8SVXLoader > c(this, &A8SVXLoader::callback);
		parser.parse(c);
	}

	bool callback(Common::IFFChunk &chunk) {
		switch (chunk._type) {
		case ID_VHDR:
			_header.load(*chunk._stream);
			break;

		case ID_BODY:
			_dataSize = chunk._size;
			_data = (int8*)malloc(_dataSize);
			assert(_data);
			loadData(chunk._stream);
			return true;
		}

		return false;
	}

	void loadData(Common::ReadStream *stream) {
		switch (_header.compression) {
		case 0:
			stream->read(_data, _dataSize);
			break;

		case 1:
			// implement other formats here
			error("compressed IFF audio is not supported");
			break;
		}

	}
};


AudioStream *make8SVXStream(Common::ReadStream &input, bool loop) {
	A8SVXLoader loader;
	loader.load(input);

	uint32 loopStart = 0, loopEnd = 0, flags = 0;
	if (loop) {
		// the standard way to loop 8SVX audio implies use of the oneShotHiSamples and
		// repeatHiSamples fields
		loopStart = 0;
		loopEnd = loader._header.oneShotHiSamples + loader._header.repeatHiSamples;
		flags |= Audio::Mixer::FLAG_LOOP;
	}

	flags |= Audio::Mixer::FLAG_AUTOFREE;

	return Audio::makeLinearInputStream((byte *)loader._data, loader._dataSize, loader._header.samplesPerSec, flags, loopStart, loopEnd);
}

}
