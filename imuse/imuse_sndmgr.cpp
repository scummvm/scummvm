// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "../stdafx.h"
#include "../bits.h"
#include "../debug.h"
#include "../timer.h"
#include "../resource.h"

#include "../mixer/mixer.h"
#include "../mixer/audiostream.h"

#include "imuse_sndmgr.h"

ImuseSndMgr::ImuseSndMgr(ScummEngine *scumm) {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		memset(&_sounds[l], 0, sizeof(soundStruct));
	}
	_vm = scumm;
	_disk = 0;
}

ImuseSndMgr::~ImuseSndMgr() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		closeSound(&_sounds[l]);
	}
}

void ImuseSndMgr::countElements(byte *ptr, int &numRegions, int &numJumps) {
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
		default:
			error("ImuseSndMgr::countElements() Unknown sfx header '%s'", tag2str(tag));
		}
	} while (tag != MKID_BE('DATA'));
}

void ImuseSndMgr::prepareSound(byte *ptr, soundStruct *sound) {
//		numBlocks = READ_BE_UINT16(data + 4);
//		codecsStart = data + 8 + numBlocks * 9;
//		codecsLen = READ_BE_UINT16(codecsStart - 2);
//		headerPos = codecsStart + codecsLen;
//		_numChannels = READ_LE_UINT16(headerPos + 22);
//		dataStart = headerPos + 28 + READ_LE_UINT32(headerPos + 16);
//		dataSize = READ_LE_UINT32(dataStart - 4);
	if (READ_UINT32(ptr) == MKID('iMUS')) {
		uint32 tag;
		int32 size = 0;
		byte *s_ptr = ptr;
		ptr += 16;

		int curIndexRegion = 0;
		int curIndexJump = 0;

		sound->numRegions = 0;
		sound->numJumps = 0;
		countElements(ptr, sound->numRegions, sound->numJumps, sound->numSyncs);
		sound->region = (_region *)malloc(sizeof(_region) * sound->numRegions);
		sound->jump = (_jump *)malloc(sizeof(_jump) * sound->numJumps);

		do {
			tag = READ_BE_UINT32(ptr); ptr += 4;
			switch(tag) {
			case MKID_BE('FRMT'):
				ptr += 12;
				sound->bits = READ_BE_UINT32(ptr); ptr += 4;
				sound->freq = READ_BE_UINT32(ptr); ptr += 4;
				sound->channels = READ_BE_UINT32(ptr); ptr += 4;
				break;
			case MKID_BE('TEXT'):
			case MKID_BE('STOP'):
				size = READ_BE_UINT32(ptr); ptr += size + 4;
				break;
			case MKID_BE('REGN'):
				ptr += 4;
				sound->region[curIndexRegion].offset = READ_BE_UINT32(ptr); ptr += 4;
				sound->region[curIndexRegion].length = READ_BE_UINT32(ptr); ptr += 4;
				curIndexRegion++;
				break;
			case MKID_BE('JUMP'):
				ptr += 4;
				sound->jump[curIndexJump].offset = READ_BE_UINT32(ptr); ptr += 4;
				sound->jump[curIndexJump].dest = READ_BE_UINT32(ptr); ptr += 4;
				sound->jump[curIndexJump].hookId = READ_BE_UINT32(ptr); ptr += 4;
				sound->jump[curIndexJump].fadeDelay = READ_BE_UINT32(ptr); ptr += 4;
				curIndexJump++;
				break;
			case MKID_BE('DATA'):
				ptr += 4;
				break;
			default:
				error("ImuseSndMgr::prepareSound(%d/%s) Unknown sfx header '%s'", sound->soundId, sound->name, tag2str(tag));
			}
		} while (tag != MKID_BE('DATA'));
		sound->ptr = ptr;
	} else {
		error("ImuseSndMgr::prepareSound(): Unknown sound format");
	}
}

ImuseSndMgr::soundStruct *ImuseSndMgr::allocSlot() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (!_sounds[l].inUse) {
			_sounds[l].inUse = true;
			return &_sounds[l];
		}
	}

	return NULL;
}

ImuseSndMgr::soundStruct *ImuseSndMgr::openSound(const char *soundName, int32 soundId, int volGroupId) {
	assert(soundId >= 0);
	assert(soundType);
	const char *extension = soundName + std::strlen(soundName) - 3;
	byte *ptr = NULL;

	soundStruct *sound = allocSlot();
	if (!sound) {
		error("ImuseSndMgr::openSound() can't alloc free sound slot");
	}

	if (strcasecmp(extension, "imu") == 0) {
		Block *b = getFileBlock(soundName);
		if (b != NULL) {
			ptr = b->data();
			delete b;
			sound->mcmpData = false;
		} else {
			closeSound(sound);
			return NULL;
		}
	} else if (strcasecmp(extension, "wav") == 0 || strcasecmp(extension, "imc") == 0) {
		if (!sound->mcmpMgr->openSound(soundName, &ptr)) {
			closeSound(sound);
			return NULL;
		}
		sound->mcmpData = true;
	} else {
		error("ImuseSndMgr::openSound() Unrecognized extension for sound file %s\n", soundName);
	}

	strcpy(sound->name, soundName);
	sound->volGroupId = volGroupId;
	sound->soundId = soundId;
	prepareSound(ptr, sound);
	return sound;
}

