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

#include "common/config-manager.h"

#include "backends/audiocd/audiocd.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Kyra {

SoundTowns_Darkmoon::SoundTowns_Darkmoon(KyraEngine_v1 *vm, Audio::Mixer *mixer) : Sound(vm, mixer) {
	_intf = new TownsAudioInterface(mixer, this);
	_pcmData = 0;
	_pcmVol = 0;
	_timer = 0;
	_timerSwitch = 0;
	memset(_resource, 0, sizeof(_resource));
}

SoundTowns_Darkmoon::~SoundTowns_Darkmoon() {
	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, 0);
	delete _intf;
	delete[] _pcmData;
}

bool SoundTowns_Darkmoon::init() {
	if (!_intf->init())
		return false;

	_intf->callback(21, 255, 1);
	_intf->callback(21, 0, 1);
	_intf->callback(22, 255, 221);

	_intf->callback(70, 0x31);
	_intf->callback(33, 1);
	_intf->callback(8, 0x47, 127);
	_intf->callback(67, 1, 127, 127);

	_intf->setSoundEffectChanMask(-1);

	_lastSfxChan = 0x46;
	_lastEnvChan = 0x40;

	updateVolumeSettings();

	return true;
}

void SoundTowns_Darkmoon::timerCallback(int timerId) {
	switch (timerId) {
	case 1:
		_timerSwitch = (_timerSwitch + 1) % 4;
		if (!_timerSwitch)
			_timer++;
		break;
	default:
		break;
	}
}

void SoundTowns_Darkmoon::initAudioResourceInfo(int set, void *info) {
	delete _resource[set];
	_resource[set] = info ? new SoundResourceInfo_TownsEoB(*(SoundResourceInfo_TownsEoB*)info) : 0;
}

void SoundTowns_Darkmoon::selectAudioResourceSet(int set) {
	delete[] _pcmData;

	if (!_resource[set] || !_resource[kMusicIngame])
		return;

	_fileList = _resource[set]->fileList;
	_fileListLen = _resource[set]->numFiles;

	_pcmDataSize = _resource[kMusicIngame]->pcmDataSize;
	_pcmData = new uint8[_pcmDataSize];
	_pcmVol = _resource[set]->pcmVolume;
	memcpy(_pcmData, _resource[kMusicIngame]->pcmData, _pcmDataSize);

	if (set == kMusicIngame)
		return;

	memcpy(_pcmData, _resource[set]->pcmData, _resource[set]->pcmDataSize);
}

bool SoundTowns_Darkmoon::hasSoundFile(uint file) const {
	return true;
}

void SoundTowns_Darkmoon::loadSoundFile(uint file) {
	if (file < _fileListLen)
		loadSoundFile(_fileList[file]);
}

void SoundTowns_Darkmoon::loadSoundFile(Common::String name) {
	Common::SeekableReadStream *s = _vm->resource()->createReadStream(Common::String::format("%s.SDT", name.c_str()));
	if (!s)
		error("Failed to load sound file '%s.SDT'", name.c_str());

	for (int i = 0; i < 120; i++) {
		_soundTable[i].type = s->readSByte();
		_soundTable[i].para1 = s->readSint32LE();
		_soundTable[i].para2 = s->readSint16LE();
	}

	delete s;

	uint32 bytesLeft;
	uint8 *pmb = _vm->resource()->fileData(Common::String::format("%s.PMB", name.c_str()).c_str(), &bytesLeft);

	_vm->delay(300);

	if (pmb) {
		uint8 *src = pmb + 8;
		for (int i = 0; i < 32; i++)
			_intf->callback(5, 0x40, i, &src[i << 7]);

		_intf->callback(35, -1);
		src += 0x1000;
		bytesLeft -= 0x1008;

		while (bytesLeft) {
			_intf->callback(34, src);
			uint32 len = READ_LE_UINT16(&src[12]) + 32;
			src = src + len;
			bytesLeft -= len;
		}

		delete[] pmb;
	} else {
		warning("Sound file '%s.PMB' not found.", name.c_str());
		// TODO
	}
}

