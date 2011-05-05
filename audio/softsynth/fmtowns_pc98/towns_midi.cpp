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
 * $URL:  $
 * $Id:  $
 */

#include "audio/softsynth/fmtowns_pc98/towns_midi.h"
#include "common/textconsole.h"

class TownsMidiOutputChannel {
friend class TownsMidiInputChannel;
public:
	TownsMidiOutputChannel(MidiDriver_TOWNS *driver, int chanId);
	~TownsMidiOutputChannel();

	void noteOn(uint8 msb, uint16 lsb);
	void noteOnPitchBend(uint8 msb, uint16 lsb);
	void setupProgram(const uint8 *data, uint8 vol1, uint8 vol2);
	void setupEffects(int index, uint8 c, const uint8 *effectData);
	void setModWheel(uint8 value);
	
	void connect(TownsMidiInputChannel *chan);
	void disconnect();

	bool update();

	enum CheckPriorityStatus {
		kDisconnected = -3,
		kHighPriority = -2
	};

	int checkPriority(int pri);

private:
	struct StateA {
		uint8 numLoop;
		uint32 fld_1;
		int32 duration;
		int32 fld_9;
		int16 effectState;
		uint8 fld_11;
		uint8 ar1[4];
		uint8 ar2[4];
		int8 modWheelSensitivity;
		int8 modWheelState;
		uint8 fld_1c;
		uint32 fld_1d;
		uint32 fld_21;
		uint32 fld_25;
		int8 dir;
		uint32 fld_2a;
		uint32 fld_2e;
	} *_stateA;

	struct StateB {
		int16 inc;
		uint8 type;
		uint8 useModWheel;
		uint8 fld_6;
		StateA *a;
	} *_stateB;

	int16 getEffectState(uint8 type);
	void initEffect(StateA *a, const uint8 *effectData);
	void updateEffectOuter3(StateA *a, StateB *b);
	int updateEffectOuter(StateA *a, StateB *b);
	void updateEffect(StateA *a);	
	int lookupVolume(int a, int b);

	void keyOn();
	void keyOff();
	void keyOnSetFreq(uint16 frq);
	void out(uint8 reg, uint8 val);

	TownsMidiInputChannel *_midi;
	TownsMidiOutputChannel *_prev;
	TownsMidiOutputChannel *_next;
	uint8 _fld_c;
	uint8 _chan;
	uint8 _note;
	uint8 _carrierTl;
	uint8 _modulatorTl;
	uint8 _sustainNoteOff;
	int32 _duration;
	
	uint16 _freq;
	int16 _freqAdjust;

	MidiDriver_TOWNS *_driver;

	static const uint8 _chanMap[];
	static const uint8 _chanMap2[];
	static const uint8 _effectDefs[];
	static const uint16 _effectData[];
	static const uint8 _freqMSB[];
	static const uint16 _freqLSB[];
};

class TownsMidiInputChannel : public MidiChannel {
friend class TownsMidiOutputChannel;
public:
	TownsMidiInputChannel(MidiDriver_TOWNS *driver, int chanIndex);
	~TownsMidiInputChannel();

	MidiDriver *device() { return _driver; }
	byte getNumber() { return _chanIndex; }
	bool allocate();
	void release();

	void send(uint32 b);

	void noteOff(byte note);
	void noteOn(byte note, byte velocity);
	void programChange(byte program);
	void pitchBend(int16 bend);
	void controlChange(byte control, byte value);
	void pitchBendFactor(byte value);
	void priority(byte value);
	void sysEx_customInstrument(uint32 type, const byte *instr);	

private:
	void controlModulationWheel(byte value);
	void controlVolume(byte value);
	void controlPanPos(byte value);
	void controlSustain(byte value);

	void releasePedal();

	TownsMidiOutputChannel *_outChan;
	
	uint8 *_instrument;
	uint8 _prg;
	uint8 _chanIndex;
	uint8 _effectLevel;
	uint8 _priority;
	uint8 _ctrlVolume;
	uint8 _tl;
	uint8 _pan;
	uint8 _panEff;
	uint8 _percS;
	int8 _transpose;
	uint8 _fld_1f;
	int8 _detune;
	int8 _modWheel;
	uint8 _sustain;
	uint8 _pitchBendFactor;
	int16 _pitchBend;
	uint16 _freqLSB;

	bool _allocated;

	MidiDriver_TOWNS *_driver;

	static const uint8 _programAdjustLevel[];
};

