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

#include "engines/grim/imuse/imuse_defs.h"
#include "engines/grim/imuse/imuse_internalmixer.h"
#include "engines/grim/imuse/imuse_groups.h"
#include "engines/grim/imuse/imuse_fades.h"
#include "engines/grim/imuse/imuse_triggers.h"
#include "engines/grim/imuse/imuse_tables.h"

#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "audio/effects/hmi/hmi_types.h"

namespace Audio {
class AudioStream;
class HMIFxLib;
class Mixer;
class QueuingAudioStream;
}

namespace Grim {

struct IMuseStateTable;
struct IMuseDispatch;
struct IMuseTrack;
struct IMuseStreamZone;

class IMuse {
private:
	Common::Mutex *_mutex;
	Audio::Mixer *_mixer;

	IMuseInternalMixer *_internalMixer;
	IMuseGroupsHandler *_groupsHandler;
	IMuseFadesHandler *_fadesHandler;
	IMuseTriggersHandler *_triggersHandler;

	int _callbackFps;
	static void timer_handler(void *refConf);

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
	uint8 *_outputLowLatencyAudioBuffers[IMUSE_MAX_TRACKS];

	int _maxQueuedStreams; // maximum number of streams which can be queued before they are played
	int _nominalBufferCount;

	int _curMixerMusicVolume, _curMixerSpeechVolume, _curMixerSFXVolume;

	int32 _attributes[228];	// internal attributes for each music file to store and check later
	int32 _nextSeqToPlay;
	int32 _curMusicState;
	int32 _curMusicSeq;

	bool _scriptInitializedFlag;
	char _emptyMarker[1];

	int _usecPerInt; // Microseconds between each callback (will be set to 50 Hz)
	bool _callbackInterruptFlag;
	void imHeartbeat();

	bool soundsIsMusicStreamIdle();
	bool soundsIsMusicCritical();

	// Script
	int scriptParse(int cmd, int a, int b);
	int scriptInit();
	int scriptTerminate();
	void scriptSaveLoad(Common::Serializer &ser);
	void scriptDeinit();
	void scriptRefresh();
	void scriptSetState(int soundId);
	void scriptSetSequence(int soundId);
	void scriptPlayMusic(const IMuseStateTable *tableEntry, int curSongEntryId, int isSequence);
	void scriptSetCuePoint(int cueId);
	int scriptSetAttribute(int attrIndex, int attrVal);

	// CMDs
	bool _initialized;
	int _cmdsPauseCount;
	int _cmdsRunning50HzCount;
	int _cmdsRunning5HzCount;

	// Streamer
	IMuseStream _streams[IMUSE_MAX_STREAMS];
	IMuseStream *_lastStreamLoaded;
	int _streamerBailFlag;

	int streamerInit();
	IMuseStream *streamerAllocateSound(int soundId, int bufId, int32 maxRead);
	int streamerClearSoundInStream(IMuseStream *streamPtr);
	int streamerProcessStreams();
	uint8 *streamerGetStreamBuffer(IMuseStream *streamPtr, int size);
	uint8 *streamerGetStreamBufferAtOffset(IMuseStream *streamPtr, int32 offset, int size);
	int streamerSetReadIndex(IMuseStream *streamPtr, int offset);
	int streamerSetLoadIndex(IMuseStream *streamPtr, int offset);
	int streamerGetFreeBufferAmount(IMuseStream *streamPtr);
	int streamerSetSoundToStreamFromOffset(IMuseStream *streamPtr, int soundId, int32 offset);
	void streamerQueryStream(IMuseStream *streamPtr, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused);
	int streamerFeedStream(IMuseStream *streamPtr, uint8 *srcBuf, int32 sizeToFeed, int paused);
	int streamerFetchData(IMuseStream *streamPtr);

	// Tracks
	IMuseTrack _tracks[IMUSE_MAX_TRACKS];
	IMuseTrack *_trackList;

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
	void tracksClear(IMuseTrack *trackPtr);
	int tracksSetParam(int soundId, int opcode, int value);
	int tracksGetParam(int soundId, int opcode);
	int tracksSetHook(int soundId, int hookId);
	int tracksGetHook(int soundId);
	IMuseTrack *tracksReserveTrack(int priority);
	int tracksDeinit();

