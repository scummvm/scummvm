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


#include "common/system.h"
#include "kyra/resource.h"
#include "kyra/sound.h"
#include "kyra/screen.h"

#include "sound/audiocd.h"
#include "sound/audiostream.h"

#include "common/util.h"
#include <math.h>

namespace Kyra {

enum EuD_ChannelState { _s_ready, _s_attacking, _s_decaying, _s_sustaining, _s_releasing };

class MidiChannel_EuD : public MidiChannel {
public:
	MidiChannel_EuD() {}
	~MidiChannel_EuD() {}

	virtual void nextTick(int32 *outbuf, int buflen) = 0;
	virtual void rate(uint16 r) = 0;

protected:
	uint16 _rate;
};

class MidiChannel_EuD_FM : public MidiChannel_EuD {
public:
	MidiChannel_EuD_FM();
	virtual ~MidiChannel_EuD_FM();

	void nextTick(int32 *outbuf, int buflen);
	void rate(uint16 r);

	// MidiChannel interface
	MidiDriver *device() { return 0; }
	byte getNumber() { return 0; }
	void release() { }
	void send(uint32 b) { }
	void noteOff(byte note);
	void noteOn(byte note, byte onVelo);
	void programChange(byte program) {}
	void pitchBend(int16 value);
	void controlChange(byte control, byte value);
	void pitchBendFactor(byte value) { }
	void sysEx_customInstrument(uint32 unused, const byte *instr);

protected:
	Voice2612 *_voice;
};

class MidiChannel_EuD_WAVE : public MidiChannel_EuD {
public:
	void nextTick(int32 *outbuf, int buflen);
	void rate(uint16 r);

	MidiChannel_EuD_WAVE();
	virtual ~MidiChannel_EuD_WAVE();

	// MidiChannel interface
	MidiDriver *device() { return 0; }
	byte getNumber() { return 0; }
	void release() { }
	void send(uint32 b) { }
	void noteOff(byte note);
	void noteOn(byte note, byte onVelo);
	void programChange(byte program) {}
	void pitchBend(int16 value);
	void controlChange(byte control, byte value);
	void pitchBendFactor(byte value) { }
	void sysEx_customInstrument(uint32 type, const byte *instr);

protected:
	void velocity(int velo);
	void panPosition(int8 pan);
	void evpNextTick();

	int _ctrl7_volume;
	int16 _velocity;
	int16 _note;
	int32 _frequencyOffs;
	float _phase;
	int8 _current;

	struct Voice {
		char name[9];
		uint16 split[8];
		uint32 id[8];
		struct Snd {
			char name[9];
			int32 id;
			int32 numSamples;
			int32 loopStart;
			int32 loopLength;
			int32 samplingRate;
			int32 keyOffset;
			int32 keyNote;
			const int8 *_samples;
		} * _snd[8];
		struct Env {
			EuD_ChannelState state;
			int32 currentLevel;
			int32 rate;
			int32 tickCount;
			int32 totalLevel;
			int32 attackRate;
			int32 decayRate;
			int32 sustainLevel;
			int32 sustainRate;
			int32 releaseLevel;
			int32 releaseRate;
			int32 rootKeyOffset;
			int32 size;
		} * _env[8];
	} * _voice;
};

class MidiParser_EuD : public MidiParser {
public:
	MidiParser_EuD();

	bool loadMusic (byte *data, uint32 unused = 0);
	int32 calculateTempo(int16 val);

protected:
	void parseNextEvent (EventInfo &info);
	void resetTracking();

	byte * _enable;
	byte * _mode;
	byte * _channel;
	byte * _adjVelo;
	int8 * _adjNote;

	byte _tempo[3];

	uint8 _firstBaseTickStep;
	uint8 _nextBaseTickStep;
	uint32 _initialTempo;
	uint32 _baseTick;
};

class FMT_EuphonyDriver : public MidiDriver_Emulated {
public:
	FMT_EuphonyDriver(Audio::Mixer *mixer);
	virtual ~FMT_EuphonyDriver();

	int open();
	void close();
	void send(uint32 b);
	void send(byte channel, uint32 b);
	uint32 property(int prop, uint32 param) { return 0; }

	void setPitchBendRange(byte channel, uint range) { }
	//void sysEx(const byte *msg, uint16 length);
	void loadFmInstruments(const byte *instr);
	void loadWaveInstruments(const byte *instr);

	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }

	void assignFmChannel(uint8 midiChannelNumber, uint8 fmChannelNumber);
	void assignWaveChannel(uint8 midiChannelNumber, uint8 waveChannelNumber);
	void removeChannel(uint8 midiChannelNumber);

	// AudioStream API
	bool isStereo() const { return true; }
	int getRate() const { return _mixer->getOutputRate(); }

	void fading(bool status = true);

protected:
	void nextTick(int16 *buf1, int buflen);
	int volume(int val = -1) { if (val >= 0) _volume = val; return _volume; }
	void rate(uint16 r);

	void generateSamples(int16 *buf, int len);

	MidiChannel_EuD_FM *_fChannel[6];
	MidiChannel_EuD_WAVE *_wChannel[8];
	MidiChannel_EuD * _channel[16];

	int _volume;
	bool _fading;
	int16 _fadestate;

	uint8 *_fmInstruments;
	uint8 *_waveInstruments;
	int8 * _waveSounds[10];
};

MidiChannel_EuD_FM::MidiChannel_EuD_FM() {
	_voice = new Voice2612;
}

MidiChannel_EuD_FM::~MidiChannel_EuD_FM() {
	delete _voice;
}

void MidiChannel_EuD_FM::noteOn(byte note, byte onVelo) {
	_voice->noteOn(note, onVelo);
}

