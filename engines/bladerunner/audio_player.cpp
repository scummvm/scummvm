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
#include "bladerunner/audio_cache.h"
#include "bladerunner/audio_mixer.h"
#include "bladerunner/bladerunner.h"

#include "common/debug.h"
#include "common/stream.h"
#include "common/random.h"

namespace Common {
	class MemoryReadStream;
}

namespace BladeRunner {

AudioPlayer::AudioPlayer(BladeRunnerEngine *vm) {
	_vm = vm;

	for (int i = 0; i != kTracks; ++i) {
		_tracks[i].priority = 0;
		_tracks[i].isActive = false;
		_tracks[i].channel = -1;
		_tracks[i].stream = nullptr;
	}

	_sfxVolume = BLADERUNNER_ORIGINAL_SETTINGS ? 65 : 100;
}

AudioPlayer::~AudioPlayer() {
	stopAll();
}

void AudioPlayer::stopAll() {
	for (int i = 0; i != kTracks; ++i) {
		stop(i, true);
	}
	for (int i = 0; i != kTracks; ++i) {
		while (isActive(i)) {
			// wait for all tracks to finish
		}
	}
}

void AudioPlayer::adjustVolume(int track, int volume, uint32 delay, bool overrideVolume) {
	if (track < 0 || track >= kTracks || !_tracks[track].isActive || _tracks[track].channel == -1) {
		return;
	}

	int actualVolume = volume;
	if (!overrideVolume) {
		actualVolume = actualVolume * _sfxVolume / 100;
	}

	_tracks[track].volume = actualVolume;
	_vm->_audioMixer->adjustVolume(_tracks[track].channel, actualVolume, 60u * delay);
}

void AudioPlayer::adjustPan(int track, int pan, uint32 delay) {
	if (track < 0 || track >= kTracks || !_tracks[track].isActive || _tracks[track].channel == -1) {
		return;
	}

	_tracks[track].pan = pan;
	_vm->_audioMixer->adjustPan(_tracks[track].channel, pan, 60u * delay);
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

int AudioPlayer::playAud(const Common::String &name, int volume, int panStart, int panEnd, int priority, byte flags, Audio::Mixer::SoundType type) {
	/* Find first available track or, alternatively, the lowest priority playing track */
	int track = -1;
	int lowestPriority = 1000000;
	int lowestPriorityTrack = -1;

	for (int i = 0; i != kTracks; ++i) {
		if (!isActive(i)) {
			//debug ("Assigned track %i to %s", i, name.c_str());
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
		//debug ("Stop lowest priority  track (with lower prio: %d %d), for %s %d!", lowestPriorityTrack, lowestPriority, name.c_str(), priority);
		stop(lowestPriorityTrack, true);
		track = lowestPriorityTrack;
	}

	/* If there's still no available track, give up */
	if (track == -1) {
		//debug ("No available track for %s %d - giving up", name.c_str(), priority);
		return -1;
	}

	/* Load audio resource and store in cache. Playback will happen directly from there. */
	int32 hash = MIXArchive::getHash(name);
	if (!_vm->_audioCache->findByHash(hash)) {
		Common::SeekableReadStream *r = _vm->getResourceStream(name);
		if (!r) {
			//debug ("Could not get stream for %s %d - giving up", name.c_str(), priority);
			return -1;
		}

		int32 size = r->size();
		while (!_vm->_audioCache->canAllocate(size)) {
			if (!_vm->_audioCache->dropOldest()) {
				delete r;
				//debug ("No available mem in cache for %s %d - giving up", name.c_str(), priority);
				return -1;
			}
		}
		_vm->_audioCache->storeByHash(hash, r);
		delete r;
	}

	AudStream *audioStream = new AudStream(_vm->_audioCache, hash);

	int actualVolume = volume;
	if (!(flags & kAudioPlayerOverrideVolume)) {
		actualVolume = _sfxVolume * volume / 100;
	}

	int channel = _vm->_audioMixer->play(
		type,
		audioStream,
		priority,
		flags & kAudioPlayerLoop,
		actualVolume,
		panStart,
		mixerChannelEnded,
		this,
		audioStream->getLength()
		);

	if (channel == -1) {
		delete audioStream;
		//debug ("No available channel for %s %d - giving up", name.c_str(), priority);
		return -1;
	}

	if (panStart != panEnd) {
		_vm->_audioMixer->adjustPan(channel, panEnd, (60 * audioStream->getLength()) / 1000);
	}

	_tracks[track].isActive = true;
	_tracks[track].channel  = channel;
	_tracks[track].priority = priority;
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

/**
* Return the track's length in milliseconds
*/
uint32 AudioPlayer::getLength(int track) const {
	Common::StackLock lock(_mutex);
	if (track < 0 || track >= kTracks) {
		return 0;
	}

	return _tracks[track].stream->getLength();
}

void AudioPlayer::stop(int track, bool immediately) {
	if (isActive(track)) {
		_vm->_audioMixer->stop(_tracks[track].channel, immediately ? 0 : 60);
	}
}

} // End of namespace BladeRunner