class TownsMidiChanState {
public:
	TownsMidiChanState();
	~TownsMidiChanState() {}	
	uint8 get(uint8 type);

	uint8 unk1;
	uint8 mulAmsFms;
	uint8 tl;
	uint8 attDec;
	uint8 sus;
	uint8 fgAlg;
	uint8 unk2;
};

TownsMidiChanState::TownsMidiChanState() {
	unk1 = mulAmsFms = tl =	attDec = sus = fgAlg = unk2 = 0;
}

uint8 TownsMidiChanState::get(uint8 type) {
	switch (type) {
	case 0:
		return unk1;
	case 1:
		return mulAmsFms;
	case 2:
		return tl;
	case 3:
		return attDec;
	case 4:
		return sus;
	case 5:
		return fgAlg;
	case 6:
		return unk2;
	default:
		break;
	}
	return 0;
}

TownsMidiOutputChannel::TownsMidiOutputChannel(MidiDriver_TOWNS *driver, int chanIndex) : _driver(driver), _chan(chanIndex),
	_midi(0), _prev(0), _next(0), _fld_c(0), _carrierTl(0), _note(0), _modulatorTl(0), _sustainNoteOff(0), _duration(0), _freq(0), _freqAdjust(0) {
	_stateA = new StateA[2];
	memset(_stateA, 0, 2 * sizeof(StateA));
	_stateB = new StateB[2];
	memset(_stateB, 0, 2 * sizeof(StateB));
}

TownsMidiOutputChannel::~TownsMidiOutputChannel() {
	delete[] _stateA;
	delete[] _stateB;
}

void TownsMidiOutputChannel::noteOn(uint8 msb, uint16 lsb) {
	_freq = (msb << 7) + lsb;
	_freqAdjust = 0;
	keyOnSetFreq(_freq);
}

void TownsMidiOutputChannel::noteOnPitchBend(uint8 msb, uint16 lsb) {
	_freq = (msb << 7) + lsb;
	keyOnSetFreq(_freq + _freqAdjust);
}

void TownsMidiOutputChannel::setupProgram(const uint8 *data, uint8 vol1, uint8 vol2) {
	// This driver uses only 2 operators and 2 algorithms (algorithm 5 and 7),
	// since it is just a modified AdLib driver. It also uses AdLib programs.
	// There are no FM-TOWNS specific programs. This is the reason for the FM-TOWNS
	// music being so bad compared to AdLib (unsuitable data is just forced into the
	// wrong audio device).

	static const uint8 mul[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 12, 12, 15, 15 };
	uint8 chan = _chanMap[_chan];	

	uint8 mulAmsFms1 = _driver->_chanState[chan].mulAmsFms = data[0];
	uint8 tl1 = _driver->_chanState[chan].tl = (data[1] | 0x3f) - vol1;
	uint8 attDec1 = _driver->_chanState[chan].attDec = ~data[2];
	uint8 sus1 = _driver->_chanState[chan].sus = ~data[3];
	uint8 unk1 = _driver->_chanState[chan].unk2 = data[4];
	chan += 3;

	out(0x30, mul[mulAmsFms1 & 0x0f]);
	out(0x40, (tl1 & 0x3f) + 15);
	out(0x50, ((attDec1 >> 4) << 1) | ((attDec1 >> 4) & 1));
	out(0x60, ((attDec1 << 1) | (attDec1 & 1)) & 0x1f);
	out(0x70, (mulAmsFms1 & 0x20) ^ 0x20 ? ((sus1 & 0x0f) << 1) | 1: 0);
	out(0x80, sus1);

	uint8 mulAmsFms2 = _driver->_chanState[chan].mulAmsFms = data[5];
	uint8 tl2 = _driver->_chanState[chan].tl = (data[6] | 0x3f) - vol2;
	uint8 attDec2 = _driver->_chanState[chan].attDec = ~data[7];
	uint8 sus2 = _driver->_chanState[chan].sus = ~data[8];
	uint8 unk2 = _driver->_chanState[chan].unk2 = data[9];

	uint8 mul2 = mul[mulAmsFms2 & 0x0f];
	tl2 = (tl2 & 0x3f) + 15;
	uint8 ar2 = ((attDec2 >> 4) << 1) | ((attDec2 >> 4) & 1);
	uint8 dec2 = ((attDec2 << 1) | (attDec2 & 1)) & 0x1f;
	uint8 sus2r = (mulAmsFms2 & 0x20) ^ 0x20 ? ((sus2 & 0x0f) << 1) | 1: 0;

	for (int i = 4; i < 16; i += 4) {
		out(0x30 + i, mul2);
		out(0x40 + i, tl2);
		out(0x50 + i, ar2);
		out(0x60 + i, dec2);
		out(0x70 + i, sus2r);
		out(0x80 + i, sus2);
	}

	_driver->_chanState[chan].fgAlg = data[10];
	uint8 alg = 5 + 2 * (data[10] & 1);
	uint8 fb = 4 * (data[10] & 0x0e);
	out(0xb0, fb | alg);
	uint8 t = mulAmsFms1 | mulAmsFms2;
	out(0xb4, 0xc0 | ((t & 0x80) >> 3) | ((t & 0x40) >> 5));
}

