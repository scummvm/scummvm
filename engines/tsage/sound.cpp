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

	if (_soundManager->_minVersion < minVersion)
		error("Attempt to play/prime sound resource that is too new");
	if (_soundManager->_minVersion > maxVersion)
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
		if ((v & _soundManager->_groupMask) == v)
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
	++_soundManager->_suspendCtr; 
	bool result = contains(_soundManager->_playList, sound);
	--_soundManager->_suspendCtr;

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
	++sfManager()._suspendCtr;
	_sfDereferenceAll();

	// Check for any active sound currently playing
	for (Common::List<Sound *>::iterator playIterator = sfManager()._playList.begin();
				playIterator != sfManager()._playList.end(); ++playIterator) {
		Sound *sound = *playIterator;
		if (sound->getCueValue() >= 0) {
			// Currently playing sound
			// TODO: Figure out how to determine when raw playback has ended
			return;
		}
	}

	// No currently playing sound, so look for any queued sounds to play
	for (Common::List<Sound *>::iterator playIterator = sfManager()._playList.begin();
				playIterator != sfManager()._playList.end(); ++playIterator) {
		Sound *sound = *playIterator;
		if (sound->getCueValue() == -1) {
			// Found a sound to start playing
			
			// Get the first sound driver
			assert(sfManager()._installedDrivers.size() > 0);
			SoundDriver *driver = *sfManager()._installedDrivers.begin();

			// Start each channel of the sound
			for (int channelNum = 0; channelNum < sound->_trackInfo._count; ++channelNum) {
				const byte *data = sound->_trackInfo._channelData[channelNum];
				int dataSize = _vm->_memoryManager.getSize(data);

				driver->play(data, dataSize, channelNum, sfManager()._volume);
			}

			sound->_cueValue = 0;
			return;
		}
	}

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
	++_soundManager->_suspendCtr; 

	int tempPriority = (sound->_priority2 == 255) ? sound->_soundPriority : sound->_priority;
	if (sound->_priority != tempPriority) {
		sound->_priority = tempPriority;
		if (_sfDoRemoveFromPlayList(sound)) {
			_sfDoAddToPlayList(sound);
			_sfRethinkVoiceTypes();
		}
	}

	--_soundManager->_suspendCtr;
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

	if (volume != _soundManager->_volume) {
		_soundManager->_volume = volume;

		for (Common::List<SoundDriver *>::iterator i = _soundManager->_installedDrivers.begin(); 
				i != _soundManager->_installedDrivers.end(); ++i) {
			(*i)->setMasterVolume(volume);
		}
	}
}

