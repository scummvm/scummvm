/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
 *
 * YM2612 tone generation code written by Tomoaki Hayasaka.
 * Used under the terms of the GNU General Public License.
 * Adpated to ScummVM by Jamieson Christian.
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
 * $Header$
 */

#include "sound/softsynth/emumidi.h"

#include <math.h>

#include "common/util.h"

////////////////////////////////////////
//
// Miscellaneous
//
////////////////////////////////////////

static int *sintbl = 0;
static int *powtbl = 0;
static int *frequencyTable = 0;
static int *keycodeTable = 0;
static int *keyscaleTable = 0;
static int *attackOut = 0;

////////////////////////////////////////
//
// Class declarations
//
////////////////////////////////////////

class Operator2612;
class Voice2612;
class MidiChannel_YM2612;
class MidiDriver_YM2612;

class Operator2612 {
protected:
	Voice2612 *_owner;
	enum State { _s_ready, _s_attacking, _s_decaying, _s_sustaining, _s_releasing };
	State _state;
	int32 _currentLevel;
	int _frequency;
	uint32 _phase;
	int _lastOutput;
	int _feedbackLevel;
	int _detune;
	int _multiple;
	int32 _totalLevel;
	int _keyScale;
	int _velocity;
	int _specifiedTotalLevel;
	int _specifiedAttackRate;
	int _specifiedDecayRate;
	int _specifiedSustainLevel;
	int _specifiedSustainRate;
	int _specifiedReleaseRate;
	int _tickCount;
	int _attackTime;
	int32 _decayRate;
	int32 _sustainLevel;
	int32 _sustainRate;
	int32 _releaseRate;

public:
	Operator2612 (Voice2612 *owner);
	~Operator2612();
	void feedbackLevel(int level);
	void setInstrument(byte const *instrument);
	void velocity(int velo);
	void keyOn();
	void keyOff();
	void frequency(int freq);
	void nextTick(const int *phaseShift, int *outbuf, int buflen);
	bool inUse() { return (_state != _s_ready); }
};

class Voice2612 {
public:
	Voice2612 *next;
	uint16 _rate;

protected:
	Operator2612 *_opr[4];
	int _velocity;
	int _control7;
	int _note;
	int _frequencyOffs;
	int _frequency;
	int _algorithm;

	int *_buffer;
	int _buflen;

public:
	Voice2612();
	~Voice2612();
	void setControlParameter(int control, int value);
	void setInstrument(byte const *instrument);
	void velocity(int velo);
	void nextTick(int *outbuf, int buflen);
	void noteOn(int n, int onVelo);
	bool noteOff(int note);
	void pitchBend(int value);
	void recalculateFrequency();
};

class MidiChannel_YM2612 : public MidiChannel {
protected:
	uint16 _rate;
	Voice2612 *_voices;
	Voice2612 *_next_voice;

public:
	void removeAllVoices();
	void nextTick(int *outbuf, int buflen);
	void rate(uint16 r);

public:
	MidiChannel_YM2612();
	virtual ~MidiChannel_YM2612();

	// MidiChannel interface
	MidiDriver *device() { return 0; }
	byte getNumber() { return 0; }
	void release() { }
	void send(uint32 b) { }
	void noteOff(byte note);
	void noteOn(byte note, byte onVelo);
	void programChange(byte program) { }
	void pitchBend(int16 value);
	void controlChange(byte control, byte value);
	void pitchBendFactor(byte value) { }
	void sysEx_customInstrument(uint32 type, byte *instr);
};

class MidiDriver_YM2612 : public MidiDriver_Emulated {
protected:
	MidiChannel_YM2612 *_channel[16];

	int _next_voice;
	int _volume;

protected:
	static void createLookupTables();
	void nextTick(int16 *buf1, int buflen);
	int volume(int val = -1) { if (val >= 0) _volume = val; return _volume; }
	void rate(uint16 r);

	void generateSamples(int16 *buf, int len);

public:
	MidiDriver_YM2612(Audio::Mixer *mixer);
	virtual ~MidiDriver_YM2612();