void TownsMidiOutputChannel::setupEffects(int index, uint8 c, const uint8 *effectData) {
	uint16 maxVal[] = { 0x2FF, 0x1F, 0x07, 0x3F, 0x0F, 0x0F, 0x0F, 0x03, 0x3F, 0x0F, 0x0F, 0x0F, 0x03, 0x3E, 0x1F };
	uint8 effectType[] = { 0x1D, 0x1C, 0x1B, 0x00, 0x03, 0x04, 0x07, 0x08, 0x0D, 0x10, 0x11, 0x14, 0x15, 0x1e, 0x1f, 0x00 };
	
	StateA *a = &_stateA[index];
	StateB *b = &_stateB[index];

	b->inc = 0;
	b->useModWheel = c & 0x40;
	a->fld_11 = c & 0x20;
	b->fld_6 = c & 0x10;
	b->type = effectType[c & 0x0f];
	a->fld_9 = maxVal[c & 0x0f];
	a->modWheelSensitivity = 31;
	a->modWheelState = b->useModWheel ? _midi->_modWheel >> 2 : 31;

	switch (b->type) {
	case 0:
		a->effectState = _carrierTl;
		break;
	case 13:
		a->effectState = _modulatorTl;
		break;
	case 30:
		a->effectState = 31;
		b->a->modWheelState = 0;
		break;
	case 31:
		a->effectState = 0;
		b->a->modWheelSensitivity = 0;
		break;
	default:
		a->effectState = getEffectState(b->type);
		break;
	}

	initEffect(a, effectData);
}

void TownsMidiOutputChannel::setModWheel(uint8 value) {
	if (_stateA[0].numLoop && _stateB[0].type)
		_stateA[0].modWheelState = value >> 2;

	if (_stateA[1].numLoop && _stateB[1].type)
		_stateA[1].modWheelState = value >> 2;
}

void TownsMidiOutputChannel::connect(TownsMidiInputChannel *chan) {
	if (!chan)
		return;
	_midi = chan;
	_next = chan->_outChan;
	_prev = 0;
	chan->_outChan = this;
	if (_next)
		_next->_prev = this;
}

void TownsMidiOutputChannel::disconnect() {
	keyOff();
	TownsMidiOutputChannel *p = _prev;
	TownsMidiOutputChannel *n = _next;

	if (n)
		n->_prev = p;
	if (p)
		p->_next = n;
	else
		_midi->_outChan = n;
	_midi = 0;
}

bool TownsMidiOutputChannel::update() {
	if (!_midi)
		return false;

	if (_duration) {
		_duration -= 17;
		if (_duration <=0) {
			disconnect();
			return true;
		}
	}

	for (int i = 0; i < 2; i++) {
		if (_stateA[i].numLoop)
			updateEffectOuter3(&_stateA[i], &_stateB[i]);
	}

	return false;
}

int TownsMidiOutputChannel::checkPriority(int pri) {
	if (!_midi)
		return kDisconnected;

	if (!_next && pri >= _midi->_priority)
		return _midi->_priority;

	return kHighPriority;
}

int16 TownsMidiOutputChannel::getEffectState(uint8 type) {
	uint8 chan = (type < 13) ? _chanMap2[_chan] : ((type < 26) ? _chanMap[_chan] : _chan);
	
	if (type == 28)
		return 15;
	else if (type == 29)
		return 383;
	else if (type > 29)
		return 0;
	else if (type > 12)
		type -= 13;

	int32 res = 0;
	uint8 cs = (_driver->_chanState[chan].get(_effectDefs[type * 4] >> 5) & _effectDefs[type * 4 + 2]) >> _effectDefs[type * 4 + 1];
	if (_effectDefs[type * 4 + 3])
		res = _effectDefs[type * 4 + 3] - cs;
	
	return res;	
}