void SoundManager::_sfExtractTrackInfo(trackInfoStruct *trackInfo, const byte *soundData, int groupNum) {
	trackInfo->_count = 0;

	const byte *p = soundData + READ_LE_UINT16(soundData + 8);
	uint32 v;
	while ((v = READ_LE_UINT32(p)) != 0) {
		if ((v == 0x80000000) || (v == (uint)groupNum)) {
			// Found group to process
			int count = READ_LE_UINT16(p + 4);
			p += 6;

			for (int idx = 0; idx < count; ++idx) {
				if (trackInfo->_count == 16) {
					trackInfo->_count = -1;
					return;
				}

				trackInfo->_rlbList[trackInfo->_count] = READ_LE_UINT16(p);
				trackInfo->_arr2[trackInfo->_count] = READ_LE_UINT16(p + 2);
				++trackInfo->_count;
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

	_soundManager->_groupMask = mask;
}

bool SoundManager::_sfInstallDriver(SoundDriver *driver) {
	if (!driver->open())
		return false;
	
	sfManager()._installedDrivers.push_back(driver);
	driver->_groupOffset = driver->getGroupData();
	driver->_groupMask =  READ_LE_UINT32(driver->_groupOffset);

	_sfExtractGroupMask();
	_sfRethinkSoundDrivers();
	driver->setMasterVolume(sfManager()._volume);

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
	++_soundManager->_suspendCtr; 

	for (int voiceIndex = 0; voiceIndex < SOUND_ARR_SIZE; ++voiceIndex) {
		VoiceStruct *vs = sfManager()._voiceStructPtrs[voiceIndex];
		if (!vs)
			continue;

		for (uint idx = 0; idx < vs->_entries.size(); ++idx) {
			VoiceStructEntry &vse = vs->_entries[idx];
			SoundDriver *driver = vse._driver;

			if (vs->_voiceType == VOICETYPE_0) {
				if (!vse._field4 && !vse._field6) {
					int vol = sound->_volume * sound->_field48[vse._field8] / 127;
					driver->setVolume0(voiceIndex, vse._field0, 7, vol);
				}
			} else {
				if (!vse._field8 && !vse._fieldA) {
					int vol = sound->_volume * sound->_field48[vse._fieldC] / 127;
					driver->setVolume1(voiceIndex, vse._field0, 7, vol);
				}
			}
		}
	}
	
	--_soundManager->_suspendCtr;
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
	_trackInfo._count = 0;
	_primed = false;
	_isEmpty = false;
	_field26E = NULL;
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
		_isEmpty = false;
		_field26E = NULL;
		byte *soundData = _resourceManager->getResource(RES_SOUND, soundNum, 0);
		_soundManager->checkResVersion(soundData);
		_groupNum = _soundManager->determineGroup(soundData);
		_soundPriority = _soundManager->extractPriority(soundData);
		_loop = _soundManager->extractLoop(soundData);
		_soundManager->extractTrackInfo(&_trackInfo, soundData, _groupNum);

		for (int idx = 0; idx < _trackInfo._count; ++idx) {
			_trackInfo._channelData[idx] = _resourceManager->getResource(RES_SOUND, soundNum, _trackInfo._rlbList[idx]);
		}

		DEALLOCATE(soundData);
	} else {
		// No sound specified
		_isEmpty = true;
		_groupNum = 0;
		_soundPriority = 0;
		_loop = 0;
		_trackInfo._count = 0;
		_trackInfo._channelData[0] = ALLOCATE(200);
		_field26E = ALLOCATE(200);
	}

	if (queFlag)
		_soundManager->addToSoundList(this);

	_primed = true;
}

void Sound::_unPrime() {
	if (_primed) {
		if (_isEmpty) {
			DEALLOCATE(_trackInfo._channelData[0]);
			DEALLOCATE(_field26E);
			_field26E = NULL;
		} else {
			for (int idx = 0; idx < _trackInfo._count; ++idx) {
				DEALLOCATE(_trackInfo._channelData[idx]);
			}
		}

		_trackInfo._count = 0;
		_soundManager->removeFromSoundList(this);

		_primed = false;
		_stopFlag = false;
	}
}

void Sound::orientAfterDriverChange() {
	if (!_isEmpty) {
		int timeIndex = getTimeIndex();

		for (int idx = 0; idx < _trackInfo._count; ++idx)
			DEALLOCATE(_trackInfo._channelData[idx]);
		
		_trackInfo._count = 0;
		_primed = false;
		_prime(_soundNum, true);
		setTimeIndex(timeIndex);
	}
}

void Sound::orientAfterRestore() {
	if (_isEmpty) {
		int timeIndex = getTimeIndex();
		_primed = false;
		_prime(_soundNum, true);
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
	return _soundNum;
}

bool Sound::isPlaying() {
	return _soundManager->isOnPlayList(this);
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
	_soundManager->suspendSoundServer();

	if (flag)
		++_pauseCtr;
	else if (_pauseCtr > 0)
		--_pauseCtr;

	_soundManager->rethinkVoiceTypes();
	_soundManager->restartSoundServer();
}

void Sound::mute(bool flag) {
	_soundManager->suspendSoundServer();

	if (flag)
		++_muteCtr;
	else if (_muteCtr > 0)
		--_muteCtr;

	_soundManager->rethinkVoiceTypes();
	_soundManager->restartSoundServer();
}

void Sound::fade(int volume1, int volume2, int volume3, int volume4) {
	_soundManager->suspendSoundServer();

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
			_soundManager->updateSoundVol(this);
	}
}

int Sound::getVol() const {
	return _volume;
}

void Sound::setPri(int priority) {
	if (priority > 127)
		priority = 127;
	_priority2 = priority;
	_soundManager->updateSoundPri(this);
}

void Sound::setLoop(bool flag) {
	_loopFlag2 = flag;
	_soundManager->updateSoundLoop(this);
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

const int PCSoundDriver::_noteTable[] = {
	0xEEE, 0xE17, 0xD4D, 0xC8C, 0xBD9, 0xB2F, 0xA8E, 0x9F7,
	0x967, 0x8E0, 0x861, 0x7E8, 0x777, 0x70B, 0x6A6, 0x647,
	0x5EC, 0x597, 0x547, 0x4FB, 0x4B3, 0x470, 0x430, 0x3F4,
	0x3BB, 0x385, 0x353, 0x323, 0x2F6, 0x2CB, 0x2A3, 0x27D,
	0x259, 0x238, 0x218, 0x1FA, 0x1DD, 0x1C2, 0x1A9, 0x191,
	0x17B, 0x165, 0x151, 0x13E, 0x12C, 0x11C, 0x10C, 0x0FD,
	0x0EE, 0x0E1, 0x0D4, 0x0C8, 0x0BD, 0x0B2, 0x0A8, 0x09F,
	0x096, 0x08E, 0x086, 0x07E, 0x077, 0x070, 0x06A, 0x064,
	0x05E, 0x059, 0x054, 0x04F, 0x04B, 0x047, 0x043, 0x03F,
	0x03B, 0x038, 0x035, 0x032, 0x02F, 0x02C, 0x02A, 0x027,
	0x025, 0x023, 0x021, 0x01F, 0x01D, 0x01C, 0x01A, 0x019,
	0x017, 0x016, 0x015, 0x013, 0x012, 0x011, 0x010, 0x00F
};

const int PCSoundDriver::_noteTableCount = ARRAYSIZE(_noteTable);

void PCSoundDriver::setUpdateCallback(UpdateCallback upCb, void *ref) {
	_upCb = upCb;
	_upRef = ref;
}

void PCSoundDriver::findNote(int freq, int *note, int *oct) const {
	*note = _noteTableCount - 1;
	for (int i = 0; i < _noteTableCount; ++i) {
		if (_noteTable[i] <= freq) {
			*note = i;
			break;
		}
	}

	*oct = *note / 12;
	*note %= 12;
}

void PCSoundDriver::resetChannel(int channel) {
	stopChannel(channel);
	stopAll();
}

void PCSoundDriver::syncSounds() {
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
	_musicVolume = music_mute ? 0 : MIN(255, ConfMan.getInt("music_volume"));
	_sfxVolume = sfx_mute ? 0 : MIN(255, ConfMan.getInt("sfx_volume"));
}

/*--------------------------------------------------------------------------*/

const int AdlibDriverBase::_freqTable[] = {
	0x157, 0x16C, 0x181, 0x198, 0x1B1, 0x1CB,
	0x1E6, 0x203, 0x222, 0x243, 0x266, 0x28A
};

const int AdlibDriverBase::_freqTableCount = ARRAYSIZE(_freqTable);

const int AdlibDriverBase::_operatorsTable[] = {
	0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13,	16, 17, 18, 19, 20, 21
};

const int AdlibDriverBase::_operatorsTableCount = ARRAYSIZE(_operatorsTable);

const int AdlibDriverBase::_voiceOperatorsTable[] = {
	0, 3, 1, 4, 2, 5, 6, 9, 7, 10, 8, 11, 12, 15, 16, 16, 14, 14, 17, 17, 13, 13
};

const int AdlibDriverBase::_voiceOperatorsTableCount = ARRAYSIZE(_voiceOperatorsTable);


AdlibDriverBase::AdlibDriverBase(Audio::Mixer *mixer)
	: _mixer(mixer) {
	_sampleRate = _mixer->getOutputRate();
	_opl = makeAdLibOPL(_sampleRate);

	for (int i = 0; i < 5; ++i) {
		_channelsVolumeTable[i].original = 0;
		_channelsVolumeTable[i].adjusted = 0;
	}
	memset(_instrumentsTable, 0, sizeof(_instrumentsTable));
	initCard();
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	_musicVolume = ConfMan.getBool("music_mute") ? 0 : MIN(255, ConfMan.getInt("music_volume"));
	_sfxVolume = ConfMan.getBool("sfx_mute") ? 0 : MIN(255, ConfMan.getInt("sfx_volume"));
}

AdlibDriverBase::~AdlibDriverBase() {
	_mixer->stopHandle(_soundHandle);
	OPLDestroy(_opl);
}

void AdlibDriverBase::syncSounds() {
	PCSoundDriver::syncSounds();

	// Force all instruments to reload on the next playing point
	for (int i = 0; i < 5; ++i) {
		adjustVolume(i, _channelsVolumeTable[i].original);
		AdLibSoundInstrument *ins = &_instrumentsTable[i];
		setupInstrument(ins, i);
	}
}

void AdlibDriverBase::adjustVolume(int channel, int volume) {
	_channelsVolumeTable[channel].original = volume;

	if (volume > 80) {
		volume = 80;
	} else if (volume < 0) {
		volume = 0;
	}
	volume += volume / 4;
	if (volume > 127) {
		volume = 127;
	}

	int volAdjust = (channel == 4) ? _sfxVolume : _musicVolume;
	volume = (volume * volAdjust) / 128;

	if (volume > 127)
		volume = 127;

	_channelsVolumeTable[channel].adjusted = volume;
}

void AdlibDriverBase::setupChannel(int channel, const byte *data, int instrument, int volume) {
	assert(channel < 5);
	if (data) {
		adjustVolume(channel, volume);
		setupInstrument(data, channel);
	}
}

void AdlibDriverBase::stopChannel(int channel) {
	assert(channel < 5);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		channel = 6;
	}
	if (ins->mode == 0 || channel == 6) {
		OPLWriteReg(_opl, 0xB0 | channel, 0);
	}
	if (ins->mode != 0) {
		_vibrato &= ~(1 << (10 - ins->channel));
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
}

void AdlibDriverBase::stopAll() {
	int i;
	for (i = 0; i < 18; ++i) {
		OPLWriteReg(_opl, 0x40 | _operatorsTable[i], 63);
	}
	for (i = 0; i < 9; ++i) {
		OPLWriteReg(_opl, 0xB0 | i, 0);
	}
	OPLWriteReg(_opl, 0xBD, 0);
}

int AdlibDriverBase::readBuffer(int16 *buffer, const int numSamples) {
	update(buffer, numSamples);
	return numSamples;
}

void AdlibDriverBase::initCard() {
	_vibrato = 0x20;
	OPLWriteReg(_opl, 0xBD, _vibrato);
	OPLWriteReg(_opl, 0x08, 0x40);

	static const int oplRegs[] = { 0x40, 0x60, 0x80, 0x20, 0xE0 };

	for (int i = 0; i < 9; ++i) {
		OPLWriteReg(_opl, 0xB0 | i, 0);
	}
	for (int i = 0; i < 9; ++i) {
		OPLWriteReg(_opl, 0xC0 | i, 0);
	}

	for (int j = 0; j < 5; j++) {
		for (int i = 0; i < 18; ++i) {
			OPLWriteReg(_opl, oplRegs[j] | _operatorsTable[i], 0);
		}
	}

	OPLWriteReg(_opl, 1, 0x20);
	OPLWriteReg(_opl, 1, 0);
}

void AdlibDriverBase::update(int16 *buf, int len) {
	static int samplesLeft = 0;
	while (len != 0) {
		int count = samplesLeft;
		if (count > len) {
			count = len;
		}
		samplesLeft -= count;
		len -= count;
		YM3812UpdateOne(_opl, buf, count);
		if (samplesLeft == 0) {
			if (_upCb) {
				(*_upCb)(_upRef);
			}
			samplesLeft = _sampleRate / 50;
		}
		buf += count;
	}
}

void AdlibDriverBase::setupInstrument(const byte *data, int channel) {
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	loadInstrument(data, ins);

	setupInstrument(ins, channel);
}

void AdlibDriverBase::setupInstrument(const AdLibSoundInstrument *ins, int channel) {
	int mod, car, tmp;
	const AdLibRegisterSoundInstrument *reg;

	if (ins->mode != 0)  {
		mod = _operatorsTable[_voiceOperatorsTable[2 * ins->channel + 0]];
		car = _operatorsTable[_voiceOperatorsTable[2 * ins->channel + 1]];
	} else {
		mod = _operatorsTable[_voiceOperatorsTable[2 * channel + 0]];
		car = _operatorsTable[_voiceOperatorsTable[2 * channel + 1]];
	}

	if (ins->mode == 0 || ins->channel == 6) {
		reg = &ins->regMod;
		OPLWriteReg(_opl, 0x20 | mod, reg->vibrato);
		if (reg->freqMod) {
			tmp = reg->outputLevel & 0x3F;
		} else {
			tmp = (63 - (reg->outputLevel & 0x3F)) * _channelsVolumeTable[channel].adjusted;
			tmp = 63 - (2 * tmp + 127) / (2 * 127);
		}
		OPLWriteReg(_opl, 0x40 | mod, tmp | (reg->keyScaling << 6));
		OPLWriteReg(_opl, 0x60 | mod, reg->attackDecay);
		OPLWriteReg(_opl, 0x80 | mod, reg->sustainRelease);
		if (ins->mode != 0) {
			OPLWriteReg(_opl, 0xC0 | ins->channel, reg->feedbackStrength);
		} else {
			OPLWriteReg(_opl, 0xC0 | channel, reg->feedbackStrength);
		}
		OPLWriteReg(_opl, 0xE0 | mod, ins->waveSelectMod);
	}

	reg = &ins->regCar;
	OPLWriteReg(_opl, 0x20 | car, reg->vibrato);
	tmp = (63 - (reg->outputLevel & 0x3F)) * _channelsVolumeTable[channel].adjusted;
	tmp = 63 - (2 * tmp + 127) / (2 * 127);
	OPLWriteReg(_opl, 0x40 | car, tmp | (reg->keyScaling << 6));
	OPLWriteReg(_opl, 0x60 | car, reg->attackDecay);
	OPLWriteReg(_opl, 0x80 | car, reg->sustainRelease);
	OPLWriteReg(_opl, 0xE0 | car, ins->waveSelectCar);
}

void AdlibDriverBase::loadRegisterInstrument(const byte *data, AdLibRegisterSoundInstrument *reg) {
	reg->vibrato = 0;
	if (READ_LE_UINT16(data + 18)) { // amplitude vibrato
		reg->vibrato |= 0x80;
	}
	if (READ_LE_UINT16(data + 20)) { // frequency vibrato
		reg->vibrato |= 0x40;
	}
	if (READ_LE_UINT16(data + 10)) { // sustaining sound
		reg->vibrato |= 0x20;
	}
	if (READ_LE_UINT16(data + 22)) { // envelope scaling
		reg->vibrato |= 0x10;
	}
	reg->vibrato |= READ_LE_UINT16(data + 2) & 0xF; // frequency multiplier

	reg->attackDecay = READ_LE_UINT16(data + 6) << 4; // attack rate
	reg->attackDecay |= READ_LE_UINT16(data + 12) & 0xF; // decay rate

	reg->sustainRelease = READ_LE_UINT16(data + 8) << 4; // sustain level
	reg->sustainRelease |= READ_LE_UINT16(data + 14) & 0xF; // release rate

	reg->feedbackStrength = READ_LE_UINT16(data + 4) << 1; // feedback
	if (READ_LE_UINT16(data + 24) == 0) { // frequency modulation
		reg->feedbackStrength |= 1;
	}

	reg->keyScaling = READ_LE_UINT16(data);
	reg->outputLevel = READ_LE_UINT16(data + 16);
	reg->freqMod = READ_LE_UINT16(data + 24);
}

/*--------------------------------------------------------------------------*/

void AdlibSoundDriverADL::loadInstrument(const byte *data, AdLibSoundInstrument *asi) {
	asi->mode = *data++;
	asi->channel = *data++;
	asi->waveSelectMod = *data++ & 3;
	asi->waveSelectCar = *data++ & 3;
	asi->amDepth = *data++;
	++data;
	loadRegisterInstrument(data, &asi->regMod); data += 26;
	loadRegisterInstrument(data, &asi->regCar); data += 26;
}

void AdlibSoundDriverADL::setChannelFrequency(int channel, int frequency) {
	assert(channel < 5);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0) {
		channel = ins->channel;
		if (channel == 9) {
			channel = 8;
		} else if (channel == 10) {
			channel = 7;
		}
	}
	int freq, note, oct;
	findNote(frequency, &note, &oct);

	note += oct * 12;
	if (ins->amDepth) {
		note = ins->amDepth;
	}
	if (note < 0) {
		note = 0;
	}

	freq = _freqTable[note % 12];
	OPLWriteReg(_opl, 0xA0 | channel, freq);
	freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
	if (ins->mode == 0) {
		freq |= 0x20;
	}
	OPLWriteReg(_opl, 0xB0 | channel, freq);
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - channel);
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
}

void AdlibSoundDriverADL::playSample(const byte *data, int size, int channel, int volume) {
	adjustVolume(channel, 127);

	setupInstrument(data, channel);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		OPLWriteReg(_opl, 0xB0 | channel, 0);
	}
	if (ins->mode != 0) {
		_vibrato &= ~(1 << (10 - ins->channel));
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
	if (ins->mode != 0) {
		channel = ins->channel;
		if (channel == 9) {
			channel = 8;
		} else if (channel == 10) {
			channel = 7;
		}
	}
	uint16 note = 48;
	if (ins->amDepth) {
		note = ins->amDepth;
	}
	int freq = _freqTable[note % 12];
	OPLWriteReg(_opl, 0xA0 | channel, freq);
	freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
	if (ins->mode == 0) {
		freq |= 0x20;
	}
	OPLWriteReg(_opl, 0xB0 | channel, freq);
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - channel);
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
}

