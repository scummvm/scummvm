/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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
 * $Header$
 *
 */

#include "sky/music/adlibmusic.h"
#include "sky/music/adlibchannel.h"
#include "sound/mixer.h"
#include "sky/sky.h"

namespace Sky {

AdlibMusic::AdlibMusic(Audio::Mixer *pMixer, Disk *pDisk)
	: MusicBase(pDisk) {

	_driverFileBase = 60202;
	_mixer = pMixer;
	_sampleRate = pMixer->getOutputRate();

	_opl = makeAdlibOPL(_sampleRate);

	_mixer->setupPremix(this);
}

AdlibMusic::~AdlibMusic(void) {

	_mixer->setupPremix(0);
}

void AdlibMusic::premixerCall(int16 *data, uint len) {

	if (_musicData == NULL) {
		// no music loaded
		memset(data, 0, 2 * len * sizeof(int16));
	} else if ((_currentMusic == 0) || (_numberOfChannels == 0)) {
		// music loaded but not played as of yet
		memset(data, 0, 2 * len * sizeof(int16));
		// poll anyways as pollMusic() can activate the music
		pollMusic();
		_nextMusicPoll = _sampleRate/50;
	} else {
		uint32 render;
		int16 *origData = data;
		uint origLen = len;
		while (len) {
			render = (len > _nextMusicPoll) ? (_nextMusicPoll) : (len);
			len -= render;
			_nextMusicPoll -= render;
			YM3812UpdateOne(_opl, data, render);
			data += render;
			if (_nextMusicPoll == 0) {
				pollMusic();
				_nextMusicPoll = _sampleRate/50;
			}
		}

		// Convert mono data to stereo
		for (int i = (origLen - 1); i >= 0; i--) {
			origData[2 * i] = origData[2 * i + 1] = origData[i];
		}
	}
}

void AdlibMusic::setupPointers(void) {

	if (SkyEngine::_systemVars.gameVersion == 109) {
		// disk demo uses a different adlib driver version, some offsets have changed
		//_musicDataLoc = (_musicData[0x11CC] << 8) | _musicData[0x11CB];
		//_initSequence = _musicData + 0xEC8;

		_musicDataLoc = READ_LE_UINT16(_musicData + 0x1200);
		_initSequence = _musicData + 0xEFB;
	} else if (SkyEngine::_systemVars.gameVersion == 267) {
		_musicDataLoc = READ_LE_UINT16(_musicData + 0x11F7);
		_initSequence = _musicData + 0xE87;
	} else {
		_musicDataLoc = READ_LE_UINT16(_musicData + 0x1201);
		_initSequence = _musicData + 0xE91;
	}
	_nextMusicPoll = 0;
}

void AdlibMusic::setupChannels(uint8 *channelData) {

	_numberOfChannels = channelData[0];
	channelData++;
	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++) {
		uint16 chDataStart = ((channelData[(cnt << 1) | 1] << 8) | channelData[cnt << 1]) + _musicDataLoc;
		_channels[cnt] = new AdlibChannel(_opl, _musicData, chDataStart);
		_channels[cnt]->updateVolume(_musicVolume);
	}
}

void AdlibMusic::startDriver(void) {

	uint16 cnt = 0;
	while (_initSequence[cnt] || _initSequence[cnt+1]) {
		OPLWriteReg (_opl, _initSequence[cnt], _initSequence[cnt+1]);
		cnt += 2;
	}
	_allowedCommands = 0xD;
}

} // End of namespace Sky
