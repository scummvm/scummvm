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

#include "common/util.h"
#include "sky/music/mt32music.h"

void SkyMT32Music::passTimerFunc(void *param) {

	((SkyMT32Music*)param)->timerCall();
}

SkyMT32Music::SkyMT32Music(MidiDriver *pMidiDrv, SkyDisk *pSkyDisk, OSystem *system)
	: SkyMusicBase(pSkyDisk, system) {

	_driverFileBase = 60200;
    _midiDrv = pMidiDrv;
	int midiRes = _midiDrv->open();
	if (midiRes != 0) {
		error("Can't open midi device. Errorcode: %d",midiRes);
	}
	_midiDrv->setTimerCallback(this, passTimerFunc);
	_ignoreNextPoll = false;
	for (uint8 cnt = 0; cnt < 128; cnt++)
		_dummyMap[cnt] = cnt;
}

SkyMT32Music::~SkyMT32Music(void) {

	_midiDrv->close();
	_midiDrv->setTimerCallback(NULL, NULL);
	delete _midiDrv;
}

void SkyMT32Music::timerCall(void) {

	// midi driver polls hundred times per sec. We only want 50 times.
	_ignoreNextPoll = !_ignoreNextPoll;
	if (!_ignoreNextPoll) return;

	if (_musicData != NULL)
		pollMusic();
}

void SkyMT32Music::setVolume(uint8 volume) {

	uint8 sysEx[6];
	_musicVolume = volume;
	sysEx[0] = 0x7F; sysEx[1] = 0x7F; sysEx[2] = 0x04; sysEx[3] = 0x01;
	sysEx[4] = 0; sysEx[5] = volume & 0x7F;
	_midiDrv->sysEx(sysEx, 6);
}

void SkyMT32Music::setupPointers(void) {

	_musicDataLoc = (_musicData[0x7DD] << 8) | _musicData[0x7DC];
	_sysExSequence = ((_musicData[0x7E1] << 8) | _musicData[0x7E0]) + _musicData;
}

void SkyMT32Music::setupChannels(uint8 *channelData) {

	_numberOfChannels = channelData[0];
	channelData++;
	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++) {
		uint16 chDataStart = ((channelData[(cnt << 1) | 1] << 8) | channelData[cnt << 1]) + _musicDataLoc;
		_channels[cnt] = new SkyGmChannel(_musicData, chDataStart, _midiDrv, _dummyMap, _dummyMap);
	}
}

#define MIDI_PACK(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

bool SkyMT32Music::processPatchSysEx(uint8 *sysExData) {

	uint8 sysExBuf[15];
	uint8 crc = 0;
	if (sysExData[0] & 0x80) return false;

	// decompress data from stream
	sysExBuf[0]  = 0x41; sysExBuf[1] = 0x10; sysExBuf[2] = 0x16; sysExBuf[3] = 0x12; sysExBuf[4] = 0x5;
	sysExBuf[5]  = sysExData[0] >> 4;			// patch offset part 1
	sysExBuf[6]  = (sysExData[0] & 0xF) << 3;	// patch offset part 2
	sysExBuf[7]  = sysExData[1] >> 6;			// timbre group
	sysExBuf[8]  = sysExData[1] & 0x3F;			// timbre num
	sysExBuf[9]  = sysExData[2] & 0x3F;			// key shift
	sysExBuf[10] = sysExData[3] & 0x7F;			// fine tune
	sysExBuf[11] = sysExData[4] & 0x7F;         // bender range
	sysExBuf[12] = sysExData[2] >> 6;			// assign mode
	sysExBuf[13] = sysExData[3] >> 7;			// reverb switch
	for (uint8 cnt = 4; cnt < 14; cnt++)
		crc -= sysExBuf[cnt];
	sysExBuf[14] = crc;							// crc
	_midiDrv->sysEx(sysExBuf, 15);
	return true;
}

void SkyMT32Music::startDriver(void) {

	// setup timbres and patches using SysEx data
	uint8* sysExData = _sysExSequence;
	uint8 timbreNum = sysExData[0];
	uint8 cnt, crc;
	sysExData++;
	uint8 sendBuf[256];
	uint8 len;
	sendBuf[0] = 0x41; sendBuf[1] = 0x10; sendBuf[2] = 0x16; sendBuf[3] = 0x12;
	for (cnt = 0; cnt < timbreNum; cnt++) {
		len = 7;
		crc = 0;
		// Timbre address
		sendBuf[4] = 0x8 | (sysExData[0] >> 6);
		sendBuf[5] = (sysExData[0] & 0x3F) << 1;
		sendBuf[6] = 0xA;
		sysExData++;
		crc -= sendBuf[4] + sendBuf[5] + sendBuf[6];
		uint8 dataLen = sysExData[0];
		sysExData++;
		// Timbre data:
		do {
			uint8 rlVal = 1;
			uint8 codeVal = sysExData[0];
			sysExData++;

			if (codeVal & 0x80) {
				codeVal &= 0x7F;
				rlVal = sysExData[0];
				sysExData++;
				dataLen--;
			}
			for (uint8 cnt2 = 0; cnt2 < rlVal; cnt2++) {
				sendBuf[len] = codeVal;
				len++;
				crc -= codeVal;
			}
			dataLen--;
		} while (dataLen > 0);
		sendBuf[len] = crc & 0x7F;
		len++;
		_midiDrv->sysEx(sendBuf, len);
	}

	while (processPatchSysEx(sysExData))
		sysExData += 5;
}