/*--------------------------------------------------------------------------*/

PCSoundFxPlayer::PCSoundFxPlayer(PCSoundDriver *driver)
	: _playing(false), _songPlayed(false), _driver(driver) {
	memset(_instrumentsData, 0, sizeof(_instrumentsData));
	_sfxData = NULL;
	_fadeOutCounter = 0;
	_driver->setUpdateCallback(updateCallback, this);
}

PCSoundFxPlayer::~PCSoundFxPlayer() {
	_driver->setUpdateCallback(NULL, NULL);
	stop();
}

bool PCSoundFxPlayer::load(const char *song) {
	/*
	debug(9, "PCSoundFxPlayer::load('%s')", song);

	// stop (w/ fade out) the previous song 
	while (_fadeOutCounter != 0 && _fadeOutCounter < 100) {
		g_system->delayMillis(50);
	}
	_fadeOutCounter = 0;

	if (_playing) {
		stop();
	}

	strcpy(_musicName, song);
	_songPlayed = false;
	_looping = false;
	_sfxData = readBundleSoundFile(song);
	if (!_sfxData) {
		warning("Unable to load soundfx module '%s'", song);
		return 0;
	}

	for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
		_instrumentsData[i] = NULL;

		char instrument[64];
		memset(instrument, 0, 64); // Clear the data first
		memcpy(instrument, _sfxData + 20 + i * 30, 12);
		instrument[63] = '\0';

		if (strlen(instrument) != 0) {
			char *dot = strrchr(instrument, '.');
			if (dot) {
				*dot = '\0';
			}
			strcat(instrument, _driver->getInstrumentExtension());
			_instrumentsData[i] = readBundleSoundFile(instrument);
			if (!_instrumentsData[i]) {
				warning("Unable to load soundfx instrument '%s'", instrument);
			}
		}
	}
	*/
	return 1;
}

