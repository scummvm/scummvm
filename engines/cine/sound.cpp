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

#include "common/endian.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/timer.h"
#include "common/mutex.h"
#include "common/config-manager.h"

#include "cine/cine.h"
#include "cine/sound.h"

#include "backends/audiocd/audiocd.h"

#include "audio/audiostream.h"
#include "audio/fmopl.h"
#include "audio/mididrv.h"
#include "audio/decoders/raw.h"
#include "audio/mods/soundfx.h"

namespace Cine {

class PCSoundDriver {
public:
	typedef void (*UpdateCallback)(void *);

	virtual ~PCSoundDriver() {}

	virtual MusicType musicType() const = 0;
	virtual void setupChannel(int channel, const byte *data, int instrument, int volume) = 0;
	virtual void setChannelFrequency(int channel, int frequency) = 0;
	virtual void stopChannel(int channel) = 0;
	virtual void playSample(int mode, int channel, int param3, int param4, int param5, int size) = 0;
	virtual void playSample(const byte *data, int size, int channel, int volume) = 0;
	virtual void stopAll() = 0;
	virtual const char *getInstrumentExtension() const { return ""; }
	virtual void notifyInstrumentLoad(const byte *data, int size, int channel) {}

	virtual void setUpdateCallback(UpdateCallback upCb, void *ref) = 0;
	void resetChannel(int channel);
	void findNote(int freq, int *note, int *oct) const;

protected:

	static const int _noteTable[];
	static const int _noteTableCount;
};

// 8 octaves, 12 notes per octave
const int PCSoundDriver::_noteTable[] = {
	0xEEE, 0xE17, 0xD4D, 0xC8C, 0xBD9, 0xB2F, 0xA8E, 0x9F7,	0x967, 0x8E0, 0x861, 0x7E8,
	0x777, 0x70B, 0x6A6, 0x647, 0x5EC, 0x597, 0x547, 0x4FB, 0x4B3, 0x470, 0x430, 0x3F4,
	0x3BB, 0x385, 0x353, 0x323, 0x2F6, 0x2CB, 0x2A3, 0x27D,	0x259, 0x238, 0x218, 0x1FA,
	0x1DD, 0x1C2, 0x1A9, 0x191,	0x17B, 0x165, 0x151, 0x13E, 0x12C, 0x11C, 0x10C, 0x0FD,
	0x0EE, 0x0E1, 0x0D4, 0x0C8, 0x0BD, 0x0B2, 0x0A8, 0x09F,	0x096, 0x08E, 0x086, 0x07E,
	0x077, 0x070, 0x06A, 0x064,	0x05E, 0x059, 0x054, 0x04F, 0x04B, 0x047, 0x043, 0x03F,
	0x03B, 0x038, 0x035, 0x032, 0x02F, 0x02C, 0x02A, 0x027,	0x025, 0x023, 0x021, 0x01F,
	0x01D, 0x01C, 0x01A, 0x019,	0x017, 0x016, 0x015, 0x013, 0x012, 0x011, 0x010, 0x00F
};

const int PCSoundDriver::_noteTableCount = ARRAYSIZE(_noteTable);

struct AdLibRegisterSoundInstrument {
	uint8 vibrato;
	uint8 attackDecay;
	uint8 sustainRelease;
	uint8 feedbackStrength;
	uint8 keyScaling;
	uint8 outputLevel;
	uint8 freqMod;
};

struct AdLibSoundInstrument {
	byte mode;
	byte channel;
	AdLibRegisterSoundInstrument regMod;
	AdLibRegisterSoundInstrument regCar;
	byte waveSelectMod;
	byte waveSelectCar;
	byte amDepth;
};

class AdLibSoundDriver : public PCSoundDriver {
public:
	AdLibSoundDriver(Audio::Mixer *mixer);
	~AdLibSoundDriver() override;

	MusicType musicType() const override { return MT_ADLIB; }
	// PCSoundDriver interface
	void setUpdateCallback(UpdateCallback upCb, void *ref) override;
	void setupChannel(int channel, const byte *data, int instrument, int volume) override;	
	void stopAll() override;

	void initCard();
	void onTimer();
	void setupPreloadedInstrument(int channel);
	void setupInstrument(const byte *data, int channel, bool loadData = true);
	void loadRegisterInstrument(const byte *data, AdLibRegisterSoundInstrument *reg);
	virtual void loadInstrument(const byte *data, AdLibSoundInstrument *asi) = 0;

	enum {
		MAX_ADLIB_CHANNELS = 8
	};

protected:
	UpdateCallback _upCb;
	void *_upRef;

	OPL::OPL *_opl;
	Audio::Mixer *_mixer;

	byte _vibrato;
	int _channelsVolumeTable[MAX_ADLIB_CHANNELS];
	AdLibSoundInstrument _instrumentsTable[MAX_ADLIB_CHANNELS];

	static const int _freqTable[];
	static const int _freqTableCount;
	static const int _operatorsTable[];
	static const int _operatorsTableCount;
	static const int _voiceOperatorsTable[];
	static const int _voiceOperatorsTableCount;
};

const int AdLibSoundDriver::_freqTable[] = {
	0x157, 0x16C, 0x181, 0x198, 0x1B1, 0x1CB,
	0x1E6, 0x203, 0x222, 0x243, 0x266, 0x28A
};

const int AdLibSoundDriver::_freqTableCount = ARRAYSIZE(_freqTable);

const int AdLibSoundDriver::_operatorsTable[] = {
	0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, 16, 17, 18, 19, 20, 21
};

const int AdLibSoundDriver::_operatorsTableCount = ARRAYSIZE(_operatorsTable);

const int AdLibSoundDriver::_voiceOperatorsTable[] = {
	0, 3, 1, 4, 2, 5, 6, 9, 7, 10, 8, 11, 12, 15, 16, 16, 14, 14, 17, 17, 13, 13
};

const int AdLibSoundDriver::_voiceOperatorsTableCount = ARRAYSIZE(_voiceOperatorsTable);

// Future Wars AdLib driver
class AdLibSoundDriverINS : public AdLibSoundDriver {
public:
	AdLibSoundDriverINS(Audio::Mixer *mixer) : AdLibSoundDriver(mixer) {}
	const char *getInstrumentExtension() const override { return ".INS"; }
	void loadInstrument(const byte *data, AdLibSoundInstrument *asi) override;
	void stopChannel(int channel) override;
	void setChannelFrequency(int channel, int frequency) override;
	void playSample(int mode, int channel, int param3, int param4, int param5, int size) override;
	void playSample(const byte *data, int size, int channel, int volume) override;
};

// Operation Stealth AdLib driver
class AdLibSoundDriverADL : public AdLibSoundDriver {
public:
	AdLibSoundDriverADL(Audio::Mixer *mixer);
	const char *getInstrumentExtension() const override { return ".ADL"; }
	void loadInstrument(const byte *data, AdLibSoundInstrument *asi) override;
	void stopChannel(int channel) override;
	void setChannelFrequency(int channel, int frequency) override;
	void playSample(int mode, int channel, int param3, int param4, int param5, int size) override;
	void playSample(const byte *data, int size, int channel, int volume) override;

protected:
	AdLibSoundInstrument _samples[49];
};

// (Future Wars) MIDI driver
class MidiSoundDriverH32 : public PCSoundDriver {
public:
	MidiSoundDriverH32(MidiDriver *output);
	~MidiSoundDriverH32() override;

