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

#include "sky/gmmusic.h"

void SkyGmMusic::passTimerFunc(void *param) {

	((SkyGmMusic*)param)->timerCall();
}

SkyGmMusic::SkyGmMusic(MidiDriver *pMidiDrv, SkyDisk *pSkyDisk)
	: SkyMusicBase(pSkyDisk) {

	_driverFileBase = 60200;
    _midiDrv = pMidiDrv;
	int midiRes = _midiDrv->open();
	if (midiRes != 0) {
		printf("Error code: %d\n",midiRes);
	}
	_midiDrv->setTimerCallback(this, passTimerFunc);
	ignoreNextPoll = false;
}

SkyGmMusic::~SkyGmMusic(void) {

	_midiDrv->close();
	_midiDrv->setTimerCallback(NULL, NULL);
	delete _midiDrv;
}

void SkyGmMusic::timerCall(void) {

	// midi driver polls hundred times per sec. We only want 50 times.
	ignoreNextPoll = !ignoreNextPoll;
	if (!ignoreNextPoll) return;

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
		_channels[cnt] = new SkyGmChannel(_musicData, chDataStart, _midiDrv);
	}
}

void SkyGmMusic::startDriver(void) {

	_midiDrv->send(0xFF);
}
