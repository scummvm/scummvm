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

struct trackInfoStruct {
	int _count;
	int _rlbList[SOUND_ARR_SIZE];
	int _arr2[SOUND_ARR_SIZE];
	byte *_channelData[SOUND_ARR_SIZE];
	int field82[SOUND_ARR_SIZE];
	int field92[SOUND_ARR_SIZE];
	int fielda2[SOUND_ARR_SIZE];
	int fieldb2[SOUND_ARR_SIZE];
	int fieldf2[SOUND_ARR_SIZE];
	int field132[SOUND_ARR_SIZE];
	int field152[SOUND_ARR_SIZE];

};

enum SoundDriverStatus {SNDSTATUS_FAILED = 0, SNDSTATUS_DETECTED = 1, SNDSTATUS_SKIPPED = 2};

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

class SoundDriver {
public:
	Common::String _shortDescription, _longDescription;
	int _driverNum;
	int _minVersion, _maxVersion;
	// The following fields were originally held in separate arrays in the SoundManager class
	uint32 _groupMask;
	const GroupData *_groupOffset;
public:
	SoundDriver();

	const Common::String &getShortDriverDescription() { return _shortDescription; }
	const Common::String &getLongDriverDescription() { return _longDescription; }

	virtual bool open() { return true; }
	virtual void close() {}
	virtual const GroupData *getGroupData() = 0;
	virtual void setMasterVolume(int volume) {}
	virtual void installPatchBank(const byte *data) {}
	virtual void setVolume0(int channel, int v2, int v3, int volume) {}
	virtual void setVolume1(int channel, int v2, int v3, int volume) {}
	virtual void play(const byte *data, int size, int channel, int volume) {}
	virtual void poll() {}
};

struct VoiceStructEntry {
	int _field0;
	int _field1;
	SoundDriver *_driver;
	int _field4;
	int _field6;
	int _field8;
	int _field9;
	int _fieldA;
	int _fieldC;
	int _fieldD;
	int _fieldE;
	int _field10;
	int _field11;
	int _field12;
	int _field13;
	int _field14;
	int _field16;
	int _field18;
	int _field19;

	int _field1A;
	int _field1B;
};

enum VoiceType {VOICETYPE_0 = 0, VOICETYPE_1 = 1};

class VoiceStruct {
public:
	VoiceType _voiceType;
	int _field1;
	int _field2;

	Common::Array<VoiceStructEntry> _entries;
};

class SoundManager : public SaveListener {
private:
	SoundDriver *instantiateDriver(int driverNum);
public:
	bool __sndmgrReady;
	int _minVersion, _maxVersion;
	Common::List<Sound *> _playList;
	int _field109[SOUND_ARR_SIZE];
	uint32 _groupMask;
	int _volume;
	int _disableCtr;
	int _suspendCtr;
	int _suspendedCount;
	bool _driversDetected;
	Common::List<Sound *> _soundList;
	Common::List<SoundDriverEntry> _availableDrivers;
	Common::List<SoundDriver *> _installedDrivers;
	int _field89[SOUND_ARR_SIZE];
	uint16 _groupList[SOUND_ARR_SIZE];
	int _fieldE9[SOUND_ARR_SIZE];
	VoiceStruct *_voiceStructPtrs[SOUND_ARR_SIZE];
	bool _needToRethink;
public:
	SoundManager();
	~SoundManager();

	void dispatch();
	virtual void listenerSynchronize(Serializer &s);
	virtual void postInit();
	void syncSounds();

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
	void disableSoundServer();
	void enableSoundServer();
	void suspendSoundServer();
	void restartSoundServer();
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

	// _so methods
	static SoundManager &sfManager();
	static void _sfTerminate();
	static void _soSetTimeIndex(int timeIndex);
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
};