	MusicType musicType() const override { return MT_MT32; }
	void setUpdateCallback(UpdateCallback upCb, void *ref) override;
	void setupChannel(int channel, const byte *data, int instrument, int volume) override;
	void setChannelFrequency(int channel, int frequency) override;
	void stopChannel(int channel) override;
	void playSample(int mode, int channel, int param3, int param4, int param5, int size) override;
	void playSample(const byte *data, int size, int channel, int volume) override;
	void stopAll() override {}
	const char *getInstrumentExtension() const override { return ".H32"; }
	void notifyInstrumentLoad(const byte *data, int size, int channel) override;


private:
	MidiDriver *_output;
	UpdateCallback _callback;
	Common::Mutex _mutex;

	void writeInstrument(int offset, const byte *data, int size);
	void selectInstrument(int channel, int timbreGroup, int timbreNumber, int volume);
	void selectInstrument2(int channel, int timbreGroup, int timbreNumber);
	void selectInstrument3(int channel, int offsetMode, int timbreGroup);
	void selectInstrument4(int offsetMode, int timbreGroup, int timbreNumber, int keyShift);
	void selectInstrument5(int messageNum);
};

class PCSoundFxPlayer {
public:

	PCSoundFxPlayer(PCSoundDriver *driver);
	~PCSoundFxPlayer();

	bool load(const char *song);
	void play();
	void stop();
	void fadeOut();

	static void updateCallback(void *ref);

	enum {
		NUM_INSTRUMENTS = 15,
		NUM_CHANNELS = 4
	};

private:

	void update();
	void handleEvents();
	void handlePattern(int channel, const byte *patternData);
	void unload();

	bool _playing;
	int _currentPos;
	int _currentOrder;
	int _numOrders;
	int _eventsDelay;
	int _fadeOutCounter;
	int _updateTicksCounter;
	int _instrumentsChannelTable[NUM_CHANNELS];
	byte *_sfxData;
	byte *_instrumentsData[NUM_INSTRUMENTS];
	PCSoundDriver *_driver;
	Common::Mutex _mutex;
};


void PCSoundDriver::findNote(int freq, int *note, int *oct) const {
	if (freq > 0x777)
		*oct = 0;
	else if (freq > 0x3BB)
		*oct = 1;
	else if (freq > 0x1DD)
		*oct = 2;
	else if (freq > 0x0EE)
		*oct = 3;
	else if (freq > 0x077)
		*oct = 4;
	else if (freq > 0x03B)
		*oct = 5;
	else if (freq > 0x01D)
		*oct = 6;
	else
		*oct = 7;

	*note = 11;
	for (int i = 0; i < 12; ++i) {
		if (_noteTable[*oct * 12 + i] <= freq) {
			*note = i;
			break;
		}
	}
}

void PCSoundDriver::resetChannel(int channel) {
	stopChannel(channel);
	stopAll();
}

AdLibSoundDriver::AdLibSoundDriver(Audio::Mixer *mixer)
	: _upCb(0), _upRef(0), _mixer(mixer) {

	_opl = OPL::Config::create();
	if (!_opl || !_opl->init())
		error("Failed to create OPL");

	memset(_channelsVolumeTable, 0, sizeof(_channelsVolumeTable));
	memset(_instrumentsTable, 0, sizeof(_instrumentsTable));
	initCard();
	// 1000/(10923000 ms / 1193180) ~= 109 Hz
	_opl->start(new Common::Functor0Mem<void, AdLibSoundDriver>(this, &AdLibSoundDriver::onTimer), 109);
}

AdLibSoundDriver::~AdLibSoundDriver() {
	delete _opl;
}

void AdLibSoundDriver::setUpdateCallback(UpdateCallback upCb, void *ref) {
	_upCb = upCb;
	_upRef = ref;
}

void AdLibSoundDriver::setupChannel(int channel, const byte *data, int instrument, int volume) {
	assert(channel < MAX_ADLIB_CHANNELS);
	if (data) {
		volume = CLIP(volume, 0, 80);
		volume += volume / 4;

		_channelsVolumeTable[channel] = volume;
		setupInstrument(data, channel);
	}
}

void AdLibSoundDriverINS::stopChannel(int channel) {
	assert(channel < MAX_ADLIB_CHANNELS);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		channel = 6;
	}
	if (ins->mode == 0 || channel == 6) {
		_opl->writeReg(0xB0 | channel, 0);
	}
	if (ins->mode != 0) {
		_vibrato &= ~(1 << (10 - ins->channel));
		_opl->writeReg(0xBD, _vibrato);
	}
}

void AdLibSoundDriverADL::stopChannel(int channel) {
	assert(channel < MAX_ADLIB_CHANNELS);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode == 0 || ins->channel == 6) {
		_opl->writeReg(0xB0 | channel, 0);
	}
	if (ins->mode != 0) {
		_vibrato &= ~(1 << (10 - ins->channel));
		_opl->writeReg(0xBD, _vibrato);
	}
}

void AdLibSoundDriver::stopAll() {
	int i;
	for (i = 0; i < 18; ++i) {
		_opl->writeReg(0x40 | _operatorsTable[i], 63);
	}
	for (i = 0; i < 9; ++i) {
		_opl->writeReg(0xB0 | i, 0);
	}
	if (g_cine->getGameType() == Cine::GType_OS) {
		_vibrato &= (1 << 5);
		_opl->writeReg(0xBD, _vibrato);
	} else {
		_opl->writeReg(0xBD, 0);
	}
}

void AdLibSoundDriver::initCard() {
	_vibrato = 0x20;
	_opl->writeReg(0xBD, _vibrato);
	_opl->writeReg(0x08, 0x40);

	static const int oplRegs[] = { 0x40, 0x60, 0x80, 0x20, 0xE0 };

	for (int i = 0; i < 9; ++i) {
		_opl->writeReg(0xB0 | i, 0);
	}
	for (int i = 0; i < 9; ++i) {
		_opl->writeReg(0xC0 | i, 0);
	}

	for (int j = 0; j < 5; j++) {
		for (int i = 0; i < 18; ++i) {
			_opl->writeReg(oplRegs[j] | _operatorsTable[i], 0);
		}
	}

	_opl->writeReg(1, 0x20);
	_opl->writeReg(1, 0);
}

