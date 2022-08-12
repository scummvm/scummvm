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

#include "common/system.h"
#include "common/timer.h"

#include "scumm/actor.h"
#include "scumm/scumm_v7.h"
#include "scumm/sound.h"
#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/imuse_digi/dimuse_defs.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"
#include "scumm/imuse_digi/dimuse_sndmgr.h"
#include "scumm/imuse_digi/dimuse_tables.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Scumm {

void IMuseDigital::timer_handler(void *refCon) {
	IMuseDigital *diMUSE = (IMuseDigital *)refCon;
	diMUSE->callback();
}

IMuseDigital::IMuseDigital(ScummEngine_v7 *scumm, Audio::Mixer *mixer)
	: _vm(scumm), _mixer(mixer) {
	assert(_vm);
	assert(mixer);

	// 50 Hz rate for the callback
	_callbackFps = 50;
	_usecPerInt = 20000;

	_splayer = nullptr;
	_isEarlyDiMUSE = (_vm->_game.id == GID_FT || (_vm->_game.id == GID_DIG && _vm->_game.features & GF_DEMO));

	if (_isEarlyDiMUSE) {
		memset(_ftCrossfadeBuffer, 0, sizeof(_ftCrossfadeBuffer));
	}

	_curMixerMusicVolume = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
	_curMixerSpeechVolume = _mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType);
	_curMixerSFXVolume = _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType);
	_currentSpeechVolume = 0;
	_currentSpeechFrequency = 0;
	_currentSpeechPan = 0;

	_waveOutXorTrigger = 0;
	_waveOutWriteIndex = 0;
	_waveOutDisableWrite = 0;
	_waveOutPreferredFeedSize = 0;

	_dispatchFadeSize = 0;

	_stopSequenceFlag = 0;
	_scriptInitializedFlag = 0;
	_callbackInterruptFlag = 0;
	_spooledMusicEnabled = true;

	_radioChatterSFX = false;
	_isEngineDisabled = false;

	_audioNames = nullptr;
	_numAudioNames = 0;

	_emptyMarker[0] = '\0';
	_internalMixer = new IMuseDigiInternalMixer(mixer, _isEarlyDiMUSE);
	_groupsHandler = new IMuseDigiGroupsHandler(this);
	_fadesHandler = new IMuseDigiFadesHandler(this);
	_triggersHandler = new IMuseDigiTriggersHandler(this);
	_filesHandler = new IMuseDigiFilesHandler(this, scumm);

	diMUSEInitialize();
	diMUSEInitializeScript();
	if (_vm->_game.id == GID_CMI) {
		_filesHandler->allocSoundBuffer(DIMUSE_BUFFER_SPEECH, 176000, 44000, 88000);
		_filesHandler->allocSoundBuffer(DIMUSE_BUFFER_MUSIC, 528000, 44000, 352000);
	} else if (_vm->_game.id == GID_DIG && !isFTSoundEngine()) {
		_filesHandler->allocSoundBuffer(DIMUSE_BUFFER_SPEECH, 132000, 22000, 44000);
		_filesHandler->allocSoundBuffer(DIMUSE_BUFFER_MUSIC, 660000, 11000, 132000);
	} else {
		_filesHandler->allocSoundBuffer(DIMUSE_BUFFER_SPEECH, 110000, 22000, 44000);
		_filesHandler->allocSoundBuffer(DIMUSE_BUFFER_MUSIC, 220000, 22000, 44000);
	}

	_filesHandler->allocSoundBuffer(DIMUSE_BUFFER_SMUSH, 198000, 0, 0);

	if (_mixer->getOutputBufSize() != 0) {
		// Let's find the optimal value for the maximum number of streams which can stay in the queue at once;
		// (A number which is too low can lead to buffer underrun, while the higher the number is, the higher is the audio latency)
		_maxQueuedStreams = (int)ceil((_mixer->getOutputBufSize() / _waveOutPreferredFeedSize) / ((float)_mixer->getOutputRate() / DIMUSE_SAMPLERATE));

		// This mixer's optimal output sample rate for this audio engine is one which is a multiple of 22050Hz;
		// if we're dealing with one which is a multiple of 48000Hz, compensate the number of queued streams...
		if (_mixer->getOutputRate() % DIMUSE_SAMPLERATE) {
			_maxQueuedStreams++;
		}

		// The lower optimal bound is always 5, except if we're operating in low latency mode
		_maxQueuedStreams = MAX(_mixer->getOutputBufSize() <= 1024 ? 4 : 5, _maxQueuedStreams);
	} else {
		debug(5, "IMuseDigital::IMuseDigital(): WARNING: output audio buffer size not specified for this platform, defaulting _maxQueuedStreams to 4");
		_maxQueuedStreams = 4;
	}

	_nominalBufferCount = _maxQueuedStreams;

	_vm->getTimerManager()->installTimerProc(timer_handler, 1000000 / _callbackFps, this, "IMuseDigital");
}

IMuseDigital::~IMuseDigital() {
	_vm->getTimerManager()->removeTimerProc(timer_handler);
	_filesHandler->deallocSoundBuffer(DIMUSE_BUFFER_SPEECH);
	_filesHandler->deallocSoundBuffer(DIMUSE_BUFFER_MUSIC);
	_filesHandler->deallocSoundBuffer(DIMUSE_BUFFER_SMUSH);
	cmdsDeinit();
	diMUSETerminate();
	delete _internalMixer;
	delete _groupsHandler;
	delete _fadesHandler;
	delete _triggersHandler;
	delete _filesHandler;

	// Deinit the Dispatch module
	free(_dispatchBuffer);
	_dispatchBuffer = nullptr;

	// Deinit the WaveOut module
	free(_waveOutOutputBuffer);
	_waveOutOutputBuffer = nullptr;

	free(_audioNames);
}

