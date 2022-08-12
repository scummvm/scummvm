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

#if !defined(SCUMM_IMUSE_DIGI_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_IMUSE_DIGI_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "common/serializer.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "scumm/scumm_v7.h"
#include "scumm/music.h"
#include "scumm/sound.h"
#include "scumm/file.h"
#include "scumm/debugger.h"

#include "scumm/imuse_digi/dimuse_defs.h"
#include "scumm/imuse_digi/dimuse_internalmixer.h"
#include "scumm/imuse_digi/dimuse_groups.h"
#include "scumm/imuse_digi/dimuse_fades.h"
#include "scumm/imuse_digi/dimuse_files.h"
#include "scumm/imuse_digi/dimuse_triggers.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"
#include "scumm/imuse_digi/dimuse_sndmgr.h"
#include "scumm/imuse_digi/dimuse_tables.h"

#include "scumm/smush/smush_player.h"

#include "audio/mixer.h"
#include "audio/decoders/raw.h"

namespace Audio {
class AudioStream;
class Mixer;
class QueuingAudioStream;
}

namespace Scumm {
class ScummEngine_v7;
class SmushPlayer;

struct imuseDigTable;
struct imuseComiTable;
struct IMuseDigiDispatch;
struct IMuseDigiTrack;
struct IMuseDigiStreamZone;

class IMuseDigital : public MusicEngine {
private:
	Common::Mutex _mutex;
	ScummEngine_v7 *_vm;
	Audio::Mixer *_mixer;
	SmushPlayer *_splayer;

	IMuseDigiInternalMixer *_internalMixer;
	IMuseDigiGroupsHandler *_groupsHandler;
	IMuseDigiFadesHandler *_fadesHandler;
	IMuseDigiTriggersHandler *_triggersHandler;
	IMuseDigiFilesHandler *_filesHandler;

	int _callbackFps;
	static void timer_handler(void *refConf);
	void callback();

	bool _isEarlyDiMUSE;
	bool _isEngineDisabled;

	// These three are manipulated in the waveOut functions
	uint8 *_outputAudioBuffer;
	int _outputFeedSize;
	int _outputSampleRate;

	int _maxQueuedStreams; // maximum number of streams which can be queued before they are played
	int _nominalBufferCount;

	int _currentSpeechVolume, _currentSpeechFrequency, _currentSpeechPan;
	int _curMixerMusicVolume, _curMixerSpeechVolume, _curMixerSFXVolume;
	bool _radioChatterSFX;

	int32 _attributes[188];	// internal attributes for each music file to store and check later
	int32 _nextSeqToPlay;
	int32 _curMusicState;
	int32 _curMusicSeq;
	int32 _curMusicCue;

	char *_audioNames;		// filenames of sound SFX used in FT
	int32 _numAudioNames;	// number of above filenames
	uint8 _currentVOCHeader[52]; // Header for the current sound for early DiMUSE

	int _stopSequenceFlag;
	int _scriptInitializedFlag;
	char _emptyMarker[1];
	bool _spooledMusicEnabled;

	int _usecPerInt; // Microseconds between each callback (will be set to 50 Hz)
	int _callbackInterruptFlag;
	void diMUSEHeartbeat();

	void setFtMusicState(int stateId);
	void setFtMusicSequence(int seqId);
	void playFtMusic(const char *songName, int opcode, int volume);
	void setDigMusicState(int stateId);
	void setDigMusicSequence(int seqId);
	void playDigMusic(const char *songName, const imuseDigTable *table, int attribPos, bool sequence);
	void setComiMusicState(int stateId);
	void setComiMusicSequence(int seqId);
	void playComiMusic(const char *songName, const imuseComiTable *table, int attribPos, bool sequence);
	void playComiDemoMusic(const char *songName, const imuseComiTable *table, int attribPos, bool sequence);
	int getSoundIdByName(const char *soundName);

	// Script
	int scriptParse(int cmd, int a, int b);
	int scriptInit();
	int scriptTerminate();
	void scriptRefresh();
	void scriptSetState(int soundId);
	void scriptSetSequence(int soundId);
	void scriptSetCuePoint(int cueId);
	int scriptSetAttribute(int attrIndex, int attrVal);

	// CMDs
	int _cmdsPauseCount;
	int _cmdsRunning60HzCount;
	int _cmdsRunning10HzCount;