void TownsMidiOutputChannel::initEffect(StateA *a, const uint8 *effectData) {
	a->numLoop = 1;
	a->fld_1 = 0;
	a->fld_1c = 31;
	a->duration = effectData[0] * 63;
	a->ar1[0] = effectData[1];
	a->ar1[1] = effectData[3];
	a->ar1[2] = effectData[5];
	a->ar1[3] = effectData[6];
	a->ar2[0] = effectData[2];
	a->ar2[1] = effectData[4];
	a->ar2[2] = 0;
	a->ar2[3] = effectData[7];
	updateEffect(a);
}

void TownsMidiOutputChannel::updateEffectOuter3(StateA *a, StateB *b) {
	uint8 f = updateEffectOuter(a, b);

	if (f & 1) {
		switch (b->type) {
		case 0:
			_carrierTl = a->effectState + b->inc; /*???*/
			break;
		case 13:
			_modulatorTl = a->effectState + b->inc; /*???*/
			break;
		case 30:
			b->a->modWheelState = b->inc;
			break;
		case 31:
			b->a->modWheelSensitivity = b->inc;
			break;
		default:
			break;
		}
	}

	if (f & 2) {
		if (b->fld_6)
			keyOn();
	}
}

int TownsMidiOutputChannel::updateEffectOuter(StateA *a, StateB *b) {
	if (a->duration) {
		a->duration -= 17;
		if (a->duration <= 0) {
			a->numLoop = 0;
			return 0;
		}
	} 

	int32 t = a->fld_1 + a->fld_25;
	
	a->fld_2e += a->fld_2a;
	if (a->fld_2e >= a->fld_1d) {
		a->fld_2e -= a->fld_1d;
		t += a->dir;
	}

	int retFlags = 0;

	if (t != a->fld_1 || a->modWheelState != a->fld_1c) {
		a->fld_1 = t;
		a->fld_1c = a->modWheelState;
		t = lookupVolume(t, a->modWheelState);
		if (t != b->inc)
			b->inc = t;
		retFlags |= 1;
	}

	if (--a->fld_21 != 0)
		return retFlags;

	if (++a->numLoop > 4) {
		if (a->fld_11 == 0) {
			a->numLoop = 0;
			return retFlags;
		}
		a->numLoop = 1;
		retFlags |= 2;
	}

	updateEffect(a);

	return retFlags;
}

void TownsMidiOutputChannel::updateEffect(StateA *a) {
	uint8 c = a->numLoop - 1;
	uint16 v = a->ar1[c];
	int32 e = _effectData[_driver->_chanOutputLevel[((v & 0x7f) << 5) + a->modWheelSensitivity]];

	if (v & 0x80)
		e = _driver->randomValue(e);
	
	if (!e)
		e = 1;

	a->fld_1d = a->fld_21 = e;
	int32 d = 0;

	if (c + 1 != 3) {
		v = a->ar2[c];
		e = lookupVolume(a->fld_9, (v & 0x7f) - 31);

		if (v & 0x80)
			e = _driver->randomValue(e);

		if (e + a->effectState > a->fld_9) {
			e = a->fld_9 - a->effectState;
		} else {
			if (e + a->effectState + 1 <= 0)
				e = -a->effectState;
		}

		d = e - a->fld_1;
	}

	a->fld_25 = d / a->fld_1d;
	a->dir = d < 0 ? -1 : 1;
	a->fld_2a = d % a->fld_1d;
	a->fld_2e = 0;
}

int TownsMidiOutputChannel::lookupVolume(int a, int b) {
	if (b == 0)
		return 0;

	if (b == 31)
		return a;

	if (a > 63 || a < -63)
		return ((a + 1) * b) >> 5;

	if (b < 0) {
		if (a < 0)			
			return _driver->_chanOutputLevel[(-a << 5) - b];
		else
			return -_driver->_chanOutputLevel[(a << 5) - b];
	} else {
		if (a < 0)			
			return -_driver->_chanOutputLevel[(-a << 5) + b];
		else
			return _driver->_chanOutputLevel[(-a << 5) + b];
	}
}