int IMuseDigital::roundRobinSetBufferCount() {
	int minStreams = _nominalBufferCount - 3;
	int maxStreams = _nominalBufferCount + 3;
	_maxQueuedStreams++;

	if (_maxQueuedStreams > maxStreams) {
		_maxQueuedStreams = minStreams;
	}

	return _maxQueuedStreams;
}

void IMuseDigital::stopSound(int sound) {
	diMUSEStopSound(sound);
}

void IMuseDigital::stopAllSounds() {
	diMUSEStopAllSounds();
}

int IMuseDigital::isSoundRunning(int soundId) {
	return diMUSEGetParam(soundId, DIMUSE_P_SND_TRACK_NUM) > 0;
}

int IMuseDigital::startVoice(int soundId, const char *soundName, byte speakingActorId) {
	_filesHandler->closeSoundImmediatelyById(soundId);

	int fileDoesNotExist = 0;
	if (_vm->_game.id == GID_DIG) {
		if (!strcmp(soundName, "PIG.018"))
			fileDoesNotExist = _filesHandler->setCurrentSpeechFilename("PIG.019");
		else
			fileDoesNotExist = _filesHandler->setCurrentSpeechFilename(soundName);

		if (fileDoesNotExist)
			return 1;

		// WORKAROUND for this particular sound file not playing (this is a bug in the original):
		// this is happening because the sound buffer responsible for speech
		// is still busy with the previous speech file playing during the SAN
		// movie. We just stop the SMUSH speech sound before playing NEXUS.029.
		if (!strcmp(soundName, "NEXUS.029")) {
			diMUSEStopSound(DIMUSE_SMUSH_SOUNDID + DIMUSE_BUFFER_SPEECH);
		}

		// Set up a trigger for extracting mouth sync times;
		// see Sound::extractSyncsFromDiMUSEMarker() for details.
		// Setting up a trigger with an empty marker is a shortcut for
		// activating the trigger for any marker.
		diMUSESetTrigger(kTalkSoundID, 0, 21);

		diMUSEStartStream(kTalkSoundID, 127, DIMUSE_BUFFER_SPEECH);
		diMUSESetParam(kTalkSoundID, DIMUSE_P_GROUP, DIMUSE_GROUP_SPEECH);
		if (speakingActorId == _vm->VAR(_vm->VAR_EGO)) {
			diMUSESetParam(kTalkSoundID, DIMUSE_P_MAILBOX, 0);
			diMUSESetParam(kTalkSoundID, DIMUSE_P_VOLUME, 127);
		} else {
			diMUSESetParam(kTalkSoundID, DIMUSE_P_MAILBOX, _radioChatterSFX);
			diMUSESetParam(kTalkSoundID, DIMUSE_P_VOLUME, 88);
		}
		_filesHandler->closeSound(kTalkSoundID);
	} else if (_vm->_game.id == GID_CMI) {
		fileDoesNotExist = _filesHandler->setCurrentSpeechFilename(soundName);
		if (fileDoesNotExist)
			return 1;

		diMUSEStartStream(kTalkSoundID, 127, DIMUSE_BUFFER_SPEECH);
		diMUSESetParam(kTalkSoundID, DIMUSE_P_GROUP, DIMUSE_GROUP_SPEECH);

		// Let's not give the occasion to raise errors here
		if (_vm->isValidActor(_vm->VAR(_vm->VAR_TALK_ACTOR))) {
			Actor *a = _vm->derefActor(_vm->VAR(_vm->VAR_TALK_ACTOR), "IMuseDigital::startVoice");
			if (_vm->VAR(_vm->VAR_VOICE_MODE) == 2)
				diMUSESetParam(kTalkSoundID, DIMUSE_P_VOLUME, 0);
			else
				diMUSESetParam(kTalkSoundID, DIMUSE_P_VOLUME, a->_talkVolume);

			diMUSESetParam(kTalkSoundID, DIMUSE_P_TRANSPOSE, a->_talkFrequency);
			diMUSESetParam(kTalkSoundID, DIMUSE_P_PAN, a->_talkPan);

			_currentSpeechVolume = a->_talkVolume;
			_currentSpeechFrequency = a->_talkFrequency;
			_currentSpeechPan = a->_talkPan;
		}

		// The interpreter really calls for processStreams two times in a row,
		// and who am I to contradict it?
		diMUSEProcessStreams();
		diMUSEProcessStreams();
	}

	return 0;
}

// Used by FT and DIG demo
int IMuseDigital::startVoice(const char *fileName, ScummFile *file, uint32 offset, uint32 size) {
	_filesHandler->setCurrentFtSpeechFile(fileName, file, offset, size);
	diMUSEStopSound(kTalkSoundID);
	diMUSEStartStream(kTalkSoundID, 127, DIMUSE_BUFFER_SPEECH);
	diMUSESetParam(kTalkSoundID, DIMUSE_P_GROUP, DIMUSE_GROUP_SPEECH);
	return 0;
}

