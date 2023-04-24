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

	// The actual volume of ambient sounds is determined by the mixer (see BladeRunnerEngine::syncSoundSettings())
	// In our BladeRunner engine ambient sounds do not have a distinct sound type of their own,
	// so they are treated as kAmbientSoundType (default type, see: ambient_sounds.h).
	//
	// _ambientVolumeFactorOriginalEngine here sets a percentage to be applied on the ambient audio tracks' volume
	// before sending them to the audio player.
	// This is how the original engine set the volume via the in-game KIA volume slider controls.
	// Setting _ambientVolumeFactorOriginalEngine to 100, for the purposes ScummVM engine, renders it indifferent,
	// so sound volume can be controlled by ScummVM's Global Main Menu / ConfMan/ syncSoundSettings().
	// Note however that _ambientVolumeFactorOriginalEngine is also changed when entering and exiting ESPER and possibly VK modes.
	_ambientVolumeFactorOriginalEngine = BLADERUNNER_ORIGINAL_SETTINGS ? 65 : 100;

	for (int i = 0; i != kNonLoopingSounds; ++i) {
		NonLoopingSound &track = _nonLoopingSounds[i];
		track.isActive = false;
#if !BLADERUNNER_ORIGINAL_BUGS
		track.name.clear();
		track.hash = 0;
		track.audioPlayerTrack = -1;
		track.delayMin = 0u;
		track.delayMax = 0u;
		track.nextPlayTimeStart = 0u;
		track.nextPlayTimeDiff = 0u;
		track.volumeMin = 0;
		track.volumeMax = 0;
		track.volume = 0;
		track.panStartMin = 0;
		track.panStartMax = 0;
		track.panEndMin = 0;
		track.panEndMax = 0;
		track.priority = 0;
		track.soundType = -1;
#endif // !BLADERUNNER_ORIGINAL_BUGS
	}

	for (int i = 0; i != kLoopingSounds; ++i) {
		LoopingSound &track = _loopingSounds[i];
		track.isActive = false;
#if !BLADERUNNER_ORIGINAL_BUGS
		track.name.clear();
		track.hash = 0;
		track.audioPlayerTrack = -1;
		track.volume = 0;
		track.pan = 0;
		track.soundType = -1;
#endif // !BLADERUNNER_ORIGINAL_BUGS
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

//
// addSound() will add a track to the non-looping tracks array list
// it will use the kAmbientSoundType for Mixer's Sound Type.
// see AmbientSounds::tick()
// Used also by:
//            Ambient_Sounds_Add_Sound()
//            Spinner::chooseDestination()
// Calls:
//            addSoundByName()
//
// volumeMin, volumeMax should be in [0, 100]
// panStartMin, panStartMax should be in [-100, 100]
// panEndMin, panEndMax should be in [-100, 100], with "-101" being a special value for skipping pan (balance) adjustment
// priority should be in [0, 100]
void AmbientSounds::addSound(int sfxId,
                             uint32 delayMinSeconds, uint32 delayMaxSeconds,
                             int volumeMin, int volumeMax,
                             int panStartMin, int panStartMax,
                             int panEndMin, int panEndMax,
                             int priority, int unk) {
	debugC(6, kDebugSound, "AmbientSounds::addSound id:%d dMin:%d,dMax:%d vMin:%d,vMax:%d pSMin:%d,pSMax:%d pEMin:%d,pEMax:%d pr:%d unk:%d",
	       sfxId,
	       delayMinSeconds, delayMaxSeconds,
	       volumeMin, volumeMax,
	       panStartMin,panStartMax,
	       panEndMin, panEndMax,
	       priority, unk);
	addSoundByName(_vm->_gameInfo->getSfxTrack(sfxId),
	               delayMinSeconds, delayMaxSeconds,
	               volumeMin, volumeMax,
	               panStartMin, panStartMax,
	               panEndMin, panEndMax,
	               priority, unk);
}

void AmbientSounds::removeNonLoopingSound(int sfxId, bool stopPlaying) {
	debugC(6, kDebugSound, "AmbientSounds::removeNonLoopingSound id:%d stop:%d", sfxId, stopPlaying? 1: 0);
	int32 hash = MIXArchive::getHash(_vm->_gameInfo->getSfxTrack(sfxId));
	int index = findNonLoopingTrackByHash(hash);
	if (index >= 0) {
		removeNonLoopingSoundByIndex(index, stopPlaying);
	}
}

void AmbientSounds::removeAllNonLoopingSounds(bool stopPlaying) {
	debugC(6, kDebugSound, "AmbientSounds::removeAllNonLoopingSounds stop:%d", stopPlaying? 1: 0);
	for (int i = 0; i < kNonLoopingSounds; ++i) {
		removeNonLoopingSoundByIndex(i, stopPlaying);
	}
}

//
// addSpeech() will add a track to the non-looping tracks array list
// it will use the kAmbientSoundType for Mixer's Sound Type
// see AmbientSounds::tick()
// Mainly used for dispatch radio ambient sounds and blimp announcements.
// Called by:
//            Ambient_Sounds_Add_Speech_Sound()
// Calls:
//            addSoundByName()
//
// volumeMin, volumeMax should be in [0, 100]
// panStartMin, panStartMax should be in [-100, 100]
// panEndMin, panEndMax should be in [-100, 100], with "-101" being a special value for skipping pan (balance) adjustment
// priority should be in [0, 100]
void AmbientSounds::addSpeech(int actorId, int sentenceId,
                              uint32 delayMinSeconds, uint32 delayMaxSeconds,
                              int volumeMin, int volumeMax,
                              int panStartMin, int panStartMax,
                              int panEndMin, int panEndMax,
                              int priority, int unk) {
	debugC(6, kDebugSound, "AmbientSounds::addSpeech id:%d-%d dMin:%d,dMax:%d vMin:%d,vMax:%d pSMin:%d,pSMax:%d pEMin:%d,pEMax:%d pr:%d unk:%d",
	       actorId, sentenceId,
	       delayMinSeconds, delayMaxSeconds,
	       volumeMin, volumeMax,
	       panStartMin,panStartMax,
	       panEndMin, panEndMax,
	       priority, unk);
	Common::String name = Common::String::format( "%02d-%04d%s.AUD", actorId, sentenceId, _vm->_languageCode.c_str());
	addSoundByName(name,
	               delayMinSeconds, delayMaxSeconds,
	               volumeMin, volumeMax,
	               panStartMin, panStartMax,
	               panEndMin, panEndMax,
	               priority, unk);
}

// Explicitly plays a sound effect (sfx) track (specified by id)
// It does not add it as a track to the non-looping tracks array list
// It uses the parameter "type" as the mixer's sound type - which determines the volume setting in effect.
// By default sound type is kAmbientSoundType (see ambient_sounds.h).
//
// volume should be in [0, 100]
// panStart should be in [-100, 100]
// panEnd should be in [-100, 100]
// priority should be in [0, 100]
void AmbientSounds::playSound(int sfxId, int volume, int panStart, int panEnd, int priority, Audio::Mixer::SoundType type) {
	debugC(6, kDebugSound, "AmbientSounds::playSound id:%d v:%d pS:%d pE:%d pr:%d typ:%d", sfxId, volume, panStart, panEnd, priority, (int32) type);
	_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(sfxId), (volume * _ambientVolumeFactorOriginalEngine) / 100, panStart, panEnd, priority, kAudioPlayerOverrideVolume, type);
}

