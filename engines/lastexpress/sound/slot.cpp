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

#include "lastexpress/sound/slot.h"
#include "lastexpress/sound/sound.h"
#include "lastexpress/sound/subtitle.h"
#include "lastexpress/data/archive.h"

#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"

namespace LastExpress {

Slot::Slot(SoundManager *soundMan, const char *filename, int typeFlags, int priority) {
	_soundMan = soundMan;
	_engine = _soundMan->_engine;

	_engine->_soundMutex->lock();

	memset(_name1, 0, sizeof(_name1));
	memset(_name2, 0, sizeof(_name2));

	_priority = priority;
	setTag(typeFlags);
	setType(typeFlags);
	_soundMan->addSlot(this);

	_engine->_soundMutex->unlock();

	// From this point onwards there could be deadlocks between the main
	// thread and the sound thread because of busy waits in getBuffer(),
	// so we leave the mutex unlocked and let the insides of the function
	// do their job...
	getBuffer();

	// This function will reacquire the mutex...
	load(filename);
}

Slot::Slot(SoundManager *soundMan, int typeFlags, int priority) {
	_soundMan = soundMan;
	_engine = _soundMan->_engine;

	_engine->_soundMutex->lock();

	memset(_name1, 0, sizeof(_name1));
	memset(_name2, 0, sizeof(_name2));

	_priority = priority;
	Common::strcpy_s(_name2, "NISSND?");
	assignDirectVolume(kVolumeFull);
	setTag(typeFlags);
	setType(typeFlags);
	_soundMan->addSlot(this);

	_engine->_soundMutex->unlock();

	// From this point onwards there could be deadlocks between the main
	// thread and the sound thread because of busy waits in getBuffer(),
	// so we leave the mutex unlocked and let the insides of the function
	// do their job...
	getBuffer();
}

Slot::Slot(SoundManager *soundMan, SaveSlot *soundEntry) {
	_soundMan = soundMan;
	_engine = _soundMan->_engine;

	_engine->_soundMutex->lock();

	memset(_name1, 0, sizeof(_name1));
	memset(_name2, 0, sizeof(_name2));

	setBlockCount(soundEntry->blockCount);
	addStatusFlag(soundEntry->status);
	setTag(_statusFlags);

	strncpy(_name1, soundEntry->name1, 15);
	strncpy(_name2, soundEntry->name2, 15);

	Common::String subName = _name2;
	subName.replace('.', '\0');

	setSub(subName.c_str());

	_time = soundEntry->time;
	_fadeDelayCounter = soundEntry->fadeDelayCounter;
	_unusedVar = soundEntry->unusedVar;
	_character = soundEntry->character;
	_delayedStartTime = _soundMan->getSoundDriver30HzCounter() + soundEntry->delayTicks;
	_priority = soundEntry->priority;

	_engine->_soundMutex->unlock();

	// From this point onwards there could be deadlocks between the main
	// thread and the sound thread because of busy waits in devirtualize(),
	// so we must activate the mutex selectively when fetching or updating
	// the class members...
	if ((getStatusFlags() & kSoundFlagPlaying) != 0 &&
		(getStatusFlags() & kSoundFlagMuteProcessed) == 0 &&
		(getStatusFlags() & kSoundVolumeMask) != 0) {
		removeStatusFlag(kSoundFlagPlaying);
		addStatusFlag(kSoundFlagMute);
		devirtualize();
	}

	_soundMan->addSlot(this);
}

Slot::~Slot() {
	addStatusFlag(kSoundFlagCloseRequested);

	while (true) {
		bool mustWait = (getStatusFlags() & kSoundFlagClosed) == 0 &&
			(_soundMan->getSoundDriverFlags() & kSoundDriverClearBufferProcessed) == 0 &&
			(_soundMan->getSoundDriverFlags() & kSoundDriverInitState) != 0;

		if (!mustWait)
			break;

		_engine->handleEvents(); // For good measure...
	}

	_soundMan->removeSlot(this);

	_engine->_soundMutex->lock();

	if (_archive)
		_engine->getArchiveManager()->closeHPF(_archive);

	_engine->_soundMutex->unlock();

	if (getSoundBuffer())
		releaseBuffer();

	if (getSubtitle()) {
		_engine->_soundMutex->lock();

		delete _subtitle;
		_subtitle = nullptr;

		_engine->_soundMutex->unlock();
	}

	if (getAssociatedCharacter()) {
		if (getAssociatedCharacter() == kCharacterSteam) {
			_soundMan->ambientAI(kAmbientSoundSteam);
		} else if (getAssociatedCharacter() != kCharacterClerk) {
			_engine->getLogicManager()->send(kCharacterCath, getAssociatedCharacter(), 2, 0);
		}
	}
}

void Slot::play() {
	if ((_soundMan->getSoundDriverFlags() & kSoundDriverNISHasRequestedFade) != 0) {
		if (getTag() != kSoundTagNIS && getTag() != kSoundTagLink && getTag() != kSoundTagConcert) {
			int volume = getStatusFlags() & kSoundVolumeMask;
			removeStatusFlag(kSoundVolumeMask);
			assignDirectVolume(volume);
			addStatusFlag((volume >> 1) + 1);
		}
	}

	addStatusFlag(kSoundFlagPlayRequested);
}

void Slot::setFade(int volume) {
	Common::StackLock lock(*_engine->_soundMutex);
	if ((_statusFlags & kSoundFlagFading) == 0) {
		int32 targetVolume = volume;
		_statusFlags |= kSoundFlagVolumeChanging;

		if (volume) {
			if ((_soundMan->_soundDriverFlags & kSoundDriverNISHasRequestedFade) != 0) {
				_volume = volume;
				targetVolume = (volume >> 1) + 1;
			}

			_fadeTargetVolume = targetVolume;
		} else {
			_fadeTargetVolume = 0;
			_statusFlags |= kSoundFlagFading;
		}
	}
}

void Slot::setVol(int volume) {
	if (volume) {
		if ((_soundMan->getSoundDriverFlags() & kSoundDriverNISHasRequestedFade) == 0 || getTag() == kSoundTagNIS || getTag() == kSoundTagLink) {
			setStatusFlags(volume + (getStatusFlags() & ~kSoundVolumeMask));
		} else {
			setFade(volume);
		}
	} else {
		assignDirectVolume(0);
		addStatusFlag(kSoundFlagMuteRequested);
		removeStatusFlag(kSoundVolumeMask);
		removeStatusFlag(kSoundFlagVolumeChanging);
	}
}

void Slot::setTag(int typeFlags) {
	Slot *cacheSlot;

	switch (typeFlags & kSoundTypeMask) {
	case kSoundTypeNormal:
		assignDirectTag(_soundMan->_curSoundSlotTag++);
		break;

	case kSoundTypeAmbient:
		cacheSlot = _soundMan->_soundCache;
		if (cacheSlot) {
			do {
				if (cacheSlot->getTag() == kSoundTagOldAmbient)
					break;

				cacheSlot = cacheSlot->getNext();
			} while (cacheSlot);

			if (cacheSlot)
				cacheSlot->setFade(0);
		}

		cacheSlot = _soundMan->_soundCache;
		if (cacheSlot) {
			do {
				if (cacheSlot->getTag() == kSoundTagAmbient)
					break;

				cacheSlot = cacheSlot->getNext();
			} while (cacheSlot);

			if (cacheSlot) {
				cacheSlot->assignDirectTag(kSoundTagOldAmbient);
				cacheSlot->setFade(0);
			}
		}

		assignDirectTag(kSoundTagAmbient);
		break;

	case kSoundTypeMenu:
		cacheSlot = _soundMan->_soundCache;
		if (cacheSlot) {
			do {
				if (cacheSlot->getTag() == kSoundTagMenu)
					break;

				cacheSlot = cacheSlot->getNext();
			} while (cacheSlot);

			if (cacheSlot)
				cacheSlot->assignDirectTag(kSoundTagOldMenu);
		}

		assignDirectTag(kSoundTagMenu);
		break;

	case kSoundTypeLink:
		cacheSlot = _soundMan->_soundCache;
		if (cacheSlot) {
			do {
				if (cacheSlot->getTag() == kSoundTagLink)
					break;

				cacheSlot = cacheSlot->getNext();
			} while (cacheSlot);

			if (cacheSlot)
				cacheSlot->assignDirectTag(kSoundTagOldLink);
		}

		assignDirectTag(kSoundTagLink);
		break;

	case kSoundTypeIntro:
		cacheSlot = _soundMan->_soundCache;
		if (cacheSlot) {
			do {
				if (cacheSlot->getTag() == kSoundTagIntro)
					break;

				cacheSlot = cacheSlot->getNext();
			} while (cacheSlot);

			if (cacheSlot)
				cacheSlot->assignDirectTag(kSoundTagOldMenu);
		}

		assignDirectTag(kSoundTagIntro);
		break;

	case kSoundTypeWalla:
		cacheSlot = _soundMan->_soundCache;
		if (cacheSlot) {
			do {
				if (cacheSlot->getTag() == kSoundTagWalla)
					break;

				cacheSlot = cacheSlot->getNext();
			} while (cacheSlot);

			if (cacheSlot) {
				cacheSlot->assignDirectTag(kSoundTagOldWalla);
				cacheSlot->setFade(0);
			}
		}

		assignDirectTag(kSoundTagWalla);
		break;

	case kSoundTypeNIS:
		cacheSlot = _soundMan->_soundCache;
		if (cacheSlot) {
			do {
				if (cacheSlot->getTag() == kSoundTagNIS)
					break;

				cacheSlot = cacheSlot->getNext();
			} while (cacheSlot);

			if (cacheSlot)
				cacheSlot->assignDirectTag(kSoundTagOldNIS);
		}

		assignDirectTag(kSoundTagNIS);
		break;

	default:
		break;
	}
}

void Slot::setType(int typeFlags) {
	int effFlags = typeFlags;

	if ((typeFlags & kSoundVolumeMask) == 0)
		effFlags |= kSoundFlagMuteRequested;

	if ((effFlags & kSoundFlagLooped) == 0)
		effFlags |= kSoundFlagCloseOnDataEnd;

	addStatusFlag(effFlags);
}

void Slot::setSub(const char *filename) {
	Common::StackLock lock(*_engine->_soundMutex);
	_subtitle = new Subtitle(_engine, filename, this);

	if ((_subtitle->_status & kSubFlagStatusKilled) != 0) {
		delete _subtitle;
		_subtitle = nullptr;
	} else {
		_statusFlags |= kSoundFlagHasSubtitles;
	}
}

bool Slot::getBuffer() {
	int effPrio = 1000;
	int count = 0;
	Slot *cacheSlots;
	Slot *chosenSlot;
	
	if (getSoundBuffer()) {
		return true;
	}

	if (_soundMan->_numActiveChannels >= 6) {
		cacheSlots = _soundMan->_soundCache;

		for (chosenSlot = nullptr; cacheSlots; cacheSlots = cacheSlots->getNext()) {
			if ((cacheSlots->getStatusFlags() & kSoundFlagMute) == 0) {
				if (cacheSlots->getPriority() + (int)(getStatusFlags() & kSoundVolumeMask) < effPrio) {
					chosenSlot = cacheSlots;
					effPrio = cacheSlots->getPriority() + (getStatusFlags() & kSoundVolumeMask);
				}
			}
		}

		if (getPriority() <= effPrio) {
			return false;
		} else {
			if (chosenSlot)
				chosenSlot->virtualize();

			if (chosenSlot) {
				while ((chosenSlot->getStatusFlags() & kSoundFlagMuteProcessed) == 0) {
					_engine->handleEvents();
				}
			}

			if (chosenSlot && chosenSlot->getSoundBuffer())
				chosenSlot->releaseBuffer();

			for (count = 0; _soundMan->_soundSlotChannels[count]; count++) {
				if (count >= ARRAYSIZE(_soundMan->_soundSlotChannels) - 1) {
					return false;
				}
			}

			_soundMan->_soundSlotChannels[count] = this;
			_soundMan->_numActiveChannels++;
			setSoundBuffer(_engine->_soundMemoryPool + 0x16800 * count);

			return true;
		}
	} else {
		for (count = 0; _soundMan->_soundSlotChannels[count]; count++) {
			if (count >= ARRAYSIZE(_soundMan->_soundSlotChannels) - 1) {
				return false;
			}
		}

		_soundMan->_soundSlotChannels[count] = this;
		_soundMan->_numActiveChannels++;
		setSoundBuffer(_engine->_soundMemoryPool + 0x16800 * count);

		return true;
	}
}

void Slot::releaseBuffer() {
	Common::StackLock lock(*_engine->_soundMutex);
	int count = 0;
	int32 size = 0;

	if (getSoundBuffer()) {
		while ((size + _engine->_soundMemoryPool) != getSoundBuffer()) {
			count++;

			size += 0x16800;
			if (size >= 0x87000)
				return;
		}

		if (_soundMan->_soundSlotChannels[count]) {
			_soundMan->_soundSlotChannels[count] = nullptr;
			_soundMan->_numActiveChannels--;
			setSoundBuffer(nullptr);
		}
	}
}

void Slot::virtualize() {
	addStatusFlag(kSoundFlagMuteRequested);
}

void Slot::devirtualize() {
	Slot *i;
	int32 chunkSize;
	int32 diff;
	int32 pagesToRead;

	_engine->_soundMutex->lock();

	if ((_statusFlags & kSoundFlagMuteProcessed) != 0 && getBuffer()) {
		if ((_statusFlags & kSoundFlagLooped) != 0) {
			load(_name2);
			_statusFlags &= ~kSoundFlagMute;
		} else {
			_archive = _engine->getArchiveManager()->openHPF(_name2);
			if (_archive) {
				_statusFlags |= kSoundFlagUnmuteRequested;

				if ((_statusFlags & kSoundFlagPlaying) != 0) {
					_statusFlags |= kSoundFlagPauseRequested;
				} else {
					_statusFlags |= kSoundFlagPaused;
				}

				if ((_statusFlags & kSoundFlagHeaderProcessed) == 0) {
					_statusFlags |= kSoundFlagHeaderProcessed;
					_engine->getArchiveManager()->readHPF(_archive, _soundBuffer, 1);

					_engine->_soundMutex->unlock();

					while ((getStatusFlags() & kSoundFlagPaused) == 0) {
						_engine->handleEvents();
					}

					_engine->_soundMutex->lock();

					_size = READ_LE_UINT32(&_soundBuffer[0]);
					_blockCount = READ_LE_UINT32(&_soundBuffer[4]) - 1;
					_blockCount -= _time;
				}

				_engine->_soundMutex->unlock();

				while ((getStatusFlags() & kSoundFlagPaused) == 0) {
					_engine->handleEvents();
				}

				_engine->_soundMutex->lock();

				chunkSize = 739 * _time + 6;
	
				_dataStart = _soundBuffer;
				_dataEnd = (_dataStart + 0x16000);
				_size = 0x16000;
				_currentDataPtr = &_dataStart[chunkSize & 0x7FF];
				_loadedBytes = chunkSize & 0xFFFF800;
				diff = (_archive->size * PAGE_SIZE) - (chunkSize & 0xFFFF800);

				if (diff <= 0x15800) {
					pagesToRead = (diff + 2047) / PAGE_SIZE;
				} else {
					pagesToRead = 43;
					_statusFlags |= (kSoundFlagCyclicBuffer | kSoundFlagHasUnreadData);
				}

				_engine->getArchiveManager()->seekHPF(_archive, chunkSize / PAGE_SIZE);
				_engine->getArchiveManager()->readHPF(_archive, _soundBuffer, pagesToRead);
				if (_archive->size <= _archive->currentPos) {
					_engine->getArchiveManager()->closeHPF(_archive);
					_statusFlags &= ~kSoundFlagHasUnreadData;
					_archive = 0;
					_loadedBytes = 4;
				}

				_loadedBytes += pagesToRead * PAGE_SIZE;
				_currentBufferPtr = &_soundBuffer[pagesToRead * PAGE_SIZE];
				_statusFlags &= ~(kSoundFlagMute | kSoundFlagPaused | kSoundFlagPauseRequested);
			} else {
				for (i = _soundMan->_soundCache; i; i = i->_next) {
					if (i->_tag == kSoundTagNIS)
						break;
				}

				if (i != this)
					_statusFlags |= kSoundFlagCloseRequested;
			}
		}
	}

	_engine->_soundMutex->unlock();
}

bool Slot::load(const char *filename) {
	Common::StackLock lock(*_engine->_soundMutex);
	_archive = _engine->getArchiveManager()->openHPF(filename);
	Common::strcpy_s(_name2, filename);

	if (!_archive)
		_archive = _engine->getArchiveManager()->openHPF("DEFAULT.SND");

	if (_archive) {
		if (_archive->size > 44 && (_statusFlags & kSoundFlagLooped) != 0) {
			_engine->getArchiveManager()->closeHPF(_archive);
			_archive = nullptr;
			_statusFlags = kSoundFlagCloseRequested;
			_loadedBytes = 5;
		} else {
			if (_soundBuffer) {
				_dataStart = _soundBuffer;
				_currentDataPtr = _soundBuffer + 6;

				if (_archive->size > 44) {
					_engine->getArchiveManager()->readHPF(_archive, _soundBuffer, 44);
					_loadedBytes = 0x16000;
					_size = 0x16000;
					_statusFlags |= (kSoundFlagCloseOnDataEnd | kSoundFlagCyclicBuffer | kSoundFlagHasUnreadData);
					_blockCount = (int32)READ_LE_UINT16(_soundBuffer + 4) - 1;
					_dataEnd = _dataStart + 0x16000;
				} else {
					_engine->getArchiveManager()->readHPF(_archive, _soundBuffer, _archive->size);
					_loadedBytes = _archive->size * PAGE_SIZE;
					_engine->getArchiveManager()->closeHPF(_archive);
					_archive = nullptr;
					_loadedBytes = 6;
					_blockCount = (int32)READ_LE_UINT16(_soundBuffer + 4);
					_size = 739 * _blockCount;
					_dataEnd = &_dataStart[_size + 6];
				}
				
				_currentBufferPtr = _dataStart;
				_statusFlags |= kSoundFlagHeaderProcessed;

				memset(_dataEnd, 0, 739);

				return true;
			} else {
				_size = _archive->size * PAGE_SIZE;
				if (_size > 0x16000)
					_statusFlags |= (kSoundFlagCyclicBuffer | kSoundFlagHasUnreadData);

				_blockCount = (_size + 1018) / 739;
				_statusFlags |= kSoundFlagMute;
				_engine->getArchiveManager()->closeHPF(_archive);
				_archive = nullptr;
				_loadedBytes = 6;

				return true;
			}
		}
	} else {
		_statusFlags = kSoundFlagCloseRequested;
	}

	return false;
}

void Slot::stream() {
	Common::StackLock lock(*_engine->_soundMutex);

	int32 numPages;
	int32 numBytes;

	if ((_statusFlags & kSoundFlagHasUnreadData) != 0) {
		if (_soundBuffer) {
			if ((_statusFlags & kSoundFlagMute) == 0) {
				if (_archive) {	
					numPages = 739 * _time - _loadedBytes + _size - 2054;
					if (numPages > _size)
						numPages = 0;

					numBytes = numPages / PAGE_SIZE;

					if (numBytes > 16) {
						if (_archive->size - _archive->currentPos <= numBytes) {
							_statusFlags &= ~kSoundFlagHasUnreadData;
							numBytes = _archive->size - _archive->currentPos;
						}

						while (numBytes > 0) {
							_engine->getArchiveManager()->readHPF(_archive, _currentBufferPtr, 1);

							_loadedBytes += 0x800;

							if (_dataStart == _currentBufferPtr)
								memcpy(_dataEnd, _currentBufferPtr, 0x800);

							_currentBufferPtr += 0x800;

							if (_currentBufferPtr > _dataEnd)
								_currentBufferPtr = _dataStart;

							if (_currentBufferPtr == _dataEnd)
								_currentBufferPtr = _dataStart;

							numBytes--;
						}

						if ((_statusFlags & kSoundFlagHasUnreadData) == 0) {
							_engine->getArchiveManager()->closeHPF(_archive);
							_archive = nullptr;
							_loadedBytes |= 1;
						}
					}
				}
			}
		}
	}
}

bool Slot::update() {
	Common::StackLock lock(*_engine->_soundMutex);

	if ((_statusFlags & kSoundFlagClosed) != 0)
		return false;

	if ((_statusFlags & kSoundFlagDelayedActivate) != 0) {
		if (_delayedStartTime <= _soundMan->_sound30HzCounter) {
			_statusFlags |= kSoundFlagPlayRequested;
			_statusFlags &= ~kSoundFlagDelayedActivate;

			Common::String subName = _name2;
			subName.replace('.', '\0');
			setSub(subName.c_str());
		}
	} else {
		if ((_soundMan->_soundDriverFlags & kSoundDriverNISHasRequestedFade) == 0 && (_statusFlags & kSoundFlagFixedVolume) == 0) {
			if (_character > 0 && _character < 128) {
				setVol(_engine->getLogicManager()->getVolume(_character));
			}
		}

		if ((_statusFlags & kSoundFlagHasUnreadData) != 0 && (_statusFlags & kSoundFlagMute) == 0 && _soundBuffer)
			stream();
	}

	return true;
}

int32 Slot::getStatusFlags() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _statusFlags;
}

