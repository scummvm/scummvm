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

#include "bladerunner/aud_stream.h"

#include "bladerunner/audio_cache.h"

#include "common/util.h"

namespace BladeRunner {

AudStream::AudStream(byte *data, int overrideFrequency) {
	_hash  = 0;
	_cache = nullptr;
	_overrideFrequency = overrideFrequency;

	init(data);
}

AudStream::AudStream(AudioCache *cache, int32 hash, int overrideFrequency) {
	assert(cache != nullptr);

	_cache = cache;
	_hash  = hash;
	_overrideFrequency = overrideFrequency;

	_cache->incRef(_hash);

	init(_cache->findByHash(_hash));
}

void AudStream::init(byte *data) {
	_data = data;
	_frequency = READ_LE_UINT16(_data);
	_size = READ_LE_UINT32(_data + 2);
	_sizeDecompressed = READ_LE_UINT32(_data + 6);
	_flags = *(_data + 10);
	_compressionType = *(_data + 11);

	_end = _data + _size + 12;
	assert(_end - _data >= 12);

	_deafBlockRemain = 0;
	_p = _data + 12;
}

AudStream::~AudStream() {
	if (_cache) {
		_cache->decRef(_hash);
	}
}

int AudStream::readBuffer(int16 *buffer, const int numSamples) {
	int samplesRead = 0;

	if (_compressionType == 99) {
		assert(numSamples % 2 == 0);

		while (samplesRead < numSamples) {
			if (_deafBlockRemain == 0) {
				if (_end - _p == 0)
					break;

				assert(_end - _p >= 6);

				uint16 blockSize     = READ_LE_UINT16(_p);
				uint16 blockOutSize  = READ_LE_UINT16(_p + 2);
				uint32 sig           = READ_LE_UINT32(_p + 4);
				_p += 8;

				assert(sig == 0xdeaf);
				assert(_end - _p >= blockSize);

				// TODO: Previously we asserted that
				// blockOutSize == 4 * blockSize, but
				// occasionally, at the end of an AUD,
				// we see blockOutSize == 4 * blockSize + 2
				// Investigate how BLADE.EXE handles this.

				assert(blockOutSize / 4 == blockSize);

				_deafBlockRemain = blockSize;
			}

			assert(_end - _p >= _deafBlockRemain);

			int bytesConsumed = MIN<int>(_deafBlockRemain, (numSamples - samplesRead) / 2);
			_decoder.decode(_p, bytesConsumed, buffer + samplesRead, false);
			_p += bytesConsumed;
			_deafBlockRemain -= bytesConsumed;

			samplesRead += 2 * bytesConsumed;
		}
	} else {
		samplesRead = MIN(numSamples, (int)(_end - _p) / 2);
		for (int i = 0; i < samplesRead; ++i, _p += 2) {
			buffer[i] = READ_LE_UINT16(_p);
		}
	}

	return samplesRead;
}

bool AudStream::rewind() {
	_p = _data + 12;
	_decoder.setParameters(0, 0);
	return true;
}

/**
* Returns audio length in milliseconds
*/
uint32 AudStream::getLength() const {
	int bytesPerSecond = _overrideFrequency > 0 ? _overrideFrequency : _frequency;
	if (_flags & 1) { // 16 bit
		bytesPerSecond *= 2;
	}
	if (_flags & 2) { // stereo
		bytesPerSecond *= 2;
	}

	// since everything is 44100, we easily get overflows with ints
	// thus we must use doubles
	double res = (double)_sizeDecompressed * 1000.0 / (double)bytesPerSecond;
	return (uint32)res;
}

} // End of namespace BladeRunner