	int open();
	void close();
	void send(uint32 b);
	void send(byte channel, uint32 b); // Supports higher than channel 15
	uint32 property(int prop, uint32 param) { return 0; }

	void setPitchBendRange(byte channel, uint range) { }
	void sysEx(byte *msg, uint16 length);

	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }


	// AudioStream API
	bool isStereo() const { return true; }
	int getRate() const { return _mixer->getOutputRate(); }
};

////////////////////////////////////////
//
// Operator2612 implementation
//
////////////////////////////////////////

Operator2612::Operator2612 (Voice2612 *owner) :
	_owner (owner),
	_state (_s_ready),
	_currentLevel ((int32)0x7f << 15),
	_phase (0),
	_lastOutput (0),
	_feedbackLevel (0),
	_detune (0),
	_multiple (1),
	_keyScale (0),
	_specifiedTotalLevel (127),
	_specifiedAttackRate (0),
	_specifiedDecayRate (0),
	_specifiedSustainRate (0),
	_specifiedReleaseRate (15) {
		velocity(0);
}

Operator2612::~Operator2612()
{ }

void Operator2612::velocity(int velo) {
	_velocity = velo;
	_totalLevel = ((int32)_specifiedTotalLevel << 15) +
					((int32)(127-_velocity) << 13);
	_sustainLevel = ((int32)_specifiedSustainLevel << 17);
}

void Operator2612::feedbackLevel(int level) {
	_feedbackLevel = level;
}

void Operator2612::setInstrument(byte const *instrument) {
	_detune = (instrument[8] >> 4) & 7;
	_multiple = instrument[8] & 15;
	_specifiedTotalLevel = instrument[12] & 127;
	_keyScale = (instrument[16] >> 6) & 3;
	_specifiedAttackRate = instrument[16] & 31;
	_specifiedDecayRate = instrument[20] & 31;
	_specifiedSustainRate = instrument[24] & 31;
	_specifiedSustainLevel = (instrument[28] >> 4) & 15;
	_specifiedReleaseRate = instrument[28] & 15;
	_state = _s_ready; // 本物ではどうなのかな?
	velocity(_velocity);
}

void Operator2612::keyOn() {
	_state = _s_attacking;
	_tickCount = 0;
	_phase = 0;			// どうも、実際こうらしい
	_currentLevel = ((int32)0x7f << 15); // これも、実際こうらしい
}

void Operator2612::keyOff() {
	if (_state != _s_ready)
		_state = _s_releasing;
}

void Operator2612::frequency(int freq) {
	double value; // Use for intermediate computations to avoid int64 arithmetic
	int r;

	_frequency = freq / _owner->_rate;

	r = _specifiedAttackRate;
	if (r != 0) {
		r = r * 2 + (keyscaleTable[freq/262205] >> (3-_keyScale));
		if (r >= 64)
		r = 63; // するべきなんだろうとは思うんだけど (赤p.207)
	}

	r = 63 - r;
	if (_specifiedTotalLevel >= 128)
		value = 0;
	else {
		value = powtbl[(r&3) << 7];
		value *= 1 << (r >> 2);
		value *= 41; // r == 20 のとき、0-96[db] が 10.01[ms] == 41.00096
		value /= 1 << (15 + 5);
		value *= 127 - _specifiedTotalLevel;
		value /= 127;
	}
	_attackTime = (int32) value; // 1 秒 == (1 << 12)
	if (_attackTime > 0)
		_attackTime = (1 << (12+10)) / (_owner->_rate * _attackTime);

	r = _specifiedDecayRate;
	if (r != 0) {
		r = r * 2 + (keyscaleTable[freq/262205] >> (3-_keyScale));
		if (r >= 64)
			r = 63;
	}
	value = (double) powtbl[(r&3) << 7] * (0x10 << (r>>2)) / 31;
	_decayRate = (int32) value / _owner->_rate;

	r = _specifiedSustainRate;
	if (r != 0) {
		r = r * 2 + (keyscaleTable[freq/262205] >> (3-_keyScale));
		if (r >= 64)
			r = 63;
	}
	value = (double) powtbl[(r&3) << 7] * (0x10 << (r>>2)) / 31;
	_sustainRate = (int32) value / _owner->_rate;

	r = _specifiedReleaseRate;
	if (r != 0) {
		r = r * 2 + 1;		// (Translated) I cannot know whether the timing is a good choice or not
		r = r * 2 + (keyscaleTable[freq/262205] >> (3-_keyScale));
		// KS による補正はあるらしい。赤p.206 では記述されてないけど。
		if (r >= 64)
			r = 63;
	}
	value = (double) powtbl[(r&3) << 7] * (0x10 << (r>>2)) / 31;
	_releaseRate = (int32) value / _owner->_rate;
}

