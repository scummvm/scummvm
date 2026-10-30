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

#ifndef GRIM_IMUSE_ENGINE_H
#define GRIM_IMUSE_ENGINE_H

#include "common/scummsys.h"
#include "common/debug.h"
#include "common/mutex.h"
#include "common/serializer.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "engines/grim/imuse/dimuse_defs.h"
#include "engines/grim/imuse/dimuse_internalmixer.h"
#include "engines/grim/imuse/dimuse_groups.h"
#include "engines/grim/imuse/dimuse_fades.h"
#include "engines/grim/imuse/dimuse_triggers.h"
#include "engines/grim/imuse/dimuse_tables.h"

#include "audio/mixer.h"
#include "audio/decoders/raw.h"

namespace Audio {
class AudioStream;
class Mixer;
class QueuingAudioStream;
}

namespace Grim {

struct IMuseDigiStateTable;
struct IMuseDigiDispatch;
struct IMuseDigiTrack;
struct IMuseDigiStreamZone;

class IMuseDigital {
private:
	Common::Mutex *_mutex;
	Audio::Mixer *_mixer;

	IMuseDigiInternalMixer *_internalMixer;
	IMuseDigiGroupsHandler *_groupsHandler;
	IMuseDigiFadesHandler *_fadesHandler;
	IMuseDigiTriggersHandler *_triggersHandler;

	int _callbackFps;
	static void timer_handler(void *refConf);

	bool _isEngineDisabled;
	bool _checkForUnderrun;
	int _underrunCooldown;
	bool _lowLatencyMode;

	int _internalFeedSize;
	int _internalSampleRate;

	// These three are manipulated in the waveOut functions
	uint8 *_outputAudioBuffer;
	int _outputFeedSize;
	int _outputSampleRate;

	// Used in low latency mode only
	uint8 *_outputLowLatencyAudioBuffers[DIMUSE_MAX_TRACKS];

	int _maxQueuedStreams; // maximum number of streams which can be queued before they are played
	int _nominalBufferCount;

	int _currentSpeechVolume, _currentSpeechFrequency, _currentSpeechPan;
	int _curMixerMusicVolume, _curMixerSpeechVolume, _curMixerSFXVolume;

	int32 _attributes[228];	// internal attributes for each music file to store and check later
	int32 _nextSeqToPlay;
	int32 _curMusicState;
	int32 _curMusicSeq;

	bool _scriptInitializedFlag;
	char _emptyMarker[1];
	bool _spooledMusicEnabled;

	int _usecPerInt; // Microseconds between each callback (will be set to 50 Hz)
	bool _callbackInterruptFlag;
	void diMUSEHeartbeat();

	bool isMusicStreamIdle();
	bool isMusicCritical();

	// Script
	int scriptParse(int cmd, int a, int b);
	int scriptInit();
	int scriptTerminate();
	void scriptSaveLoad(Common::Serializer &ser);
	void scriptDeinit();
	void scriptRefresh();
	void scriptSetState(int soundId);
	void scriptSetSequence(int soundId);
	void scriptPlayMusic(const IMuseDigiStateTable *tableEntry, int curSongEntryId, int isSequence);
	void scriptSetCuePoint(int cueId);
	int scriptSetAttribute(int attrIndex, int attrVal);

	// CMDs
	bool _initialized;
	int _cmdsPauseCount;
	int _cmdsRunning50HzCount;
	int _cmdsRunning5HzCount;

	// Streamer
	IMuseDigiStream _streams[DIMUSE_MAX_STREAMS];
	IMuseDigiStream *_lastStreamLoaded;
	int _streamerBailFlag;