void AdLibSoundDriver::onTimer() {
	if (_upCb) {
		(*_upCb)(_upRef);
	}
}

void AdLibSoundDriver::setupPreloadedInstrument(int channel) {
	setupInstrument(NULL, channel, false);
}

void AdLibSoundDriver::setupInstrument(const byte *data, int channel, bool loadData) {
	assert(channel < MAX_ADLIB_CHANNELS);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (loadData && data) {
		loadInstrument(data, ins);
	}

	int mod, car, tmp;
	const AdLibRegisterSoundInstrument *reg;

	if (ins->mode != 0)  {
		mod = _operatorsTable[_voiceOperatorsTable[2 * ins->channel + 0]];
		car = _operatorsTable[_voiceOperatorsTable[2 * ins->channel + 1]];
	} else {
		mod = _operatorsTable[_voiceOperatorsTable[2 * channel + 0]];
		car = _operatorsTable[_voiceOperatorsTable[2 * channel + 1]];
	}

	if (ins->mode == 0 || ins->channel == 6) {
		reg = &ins->regMod;
		_opl->writeReg(0x20 | mod, reg->vibrato);
		if (reg->freqMod) {
			tmp = reg->outputLevel & 0x3F;
		} else {
			tmp = (63 - (reg->outputLevel & 0x3F)) * _channelsVolumeTable[channel];
			tmp = 63 - (2 * tmp + 127) / (2 * 127);
		}
		_opl->writeReg(0x40 | mod, tmp | (reg->keyScaling << 6));
		_opl->writeReg(0x60 | mod, reg->attackDecay);
		_opl->writeReg(0x80 | mod, reg->sustainRelease);
		if (ins->mode != 0) {
			_opl->writeReg(0xC0 | ins->channel, reg->feedbackStrength);
		} else {
			_opl->writeReg(0xC0 | channel, reg->feedbackStrength);
		}
		_opl->writeReg(0xE0 | mod, ins->waveSelectMod);
	}

	reg = &ins->regCar;
	_opl->writeReg(0x20 | car, reg->vibrato);
	tmp = (63 - (reg->outputLevel & 0x3F)) * _channelsVolumeTable[channel];
	tmp = 63 - (2 * tmp + 127) / (2 * 127);
	_opl->writeReg(0x40 | car, tmp | (reg->keyScaling << 6));
	_opl->writeReg(0x60 | car, reg->attackDecay);
	_opl->writeReg(0x80 | car, reg->sustainRelease);
	_opl->writeReg(0xE0 | car, ins->waveSelectCar);
}

void AdLibSoundDriver::loadRegisterInstrument(const byte *data, AdLibRegisterSoundInstrument *reg) {
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

void AdLibSoundDriverINS::loadInstrument(const byte *data, AdLibSoundInstrument *asi) {
	asi->mode = *data++;
	asi->channel = *data++;
	loadRegisterInstrument(data, &asi->regMod); data += 26;
	loadRegisterInstrument(data, &asi->regCar); data += 26;
	asi->waveSelectMod = data[0] & 3; data += 2;
	asi->waveSelectCar = data[0] & 3; data += 2;
	asi->amDepth = data[0]; data += 2;
}

void AdLibSoundDriverINS::setChannelFrequency(int channel, int frequency) {
	assert(channel < MAX_ADLIB_CHANNELS);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		channel = 6;
	}
	if (ins->mode == 0 || ins->channel == 6) {
		int freq, note, oct;
		findNote(frequency, &note, &oct);
		if (channel == 6)
			oct = 0;
		freq = _freqTable[note % 12];
		_opl->writeReg(0xA0 | channel, freq);
		freq = (oct << 2) | ((freq & 0x300) >> 8);
		if (ins->mode == 0) {
			freq |= 0x20;
		}
		_opl->writeReg(0xB0 | channel, freq);
	}
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - ins->channel);
		_opl->writeReg(0xBD, _vibrato);
	}
}

void AdLibSoundDriverINS::playSample(int mode, int channel, int param3, int param4, int param5, int size) {
}

void AdLibSoundDriverINS::playSample(const byte *data, int size, int channel, int volume) {
	assert(channel < MAX_ADLIB_CHANNELS);
	_channelsVolumeTable[channel] = 127;
	resetChannel(channel);
	setupInstrument(data + 257, channel);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		channel = 6;
	}
	if (ins->mode == 0 || channel == 6) {
		uint16 note = 12;
		int freq = _freqTable[note % 12];
		_opl->writeReg(0xA0 | channel, freq);
		freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
		if (ins->mode == 0) {
			freq |= 0x20;
		}
		_opl->writeReg(0xB0 | channel, freq);
	}
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - ins->channel);
		_opl->writeReg(0xBD, _vibrato);
	}
}

AdLibSoundDriverADL::AdLibSoundDriverADL(Audio::Mixer *mixer)
	: AdLibSoundDriver(mixer), _samples() {

	memset(_samples, 0, sizeof(_samples));
}

void AdLibSoundDriverADL::loadInstrument(const byte *data, AdLibSoundInstrument *asi) {
	asi->mode = *data++;
	asi->channel = *data++;
	asi->waveSelectMod = *data++ & 3;
	asi->waveSelectCar = *data++ & 3;
	asi->amDepth = *data++;
	++data;
	loadRegisterInstrument(data, &asi->regMod); data += 26;
	loadRegisterInstrument(data, &asi->regCar); data += 26;
}

void AdLibSoundDriverADL::setChannelFrequency(int channel, int frequency) {
	assert(channel < MAX_ADLIB_CHANNELS);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	setupPreloadedInstrument(channel);
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
	if (ins->amDepth) {
		note = ins->amDepth;
		oct = note / 12;
	}
	if (note < 0) {
		note = 0;
		oct = 0;
	}

	freq = _freqTable[note % 12];
	_opl->writeReg(0xA0 | channel, freq);
	freq = (oct << 2) | ((freq & 0x300) >> 8);
	if (ins->mode == 0) {
		freq |= 0x20;
	}
	_opl->writeReg(0xB0 | channel, freq);
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - channel);
		_opl->writeReg(0xBD, _vibrato);
	}
}