	// Dispatch
	IMuseDispatch _dispatches[IMUSE_MAX_DISPATCHES];
	IMuseStreamZone _streamZones[IMUSE_MAX_STREAMZONES];
	uint8 *_dispatchBuffer;
	int32 _dispatchSize;
	uint8 *_dispatchSmallFadeBufs;
	uint8 *_dispatchLargeFadeBufs;
	int32 _dispatchFadeSize;
	int _dispatchLargeFadeFlags[IMUSE_LARGE_FADES];
	int _dispatchSmallFadeFlags[IMUSE_SMALL_FADES];
	int _dispatchFadeStartedFlag;
	int _dispatchBufferedHookId;
	int32 _dispatchJumpFadeSize;
	int32 _dispatchCurStreamBufSize;
	int32 _dispatchCurStreamCriticalSize;
	int32 _dispatchCurStreamFreeSpace;
	int _dispatchCurStreamPaused;

	int dispatchInit();
	int dispatchTerminate();
	IMuseDispatch *dispatchGetDispatchByTrackId(int trackId);
	void dispatchSaveLoad(Common::Serializer &ser);
	int dispatchRestoreStreamZones();
	int dispatchAllocateSound(IMuseTrack *trackPtr, int groupId);
	int dispatchRelease(IMuseTrack *trackPtr);
	int dispatchSwitchStream(int oldSoundId, int newSoundId, int fadeLength, int exitTriggerSyncFlag, int offsetFadeSyncFlag);
	void dispatchProcessDispatches(IMuseTrack *trackPtr, int feedSize, int sampleRate);
	void dispatchPredictFirstStream();
	int dispatchNavigateMap(IMuseDispatch *dispatchPtr);
	int dispatchGetMap(IMuseDispatch *dispatchPtr);
	int dispatchConvertMap(uint8 *rawMap, int32 *destMap);
	uint8 *dispatchGetNextMapEvent(int32 *mapPtr, int32 soundOffset, uint8 *mapEvent);
	void dispatchPredictStream(IMuseDispatch *dispatchPtr);
	uint8 *dispatchCheckForJump(int32 *mapPtr, IMuseStreamZone *strZnPtr, int &candidateHookId);
	void dispatchPrepareToJump(IMuseDispatch *dispatchPtr, IMuseStreamZone *strZnPtr, uint8 *jumpParamsFromMap, int calledFromGetNextMapEvent);
	void dispatchStreamNextZone(IMuseDispatch *dispatchPtr, IMuseStreamZone *strZnPtr);
	IMuseStreamZone *dispatchAllocateStreamZone();
	uint8 *dispatchAllocateFade(int32 &fadeSize, const char *functionName);
	void dispatchDeallocateFade(IMuseDispatch *dispatchPtr, const char *functionName);
	int dispatchGetFadeSize(IMuseDispatch *dispatchPtr, int fadeLength);
	void dispatchValidateFadeSize(IMuseDispatch *dispatchPtr, int32 &dispatchSize);
	int dispatchUpdateFadeMixVolume(IMuseDispatch *dispatchPtr, int32 remainingFade);
	int dispatchUpdateFadeSlope(IMuseDispatch *dispatchPtr);

	// Files
	IMuseSoundList _mainSoundList[IMUSE_MAX_SOUND_LIST];
	IMuseSoundList *_loadedSoundList;
	IMuseSoundList *_openedSoundsList;
	bool _fileListInitialized;
	uint16 _imuseDestTable[5786];
	IMuseOpenAudioFile _openAudioFiles[IMUSE_MAX_OPEN_FILES];
	IMuseSndBuffer _soundBuffers[4];

