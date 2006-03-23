/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#include "cine/cine.h"
#include "cine/sound_driver.h"

#include "sound/mixer.h"

namespace Cine {

void SoundDriver::setUpdateCallback(UpdateCallback upCb, void *ref) {
	_upCb = upCb;
	_upRef = ref;
}

void SoundDriver::findNote(int freq, int *note, int *oct) const {
	*note = _noteTableCount - 1;
	for (int i = 0; i < _noteTableCount; ++i) {
		if (_noteTable[i] <= freq) {
			*note = i;
			break;
		}
	}
	*oct = *note / 12;
}

void SoundDriver::resetChannel(int channel) {
	stopChannel(channel);
	stopSound();
}

AdlibSoundDriver::AdlibSoundDriver(Audio::Mixer *mixer)
	: _mixer(mixer) {
	_sampleRate = _mixer->getOutputRate();
	_opl = makeAdlibOPL(_sampleRate);
	memset(_channelsVolumeTable, 0, sizeof(_channelsVolumeTable));
	memset(_instrumentsTable, 0, sizeof(_instrumentsTable));
	initCard();
	_mixer->setupPremix(this);
}

AdlibSoundDriver::~AdlibSoundDriver() {
	_mixer->setupPremix(NULL);
}

void AdlibSoundDriver::setupChannel(int channel, const byte *data, int instrument, int volume) {
	assert(channel < 4);
	if (data) {
		if (volume > 80) {
			volume = 80;
		} else if (volume < 0) {
			volume = 0;
		}
		volume += volume / 4;
		if (volume > 127) {
			volume = 127;
		}
		_channelsVolumeTable[channel] = volume;
		setupInstrument(data, channel);
	}
}

void AdlibSoundDriver::stopChannel(int channel) {
	assert(channel < 4);
	AdlibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins) {
		if (ins->mode != 0 && ins->channel == 6) {
			channel = 6;
		}
		if (ins->mode == 0 || ins->channel == 6) {
			OPLWriteReg(_opl, 0xB0 | channel, 0);
		}
		if (ins->mode != 0) {
			_vibrato &= (1 << (10 - ins->channel)) ^ 0xFF;
			OPLWriteReg(_opl, 0xBD, _vibrato);
		}
	}
}

void AdlibSoundDriver::stopSound() {
	int i;
	for (i = 0; i < 18; ++i) {
		OPLWriteReg(_opl, 0x40 | _operatorsTable[i], 63);
	}
	for (i = 0; i < 9; ++i) {
		OPLWriteReg(_opl, 0xB0 | i, 0);
	}
	OPLWriteReg(_opl, 0xBD, 0);
}

int AdlibSoundDriver::readBuffer(int16 *buffer, const int numSamples) {
	update(buffer, numSamples / 2);
	// convert mono to stereo
	for (int i = numSamples / 2 - 1; i >= 0; i--) {
		buffer[2 * i] = buffer[2 * i + 1] = buffer[i];
	}
	return numSamples;
}

void AdlibSoundDriver::initCard() {
	_vibrato = 0x20;
	OPLWriteReg(_opl, 0xBD, _vibrato);
	OPLWriteReg(_opl, 0x08, 0x40);
	
	int i;
	for (i = 0; i < 18; ++i) {
		OPLWriteReg(_opl, 0x40 | _operatorsTable[i], 0);
	}
	for (i = 0; i < 9; ++i) {
		OPLWriteReg(_opl, 0xB0 | i, 0);
	}
	for (i = 0; i < 9; ++i) {
		OPLWriteReg(_opl, 0xC0 | i, 0);
	}
	for (i = 0; i < 18; ++i) {
		OPLWriteReg(_opl, 0x60 | _operatorsTable[i], 0);
	}
	for (i = 0; i < 18; ++i) {
		OPLWriteReg(_opl, 0x80 | _operatorsTable[i], 0);
	}
	for (i = 0; i < 18; ++i) {
		OPLWriteReg(_opl, 0x20 | _operatorsTable[i], 0);
	}
	for (i = 0; i < 18; ++i) {
		OPLWriteReg(_opl, 0xE0 | _operatorsTable[i], 0);
	}

	OPLWriteReg(_opl, 1, 0x20);
	OPLWriteReg(_opl, 1, 0);
}

