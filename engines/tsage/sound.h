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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_SOUND_H
#define TSAGE_SOUND_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "common/queue.h"
#include "audio/audiostream.h"
#include "audio/fmopl.h"
#include "audio/mixer.h"
#include "common/list.h"
#include "tsage/saveload.h"
#include "tsage/core.h"

namespace tSage {

class Sound;

#define SOUND_ARR_SIZE 16
#define ROLAND_DRIVER_NUM 2
#define ADLIB_DRIVER_NUM 3
#define SBLASTER_DRIVER_NUM 4
#define CALLBACKS_PER_SECOND 60

struct trackInfoStruct {
	int _numTracks;
	int _chunks[SOUND_ARR_SIZE];
	int _voiceTypes[SOUND_ARR_SIZE];
};

enum SoundDriverStatus {SNDSTATUS_FAILED = 0, SNDSTATUS_DETECTED = 1, SNDSTATUS_SKIPPED = 2};
enum VoiceType {VOICETYPE_0 = 0, VOICETYPE_1 = 1};

class SoundDriverEntry {
public:
	int driverNum;
	SoundDriverStatus status;
	int field2, field6;
	Common::String shortDescription;
	Common::String longDescription;
};

struct GroupData {
	uint32 groupMask;
	byte v1;
	byte v2;
	const byte *pData;
};

struct RegisterValue {
	uint8 _regNum;
	uint8 _value;

	RegisterValue(int regNum, int value) { 
		_regNum = regNum; _value = value;
	}
};

class SoundDriver {
public:
	Common::String _shortDescription, _longDescription;
	int _minVersion, _maxVersion;
	// The following fields were originally held in separate arrays in the SoundManager class
	uint32 _groupMask;
	const GroupData *_groupOffset;
	int _driverResID;
public:
	SoundDriver();
	virtual ~SoundDriver() {};

	const Common::String &getShortDriverDescription() { return _shortDescription; }
	const Common::String &getLongDriverDescription() { return _longDescription; }

	virtual bool open() { return true; }						// Method #0
	virtual void close() {}										// Method #1
	virtual bool reset() { return true; }						// Method #2
	virtual const GroupData *getGroupData() { return NULL; }	// Method #3
	virtual void installPatch(const byte *data, int size) {}	// Method #4
	virtual void poll() {}										// Method #5
	virtual void proc12() {}									// Method #6
	virtual int setMasterVolume(int volume) { return 0; }		// Method #7
	virtual void proc16() {}									// Method #8
	virtual void proc18(int al, VoiceType voiceType) {}			// Method #9
	virtual void proc20(int al, VoiceType voiceType) {}			// Method #10
	virtual void proc22(int al, VoiceType voiceType, int v3) {}	// Method #11
	virtual void proc24(int channel, int voiceIndex, Sound *sound, int v1, int v2) {}
	virtual void setProgram(int channel, int program) {}			// Method #13
	virtual void setVolume1(int channel, int v2, int v3, int volume) {}
	virtual void setPitchBlend(int channel, int pitchBlend) {}		// Method #15
	virtual void proc32(const byte *channelData, int dataOffset, int program, int channel, int v0, int v1) {}// Method #16
	virtual void updateVoice(int channel) {}						// Method #17
	virtual void proc36() {}										// Method #18
	virtual void proc38(int channel, int cmd, int value) {}			// Method #19
	virtual void setPitch(int channel, int pitchBlend) {}			// Method #20
	virtual void proc42(int channel, int cmd, int value, int *v1, int *v2) {}	// Method #21
};

struct VoiceStructEntryType0 {
	Sound *_sound;
	int _channelNum;
	int _priority;
	int _fieldA;
	Sound *_sound2;
	int _channelNum2;
	int _priority2;
	int _field12;
	Sound *_sound3;
	int _channelNum3;
	int _priority3;
	int _field1A;
};

struct VoiceStructEntryType1 {
	int _field4;
	int _field5;
	int _field6;
	Sound *_sound;
	int _channelNum;
	int _priority;
	Sound *_sound2;
	int _channelNum2;
	int _priority2;
	Sound *_sound3;
	int _channelNum3;
	int _priority3;
};

struct VoiceStructEntry {
	int _voiceNum;
	int _field1;
	SoundDriver *_driver;

	VoiceStructEntryType0 _type0;
	VoiceStructEntryType1 _type1;
};

class VoiceTypeStruct {
public:
	VoiceType _voiceType;
	int _total;
	int _numVoices;
	int _field3;