void AdLibSoundDriverADL::playSample(int mode, int channel, int param3, int param4, int param5, int size) {
	switch (mode) {
	case 0:
		_instrumentsTable[(channel & 1) + 4] = _samples[param3];
		_channelsVolumeTable[(channel & 1) + 4] = 0x7F;
		stopChannel((channel & 1) + 4);
		if (param5 >= 0x0C && param5 <= 0x6C) {
			setChannelFrequency((channel & 1) + 4, param5);
		}
		break;
	case 1:
		if (channel <= 0x30) {
			AnimData& animData = g_cine->_animDataTable[param3];
			const byte *data = animData.data();
			if (data && animData.size() >= 58) {
				loadInstrument(data, &_samples[channel]);
			}
		}
		break;
	}
}

void AdLibSoundDriverADL::playSample(const byte *data, int size, int channel, int volume) {
	assert(channel < MAX_ADLIB_CHANNELS);
	_channelsVolumeTable[channel] = 127;
	setupInstrument(data, channel);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		_opl->writeReg(0xB0 | channel, 0);
	}
	if (ins->mode != 0) {
		_vibrato &= ~(1 << (10 - ins->channel));
		_opl->writeReg(0xBD, _vibrato);
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
	_opl->writeReg(0xA0 | channel, freq);
	freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
	if (ins->mode == 0) {
		freq |= 0x20;
	}
	_opl->writeReg(0xB0 | channel, freq);
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - channel);
		_opl->writeReg(0xBD, _vibrato);
	}
}

MidiSoundDriverH32::MidiSoundDriverH32(MidiDriver *output)
	: _output(output), _callback(0), _mutex() {
}

MidiSoundDriverH32::~MidiSoundDriverH32() {
	if (_callback)
		g_system->getTimerManager()->removeTimerProc(_callback);

	_output->close();
	delete _output;
}

void MidiSoundDriverH32::setUpdateCallback(UpdateCallback upCb, void *ref) {
	Common::StackLock lock(_mutex);

	Common::TimerManager *timer = g_system->getTimerManager();
	assert(timer);

	if (_callback)
		timer->removeTimerProc(_callback);

	_callback = upCb;
	if (_callback) // 10923000 ms / 1193180 ~= 9155 microseconds
		timer->installTimerProc(_callback, 9155, ref, "MidiSoundDriverH32");
}

void MidiSoundDriverH32::setupChannel(int channel, const byte *data, int instrument, int volume) {
	Common::StackLock lock(_mutex);

	if (volume < 0 ||  volume > 100)
		volume = 0;

	if (!data)
		selectInstrument(channel, 0, 0, volume);
	// In case the instrument is a builtin instrument select it directly.
	else if (data[0] < 0x80)
		selectInstrument(channel, data[0] / 0x40, data[0] % 0x40, volume);
	// In case we use a custom instrument we need to specify the timbre group
	// 2, which means it's a timbre from the timbre memory area.
	else
		selectInstrument(channel, 2, instrument, volume);
}

void MidiSoundDriverH32::setChannelFrequency(int channel, int frequency) {
	Common::StackLock lock(_mutex);

	int note, oct;
	findNote(frequency, &note, &oct);
	note %= 12;
	note = oct * 12 + note + 12;

	_output->send(0x91 + channel, note, 0x7F);
}

void MidiSoundDriverH32::stopChannel(int channel) {
	Common::StackLock lock(_mutex);

	_output->send(0xB1 + channel, 0x7B, 0x00);
}

void MidiSoundDriverH32::playSample(int mode, int channel, int param3, int param4, int param5, int size) {
	Common::StackLock lock(_mutex);

	switch (mode) {
	case 0: // play instrument
		if (param5 >= 0x0C && param5 <= 0x6C) {
			selectInstrument2(channel + 4, 2, param3 + 0xF);
			selectInstrument3(channel + 4, 1, param4);
		}

		stopChannel(channel + 4);

		if (param5 >= 0x0C && param5 <= 0x6C) {
			_output->send(0x91 + channel + 4, param5, 0x7F);
		}
		break;
	case 1: // load instrument
		if (channel <= 0x30) {
			AnimData& animData = g_cine->_animDataTable[param3];
			const byte *data = animData.data();
			if (data && data[0] >= 0x80 && data[0] < 0xC0) {
				writeInstrument((channel + 15) * 512 + 0x80000, data + 1, animData.size() - 1);
			}
		}
		break;
	case 2:
		selectInstrument3(channel + 4, param3, param4);
		break;
	case 3:
		selectInstrument4(channel, param3, param4, param5);
		break;
	case 4: // show text in Roland MT-32 LCD display
		// Don't display text in Roland MT-32 LCD display when loading a savegame
		if (!runOnlyUntilFreePartRangeFirst200) {
			selectInstrument5(channel);
		}
		break;
	}
}

void MidiSoundDriverH32::playSample(const byte *data, int size, int channel, int volume) {
	Common::StackLock lock(_mutex);

	stopChannel(channel);

	volume = volume * 8 / 5;

	if (data[0] < 0x80) {
		selectInstrument(channel, data[0] / 0x40, data[0] % 0x40, volume);
	} else {
		writeInstrument(channel * 512 + 0x80000, data + 1, 256);
		selectInstrument(channel, 2, channel, volume);
	}

	_output->send(0x91 + channel, 12, 0x7F);
}

void MidiSoundDriverH32::notifyInstrumentLoad(const byte *data, int size, int channel) {
	Common::StackLock lock(_mutex);

	// In case we specify a standard instrument or standard rhythm instrument
	// do not do anything here. It might be noteworthy that the instrument
	// selection client code does not support rhythm instruments!
	if (data[0] < 0x80 || data[0] >= 0xC0)
		return;

	writeInstrument(channel * 512 + 0x80000, data + 1, size - 1);
}

void MidiSoundDriverH32::writeInstrument(int offset, const byte *data, int size) {
	byte sysEx[254];

	sysEx[0] = 0x41;
	sysEx[1] = 0x10;
	sysEx[2] = 0x16;
	sysEx[3] = 0x12;
	sysEx[4] = (offset >> 16) & 0xFF;
	sysEx[5] = (offset >>  8) & 0xFF;
	sysEx[6] = (offset >>  0) & 0xFF;
	int copySize = MIN(246, size);
	memcpy(&sysEx[7], data, copySize);

	byte checkSum = 0;
	for (int i = 0; i < copySize + 3; ++i)
		checkSum += sysEx[4 + i];
	sysEx[7 + copySize] = 0x80 - (checkSum & 0x7F);

	_output->sysEx(sysEx, copySize + 8);
}