	int imInitFilelist();
	void imSaveLoadFilelist(Common::Serializer &ser);
	int imLoadSound(const char *soundName, int requestedSoundId);
	void imUnloadSound(int soundId);
	void imUnloadAll();
	int imOpenSound(const char *filename, int requestedSoundId);
	void imCloseSound(int soundId);
	void imCloseAll();
	int imFindSound(const char *soundName);
	void imFlushSounds();
	int imAddSoundToList(IMuseSoundList **listPtr, IMuseSoundList *item);
	int imRemoveSoundFromList(IMuseSoundList **listPtr, IMuseSoundList *item);

	bool filesCheckRange(int soundId);
	uint8 *filesGetSoundAddrData(int soundId);
	uint8 *filesFetchMap(int soundId);
	int filesSeek(int soundId, int32 offset, int mode);
	int filesRead(int soundId, uint8 *buf, int32 size);
	IMuseSndBuffer *filesGetBufInfo(int bufId);

	int ihostLoadSound(const char *soundName, int requestedSoundId);
	int ihostUnloadSound(int soundId);
	int ihostOpenSound(const char *filename, int requestedSoundId);
	int ihostCloseSound(int soundId);
	uint8 *ihostGetMapAddr(int soundId);
	uint8 *ihostGetSoundAddr(int soundId);
	IMuseSndBuffer *ihostGetBufferInfo(int bufId);
	int ihostFseek(int soundId, int32 offset, int mode);
	int ihostRead(int soundId, uint8 *destBuf, int32 size);

	IMuseCachedSound _soundCache[IMUSE_MAX_CACHED_SOUNDS];
	int _currentSoundSlot;

	int cacheGetSoundId(const char *soundName);
	int cachePurgeOldestStoppedSound();
	void cacheReset();
	uint8 *soundsGetExtFormatBuffer(uint8 *streamHeader, int *formatId, int *targetSampleRate, int *targetWordSize, int *targetNumChannels, int *swapEndianFlag, int *regionLength);
	int createSoundHeader(uint8 *buffer, int mode, int sampleRate, int wordSize, int numChannels, int regionLength);

	void soundsAllocateBuffer(int bufferId, int32 bufSize, int32 loadSize, int32 criticalSize);
	void soundsDeallocateBuffer(int bufferId);
	IMuseResource *soundsOpenFile(const char *filename);
	void soundsCloseFile(IMuseResource *res);
	void soundsFileSeek(IMuseResource *res, int32 offset, int mode);
	int32 soundsFileTell(IMuseResource *res);
	int soundsGetCompBlockId(IMuseResource *res, int32 offset);
	int32 copyDataFromResource(IMuseResource *res, uint8 *dest, int32 size);

	// Voice and lipsync
	char _curSpeechFile[32];
	int32 _millisecsInCurSpeechFile;
	int _lipsyncSoundIds[IMUSE_MAX_LIPSYNCS];
	uint8 _lipsyncBufs[IMUSE_MAX_LIPSYNCS][IMUSE_LIPSYNC_MAX_SIZE + 48];
	int _globalVolume;

	int soundsRegisterVoice(const char *filename);
	void soundsCloseVoiceFile();
	void soundsReOpenVoiceFile();
	int loadLip(const char *lipsyncFilename, int soundId);
	void soundsResetLipsyncState(int soundId);
	int soundsGetPhoneme(int soundId, int offset);

	// HMI voice effects
	Audio::HMIFxLib *_hmiFxLib;
	Audio::HMIInitData _hmiInitData;
	Audio::HMILibrary *_presetCollection;
	Audio::HMIProcessRequest _hmiRequest;
	Audio::HMIPreset *_loadedPreset;
	Audio::HMIFormat _hmiInputFmt;
	Audio::HMIFormat _hmiOutputFmt;
	bool _hmiFxInitialized;
	int _curHMIEffectIdInternal;
	int _curEffectId;
	char _curEffectName[40];
	uint8 *_voiceSrcBuf;
	int32 _maxChunkRequested;
	uint8 *_voiceRemainderBuf;
	int32 _voiceRemainderSize;
	int32 _voiceRemainderCapacity;
	int32 _voiceRemainderPos;