void Slot::setStatusFlags(int32 flags) {
	Common::StackLock lock(*_engine->_soundMutex);
	_statusFlags = flags;
}

void Slot::addStatusFlag(int32 flag) {
	Common::StackLock lock(*_engine->_soundMutex);
	_statusFlags |= flag;
}

void Slot::removeStatusFlag(int32 flag) {
	Common::StackLock lock(*_engine->_soundMutex);
	_statusFlags &= ~flag;
}

int32 Slot::getTag() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _tag;
}

bool Slot::hasTag(int32 tag) {
	Common::StackLock lock(*_engine->_soundMutex);
	return _tag == tag;
}

void Slot::assignDirectTag(int32 tag) {
	Common::StackLock lock(*_engine->_soundMutex);
	_tag = tag;
}

int Slot::getAssociatedCharacter() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _character;
}

void Slot::setAssociatedCharacter(int character) {
	Common::StackLock lock(*_engine->_soundMutex);
	_character = character;
}

int32 Slot::getTime() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _time;
}

Slot *Slot::getNext() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _next;
}

void Slot::setNext(Slot *next) {
	Common::StackLock lock(*_engine->_soundMutex);
	_next = next;
}

int Slot::getPriority() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _priority;
}

bool Slot::closeArchive() {
	Common::StackLock lock(*_engine->_soundMutex);
	bool hadArchive = false;

	if (_archive) {
		_engine->getArchiveManager()->closeHPF(_archive);
		hadArchive = true;
	}

	_archive = nullptr;

	return hadArchive;
}

