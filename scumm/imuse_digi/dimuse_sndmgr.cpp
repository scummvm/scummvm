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
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		memset(&_sounds[l], 0, sizeof(soundStruct));
	}
	_vm = scumm;
	_disk = 0;
	_cacheBundleDir = new BundleDirCache();
	BundleCodecs::initializeImcTables();
}

ImuseDigiSndMgr::~ImuseDigiSndMgr() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		closeSound(&_sounds[l]);
	}
#ifdef __PALM_OS__
	BundleCodecs::releaseImcTables();
#endif
}

void ImuseDigiSndMgr::countElements(byte *ptr, int &numRegions, int &numJumps, int &numSyncs) {
	uint32 tag;
	int32 size = 0;

	do {
		tag = READ_BE_UINT32(ptr); ptr += 4;
		switch(tag) {
		case MKID_BE('TEXT'):
		case MKID_BE('STOP'):
		case MKID_BE('FRMT'):
		case MKID_BE('DATA'):
			size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;
		case MKID_BE('REGN'):
			numRegions++;
			size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;
		case MKID_BE('JUMP'):
			numJumps++;
			size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;
		case MKID_BE('SYNC'):
			numSyncs++;
			size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;
		default:
			error("ImuseDigiSndMgr::countElements() Unknown sfx header '%s'", tag2str(tag));
		}
	} while (tag != MKID_BE('DATA'));
}

