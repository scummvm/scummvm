/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/util.h"
#include "sound/voc.h"
#include "scumm/scumm.h"
#include "scumm/imuse_digi/dimuse_sndmgr.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

namespace Scumm {

ImuseDigiSndMgr::ImuseDigiSndMgr(ScummEngine *scumm) {
	memset(&_sounds, 0, sizeof(_sounds));
	_scumm = scumm;
	_mutex = g_system->create_mutex();
	_disk = 0;
	_cacheBundleDir = new BundleDirCache();
	BundleCodecs::initializeImcTables();
}

ImuseDigiSndMgr::~ImuseDigiSndMgr() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (&_sounds[l])
			closeSound(&_sounds[l]);
	}
	delete _cacheBundleDir;
#ifdef __PALM_OS__
	BundleCodecs::releaseImcTables();
#endif
}

void ImuseDigiSndMgr::prepareSound(byte *ptr, int slot) {
	if (READ_UINT32(ptr) == MKID('Crea')) {
		int size = 0, rate = 0, loops = 0;
		_sounds[slot].resPtr = readVOCFromMemory(ptr, size, rate, loops);
		_sounds[slot].freeResPtr = true;
		_sounds[slot].bits = 8;
		_sounds[slot].freq = rate;
		_sounds[slot].channels = 1;
		_sounds[slot].region[0].length = size;
		_sounds[slot].numRegions++;
		if (loops != 0)
			_sounds[slot].numJumps++;
	} else if (READ_UINT32(ptr) == MKID('iMUS')) {
		uint32 tag;
		int32 size = 0;
		byte *s_ptr = ptr;

		ptr += 16;
		do {
			tag = READ_BE_UINT32(ptr); ptr += 4;
			switch(tag) {
			case MKID_BE('FRMT'):
				ptr += 12;
				_sounds[slot].bits = READ_BE_UINT32(ptr); ptr += 4;
				_sounds[slot].freq = READ_BE_UINT32(ptr); ptr += 4;
				_sounds[slot].channels = READ_BE_UINT32(ptr); ptr += 4;
			break;
			case MKID_BE('TEXT'):
				size = READ_BE_UINT32(ptr); ptr += 4;
				if (_sounds[slot].numMarkers >= MAX_IMUSE_MARKERS) {
					warning("ImuseDigiSndMgr::prepareSound(%s) Not enough space for Marker", _sounds[slot].name);
					ptr += size;
					break;
				}
				strcpy(_sounds[slot].marker[_sounds[slot].numMarkers].name, (char *)ptr + 4);
				_sounds[slot].numMarkers++;
				ptr += size;
				break;
			case MKID_BE('REGN'):
				size = READ_BE_UINT32(ptr); ptr += 4;
				if (_sounds[slot].numRegions >= MAX_IMUSE_REGIONS) {
					warning("ImuseDigiSndMgr::prepareSound(%s) Not enough space for Region", _sounds[slot].name);
					ptr += 8;
					break;
				}
				_sounds[slot].region[_sounds[slot].numRegions].offset = READ_BE_UINT32(ptr); ptr += 4;
				_sounds[slot].region[_sounds[slot].numRegions].length = READ_BE_UINT32(ptr); ptr += 4;
				_sounds[slot].numRegions++;
				break;
			case MKID_BE('STOP'):
				ptr += 4;
				_sounds[slot].offsetStop = READ_BE_UINT32(ptr); ptr += 4;
				break;
			case MKID_BE('JUMP'):
				size = READ_BE_UINT32(ptr); ptr += 4;
				if (_sounds[slot].numJumps >= MAX_IMUSE_JUMPS) {
					warning("ImuseDigiSndMgr::prepareSound(%s) Not enough space for Jump", _sounds[slot].name);
					ptr += size;
					break;
				}
				_sounds[slot].jump[_sounds[slot].numJumps].offset = READ_BE_UINT32(ptr); ptr += 4;
				_sounds[slot].jump[_sounds[slot].numJumps].dest = READ_BE_UINT32(ptr); ptr += 4;
				_sounds[slot].jump[_sounds[slot].numJumps].hookId = READ_BE_UINT32(ptr); ptr += 4;
				_sounds[slot].jump[_sounds[slot].numJumps].fadeDelay = READ_BE_UINT32(ptr); ptr += 4;
				_sounds[slot].numJumps++;
				break;
			case MKID_BE('SYNC'):
				size = READ_BE_UINT32(ptr); ptr += size + 4;
				break;
			case MKID_BE('DATA'):
				size = READ_BE_UINT32(ptr); ptr += 4;
				break;
			default:
				error("ImuseDigiSndMgr::prepareSound(%s) Unknown sfx header '%s'",  _sounds[slot].name, tag2str(tag));
			}
		} while (tag != MKID_BE('DATA'));
		_sounds[slot].offsetData =  ptr - s_ptr;
	} else {
		error("ImuseDigiSndMgr::prepareSound(): Unknown sound format");
	}
}

