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
#include "bladerunner/savefile.h"
#include "bladerunner/time.h"
#include "bladerunner/game_constants.h"

#include "common/debug.h"
#include "common/system.h"

namespace BladeRunner {

AmbientSounds::AmbientSounds(BladeRunnerEngine *vm) {
	_vm = vm;
	_nonLoopingSounds = new NonLoopingSound[kNonLoopingSounds];
	_loopingSounds = new LoopingSound[kLoopingSounds];

	_ambientVolume = BLADERUNNER_ORIGINAL_SETTINGS ? 65 : 100;

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

static inline void sort(int *a, int *b) {
	if (*a > *b) {
		int t = *a;
		*a = *b;
		*b = t;
	}
}

static inline void sort(uint32 *a, uint32 *b) {
	if (*a > *b) {
		uint32 t = *a;
		*a = *b;
		*b = t;
	}
}

void AmbientSounds::addSound(
	int sfxId,
	uint32 timeMin, uint32 timeMax,
	int volumeMin, int volumeMax,
	int panStartMin, int panStartMax,
	int panEndMin, int panEndMax,
	int priority, int unk) {

#if BLADERUNNER_ORIGINAL_BUGS
#else
	sort(&timeMin, &timeMax);
#endif // BLADERUNNER_ORIGINAL_BUGS
	sort(&volumeMin, &volumeMax);
	sort(&panStartMin, &panStartMax);
	sort(&panEndMin, &panEndMax);

	addSoundByName(
				_vm->_gameInfo->getSfxTrack(sfxId),
				timeMin, timeMax,
				volumeMin, volumeMax,
				panStartMin, panStartMax,
				panEndMin, panEndMax,
				priority, unk
				);
}

void AmbientSounds::removeNonLoopingSound(int sfxId, bool stopPlaying) {
	int32 hash = MIXArchive::getHash(_vm->_gameInfo->getSfxTrack(sfxId));
	int index = findNonLoopingTrackByHash(hash);
	if (index >= 0) {
		removeNonLoopingSoundByIndex(index, stopPlaying);
	}
}

void AmbientSounds::removeAllNonLoopingSounds(bool stopPlaying) {
	for (int i = 0; i < kNonLoopingSounds; ++i) {
		removeNonLoopingSoundByIndex(i, stopPlaying);
	}
}

void AmbientSounds::addSpeech(int actorId, int sentenceId, uint32 timeMin, uint32 timeMax, int volumeMin, int volumeMax, int panStartMin, int panStartMax, int panEndMin, int panEndMax, int priority, int unk) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
	sort(&timeMin, &timeMax);
#endif // BLADERUNNER_ORIGINAL_BUGS
	sort(&volumeMin, &volumeMax);
	sort(&panStartMin, &panStartMax);
	sort(&panEndMin, &panEndMax);

	Common::String name = Common::String::format( "%02d-%04d%s.AUD", actorId, sentenceId, _vm->_languageCode.c_str());
	addSoundByName(name,
					timeMin, timeMax,
					volumeMin, volumeMax,
					panStartMin, panStartMax,
					panEndMin, panEndMax,
					priority, unk);
}

void AmbientSounds::playSound(int sfxId, int volume, int panStart, int panEnd, int priority) {
	_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(sfxId), volume * _ambientVolume / 100, panStart, panEnd, priority, kAudioPlayerOverrideVolume);
}

void AmbientSounds::playSpeech(int actorId, int sentenceId, int volume, int panStart, int panEnd, int priority) {
	Common::String name = Common::String::format( "%02d-%04d%s.AUD", actorId, sentenceId, _vm->_languageCode.c_str());
	_vm->_audioPlayer->playAud(name, volume * _ambientVolume / 100, panStart, panEnd, priority, kAudioPlayerOverrideVolume, Audio::Mixer::kSpeechSoundType);
}

void AmbientSounds::addLoopingSound(int sfxId, int volume, int pan, uint32 delay) {
	const Common::String &name = _vm->_gameInfo->getSfxTrack(sfxId);
	int32 hash = MIXArchive::getHash(name);

	if (findLoopingTrackByHash(hash) >= 0) {
		return;
	}

	int i = findAvailableLoopingTrack();
	if (i == -1) {
		return;
	}
	LoopingSound &track = _loopingSounds[i];

	track.isActive = true;
	track.name = name;
	track.hash = hash;
	track.pan = pan;
	track.volume = volume;

	int actualVolumeStart = volume * _ambientVolume / 100;
	int actualVolumeEnd = actualVolumeStart;

	if (delay > 0u) {
		actualVolumeStart = 0;
	}

	track.audioPlayerTrack = _vm->_audioPlayer->playAud(name, actualVolumeStart, pan, pan, 99, kAudioPlayerLoop | kAudioPlayerOverrideVolume);

	if (track.audioPlayerTrack == -1) {
		removeLoopingSoundByIndex(i, 0);
	} else {
		if (delay) {
			_vm->_audioPlayer->adjustVolume(track.audioPlayerTrack, actualVolumeEnd, delay, false);
		}
	}
}

