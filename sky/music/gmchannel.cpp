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

// the MT32 is a lot more sensitive to velocities than most general midi devices,
// so we need to boost them a little
/*uint8 SkyGmChannel::_veloTab[128] = {
0x00, 0x30, 0x31, 0x31, 0x32, 0x33, 0x33, 0x34, 0x34, 0x35, 0x36, 0x36, 0x37,
0x38, 0x38, 0x39, 0x39, 0x3A, 0x3B, 0x3B, 0x3C, 0x3D, 0x3D, 0x3E, 0x3E, 0x3F,
0x40, 0x40, 0x41, 0x42, 0x42, 0x43, 0x43, 0x44, 0x45, 0x45, 0x46, 0x47, 0x47,
0x48, 0x48, 0x49, 0x4A, 0x4A, 0x4B, 0x4C, 0x4C, 0x4D, 0x4D, 0x4E, 0x4F, 0x4F,
0x50, 0x50, 0x51, 0x52, 0x52, 0x53, 0x54, 0x54, 0x55, 0x55, 0x56, 0x57, 0x57,
0x58, 0x59, 0x59, 0x5A, 0x5A, 0x5B, 0x5C, 0x5C, 0x5D, 0x5E, 0x5E, 0x5F, 0x5F,
0x60, 0x61, 0x61, 0x62, 0x63, 0x63, 0x64, 0x64, 0x65, 0x66, 0x66, 0x67, 0x68,
0x68, 0x69, 0x69, 0x6A, 0x6B, 0x6B, 0x6C, 0x6C, 0x6D, 0x6E, 0x6E, 0x6F, 0x70,
0x70, 0x71, 0x71, 0x72, 0x73, 0x73, 0x74, 0x75, 0x75, 0x76, 0x76, 0x77, 0x78,
0x78, 0x79, 0x7A, 0x7A, 0x7B, 0x7B, 0x7C, 0x7D, 0x7D, 0x7E, 0x7F };*/
uint8 SkyGmChannel::_veloTab[128] = {
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

SkyGmChannel::SkyGmChannel(uint8 *pMusicData, uint16 startOfData, MidiDriver *pMidiDrv, byte *pInstMap)
{
	_musicData = pMusicData;
	_midiDrv = pMidiDrv;
	_channelData.midiChannelNumber = 0;
	_channelData.startOfData = startOfData;
	_channelData.eventDataPtr = startOfData;
	_channelData.channelActive = 1;
	_channelData.nextEventTime = getNextEventTime();
	_mt32_to_gm = pInstMap;

	_musicVolume = 0x100;
}

bool SkyGmChannel::isActive(void) {

	return _channelData.channelActive != 0;
}

void SkyGmChannel::updateVolume(uint16 pVolume) {

	_musicVolume = pVolume;
}

void SkyGmChannel::stopNote(void) {

	_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | 0x7B00 | 0 | 0x79000000);
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
			uint8 velocity = _veloTab[_musicData[_channelData.eventDataPtr]];
			_channelData.eventDataPtr++;
			_midiDrv->send((0x90 | _channelData.midiChannelNumber) | (opcode << 8) | (velocity << 16));
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