	int streamerInit();
	IMuseDigiStream *streamerAllocateSound(int soundId, int bufId, int32 maxRead);
	int streamerClearSoundInStream(IMuseDigiStream *streamPtr);
	int streamerProcessStreams();
	uint8 *streamerGetStreamBuffer(IMuseDigiStream *streamPtr, int size);
	uint8 *streamerGetStreamBufferAtOffset(IMuseDigiStream *streamPtr, int32 offset, int size);
	int streamerSetReadIndex(IMuseDigiStream *streamPtr, int offset);
	int streamerSetLoadIndex(IMuseDigiStream *streamPtr, int offset);
	int streamerGetFreeBufferAmount(IMuseDigiStream *streamPtr);
	int streamerSetSoundToStreamFromOffset(IMuseDigiStream *streamPtr, int soundId, int32 offset);
	void streamerQueryStream(IMuseDigiStream *streamPtr, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused);
	int streamerFeedStream(IMuseDigiStream *streamPtr, uint8 *srcBuf, int32 sizeToFeed, int paused);
	int streamerFetchData(IMuseDigiStream *streamPtr);

	// Tracks
	IMuseDigiTrack _tracks[DIMUSE_MAX_TRACKS];
	IMuseDigiTrack *_trackList;

	int _trackCount;
	int _tracksPauseTimer;
	int _tracksMicroSecsToFeed;

	int tracksInit();
	void tracksPause();
	void tracksResume();
	void tracksSaveLoad(Common::Serializer &ser);
	void tracksCallback();
	void tracksLowLatencyCallback();
	int tracksStartSound(int soundId, int tryPriority, int group);
	int tracksStopSound(int soundId);
	int tracksStopAllSounds();
	int tracksGetNextSound(int soundId);
	int tracksQueryStream(int soundId, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused);
	int tracksFeedStream(int soundId, uint8 *srcBuf, int32 sizeToFeed, int paused);
	void tracksClear(IMuseDigiTrack *trackPtr);
	int tracksSetParam(int soundId, int opcode, int value);
	int tracksGetParam(int soundId, int opcode);
	int tracksSetHook(int soundId, int hookId);
	int tracksGetHook(int soundId);
	IMuseDigiTrack *tracksReserveTrack(int priority);
	int tracksDeinit();

	// Dispatch
	IMuseDigiDispatch _dispatches[DIMUSE_MAX_DISPATCHES];
	IMuseDigiStreamZone _streamZones[DIMUSE_MAX_STREAMZONES];
	uint8 *_dispatchBuffer;
	int32 _dispatchSize;
	uint8 *_dispatchSmallFadeBufs;
	uint8 *_dispatchLargeFadeBufs;
	int32 _dispatchFadeSize;
	int _dispatchLargeFadeFlags[DIMUSE_LARGE_FADES];
	int _dispatchSmallFadeFlags[DIMUSE_SMALL_FADES];
	int _dispatchFadeStartedFlag;
	int _dispatchBufferedHookId;
	int32 _dispatchJumpFadeSize;
	int32 _dispatchCurStreamBufSize;
	int32 _dispatchCurStreamCriticalSize;
	int32 _dispatchCurStreamFreeSpace;
	int _dispatchCurStreamPaused;

