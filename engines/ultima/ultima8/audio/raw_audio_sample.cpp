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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/audio/raw_audio_sample.h"
#include "ultima/ultima8/filesys/idata_source.h"

namespace Ultima {
namespace Ultima8 {

RawAudioSample::RawAudioSample(const uint8 *buffer, uint32 size, uint32 rate,
                               bool signedData, bool stereo)
	: AudioSample(buffer, size, 8, stereo, false), _signedData(signedData) {
	_sampleRate = rate;
	_frameSize = 512;
	_decompressorSize = sizeof(RawDecompData);
	_length = size;
}

RawAudioSample::~RawAudioSample() {
}

void RawAudioSample::initDecompressor(void *DecompData) const {
	RawDecompData *decomp = reinterpret_cast<RawDecompData *>(DecompData);
	decomp->_pos = 0;
}

void RawAudioSample::rewind(void *DecompData) const {
	initDecompressor(DecompData);
}

uint32 RawAudioSample::decompressFrame(void *DecompData, void *samples) const {
	RawDecompData *decomp = reinterpret_cast<RawDecompData *>(DecompData);

	if (decomp->_pos == _bufferSize) return 0;

	uint32 count = _frameSize;
	if (decomp->_pos + count > _bufferSize)
		count = _bufferSize - decomp->_pos;

	if (!_signedData) {
		Std::memcpy(samples, _buffer + decomp->_pos, count);
	} else {
		uint8 *dest = static_cast<uint8 *>(samples);
		for (unsigned int i = 0; i < count; ++i)
			dest[i] = _buffer[decomp->_pos + i] + 128;
	}

	decomp->_pos += count;

	return count;
}

} // End of namespace Ultima8
} // End of namespace Ultima
