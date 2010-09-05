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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#include "scumm/sound.h"
#include "scumm/player_towns.h"

namespace Scumm {

Player_Towns::Player_Towns(ScummEngine *vm, Audio::Mixer *mixer) : _vm(vm) {
	_cdaCurrentSound = _eupCurrentSound = _cdaNumLoops = 0;
	_cdaForceRestart = 0;
	memset(_pcmCurrentSound, 0, sizeof(_pcmCurrentSound));
	_cdaVolLeft = _cdaVolRight = 0;

	_eupVolLeft = _eupVolRight = 0;
	memset(&_ovrCur, 0, sizeof(SoundOvrParameters));
	_soundOverride = 0;

	if (_vm->_game.version == 3) {
		_soundOverride = new SoundOvrParameters[200];
		memset(_soundOverride, 0, 200 * sizeof(SoundOvrParameters));
	}

	_eupLooping = false;
	_unkFlags = 0x33;

	_driver = new TownsEuphonyDriver(mixer);
}

Player_Towns::~Player_Towns() {
	delete[] _soundOverride;
	delete _driver;
}

bool Player_Towns::init() {
	if (!_driver)
		return false;
	
	if (!_driver->init())
		return false;

	_driver->reserveSoundEffectChannels(8);

	// Treat all 6 fm channels and all 8 pcm channels as sound effect channels
	// since music seems to exist as CD audio only in the games which use this
	// MusicEngine implementation.
	_driver->intf()->setSoundEffectChanMask(-1);

	setVolumeCD(255, 255);

	return true;
}

void Player_Towns::setMusicVolume(int vol) {
	_driver->setMusicVolume(vol);
}

void Player_Towns::setSfxVolume(int vol) {
	_driver->setSoundEffectVolume(vol);
}

void Player_Towns::startSound(int sound) {
	uint8 *ptr = _vm->getResourceAddress(rtSound, sound);
	if (_vm->_game.version != 3) {
		ptr += 2;
	} else if (_soundOverride && sound > 0 && sound < 200) {
		memcpy(&_ovrCur, &_soundOverride[sound], sizeof(SoundOvrParameters));
		memset(&_soundOverride[sound], 0, sizeof(SoundOvrParameters));
	}

	int type = ptr[13];

	if (type == 0) {
		playPcmTrack(sound, ptr + 6);
	} else if (type == 1) {
		playEuphonyTrack(sound, ptr + 6);
	} else if (type == 2) {
		playCdaTrack(sound, ptr + 6);
	}
	memset(&_ovrCur, 0, sizeof(SoundOvrParameters));
}

void Player_Towns::stopSound(int sound) {
	if (sound == 0 || sound == _cdaCurrentSound) {
		_cdaCurrentSound = 0;
		_vm->_sound->stopCD();
		_vm->_sound->stopCDTimer();
	}

	if (sound != 0 && sound == _eupCurrentSound) {
		_eupCurrentSound = 0;
		_eupLooping = false;
		_driver->stopParser();
	}
	
	stopPcmTrack(sound);
}

void Player_Towns::stopAllSounds() {
	_cdaCurrentSound = 0;
	_vm->_sound->stopCD();
	_vm->_sound->stopCDTimer();

	_eupCurrentSound = 0;
	_eupLooping = false;
	_driver->stopParser();

	stopPcmTrack(0);
}

int Player_Towns::getSoundStatus(int sound) const {
	if (sound == _cdaCurrentSound)
		return _vm->_sound->pollCD();
	if (sound == _eupCurrentSound)
		return _driver->parserIsPlaying() ? 1 : 0;
	for (int i = 1; i < 9; i++) {
		if (_pcmCurrentSound[i].index == sound)
			return _driver->soundEffectIsPlaying(i + 0x3f) ? 1 : 0;
	}
	return 0;
}

int32 Player_Towns::doCommand(int numargs, int args[]) {
	int32 res = 0;
	
	switch (args[0]) {
	case 2:
		_driver->intf()->callback(73, 0);
		break;

	case 3:
		restartLoopingSounds();
		break;

	case 8:
		startSound(args[1]);
		break;

	case 9:
		_vm->_sound->stopSound(args[1]);
		break;

	case 11:
		stopPcmTrack(0);
		break;

	case 14:
		startSoundEx(args[1], args[2], args[3], args[4]);
		break;

	case 15:
		stopSoundSuspendLooping(args[1]);
		break;

	default:
		warning("Player_Towns::doCommand: Unknown command %d", args[0]);
		break;
	}

	return res;
}

void Player_Towns::setVolumeCD(int left, int right) {
	_cdaVolLeft = left & 0xff;
	_cdaVolRight = right & 0xff;
	_driver->setOutputVolume(1, left >> 1, right >> 1);
}

void Player_Towns::setSoundVolume(int sound, int left, int right) {
	if (_soundOverride && sound > 0 && sound < 200) {
		_soundOverride[sound].vLeft = left;
		_soundOverride[sound].vRight = right;
	}
}

void Player_Towns::setSoundNote(int sound, int note) {
	if (_soundOverride && sound > 0 && sound < 200)
		_soundOverride[sound].note = note;
}

void Player_Towns::saveLoadWithSerializer(Serializer *ser) {
	_cdaCurrentSoundTemp = (_vm->_sound->pollCD() && _cdaNumLoops > 1) ? _cdaCurrentSound & 0xff : 0;
	_cdaNumLoopsTemp = _cdaNumLoops & 0xff;

	static const SaveLoadEntry cdEntries[] = {
		MKLINE(Player_Towns, _cdaCurrentSoundTemp, sleUint8, VER(81)),
		MKLINE(Player_Towns, _cdaNumLoopsTemp, sleUint8, VER(81)),
		MKLINE(Player_Towns, _cdaVolLeft, sleUint8, VER(81)),
		MKLINE(Player_Towns, _cdaVolRight, sleUint8, VER(81)),
		MKEND()
	};

	ser->saveLoadEntries(this, cdEntries);

	if (!_eupLooping && !_driver->parserIsPlaying())
		_eupCurrentSound = 0;

	static const SaveLoadEntry eupEntries[] = {
		MKLINE(Player_Towns, _eupCurrentSound, sleUint8, VER(81)),
		MKLINE(Player_Towns, _eupLooping, sleUint8, VER(81)),
		MKLINE(Player_Towns, _eupVolLeft, sleUint8, VER(81)),
		MKLINE(Player_Towns, _eupVolRight, sleUint8, VER(81)),
		MKEND()
	};

	ser->saveLoadEntries(this, eupEntries);

	static const SaveLoadEntry pcmEntries[] = {
		MKLINE(PcmCurrentSound, index, sleInt16, VER(81)),
		MKLINE(PcmCurrentSound, chan, sleInt16, VER(81)),
		MKLINE(PcmCurrentSound, note, sleUint8, VER(81)),
		MKLINE(PcmCurrentSound, velo, sleUint8, VER(81)),
		MKLINE(PcmCurrentSound, pan, sleUint8, VER(81)),
		MKLINE(PcmCurrentSound, paused, sleUint8, VER(81)),
		MKLINE(PcmCurrentSound, looping, sleUint8, VER(81)),
		MKLINE(PcmCurrentSound, priority, sleUint32, VER(81)),
		MKEND()
	};

	for (int i = 1; i < 9; i++) {
		if (!_pcmCurrentSound[i].index)
			continue;

		if (_driver->soundEffectIsPlaying(i + 0x3f))
			continue;

		_driver->stopSoundEffect(i + 0x3f);

		_pcmCurrentSound[i].index = 0;
	}

	ser->saveLoadArrayOf(_pcmCurrentSound, 9, sizeof(PcmCurrentSound), pcmEntries);
}

void Player_Towns::restoreAfterLoad() {
	setVolumeCD(_cdaVolLeft, _cdaVolRight);
	
	if (_cdaCurrentSoundTemp) {
		uint8 *ptr = _vm->getResourceAddress(rtSound, _cdaCurrentSoundTemp) + 6;
		if (_vm->_game.version != 3)
			ptr += 2;
		
		if (ptr[7] == 2) {
			playCdaTrack(_cdaCurrentSoundTemp, ptr, true);
			_cdaCurrentSound = _cdaCurrentSoundTemp;
			_cdaNumLoops = _cdaNumLoopsTemp;
		}
	}

	if (_eupCurrentSound) {
		uint8 *ptr = _vm->getResourceAddress(rtSound, _eupCurrentSound) + 6;
		if (_vm->_game.version != 3)
			ptr += 2;
		
		if (ptr[7] == 1) {
			setSoundVolume(_eupCurrentSound, _eupVolLeft, _eupVolRight);
			playEuphonyTrack(_eupCurrentSound, ptr);
		}
	}

	for (int i = 1; i < 9; i++) {
		if (!_pcmCurrentSound[i].index)
			continue;

		uint8 *ptr = _vm->getResourceAddress(rtSound, _pcmCurrentSound[i].index);
		if (!ptr)
			continue;

		if (_vm->_game.version != 3)
			ptr += 2;

		if (ptr[13])
			continue;

		playPcmTrack(_pcmCurrentSound[i].index, ptr + 6, _pcmCurrentSound[i].velo, _pcmCurrentSound[i].pan, _pcmCurrentSound[i].note);
	}
}

int Player_Towns::getNextFreePcmChannel(int sound, int sfxChanRelIndex) {
	int chan = 0;
	for (int i = 8; i; i--) {
		if (!_pcmCurrentSound[i].index) {
			chan = i;
			continue;
		}

		if (_driver->soundEffectIsPlaying(i + 0x3f))
			continue;

		chan = i;
		_vm->_sound->stopSound(_pcmCurrentSound[chan].index);
	}

	if (!chan) {
		uint16 l = 0xffff;
		uint8 *ptr = 0;
		for (int i = 8; i; i--) {
			ptr = _vm->getResourceAddress(rtSound, _pcmCurrentSound[i].index) + 6;
			uint16 a = READ_LE_UINT16(ptr + 10);
			if (a <= l) {
				chan = i;
				l = a;
			}
		}

		ptr = _vm->getResourceAddress(rtSound, sound) + 6;
		if (l <= READ_LE_UINT16(ptr + 10))
			_vm->_sound->stopSound(_pcmCurrentSound[chan].index);
		else
			chan = 0;
	}

	if (chan) {
		_pcmCurrentSound[chan].index = sound;
		_pcmCurrentSound[chan].chan = sfxChanRelIndex;
	}

	return chan;
}

void Player_Towns::restartLoopingSounds() {
	if (_cdaNumLoops && !_cdaForceRestart)
		_cdaForceRestart = 1;

	for (int i = 1; i < 9; i++) {
		if (!_pcmCurrentSound[i].paused)
			continue;

		_pcmCurrentSound[i].paused = 0;

		uint8 *ptr = _vm->getResourceAddress(rtSound, _pcmCurrentSound[i].index);
		if (!ptr)
			continue;
		ptr += 24;

		int c = 1;
		while (_pcmCurrentSound[i].chan != c) {
			ptr = ptr + READ_LE_UINT32(&ptr[12]) + 32;
			c++;
		}

		_driver->playSoundEffect(i + 0x3f, _pcmCurrentSound[i].note, _pcmCurrentSound[i].velo, ptr);
	}

	_driver->intf()->callback(73, 1);
}

void Player_Towns::startSoundEx(int sound, int velo, int pan, int note) {
	uint8 *ptr = _vm->getResourceAddress(rtSound, sound) + 2;

	if (pan > 99)
		pan = 99;

	velo = velo ? (velo * ptr[14] + 50) / 100 : ptr[14];
	velo = CLIP(velo, 1, 255);

	if (ptr[13] == 0) {
		velo >>= 1;

		if (!velo)
			velo = 1;

		pan = pan ? (((pan << 7) - pan) + 50) / 100 : 64;

		playPcmTrack(sound, ptr + 6, velo, pan, note);

	} else if (ptr[13] == 2) {
		int volLeft = velo;
		int volRight = velo;
		
		if (pan < 50)
			volRight = ((pan * 2 + 1) * velo + 50) / 100;
		else if (pan > 50)
			volLeft = (((99 - pan) * 2 + 1) * velo + 50) / 100;

		setVolumeCD(volLeft, volRight);

		if (!_cdaForceRestart && sound == _cdaCurrentSound)
			return;

		playCdaTrack(sound, ptr + 6, true);
	}
}

void Player_Towns::stopSoundSuspendLooping(int sound) {
	if (!sound) {
		return;
	} else if (sound == _cdaCurrentSound) {
		if (_cdaNumLoops && _cdaForceRestart)
			_cdaForceRestart = 1;		
	} else {
		for (int i = 1; i < 9; i++) {
			if (sound == _pcmCurrentSound[i].index) {
				if (!_driver->soundEffectIsPlaying(i + 0x3f))
					continue;
				_driver->stopSoundEffect(i + 0x3f);
				if (_pcmCurrentSound[i].looping)
					_pcmCurrentSound[i].paused = 1;
				else 
					_pcmCurrentSound[i].index = 0;
			}
		}
	}
}

void Player_Towns::playEuphonyTrack(int sound, const uint8 *data) {
	const uint8 *pos = data + 16;
	const uint8 *src = pos + data[14] * 48;
	const uint8 *trackData = src + 150;

	for (int i = 0; i < 32; i++)
		_driver->chanEnable(i, *src++);
	for (int i = 0; i < 32; i++)
		_driver->chanMode(i, 0xff);
	for (int i = 0; i < 32; i++)
		_driver->chanOrdr(i, *src++);
	for (int i = 0; i < 32; i++)
		_driver->chanVolumeShift(i, *src++);
	for (int i = 0; i < 32; i++)
		_driver->chanNoteShift(i, *src++);

	src += 8;
	for (int i = 0; i < 6; i++)
		_driver->assignChannel(i, *src++);

	for (int i = 0; i < data[14]; i++) {
		_driver->loadInstrument(i, i, pos + i * 48);
		_driver->intf()->callback(4, i, i);
	}

	_eupVolLeft = _ovrCur.vLeft;
	_eupVolRight = _ovrCur.vRight;
	int lvl = _ovrCur.vLeft + _ovrCur.vRight;
	if (!lvl)
		lvl = data[8] + data[9];
	lvl >>= 2;

	for (int i = 0; i < 6; i++)
		_driver->chanVolume(i, lvl);

	uint32 trackSize = READ_LE_UINT32(src);
	src += 4;
	uint8 startTick = *src++;
	
	_driver->setMusicTempo(*src++);
	_driver->startMusicTrack(trackData, trackSize, startTick);

	_eupLooping = (*src != 1) ? 1 : 0;
	_driver->setMusicLoop(_eupLooping != 0);
	_driver->continueParsing();
	_eupCurrentSound = sound;
}

void Player_Towns::playPcmTrack(int sound, const uint8 *data, int velo, int pan, int note) {
	const uint8 *ptr = data;
	const uint8 *sfxData = ptr + 16;
	
	int note2, velocity;

	if (velo)
		velocity = velo;
	else if (_ovrCur.vLeft + _ovrCur.vRight)
		velocity = (_ovrCur.vLeft + _ovrCur.vRight) >> 2;
	else
		velocity = ptr[8] >> 1;

	int numChan = ptr[14];
	for (int i = 0; i < numChan; i++) {
		int chan = getNextFreePcmChannel(sound, i);
		if (!chan)
			return;
		
		_driver->intf()->callback(70, _unkFlags);
		_driver->chanPanPos(chan + 0x3f, pan);
		
		if (note)
			note2 = note;
		else if (_ovrCur.note)
			note2 = _ovrCur.note;
		else
			note2 = sfxData[28];
		
		_driver->playSoundEffect(chan + 0x3f, note2, velocity, sfxData);
		
		_pcmCurrentSound[chan].note = note2;
		_pcmCurrentSound[chan].velo = velocity;
		_pcmCurrentSound[chan].pan = pan;
		_pcmCurrentSound[chan].paused = 0;
		_pcmCurrentSound[chan].looping = READ_LE_UINT32(&sfxData[20]) ? 1 : 0;

		sfxData += (READ_LE_UINT32(&sfxData[12]) + 32);
	}
}

void Player_Towns::playCdaTrack(int sound, const uint8 *data, bool skipTrackVelo) {
	const uint8 *ptr = data;

	if (!sound)
		return;

	if (!skipTrackVelo) {
		if (_ovrCur.vLeft + _ovrCur.vRight)
			setVolumeCD(_ovrCur.vLeft, _ovrCur.vRight);
		else
			setVolumeCD(ptr[8], ptr[9]);
	}

	if (sound == _cdaCurrentSound && _vm->_sound->pollCD() == 1)			
		return;

	ptr += 16;

	int track = ptr[0];
	_cdaNumLoops = ptr[1];
	int start = (ptr[2] * 60 + ptr[3]) * 75 + ptr[4];
	int end = (ptr[5] * 60 + ptr[6]) * 75 + ptr[7];

	_vm->_sound->playCDTrack(track, _cdaNumLoops == 0xff ? -1 : _cdaNumLoops, start, end <= start ? 0 : end - start);
	_cdaForceRestart = 0;
	_cdaCurrentSound = sound;
}

void Player_Towns::stopPcmTrack(int sound) {
	for (int i = 1; i < 9; i++) {
		if (sound == _pcmCurrentSound[i].index || !sound) {
			_driver->stopSoundEffect(i + 0x3f);
			_pcmCurrentSound[i].index = 0;
		}
	}
}

} // End of namespace Scumm

