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

	// _sfxVolumeFactorOriginalEngine here sets a percentage to be applied on the audio tracks' volume
	// before sending them to the audio player.
	// This is how the original engine set the volume via the in-game KIA volume slider controls.
	// Setting _sfxVolumeFactorOriginalEngine to 100, for the purposes ScummVM engine, renders it indifferent,
	// so sound volume can be controlled by ScummVM's Global Main Menu / ConfMan/ syncSoundSettings().
	_sfxVolumeFactorOriginalEngine = BLADERUNNER_ORIGINAL_SETTINGS ? 65 : 100;
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

// This method sets the target volume (as a percent) for the track
// and the time (delaySeconds) to reach that target in.
// volume (and actualVolume) should be in [0, 100]
void AudioPlayer::adjustVolume(int track, int volume, uint32 delaySeconds, bool explicitVolumeAdjustment) {
	if (track < 0 || track >= kTracks || !_tracks[track].isActive || _tracks[track].channel == -1) {
		return;
	}

	int actualVolume = volume;
	if (explicitVolumeAdjustment) {
		actualVolume = (actualVolume * _sfxVolumeFactorOriginalEngine) / 100;
	}

	_tracks[track].volume = actualVolume;
	_vm->_audioMixer->adjustVolume(_tracks[track].channel, actualVolume, 60u * delaySeconds);
}

// This method sets the target pan (as a percent) for the track
// and the time (delaySeconds) to reach that target in.
// pan should be in [-100, 100]
void AudioPlayer::adjustPan(int track, int pan, uint32 delaySeconds) {
	if (track < 0 || track >= kTracks || !_tracks[track].isActive || _tracks[track].channel == -1) {
		return;
	}

	_tracks[track].pan = pan;
	_vm->_audioMixer->adjustPan(_tracks[track].channel, pan, 60u * delaySeconds);
}

#if BLADERUNNER_ORIGINAL_SETTINGS
// We no longer set the _sfxVolumeFactorOriginalEngine via a public method.
// For the ScummVM Engine's purposes it is set in AudioPlayer::AudioPlayer() constructor and keeps its value constant.
void AudioPlayer::setVolume(int volume) {
	_sfxVolumeFactorOriginalEngine = volume;
}

int AudioPlayer::getVolume() const {
	return _sfxVolumeFactorOriginalEngine;
}
#endif // BLADERUNNER_ORIGINAL_SETTINGS