void MidiChannel_EuD_FM::noteOff(byte note) {
	_voice->noteOff(note);
}

void MidiChannel_EuD_FM::controlChange(byte control, byte value) {
	if (control == 121) {
		// Reset controller
		delete _voice;
		_voice = new Voice2612;
	} else if (control == 10) {
		// pan position
	} else {
		_voice->setControlParameter(control, value);
	}
}

void MidiChannel_EuD_FM::sysEx_customInstrument(uint32, const byte *fmInst) {
	_voice->_rate = _rate;
	_voice->setInstrument(fmInst);
}

void MidiChannel_EuD_FM::pitchBend(int16 value) {
	_voice->pitchBend(value);
}

void MidiChannel_EuD_FM::nextTick(int32 *outbuf, int buflen) {
	_voice->nextTick((int*) outbuf, buflen);
}

void MidiChannel_EuD_FM::rate(uint16 r) {
	_rate = r;
	_voice->_rate = r;
}

MidiChannel_EuD_WAVE::MidiChannel_EuD_WAVE() {
	_voice = new Voice;
	for (uint8 i = 0; i < 8; i++) {
		_voice->_env[i] = new Voice::Env;
		_voice->_snd[i] = 0;
	}

	_ctrl7_volume = 127;
	velocity(0);
	_frequencyOffs = 0x2000;
	_current = -1;
}

MidiChannel_EuD_WAVE::~MidiChannel_EuD_WAVE() {
	for (uint8 i = 0; i < 8; i++) {
		if (_voice->_snd[i])
			delete _voice->_snd[i];
		delete _voice->_env[i];
	}
	delete _voice;
}

void MidiChannel_EuD_WAVE::noteOn(byte note, byte onVelo) {
	_note = note;
	velocity(onVelo);
	_phase = 0;

	for (_current = 0; _current < 7; _current++) {
		if (note <=	_voice->split[_current])
			break;
	}

	_voice->_env[_current]->state = _s_attacking;
	_voice->_env[_current]->currentLevel = 0;
	_voice->_env[_current]->rate = _rate;
	_voice->_env[_current]->tickCount = 0;
}

void MidiChannel_EuD_WAVE::noteOff(byte note) {
    if (_current == -1)
		return;
	if (_voice->_env[_current]->state == _s_ready)
		return;

	_voice->_env[_current]->state = _s_releasing;
	_voice->_env[_current]->releaseLevel = _voice->_env[_current]->currentLevel;
	_voice->_env[_current]->tickCount = 0;
}

void MidiChannel_EuD_WAVE::controlChange(byte control, byte value) {
	switch (control) {
		case 0x07:
			// volume
			_ctrl7_volume = value;
			break;
		case 0x0A:
			// pan position
			break;
		case 0x79:
			// Reset controller
			for (uint8 i = 0; i < 8; i++) {
				if (_voice->_snd[i])
					delete _voice->_snd[i];
				delete _voice->_env[i];
			}
			delete _voice;
			_voice = new Voice;
			for (uint8 i = 0; i < 8; i++) {
				_voice->_env[i] = new Voice::Env;
				_voice->_snd[i] = 0;
			}
			break;
		case 0x7B:
			noteOff(_note);
			break;
		default:
			break;
	}
}

void MidiChannel_EuD_WAVE::sysEx_customInstrument(uint32 type, const byte *fmInst) {
	if (type == 0x80) {
		for (uint8 i = 0; i < 8; i++) {
			const byte * const* pos = (const byte * const*) fmInst;
			for (uint8 ii = 0; ii < 10; ii++) {
				if (_voice->id[i] == *(pos[ii] + 8)) {
					if (!_voice->_snd[i])
						_voice->_snd[i] = new Voice::Snd;
					memset (_voice->_snd[i]->name, 0, 9);
					memcpy (_voice->_snd[i]->name, (const char*) pos[ii], 8);
					_voice->_snd[i]->id = READ_LE_UINT32(pos[ii] + 8);
					_voice->_snd[i]->numSamples = READ_LE_UINT32(pos[ii] + 12);
					_voice->_snd[i]->loopStart = READ_LE_UINT32(pos[ii] + 16);
					_voice->_snd[i]->loopLength = READ_LE_UINT32(pos[ii] + 20);
					_voice->_snd[i]->samplingRate = READ_LE_UINT16(pos[ii] + 24);
					_voice->_snd[i]->keyOffset = READ_LE_UINT16(pos[ii] + 26);
					_voice->_snd[i]->keyNote = *(const uint8*)(pos[ii] + 28);
					_voice->_snd[i]->_samples = (const int8*)(pos[ii] + 32);
				}
			}
		}
	} else {
		memset (_voice->name, 0, 9);
		memcpy (_voice->name, (const char*) fmInst, 8);

		for (uint8 i = 0; i < 8; i++) {
			_voice->split[i] = READ_LE_UINT16(fmInst + 16 + 2 * i);
			_voice->id[i] = READ_LE_UINT32(fmInst + 32 + 4 * i);
			_voice->_snd[i] = 0;
			_voice->_env[i]->state = _s_ready;
			_voice->_env[i]->currentLevel = 0;
			_voice->_env[i]->totalLevel = *(fmInst + 64 + 8 * i);
			_voice->_env[i]->attackRate = *(fmInst + 65 + 8 * i) * 10;
			_voice->_env[i]->decayRate = *(fmInst + 66 + 8 * i) * 10;
			_voice->_env[i]->sustainLevel = *(fmInst + 67 + 8 * i);
			_voice->_env[i]->sustainRate = *(fmInst + 68 + 8 * i) * 20;
			_voice->_env[i]->releaseRate = *(fmInst + 69 + 8 * i) * 10;
			_voice->_env[i]->rootKeyOffset = *(fmInst + 70 + 8 * i);
		}
	}
}

