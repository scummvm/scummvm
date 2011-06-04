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

#include "common/config-manager.h"
#include "tsage/core.h"
#include "tsage/globals.h"
#include "tsage/debugger.h"
#include "tsage/graphics.h"
#include "tsage/tsage.h"

namespace tSage {

static SoundManager *_soundManager = NULL;

/*--------------------------------------------------------------------------*/

SoundManager::SoundManager() {
	_soundManager = this;
	__sndmgrReady = false;
	_ourSndResVersion = 0x102;
	_ourDrvResVersion = 0x10A;

	for (int i = 0; i < SOUND_ARR_SIZE; ++i)
		_voiceStructPtrs[i] = NULL;

	_groupsAvail = 0;
	_masterVol = 127;
	_serverSuspendedCount = 0;
	_serverDisabledCount = 0;
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

	_soundManager = NULL;
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
		if (sound->_stoppedAsynchronously) {
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
	installDriver(ADLIB_DRIVER_NUM);
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
	++_serverDisabledCount;
}

void SoundManager::enableSoundServer() {
	if (_serverDisabledCount > 0)
		--_serverDisabledCount;
}

void SoundManager::suspendSoundServer() {
	++_serverSuspendedCount;
}

void SoundManager::restartSoundServer() {
	if (_serverSuspendedCount > 0)
		--_serverSuspendedCount;
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

	assert((_ourDrvResVersion >= driver->_minVersion) && (_ourDrvResVersion <= driver->_maxVersion));

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
		if ((*i)->_driverResID == driverNum) {
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
		if ((*i)->_driverResID == driverNum)
			return true;
	}

	return false;
}

void SoundManager::setMasterVol(int volume) {
	_sfSetMasterVol(volume);
}

int SoundManager::getMasterVol() const {
	return _masterVol;
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

	if (_soundManager->_ourSndResVersion < minVersion)
		error("Attempt to play/prime sound resource that is too new");
	if (_soundManager->_ourSndResVersion > maxVersion)
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
	if (!sfManager()._serverDisabledCount && !sfManager()._serverSuspendedCount)
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
	} while (sfManager()._serverSuspendedCount > 0);
	sfManager()._serverSuspendedCount = 0;

	// Poll all sound drivers in case they need it
	for (Common::List<SoundDriver *>::iterator i = sfManager()._installedDrivers.begin(); 
				i != sfManager()._installedDrivers.end(); ++i) {
		(*i)->poll();
	}
}

void SoundManager::_sfProcessFading() {
	//TODO
}

void SoundManager::_sfUpdateVoiceStructs() {
	for (int voiceIndex = 0; voiceIndex < SOUND_ARR_SIZE; ++voiceIndex) {
		VoiceStruct *vs = sfManager()._voiceStructPtrs[voiceIndex];

		for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
			VoiceStructEntry &vse = vs->_entries[idx];

			if (vs->_voiceType == VOICETYPE_0) {
				vse._field4 = vse._fieldC;
				vse._field6 = vse._fieldE;
				vse._field8 = vse._field10;
				vse._field9 = vse._field11;
				vse._fieldA = vse._field12;
			} else {
				vse._field8 = vse._fieldE;
				vse._fieldA = vse._field10;
				vse._fieldC = vse._field12;
				vse._fieldD = vse._field13;
			}
		}
	}
}

/*--------------------------------------------------------------------------*/

void SoundManager::saveNotifier(bool postFlag) {
	_soundManager->saveNotifierProc(postFlag);
}

void SoundManager::saveNotifierProc(bool postFlag) {
	warning("TODO: SoundManager::saveNotifierProc");
}

void SoundManager::loadNotifier(bool postFlag) {
	_soundManager->loadNotifierProc(postFlag);
}

void SoundManager::loadNotifierProc(bool postFlag) {
	warning("TODO: SoundManager::loadNotifierProc");
}

void SoundManager::listenerSynchronize(Serializer &s) {
	s.validate("SoundManager");
	warning("TODO: SoundManager listenerSynchronise");
}

/*--------------------------------------------------------------------------*/

SoundManager &SoundManager::sfManager() {
	return *_soundManager;
}

void SoundManager::_soSetTimeIndex(int timeIndex) {
	warning("TODO: _soSetTimeIndex");
}

int SoundManager::_sfDetermineGroup(const byte *soundData) {
	const byte *p = soundData + READ_LE_UINT16(soundData + 8);
	uint32 v;
	while ((v = READ_LE_UINT32(p)) != 0) {
		if ((v & _soundManager->_groupsAvail) == v)
			return v;

		p += 6 + (READ_LE_UINT16(p + 4) * 4);
	}

	return 0;
}