	Common::Array<VoiceStructEntry> _entries;
};

class SoundManager : public SaveListener {
private:
	SoundDriver *instantiateDriver(int driverNum);
public:
	bool __sndmgrReady;
	int _ourSndResVersion, _ourDrvResVersion;
	SynchronizedList<Sound *> _playList;
	Common::List<SoundDriver *> _installedDrivers;
	VoiceTypeStruct *_voiceTypeStructPtrs[SOUND_ARR_SIZE];
	uint32 _groupsAvail;
	int _masterVol;
	int _newVolume;
	Common::Mutex _serverDisabledMutex;
	Common::Mutex _serverSuspendedMutex;
	bool _driversDetected;
	SynchronizedList<Sound *> _soundList;
	Common::List<SoundDriverEntry> _availableDrivers;
	bool _needToRethink;
	// Misc flags
	bool _soTimeIndexFlag;
public:
	SoundManager();
	~SoundManager();

	void dispatch();
	virtual void listenerSynchronize(Serializer &s);
	virtual void postInit();
	void syncSounds();
	void update();

	static void saveNotifier(bool postFlag);
	void saveNotifierProc(bool postFlag);
	static void loadNotifier(bool postFlag);
	void loadNotifierProc(bool postFlag);

	void installConfigDrivers();
	Common::List<SoundDriverEntry> &buildDriverList(bool detectFlag);
	Common::List<SoundDriverEntry> &getDriverList(bool detectFlag);
	void dumpDriverList();
	void installDriver(int driverNum);
	bool isInstalled(int driverNum) const;
	void unInstallDriver(int driverNum);
	void checkResVersion(const byte *soundData);
	int determineGroup(const byte *soundData);
	int extractPriority(const byte *soundData);
	int extractLoop(const byte *soundData);
	bool isOnPlayList(Sound *sound);
	void extractTrackInfo(trackInfoStruct *trackInfo, const byte *soundData, int groupNum);
	void addToSoundList(Sound *sound);
	void removeFromSoundList(Sound *sound);
	void addToPlayList(Sound *sound);
	void removeFromPlayList(Sound *sound);
	void rethinkVoiceTypes();
	void updateSoundVol(Sound *sound);
	void updateSoundPri(Sound *sound);
	void updateSoundLoop(Sound *sound);
	void setMasterVol(int volume);
	int getMasterVol() const;
	void loadSound(int soundNum, bool showErrors);
	void unloadSound(int soundNum);

	// _sf methods
	static SoundManager &sfManager();
	static void _sfTerminate();
	static int _sfDetermineGroup(const byte *soundData);
	static void _sfAddToPlayList(Sound *sound);
	static void _sfRemoveFromPlayList(Sound *sound);
	static bool _sfIsOnPlayList(Sound *sound);
	static void _sfRethinkSoundDrivers();
	static void _sfRethinkVoiceTypes();
	static void _sfUpdateVolume(Sound *sound);
	static void _sfDereferenceAll();
	static void _sfUpdatePriority(Sound *sound);
	static void _sfUpdateLoop(Sound *sound);
	static void _sfSetMasterVol(int volume);
	static void _sfExtractTrackInfo(trackInfoStruct *trackInfo, const byte *soundData, int groupNum);
	static void _sfExtractGroupMask();
	static bool _sfInstallDriver(SoundDriver *driver);
	static void _sfUnInstallDriver(SoundDriver *driver);
	static void _sfInstallPatchBank(SoundDriver *driver, const byte *bankData);
	static void _sfDoAddToPlayList(Sound *sound);
	static bool _sfDoRemoveFromPlayList(Sound *sound);
	static void _sfDoUpdateVolume(Sound *sound);
	static void _sfSoundServer();
	static void _sfProcessFading();
	static void _sfUpdateVoiceStructs();
	static void _sfUpdateVoiceStructs2();
	static void _sfUpdateCallback(void *ref);
};

class Sound: public EventHandler {
private:
	void _prime(int soundResID, bool dontQueue);
	void _unPrime();
public:
	bool _stoppedAsynchronously;
	int _soundResID;
	int _group;
	int _sndResPriority;
	int _fixedPriority;
	int _sndResLoop;
	int _fixedLoop;
	int _priority;
	int _volume;
	int _loop;
	int _pausedCount;
	int _mutedCount;
	int _hold;
	int _cueValue;
	int _fadeDest;
	int _fadeSteps;
	int _fadeTicks;
	int _fadeCounter;
	bool _stopAfterFadeFlag;
	uint32 _timer;
	uint32 _newTimeIndex;
	int _loopTimer;
	int _chProgram[SOUND_ARR_SIZE];
	int _chModulation[SOUND_ARR_SIZE];
	int _chVolume[SOUND_ARR_SIZE];
	int _chPan[SOUND_ARR_SIZE];
	int _chDamper[SOUND_ARR_SIZE];
	int _chPitchBlend[SOUND_ARR_SIZE];
	int _chVoiceType[SOUND_ARR_SIZE];
	int _chNumVoices[SOUND_ARR_SIZE];
	int _chSubPriority[SOUND_ARR_SIZE];
	int _chFlags[SOUND_ARR_SIZE];
	bool _chWork[SOUND_ARR_SIZE];
	trackInfoStruct _trackInfo;
	byte *_channelData[SOUND_ARR_SIZE];
	int _trkChannel[SOUND_ARR_SIZE];
	int _trkState[SOUND_ARR_SIZE];
	int _trkLoopState[SOUND_ARR_SIZE];
	int _trkIndex[SOUND_ARR_SIZE];
	int _trkLoopIndex[SOUND_ARR_SIZE];
	int _trkRest[SOUND_ARR_SIZE];
	int _trkLoopRest[SOUND_ARR_SIZE];