void TownsMidiOutputChannel::keyOn() {
	// This driver uses only 2 operators and 2 algorithms (algorithm 5 and 7),
	// since it is just a modified AdLib driver. It also uses AdLib programs.
	// There are no FM-TOWNS specific programs. This is the reason for the FM-TOWNS
	// music being so bad compared to AdLib (unsuitable data is just forced into the
	// wrong audio device).
	out(0x28, 0x30);
}

void TownsMidiOutputChannel::keyOff() {
	out(0x28, 0);
}

void TownsMidiOutputChannel::keyOnSetFreq(uint16 frq) {
	uint8 t = (frq << 1) >> 8;	
	frq = (_freqMSB[t] << 11) | _freqLSB[t] ;
	out(0xa4, frq >> 8);
	out(0xa0, frq & 0xff);
	out(0x28, 0);
	// This driver uses only 2 operators and 2 algorithms (algorithm 5 and 7),
	// since it is just a modified AdLib driver. It also uses AdLib programs.
	// There are no FM-TOWNS specific programs. This is the reason for the FM-TOWNS
	// music being so bad compared to AdLib (unsuitable data is just forced into the
	// wrong audio device).
	out(0x28, 0x30);
}

void TownsMidiOutputChannel::out(uint8 reg, uint8 val) {
	static const uint8 chanRegOffs[] = { 0, 1, 2, 0, 1, 2 };
	static const uint8 keyValOffs[] = { 0, 1, 2, 4, 5, 6 };

	if (reg == 0x28)
		val = (val & 0xf0) | keyValOffs[_chan];
	if (reg < 0x30)
		_driver->_intf->callback(17, 0, reg, val);
	else
		_driver->_intf->callback(17, _chan / 3, (reg & ~3) | chanRegOffs[_chan], val);
}

const uint8 TownsMidiOutputChannel::_chanMap[] = {
	0, 1, 2, 8, 9, 10
};

const uint8 TownsMidiOutputChannel::_chanMap2[] = {
	3, 4, 5, 11, 12, 13
};

const uint8 TownsMidiOutputChannel::_effectDefs[] = {
	0x40, 0x00, 0x3F, 0x3F, 0xE0, 0x02, 0x00, 0x00, 0x40, 0x06, 0xC0, 0x00,
	0x20, 0x00, 0x0F, 0x00, 0x60, 0x04, 0xF0, 0x0F, 0x60, 0x00, 0x0F, 0x0F,
	0x80, 0x04, 0xF0, 0x0F, 0x80, 0x00, 0x0F, 0x0F, 0xE0, 0x00, 0x03, 0x00,
	0x20, 0x07, 0x80, 0x00, 0x20, 0x06, 0x40, 0x00, 0x20, 0x05, 0x20, 0x00,
	0x20, 0x04, 0x10, 0x00, 0xC0, 0x00, 0x01, 0x00, 0xC0, 0x01, 0x0E, 0x00
};

const uint16 TownsMidiOutputChannel::_effectData[] = {
	0x0001, 0x0002, 0x0004, 0x0005, 0x0006, 0x0007,	0x0008, 0x0009,
	0x000A, 0x000C, 0x000E, 0x0010,	0x0012, 0x0015, 0x0018, 0x001E,
	0x0024, 0x0032,	0x0040, 0x0052, 0x0064, 0x0088, 0x00A0, 0x00C0,
	0x00F0, 0x0114, 0x0154, 0x01CC, 0x0258, 0x035C,	0x04B0, 0x0640
};

const uint8 TownsMidiOutputChannel::_freqMSB[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x80, 0x81, 0x83, 0x85,
	0x87, 0x88, 0x8A, 0x8C, 0x8E, 0x8F, 0x91, 0x93, 0x95, 0x96, 0x98, 0x9A,
	0x9C, 0x9E, 0x9F, 0xA1, 0xA3, 0xA5, 0xA6, 0xA8, 0xAA, 0xAC, 0xAD, 0xAF,
	0xB1, 0xB3, 0xB4, 0xB6, 0xB8, 0xBA, 0xBC, 0xBD, 0xBF, 0xC1, 0xC3, 0xC4,
	0xC6, 0xC8, 0xCA, 0xCB, 0xCD, 0xCF, 0xD1, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA,
	0xDB, 0xDD, 0xDF, 0xE1, 0xE2, 0xE4, 0xE6, 0xE8, 0xE9, 0xEB, 0xED, 0xEF
};

