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

#include "kyra/sound/sound_intern.h"
#include "kyra/resource/resource.h"

#include "audio/softsynth/fmtowns_pc98/towns_pc98_driver.h"

#include "common/config-manager.h"

namespace Kyra {

SoundPC98_LoK::SoundPC98_LoK(KyraEngine_v1 *vm, Audio::Mixer *mixer) :
	Sound(vm, mixer), _musicTrackData(nullptr), _sfxTrackData(nullptr), _lastTrack(-1), _driver(nullptr), _currentResourceSet(0) {
	memset(&_resInfo, 0, sizeof(_resInfo));
}

SoundPC98_LoK::~SoundPC98_LoK() {
	delete[] _musicTrackData;
	delete[] _sfxTrackData;
	delete _driver;
	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, nullptr);
}

bool SoundPC98_LoK::init() {
	_driver = new TownsPC98_AudioDriver(_mixer, TownsPC98_AudioDriver::kType26);
	bool reslt = _driver->init();
	updateVolumeSettings();

	return reslt;
}

void SoundPC98_LoK::initAudioResourceInfo(int set, void *info) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		delete _resInfo[set];
		_resInfo[set] = info ? new Common::String(((SoundResourceInfo_PC98*)info)->pattern) : nullptr;
	}
}

void SoundPC98_LoK::selectAudioResourceSet(int set) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		if (_resInfo[set])
			_currentResourceSet = set;
	}
}

bool SoundPC98_LoK::hasSoundFile(uint file) const {
	return true;
}

void SoundPC98_LoK::loadSoundFile(uint) {
	if (_currentResourceSet == kMusicIntro) {
		delete[] _sfxTrackData;
		_sfxTrackData = nullptr;

		int dataSize = 0;
		const uint8 *tmp = _vm->staticres()->loadRawData(k1PC98IntroSfx, dataSize);

		if (!tmp) {
			warning("Could not load static intro sound effects data\n");
			return;
		}

		_sfxTrackData = new uint8[dataSize];
		memcpy(_sfxTrackData, tmp, dataSize);
	}
}

void SoundPC98_LoK::loadSoundFile(Common::String file) {
	delete[] _sfxTrackData;
	_sfxTrackData = _vm->resource()->fileData(file.c_str(), nullptr);
}

void SoundPC98_LoK::playTrack(uint8 track) {
	track -= 1;

	if (track == _lastTrack && _musicEnabled)
		return;

	beginFadeOut();

	Common::String musicFile = Common::String::format(resPattern(), track);
	delete[] _musicTrackData;
	_musicTrackData = _vm->resource()->fileData(musicFile.c_str(), nullptr);
	if (_musicEnabled)
		_driver->loadMusicData(_musicTrackData);

	_lastTrack = track;
}

void SoundPC98_LoK::haltTrack() {
	_lastTrack = -1;
	_driver->reset();
}

void SoundPC98_LoK::beginFadeOut() {
	if (!_driver->musicPlaying())
		return;

	for (int i = 0; i < 20; i++) {
		_driver->fadeStep();
		_vm->delay(32);
	}
	haltTrack();
}

void SoundPC98_LoK::playSoundEffect(uint16 track, uint8) {
	if (!_sfxTrackData)
		return;

	_driver->loadSoundEffectData(_sfxTrackData, track);
}

void SoundPC98_LoK::updateVolumeSettings() {
	if (!_driver)
		return;

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_driver->setMusicVolume((mute ? 0 : ConfMan.getInt("music_volume")));
	_driver->setSoundEffectVolume((mute ? 0 : ConfMan.getInt("sfx_volume")));
}

} // End of namespace Kyra