static void skipLegacyTrackEntry(Common::Serializer &s) {
	s.skip(1, VER(31)); // t.pan
	s.skip(4, VER(31)); // t.vol
	s.skip(4, VER(31)); // t.volFadeDest
	s.skip(4, VER(31)); // t.volFadeStep
	s.skip(4, VER(31)); // t.volFadeDelay
	s.skip(1, VER(31)); // t.volFadeUsed
	s.skip(4, VER(31)); // t.soundId
	s.skip(15, VER(31)); // t.soundName
	s.skip(1, VER(31)); // t.used
	s.skip(1, VER(31)); // t.toBeRemoved
	s.skip(1, VER(31)); // t.souStreamUsed
	s.skip(1, VER(31), VER(76)); // mixerStreamRunning
	s.skip(4, VER(31)); // t.soundPriority
	s.skip(4, VER(31)); // t.regionOffset
	s.skip(4, VER(31), VER(31)); // trackOffset
	s.skip(4, VER(31)); // t.dataOffset
	s.skip(4, VER(31)); // t.curRegion
	s.skip(4, VER(31)); // t.curHookId
	s.skip(4, VER(31)); // t.volGroupId
	s.skip(4, VER(31)); // t.soundType
	s.skip(4, VER(31)); // t.feedSize
	s.skip(4, VER(31)); // t.dataMod12Bit
	s.skip(4, VER(31)); // t.mixerFlags
	s.skip(4, VER(31), VER(42)); // mixerVol
	s.skip(4, VER(31), VER(42)); // mixerPan
	s.skip(1, VER(45)); // t.sndDataExtComp
}

void IMuseDigital::saveLoadEarly(Common::Serializer &s) {
	Common::StackLock lock(_mutex, "IMuseDigital::saveLoadEarly()");

	if (s.isLoading()) {
		diMUSEStopAllSounds();
		_filesHandler->closeSoundImmediatelyById(kTalkSoundID);
	}

	if (s.getVersion() < 103) {
		// Just load the current state and sequence, and play them
		debug(5, "IMuseDigital::saveLoadEarly(): old savegame detected (version %d), game may load with an undesired audio status", s.getVersion());
		s.skip(4, VER(31), VER(42)); // _volVoice
		s.skip(4, VER(31), VER(42)); // _volSfx
		s.skip(4, VER(31), VER(42)); // _volMusic
		s.syncAsSint32LE(_curMusicState, VER(31));
		s.syncAsSint32LE(_curMusicSeq, VER(31));
		s.syncAsSint32LE(_curMusicCue, VER(31));
		s.syncAsSint32LE(_nextSeqToPlay, VER(31));
		s.syncAsByte(_radioChatterSFX, VER(76));
		s.syncArray(_attributes, 188, Common::Serializer::Sint32LE, VER(31));

		for (int j = 0; j < 16; ++j)
			skipLegacyTrackEntry(s);

		int stateSoundId = 0;
		int seqSoundId = 0;

		if (_vm->_game.id == GID_DIG) {
			stateSoundId = _digStateMusicTable[_curMusicState].soundId;
			seqSoundId = _digSeqMusicTable[_curMusicSeq].soundId;
		} else {
			if (_vm->_game.features & GF_DEMO) {
				stateSoundId = _comiDemoStateMusicTable[_curMusicState].soundId;
			} else {
				stateSoundId = _comiStateMusicTable[_curMusicState].soundId;
				seqSoundId = _comiSeqMusicTable[_curMusicSeq].soundId;
			}
		}

		_curMusicState = 0;
		_curMusicSeq = 0;
		scriptSetSequence(seqSoundId);
		scriptSetState(stateSoundId);
		scriptSetCuePoint(_curMusicCue);
		_curMusicCue = 0;
	} else {
		diMUSESaveLoad(s);

		if (s.isLoading() && _vm->isUsingOriginalGUI()) {
			diMUSESetMusicGroupVol(diMUSEGetMusicGroupVol());
			diMUSESetVoiceGroupVol(diMUSEGetVoiceGroupVol());
			diMUSESetSFXGroupVol(diMUSEGetSFXGroupVol());
		}
	}
}

void IMuseDigital::refreshScripts() {
	if (isFTSoundEngine()) {
		diMUSEProcessStreams();
	} else if (!_vm->isSmushActive()) {
		diMUSEProcessStreams();
		diMUSERefreshScript();
	}
}

void IMuseDigital::setRadioChatterSFX(bool state) {
	_radioChatterSFX = state;
}

int IMuseDigital::startSfx(int soundId, int priority) {
	diMUSEStartSound(soundId, priority);
	diMUSESetParam(soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_SFX);
	return 0;
}

void IMuseDigital::callback() {
	if (_cmdsPauseCount)
		return;

	if (!_callbackInterruptFlag) {
		_callbackInterruptFlag = 1;
		diMUSEHeartbeat();
		_callbackInterruptFlag = 0;
	}
}