const uint16 TownsMidiOutputChannel::_freqLSB[] = {
	0x02D6, 0x02D6, 0x02D6, 0x02D6, 0x02D6, 0x02D6, 0x02D6, 0x02D6,
	0x02D6, 0x02D6, 0x02D6, 0x02D6, 0x02D6, 0x02D6, 0x0301, 0x032F,
	0x0360, 0x0393, 0x03C9, 0x0403, 0x0440, 0x0481, 0x04C6, 0x050E,
	0x055B, 0x02D6, 0x0301, 0x032F, 0x0360, 0x0393, 0x03C9, 0x0403,
	0x0440, 0x0481, 0x04C6, 0x050E, 0x055B, 0x02D6, 0x0301, 0x032F,
	0x0360, 0x0393, 0x03C9, 0x0403, 0x0440, 0x0481, 0x04C6, 0x050E,
	0x055B, 0x02D6, 0x0301, 0x032F, 0x0360, 0x0393, 0x03C9, 0x0403,
	0x0440, 0x0481, 0x04C6, 0x050E, 0x055B, 0x02D6, 0x0301, 0x032F,
	0x0360, 0x0393, 0x03C9, 0x0403, 0x0440, 0x0481, 0x04C6, 0x050E,
	0x055B, 0x02D6, 0x0301, 0x032F, 0x0360, 0x0393, 0x03C9, 0x0403,
	0x0440, 0x0481, 0x04C6, 0x050E, 0x055B, 0x02D6, 0x0301, 0x032F,
	0x0360, 0x0393, 0x03C9, 0x0403, 0x0440, 0x0481, 0x04C6, 0x050E,
	0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B,
	0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B,
	0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B,
	0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B
};

TownsMidiInputChannel::TownsMidiInputChannel(MidiDriver_TOWNS *driver, int chanIndex) : MidiChannel(), _driver(driver), _outChan(0), _prg(0), _chanIndex(chanIndex),
	_effectLevel(0), _priority(0), _ctrlVolume(0), _tl(0), _pan(0), _panEff(0), _transpose(0), _percS(0), _pitchBendFactor(0), _pitchBend(0), _sustain(0), _freqLSB(0),
	_fld_1f(0), _detune(0), _modWheel(0), _allocated(false) {
	_instrument = new uint8[30];
	memset(_instrument, 0, 30);
}

TownsMidiInputChannel::~TownsMidiInputChannel() {
	delete _instrument;
}

bool TownsMidiInputChannel::allocate() {
	if (_allocated)
		return false;
	_allocated = true;
	return true;
}

void TownsMidiInputChannel::release() {
	_allocated = false;
}

void TownsMidiInputChannel::send(uint32 b) {
	_driver->send(b | _chanIndex);
}

void TownsMidiInputChannel::noteOff(byte note) {
	if (!_outChan)
		return;

	if (_outChan->_note != note)
		return;

	if (_sustain)
		_outChan->_sustainNoteOff = 1;
	else
		_outChan->disconnect();
}

void TownsMidiInputChannel::noteOn(byte note, byte velocity) {
	TownsMidiOutputChannel *oc = _driver->allocateOutputChannel(_priority);
	
	if (!oc)
		return;

	oc->connect(this);

	oc->_fld_c = _instrument[10] & 1;
	oc->_note = note;
	oc->_sustainNoteOff = 0;
	oc->_duration = _instrument[29] * 63;
	
	oc->_modulatorTl = (_instrument[1] & 0x3f) + _driver->_chanOutputLevel[((velocity >> 1) << 5) + (_instrument[4] >> 2)];
	if (oc->_modulatorTl > 63)
		oc->_modulatorTl = 63;

	oc->_carrierTl = (_instrument[6] & 0x3f) + _driver->_chanOutputLevel[((velocity >> 1) << 5) + (_instrument[9] >> 2)];
	if (oc->_carrierTl > 63)
		oc->_carrierTl = 63;

	oc->setupProgram(_instrument, oc->_fld_c == 1 ? _programAdjustLevel[_driver->_chanOutputLevel[(_tl >> 2) + (oc->_modulatorTl << 5)]] : oc->_modulatorTl, _programAdjustLevel[_driver->_chanOutputLevel[(_tl >> 2) + (oc->_carrierTl << 5)]]);
	oc->noteOn(note + _transpose, _freqLSB);

	if (_instrument[11] & 0x80)
		oc->setupEffects(0, _instrument[11], &_instrument[12]);
	else
		oc->_stateA[0].numLoop = 0;

	if (_instrument[20] & 0x80)
		oc->setupEffects(1, _instrument[20], &_instrument[21]);
	else
		oc->_stateA[1].numLoop = 0;	
}