void SoundManager::_sfAddToPlayList(Sound *sound) {
	++sfManager()._serverSuspendedCount;
	_sfDoAddToPlayList(sound);
	sound->_stoppedAsynchronously = false;
	_sfRethinkVoiceTypes();
	--sfManager()._serverSuspendedCount;
}
	
void SoundManager::_sfRemoveFromPlayList(Sound *sound) {
	++sfManager()._serverSuspendedCount;
	if (_sfDoRemoveFromPlayList(sound))
		_sfRethinkVoiceTypes();
	--sfManager()._serverSuspendedCount;
}

bool SoundManager::_sfIsOnPlayList(Sound *sound) {
	++_soundManager->_serverSuspendedCount; 
	bool result = contains(_soundManager->_playList, sound);
	--_soundManager->_serverSuspendedCount;

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
				vs->_voiceType = VOICETYPE_0;
				vs->_field1 = total;
			} else {
				vs->_voiceType = VOICETYPE_1;
				vs->_field1 = vs->_field2 = total;
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
	++sfManager()._serverSuspendedCount;
	_sfDereferenceAll();


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
	++_soundManager->_serverSuspendedCount; 

	int tempPriority = (sound->_fixedPriority == 255) ? sound->_sndResPriority : sound->_priority;
	if (sound->_priority != tempPriority) {
		sound->_priority = tempPriority;
		if (_sfDoRemoveFromPlayList(sound)) {
			_sfDoAddToPlayList(sound);
			_sfRethinkVoiceTypes();
		}
	}

	--_soundManager->_serverSuspendedCount;
}

void SoundManager::_sfUpdateLoop(Sound *sound) {
	if (sound->_fixedLoop)
		sound->_loop = sound->_sndResLoop;
	else
		sound->_loop = sound->_fixedLoop;
}

void SoundManager::_sfSetMasterVol(int volume) {
	if (volume > 127)
		volume = 127;

	if (volume != _soundManager->_masterVol) {
		_soundManager->_masterVol = volume;

		for (Common::List<SoundDriver *>::iterator i = _soundManager->_installedDrivers.begin(); 
				i != _soundManager->_installedDrivers.end(); ++i) {
			(*i)->setMasterVolume(volume);
		}
	}
}

void SoundManager::_sfExtractTrackInfo(trackInfoStruct *trackInfo, const byte *soundData, int groupNum) {
	trackInfo->_numTracks = 0;

	const byte *p = soundData + READ_LE_UINT16(soundData + 8);
	uint32 v;
	while ((v = READ_LE_UINT32(p)) != 0) {
		if ((v == 0x80000000) || (v == (uint)groupNum)) {
			// Found group to process
			int count = READ_LE_UINT16(p + 4);
			p += 6;

			for (int idx = 0; idx < count; ++idx) {
				if (trackInfo->_numTracks == 16) {
					trackInfo->_numTracks = -1;
					return;
				}

				trackInfo->_chunks[trackInfo->_numTracks] = READ_LE_UINT16(p);
				trackInfo->_voiceTypes[trackInfo->_numTracks] = READ_LE_UINT16(p + 2);
				++trackInfo->_numTracks;
				p += 4;
			}
		} else {
			// Not correct group, so move to next one
			p += 6 + (READ_LE_UINT16(p + 4) * 4);
		}
	}
}

void SoundManager::_sfTerminate() {

}

void SoundManager::_sfExtractGroupMask() {
	uint32 mask = 0;

	for (Common::List<SoundDriver *>::iterator i = sfManager()._installedDrivers.begin(); 
				i != sfManager()._installedDrivers.end(); ++i) 
		mask |= (*i)->_groupMask;

	_soundManager->_groupsAvail = mask;
}

bool SoundManager::_sfInstallDriver(SoundDriver *driver) {
	if (!driver->open())
		return false;
	
	sfManager()._installedDrivers.push_back(driver);
	driver->_groupOffset = driver->getGroupData();
	driver->_groupMask =  READ_LE_UINT32(driver->_groupOffset);

	_sfExtractGroupMask();
	_sfRethinkSoundDrivers();
	driver->setMasterVolume(sfManager()._masterVol);

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
	++sfManager()._serverSuspendedCount;

	Common::List<Sound *>::iterator i = sfManager()._playList.begin();
	while ((i != sfManager()._playList.end()) && (sound->_priority > (*i)->_priority))
		++i;

	sfManager()._playList.insert(i, sound);
	--sfManager()._serverSuspendedCount;
}