void IMuseDigital::diMUSEHeartbeat() {
	// This is what happens:
	// - Usual audio stuff like fetching and playing sound (and everything
	//   within waveOutCallback()) happens at a base 50Hz rate;
	// - Triggers and fades handling happens at a (somewhat hacky) 60Hz rate;
	// - Music gain reduction happens at a 10Hz rate.

	int soundId, foundGroupId, musicTargetVolume, musicEffVol, musicVol, tempVol, tempEffVol, factor, step;

	waveOutCallback();

	if (!_vm->isUsingOriginalGUI()) {
		// Update volumes

		if (_curMixerMusicVolume != _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType)) {
			_curMixerMusicVolume = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
			diMUSESetMusicGroupVol(CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 2, 0, 127));
		}

		if (_curMixerSpeechVolume != _mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType)) {
			_curMixerSpeechVolume = _mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType);
			diMUSESetVoiceGroupVol(CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType) / 2, 0, 127));
		}

		if (_curMixerSFXVolume != _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType)) {
			_curMixerSFXVolume = _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType);
			diMUSESetSFXGroupVol(CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 2, 0, 127));
		}
	}

	// Handle fades and triggers

	_cmdsRunning60HzCount += _usecPerInt;
	while (_cmdsRunning60HzCount >= 16667) {
		_cmdsRunning60HzCount -= 16667;
		_fadesHandler->loop();
		_triggersHandler->loop();
	}

	_cmdsRunning10HzCount += _usecPerInt;
	if (_cmdsRunning10HzCount < 100000)
		return;

	do {
		// SPEECH GAIN REDUCTION 10Hz
		_cmdsRunning10HzCount -= 100000;
		soundId = 0;
		musicTargetVolume = _groupsHandler->setGroupVol(DIMUSE_GROUP_MUSIC, -1);
		while (1) { // Check all tracks to see if there's a speech file playing
			soundId = waveGetNextSound(soundId);
			if (!soundId)
				break;

			foundGroupId = -1;
			if (_filesHandler->getNextSound(soundId) == 2) {
				foundGroupId = waveGetParam(soundId, DIMUSE_P_GROUP); // Check the groupId of this sound
			}

			if (foundGroupId == DIMUSE_GROUP_SPEECH) {
				// Remember: when a speech file stops playing this block stops
				// being executed, so musicTargetVolume returns back to its original value
				factor = _isEarlyDiMUSE ? 82 : 80;
				musicTargetVolume = (musicTargetVolume * factor) / 128;
				break;
			}
		}

		musicEffVol = _groupsHandler->setGroupVol(DIMUSE_GROUP_MUSICEFF, -1); // MUSIC EFFECTIVE VOLUME GROUP (used for gain reduction)
		musicVol = _groupsHandler->setGroupVol(DIMUSE_GROUP_MUSIC, -1); // MUSIC VOLUME SUBGROUP (keeps track of original music volume)

		if (musicEffVol < musicTargetVolume) { // If there is gain reduction already going on...
			tempEffVol = musicEffVol + 3;
			if (tempEffVol < musicTargetVolume) {
				if (musicVol <= tempEffVol) {
					musicVol = tempEffVol;
				}
			} else if (musicVol <= musicTargetVolume) { // Bring up the music volume immediately when speech stops playing
				musicVol = musicTargetVolume;
			}
			_groupsHandler->setGroupVol(DIMUSE_GROUP_MUSICEFF, musicVol);
		} else if (musicEffVol > musicTargetVolume) {
			// Bring music volume down to target volume with a -18 (or -6 for FT & DIG demo) step
		    // if there's speech playing or else, just cap it to the target if it's out of range
			step = _isEarlyDiMUSE ? 6 : 18;
			tempVol = musicEffVol - step;
			if (tempVol <= musicTargetVolume) {
				if (musicVol >= musicTargetVolume) {
					musicVol = musicTargetVolume;
				}
			} else {
				if (musicVol >= tempVol) {
					musicVol = tempVol;
				}
			}
			_groupsHandler->setGroupVol(DIMUSE_GROUP_MUSICEFF, musicVol);
		}

	} while (_cmdsRunning10HzCount >= 100000);
}

void IMuseDigital::setPriority(int soundId, int priority) {
	diMUSESetParam(soundId, DIMUSE_P_PRIORITY, priority);
}

void IMuseDigital::setVolume(int soundId, int volume) {
	diMUSESetParam(soundId, DIMUSE_P_VOLUME, volume);
	if (soundId == kTalkSoundID)
		_currentSpeechVolume = volume;
}

void IMuseDigital::setPan(int soundId, int pan) {
	diMUSESetParam(soundId, DIMUSE_P_PAN, pan);
	if (soundId == kTalkSoundID)
		_currentSpeechPan = pan;
}

void IMuseDigital::setFrequency(int soundId, int frequency) {
	diMUSESetParam(soundId, DIMUSE_P_TRANSPOSE, frequency);
	if (soundId == kTalkSoundID)
		_currentSpeechFrequency = frequency;
}

int IMuseDigital::getCurSpeechVolume() const {
	return _currentSpeechVolume;
}

int IMuseDigital::getCurSpeechPan() const {
	return _currentSpeechPan;
}

int IMuseDigital::getCurSpeechFrequency() const {
	return _currentSpeechFrequency;
}

void IMuseDigital::flushTracks() {
	_filesHandler->flushSounds();
}

// This is used in order to avoid crash everything
// if a compressed audio resource file is found
void IMuseDigital::disableEngine() {
	_isEngineDisabled = true;
}

bool IMuseDigital::isEngineDisabled() {
	return _isEngineDisabled;
}