// Explicitly plays a speech cue
// It does not add it as a track to the non-looping tracks array list
// It uses mixer's sound type kSpeechSoundType - which determines the volume setting in effect (Speech)
//
// volume should be in [0, 100]
// panStart should be in [-100, 100]
// panEnd should be in [-100, 100]
// priority should be in [0, 100]
void AmbientSounds::playSpeech(int actorId, int sentenceId, int volume, int panStart, int panEnd, int priority) {
	debugC(6, kDebugSound, "AmbientSounds::playSpeech id:%d-%d v:%d pS:%d pE:%d pr:%d", actorId, sentenceId, volume, panStart, panEnd, priority);
	Common::String name = Common::String::format( "%02d-%04d%s.AUD", actorId, sentenceId, _vm->_languageCode.c_str());
	// (volume * _ambientVolume) / 100 should result in a value in [0, 100]
	_vm->_audioPlayer->playAud(name, (volume * _ambientVolumeFactorOriginalEngine) / 100, panStart, panEnd, priority, kAudioPlayerOverrideVolume, Audio::Mixer::kSpeechSoundType);
}

// Looping Sound will use paramerter "type" as the mixer's SoundType when playing this track.
// By default sound type is kAmbientSoundType (see ambient_sounds.h).
// This determines the volume setting that will be in effect for the audio.
//
// NOTE If restoring from a saved game, a looping track will always use the default SoundType (kAmbientSoundType)
// because sound type is not stored.
// TODO We could save the sound type re-using the space for field "track.audioPlayerTrack"
// which is skipped for *both* looping and non-looping tracks in save() and load() code
// However, the issue is negligible; the default SoundType for looping tracks is overridden
// only in one special case so far (restored content Outtake "FLYTRU_E.VQA", see: outtake.cpp)
//
// volume should be in [0, 100]
// pan should be in [-100, 100]
void AmbientSounds::addLoopingSound(int sfxId, int volume, int pan, uint32 delaySeconds, Audio::Mixer::SoundType type) {
	debugC(6, kDebugSound, "AmbientSounds::addLoopingSound id:%d v:%d p:%d d:%u typ:%d", sfxId, volume, pan, delaySeconds, (int32) type);
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
	track.soundType = (int32) type;

	int actualVolumeStart = (volume * _ambientVolumeFactorOriginalEngine) / 100;
	int actualVolumeEnd = actualVolumeStart;

	if (delaySeconds > 0u) {
		actualVolumeStart = 0;
	}

	// actualVolumeStart should be in [0, 100]
	// pan should be in [-100, 100]
	// priority should be in [0, 100]
	track.audioPlayerTrack = _vm->_audioPlayer->playAud(name, actualVolumeStart, pan, pan, 99, kAudioPlayerLoop | kAudioPlayerOverrideVolume, type);

	if (track.audioPlayerTrack == -1) {
		removeLoopingSoundByIndex(i, 0u);
	} else {
		if (delaySeconds) {
			_vm->_audioPlayer->adjustVolume(track.audioPlayerTrack, actualVolumeEnd, delaySeconds, false);
		}
	}
}