void ImuseSndMgr::closeSound(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));

	if (soundHandle->resPtr) {
		bool found = false;
		for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
			if ((_sounds[l].soundId == soundHandle->soundId) && (&_sounds[l] != soundHandle))
				found = true;
		}
		if (!found)
			_vm->unlock(rtSound, soundHandle->soundId);
	}

	delete soundHandle->bundle;
	for (int r = 0; r < soundHandle->numSyncs; r++)
		free(soundHandle->sync[r].ptr);
	free(soundHandle->region);
	free(soundHandle->jump);
	free(soundHandle->sync);
	memset(soundHandle, 0, sizeof(soundStruct));
}

ImuseSndMgr::soundStruct *ImuseDSndMgr::cloneSound(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));

	return openSound(soundHandle->soundId, soundHandle->name, soundHandle->type, soundHandle->volGroupId, soundHandle->disk);
}

bool ImuseSndMgr::checkForProperHandle(soundStruct *soundHandle) {
	if (!soundHandle)
		return false;
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (soundHandle == &_sounds[l])
			return true;
	}
	return false;
}

int ImuseSndMgr::getFreq(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->freq;
}

int ImuseSndMgr::getBits(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->bits;
}

int ImuseSndMgr::getChannels(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->channels;
}

bool ImuseSndMgr::isEndOfRegion(soundStruct *soundHandle, int region) {
	assert(checkForProperHandle(soundHandle));
	assert(region >= 0 && region < soundHandle->numRegions);
	return soundHandle->endFlag;
}

int ImuseSndMgr::getNumRegions(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->numRegions;
}

int ImuseSndMgr::getNumJumps(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->numJumps;
}

int ImuseSndMgr::getRegionOffset(soundStruct *soundHandle, int region) {
	debug(5, "getRegionOffset() region:%d", region);
	assert(checkForProperHandle(soundHandle));
	assert(region >= 0 && region < soundHandle->numRegions);
	return soundHandle->region[region].offset;
}

int ImuseSndMgr::getJumpIdByRegionAndHookId(soundStruct *soundHandle, int region, int hookId) {
	debug(5, "getJumpIdByRegionAndHookId() region:%d, hookId:%d", region, hookId);
	assert(checkForProperHandle(soundHandle));
	assert(region >= 0 && region < soundHandle->numRegions);
	int32 offset = soundHandle->region[region].offset;
	for (int l = 0; l < soundHandle->numJumps; l++) {
		if (offset == soundHandle->jump[l].offset) {
			if (soundHandle->jump[l].hookId == hookId)
				return l;
		}
	}
	
	return -1;
}

int ImuseSndMgr::getRegionIdByJumpId(soundStruct *soundHandle, int jumpId) {
	debug(5, "getRegionIdByJumpId() jumpId:%d", jumpId);
	assert(checkForProperHandle(soundHandle));
	assert(jumpId >= 0 && jumpId < soundHandle->numJumps);
	int32 dest = soundHandle->jump[jumpId].dest;
	for (int l = 0; l < soundHandle->numRegions; l++) {
		if (dest == soundHandle->region[l].offset) {
			return l;
		}
	}

	return -1;
}

int ImuseSndMgr::getJumpHookId(soundStruct *soundHandle, int number) {
	debug(5, "getJumpHookId() number:%d", number);
	assert(checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numJumps);
	return soundHandle->jump[number].hookId;
}

int ImuseSndMgr::getJumpFade(soundStruct *soundHandle, int number) {
	debug(5, "getJumpFade() number:%d", number);
	assert(checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numJumps);
	return soundHandle->jump[number].fadeDelay;
}

int32 ImuseSndMgr::getDataFromRegion(soundStruct *soundHandle, int region, byte **buf, int32 offset, int32 size) {
	debug(5, "getDataFromRegion() region:%d, offset:%d, size:%d, numRegions:%d", region, offset, size, soundHandle->numRegions);
	assert(checkForProperHandle(soundHandle));
	assert(buf && offset >= 0 && size >= 0);
	assert(region >= 0 && region < soundHandle->numRegions);

	int32 region_offset = soundHandle->region[region].offset;
	int32 region_length = soundHandle->region[region].length;

	if (offset + size > region_length) {
		size = region_length - offset;
		soundHandle->endFlag = true;
	} else {
		soundHandle->endFlag = false;
	}

	if (sound->mcmpData) {
		size = soundHandle->mcmpMgr->decompressSample(region_offset + offset, size, buf);
	} else {
		*buf = (byte *)malloc(size);
		memcpy(*buf, soundHandle->resPtr + start + offset, size);
	}
	
	return size;
}
