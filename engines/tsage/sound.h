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
#include "common/list.h"
#include "tsage/saveload.h"
#include "tsage/core.h"

namespace tSage {

class Sound;

#define SOUND_ARR_SIZE 16
#define ROLAND_DRIVER_NUM 2
#define ADLIB_DRIVER_NUM 3

struct trackInfoStruct {
	int count;
	int rlbList[SOUND_ARR_SIZE];
	int arr2[SOUND_ARR_SIZE];
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

class SoundDriver {
public:
	Common::String _shortDescription, _longDescription;
	int _driverNum;
	int _minVersion, _maxVersion;
public:
	const Common::String &getShortDriverDescription() { return _shortDescription; }
	const Common::String &getLongDriverDescription() { return _longDescription; }

	virtual void setVolume(int volume) = 0;
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
	int _field153;
	bool _driversDetected;
	Common::List<Sound *> _soundList;
	Common::List<SoundDriverEntry> _availableDrivers;
	Common::List<SoundDriver *> _installedDrivers;
	int _field89[SOUND_ARR_SIZE];
	uint16 _groupList[SOUND_ARR_SIZE];
	int _fieldE9[SOUND_ARR_SIZE];
	Sound *_voiceStructPtrs[SOUND_ARR_SIZE];
public:
	SoundManager();
	~SoundManager();

	void dispatch() {}
	virtual void listenerSynchronize(Serializer &s);
	virtual void postInit();

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
	static void _sfInstallPatchBank(const byte *bankData);
	static void _sfDoAddToPlayList(Sound *sound);
	static bool _sfDoRemoveFromPlayList(Sound *sound);
	static void _sfDoUpdateVolume(Sound *sound);
};

class Sound: public EventHandler {
private:
	void _prime(int soundNum, bool queFlag);
	void _unPrime();
	void orientAfterRestore();
public:
	int _field0;
	int _field6;
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
	int _field1F;
	int _volume2;
	int _field21;
	int _field22;
	uint _timeIndex;
	int _field26;
	int _field28[SOUND_ARR_SIZE];
	int _field38[SOUND_ARR_SIZE];
	int _field48[SOUND_ARR_SIZE];
	int _field58[SOUND_ARR_SIZE];
	int _field68[SOUND_ARR_SIZE];
	int _field78[SOUND_ARR_SIZE];
	int _field98[SOUND_ARR_SIZE];
	int _fieldA8[SOUND_ARR_SIZE];
	int _fieldB8[SOUND_ARR_SIZE];
	int _fieldC8[SOUND_ARR_SIZE];
	int _fieldE8[SOUND_ARR_SIZE];
	trackInfoStruct _trackInfo;
	byte *_handleList[75];
	int _field266;
	int _field268;
	bool _primed;
	int _field26C;
	byte *_field26E;
public:
	Sound();

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
	void fade(int v1, int v2, int v3, int v4);
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
	bool _cueFlag;

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

class AdlibSoundDriver: public SoundDriver {
public:
	void setVolume(int volume) {}
};

} // End of namespace tSage

#endif