void PCSoundFxPlayer::play() {
	debug(9, "PCSoundFxPlayer::play()");
	if (_sfxData) {
		for (int i = 0; i < NUM_CHANNELS; ++i) {
			_instrumentsChannelTable[i] = -1;
		}
		_currentPos = 0;
		_currentOrder = 0;
//		_numOrders = _sfxData[470];
		_eventsDelay = 5;	// TODO: What to do with this?
		_updateTicksCounter = 0;
		_playing = true;
	}
}

void PCSoundFxPlayer::stop() {
	if (_playing || _fadeOutCounter != 0) {
		_fadeOutCounter = 0;
		_playing = false;
		for (int i = 0; i < NUM_CHANNELS; ++i) {
			_driver->stopChannel(i);
		}
		_driver->stopAll();
	}
	unload();
}

void PCSoundFxPlayer::fadeOut() {
	if (_playing) {
		_fadeOutCounter = 1;
		_playing = false;
	}
}

void PCSoundFxPlayer::updateCallback(void *ref) {
	((PCSoundFxPlayer *)ref)->update();
}

void PCSoundFxPlayer::update() {
	if (_playing || (_fadeOutCounter != 0 && _fadeOutCounter < 100)) {
		++_updateTicksCounter;
		if (_updateTicksCounter > _eventsDelay) {
			handleEvents();
			_updateTicksCounter = 0;
		}
	}
}