void TownsMidiInputChannel::programChange(byte program) {
	// Dysfunctional since this is all done inside the imuse code
}

void TownsMidiInputChannel::pitchBend(int16 bend) {
	_pitchBend = bend;
	_freqLSB = ((_pitchBend * _pitchBendFactor) >> 6) + _detune;
	for (TownsMidiOutputChannel *oc = _outChan; oc; oc = oc->_next)
		oc->noteOnPitchBend(oc->_note + oc->_midi->_transpose, _freqLSB);
}

void TownsMidiInputChannel::controlChange(byte control, byte value) {
	switch (control) {
	case 1:
		controlModulationWheel(value);
		break;
	case 7:
		controlVolume(value);
		break;
	case 10:
		controlPanPos(value);
		break;
	case 64:
		controlSustain(value);
		break;
	case 123:
		while (_outChan)
			_outChan->disconnect();
		break;
	default:
		break;
	}
}

void TownsMidiInputChannel::pitchBendFactor(byte value) {
	_pitchBendFactor = value;
	_freqLSB = ((_pitchBend * _pitchBendFactor) >> 6) + _detune;
	for (TownsMidiOutputChannel *oc = _outChan; oc; oc = oc->_next)
		oc->noteOnPitchBend(oc->_note + oc->_midi->_transpose, _freqLSB);
}

void TownsMidiInputChannel::priority(byte value) {
	_priority = value;
}

void TownsMidiInputChannel::sysEx_customInstrument(uint32 type, const byte *instr) {
	memcpy(_instrument, instr, 30);
}

void TownsMidiInputChannel::controlModulationWheel(byte value) {
	_modWheel = value;
	for (TownsMidiOutputChannel *oc = _outChan; oc; oc = oc->_next)
		oc->setModWheel(value);
}

void TownsMidiInputChannel::controlVolume(byte value) {
	/* This is all done inside the imuse code

	uint16 v1 = _ctrlVolume + 1;
	uint16 v2 = value;
	if (_chanIndex != 16) {
		_ctrlVolume = value;
		v2 = _player->getEffectiveVolume();
	}
	_tl = (v1 * v2) >> 7;*/

	_tl = value;
	
	/* nullsub
	_outChan->setVolume(_tl);
	*/
}

void TownsMidiInputChannel::controlPanPos(byte value) {
	// not supported
}

void TownsMidiInputChannel::controlSustain(byte value) {
	_sustain = value;
	if (!value)
		releasePedal();
}

void TownsMidiInputChannel::releasePedal() {
	for (TownsMidiOutputChannel *oc = _outChan; oc; oc = oc->_next) {
		if (oc->_sustainNoteOff)
			oc->disconnect();
	}
}

const uint8 TownsMidiInputChannel::_programAdjustLevel[] = {
	0x00, 0x04, 0x07, 0x0B, 0x0D, 0x10, 0x12, 0x14,
	0x16, 0x18, 0x1A, 0x1B, 0x1D, 0x1E, 0x1F, 0x21,
	0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
	0x2A, 0x2B, 0x2C, 0x2C, 0x2D, 0x2E, 0x2F, 0x2F,
	0x30, 0x31, 0x31, 0x32, 0x33, 0x33, 0x34, 0x35,
	0x35, 0x36, 0x36, 0x37, 0x37, 0x38, 0x38, 0x39,
	0x39, 0x3A, 0x3A, 0x3B, 0x3B, 0x3C, 0x3C, 0x3C,
	0x3D, 0x3D, 0x3E, 0x3E, 0x3E, 0x3F, 0x3F, 0x3F
};

MidiDriver_TOWNS::MidiDriver_TOWNS(Audio::Mixer *mixer) : _timerProc(0), _timerProcPara(0), _tickCounter1(0), _tickCounter2(0), _curChan(0), _rand(1), _open(false) {
	_intf = new TownsAudioInterface(mixer, this);

	_channels = new TownsMidiInputChannel*[32];
	for (int i = 0; i < 32; i++)
		_channels[i] = new TownsMidiInputChannel(this, i);
	
	_out = new TownsMidiOutputChannel*[6];
	for (int i = 0; i < 6; i++)
		_out[i] = new TownsMidiOutputChannel(this, i);

	_chanState = new TownsMidiChanState[32];

	_chanOutputLevel = new uint8[2048];
	for (int i = 0; i < 64; i++) {
		for (int ii = 0; ii < 32; ii++)
			_chanOutputLevel[(i << 5) + ii] = ((i * (ii + 1)) >> 5) & 0xff;
	}
	for (int i = 0; i < 64; i++)
		_chanOutputLevel[i << 5] = 0;
}

