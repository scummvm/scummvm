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

#ifdef ENABLE_EOB

#include "kyra/sound/sound_intern.h"
#include "kyra/resource/resource.h"
#include "kyra/sound/drivers/capcom98.h"

#include "common/config-manager.h"

namespace Kyra {

SoundPC98_Darkmoon::SoundPC98_Darkmoon(KyraEngine_v1 *vm, MidiDriver::DeviceHandle dev, Audio::Mixer *mixer) : Sound(vm, mixer),
	_vm(vm), _driver(0), _currentResourceSet(-1), _ready(false), _drvType(kPC98) {
	MusicType type = MidiDriver::getMusicType(dev);
	if (type == MT_MT32) {
		_drvType = kMidiMT32;
	} else if (type == MT_GM) {
		_drvType = kMidiGM;
	} else {

	}
}

SoundPC98_Darkmoon::~SoundPC98_Darkmoon() {
	delete _driver;
}

Sound::kType SoundPC98_Darkmoon::getMusicType() const {
	return _drvType;
}

bool SoundPC98_Darkmoon::init() {
	_driver = new CapcomPC98AudioDriver();
	_ready = true;
	return true;
}

void SoundPC98_Darkmoon::initAudioResourceInfo(int set, void *info) {
	//delete _resInfo[set];
	//_resInfo[set] = info ? new SoundResourceInfo_PC(*(SoundResourceInfo_PC*)info) : 0;
}

void SoundPC98_Darkmoon::selectAudioResourceSet(int set) {
	if (set == _currentResourceSet || !_ready)
		return;

	//if (!_resInfo[set])
	//	return;

	_currentResourceSet = set;
}

void SoundPC98_Darkmoon::loadSoundFile(Common::String name) {
	if (!_ready)
		return;

	//if (file >= _resInfo[_currentResourceSet]->fileListSize)
	//	return;

	//Common::SeekableReadStream *s = _vm->resource()->createReadStream(_resInfo[_currentResourceSet]->fileList[file]);
	//_driver->loadMusicData(s);
	//delete s;
}

void SoundPC98_Darkmoon::playTrack(uint8 track) {
	if (!_musicEnabled || !_ready)
		return;
}

void SoundPC98_Darkmoon::haltTrack() {
	if (!_musicEnabled || !_ready)
		return;
	//playTrack(0);
}

bool SoundPC98_Darkmoon::isPlaying() const {
	return false;
}

void SoundPC98_Darkmoon::playSoundEffect(uint16 track, uint8) {
	if (!_sfxEnabled || !_ready || track >= 120)
		return;
	//_driver->startSoundEffect(track);
}

void SoundPC98_Darkmoon::stopAllSoundEffects() {

}

void SoundPC98_Darkmoon::beginFadeOut() {

}

int SoundPC98_Darkmoon::checkTrigger() {
	return 99;
}

void SoundPC98_Darkmoon::resetTrigger() {

}

void SoundPC98_Darkmoon::updateVolumeSettings() {
	if (!_driver || !_ready)
		return;

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	//_driver->setMusicVolume((mute ? 0 : ConfMan.getInt("music_volume")));
	//_driver->setSoundEffectVolume((mute ? 0 : ConfMan.getInt("sfx_volume")));
}

} // End of namespace Kyra

#endif