void MidiChannel_EuD_WAVE::pitchBend(int16 value) {
	_frequencyOffs = value;
}

void MidiChannel_EuD_WAVE::nextTick(int32 *outbuf, int buflen) {
	if (_current == -1 || !_voice->_snd[_current] || !_voice->_env[_current]->state || !_velocity) {
		velocity(0);
		_current = -1;
		return;
	}

	float phaseStep = SoundTowns::semitoneAndSampleRate_to_sampleStep(_note, _voice->_snd[_current]->keyNote -
		_voice->_env[_current]->rootKeyOffset, _voice->_snd[_current]->samplingRate, _rate, _frequencyOffs);

	int32 looplength = _voice->_snd[_current]->loopLength;
	int32 numsamples = _voice->_snd[_current]->numSamples;
	const int8 * samples = _voice->_snd[_current]->_samples;

	for (int i = 0; i < buflen; i++) {
		if (looplength > 0) {
			while (_phase >= numsamples)
				_phase -= looplength;
		} else {
			if (_phase >= numsamples) {
				velocity(0);
				_current = -1;
				break;
			}
		}

		int32 output;

		int32 phase0 = int32(_phase);
		int32 phase1 = int32(_phase + 1);
		if (phase1 >= numsamples)
			phase1 -= looplength;
		float weight0 = _phase - phase0;
		float weight1 = phase1 - _phase;
		output = int32(samples[phase0] * weight0 + samples[phase1] * weight1);

		output *= _velocity;
		output <<= 1;

		evpNextTick();
		output *= _voice->_env[_current]->currentLevel;
		output >>= 7;
		output *= _ctrl7_volume;
		output >>= 7;

		output *= 185;
		output >>= 8;
		outbuf[i] += output;
		_phase += phaseStep;
	}
}

void MidiChannel_EuD_WAVE::evpNextTick() {
	switch (_voice->_env[_current]->state) {
		case _s_ready:
			_voice->_env[_current]->currentLevel = 0;
			return;

		case _s_attacking:
			if (_voice->_env[_current]->attackRate == 0)
				_voice->_env[_current]->currentLevel = _voice->_env[_current]->totalLevel;
			else if (_voice->_env[_current]->attackRate >= 1270)
				_voice->_env[_current]->currentLevel = 0;
			else
				_voice->_env[_current]->currentLevel = (_voice->_env[_current]->totalLevel *
					_voice->_env[_current]->tickCount++ * 1000) /
						(_voice->_env[_current]->attackRate * _voice->_env[_current]->rate);

			if (_voice->_env[_current]->currentLevel >= _voice->_env[_current]->totalLevel) {
				_voice->_env[_current]->currentLevel = _voice->_env[_current]->totalLevel;
				_voice->_env[_current]->state = _s_decaying;
				_voice->_env[_current]->tickCount = 0;
			}
			break;

		case _s_decaying:
			if (_voice->_env[_current]->decayRate == 0)
				_voice->_env[_current]->currentLevel = _voice->_env[_current]->sustainLevel;
			else if (_voice->_env[_current]->decayRate >= 1270)
				_voice->_env[_current]->currentLevel = _voice->_env[_current]->totalLevel;
			else {
				_voice->_env[_current]->currentLevel = _voice->_env[_current]->totalLevel;
				_voice->_env[_current]->currentLevel -= ((_voice->_env[_current]->totalLevel -
					_voice->_env[_current]->sustainLevel) * _voice->_env[_current]->tickCount++ * 1000) /
						(_voice->_env[_current]->decayRate * _voice->_env[_current]->rate);
			}

			if (_voice->_env[_current]->currentLevel <= _voice->_env[_current]->sustainLevel) {
				_voice->_env[_current]->currentLevel = _voice->_env[_current]->sustainLevel;
				_voice->_env[_current]->state = _s_sustaining;
				_voice->_env[_current]->tickCount = 0;
			}
			break;

			case _s_sustaining:
				if (_voice->_env[_current]->sustainRate == 0)
					_voice->_env[_current]->currentLevel = 0;
				else if (_voice->_env[_current]->sustainRate >= 2540)
					_voice->_env[_current]->currentLevel = _voice->_env[_current]->sustainLevel;
				else {
					_voice->_env[_current]->currentLevel = _voice->_env[_current]->sustainLevel;
					_voice->_env[_current]->currentLevel -= (_voice->_env[_current]->sustainLevel *
						_voice->_env[_current]->tickCount++ * 1000) / (_voice->_env[_current]->sustainRate *
							_voice->_env[_current]->rate);
				}

				if (_voice->_env[_current]->currentLevel <= 0) {
					_voice->_env[_current]->currentLevel = 0;
					_voice->_env[_current]->state = _s_ready;
					_voice->_env[_current]->tickCount = 0;
				}
				break;

			case _s_releasing:
				if (_voice->_env[_current]->releaseRate == 0)
					_voice->_env[_current]->currentLevel = 0;
				else if (_voice->_env[_current]->releaseRate >= 1270)
					_voice->_env[_current]->currentLevel = _voice->_env[_current]->releaseLevel;
				else {
					_voice->_env[_current]->currentLevel = _voice->_env[_current]->releaseLevel;
					_voice->_env[_current]->currentLevel -= (_voice->_env[_current]->releaseLevel *
						_voice->_env[_current]->tickCount++ * 1000) / (_voice->_env[_current]->releaseRate *
							_voice->_env[_current]->rate);
				}

				if (_voice->_env[_current]->currentLevel <= 0) {
					_voice->_env[_current]->currentLevel = 0;
					_voice->_env[_current]->state = _s_ready;
				}
				break;

			default:
			break;
	}
}

