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

#include "bladerunner/audio_cache.h"

#include "common/stream.h"

namespace BladeRunner {

AudioCache::AudioCache() :
	_totalSize(0),
	_maxSize(2457600),
	_accessCounter(0) {}

AudioCache::~AudioCache() {
	for (uint i = 0; i != _cacheItems.size(); ++i) {
		free(_cacheItems[i].data);
	}
}

bool AudioCache::canAllocate(uint32 size) const {
	Common::StackLock lock(_mutex);

	return _maxSize - _totalSize >= size;
}

bool AudioCache::dropOldest() {
	Common::StackLock lock(_mutex);

	if (_cacheItems.size() == 0)
		return false;

	int oldest = -1;
	for (uint i = 1; i != _cacheItems.size(); ++i) {
		if (_cacheItems[i].refs == 0) {
			if (oldest == -1 || _cacheItems[i].lastAccess < _cacheItems[oldest].lastAccess) {
				oldest = i;
			}
		}
	}

	if (oldest == -1) {
		return false;
	}

	memset(_cacheItems[oldest].data, 0x00, _cacheItems[oldest].size);
	free(_cacheItems[oldest].data);
	_totalSize -= _cacheItems[oldest].size;
	_cacheItems.remove_at(oldest);
	return true;
}

byte *AudioCache::findByHash(int32 hash) {
	Common::StackLock lock(_mutex);

	for (uint i = 0; i != _cacheItems.size(); ++i) {
		if (_cacheItems[i].hash == hash) {
			_cacheItems[i].lastAccess = _accessCounter++;
			return _cacheItems[i].data;
		}
	}

	return nullptr;
}

void  AudioCache::storeByHash(int32 hash, Common::SeekableReadStream *stream) {
	Common::StackLock lock(_mutex);

	uint32 size = stream->size();
	byte *data = (byte *)malloc(size);
	stream->read(data, size);

	cacheItem item = {
		hash,
		0,
		_accessCounter++,
		data,
		size
	};

	_cacheItems.push_back(item);
	_totalSize += size;
}

void AudioCache::incRef(int32 hash) {
	Common::StackLock lock(_mutex);

	for (uint i = 0; i != _cacheItems.size(); ++i) {
		if (_cacheItems[i].hash == hash) {
			++(_cacheItems[i].refs);
			return;
		}
	}
	assert(false && "AudioCache::incRef: hash not found");
}

void AudioCache::decRef(int32 hash) {
	Common::StackLock lock(_mutex);

	for (uint i = 0; i != _cacheItems.size(); ++i) {
		if (_cacheItems[i].hash == hash) {
			assert(_cacheItems[i].refs > 0);
			--(_cacheItems[i].refs);
			return;
		}
	}
	assert(false && "AudioCache::decRef: hash not found");
}

} // End of namespace BladeRunner
