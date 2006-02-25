/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/util.h"

#include "sound/voc.h"
#include "sound/vorbis.h"
#include "sound/mp3.h"

#include "scumm/scumm.h"
#include "scumm/util.h"
#include "scumm/imuse_digi/dimuse.h"
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
	assert(_cacheBundleDir);
	BundleCodecs::initializeImcTables();
}

ImuseDigiSndMgr::~ImuseDigiSndMgr() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		closeSound(&_sounds[l]);
	}

	delete _cacheBundleDir;
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

void ImuseDigiSndMgr::prepareSoundFromRMAP(Common::File *file, soundStruct *sound, int32 offset, int32 size) {
	int l;

	file->seek(offset, SEEK_SET);
	uint32 tag = file->readUint32BE();
	assert(tag == 'RMAP');
	int32 version = file->readUint32BE();
	if (version != 2) {
		error("ImuseDigiSndMgr::prepareSoundFromRMAP: Wrong version number, expected 2, but it's: %d.", version);
	}
	sound->bits = file->readUint32BE();
	sound->freq = file->readUint32BE();
	sound->channels = file->readUint32BE();
	sound->numRegions = file->readUint32BE();
	sound->numJumps = file->readUint32BE();
	sound->numSyncs = file->readUint32BE();
	sound->region = (_region *)malloc(sizeof(_region) * sound->numRegions);
	assert(sound->region);
	sound->jump = (_jump *)malloc(sizeof(_jump) * sound->numJumps);
	assert(sound->jump);
	sound->sync = (_sync *)malloc(sizeof(_sync) * sound->numSyncs);
	assert(sound->sync);
	for (l = 0; l < sound->numRegions; l++) {
		sound->region[l].offset = file->readUint32BE();
		sound->region[l].length = file->readUint32BE();
	}
	for (l = 0; l < sound->numJumps; l++) {
		sound->jump[l].offset = file->readUint32BE();
		sound->jump[l].dest = file->readUint32BE();
		sound->jump[l].hookId = file->readUint32BE();
		sound->jump[l].fadeDelay = file->readUint32BE();
	}
	for (l = 0; l < sound->numSyncs; l++) {
		sound->sync[l].size = file->readUint32BE();
		sound->sync[l].ptr = (byte *)malloc(sound->sync[l].size);
		file->read(sound->sync[l].ptr, sound->sync[l].size);
	}
}

void ImuseDigiSndMgr::prepareSound(byte *ptr, soundStruct *sound) {
	if (READ_BE_UINT32(ptr) == MKID_BE('Crea')) {
		bool quit = false;
		int len;

		int32 offset = READ_LE_UINT16(ptr + 20);
		int16 code = READ_LE_UINT16(ptr + 24);

		sound->region = (_region *)malloc(sizeof(_region) * 70);
		assert(sound->region);
		sound->jump = (_jump *)malloc(sizeof(_jump));
		assert(sound->jump);
		sound->resPtr = ptr;
		sound->bits = 8;
		sound->channels = 1;

		while (!quit) {
			len = READ_LE_UINT32(ptr + offset);
			code = len & 0xFF;
			if ((code != 0) && (code != 1) && (code != 6) && (code != 7)) {
				// try again with 2 bytes forward (workaround for some FT sounds (ex.362, 363)
				offset += 2;
				len = READ_LE_UINT32(ptr + offset);
				code = len & 0xFF;
				if ((code != 0) && (code != 1) && (code != 6) && (code != 7)) {
					error("Invalid code in VOC file : %d", code);
				}
			}
			offset += 4;
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
					sound->freq = getSampleRateFromVOCRate(time_constant);
					sound->region[sound->numRegions].offset = offset;
					sound->region[sound->numRegions].length = len;
					sound->numRegions++;
				}
				break;
			case 6:	// begin of loop
				sound->jump[0].dest = offset + 8;
				sound->jump[0].hookId = 0;
				sound->jump[0].fadeDelay = 0;
				break;
			case 7:	// end of loop
				sound->jump[0].offset = offset - 4;
				sound->numJumps++;
				sound->region[sound->numRegions].offset = offset - 4;
				sound->region[sound->numRegions].length = 0;
				sound->numRegions++;
				break;
			default:
				error("Invalid code in VOC file : %d", code);
				quit = true;
				break;
			}
			offset += len;
		}
	} else if (READ_BE_UINT32(ptr) == MKID_BE('iMUS')) {
		uint32 tag;
		int32 size = 0;
		byte *s_ptr = ptr;
		ptr += 16;

		int curIndexRegion = 0;
		int curIndexJump = 0;
		int curIndexSync = 0;

		sound->numRegions = 0;
		sound->numJumps = 0;
		sound->numSyncs = 0;
		countElements(ptr, sound->numRegions, sound->numJumps, sound->numSyncs);
		sound->region = (_region *)malloc(sizeof(_region) * sound->numRegions);
		assert(sound->region);
		sound->jump = (_jump *)malloc(sizeof(_jump) * sound->numJumps);
		assert(sound->jump);
		sound->sync = (_sync *)malloc(sizeof(_sync) * sound->numSyncs);
		assert(sound->sync);

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
			case MKID_BE('SYNC'):
				size = READ_BE_UINT32(ptr); ptr += 4;
				sound->sync[curIndexSync].size = size;
				sound->sync[curIndexSync].ptr = (byte *)malloc(size);
				memcpy(sound->sync[curIndexSync].ptr, ptr, size);
				curIndexSync++;
				ptr += size;
				break;
			case MKID_BE('DATA'):
				ptr += 4;
				break;
			default:
				error("ImuseDigiSndMgr::prepareSound(%d/%s) Unknown sfx header '%s'", sound->soundId, sound->name, tag2str(tag));
			}
		} while (tag != MKID_BE('DATA'));
		sound->offsetData =  ptr - s_ptr;
	} else {
		error("ImuseDigiSndMgr::prepareSound(): Unknown sound format");
	}
}

