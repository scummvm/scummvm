/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
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

#include "sky/music/gmmusic.h"
#include "sky/music/gmchannel.h"
#include "sky/sky.h"
#include "common/util.h"
#include "sound/mididrv.h"

namespace Sky {

void GmMusic::passTimerFunc(void *param) {

	((GmMusic*)param)->timerCall();
}

GmMusic::GmMusic(MidiDriver *pMidiDrv, Disk *pDisk)
	: MusicBase(pDisk) {

	_driverFileBase = 60200;
	_midiDrv = pMidiDrv;
	int midiRes = _midiDrv->open();
	if (midiRes != 0)
		error("Can't open midi device. Errorcode: %d", midiRes);
	_timerCount = 0;
	_midiDrv->setTimerCallback(this, passTimerFunc);
}

GmMusic::~GmMusic(void) {

	_midiDrv->setTimerCallback(NULL, NULL);
	if (_currentMusic)
		stopMusic();
	// Send All Sound Off and All Notes Off (for external synths)
	for (int i = 0; i < 16; ++i) {
		_midiDrv->send ((120 << 8) | 0xB0 | i);
		_midiDrv->send ((123 << 8) | 0xB0 | i);
	}
	_midiDrv->close();
	delete _midiDrv;
}

void GmMusic::setVolume(uint8 volume) {

	setFMVolume(volume);
}

void GmMusic::timerCall(void) {
	_timerCount += _midiDrv->getBaseTempo();
	if (_timerCount > (1000000 / 50)) {
		// call pollMusic() 50 times per second
		_timerCount -= 1000000 / 50;
		if (_musicData != NULL)
			pollMusic();
	}
}

void GmMusic::setupPointers(void) {

	if (SkyEngine::_systemVars.gameVersion == 109) {
		_musicDataLoc = (_musicData[0x79C] << 8) | _musicData[0x79B];
		_sysExSequence = _musicData + 0x1EF2;
	} else {
		_musicDataLoc = (_musicData[0x7DD] << 8) | _musicData[0x7DC];
		_sysExSequence = ((_musicData[0x7E1] << 8) | _musicData[0x7E0]) + _musicData;
	}
}

void GmMusic::setupChannels(uint8 *channelData) {

	_numberOfChannels = channelData[0];
	channelData++;
	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++) {
		uint16 chDataStart = ((channelData[(cnt << 1) | 1] << 8) | channelData[cnt << 1]) + _musicDataLoc;
		_channels[cnt] = new GmChannel(_musicData, chDataStart, _midiDrv, MidiDriver::_mt32ToGm, _veloTab);
		_channels[cnt]->updateVolume(_musicVolume);
	}
}

void GmMusic::startDriver(void) {
	// Send GM System On to reset channel parameters on external and capa$
	uint8 sysEx[] = "\xf0\x7e\x7f\x09\x01\xf7";
	_midiDrv->sysEx(sysEx, 6);
	//_midiDrv->send(0xFF);  //ALSA can't handle this.
	// skip all sysEx as it can't be handled anyways.
}

const byte GmMusic::_veloTab[128] = {
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
	0x7C, 0x7C, 0x7D, 0x7D, 0x7E, 0x7E, 0x7F, 0x7F
};

} // End of namespace Sky
