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

#include "lastexpress/sound/sound.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

SoundManager::SoundManager(LastExpressEngine *engine) {
	_engine = engine;
	_mixer = _engine->_mixer;

	for (int i = 0; i < ARRAYSIZE(_soundSlotChannels); i++) {
		_soundSlotChannels[i] = nullptr;
	}

	memset(_soundChannelsMixBuffers, 0, sizeof(_soundChannelsMixBuffers));

	soundDriverInit();
}

SoundManager::~SoundManager() {
	_engine = nullptr;
}

int SoundManager::playSoundFile(const char *sndName, int typeFlags, int character, int delay) {
	Slot *slot = new Slot(this, sndName, typeFlags, 30);

	slot->setAssociatedCharacter(character);

	if (delay) {
		slot->setDelayedStartTime(getSoundDriver30HzCounter() + 2 * delay);
		slot->addStatusFlag(kSoundFlagDelayedActivate);
	} else {
		Common::String subName = sndName;
		subName.replace('.', '\0');

		slot->setSub(subName.c_str());
		slot->play();
	}

	return slot->getTag();
}

void SoundManager::startAmbient() {
	_soundSlotAmbientFlag |= kAmbientSoundEnabled;
}

void SoundManager::startSteam(int cityIndex) {
	Slot *cachedSlot;
	bool playSoundNeeded = true;

	_soundSlotAmbientFlag |= kAmbientSoundSteam;

	if (_soundCache) {
		cachedSlot = _soundCache;
		do {
			if (cachedSlot->hasTag(kSoundTagAmbient))
				break;

			cachedSlot = cachedSlot->getNext();
		} while (cachedSlot);

		if (cachedSlot)
			playSoundNeeded = false;
	}

	if (playSoundNeeded)
		playSoundFile("STEAM.SND", kSoundTypeAmbient | kSoundFlagLooped | kVolume7, kCharacterSteam, 0);

	cachedSlot = _soundCache;
	if (_soundCache) {
		do {
			if (cachedSlot->hasTag(kSoundTagAmbient))
				break;

			cachedSlot = cachedSlot->getNext();
		} while (cachedSlot);

		if (cachedSlot) {
			cachedSlot = _soundCache;
			if (_soundCache) {
				do {
					if (cachedSlot->hasTag(kSoundTagAmbient))
						break;

					cachedSlot = cachedSlot->getNext();
				} while (cachedSlot);

				if (cachedSlot) {
					cachedSlot->setSub(_cities[cityIndex]);
				}
			}
		}
	}
}

void SoundManager::endAmbient() {
	_soundSlotAmbientFlag = 0;

	Slot *cachedSlot = _soundCache;
	if (_soundCache) {
		do {
			if (cachedSlot->hasTag(kSoundTagAmbient))
				break;

			cachedSlot = cachedSlot->getNext();
		} while (cachedSlot);

		if (cachedSlot)
			cachedSlot->setFade(0);
	}

	cachedSlot = _soundCache;
	if (_soundCache) {
		do {
			if (cachedSlot->hasTag(kSoundTagOldAmbient))
				break;

			cachedSlot = cachedSlot->getNext();
		} while (cachedSlot);

		if (cachedSlot)
			cachedSlot->setFade(0);
	}
}

void SoundManager::killAmbient() {
	_soundSlotAmbientFlag = 0;

	Slot *cachedSlot = _soundCache;
	if (_soundCache) {
		do {
			if (cachedSlot->hasTag(kSoundTagAmbient))
				break;

			cachedSlot = cachedSlot->getNext();
		} while (cachedSlot);

		if (cachedSlot) {
			cachedSlot->addStatusFlag(kSoundFlagCloseRequested);
			cachedSlot->setAssociatedCharacter(0);

			cachedSlot->closeArchive();
		}
	}

	cachedSlot = _soundCache;
	if (_soundCache) {
		do {
			if (cachedSlot->hasTag(kSoundTagOldAmbient))
				break;

			cachedSlot = cachedSlot->getNext();
		} while (cachedSlot);

		if (cachedSlot) {
			cachedSlot->addStatusFlag(kSoundFlagCloseRequested);
			cachedSlot->setAssociatedCharacter(0);

			cachedSlot->closeArchive();
		}
	}
}