MidiDriver_TOWNS::~MidiDriver_TOWNS() {
	close();
	delete _intf;
	setTimerCallback(0, 0);

	for (int i = 0; i < 32; i++)
		delete _channels[i];
	delete[] _channels;

	for (int i = 0; i < 6; i++)
		delete _out[i];
	delete[] _out;

	delete[] _chanState;
	delete[] _chanOutputLevel;
}

int MidiDriver_TOWNS::open() {
	if (_open)
		return MERR_ALREADY_OPEN;

	if (!_intf->init())
		return MERR_CANNOT_CONNECT;

	_intf->callback(0);

	_intf->callback(21, 255, 1);
	_intf->callback(21, 0, 1);
	_intf->callback(22, 255, 221);

	_intf->callback(33, 8);
	_intf->setSoundEffectChanMask(~0x3f);

	_open = true;

	return 0;
}

void MidiDriver_TOWNS::close() {
	_open = false;
}

void MidiDriver_TOWNS::send(uint32 b) {
	byte param2 = (b >> 16) & 0xFF;
	byte param1 = (b >> 8) & 0xFF;
	byte cmd = b & 0xF0;

	TownsMidiInputChannel *c = _channels[b & 0x0F];

	switch (cmd) {
	case 0x80:
		c->noteOff(param1);
		break;
	case 0x90:
		if (param2)
			c->noteOn(param1, param2);
		else
			c->noteOff(param1);
		break;
	case 0xB0:
		c->controlChange(param1, param2);
		break;
	case 0xC0:
		c->programChange(param1);
		break;
	case 0xE0:
		c->pitchBend((param1 | (param2 << 7)) - 0x2000);
		break;
	case 0xF0:
		warning("MidiDriver_TOWNS: Receiving SysEx command on a send() call");
		break;

	default:
		break;
	}
}

void MidiDriver_TOWNS::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	_timerProc = timer_proc;
	_timerProcPara = timer_param;
}

uint32 MidiDriver_TOWNS::getBaseTempo() {
	return 10080;
}

MidiChannel *MidiDriver_TOWNS::allocateChannel() {
	for (int i = 0; i < 32; ++i) {		
		TownsMidiInputChannel *chan = _channels[i];
		if (chan->allocate())
			return chan;
	}

	return 0;
}

MidiChannel *MidiDriver_TOWNS::getPercussionChannel() {
	return 0;//_channels[16];
}

void MidiDriver_TOWNS::timerCallback(int timerId) {
	if (!_open)
		return;

	switch (timerId) {
	case 1:
		updateParser();
		updateOutputChannels();

		/*_tickCounter1 += 10000;
		while (_tickCounter1 >= 4167) {
			_tickCounter1 -= 4167;
			unkUpdate();
		}*/
		break;
	default:
		break;
	}
}

TownsMidiOutputChannel *MidiDriver_TOWNS::allocateOutputChannel(int pri) {
	TownsMidiOutputChannel *res = 0;

	for (int i = 0; i < 6; i++) {
		if (++_curChan == 6)
			_curChan = 0;

		int s = _out[i]->checkPriority(pri);
		if (s == TownsMidiOutputChannel::kDisconnected)
			return _out[i];

		if (s != TownsMidiOutputChannel::kHighPriority) {
			pri = s;
			res = _out[i];
		}
	}
	
	if (res)
		res->disconnect();

	return res;
}

void MidiDriver_TOWNS::updateParser() {
	if (_timerProc)
		_timerProc(_timerProcPara);
}

void MidiDriver_TOWNS::updateOutputChannels() {
	_tickCounter2 += 10000;
	while (_tickCounter2 >= 16667) {
		_tickCounter2 -= 16667;
		for (int i = 0; i < 6; i++) {
			TownsMidiOutputChannel *oc = _out[i];
			if (oc->update())
				return;
		}
	}
}

int MidiDriver_TOWNS::randomValue(int para) {
	_rand = (_rand & 1) ? (_rand >> 1) ^ 0xb8 : (_rand >> 1);
	return (_rand * para) >> 8;
}
