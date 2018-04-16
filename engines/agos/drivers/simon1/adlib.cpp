/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "agos/drivers/simon1/adlib.h"

#include "common/textconsole.h"
#include "common/util.h"
#include "common/file.h"

namespace AGOS {

enum {
	kChannelUnused       = 0xFF,
	kChannelOrphanedFlag = 0x80,

	kOPLVoicesCount = 9
};

MidiDriver_Simon1_AdLib::Voice::Voice()
    : channel(kChannelUnused), note(0), instrTotalLevel(0), instrScalingLevel(0), frequency(0) {
}

MidiDriver_Simon1_AdLib::MidiDriver_Simon1_AdLib(const byte *instrumentData)
    : _isOpen(false), _opl(nullptr), _timerProc(nullptr), _timerParam(nullptr),
      _melodyVoices(0), _amvdrBits(0), _rhythmEnabled(false), _voices(), _midiPrograms(),
      _instruments(instrumentData) {
}

MidiDriver_Simon1_AdLib::~MidiDriver_Simon1_AdLib() {
	close();
	delete[] _instruments;
}

int MidiDriver_Simon1_AdLib::open() {
	if (_isOpen) {
		return MERR_ALREADY_OPEN;
	}

	_opl = OPL::Config::create();
	if (!_opl) {
		return MERR_DEVICE_NOT_AVAILABLE;
	}

	if (!_opl->init()) {
		delete _opl;
		_opl = nullptr;

		return MERR_CANNOT_CONNECT;
	}

	_opl->start(new Common::Functor0Mem<void, MidiDriver_Simon1_AdLib>(this, &MidiDriver_Simon1_AdLib::onTimer));

	_opl->writeReg(0x01, 0x20);
	_opl->writeReg(0x08, 0x40);
	_opl->writeReg(0xBD, 0xC0);
	reset();

	_isOpen = true;
	return 0;
}

bool MidiDriver_Simon1_AdLib::isOpen() const {
	return _isOpen;
}

void MidiDriver_Simon1_AdLib::close() {
	setTimerCallback(nullptr, nullptr);

	if (_isOpen) {
		_opl->stop();
		delete _opl;
		_opl = nullptr;

		_isOpen = false;
	}
}

void MidiDriver_Simon1_AdLib::send(uint32 b) {
	int channel = b & 0x0F;
	int command = b & 0xF0;
	int param1  = (b >>  8) & 0xFF;
	int param2  = (b >> 16) & 0xFF;

	// The percussion channel is handled specially. The AdLib output uses
	// channels 11 to 15 for percussions. For this, the original converted
	// note on on the percussion channel to note on channels 11 to 15 before
	// giving it to the AdLib output. We do this in here for simplicity.
	if (command == 0x90 && channel == 9) {
		param1 -= 36;
		if (param1 < 0 || param1 >= ARRAYSIZE(_rhythmMap)) {
			return;
		}

		channel = _rhythmMap[param1].channel;
		MidiDriver::send(0xC0 | channel, _rhythmMap[param1].program, 0);

		param1 = _rhythmMap[param1].note;
		MidiDriver::send(0x80 | channel, param1, param2);

		param2 >>= 1;
	}

	switch (command) {
	case 0x80: // note OFF
		noteOff(channel, param1);
		break;

	case 0x90: // note ON
		if (param2 == 0) {
			noteOff(channel, param1);
		} else {
			noteOn(channel, param1, param2);
		}
		break;

	case 0xB0: // control change
		controlChange(channel, param1, param2);
		break;

	case 0xC0: // program change
		programChange(channel, param1);
		break;

	default:
		break;
	}
}

void MidiDriver_Simon1_AdLib::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	_timerParam = timer_param;
	_timerProc = timer_proc;
}

uint32 MidiDriver_Simon1_AdLib::getBaseTempo() {
	return 1000000 / OPL::OPL::kDefaultCallbackFrequency;
}

void MidiDriver_Simon1_AdLib::onTimer() {
	if (_timerProc) {
		(*_timerProc)(_timerParam);
	}
}

void MidiDriver_Simon1_AdLib::reset() {
	resetOPLVoices();
	resetRhythm();
	for (int i = 0; i < kNumberOfVoices; ++i) {
		_voices[i].channel = kChannelUnused;
	}
	resetVoices();
}

