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

#include "tsage/core.h"
#include "tsage/globals.h"
#include "tsage/debugger.h"
#include "tsage/graphics.h"
#include "tsage/tsage.h"

namespace tSage {

SoundManager::SoundManager() {
	__sndmgrReady = false;
	_minVersion = 0x102;
	_maxVersion = 0x10A;

	for (int i = 0; i < 16; ++i)
		_field109[i] = 0;

	_groupMask = 0;
	_volume = 127;
	_suspendCtr = 0;
	_disableCtr = 0;
	_field153 = 0;
	_field16D = 0;
}

SoundManager::~SoundManager() {
	if (__sndmgrReady) {
		for (Common::List<Sound *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
			(*i)->stop();
		for (Common::List<SoundDriver *>::iterator i = _installedDrivers.begin(); i != _installedDrivers.end(); ++i)
			unInstallDriver(*i);
		_sfTerminate();
	}
}

void SoundManager::postInit() {
	if (!__sndmgrReady) {
		_saver->addSaveNotifier(&SoundManager::saveNotifier);
		_saver->addLoadNotifier(&SoundManager::loadNotifier);
		_saver->addListener(this);
		__sndmgrReady = true;
	}
}

void SoundManager::saveNotifier(bool postFlag) {
	_globals->_soundManager.saveNotifierProc(postFlag);
}

void SoundManager::saveNotifierProc(bool postFlag) {
	warning("TODO: SoundManager::saveNotifierProc");
}

void SoundManager::loadNotifier(bool postFlag) {
	_globals->_soundManager.loadNotifierProc(postFlag);
}

void SoundManager::loadNotifierProc(bool postFlag) {
	warning("TODO: SoundManager::loadNotifierProc");
}

void SoundManager::listenerSynchronize(Serializer &s) {
	s.validate("SoundManager");
	warning("TODO: SoundManager listenerSynchronize");
}

void SoundManager::checkResVersion(const byte *soundData) {
	int minVersion = READ_LE_UINT16(soundData + 4);
	int maxVersion = READ_LE_UINT16(soundData + 6);

	if (_globals->_soundManager._minVersion >= maxVersion)
		error("Attempt to play/prime sound resource that is too new");
	if (_globals->_soundManager._minVersion > minVersion)
		error("Attempt to play/prime sound resource that is too old");
}

/*--------------------------------------------------------------------------*/

void SoundManager::suspendSoundServer() {
	++_globals->_soundManager._suspendCtr;
}


void SoundManager::restartSoundServer() {
	if (_globals->_soundManager._suspendCtr > 0)
		--_globals->_soundManager._suspendCtr;
}

void SoundManager::unInstallDriver(SoundDriver *driver) {
	
}

Common::List<SoundDriverEntry> &SoundManager::buildDriverList(bool flag) {
	assert(__sndmgrReady);
	_driverList.clear();

	warning("TODO: SoundManager::buildDriverList");
	return _driverList;
}

Common::List<SoundDriverEntry> &SoundManager::getDriverList(bool flag) {
	if (flag)
		return _driverList;
	else
		return buildDriverList(false);
}

void SoundManager::dumpDriverList() {
	_driverList.clear();
}

void SoundManager::setMasterVol(int volume) {
	_sfSetMasterVol(volume);
}

int SoundManager::getMasterVol() const {
	return _volume;
}

void SoundManager::loadSound(int soundNum, bool showErrors) {
	// This method preloaded the data associated with a given sound, so is now redundant
}

void SoundManager::unloadSound(int soundNum) {
	// This method signalled the resource manager to unload the data for a sound, and is now redundant
}

int SoundManager::determineGroup(const byte *soundData) {
	return _sfDetermineGroup(soundData);
}

int SoundManager::extractPriority(const byte *soundData) {
	return READ_LE_UINT16(soundData + 12);
}

int SoundManager::extractLoop(const byte *soundData) {
	return READ_LE_UINT16(soundData + 14);
}

void SoundManager::extractTrackInfo(trackInfoStruct *trackInfo, const byte *soundData, int groupNum) {
	_sfExtractTrackInfo(trackInfo, soundData, groupNum);
}

void SoundManager::addToSoundList(Sound *sound) {
	if (!contains(_soundList, sound))
		_soundList.push_back(sound);
}

void SoundManager::removeFromSoundList(Sound *sound) {
	_soundList.remove(sound);
}

void SoundManager::addToPlayList(Sound *sound) {
	_sfAddToPlayList(sound);
}

void SoundManager::removeFromPlayList(Sound *sound) {
	_sfRemoveFromPlayList(sound);
}

bool SoundManager::isOnPlayList(Sound *sound) {
	return _sfIsOnPlayList(sound);
}

void SoundManager::updateSoundVol(Sound *sound) {
	_sfUpdateVolume(sound);
}

void SoundManager::updateSoundPri(Sound *sound) {
	_sfUpdatePriority(sound);
}

void SoundManager::updateSoundLoop(Sound *sound) {
	_sfUpdateLoop(sound);
}

void SoundManager::rethinkVoiceTypes() {
	_sfRethinkVoiceTypes();
}

/*--------------------------------------------------------------------------*/

void SoundManager::_soSetTimeIndex(int timeIndex) {
	warning("TODO: _soSetTimeIndex");
}

int SoundManager::_sfDetermineGroup(const byte *soundData) {
	const byte *p = soundData + READ_LE_UINT16(soundData + 8);
	uint32 v;
	while ((v = READ_LE_UINT32(p)) != 0) {
		if ((v & _globals->_soundManager._groupMask) == v)
			return v;

		p = soundData + 6 + (READ_LE_UINT16(p + 4) * 4);
	}

	return 0;
}

void SoundManager::_sfAddToPlayList(Sound*soundData) {
	
}
	
void SoundManager::_sfRemoveFromPlayList(Sound *sound) {

}

bool SoundManager::_sfIsOnPlayList(Sound *sound) {
	++_globals->_soundManager._suspendCtr; 
	bool result = contains(_globals->_soundManager._playList, sound);
	--_globals->_soundManager._suspendCtr;

	return result;
}

void SoundManager::_sfRethinkVoiceTypes() {
	
}

void SoundManager::_sfUpdateVolume(Sound *sound) {
	_sfDereferenceAll();
	sub_233EE(sound);
}

void SoundManager::_sfDereferenceAll() {

}

void SoundManager::sub_233EE(Sound *sound) {

}

void SoundManager::_sfUpdatePriority(Sound *sound) {

}

void SoundManager::_sfUpdateLoop(Sound *sound) {

}

void SoundManager::_sfSetMasterVol(int volume) {
	if (volume > 127)
		volume = 127;

	if (volume != _globals->_soundManager._volume) {
		_globals->_soundManager._volume = volume;

		for (Common::List<SoundDriver *>::iterator i = _globals->_soundManager._installedDrivers.begin(); 
				i != _globals->_soundManager._installedDrivers.end(); ++i) {
			(*i)->setVolume(volume);
		}
	}
}

void SoundManager::_sfExtractTrackInfo(trackInfoStruct *trackInfo, const byte *soundData, int groupNum) {
	trackInfo->count = 0;

	const byte *p = soundData + READ_LE_UINT16(soundData + 8);
	uint32 v;
	while ((v = READ_LE_UINT32(p)) != 0) {
		while ((v == 0x80000000) || (v == (uint)groupNum)) {
			int count = READ_LE_UINT16(p + 4);
			p += 6;

			for (int idx = 0; idx < count; ++idx) {
				if (trackInfo->count == 16) {
					trackInfo->count = -1;
					return;
				}

				trackInfo->rlbList[trackInfo->count] = READ_LE_UINT16(p);
				trackInfo->arr2[trackInfo->count] = READ_LE_UINT16(p + 2);
				++trackInfo->count;
				p += 4;
			} 
		}

		p = soundData + 6 + (READ_LE_UINT16(p + 4) * 4);
	}
}

void SoundManager::_sfTerminate() {

}

/*--------------------------------------------------------------------------*/

Sound::Sound() {
	_field6 = 0;
	_soundNum = 0;
	_groupNum = 0;
	_soundPriority = 0;
	_priority2 = -1;
	_loop = true;
	_loopFlag2 = true;
	_priority = 0;
	_volume = 127;
	_loopFlag = false;
	_pauseCtr = 0;
	_muteCtr = 0;
	_holdAt = false;
	_cueValue = -1;
	_volume1 = -1;
	_field1F = 0;
	_volume2 = 0;
	_field21 = 0;
	_field22 = 0;
	_timeIndex = 0;
	_field26 = 0;
	_trackInfo.count = 0;
	_primed = false;
	_field26C = 0;
	_field26E = NULL;
}

void Sound::play(int soundNum) {
	prime(soundNum);
	_globals->_soundManager.addToPlayList(this);
}

void Sound::stop() {
	_globals->_soundManager.removeFromPlayList(this);
	_unPrime();
}

void Sound::prime(int soundNum) {
	if (_soundNum != -1) {
		stop();
		_prime(soundNum, false);
	}
}

void Sound::unPrime() {
	stop();
}

void Sound::_prime(int soundNum, bool queFlag) {
	if (_primed)
		unPrime();

	if (_soundNum != -1) {
		// Sound number specified
		_field26E = NULL;
		byte *soundData = _resourceManager->getResource(RES_SOUND, soundNum, 0);
		_globals->_soundManager.checkResVersion(soundData);
		_groupNum = _globals->_soundManager.determineGroup(soundData);
		_soundPriority = _globals->_soundManager.extractPriority(soundData);
		_loop = _globals->_soundManager.extractLoop(soundData);
		_globals->_soundManager.extractTrackInfo(&_trackInfo, soundData, _groupNum);

		for (int idx = 0; idx < _trackInfo.count; ++idx) {
			_handleList[idx] = _resourceManager->getResource(RES_SOUND, soundNum, _trackInfo.rlbList[idx]);
		}

		DEALLOCATE(soundData);
	} else {
		// No sound specified
		_groupNum = 0;
		_soundPriority = 0;
		_loop = 0;
		_trackInfo.count = 0;
		_handleList[0] = ALLOCATE(200);
		_field26E = ALLOCATE(200);
	}

	if (queFlag)
		_globals->_soundManager.addToSoundList(this);

	_primed = true;
}

void Sound::_unPrime() {
	if (_primed) {
		if (_field26C) {
			DEALLOCATE(_handleList[0]);
			DEALLOCATE(_field26E);
			_field26E = NULL;
		} else {
			for (int idx = 0; idx < _trackInfo.count; ++idx) {
				DEALLOCATE(_handleList[idx]);
			}
		}

		_trackInfo.count = 0;
		_globals->_soundManager.removeFromSoundList(this);

		_primed = false;
		_field6 = 0;
	}
}

void Sound::orientAfterDriverChange() {
	if (!_field26C) {
		int timeIndex = getTimeIndex();

		for (int idx = 0; idx < _trackInfo.count; ++idx)
			DEALLOCATE(_handleList[idx]);
		
		_trackInfo.count = 0;
		_primed = false;
		_prime(_soundNum, true);
		setTimeIndex(timeIndex);
	}
}

void Sound::orientAfterRestore() {
	if (_field26C) {
		int timeIndex = getTimeIndex();
		_primed = false;
		_prime(_soundNum, true);
		setTimeIndex(timeIndex);
	}
}

void Sound::go() {
	if (!_primed)
		error("Attempt to execute Sound::go() on an unprimed Sound");

	_globals->_soundManager.addToPlayList(this);
}

void Sound::halt(void) {
	_globals->_soundManager.removeFromPlayList(this);
}

int Sound::getSoundNum() const {
	return _soundNum;
}

bool Sound::isPlaying() {
	return _globals->_soundManager.isOnPlayList(this);
}

bool Sound::isPrimed() const {
	return _primed;
}

bool Sound::isPaused() const {
	return _pauseCtr != 0;
}

bool Sound::isMuted() const {
	return _muteCtr != 0;
}

void Sound::pause(bool flag) {
	_globals->_soundManager.suspendSoundServer();

	if (flag)
		++_pauseCtr;
	else if (_pauseCtr > 0)
		--_pauseCtr;

	_globals->_soundManager.rethinkVoiceTypes();
	_globals->_soundManager.restartSoundServer();
}

void Sound::mute(bool flag) {
	_globals->_soundManager.suspendSoundServer();

	if (flag)
		++_muteCtr;
	else if (_muteCtr > 0)
		--_muteCtr;

	_globals->_soundManager.rethinkVoiceTypes();
	_globals->_soundManager.restartSoundServer();
}

void Sound::fade(int volume1, int volume2, int v3, int v4) {
	_globals->_soundManager.suspendSoundServer();

	if (volume1 > 127)
		volume1 = 127;
	if (volume2 > 127)
		volume2 = 127;
	if (v3 > 255)
		v3 = 255;

	_volume1 = volume1;
	_volume2 = volume2;
	_field1F = v3;
	_field21 = 0;
	_field22 = v4;

	_globals->_soundManager.restartSoundServer();
}

void Sound::setTimeIndex(uint32 timeIndex) {
	if (_primed) {
		mute(true);
		SoundManager::_soSetTimeIndex(timeIndex);
		mute(false);
	}
}

uint32 Sound::getTimeIndex() const {
	return _timeIndex;
}

int Sound::getCueValue() const {
	return _cueValue;
}

void Sound::setCueValue(int cueValue) {
	_cueValue = cueValue;
}

void Sound::setVol(int volume) {
	if (volume > 127)
		volume = 127;

	if (_volume != volume) {
		_volume = volume;
		if (isPlaying())
			_globals->_soundManager.updateSoundVol(this);
	}
}

int Sound::getVol() const {
	return _volume;
}

void Sound::setPri(int priority) {
	if (priority > 127)
		priority = 127;
	_priority2 = priority;
	_globals->_soundManager.updateSoundPri(this);
}

void Sound::setLoop(bool flag) {
	_loopFlag2 = flag;
	_globals->_soundManager.updateSoundLoop(this);
}

int Sound::getPri() const {
	return _priority;
}

bool Sound::getLoop() {
	return _loopFlag;
}

void Sound::holdAt(int amount) {
	if (amount > 127)
		amount = 127;
	_holdAt = amount;
}

void Sound::release() {
	_holdAt = -1;
}

/*--------------------------------------------------------------------------*/

ASound::ASound(): EventHandler() {
	_action = NULL;
	_cueFlag = false;
}

void ASound::synchronize(Serializer &s) {
	EventHandler::synchronize(s);
	SYNC_POINTER(_action);
	s.syncAsByte(_cueFlag);
}

void ASound::dispatch() {
	EventHandler::dispatch();

	if (!_sound.getCueValue()) {
		_cueFlag = false;
		_sound.setCueValue(1);

		if (_action)
			_action->signal();
	}

	if (!_cueFlag) {
		if (!_sound.isPrimed()) {
			_cueFlag = true;
			if (_action) {
				_action->signal();
				_action = NULL;
			}
		}
	}
}

void ASound::play(int soundNum, Action *action, int volume) {
	_action = action;
	_cueFlag = false;
	
	setVol(volume);
	_sound.play(soundNum);
}

void ASound::stop() {
	_sound.stop();
	_action = NULL;
}

void ASound::prime(int soundNum, Action *action) {
	_action = action;
	_cueFlag = false;
	_sound.prime(soundNum);
}

void ASound::unPrime() {
	_sound.unPrime();
	_action = NULL;
}

void ASound::fade(int v1, int v2, int v3, int v4, Action *action) {
	if (action)
		_action = action;

	_sound.fade(v1, v2, v3, v4);
}

} // End of namespace tSage