void ImuseDigiSndMgr::prepareSound(byte *ptr, int slot) {
	if (READ_UINT32(ptr) == MKID('Crea')) {
		bool quit = false;

		int32 offset = READ_LE_UINT16(ptr + 20);
		int16 version = READ_LE_UINT16(ptr + 22);
		int16 code = READ_LE_UINT16(ptr + 24);
		assert(version == 0x010A || version == 0x0114);
		assert(code == ~version + 0x1234);

		_sounds[slot].region = (_region *)malloc(sizeof(_region) * 4);
		_sounds[slot].jump = (_jump *)malloc(sizeof(_jump));
		_sounds[slot].resPtr = ptr;
		_sounds[slot].bits = 8;
		_sounds[slot].channels = 1;

		while (!quit) {
			int len = READ_LE_UINT32(ptr + offset);
			offset += 4;
			code = len & 0xFF;
			len >>= 8;
			switch(code) {
			case 0:
				quit = true;
				break;
			case 1:
				{
					int time_constant = ptr[offset];
					offset += 2;
					len -= 2;
					_sounds[slot].freq = getSampleRateFromVOCRate(time_constant);
					_sounds[slot].region[_sounds[slot].numRegions].offset = offset;
					_sounds[slot].region[_sounds[slot].numRegions].length = len;
					_sounds[slot].numRegions++;
				}
				break;
			case 6:	// begin of loop
				_sounds[slot].jump[0].dest = offset + 8;
				_sounds[slot].jump[0].hookId = 0;
				_sounds[slot].jump[0].fadeDelay = 0;
				break;
			case 7:	// end of loop
				_sounds[slot].jump[0].offset = offset - 4;
				_sounds[slot].numJumps++;
				_sounds[slot].region[_sounds[slot].numRegions].offset = offset - 4;
				_sounds[slot].region[_sounds[slot].numRegions].length = 0;
				_sounds[slot].numRegions++;
				break;
			default:
				error("Invalid code in VOC file : %d", code);
				quit = true;
				break;
			}
			// FIXME some FT samples (ex. 362) has bad length, 2 bytes too short
			offset += len;
		}
	} else if (READ_UINT32(ptr) == MKID('iMUS')) {
		uint32 tag;
		int32 size = 0;
		byte *s_ptr = ptr;
		ptr += 16;

		int curIndexRegion = 0;
		int curIndexJump = 0;
		int curIndexSync = 0;

		_sounds[slot].numRegions = 0;
		_sounds[slot].numJumps = 0;
		_sounds[slot].numSyncs = 0;
		countElements(ptr, _sounds[slot].numRegions, _sounds[slot].numJumps, _sounds[slot].numSyncs);
		_sounds[slot].region = (_region *)malloc(sizeof(_region) * _sounds[slot].numRegions);
		_sounds[slot].jump = (_jump *)malloc(sizeof(_jump) * _sounds[slot].numJumps);
		_sounds[slot].sync = (_sync *)malloc(sizeof(_sync) * _sounds[slot].numSyncs);

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
			case MKID_BE('STOP'):
				size = READ_BE_UINT32(ptr); ptr += size + 4;
				break;
			case MKID_BE('REGN'):
				ptr += 4;
				_sounds[slot].region[curIndexRegion].offset = READ_BE_UINT32(ptr); ptr += 4;
				_sounds[slot].region[curIndexRegion].length = READ_BE_UINT32(ptr); ptr += 4;
				curIndexRegion++;
				break;
			case MKID_BE('JUMP'):
				ptr += 4;
				_sounds[slot].jump[curIndexJump].offset = READ_BE_UINT32(ptr); ptr += 4;
				_sounds[slot].jump[curIndexJump].dest = READ_BE_UINT32(ptr); ptr += 4;
				_sounds[slot].jump[curIndexJump].hookId = READ_BE_UINT32(ptr); ptr += 4;
				_sounds[slot].jump[curIndexJump].fadeDelay = READ_BE_UINT32(ptr); ptr += 4;
				curIndexJump++;
				break;
			case MKID_BE('SYNC'):
				size = READ_BE_UINT32(ptr); ptr += 4;
				_sounds[slot].sync[curIndexSync].size = size;
				_sounds[slot].sync[curIndexSync].ptr = (byte *)malloc(size);
				memcpy(_sounds[slot].sync[curIndexSync].ptr, ptr, size);
				curIndexSync++;
				ptr += size;
				break;
			case MKID_BE('DATA'):
				ptr += 4;
				break;
			default:
				error("ImuseDigiSndMgr::prepareSound(%d/%s) Unknown sfx header '%s'", _sounds[slot].soundId, _sounds[slot].name, tag2str(tag));
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

	_sounds[slot].bundle = new BundleMgr(_cacheBundleDir);
	if (_vm->_gameId == GID_CMI) {
		if (_vm->_features & GF_DEMO) {
			result = _sounds[slot].bundle->openFile("music.bun", _vm->getGameDataPath());
		} else {
			char musicfile[20];
			sprintf(musicfile, "musdisk%d.bun", _vm->VAR(_vm->VAR_CURRENTDISK));
			if (_disk != _vm->VAR(_vm->VAR_CURRENTDISK))
				_sounds[slot].bundle->closeFile();

			result = _sounds[slot].bundle->openFile(musicfile, _vm->getGameDataPath());

			if (result == false)
				result = _sounds[slot].bundle->openFile("music.bun", _vm->getGameDataPath());
			_disk = (byte)_vm->VAR(_vm->VAR_CURRENTDISK);
		}
	} else if (_vm->_gameId == GID_DIG)
		result = _sounds[slot].bundle->openFile("digmusic.bun", _vm->getGameDataPath());
	else
		error("ImuseDigiSndMgr::openMusicBundle() Don't know which bundle file to load");

	return result;
}

bool ImuseDigiSndMgr::openVoiceBundle(int slot) {
	bool result = false;

	_sounds[slot].bundle = new BundleMgr(_cacheBundleDir);
	if (_vm->_gameId == GID_CMI) {
		if (_vm->_features & GF_DEMO) {
			result = _sounds[slot].bundle->openFile("voice.bun", _vm->getGameDataPath());
		} else {
			char voxfile[20];
			sprintf(voxfile, "voxdisk%d.bun", _vm->VAR(_vm->VAR_CURRENTDISK));
			if (_disk != _vm->VAR(_vm->VAR_CURRENTDISK))
				_sounds[slot].bundle->closeFile();

			result = _sounds[slot].bundle->openFile(voxfile, _vm->getGameDataPath());

			if (result == false)
				result = _sounds[slot].bundle->openFile("voice.bun", _vm->getGameDataPath());
			_disk = (byte)_vm->VAR(_vm->VAR_CURRENTDISK);
		}
	} else if (_vm->_gameId == GID_DIG)
		result = _sounds[slot].bundle->openFile("digvoice.bun", _vm->getGameDataPath());
	else
		error("ImuseDigiSndMgr::openVoiceBundle() Don't know which bundle file to load");

	return result;
}

ImuseDigiSndMgr::soundStruct *ImuseDigiSndMgr::openSound(int32 soundId, const char *soundName, int soundType, int volGroupId) {
	assert(soundId >= 0);
	assert(soundType);

	int slot = allocSlot();
	if (slot == -1) {
		error("ImuseDigiSndMgr::openSound() can't alloc free sound slot");
	}

	bool result = false;
	byte *ptr = NULL;

	if (soundName == NULL) {
		if ((soundType == IMUSE_RESOURCE)) {
			ptr = _vm->getResourceAddress(rtSound, soundId);
			if (ptr == NULL) {
				closeSound(&_sounds[slot]);
				return NULL;
			}
			_sounds[slot].resPtr = ptr;
			result = true;
		} else if (soundType == IMUSE_BUNDLE) {
			bool header_outside = ((_vm->_gameId == GID_CMI) && !(_vm->_features & GF_DEMO));
			if (volGroupId == IMUSE_VOLGRP_VOICE)
				result = openVoiceBundle(slot);
			else if (volGroupId == IMUSE_VOLGRP_MUSIC)
				result = openMusicBundle(slot);
			else
				error("ImuseDigiSndMgr::openSound() Don't know how load sound: %d", soundId);
			_sounds[slot].bundle->decompressSampleByIndex(soundId, 0, 0x2000, &ptr, 0, header_outside);
			_sounds[slot].name[0] = 0;
			_sounds[slot].soundId = soundId;
		} else {
			error("ImuseDigiSndMgr::openSound() Don't know how load sound: %d", soundId);
		}
	} else {
		if (soundType == IMUSE_BUNDLE) {
			bool header_outside = ((_vm->_gameId == GID_CMI) && !(_vm->_features & GF_DEMO));
			if (volGroupId == IMUSE_VOLGRP_VOICE)
				result = openVoiceBundle(slot);
			else if (volGroupId == IMUSE_VOLGRP_MUSIC)
				result = openMusicBundle(slot);
			else
				error("ImuseDigiSndMgr::openSound() Don't know how load sound: %d", soundId);
			_sounds[slot].bundle->decompressSampleByName(soundName, 0, 0x2000, &ptr, header_outside);
			strcpy(_sounds[slot].name, soundName);
			_sounds[slot].soundId = soundId;
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

	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (&_sounds[l] == soundHandle) {
			if (_sounds[l].bundle)
				delete _sounds[l].bundle;
			for (int r = 0; r < _sounds[l].numSyncs; r++)
				if (_sounds[l].sync[r].ptr)
					free(_sounds[l].sync[r].ptr);
			if (_sounds[l].region)
					free(_sounds[l].region);
			if (_sounds[l].jump)
					free(_sounds[l].jump);
			if (_sounds[l].sync)
					free(_sounds[l].sync);
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
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->freq;
}

int ImuseDigiSndMgr::getBits(soundStruct *soundHandle) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->bits;
}

int ImuseDigiSndMgr::getChannels(soundStruct *soundHandle) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->channels;
}

bool ImuseDigiSndMgr::isEndOfRegion(soundStruct *soundHandle, int region) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(region >= 0 && region < soundHandle->numRegions);
	return soundHandle->endFlag;
}

int ImuseDigiSndMgr::getNumRegions(soundStruct *soundHandle) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->numRegions;
}