class Sound: public EventHandler {
private:
	void _prime(int soundNum, bool queFlag);
	void _unPrime();
	void orientAfterRestore();
public:
	int _field0;
	bool _stopFlag;
	int _soundNum;
	int _groupNum;
	int _soundPriority;
	int _priority2;
	int _loop;
	bool _loopFlag2;
	int _priority;
	int _volume;
	bool _loopFlag;
	int _pauseCtr;
	int _muteCtr;
	int _holdAt;
	int _cueValue;
	int _volume1;
	int _volume3;
	int _volume2;
	int _volume5;
	int _volume4;
	uint _timeIndex;
	int _field26;
	int _field28[SOUND_ARR_SIZE];
	int _field38[SOUND_ARR_SIZE];
	int _field48[SOUND_ARR_SIZE];
	int _field58[SOUND_ARR_SIZE];
	int _field68[SOUND_ARR_SIZE];
	int _field78[SOUND_ARR_SIZE];
	int _voiceStructIndex[SOUND_ARR_SIZE];
	int _fieldA8[SOUND_ARR_SIZE];
	int _fieldB8[SOUND_ARR_SIZE];
	int _fieldC8[SOUND_ARR_SIZE];
	int _fieldE8[SOUND_ARR_SIZE];
	trackInfoStruct _trackInfo;
	bool _primed;
	bool _isEmpty;
	byte *_field26E;
public:
	Sound();
	~Sound();

	void play(int soundNum);
	void stop();
	void prime(int soundNum);
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
	void fade(int volume1, int volume2, int volume3, int v4);
	void setTimeIndex(uint32 timeIndex);
	uint32 getTimeIndex() const;
	int getCueValue() const;
	void setCueValue(int cueValue);
	void setVol(int volume);
	int getVol() const;
	void setPri(int priority);
	void setLoop(bool flag);
	int getPri() const;
	bool getLoop();
	void holdAt(int amount);
	void release();

	void orientAfterDriverChange();
};

class ASound: public EventHandler {
public:
	Sound _sound;
	Action *_action;
	int _cueValue;

	ASound();
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
	void fadeIn() { fade(127, 5, 10, 0, NULL); }
	void fadeOut(Action *action) { fade(0, 5, 10, 1, action); }
	void fade(int v1, int v2, int v3, int v4, Action *action); 
	void setTimeIndex(uint32 timeIndex) { _sound.setTimeIndex(timeIndex); }
	uint32 getTimeIndex() const { return _sound.getTimeIndex(); }
	void setPri(int v) { _sound.setPri(v); }
	void setLoop(bool flag) { _sound.setLoop(flag); }
	int getPri() const { return _sound.getPri(); }
	bool getLoop() { return _sound.getLoop(); }
	void setVol(int volume) { _sound.setVol(volume); }
	int getVol() const { return _sound.getVol(); }
	void holdAt(int v) { _sound.holdAt(v); }
	void release() { _sound.release(); }
};

/*--------------------------------------------------------------------------
 * Adlib related classes
 *--------------------------------------------------------------------------
 */

struct AdLibRegisterSoundInstrument {
	uint8 vibrato;
	uint8 attackDecay;
	uint8 sustainRelease;
	uint8 feedbackStrength;
	uint8 keyScaling;
	uint8 outputLevel;
	uint8 freqMod;
};

struct AdLibSoundInstrument {
	byte mode;
	byte channel;
	AdLibRegisterSoundInstrument regMod;
	AdLibRegisterSoundInstrument regCar;
	byte waveSelectMod;
	byte waveSelectCar;
	byte amDepth;
};

struct VolumeEntry {
	int original;
	int adjusted;
};

class PCSoundDriver {
public:
	typedef void (*UpdateCallback)(void *);

	PCSoundDriver() { _upCb = NULL, _upRef = NULL, _musicVolume = 0, _sfxVolume = 0; }
	virtual ~PCSoundDriver() {}

	virtual void setupChannel(int channel, const byte *data, int instrument, int volume) = 0;
	virtual void setChannelFrequency(int channel, int frequency) = 0;
	virtual void stopChannel(int channel) = 0;
	virtual void playSample(const byte *data, int size, int channel, int volume) = 0;
	virtual void stopAll() = 0;
	virtual const char *getInstrumentExtension() const { return ""; }
	virtual void syncSounds();

	void setUpdateCallback(UpdateCallback upCb, void *ref);
	void resetChannel(int channel);
	void findNote(int freq, int *note, int *oct) const;
protected:
	UpdateCallback _upCb;
	void *_upRef;
	uint8 _musicVolume;
	uint8 _sfxVolume;