void IMuseDigital::stopSMUSHAudio() {
	if (!isFTSoundEngine()) {
		if (_vm->_game.id == GID_DIG) {
			int foundSoundId, paused;
			int32 bufSize, criticalSize, freeSpace;
			foundSoundId = diMUSEGetNextSound(0);
			while (foundSoundId) {
				if (diMUSEGetParam(foundSoundId, DIMUSE_P_SND_HAS_STREAM)) {
					diMUSEQueryStream(foundSoundId, bufSize, criticalSize, freeSpace, paused);

					// Here, the original engine for DIG explicitly asks for "bufSize == 193900";
					// since this works half of the time in ScummVM (because of how we handle sound buffers),
					// we do the check but we alternatively check for the SMUSH channel soundId, so to cover the
					// remaining cases. This fixes instances in which exiting from a cutscene leaves both
					// DiMUSE streams locked, with speech consequently unable to play and a "WARNING: three
					// streams in use" message from streamerProcessStreams()
					if (bufSize == 193900 || foundSoundId == DIMUSE_SMUSH_SOUNDID + DIMUSE_BUFFER_SMUSH)
						diMUSEStopSound(foundSoundId);
				}

				foundSoundId = diMUSEGetNextSound(foundSoundId);
			}
		}

		diMUSESetSequence(0);
	}
}

bool IMuseDigital::isFTSoundEngine() {
	return _isEarlyDiMUSE;
}

int32 IMuseDigital::getCurMusicPosInMs() {
	int soundId, curSoundId;

	curSoundId = 0;
	soundId = 0;
	while (1) {
		curSoundId = diMUSEGetNextSound(curSoundId);
		if (!curSoundId)
			break;

		if (diMUSEGetParam(curSoundId, DIMUSE_P_SND_HAS_STREAM) && diMUSEGetParam(curSoundId, DIMUSE_P_STREAM_BUFID) == DIMUSE_BUFFER_MUSIC) {
			soundId = curSoundId;
			return diMUSEGetParam(soundId, DIMUSE_P_SND_POS_IN_MS);
		}
	}

	return diMUSEGetParam(soundId, DIMUSE_P_SND_POS_IN_MS);
}

int32 IMuseDigital::getCurVoiceLipSyncWidth() {
	int32 width, height;
	getSpeechLipSyncInfo(width, height);
	return width;
}

int32 IMuseDigital::getCurVoiceLipSyncHeight() {
	int32 width, height;
	getSpeechLipSyncInfo(width, height);
	return height;
}

int32 IMuseDigital::getCurMusicLipSyncWidth(int syncId) {
	int32 width, height;
	getMusicLipSyncInfo(syncId, width, height);
	return width;
}

int32 IMuseDigital::getCurMusicLipSyncHeight(int syncId) {
	int32 width, height;
	getMusicLipSyncInfo(syncId, width, height);
	return height;
}

void IMuseDigital::getSpeechLipSyncInfo(int32 &width, int32 &height) {
	int curSpeechPosInMs;

	width = 0;
	height = 0;

	if (diMUSEGetParam(kTalkSoundID, DIMUSE_P_SND_TRACK_NUM) > 0) {
		curSpeechPosInMs = diMUSEGetParam(kTalkSoundID, DIMUSE_P_SND_POS_IN_MS);
		diMUSELipSync(kTalkSoundID, 0, _vm->VAR(_vm->VAR_SYNC) + curSpeechPosInMs + 50, width, height);
	}
}

void IMuseDigital::getMusicLipSyncInfo(int syncId, int32 &width, int32 &height) {
	int soundId;
	int speechSoundId;
	int curSpeechPosInMs;

	soundId = 0;
	speechSoundId = 0;
	width = 0;
	height = 0;
	while (1) {
		soundId = diMUSEGetNextSound(soundId);
		if (!soundId)
			break;
		if (diMUSEGetParam(soundId, DIMUSE_P_SND_HAS_STREAM)) {
			if (diMUSEGetParam(soundId, DIMUSE_P_STREAM_BUFID) == DIMUSE_BUFFER_MUSIC) {
				speechSoundId = soundId;
				break;
			}
		}
	}

	if (speechSoundId) {
		curSpeechPosInMs = diMUSEGetParam(speechSoundId, DIMUSE_P_SND_POS_IN_MS);
		diMUSELipSync(speechSoundId, syncId, _vm->VAR(_vm->VAR_SYNC) + curSpeechPosInMs + 50, width, height);
	}
}

int32 IMuseDigital::getSoundElapsedTimeInMs(int soundId) {
	if (diMUSEGetParam(soundId, DIMUSE_P_SND_HAS_STREAM)) {
		return diMUSEGetParam(soundId, DIMUSE_P_SND_POS_IN_MS);
	}
	return 0;
}

void IMuseDigital::pause(bool p) {
	if (p) {
		debug(5, "IMuseDigital::pause(): pausing...");
		diMUSEPause();
	} else {
		debug(5, "IMuseDigital::pause(): resuming...");
		diMUSEResume();
	}
}

void IMuseDigital::setAudioNames(int32 num, char *names) {
	free(_audioNames);
	_numAudioNames = num;
	_audioNames = names;
}

int IMuseDigital::getSoundIdByName(const char *soundName) {
	if (soundName && soundName[0] != 0) {
		for (int r = 0; r < _numAudioNames; r++) {
			if (strcmp(soundName, &_audioNames[r * 9]) == 0) {
				return r;
			}
		}
	}

	return 0;
}

void IMuseDigital::setSmushPlayer(SmushPlayer *splayer) {
	_splayer = splayer;
	// Perform a first-time volume update for both SMUSH and iMUSE
	diMUSESetMusicGroupVol(CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 2, 0, 127));
	diMUSESetVoiceGroupVol(CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType) / 2, 0, 127));
	diMUSESetSFXGroupVol(CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 2, 0, 127));
}