void SoundManager::raiseAmbient(int level, int delay) {
	if (level > 7) {
		_soundAmbientFadeLevel = level;
		_soundAmbientFadeTime = getSoundDriver30HzCounter() + 2 * delay;
	}
}

void SoundManager::levelAmbient(int delay) {
	_soundAmbientFadeLevel = 7;
	_soundAmbientFadeTime = getSoundDriver30HzCounter() + 2 * delay;
}

Slot *SoundManager::findSlotWho(int32 character) {
	Slot *result = _soundCache;

	if (!_soundCache)
		return nullptr;

	while (result->getAssociatedCharacter() != character) {
		result = result->getNext();

		if (!result)
			return nullptr;
	}

	return result;
}

Slot *SoundManager::findSlotName(char *name) {
	Common::StackLock lock(*_engine->_soundMutex);
	Slot *result = _soundCache;

	if (!_soundCache)
		return nullptr;

	while (scumm_stricmp(name, result->getName2())) {
		result = result->getNext();

		if (!result)
			return nullptr;
	}

	return result;
}

void SoundManager::ambientAI(int id) {
	Slot *oldAmbientSlot1;
	Slot *oldAmbientSlot2;

	uint fileNameLen = 6;
	int soundId = 1;

	byte numLoops[9] = {0, 4, 2, 2, 2, 2, 2, 0, 0};

	int positions[8] = {
		8200, 7500, 6470, 5790,
		4840, 4070, 3050, 2740
	};

	char newAmbientSoundName[80];
	memset(newAmbientSoundName, 0, sizeof(newAmbientSoundName));

	for (oldAmbientSlot1 = _soundCache; oldAmbientSlot1; oldAmbientSlot1 = oldAmbientSlot1->getNext()) {
		if (oldAmbientSlot1->hasTag(kSoundTagAmbient))
			break;
	}

	if ((_soundSlotAmbientFlag & kAmbientSoundEnabled) != 0 && (id == 69 || id == 70)) {
		if ((_soundSlotAmbientFlag & kAmbientSoundSteam) != 0) {
			Common::strcpy_s(newAmbientSoundName, "STEAM.SND");
			_loopingSoundDuration = 0x7FFF;
		} else {
			if (getCharacter(kCharacterCath).characterPosition.location == kLocationOutsideTrain) {
				soundId = 6;
			} else if (_engine->getLogicManager()->inComp(kCharacterCath)) {
				numLoops[0] = 0;

				int objNum = (getCharacter(kCharacterCath).characterPosition.car == kCarGreenSleeping) ? 9 : 40;

				for (int pos = 0; pos < ARRAYSIZE(positions); pos++) {
					if (numLoops[0] == 1)
						break;

					if (_engine->getLogicManager()->inComp(kCharacterCath, getCharacter(kCharacterCath).characterPosition.car, positions[pos])) {
						numLoops[0] = 1;
						soundId = _engine->getLogicManager()->_doors[objNum].status == 2 ? 6 : 1;
					}

					objNum++;
				}
			} else {
				switch (getCharacter(kCharacterCath).characterPosition.car) {
				case kCarBaggageRear:
				case kCarBaggage:
					soundId = 4;
					break;
				case kCarKronos:
				case kCarGreenSleeping:
				case kCarRedSleeping:
				case kCarRestaurant:
					soundId = 1;
					break;
				case kCarCoalTender:
					soundId = 5;
					break;
				case kCarLocomotive:
					soundId = 99;
					break;
				case kCarVestibule:
					soundId = 3;
					break;
				default:
					soundId = 6;
					break;
				}
			}

			if (soundId != 99) {
				char soundSection = (char)((rnd(UINT_MAX)) % numLoops[soundId]) + 'A';
				Common::sprintf_s(newAmbientSoundName, "LOOP%d%c.SND", soundId, soundSection);
			}
		}

		if (_scanAnySoundLoopingSection)
			fileNameLen = 5;

		if (!oldAmbientSlot1 || scumm_strnicmp(oldAmbientSlot1->_name2, newAmbientSoundName, fileNameLen)) {
			_loopingSoundDuration = ((rnd(UINT_MAX)) % 320) + 260;

			if (soundId != 99) {
				if (_engine->isDemo()) {
					playSoundFile(newAmbientSoundName, kSoundTypeAmbient | kSoundFlagLooped | kVolume2, kCharacterSteam, 0);
				} else {
					playSoundFile(newAmbientSoundName, kSoundTypeAmbient | kSoundFlagLooped | kVolume1, kCharacterSteam, 0);
				}

				if (oldAmbientSlot1)
					oldAmbientSlot1->setFade(kVolumeNone);

				oldAmbientSlot2 = _soundCache;

				if (_soundCache) {
					do {
						if (oldAmbientSlot2->hasTag(kSoundTagAmbient))
							break;

						oldAmbientSlot2 = oldAmbientSlot2->getNext();
					} while (oldAmbientSlot2);

					if (oldAmbientSlot2)
						oldAmbientSlot2->setFade(kVolume7);
				}
			}
		}
	}
}