	int dispatchInit();
	int dispatchTerminate();
	IMuseDigiDispatch *dispatchGetDispatchByTrackId(int trackId);
	void dispatchSaveLoad(Common::Serializer &ser);
	int dispatchRestoreStreamZones();
	int dispatchAllocateSound(IMuseDigiTrack *trackPtr, int groupId);
	int dispatchRelease(IMuseDigiTrack *trackPtr);
	int dispatchSwitchStream(int oldSoundId, int newSoundId, int fadeLength, int exitTriggerSyncFlag, int offsetFadeSyncFlag);
	void dispatchProcessDispatches(IMuseDigiTrack *trackPtr, int feedSize, int sampleRate);
	void dispatchPredictFirstStream();
	int dispatchNavigateMap(IMuseDigiDispatch *dispatchPtr);
	int dispatchGetMap(IMuseDigiDispatch *dispatchPtr);
	int dispatchConvertMap(uint8 *rawMap, int32 *destMap);
	uint8 *dispatchGetNextMapEvent(int32 *mapPtr, int32 soundOffset, uint8 *mapEvent);
	void dispatchPredictStream(IMuseDigiDispatch *dispatchPtr);
	uint8 *dispatchCheckForJump(int32 *mapPtr, IMuseDigiStreamZone *strZnPtr, int &candidateHookId);
	void dispatchPrepareToJump(IMuseDigiDispatch *dispatchPtr, IMuseDigiStreamZone *strZnPtr, uint8 *jumpParamsFromMap, int calledFromGetNextMapEvent);
	void dispatchStreamNextZone(IMuseDigiDispatch *dispatchPtr, IMuseDigiStreamZone *strZnPtr);
	IMuseDigiStreamZone *dispatchAllocateStreamZone();
	uint8 *dispatchAllocateFade(int32 &fadeSize, const char *functionName);
	void dispatchDeallocateFade(IMuseDigiDispatch *dispatchPtr, const char *functionName);
	int dispatchGetFadeSize(IMuseDigiDispatch *dispatchPtr, int fadeLength);
	void dispatchValidateFadeSize(IMuseDigiDispatch *dispatchPtr, int32 &dispatchSize);
	int dispatchUpdateFadeMixVolume(IMuseDigiDispatch *dispatchPtr, int32 remainingFade);
	int dispatchUpdateFadeSlope(IMuseDigiDispatch *dispatchPtr);

	// Files
	IMuseDigiSoundList _mainSoundList[DIMUSE_MAX_SOUND_LIST];
	IMuseDigiSoundList *_loadedSoundList;
	IMuseDigiSoundList *_openedSoundsList;
	bool _fileListInitialized;
	uint16 _imuseDestTable[5786];
	IMuseDigiOpenAudioFile _openAudioFiles[DIMUSE_MAX_OPEN_FILES];
	IMuseDigiSndBuffer _soundBuffers[4];

	int diMUSEInitFilelist();
	void diMUSESaveLoadFilelist(Common::Serializer &ser);
	int diMUSELoadSound(const char *soundName, int requestedSoundId);
	void diMUSEUnloadSound(int soundId);
	void diMUSEUnloadAll();
	int diMUSEOpenSound(const char *filename, int requestedSoundId);
	void diMUSECloseSound(int soundId);
	void diMUSECloseAll();
	int diMUSEFindSound(const char *soundName);
	void diMUSEFlushSounds();
	int diMUSEAddSoundToList(IMuseDigiSoundList **listPtr, IMuseDigiSoundList *item);
	int diMUSERemoveSoundFromList(IMuseDigiSoundList **listPtr, IMuseDigiSoundList *item);

	bool filesCheckRange(int soundId);
	uint8 *filesGetSoundAddrData(int soundId);
	uint8 *filesFetchMap(int soundId);
	int filesSeek(int soundId, int32 offset, int mode);
	int filesRead(int soundId, uint8 *buf, int32 size);
	IMuseDigiSndBuffer *filesGetBufInfo(int bufId);

	int ihostLoadSound(const char *soundName, int requestedSoundId);
	int ihostUnloadSound(int soundId);
	int ihostOpenSound(const char *filename, int requestedSoundId);
	int ihostCloseSound(int soundId);
	uint8 *ihostGetMapAddr(int soundId);
	uint8 *ihostGetSoundAddr(int soundId);
	IMuseDigiSndBuffer *ihostGetBufferInfo(int bufId);
	int ihostFseek(int soundId, int32 offset, int mode);
	int ihostRead(int soundId, uint8 *destBuf, int32 size);

	IMuseDigiCachedSound _soundCache[DIMUSE_MAX_CACHED_SOUNDS];
	int _currentSoundSlot;

	int cacheGetSoundId(const char *soundName);
	int cachePurgeOldestStoppedSound();
	void cacheReset();
	uint8 *soundsGetExtFormatBuffer(uint8 *streamHeader, int *formatId, int *targetSampleRate, int *targetWordSize, int *targetNumChannels, int *swapEndianFlag, int *regionLength);
	int createSoundHeader(uint8 *buffer, int mode, int sampleRate, int wordSize, int numChannels, int regionLength);

