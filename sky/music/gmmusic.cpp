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

#include "sky/music/gmmusic.h"

void SkyGmMusic::passTimerFunc(void *param) {

	((SkyGmMusic*)param)->timerCall();
}

SkyGmMusic::SkyGmMusic(MidiDriver *pMidiDrv, SkyDisk *pSkyDisk, OSystem *system)
	: SkyMusicBase(pSkyDisk, system) {

	_driverFileBase = 60200;
    _midiDrv = pMidiDrv;
	int midiRes = _midiDrv->open();
	if (midiRes != 0) {
		error("Can't open midi device. Errorcode: %d\n",midiRes);
	}
	_midiDrv->setTimerCallback(this, passTimerFunc);
	_ignoreNextPoll = false;
}

SkyGmMusic::~SkyGmMusic(void) {

	_midiDrv->setTimerCallback(NULL, NULL);
	if (_currentMusic) stopMusic();
	_midiDrv->close();
	delete _midiDrv;
}

void SkyGmMusic::setVolume(uint8 volume) {

	uint8 sysEx[6];
	_musicVolume = volume;
	if (volume > 0) volume = (volume * 2) / 3 + 43; // GM synths behave kinda logarithmic
	sysEx[0] = 0x7F; sysEx[1] = 0x7F; sysEx[2] = 0x04; sysEx[3] = 0x01;
	sysEx[4] = 0; sysEx[5] = volume & 0x7F;
	_midiDrv->sysEx(sysEx, 6);
}

void SkyGmMusic::timerCall(void) {

	// midi driver polls hundred times per sec. We only want 50 times.
	_ignoreNextPoll = !_ignoreNextPoll;
	if (!_ignoreNextPoll) return;

	if (_musicData != NULL)
		pollMusic();
}

void SkyGmMusic::setupPointers(void) {

	_musicDataLoc = (_musicData[0x7DD] << 8) | _musicData[0x7DC];
	_sysExSequence = ((_musicData[0x7E1] << 8) | _musicData[0x7E0]) + _musicData;
}

void SkyGmMusic::setupChannels(uint8 *channelData) {

	_numberOfChannels = channelData[0];
	channelData++;
	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++) {
		uint16 chDataStart = ((channelData[(cnt << 1) | 1] << 8) | channelData[cnt << 1]) + _musicDataLoc;
		_channels[cnt] = new SkyGmChannel(_musicData, chDataStart, _midiDrv, _mt32_to_gm, _veloTab);
	}
}

void SkyGmMusic::startDriver(void) {

	//_midiDrv->send(0xFF);  //ALSA can't handle this.
	// skip all sysEx as it can't be handled anyways.
}

byte SkyGmMusic::_mt32_to_gm[128] = {
//    0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	  0,   1,   0,   2,   4,   4,   5,   3,  16,  17,  18,  16,  16,  19,  20,  21, // 0x
	  6,   6,   6,   7,   7,   7,   8, 112,  62,  62,  63,  63,  38,  38,  39,  39, // 1x
	 88,  95,  52,  98,  97,  99,  14,  54, 102,  96,  53, 102,  81, 100,  14,  80, // 2x
	 48,  48,  49,  45,  41,  40,  42,  42,  43,  46,  45,  24,  25,  28,  27, 104, // 3x
	 32,  32,  34,  33,  36,  37,  35,  35,  79,  73,  72,  72,  74,  75,  64,  65, // 4x
	 66,  67,  71,  71,  68,  69,  70,  22,  56,  59,  57,  57,  60,  60,  58,  61, // 5x
	 61,  11,  11,  98,  14,   9,  14,  13,  12, 107, 107,  77,  78,  78,  76,  76, // 6x
	 47, 117, 127, 118, 118, 116, 115, 119, 115, 112,  55, 124, 123,   0,  14, 117, // 7x
};

uint8 SkyGmMusic::_veloTab[128] = {
0x00, 0x40, 0x41, 0x41, 0x42, 0x42, 0x43, 0x43, 0x44, 0x44,
0x45, 0x45, 0x46, 0x46, 0x47, 0x47, 0x48, 0x48, 0x49, 0x49,
0x4A, 0x4A, 0x4B, 0x4B, 0x4C, 0x4C, 0x4D, 0x4D, 0x4E, 0x4E,
0x4F, 0x4F, 0x50, 0x50, 0x51, 0x51, 0x52, 0x52, 0x53, 0x53,
0x54, 0x54, 0x55, 0x55, 0x56, 0x56, 0x57, 0x57, 0x58, 0x58,
0x59, 0x59, 0x5A, 0x5A, 0x5B, 0x5B, 0x5C, 0x5C, 0x5D, 0x5D,
0x5E, 0x5E, 0x5F, 0x5F, 0x60, 0x60, 0x61, 0x61, 0x62, 0x62,
0x63, 0x63, 0x64, 0x64, 0x65, 0x65, 0x66, 0x66, 0x67, 0x67,
0x68, 0x68, 0x69, 0x69, 0x6A, 0x6A, 0x6B, 0x6B, 0x6C, 0x6C,
0x6D, 0x6D, 0x6E, 0x6E, 0x6F, 0x6F, 0x70, 0x70, 0x71, 0x71,
0x72, 0x72, 0x73, 0x73, 0x74, 0x74, 0x75, 0x75, 0x76, 0x76,
0x77, 0x77, 0x78, 0x78, 0x79, 0x79, 0x7A, 0x7A, 0x7B, 0x7B,
0x7C, 0x7C, 0x7D, 0x7D, 0x7E, 0x7E, 0x7F, 0x7F };