	int setupHMI(int heapSize);
	int registerHMIEffect(const char *effectName);
	int processSound(uint8 *srcBuf, uint32 soundLength, uint8 *destBuf, uint8 **outBuf, int32 *outLength, bool isLastChunk);
	void soundsTerminateProcessing();
	void allocateProcessedSound(int luaSoundId);

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
	IMuse(int sampleRate, Audio::Mixer *mixer, Common::Mutex *mutex, bool lowLatencyMode = false);
	~IMuse();

	// Wrapper functions used by the main engine

	void pause(bool pause);
	void soundsFloodMusicBuffer();
	void soundsFillStreamsWhileMusicCritical(int fillTimesAfter);
	bool queryMusicStream(int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused);

	int32 soundsGetMusicPosition();

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
	void iMUSESetVoiceEffect(const char *effectName);
	void soundsSaveLoad(Common::Serializer &ser);
	void soundsRestoreLegacy(int32 stateIndex, int32 seqIndex);
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
	int imTerminate();
	int imInitialize();
	void imPause();
	void imResume();
	int imSaveLoad(Common::Serializer &ser);
	int imSetGroupVol(int groupId, int volume);
	int imStartSound(int soundId, int priority);
	int imStopSound(int soundId);
	int imStopAllSounds();
	int imGetNextSound(int soundId);
	int imSetParam(int soundId, int paramId, int value);
	int imGetParam(int soundId, int paramId);
	int imFadeParam(int soundId, int opcode, int destValue, int fadeLength);
	int imSetHook(int soundId, int hookId);
	int imGetHook(int soundId);

	int imSetTrigger(int soundId, const char *marker, int opcode,
		int d = -1, int e = -1, int f = -1, int g = -1,
		int h = -1, int i = -1, int j = -1, int k = -1,
		int l = -1, int m = -1, int n = -1);
	int imCheckTrigger(int soundId, const char *marker, int opcode);
	int imClearTrigger(int soundId, const char *marker, int opcode);
	int imDeferCommand(int count, int opcode, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n);

	int imStartStream(int soundId, int priority, int bufferId);
	int imSwitchStream(int oldSoundId, int newSoundId, int fadeDelay, int fadeSyncFlag2, int fadeSyncFlag1);
	int imProcessStreams();
	int imQueryStream(int soundId, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused);
	int imFeedStream(int soundId, uint8 *srcBuf, int32 sizeToFeed, int paused);
	int getMusicGroupVol();
	int getSFXGroupVol();
	int getVoiceGroupVol();
	int setMusicGroupVol(int volume);
	int setSFXGroupVol(int volume);
	int setVoiceGroupVol(int volume);
	int imInitializeScript();
	int imTerminateScript();
	void imSaveLoadScript(Common::Serializer &ser);
	void imRefreshScript();
	int imSetState(int soundId);
	int imSetSequence(int soundId);
	int imSetCuePoint(int cueId);
	int imSetAttribute(int attrIndex, int attrVal);

	// Utils
	int addTrackToList(IMuseTrack **listPtr, IMuseTrack *listPtr_Item);
	int removeTrackFromList(IMuseTrack **listPtr, IMuseTrack *itemPtr);
	int addStreamZoneToList(IMuseStreamZone **listPtr, IMuseStreamZone *listPtr_Item);
	int removeStreamZoneFromList(IMuseStreamZone **listPtr, IMuseStreamZone *itemPtr);
	int clampNumber(int value, int minValue, int maxValue);
	int checkHookId(int &trackHookId, int sampleHookId);
	void adaptBufferCount();

	// CMDs
	int imSoundCall(int cmd, uint8 *ptr = nullptr,
		int a = -1, int b = -1, int c = -1, int d = -1, int e = -1,
		int f = -1, int g = -1, int h = -1, int i = -1, int j = -1,
		int k = -1, int l = -1, int m = -1, int n = -1);

	// Script
	int scriptTriggerCallback(char *marker);
};

extern IMuse *g_imuse;

} // End of namespace Grim

#endif