/**
 * Removes the specified sound from the play list
 */
bool SoundManager::_sfDoRemoveFromPlayList(Sound *sound) {
	++sfManager()._serverSuspendedCount;

	bool result = false;
	for (Common::List<Sound *>::iterator i = sfManager()._playList.begin(); i != sfManager()._playList.end(); ++i) {
		if (*i == sound) {
			result = true;
			sfManager()._playList.erase(i);
			break;
		}
	}
	
	--sfManager()._serverSuspendedCount;
	return result;
}

void SoundManager::_sfDoUpdateVolume(Sound *sound) {
	++_soundManager->_serverSuspendedCount; 

	for (int voiceIndex = 0; voiceIndex < SOUND_ARR_SIZE; ++voiceIndex) {
		VoiceStruct *vs = sfManager()._voiceStructPtrs[voiceIndex];
		if (!vs)
			continue;

		for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
			VoiceStructEntry &vse = vs->_entries[idx];
			SoundDriver *driver = vse._driver;

			if (vs->_voiceType == VOICETYPE_0) {
				if (!vse._field4 && !vse._field6) {
					int vol = sound->_volume * sound->_chVolume[vse._field8] / 127;
					driver->setVolume0(voiceIndex, vse._field0, 7, vol);
				}
			} else {
				if (!vse._field8 && !vse._fieldA) {
					int vol = sound->_volume * sound->_chVolume[vse._fieldC] / 127;
					driver->setVolume1(voiceIndex, vse._field0, 7, vol);
				}
			}
		}
	}
	
	--_soundManager->_serverSuspendedCount;
}

/*--------------------------------------------------------------------------*/

Sound::Sound() {
	_stoppedAsynchronously = false;
	_soundResID = 0;
	_group = 0;
	_sndResPriority = 0;
	_fixedPriority = -1;
	_sndResLoop = true;
	_fixedLoop = true;
	_priority = 0;
	_volume = 127;
	_loop = false;
	_pausedCount = 0;
	_mutedCount = 0;
	_hold = 0xff;
	_cueValue = -1;
	_fadeDest = -1;
	_fadeSteps = 0;
	_fadeTicks = 0;
	_fadeCounter = 0;
	_stopAfterFadeFlag = false;
	_timer = 0;
	_loopTimer = 0;
	_trackInfo._numTracks = 0;
	_primed = false;
	_isEmpty = false;
	_remoteReceiver = NULL;
}

Sound::~Sound() {
	stop();
}

void Sound::play(int soundNum) {
	prime(soundNum);
	_soundManager->addToPlayList(this);
}

void Sound::stop() {
	_soundManager->removeFromPlayList(this);
	_unPrime();
}

void Sound::prime(int soundResID) {
	if (_soundResID != -1) {
		stop();
		_prime(soundResID, false);
	}
}

void Sound::unPrime() {
	stop();
}

void Sound::_prime(int soundResID, bool queFlag) {
	if (_primed)
		unPrime();

	if (_soundResID != -1) {
		// Sound number specified
		_isEmpty = false;
		_remoteReceiver = NULL;
		byte *soundData = _resourceManager->getResource(RES_SOUND, soundResID, 0);
		_soundManager->checkResVersion(soundData);
		_group = _soundManager->determineGroup(soundData);
		_sndResPriority = _soundManager->extractPriority(soundData);
		_sndResLoop = _soundManager->extractLoop(soundData);
		_soundManager->extractTrackInfo(&_trackInfo, soundData, _group);

		for (int idx = 0; idx < _trackInfo._numTracks; ++idx) {
			_channelData[idx] = _resourceManager->getResource(RES_SOUND, soundResID, _trackInfo._chunks[idx]);
		}

		DEALLOCATE(soundData);
	} else {
		// No sound specified
		_isEmpty = true;
		_group = 0;
		_sndResPriority = 0;
		_sndResLoop = 0;
		_trackInfo._numTracks = 0;
		_channelData[0] = ALLOCATE(200);
		_remoteReceiver = ALLOCATE(200);
	}

	if (queFlag)
		_soundManager->addToSoundList(this);

	_primed = true;
}

void Sound::_unPrime() {
	if (_primed) {
		if (_isEmpty) {
			DEALLOCATE(_channelData[0]);
			DEALLOCATE(_remoteReceiver);
			_remoteReceiver = NULL;
		} else {
			for (int idx = 0; idx < _trackInfo._numTracks; ++idx) {
				DEALLOCATE(_channelData[idx]);
			}
		}

		_trackInfo._numTracks = 0;
		_soundManager->removeFromSoundList(this);

		_primed = false;
		_stoppedAsynchronously = false;
	}
}