void Operator2612::nextTick(const int *phasebuf, int *outbuf, int buflen) {
	if (_state == _s_ready)
		return;
	if (_state == _s_attacking && _attackTime <= 0) {
		_currentLevel = 0;
		_state = _s_decaying;
	}

	int32 levelIncrement = 0;
	int32 target = 0;
	State next_state = _s_ready;
	const int32 zero_level = ((int32)0x7f << 15);
	const int phaseIncrement = (_multiple > 0) ? (_frequency * _multiple) : (_frequency / 2);

	int32 output = _lastOutput;
	int32 level = _currentLevel + _totalLevel;

	while (buflen) {
		switch (_state) {
		case _s_ready:
			return;
			break;
		case _s_attacking:
			next_state = _s_attacking;
			break;
		case _s_decaying:
			levelIncrement = _decayRate;
			target = _sustainLevel + _totalLevel;
			next_state = _s_sustaining;
			break;
		case _s_sustaining:
			levelIncrement = _sustainRate;
			target = zero_level + _totalLevel;
			next_state = _s_ready;
			break;
		case _s_releasing:
			levelIncrement = _releaseRate;
			target = zero_level + _totalLevel;
			next_state = _s_ready;
			break;
		}

		bool switching = false;
		do {
			if (next_state == _s_attacking) {
				// Attack phase
				++_tickCount;
				int i = (int) (_tickCount * _attackTime);
				if (i >= 1024) {
					level = _totalLevel;
					_state = _s_decaying;
					switching = true;
				} else {
					level = (attackOut[i] << (31 - 8 - 16)) + _totalLevel;
				}
			} else {
				// Decay, Sustain and Release phases
				level += levelIncrement;
				if (level >= target) {
					level = target;
					_state = next_state;
					switching = true;
				}
			}

			if (level < zero_level) {
				int phaseShift = *phasebuf >> 2; // 正しい変調量は?  3 じゃ小さすぎで 2 じゃ大きいような。
				if (_feedbackLevel)
					phaseShift += (output << (_feedbackLevel - 1)) / 1024;
				output = sintbl[((_phase >> 7) + phaseShift) & 0x7ff];
				output >>= (level >> 18);	// 正しい減衰量は?
				// Here is the original code, which requires 64-bit ints
//				output *= powtbl[511 - ((level>>25)&511)];
//				output >>= 16;
//				output >>= 1;
				// And here's our 32-bit trick for doing it. (Props to Fingolfin!)
				// Result varies from original code by max of 1.
//				int powVal = powtbl[511 - ((level>>9)&511)];
//				int outputHI = output / 256;
//				int powHI = powVal / 256;
//				output = (outputHI * powHI) / 2 + (outputHI * (powVal % 256) + powHI * (output % 256)) / 512;
				// And here's the even faster code.
				// Result varies from original code by max of 8.
				output = ((output >> 4) * (powtbl[511-((level>>9)&511)] >> 3)) / 1024;

				_phase += phaseIncrement;
				_phase &= 0x3ffff;
			} else
				output = 0;

			*outbuf += output;
			 --buflen;
			 ++phasebuf;
			 ++outbuf;
		} while (buflen && !switching);
	}
	_lastOutput = output;
	_currentLevel = level - _totalLevel;
}