void SoundManager::soundThread() {
	int priority;
	int maxPriority = 0;
	bool loopedPlaying = false;
	Slot *ambientSlot1;
	Slot *ambientSlot2;
	Slot *slotToDevirtualize;
	Slot *cachedSlot;
	Slot *next;

	if (!isCopyingDataToSoundDriver()) {
		ambientSlot1 = _soundCache;
		_inSoundThreadFunction++;

		if (_soundCache) {
			do {
				if (ambientSlot1->hasTag(kSoundTagAmbient))
					break;

				ambientSlot1 = ambientSlot1->getNext();
			} while (ambientSlot1);
		}

		if ((_soundSlotAmbientFlag & kAmbientSoundEnabled) != 0) {
			ambientSlot2 = _soundCache;
			if (!_soundCache) {
				loopedPlaying = true;
			} else {
				do {
					if (ambientSlot2->hasTag(kSoundTagAmbient))
						break;

					ambientSlot2 = ambientSlot2->getNext();
				} while (ambientSlot2);

				if (!ambientSlot2 || _scanAnySoundLoopingSection || (ambientSlot1 && ambientSlot1->getTime() > _loopingSoundDuration))
					loopedPlaying = true;
			}

			if (loopedPlaying) {
				ambientAI(kAmbientLooping);
			} else if (_soundAmbientFadeTime && getSoundDriver30HzCounter() >= _soundAmbientFadeTime) {
				assert(ambientSlot1);
				ambientSlot1->setFade(_soundAmbientFadeLevel);
				_soundAmbientFadeTime = 0;
			}
		}

		slotToDevirtualize = nullptr;
		cachedSlot = _soundCache;
		if (_soundCache) {
			do {
				next = cachedSlot->getNext();

				if ((cachedSlot->getStatusFlags() & kSoundFlagMuteProcessed) != 0) {
					if (cachedSlot->getSoundBuffer())
						cachedSlot->releaseBuffer();

					if (cachedSlot->closeArchive()) {
						cachedSlot->setNumLoadedBytes(3);
					}

					if (_numActiveChannels < 6 && (cachedSlot->getStatusFlags() & kSoundVolumeMask) != 0) {
						priority = cachedSlot->getPriority();

						if (priority + (int)(cachedSlot->getStatusFlags() & kSoundVolumeMask) > maxPriority) {
							slotToDevirtualize = cachedSlot;
							maxPriority = (cachedSlot->getStatusFlags() & kSoundVolumeMask) + priority;
						}
					}
				}

				if (!cachedSlot->update() && (cachedSlot->getStatusFlags() & kSoundFlagKeepAfterFinish) == 0) {
					if (slotToDevirtualize == cachedSlot) {
						maxPriority = 0;
						slotToDevirtualize = nullptr;
					}

					if (cachedSlot) {
						if (cachedSlot == _engine->getNISManager()->getChainedSound()) {
							// The original deleted the cachedSlot and probably set
							// all its values to zero, which might not be the case on
							// modern compilers and might instead trigger an exception
							// on the NIS code...
							_engine->getNISManager()->setChainedSound(nullptr);
						}

						delete cachedSlot;
						cachedSlot = nullptr;
					}
				}

				cachedSlot = next;
			} while (next);
		}

		if (slotToDevirtualize)
			slotToDevirtualize->devirtualize();

		_scanAnySoundLoopingSection = false;
		_inSoundThreadFunction--;
	}
}