	static const int _noteTable[];
	static const int _noteTableCount;
};

class AdlibDriverBase : public PCSoundDriver, Audio::AudioStream {
public:
	AdlibDriverBase(Audio::Mixer *mixer);
	virtual ~AdlibDriverBase();

	// PCSoundDriver interface
	virtual void setupChannel(int channel, const byte *data, int instrument, int volume);
	virtual void stopChannel(int channel);
	virtual void stopAll();

	// AudioStream interface
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return false; }
	virtual bool endOfData() const { return false; }
	virtual int getRate() const { return _sampleRate; }

	void initCard();
	void update(int16 *buf, int len);
	void setupInstrument(const byte *data, int channel);
	void setupInstrument(const AdLibSoundInstrument *ins, int channel);
	void loadRegisterInstrument(const byte *data, AdLibRegisterSoundInstrument *reg);
	virtual void loadInstrument(const byte *data, AdLibSoundInstrument *asi) = 0;
	virtual void syncSounds();

	void adjustVolume(int channel, int volume);

protected:
	FM_OPL *_opl;
	int _sampleRate;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;

	byte _vibrato;
	VolumeEntry _channelsVolumeTable[10];
	AdLibSoundInstrument _instrumentsTable[10];

	static const int _freqTable[];
	static const int _freqTableCount;
	static const int _operatorsTable[];
	static const int _operatorsTableCount;
	static const int _voiceOperatorsTable[];
	static const int _voiceOperatorsTableCount;
};

class AdlibSoundDriverADL : public AdlibDriverBase {
public:
	AdlibSoundDriverADL(Audio::Mixer *mixer) : AdlibDriverBase(mixer) {}
	virtual const char *getInstrumentExtension() const { return ".ADL"; }
	virtual void loadInstrument(const byte *data, AdLibSoundInstrument *asi);
	virtual void setChannelFrequency(int channel, int frequency);
	virtual void playSample(const byte *data, int size, int channel, int volume);
};

class PCSoundFxPlayer {
private:
	enum {
		NUM_INSTRUMENTS = 15,
		NUM_CHANNELS = 4
	};

	void update();
	void handleEvents();
	void handlePattern(int channel, const byte *patternData);

	char _musicName[33];
	bool _playing;
	bool _songPlayed;
	int _currentPos;
	int _currentOrder;
	int _numOrders;
	int _eventsDelay;
	bool _looping;
	int _fadeOutCounter;
	int _updateTicksCounter;
	int _instrumentsChannelTable[NUM_CHANNELS];
	byte *_sfxData;
	byte *_instrumentsData[NUM_INSTRUMENTS];
	PCSoundDriver *_driver;

public:
	PCSoundFxPlayer(PCSoundDriver *driver);
	~PCSoundFxPlayer();

	bool load(const char *song);
	void play();
	void stop();
	void unload();
	void fadeOut();
	void doSync(Common::Serializer &s);

	static void updateCallback(void *ref);

	bool songLoaded() const { return _sfxData != NULL; }
	bool songPlayed() const { return _songPlayed; }
	bool playing() const { return _playing; }
	uint8 numOrders() const { assert(_sfxData); return _sfxData[470]; }
	void setNumOrders(uint8 v) { assert(_sfxData); _sfxData[470] = v; }
	void setPattern(int offset, uint8 value) { assert(_sfxData); _sfxData[472 + offset] = value; }
	const char *musicName() { return _musicName; }

	// Note: Original game never actually uses looping variable. Songs are hardcoded to loop
	bool looping() const { return _looping; }
	void setLooping(bool v) { _looping = v; }
};

class AdlibSoundDriver: public SoundDriver {
private:
	GroupData _groupData;
	Audio::Mixer *_mixer;
	PCSoundDriver *_soundDriver;
	PCSoundFxPlayer *_player;
public:
	AdlibSoundDriver();

	virtual void setVolume(int volume) {}
	virtual void installPatchBank(const byte *data) {}
	virtual const GroupData *getGroupData() { return &_groupData; }
	virtual void play(const byte *data, int size, int channel, int volume) { 
		_soundDriver->playSample(data, size, channel, volume); 
	}
};

} // End of namespace tSage

#endif
