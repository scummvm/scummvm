/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#ifdef ENABLE_EOB

#include "kyra/sound/sound_intern.h"
#include "kyra/resource/resource.h"
#include "kyra/sound/drivers/audiomaster2.h"

#include "common/config-manager.h"
#include "common/memstream.h"

namespace Kyra {

SoundAmiga_EoB::SoundAmiga_EoB(KyraEngine_v1 *vm, Audio::Mixer *mixer) : Sound(vm, mixer),
	_vm(vm), _driver(0), _currentResourceSet(-1), _ready(false) {
	_fileBuffer = new uint8[64000];
	memset(_resInfo, 0, sizeof(_resInfo));
}

SoundAmiga_EoB::~SoundAmiga_EoB() {
	delete _driver;
	delete[] _fileBuffer;
	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, 0);
}

Sound::kType SoundAmiga_EoB::getMusicType() const {
	return kAmiga;
}

bool SoundAmiga_EoB::init() {
	_driver = new AudioMaster2(_mixer);
	if (!_driver->init())
		return false;

	_ready = true;

	return true;
}

void SoundAmiga_EoB::initAudioResourceInfo(int set, void *info) {
	delete _resInfo[set];
	_resInfo[set] = info ? new SoundResourceInfo_AmigaEoB(*(SoundResourceInfo_AmigaEoB*)info) : 0;
}

void SoundAmiga_EoB::selectAudioResourceSet(int set) {
	if (set == _currentResourceSet || !_ready)
		return;

	_driver->flushAllResources();
	if (!_resInfo[set])
		return;

	for (uint i = 0; i < _resInfo[set]->fileListSize; ++i)
		loadSoundFile(_resInfo[set]->fileList[i]);

	_currentResourceSet = set;
}

void SoundAmiga_EoB::loadSoundFile(Common::String file) {
	if (!_ready)
		return;

	Common::SeekableReadStream *in = _vm->resource()->createReadStream(file);
	debugC(6, kDebugLevelSound, "SoundAmiga_EoB::loadSoundFile(): Attempting to load sound file '%s'...%s", file.c_str(), in ? "SUCCESS" : "FILE NOT FOUND");
	if (!in)
		return;

	// This value can deviate up to 5 bytes from the real size in EOB II Amiga.
	// The original simply tries to read 64000 bytes from the file (ignoring this
	// value). We do the same.
	// EOB I strangely always seems to have correct values.
	uint16 readSize = in->readUint16LE() - 10;
	uint8 cmp = in->readByte();
	in->seek(1, SEEK_CUR);
	uint32 outSize = in->readUint32LE();
	in->seek(2, SEEK_CUR);

	readSize = in->read(_fileBuffer, 64000);
	delete in;

	if (cmp == 0 && readSize < outSize)
		outSize = readSize;

	uint8 *buf = new uint8[outSize];

	if (cmp == 0) {
		memcpy(buf, _fileBuffer, outSize);
	} else if (cmp == 3) {
		Screen::decodeFrame3(_fileBuffer, buf, outSize, true);
	} else if (cmp == 4) {
		Screen::decodeFrame4(_fileBuffer, buf, outSize);
	} else {
		error("SoundAmiga_EoB::loadSoundFile(): Failed to load sound file '%s'", file.c_str());
	}

	Common::MemoryReadStream soundFile(buf, outSize);
	if (!_driver->loadRessourceFile(&soundFile))
		error("SoundAmiga_EoB::loadSoundFile(): Failed to load sound file '%s'", file.c_str());

	delete[] buf;
}

void SoundAmiga_EoB::unloadSoundFile(Common::String file) {
	if (!_ready)
		return;
	debugC(5, kDebugLevelSound, "SoundAmiga_EoB::unloadSoundFile(): Attempting to free resource '%s'...%s", file.c_str(), _driver->stopSound(file) ? "SUCCESS" : "FAILURE");
	_driver->flushResource(file);
}

void SoundAmiga_EoB::playTrack(uint8 track) {
	if (!_musicEnabled || !_ready)
		return;

	Common::String newSound;
	if (_vm->game() == GI_EOB1) {
		if (_currentResourceSet == kMusicIntro) {
			if (track == 1)
				newSound = "NEWINTRO1.SMUS";
			else if (track == 20)
				newSound = "CHARGEN1.SMUS";
		} else if (_currentResourceSet == kMusicFinale) {
			newSound = "FINALE.SMUS";
		}
	} else if (_vm->game() == GI_EOB2) {
		if (_currentResourceSet == kMusicIntro) {
			if (track > 11 && track < 16) {
				const char *const songs[] = { "INTRO1A.SMUS", "CHARGEN3.SMUS", "INTRO1B.SMUS", "INTRO1C.SMUS" };
				newSound = songs[track - 12];
			}
		} else if (_currentResourceSet == kMusicFinale) {
			if (track > 0 && track < 4) {
				const char *const songs[] = { "FINALE1B.SMUS", "FINALE1C.SMUS", "FINALE1D.SMUS" };
				newSound = songs[track - 1];
			}
		}
	}

	if (!newSound.empty() && _ready) {
		_driver->startSound(newSound);
		_lastSound = newSound;
	}
}

void SoundAmiga_EoB::haltTrack() {
	if (!_lastSound.empty())
		_driver->stopSound(_lastSound);
	_lastSound.clear();
}

void SoundAmiga_EoB::playSoundEffect(uint16 track, uint8 volume) {
	if (_currentResourceSet == -1 || !_sfxEnabled || !_ready)
		return;

	if (_vm->game() == GI_EOB2 && _currentResourceSet == kMusicIntro && track == 14) {
		_driver->startSound("TELEPORT.SAM");
		return;
	}

	if (!_resInfo[_currentResourceSet]->soundList || track >= 120 || !_sfxEnabled)
		return;

	if (_vm->game() == GI_EOB2 && track == 2) {
		beginFadeOut(60);
		return;
	}

	Common::String newSound = _resInfo[_currentResourceSet]->soundList[track];
	const char *suffix = (_vm->game() == GI_EOB1) ? "1.SAM" : ((track > 51 && track < 68) ? ".SMUS" : ".SAM");

	if (!newSound.empty()) {
		if (volume == 255) {
			if (_driver->startSound(newSound + suffix)) {
				_lastSound = newSound + suffix;
				return;
			} else {
				volume = 1;
			}
		}

		if (volume > 0 && volume < 5)
			newSound = Common::String::format("%s%d", newSound.c_str(), volume);

		if (!_driver->startSound(newSound)) {
			// WORKAROUND for wrongly named resources. This applies to at least 'BLADE' in the EOB II dungeons (instead of 'BLADE1').
			newSound = _resInfo[_currentResourceSet]->soundList[track];
			if (_driver->startSound(newSound))
				debugC(5, kDebugLevelSound, "SoundAmiga_EoB::playSoundEffect(): Triggered workaround for wrongly named resource: '%s'", newSound.c_str());
		}

		_lastSound = newSound;
	}
}

void SoundAmiga_EoB::beginFadeOut(int delay) {
	_driver->fadeOut(delay);
	while (_driver->isFading() && !_vm->shouldQuit())
		_vm->delay(5);
	haltTrack();
}

void SoundAmiga_EoB::updateVolumeSettings() {
	if (!_driver || !_ready)
		return;

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_driver->setMusicVolume((mute ? 0 : ConfMan.getInt("music_volume")));
	_driver->setSoundEffectVolume((mute ? 0 : ConfMan.getInt("sfx_volume")));
}

int SoundAmiga_EoB::checkTrigger() {
	return _driver->getPlayDuration();
}

} // End of namespace Kyra

#endif
