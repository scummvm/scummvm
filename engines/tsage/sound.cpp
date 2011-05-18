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
 * $URL$
 * $Id$
 *
 */

#include "common/config-manager.h"
#include "common/endian.h"
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

	for (int i = 0; i < SOUND_ARR_SIZE; ++i) {
		_field89[i] = 0;
		_groupList[i] = 0;
		_fieldE9[i] = 0;
		_field109[i] = 0;
		_voiceStructPtrs[i] = NULL;
	}

	_groupMask = 0;
	_volume = 127;
	_suspendCtr = 0;
	_disableCtr = 0;
	_suspendedCount = 0;
	_driversDetected = false;
	_needToRethink = false;
}

SoundManager::~SoundManager() {
	if (__sndmgrReady) {
		for (Common::List<Sound *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
			(*i)->stop();
		for (Common::List<SoundDriver *>::iterator i = _installedDrivers.begin(); i != _installedDrivers.end(); ) {
			SoundDriver *driver = *i;
			++i;
			delete driver;
		}
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

/**
 * Loops through all the loaded sounds, and stops any that have been flagged for stopping
 */
void SoundManager::dispatch() {
	Common::List<Sound *>::iterator i = _soundList.begin();
	while (i != _soundList.end()) {
		Sound *sound = *i;
		++i;

		// If the sound is flagged for stopping, then stop it
		if (sound->_stopFlag) {
			sound->stop();
		}
	}
}

void SoundManager::syncSounds() {
	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	bool music_mute = mute;
	bool sfx_mute = mute;

	if (!mute) {
		music_mute = ConfMan.getBool("music_mute");
		sfx_mute = ConfMan.getBool("sfx_mute");
	}

	// Get the new music and sfx volumes
	int musicVolume = music_mute ? 0 : MIN(255, ConfMan.getInt("music_volume"));
	int sfxVolume = sfx_mute ? 0 : MIN(255, ConfMan.getInt("sfx_volume"));

	warning("Set volume music=%d sfx=%d", musicVolume, sfxVolume);
	this->setMasterVol(musicVolume / 2);
}

Common::List<SoundDriverEntry> &SoundManager::buildDriverList(bool detectFlag) {
	assert(__sndmgrReady);
	_availableDrivers.clear();

	// Build up a list of available drivers. Currently we only implement an Adlib driver
	SoundDriverEntry sd;
	sd.driverNum = ADLIB_DRIVER_NUM;
	sd.status = detectFlag ? SNDSTATUS_DETECTED : SNDSTATUS_SKIPPED;
	sd.field2 = 0;
	sd.field6 = 15000;
	sd.shortDescription = "Adlib or SoundBlaster";
	sd.longDescription = "3812fm";
	_availableDrivers.push_back(sd);

	_driversDetected = true;
	return _availableDrivers;
}

void SoundManager::installConfigDrivers() {
#ifdef TSAGE_SOUND
	installDriver(ADLIB_DRIVER_NUM);
#endif
}

Common::List<SoundDriverEntry> &SoundManager::getDriverList(bool detectFlag) {
	if (detectFlag)
		return _availableDrivers;
	else
		return buildDriverList(false);
}

void SoundManager::dumpDriverList() {
	_availableDrivers.clear();
}

void SoundManager::disableSoundServer() {
	++_disableCtr;
}

void SoundManager::enableSoundServer() {
	if (_disableCtr > 0)
		--_disableCtr;
}

void SoundManager::suspendSoundServer() {
	++_suspendCtr;
}

void SoundManager::restartSoundServer() {
	if (_suspendCtr > 0)
		--_suspendCtr;
}

/**
 * Install the specified driver number
 */
void SoundManager::installDriver(int driverNum) {
	// If driver is already installed, no need to install it
	if (isInstalled(driverNum))
		return;

	// Instantiate the sound driver
	SoundDriver *driver = instantiateDriver(driverNum);
	if (!driver)
		return;

	assert((_maxVersion >= driver->_minVersion) && (_maxVersion <= driver->_maxVersion));

	// Mute any loaded sounds
	disableSoundServer();
	for (Common::List<Sound *>::iterator i = _playList.begin(); i != _playList.end(); ++i)
		(*i)->mute(true);

	// Install the driver
	if (!_sfInstallDriver(driver))
		error("Sound driver initialization failed");

	switch (driverNum) {
	case ROLAND_DRIVER_NUM:
	case ADLIB_DRIVER_NUM: {
		// Handle loading bank infomation
		byte *bankData = _resourceManager->getResource(RES_BANK, ROLAND_DRIVER_NUM, 0, true);
		if (bankData) {
			// Install the patch bank data
			_sfInstallPatchBank(driver, bankData);
			DEALLOCATE(bankData);
		} else {
			// Could not locate patch bank data, so unload the driver
			_sfUnInstallDriver(driver);

			// Unmute currently active sounds
			for (Common::List<Sound *>::iterator i = _playList.begin(); i != _playList.end(); ++i)
				(*i)->mute(false);
	
			enableSoundServer();
		}
		break;
	}
	}
}

/**
 * Instantiate a driver class for the specified driver number
 */
SoundDriver *SoundManager::instantiateDriver(int driverNum) {
	assert(driverNum == ADLIB_DRIVER_NUM);
	return new AdlibSoundDriver();
}

/**
 * Uninstall the specified driver
 */
void SoundManager::unInstallDriver(int driverNum) {
	Common::List<SoundDriver *>::const_iterator i;
	for (i = _installedDrivers.begin(); i != _installedDrivers.end(); ++i) {
		if ((*i)->_driverNum == driverNum) {
			// Found driver to remove

			// Mute any loaded sounds
			disableSoundServer();
			for (Common::List<Sound *>::iterator i = _playList.begin(); i != _playList.end(); ++i)
				(*i)->mute(true);

			// Uninstall the driver
			_sfUnInstallDriver(*i);

			// Re-orient all the loaded sounds 
			for (Common::List<Sound *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
				(*i)->orientAfterDriverChange();

			// Unmute currently active sounds
			for (Common::List<Sound *>::iterator i = _playList.begin(); i != _playList.end(); ++i)
				(*i)->mute(false);
	
			enableSoundServer();
		}
	}
}

/**
 * Returns true if a specified driver number is currently installed
 */
bool SoundManager::isInstalled(int driverNum) const {
	Common::List<SoundDriver *>::const_iterator i;
	for (i = _installedDrivers.begin(); i != _installedDrivers.end(); ++i) {
		if ((*i)->_driverNum == driverNum)
			return true;
	}

	return false;
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

void SoundManager::checkResVersion(const byte *soundData) {
	int maxVersion = READ_LE_UINT16(soundData + 4);
	int minVersion = READ_LE_UINT16(soundData + 6);

	if (_globals->_soundManager._minVersion < minVersion)
		error("Attempt to play/prime sound resource that is too new");
	if (_globals->_soundManager._minVersion > maxVersion)
		error("Attempt to play/prime sound resource that is too old");
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

void SoundManager::_sfSoundServer() {
	if (!sfManager()._disableCtr && !sfManager()._suspendCtr)
		return;

	if (sfManager()._needToRethink) {
		_sfRethinkVoiceTypes();
		sfManager()._needToRethink = false;
	} else {
		_sfDereferenceAll();
	}

	// Handle any fading if necessary
	do {
		_sfProcessFading();
	} while (sfManager()._suspendCtr > 0);
	sfManager()._suspendCtr = 0;

	// Poll all sound drivers in case they need it
	for (Common::List<SoundDriver *>::iterator i = sfManager()._installedDrivers.begin(); 
				i != sfManager()._installedDrivers.end(); ++i) {
		(*i)->poll();
	}
}

void SoundManager::_sfProcessFading() {
	//TODO
}

/*--------------------------------------------------------------------------*/

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

void SoundManager::listenerSynchronise(Serialiser &s) {
	s.validate("SoundManager");
	warning("TODO: SoundManager listenerSynchronise");
}

/*--------------------------------------------------------------------------*/

SoundManager &SoundManager::sfManager() {
	return _globals->_soundManager;
}

void SoundManager::_soSetTimeIndex(int timeIndex) {
	warning("TODO: _soSetTimeIndex");
}

int SoundManager::_sfDetermineGroup(const byte *soundData) {
	const byte *p = soundData + READ_LE_UINT16(soundData + 8);
	uint32 v;
	while ((v = READ_LE_UINT32(p)) != 0) {
		if ((v & _globals->_soundManager._groupMask) == v)
			return v;

		p += 6 + (READ_LE_UINT16(p + 4) * 4);
	}

	return 0;
}

void SoundManager::_sfAddToPlayList(Sound *sound) {
	++sfManager()._suspendCtr;
	_sfDoAddToPlayList(sound);
	sound->_stopFlag = false;
	_sfRethinkVoiceTypes();
	--sfManager()._suspendCtr;
}
	
void SoundManager::_sfRemoveFromPlayList(Sound *sound) {
	++sfManager()._suspendCtr;
	if (_sfDoRemoveFromPlayList(sound))
		_sfRethinkVoiceTypes();
	--sfManager()._suspendCtr;
}

bool SoundManager::_sfIsOnPlayList(Sound *sound) {
	++_globals->_soundManager._suspendCtr; 
	bool result = contains(_globals->_soundManager._playList, sound);
	--_globals->_soundManager._suspendCtr;

	return result;
}

void SoundManager::_sfRethinkSoundDrivers() {
	// Free any existing entries
	for (int idx = 0; idx < SOUND_ARR_SIZE; ++idx) {
		if (sfManager()._voiceStructPtrs[idx]) {
			delete sfManager()._voiceStructPtrs[idx];
			sfManager()._voiceStructPtrs[idx] = NULL;
		}
	}

	for (int idx = 0; idx < SOUND_ARR_SIZE; ++idx) {
		byte flag = 0xff;
		int total = 0;

		// Loop through the sound drivers
		for (Common::List<SoundDriver *>::iterator i = sfManager()._installedDrivers.begin();
				i != sfManager()._installedDrivers.end(); ++i) {
			// Process the group data for each sound driver
			SoundDriver *driver = *i;
			const byte *groupData = driver->_groupOffset->pData;

			while (*groupData != 0xff) {
				byte byteVal = *groupData++;

				if (byteVal == idx) {
					byte byteVal2 = *groupData++;
					if (flag == 0xff)
						flag = byteVal2;
					else {
						assert(flag == byteVal2);
					}

					if (!flag) {
						while (*groupData++ != 0xff)
							++total;
					} else {
						total += *groupData;
						groupData += 2;
					}
				} else if (*groupData++ == 0) {
					while (*groupData != 0xff)
						++groupData;
					++groupData;
				} else {
					groupData += 2;
				}
			}
		}

		if (total) {
			int dataSize = !flag ? total * 28 + 30 : total * 26 + 30;
			debugC(9, ktSageSound, "data Size = %d\n", dataSize);

			VoiceStruct *vs = new VoiceStruct();
			sfManager()._voiceStructPtrs[idx] = vs;

			if (!flag) {
				vs->_field0 = 0;
				vs->_field1 = total;
//				offset = 2;
			} else {
				vs->_field0 = 1;
				vs->_field1 = vs->_field2 = total;
//				offset = 4;
			}

			for (Common::List<SoundDriver *>::iterator i = sfManager()._installedDrivers.begin();
							i != sfManager()._installedDrivers.end(); ++i) {
				// Process the group data for each sound driver
				SoundDriver *driver = *i;
				const byte *groupData = driver->_groupOffset->pData;

				while (*groupData != 0xff) {
					byte byteVal = *groupData++;
				
					if (byteVal == idx) {
						if (!flag) {
							while ((byteVal = *groupData++) != 0xff) {
								VoiceStructEntry ve;
								ve._field1 = (byteVal & 0x80) ? 0 : 1;
								ve._driver = driver;
								ve._field4 = 0;
								ve._field6 = 0;
								ve._field8 = 0;
								ve._field9 = 0;
								ve._fieldA = 0;

								vs->_entries.push_back(ve);
							}
						} else {
							byteVal = *groupData;
							groupData += 2;

							for (int idx = 0; idx < byteVal; ++idx) {
								VoiceStructEntry ve;
								ve._field0 = idx;
								ve._driver = driver;
								ve._field4 = 0xff;
								ve._field6 = 0;
								ve._field8 = 0;
								ve._fieldA = 0;
								ve._fieldC = 0;
								ve._fieldD = 0;

								vs->_entries.push_back(ve);
							}
						}
					} else {
						if (*groupData++ != 0) {
							while (*groupData != 0xff)
								++groupData;
						}
					}
				}
			}				
		}
	}
}

void SoundManager::_sfRethinkVoiceTypes() {
	
}

void SoundManager::_sfUpdateVolume(Sound *sound) {
	_sfDereferenceAll();
	_sfDoUpdateVolume(sound);
}

void SoundManager::_sfDereferenceAll() {
	// Orignal used handles for both the driver list and voiceStructPtrs list. This method then refreshed 
	// pointer lists based on the handles. Since in ScummVM we're just using pointers directly, this
	// method doesn't need any implementation
}

void SoundManager::_sfUpdatePriority(Sound *sound) {
	++_globals->_soundManager._suspendCtr; 

	int tempPriority = (sound->_priority2 == 255) ? sound->_soundPriority : sound->_priority;
	if (sound->_priority != tempPriority) {
		sound->_priority = tempPriority;
		if (_sfDoRemoveFromPlayList(sound)) {
			_sfDoAddToPlayList(sound);
			_sfRethinkVoiceTypes();
		}
	}

	--_globals->_soundManager._suspendCtr;
}

void SoundManager::_sfUpdateLoop(Sound *sound) {
	if (sound->_loopFlag2)
		sound->_loopFlag = sound->_loop;
	else
		sound->_loopFlag = sound->_loopFlag2;
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

		p += 6 + (READ_LE_UINT16(p + 4) * 4);
	}
}

void SoundManager::_sfTerminate() {

}

void SoundManager::_sfExtractGroupMask() {
	uint32 mask = 0;
	for (int idx = 0; idx < SOUND_ARR_SIZE; ++idx)
		mask |= _globals->_soundManager._groupList[idx];

	_globals->_soundManager._groupMask = mask;
}

bool SoundManager::_sfInstallDriver(SoundDriver *driver) {
	if (!driver->open())
		return false;
	
	sfManager()._installedDrivers.push_back(driver);
	driver->_groupOffset = driver->getGroupData();
	driver->_groupMask =  READ_LE_UINT32(driver->_groupOffset);

	_sfExtractGroupMask();
	_sfRethinkSoundDrivers();
	driver->setVolume(sfManager()._volume);

	return true;
}

void SoundManager::_sfUnInstallDriver(SoundDriver *driver) {
	sfManager()._installedDrivers.remove(driver);
	delete driver;

	_sfExtractGroupMask();
	_sfRethinkSoundDrivers();
}

void SoundManager::_sfInstallPatchBank(SoundDriver *driver, const byte *bankData) {
	driver->installPatchBank(bankData);
}

/**
 * Adds the specified sound in the playing sound list, inserting in order of priority
 */
void SoundManager::_sfDoAddToPlayList(Sound *sound) {
	++sfManager()._suspendCtr;

	Common::List<Sound *>::iterator i = sfManager()._playList.begin();
	while ((i != sfManager()._playList.end()) && (sound->_priority > (*i)->_priority))
		++i;

	sfManager()._playList.insert(i, sound);
	--sfManager()._suspendCtr;
}

/**
 * Removes the specified sound from the play list
 */
bool SoundManager::_sfDoRemoveFromPlayList(Sound *sound) {
	++sfManager()._suspendCtr;

	bool result = false;
	for (Common::List<Sound *>::iterator i = sfManager()._playList.begin(); i != sfManager()._playList.end(); ++i) {
		if (*i == sound) {
			result = true;
			sfManager()._playList.erase(i);
			break;
		}
	}
	
	--sfManager()._suspendCtr;
	return result;
}

void SoundManager::_sfDoUpdateVolume(Sound *sound) {
	++_globals->_soundManager._suspendCtr; 

	for (int idx = 0; idx < 16; ++idx) {
		/*
		Sound *snd = sfManager()._voiceStructPtrs[idx];
		if (!snd)
			continue;
*/
		// TODO: More stuff
	}
	
	--_globals->_soundManager._suspendCtr;
}

/*--------------------------------------------------------------------------*/

Sound::Sound() {
	_stopFlag = false;
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
	_holdAt = 0xff;
	_cueValue = -1;
	_volume1 = -1;
	_volume3 = 0;
	_volume2 = 0;
	_volume5 = 0;
	_volume4 = 0;
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
		_stopFlag = false;
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

void Sound::fade(int volume1, int volume2, int volume3, int volume4) {
	_globals->_soundManager.suspendSoundServer();

	if (volume1 > 127)
		volume1 = 127;
	if (volume2 > 127)
		volume2 = 127;
	if (volume3 > 255)
		volume3 = 255;

	_volume1 = volume1;
	_volume2 = volume2;
	_volume3 = volume3;
	_volume5 = 0;
	_volume4 = volume4;

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
	_cueValue = -1;
}

void ASound::synchronise(Serialiser &s) {
	EventHandler::synchronise(s);
	SYNC_POINTER(_action);
	s.syncAsByte(_cueValue);
}

void ASound::dispatch() {
	EventHandler::dispatch();

	int cueValue = _sound.getCueValue();
	if (cueValue != -1) {
		_cueValue = cueValue;
		_sound.setCueValue(-1);

		if (_action)
			_action->signal();
	}

	if (_cueValue != -1) {
		if (!_sound.isPrimed()) {
			_cueValue = -1;
			if (_action) {
				_action->signal();
				_action = NULL;
			}
		}
	}
}

void ASound::play(int soundNum, Action *action, int volume) {
	_action = action;
	_cueValue = 0;
	
	setVol(volume);
	_sound.play(soundNum);
}

void ASound::stop() {
	_sound.stop();
	_action = NULL;
}

void ASound::prime(int soundNum, Action *action) {
	_action = action;
	_cueValue = 0;
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


/*--------------------------------------------------------------------------*/

SoundDriver::SoundDriver() {
	_driverNum = 0;
	_minVersion = _maxVersion = 0;
	_groupMask = 0;
}

/*--------------------------------------------------------------------------*/

const byte adlib_group_data[] = { 1, 1, 9, 1, 0xff };

AdlibSoundDriver::AdlibSoundDriver() {
	_groupData.groupMask = 1;
	_groupData.v1 = 0x46;
	_groupData.v2 = 0;
	_groupData.pData = &adlib_group_data[0];
}

} // End of namespace tSage