	int cmdsInit();
	int cmdsDeinit();
	int cmdsTerminate();
	int cmdsPause();
	int cmdsResume();
	void cmdsSaveLoad(Common::Serializer &ser);
	int cmdsStartSound(int soundId, int priority);
	int cmdsStopSound(int soundId);
	int cmdsStopAllSounds();
	int cmdsGetNextSound(int soundId);
	int cmdsSetParam(int soundId, int opcode, int value);
	int cmdsGetParam(int soundId, int opcode);
	int cmdsSetHook(int soundId, int hookId);
	int cmdsGetHook(int soundId);

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
	int streamerQueryStream(IMuseDigiStream *streamPtr, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused);
	int streamerFeedStream(IMuseDigiStream *streamPtr, uint8 *srcBuf, int32 sizeToFeed, int paused);
	int streamerFetchData(IMuseDigiStream *streamPtr);
	void streamerSetLoopFlag(IMuseDigiStream *streamPtr, int offset);
	void streamerRemoveLoopFlag(IMuseDigiStream *streamPtr);

	// Tracks
	IMuseDigiTrack _tracks[DIMUSE_MAX_TRACKS];
	IMuseDigiTrack *_trackList;

	int _trackCount;
	int _tracksPauseTimer;
	int _tracksPrefSampleRate;
	int _tracksMicroSecsToFeed;

	int tracksInit();
	void tracksPause();
	void tracksResume();
	void tracksSaveLoad(Common::Serializer &ser);
	void tracksSetGroupVol();
	void tracksCallback();
	int tracksStartSound(int soundId, int tryPriority, int group);
	int tracksStopSound(int soundId);
	int tracksStopAllSounds();
	int tracksGetNextSound(int soundId);
	void tracksQueryStream(int soundId, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused);
	int tracksFeedStream(int soundId, uint8 *srcBuf, int32 sizeToFeed, int paused);
	void tracksClear(IMuseDigiTrack *trackPtr);
	int tracksSetParam(int soundId, int opcode, int value);
	int tracksGetParam(int soundId, int opcode);
	int tracksLipSync(int soundId, int syncId, int msPos, int32 &width, int32 &height);
	int tracksSetHook(int soundId, int hookId);
	int tracksGetHook(int soundId);
	IMuseDigiTrack *tracksReserveTrack(int priority);
	void tracksDeinit();

	// Dispatch
	IMuseDigiDispatch _dispatches[DIMUSE_MAX_DISPATCHES];
	IMuseDigiStreamZone _streamZones[DIMUSE_MAX_STREAMZONES];
	uint8 *_dispatchBuffer;
	uint8 _ftCrossfadeBuffer[30000]; // Used by FT & DIG demo
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
	IMuseDigiDispatch *dispatchGetDispatchByTrackId(int trackId);
	void dispatchSaveLoad(Common::Serializer &ser);
	int dispatchRestoreStreamZones();
	int dispatchAllocateSound(IMuseDigiTrack *trackPtr, int groupId);
	int dispatchRelease(IMuseDigiTrack *trackPtr);
	int dispatchSwitchStream(int oldSoundId, int newSoundId, int fadeLength, int unusedFadeSyncFlag, int offsetFadeSyncFlag);
	int dispatchSwitchStream(int oldSoundId, int newSoundId, uint8 *crossfadeBuffer, int crossfadeBufferSize, int vocLoopFlag);
	void dispatchProcessDispatches(IMuseDigiTrack *trackPtr, int feedSize, int sampleRate);
	void dispatchProcessDispatches(IMuseDigiTrack *trackPtr, int feedSize);
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
	void dispatchValidateFadeSize(IMuseDigiDispatch *dispatchPtr, int32 &dispatchSize, const char *functionName);
	int dispatchUpdateFadeMixVolume(IMuseDigiDispatch *dispatchPtr, int32 remainingFade);
	int dispatchUpdateFadeSlope(IMuseDigiDispatch *dispatchPtr);
	void dispatchVOCLoopCallback(int soundId);
	int dispatchSeekToNextChunk(IMuseDigiDispatch *dispatchPtr);

	// Wave (mainly a wrapper for Tracks functions)
	int waveInit();
	int waveTerminate();
	int wavePause();
	int waveResume();
	void waveSaveLoad(Common::Serializer &ser);
	void waveUpdateGroupVolumes();
	int waveStartSound(int soundId, int priority);
	int waveStopSound(int soundId);
	int waveStopAllSounds();
	int waveGetNextSound(int soundId);
	int waveSetParam(int soundId, int opcode, int value);
	int waveGetParam(int soundId, int opcode);
	int waveSetHook(int soundId, int hookId);
	int waveGetHook(int soundId);
	int waveStartStream(int soundId, int priority, int groupId);
	int waveSwitchStream(int oldSoundId, int newSoundId, int fadeLengthMs, int fadeSyncFlag2, int fadeSyncFlag1);
	int waveSwitchStream(int oldSoundId, int newSoundId, uint8 *crossfadeBuffer, int crossfadeBufferSize, int vocLoopFlag);
	int waveProcessStreams();
	void waveQueryStream(int soundId, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused);
	int waveFeedStream(int soundId, uint8 *srcBuf, int32 sizeToFeed, int paused);
	int waveLipSync(int soundId, int syncId, int msPos, int32 &width, int32 &height);

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