void MidiDriver_Simon1_AdLib::resetOPLVoices() {
	_amvdrBits &= 0xE0;
	_opl->writeReg(0xBD, _amvdrBits);
	for (int i = 8; i >= 0; --i) {
		_opl->writeReg(0xB0 + i, 0);
	}
}

void MidiDriver_Simon1_AdLib::resetRhythm() {
	_melodyVoices = 9;
	_amvdrBits = 0xC0;
	_opl->writeReg(0xBD, _amvdrBits);
}

void MidiDriver_Simon1_AdLib::resetVoices() {
	memset(_midiPrograms, 0, sizeof(_midiPrograms));
	for (int i = 0; i < kNumberOfVoices; ++i) {
		_voices[i].channel = kChannelUnused;
	}

	for (int i = 0; i < kOPLVoicesCount; ++i) {
		resetRhythm();
		_opl->writeReg(0x08, 0x00);

		int oplRegister = _operatorMap[i];
		for (int j = 0; j < 4; ++j) {
			oplRegister += 0x20;

			_opl->writeReg(oplRegister + 0, _operatorDefaults[2 * j + 0]);
			_opl->writeReg(oplRegister + 3, _operatorDefaults[2 * j + 1]);
		}

		_opl->writeReg(oplRegister + 0x60, 0x00);
		_opl->writeReg(oplRegister + 0x63, 0x00);

		// This seems to be serious bug but the original does it the same way.
		_opl->writeReg(_operatorMap[i] + i, 0x08);
	}
}

int MidiDriver_Simon1_AdLib::allocateVoice(uint channel) {
	for (int i = 0; i < _melodyVoices; ++i) {
		if (_voices[i].channel == (channel | kChannelOrphanedFlag)) {
			return i;
		}
	}

	for (int i = 0; i < _melodyVoices; ++i) {
		if (_voices[i].channel == kChannelUnused) {
			return i;
		}
	}

	for (int i = 0; i < _melodyVoices; ++i) {
		if (_voices[i].channel > 0x7F) {
			return i;
		}
	}

	// The original had some logic for a priority based reuse of channels.
	// However, the priority value is always 0, which causes the first channel
	// to be picked all the time.
	const int voice = 0;
	_opl->writeReg(0xA0 + voice, (_voices[voice].frequency     ) & 0xFF);
	_opl->writeReg(0xB0 + voice, (_voices[voice].frequency >> 8) & 0xFF);
	return voice;
}

void MidiDriver_Simon1_AdLib::noteOff(uint channel, uint note) {
	if (_melodyVoices <= 6 && channel >= 11) {
		_amvdrBits &= ~(_rhythmInstrumentMask[channel - 11]);
		_opl->writeReg(0xBD, _amvdrBits);
	} else {
		for (int i = 0; i < _melodyVoices; ++i) {
			if (_voices[i].note == note && _voices[i].channel == channel) {
				_voices[i].channel |= kChannelOrphanedFlag;
				_opl->writeReg(0xA0 + i, (_voices[i].frequency     ) & 0xFF);
				_opl->writeReg(0xB0 + i, (_voices[i].frequency >> 8) & 0xFF);
				return;
			}
		}
	}
}

void MidiDriver_Simon1_AdLib::noteOn(uint channel, uint note, uint velocity) {
	if (_rhythmEnabled && channel >= 11) {
		noteOnRhythm(channel, note, velocity);
		return;
	}

	const int voiceNum = allocateVoice(channel);
	Voice &voice = _voices[voiceNum];

	if ((voice.channel & 0x7F) != channel) {
		setupInstrument(voiceNum, _midiPrograms[channel]);
	}
	voice.channel = channel;

	_opl->writeReg(0x43 + _operatorMap[voiceNum], (0x3F - (((velocity | 0x80) * voice.instrTotalLevel) >> 8)) | voice.instrScalingLevel);

	voice.note = note;
	if (note >= 0x80) {
		note = 0;
	}

	const int frequencyAndOctave = _frequencyIndexAndOctaveTable[note];
	const uint frequency = _frequencyTable[frequencyAndOctave & 0x0F];

	uint highByte = ((frequency & 0xFF00) >> 8) | ((frequencyAndOctave & 0x70) >> 2);
	uint lowByte  = frequency & 0x00FF;
	voice.frequency = (highByte << 8) | lowByte;

	_opl->writeReg(0xA0 + voiceNum, lowByte);
	_opl->writeReg(0xB0 + voiceNum, highByte | 0x20);
}