void Slot::assignDirectVolume(int volume) {
	Common::StackLock lock(*_engine->_soundMutex);
	_volume = volume;
}

int Slot::getVolume() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _volume;
}

void Slot::setChainedSound(Slot *chainedSound) {
	Common::StackLock lock(*_engine->_soundMutex);
	_chainedSound = chainedSound;
}

Subtitle *Slot::getSubtitle() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _subtitle;
}

int32 Slot::getSize() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _size;
}

void Slot::setSize(int32 size) {
	Common::StackLock lock(*_engine->_soundMutex);
	_size = size;
}

int32 Slot::getNumLoadedBytes() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _loadedBytes;
}

void Slot::setNumLoadedBytes(int32 bytes) {
	Common::StackLock lock(*_engine->_soundMutex);
	_loadedBytes = bytes;
}

void Slot::advanceLoadedBytesBy(int32 loadedBytes) {
	Common::StackLock lock(*_engine->_soundMutex);
	_loadedBytes += loadedBytes;
}

byte *Slot::getDataStart() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _dataStart;
}

void Slot::setDataStart(byte *dataStart) {
	Common::StackLock lock(*_engine->_soundMutex);
	_dataStart = dataStart;
}

byte *Slot::getDataEnd() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _dataEnd;
}

void Slot::setDataEnd(byte *dataEnd) {
	Common::StackLock lock(*_engine->_soundMutex);
	_dataEnd = dataEnd;
}

