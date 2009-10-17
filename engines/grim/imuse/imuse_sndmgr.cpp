/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
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

#include "common/endian.h"

#include "engines/grim/grim.h"
#include "engines/grim/resource.h"

#include "engines/grim/imuse/imuse_sndmgr.h"
#include "engines/grim/imuse/imuse_mcmp_mgr.h"

namespace Grim {

ImuseSndMgr::ImuseSndMgr() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		memset(&_sounds[l], 0, sizeof(SoundDesc));
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
			error("ImuseSndMgr::countElements() Unknown MAP tag '%s'", Common::tag2string(tag).c_str());
		}
	} while (tag != MKID_BE('DATA'));
}

void ImuseSndMgr::parseSoundHeader(byte *ptr, SoundDesc *sound, int &headerSize) {
	if (READ_BE_UINT32(ptr) == MKID_BE('RIFF')) {
		sound->region = new Region[1];
		sound->jump = new Jump[1];
		sound->numJumps = 0;
		sound->numRegions = 1;
		sound->region[0].offset = 0;
		sound->region[0].length = READ_LE_UINT32(ptr + 40);
		sound->bits = *(ptr + 34);
		sound->freq = READ_LE_UINT32(ptr + 24);
		sound->channels = *(ptr + 22);
		headerSize = 44;
	} else if (READ_BE_UINT32(ptr) == MKID_BE('iMUS')) {
		uint32 tag;
		int32 size = 0;
		byte *s_ptr = ptr;
		ptr += 16;

		int curIndexRegion = 0;
		int curIndexJump = 0;

		sound->numRegions = 0;
		sound->numJumps = 0;
		countElements(ptr, sound->numRegions, sound->numJumps);
		sound->region = new Region [sound->numRegions];
		sound->jump = new Jump [sound->numJumps];

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
				error("ImuseSndMgr::prepareSound(%s) Unknown MAP tag '%s'", sound->name, Common::tag2string(tag).c_str());
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

ImuseSndMgr::SoundDesc *ImuseSndMgr::allocSlot() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (!_sounds[l].inUse) {
			_sounds[l].inUse = true;
			return &_sounds[l];
		}
	}

	return NULL;
}