void AdlibSoundDriver::update(int16 *buf, int len) {
	static int samplesLeft = 0;
	while (len != 0) {
		int count = samplesLeft;
		if (count > len) {
			count = len;
		}
		samplesLeft -= count;
		len -= count;
		YM3812UpdateOne(_opl, buf, count);
		if (samplesLeft == 0) {
			if (_upCb) {
				(*_upCb)(_upRef);
			}
			samplesLeft = _sampleRate / 50;
		}
		buf += count;
	}
}

void AdlibSoundDriver::setupInstrument(const byte *data, int channel) {
	assert(channel < 4);
	AdlibSoundInstrument *ins = &_instrumentsTable[channel];
	loadInstrument(data, ins);

	int mod, car, tmp;
	const AdlibRegisterSoundInstrument *reg;

	if (ins->mode != 0)  {
		mod = _operatorsTable[_voiceOperatorsTable[2 * ins->channel + 0]];
		car = _operatorsTable[_voiceOperatorsTable[2 * ins->channel + 1]];
	} else {
		mod = _operatorsTable[_voiceOperatorsTable[2 * channel + 0]];
		car = _operatorsTable[_voiceOperatorsTable[2 * channel + 1]];
	}
	
	if (ins->mode == 0 || ins->channel == 6) {
		reg = &ins->regMod;
		OPLWriteReg(_opl, 0x20 | mod, reg->vibrato);
		if (reg->freqMod) {
			tmp = reg->outputLevel & 0x3F;
		} else {
			tmp = (63 - (reg->outputLevel & 0x3F)) * _channelsVolumeTable[channel];
			tmp = 63 - (2 * tmp + 127) / (2 * 127);
		}
		OPLWriteReg(_opl, 0x40 | mod, tmp | (reg->keyScaling << 6));
		OPLWriteReg(_opl, 0x60 | mod, reg->attackDecay);
		OPLWriteReg(_opl, 0x80 | mod, reg->sustainRelease);
		if (ins->mode != 0) {
			OPLWriteReg(_opl, 0xC0 | ins->channel, reg->feedbackStrength);
		} else {
			OPLWriteReg(_opl, 0xC0 | channel, reg->feedbackStrength);
		}
		OPLWriteReg(_opl, 0xE0 | mod, ins->waveSelectMod);
	}

	reg = &ins->regCar;
	OPLWriteReg(_opl, 0x20 | car, reg->vibrato);
	tmp = (63 - (reg->outputLevel & 0x3F)) * _channelsVolumeTable[channel];
	tmp = 63 - (2 * tmp + 127) / (2 * 127);
	OPLWriteReg(_opl, 0x40 | car, tmp | (reg->keyScaling << 6));
	OPLWriteReg(_opl, 0x60 | car, reg->attackDecay);
	OPLWriteReg(_opl, 0x80 | car, reg->sustainRelease);
	OPLWriteReg(_opl, 0xE0 | car, ins->waveSelectCar);
}