////////////////////////////////////////
//
// Voice2612 implementation
//
////////////////////////////////////////

Voice2612::Voice2612() {
	next = 0;
	_control7 = 127;
	_note = 40;
	_frequency = 440;
	_frequencyOffs = 0x2000;
	_algorithm = 7;

	_buffer = 0;
	_buflen = 0;

	int i;
	for (i = 0; i < ARRAYSIZE(_opr); ++i)
		_opr[i] = new Operator2612 (this);
	velocity(0);
}

Voice2612::~Voice2612() {
	int i;
	for (i = 0; i < ARRAYSIZE(_opr); ++i)
		delete _opr[i];
	free(_buffer);
}

void Voice2612::velocity(int velo) {
	_velocity = velo;
#if 0
	int v = (velo * _control7) >> 7; // これだと精度良くないですね
#else
	int v = velo + (_control7 - 127) * 4;
#endif
	bool iscarrier[8][4] = {
		{ false, false, false,  true, }, //0
		{ false, false, false,  true, }, //1
		{ false, false, false,  true, }, //2
		{ false, false, false,  true, }, //3
		{ false,  true, false,  true, }, //4
		{ false,  true,  true,  true, }, //5
		{ false,  true,  true,  true, }, //6
		{  true,  true,  true,  true, }, //7
	};
	int opr;
	for (opr = 0; opr < 4; opr++)
		if (iscarrier[_algorithm][opr])
			_opr[opr]->velocity(v);
		else
			_opr[opr]->velocity(127);
}

void Voice2612::setControlParameter(int control, int value) {
	switch (control) {
	case 7:
		_control7 = value;
		velocity(_velocity);
		break;
	case 123:
		// All notes off
		noteOff(_note);
	};
}

void Voice2612::setInstrument(byte const *instrument) {
	if (instrument == NULL)
		return;

	_algorithm = instrument[32] & 7;
	_opr[0]->feedbackLevel((instrument[32] >> 3) & 7);
	_opr[1]->feedbackLevel(0);
	_opr[2]->feedbackLevel(0);
	_opr[3]->feedbackLevel(0);
	_opr[0]->setInstrument(instrument + 0);
	_opr[1]->setInstrument(instrument + 2);
	_opr[2]->setInstrument(instrument + 1);
	_opr[3]->setInstrument(instrument + 3);
}

void Voice2612::nextTick(int *outbuf, int buflen) {
	if (_velocity == 0)
		return;

	if (_buflen < buflen) {
		free(_buffer);
		_buflen = buflen;
		_buffer = (int *) malloc(sizeof(int) * buflen * 2);
	}

	int *buf1 = _buffer;
	int *buf2 = _buffer + buflen;
	memset(_buffer, 0, sizeof(int) * buflen * 2);

	switch (_algorithm) {
	case 0:
		_opr[0]->nextTick(buf1, buf2, buflen);
		_opr[1]->nextTick(buf2, buf1, buflen);
		memset (buf2, 0, sizeof (int) * buflen);
		_opr[2]->nextTick(buf1, buf2, buflen);
		_opr[3]->nextTick(buf2, outbuf, buflen);
		break;
	case 1:
		_opr[0]->nextTick(buf1, buf2, buflen);
		_opr[1]->nextTick(buf1, buf2, buflen);
		_opr[2]->nextTick(buf2, buf1, buflen);
		_opr[3]->nextTick(buf1, outbuf, buflen);
		break;
	case 2:
		_opr[1]->nextTick(buf1, buf2, buflen);
		_opr[2]->nextTick(buf2, buf1, buflen);
		memset(buf2, 0, sizeof(int) * buflen);
		_opr[0]->nextTick(buf2, buf1, buflen);
		_opr[3]->nextTick(buf1, outbuf, buflen);
		break;
	case 3:
		_opr[0]->nextTick(buf1, buf2, buflen);
		_opr[1]->nextTick(buf2, buf1, buflen);
		memset(buf2, 0, sizeof(int) * buflen);
		_opr[2]->nextTick(buf2, buf1, buflen);
		_opr[3]->nextTick(buf1, outbuf, buflen);
		break;
	case 4:
		_opr[0]->nextTick(buf1, buf2, buflen);
		_opr[1]->nextTick(buf2, outbuf, buflen);
		_opr[2]->nextTick(buf1, buf1, buflen);
		_opr[3]->nextTick(buf1, outbuf, buflen);
		break;
	case 5:
		_opr[0]->nextTick(buf1, buf2, buflen);
		_opr[1]->nextTick(buf2, outbuf, buflen);
		_opr[2]->nextTick(buf2, outbuf, buflen);
		_opr[3]->nextTick(buf2, outbuf, buflen);
		break;
	case 6:
		_opr[0]->nextTick(buf1, buf2, buflen);
		_opr[1]->nextTick(buf2, outbuf, buflen);
		_opr[2]->nextTick(buf1, outbuf, buflen);
		_opr[3]->nextTick(buf1, outbuf, buflen);
		break;
	case 7:
		_opr[0]->nextTick(buf1, outbuf, buflen);
		_opr[1]->nextTick(buf1, outbuf, buflen);
		_opr[2]->nextTick(buf1, outbuf, buflen);
		_opr[3]->nextTick(buf1, outbuf, buflen);
		break;
	};
}

