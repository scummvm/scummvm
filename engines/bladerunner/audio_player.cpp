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

#include "audio_player.h"

#include "bladerunner/archive.h"
#include "bladerunner/aud_stream.h"

#include "bladerunner/bladerunner.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "common/debug.h"
#include "common/stream.h"

namespace Common {
	class MemoryReadStream;
}

namespace BladeRunner {

AudioCache::~AudioCache() {
	for (uint i = 0; i != _cacheItems.size(); ++i) {
		free(_cacheItems[i].data);
	}
}

bool AudioCache::canAllocate(uint32 size) {
	Common::StackLock lock(_mutex);

	return _maxSize - _totalSize >= size;
}

bool AudioCache::dropOldest() {
	Common::StackLock lock(_mutex);

	if (_cacheItems.size() == 0)
		return false;

	uint oldest = 0;
	for (uint i = 1; i != _cacheItems.size(); ++i) {
		if (_cacheItems[i].refs == 0 && _cacheItems[i].lastAccess < _cacheItems[oldest].lastAccess)
			oldest = i;
	}

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

	return NULL;
}

void AudioCache::storeByHash(int32 hash, Common::SeekableReadStream *stream) {
	Common::StackLock lock(_mutex);

	uint32 size = stream->size();
	byte *data = (byte*)malloc(size);
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
			_cacheItems[i].refs++;
			return;
		}
	}
	assert(0 && "AudioCache::incRef: hash not found");
}

void AudioCache::decRef(int32 hash) {
	Common::StackLock lock(_mutex);

	for (uint i = 0; i != _cacheItems.size(); ++i) {
		if (_cacheItems[i].hash == hash) {
			assert(_cacheItems[i].refs > 0);
			_cacheItems[i].refs--;
			return;
		}
	}
	assert(0 && "AudioCache::decRef: hash not found");
}

AudioPlayer::AudioPlayer(BladeRunnerEngine *vm) : _vm(vm) {
	_cache = new AudioCache();

	for (int i = 0; i != 6; ++i) {
		_tracks[i].hash = 0;
		_tracks[i].priority = 0;
	}
}

AudioPlayer::~AudioPlayer() {
	delete _cache;
}

bool AudioPlayer::isTrackActive(Track *track) {
	if (!track->isMaybeActive)
		return false;

	return track->isMaybeActive = _vm->_mixer->isSoundHandleActive(track->soundHandle);
}

void AudioPlayer::stopAll() {
	for (int i = 0; i != TRACKS; ++i) {
		_vm->_mixer->stopHandle(_tracks[i].soundHandle);
	}
}

void AudioPlayer::fadeAndStopTrack(Track *track, int time) {
	(void)time;

	_vm->_mixer->stopHandle(track->soundHandle);
}

int AudioPlayer::playAud(const Common::String &name, int volume, int panFrom, int panTo, int priority, byte flags) {
	/* Find first available track or, alternatively, the lowest priority playing track */
	Track *track = NULL;
	int    lowestPriority = 1000000;
	Track *lowestPriorityTrack = NULL;

	for (int i = 0; i != 6; ++i) {
		Track *ti = &_tracks[i];
		if (!isTrackActive(ti)) {
			track = ti;
			break;
		}

		if (lowestPriorityTrack == NULL || ti->priority < lowestPriority) {
			lowestPriority = ti->priority;
			lowestPriorityTrack = ti;
		}
	}

	/* If there's no available track, stop the lowest priority track if it's lower than
	 * the new priority
	 */
	if (track == NULL && lowestPriority < priority) {
		fadeAndStopTrack(lowestPriorityTrack, 1);
		track = lowestPriorityTrack;
	}

	/* If there's still no available track, give up */
	if (track == NULL)
		return -1;

	/* Load audio resource and store in cache. Playback will happen directly from there. */
	int32 hash = mix_id(name);
	if (!_cache->findByHash(hash)) {
		Common::SeekableReadStream *r = _vm->getResourceStream(name);
		if (!r)
			return -1;

		int32 size = r->size();
		while (!_cache->canAllocate(size)) {
			if (!_cache->dropOldest()) {
				delete r;
				return -1;
			}
		}
		_cache->storeByHash(hash, r);
		delete r;
	}

	AudStream *audStream = new AudStream(_cache, hash);

	Audio::AudioStream *audioStream = audStream;
	if (flags & LOOP) {
		audioStream = new Audio::LoopingAudioStream(audStream, 0, DisposeAfterUse::YES);
	}

	Audio::SoundHandle soundHandle;

	// debug("PlayStream: %s", name.c_str());

	int balance = panFrom;

	_vm->_mixer->playStream(
		Audio::Mixer::kPlainSoundType,
		&soundHandle,
		audioStream,
		-1,
		volume * 255 / 100,
		balance);

	track->isMaybeActive = true;
	track->soundHandle   = soundHandle;
	track->priority      = priority;
	track->hash          = hash;
	track->volume        = volume;

	return track - &_tracks[0];
}

} // End of namespace BladeRunner