void MidiSoundDriverH32::selectInstrument2(int channel, int timbreGroup, int timbreNumber) {
	const int offset = channel * 16 + 0x30000; // 0x30000 is the start of the patch temp area

	byte sysEx[14] = {
		0x41, 0x10, 0x16, 0x12,
		0x00, 0x00, 0x00,       // offset
		0x00, // Timbre group   _ timbreGroup * 64 + timbreNumber should be the
		0x00, // Timbre number /  MT-32 instrument in case timbreGroup is 0 or 1.
		0x18, // Key shift (= 0)
		0x32, // Fine tune (= 0)
		0x0C, // Bender Range
		0x03, // Assign Mode
		0x00 // Checksum
	};


	sysEx[4] = (offset >> 16) & 0xFF;
	sysEx[5] = (offset >> 8) & 0xFF;
	sysEx[6] = (offset >> 0) & 0xFF;

	sysEx[7] = timbreGroup;
	sysEx[8] = timbreNumber;

	byte checkSum = 0;

	for (int i = 4; i < sizeof(sysEx) - 1; ++i)
		checkSum += sysEx[i];

	sysEx[sizeof(sysEx) - 1] = 0x80 - (checkSum & 0x7F);

	_output->sysEx(sysEx, sizeof(sysEx));
}

void MidiSoundDriverH32::selectInstrument3(int channel, int offsetMode, int timbreGroup) {
	int offset = channel * 16 + 0x30000; // 0x30000 is the start of the patch temp area

	switch (offsetMode) {
	case 1:
		offset += 8;
		break;
	case 2:
		offset += 6;
		break;
	case 3:
		offset += 9;
		break;
	}

	byte sysEx[9] = {
		0x41, 0x10, 0x16, 0x12,
		0x00, 0x00, 0x00,       // offset
		0x00, // Timbre group
		0x00 // Checksum
	};


	sysEx[4] = (offset >> 16) & 0xFF;
	sysEx[5] = (offset >> 8) & 0xFF;
	sysEx[6] = (offset >> 0) & 0xFF;

	sysEx[7] = timbreGroup;

	byte checkSum = 0;

	for (int i = 4; i < sizeof(sysEx) - 1; ++i)
		checkSum += sysEx[i];

	sysEx[sizeof(sysEx) - 1] = 0x80 - (checkSum & 0x7F);

	_output->sysEx(sysEx, sizeof(sysEx));
}

void MidiSoundDriverH32::selectInstrument4(int offsetMode, int timbreGroup, int timbreNumber, int keyShift) {
	int offset = 0x100000;
	const int len = (offsetMode == 2) ? 11 : 9;

	switch (offsetMode) {
	case 1:
		offset += 0x16;
		break;
	case 2:
		offset += 1;
		break;
	}

	byte sysEx[11] = {
		0x41, 0x10, 0x16, 0x12,
		0x00, 0x00, 0x00,       // offset
		0x00, // Timbre group
		0x00, // Timbre number /  MT-32 instrument in case timbreGroup is 0 or 1.
		0x18, // Key shift (= 0)
		0x00 // Checksum
	};


	sysEx[4] = (offset >> 16) & 0xFF;
	sysEx[5] = (offset >> 8) & 0xFF;
	sysEx[6] = (offset >> 0) & 0xFF;

	sysEx[7] = timbreGroup;

	if (offsetMode == 2) {
		sysEx[8] = timbreNumber;
		sysEx[9] = keyShift;
	}

	byte checkSum = 0;

	for (int i = 4; i < len - 1; ++i)
		checkSum += sysEx[i];

	sysEx[len - 1] = 0x80 - (checkSum & 0x7F);

	_output->sysEx(sysEx, len);
}

void MidiSoundDriverH32::selectInstrument5(int messageNum) {
	int offset = 0x200000;

	byte sysEx[28] = {
		0x41, 0x10, 0x16, 0x12,
		0x00, 0x00, 0x00       // offset
	};

	memset(sysEx + 7, 0x20, 20);

	if (messageNum >= 0 && messageNum < g_cine->_messageTable.size()) {
		Common::String msg = g_cine->_messageTable[messageNum];
		memcpy(sysEx + 7, msg.c_str(), MIN<int>(20, msg.size()));
	}

	sysEx[4] = (offset >> 16) & 0xFF;
	sysEx[5] = (offset >> 8) & 0xFF;
	sysEx[6] = (offset >> 0) & 0xFF;

	byte checkSum = 0;

	for (int i = 4; i < sizeof(sysEx) - 1; ++i)
		checkSum += sysEx[i];

	sysEx[sizeof(sysEx) - 1] = 0x80 - (checkSum & 0x7F);

	_output->sysEx(sysEx, sizeof(sysEx));
}

void MidiSoundDriverH32::selectInstrument(int channel, int timbreGroup, int timbreNumber, int volume) {
	const int offset = channel * 16 + 0x30000; // 0x30000 is the start of the patch temp area

	byte sysEx[24] = {
		0x41, 0x10, 0x16, 0x12,
		0x00, 0x00, 0x00,       // offset
		0x00, // Timbre group   _ timbreGroup * 64 + timbreNumber should be the
		0x00, // Timbre number /  MT-32 instrument in case timbreGroup is 0 or 1.
		0x18, // Key shift (= 0)
		0x32, // Fine tune (= 0)
		0x0C, // Bender Range
		0x03, // Assign Mode
		0x01, // Reverb Switch (= enabled)
		0x00, // dummy
		0x00, // Output level
		0x07, // Panpot (= balanced)
		0x00, // dummy
		0x00, // dummy
		0x00, // dummy
		0x00, // dummy
		0x00, // dummy
		0x00, // dummy
		0x00  // checksum
	};


	sysEx[4] = (offset >> 16) & 0xFF;
	sysEx[5] = (offset >>  8) & 0xFF;
	sysEx[6] = (offset >>  0) & 0xFF;

	sysEx[7] = timbreGroup;
	sysEx[8] = timbreNumber;

	sysEx[15] = volume;

	byte checkSum = 0;

	for (int i = 4; i < 23; ++i)
		checkSum += sysEx[i];

	sysEx[23] = 0x80 - (checkSum & 0x7F);

	_output->sysEx(sysEx, 24);
}

PCSoundFxPlayer::PCSoundFxPlayer(PCSoundDriver *driver)
	: _playing(false), _driver(driver), _mutex() {
	memset(_instrumentsData, 0, sizeof(_instrumentsData));
	_sfxData = NULL;
	_fadeOutCounter = 0;
	_driver->setUpdateCallback(updateCallback, this);
}

PCSoundFxPlayer::~PCSoundFxPlayer() {
	Common::StackLock lock(_mutex);

	_driver->setUpdateCallback(NULL, NULL);
	stop();
}

