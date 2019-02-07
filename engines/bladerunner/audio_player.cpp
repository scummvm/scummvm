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

#include "bladerunner/audio_player.h"

#include "bladerunner/archive.h"
#include "bladerunner/aud_stream.h"
#include "bladerunner/audio_mixer.h"
#include "bladerunner/bladerunner.h"

#include "common/debug.h"
#include "common/stream.h"
#include "common/random.h"

namespace Common {
	class MemoryReadStream;
}

namespace BladeRunner {

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

	return nullptr;
}

void AudioCache::storeByHash(int32 hash, Common::SeekableReadStream *stream) {
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
			_cacheItems[i].refs++;
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
			_cacheItems[i].refs--;
			return;
		}
	}
	assert(false && "AudioCache::decRef: hash not found");
}

AudioPlayer::AudioPlayer(BladeRunnerEngine *vm) {
	_vm = vm;
	_cache = new AudioCache();

	for (int i = 0; i != 6; ++i) {
		_tracks[i].hash = 0;
		_tracks[i].priority = 0;
		_tracks[i].isActive = false;
		_tracks[i].channel = -1;
		_tracks[i].stream = nullptr;
	}

	_sfxVolume = 65;
}

AudioPlayer::~AudioPlayer() {
	stopAll();
	delete _cache;
}

void AudioPlayer::stopAll() {
	for (int i = 0; i != kTracks; ++i) {
		stop(i, false);
	}
	for (int i = 0; i != kTracks; ++i) {
		while (isActive(i)) {
			// wait for all tracks to finish
		}
	}
}

void AudioPlayer::adjustVolume(int track, int volume, int delay, bool overrideVolume) {
	if (track < 0 || track >= kTracks || !_tracks[track].isActive || _tracks[track].channel == -1) {
		return;
	}

	int actualVolume = volume;
	if (!overrideVolume) {
		actualVolume = actualVolume * _sfxVolume / 100;
	}

	_tracks[track].volume = volume;
	_vm->_audioMixer->adjustVolume(_tracks[track].channel, volume, 60 * delay);
}

void AudioPlayer::adjustPan(int track, int pan, int delay) {
	if (track < 0 || track >= kTracks || !_tracks[track].isActive || _tracks[track].channel == -1) {
		return;
	}

	_tracks[track].pan = pan;
	_vm->_audioMixer->adjustPan(_tracks[track].channel, pan, 60 * delay);
}

void AudioPlayer::setVolume(int volume) {
	_sfxVolume = volume;
}

int AudioPlayer::getVolume() const {
	return _sfxVolume;
}

void AudioPlayer::playSample() {
	Common::String name;

	int rnd = _vm->_rnd.getRandomNumber(3);
	if (rnd == 0) {
		name = "gunmiss1.aud";
	} else if (rnd == 1) {
		name = "gunmiss2.aud";
	} else if (rnd == 2) {
		name = "gunmiss3.aud";
	} else {
		name = "gunmiss4.aud";
	}

	playAud(name, 100, 0, 0, 100, 0);
}

void AudioPlayer::remove(int channel) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != kTracks; ++i) {
		if (_tracks[i].channel == channel) {
			_tracks[i].isActive = false;
			_tracks[i].priority = 0;
			_tracks[i].channel = -1;
			_tracks[i].stream = nullptr;
			break;
		}
	}
}

void AudioPlayer::mixerChannelEnded(int channel, void *data) {
	AudioPlayer *audioPlayer = (AudioPlayer *)data;
	audioPlayer->remove(channel);
}

int AudioPlayer::playAud(const Common::String &name, int volume, int panFrom, int panTo, int priority, byte flags) {
	/* Find first available track or, alternatively, the lowest priority playing track */
	int track = -1;
	int lowestPriority = 1000000;
	int lowestPriorityTrack = -1;

	for (int i = 0; i != 6; ++i) {
		if (!isActive(i)) {
			track = i;
			break;
		}

		if (lowestPriorityTrack == -1 || _tracks[i].priority < lowestPriority) {
			lowestPriority = _tracks[i].priority;
			lowestPriorityTrack = i;
		}
	}

	/* If there's no available track, stop the lowest priority track if it's lower than
	 * the new priority
	 */
	if (track == -1 && lowestPriority < priority) {
		stop(lowestPriorityTrack, true);
		track = lowestPriorityTrack;
	}

	/* If there's still no available track, give up */
	if (track == -1) {
		return -1;
	}

	/* Load audio resource and store in cache. Playback will happen directly from there. */
	int32 hash = MIXArchive::getHash(name);
	if (!_cache->findByHash(hash)) {
		Common::SeekableReadStream *r = _vm->getResourceStream(name);
		if (!r) {
			return -1;
		}

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

	AudStream *audioStream = new AudStream(_cache, hash);

	int actualVolume = volume;
	if (!(flags & kAudioPlayerOverrideVolume)) {
		actualVolume = _sfxVolume * volume / 100;
	}

	int channel = _vm->_audioMixer->play(
		Audio::Mixer::kPlainSoundType,
		audioStream,
		priority,
		flags & kAudioPlayerLoop,
		actualVolume,
		panFrom,
		mixerChannelEnded,
		this);

	if (channel == -1) {
		delete audioStream;
		_cache->decRef(hash);
		return -1;
	}

	if (panFrom != panTo) {
		_vm->_audioMixer->adjustPan(channel, panTo, (60 * audioStream->getLength()) / 1000);
	}

	_tracks[track].isActive = true;
	_tracks[track].channel  = channel;
	_tracks[track].priority = priority;
	_tracks[track].hash     = hash;
	_tracks[track].volume   = actualVolume;
	_tracks[track].stream   = audioStream;

	return track;
}

bool AudioPlayer::isActive(int track) const {
	Common::StackLock lock(_mutex);
	if (track < 0 || track >= kTracks) {
		return false;
	}

	return _tracks[track].isActive;
}

void AudioPlayer::stop(int track, bool immediately) {
	if (isActive(track)) {
		_vm->_audioMixer->stop(_tracks[track].channel, immediately ? 0 : 60);
	}
}

} // End of namespace BladeRunner