int ImuseDigiSndMgr::getNumJumps(soundStruct *soundHandle) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->numJumps;
}

int ImuseDigiSndMgr::getRegionOffset(soundStruct *soundHandle, int region) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(region >= 0 && region < soundHandle->numRegions);
	return soundHandle->region[region].offset;
}

int ImuseDigiSndMgr::getJumpIdByRegion(soundStruct *soundHandle, int region) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(region >= 0 && region < soundHandle->numRegions);
	for (int l = 0; l < soundHandle->numJumps; l++) {
		if (soundHandle->jump[l].offset == soundHandle->region[region].offset) {
			return l;
		}
	}
	
	return -1;
}

void ImuseDigiSndMgr::getSyncSizeAndPtrById(soundStruct *soundHandle, int number, int32 &sync_size, byte **sync_ptr) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(number >= 0);
	if (number < soundHandle->numSyncs) {
		sync_size = soundHandle->sync[number].size;
		*sync_ptr = soundHandle->sync[number].ptr;
	} else {
		sync_size = 0;
		*sync_ptr = NULL;
	}
}

int ImuseDigiSndMgr::getRegionIdByJumpId(soundStruct *soundHandle, int jumpId) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(jumpId >= 0 && jumpId < soundHandle->numJumps);
	for (int l = 0; l < soundHandle->numRegions; l++) {
		if (soundHandle->jump[jumpId].dest == soundHandle->region[l].offset) {
			return l;
		}
	}

	return -1;
}

int ImuseDigiSndMgr::getJumpHookId(soundStruct *soundHandle, int number) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numJumps);
	return soundHandle->jump[number].hookId;
}

int ImuseDigiSndMgr::getJumpFade(soundStruct *soundHandle, int number) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numJumps);
	return soundHandle->jump[number].fadeDelay;
}

int32 ImuseDigiSndMgr::getDataFromRegion(soundStruct *soundHandle, int region, byte **buf, int32 offset, int32 size) {
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
	bool header_outside = ((_vm->_gameId == GID_CMI) && !(_vm->_features & GF_DEMO));
	if (soundHandle->bundle) {
		size = soundHandle->bundle->decompressSampleByCurIndex(start + offset, size, buf, header_size, header_outside);
	} else if (soundHandle->resPtr) {
		*buf = (byte *)malloc(size);
		memcpy(*buf, soundHandle->resPtr + start + offset + header_size, size);
	}
	
	return size;
}

} // End of namespace Scumm