ImuseDigiSndMgr::soundStruct *ImuseDigiSndMgr::allocSlot() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (!_sounds[l].inUse) {
			_sounds[l].inUse = true;
			return &_sounds[l];
		}
	}

	return NULL;
}

bool ImuseDigiSndMgr::openMusicBundle(soundStruct *sound, int disk) {
	bool result = false;

	sound->bundle = new BundleMgr(_cacheBundleDir);
	assert(sound->bundle);
	if (_vm->_game.id == GID_CMI) {
		if (_vm->_game.features & GF_DEMO) {
			result = sound->bundle->open("music.bun", sound->compressed);
		} else {
			char musicfile[20];
			if (disk == -1)
				disk = _vm->VAR(_vm->VAR_CURRENTDISK);
			sprintf(musicfile, "musdisk%d.bun", disk);
//			if (_disk != _vm->VAR(_vm->VAR_CURRENTDISK)) {
//				_vm->_imuseDigital->parseScriptCmds(0x1000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_imuseDigital->parseScriptCmds(0x2000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_imuseDigital->stopAllSounds();
//				sound->bundle->closeFile();
//			}

			result = sound->bundle->open(musicfile, sound->compressed, true);

			// FIXME: Shouldn't we only set _disk if result == true?
			_disk = (byte)_vm->VAR(_vm->VAR_CURRENTDISK);
		}
	} else if (_vm->_game.id == GID_DIG)
		result = sound->bundle->open("digmusic.bun", sound->compressed, true);
	else
		error("ImuseDigiSndMgr::openMusicBundle() Don't know which bundle file to load");

	_vm->VAR(_vm->VAR_MUSIC_BUNDLE_LOADED) = result ? 1 : 0;

	return result;
}

bool ImuseDigiSndMgr::openVoiceBundle(soundStruct *sound, int disk) {
	bool result = false;

	sound->bundle = new BundleMgr(_cacheBundleDir);
	assert(sound->bundle);
	if (_vm->_game.id == GID_CMI) {
		if (_vm->_game.features & GF_DEMO) {
			result = sound->bundle->open("voice.bun", sound->compressed);
		} else {
			char voxfile[20];
			if (disk == -1)
				disk = _vm->VAR(_vm->VAR_CURRENTDISK);
			sprintf(voxfile, "voxdisk%d.bun", disk);
//			if (_disk != _vm->VAR(_vm->VAR_CURRENTDISK)) {
//				_vm->_imuseDigital->parseScriptCmds(0x1000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_imuseDigital->parseScriptCmds(0x2000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_imuseDigital->stopAllSounds();
//				sound->bundle->closeFile();
//			}

			result = sound->bundle->open(voxfile, sound->compressed);

			// FIXME: Shouldn't we only set _disk if result == true?
			_disk = (byte)_vm->VAR(_vm->VAR_CURRENTDISK);
		}
	} else if (_vm->_game.id == GID_DIG)
		result = sound->bundle->open("digvoice.bun", sound->compressed);
	else
		error("ImuseDigiSndMgr::openVoiceBundle() Don't know which bundle file to load");

	_vm->VAR(_vm->VAR_VOICE_BUNDLE_LOADED) = result ? 1 : 0;

	return result;
}