void MidiDriver_Simon1_AdLib::noteOnRhythm(uint channel, uint note, uint velocity) {
	const uint voiceNum = channel - 5;
	Voice &voice = _voices[voiceNum];

	_amvdrBits |= _rhythmInstrumentMask[voiceNum - 6];

	const uint level = (0x3F - (((velocity | 0x80) * voice.instrTotalLevel) >> 8)) | voice.instrScalingLevel;
	if (voiceNum == 6) {
		_opl->writeReg(0x43 + _rhythmOperatorMap[voiceNum - 6], level);
	} else {
		_opl->writeReg(0x40 + _rhythmOperatorMap[voiceNum - 6], level);
	}

	voice.note = note;
	if (note >= 0x80) {
		note = 0;
	}

	const int frequencyAndOctave = _frequencyIndexAndOctaveTable[note];
	const uint frequency = _frequencyTable[frequencyAndOctave & 0x0F];

	uint highByte = ((frequency & 0xFF00) >> 8) | ((frequencyAndOctave & 0x70) >> 2);
	uint lowByte  = frequency & 0x00FF;
	voice.frequency = (highByte << 8) | lowByte;

	const uint oplOperator = _rhythmVoiceMap[voiceNum - 6];
	_opl->writeReg(0xA0 + oplOperator, lowByte);
	_opl->writeReg(0xB0 + oplOperator, highByte);

	_opl->writeReg(0xBD, _amvdrBits);
}

void MidiDriver_Simon1_AdLib::controlChange(uint channel, uint controller, uint value) {
	// Enable/Disable Rhythm Section
	if (controller == 0x67) {
		resetVoices();
		_rhythmEnabled = (value != 0);

		if (_rhythmEnabled) {
			_melodyVoices = 6;
			_amvdrBits = 0xE0;
		} else {
			_melodyVoices = 9;
			_amvdrBits = 0xC0;
		}

		_voices[6].channel = kChannelUnused;
		_voices[7].channel = kChannelUnused;
		_voices[8].channel = kChannelUnused;

		_opl->writeReg(0xBD, _amvdrBits);
	}
}

void MidiDriver_Simon1_AdLib::programChange(uint channel, uint program) {
	_midiPrograms[channel] = program;

	if (_rhythmEnabled && channel >= 11) {
		setupInstrument(channel - 5, program);
	} else {
		// Fully unallocate all previously allocated but now unused voices for
		// this MIDI channel.
		for (uint i = 0; i < kOPLVoicesCount; ++i) {
			if (_voices[i].channel == (channel | kChannelOrphanedFlag)) {
				_voices[i].channel = kChannelUnused;
			}
		}

		// Set the program for all voices allocted for this MIDI channel.
		for (uint i = 0; i < kOPLVoicesCount; ++i) {
			if (_voices[i].channel == channel) {
				setupInstrument(i, program);
			}
		}
	}
}

void MidiDriver_Simon1_AdLib::setupInstrument(uint voice, uint instrument) {
	const byte *instrumentData = _instruments + instrument * 16;

	int scaling = instrumentData[3];
	if (_rhythmEnabled && voice >= 7) {
		scaling = instrumentData[2];
	}

	const int scalingLevel = scaling & 0xC0;
	const int totalLevel   = scaling & 0x3F;

	_voices[voice].instrScalingLevel = scalingLevel;
	_voices[voice].instrTotalLevel   = (-(totalLevel - 0x3F)) & 0xFF;

	if (!_rhythmEnabled || voice <= 6) {
		int oplRegister = _operatorMap[voice];
		for (int j = 0; j < 4; ++j) {
			oplRegister += 0x20;
			_opl->writeReg(oplRegister + 0, *instrumentData++);
			_opl->writeReg(oplRegister + 3, *instrumentData++);
		}
		oplRegister += 0x60;
		_opl->writeReg(oplRegister + 0, *instrumentData++);
		_opl->writeReg(oplRegister + 3, *instrumentData++);

		_opl->writeReg(0xC0 + voice, *instrumentData++);
	} else {
		voice -= 7;

		int oplRegister = _rhythmOperatorMap[voice + 1];
		for (int j = 0; j < 4; ++j) {
			oplRegister += 0x20;
			_opl->writeReg(oplRegister + 0, *instrumentData++);
			++instrumentData;
		}
		oplRegister += 0x60;
		_opl->writeReg(oplRegister + 0, *instrumentData++);
		++instrumentData;

		_opl->writeReg(0xC0 + _rhythmVoiceMap[voice + 1], *instrumentData++);
	}
}

