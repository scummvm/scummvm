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

#include "kyra/sound/sound_intern.h"
#include "kyra/resource/resource.h"
#include "kyra/sound/drivers/audiomaster2.h"

#include "common/config-manager.h"
#include "common/memstream.h"

namespace Kyra {

SoundAmiga_EoB::SoundAmiga_EoB(KyraEngine_v1 *vm, Audio::Mixer *mixer) : Sound(vm, mixer),
	_vm(vm), _driver(0), _currentResourceSet(-1), _currentFile(-1), _levelSoundList1(0), _levelSoundList2(0), _ready(false) {
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

	int temp = 0;
	_levelSoundList1 = _vm->staticres()->loadStrings(kEoB1BaseLevelSounds1, temp);
	_levelSoundList2 = _vm->staticres()->loadStrings(kEoB1BaseLevelSounds2, temp);

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

void SoundAmiga_EoB::loadSoundFile(uint file) {
	if (_vm->gameFlags().platform != Common::kPlatformAmiga || _currentResourceSet != kMusicIngame || !_ready)
		return;

	unloadLevelSounds();

	for (int i = 0; i < 2; ++i) {
		if (_levelSoundList1[file * 2 + i][0])
			loadSoundFile(Common::String::format("%s.CPS", _levelSoundList1[file * 2 + i]));
		if (_levelSoundList2[file * 2 + i][0])
			loadSoundFile(Common::String::format("%s.CPS", _levelSoundList2[file * 2 + i]));
	}

	loadSoundFile(Common::String::format("LEVELSAM%d.CPS", file));

	_currentFile = file;
}

void SoundAmiga_EoB::loadSoundFile(Common::String file) {
	if (!_ready)
		return;

	Common::SeekableReadStream *in = _vm->resource()->createReadStream(file);
	if (!in)
		return;

	uint16 readSize = in->readUint16LE() - 10;
	uint8 cmp = in->readByte();
	in->seek(1, SEEK_CUR);
	uint32 outSize = in->readUint32LE();
	in->seek(2, SEEK_CUR);

	if (in->read(_fileBuffer, readSize) != readSize)
		error("SoundAmiga_EoB::loadSoundFile(): Failed to load sound file '%s'", file.c_str());
	delete in;

	uint8 *buf = new uint8[outSize];

	if (cmp == 0) {
		assert(readSize == outSize);
		memcpy(buf, _fileBuffer, outSize);
	} else if (cmp == 3) {			
		Screen::decodeFrame3(_fileBuffer, buf, outSize);
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

void SoundAmiga_EoB::playTrack(uint8 track) {
	Common::String newSound;
	if (_vm->game() == GI_EOB1) {
		if (_currentResourceSet == kMusicIntro) {
			if (track == 1)
				newSound = "newintro1.smus";
			else if (track == 20)
				newSound = "chargen1.smus";
		}
	} else if (_vm->game() == GI_EOB2) {
		
	}

	if (!newSound.empty() && _ready) {
		_driver->startSound(newSound);
		_lastSound = newSound;
	}
}

void SoundAmiga_EoB::haltTrack() {
	if (!_lastSound.empty())
		_driver->stopSound(_lastSound);
}

void SoundAmiga_EoB::playSoundEffect(uint8 track, uint8 volume) {
	if (_currentResourceSet == -1 || !_ready)
		return;

	if (!_resInfo[_currentResourceSet]->soundList || track >= 120 || !_sfxEnabled)
		return;

	Common::String newSound = _resInfo[_currentResourceSet]->soundList[track];

	if (!newSound.empty()) {
		_driver->startSound(newSound);
		_lastSound = newSound;
	}
}

void SoundAmiga_EoB::beginFadeOut() {
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

void SoundAmiga_EoB::unloadLevelSounds() {
	if (_currentFile != -1) {
		_driver->flushResource(Common::String::format("L%dM1A1", _currentFile));
		_driver->flushResource(Common::String::format("L%dM2A1", _currentFile));

		for (int i = 1; i < 5; ++i) {
			_driver->flushResource(Common::String::format("L%dM1M%d", _currentFile, i));
			_driver->flushResource(Common::String::format("L%dM2M%d", _currentFile, i));
		}

		for (int i = 0; i < 2; ++i) {
			if (_levelSoundList1[_currentFile * 2 + i][0])
				_driver->flushResource(_levelSoundList1[_currentFile * 2 + i]);
			if (_levelSoundList2[_currentFile * 2 + i][0])
				_driver->flushResource(_levelSoundList2[_currentFile * 2 + i]);
		}
	}
}

} // End of namespace Kyra