// volume should be in [0, 100], with "-1" being a special value for skipping volume adjustment
// pan should be in [-100, 100], with "-101" being a special value for skipping pan (balance) adjustment
void AmbientSounds::adjustLoopingSound(int sfxId, int volume, int pan, uint32 delaySeconds) {
	debugC(6, kDebugSound, "AmbientSounds::adjustLoopingSound id:%d v:%d p:%d d:%u", sfxId, volume, pan, delaySeconds);
	int32 hash = MIXArchive::getHash(_vm->_gameInfo->getSfxTrack(sfxId));
	int index = findLoopingTrackByHash(hash);

	if (index >= 0 && _loopingSounds[index].audioPlayerTrack != -1 && _vm->_audioPlayer->isActive(_loopingSounds[index].audioPlayerTrack)) {
		if (volume != -1) {
			_loopingSounds[index].volume = volume;
			_vm->_audioPlayer->adjustVolume(_loopingSounds[index].audioPlayerTrack, (volume * _ambientVolumeFactorOriginalEngine) / 100, delaySeconds, false);
		}
		if (pan != -101) {
			_loopingSounds[index].pan = pan;
			_vm->_audioPlayer->adjustPan(_loopingSounds[index].audioPlayerTrack, pan, delaySeconds);
		}
	}
}

void AmbientSounds::removeLoopingSound(int sfxId, uint32 delaySeconds) {
	debugC(6, kDebugSound, "AmbientSounds::removeLoopingSound id:%d d:%u", sfxId, delaySeconds);
	int32 hash = MIXArchive::getHash(_vm->_gameInfo->getSfxTrack(sfxId));
	int index = findLoopingTrackByHash(hash);
	if (index >= 0) {
		removeLoopingSoundByIndex(index, delaySeconds);
	}
}

void AmbientSounds::removeAllLoopingSounds(uint32 delaySeconds) {
	debugC(6, kDebugSound, "AmbientSounds::removeAllLoopingSounds d:%u", delaySeconds);
	for (int i = 0; i < kLoopingSounds; ++i) {
		removeLoopingSoundByIndex(i, delaySeconds);
	}
}

// tick() only handles the non-looping added ambient sounds
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
		// typically when using the -101 special value, both panEndMin and panEndMax have this value
		// -101 here means "do not adjust the panning for the track (while it's playing)"
		if (track.panEndMin != -101) {
			panEnd = _vm->_rnd.getRandomNumberRng(track.panEndMin, track.panEndMax);
		} else {
			panEnd = panStart;
		}

		track.volume = _vm->_rnd.getRandomNumberRng(track.volumeMin, track.volumeMax);

		Audio::Mixer::SoundType mixerAmbientSoundType = kAmbientSoundType;
		if (track.soundType >= 0) {
			mixerAmbientSoundType = (Audio::Mixer::SoundType) track.soundType;
		}
		track.audioPlayerTrack = _vm->_audioPlayer->playAud(track.name,
		                                                    (track.volume * _ambientVolumeFactorOriginalEngine) / 100,
		                                                    panStart,
		                                                    panEnd,
		                                                    track.priority,
		                                                    kAudioPlayerOverrideVolume,
		                                                    mixerAmbientSoundType);

		track.nextPlayTimeStart = now;
		track.nextPlayTimeDiff  = _vm->_rnd.getRandomNumberRng(track.delayMin, track.delayMax);
	}
}