ImuseDigiSndMgr::soundStruct *ImuseDigiSndMgr::openSound(int32 soundId, const char *soundName, int soundType, int volGroupId, int disk) {
	assert(soundId >= 0);
	assert(soundType);

	soundStruct *sound = allocSlot();
	if (!sound) {
		error("ImuseDigiSndMgr::openSound() can't alloc free sound slot");
	}

	const bool header_outside = ((_vm->_game.id == GID_CMI) && !(_vm->_game.features & GF_DEMO));
	bool result = false;
	byte *ptr = NULL;

	switch (soundType) {
	case IMUSE_RESOURCE:
		assert(soundName[0] == 0);	// Paranoia check

		_vm->ensureResourceLoaded(rtSound, soundId);
		_vm->res.lock(rtSound, soundId);
		ptr = _vm->getResourceAddress(rtSound, soundId);
		if (ptr == NULL) {
			closeSound(sound);
			return NULL;
		}
		sound->resPtr = ptr;
		break;
	case IMUSE_BUNDLE:
		if (volGroupId == IMUSE_VOLGRP_VOICE)
			result = openVoiceBundle(sound, disk);
		else if (volGroupId == IMUSE_VOLGRP_MUSIC)
			result = openMusicBundle(sound, disk);
		else
			error("ImuseDigiSndMgr::openSound() Don't know how load sound: %d", soundId);
		if (!result) {
			closeSound(sound);
			return NULL;
		}
		if (sound->compressed) {
			char fileName[24];
			int32 offset = 0, size = 0;
			sprintf(fileName, "%s.map", soundName);
			Common::File *rmapFile = sound->bundle->getFile(fileName, offset, size);
			if (!rmapFile) {
				closeSound(sound);
				return NULL;
			}
			prepareSoundFromRMAP(rmapFile, sound, offset, size);
			strcpy(sound->name, soundName);
			sound->soundId = soundId;
			sound->type = soundType;
			sound->volGroupId = volGroupId;
			sound->disk = _disk;
			return sound;
		} else if (soundName[0] == 0) {
			if (sound->bundle->decompressSampleByIndex(soundId, 0, 0x2000, &ptr, 0, header_outside) == 0 || ptr == NULL) {
				closeSound(sound);
				return NULL;
			}
		} else {
			if (sound->bundle->decompressSampleByName(soundName, 0, 0x2000, &ptr, header_outside) == 0 || ptr == NULL) {
				closeSound(sound);
				return NULL;
			}
		}
		sound->resPtr = 0;
		break;
	default:
		error("ImuseDigiSndMgr::openSound() Unknown soundType %d (trying to load sound %d)", soundType, soundId);
	}

	strcpy(sound->name, soundName);
	sound->soundId = soundId;
	sound->type = soundType;
	sound->volGroupId = volGroupId;
	sound->disk = _disk;
	prepareSound(ptr, sound);
	if ((soundType == IMUSE_BUNDLE) && !sound->compressed) {
		free(ptr);
	}
	return sound;
}

void ImuseDigiSndMgr::closeSound(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));

	if (soundHandle->resPtr) {
		bool found = false;
		for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
			if ((_sounds[l].soundId == soundHandle->soundId) && (&_sounds[l] != soundHandle))
				found = true;
		}
		if (!found)
			_vm->res.unlock(rtSound, soundHandle->soundId);
	}

	if (soundHandle->compressedStream)
		delete soundHandle->compressedStream;

	delete soundHandle->bundle;

	for (int r = 0; r < soundHandle->numSyncs; r++)
		free(soundHandle->sync[r].ptr);
	free(soundHandle->region);
	free(soundHandle->jump);
	free(soundHandle->sync);
	memset(soundHandle, 0, sizeof(soundStruct));
}

ImuseDigiSndMgr::soundStruct *ImuseDigiSndMgr::cloneSound(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));

	return openSound(soundHandle->soundId, soundHandle->name, soundHandle->type, soundHandle->volGroupId, soundHandle->disk);
}

bool ImuseDigiSndMgr::checkForProperHandle(soundStruct *soundHandle) {
	if (!soundHandle)
		return false;
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (soundHandle == &_sounds[l])
			return true;
	}
	return false;
}

bool ImuseDigiSndMgr::isCompressed(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->compressed;
}

int ImuseDigiSndMgr::getFreq(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->freq;
}

