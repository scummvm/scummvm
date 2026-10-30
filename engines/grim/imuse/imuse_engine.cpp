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

#include "engines/grim/imuse/imuse_engine.h"
#include "engines/grim/imuse/imuse_defs.h"
#include "engines/grim/imuse/imuse_tables.h"

#include "engines/grim/movie/codecs/vima.h"

#include "audio/effects/hmi/hmifxlib.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Grim {

IMuse *g_imuse = nullptr;

void IMuse::timer_handler(void *refCon) {
	IMuse *imuse = (IMuse *)refCon;
	imuse->imHeartbeat();
}

IMuse::IMuse(int sampleRate, Audio::Mixer *mixer, Common::Mutex *mutex, bool lowLatencyMode)
	: _mixer(mixer), _mutex(mutex) {
	assert(mixer);

	// 50 Hz rate for the callback
	_callbackFps = IMUSE_TIMER_BASE_RATE_HZ;
	_usecPerInt = IMUSE_TIMER_BASE_RATE_USEC;

	_lowLatencyMode = lowLatencyMode;
	_internalSampleRate = sampleRate;
	_internalFeedSize = (int)(IMUSE_BASE_FEEDSIZE * ((float)_internalSampleRate / IMUSE_BASE_SAMPLERATE));

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
	_internalMixer = new IMuseInternalMixer(mixer, _internalSampleRate, _lowLatencyMode);
	_groupsHandler = new IMuseGroupsHandler(this, mutex);
	_fadesHandler = new IMuseFadesHandler(this, mutex);
	_triggersHandler = new IMuseTriggersHandler(this, mutex);

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

	_hmiFxLib = new Audio::HMIFxLib();
	memset(&_hmiInitData, 0, sizeof(_hmiInitData));
	memset(&_hmiRequest, 0, sizeof(_hmiRequest));
	memset(&_hmiInputFmt, 0, sizeof(_hmiInputFmt));
	memset(&_hmiOutputFmt, 0, sizeof(_hmiOutputFmt));
	_presetCollection = nullptr;
	_loadedPreset = nullptr;
	_hmiFxInitialized = false;
	_curHMIEffectIdInternal = 0;
	_curEffectId = 0;
	_curEffectName[0] = '\0';
	_voiceSrcBuf = nullptr;
	_maxChunkRequested = 0;
	_voiceRemainderBuf = nullptr;
	_voiceRemainderSize = 0;
	_voiceRemainderCapacity = 0;
	_voiceRemainderPos = 0;
	setupHMI(IMUSE_HMI_HEAP_SIZE);

	imInitialize();
	soundsAllocateBuffer(IMUSE_BUFFER_MUSIC, 440000, 44000, 352000);
	soundsAllocateBuffer(IMUSE_BUFFER_SPEECH, 88000, 44000, 88000);
	imInitializeScript();

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
		debug(5, "IMuse::IMuse(): WARNING: output audio buffer size not specified for this platform, defaulting _maxQueuedStreams to 4");
		_maxQueuedStreams = 4;
	}

	// This value has been calculated in a way that is a good compromise
	// between the absence of underruns and the lowest latency achievable.
	// Of course this is never perfect, given the vast number of platforms
	// we support, so the value might eventually be corrected by our custom
	// adaptive buffer underrun correction routine.
	_nominalBufferCount = _maxQueuedStreams;
	_underrunCooldown = _maxQueuedStreams;

	g_system->getTimerManager()->installTimerProc(timer_handler, 1000000 / _callbackFps, this, "IMuse");
}