void SoundManager::killAllSlots() {
	for (Slot *i = _soundCache; i; i = i->getNext())
		i->addStatusFlag(kSoundFlagCloseRequested);
}

void SoundManager::killAllExcept(int tag1, int tag2, int tag3, int tag4, int tag5, int tag6, int tag7) {
	Common::StackLock lock(*_engine->_soundMutex);

	Slot *slot = _soundCache;

	if (!tag2)
		tag2 = tag1;

	if (!tag3)
		tag3 = tag1;

	if (!tag4)
		tag4 = tag1;

	if (!tag5)
		tag5 = tag1;

	if (!tag6)
		tag6 = tag1;

	if (!tag7)
		tag7 = tag1;

	if (_soundCache) {
		do {
			int tag = slot->getTag();
			if (tag1 != tag && tag2 != tag && tag3 != tag && tag4 != tag && tag5 != tag && tag6 != tag && tag7 != tag) {
				slot->addStatusFlag(kSoundFlagCloseRequested);
				slot->setAssociatedCharacter(0);

				slot->closeArchive();
			}

			slot = slot->getNext();
		} while (slot);
	}
}

void SoundManager::saveSoundInfo(CVCRFile *file) {
	Common::StackLock lock(*_engine->_soundMutex);

	SaveSlot *saveSlot = new SaveSlot();
	int numSounds = 0;

	file->writeRLE(&_soundSlotAmbientFlag, 4, 1);
	file->writeRLE(&_curSoundSlotTag, 4, 1);
	for (Slot *i = _soundCache; i; i = i->_next) {
		if (scumm_stricmp("NISSND?", i->_name2) && (i->_statusFlags & kSoundTypeMask) != kSoundTypeMenu)
			numSounds++;
	}

	file->writeRLE(&numSounds, 4, 1);

	for (Slot *j = _soundCache; j; j = j->_next) {
		if (scumm_stricmp("NISSND?", j->_name2) && (j->_statusFlags & kSoundTypeMask) != kSoundTypeMenu) {
			saveSlot->tag = j->_tag;
			saveSlot->blockCount = j->_blockCount;
			saveSlot->status = j->_statusFlags;
			saveSlot->time = j->_time;
			saveSlot->fadeDelayCounter = j->_fadeDelayCounter;
			saveSlot->unusedVar = j->_unusedVar;
			saveSlot->character = j->_character;
			saveSlot->delayTicks = j->_delayedStartTime - _sound30HzCounter;
			if (saveSlot->delayTicks > 0x8000000)
				saveSlot->delayTicks = 0;

			saveSlot->priority = j->_priority;
			strncpy(saveSlot->name1, j->_name1, sizeof(saveSlot->name1));
			strncpy(saveSlot->name2, j->_name2, sizeof(saveSlot->name2));

			// I have verified that ANY possible name in here won't be longer than 12 characters,
			// so it's safe to put this here, like Coverity asked to...
			saveSlot->name1[15] = '\0';
			saveSlot->name2[15] = '\0';

			file->writeRLE(saveSlot, sizeof(SaveSlot), 1);
		}
	}

	delete saveSlot;
}

void SoundManager::destroyAllSound() {
	Slot *i;
	Slot *next;
	int32 waitCycles = 0;

	addSoundDriverFlags(kSoundDriverClearBufferRequested);

	// Wait for the driver to clear the mix buffer
	for (i = _soundCache; (getSoundDriverFlags() & kSoundDriverClearBufferProcessed) == 0; waitCycles++) {
		if (waitCycles >= 3000000)
			break;
	}

	addSoundDriverFlags(kSoundDriverClearBufferProcessed);

	if (_soundCache) {
		do {
			next = i->getNext();
			i->setAssociatedCharacter(0);

			if (i->getSoundBuffer())
				i->releaseBuffer();

			if (i) {
				delete i;
				i = nullptr;
			}

			i = next;
		} while (next);
	}

	_engine->getSubtitleManager()->subThread();
}

