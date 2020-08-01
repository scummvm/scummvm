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
#include "kyra/sound/drivers/pc_base.h"

#include "common/system.h"
#include "common/config-manager.h"


namespace Kyra {

// Kyra 1 sound triggers. Most noticeably, these are used towards the end of
// the game, in the castle, to cycle between different songs. The same music is
// used in other places throughout the game, but the player is less likely to
// spend enough time there to notice.

const int SoundPC_v1::_kyra1SoundTriggers[] = {
	0, 4, 5, 3
};

const int SoundPC_v1::_kyra1NumSoundTriggers = ARRAYSIZE(SoundPC_v1::_kyra1SoundTriggers);

SoundPC_v1::SoundPC_v1(KyraEngine_v1 *vm, Audio::Mixer *mixer, kType type)
	: Sound(vm, mixer), _driver(0), _trackEntries(), _soundDataPtr(0), _type(type) {
	memset(_trackEntries, 0, sizeof(_trackEntries));

	_soundTriggers = 0;
	_numSoundTriggers = 0;
	_sfxPlayingSound = -1;
	_soundFileLoaded.clear();
	_currentResourceSet = 0;
	memset(&_resInfo, 0, sizeof(_resInfo));

	switch (vm->game()) {
	case GI_LOL:
		_version = _vm->gameFlags().isDemo ? 3 : 4;
		break;
	case GI_KYRA2:
		_version = 4;
		break;
	case GI_KYRA1:
		_version = 3;
		_soundTriggers = _kyra1SoundTriggers;
		_numSoundTriggers = _kyra1NumSoundTriggers;
		break;
	case GI_EOB2:
		_version = 2;
		break;
	case GI_EOB1:
		_version = 1;
		break;
	default:
		break;
	}

#ifdef ENABLE_EOB
	// Correct the type to someting we support. NullSound is treated as a silent AdLib driver.
	if (_type != kAdLib && _type != kPCSpkr && _type != kPCjr)
		_type = kAdLib;
	_driver = (type == kAdLib) ? PCSoundDriver::createAdLib(mixer, _version) : PCSoundDriver::createPCSpk(mixer, _type == kPCjr);
#else
	_type = kAdLib;
	_driver = PCSoundDriver::createAdLib(mixer, _version);
#endif
	assert(_driver);
}

SoundPC_v1::~SoundPC_v1() {
	delete _driver;
	delete[] _soundDataPtr;
	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, 0);
}

bool SoundPC_v1::init() {
	_driver->initDriver();
	return true;
}

void SoundPC_v1::process() {
	int trigger = _driver->getSoundTrigger();

	if (trigger < _numSoundTriggers) {
		int soundId = _soundTriggers[trigger];

		if (soundId)
			playTrack(soundId);
	} else {
		warning("Unknown sound trigger %d", trigger);
		// TODO: At this point, we really want to clear the trigger...
	}
}

void SoundPC_v1::updateVolumeSettings() {
	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	int newMusicVolume = mute ? 0 : ConfMan.getInt("music_volume");
	//newMusicVolume = (newMusicVolume * 145) / Audio::Mixer::kMaxMixerVolume + 110;
	newMusicVolume = CLIP(newMusicVolume, 0, 255);

	int newSfxVolume = mute ? 0 : ConfMan.getInt("sfx_volume");
	//newSfxVolume = (newSfxVolume * 200) / Audio::Mixer::kMaxMixerVolume + 55;
	newSfxVolume = CLIP(newSfxVolume, 0, 255);

	_driver->setMusicVolume(newMusicVolume);
	_driver->setSfxVolume(newSfxVolume);
}

void SoundPC_v1::playTrack(uint8 track) {
	if (_musicEnabled) {
		// WORKAROUND: There is a bug in the Kyra 1 "Pool of Sorrow"
		// music which causes the channels to get progressively out of
		// sync for each loop. To avoid that, we declare that all four
		// of the song channels have to jump "in sync".

		if (track == 4 && _soundFileLoaded.equalsIgnoreCase("KYRA1B.ADL"))
			_driver->setSyncJumpMask(0x000F);
		else
			_driver->setSyncJumpMask(0);
		play(track, 0xFF);
	}
}

void SoundPC_v1::haltTrack() {
	play(0, 0);
	play(0, 0);
	//_vm->_system->delayMillis(3 * 60);
}

bool SoundPC_v1::isPlaying() const {
	return _driver->isChannelPlaying(0);
}

void SoundPC_v1::playSoundEffect(uint16 track, uint8 volume) {
	if (_sfxEnabled)
		play(track, volume);
}

void SoundPC_v1::play(uint8 track, uint8 volume) {
	uint16 soundId = 0;

	if (_version == 4)
		soundId = READ_LE_UINT16(&_trackEntries[track<<1]);
	else
		soundId = _trackEntries[track];

	if ((soundId == 0xFFFF && _version == 4) || (soundId == 0xFF && _version < 4) || !_soundDataPtr)
		return;

	_driver->startSound(soundId, volume);
}

void SoundPC_v1::beginFadeOut() {
	play(_version > 2 ? 1 : 15, 0xFF);
}

int SoundPC_v1::checkTrigger() {
	return _driver->getSoundTrigger();
}

void SoundPC_v1::resetTrigger() {
	_driver->resetSoundTrigger();
}

void SoundPC_v1::initAudioResourceInfo(int set, void *info) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		delete _resInfo[set];
		_resInfo[set] = info ? new SoundResourceInfo_PC(*(SoundResourceInfo_PC*)info) : 0;
	}
}

void SoundPC_v1::selectAudioResourceSet(int set) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		if (_resInfo[set])
			_currentResourceSet = set;
	}
}

bool SoundPC_v1::hasSoundFile(uint file) const {
	if (file < res()->fileListSize)
		return (res()->fileList[file] != 0);
	return false;
}

void SoundPC_v1::loadSoundFile(uint file) {
	if (_version == 1 && (_type == kPCSpkr || _type == kPCjr))
		file += 1;
	if (file < res()->fileListSize)
		internalLoadFile(res()->fileList[file]);
}

void SoundPC_v1::loadSoundFile(Common::String file) {
	internalLoadFile(file);
}

void SoundPC_v1::internalLoadFile(Common::String file) {
	file += ((_version == 1) ? ".DAT" : (_type == kPCSpkr ? ".SND" : ".ADL"));
	if (_soundFileLoaded == file)
		return;

	if (_soundDataPtr)
		haltTrack();

	uint8 *fileData = 0; uint32 fileSize = 0;

	fileData = _vm->resource()->fileData(file.c_str(), &fileSize);
	if (!fileData) {
		warning("Couldn't find music file: '%s'", file.c_str());
		return;
	}

	playSoundEffect(0);
	playSoundEffect(0);

	_driver->stopAllChannels();
	_soundDataPtr = 0;

	int soundDataSize = fileSize;
	uint8 *p = fileData;

	if (_version == 4) {
		memcpy(_trackEntries, p, 500);
		p += 500;
		soundDataSize -= 500;
	} else {
		memcpy(_trackEntries, p, 120);
		p += 120;
		soundDataSize -= 120;
	}

	_soundDataPtr = new uint8[soundDataSize];
	assert(_soundDataPtr);

	memcpy(_soundDataPtr, p, soundDataSize);

	delete[] fileData;
	fileData = p = 0;
	fileSize = 0;

	_driver->setSoundData(_soundDataPtr, soundDataSize);

	_soundFileLoaded = file;
}

} // End of namespace Kyra
