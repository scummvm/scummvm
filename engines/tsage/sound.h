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

struct trackInfoStruct {
	int count;
	int rlbList[32];
	byte *handleList[75];
};

class SoundManager : public SaveListener {
public:
	int _minVersion, _maxVersion;
	Common::List<Sound *> _playList;
	void *driverList2[16];
	int _field89[16];
	int _fieldA9[16];
	int _fieldE9[16];
	int _field109[16];
	uint32 _groupMask;
	int volume;
	int _disableCtr;
	int _suspendCtr;
	int _field153;
	Common::List<Sound *> _soundList;
	Common::List<void *> _driverList;
	int _field16D;
public:
	void dispatch() {}
	virtual void listenerSynchronize(Serializer &s);
	virtual void postInit();

	void proc2() {}
	static void saveNotifier(bool postFlag);
	void saveNotifierProc(bool postFlag);
	static void loadNotifier(bool postFlag);
	void loadNotifierProc(bool postFlag);

	void checkResVersion(const byte *soundData);
	int determineGroup(const byte *soundData);
	int extractPriority(const byte *soundData);
	int extractLoop(const byte *soundData);
	void addToSoundList(Sound *sound);
	void removeFromSoundList(Sound *sound);
	void addToPlayList(Sound *sound);
	void removeFromPlayList(Sound *sound);
	bool isOnPlayList(Sound *sound);
	void extractTrackInfo(trackInfoStruct *data, const byte *soundData, int groupNum);
	void suspendSoundServer();
	void rethinkVoiceTypes();
	void restartSoundServer();
	void updateSoundVol(Sound *sound);
	void updateSoundPri(Sound *sound);
	void updateSoundLoop(Sound *sound);

	// _so methods
	static void _soSetTimeIndex(int timeIndex);
	static int _sfDetermineGroup(const byte *soundData);
	static void _sfAddToPlayList(Sound *sound);
	static void _sfRemoveFromPlayList(Sound *sound);
	static bool _sfIsOnPlayList(Sound *sound);
	static void _sfRethinkVoiceTypes();
	static void _sfUpdateVolume(Sound *sound);
	static void _sfDereferenceAll();
	static void sub_233EE(Sound *sound);
	static void _sfUpdatePriority(Sound *sound);
	static void _sfUpdateLoop(Sound *sound);

};

#define SOUND_ARR_SIZE 16

class Sound: public EventHandler {
private:
	void _prime(int soundNum, bool queFlag);
	void _unPrime();
	void orientAfterDriverChange();
	void orientAfterRestore();
public:
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

} // End of namespace tSage

#endif