void MidiChannel_EuD_WAVE::rate(uint16 r) {
	_rate = r;
}

void MidiChannel_EuD_WAVE::velocity(int velo) {
	_velocity = velo;
}

FMT_EuphonyDriver::FMT_EuphonyDriver(Audio::Mixer *mixer)
: MidiDriver_Emulated(mixer) {

	_volume = 255;
	_fadestate = 300;

	MidiDriver_YM2612::createLookupTables();

	for (uint8 i = 0; i < 6; i++)
		_channel[i] = _fChannel[i] = new MidiChannel_EuD_FM;
	for (uint8 i = 0; i < 8; i++)
		_channel[i + 6] = _wChannel[i] = new MidiChannel_EuD_WAVE;
	_channel[14] = _channel[15] = 0;

	_fmInstruments = _waveInstruments = 0;
	memset(_waveSounds, 0, sizeof(uint8*) * 10);

	rate(getRate());
	fading(0);
}

FMT_EuphonyDriver::~FMT_EuphonyDriver() {
	for (int i = 0; i < 6; i++)
		delete _fChannel[i];
	for (int i = 0; i < 8; i++)
		delete _wChannel[i];

	MidiDriver_YM2612::removeLookupTables();

	if (_fmInstruments) {
		delete [] _fmInstruments;
		_fmInstruments = 0;
	}

	if (_waveInstruments) {
		delete [] _waveInstruments;
		_waveInstruments = 0;
	}

	for (int i = 0; i < 10; i++) {
		if (_waveSounds[i]) {
			delete [] _waveSounds[i];
			_waveSounds[i] = 0;
		}
	}
}

int FMT_EuphonyDriver::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	MidiDriver_Emulated::open();

	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_mixerSoundHandle,
		this, -1, Audio::Mixer::kMaxChannelVolume, 0, false, true);
	return 0;
}

void FMT_EuphonyDriver::close() {
	if (!_isOpen)
		return;
	_isOpen = false;
	_mixer->stopHandle(_mixerSoundHandle);
}

void FMT_EuphonyDriver::send(uint32 b) {
	send(b & 0xF, b & 0xFFFFFFF0);
}

void FMT_EuphonyDriver::send(byte chan, uint32 b) {
	//byte param3 = (byte) ((b >> 24) & 0xFF);

	byte param2 = (byte) ((b >> 16) & 0xFF);
	byte param1 = (byte) ((b >>  8) & 0xFF);
	byte cmd    = (byte) (b & 0xF0);
	if (chan > ARRAYSIZE(_channel))
		return;

	switch (cmd) {
	case 0x80:// Note Off
		if (_channel[chan])
			_channel[chan]->noteOff(param1);
		break;
	case 0x90: // Note On
		if (_channel[chan])
			_channel[chan]->noteOn(param1, param2);
		break;
	case 0xA0: // Aftertouch
		break; // Not supported.
	case 0xB0: // Control Change
		if (param1 == 0x79) {
			fading(0);
			for (int i = 0; i < 15; i++) {
				if (_channel[i]) {
					_channel[i]->controlChange(param1, param2);
					_channel[i]->programChange(0);
				}
			}
		} else if (param1 == 0x7B) {
			for (int i = 0; i < 15; i++) {
				if (_channel[i])
					_channel[i]->controlChange(param1, param2);
			}
		} else {
			if (_channel[chan])
				_channel[chan]->controlChange(param1, param2);
		}
		break;
	case 0xC0: // Program Change
        for (int i = 0; i < 6; i++) {
			if (_channel[chan] == _fChannel[i]) {
				_channel[chan]->sysEx_customInstrument(0, _fmInstruments + param1 * 0x30);
				break;
			}
		}
		for (int i = 0; i < 8; i++) {
			if (_channel[chan] == _wChannel[i]) {
				_channel[chan]->sysEx_customInstrument(0, _waveInstruments + param1 * 0x80);
				_channel[chan]->sysEx_customInstrument(0x80, (const byte*) _waveSounds);
				break;
			}
		}
		break;
	case 0xD0: // Channel Pressure
		break; // Not supported.
	case 0xE0: // Pitch Bend
		if (_channel[chan])
			_channel[chan]->pitchBend((param1 | (param2 << 7)) - 0x2000);
		break;
	default:
		warning("FMT_EuphonyDriver: Unknown send() command 0x%02X", cmd);
	}
}

void FMT_EuphonyDriver::loadFmInstruments(const byte *instr) {
	if (_fmInstruments)
		delete [] _fmInstruments;
	_fmInstruments = new uint8[0x1800];
	memcpy(_fmInstruments, instr, 0x1800);
}

void FMT_EuphonyDriver::loadWaveInstruments(const byte *instr) {
	if (_waveInstruments)
		delete [] _waveInstruments;
	_waveInstruments = new uint8[0x1000];
	memcpy(_waveInstruments, instr, 0x1000);

	const uint8 *pos = (const uint8 *)(instr + 0x1000);

	for (uint8 i = 0; i < 10; i++) {
		if (_waveSounds[i])
			delete [] _waveSounds[i];
		uint32 numsamples = READ_LE_UINT32(pos + 0x0C);
		_waveSounds[i] = new int8[numsamples + 0x20];
        memcpy(_waveSounds[i], pos, 0x20);
		pos += 0x20;
		for (uint32 ii = 0; ii < numsamples; ii++) {
			uint8 s = *(pos + ii);
			s = (s < 0x80) ? 0x80 - s : s;
			_waveSounds[i][ii + 0x20] = s ^ 0x80;
		}
		pos += numsamples;
	}
}


void FMT_EuphonyDriver::assignFmChannel(uint8 midiChannelNumber, uint8 fmChannelNumber) {
	_channel[midiChannelNumber] = _fChannel[fmChannelNumber];
}

