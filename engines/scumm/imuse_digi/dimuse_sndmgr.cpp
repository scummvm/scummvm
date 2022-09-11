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


#include "common/scummsys.h"

#include "audio/audiostream.h"
#include "audio/decoders/flac.h"
#include "audio/decoders/voc.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/mp3.h"

#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"
#include "scumm/imuse_digi/dimuse_codecs.h"
#include "scumm/imuse_digi/dimuse_sndmgr.h"

namespace Scumm {

ImuseDigiSndMgr::ImuseDigiSndMgr(ScummEngine *scumm) {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		memset(&_sounds[l], 0, sizeof(SoundDesc));
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
	BundleCodecs::releaseImcTables();
}

ImuseDigiSndMgr::SoundDesc *ImuseDigiSndMgr::allocSlot() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (!_sounds[l].inUse) {
			_sounds[l].inUse = true;
			_sounds[l].scheduledForDealloc = false;
			return &_sounds[l];
		}
	}

	return nullptr;
}

bool ImuseDigiSndMgr::openMusicBundle(SoundDesc *sound, int &disk) {
	bool result = false;
	bool compressed = false;

	sound->bundle = new BundleMgr(_cacheBundleDir);
	assert(sound->bundle);
	if (_vm->_game.id == GID_CMI) {
		if (_vm->_game.features & GF_DEMO) {
			result = sound->bundle->open("music.bun", compressed);
		} else {
			char musicfile[20];
			if (disk == -1)
				disk = _vm->VAR(_vm->VAR_CURRENTDISK);
			sprintf(musicfile, "musdisk%d.bun", disk);
//			if (_disk != _vm->VAR(_vm->VAR_CURRENTDISK)) {
//				_vm->_DiMUSE_v1->parseScriptCmds(0x1000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_DiMUSE_v1->parseScriptCmds(0x2000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_DiMUSE_v1->stopAllSounds();
//				sound->bundle->closeFile();
//			}

			result = sound->bundle->open(musicfile, compressed);

			// FIXME: Shouldn't we only set _disk if result == true?
			_disk = (byte)_vm->VAR(_vm->VAR_CURRENTDISK);
		}
	} else if (_vm->_game.id == GID_DIG)
		result = sound->bundle->open("digmusic.bun", compressed);
	else
		error("ImuseDigiSndMgr::openMusicBundle() Don't know which bundle file to load");

	_vm->VAR(_vm->VAR_MUSIC_BUNDLE_LOADED) = result ? 1 : 0;

	return result;
}

bool ImuseDigiSndMgr::openVoiceBundle(SoundDesc *sound, int &disk) {
	bool result = false;
	bool compressed = false;

	sound->bundle = new BundleMgr(_cacheBundleDir);
	assert(sound->bundle);
	if (_vm->_game.id == GID_CMI) {
		if (_vm->_game.features & GF_DEMO) {
			result = sound->bundle->open("voice.bun", compressed);
		} else {
			char voxfile[20];
			if (disk == -1)
				disk = _vm->VAR(_vm->VAR_CURRENTDISK);
			sprintf(voxfile, "voxdisk%d.bun", disk);
//			if (_disk != _vm->VAR(_vm->VAR_CURRENTDISK)) {
//				_vm->_DiMUSE_v1->parseScriptCmds(0x1000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_DiMUSE_v1->parseScriptCmds(0x2000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_DiMUSE_v1->stopAllSounds();
//				sound->bundle->closeFile();
//			}

			result = sound->bundle->open(voxfile, compressed);

			// FIXME: Shouldn't we only set _disk if result == true?
			_disk = (byte)_vm->VAR(_vm->VAR_CURRENTDISK);
		}
	} else if (_vm->_game.id == GID_DIG)
		result = sound->bundle->open("digvoice.bun", compressed);
	else
		error("ImuseDigiSndMgr::openVoiceBundle() Don't know which bundle file to load");

	_vm->VAR(_vm->VAR_VOICE_BUNDLE_LOADED) = result ? 1 : 0;

	return result;
}