void AudioPlayer::playSample() {
	Common::String name;

	switch (_vm->_rnd.getRandomNumber(3)) {
	case 0:
		name = "gunmiss1.aud";
		break;

	case 1:
		name = "gunmiss2.aud";
		break;

	case 2:
		name = "gunmiss3.aud";
		break;

	default:
		name = "gunmiss4.aud";
		break;
	}

	// Sample plays with priority 100 (max)
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

// This method plays an audio file, at volume "volume" (as a percent, set immediately, no fade-in),
// with audio balance starting at panStart (as a signed percent) and ending at panEnd.
// The balance shifting happens throughout the duration of the audio track.
// volume (and actualVolume) should be in [0, 100]
// panStart and panEnd should be in [-100, 100]
// priority should be in [0, 100]
// The higher the priority, the more likely it is that this track will replace another active one
// (with the lowest priority), if no available track slots exists.
// Returns the index of the track slot assigned (for _tracks array) or -1 otherwise (gave up)
// The code here is very similar to AudioMixer::play()
// Note that this method calls AudioMixer::play() which also uses the priority value to assign a channel to the track.
// If that fails, the new track is dropped (gave up).
// TODO Maybe explain why we need this two step priority check (for track slot and then channel slot)
int AudioPlayer::playAud(const Common::String &name, int volume, int panStart, int panEnd, int priority, byte flags, Audio::Mixer::SoundType type) {
	debugC(6, kDebugSound, "AudioPlayer::playAud name:%s v:%d pS:%d pE:%d pr:%d type:%d", name.c_str(), volume, panStart, panEnd, priority, (int)type);
	// Find first available track or, alternatively, the lowest priority playing track
	int trackSlotToAssign = -1;
	int lowestPriority = 1000000; // TODO wouldn't a lower value work as well? eg. 1000? Original uses 100 but code is a bit different
	int lowestPriorityTrackSlot = -1;

	// Find an available track slot
	for (int i = 0; i != kTracks; ++i) {
		if (!isActive(i)) {
			//debug("Assigned track %i to %s", i, name.c_str());
			trackSlotToAssign = i;
			break;
		}

		if (lowestPriorityTrackSlot == -1 || _tracks[i].priority < lowestPriority) {
			lowestPriority = _tracks[i].priority;
			lowestPriorityTrackSlot = i;
		}
	}

	// If there's still no available track slot
	if (trackSlotToAssign == -1) {
		// Give up if the lowest priority track still has greater priority (lowestPriority).
#if BLADERUNNER_ORIGINAL_BUGS
		// NOTE If the new priority is *equal* to the existing lowestPriority,
		// then the new audio would still not replace the old one.
		if (priority <= lowestPriority) {
			return -1;
		}
#else
		// NOTE If the new priority is *equal* to the existing lowestPriority,
		// then the new audio will replace the old one.
		if (priority < lowestPriority) {
			//debug("No available track for %s %d - giving up", name.c_str(), priority);
			return -1;
		}
#endif
		// Otherwise, stop the lowest priority track, and assign the slot to the new one.
		//debug("Stop lowest priority  track (with lower prio: %d %d), for %s %d!", lowestPriorityTrackSlot, lowestPriority, name.c_str(), priority);
		stop(lowestPriorityTrackSlot, true);
		trackSlotToAssign = lowestPriorityTrackSlot;
	}

	// Load audio resource and store in cache. Playback will happen directly from there.
	int32 hash = MIXArchive::getHash(name);
	if (!_vm->_audioCache->findByHash(hash)) {
		Common::SeekableReadStream *r = _vm->getResourceStream(_vm->_enhancedEdition ? ("audio/" + name) : name);
		if (!r) {
			//debug("Could not get stream for %s %d - giving up", name.c_str(), priority);
			return -1;
		}

		int32 size = r->size();
		while (!_vm->_audioCache->canAllocate(size)) {
			if (!_vm->_audioCache->dropOldest()) {
				delete r;
				//debug("No available mem in cache for %s %d - giving up", name.c_str(), priority);
				return -1;
			}
		}
		_vm->_audioCache->storeByHash(hash, r);
		delete r;
	}

	AudStream *audioStream = new AudStream(_vm->_audioCache, hash);

	int actualVolume = volume;
	if (!(flags & kAudioPlayerOverrideVolume)) {
		actualVolume = (actualVolume * _sfxVolumeFactorOriginalEngine) / 100;
	}

	int channel = _vm->_audioMixer->play(type,
	                                     audioStream,
	                                     priority,
	                                     flags & kAudioPlayerLoop,
	                                     actualVolume,
	                                     panStart,
	                                     mixerChannelEnded,
	                                     this,
	                                     audioStream->getLength());

	if (channel == -1) {
		delete audioStream;
		//debug("No available channel for %s %d - giving up", name.c_str(), priority);
		return -1;
	}

	if (panStart != panEnd) {
		_vm->_audioMixer->adjustPan(channel, panEnd, (60u * audioStream->getLength()) / 1000u);
	}

	_tracks[trackSlotToAssign].isActive = true;
	_tracks[trackSlotToAssign].channel  = channel;
	_tracks[trackSlotToAssign].priority = priority;
	_tracks[trackSlotToAssign].volume   = actualVolume;
	_tracks[trackSlotToAssign].stream   = audioStream;

	return trackSlotToAssign;
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
		// If parameter "immediately" is not set,
		// the delay for audio stop is 1 second (multiplied by 60u as expected by AudioMixer::stop())
		_vm->_audioMixer->stop(_tracks[track].channel, immediately ? 0u : 60u);
	}
}

} // End of namespace BladeRunner
