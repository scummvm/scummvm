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
//#include "kyra/sound/drivers/mlalf98.h"
#include "common/config-manager.h"
#include "backends/audiocd/audiocd.h"

namespace Kyra {

SoundSegaCD_EoB::SoundSegaCD_EoB(KyraEngine_v1 *vm, Audio::Mixer *mixer) : Sound(vm, mixer),
	_vm(vm), /*_driver(0), _currentResourceSet(-1), _sfxDelay(0),*/ _ready(false) {
	//memset(_resInfo, 0, sizeof(_resInfo));
}

SoundSegaCD_EoB::~SoundSegaCD_EoB() {
	//delete _driver;

	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, 0);
}

Sound::kType SoundSegaCD_EoB::getMusicType() const {
	return kSegaCD;
}

bool SoundSegaCD_EoB::init() {
	//_driver = new MLALF98(_mixer, MLALF98::kType9801_86);
	g_system->getAudioCDManager()->open();
	_ready = true;
	return true;
}

void SoundSegaCD_EoB::initAudioResourceInfo(int set, void *info) {
	//delete _resInfo[set];
	//_resInfo[set] = info ? new SoundResourceInfo_PC(*(SoundResourceInfo_PC*)info) : 0;
}

void SoundSegaCD_EoB::selectAudioResourceSet(int set) {
	/*if (set == _currentResourceSet || !_ready)
		return;

	if (!_resInfo[set])
		return;

	_currentResourceSet = set;*/
}

void SoundSegaCD_EoB::loadSfxFile(Common::String file) {

}

void SoundSegaCD_EoB::playTrack(uint8 track) {
	static const uint8 levelCDATracks[13] = {
		7, 7, 7, 7, 6, 6, 6, 4, 4, 4, 5, 5, 10
	};

	if (!_musicEnabled || !_ready)
		return;
	
	g_system->getAudioCDManager()->play(track, 1, 0, 0);
}

void SoundSegaCD_EoB::haltTrack() {
	if (!_musicEnabled || !_ready)
		return;
	g_system->getAudioCDManager()->stop();
}

void SoundSegaCD_EoB::playSoundEffect(uint8 track, uint8) {
	if (!_sfxEnabled || !_ready)
		return;
//	_driver->startSoundEffect(track);
}

bool SoundSegaCD_EoB::isPlaying() const {
	return g_system->getAudioCDManager()->isPlaying();
}

void SoundSegaCD_EoB::updateVolumeSettings() {
	if (/*!_driver ||*/ !_ready)
		return;

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	//_driver->setMusicVolume((mute ? 0 : ConfMan.getInt("music_volume")));
	//_driver->setSoundEffectVolume((mute ? 0 : ConfMan.getInt("sfx_volume")));
}

} // End of namespace Kyra

#endif