void FMT_EuphonyDriver::assignWaveChannel(uint8 midiChannelNumber, uint8 waveChannelNumber) {
	_channel[midiChannelNumber] = _wChannel[waveChannelNumber];
}

void FMT_EuphonyDriver::removeChannel(uint8 midiChannelNumber) {
	_channel[midiChannelNumber] = 0;
}

void FMT_EuphonyDriver::generateSamples(int16 *data, int len) {
	memset(data, 0, 2 * sizeof(int16) * len);
	nextTick(data, len);
}

void FMT_EuphonyDriver::nextTick(int16 *buf1, int buflen) {
	int32 *buf0 = (int32 *)buf1;

	for (int i = 0; i < ARRAYSIZE(_channel); i++) {
		if (_channel[i])
			_channel[i]->nextTick(buf0, buflen);
	}

	for (int i = 0; i < buflen; ++i) {
		int s = int( float(buf0[i] * volume()) * float((float)_fadestate / 300) );
		buf1[i*2] = buf1[i*2+1] = (s >> 9) & 0xffff;
	}

	if (_fading) {
		if (_fadestate)
			_fadestate--;
		else
			_fading = false;
	}
}

void FMT_EuphonyDriver::rate(uint16 r) {
	for (uint8 i = 0; i < 16; i++) {
		if (_channel[i])
			_channel[i]->rate(r);
	}
}

void FMT_EuphonyDriver::fading(bool status) {
	_fading = status;
	if (!_fading)
		_fadestate = 300;
}

MidiParser_EuD::MidiParser_EuD() : MidiParser(),
	_firstBaseTickStep(0x33), _nextBaseTickStep(0x33) {
		_initialTempo = calculateTempo(0x5a);
}

void MidiParser_EuD::parseNextEvent(EventInfo &info) {
	byte *pos = _position._play_pos;

	while (true) {
		byte cmd = *pos;
		byte evt = (cmd & 0xF0);

		if (evt == 0x90) {
			byte chan = pos[1];

			if (_enable[chan]) {
				uint16 tick = (pos[2] | ((uint16) pos[3] << 7)) + _baseTick;
				info.start = pos + 6;
				uint32 last = _position._last_event_tick;
				info.delta = (tick < last) ? 0 : (tick - last);

				info.event = 0x90 | _channel[chan];
				info.length = pos[7] | (pos[8] << 4) | (pos[9] << 8) | (pos[10] << 12);

				int8 note = (int8) pos[4];
				if (_adjNote[chan]) {
					note = (note & 0x7f) & _adjNote[chan];
					if (note > 0x7c)
						note -= 0x0c;
					else if (note < 0)
						note += 0x0c;
				}
				info.basic.param1 = (byte) note;

				uint8 onVelo = (pos[5] & 0x7f) + _adjVelo[chan];
				if (onVelo > 0x7f)
					onVelo = 0x7f;
				if (onVelo < 1)
					onVelo = 1;
				info.basic.param2 = onVelo;

				pos += 12;
				break;
			} else {
				pos += 6;
			}
		} else if (evt == 0xB0 || evt == 0xC0 || evt == 0xe0) {
			byte chan = pos[1];

			if (_enable[chan]) {
				info.start = pos;
				uint16 tick = (pos[2] | ((uint16) pos[3] << 7)) + _baseTick;
				uint32 last = _position._last_event_tick;
				info.delta = (tick < last) ? 0 : (tick - last);
				info.event = evt | _channel[chan];
				info.length = 0;
				info.basic.param1 = pos[4];
				info.basic.param2 = pos[5];
				pos += 6;
				break;
			} else {
				pos += 6;
			}
		} else if (cmd == 0xF2) {
			static uint16 tickTable [] = { 0x180, 0xC0, 0x80, 0x60, 0x40, 0x30, 0x20, 0x18 };
			_baseTick += tickTable[_nextBaseTickStep >> 4] * ((_nextBaseTickStep & 0x0f) + 1);
			_nextBaseTickStep = pos[1];
			pos += 6;
		} else if (cmd == 0xF8) {
			int32 tempo = calculateTempo(pos[4] | (pos[5] << 7));
			info.event = 0xff;
			info.length = 3;
			info.ext.type = 0x51;
			_tempo[0] = (tempo >> 16) & 0xff;
			_tempo[1] = (tempo >> 8) & 0xff;
			_tempo[2] = tempo & 0xff;
			info.ext.data = (byte*) _tempo;
			pos += 6;
			break;
		} else if (cmd == 0xFD || cmd == 0xFE) {
			// End of track.
			if (_autoLoop)
				pos = info.start = _tracks[0];
			else
				info.start = pos;

			uint32 last = _position._last_event_tick;
			uint16 tick = (pos[2] | ((uint16) pos[3] << 7)) + _baseTick;
			info.delta = (tick < last) ? 0 : (tick - last);
			info.event = 0xFF;
			info.ext.type = 0x2F;
			info.ext.data = pos;
			break;
		} else {
			error("Unknown Euphony music event 0x%02X", (int)cmd);
			memset(&info, 0, sizeof(info));
			pos = 0;
			break;
		}
	}
	_position._play_pos = pos;
}

bool MidiParser_EuD::loadMusic(byte *data, uint32) {
	unloadMusic();

	_enable = data + 0x354;
	_mode = data + 0x374;
	_channel = data + 0x394;
	_adjVelo = data + 0x3B4;
	_adjNote = (int8*) data + 0x3D4;

	_firstBaseTickStep = data[0x804];
	_initialTempo = calculateTempo((data[0x805] > 0xfc) ? 0x5a : data[0x805]);

	_num_tracks = 1;
	_ppqn = 120;
	_tracks[0] = data + 0x806;

	resetTracking();
	setTrack (0);

	return true;
}