ImuseSndMgr::SoundDesc *ImuseSndMgr::openSound(const char *soundName, int volGroupId) {
	Common::String s = soundName;
	s.toLowercase();
	soundName = s.c_str();
	const char *extension = soundName + strlen(soundName) - 3;
	byte *ptr = NULL;
	int headerSize = 0;

	SoundDesc *sound = allocSlot();
	if (!sound) {
		error("ImuseSndMgr::openSound() Can't alloc free sound slot");
	}

	strcpy(sound->name, soundName);
	sound->volGroupId = volGroupId;

	if (!(g_grim->getGameFlags() & GF_DEMO) && strcasecmp(extension, "imu") == 0) {
		sound->blockRes = g_resourceloader->getFileBlock(soundName);
		if (sound->blockRes) {
			ptr = (byte *)sound->blockRes->data();
			parseSoundHeader(ptr, sound, headerSize);
			sound->mcmpData = false;
			sound->resPtr = ptr + headerSize;
		} else {
			closeSound(sound);
			return NULL;
		}
	} else if (strcasecmp(extension, "wav") == 0 || strcasecmp(extension, "imc") == 0 ||
			(g_grim->getGameFlags() & GF_DEMO && strcasecmp(extension, "imu") == 0)) {
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

void ImuseSndMgr::closeSound(SoundDesc *sound) {
	assert(checkForProperHandle(sound));

	if (sound->mcmpMgr) {
		delete sound->mcmpMgr;
		sound->mcmpMgr = NULL;
	}

	if (sound->blockRes) {
		delete sound->blockRes;
		sound->blockRes = NULL;
	}

	if (sound->region) {
		delete[] sound->region;
		sound->region = NULL;
	}

	if (sound->jump) {
		delete[] sound->jump;
		sound->jump = NULL;
	}

	memset(sound, 0, sizeof(SoundDesc));
}

ImuseSndMgr::SoundDesc *ImuseSndMgr::cloneSound(SoundDesc *sound) {
	assert(checkForProperHandle(sound));

	return openSound(sound->name, sound->volGroupId);
}

bool ImuseSndMgr::checkForProperHandle(SoundDesc *sound) {
	if (!sound)
		return false;

	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (sound == &_sounds[l])
			return true;
	}

	return false;
}

int ImuseSndMgr::getFreq(SoundDesc *sound) {
	assert(checkForProperHandle(sound));
	return sound->freq;
}

int ImuseSndMgr::getBits(SoundDesc *sound) {
	assert(checkForProperHandle(sound));
	return sound->bits;
}

int ImuseSndMgr::getChannels(SoundDesc *sound) {
	assert(checkForProperHandle(sound));
	return sound->channels;
}

bool ImuseSndMgr::isEndOfRegion(SoundDesc *sound, int region) {
	assert(checkForProperHandle(sound));
	assert(region >= 0 && region < sound->numRegions);
	return sound->endFlag;
}

int ImuseSndMgr::getNumRegions(SoundDesc *sound) {
	assert(checkForProperHandle(sound));
	return sound->numRegions;
}

int ImuseSndMgr::getNumJumps(SoundDesc *sound) {
	assert(checkForProperHandle(sound));
	return sound->numJumps;
}

int ImuseSndMgr::getRegionOffset(SoundDesc *sound, int region) {
	assert(checkForProperHandle(sound));
	assert(region >= 0 && region < sound->numRegions);
	return sound->region[region].offset;
}

int ImuseSndMgr::getJumpIdByRegionAndHookId(SoundDesc *sound, int region, int hookId) {
	assert(checkForProperHandle(sound));
	assert(region >= 0 && region < sound->numRegions);
	int32 offset = sound->region[region].offset;
	for (int l = 0; l < sound->numJumps; l++) {
		if (offset == sound->jump[l].offset) {
			if (sound->jump[l].hookId == hookId)
				return l;
		}
	}

	return -1;
}

int ImuseSndMgr::getRegionIdByJumpId(SoundDesc *sound, int jumpId) {
	assert(checkForProperHandle(sound));
	assert(jumpId >= 0 && jumpId < sound->numJumps);
	int32 dest = sound->jump[jumpId].dest;
	for (int l = 0; l < sound->numRegions; l++) {
		if (dest == sound->region[l].offset) {
			return l;
		}
	}

	return -1;
}

int ImuseSndMgr::getJumpHookId(SoundDesc *sound, int number) {
	assert(checkForProperHandle(sound));
	assert(number >= 0 && number < sound->numJumps);
	return sound->jump[number].hookId;
}

int ImuseSndMgr::getJumpFade(SoundDesc *sound, int number) {
	assert(checkForProperHandle(sound));
	assert(number >= 0 && number < sound->numJumps);
	return sound->jump[number].fadeDelay;
}

int32 ImuseSndMgr::getDataFromRegion(SoundDesc *sound, int region, byte **buf, int32 offset, int32 size) {
	assert(checkForProperHandle(sound));
	assert(buf && offset >= 0 && size >= 0);
	assert(region >= 0 && region < sound->numRegions);

	int32 region_offset = sound->region[region].offset;
	int32 region_length = sound->region[region].length;

	if (offset + size > region_length) {
		size = region_length - offset;
		sound->endFlag = true;
	} else {
		sound->endFlag = false;
	}

	if (sound->mcmpData) {
		size = sound->mcmpMgr->decompressSample(region_offset + offset, size, buf);
	} else {
		*buf = new byte[size];
		memcpy(*buf, sound->resPtr + region_offset + offset, size);
	}

	return size;
}

} // end of namespace Grim
