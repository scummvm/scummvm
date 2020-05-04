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
#include "ultima/ultima8/audio/audio_sample.h"

namespace Ultima {
namespace Ultima8 {

AudioSample::AudioSample(const uint8 *buffer, uint32 size, uint32 bits, bool stereo, bool deleteBuffer) :
	_sampleRate(0), _bits(bits), _stereo(stereo),
	_frameSize(0), _decompressorSize(0), _length(0),
	_bufferSize(size), _buffer(buffer), _deleteBuffer(deleteBuffer) {
}

AudioSample::~AudioSample(void) {
	if (_deleteBuffer)
		delete [] _buffer;
}

} // End of namespace Ultima8
} // End of namespace Ultima