void IMuseDigital::receiveAudioFromSMUSH(uint8 *srcBuf, int32 inFrameCount, int32 feedSize, int32 mixBufStartIndex, int volume, int pan, bool is11025Hz) {
	_internalMixer->mix(srcBuf, inFrameCount, 8, 1, feedSize, mixBufStartIndex, volume, pan, is11025Hz);
}

void IMuseDigital::parseScriptCmds(int cmd, int soundId, int sub_cmd, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n, int o, int p) {
	int b = soundId;
	int c = sub_cmd;
	int id;
	int volume = b;
	switch (cmd) {
	case 0x1000:
		// SetState
		diMUSESetState(soundId);
		break;
	case 0x1001:
		// SetSequence
		diMUSESetSequence(soundId);
		break;
	case 0x1002:
		// SetCuePoint
		diMUSESetCuePoint(soundId);
		break;
	case 0x1003:
		// SetAttribute
		diMUSESetAttribute(b, c);
		break;
	case 0x2000:
		// SetGroupSfxVolume
		if (!_vm->isUsingOriginalGUI()) {
			volume = CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 2, 0, 127);
		}

		diMUSESetSFXGroupVol(volume);
		break;
	case 0x2001:
		// SetGroupVoiceVolume
		if (!_vm->isUsingOriginalGUI()) {
			volume = CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType) / 2, 0, 127);
		}

		diMUSESetVoiceGroupVol(volume);
		break;
	case 0x2002:
		// SetGroupMusicVolume
		if (!_vm->isUsingOriginalGUI()) {
			volume = CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 2, 0, 127);
		}

		diMUSESetMusicGroupVol(volume);
		break;
	case 10: // StopAllSounds
	case 12: // SetParam
	case 14: // FadeParam
		cmdsHandleCmd(cmd, nullptr, soundId, sub_cmd, d, e, f, g, h, i, j, k, l, m, n, o);
		break;
	case 25: // OpenSound
		if (_vm->_game.id == GID_FT) {
			id = getSoundIdByName("kstand");
			_filesHandler->openSound(id);
		} else if (_vm->_game.id == GID_DIG && _vm->_game.features & GF_DEMO) {
			// Special opcode used in place of the first setState instruction
			_filesHandler->openSound(soundId);
			diMUSEStartStream(soundId, 126, DIMUSE_BUFFER_MUSIC);
		}

		break;
	case 26:
		// Special opcode used in place of successive setState instructions
		if (_vm->_game.id == GID_DIG && _vm->_game.features & GF_DEMO) {
			_filesHandler->openSound(c);
			diMUSESwitchStream(soundId, c, _ftCrossfadeBuffer, sizeof(_ftCrossfadeBuffer), 0);
			_filesHandler->closeSound(soundId);
		}
		break;
	default:
		debug("IMuseDigital::parseScriptCmds(): WARNING: unhandled command %d", cmd);
	}
}

int IMuseDigital::diMUSETerminate() {
	if (_scriptInitializedFlag) {
		diMUSEStopAllSounds();
		_filesHandler->closeAllSounds();
	}

	return 0;
}

int IMuseDigital::diMUSEInitialize() {
	return cmdsHandleCmd(0);
}

int IMuseDigital::diMUSEPause() {
	return cmdsHandleCmd(3);
}

int IMuseDigital::diMUSEResume() {
	return cmdsHandleCmd(4);
}

void IMuseDigital::diMUSESaveLoad(Common::Serializer &ser) {
	cmdsSaveLoad(ser);
}

int IMuseDigital::diMUSESetGroupVol(int groupId, int volume) {
	return cmdsHandleCmd(7, nullptr, groupId, volume);
}

int IMuseDigital::diMUSEStartSound(int soundId, int priority) {
	return cmdsHandleCmd(8, nullptr, soundId, priority);
}

int IMuseDigital::diMUSEStopSound(int soundId) {
	debug(5, "IMuseDigital::diMUSEStopSound(): %d", soundId);
	return cmdsHandleCmd(9, nullptr, soundId);
}

int IMuseDigital::diMUSEStopAllSounds() {
	debug(5, "IMuseDigital::diMUSEStopAllSounds()");
	return cmdsHandleCmd(10);
}

int IMuseDigital::diMUSEGetNextSound(int soundId) {
	return cmdsHandleCmd(11, nullptr, soundId);
}

int IMuseDigital::diMUSESetParam(int soundId, int paramId, int value) {
	return cmdsHandleCmd(12, nullptr, soundId, paramId, value);
}

int IMuseDigital::diMUSEGetParam(int soundId, int paramId) {
	return cmdsHandleCmd(13, nullptr, soundId, paramId);
}

int IMuseDigital::diMUSEFadeParam(int soundId, int opcode, int destValue, int fadeLength) {
	return cmdsHandleCmd(14, nullptr, soundId, opcode, destValue, fadeLength);
}

int IMuseDigital::diMUSESetHook(int soundId, int hookId) {
	return cmdsHandleCmd(15, nullptr, soundId, hookId);
}

int IMuseDigital::diMUSESetTrigger(int soundId, int marker, int opcode, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n) {
	return cmdsHandleCmd(17, nullptr, soundId, marker, opcode, d, e, f, g, h, i, j, k, l, m, n);
}