void Sound::orientAfterDriverChange() {
	if (!_isEmpty) {
		int timeIndex = getTimeIndex();

		for (int idx = 0; idx < _trackInfo._numTracks; ++idx)
			DEALLOCATE(_channelData[idx]);
		
		_trackInfo._numTracks = 0;
		_primed = false;
		_prime(_soundResID, true);
		setTimeIndex(timeIndex);
	}
}

void Sound::orientAfterRestore() {
	if (_isEmpty) {
		int timeIndex = getTimeIndex();
		_primed = false;
		_prime(_soundResID, true);
		setTimeIndex(timeIndex);
	}
}

void Sound::go() {
	if (!_primed)
		error("Attempt to execute Sound::go() on an unprimed Sound");

	_soundManager->addToPlayList(this);
}

void Sound::halt(void) {
	_soundManager->removeFromPlayList(this);
}

int Sound::getSoundNum() const {
	return _soundResID;
}

bool Sound::isPlaying() {
	return _soundManager->isOnPlayList(this);
}

bool Sound::isPrimed() const {
	return _primed;
}

bool Sound::isPaused() const {
	return _pausedCount != 0;
}

bool Sound::isMuted() const {
	return _mutedCount != 0;
}

void Sound::pause(bool flag) {
	_soundManager->suspendSoundServer();

	if (flag)
		++_pausedCount;
	else if (_pausedCount > 0)
		--_pausedCount;

	_soundManager->rethinkVoiceTypes();
	_soundManager->restartSoundServer();
}

void Sound::mute(bool flag) {
	_soundManager->suspendSoundServer();

	if (flag)
		++_mutedCount;
	else if (_mutedCount > 0)
		--_mutedCount;

	_soundManager->rethinkVoiceTypes();
	_soundManager->restartSoundServer();
}

void Sound::fade(int fadeDest, int fadeTicks, int fadeSteps, bool stopAfterFadeFlag) {
	_soundManager->suspendSoundServer();

	if (fadeDest > 127)
		fadeDest = 127;
	if (fadeTicks > 127)
		fadeTicks = 127;
	if (fadeSteps > 255)
		fadeSteps = 255;

	_fadeDest = fadeDest;
	_fadeTicks = fadeTicks;
	_fadeSteps = fadeSteps;
	_fadeCounter = 0;
	_stopAfterFadeFlag = stopAfterFadeFlag;

	_soundManager->restartSoundServer();
}

void Sound::setTimeIndex(uint32 timeIndex) {
	if (_primed) {
		mute(true);
		SoundManager::_soSetTimeIndex(timeIndex);
		mute(false);
	}
}

uint32 Sound::getTimeIndex() const {
	return _timer;
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
			_soundManager->updateSoundVol(this);
	}
}

int Sound::getVol() const {
	return _volume;
}

void Sound::setPri(int priority) {
	if (priority > 127)
		priority = 127;
	_fixedPriority = priority;
	_soundManager->updateSoundPri(this);
}

void Sound::setLoop(bool flag) {
	_fixedLoop = flag;
	_soundManager->updateSoundLoop(this);
}

int Sound::getPri() const {
	return _priority;
}

bool Sound::getLoop() {
	return _loop;
}

void Sound::holdAt(int amount) {
	if (amount > 127)
		amount = 127;
	_hold = amount;
}

void Sound::release() {
	_hold = -1;
}

/*--------------------------------------------------------------------------*/

ASound::ASound(): EventHandler() {
	_action = NULL;
	_cueValue = -1;
}

void ASound::synchronize(Serializer &s) {
	EventHandler::synchronize(s);

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

void ASound::prime(int soundResID, Action *action) {
	_action = action;
	_cueValue = 0;
	_sound.prime(soundResID);
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
	_driverResID = 0;
	_minVersion = _maxVersion = 0;
	_groupMask = 0;
}

/*--------------------------------------------------------------------------*/

const byte adlib_group_data[] = { 1, 1, 9, 1, 0xff };

AdlibSoundDriver::AdlibSoundDriver() {
	_minVersion = 0x102;
	_maxVersion = 0x10A;

	_groupData.groupMask = 9;
	_groupData.v1 = 0x46;
	_groupData.v2 = 0;
	_groupData.pData = &adlib_group_data[0];

	_mixer = _vm->_mixer;
}

} // End of namespace tSage