int ImuseDigiSndMgr::allocSlot() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (!_sounds[l].inUse) {
			_sounds[l].inUse = true;
			return l;
		}
	}

	return -1;
}

bool ImuseDigiSndMgr::openMusicBundle(int slot) {
	bool result = false;

	_sounds[slot]._bundle = new BundleMgr(_cacheBundleDir);
	if (_scumm->_gameId == GID_CMI) {
		if (_scumm->_features & GF_DEMO) {
			result = _sounds[slot]._bundle->openFile("music.bun", _scumm->getGameDataPath());
		} else {
			char musicfile[20];
			sprintf(musicfile, "musdisk%d.bun", _scumm->VAR(_scumm->VAR_CURRENTDISK));
			if (_disk != _scumm->VAR(_scumm->VAR_CURRENTDISK))
				_sounds[slot]._bundle->closeFile();

			result = _sounds[slot]._bundle->openFile(musicfile, _scumm->getGameDataPath());

			if (result == false)
				result = _sounds[slot]._bundle->openFile("music.bun", _scumm->getGameDataPath());
			_disk = (byte)_scumm->VAR(_scumm->VAR_CURRENTDISK);
		}
	} else if (_scumm->_gameId == GID_DIG)
		result = _sounds[slot]._bundle->openFile("digmusic.bun", _scumm->getGameDataPath());
	else
		error("ImuseDigiSndMgr::openMusicBundle() Don't know which bundle file to load");

	return result;
}

bool ImuseDigiSndMgr::openVoiceBundle(int slot) {
	bool result = false;

	_sounds[slot]._bundle = new BundleMgr(_cacheBundleDir);
	if (_scumm->_gameId == GID_CMI) {
		if (_scumm->_features & GF_DEMO) {
			result = _sounds[slot]._bundle->openFile("voice.bun", _scumm->getGameDataPath());
		} else {
			char voxfile[20];
			sprintf(voxfile, "voxdisk%d.bun", _scumm->VAR(_scumm->VAR_CURRENTDISK));
			if (_disk != _scumm->VAR(_scumm->VAR_CURRENTDISK))
				_sounds[slot]._bundle->closeFile();

			result = _sounds[slot]._bundle->openFile(voxfile, _scumm->getGameDataPath());

			if (result == false)
				result = _sounds[slot]._bundle->openFile("voice.bun", _scumm->getGameDataPath());
			_disk = (byte)_scumm->VAR(_scumm->VAR_CURRENTDISK);
		}
	} else if (_scumm->_gameId == GID_DIG)
		result = _sounds[slot]._bundle->openFile("digvoice.bun", _scumm->getGameDataPath());
	else
		error("ImuseDigiSndMgr::openVoiceBundle() Don't know which bundle file to load");

	return result;
}

ImuseDigiSndMgr::soundStruct *ImuseDigiSndMgr::openSound(int32 soundId, const char *soundName, int soundType, int soundGroup) {
	assert(soundId >= 0);
	assert(soundType);
	Common::StackLock tmpLock(_mutex);

	int slot = allocSlot();
	if (slot == -1) {
		error("ImuseDigiSndMgr::openSound() can't alloc free sound slot");
	}

	bool result = false;
	byte *ptr = NULL;

	if (soundName == NULL) {
		if ((soundType == IMUSE_RESOURCE)) {
			ptr = _scumm->getResourceAddress(rtSound, soundId);
			if (ptr == NULL) {
				closeSound(&_sounds[slot]);
				return NULL;
			}
			_sounds[slot].resPtr = ptr;
			result = true;
		} else if (soundType == IMUSE_BUNDLE) {
			if (soundGroup == IMUSE_VOICE)
				result = openVoiceBundle(slot);
			else if (soundGroup == IMUSE_MUSIC)
				result = openMusicBundle(slot);
			else 
				error("ImuseDigiSndMgr::openSound() Don't know how load sound: %d", soundId);
			_sounds[slot]._bundle->decompressSampleByIndex(soundId, 0, 0x2000, &ptr, 0);
			strcpy(_sounds[slot].name, soundName);
		} else {
			error("ImuseDigiSndMgr::openSound() Don't know how load sound: %d", soundId);
		}
	} else if (soundName != NULL) {
		if (soundType == IMUSE_BUNDLE) {
			if (soundGroup == IMUSE_VOICE)
				result = openVoiceBundle(slot);
			else if (soundGroup == IMUSE_MUSIC)
				result = openMusicBundle(slot);
			else 
				error("ImuseDigiSndMgr::openSound() Don't know how load sound: %d", soundId);
			_sounds[slot]._bundle->decompressSampleByName(soundName, 0, 0x2000, &ptr);
			_sounds[slot].name[0] = 0;
		} else {
			error("ImuseDigiSndMgr::openSound() Don't know how load sound: %s", soundName);
		}
	}	

	if (result) {
		if (ptr == NULL) {
			closeSound(&_sounds[slot]);
			return NULL;
		}
		prepareSound(ptr, slot);
		return &_sounds[slot];
	}

	return NULL;
}