void Voice2612::noteOn(int n, int onVelo) {
	_note = n;
	velocity(onVelo);
	recalculateFrequency();
	int i;
	for (i = 0; i < ARRAYSIZE(_opr); i++)
		_opr[i]->keyOn();
}

bool Voice2612::noteOff(int note) {
	if (_note != note)
		return false;
	int i;
	for (i = 0; i < ARRAYSIZE(_opr); i++)
		_opr[i]->keyOff();
	return true;
}

void Voice2612::pitchBend(int value) {
	_frequencyOffs = value;
	recalculateFrequency();
}

void Voice2612::recalculateFrequency() {
	// MIDI とも違うし....
	// どういう仕様なんだろうか?
	// と思ったら、なんと、これ (↓) が正解らしい。
	int32 basefreq = frequencyTable[_note];
	int cfreq = frequencyTable[_note - (_note % 12)];
	int oct = _note / 12;
	int fnum = (int) (((double)basefreq * (1 << 13)) / cfreq); // OPL の fnum と同じようなもの。
	fnum += _frequencyOffs - 0x2000;
	if (fnum < 0x2000) {
		fnum += 0x2000;
		oct--;
	}
	if (fnum >= 0x4000) {
		fnum -= 0x2000;
		oct++;
	}

	// _frequency は最終的にバイアス 256*1024 倍
	_frequency = (int) ((frequencyTable[oct*12] * (double)fnum) / 8);

	int i;
	for (i = 0; i < ARRAYSIZE(_opr); i++)
		_opr[i]->frequency(_frequency);
}

////////////////////////////////////////
//
// MidiChannel_YM2612
//
////////////////////////////////////////

MidiChannel_YM2612::MidiChannel_YM2612() {
	_voices = 0;
	_next_voice = 0;
}

MidiChannel_YM2612::~MidiChannel_YM2612() {
	removeAllVoices();
}

void MidiChannel_YM2612::removeAllVoices() {
	if (!_voices)
		return;
	Voice2612 *last, *voice = _voices;
	for (; voice; voice = last) {
		last = voice->next;
		delete voice;
	}
	_voices = _next_voice = 0;
}

void MidiChannel_YM2612::noteOn(byte note, byte onVelo) {
	if (!_voices)
		return;
	_next_voice = _next_voice ? _next_voice : _voices;
	_next_voice->noteOn(note, onVelo);
	_next_voice = _next_voice->next;
}

void MidiChannel_YM2612::noteOff(byte note) {
	if (!_voices)
		return;
	if (_next_voice == _voices)
		_next_voice = 0;
	Voice2612 *voice = _next_voice;
	do {
		if (!voice)
			voice = _voices;
		if (voice->noteOff(note)) {
			_next_voice = voice;
			break;
		}
		voice = voice->next;
	} while (voice != _next_voice);
}

