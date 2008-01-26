/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "stdafx.h"
#include "bits.h"
#include "debug.h"
#include "timer.h"
#include "resource.h"

#include "mixer/mixer.h"
#include "mixer/audiostream.h"

#include "imuse/imuse_sndmgr.h"
#include "imuse/imuse_mcmp_mgr.h"

ImuseSndMgr::ImuseSndMgr() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		memset(&_sounds[l], 0, sizeof(SoundStruct));
	}
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
			error("ImuseSndMgr::countElements() Unknown MAP tag '%s'", tag2str(tag));
		}
	} while (tag != MKID_BE('DATA'));
}

void ImuseSndMgr::parseSoundHeader(byte *ptr, SoundStruct *sound, int &headerSize) {
	if (READ_UINT32(ptr) == MKID('RIFF')) {
		sound->region = (Region *)malloc(sizeof(Region));
		sound->jump = (Jump *)malloc(0);
		sound->numJumps = 0;
		sound->numRegions = 1;
		sound->region[0].offset = 0;
		sound->region[0].length = READ_LE_UINT32(ptr + 40);
		sound->bits = *(ptr + 34);
		sound->freq = READ_LE_UINT32(ptr + 24);
		sound->channels = *(ptr + 22);
		headerSize = 44;
	} else if (READ_UINT32(ptr) == MKID('iMUS')) {
		uint32 tag;
		int32 size = 0;
		byte *s_ptr = ptr;
		ptr += 16;

		int curIndexRegion = 0;
		int curIndexJump = 0;

		sound->numRegions = 0;
		sound->numJumps = 0;
		countElements(ptr, sound->numRegions, sound->numJumps);
		sound->region = (Region *)malloc(sizeof(Region) * sound->numRegions);
		sound->jump = (Jump *)malloc(sizeof(Jump) * sound->numJumps);

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
				error("ImuseSndMgr::prepareSound(%s) Unknown MAP tag '%s'", sound->name, tag2str(tag));
			}
		} while (tag != MKID_BE('DATA'));
		headerSize = ptr - s_ptr;
		int i;
		for (i = 0; i < sound->numRegions; i++) {
			sound->region[i].offset -= headerSize;
		}
		for (i = 0; i < sound->numJumps; i++) {
			sound->jump[i].offset -= headerSize;
			sound->jump[i].dest -= headerSize;
		}
	} else {
		error("ImuseSndMgr::prepareSound() Unknown sound format");
	}
}

ImuseSndMgr::SoundStruct *ImuseSndMgr::allocSlot() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (!_sounds[l].inUse) {
			_sounds[l].inUse = true;
			return &_sounds[l];
		}
	}

	return NULL;
}

ImuseSndMgr::SoundStruct *ImuseSndMgr::openSound(const char *soundName, int volGroupId) {
	const char *extension = soundName + std::strlen(soundName) - 3;
	byte *ptr = NULL;
	int headerSize = 0;

	SoundStruct *sound = allocSlot();
	if (!sound) {
		error("ImuseSndMgr::openSound() Can't alloc free sound slot");
	}

	strcpy(sound->name, soundName);
	sound->volGroupId = volGroupId;

	if (strcasecmp(extension, "imu") == 0) {
		sound->blockRes = g_resourceloader->getFileBlock(soundName);
		if (sound->blockRes != NULL) {
			ptr = (byte *)sound->blockRes->data();
			parseSoundHeader(ptr, sound, headerSize);
			sound->mcmpData = false;
			sound->resPtr = ptr + headerSize;
		} else {
			closeSound(sound);
			return NULL;
		}
	} else if (strcasecmp(extension, "wav") == 0 || strcasecmp(extension, "imc") == 0) {
		sound->mcmpMgr = new McmpMgr();
		if (!sound->mcmpMgr->openSound(soundName, &ptr, headerSize)) {
			closeSound(sound);
			return NULL;
		}
		parseSoundHeader(ptr, sound, headerSize);
		sound->mcmpData = true;
	} else {
		error("ImuseSndMgr::openSound() Unrecognized extension for sound file %s", soundName);
	}

	return sound;
}

void ImuseSndMgr::closeSound(SoundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));

	if (soundHandle->mcmpMgr) {
		delete soundHandle->mcmpMgr;
		soundHandle->mcmpMgr = NULL;
	}

	if (soundHandle->blockRes) {
		delete soundHandle->blockRes;
		soundHandle->blockRes = NULL;
	}

	if (soundHandle->region) {
		free(soundHandle->region);
		soundHandle->region = NULL;
	}

	if (soundHandle->jump) {
		free(soundHandle->jump);
		soundHandle->jump = NULL;
	}

	memset(soundHandle, 0, sizeof(SoundStruct));
}

ImuseSndMgr::SoundStruct *ImuseSndMgr::cloneSound(SoundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));

	return openSound(soundHandle->name, soundHandle->volGroupId);
}

bool ImuseSndMgr::checkForProperHandle(SoundStruct *soundHandle) {
	if (!soundHandle)
		return false;

	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (soundHandle == &_sounds[l])
			return true;
	}

	return false;
}

int ImuseSndMgr::getFreq(SoundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->freq;
}

int ImuseSndMgr::getBits(SoundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->bits;
}

int ImuseSndMgr::getChannels(SoundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->channels;
}

bool ImuseSndMgr::isEndOfRegion(SoundStruct *soundHandle, int region) {
	assert(checkForProperHandle(soundHandle));
	assert(region >= 0 && region < soundHandle->numRegions);
	return soundHandle->endFlag;
}

int ImuseSndMgr::getNumRegions(SoundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->numRegions;
}

int ImuseSndMgr::getNumJumps(SoundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->numJumps;
}

int ImuseSndMgr::getRegionOffset(SoundStruct *soundHandle, int region) {
	assert(checkForProperHandle(soundHandle));
	assert(region >= 0 && region < soundHandle->numRegions);
	return soundHandle->region[region].offset;
}

int ImuseSndMgr::getJumpIdByRegionAndHookId(SoundStruct *soundHandle, int region, int hookId) {
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

int ImuseSndMgr::getRegionIdByJumpId(SoundStruct *soundHandle, int jumpId) {
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

int ImuseSndMgr::getJumpHookId(SoundStruct *soundHandle, int number) {
	assert(checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numJumps);
	return soundHandle->jump[number].hookId;
}

int ImuseSndMgr::getJumpFade(SoundStruct *soundHandle, int number) {
	assert(checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numJumps);
	return soundHandle->jump[number].fadeDelay;
}

int32 ImuseSndMgr::getDataFromRegion(SoundStruct *soundHandle, int region, byte **buf, int32 offset, int32 size) {
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

	if (soundHandle->mcmpData) {
		size = soundHandle->mcmpMgr->decompressSample(region_offset + offset, size, buf);
	} else {
		*buf = (byte *)malloc(size);
		memcpy(*buf, soundHandle->resPtr + region_offset + offset, size);
	}
	
	return size;
}