ImuseDigiSndMgr::SoundDesc *ImuseDigiSndMgr::openSound(int32 soundId, const char *soundName, int soundType, int volGroupId, int disk) {
	assert(soundId >= 0);
	assert(soundType);

	SoundDesc *sound = allocSlot();
	if (!sound) {
		error("ImuseDigiSndMgr::openSound() can't alloc free sound slot");
	}

	const bool header_outside = ((_vm->_game.id == GID_CMI) && !(_vm->_game.features & GF_DEMO));
	bool result = false;
	byte *ptr = nullptr;

	switch (soundType) {
	case IMUSE_RESOURCE:
		assert(soundName[0] == 0);	// Paranoia check

		_vm->_res->lock(rtSound, soundId);
		ptr = _vm->getResourceAddress(rtSound, soundId);
		if (ptr == nullptr) {
			closeSound(sound);
			return nullptr;
		}
		sound->resPtr = ptr;
		sound->resSize = _vm->getResourceSize(rtSound, soundId) - 8;
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
			return nullptr;
		}

		if (soundName[0] != 0) {
			if (sound->bundle->readFile(soundName, 0x2000, &ptr, header_outside) == 0 || ptr == nullptr) {
				closeSound(sound);
				free(ptr);
				return nullptr;
			}
		}

		sound->resPtr = nullptr;
		break;
	default:
		error("ImuseDigiSndMgr::openSound() Unknown soundType %d (trying to load sound %d)", soundType, soundId);
	}

	Common::strlcpy(sound->name, soundName, sizeof(sound->name));
	sound->soundId = soundId;

	if (soundType == IMUSE_BUNDLE) {
		free(ptr);
	}
	return sound;
}

void ImuseDigiSndMgr::closeSound(SoundDesc *soundDesc) {
	assert(checkForProperHandle(soundDesc));

	if (soundDesc->resPtr) {
		bool found = false;
		for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
			if ((_sounds[l].soundId == soundDesc->soundId) && (&_sounds[l] != soundDesc))
				found = true;
		}
		if (!found)
			_vm->_res->unlock(rtSound, soundDesc->soundId);
	}

	delete soundDesc->bundle;

	memset(soundDesc, 0, sizeof(SoundDesc));
}

ImuseDigiSndMgr::SoundDesc *ImuseDigiSndMgr::findSoundById(int soundId) {
	SoundDesc *soundDesc = nullptr;
	for (int i = 0; i < MAX_IMUSE_SOUNDS; i++) {
		if (_sounds[i].soundId == soundId) {
			soundDesc = &_sounds[i];
			break;
		}
	}
	return soundDesc;
}

ImuseDigiSndMgr::SoundDesc *ImuseDigiSndMgr::getSounds() {
	return _sounds;
}

void ImuseDigiSndMgr::scheduleSoundForDeallocation(int soundId) {
	SoundDesc *soundDesc = nullptr;
	for (int i = 0; i < MAX_IMUSE_SOUNDS; i++) {
		if (_sounds[i].soundId == soundId) {
			soundDesc = &_sounds[i];
		}
	}

	assert(checkForProperHandle(soundDesc));

	soundDesc->scheduledForDealloc = true;
}

void ImuseDigiSndMgr::closeSoundById(int soundId) {
	SoundDesc *soundDesc = nullptr;
	for (int i = 0; i < MAX_IMUSE_SOUNDS; i++) {
		if (_sounds[i].soundId == soundId) {
			soundDesc = &_sounds[i];
		}
	}

	if (soundDesc) {
		assert(checkForProperHandle(soundDesc));

		if (soundDesc->resPtr) {
			_vm->_res->unlock(rtSound, soundDesc->soundId);
		}

		delete soundDesc->bundle;

		memset(soundDesc, 0, sizeof(SoundDesc));
	}
}

bool ImuseDigiSndMgr::checkForProperHandle(SoundDesc *soundDesc) {
	if (!soundDesc)
		return false;
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (soundDesc == &_sounds[l])
			return true;
	}
	return false;
}

} // End of namespace Scumm