void AdlibSoundDriver::loadRegisterInstrument(const byte *data, AdlibRegisterSoundInstrument *reg) {
	reg->vibrato = 0;
	if (READ_LE_UINT16(data + 18)) { // amplitude vibrato
		reg->vibrato |= 0x80;
	}
	if (READ_LE_UINT16(data + 20)) { // frequency vibrato
		reg->vibrato |= 0x40;
	}
	if (READ_LE_UINT16(data + 10)) { // sustaining sound
		reg->vibrato |= 0x20;
	}
	if (READ_LE_UINT16(data + 22)) { // envelope scaling
		reg->vibrato |= 0x10;
	}
	reg->vibrato |= READ_LE_UINT16(data + 2) & 0xF; // frequency multiplier
	
	reg->attackDecay = READ_LE_UINT16(data + 6) << 4; // attack rate
	reg->attackDecay |= READ_LE_UINT16(data + 12) & 0xF; // decay rate
	
	reg->sustainRelease = READ_LE_UINT16(data + 8) << 4; // sustain level
	reg->sustainRelease |= READ_LE_UINT16(data + 14) & 0xF; // release rate
	
	reg->feedbackStrength = READ_LE_UINT16(data + 4) << 1; // feedback
	if (READ_LE_UINT16(data + 24) == 0) { // frequency modulation
		reg->feedbackStrength |= 1;
	}
	
	reg->keyScaling = READ_LE_UINT16(data);
	reg->outputLevel = READ_LE_UINT16(data + 16);
	reg->freqMod = READ_LE_UINT16(data + 24);
}

void AdlibSoundDriverINS::loadInstrument(const byte *data, AdlibSoundInstrument *asi) {
	asi->mode = *data++;
	asi->channel = *data++;
	loadRegisterInstrument(data, &asi->regMod); data += 26;
	loadRegisterInstrument(data, &asi->regCar); data += 26;
	asi->waveSelectMod = data[0] & 3; data += 2;
	asi->waveSelectCar = data[0] & 3; data += 2;
	asi->amDepth = data[0]; data += 2;
}

void AdlibSoundDriverINS::setChannelFrequency(int channel, int frequency) {
	assert(channel < 4);
	AdlibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins) {
		if (ins->mode != 0 && ins->channel == 6) {
			channel = 6;
		}
		if (ins->mode == 0 || ins->channel == 6) {
			int freq, note, oct;
			findNote(frequency, &note, &oct);
			if (channel == 6) {
				note %= 12;
			}
			freq = _freqTable[note % 12];
			OPLWriteReg(_opl, 0xA0 | channel, freq);
			freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
			if (ins->mode == 0) {
				freq |= 0x20;
			}
			OPLWriteReg(_opl, 0xB0 | channel, freq);
		}
		if (ins->mode != 0) {
			_vibrato |= 1 << (10 - ins->channel);
			OPLWriteReg(_opl, 0xBD, _vibrato);
		}
	}
}

void AdlibSoundDriverINS::playSound(const byte *data, int channel, int volume) {
	assert(channel < 4);
	_channelsVolumeTable[channel] = 127;
	resetChannel(channel);
	setupInstrument(data + 257, channel);
	AdlibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		channel = 6;
	}
	if (ins->mode == 0 || channel == 6) {
		int freq = _freqTable[0];
		OPLWriteReg(_opl, 0xA0 | channel, freq);
		freq = 4 | ((freq & 0x300) >> 8);
		if (ins->mode == 0) {
			freq |= 0x20;
		}
		OPLWriteReg(_opl, 0xB0 | channel, freq);
	}
	if (ins->mode != 0) {
		_vibrato = 1 << (10 - ins->channel);
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
}

void AdlibSoundDriverADL::loadInstrument(const byte *data, AdlibSoundInstrument *asi) {
	asi->mode = *data++;
	asi->channel = *data++;
	asi->waveSelectMod = *data++ & 3;
	asi->waveSelectCar = *data++ & 3;
	asi->amDepth = *data++;
	++data;
	loadRegisterInstrument(data, &asi->regMod); data += 26;
	loadRegisterInstrument(data, &asi->regCar); data += 26;
}

void AdlibSoundDriverADL::setChannelFrequency(int channel, int frequency) {
	assert(channel < 4);
	AdlibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins) {
		if (ins->mode != 0) {
			channel = ins->channel;
			if (channel == 9) {
				channel = 8;
			} else if (channel == 10) {
				channel = 7;
			}
		}
		int freq, note, oct;
		findNote(frequency, &note, &oct);
		
		note += oct * 12;
		if (ins->amDepth) {
			note = ins->amDepth;
		}
		if (note < 0) {
			note = 0;
		}

		freq = _freqTable[note % 12];
		OPLWriteReg(_opl, 0xA0 | channel, freq);
		freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
		if (ins->mode == 0) {
			freq |= 0x20;
		}
		OPLWriteReg(_opl, 0xB0 | channel, freq);
		if (ins->mode != 0) {
			_vibrato = 1 << (10 - channel);
			OPLWriteReg(_opl, 0xBD, _vibrato);
		}
	}
}