void Slot::setDataEndByte(int32 pos, byte value) {
	Common::StackLock lock(*_engine->_soundMutex);
	_dataEnd[pos] = value;
}

void Slot::advanceDataEndBy(int32 size) {
	Common::StackLock lock(*_engine->_soundMutex);
	_dataEnd += size;
}

byte *Slot::getSoundBuffer() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _soundBuffer;
}

void Slot::setSoundBuffer(byte *bufferPtr) {
	Common::StackLock lock(*_engine->_soundMutex);
	_soundBuffer = bufferPtr;
}

byte *Slot::getCurrentBufferPtr() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _currentBufferPtr;
}

void Slot::advanceCurrentBufferPtrBy(int32 size) {
	Common::StackLock lock(*_engine->_soundMutex);
	_currentBufferPtr += size;
}

void Slot::setCurrentBufferPtr(byte *bufferPtr) {
	Common::StackLock lock(*_engine->_soundMutex);
	_currentBufferPtr = bufferPtr;
}

byte *Slot::getCurrentDataPtr() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _currentDataPtr;
}

void Slot::setCurrentDataPtr(byte *dataPtr) {
	Common::StackLock lock(*_engine->_soundMutex);
	_currentDataPtr = dataPtr;
}

int32 Slot::getBlockCount() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _blockCount;
}

void Slot::setBlockCount(int32 blockCount) {
	Common::StackLock lock(*_engine->_soundMutex);
	_blockCount = blockCount;
}

void Slot::setDelayedStartTime(int32 time) {
	Common::StackLock lock(*_engine->_soundMutex);
	_delayedStartTime = time;
}

char *Slot::getName2() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _name2;
}

} // End of namespace LastExpress
