/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "sky/music/adlibmusic.h"
#include "sound/fmopl.h"

void SkyAdlibMusic::passMixerFunc(void *param, int16 *buf, uint len) {

	((SkyAdlibMusic*)param)->premixerCall(buf, len);
}

SkyAdlibMusic::SkyAdlibMusic(SoundMixer *pMixer, SkyDisk *pSkyDisk, OSystem *system)
	: SkyMusicBase(pSkyDisk, system) {

	_driverFileBase = 60202;
    _mixer = pMixer;
	_sampleRate = g_system->property(OSystem::PROP_GET_SAMPLE_RATE, 0);

	int env_bits = g_system->property(OSystem::PROP_GET_FMOPL_ENV_BITS, NULL);   
	int eg_ent = g_system->property(OSystem::PROP_GET_FMOPL_EG_ENT, NULL);   
	OPLBuildTables((env_bits ? env_bits : FMOPL_ENV_BITS_HQ), (eg_ent ? eg_ent : FMOPL_EG_ENT_HQ));
	_opl = OPLCreate(OPL_TYPE_YM3812, 3579545, g_system->property(OSystem::PROP_GET_SAMPLE_RATE, 0));
  
	_mixer->setupPremix(this, passMixerFunc);
}

SkyAdlibMusic::~SkyAdlibMusic(void) {

	_mixer->setupPremix(NULL, NULL);
//	YM3812Shutdown();
}

void SkyAdlibMusic::setVolume(uint8 volume) {

	_musicVolume = volume;
	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++)
		_channels[cnt]->updateVolume(volume | 128);
}

void SkyAdlibMusic::premixerCall(int16 *buf, uint len) {

	if (_musicData == NULL) {
		// no music loaded
		memset(buf, 0, len * sizeof(int16));
		return;
	} else if ((_currentMusic == 0) || (_numberOfChannels == 0)) {
		// music loaded but not played as of yet
		memset(buf, 0, len * sizeof(int16));
		// poll anyways as pollMusic() can activate the music
		pollMusic();
		_nextMusicPoll = _sampleRate/50;
		return;
	}
	uint32 render;
	while (len) {
		render = (len > _nextMusicPoll) ? (_nextMusicPoll) : (len);
		len -= render;
		_nextMusicPoll -= render;
		YM3812UpdateOne (_opl, buf, render);
		buf += render;
		if (_nextMusicPoll == 0) {
			pollMusic();
			_nextMusicPoll = _sampleRate/50;
		}
	}
}

void SkyAdlibMusic::setupPointers(void) {

	if (SkyState::_systemVars.gameVersion == 109) {
		// disk demo uses a different adlib driver version, some offsets have changed
		//_musicDataLoc = (_musicData[0x11CC] << 8) | _musicData[0x11CB];
		//_initSequence = _musicData + 0xEC8;
		
		_musicDataLoc = READ_LE_UINT16(_musicData + 0x1200);
		_initSequence = _musicData + 0xEFB;
	} else if (SkyState::_systemVars.gameVersion == 267) {
		_musicDataLoc = READ_LE_UINT16(_musicData + 0x11F7);
		_initSequence = _musicData + 0xE87;
	} else {
		_musicDataLoc = READ_LE_UINT16(_musicData + 0x1201);
		_initSequence = _musicData + 0xE91;
	}
	_nextMusicPoll = 0;
}

void SkyAdlibMusic::setupChannels(uint8 *channelData) {

	_numberOfChannels = channelData[0];
	channelData++;
	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++) {
		uint16 chDataStart = ((channelData[(cnt << 1) | 1] << 8) | channelData[cnt << 1]) + _musicDataLoc;
		_channels[cnt] = new SkyAdlibChannel(_opl, _musicData, chDataStart);
		_channels[cnt]->updateVolume(_musicVolume | 128);
	}
}

void SkyAdlibMusic::startDriver(void) {

	uint16 cnt = 0;
	while (_initSequence[cnt] || _initSequence[cnt+1]) {
		OPLWriteReg (_opl, _initSequence[cnt], _initSequence[cnt+1]);
		cnt += 2;
	}
	_allowedCommands = 0xD;
}