void AdlibSoundDriverADL::playSound(const byte *data, int channel, int volume) {
	assert(channel < 4);
	_channelsVolumeTable[channel] = 127;
	setupInstrument(data, channel);
	AdlibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		OPLWriteReg(_opl, 0xB0 | channel, 0);
	}
	if (ins->mode != 0) {
		_vibrato = (1 << (10 - ins->channel)) ^ 0xFF;
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
	if (ins->mode != 0) {
		channel = ins->channel;
		if (channel == 9) {
			channel = 8;
		} else if (channel == 10) {
			channel = 7;
		}
	}
	uint16 note = 48;
	if (ins->amDepth) {
		note = ins->amDepth;
	}
	int freq = _freqTable[note % 12];
	OPLWriteReg(_opl, 0xA0 | channel, freq);
	freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
	if (ins->mode == 0) {
		freq |= 0x20;
	}
	OPLWriteReg(_opl, 0xB0 | channel, freq);
	if (ins->mode != 0) {
		_vibrato = 1 << (10 - channel);
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
}

const int SoundDriver::_noteTable[] = {
	0xEEE, 0xE17, 0xD4D, 0xC8C, 0xBD9, 0xB2F, 0xA8E, 0x9F7,
	0x967, 0x8E0, 0x861, 0x7E8, 0x777, 0x70B, 0x6A6, 0x647,
	0x5EC, 0x597, 0x547, 0x4FB, 0x4B3, 0x470, 0x430, 0x3F4,
	0x3BB, 0x385, 0x353, 0x323, 0x2F6, 0x2CB, 0x2A3, 0x27D,
	0x259, 0x238, 0x218, 0x1FA, 0x1DD, 0x1C2, 0x1A9, 0x191,
	0x17B, 0x165, 0x151, 0x13E, 0x12C, 0x11C, 0x10C, 0x0FD,
	0x0EE, 0x0E1, 0x0D4, 0x0C8, 0x0BD, 0x0B2, 0x0A8, 0x09F,
	0x096, 0x08E, 0x086, 0x07E, 0x077, 0x070, 0x06A, 0x064,
	0x05E, 0x059, 0x054, 0x04F, 0x04B, 0x047, 0x043, 0x03F,
	0x03B, 0x038, 0x035, 0x032, 0x02F, 0x02C, 0x02A, 0x027,
	0x025, 0x023, 0x021, 0x01F, 0x01D, 0x01C, 0x01A, 0x019,
	0x017, 0x016, 0x015, 0x013, 0x012, 0x011, 0x010, 0x00F
};

const int SoundDriver::_noteTableCount = ARRAYSIZE(_noteTable);

const int AdlibSoundDriver::_freqTable[] = {
	0x157, 0x16C, 0x181, 0x198, 0x1B1, 0x1CB,
	0x1E6, 0x203, 0x222, 0x243, 0x266, 0x28A
};

const int AdlibSoundDriver::_freqTableCount = ARRAYSIZE(_freqTable);

const int AdlibSoundDriver::_operatorsTable[] = {
	0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13,	16, 17, 18, 19, 20, 21
};

const int AdlibSoundDriver::_operatorsTableCount = ARRAYSIZE(_operatorsTable);

const int AdlibSoundDriver::_voiceOperatorsTable[] = {
	0, 3, 1, 4, 2, 5, 6, 9, 7, 10, 8, 11, 12, 15, 16, 16, 14, 14, 17, 17, 13, 13
};

const int AdlibSoundDriver::_voiceOperatorsTableCount = ARRAYSIZE(_voiceOperatorsTable);

} // End of namespace Cine
