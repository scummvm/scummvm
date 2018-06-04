/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/endian.h"
#include "common/stream.h"

#include "engines/grim/resource.h"

#include "engines/grim/imuse/imuse_sndmgr.h"
#include "engines/grim/imuse/imuse_mcmp_mgr.h"

namespace Grim {

ImuseSndMgr::ImuseSndMgr(bool demo) {
	_demo = demo;
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		memset(&_sounds[l], 0, sizeof(SoundDesc));
	}
}

ImuseSndMgr::~ImuseSndMgr() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		closeSound(&_sounds[l]);
	}
}

void ImuseSndMgr::countElements(SoundDesc *sound) {
	uint32 tag;
	int32 size = 0;
	uint32 pos = sound->inStream->pos();

	do {
		tag = sound->inStream->readUint32BE();
		switch(tag) {
		case MKTAG('T','E','X','T'):
		case MKTAG('S','T','O','P'):
		case MKTAG('F','R','M','T'):
			size = sound->inStream->readUint32BE();
			sound->inStream->seek(size, SEEK_CUR);
			break;
		case MKTAG('R','E','G','N'):
			sound->numRegions++;
			size = sound->inStream->readUint32BE();
			sound->inStream->seek(size, SEEK_CUR);
			break;
		case MKTAG('J','U','M','P'):
			sound->numJumps++;
			size = sound->inStream->readUint32BE();
			sound->inStream->seek(size, SEEK_CUR);
			break;
		case MKTAG('D','A','T','A'):
			break;
		default:
			error("ImuseSndMgr::countElements() Unknown MAP tag '%s'", Common::tag2string(tag).c_str());
		}
	} while (tag != MKTAG('D','A','T','A'));

	sound->inStream->seek(pos, SEEK_SET);
}

void ImuseSndMgr::parseSoundHeader(SoundDesc *sound, int &headerSize) {
	Common::SeekableReadStream *data = sound->inStream;

	uint32 tag = data->readUint32BE();
	if (tag == MKTAG('R','I','F','F')) {
		sound->region = new Region[1];
		sound->jump = new Jump[1];
		sound->numJumps = 0;
		sound->numRegions = 1;
		sound->region[0].offset = 0;
		data->seek(18, SEEK_CUR);
		sound->channels = data->readByte();
		data->readByte();
		sound->freq = data->readUint32LE();
		data->seek(6, SEEK_CUR);
		sound->bits = data->readByte();
		data->seek(5, SEEK_CUR);
		sound->region[0].length = data->readUint32LE();
		headerSize = 44;
	} else if (tag == MKTAG('i','M','U','S')) {
		int32 size = 0;
		int32 headerStart = data->pos();
		data->seek(12, SEEK_CUR);

		int curIndexRegion = 0;
		int curIndexJump = 0;

		sound->numRegions = 0;
		sound->numJumps = 0;
		countElements(sound);
		sound->region = new Region [sound->numRegions];
		sound->jump = new Jump [sound->numJumps];

		do {
			tag = data->readUint32BE();
			switch(tag) {
			case MKTAG('F','R','M','T'):
				data->seek(12, SEEK_CUR);
				sound->bits = data->readUint32BE();
				sound->freq = data->readUint32BE();
				sound->channels = data->readUint32BE();
				break;
			case MKTAG('T','E','X','T'):
			case MKTAG('S','T','O','P'):
				size = data->readUint32BE();
				data->seek(size, SEEK_CUR);
				break;
			case MKTAG('R','E','G','N'):
				data->seek(4, SEEK_CUR);
				sound->region[curIndexRegion].offset = data->readUint32BE();
				sound->region[curIndexRegion].length = data->readUint32BE();
				curIndexRegion++;
				break;
			case MKTAG('J','U','M','P'):
				data->seek(4, SEEK_CUR);
				sound->jump[curIndexJump].offset = data->readUint32BE();
				sound->jump[curIndexJump].dest = data->readUint32BE();
				sound->jump[curIndexJump].hookId = data->readUint32BE();
				sound->jump[curIndexJump].fadeDelay = data->readUint32BE();
				curIndexJump++;
				break;
			case MKTAG('D','A','T','A'):
				data->seek(4, SEEK_CUR);
				break;
			default:
				error("ImuseSndMgr::prepareSound(%s) Unknown MAP tag '%s'", sound->name, Common::tag2string(tag).c_str());
			}
		} while (tag != MKTAG('D','A','T','A'));
		headerSize = data->pos() - headerStart;
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

	return nullptr;
}

ImuseSndMgr::SoundDesc *ImuseSndMgr::openSound(const char *soundName, int volGroupId) {
	Common::String s = soundName;
	s.toLowercase();
	soundName = s.c_str();
	const char *extension = soundName + strlen(soundName) - 3;
	int headerSize = 0;

	SoundDesc *sound = allocSlot();
	if (!sound) {
		error("ImuseSndMgr::openSound() Can't alloc free sound slot");
	}

	strcpy(sound->name, soundName);
	sound->volGroupId = volGroupId;
	sound->inStream = nullptr;

	sound->inStream = g_resourceloader->openNewStreamFile(soundName);
	if (!sound->inStream) {
		closeSound(sound);
		return nullptr;
	}

	if (!_demo && scumm_stricmp(extension, "imu") == 0) {
		parseSoundHeader(sound, headerSize);
		sound->mcmpData = false;
		sound->headerSize = headerSize;
	} else if (scumm_stricmp(extension, "wav") == 0 || scumm_stricmp(extension, "imc") == 0 ||
			(_demo && scumm_stricmp(extension, "imu") == 0)) {
		sound->mcmpMgr = new McmpMgr();
		if (!sound->mcmpMgr->openSound(soundName, sound->inStream, headerSize)) {
			closeSound(sound);
			return nullptr;
		}
		parseSoundHeader(sound, headerSize);
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
		sound->mcmpMgr = nullptr;
	}

	if (sound->region) {
		delete[] sound->region;
		sound->region = nullptr;
	}

	if (sound->jump) {
		delete[] sound->jump;
		sound->jump = nullptr;
	}

	if (sound->inStream) {
		delete sound->inStream;
		sound->inStream = nullptr;
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

int ImuseSndMgr::getRegionLength(SoundDesc *sound, int region) {
	assert(checkForProperHandle(sound));
	assert(region >= 0 && region < sound->numRegions);
	return sound->region[region].length;
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
		*buf = static_cast<byte *>(malloc(size));
		sound->inStream->seek(region_offset + offset + sound->headerSize, SEEK_SET);
		sound->inStream->read(*buf, size);
	}

	return size;
}

} // end of namespace Grim