bool PCSoundFxPlayer::load(const char *song) {
	debug(9, "PCSoundFxPlayer::load('%s')", song);

	/* stop (w/ fade out) the previous song */
	while (_fadeOutCounter != 0 && _fadeOutCounter < 100) {
		g_system->delayMillis(50);
	}
	_fadeOutCounter = 0;

	Common::StackLock lock(_mutex);

	stop();

	_sfxData = readBundleSoundFile(song);
	if (!_sfxData) {
		warning("Unable to load soundfx module '%s'", song);
		return 0;
	}

	for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
		_instrumentsData[i] = NULL;
		char instrument[64];
		memset(instrument, 0, 64); // Clear the data first
		memcpy(instrument, _sfxData + 20 + i * 30, 12);
		instrument[63] = '\0';

		if (instrument[0] != '\0') {
			char *dot = strrchr(instrument, '.');
			if (dot) {
				*dot = '\0';
			}
			Common::strlcat(instrument, _driver->getInstrumentExtension(), sizeof(instrument));
			uint32 instrumentSize;
			byte *data = readBundleSoundFile(instrument, &instrumentSize);
			if (g_cine->getGameType() == Cine::GType_OS && _driver->musicType() == MT_MT32 &&
				data && instrumentSize > 0x16) {
				instrumentSize -= 0x16;
				byte *tmp = (byte *)calloc(instrumentSize, 1);
				if (tmp == NULL) {
					error("PCSoundFxPlayer::load('%s'): Out of memory (%d bytes)", song, instrumentSize);
				}
				memcpy(tmp, data + 0x16, instrumentSize);
				free(data);
				data = tmp;
			}
			_instrumentsData[i] = data;
			if (!_instrumentsData[i]) {
				warning("Unable to load soundfx instrument '%s'", instrument);
			} else {
				_driver->notifyInstrumentLoad(_instrumentsData[i], instrumentSize, i);
			}
		}
	}
	return 1;
}

void PCSoundFxPlayer::play() {
	debug(9, "PCSoundFxPlayer::play()");
	Common::StackLock lock(_mutex);
	if (_sfxData) {
		for (int i = 0; i < NUM_CHANNELS; ++i) {
			_instrumentsChannelTable[i] = -1;
		}
		_currentPos = 0;
		_currentOrder = 0;
		_numOrders = _sfxData[470];
		int timerIntsPerMusicUpdate = (g_cine->getGameType() == Cine::GType_OS) ? 1 : 2;
		if (g_cine->getGameType() == Cine::GType_OS) {
			_eventsDelay = (244 - _sfxData[471]) * 109 * timerIntsPerMusicUpdate / 1060;
		} else {
			_eventsDelay = (252 - _sfxData[471]) * 55 * timerIntsPerMusicUpdate / 1060;
		}
		_updateTicksCounter = 0;
		_playing = true;
	}
}

void PCSoundFxPlayer::stop() {
	Common::StackLock lock(_mutex);
	if (_playing || _fadeOutCounter != 0) {
		_fadeOutCounter = 0;
		_playing = false;
		int numChannels = (g_cine->getGameType() == Cine::GType_OS) ? 8 : 4;
		for (int i = 0; i < numChannels; ++i) {
			_driver->stopChannel(i);
		}
		_driver->stopAll();
	}
	unload();
}

void PCSoundFxPlayer::fadeOut() {
	Common::StackLock lock(_mutex);
	if (_playing) {
		_fadeOutCounter = 1;
		_playing = false;
	}
}

void PCSoundFxPlayer::updateCallback(void *ref) {
	((PCSoundFxPlayer *)ref)->update();
}

void PCSoundFxPlayer::update() {
	Common::StackLock lock(_mutex);
	if (_playing || (_fadeOutCounter != 0 && _fadeOutCounter < 100)) {
		++_updateTicksCounter;
		if (_updateTicksCounter > _eventsDelay) {
			handleEvents();
			_updateTicksCounter = 0;
		}
	}
}

void PCSoundFxPlayer::handleEvents() {
	const byte *patternData = _sfxData + ((g_cine->getGameType() == Cine::GType_OS) ? 2400 : 600);
	const byte *orderTable = _sfxData + 472;
	uint16 patternNum = orderTable[_currentOrder] * 1024;

	for (int i = 0; i < 4; ++i) {
		handlePattern(i, patternData + patternNum + _currentPos);
		patternData += 4;
	}

	if (_fadeOutCounter != 0 && _fadeOutCounter < 100) {
		_fadeOutCounter += 2;
	}
	_currentPos += 16;
	if (_currentPos >= 1024) {
		_currentPos = 0;
		++_currentOrder;
		if (_currentOrder == _numOrders) {
			_currentOrder = 0;
		}
	}
	debug(7, "_currentOrder=%d/%d _currentPos=%d", _currentOrder, _numOrders, _currentPos);
}

void PCSoundFxPlayer::handlePattern(int channel, const byte *patternData) {
	int instrument = patternData[2] >> 4;
	if (instrument != 0) {
		--instrument;
		if (_instrumentsChannelTable[channel] != instrument || _fadeOutCounter != 0) {
			_instrumentsChannelTable[channel] = instrument;
			const int volume = _sfxData[instrument] - _fadeOutCounter;
			_driver->setupChannel(channel, _instrumentsData[instrument], instrument, volume);
		}
	}
	int16 freq = (int16)READ_BE_UINT16(patternData);
	if (freq > 0) {
		_driver->stopChannel(channel);
		_driver->setChannelFrequency(channel, freq);
	}
}

void PCSoundFxPlayer::unload() {
	for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
		free(_instrumentsData[i]);
		_instrumentsData[i] = NULL;
	}
	free(_sfxData);
	_sfxData = NULL;
}

MusicType Sound::musicType() {
	return _musicType;
}

PCSound::PCSound(Audio::Mixer *mixer, CineEngine *vm)
	: Sound(mixer, vm), _soundDriver(0) {

	_currentMusic = 0;
	_currentMusicStatus = 0;
	_currentBgSlot = 0;
	_musicType = MT_INVALID;

	const MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB);
	const MusicType musicType = MidiDriver::getMusicType(dev);
	if (musicType == MT_MT32 || musicType == MT_GM) {
		const bool isMT32 = (musicType == MT_MT32 || ConfMan.getBool("native_mt32"));
		if (isMT32) {
			MidiDriver *driver = MidiDriver::createMidi(dev);
			if (driver && driver->open() == 0) {
				driver->sendMT32Reset();
				_soundDriver = new MidiSoundDriverH32(driver);
				_musicType = MT_MT32;
			} else {
				warning("Could not create MIDI output, falling back to AdLib");
			}
		} else {
			warning("General MIDI output devices are not supported, falling back to AdLib");
		}
	}

	if (!_soundDriver) {
		if (_vm->getGameType() == GType_FW) {
			_soundDriver = new AdLibSoundDriverINS(_mixer);
		} else {
			_soundDriver = new AdLibSoundDriverADL(_mixer);
		}
		_musicType = MT_ADLIB;
	}

	_player = new PCSoundFxPlayer(_soundDriver);

	// Ensure the CD is open
	if (_vm->getGameType() == GType_FW && (_vm->getFeatures() & GF_CD))
		g_system->getAudioCDManager()->open();
}