	void soundsAllocateBuffer(int bufferId, int32 bufSize, int32 loadSize, int32 criticalSize);
	void soundsDeallocateBuffer(int bufferId);
	IMuseDigiResource *soundsOpenFile(const char *filename);
	void soundsCloseFile(IMuseDigiResource *res);
	void soundsFileSeek(IMuseDigiResource *res, int32 offset, int mode);
	int32 soundsFileTell(IMuseDigiResource *res);
	int soundsGetCompBlockId(IMuseDigiResource *res, int32 offset);
	int32 copyDataFromResource(IMuseDigiResource *res, uint8 *dest, int32 size);

	// Voice and lipsync
	char _curSpeechFile[32];
	int32 _millisecsInCurSpeechFile;
	int _lipsyncSoundIds[DIMUSE_MAX_LIPSYNCS];
	uint8 _lipsyncBufs[DIMUSE_MAX_LIPSYNCS][DIMUSE_LIPSYNC_MAX_SIZE + 48];
	int _globalVolume;

	int soundsRegisterVoice(const char *filename);
	void soundsCloseVoiceFile();
	void soundsReOpenVoiceFile();
	int loadLip(const char *lipsyncFilename, int soundId);
	void soundsResetLipsyncState(int soundId);
	int soundsGetPhoneme(int soundId, int offset);

	// Waveapi
	waveOutParamsStruct waveOutSettings;

	int _waveOutSampleRate;
	int _waveOutBytesPerSample;
	int _waveOutNumChannels;
	int _waveOutZeroLevel;
	int _waveOutPreferredFeedSize;
	uint8 *_waveOutMixBuffer;
	uint8 *_waveOutOutputBuffer;

	int _waveOutXorTrigger;
	int _waveOutWriteIndex;
	int _waveOutDisableWrite;

	int waveOutInit(waveOutParamsStruct *waveOutSettings);
	void waveOutWrite(uint8 **audioBuffer, int &feedSize, int &sampleRate);
	int waveOutDeinit();
	void waveOutCallback();
	byte waveOutGetStreamFlags();

	// Low latency mode
	void waveOutLowLatencyWrite(uint8 **audioBuffer, int &feedSize, int &sampleRate, int idx);
	void waveOutEmptyBuffer(int idx);

	uint8 *_waveOutLowLatencyOutputBuffer;

public:
	IMuseDigital(int sampleRate, Audio::Mixer *mixer, Common::Mutex *mutex, bool lowLatencyMode = false);
	~IMuseDigital();

	// Wrapper functions used by the main engine

	void stopSound(int sound);
	void stopAllSounds();
	int isSoundRunning(int soundId);
	void setPriority(int soundId, int priority);
	void setVolume(int soundId, int volume);
	void setPan(int soundId, int pan);
	void setFrequency(int soundId, int frequency);
	int  getCurSpeechVolume() const;
	int  getCurSpeechPan() const;
	int  getCurSpeechFrequency() const;
	void pause(bool pause);
	void disableEngine();
	bool isEngineDisabled();
	void floodMusicBuffer();
	void fillStreamsWhileMusicCritical(int fillTimesAfter);
	bool queryNextSoundFile(int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused);
	int getSampleRate();
	int getFeedSize();

	int32 getCurMusicPosInMs();
	int32 getSoundElapsedTimeInMs(int soundId);

	// Sounds API
	int soundsLoadStartSound(const char *soundName, int priority, int group);
	int soundsLoadProcessStartSound(const char *soundName, int priority, int group);
	void soundsStopSound(const char *soundName);
	int soundsGetParam(const char *soundName, int paramId);
	void soundsSetParam(const char *soundName, int paramId, int value);
	int soundsFadeParam(const char *soundName, int paramId, int value, int length);
	bool soundsIsSoundInUse(const char *soundName);
	bool soundsIsSoundPlaying(const char *soundName);
	int iMUSESetHook(const char *soundName, int hookId);
	int iMUSEGetHook(const char *soundName);
	void soundsPeriodic();
	int soundsPause();
	int soundsResume();
	int soundStartVoiceStream(const char *filename, int volume, int pan);
	int soundStopVoiceStream();
	int soundGetVoiceTime();
	int iMUSEGetPhoneme(int lipsyncId, int offset);