// TODO Evaluate if for ScummVM we can avoid using and modifying _ambientVolumeFactorOriginalEngine altogether.
// While we no longer use the original engine's mechanism to set the ambient sounds volume
// with the AmbientSounds::setVolume() public method, when using the in-game KIA volume slider,
// we do use this method as did the original engine to temporarily set the volume levels
// in ESPER and VK modes.
// This affects only looping ambient sounds.
// volume should be in [0, 100]
void AmbientSounds::setVolume(int volume) {
	debugC(6, kDebugSound, "AmbientSounds::setVolume v:%d", volume);
	if (_loopingSounds) {
		for (int i = 0; i < kLoopingSounds; ++i) {
			if (_loopingSounds[i].isActive && _loopingSounds[i].audioPlayerTrack != -1) {
				int newVolume = (_loopingSounds[i].volume * volume) / 100;
				if (_vm->_audioPlayer->isActive(_loopingSounds[i].audioPlayerTrack)) {
					_vm->_audioPlayer->adjustVolume(_loopingSounds[i].audioPlayerTrack, newVolume, 1u, false);
				} else {
					Audio::Mixer::SoundType mixerAmbientSoundType = kAmbientSoundType;
					if (_loopingSounds[i].soundType >= 0) {
						mixerAmbientSoundType = (Audio::Mixer::SoundType) _loopingSounds[i].soundType;
					}
					_loopingSounds[i].audioPlayerTrack = _vm->_audioPlayer->playAud(_loopingSounds[i].name, 1, _loopingSounds[i].pan, _loopingSounds[i].pan, 99, kAudioPlayerLoop | kAudioPlayerOverrideVolume, mixerAmbientSoundType);
					if (_loopingSounds[i].audioPlayerTrack == -1) {
						removeLoopingSound(i, 0u);
					} else {
						_vm->_audioPlayer->adjustVolume(_loopingSounds[i].audioPlayerTrack, newVolume, 1u, false);
					}
				}
			}
		}
	}
	_ambientVolumeFactorOriginalEngine = volume;
}

int AmbientSounds::getVolume() const {
	return _ambientVolumeFactorOriginalEngine;
}