PCSound::~PCSound() {
	delete _player;
	delete _soundDriver;
}

static const char *const musicFileNames[11] = {
	"DUGGER.DAT",
	"SUITE21.DAT",
	"FWARS.DAT",
	"SUITE23.DAT",
	"SUITE22.DAT",
	"ESCAL",
	"MOINES.DAT",
	"MEDIAVAL.DAT",
	"SFUTUR",
	"ALIENS",
	"TELESONG.DAT",
};

static uint8 musicCDTracks[11] = {
	20, 21, 22, 23, 24, 25, 26, 27, 28, 30, 22,
};

void PCSound::loadMusic(const char *name) {
	debugC(5, kCineDebugSound, "PCSound::loadMusic('%s')", name);

	if (_vm->getGameType() == GType_FW && (_vm->getFeatures() & GF_CD)) {
		_currentMusic = 0;
		_currentMusicStatus = 0;
		for (int i = 0; i < 11; i++) {
			if (!strcmp((const char *)name, musicFileNames[i])) {
				_currentMusic = musicCDTracks[i];
				_currentMusicStatus = musicCDTracks[i];
			}
		}
	} else {
		_player->load(name);
	}
}

void PCSound::playMusic() {
	debugC(5, kCineDebugSound, "PCSound::playMusic()");
	if (_vm->getGameType() == GType_FW && (_vm->getFeatures() & GF_CD)) {
		g_system->getAudioCDManager()->stop();
		g_system->getAudioCDManager()->play(_currentMusic - 1, -1, 0, 0);
	} else {
		_player->play();
	}
}

static uint8 bgCDTracks[49] = {
	0, 21, 21, 23, 0, 29, 0, 0, 0, 0,
	0, 27,  0,  0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 22, 22, 23, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0
};

void PCSound::setBgMusic(int num) {
	debugC(5, kCineDebugSound, "PCSound::setBgMusic(%d)", num);
	_currentBgSlot = num;
	if (!bgCDTracks[_currentBgSlot])
		return;

	if ((_currentBgSlot == 1) || (_currentMusicStatus == 0 && _currentMusic != bgCDTracks[_currentBgSlot])) {
		_currentMusic = bgCDTracks[_currentBgSlot];
		g_system->getAudioCDManager()->stop();
		g_system->getAudioCDManager()->play(bgCDTracks[_currentBgSlot] - 1, -1, 0, 0);
	}
}

void PCSound::stopMusic() {
	debugC(5, kCineDebugSound, "PCSound::stopMusic()");

	if (_vm->getGameType() == GType_FW && (_vm->getFeatures() & GF_CD)) {
		if (_currentBgSlot != 1)
			g_system->getAudioCDManager()->stop();
	}
	_player->stop();
}

void PCSound::fadeOutMusic() {
	debugC(5, kCineDebugSound, "PCSound::fadeOutMusic()");

	if (_vm->getGameType() == GType_FW && (_vm->getFeatures() & GF_CD)) {
		if (_currentMusicStatus) {
			if (_currentBgSlot == 1) {
				_currentMusicStatus = 0;
			} else {
				_currentMusic = 0;
				_currentMusicStatus = 0;
				g_system->getAudioCDManager()->stop();
				if (bgCDTracks[_currentBgSlot]) {
					g_system->getAudioCDManager()->play(_currentBgSlot - 1, -1, 0, 0);
				}
			}
		}
	}
	_player->fadeOut();
}

void PCSound::playSound(int mode, int channel, int param3, int param4, int param5, int size) {
	debugC(5, kCineDebugSound, "PCSound::playSound() channel %d size %d, new", channel, size);
	_soundDriver->playSample(mode, channel, param3, param4, param5, size);
}

void PCSound::playSound(int channel, int frequency, const uint8 *data, int size, int volumeStep, int stepCount, int volume, int repeat) {
	debugC(5, kCineDebugSound, "PCSound::playSound() channel %d size %d", channel, size);
	_soundDriver->playSample(data, size, channel, volume);
}

void PCSound::stopSound(int channel) {
	debugC(5, kCineDebugSound, "PCSound::stopSound() channel %d", channel);
	_soundDriver->resetChannel(channel);
}

PaulaSound::PaulaSound(Audio::Mixer *mixer, CineEngine *vm)
	: Sound(mixer, vm), _sfxTimer(0), _musicTimer(0), _musicFadeTimer(0) {
	_musicType = MT_AMIGA;
	_moduleStream = 0;
	// The original is using the following timer frequency:
	// 0.709379Mhz / 8000 = 88.672375Hz
	// 1000000 / 88.672375Hz = 11277.46944863us
	g_system->getTimerManager()->installTimerProc(&PaulaSound::sfxTimerProc, 11277, this, "PaulaSound::sfxTimerProc");
	// The original is using the following timer frequency:
	// 0.709379Mhz / 14565 = 48.704359Hz
	// 1000000 / 48.704359Hz = 20532.04313806us
	g_system->getTimerManager()->installTimerProc(&PaulaSound::musicTimerProc, 20532, this, "PaulaSound::musicTimerProc");
}

PaulaSound::~PaulaSound() {
	Common::StackLock sfxLock(_sfxMutex);
	g_system->getTimerManager()->removeTimerProc(&PaulaSound::sfxTimerProc);
	for (int i = 0; i < NUM_CHANNELS; ++i) {
		stopSound(i);
	}

	Common::StackLock musicLock(_musicMutex);
	g_system->getTimerManager()->removeTimerProc(&PaulaSound::musicTimerProc);
	stopMusic();
}