void MidiChannel_YM2612::controlChange(byte control, byte value) {
	// いいのかこれで?
	if (control == 121) {
		// Reset controller
		removeAllVoices();
	} else {
		Voice2612 *voice = _voices;
		for (; voice; voice = voice->next)
			voice->setControlParameter(control, value);
	}
}

void MidiChannel_YM2612::sysEx_customInstrument(uint32 type, byte *fmInst) {
	if (type != 'EUP ')
		return;
	Voice2612 *voice = new Voice2612;
	voice->next = _voices;
	_voices = voice;
	voice->_rate = _rate;
	voice->setInstrument(fmInst);
}

void MidiChannel_YM2612::pitchBend(int16 value) {
  // いいのかこれで?
	Voice2612 *voice = _voices;
	for (; voice; voice = voice->next)
		voice->pitchBend(value);
}

void MidiChannel_YM2612::nextTick(int *outbuf, int buflen) {
	Voice2612 *voice = _voices;
	for (; voice; voice = voice->next)
		voice->nextTick(outbuf, buflen);
}

void MidiChannel_YM2612::rate(uint16 r) {
	_rate = r;
	Voice2612 *voice = _voices;
	for (; voice; voice = voice->next)
		voice->_rate = r;
}

////////////////////////////////////////
//
// MidiDriver_YM2612
//
////////////////////////////////////////

MidiDriver_YM2612::MidiDriver_YM2612(Audio::Mixer *mixer)
	: MidiDriver_Emulated(mixer) {
	_next_voice = 0;

	createLookupTables();
	_volume = 256;
	int i;
	for (i = 0; i < ARRAYSIZE(_channel); i++)
		_channel[i] = new MidiChannel_YM2612;
	rate(getRate());
}

MidiDriver_YM2612::~MidiDriver_YM2612() {
	int i;
	for (i = 0; i < ARRAYSIZE(_channel); i++)
		delete _channel[i];
	delete sintbl;
	delete powtbl;
	delete frequencyTable;
	delete keycodeTable;
	delete keyscaleTable;
	delete attackOut;
	sintbl = powtbl = frequencyTable = keycodeTable = keyscaleTable = attackOut = 0;
}

int MidiDriver_YM2612::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	MidiDriver_Emulated::open();

	_mixer->setupPremix(this);
	return 0;
}

void MidiDriver_YM2612::close() {
	if (!_isOpen)
		return;
	_isOpen = false;

	// Detach the premix callback handler
	_mixer->setupPremix(0);
}

void MidiDriver_YM2612::send(uint32 b) {
	send(b & 0xF, b & 0xFFFFFFF0);
}

void MidiDriver_YM2612::send(byte chan, uint32 b) {
	//byte param3 = (byte) ((b >> 24) & 0xFF);
	byte param2 = (byte) ((b >> 16) & 0xFF);
	byte param1 = (byte) ((b >>  8) & 0xFF);
	byte cmd    = (byte) (b & 0xF0);
	if (chan > ARRAYSIZE(_channel))
		return;

	switch (cmd) {
	case 0x80:// Note Off
		_channel[chan]->noteOff(param1);
		break;
	case 0x90: // Note On
		_channel[chan]->noteOn(param1, param2);
		break;
	case 0xA0: // Aftertouch
		break; // Not supported.
	case 0xB0: // Control Change
		_channel[chan]->controlChange(param1, param2);
		break;
	case 0xC0: // Program Change
		_channel[chan]->programChange(param1);
		break;
	case 0xD0: // Channel Pressure
		break; // Not supported.
	case 0xE0: // Pitch Bend
		_channel[chan]->pitchBend((param1 | (param2 << 7)) - 0x2000);
		break;
	case 0xF0: // SysEx
		// We should never get here! SysEx information has to be
		// sent via high-level semantic methods.
		warning("MidiDriver_YM2612: Receiving SysEx command on a send() call");
		break;

	default:
		warning("MidiDriver_YM2612: Unknown send() command 0x%02X", cmd);
	}
}