void AmbientSounds::playSample() {
	// Original uses priority 0 here also
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

// volumeMin, volumeMax should be in [0, 100]
// panStartMin, panStartMax should be in [-100, 100]
// panEndMin, panEndMax should be in [-100, 100], with "-101" being a special value for skipping pan (balance) adjustment
// priority should be in [0, 100]
void AmbientSounds::addSoundByName(const Common::String &name,
                                   uint32 delayMinSeconds, uint32 delayMaxSeconds,
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

#if !BLADERUNNER_ORIGINAL_BUGS
	sort(&delayMinSeconds, &delayMaxSeconds);
#endif // BLADERUNNER_ORIGINAL_BUGS
	sort(&volumeMin, &volumeMax);
	sort(&panStartMin, &panStartMax);
	sort(&panEndMin, &panEndMax);

	track.isActive = true;
	track.name = name;
	track.hash = MIXArchive::getHash(name);
	track.delayMin = 1000u * delayMinSeconds; // store as milliseconds
	track.delayMax = 1000u * delayMaxSeconds; // store as milliseconds
	track.nextPlayTimeStart = now;
	track.nextPlayTimeDiff  = _vm->_rnd.getRandomNumberRng(track.delayMin, track.delayMax);
	track.volumeMin = volumeMin;
	track.volumeMax = volumeMax;
	track.volume = 0;
	track.panStartMin = panStartMin;
	track.panStartMax = panStartMax;
	track.panEndMin = panEndMin;
	track.panEndMax = panEndMax;
	track.priority = priority;
	track.soundType = -1;
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
	track.soundType = -1;
#if !BLADERUNNER_ORIGINAL_BUGS
	track.name.clear();
	track.hash = 0;
	track.delayMin = 0u;
	track.delayMax = 0u;
	track.nextPlayTimeStart = 0u;
	track.nextPlayTimeDiff = 0u;
	track.volumeMin = 0;
	track.volumeMax = 0;
	track.volume = 0;
	track.panStartMin = 0;
	track.panStartMax = 0;
	track.panEndMin = 0;
	track.panEndMax = 0;
	track.priority = 0;
#endif // !BLADERUNNER_ORIGINAL_BUGS
}

void AmbientSounds::removeLoopingSoundByIndex(int index, uint32 delaySeconds) {
	LoopingSound &track = _loopingSounds[index];
	if (track.isActive && track.audioPlayerTrack != -1 && _vm->_audioPlayer->isActive(track.audioPlayerTrack)) {
		if (delaySeconds > 0u) {
			_vm->_audioPlayer->adjustVolume(track.audioPlayerTrack, 0, delaySeconds, false);
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
	track.soundType = -1;
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
		f.writeInt(track.delayMin);
		f.writeInt(track.delayMax);
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
	removeAllLoopingSounds(0u);
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
		track.delayMin = (uint32)f.readInt();
		track.delayMax = (uint32)f.readInt();
		f.skip(4); // track.nextPlayTime is not used after load
		track.nextPlayTimeStart = now;
#if BLADERUNNER_ORIGINAL_BUGS
		track.nextPlayTimeDiff  = _vm->_rnd.getRandomNumberRng(track.delayMin, track.delayMax);
#endif // BLADERUNNER_ORIGINAL_BUGS
		track.volumeMin = f.readInt();
		track.volumeMax = f.readInt();
		track.volume = f.readInt();
		track.panStartMin = f.readInt();
		track.panStartMax = f.readInt();
		track.panEndMin = f.readInt();
		track.panEndMax = f.readInt();
		track.priority = f.readInt();
		f.skip(4); // field_45
		track.soundType = -1;
#if !BLADERUNNER_ORIGINAL_BUGS
		// Since unused ambient sound track fields are unitialized
		// don't keep garbage field values for non-active tracks
		// This was basically an issue when calling _vm->_rnd.getRandomNumberRng()
		// with uninitialized fields, but it's a good practice to sanitize the fields here anyway
		if (!track.isActive) {
			track.delayMin = 0u;
			track.delayMax = 0u;
			track.nextPlayTimeDiff  = 0u;
			track.volumeMin = 0;
			track.volumeMax = 0;
			track.volume = 0;
			track.panStartMin = 0;
			track.panStartMax = 0;
			track.panEndMin = 0;
			track.panEndMax = 0;
			track.priority = 0;
		} else {
			sort(&(track.delayMin), &(track.delayMax));
			track.nextPlayTimeDiff  = _vm->_rnd.getRandomNumberRng(track.delayMin, track.delayMax);
			sort(&(track.volumeMin), &(track.volumeMax));
			sort(&(track.panStartMin), &(track.panStartMax));
			sort(&(track.panEndMin), &(track.panEndMax));
		}
#endif // !BLADERUNNER_ORIGINAL_BUGS
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
		track.soundType = -1;
#if !BLADERUNNER_ORIGINAL_BUGS
		// Since unused ambient sound track fields are unitialized
		// don't keep garbage field values for non-active tracks
		if (!track.isActive) {
			track.volume = 0;
			track.pan = 0;
		}
#endif // !BLADERUNNER_ORIGINAL_BUGS
	}

	for (int i = 0; i != kLoopingSounds; ++i) {
		LoopingSound &track = _loopingSounds[i];
		if (track.isActive) {
			Audio::Mixer::SoundType mixerAmbientSoundType = kAmbientSoundType;
			if (track.soundType >= 0) {
				mixerAmbientSoundType = (Audio::Mixer::SoundType) track.soundType;
			}
			// Looping sound (loaded) gets high priority (99)
			// Also started with volume at 1 (but adjusted below appropriately)
			track.audioPlayerTrack = _vm->_audioPlayer->playAud(track.name, 1, track.pan, track.pan, 99, kAudioPlayerLoop | kAudioPlayerOverrideVolume, mixerAmbientSoundType);
			if (track.audioPlayerTrack == -1) {
				removeLoopingSoundByIndex(i, 0u);
			} else {
				_vm->_audioPlayer->adjustVolume(track.audioPlayerTrack, (track.volume * _ambientVolumeFactorOriginalEngine) / 100, 2u, false);
			}
		}
	}
}

} // End of namespace BladeRunner