int IMuseDigital::diMUSEStartStream(int soundId, int priority, int bufferId) {
	return cmdsHandleCmd(25, nullptr, soundId, priority, bufferId);
}

int IMuseDigital::diMUSESwitchStream(int oldSoundId, int newSoundId, int fadeDelay, int fadeSyncFlag2, int fadeSyncFlag1) {
	return cmdsHandleCmd(26, nullptr, oldSoundId, newSoundId, fadeDelay, fadeSyncFlag2, fadeSyncFlag1);
}

// Variation for FT and DIG demo
int IMuseDigital::diMUSESwitchStream(int oldSoundId, int newSoundId, uint8 *crossfadeBuffer, int crossfadeBufferSize, int vocLoopFlag) {
	return cmdsHandleCmd(26, crossfadeBuffer, oldSoundId, newSoundId, -1, crossfadeBufferSize, vocLoopFlag);
}

int IMuseDigital::diMUSEProcessStreams() {
	return cmdsHandleCmd(27);
}

void IMuseDigital::diMUSEQueryStream(int soundId, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused) {
	waveQueryStream(soundId, bufSize, criticalSize, freeSpace, paused);
}

int IMuseDigital::diMUSEFeedStream(int soundId, uint8 *srcBuf, int32 sizeToFeed, int paused) {
	return cmdsHandleCmd(29, srcBuf, soundId, -1, sizeToFeed, paused);
}

int IMuseDigital::diMUSELipSync(int soundId, int syncId, int msPos, int32 &width, int32 &height) {
	return waveLipSync(soundId, syncId, msPos, width, height);
}

int IMuseDigital::diMUSEGetMusicGroupVol() {
	if (_vm->isUsingOriginalGUI()) {
		return diMUSESetGroupVol(DIMUSE_GROUP_MUSIC, -1);
	}

	return _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 2;
}

int IMuseDigital::diMUSEGetSFXGroupVol() {
	if (_vm->isUsingOriginalGUI()) {
		return diMUSESetGroupVol(DIMUSE_GROUP_SFX, -1);
	}

	return _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 2;
}

int IMuseDigital::diMUSEGetVoiceGroupVol() {
	if (_vm->isUsingOriginalGUI()) {
		return diMUSESetGroupVol(DIMUSE_GROUP_SPEECH, -1);
	}

	return _mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType) / 2;
}

int IMuseDigital::diMUSESetMusicGroupVol(int volume) {
	debug(5, "IMuseDigital::diMUSESetMusicGroupVol(): %d", volume);
	if (_isEarlyDiMUSE)
		_splayer->setGroupVolume(GRP_BKGMUS, volume);
	return diMUSESetGroupVol(DIMUSE_GROUP_MUSIC, volume);
}

int IMuseDigital::diMUSESetSFXGroupVol(int volume) {
	debug(5, "IMuseDigital::diMUSESetSFXGroupVol(): %d", volume);
	if (_isEarlyDiMUSE)
		_splayer->setGroupVolume(GRP_SFX, volume);
	return diMUSESetGroupVol(DIMUSE_GROUP_SFX, volume);
}

int IMuseDigital::diMUSESetVoiceGroupVol(int volume) {
	debug(5, "IMuseDigital::diMUSESetVoiceGroupVol(): %d", volume);
	if (_isEarlyDiMUSE)
		_splayer->setGroupVolume(GRP_SPEECH, volume);
	return diMUSESetGroupVol(DIMUSE_GROUP_SPEECH, volume);
}

void IMuseDigital::diMUSEUpdateGroupVolumes() {
	waveUpdateGroupVolumes();
}

int IMuseDigital::diMUSEInitializeScript() {
	return scriptParse(0, -1, -1);
}

void IMuseDigital::diMUSERefreshScript() {
	scriptParse(4, -1, -1);
}

int IMuseDigital::diMUSESetState(int soundId) {
	return scriptParse(5, soundId, -1);
}

int IMuseDigital::diMUSESetSequence(int soundId) {
	return scriptParse(6, soundId, -1);
}

int IMuseDigital::diMUSESetCuePoint(int cueId) {
	return scriptParse(7, cueId, -1);
}

int IMuseDigital::diMUSESetAttribute(int attrIndex, int attrVal) {
	return scriptParse(8, attrIndex, attrVal);
}

void IMuseDigital::diMUSEEnableSpooledMusic() {
	_spooledMusicEnabled = true;
}

void IMuseDigital::diMUSEDisableSpooledMusic() {
	_spooledMusicEnabled = true;
	diMUSESetState(0);
	diMUSESetSequence(0);
	_spooledMusicEnabled = false;
}

// Debugger utility functions