void MidiDriver_YM2612::sysEx(byte *msg, uint16 length) {
	if (msg[0] != 0x7C || msg[1] >= ARRAYSIZE(_channel))
		return;
	_channel[msg[1]]->sysEx_customInstrument('EUP ', &msg[2]);
}

void MidiDriver_YM2612::generateSamples(int16 *data, int len) {
	memset(data, 0, 2 * sizeof(int16) * len);
	nextTick(data, len);
}

void MidiDriver_YM2612::nextTick(int16 *buf1, int buflen) {
	int *buf0 = (int *)buf1;

	int i;
	for (i = 0; i < ARRAYSIZE(_channel); i++)
		_channel[i]->nextTick(buf0, buflen);

	for (i = 0; i < buflen; ++i)
		buf1[i*2+1] = buf1[i*2] = ((buf0[i] * volume()) >> 10) & 0xffff;
}

void MidiDriver_YM2612::rate(uint16 r)
{
	int i;
	for (i = 0; i < ARRAYSIZE(_channel); i++)
		_channel[i]->rate(r);
}

void MidiDriver_YM2612::createLookupTables() {
	{
		int i;
		sintbl = new int [2048];
		for (i = 0; i < 2048; i++)
			sintbl[i] = (int)(0xffff * sin(i/2048.0*2.0*PI));
	}

	{
		int i;
		powtbl = new int [1025];
		for (i = 0; i <= 1024; i++)
			powtbl[i] = (int)(0x10000 * pow(2.0, (i-512)/512.0));
	}

	{
		int i;
		int block;

		static int fnum[] = {
			0x026a, 0x028f, 0x02b6, 0x02df,
			0x030b, 0x0339, 0x036a, 0x039e,
			0x03d5, 0x0410, 0x044e, 0x048f,
		};

		// (int)(880.0 * 256.0 * pow(2.0, (note-0x51)/12.0)); // バイアス 256 倍
		// 0x45 が 440Hz (a4)、0x51 が 880Hz (a5) らしい
		frequencyTable = new int [120];
		for (block = -1; block < 9; block++) {
			for (i = 0; i < 12; i++) {
				double freq = fnum[i] * (166400.0 / 3) * pow(2.0, block-21);
				frequencyTable[(block+1)*12+i] = (int)(256.0 * freq);
			}
		}

		keycodeTable = new int [120];
		// detune 量の計算や KS による rate 変換に使うんじゃないかな
		for (block = -1; block < 9; block++) {
			for (i = 0; i < 12; i++) {
				// see p.204
				int  f8 = (fnum[i] >>  7) & 1;
				int  f9 = (fnum[i] >>  8) & 1;
				int f10 = (fnum[i] >>  9) & 1;
				int f11 = (fnum[i] >> 10) & 1;
				int  n4 = f11;
				int  n3 = f11&(f10|f9|f8) | (~f11&f10&f9&f8);
				int note = n4*2 + n3;
				// see p.207
				keycodeTable[(block+1)*12+i] = block*4 + note;
			}
		}
	}

	{
		int freq;
		keyscaleTable = new int [8192];
		keyscaleTable[0] = 0;
		for (freq = 1; freq < 8192; freq++) {
			keyscaleTable[freq] = (int)(log((double)freq) / 9.03 * 32.0) - 1;
			// 8368[Hz] (o9c) で 32くらい。9.03 =:= ln 8368
		}
	}

	{
		int i;
		attackOut = new int [1024];
		for (i = 0; i < 1024; i++)
			attackOut[i] = (int)(((0x7fff+0x03a5)*30.0) / (30.0+i)) - 0x03a5;
	}
}

////////////////////////////////////////
//
// MidiDriver_YM2612 factory
//
////////////////////////////////////////

MidiDriver *MidiDriver_YM2612_create(Audio::Mixer *mixer) {
	return new MidiDriver_YM2612(mixer);
}