int32 MidiParser_EuD::calculateTempo(int16 val) {
	int32 tempo = val;

	if (tempo < 0)
		tempo = 0;
	if (tempo > 0x1F4)
		tempo = 0x1F4;

	tempo = 0x4C4B4 / (tempo + 0x1E);
	while (tempo < 0x451)
		tempo <<= 1;
	tempo <<= 8;

	return tempo;
}

void MidiParser_EuD::resetTracking() {
	MidiParser::resetTracking();
	_nextBaseTickStep = _firstBaseTickStep;
	_baseTick = 0;
	setTempo(_initialTempo);
}

SoundTowns::SoundTowns(KyraEngine *vm, Audio::Mixer *mixer) : Sound(vm, mixer), _lastTrack(-1),
	 _currentSFX(0), _sfxFileData(0), _sfxFileIndex((uint)-1), _sfxWDTable(0), _parser(0), _musicTrackData(0) {

	_driver = new FMT_EuphonyDriver(_mixer);
	int ret = open();
	if (ret != MERR_ALREADY_OPEN && ret != 0) {
		error("couldn't open midi driver");
	}
}

SoundTowns::~SoundTowns() {
	AudioCD.stop();
	haltTrack();
	delete [] _sfxFileData;

	Common::StackLock lock(_mutex);
	_driver->setTimerCallback(0, 0);
	close();

	if (_musicTrackData)
		delete [] _musicTrackData;

	_driver = 0;
}

bool SoundTowns::init() {
	_vm->checkCD();
	int unused = 0;
	_sfxWDTable = _vm->staticres()->loadRawData(kKyra1TownsSFXTable, unused);

	return loadInstruments();
}

void SoundTowns::process() {
	AudioCD.updateCD();
}

namespace {

struct CDTrackTable {
	uint32 fileOffset;
	bool loop;
	int track;
};

} // end of anonymous namespace

void SoundTowns::playTrack(uint8 track) {
	if (track < 2)
		return;
	track -= 2;

	static const CDTrackTable tTable[] = {
		{ 0x04000, 1,  0 },
		{ 0x05480, 1,  6 },
		{ 0x05E70, 0,  1 },
		{ 0x06D90, 1,  3 },
		{ 0x072C0, 0, -1 },
		{ 0x075F0, 1, -1 },
		{ 0x07880, 1, -1 },
		{ 0x089C0, 0, -1 },
		{ 0x09080, 0, -1 },
		{ 0x091D0, 1,  4 },
		{ 0x0A880, 1,  5 },
		{ 0x0AF50, 0, -1 },
		{ 0x0B1A0, 1, -1 },
		{ 0x0B870, 0, -1 },
		{ 0x0BCF0, 1, -1 },
		{ 0x0C5D0, 1,  7 },
		{ 0x0D3E0, 1,  8 },
		{ 0x0e7b0, 1,  2 },
		{ 0x0edc0, 0, -1 },
		{ 0x0eef0, 1,  9 },
		{ 0x10540, 1, 10 },
		{ 0x10d80, 0, -1 },
		{ 0x10E30, 0, -1 },
		{ 0x10FC0, 0, -1 },
		{ 0x11310, 1, -1 },
		{ 0x11A20, 1, -1 },
		{ 0x12380, 0, -1 },
		{ 0x12540, 1, -1 },
		{ 0x12730, 1, -1 },
		{ 0x12A90, 1, 11 },
		{ 0x134D0, 0, -1 },
		{ 0x00000, 0, -1 },
		{ 0x13770, 0, -1 },
		{ 0x00000, 0, -1 },
		{ 0x00000, 0, -1 },
		{ 0x00000, 0, -1 },
		{ 0x00000, 0, -1 },
		{ 0x14710, 1, 12 },
		{ 0x15DF0, 1, 13 },
		{ 0x16030, 1, 14 },
		{ 0x17030, 0, -1 },
		{ 0x17650, 0, -1 },
		{ 0x134D0, 0, -1 },
		{ 0x178E0, 1, -1 },
		{ 0x18200, 0, -1 },
		{ 0x18320, 0, -1 },
		{ 0x184A0, 0, -1 },
		{ 0x18BB0, 0, -1 },
		{ 0x19040, 0, 19 },
		{ 0x19B50, 0, 20 },
		{ 0x17650, 0, -1 },
		{ 0x1A730, 1, 21 },
		{ 0x00000, 0, -1 },
		{ 0x12380, 0, -1 },
		{ 0x1B810, 0, -1 },
		{ 0x1BA50, 0, 15 },
		{ 0x1C190, 0, 16 },
		{ 0x1CA50, 0, 17 },
		{ 0x1D100, 0, 18 },
	};

	int trackNum = tTable[track].track;
	bool loop = tTable[track].loop;

	if (track == _lastTrack && _musicEnabled)
		return;

	haltTrack();

	if (_musicEnabled == 2 && trackNum != -1) {
		AudioCD.play(trackNum+1, loop ? -1 : 1, 0, 0);
		AudioCD.updateCD();
	} else if (_musicEnabled) {
		playEuphonyTrack(tTable[track].fileOffset, loop);
	}

	_lastTrack = track;
}

void SoundTowns::haltTrack() {
	_lastTrack = -1;
	AudioCD.stop();
	AudioCD.updateCD();
	if (_parser) {
		Common::StackLock lock(_mutex);

		_parser->setTrack(0);
		_parser->jumpToTick(0);

		_parser->unloadMusic();
		delete _parser;
		_parser = 0;

		setVolume(255);
	}
}

