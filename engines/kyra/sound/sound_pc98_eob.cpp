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
#include "kyra/sound/drivers/mlalf98.h"

#include "common/config-manager.h"

namespace Kyra {

SoundPC98_EoB::SoundPC98_EoB(KyraEngine_v1 *vm, Audio::Mixer *mixer) : Sound(vm, mixer),
	_vm(vm), _driver(0), _currentResourceSet(-1), _sfxDelay(0), _ready(false) {
	memset(_resInfo, 0, sizeof(_resInfo));
}

SoundPC98_EoB::~SoundPC98_EoB() {
	delete _driver;

	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, 0);
}

Sound::kType SoundPC98_EoB::getMusicType() const {
	return kPC98;
}

bool SoundPC98_EoB::init() {
	_driver = new MLALF98(_mixer, MLALF98::kType9801_86);
	_ready = true;
	return true;
}

void SoundPC98_EoB::initAudioResourceInfo(int set, void *info) {
	delete _resInfo[set];
	_resInfo[set] = info ? new SoundResourceInfo_PC(*(SoundResourceInfo_PC*)info) : 0;
}

void SoundPC98_EoB::selectAudioResourceSet(int set) {
	if (set == _currentResourceSet || !_ready)
		return;

	if (!_resInfo[set])
		return;

	_currentResourceSet = set;
}

void SoundPC98_EoB::loadSoundFile(uint file) {
	if (!_ready)
		return;

	if (file >= _resInfo[_currentResourceSet]->fileListSize)
		return;

	Common::SeekableReadStream *s = _vm->resource()->createReadStream(_resInfo[_currentResourceSet]->fileList[file]);
	_driver->loadMusicData(s);
	delete s;
}

void SoundPC98_EoB::loadSfxFile(Common::String file) {
	if (!_ready)
		return;

	Common::SeekableReadStream *s = _vm->resource()->createReadStream(file);
	_driver->loadSoundEffectData(s);
	delete s;
}

void SoundPC98_EoB::playTrack(uint8 track) {
	if (!_musicEnabled || !_ready)
		return;

	_driver->allChannelsOff();
	loadSoundFile(track);
	_driver->startMusic(0);
}

void SoundPC98_EoB::haltTrack() {
	if (!_musicEnabled || !_ready)
		return;
	playTrack(0);
}

void SoundPC98_EoB::playSoundEffect(uint16 track, uint8) {
	if (_currentResourceSet != kMusicIngame || !_sfxEnabled || !_ready || track >= 120 || (track != 28 && _sfxDelay > _vm->_system->getMillis()))
		return;
	_driver->startSoundEffect(track);
	if (track == 28)
		_sfxDelay = _vm->_system->getMillis() + 1440;
}

void SoundPC98_EoB::updateVolumeSettings() {
	if (!_driver || !_ready)
		return;

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_driver->setMusicVolume((mute ? 0 : ConfMan.getInt("music_volume")));
	_driver->setSoundEffectVolume((mute ? 0 : ConfMan.getInt("sfx_volume")));
}

} // End of namespace Kyra

#endif