	int waveOutInit(int sampleRate, waveOutParamsStruct *waveOutSettings);
	void waveOutWrite(uint8 **audioBuffer, int &feedSize, int &sampleRate);
	int waveOutDeinit();
	void waveOutCallback();
	byte waveOutGetStreamFlags();

public:
	IMuseDigital(ScummEngine_v7 *scumm, Audio::Mixer *mixer);
	~IMuseDigital() override;

	// Wrapper functions used by the main engine

	void startSound(int sound) override { error("IMuseDigital::startSound(int) should be never called"); }
	void setMusicVolume(int vol) override {}
	void stopSound(int sound) override;
	void stopAllSounds() override;
	int getSoundStatus(int sound) const override { return 0; }
	int isSoundRunning(int soundId); // Needed because getSoundStatus is a const function, and I needed a workaround
	int startVoice(int soundId, const char *soundName, byte speakingActorId);
	int startVoice(const char *fileName, ScummFile *file, uint32 offset, uint32 size);
	void saveLoadEarly(Common::Serializer &ser);
	void setRadioChatterSFX(bool state);
	void setAudioNames(int32 num, char *names);
	int  startSfx(int soundId, int priority) ;
	void setPriority(int soundId, int priority);
	void setVolume(int soundId, int volume);
	void setPan(int soundId, int pan);
	void setFrequency(int soundId, int frequency);
	int  getCurSpeechVolume() const;
	int  getCurSpeechPan() const;
	int  getCurSpeechFrequency() const;
	void pause(bool pause);
	void parseScriptCmds(int cmd, int soundId, int sub_cmd, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n, int o, int p);
	void refreshScripts();
	void flushTracks();
	void disableEngine();
	bool isEngineDisabled();
	void stopSMUSHAudio();
	void receiveAudioFromSMUSH(uint8 *srcBuf, int32 inFrameCount, int32 feedSize, int32 mixBufStartIndex, int volume, int pan, bool is11025Hz);
	void setSmushPlayer(SmushPlayer *splayer);

	bool isFTSoundEngine(); // Used in the handlers to check if we're using the FT version of the engine

	int32 getCurMusicPosInMs();
	int32 getCurVoiceLipSyncWidth();
	int32 getCurVoiceLipSyncHeight();
	int32 getCurMusicLipSyncWidth(int syncId);
	int32 getCurMusicLipSyncHeight(int syncId);
	void getSpeechLipSyncInfo(int32 &width, int32 &height);
	void getMusicLipSyncInfo(int syncId, int32 &width, int32 &height);
	int32 getSoundElapsedTimeInMs(int soundId);

	// General engine functions
	int diMUSETerminate();
	int diMUSEInitialize();
	int diMUSEPause();
	int diMUSEResume();
	void diMUSESaveLoad(Common::Serializer &ser);
	int diMUSESetGroupVol(int groupId, int volume);
	int diMUSEStartSound(int soundId, int priority);
	int diMUSEStopSound(int soundId);
	int diMUSEStopAllSounds();
	int diMUSEGetNextSound(int soundId);
	int diMUSESetParam(int soundId, int paramId, int value);
	int diMUSEGetParam(int soundId, int paramId);
	int diMUSEFadeParam(int soundId, int opcode, int destValue, int fadeLength);
	int diMUSESetHook(int soundId, int hookId);

	int diMUSESetTrigger(int soundId, int marker, int opcode,
		int d = -1, int e = -1, int f = -1, int g = -1,
		int h = -1, int i = -1, int j = -1, int k = -1,
		int l = -1, int m = -1, int n = -1);

	int diMUSEStartStream(int soundId, int priority, int groupId);
	int diMUSESwitchStream(int oldSoundId, int newSoundId, int fadeDelay, int fadeSyncFlag2, int fadeSyncFlag1);
	int diMUSESwitchStream(int oldSoundId, int newSoundId, uint8 *crossfadeBuffer, int crossfadeBufferSize, int vocLoopFlag);
	int diMUSEProcessStreams();
	void diMUSEQueryStream(int soundId, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused);
	int diMUSEFeedStream(int soundId, uint8 *srcBuf, int32 sizeToFeed, int paused);
	int diMUSELipSync(int soundId, int syncId, int msPos, int32 &width, int32 &height);
	int diMUSEGetMusicGroupVol();
	int diMUSEGetSFXGroupVol();
	int diMUSEGetVoiceGroupVol();
	int diMUSESetMusicGroupVol(int volume);
	int diMUSESetSFXGroupVol(int volume);
	int diMUSESetVoiceGroupVol(int volume);
	void diMUSEUpdateGroupVolumes();
	int diMUSEInitializeScript();
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
	int clampTuning(int value, int minValue, int maxValue);
	int checkHookId(int &trackHookId, int sampleHookId);
	int roundRobinSetBufferCount();

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

} // End of namespace Scumm

#endif