void ImuseDigiSndMgr::closeSound(soundStruct *soundHandle) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	Common::StackLock tmpLock(_mutex);

	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (&_sounds[l] == soundHandle) {
			if (_sounds[l].freeResPtr)
				free(_sounds[l].resPtr);
			if (_sounds[l]._bundle)
				delete _sounds[l]._bundle;
			memset(&_sounds[l], 0, sizeof(soundStruct));
		}
	}
}

bool ImuseDigiSndMgr::checkForProperHandle(soundStruct *soundHandle) {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (soundHandle == &_sounds[l])
			return true;
	}
	return false;
}

int ImuseDigiSndMgr::getFreq(soundStruct *soundHandle) {
	Common::StackLock tmpLock(_mutex);
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->freq;
}

int ImuseDigiSndMgr::getBits(soundStruct *soundHandle) {
	Common::StackLock tmpLock(_mutex);
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->bits;
}

int ImuseDigiSndMgr::getChannels(soundStruct *soundHandle) {
	Common::StackLock tmpLock(_mutex);
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->channels;
}

bool ImuseDigiSndMgr::isEndOfRegion(soundStruct *soundHandle, int region) {
	Common::StackLock tmpLock(_mutex);
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(region >= 0 && region < soundHandle->numRegions);
	return soundHandle->endFlag;
}

int ImuseDigiSndMgr::getNumRegions(soundStruct *soundHandle) {
	Common::StackLock tmpLock(_mutex);
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->numRegions;
}

int ImuseDigiSndMgr::getNumJumps(soundStruct *soundHandle) {
	Common::StackLock tmpLock(_mutex);
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->numJumps;
}

int ImuseDigiSndMgr::getNumMarkers(soundStruct *soundHandle) {
	Common::StackLock tmpLock(_mutex);
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->numMarkers;
}

int ImuseDigiSndMgr::getJumpIdByRegion(soundStruct *soundHandle, int number) {
	Common::StackLock tmpLock(_mutex);
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numRegions);
	for (int l = 0; l < soundHandle->numJumps; l++) {
		if (soundHandle->jump[number].offset == soundHandle->region[l].offset) {
			return l;
		}
	}
	
	return -1;
}

int ImuseDigiSndMgr::getJumpDestRegionId(soundStruct *soundHandle, int number) {
	Common::StackLock tmpLock(_mutex);
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numJumps);
	for (int l = 0; l < soundHandle->numRegions; l++) {
		if (soundHandle->jump[number].dest == soundHandle->region[l].offset) {
			return l;
		}
	}

	return -1;
}

int ImuseDigiSndMgr::getJumpHookId(soundStruct *soundHandle, int number) {
	Common::StackLock tmpLock(_mutex);
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numJumps);
	return soundHandle->jump[number].hookId;
}

int ImuseDigiSndMgr::getJumpFade(soundStruct *soundHandle, int number) {
	Common::StackLock tmpLock(_mutex);
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numJumps);
	return soundHandle->jump[number].fadeDelay;
}

char *ImuseDigiSndMgr::getMarker(soundStruct *soundHandle, int number) {
	Common::StackLock tmpLock(_mutex);
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numMarkers);
	return (char *)(soundHandle->marker[number].name);
}

int32 ImuseDigiSndMgr::getDataFromRegion(soundStruct *soundHandle, int region, byte **buf, int32 offset, int32 size) {
	Common::StackLock tmpLock(_mutex);
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(buf && offset >= 0 && size >= 0);
	assert(region >= 0 && region < soundHandle->numRegions);

	int32 region_offset = soundHandle->region[region].offset;
	int32 region_length = soundHandle->region[region].length;
	int32 offset_data = soundHandle->offsetData;
	int32 start = region_offset - offset_data;

	if (offset + size + offset_data > region_length) {
		size = region_length - offset;
		soundHandle->endFlag = true;
	} else {
		soundHandle->endFlag = false;
	}

	int header_size = soundHandle->offsetData;

	if (soundHandle->_bundle) {
		size = soundHandle->_bundle->decompressSampleByCurIndex(start + offset, size, buf, header_size);
	} else if (soundHandle->resPtr) {
		*buf = (byte *)malloc(size);
		memcpy(*buf, soundHandle->resPtr + start + offset + header_size, size);
	}
	
	return size;
}

} // End of namespace Scumm