void PCSoundFxPlayer::handleEvents() {
	const byte *patternData = _sfxData + 600 + 1800;
	const byte *orderTable = _sfxData + 472;
	uint16 patternNum = orderTable[_currentOrder] * 1024;

	for (int i = 0; i < 4; ++i) {
		handlePattern(i, patternData + patternNum + _currentPos);
		patternData += 4;
	}

	if (_fadeOutCounter != 0 && _fadeOutCounter < 100) {
		_fadeOutCounter += 2;
	}
	if (_fadeOutCounter >= 100) {
		stop();
		return;
	}

	_currentPos += 16;
	if (_currentPos >= 1024) {
		_currentPos = 0;
		++_currentOrder;
		if (_currentOrder == _numOrders) {
			_currentOrder = 0;
		}
	}
	debug(7, "_currentOrder=%d/%d _currentPos=%d", _currentOrder, _numOrders, _currentPos);
}

void PCSoundFxPlayer::handlePattern(int channel, const byte *patternData) {
	int instrument = patternData[2] >> 4;
	if (instrument != 0) {
		--instrument;
		if (_instrumentsChannelTable[channel] != instrument || _fadeOutCounter != 0) {
			_instrumentsChannelTable[channel] = instrument;
			const int volume = _sfxData[instrument] - _fadeOutCounter;
			_driver->setupChannel(channel, _instrumentsData[instrument], instrument, volume);
		}
	}
	int16 freq = (int16)READ_BE_UINT16(patternData);
	if (freq > 0) {
		_driver->stopChannel(channel);
		_driver->setChannelFrequency(channel, freq);
	}
}

void PCSoundFxPlayer::unload() {
	for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
		free(_instrumentsData[i]);
		_instrumentsData[i] = NULL;
	}
	free(_sfxData);
	_sfxData = NULL;
	_songPlayed = true;
}

void PCSoundFxPlayer::doSync(Common::Serializer &s) {
	s.syncBytes((byte *)_musicName, 33);
	uint16 v = (uint16)songLoaded();
	s.syncAsSint16LE(v);

	if (s.isLoading() && v) {
		load(_musicName);

		for (int i = 0; i < NUM_CHANNELS; ++i) {
			_instrumentsChannelTable[i] = -1;
		}

		_numOrders = _sfxData[470];
		_eventsDelay = (244 - _sfxData[471]) * 100 / 1060;
		_updateTicksCounter = 0;
	}

	s.syncAsSint16LE(_songPlayed);
	s.syncAsSint16LE(_looping);
	s.syncAsSint16LE(_currentPos);
	s.syncAsSint16LE(_currentOrder);
	s.syncAsSint16LE(_playing);
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
	_soundDriver = new AdlibSoundDriverADL(_mixer);
	_player = new PCSoundFxPlayer(_soundDriver);
}

} // End of namespace tSage