	bool _primed;
	bool _isEmpty;
	byte *_remoteReceiver;
public:
	Sound();
	~Sound();

	void synchronize(Serializer &s);
	void orientAfterRestore();

	void play(int soundResID);
	void stop();
	void prime(int soundResID);
	void unPrime();
	void go();
	void halt(void);
	bool isPlaying();
	int getSoundNum() const;
	bool isPrimed() const;
	bool isPaused() const;
	bool isMuted() const;
	void pause(bool flag);
	void mute(bool flag);
	void fade(int fadeDest, int fadeSteps, int fadeTicks, bool stopAfterFadeFlag);
	void setTimeIndex(uint32 timeIndex);
	uint32 getTimeIndex() const;
	int getCueValue() const;
	void setCueValue(int cueValue);
	void setVol(int volume);
	int getVol() const;
	void setPri(int priority);
	void setLoop(int flag);
	int getPri() const;
	int getLoop();
	void holdAt(int amount);
	void release();
	void orientAfterDriverChange();

	// _so methods
	void _soPrimeSound(bool dontQueue);
	void _soSetTimeIndex(uint timeIndex);
	bool _soServiceTracks();
	void _soPrimeChannelData();
	void _soRemoteReceive();
	void _soServiceTrackType0(int trackIndex, const byte *channelData);
	void _soUpdateDamper(VoiceTypeStruct *voiceType, int channelNum, VoiceType mode, int v0);
	void _soProc32(VoiceTypeStruct *vtStruct, const byte *channelData, int channelNum, VoiceType voiceType, int v0, int v1);
	void _soProc42(VoiceTypeStruct *vtStruct, const byte *channelData, int channelNum, VoiceType voiceType, int v0);
	void _soProc38(VoiceTypeStruct *vtStruct, int channelNum, VoiceType voiceType, int cmd, int value);
	void _soProc40(VoiceTypeStruct *vtStruct, int channelNum, int pitchBlend);
	void _soDoTrackCommand(int channelNum, int command, int value);
	bool _soDoUpdateTracks(int command, int value);
	void _soSetTrackPos(int trackIndex, int trackPos, int cueValue);

	void _soServiceTrackType1(int trackIndex, const byte *channelData);
	int _soFindSound(VoiceTypeStruct *vtStruct, int channelNum);
};

class ASound: public EventHandler {
public:
	Sound _sound;
	Action *_action;
	int _cueValue;

	ASound();
	~ASound();
	virtual void synchronize(Serializer &s);
	virtual void dispatch();