	// Called by the Groups module
	void tracksSetGroupVol();

	// General engine functions
	int diMUSETerminate();
	int diMUSEInitialize();
	void diMUSEPause();
	void diMUSEResume();
	int diMUSESaveLoad(Common::Serializer &ser);
	int diMUSESetGroupVol(int groupId, int volume);
	int diMUSEStartSound(int soundId, int priority);
	int diMUSEStopSound(int soundId);
	int diMUSEStopAllSounds();
	int diMUSEGetNextSound(int soundId);
	int diMUSESetParam(int soundId, int paramId, int value);
	int diMUSEGetParam(int soundId, int paramId);
	int diMUSEFadeParam(int soundId, int opcode, int destValue, int fadeLength);
	int diMUSESetHook(int soundId, int hookId);
	int diMUSEGetHook(int soundId);

	int diMUSESetTrigger(int soundId, const char *marker, int opcode,
		int d = -1, int e = -1, int f = -1, int g = -1,
		int h = -1, int i = -1, int j = -1, int k = -1,
		int l = -1, int m = -1, int n = -1);
	int diMUSECheckTrigger(int soundId, const char *marker, int opcode);
	int diMUSEClearTrigger(int soundId, const char *marker, int opcode);
	int diMUSEDeferCommand(int count, int opcode, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n);

	int diMUSEStartStream(int soundId, int priority, int bufferId);
	int diMUSESwitchStream(int oldSoundId, int newSoundId, int fadeDelay, int fadeSyncFlag2, int fadeSyncFlag1);
	int diMUSEProcessStreams();
	int diMUSEQueryStream(int soundId, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused);
	int diMUSEFeedStream(int soundId, uint8 *srcBuf, int32 sizeToFeed, int paused);
	int diMUSEGetMusicGroupVol();
	int diMUSEGetSFXGroupVol();
	int diMUSEGetVoiceGroupVol();
	int diMUSESetMusicGroupVol(int volume);
	int diMUSESetSFXGroupVol(int volume);
	int diMUSESetVoiceGroupVol(int volume);
	int diMUSEInitializeScript();
	int diMUSETerminateScript();
	void diMUSESaveLoadScript(Common::Serializer &ser);
	void diMUSERefreshScript();
	int diMUSESetState(int soundId);
	int diMUSESetSequence(int soundId);
	int diMUSESetCuePoint(int cueId);
	int diMUSESetAttribute(int attrIndex, int attrVal);
	void diMUSEEnableSpooledMusic();
	void diMUSEDisableSpooledMusic();

	// Utils
	int addTrackToList(IMuseDigiTrack **listPtr, IMuseDigiTrack *listPtr_Item);
	int removeTrackFromList(IMuseDigiTrack **listPtr, IMuseDigiTrack *itemPtr);
	int addStreamZoneToList(IMuseDigiStreamZone **listPtr, IMuseDigiStreamZone *listPtr_Item);
	int removeStreamZoneFromList(IMuseDigiStreamZone **listPtr, IMuseDigiStreamZone *itemPtr);
	int clampNumber(int value, int minValue, int maxValue);
	int checkHookId(int &trackHookId, int sampleHookId);
	int roundRobinSetBufferCount();
	void adaptBufferCount();

	// CMDs
	int cmdsHandleCmd(int cmd, uint8 *ptr = nullptr,
		int a = -1, int b = -1, int c = -1, int d = -1, int e = -1,
		int f = -1, int g = -1, int h = -1, int i = -1, int j = -1,
		int k = -1, int l = -1, int m = -1, int n = -1);

	// Script
	int scriptTriggerCallback(char *marker);

	// Debugger utility functions
	void listStates();
	void listSeqs();
	void listCues();
	void listTracks();
	void listGroups();
};

} // End of namespace Grim

#endif
