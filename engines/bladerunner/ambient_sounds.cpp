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

#include "bladerunner/ambient_sounds.h"

#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/game_info.h"

#include "common/debug.h"
#include "common/system.h"

namespace BladeRunner {

AmbientSounds::AmbientSounds(BladeRunnerEngine *vm) {
	_vm = vm;
	_nonLoopingSounds = new NonLoopingSound[kNonLoopingSounds];
	_loopingSounds = new LoopingSound[kLoopingSounds];
	_ambientVolume = 65;

	for (int i = 0; i != kNonLoopingSounds; ++i) {
		NonLoopingSound &track = _nonLoopingSounds[i];
		track.isActive = false;
	}

	for (int i = 0; i != kLoopingSounds; ++i) {
		LoopingSound &track = _loopingSounds[i];
		track.isActive = false;
	}
}

AmbientSounds::~AmbientSounds() {
	delete[] _nonLoopingSounds;
	delete[] _loopingSounds;
}

static inline void sort(int &a, int &b) {
	if (a > b) {
		int t = a;
		a = b;
		b = t;
	}
}

void AmbientSounds::addSound(
	int sfxId,
	int timeMin, int timeMax,
	int volumeMin, int volumeMax,
	int panStartMin, int panStartMax,
	int panEndMin, int panEndMax,
	int priority, int unk) {
	const char *name = _vm->_gameInfo->getSfxTrack(sfxId);

	sort(volumeMin, volumeMax);
	sort(panStartMin, panStartMax);
	sort(panEndMin, panEndMax);

	addSoundByName(
				 name,
				 timeMin, timeMax,
				 volumeMin, volumeMax,
				 panStartMin, panStartMax,
				 panEndMin, panEndMax,
				 priority, unk
				);
}

void AmbientSounds::removeNonLoopingSound(int sfxId, bool stopPlaying) {
	const char *name = _vm->_gameInfo->getSfxTrack(sfxId);
	int32 hash = mix_id(name);
	int index = findNonLoopingTrackByHash(hash);
	if (index >= 0) {
		removeNonLoopingSoundByIndex(index, stopPlaying);
	}
}

void AmbientSounds::removeAllNonLoopingSounds(bool stopPlaying) {
	for (int i = 0; i < kNonLoopingSounds; i++) {
		removeNonLoopingSoundByIndex(i, stopPlaying);
	}
}

void AmbientSounds::addSpeech(int actorId, int sentenceId, int timeMin, int timeMax, int volumeMin, int volumeMax, int panStartMin, int panStartMax, int panEndMin, int panEndMax, int priority, int unk) {
	sort(volumeMin, volumeMax);
	sort(panStartMin, panStartMax);
	sort(panEndMin, panEndMax);

	char name[13];
	sprintf(name, "%02d-%04d%s.AUD", actorId, sentenceId, _vm->_languageCode);
	addSoundByName(name,
					timeMin, timeMax,
					volumeMin, volumeMax,
					panStartMin, panStartMax,
					panEndMin, panEndMax,
					priority, unk);
}

void AmbientSounds::playSound(int sfxId, int volume, int panStart, int panEnd, int priority) {
	const char *name = _vm->_gameInfo->getSfxTrack(sfxId);

	_vm->_audioPlayer->playAud(name, volume * _ambientVolume / 100, panStart, panEnd, priority, AudioPlayer::OVERRIDE_VOLUME);
}

void AmbientSounds::addLoopingSound(int sfxId, int volume, int pan, int delay) {
	const char *name = _vm->_gameInfo->getSfxTrack(sfxId);

	int32 hash = mix_id(name);

	if (findLoopingTrackByHash(hash) >= 0) {
		return;
	}

	int i = findAvailableLoopingTrack();
	if (i == -1) {
		return;
	}
	LoopingSound &track = _loopingSounds[i];

	track.isActive = true;
	strcpy(track.name, name);
	track.hash = hash;
	track.pan = pan;
	track.volume = volume;

	int actualVolumeStart = volume * _ambientVolume / 100;
	int actualVolumeEnd = actualVolumeStart;

	if (delay > 0) {
		actualVolumeStart = 0;
	}

	track.audioPlayerTrack = _vm->_audioPlayer->playAud(name, actualVolumeStart, pan, pan, 99, AudioPlayer::LOOP | AudioPlayer::OVERRIDE_VOLUME);

	if (track.audioPlayerTrack == -1) {
		removeLoopingSoundByIndex(i, 0);
	} else {
		if (delay) {
			_vm->_audioPlayer->adjustVolume(track.audioPlayerTrack, actualVolumeEnd, delay, false);
		}
	}
}

void AmbientSounds::adjustLoopingSound(int sfxId, int volume, int pan, int delay) {
	const char *name = _vm->_gameInfo->getSfxTrack(sfxId);
	int32 hash = mix_id(name);
	int index = findLoopingTrackByHash(hash);

	if (index >= 0 && _loopingSounds[index].audioPlayerTrack != -1 && _vm->_audioPlayer->isActive(_loopingSounds[index].audioPlayerTrack)) {
		if (volume != -1) {
			_loopingSounds[index].volume = volume;
			_vm->_audioPlayer->adjustVolume(_loopingSounds[index].audioPlayerTrack, _ambientVolume * volume / 100, delay, false);
		}
		if (pan != -101) {
			_loopingSounds[index].pan = pan;
			_vm->_audioPlayer->adjustPan(_loopingSounds[index].audioPlayerTrack, pan, delay);
		}
	}
}

void AmbientSounds::removeLoopingSound(int sfxId, int delay) {
	const char *name = _vm->_gameInfo->getSfxTrack(sfxId);
	int32 hash = mix_id(name);
	int index = findLoopingTrackByHash(hash);
	if (index >= 0) {
		removeLoopingSoundByIndex(index, delay);
	}
}

void AmbientSounds::removeAllLoopingSounds(int delay) {
	for (int i = 0; i < kLoopingSounds; i++) {
		removeLoopingSoundByIndex(i, delay);
	}
}

void AmbientSounds::tick() {
	uint32 now = g_system->getMillis();

	for (int i = 0; i != kNonLoopingSounds; ++i) {
		NonLoopingSound &track = _nonLoopingSounds[i];

		if (!track.isActive || track.nextPlayTime > now) {
			continue;
		}

		int panEnd;
		int panStart = _vm->_rnd.getRandomNumberRng(track.panStartMin, track.panStartMax);
		if (track.panEndMin == -101) {
			panEnd = panStart;
		} else {
			panEnd = _vm->_rnd.getRandomNumberRng(track.panEndMin, track.panEndMax);
		}

		track.volume = _vm->_rnd.getRandomNumberRng(track.volumeMin, track.volumeMax);

		track.audioPlayerTrack = _vm->_audioPlayer->playAud(
															 track.name,
															 track.volume * _ambientVolume / 100,
															 panStart,
															 panEnd,
															 track.priority,
															 AudioPlayer::OVERRIDE_VOLUME
															);

		track.nextPlayTime = now + _vm->_rnd.getRandomNumberRng(track.timeMin, track.timeMax);
	}
}

void AmbientSounds::setVolume(int volume) {
	if (_loopingSounds) {
		for (int i = 0; i < kLoopingSounds; i++) {
			if (_loopingSounds[i].isActive && _loopingSounds[i].audioPlayerTrack != -1) {
				int newVolume = _loopingSounds[i].volume * volume / 100;
				if (_vm->_audioPlayer->isActive(_loopingSounds[i].audioPlayerTrack)) {
					_vm->_audioPlayer->adjustVolume(_loopingSounds[i].audioPlayerTrack, newVolume, 1, false);
				} else {
					_loopingSounds[i].audioPlayerTrack = _vm->_audioPlayer->playAud(_loopingSounds[i].name, 1, _loopingSounds[i].pan, _loopingSounds[i].pan, 99, AudioPlayer::LOOP | AudioPlayer::OVERRIDE_VOLUME);
					if (_loopingSounds[i].audioPlayerTrack == -1) {
						removeLoopingSound(i, 0);
					} else {
						_vm->_audioPlayer->adjustVolume(_loopingSounds[i].audioPlayerTrack, newVolume, 1, false);
					}
				}
			}
		}
	}
	_ambientVolume = volume;
}

int AmbientSounds::getVolume() const {
	return _ambientVolume;
}

void AmbientSounds::playSample() {
	playSound(66, 100, 0, 0, 0);
}

int AmbientSounds::findAvailableNonLoopingTrack() const {
	for (int i = 0; i != kNonLoopingSounds; ++i) {
		if (!_nonLoopingSounds[i].isActive) {
			return i;
		}
	}

	return -1;
}

int AmbientSounds::findNonLoopingTrackByHash(int32 hash) const {
	for (int i = 0; i != kNonLoopingSounds; ++i) {
		NonLoopingSound &track = _nonLoopingSounds[i];

		if (track.isActive && track.hash == hash) {
			return i;
		}
	}

	return -1;
}

int AmbientSounds::findAvailableLoopingTrack() const {
	for (int i = 0; i != kLoopingSounds; ++i) {
		if (!_loopingSounds[i].isActive) {
			return i;
		}
	}

	return -1;
}

int AmbientSounds::findLoopingTrackByHash(int32 hash) const {
	for (int i = 0; i != kLoopingSounds; ++i) {
		LoopingSound &track = _loopingSounds[i];

		if (track.isActive && track.hash == hash) {
			return i;
		}
	}

	return -1;
}

void AmbientSounds::addSoundByName(
	const char *name,
	int timeMin, int timeMax,
	int volumeMin, int volumeMax,
	int panStartMin, int panStartMax,
	int panEndMin, int panEndMax,
	int priority, int unk) {
	if (strlen(name) > 12) {
		error("AmbientSounds::addSoundByName: Overlong name '%s'", name);
	}

	int i = findAvailableNonLoopingTrack();
	if (i < 0) {
		return;
	}

	NonLoopingSound &track = _nonLoopingSounds[i];

	uint32 now = _vm->getTotalPlayTime();

	track.isActive = true;
	strcpy(track.name, name);
	track.hash = mix_id(name);
	track.timeMin = 1000 * timeMin;
	track.timeMax = 1000 * timeMax;
	track.nextPlayTime = now + _vm->_rnd.getRandomNumberRng(track.timeMin, track.timeMax);
	track.volumeMin = volumeMin;
	track.volumeMax = volumeMax;
	track.volume = 0;
	track.panStartMin = panStartMin;
	track.panStartMax = panStartMax;
	track.panEndMin = panEndMin;
	track.panEndMax = panEndMax;
	track.priority = priority;
}

void AmbientSounds::removeNonLoopingSoundByIndex(int index, bool stopPlaying) {
	NonLoopingSound &track = _nonLoopingSounds[index];
	if (stopPlaying) {
		if (track.isActive && track.audioPlayerTrack != -1 && _vm->_audioPlayer->isActive(track.audioPlayerTrack)) {
			_vm->_audioPlayer->stop(track.audioPlayerTrack, stopPlaying);
		}
	}
	track.isActive = false;
	track.audioPlayerTrack = -1;
	//	track.field_45 = 0;
}

void AmbientSounds::removeLoopingSoundByIndex(int index, int delay) {
	LoopingSound &track = _loopingSounds[index];
	if (track.isActive && track.audioPlayerTrack != -1 && _vm->_audioPlayer->isActive(track.audioPlayerTrack)) {
		if (delay > 0) {
			_vm->_audioPlayer->adjustVolume(track.audioPlayerTrack, 0, delay, false);
		} else {
			_vm->_audioPlayer->stop(track.audioPlayerTrack, false);
		}
	}
	track.isActive = false;
	track.name[0] = 0;
	track.hash = 0;
	track.audioPlayerTrack = -1;
	track.volume = 0;
	track.pan = 0;
}

} // End of namespace BladeRunner