	void play(int soundNum, Action *action = NULL, int volume = 127);
	void stop();
	void prime(int soundNum, Action *action = NULL);
	void unPrime();
	void go() { _sound.go(); }
	void hault(void) { _sound.halt(); }
	bool isPlaying() { return _sound.isPlaying(); }
	int getSoundNum() const { return _sound.getSoundNum(); }
	bool isPaused() const { return _sound.isPaused(); }
	bool isMuted() const { return _sound.isMuted(); }
	void pause(bool flag) { _sound.pause(flag); }
	void mute(bool flag) { _sound.mute(flag); }
	void fade(int fadeDest, int fadeSteps, int fadeTicks, bool stopAfterFadeFlag, Action *action);
	void fadeIn() { fade(127, 5, 10, false, NULL); }
	void fadeOut(Action *action) { fade(0, 5, 10, true, action); }
	void setTimeIndex(uint32 timeIndex) { _sound.setTimeIndex(timeIndex); }
	uint32 getTimeIndex() const { return _sound.getTimeIndex(); }
	void setPri(int v) { _sound.setPri(v); }
	void setLoop(int total) { _sound.setLoop(total); }
	int getPri() const { return _sound.getPri(); }
	int getLoop() { return _sound.getLoop(); }
	void setVol(int volume) { _sound.setVol(volume); }
	int getVol() const { return _sound.getVol(); }
	void holdAt(int v) { _sound.holdAt(v); }
	void release() { _sound.release(); }
};

#define ADLIB_CHANNEL_COUNT 9

class AdlibSoundDriver: public SoundDriver, Audio::AudioStream {
private:
	GroupData _groupData;
	Audio::Mixer *_mixer;
	FM_OPL *_opl;
	Audio::SoundHandle _soundHandle;
	int _sampleRate;
	byte _portContents[256];
	const byte *_patchData;
	int _masterVolume;
	Common::Queue<RegisterValue> _queue;
	int _samplesTillCallback;
	int _samplesTillCallbackRemainder;
	int _samplesPerCallback;
	int _samplesPerCallbackRemainder;

	bool _channelVoiced[ADLIB_CHANNEL_COUNT];
	int _channelVolume[ADLIB_CHANNEL_COUNT];
	int _v4405E[ADLIB_CHANNEL_COUNT];
	int _v44067[ADLIB_CHANNEL_COUNT];
	int _v44070[ADLIB_CHANNEL_COUNT];
	int _v44079[ADLIB_CHANNEL_COUNT];
	int _v44082[ADLIB_CHANNEL_COUNT + 1];
	int _pitchBlend[ADLIB_CHANNEL_COUNT];
	int _v4409E[ADLIB_CHANNEL_COUNT];


	void write(byte reg, byte value);
	void flush();
	void updateChannelVolume(int channel);
	void setVoice(int channel);
	void clearVoice(int channel);
	void updateChannel(int channel);
	void setFrequency(int channel);
public:
	AdlibSoundDriver();
	virtual ~AdlibSoundDriver();

	virtual bool open();
	virtual void close();
	virtual bool reset();
	virtual const GroupData *getGroupData();
	virtual void installPatch(const byte *data, int size);
	virtual int setMasterVolume(int volume);
	virtual void proc32(const byte *channelData, int dataOffset, int program, int channel, int v0, int v1);
	virtual void updateVoice(int channel);
	virtual void proc38(int channel, int cmd, int value);
	virtual void setPitch(int channel, int pitchBlend);

	// AudioStream interface
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return false; }
	virtual bool endOfData() const { return false; }
	virtual int getRate() const { return _sampleRate; }

	void update(int16 *buf, int len);
};

class AdlibFxSoundDriver: public SoundDriver, Audio::AudioStream {
private:
	Common::Queue<RegisterValue> _queue;
	GroupData _groupData;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	int _sampleRate;

	int _v45062;
	int _v45066;
	int _v45068;
	int _v4506A;
	int _v4506B;
	bool _v45046;
	byte _masterVolume;
	byte _channelVolume;
	Sound *_sound;

	void write(int v);
	void flush();
	void sub_4556E();
	void write209();
	void write211();
public:
	AdlibFxSoundDriver();
	virtual ~AdlibFxSoundDriver();

	virtual bool open();
	virtual void close();
	virtual bool reset();
	virtual const GroupData *getGroupData();
	virtual void poll();
	virtual int setMasterVolume(int volume);
	virtual void proc32(const byte *channelData, int dataOffset, int program, int channel, int v0, int v1);
	virtual void updateVoice(int channel);
	virtual void proc38(int channel, int cmd, int value);
	virtual void proc42(int channel, int cmd, int value, int *v1, int *v2);

	// AudioStream interface
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return false; }
	virtual bool endOfData() const { return false; }
	virtual int getRate() const { return _sampleRate; }

	void update(int16 *buf, int len);
};


} // End of namespace tSage

#endif