void SoundManager::loadSoundInfo(CVCRFile *file, bool skipSoundLoading) {
	Common::StackLock lock(*_engine->_soundMutex);

	int numSounds;

	SaveSlot *saveSlot = new SaveSlot();

	if (skipSoundLoading) {
		int skippedValue;
		
		file->readRLE(&skippedValue, 4, 1);
		file->readRLE(&skippedValue, 4, 1);
		file->readRLE(&numSounds, 4, 1);

		for (int j = 0; j < numSounds; j++) {
			file->readRLE(saveSlot, sizeof(SaveSlot), 1);
		}
	} else {
		file->readRLE(&_soundSlotAmbientFlag, 4, 1);
		file->readRLE(&_curSoundSlotTag, 4, 1);
		file->readRLE(&numSounds, 4, 1);

		for (int j = 0; j < numSounds; j++) {
			file->readRLE(saveSlot, sizeof(SaveSlot), 1);
			// This apparently useless instruction automatically adds the saveSlot pointer to the cache
			Slot *tmp = new Slot(this, saveSlot);
			assert(tmp);
		}

		for (Slot *i = _soundCache; i; i = i->_next) {
			if ((i->_statusFlags & kSoundFlagHasLinkAfter) != 0) {
				Slot *cachedSlot = _soundCache;

				if (_soundCache) {
					while (scumm_stricmp(cachedSlot->_name2, i->_name1)) {
						cachedSlot = cachedSlot->_next;

						if (!cachedSlot)
							break;
					}

					if (cachedSlot)
						i->_chainedSound = cachedSlot;
				}
			}
		}

		_soundDriverFlags &= ~(kSoundDriverClearBufferRequested | kSoundDriverClearBufferProcessed);
	}

	delete saveSlot;
}

void SoundManager::addSlot(Slot *entry) {
	Slot *cachedSlot = _soundCache;
	if (_soundCache) {
		if (_soundCache->getNext()) {
			do {
				cachedSlot = cachedSlot->getNext();
			} while (cachedSlot->getNext());
		}

		cachedSlot->setNext(entry);
		_soundCacheCount++;
	} else {
		_soundCacheCount++;
		_soundCache = entry;
	}
}

void SoundManager::removeSlot(Slot *entry) {
	Slot *cachedSlot;
	Slot *next;

	cachedSlot = _soundCache;

	if (_soundCache && entry) {
		if (_soundCache == entry) {
			_soundCache = _soundCache->getNext();
			_soundCacheCount--;
		} else {
			if (_soundCache->getNext() != entry) {
				do {
					next = cachedSlot->getNext();
					if (!next)
						break;

					cachedSlot = cachedSlot->getNext();
				} while (next->getNext() != entry);
			}

			if (cachedSlot->getNext()) {
				cachedSlot->setNext(cachedSlot->getNext()->getNext());
				_soundCacheCount--;
			}
		}
	}
}

void SoundManager::NISFadeOut() {
	for (Slot *i = _soundCache; i; i = i->getNext()) {
		i->assignDirectVolume(i->getStatusFlags() & kSoundVolumeMask);

		if (i->getVolume())
			i->setFade((i->getVolume() >> 1) + 1);
	}

	addSoundDriverFlags(kSoundDriverNISHasRequestedFade);
}

void SoundManager::NISFadeIn() {
	removeSoundDriverFlags(kSoundDriverNISHasRequestedFade);

	for (Slot *i = _soundCache; i; i = i->getNext()) {
		if (i->getVolume()) {
			if (!i->hasTag(kSoundTagNIS) && !i->hasTag(kSoundTagLink))
				i->setFade(i->getVolume());
		}
	}
}

int SoundManager::getMasterVolume() {
	int result = soundDriverGetVolume();

	if (result < 0)
		return 0;

	if (result > 7)
		return 7;

	return result;
}

void SoundManager::setMasterVolume(int volume) {
	int effVolume = volume;

	if (volume < 0) {
		effVolume = 0;
	}

	if (volume > 7)
		effVolume = 7;

	soundDriverSetVolume(effVolume);
}

} // End of namespace LastExpress
