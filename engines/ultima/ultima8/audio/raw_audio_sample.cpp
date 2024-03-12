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

#include "ultima/ultima8/misc/common_types.h"
#include "ultima/ultima8/audio/raw_audio_sample.h"
#include "common/memstream.h"
#include "audio/decoders/raw.h"

namespace Ultima {
namespace Ultima8 {

RawAudioSample::RawAudioSample(const uint8 *buffer, uint32 size, uint32 rate,
							   bool signedData, bool stereo)
	: AudioSample(buffer, size, 8, stereo, false), _signedData(signedData) {
	_sampleRate = rate;
	_length = size;
}

RawAudioSample::~RawAudioSample() {
}

Audio::SeekableAudioStream *RawAudioSample::makeStream() const {
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(_buffer, _bufferSize, DisposeAfterUse::NO);
	byte flags = 0;
	if (isStereo())
		flags |= Audio::FLAG_STEREO;
	if (!_signedData)
		flags |= Audio::FLAG_UNSIGNED;

	return Audio::makeRawStream(stream, getRate(), flags, DisposeAfterUse::YES);
}

} // End of namespace Ultima8
} // End of namespace Ultima