void SoundTowns::loadSoundFile(uint file) {
	if (_sfxFileIndex == file)
		return;
	_sfxFileIndex = file;
	delete [] _sfxFileData;
	_sfxFileData = _vm->resource()->fileData(soundFilename(file), 0);
}

void SoundTowns::playSoundEffect(uint8 track) {
	if (!_sfxEnabled || !_sfxFileData)
		return;

	if (track == 0 || track == 10) {
		_mixer->stopHandle(_sfxHandle);
		return;
	} else if (track == 1) {
		// sfx fadeout
		_mixer->stopHandle(_sfxHandle);
		return;
	}

	uint8 note = 0x3c;
	if (_sfxFileIndex == 5) {
		if (track == 0x10) {
			note = 0x3e;
			track = 0x0f;
		} else if (track == 0x11) {
			note = 0x40;
			track = 0x0f;
		} else if (track == 0x12) {
			note = 0x41;
			track = 0x0f;
		}
	}

	uint8 * fileBody = _sfxFileData + 0x01b8;
	int32 offset = (int32)READ_LE_UINT32(_sfxFileData + (track - 0x0b) * 4);
	if (offset == -1)
		return;

	struct SfxHeader {
		uint32 id;
		uint32 inBufferSize;
		uint32 unused1;
		uint32 outBufferSize;
		uint32 unused2;
		uint32 unused3;
		uint32 rate;
		uint32 rootNoteOffs;
	} *sfxHeader = (SfxHeader*)(fileBody + offset);

	uint32 sfxHeaderID = TO_LE_32(sfxHeader->id);
	uint32 sfxHeaderInBufferSize = TO_LE_32(sfxHeader->inBufferSize);
	uint32 sfxHeaderOutBufferSize = TO_LE_32(sfxHeader->outBufferSize);
	uint32 sfxRootNoteOffs = TO_LE_32(sfxHeader->rootNoteOffs);
	uint32 sfxRate = TO_LE_32(sfxHeader->rate);

	uint32 playbackBufferSize = (sfxHeaderID == 1) ? sfxHeaderInBufferSize : sfxHeaderOutBufferSize;

	uint8 *sfxPlaybackBuffer = (uint8 *)malloc(playbackBufferSize);
	memset(sfxPlaybackBuffer, 0x80, playbackBufferSize);

	uint8 *sfxBody = ((uint8 *)sfxHeader) + 0x20;

	if (!sfxHeaderID) {
		memcpy(sfxPlaybackBuffer, sfxBody, playbackBufferSize);
	} else if (sfxHeaderID == 1) {
		Screen::decodeFrame4(sfxBody, sfxPlaybackBuffer, playbackBufferSize);
	} else if (_sfxWDTable) {
		uint8 * tgt = sfxPlaybackBuffer;
		uint32 sfx_BtTable_Offset = 0;
		uint32 sfx_WdTable_Offset = 0;
		uint32 sfx_WdTable_Number = 5;

		for (uint32 i = 0; i < sfxHeaderInBufferSize; i++) {
			sfx_WdTable_Offset = (sfx_WdTable_Number * 3 << 9) + sfxBody[i] * 6;
			sfx_WdTable_Number = READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset);

			sfx_BtTable_Offset += (int16)READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset + 2);
			*tgt++ = _sfxBTTable[((sfx_BtTable_Offset >> 2) & 0xff)];

			sfx_BtTable_Offset += (int16)READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset + 4);
			*tgt++ = _sfxBTTable[((sfx_BtTable_Offset >> 2) & 0xff)];
		}
	}

	for (uint32 i = 0; i < playbackBufferSize; i++) {
		if (sfxPlaybackBuffer[i] < 0x80)
			sfxPlaybackBuffer[i] = 0x80 - sfxPlaybackBuffer[i];
	}

	playbackBufferSize -= 0x20;
	uint32 outputRate = uint32(11025 * semitoneAndSampleRate_to_sampleStep(note, sfxRootNoteOffs, sfxRate, 11025, 0x2000));

	_currentSFX = Audio::makeLinearInputStream(sfxPlaybackBuffer, playbackBufferSize,
		outputRate, Audio::Mixer::FLAG_UNSIGNED | Audio::Mixer::FLAG_LITTLE_ENDIAN | Audio::Mixer::FLAG_AUTOFREE, 0, 0);
	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, _currentSFX);
}

void SoundTowns::beginFadeOut() {
	_lastTrack = -1;
	_driver->fading();

	// TODO: this should fade out too
	AudioCD.stop();
	AudioCD.updateCD();
}

int SoundTowns::open() {
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;

	_driver->setTimerCallback(this, &onTimer);
	return 0;
}

void SoundTowns::close() {
	if (_driver)
		_driver->close();
}

void SoundTowns::send(uint32 b) {
	_driver->send(b);
}

uint32 SoundTowns::getBaseTempo(void) {
	return _driver ? _driver->getBaseTempo() : 0;
}

bool SoundTowns::loadInstruments() {
	if (!_musicTrackData)
		_musicTrackData = new uint8[0xC58A];

	memset(_musicTrackData, 0, 0xC58A);
	uint8 * twm = _vm->resource()->fileData("twmusic.pak", 0);
	if (!twm)
		return false;
	Screen::decodeFrame4(twm, _musicTrackData, 0x8BF0);
	_driver->loadFmInstruments(_musicTrackData + 8);

	memset (_musicTrackData, 0, 0xC58A);
	Screen::decodeFrame4(twm + 0x0CA0, _musicTrackData, 0xC58A);
	delete [] twm;
	_driver->loadWaveInstruments(_musicTrackData + 8);

	return true;
}

