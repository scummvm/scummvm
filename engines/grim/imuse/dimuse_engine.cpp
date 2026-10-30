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

#include "engines/grim/imuse/dimuse_engine.h"
#include "engines/grim/imuse/dimuse_defs.h"
#include "engines/grim/imuse/dimuse_tables.h"

#include "engines/grim/movie/codecs/vima.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Grim {

void IMuseDigital::timer_handler(void *refCon) {
	IMuseDigital *diMUSE = (IMuseDigital *)refCon;
	diMUSE->diMUSEHeartbeat();
}

IMuseDigital::IMuseDigital(int sampleRate, Audio::Mixer *mixer, Common::Mutex *mutex, bool lowLatencyMode)
	: _mixer(mixer), _mutex(mutex) {
	assert(mixer);

	// 50 Hz rate for the callback
	_callbackFps = DIMUSE_TIMER_BASE_RATE_HZ;
	_usecPerInt = DIMUSE_TIMER_BASE_RATE_USEC;

	_lowLatencyMode = lowLatencyMode;
	_internalSampleRate = sampleRate;
	_internalFeedSize = (int)(DIMUSE_BASE_FEEDSIZE * ((float)_internalSampleRate / DIMUSE_BASE_SAMPLERATE));

	if (_lowLatencyMode) {
		_internalFeedSize *= 2;
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

	_initialized = false;
	_fileListInitialized = false;
	_loadedSoundList = nullptr;
	_openedSoundsList = nullptr;
	_scriptInitializedFlag = false;
	_callbackInterruptFlag = false;
	_spooledMusicEnabled = true;

	_isEngineDisabled = false;
	_checkForUnderrun = false;
	_underrunCooldown = 0;

	_emptyMarker[0] = '\0';
	_internalMixer = new IMuseDigiInternalMixer(mixer, _internalSampleRate, _lowLatencyMode);
	_groupsHandler = new IMuseDigiGroupsHandler(this, mutex);
	_fadesHandler = new IMuseDigiFadesHandler(this, mutex);
	_triggersHandler = new IMuseDigiTriggersHandler(this, mutex);

	memset(_soundCache, 0, sizeof(_soundCache));
	_currentSoundSlot = 0;
	memset(_openAudioFiles, 0, sizeof(_openAudioFiles));
	memset(_soundBuffers, 0, sizeof(_soundBuffers));
	memset(_lipsyncSoundIds, 0, sizeof(_lipsyncSoundIds));
	memset(_lipsyncBufs, 0, sizeof(_lipsyncBufs));
	_curSpeechFile[0] = '\0';
	_millisecsInCurSpeechFile = 0;
	_globalVolume = 0;
	vimaInit(_imuseDestTable);

	diMUSEInitialize();
	soundsAllocateBuffer(DIMUSE_BUFFER_MUSIC, 440000, 44000, 352000);
	soundsAllocateBuffer(DIMUSE_BUFFER_SPEECH, 88000, 44000, 88000);
	diMUSEInitializeScript();

	if (_mixer->getOutputBufSize() != 0) {
		// Let's find the optimal value for the maximum number of streams which can stay in the queue at once;
		// (A number which is too low can lead to buffer underrun, while the higher the number is, the higher is the audio latency)
		_maxQueuedStreams = (int)ceil((_mixer->getOutputBufSize() / _waveOutPreferredFeedSize) / ((float)_mixer->getOutputRate() / _internalSampleRate));

		// This mixer's optimal output sample rate for this audio engine is one which is a multiple of 22050Hz;
		// if we're dealing with one which is a multiple of 48000Hz, compensate the number of queued streams...
		if (_mixer->getOutputRate() % _internalSampleRate) {
			_maxQueuedStreams++;
		}

		// The lower optimal bound is always 4, except if we're operating in low latency mode
		_maxQueuedStreams = MAX(_mixer->getOutputBufSize() <= 1024 ? 3 : 4, _maxQueuedStreams);
	} else {
		debug(5, "IMuseDigital::IMuseDigital(): WARNING: output audio buffer size not specified for this platform, defaulting _maxQueuedStreams to 4");
		_maxQueuedStreams = 4;
	}

	// This value has been calculated in a way that is a good compromise
	// between the absence of underruns and the lowest latency achievable.
	// Of course this is never perfect, given the vast number of platforms
	// we support, so the value might eventually be corrected by our custom
	// adaptive buffer underrun correction routine.
	_nominalBufferCount = _maxQueuedStreams;
	_underrunCooldown = _maxQueuedStreams;

	g_system->getTimerManager()->installTimerProc(timer_handler, 1000000 / _callbackFps, this, "IMuseDigital");
}

IMuseDigital::~IMuseDigital() {
	g_system->getTimerManager()->removeTimerProc(timer_handler);
	diMUSETerminateScript();
	diMUSETerminate();

	cacheReset();
	soundsDeallocateBuffer(DIMUSE_BUFFER_SPEECH);
	soundsDeallocateBuffer(DIMUSE_BUFFER_MUSIC);

	delete _internalMixer;
	delete _groupsHandler;
	delete _fadesHandler;
	delete _triggersHandler;

	// Deinit the WaveOut module
	free(_waveOutOutputBuffer);
	_waveOutOutputBuffer = nullptr;

	free(_waveOutLowLatencyOutputBuffer);
	_waveOutLowLatencyOutputBuffer = nullptr;
}

int IMuseDigital::roundRobinSetBufferCount() {
	int minStreams = MAX<int>(_nominalBufferCount - 5, 1);
	int maxStreams = _nominalBufferCount + 5;
	_maxQueuedStreams++;

	if (_maxQueuedStreams > maxStreams) {
		_maxQueuedStreams = minStreams;
	}

	return _maxQueuedStreams;
}

void IMuseDigital::adaptBufferCount() {
	_maxQueuedStreams++;
	_nominalBufferCount = _maxQueuedStreams;
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

void IMuseDigital::diMUSEHeartbeat() {
	int soundId, musicTargetVolume, musicEffVol, musicVol, tempVol, tempEffVol;

	if (_callbackInterruptFlag)
		return;

	_callbackInterruptFlag = true;

	waveOutCallback();

	// TODO
	// if (!_vm->isUsingOriginalGUI()) {
	// 	// Update volumes
	//
	// 	if (_curMixerMusicVolume != _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType)) {
	// 		_curMixerMusicVolume = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
	// 		diMUSESetMusicGroupVol(CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 2, 0, 127));
	// 	}
	//
	// 	if (_curMixerSpeechVolume != _mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType)) {
	// 		_curMixerSpeechVolume = _mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType);
	// 		diMUSESetVoiceGroupVol(CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType) / 2, 0, 127));
	// 	}
	//
	// 	if (_curMixerSFXVolume != _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType)) {
	// 		_curMixerSFXVolume = _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType);
	// 		diMUSESetSFXGroupVol(CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 2, 0, 127));
	// 	}
	// }

	if (!_cmdsPauseCount) {
		// Handle fades and triggers
		_cmdsRunning50HzCount += _usecPerInt;
		while (_cmdsRunning50HzCount >= DIMUSE_TIMER_FADES_RATE_USEC) {
			_cmdsRunning50HzCount -= DIMUSE_TIMER_FADES_RATE_USEC;
			_fadesHandler->loop();
			_triggersHandler->loop();
		}

		// Music gain reduction while speech is playing
		_cmdsRunning5HzCount += _usecPerInt;
		if (_cmdsRunning5HzCount >= DIMUSE_TIMER_GAIN_RED_RATE_USEC) {
			while (1) {
				_cmdsRunning5HzCount -= DIMUSE_TIMER_GAIN_RED_RATE_USEC;

				musicTargetVolume = _groupsHandler->setGroupVol(DIMUSE_GROUP_MUSIC, -1);
				for (soundId = diMUSEGetNextSound(0); soundId; soundId = diMUSEGetNextSound(soundId)) {
					if (diMUSEGetParam(soundId, DIMUSE_P_GROUP) == DIMUSE_GROUP_SPEECH) {
						musicTargetVolume = (77 * musicTargetVolume) >> 7;
						break;
					}
				}

				musicEffVol = _groupsHandler->setGroupVol(DIMUSE_GROUP_MUSICEFF, -1);
				musicVol = _groupsHandler->setGroupVol(DIMUSE_GROUP_MUSIC, -1);

				if (musicEffVol > musicTargetVolume) {
					tempVol = musicEffVol - 36;
					if (tempVol <= musicTargetVolume) {
						if (musicTargetVolume < musicVol)
							musicVol = musicTargetVolume;
					} else if (tempVol < musicVol) {
						musicVol = tempVol;
					}
					_groupsHandler->setGroupVol(DIMUSE_GROUP_MUSICEFF, musicVol);
				} else if (musicEffVol < musicTargetVolume) {
					tempEffVol = musicEffVol + 1;
					if (tempEffVol < musicTargetVolume) {
						if (tempEffVol > musicVol)
							musicVol = tempEffVol;
					} else if (musicTargetVolume > musicVol) {
						musicVol = musicTargetVolume;
					}
					_groupsHandler->setGroupVol(DIMUSE_GROUP_MUSICEFF, musicVol);
				}

				if (_cmdsRunning5HzCount < DIMUSE_TIMER_GAIN_RED_RATE_USEC)
					break;
			}
		}
	}

	_callbackInterruptFlag = false;
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

// This is used in order to avoid crash everything
// if a compressed audio resource file is found
void IMuseDigital::disableEngine() {
	_isEngineDisabled = true;
}

bool IMuseDigital::isEngineDisabled() {
	return _isEngineDisabled;
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

int32 IMuseDigital::getSoundElapsedTimeInMs(int soundId) {
	if (diMUSEGetParam(soundId, DIMUSE_P_SND_HAS_STREAM)) {
		return diMUSEGetParam(soundId, DIMUSE_P_SND_POS_IN_MS);
	}
	return 0;
}

void IMuseDigital::pause(bool p) {
	if (p) {
		debug(5, "IMuseDigital::pause(): pausing...");
		soundsPause();
	} else {
		debug(5, "IMuseDigital::pause(): resuming...");
		soundsResume();
	}
}

void IMuseDigital::floodMusicBuffer() {
	while (!isMusicStreamIdle()) {
		diMUSEProcessStreams();
	}
}

void IMuseDigital::fillStreamsWhileMusicCritical(int fillTimesAfter) {
	while (isMusicCritical()) {
		diMUSEProcessStreams();
	}

	for (int i = 0; i < fillTimesAfter; i++) {
		diMUSEProcessStreams();
	}
}

bool IMuseDigital::isMusicStreamIdle() {
	int32 bufSize, criticalSize, freeSpace;
	int paused;
	IMuseDigiSndBuffer *bufInfo = filesGetBufInfo(DIMUSE_BUFFER_MUSIC);

	if (!queryNextSoundFile(bufSize, criticalSize, freeSpace, paused))
		return true;
	return (paused > 0) || (bufSize - bufInfo->loadSize < freeSpace);
}

bool IMuseDigital::isMusicCritical() {
	int32 bufSize, criticalSize, freeSpace;
	int paused;

	if (!queryNextSoundFile(bufSize, criticalSize, freeSpace, paused))
		return false;
	return (paused == 0) && freeSpace <= criticalSize;
}

bool IMuseDigital::queryNextSoundFile(int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused) {
	int soundId;
	soundId = diMUSEGetNextSound(0);
	while (soundId) {
		if (diMUSEGetParam(soundId, DIMUSE_P_SND_HAS_STREAM) &&
			(diMUSEGetParam(soundId, DIMUSE_P_GROUP) == DIMUSE_GROUP_MUSIC || diMUSEGetParam(soundId, DIMUSE_P_GROUP) == DIMUSE_GROUP_MUSICEFF)) {
			diMUSEQueryStream(soundId, bufSize, criticalSize, freeSpace, paused);
			return true;
		}
		soundId = diMUSEGetNextSound(soundId);
	}
	return false;
}

int IMuseDigital::getSampleRate() {
	return _internalSampleRate;
}

int IMuseDigital::getFeedSize() {
	return _internalFeedSize;
}

int IMuseDigital::diMUSETerminate() {
	return tracksDeinit();
}

int IMuseDigital::diMUSEInitialize() {
	if (_initialized)
		return -1;

	_cmdsRunning50HzCount = 0;
	_cmdsRunning5HzCount = 0;

	if (_groupsHandler->init() || _fadesHandler->init() ||
		_triggersHandler->init() || tracksInit()) {
		return -1;
	}

	_callbackInterruptFlag = false;
	_cmdsPauseCount = 0;
	_initialized = true;

	return 49;
}

void IMuseDigital::diMUSEPause() {
	Common::StackLock lock(*_mutex);

	if (_cmdsPauseCount++ == 0)
		tracksPause();
}

void IMuseDigital::diMUSEResume() {
	Common::StackLock lock(*_mutex);

	if (--_cmdsPauseCount) {
		if (_cmdsPauseCount < 0)
			_cmdsPauseCount = 0;
	} else {
		tracksResume();
	}
}

int IMuseDigital::diMUSESaveLoad(Common::Serializer &ser) {
	Common::StackLock lock(*_mutex);

	if (ser.isLoading())
		diMUSEStopAllSounds();

	_fadesHandler->saveLoad(ser);
	_triggersHandler->saveLoad(ser);
	tracksSaveLoad(ser);
	return 0;
}

int IMuseDigital::diMUSESetGroupVol(int groupId, int volume) {
	return _groupsHandler->setGroupVol(groupId, volume);
}

int IMuseDigital::diMUSEStartSound(int soundId, int priority) {
	uint8 *src = filesFetchMap(soundId);

	if (src == nullptr) {
		debug(5, "IMuseDigital::diMUSEStartSound(): ERROR: map address for sound %d is NULL", soundId);
		return -1;
	}

	// Check for the "iMUS" header
	if (READ_BE_UINT32(src) == MKTAG('i', 'M', 'U', 'S')) {
		Common::StackLock lock(*_mutex);
		return tracksStartSound(soundId, priority, 0);
	}

	return -1;
}

int IMuseDigital::diMUSEStopSound(int soundId) {
	debug(5, "IMuseDigital::diMUSEStopSound(): %d", soundId);
	Common::StackLock lock(*_mutex);
	return tracksStopSound(soundId);
}

int IMuseDigital::diMUSEStopAllSounds() {
	debug(5, "IMuseDigital::diMUSEStopAllSounds()");
	Common::StackLock lock(*_mutex);
	return tracksStopAllSounds();
}

int IMuseDigital::diMUSEGetNextSound(int soundId) {
	return tracksGetNextSound(soundId);
}

int IMuseDigital::diMUSESetParam(int soundId, int paramId, int value) {
	Common::StackLock lock(*_mutex);
	return tracksSetParam(soundId, paramId, value);
}

int IMuseDigital::diMUSEGetParam(int soundId, int paramId) {
	int param;

	if (!paramId)
		return 2;

	if (paramId == DIMUSE_P_TRIGS_SNDS)
		return _triggersHandler->countPendingSounds(soundId);

	{
		Common::StackLock lock(*_mutex);
		param = tracksGetParam(soundId, paramId);
	}

	if (param < 0 && paramId == DIMUSE_P_SND_TRACK_NUM)
		return 0;

	return param;
}

int IMuseDigital::diMUSEFadeParam(int soundId, int opcode, int destValue, int fadeLength) {
	return _fadesHandler->fadeParam(soundId, opcode, destValue, fadeLength);
}

int IMuseDigital::diMUSESetHook(int soundId, int hookId) {
	return tracksSetHook(soundId, hookId);
}

int IMuseDigital::diMUSEGetHook(int soundId) {
	return tracksGetHook(soundId);
}

int IMuseDigital::diMUSESetTrigger(int soundId, const char *marker, int opcode, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n) {
	return _triggersHandler->setTrigger(soundId, (char *)marker, opcode, d, e, f, g, h, i, j, k, l, m, n);
}

int IMuseDigital::diMUSECheckTrigger(int soundId, const char *marker, int opcode) {
	return _triggersHandler->checkTrigger(soundId, (char *)marker, opcode);
}

int IMuseDigital::diMUSEClearTrigger(int soundId, const char *marker, int opcode) {
	return _triggersHandler->clearTrigger(soundId, (char *)marker, opcode);
}

int IMuseDigital::diMUSEDeferCommand(int count, int opcode, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n) {
	return _triggersHandler->deferCommand(count, opcode, c, d, e, f, g, h, i, j, k, l, m, n);
}

int IMuseDigital::diMUSEStartStream(int soundId, int priority, int bufferId) {
	if (!filesCheckRange(soundId))
		return -1;

	Common::StackLock lock(*_mutex);
	return tracksStartSound(soundId, priority, bufferId);
}

int IMuseDigital::diMUSESwitchStream(int oldSoundId, int newSoundId, int fadeDelay, int fadeSyncFlag2, int fadeSyncFlag1) {
	Common::StackLock lock(*_mutex);
	return dispatchSwitchStream(oldSoundId, newSoundId, fadeDelay, fadeSyncFlag2, fadeSyncFlag1);
}

int IMuseDigital::diMUSEProcessStreams() {
	Common::StackLock lock(*_mutex);
	return streamerProcessStreams();
}

int IMuseDigital::diMUSEQueryStream(int soundId, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused) {
	Common::StackLock lock(*_mutex);
	return tracksQueryStream(soundId, bufSize, criticalSize, freeSpace, paused);
}

int IMuseDigital::diMUSEFeedStream(int soundId, uint8 *srcBuf, int32 sizeToFeed, int paused) {
	Common::StackLock lock(*_mutex);
	return tracksFeedStream(soundId, srcBuf, sizeToFeed, paused);
}

int IMuseDigital::diMUSEGetMusicGroupVol() {
	// TODO
	// if (_vm->isUsingOriginalGUI()) {
	// 	return diMUSESetGroupVol(DIMUSE_GROUP_MUSIC, -1);
	// }
	//
	// return _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 2;
	return diMUSESetGroupVol(DIMUSE_GROUP_MUSIC, -1);
}

int IMuseDigital::diMUSEGetSFXGroupVol() {
	// TODO
	// if (_vm->isUsingOriginalGUI()) {
	// 	return diMUSESetGroupVol(DIMUSE_GROUP_SFX, -1);
	// }
	//
	// return _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 2;
	return diMUSESetGroupVol(DIMUSE_GROUP_SFX, -1);
}

int IMuseDigital::diMUSEGetVoiceGroupVol() {
	// TODO
	// if (_vm->isUsingOriginalGUI()) {
	// 	return diMUSESetGroupVol(DIMUSE_GROUP_SPEECH, -1);
	// }
	//
	// return _mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType) / 2;
	return diMUSESetGroupVol(DIMUSE_GROUP_SPEECH, -1);
}

int IMuseDigital::diMUSESetMusicGroupVol(int volume) {
	debug(5, "IMuseDigital::diMUSESetMusicGroupVol(): %d", volume);
	return diMUSESetGroupVol(DIMUSE_GROUP_MUSIC, volume);
}

int IMuseDigital::diMUSESetSFXGroupVol(int volume) {
	debug(5, "IMuseDigital::diMUSESetSFXGroupVol(): %d", volume);
	return diMUSESetGroupVol(DIMUSE_GROUP_SFX, volume);
}

int IMuseDigital::diMUSESetVoiceGroupVol(int volume) {
	debug(5, "IMuseDigital::diMUSESetVoiceGroupVol(): %d", volume);
	return diMUSESetGroupVol(DIMUSE_GROUP_SPEECH, volume);
}

int IMuseDigital::diMUSEInitializeScript() {
	return scriptParse(DIMUSE_C_SCRIPT_INIT, -1, -1);
}

int IMuseDigital::diMUSETerminateScript() {
	return scriptParse(DIMUSE_C_SCRIPT_TERMINATE, -1, -1);
}

void IMuseDigital::diMUSESaveLoadScript(Common::Serializer &ser) {
	scriptSaveLoad(ser);
}

void IMuseDigital::diMUSERefreshScript() {
	scriptParse(DIMUSE_C_SCRIPT_REFRESH, -1, -1);
}

int IMuseDigital::diMUSESetState(int soundId) {
	return scriptParse(DIMUSE_C_SCRIPT_SET_STATE, soundId, -1);
}

int IMuseDigital::diMUSESetSequence(int soundId) {
	return scriptParse(DIMUSE_C_SCRIPT_SET_SEQUENCE, soundId, -1);
}

int IMuseDigital::diMUSESetCuePoint(int cueId) {
	return scriptParse(DIMUSE_C_SCRIPT_CUE_POINT, cueId, -1);
}

int IMuseDigital::diMUSESetAttribute(int attrIndex, int attrVal) {
	return scriptParse(DIMUSE_C_SCRIPT_SET_ATTRIBUTE, attrIndex, attrVal);
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

// TODO
#if 0
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
#endif

} // End of namespace Grim