const int MidiDriver_Simon1_AdLib::_operatorMap[9] = {
	0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11,
	0x12
};

const int MidiDriver_Simon1_AdLib::_operatorDefaults[8] = {
	0x01, 0x11, 0x4F, 0x00, 0xF1, 0xF2, 0x53, 0x74
};

const int MidiDriver_Simon1_AdLib::_rhythmOperatorMap[5] = {
	0x10, 0x14, 0x12, 0x15, 0x11
};

const uint MidiDriver_Simon1_AdLib::_rhythmInstrumentMask[5] = {
	0x10, 0x08, 0x04, 0x02, 0x01
};

const int MidiDriver_Simon1_AdLib::_rhythmVoiceMap[5] = {
	6, 7, 8, 8, 7
};

const int MidiDriver_Simon1_AdLib::_frequencyIndexAndOctaveTable[128] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x00, 0x01, 0x02, 0x03,
	0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1A, 0x1B, 0x20, 0x21, 0x22, 0x23,
	0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3A, 0x3B, 0x40, 0x41, 0x42, 0x43,
	0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5A, 0x5B, 0x60, 0x61, 0x62, 0x63,
	0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7A, 0x7B, 0x70, 0x71, 0x72, 0x73,
	0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B,
	0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B
};

const int MidiDriver_Simon1_AdLib::_frequencyTable[16] = {
	0x0157, 0x016B, 0x0181, 0x0198, 0x01B0, 0x01CA, 0x01E5, 0x0202,
	0x0220, 0x0241, 0x0263, 0x0287, 0x2100, 0xD121, 0xA307, 0x46A4
};

const MidiDriver_Simon1_AdLib::RhythmMap MidiDriver_Simon1_AdLib::_rhythmMap[39] = {
	{ 11, 123,  40 },
	{ 12, 127,  50 },
	{ 12, 124,   1 },
	{ 12, 124,  90 },
	{ 13, 125,  50 },
	{ 13, 125,  25 },
	{ 15, 127,  80 },
	{ 13, 125,  25 },
	{ 15, 127,  40 },
	{ 13, 125,  35 },
	{ 15, 127,  90 },
	{ 13, 125,  35 },
	{ 13, 125,  45 },
	{ 14, 126,  90 },
	{ 13, 125,  45 },
	{ 15, 127,  90 },
	{  0,   0,   0 },
	{ 15, 127,  60 },
	{  0,   0,   0 },
	{ 13, 125,  60 },
	{  0,   0,   0 },
	{  0,   0,   0 },
	{  0,   0,   0 },
	{ 13, 125,  45 },
	{ 13, 125,  40 },
	{ 13, 125,  35 },
	{ 13, 125,  30 },
	{ 13, 125,  25 },
	{ 13, 125,  80 },
	{ 13, 125,  40 },
	{ 13, 125,  80 },
	{ 13, 125,  40 },
	{ 14, 126,  40 },
	{ 15, 127,  60 },
	{  0,   0,   0 },
	{  0,   0,   0 },
	{ 14, 126,  80 },
	{  0,   0,   0 },
	{ 13, 125, 100 }
};

MidiDriver *createMidiDriverSimon1AdLib(const char *instrumentFilename) {
	// Load instrument data.
	Common::File ibk;

	if (!ibk.open(instrumentFilename)) {
		return nullptr;
	}

	if (ibk.readUint32BE() != 0x49424b1a) {
		return nullptr;
	}

	byte *instrumentData = new byte[128 * 16];
	if (ibk.read(instrumentData, 128 * 16) != 128 * 16) {
		delete[] instrumentData;
		return nullptr;
	}

	return new MidiDriver_Simon1_AdLib(instrumentData);
}

} // End of namespace AGOS