void SoundTowns::playEuphonyTrack(uint32 offset, int loop) {
	if (!_musicTrackData)
		_musicTrackData = new uint8[0xC58A];

	memset(_musicTrackData, 0, 0xC58A);
	uint8 * twm = _vm->resource()->fileData("twmusic.pak", 0);
	Screen::decodeFrame4(twm + 0x4b70 + offset, _musicTrackData, 0xC58A);
	delete [] twm;

	Common::StackLock lock(_mutex);

	uint8 * used = _musicTrackData + 0x374;
	uint8 * fchan = _musicTrackData + 0x6d4;
	uint8 * wchan = _musicTrackData + 0x6dA;

	for (uint8 i = 0; i < 6; i++) {
		if (used[fchan[i]])
			_driver->assignFmChannel(fchan[i], i);
	}

	for (uint8 i = 0; i < 8; i++) {
		if (used[wchan[i]])
			_driver->assignWaveChannel(wchan[i], i);
	}

	for (uint8 i = 0; i < 16; i++) {
		if (!used[i])
			_driver->removeChannel(i);
	}
	_driver->send(0x79B0);

	if (_parser)
		delete _parser;

	_parser = new MidiParser_EuD;
	_parser->property(MidiParser::mpAutoLoop, loop);
	_parser->loadMusic(_musicTrackData, 0);
	_parser->jumpToTick(0);

	_parser->setMidiDriver(this);
	_parser->setTimerRate(getBaseTempo());
}

void SoundTowns::onTimer(void * data) {
	SoundTowns *music = (SoundTowns *)data;
	Common::StackLock lock(music->_mutex);
	if (music->_parser)
		music->_parser->onTimer();
}

float SoundTowns::semitoneAndSampleRate_to_sampleStep(int8 semiTone, int8 semiToneRootkey,
	uint32 sampleRate, uint32 outputRate, int32 pitchWheel) {
	if (semiTone < 0)
		semiTone = 0;
	if (semiTone > 119)
		semiTone = 119;
	if (semiTone < 0)
		semiTone = 0;
	if (semiTone > 119)
		semiTone = 119;

	static const float noteFrq[] = {
		0004.13f, 0004.40f, 0004.64f, 0004.95f, 0005.16f, 0005.50f, 0005.80f, 0006.19f, 0006.60f, 0006.86f,
		0007.43f, 0007.73f, 0008.25f, 0008.80f, 0009.28f, 0009.90f, 0010.31f, 0011.00f, 0011.60f, 0012.38f,
		0013.20f, 0013.75f, 0014.85f, 0015.47f,	0016.50f, 0017.60f, 0018.56f, 0019.80f, 0020.63f, 0022.00f,
		0023.21f, 0024.75f, 0026.40f, 0027.50f, 0029.70f, 0030.94f, 0033.00f, 0035.20f, 0037.16f, 0039.60f,
		0041.25f, 0044.00f, 0046.41f, 0049.50f, 0052.80f, 0055.00f, 0059.40f, 0061.88f, 0066.00f, 0070.40f,
		0074.25f, 0079.20f, 0082.50f, 0088.00f, 0092.83f, 0099.00f, 0105.60f, 0110.00f, 0118.80f, 0123.75f,
		0132.00f, 0140.80f, 0148.50f, 0158.40f, 0165.00f, 0176.00f, 0185.65f, 0198.00f, 0211.20f, 0220.00f,
		0237.60f, 0247.50f, 0264.00f, 0281.60f, 0297.00f, 0316.80f, 0330.00f, 0352.00f, 0371.30f, 0396.00f,
		0422.40f, 0440.00f, 0475.20f, 0495.00f,	0528.00f, 0563.20f, 0594.00f, 0633.60f, 0660.00f, 0704.00f,
		0742.60f, 0792.00f, 0844.80f, 0880.00f, 0950.40f, 0990.00f, 1056.00f, 1126.40f, 1188.00f, 1267.20f,
		1320.00f, 1408.00f, 1485.20f, 1584.00f, 1689.60f, 1760.00f, 1900.80f, 1980.00f, 2112.00f, 2252.80f,
		2376.00f, 2534.40f, 2640.00f, 2816.00f, 2970.40f, 3168.00f, 3379.20f, 3520.00f, 3801.60f, 3960.00f
	};

	float pwModifier = (pitchWheel - 0x2000) / 0x2000;
	int8 d = pwModifier ? (pwModifier < 0 ? -1 : 1) : 0;
	float rateshift = (noteFrq[semiTone] - ((noteFrq[semiTone] -
		noteFrq[semiTone + d]) * pwModifier * d)) / noteFrq[semiToneRootkey];

	return (float) sampleRate * 10.0f * rateshift / outputRate;
}

const uint8 SoundTowns::_sfxBTTable[256] = {
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFE,
	0x7F, 0x7F, 0x7E, 0x7D, 0x7C, 0x7B, 0x7A, 0x79, 0x78, 0x77, 0x76, 0x75, 0x74, 0x73, 0x72, 0x71,
	0x70, 0x6F, 0x6E, 0x6D, 0x6C, 0x6B, 0x6A, 0x69, 0x68, 0x67, 0x66, 0x65, 0x64, 0x63, 0x62, 0x61,
	0x60, 0x5F, 0x5E, 0x5D, 0x5C, 0x5B, 0x5A, 0x59, 0x58, 0x57, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51,
	0x50, 0x4F, 0x4E, 0x4D, 0x4C, 0x4B, 0x4A, 0x49, 0x48, 0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0x41,
	0x40, 0x3F, 0x3E, 0x3D, 0x3C, 0x3B, 0x3A, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31,
	0x30, 0x2F, 0x2E, 0x2D, 0x2C, 0x2B, 0x2A, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21,
	0x20, 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11,
	0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01
};

} // end of namespace Kyra