void PaulaSound::loadMusic(const char *name) {
	debugC(5, kCineDebugSound, "PaulaSound::loadMusic('%s')", name);
	for (int i = 0; i < NUM_CHANNELS; ++i) {
		stopSound(i);
	}

	// Fade music out when there is music playing.
	_musicMutex.lock();
	if (_mixer->isSoundHandleActive(_moduleHandle)) {
		// Only start fade out when it is not in progress.
		if (!_musicFadeTimer) {
			_musicFadeTimer = 1;
		}

		_musicMutex.unlock();
		while (_musicFadeTimer != 64) {
			g_system->delayMillis(50);
		}
	} else {
		_musicMutex.unlock();
	}

	Common::StackLock lock(_musicMutex);
	assert(!_mixer->isSoundHandleActive(_moduleHandle));

	bool foundFile = false;
	if (_vm->getGameType() == GType_FW) {
		// look for separate files
		Common::File f;
		if (f.open(name)) {
			_moduleStream = Audio::makeSoundFxStream(&f, 0, _mixer->getOutputRate());
			foundFile = true;
		}
	} else {
		// look in bundle files
		uint32 size;
		byte *buf = readBundleSoundFile(name, &size);
		if (buf) {
			Common::MemoryReadStream s(buf, size);
			// Operation Stealth for Amiga has to have its music frequency halved
			// or otherwise the music sounds too high pitched.
			const int periodScaleDivisor = 2;
			_moduleStream = Audio::makeSoundFxStream(&s, readBundleSoundFile, _mixer->getOutputRate(), true, true, periodScaleDivisor);
			free(buf);
			foundFile = true;
		}
	}

	if (!foundFile) {
		warning("Unable to find music file '%s', not playing music...", name);

		// Remove the old module stream so that it won't be played.
		// Fixes not trying to play a null stream or an old wrong music in
		// e.g. Italian version of Future Wars when first teleporting from
		// the office to to the swamp.
		_moduleStream = nullptr;
	}
}

void PaulaSound::playMusic() {
	debugC(5, kCineDebugSound, "PaulaSound::playMusic()");
	Common::StackLock lock(_musicMutex);

	_mixer->stopHandle(_moduleHandle);
	if (_moduleStream) {
		_musicFadeTimer = 0;
		_mixer->playStream(Audio::Mixer::kMusicSoundType, &_moduleHandle, _moduleStream);
	}
}

void PaulaSound::stopMusic() {
	debugC(5, kCineDebugSound, "PaulaSound::stopMusic()");
	Common::StackLock lock(_musicMutex);

	_mixer->stopHandle(_moduleHandle);
}

void PaulaSound::setBgMusic(int num) {
}

void PaulaSound::fadeOutMusic() {
	debugC(5, kCineDebugSound, "PaulaSound::fadeOutMusic()");
	Common::StackLock lock(_musicMutex);

	_musicFadeTimer = 1;
}

void PaulaSound::playSound(int mode, int channel, int param3, int param4, int param5, int size) {
	// Newer playSound from Operation Stealth might not do anything on Amiga
}

void PaulaSound::playSound(int channel, int frequency, const uint8 *data, int size, int volumeStep, int stepCount, int volume, int repeat) {
	debugC(5, kCineDebugSound, "PaulaSound::playSound() channel %d size %d", channel, size);
	Common::StackLock lock(_sfxMutex);

	if (channel < 0 || channel >= NUM_CHANNELS) {
		warning("PaulaSound::playSound: Channel number out of range (%d)", channel);
		return;
	}

	stopSound(channel);
	if (frequency > 0 && size > 0) {
		byte *sound = (byte *)malloc(size);
		if (sound) {
			// Create the audio stream
			memcpy(sound, data, size);

			// Clear the first and last 16 bits like in the original.
			sound[0] = sound[1] = sound[size - 2] = sound[size - 1] = 0;

			if (g_cine->getGameType() == Cine::GType_OS) {
				frequency = ((frequency * 2) / 20) + 50;
			}

			Audio::SeekableAudioStream *stream = Audio::makeRawStream(sound, size, PAULA_FREQ / frequency, 0);

			// Initialize the volume control
			_channelsTable[channel].initialize(volume, volumeStep, stepCount);

			// Start the sfx
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &_channelsTable[channel].handle,
			                   Audio::makeLoopingAudioStream(stream, repeat ? 0 : 1),
			                   -1, volume * Audio::Mixer::kMaxChannelVolume / 63,
			                   _channelBalance[channel]);
		}
	}
}

void PaulaSound::stopSound(int channel) {
	debugC(5, kCineDebugSound, "PaulaSound::stopSound() channel %d", channel);
	Common::StackLock lock(_sfxMutex);

	_mixer->stopHandle(_channelsTable[channel].handle);
}

void PaulaSound::sfxTimerProc(void *param) {
	PaulaSound *sound = (PaulaSound *)param;
	sound->sfxTimerCallback();
}

void PaulaSound::sfxTimerCallback() {
	Common::StackLock lock(_sfxMutex);

	if (_sfxTimer < 6) {
		++_sfxTimer;

		for (int i = 0; i < NUM_CHANNELS; ++i) {
			// Only process active channels
			if (!_mixer->isSoundHandleActive(_channelsTable[i].handle)) {
				continue;
			}

			if (_channelsTable[i].curStep) {
				--_channelsTable[i].curStep;
			} else {
				_channelsTable[i].curStep = _channelsTable[i].stepCount;
				const int volume = CLIP(_channelsTable[i].volume + _channelsTable[i].volumeStep, 0, 63);
				_channelsTable[i].volume = volume;
				// Unlike the original we stop silent sounds
				if (volume) {
					_mixer->setChannelVolume(_channelsTable[i].handle, volume * Audio::Mixer::kMaxChannelVolume / 63);
				} else {
					_mixer->stopHandle(_channelsTable[i].handle);
				}
			}
		}
	} else {
		_sfxTimer = 0;
		// Possible TODO: The original only ever started sounds here. This
		// should not be noticable though. So we do not do it for now.
	}
}

void PaulaSound::musicTimerProc(void *param) {
	PaulaSound *sound = (PaulaSound *)param;
	sound->musicTimerCallback();
}

void PaulaSound::musicTimerCallback() {
	Common::StackLock lock(_musicMutex);

	++_musicTimer;
	if (_musicTimer == 6) {
		_musicTimer = 0;
		if (_musicFadeTimer) {
			++_musicFadeTimer;
			if (_musicFadeTimer == 64) {
				stopMusic();
			} else {
				if (_mixer->isSoundHandleActive(_moduleHandle)) {
					_mixer->setChannelVolume(_moduleHandle, (64 - _musicFadeTimer) * Audio::Mixer::kMaxChannelVolume / 64);
				}
			}
		}
	}
}

const int PaulaSound::_channelBalance[NUM_CHANNELS] = {
	// L/R/R/L This is according to the Hardware Reference Manual.
	// TODO: It seems the order is swapped for some Amiga models:
	// http://www.amiga.org/forums/archive/index.php/t-7862.html
	// Maybe we should consider using R/L/L/R to match Amiga 500?
	// This also is a bit more drastic to what WineUAE defaults,
	// which is only 70% of full panning.
	-127, 127, 127, -127
};

} // End of namespace Cine