void SoundTowns_Darkmoon::playTrack(uint8 track) {
	if (track >= 120 || !_sfxEnabled)
		return;

	uint8 *pcm = 0;

	switch (_soundTable[track].type) {
	case -1:
		if (track == 0)
			haltTrack();
		else if (track == 2)
			beginFadeOut();
		break;

	case 0:
		if (_soundTable[track].para1 == -1 || (uint32)_soundTable[track].para1 > _pcmDataSize)
			return;

		pcm = _pcmData + _soundTable[track].para1;
		WRITE_LE_UINT16(&pcm[24], _soundTable[track].para2 * 98 / 1000);

		_intf->callback(39, 0x47);
		_intf->callback(37, 0x47, 60, track == 11 ? 127 : _pcmVol, pcm);
		break;

	case 2:
		resetTrigger();
		g_system->getAudioCDManager()->play(_soundTable[track].para1 - 1, 1, 0, 0);
		break;

	case 3:
		_lastSfxChan ^= 3;
		_intf->callback(39, _lastSfxChan);
		_intf->callback(4, _lastSfxChan, _soundTable[track].para1);
		_intf->callback(1, _lastSfxChan, _soundTable[track].para2, 127);
		break;

	default:
		break;
	}
}

void SoundTowns_Darkmoon::haltTrack() {
	_intf->callback(39, 0x47);
	_intf->callback(39, 0x46);
	_intf->callback(39, 0x45);

	g_system->getAudioCDManager()->stop();
}

bool SoundTowns_Darkmoon::isPlaying() const {
	return g_system->getAudioCDManager()->isPlaying();
}

void SoundTowns_Darkmoon::playSoundEffect(uint16 track, uint8 volume) {
	if (!_sfxEnabled)
		return;

	if (volume == 255)
		return playTrack(track);

	uint8 *pcm = 0;

	switch (_soundTable[track].type) {
	case 0:
		if (_soundTable[track].para1 == -1 || (uint32)_soundTable[track].para1 > _pcmDataSize)
			return;

		pcm = _pcmData + _soundTable[track].para1;
		WRITE_LE_UINT16(&pcm[24], _soundTable[track].para2 * 98 / 1000);

		_intf->callback(39, 0x47);
		_intf->callback(37, 0x47, 60, volume, pcm);
		break;

	case 3:
		_intf->callback(2, _lastEnvChan);
		_intf->callback(4, _lastEnvChan, _soundTable[track].para1);
		_intf->callback(1, _lastEnvChan, _soundTable[track].para2, volume);
		break;

	default:
		break;
	}

	if (++_lastEnvChan == 0x43)
		_lastEnvChan = 0x40;
}

void SoundTowns_Darkmoon::stopAllSoundEffects() {
	_intf->callback(39, 0x42);
	_intf->callback(39, 0x41);
	_intf->callback(39, 0x40);
}

void SoundTowns_Darkmoon::beginFadeOut() {
	for (int vol = 127; vol >= 0; vol -= 2) {
		_intf->callback(67, 1, vol, vol);
		_vm->delay(16);
	}

	_intf->callback(67, 1, 0, 0);
	_intf->callback(70, 1);

	g_system->getAudioCDManager()->stop();

	_intf->callback(70, 0x31);
	_intf->callback(67, 1, 127, 127);
}

void SoundTowns_Darkmoon::updateVolumeSettings() {
	bool mute = (ConfMan.hasKey("mute")) ? ConfMan.getBool("mute") : false;
	_intf->setSoundEffectVolume((mute ? 0 : ConfMan.getInt("sfx_volume")));
}

int SoundTowns_Darkmoon::checkTrigger() {
	return _timer;
}

void SoundTowns_Darkmoon::resetTrigger() {
	_timer = 0;
	_timerSwitch = 0;
}

} // End of namespace Kyra

#endif