int ImuseDigiSndMgr::getBits(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->bits;
}

int ImuseDigiSndMgr::getChannels(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->channels;
}

bool ImuseDigiSndMgr::isEndOfRegion(soundStruct *soundHandle, int region) {
	assert(checkForProperHandle(soundHandle));
	assert(region >= 0 && region < soundHandle->numRegions);
	return soundHandle->endFlag;
}

int ImuseDigiSndMgr::getNumRegions(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->numRegions;
}

int ImuseDigiSndMgr::getNumJumps(soundStruct *soundHandle) {
	assert(checkForProperHandle(soundHandle));
	return soundHandle->numJumps;
}

int ImuseDigiSndMgr::getRegionOffset(soundStruct *soundHandle, int region) {
	debug(5, "getRegionOffset() region:%d", region);
	assert(checkForProperHandle(soundHandle));
	assert(region >= 0 && region < soundHandle->numRegions);
	return soundHandle->region[region].offset;
}

int ImuseDigiSndMgr::getJumpIdByRegionAndHookId(soundStruct *soundHandle, int region, int hookId) {
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

void ImuseDigiSndMgr::getSyncSizeAndPtrById(soundStruct *soundHandle, int number, int32 &sync_size, byte **sync_ptr) {
	assert(checkForProperHandle(soundHandle));
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

int ImuseDigiSndMgr::getJumpHookId(soundStruct *soundHandle, int number) {
	debug(5, "getJumpHookId() number:%d", number);
	assert(checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numJumps);
	return soundHandle->jump[number].hookId;
}

int ImuseDigiSndMgr::getJumpFade(soundStruct *soundHandle, int number) {
	debug(5, "getJumpFade() number:%d", number);
	assert(checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numJumps);
	return soundHandle->jump[number].fadeDelay;
}

int32 ImuseDigiSndMgr::getDataFromRegion(soundStruct *soundHandle, int region, byte **buf, int32 offset, int32 size) {
	debug(5, "getDataFromRegion() region:%d, offset:%d, size:%d, numRegions:%d", region, offset, size, soundHandle->numRegions);
	assert(checkForProperHandle(soundHandle));
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
	bool header_outside = ((_vm->_game.id == GID_CMI) && !(_vm->_game.features & GF_DEMO));
	if ((soundHandle->bundle) && (!soundHandle->compressed)) {
		size = soundHandle->bundle->decompressSampleByCurIndex(start + offset, size, buf, header_size, header_outside);
	} else if (soundHandle->resPtr) {
		*buf = (byte *)malloc(size);
		assert(*buf);
		memcpy(*buf, soundHandle->resPtr + start + offset + header_size, size);
	} else if ((soundHandle->bundle) && (soundHandle->compressed)) {
		*buf = (byte *)malloc(size);
		assert(*buf);
		char fileName[24];
		sprintf(fileName, "%s_reg%03d", soundHandle->name, region);
		if (scumm_stricmp(fileName, soundHandle->lastFileName) != 0) {
			int32 offs = 0, len = 0;
			Common::File *cmpFile;
			bool oggMode = false;
			sprintf(fileName, "%s_reg%03d.mp3", soundHandle->name, region);
			cmpFile = soundHandle->bundle->getFile(fileName, offs, len);
#ifndef USE_MAD
			if (len)
				error("Mad library compiled support needed!");
#endif
			if (!len) {
				sprintf(fileName, "%s_reg%03d.ogg", soundHandle->name, region);
				cmpFile = soundHandle->bundle->getFile(fileName, offs, len);
#ifndef USE_VORBIS
				if (len)
					error("Vorbis library compiled support needed!");
#endif
				assert(len);
				oggMode = true;
			}
			if (!soundHandle->compressedStream) {
#ifdef USE_VORBIS
				if (oggMode)
					soundHandle->compressedStream = makeVorbisStream(cmpFile, len);
#endif
#ifdef USE_MAD
				if (!oggMode)
					soundHandle->compressedStream = makeMP3Stream(cmpFile, len);
#endif
				assert(soundHandle->compressedStream);
			}
			strcpy(soundHandle->lastFileName, fileName);
		}
		size = soundHandle->compressedStream->readBuffer((int16 *)*buf, size / 2) * 2;
		if (soundHandle->compressedStream->endOfData()) {
			delete soundHandle->compressedStream;
			soundHandle->compressedStream = NULL;
			soundHandle->lastFileName[0] = 0;
		}
	}

	return size;
}

} // End of namespace Scumm
