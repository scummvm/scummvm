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

#include "gmchannel.h"

// instrument map copied from scumm/instrument.cpp

const byte SkyGmChannel::_mt32_to_gm[128] = {
//    0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	  0,   1,   0,   2,   4,   4,   5,   3,  16,  17,  18,  16,  16,  19,  20,  21, // 0x
	  6,   6,   6,   7,   7,   7,   8, 112,  62,  62,  63,  63,  38,  38,  39,  39, // 1x
	 88,  95,  52,  98,  97,  99,  14,  54, 102,  96,  53, 102,  81, 100,  14,  80, // 2x
	 48,  48,  49,  45,  41,  40,  42,  42,  43,  46,  45,  24,  25,  28,  27, 104, // 3x
	 32,  32,  34,  33,  36,  37,  35,  35,  79,  73,  72,  72,  74,  75,  64,  65, // 4x
	 66,  67,  71,  71,  68,  69,  70,  22,  56,  59,  57,  57,  60,  60,  58,  61, // 5x
	 61,  11,  11,  98,  14,   9,  14,  13,  12, 107, 107,  77,  78,  78,  76,  76, // 6x
	 47, 117, 127, 118, 118, 116, 115, 119, 115, 112,  55, 124, 123,   0,  14, 117  // 7x
};

SkyGmChannel::SkyGmChannel(uint8 *pMusicData, uint16 startOfData, MidiDriver *pMidiDrv)
{
	_musicData = pMusicData;
	_midiDrv = pMidiDrv;
	_channelData.startOfData = startOfData;
	_channelData.eventDataPtr = startOfData;
	_channelData.channelActive = 1;
	_channelData.nextEventTime = getNextEventTime();

	_musicVolume = 0x100;
}

void SkyGmChannel::updateVolume(uint16 pVolume) {

	_musicVolume = pVolume;
}

void SkyGmChannel::stopNote(void) {

	_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | 0x7B00 | 0 | 0x79000000);
	_midiDrv->send(0);
}

int32 SkyGmChannel::getNextEventTime(void)
{
	int32 retV = 0; 
	uint8 cnt, lVal;
	for (cnt = 0; cnt < 4; cnt++) {
		lVal = _musicData[_channelData.eventDataPtr];
		_channelData.eventDataPtr++;
		retV = (retV << 7) | (lVal & 0x7F);
		if (!(lVal & 0x80)) break;
	}
	if (lVal & 0x80) { // should never happen
		return -1;
	} else return retV;

}

uint8 SkyGmChannel::process(uint16 aktTime) {

	if (!_channelData.channelActive)
		return 0;
	
	uint8 returnVal = 0;

	_channelData.nextEventTime -= aktTime;
	uint8 opcode;

	while ((_channelData.nextEventTime < 0) && (_channelData.channelActive)) {
		opcode = _musicData[_channelData.eventDataPtr];
		_channelData.eventDataPtr++;
		if (opcode&0x80) {
			if (opcode == 0xFF) {
				// dummy opcode
			} else if (opcode >= 0x90) {
				switch (opcode&0xF) {
					case 0: com90_caseNoteOff(); break;
					case 1: com90_stopChannel(); break;
					case 2: com90_setupInstrument(); break;
					case 3: 
						returnVal = com90_updateTempo();
						break;
					case 5: com90_getPitch(); break;
					case 6: com90_getChannelVolume(); break;
					case 8: com90_rewindMusic(); break;
					case 9: com90_keyOff(); break;
					case 11: com90_getChannelPanValue(); break;
					case 12: com90_setStartOfData(); break;
					case 13: com90_getChannelControl(); break;
					case 4: //com90_dummy();
					case 7: //com90_skipTremoVibro();
					case 10: //com90_error();
						error("SkyChannel: dummy music routine 0x%02X was called",opcode);
						_channelData.channelActive = 0;
						break;
					default:
						// these opcodes aren't implemented in original music driver
						error("SkyChannel: Not existant routine 0x%02X was called",opcode);
						_channelData.channelActive = 0;
						break;
				}
			} else {
				// new midi channel assignment
				_channelData.midiChannelNumber = opcode&0xF;
			}
		} else {
			_channelData.note = opcode;
			_midiDrv->send((0x90 | _channelData.midiChannelNumber) | (opcode << 8) | (_musicData[_channelData.eventDataPtr] << 16));
			_channelData.eventDataPtr++;
		}
		if (_channelData.channelActive)
			_channelData.nextEventTime += getNextEventTime();
	}
	return returnVal;
}

//- command 90h routines

void SkyGmChannel::com90_caseNoteOff(void) {

	_midiDrv->send((0x90 | _channelData.midiChannelNumber) | (_musicData[_channelData.eventDataPtr] << 8));
	_channelData.eventDataPtr++;
}

void SkyGmChannel::com90_stopChannel(void) {

	stopNote();
	_channelData.channelActive = 0;
}

void SkyGmChannel::com90_setupInstrument(void) {

	_midiDrv->send((0xC0 | _channelData.midiChannelNumber) | (_mt32_to_gm[_musicData[_channelData.eventDataPtr]] << 8));
	_channelData.eventDataPtr++;
}

uint8 SkyGmChannel::com90_updateTempo(void) {

	uint8 retV = _musicData[_channelData.eventDataPtr];
	_channelData.eventDataPtr++;
	return retV;
}

void SkyGmChannel::com90_getPitch(void) {

	_midiDrv->send((0xE0 | _channelData.midiChannelNumber) | 0 | (_musicData[_channelData.eventDataPtr] << 16));
	_channelData.eventDataPtr++;
}

void SkyGmChannel::com90_getChannelVolume(void) {

	_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | 0x700 | (_musicData[_channelData.eventDataPtr] << 16));
	_channelData.eventDataPtr++;
}

void SkyGmChannel::com90_rewindMusic(void) {

	_channelData.eventDataPtr = _channelData.startOfData;
}

void SkyGmChannel::com90_keyOff(void) {

	_midiDrv->send((0x90 | _channelData.midiChannelNumber) | (_channelData.note << 8) | 0);
}

void SkyGmChannel::com90_setStartOfData(void) {

	_channelData.startOfData = _channelData.eventDataPtr;
}

void SkyGmChannel::com90_getChannelPanValue(void) {

	_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | 0x0A00 | (_musicData[_channelData.eventDataPtr] << 16));
	_channelData.eventDataPtr++;
}

void SkyGmChannel::com90_getChannelControl(void) {

	uint8 conNum = _musicData[_channelData.eventDataPtr];
	uint8 conDat = _musicData[_channelData.eventDataPtr + 1];
	_channelData.eventDataPtr += 2;
	_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | (conNum << 8) | (conDat << 16));
}