IMuse::~IMuse() {
	g_system->getTimerManager()->removeTimerProc(timer_handler);
	imTerminateScript();

	soundsTerminateProcessing();
	free(_voiceSrcBuf);
	_voiceSrcBuf = nullptr;
	_maxChunkRequested = 0;
	free(_voiceRemainderBuf);
	_voiceRemainderBuf = nullptr;
	delete _hmiFxLib;

	imTerminate();

	cacheReset();
	soundsDeallocateBuffer(IMUSE_BUFFER_SPEECH);
	soundsDeallocateBuffer(IMUSE_BUFFER_MUSIC);

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

int IMuse::roundRobinSetBufferCount() {
	int minStreams = MAX<int>(_nominalBufferCount - 5, 1);
	int maxStreams = _nominalBufferCount + 5;
	_maxQueuedStreams++;

	if (_maxQueuedStreams > maxStreams) {
		_maxQueuedStreams = minStreams;
	}

	return _maxQueuedStreams;
}

void IMuse::adaptBufferCount() {
	_maxQueuedStreams++;
	_nominalBufferCount = _maxQueuedStreams;
}

void IMuse::stopSound(int sound) {
	imStopSound(sound);
}

void IMuse::stopAllSounds() {
	imStopAllSounds();
}

int IMuse::isSoundRunning(int soundId) {
	return imGetParam(soundId, IMUSE_P_SND_TRACK_NUM) > 0;
}

void IMuse::imHeartbeat() {
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
	// 		setMusicGroupVol(CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 2, 0, 127));
	// 	}
	//
	// 	if (_curMixerSpeechVolume != _mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType)) {
	// 		_curMixerSpeechVolume = _mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType);
	// 		setVoiceGroupVol(CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType) / 2, 0, 127));
	// 	}
	//
	// 	if (_curMixerSFXVolume != _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType)) {
	// 		_curMixerSFXVolume = _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType);
	// 		setSFXGroupVol(CLIP(_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 2, 0, 127));
	// 	}
	// }

	if (!_cmdsPauseCount) {
		// Handle fades and triggers
		_cmdsRunning50HzCount += _usecPerInt;
		while (_cmdsRunning50HzCount >= IMUSE_TIMER_FADES_RATE_USEC) {
			_cmdsRunning50HzCount -= IMUSE_TIMER_FADES_RATE_USEC;
			_fadesHandler->loop();
			_triggersHandler->loop();
		}

		// Music gain reduction while speech is playing
		_cmdsRunning5HzCount += _usecPerInt;
		if (_cmdsRunning5HzCount >= IMUSE_TIMER_GAIN_RED_RATE_USEC) {
			while (1) {
				_cmdsRunning5HzCount -= IMUSE_TIMER_GAIN_RED_RATE_USEC;

				musicTargetVolume = _groupsHandler->setGroupVol(IMUSE_GROUP_MUSIC, -1);
				for (soundId = imGetNextSound(0); soundId; soundId = imGetNextSound(soundId)) {
					if (imGetParam(soundId, IMUSE_P_GROUP) == IMUSE_GROUP_SPEECH) {
						musicTargetVolume = (77 * musicTargetVolume) >> 7;
						break;
					}
				}

				musicEffVol = _groupsHandler->setGroupVol(IMUSE_GROUP_MUSICEFF, -1);
				musicVol = _groupsHandler->setGroupVol(IMUSE_GROUP_MUSIC, -1);

				if (musicEffVol > musicTargetVolume) {
					tempVol = musicEffVol - 36;
					if (tempVol <= musicTargetVolume) {
						if (musicTargetVolume < musicVol)
							musicVol = musicTargetVolume;
					} else if (tempVol < musicVol) {
						musicVol = tempVol;
					}
					_groupsHandler->setGroupVol(IMUSE_GROUP_MUSICEFF, musicVol);
				} else if (musicEffVol < musicTargetVolume) {
					tempEffVol = musicEffVol + 1;
					if (tempEffVol < musicTargetVolume) {
						if (tempEffVol > musicVol)
							musicVol = tempEffVol;
					} else if (musicTargetVolume > musicVol) {
						musicVol = musicTargetVolume;
					}
					_groupsHandler->setGroupVol(IMUSE_GROUP_MUSICEFF, musicVol);
				}

				if (_cmdsRunning5HzCount < IMUSE_TIMER_GAIN_RED_RATE_USEC)
					break;
			}
		}
	}

	_callbackInterruptFlag = false;
}

void IMuse::setPriority(int soundId, int priority) {
	imSetParam(soundId, IMUSE_P_PRIORITY, priority);
}

void IMuse::setVolume(int soundId, int volume) {
	imSetParam(soundId, IMUSE_P_VOLUME, volume);
	if (soundId == kTalkSoundID)
		_currentSpeechVolume = volume;
}

void IMuse::setPan(int soundId, int pan) {
	imSetParam(soundId, IMUSE_P_PAN, pan);
	if (soundId == kTalkSoundID)
		_currentSpeechPan = pan;
}

void IMuse::setFrequency(int soundId, int frequency) {
	imSetParam(soundId, IMUSE_P_TRANSPOSE, frequency);
	if (soundId == kTalkSoundID)
		_currentSpeechFrequency = frequency;
}

int IMuse::getCurSpeechVolume() const {
	return _currentSpeechVolume;
}

int IMuse::getCurSpeechPan() const {
	return _currentSpeechPan;
}

int IMuse::getCurSpeechFrequency() const {
	return _currentSpeechFrequency;
}

// This is used in order to avoid crash everything
// if a compressed audio resource file is found
void IMuse::disableEngine() {
	_isEngineDisabled = true;
}

bool IMuse::isEngineDisabled() {
	return _isEngineDisabled;
}

int32 IMuse::getCurMusicPosInMs() {
	int soundId, curSoundId;

	curSoundId = 0;
	soundId = 0;
	while (1) {
		curSoundId = imGetNextSound(curSoundId);
		if (!curSoundId)
			break;

		if (imGetParam(curSoundId, IMUSE_P_SND_HAS_STREAM) && imGetParam(curSoundId, IMUSE_P_STREAM_BUFID) == IMUSE_BUFFER_MUSIC) {
			soundId = curSoundId;
			return imGetParam(soundId, IMUSE_P_SND_POS_IN_MS);
		}
	}

	return imGetParam(soundId, IMUSE_P_SND_POS_IN_MS);
}

int32 IMuse::getSoundElapsedTimeInMs(int soundId) {
	if (imGetParam(soundId, IMUSE_P_SND_HAS_STREAM)) {
		return imGetParam(soundId, IMUSE_P_SND_POS_IN_MS);
	}
	return 0;
}

void IMuse::pause(bool p) {
	if (p) {
		debug(5, "IMuse::pause(): pausing...");
		imPause();
	} else {
		debug(5, "IMuse::pause(): resuming...");
		imResume();
	}
}

void IMuse::soundsFloodMusicBuffer() {
	while (!soundsIsMusicStreamIdle()) {
		imProcessStreams();
	}
}

void IMuse::soundsFillStreamsWhileMusicCritical(int fillTimesAfter) {
	while (soundsIsMusicCritical()) {
		imProcessStreams();
	}

	for (int i = 0; i < fillTimesAfter; i++) {
		imProcessStreams();
	}
}

bool IMuse::soundsIsMusicStreamIdle() {
	int32 bufSize, criticalSize, freeSpace;
	int paused;
	IMuseSndBuffer *bufInfo = filesGetBufInfo(IMUSE_BUFFER_MUSIC);

	if (!queryMusicStream(bufSize, criticalSize, freeSpace, paused))
		return true;
	return (paused > 0) || (bufSize - bufInfo->loadSize < freeSpace);
}

bool IMuse::soundsIsMusicCritical() {
	int32 bufSize, criticalSize, freeSpace;
	int paused;

	if (!queryMusicStream(bufSize, criticalSize, freeSpace, paused))
		return false;
	return (paused == 0) && freeSpace <= criticalSize;
}

bool IMuse::queryMusicStream(int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused) {
	int soundId;
	soundId = imGetNextSound(0);
	while (soundId) {
		if (imGetParam(soundId, IMUSE_P_SND_HAS_STREAM) &&
			(imGetParam(soundId, IMUSE_P_GROUP) == IMUSE_GROUP_MUSIC || imGetParam(soundId, IMUSE_P_GROUP) == IMUSE_GROUP_MUSICEFF)) {
			imQueryStream(soundId, bufSize, criticalSize, freeSpace, paused);
			return true;
		}
		soundId = imGetNextSound(soundId);
	}
	return false;
}

int IMuse::getSampleRate() {
	return _internalSampleRate;
}

int IMuse::getFeedSize() {
	return _internalFeedSize;
}

int IMuse::imTerminate() {
	return tracksDeinit();
}

int IMuse::imInitialize() {
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

void IMuse::imPause() {
	Common::StackLock lock(*_mutex);

	if (_cmdsPauseCount++ == 0)
		tracksPause();
}

void IMuse::imResume() {
	Common::StackLock lock(*_mutex);

	if (--_cmdsPauseCount) {
		if (_cmdsPauseCount < 0)
			_cmdsPauseCount = 0;
	} else {
		tracksResume();
	}
}

int IMuse::imSaveLoad(Common::Serializer &ser) {
	Common::StackLock lock(*_mutex);

	if (ser.isLoading())
		imStopAllSounds();

	_fadesHandler->saveLoad(ser);
	_triggersHandler->saveLoad(ser);
	tracksSaveLoad(ser);
	return 0;
}

int IMuse::imSetGroupVol(int groupId, int volume) {
	return _groupsHandler->setGroupVol(groupId, volume);
}

int IMuse::imStartSound(int soundId, int priority) {
	uint8 *src = filesFetchMap(soundId);

	if (src == nullptr) {
		debug(5, "IMuse::imStartSound(): ERROR: map address for sound %d is NULL", soundId);
		return -1;
	}

	// Check for the "iMUS" header
	if (READ_BE_UINT32(src) == MKTAG('i', 'M', 'U', 'S')) {
		Common::StackLock lock(*_mutex);
		return tracksStartSound(soundId, priority, 0);
	}

	return -1;
}

int IMuse::imStopSound(int soundId) {
	debug(5, "IMuse::imStopSound(): %d", soundId);
	Common::StackLock lock(*_mutex);
	return tracksStopSound(soundId);
}

int IMuse::imStopAllSounds() {
	debug(5, "IMuse::imStopAllSounds()");
	Common::StackLock lock(*_mutex);
	return tracksStopAllSounds();
}

int IMuse::imGetNextSound(int soundId) {
	return tracksGetNextSound(soundId);
}

int IMuse::imSetParam(int soundId, int paramId, int value) {
	Common::StackLock lock(*_mutex);
	return tracksSetParam(soundId, paramId, value);
}

int IMuse::imGetParam(int soundId, int paramId) {
	int param;

	if (!paramId)
		return 2;

	if (paramId == IMUSE_P_TRIGS_SNDS)
		return _triggersHandler->countPendingSounds(soundId);

	{
		Common::StackLock lock(*_mutex);
		param = tracksGetParam(soundId, paramId);
	}

	if (param < 0 && paramId == IMUSE_P_SND_TRACK_NUM)
		return 0;

	return param;
}

int IMuse::imFadeParam(int soundId, int opcode, int destValue, int fadeLength) {
	return _fadesHandler->fadeParam(soundId, opcode, destValue, fadeLength);
}

int IMuse::imSetHook(int soundId, int hookId) {
	return tracksSetHook(soundId, hookId);
}

int IMuse::imGetHook(int soundId) {
	return tracksGetHook(soundId);
}

int IMuse::imSetTrigger(int soundId, const char *marker, int opcode, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n) {
	return _triggersHandler->setTrigger(soundId, (char *)marker, opcode, d, e, f, g, h, i, j, k, l, m, n);
}

int IMuse::imCheckTrigger(int soundId, const char *marker, int opcode) {
	return _triggersHandler->checkTrigger(soundId, (char *)marker, opcode);
}

int IMuse::imClearTrigger(int soundId, const char *marker, int opcode) {
	return _triggersHandler->clearTrigger(soundId, (char *)marker, opcode);
}

int IMuse::imDeferCommand(int count, int opcode, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n) {
	return _triggersHandler->deferCommand(count, opcode, c, d, e, f, g, h, i, j, k, l, m, n);
}

int IMuse::imStartStream(int soundId, int priority, int bufferId) {
	if (!filesCheckRange(soundId))
		return -1;

	Common::StackLock lock(*_mutex);
	return tracksStartSound(soundId, priority, bufferId);
}

int IMuse::imSwitchStream(int oldSoundId, int newSoundId, int fadeDelay, int fadeSyncFlag2, int fadeSyncFlag1) {
	Common::StackLock lock(*_mutex);
	return dispatchSwitchStream(oldSoundId, newSoundId, fadeDelay, fadeSyncFlag2, fadeSyncFlag1);
}

int IMuse::imProcessStreams() {
	Common::StackLock lock(*_mutex);
	return streamerProcessStreams();
}

int IMuse::imQueryStream(int soundId, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused) {
	Common::StackLock lock(*_mutex);
	return tracksQueryStream(soundId, bufSize, criticalSize, freeSpace, paused);
}

int IMuse::imFeedStream(int soundId, uint8 *srcBuf, int32 sizeToFeed, int paused) {
	Common::StackLock lock(*_mutex);
	return tracksFeedStream(soundId, srcBuf, sizeToFeed, paused);
}

int IMuse::getMusicGroupVol() {
	// TODO
	// if (_vm->isUsingOriginalGUI()) {
	// 	return imSetGroupVol(IMUSE_GROUP_MUSIC, -1);
	// }
	//
	// return _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 2;
	return imSetGroupVol(IMUSE_GROUP_MUSIC, -1);
}

int IMuse::getSFXGroupVol() {
	// TODO
	// if (_vm->isUsingOriginalGUI()) {
	// 	return imSetGroupVol(IMUSE_GROUP_SFX, -1);
	// }
	//
	// return _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 2;
	return imSetGroupVol(IMUSE_GROUP_SFX, -1);
}

int IMuse::getVoiceGroupVol() {
	// TODO
	// if (_vm->isUsingOriginalGUI()) {
	// 	return imSetGroupVol(IMUSE_GROUP_SPEECH, -1);
	// }
	//
	// return _mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType) / 2;
	return imSetGroupVol(IMUSE_GROUP_SPEECH, -1);
}

int IMuse::setMusicGroupVol(int volume) {
	debug(5, "IMuse::setMusicGroupVol(): %d", volume);
	return imSetGroupVol(IMUSE_GROUP_MUSIC, volume);
}

int IMuse::setSFXGroupVol(int volume) {
	debug(5, "IMuse::setSFXGroupVol(): %d", volume);
	return imSetGroupVol(IMUSE_GROUP_SFX, volume);
}

int IMuse::setVoiceGroupVol(int volume) {
	debug(5, "IMuse::setVoiceGroupVol(): %d", volume);
	return imSetGroupVol(IMUSE_GROUP_SPEECH, volume);
}

int IMuse::imInitializeScript() {
	return scriptParse(IMUSE_C_SCRIPT_INIT, -1, -1);
}

int IMuse::imTerminateScript() {
	return scriptParse(IMUSE_C_SCRIPT_TERMINATE, -1, -1);
}

void IMuse::imSaveLoadScript(Common::Serializer &ser) {
	scriptSaveLoad(ser);
}

void IMuse::imRefreshScript() {
	scriptParse(IMUSE_C_SCRIPT_REFRESH, -1, -1);
}

int IMuse::imSetState(int soundId) {
	return scriptParse(IMUSE_C_SCRIPT_SET_STATE, soundId, -1);
}

int IMuse::imSetSequence(int soundId) {
	return scriptParse(IMUSE_C_SCRIPT_SET_SEQUENCE, soundId, -1);
}

int IMuse::imSetCuePoint(int cueId) {
	return scriptParse(IMUSE_C_SCRIPT_CUE_POINT, cueId, -1);
}

int IMuse::imSetAttribute(int attrIndex, int attrVal) {
	return scriptParse(IMUSE_C_SCRIPT_SET_ATTRIBUTE, attrIndex, attrVal);
}

void IMuse::enableSpooledMusic() {
	_spooledMusicEnabled = true;
}

void IMuse::disableSpooledMusic() {
	_spooledMusicEnabled = true;
	imSetState(0);
	imSetSequence(0);
	_spooledMusicEnabled = false;
}

// Debugger utility functions

// TODO
#if 0
void IMuse::listStates() {
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

void IMuse::listSeqs() {
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

void IMuse::listCues() {
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

void IMuse::listTracks() {
	_vm->getDebugger()->debugPrintf("Virtual audio tracks currently playing:\n");
	_vm->getDebugger()->debugPrintf("+-------------------------------------------------------------------------+\n");
	_vm->getDebugger()->debugPrintf("| # | soundId | group | hasStream | vol/effVol/pan  | priority | jumpHook |\n");
	_vm->getDebugger()->debugPrintf("+---+---------+-------+-----------+-----------------+----------+----------+\n");

	for (int i = 0; i < _trackCount; i++) {
		IMuseTrack curTrack = _tracks[i];
		if (curTrack.soundId != 0) {
			_vm->getDebugger()->debugPrintf("| %1d |  %5d  |   %d   |     %d     |   %3d/%3d/%3d   |   %3d    |   %3d    |\n",
				i, curTrack.soundId, curTrack.group, imGetParam(curTrack.soundId, IMUSE_P_SND_HAS_STREAM),
				curTrack.vol, curTrack.effVol, curTrack.pan, curTrack.priority, curTrack.jumpHook);
		} else {
			_vm->getDebugger()->debugPrintf("| %1d |   ---   |  ---  |    ---    |   ---/---/---   |   ---    |   ---    |\n", i);
		}
	}
	_vm->getDebugger()->debugPrintf("+---+---------+-------+-----------+-----------------+----------+----------+\n\n");
}

void IMuse::listGroups() {
	_vm->getDebugger()->debugPrintf("Volume groups:\n");
	_vm->getDebugger()->debugPrintf("\tSFX:      %3d\n", _groupsHandler->getGroupVol(IMUSE_GROUP_SFX));
	_vm->getDebugger()->debugPrintf("\tSPEECH:   %3d\n", _groupsHandler->getGroupVol(IMUSE_GROUP_SPEECH));
	_vm->getDebugger()->debugPrintf("\tMUSIC:    %3d\n", _groupsHandler->getGroupVol(IMUSE_GROUP_MUSIC));
	_vm->getDebugger()->debugPrintf("\tMUSICEFF: %3d\n\n", _groupsHandler->getGroupVol(IMUSE_GROUP_MUSICEFF));
}
#endif

} // End of namespace Grim
