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

#include "sky/mt32music.h"

void SkyMT32Music::passTimerFunc(void *param) {

	((SkyMT32Music*)param)->timerCall();
}

SkyMT32Music::SkyMT32Music(MidiDriver *pMidiDrv, SkyDisk *pSkyDisk, uint32 version)
	: SkyMusicBase(pSkyDisk, version) {

	_driverFileBase = 60200;
    _midiDrv = pMidiDrv;
	int midiRes = _midiDrv->open();
	if (midiRes != 0) {
		error("Can't open midi device. Errorcode: %d\n",midiRes);
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

void SkyMT32Music::setupPointers(void) {

	_musicDataLoc = (_musicData[0x7DD] << 8) | _musicData[0x7DC];
	_sysExSequence = ((_musicData[0x7E1] << 8) | _musicData[0x7E0]) + _musicData;
}

void SkyMT32Music::setupChannels(uint8 *channelData) {

	_numberOfChannels = channelData[0];
	channelData++;
	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++) {
		uint16 chDataStart = ((channelData[(cnt << 1) | 1] << 8) | channelData[cnt << 1]) + _musicDataLoc;
		_channels[cnt] = new SkyGmChannel(_musicData, chDataStart, _midiDrv, _dummyMap);
	}
}

#define MIDI_PACK(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

bool SkyMT32Music::processPatchSysEx(uint8 *sysExData) {

	uint8 crc = 0;
	if (sysExData[0] & 0x80) return false;
	uint8 patchNum = sysExData[0];
	sysExData++;

	uint8 timbreGroup = sysExData[0] >> 6;
	uint8 timbreNumber = sysExData[0] & 0x3F;
	uint8 keyShift = sysExData[1] & 0x3F;
	uint8 fineTune = sysExData[2] & 0x7F;
	uint8 benderRange = sysExData[3] & 0x7F;
	uint8 assignMode = sysExData[1] >> 6;
	uint8 reverbSwitch = sysExData[2] >> 7;
	
	_midiDrv->send(MIDI_PACK(0xF0, 0x41, 0x10, 0x16));
	_midiDrv->send(MIDI_PACK(0x12, 5, patchNum >> 4, (patchNum & 0xF) << 3));

	crc -= 5 + (patchNum >> 4) + ((patchNum & 0xF) << 3);
	crc -= timbreGroup + timbreNumber + keyShift + fineTune;
	crc -= benderRange + assignMode + reverbSwitch;

	_midiDrv->send(MIDI_PACK(timbreGroup, timbreNumber, keyShift, fineTune));
	_midiDrv->send(MIDI_PACK(benderRange, assignMode, reverbSwitch, crc));
	_midiDrv->send(0xF7);

	debug(3," Patch %02X:\n",patchNum);
	debug(3," Timbre Group:  %d\n",timbreGroup);
	debug(3," Timbre Number: %d\n",timbreNumber);
	debug(3," Key Shift:     %d\n",keyShift);
	debug(3," Fine Tune:     %d\n",fineTune);
	debug(3," Bender Range:  %d\n",benderRange);
	debug(3," Assign Mode:   %d\n",assignMode);
	debug(3," Reverb Switch: %d\n\n",reverbSwitch);
	return true;
}

void SkyMT32Music::startDriver(void) {

	_midiDrv->send(0xFF); // reset midi device
	
	// setup timbres and patches using SysEx data
	uint8* sysExData = _sysExSequence;
	uint8 timbreNum = sysExData[0];
	uint8 cnt, crc;
	uint32 sysComb;
	sysExData++;
	for (cnt = 0; cnt < timbreNum; cnt++) {
		crc = 0;
		_midiDrv->send(MIDI_PACK(0xF0, 0x41, 0x10, 0x16));
		//- sendTimbreAddress
		sysComb = (0x2 << 16) | (sysExData[0] << 8) | 0xA;
		sysExData++;
		uint8 sysByte1 = (uint8)(sysComb >> 14);
		uint8 sysByte2 = (uint8)((sysComb & 0x3FFF) >> 7);
		uint8 sysByte3 = (uint8)(sysComb & 0x7F);
		_midiDrv->send(MIDI_PACK(0x12, sysByte1, sysByte2, sysByte3));
		debug(3,"InitBySysEx: Timbre address: %02X:%02X:%02X (%02X)\n",sysByte1,sysByte2,sysByte3,(sysExData-1)[0]);
		crc -= sysByte1 + sysByte2 + sysByte3;
		//- sendTimbreData
		uint8 dataLen = sysExData[0];
		debug(3,"[%02X]",dataLen);
		sysExData++;
		uint32 nextSend = 0; 
		uint8 bytesInSend = 0;
		debug(3,"         Timbre Data:");
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
			for (uint8 cnt = 0; cnt < rlVal; cnt++) {
				nextSend |= codeVal << (bytesInSend << 3);
				crc -= codeVal;
				debug(3," %02X",codeVal);
				bytesInSend++;
				if (bytesInSend == 4) {
					_midiDrv->send(nextSend);
					nextSend = bytesInSend = 0;
				}
			}
			dataLen--;
		} while (dataLen > 0);
		crc &= 0x7F;
		debug(3," %02X F7\n",crc);
		nextSend |= crc << (bytesInSend << 3);
		bytesInSend++;
		if (bytesInSend == 4) {
			_midiDrv->send(nextSend);
			nextSend = bytesInSend = 0;
		}
		nextSend |= 0xF7 << (bytesInSend << 3);
		_midiDrv->send(nextSend);
	}

	while (processPatchSysEx(sysExData))
		sysExData += 5;
}