void IMuseDigital::listStates() {
	_vm->getDebugger()->debugPrintf("+---------------------------------+\n");
	_vm->getDebugger()->debugPrintf("| stateId |         name          |\n");
	_vm->getDebugger()->debugPrintf("+---------+-----------------------+\n");
	if (_vm->_game.id == GID_CMI) {
		if (_vm->_game.features & GF_DEMO) {
			for (int i = 0; _comiDemoStateMusicTable[i].soundId != -1; i++) {
				_vm->getDebugger()->debugPrintf("|  %4d   | %20s  |\n", _comiDemoStateMusicTable[i].soundId, _comiDemoStateMusicTable[i].name);
			}
		} else {
			for (int i = 0; _comiStateMusicTable[i].soundId != -1; i++) {
				_vm->getDebugger()->debugPrintf("|  %4d   | %20s  |\n", _comiStateMusicTable[i].soundId, _comiStateMusicTable[i].name);
			}
		}
	} else if (_vm->_game.id == GID_DIG) {
		for (int i = 0; _digStateMusicTable[i].soundId != -1; i++) {
			_vm->getDebugger()->debugPrintf("|  %4d   | %20s  |\n", _digStateMusicTable[i].soundId, _digStateMusicTable[i].name);
		}
	} else if (_vm->_game.id == GID_FT) {
		for (int i = 0; _ftStateMusicTable[i].name[0]; i++) {
			_vm->getDebugger()->debugPrintf("|  %4d   | %21s |\n", i, _ftStateMusicTable[i].name);
		}
	}
	_vm->getDebugger()->debugPrintf("+---------+-----------------------+\n\n");
}

void IMuseDigital::listSeqs() {
	_vm->getDebugger()->debugPrintf("+--------------------------------+\n");
	_vm->getDebugger()->debugPrintf("|  seqId  |         name         |\n");
	_vm->getDebugger()->debugPrintf("+---------+----------------------+\n");
	if (_vm->_game.id == GID_CMI) {
		for (int i = 0; _comiSeqMusicTable[i].soundId != -1; i++) {
			_vm->getDebugger()->debugPrintf("|  %4d   | %20s |\n", _comiSeqMusicTable[i].soundId, _comiSeqMusicTable[i].name);
		}
	} else if (_vm->_game.id == GID_DIG) {
		for (int i = 0; _digSeqMusicTable[i].soundId != -1; i++) {
			_vm->getDebugger()->debugPrintf("|  %4d   | %20s |\n", _digSeqMusicTable[i].soundId, _digSeqMusicTable[i].name);
		}
	} else if (_vm->_game.id == GID_FT) {
		for (int i = 0; _ftSeqNames[i].name[0]; i++) {
			_vm->getDebugger()->debugPrintf("|  %4d   | %20s |\n", i, _ftSeqNames[i].name);
		}
	}
	_vm->getDebugger()->debugPrintf("+---------+----------------------+\n\n");
}

void IMuseDigital::listCues() {
	int curId = -1;
	if (_curMusicSeq) {
		_vm->getDebugger()->debugPrintf("Available cues for current sequence:\n");
		_vm->getDebugger()->debugPrintf("+---------------------------------------+\n");
		_vm->getDebugger()->debugPrintf("|   cueName   | transitionType | volume |\n");
		_vm->getDebugger()->debugPrintf("+-------------+----------------+--------+\n");
		for (int i = 0; i < 4; i++) {
			curId = ((_curMusicSeq - 1) * 4) + i;
			_vm->getDebugger()->debugPrintf("|  %9s  |        %d       |  %3d   |\n",
				_ftSeqMusicTable[curId].audioName, (int)_ftSeqMusicTable[curId].transitionType, (int)_ftSeqMusicTable[curId].volume);
		}
		_vm->getDebugger()->debugPrintf("+-------------+----------------+--------+\n\n");
	} else {
		_vm->getDebugger()->debugPrintf("Current sequence is NULL, no cues available.\n\n");
	}
}

void IMuseDigital::listTracks() {
	_vm->getDebugger()->debugPrintf("Virtual audio tracks currently playing:\n");
	_vm->getDebugger()->debugPrintf("+-------------------------------------------------------------------------+\n");
	_vm->getDebugger()->debugPrintf("| # | soundId | group | hasStream | vol/effVol/pan  | priority | jumpHook |\n");
	_vm->getDebugger()->debugPrintf("+---+---------+-------+-----------+-----------------+----------+----------+\n");

	for (int i = 0; i < _trackCount; i++) {
		IMuseDigiTrack curTrack = _tracks[i];
		if (curTrack.soundId != 0) {
			_vm->getDebugger()->debugPrintf("| %1d |  %5d  |   %d   |     %d     |   %3d/%3d/%3d   |   %3d    |   %3d    |\n",
				i, curTrack.soundId, curTrack.group, diMUSEGetParam(curTrack.soundId, DIMUSE_P_SND_HAS_STREAM),
				curTrack.vol, curTrack.effVol, curTrack.pan, curTrack.priority, curTrack.jumpHook);
		} else {
			_vm->getDebugger()->debugPrintf("| %1d |   ---   |  ---  |    ---    |   ---/---/---   |   ---    |   ---    |\n", i);
		}
	}
	_vm->getDebugger()->debugPrintf("+---+---------+-------+-----------+-----------------+----------+----------+\n\n");
}

void IMuseDigital::listGroups() {
	_vm->getDebugger()->debugPrintf("Volume groups:\n");
	_vm->getDebugger()->debugPrintf("\tSFX:      %3d\n", _groupsHandler->getGroupVol(DIMUSE_GROUP_SFX));
	_vm->getDebugger()->debugPrintf("\tSPEECH:   %3d\n", _groupsHandler->getGroupVol(DIMUSE_GROUP_SPEECH));
	_vm->getDebugger()->debugPrintf("\tMUSIC:    %3d\n", _groupsHandler->getGroupVol(DIMUSE_GROUP_MUSIC));
	_vm->getDebugger()->debugPrintf("\tMUSICEFF: %3d\n\n", _groupsHandler->getGroupVol(DIMUSE_GROUP_MUSICEFF));
}

} // End of namespace Scumm