void AmbientSounds::adjustLoopingSound(int sfxId, int volume, int pan, uint32 delay) {
	int32 hash = MIXArchive::getHash(_vm->_gameInfo->getSfxTrack(sfxId));
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

void AmbientSounds::removeLoopingSound(int sfxId, uint32 delay) {
	int32 hash = MIXArchive::getHash(_vm->_gameInfo->getSfxTrack(sfxId));
	int index = findLoopingTrackByHash(hash);
	if (index >= 0) {
		removeLoopingSoundByIndex(index, delay);
	}
}

void AmbientSounds::removeAllLoopingSounds(uint32 delay) {
	for (int i = 0; i < kLoopingSounds; ++i) {
		removeLoopingSoundByIndex(i, delay);
	}
}

void AmbientSounds::tick() {
	uint32 now = _vm->_time->current();

	for (int i = 0; i != kNonLoopingSounds; ++i) {
		NonLoopingSound &track = _nonLoopingSounds[i];

		// unsigned difference is intentional
		if (!track.isActive || now - track.nextPlayTimeStart < track.nextPlayTimeDiff) {
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

		track.audioPlayerTrack = _vm->_audioPlayer->playAud(track.name,
															track.volume * _ambientVolume / 100,
															panStart,
															panEnd,
															track.priority,
															kAudioPlayerOverrideVolume);

		track.nextPlayTimeStart = now;
		track.nextPlayTimeDiff  = _vm->_rnd.getRandomNumberRng(track.timeMin, track.timeMax);
	}
}

void AmbientSounds::setVolume(int volume) {
	if (_loopingSounds) {
		for (int i = 0; i < kLoopingSounds; ++i) {
			if (_loopingSounds[i].isActive && _loopingSounds[i].audioPlayerTrack != -1) {
				int newVolume = _loopingSounds[i].volume * volume / 100;
				if (_vm->_audioPlayer->isActive(_loopingSounds[i].audioPlayerTrack)) {
					_vm->_audioPlayer->adjustVolume(_loopingSounds[i].audioPlayerTrack, newVolume, 1u, false);
				} else {
					_loopingSounds[i].audioPlayerTrack = _vm->_audioPlayer->playAud(_loopingSounds[i].name, 1, _loopingSounds[i].pan, _loopingSounds[i].pan, 99, kAudioPlayerLoop | kAudioPlayerOverrideVolume);
					if (_loopingSounds[i].audioPlayerTrack == -1) {
						removeLoopingSound(i, 0);
					} else {
						_vm->_audioPlayer->adjustVolume(_loopingSounds[i].audioPlayerTrack, newVolume, 1u, false);
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
	playSound(kSfxSPIN1A, 100, 0, 0, 0);
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
	const Common::String &name,
	uint32 timeMin, uint32 timeMax,
	int volumeMin, int volumeMax,
	int panStartMin, int panStartMax,
	int panEndMin, int panEndMax,
	int priority, int unk) {

	int i = findAvailableNonLoopingTrack();
	if (i < 0) {
		return;
	}

	NonLoopingSound &track = _nonLoopingSounds[i];

	uint32 now = _vm->_time->current();

#if BLADERUNNER_ORIGINAL_BUGS
#else
	sort(&timeMin, &timeMax);
	sort(&volumeMin, &volumeMax);
	sort(&panStartMin, &panStartMax);
	sort(&panEndMin, &panEndMax);
#endif // BLADERUNNER_ORIGINAL_BUGS

	track.isActive = true;
	track.name = name;
	track.hash = MIXArchive::getHash(name);
	track.timeMin = 1000u * timeMin;
	track.timeMax = 1000u * timeMax;
	track.nextPlayTimeStart = now;
	track.nextPlayTimeDiff  = _vm->_rnd.getRandomNumberRng(track.timeMin, track.timeMax);
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

void AmbientSounds::removeLoopingSoundByIndex(int index, uint32 delay) {
	LoopingSound &track = _loopingSounds[index];
	if (track.isActive && track.audioPlayerTrack != -1 && _vm->_audioPlayer->isActive(track.audioPlayerTrack)) {
		if (delay > 0u) {
			_vm->_audioPlayer->adjustVolume(track.audioPlayerTrack, 0, delay, false);
		} else {
			_vm->_audioPlayer->stop(track.audioPlayerTrack, false);
		}
	}
	track.isActive = false;
	track.name.clear();
	track.hash = 0;
	track.audioPlayerTrack = -1;
	track.volume = 0;
	track.pan = 0;
}

void AmbientSounds::save(SaveFileWriteStream &f) {
	f.writeBool(false); // _isDisabled - not used

	for (int i = 0; i != kNonLoopingSounds; ++i) {
		// 73 bytes per non-looping sound
		NonLoopingSound &track = _nonLoopingSounds[i];
		f.writeBool(track.isActive);
		f.writeStringSz(track.name, 13);
		f.writeSint32LE(track.hash);
		f.writeInt(-1); // track.audioPlayerTrack is not used after load
		f.writeInt(track.timeMin);
		f.writeInt(track.timeMax);
		f.writeInt(0); // track.nextPlayTime is not used after load
		f.writeInt(track.volumeMin);
		f.writeInt(track.volumeMax);
		f.writeInt(track.volume);
		f.writeInt(track.panStartMin);
		f.writeInt(track.panStartMax);
		f.writeInt(track.panEndMin);
		f.writeInt(track.panEndMax);
		f.writeInt(track.priority);
		f.padBytes(4); // field_45
	}

	for (int i = 0; i != kLoopingSounds; ++i) {
		// 33 bytes per looping sound
		LoopingSound &track = _loopingSounds[i];
		f.writeBool(track.isActive);
		f.writeStringSz(track.name, 13);
		f.writeSint32LE(track.hash);
		f.writeInt(-1); // track.audioPlayerTrack is not used after load
		f.writeInt(track.volume);
		f.writeInt(track.pan);
	}
}

void AmbientSounds::load(SaveFileReadStream &f) {
	removeAllLoopingSounds(0);
	removeAllNonLoopingSounds(true);

	f.skip(4); // _isDisabled - not used

	uint32 now = _vm->_time->getPauseStart();

	for (int i = 0; i != kNonLoopingSounds; ++i) {
		NonLoopingSound &track = _nonLoopingSounds[i];
		track.isActive = f.readBool();
		track.name = f.readStringSz(13);
		track.hash = f.readSint32LE();
		f.skip(4); // track.audioPlayerTrack is not used after load
		track.audioPlayerTrack = -1;
		track.timeMin = (uint32)f.readInt();
		track.timeMax = (uint32)f.readInt();
		f.skip(4); // track.nextPlayTime is not used after load
		track.nextPlayTimeStart = now;
#if BLADERUNNER_ORIGINAL_BUGS
#else
		sort(&(track.timeMin), &(track.timeMax));
#endif // BLADERUNNER_ORIGINAL_BUGS
		track.nextPlayTimeDiff  = _vm->_rnd.getRandomNumberRng(track.timeMin, track.timeMax);
		track.volumeMin = f.readInt();
		track.volumeMax = f.readInt();
		track.volume = f.readInt();
		track.panStartMin = f.readInt();
		track.panStartMax = f.readInt();
		track.panEndMin = f.readInt();
		track.panEndMax = f.readInt();
#if BLADERUNNER_ORIGINAL_BUGS
#else
		sort(&(track.volumeMin), &(track.volumeMax));
		sort(&(track.panStartMin), &(track.panStartMax));
		sort(&(track.panEndMin), &(track.panEndMax));
#endif // BLADERUNNER_ORIGINAL_BUGS
		track.priority = f.readInt();
		f.skip(4); // field_45
	}

	for (int i = 0; i != kLoopingSounds; ++i) {
		LoopingSound &track = _loopingSounds[i];
		track.isActive = f.readBool();
		track.name = f.readStringSz(13);
		track.hash = f.readSint32LE();
		f.skip(4); // track.audioPlayerTrack is not used after load
		track.audioPlayerTrack = -1;
		track.volume = f.readInt();
		track.pan = f.readInt();
	}

	for (int i = 0; i != kLoopingSounds; ++i) {
		LoopingSound &track = _loopingSounds[i];
		if (track.isActive) {
			track.audioPlayerTrack = _vm->_audioPlayer->playAud(track.name, 1, track.pan, track.pan, 99, kAudioPlayerLoop | kAudioPlayerOverrideVolume);
			if (track.audioPlayerTrack == -1) {
				removeLoopingSoundByIndex(i, 0);
			} else {
				_vm->_audioPlayer->adjustVolume(track.audioPlayerTrack, _ambientVolume * track.volume / 100, 2u, false);
			}
		}
	}
}

} // End of namespace BladeRunner
