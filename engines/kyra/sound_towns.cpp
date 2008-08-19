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

#define		EUPHONY_FADEOUT_TICKS		600

namespace Kyra {

enum EnvelopeState { s_ready, s_attacking, s_decaying, s_sustaining, s_releasing };

class Towns_EuphonyChannel : public MidiChannel {
public:
	Towns_EuphonyChannel() {}
	~Towns_EuphonyChannel() {}

	virtual void nextTick(int32 *outbuf, int buflen) = 0;
	virtual void rate(uint16 r) = 0;

protected:
	uint16 _rate;
};

class Towns_EuphonyFmChannel : public Towns_EuphonyChannel {
public:
	Towns_EuphonyFmChannel();
	virtual ~Towns_EuphonyFmChannel();

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

class Towns_EuphonyPcmChannel : public Towns_EuphonyChannel {
public:
	void nextTick(int32 *outbuf, int buflen);
	void rate(uint16 r);

	Towns_EuphonyPcmChannel();
	virtual ~Towns_EuphonyPcmChannel();

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
		} *_snd[8];
		struct Env {
			EnvelopeState state;
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
		} *_env[8];
	} *_voice;
};

class Towns_EuphonyTrackQueue {
public:
	Towns_EuphonyTrackQueue(Towns_EuphonyDriver *driver, Towns_EuphonyTrackQueue *last);
	~Towns_EuphonyTrackQueue() {}

	Towns_EuphonyTrackQueue *release();
	void initDriver();
	void loadDataToCurrentPosition(uint8 *trackdata, uint32 size, bool loop = 0);
	void loadDataToEndOfQueue(uint8 *trackdata, uint32 size, bool loop = 0);
	void setPlayBackStatus(bool playing);
	bool isPlaying() {return _playing; }
	uint8 *trackData() {return _trackData; }

	bool _loop;
	Towns_EuphonyTrackQueue *_next;

private:
	uint8 *_trackData;
	uint8 *_used;
	uint8 *_fchan;
	uint8 *_wchan;
	bool _playing;
	Towns_EuphonyDriver *_driver;
	Towns_EuphonyTrackQueue *_last;
};

class Towns_EuphonyParser : public MidiParser {
public:
	Towns_EuphonyParser(Towns_EuphonyTrackQueue * queue);
	bool loadMusic (byte *data, uint32 size);
	int32 calculateTempo(int16 val);

protected:
	void parseNextEvent (EventInfo &info);
	void resetTracking();
	void setup();

	byte *_enable;
	byte *_mode;
	byte *_channel;
	byte *_adjVelo;
	int8 *_adjNote;

	uint8 _firstBaseTickStep;
	uint8 _nextBaseTickStep;
	uint32 _initialTempo;
	uint32 _baseTick;

	byte _tempo[3];
	Towns_EuphonyTrackQueue *_queue;
};

class Towns_EuphonyDriver : public MidiDriver_Emulated {
public:
	Towns_EuphonyDriver(Audio::Mixer *mixer);
	virtual ~Towns_EuphonyDriver();

	int open();
	void close();
	void send(uint32 b);
	void send(byte channel, uint32 b);
	uint32 property(int prop, uint32 param) { return 0; }

	void setPitchBendRange(byte channel, uint range) { }
	void loadFmInstruments(const byte *instr);
	void loadWaveInstruments(const byte *instr);

	Towns_EuphonyTrackQueue *queue() { return _queue; }

	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }

	void assignFmChannel(uint8 midiChannelNumber, uint8 fmChannelNumber);
	void assignWaveChannel(uint8 midiChannelNumber, uint8 waveChannelNumber);
	void removeChannel(uint8 midiChannelNumber);

	void setVolume(int val = -1) { if (val >= 0) _volume = val; }
	int getVolume(int val = -1) { return _volume; }

	// AudioStream API
	bool isStereo() const { return true; }
	int getRate() const { return _mixer->getOutputRate(); }

	void fading(bool status = true);

protected:
	void nextTick(int16 *buf1, int buflen);
	void rate(uint16 r);

	void generateSamples(int16 *buf, int len);

	Towns_EuphonyFmChannel *_fChannel[6];
	Towns_EuphonyPcmChannel *_wChannel[8];
	Towns_EuphonyChannel *_channel[16];
	Towns_EuphonyTrackQueue *_queue;

	int _volume;
	bool _fading;
	int16 _fadestate;

	uint8 *_fmInstruments;
	uint8 *_waveInstruments;
	int8 * _waveSounds[10];
};

Towns_EuphonyFmChannel::Towns_EuphonyFmChannel() {
	_voice = new Voice2612;
}

Towns_EuphonyFmChannel::~Towns_EuphonyFmChannel() {
	delete _voice;
}

void Towns_EuphonyFmChannel::noteOn(byte note, byte onVelo) {
	_voice->noteOn(note, onVelo);
}

void Towns_EuphonyFmChannel::noteOff(byte note) {
	_voice->noteOff(note);
}

void Towns_EuphonyFmChannel::controlChange(byte control, byte value) {
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

void Towns_EuphonyFmChannel::sysEx_customInstrument(uint32, const byte *fmInst) {
	_voice->_rate = _rate;
	_voice->setInstrument(fmInst);
}

void Towns_EuphonyFmChannel::pitchBend(int16 value) {
	_voice->pitchBend(value);
}

void Towns_EuphonyFmChannel::nextTick(int32 *outbuf, int buflen) {
	_voice->nextTick((int*) outbuf, buflen);
}

void Towns_EuphonyFmChannel::rate(uint16 r) {
	_rate = r;
	_voice->_rate = r;
}

Towns_EuphonyPcmChannel::Towns_EuphonyPcmChannel() {
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

Towns_EuphonyPcmChannel::~Towns_EuphonyPcmChannel() {
	for (uint8 i = 0; i < 8; i++) {
		if (_voice->_snd[i])
			delete _voice->_snd[i];
		delete _voice->_env[i];
	}
	delete _voice;
}

void Towns_EuphonyPcmChannel::noteOn(byte note, byte onVelo) {
	_note = note;
	velocity(onVelo);
	_phase = 0;

	for (_current = 0; _current < 7; _current++) {
		if (note <=	_voice->split[_current])
			break;
	}

	_voice->_env[_current]->state = s_attacking;
	_voice->_env[_current]->currentLevel = 0;
	_voice->_env[_current]->rate = _rate;
	_voice->_env[_current]->tickCount = 0;
}

void Towns_EuphonyPcmChannel::noteOff(byte note) {
    if (_current == -1)
		return;
	if (_voice->_env[_current]->state == s_ready)
		return;

	_voice->_env[_current]->state = s_releasing;
	_voice->_env[_current]->releaseLevel = _voice->_env[_current]->currentLevel;
	_voice->_env[_current]->tickCount = 0;
}

void Towns_EuphonyPcmChannel::controlChange(byte control, byte value) {
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

void Towns_EuphonyPcmChannel::sysEx_customInstrument(uint32 type, const byte *fmInst) {
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
			_voice->_env[i]->state = s_ready;
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

void Towns_EuphonyPcmChannel::pitchBend(int16 value) {
	_frequencyOffs = value;
}

void Towns_EuphonyPcmChannel::nextTick(int32 *outbuf, int buflen) {
	if (_current == -1 || !_voice->_snd[_current] || !_voice->_env[_current]->state || !_velocity) {
		velocity(0);
		_current = -1;
		return;
	}

	float phaseStep = SoundTowns::calculatePhaseStep(_note, _voice->_snd[_current]->keyNote -
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

void Towns_EuphonyPcmChannel::evpNextTick() {
	switch (_voice->_env[_current]->state) {
		case s_ready:
			_voice->_env[_current]->currentLevel = 0;
			return;

		case s_attacking:
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
				_voice->_env[_current]->state = s_decaying;
				_voice->_env[_current]->tickCount = 0;
			}
			break;

		case s_decaying:
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
				_voice->_env[_current]->state = s_sustaining;
				_voice->_env[_current]->tickCount = 0;
			}
			break;

			case s_sustaining:
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
					_voice->_env[_current]->state = s_ready;
					_voice->_env[_current]->tickCount = 0;
				}
				break;

			case s_releasing:
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
					_voice->_env[_current]->state = s_ready;
				}
				break;

			default:
			break;
	}
}

void Towns_EuphonyPcmChannel::rate(uint16 r) {
	_rate = r;
}

void Towns_EuphonyPcmChannel::velocity(int velo) {
	_velocity = velo;
}

Towns_EuphonyDriver::Towns_EuphonyDriver(Audio::Mixer *mixer)
	: MidiDriver_Emulated(mixer) {
	_volume = 255;
	_fadestate = EUPHONY_FADEOUT_TICKS;
	_queue = 0;

	MidiDriver_YM2612::createLookupTables();

	for (uint8 i = 0; i < 6; i++)
		_channel[i] = _fChannel[i] = new Towns_EuphonyFmChannel;
	for (uint8 i = 0; i < 8; i++)
		_channel[i + 6] = _wChannel[i] = new Towns_EuphonyPcmChannel;
	_channel[14] = _channel[15] = 0;

	_fmInstruments = _waveInstruments = 0;
	memset(_waveSounds, 0, sizeof(uint8*) * 10);

	rate(getRate());
	fading(0);

	_queue = new Towns_EuphonyTrackQueue(this, 0);
}

Towns_EuphonyDriver::~Towns_EuphonyDriver() {
	for (int i = 0; i < 6; i++)
		delete _fChannel[i];
	for (int i = 0; i < 8; i++)
		delete _wChannel[i];

	MidiDriver_YM2612::removeLookupTables();

	if (_fmInstruments) {
		delete[] _fmInstruments;
		_fmInstruments = 0;
	}

	if (_waveInstruments) {
		delete[] _waveInstruments;
		_waveInstruments = 0;
	}

	for (int i = 0; i < 10; i++) {
		if (_waveSounds[i]) {
			delete[] _waveSounds[i];
			_waveSounds[i] = 0;
		}
	}

	if (_queue) {
		_queue->release();
		delete _queue;
		_queue = 0;
	}
}

int Towns_EuphonyDriver::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;
	MidiDriver_Emulated::open();

	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_mixerSoundHandle,
		this, -1, Audio::Mixer::kMaxChannelVolume, 0, false, true);

	return 0;
}

void Towns_EuphonyDriver::close() {
	if (!_isOpen)
		return;
	_isOpen = false;
	_mixer->stopHandle(_mixerSoundHandle);
}

void Towns_EuphonyDriver::send(uint32 b) {
	send(b & 0xF, b & 0xFFFFFFF0);
}

void Towns_EuphonyDriver::send(byte chan, uint32 b) {
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
		warning("Towns_EuphonyDriver: Unknown send() command 0x%02X", cmd);
	}
}

void Towns_EuphonyDriver::loadFmInstruments(const byte *instr) {
	if (_fmInstruments)
		delete[] _fmInstruments;
	_fmInstruments = new uint8[0x1800];
	memcpy(_fmInstruments, instr, 0x1800);
}

void Towns_EuphonyDriver::loadWaveInstruments(const byte *instr) {
	if (_waveInstruments)
		delete[] _waveInstruments;
	_waveInstruments = new uint8[0x1000];
	memcpy(_waveInstruments, instr, 0x1000);

	const uint8 *pos = (const uint8 *)(instr + 0x1000);

	for (uint8 i = 0; i < 10; i++) {
		if (_waveSounds[i])
			delete[] _waveSounds[i];
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


void Towns_EuphonyDriver::assignFmChannel(uint8 midiChannelNumber, uint8 fmChannelNumber) {
	_channel[midiChannelNumber] = _fChannel[fmChannelNumber];
}

void Towns_EuphonyDriver::assignWaveChannel(uint8 midiChannelNumber, uint8 waveChannelNumber) {
	_channel[midiChannelNumber] = _wChannel[waveChannelNumber];
}

void Towns_EuphonyDriver::removeChannel(uint8 midiChannelNumber) {
	_channel[midiChannelNumber] = 0;
}

void Towns_EuphonyDriver::generateSamples(int16 *data, int len) {
	memset(data, 0, 2 * sizeof(int16) * len);
	nextTick(data, len);
}

void Towns_EuphonyDriver::nextTick(int16 *buf1, int buflen) {
	int32 *buf0 = (int32 *)buf1;

	for (int i = 0; i < ARRAYSIZE(_channel); i++) {
		if (_channel[i])
			_channel[i]->nextTick(buf0, buflen);
	}

	for (int i = 0; i < buflen; ++i) {
		int s = int( float(buf0[i] * _volume) * float((float)_fadestate / EUPHONY_FADEOUT_TICKS) );
		buf1[i*2] = buf1[i*2+1] = (s >> 9) & 0xffff;
	}

	if (_fading) {
		if (_fadestate) {
			_fadestate--;
		} else {
			_fading = false;
			_queue->setPlayBackStatus(false);
		}
	}
}

void Towns_EuphonyDriver::rate(uint16 r) {
	for (uint8 i = 0; i < 16; i++) {
		if (_channel[i])
			_channel[i]->rate(r);
	}
}

void Towns_EuphonyDriver::fading(bool status) {
	_fading = status;
	if (!_fading)
		_fadestate = EUPHONY_FADEOUT_TICKS;
}

Towns_EuphonyParser::Towns_EuphonyParser(Towns_EuphonyTrackQueue * queue) : MidiParser(),
	_firstBaseTickStep(0x33), _nextBaseTickStep(0x33) {
		_initialTempo = calculateTempo(0x5a);
		_queue = queue;
}

void Towns_EuphonyParser::parseNextEvent(EventInfo &info) {
	byte *pos = _position._play_pos;

	if (_queue->_next) {
		if (info.ext.type == 0x2F) {
			unloadMusic();
			memset(&info, 0, sizeof(EventInfo));
			pos = _position._play_pos = _tracks[0] = _queue->trackData() + 0x806;
		} else if (_active_track == 255) {
			_queue = _queue->_next;
			setup();
			setTrack(0);
			_queue->setPlayBackStatus(true);
			return;
		} else if (!_queue->isPlaying()) {
			unloadMusic();
			_queue = _queue->_next;
			setup();
			setTrack(0);
			_queue->setPlayBackStatus(true);
			return;
		}
	}

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
			static const uint16 tickTable[] = { 0x180, 0xC0, 0x80, 0x60, 0x40, 0x30, 0x20, 0x18 };
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
			if (_autoLoop) {
				unloadMusic();
				_queue->setPlayBackStatus(true);
				pos = info.start = _tracks[0];
			} else {
				info.start = pos;
			}

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

bool Towns_EuphonyParser::loadMusic(byte *data, uint32 size) {
	bool loop = _autoLoop;

	if (_queue->isPlaying() && !_queue->_loop) {
		_queue->loadDataToEndOfQueue(data, size, loop);
	} else {
		unloadMusic();
		_queue = _queue->release();
		_queue->loadDataToCurrentPosition(data, size, loop);
		setup();
		setTrack(0);
		_queue->setPlayBackStatus(true);
	}
	return true;
}

int32 Towns_EuphonyParser::calculateTempo(int16 val) {
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

void Towns_EuphonyParser::resetTracking() {
	MidiParser::resetTracking();

	_nextBaseTickStep = _firstBaseTickStep;
	_baseTick = 0;
	setTempo(_initialTempo);
	_queue->setPlayBackStatus(false);
}

void Towns_EuphonyParser::setup() {
	uint8 *data = _queue->trackData();
	if (!data)
		return;
	_queue->initDriver();

	_enable = data + 0x354;
	_mode = data + 0x374;
	_channel = data + 0x394;
	_adjVelo = data + 0x3B4;
	_adjNote = (int8*) data + 0x3D4;

	_nextBaseTickStep = _firstBaseTickStep = data[0x804];
	_initialTempo = calculateTempo((data[0x805] > 0xfc) ? 0x5a : data[0x805]);

	property(MidiParser::mpAutoLoop, _queue->_loop);

	_num_tracks = 1;
	_ppqn = 120;
	_tracks[0] = data + 0x806;
}

Towns_EuphonyTrackQueue::Towns_EuphonyTrackQueue(Towns_EuphonyDriver * driver, Towns_EuphonyTrackQueue * last) {
	_trackData = 0;
	_next = 0;
	_driver = driver;
	_last = last;
	_used = _fchan = _wchan = 0;
	_playing = false;
}

void Towns_EuphonyTrackQueue::setPlayBackStatus(bool playing) {
	Towns_EuphonyTrackQueue * i = this;
	do {
		i->_playing = playing;
		i = i->_next;
	} while (i);
}

void Towns_EuphonyTrackQueue::loadDataToCurrentPosition(uint8 * trackdata, uint32 size, bool loop) {
	if (_trackData)
		delete[] _trackData;
	_trackData = new uint8[0xC58A];
	memset(_trackData, 0, 0xC58A);
	Screen::decodeFrame4(trackdata, _trackData, size);

	_used = _trackData + 0x374;
	_fchan = _trackData + 0x6d4;
	_wchan = _trackData + 0x6dA;
	_loop = loop;
	_playing = false;
}

void Towns_EuphonyTrackQueue::loadDataToEndOfQueue(uint8 * trackdata, uint32 size, bool loop) {
	if (!_trackData) {
		loadDataToCurrentPosition(trackdata, size, loop);
		return;
	}

	Towns_EuphonyTrackQueue * i = this;
	while (i->_next)
		i = i->_next;

	i = i->_next = new Towns_EuphonyTrackQueue(_driver, i);
	i->_trackData = new uint8[0xC58A];
	memset(i->_trackData, 0, 0xC58A);
	Screen::decodeFrame4(trackdata, i->_trackData, size);

	i->_used = i->_trackData + 0x374;
	i->_fchan = i->_trackData + 0x6d4;
	i->_wchan = i->_trackData + 0x6dA;
	i->_loop = loop;
	i->_playing = _playing;
}

Towns_EuphonyTrackQueue *Towns_EuphonyTrackQueue::release() {
	Towns_EuphonyTrackQueue *i = this;
	while (i->_next)
		i = i->_next;

	Towns_EuphonyTrackQueue *res = i;

	while (i) {
		i->_playing = false;
		i->_used = i->_fchan = i->_wchan = 0;
		if (i->_trackData) {
			delete[] i->_trackData;
			i->_trackData = 0;
		}
		i = i->_last;
		if (i) {
			res = i;
			if (i->_next) {
				delete i->_next;
				i->_next = 0;
			}
		}
	}

	if (res->_trackData) {
		delete[] res->_trackData;
		res->_trackData = 0;
	}

	return res;
}

void Towns_EuphonyTrackQueue::initDriver() {
	for (uint8 i = 0; i < 6; i++) {
		if (_used[_fchan[i]])
			_driver->assignFmChannel(_fchan[i], i);
	}

	for (uint8 i = 0; i < 8; i++) {
		if (_used[_wchan[i]])
			_driver->assignWaveChannel(_wchan[i], i);
	}

	for (uint8 i = 0; i < 16; i++) {
		if (!_used[i])
			_driver->removeChannel(i);
	}
	_driver->send(0x79B0);
}

class TownsPC98_OpnOperator {
public:
	TownsPC98_OpnOperator(const float rate, const uint8 *rateTable,
		const uint8 *shiftTable, const uint8 *attackDecayTable, const uint32 *frqTable,
		const uint32 *sineTable, const int32 *tlevelOut, const int32 *detuneTable);
	~TownsPC98_OpnOperator() {}

	void keyOn();
	void keyOff();
	void frequency(int freq);
	void updatePhaseIncrement();
	void recalculateRates();
	void generateOutput(int32 phasebuf, int32 *feedbuf, int32 &out);

	void feedbackLevel(int32 level) {_feedbackLevel = level ? level + 6 : 0; }
	void detune(int value) { _detn = &_detnTbl[value << 5]; }
	void multiple(uint32 value) { _multiple = value ? (value << 1) : 1;	}
	void attackRate(uint32 value) { _specifiedAttackRate = value; }
	bool scaleRate(uint8 value);
	void decayRate(uint32 value) { _specifiedDecayRate = value;	recalculateRates();	}
	void sustainRate(uint32 value) { _specifiedSustainRate = value;	recalculateRates();	}
	void sustainLevel(uint32 value) { _sustainLevel = (value == 0x0f) ? 0x3e0 : value << 5; }
	void releaseRate(uint32 value) { _specifiedReleaseRate = value;	recalculateRates();	}
	void totalLevel(uint32 value) { _totalLevel = value << 3; }
	void reset();

protected:
	EnvelopeState _state;
	uint32 _feedbackLevel;
	uint32 _multiple;
	uint32 _totalLevel;
	uint8 _keyScale1;
	uint8 _keyScale2;
	uint32 _specifiedAttackRate;
	uint32 _specifiedDecayRate;
	uint32 _specifiedSustainRate;
	uint32 _specifiedReleaseRate;
	uint32 _tickCount;
	uint32 _sustainLevel;

	uint32 _frequency;
	uint8 _kcode;
	uint32 _phase;
	uint32 _phaseIncrement;
	const int32 *_detn;

	const uint8 *_rateTbl;
	const uint8 *_rshiftTbl;
	const uint8 *_adTbl;
	const uint32 *_fTbl;
	const uint32 *_sinTbl;
	const int32 *_tLvlTbl;
	const int32 *_detnTbl;

	const int _tickLength;
	int _tick;
	int32 _currentLevel;

	struct EvpState {
		uint8 rate;
		uint8 shift;
	} fs_a, fs_d, fs_s, fs_r;
};

TownsPC98_OpnOperator::TownsPC98_OpnOperator(const float rate, const uint8 *rateTable,
	const uint8 *shiftTable, const uint8 *attackDecayTable,	const uint32 *frqTable,
	const uint32 *sineTable, const int32 *tlevelOut, const int32 *detuneTable) :
	_rateTbl(rateTable), _rshiftTbl(shiftTable), _adTbl(attackDecayTable), _fTbl(frqTable),
	_sinTbl(sineTable), _tLvlTbl(tlevelOut), _detnTbl(detuneTable), _tickLength((int)(rate * 65536.0f)),
	_specifiedAttackRate(0), _specifiedDecayRate(0), _specifiedReleaseRate(0), _specifiedSustainRate(0),
	_phase(0), _state(s_ready) {

	reset();
}

void TownsPC98_OpnOperator::keyOn() {
	_state = s_attacking;
	_phase = 0;
}

void TownsPC98_OpnOperator::keyOff() {
	if (_state != s_ready)
		_state = s_releasing;
}

void TownsPC98_OpnOperator::frequency(int freq) {
	uint8 block = (freq >> 11);
	uint16 pos = (freq & 0x7ff);
	uint8 c = pos >> 7;

	_kcode = (block << 2) | ((c < 7) ? 0 : ((c > 8) ? 3 : c - 6 ));
	_frequency = _fTbl[pos << 1] >> (7 - block);
}

void TownsPC98_OpnOperator::updatePhaseIncrement() {
	_phaseIncrement = ((_frequency + _detn[_kcode]) * _multiple) >> 1;
	uint8 keyscale = _kcode >> _keyScale1;
	if (_keyScale2 != keyscale) {
		_keyScale2 = keyscale;
		recalculateRates();
	}
}

void TownsPC98_OpnOperator::recalculateRates() {
	int k = _keyScale2;
	int r = _specifiedAttackRate ? (_specifiedAttackRate << 1) + 0x20 : 0;
	fs_a.rate = ((r + k) < 94) ? _rateTbl[r + k] : 136;
	fs_a.shift = ((r + k) < 94) ? _rshiftTbl[r + k] : 0;

	r = _specifiedDecayRate ? (_specifiedDecayRate << 1) + 0x20 : 0;
	fs_d.rate = _rateTbl[r + k];
	fs_d.shift = _rshiftTbl[r + k];

	r = _specifiedSustainRate ? (_specifiedSustainRate << 1) + 0x20 : 0;
	fs_s.rate = _rateTbl[r + k];
	fs_s.shift = _rshiftTbl[r + k];

	r = (_specifiedReleaseRate << 2) + 0x22;
	fs_r.rate = _rateTbl[r + k];
	fs_r.shift = _rshiftTbl[r + k];
}

void TownsPC98_OpnOperator::generateOutput(int32 phasebuf, int32 *feed, int32 &out) {
	if (_state == s_ready)
		return;

	_tick += _tickLength;
	while (_tick > 0x30000) {
		_tick -= 0x30000;
		++_tickCount;

		int32 levelIncrement = 0;
		uint32 targetTime = 0;
		int32 targetLevel = 0;
		EnvelopeState nextState = s_ready;

		switch (_state) {
			case s_ready:
				return;
			case s_attacking:
				nextState = s_decaying;
				targetTime = (1 << fs_a.shift) - 1;
				targetLevel = 0;
				levelIncrement = (~_currentLevel * _adTbl[fs_a.rate + ((_tickCount >> fs_a.shift) & 7)]) >> 4;
				break;
			case s_decaying:
				targetTime = (1 << fs_d.shift) - 1;
				nextState = s_sustaining;
				targetLevel = _sustainLevel;
				levelIncrement = _adTbl[fs_d.rate + ((_tickCount >> fs_d.shift) & 7)];
				break;
			case s_sustaining:
				targetTime = (1 << fs_s.shift) - 1;
				nextState = s_sustaining;
				targetLevel = 1023;
				levelIncrement = _adTbl[fs_s.rate + ((_tickCount >> fs_s.shift) & 7)];
				break;
			case s_releasing:
				targetTime = (1 << fs_r.shift) - 1;
				nextState = s_ready;
				targetLevel = 1023;
				levelIncrement = _adTbl[fs_r.rate + ((_tickCount >> fs_r.shift) & 7)];
				break;
		}

		if (!(_tickCount & targetTime)) {
			_currentLevel += levelIncrement;
			if ((_state == s_attacking && _currentLevel <= targetLevel) || (_state != s_attacking && _currentLevel >= targetLevel)) {
				if (_state != s_decaying)
					_currentLevel = targetLevel;
				_state = nextState;
			}
		}
	}

	uint32 lvlout = _totalLevel + (uint32) _currentLevel;

	int outp = 0;
	int *i = &outp, *o = &outp;
	int phaseShift = 0;

	if (feed) {
		o = &feed[0];
		i = &feed[1];
		phaseShift = _feedbackLevel ? ((*o + *i) << _feedbackLevel) : 0;
		if (phasebuf == -1)
			*i = 0;
		*o = *i;
	} else {
		phaseShift = phasebuf << 15;
	}

	if (lvlout < 832) {
		uint32 index = (lvlout << 3) + _sinTbl[(((int32)((_phase & 0xffff0000)
			+ phaseShift)) >> 16) & 0x3ff];
		*i = ((index < 6656) ? _tLvlTbl[index] : 0);
	} else {
		*i = 0;
	}

	_phase += _phaseIncrement;
	out += *o;
}

void TownsPC98_OpnOperator::reset(){
	keyOff();
	_tick = 0;
	_keyScale2 = 0;
	_currentLevel = 1023;

	frequency(0);
	detune(0);
	scaleRate(0);
	multiple(0);
	updatePhaseIncrement();
	attackRate(0);
	decayRate(0);
	releaseRate(0);
	sustainRate(0);
	feedbackLevel(0);
	totalLevel(127);
}

bool TownsPC98_OpnOperator::scaleRate(uint8 value) {
	value = 3 - value;
	if (_keyScale1 != value) {
		_keyScale1 = value;
		return true;
	}

	int k = _keyScale2;
	int r = _specifiedAttackRate ? (_specifiedAttackRate << 1) + 0x20 : 0;
	fs_a.rate = ((r + k) < 94) ? _rateTbl[r + k] : 136;
	fs_a.shift = ((r + k) < 94) ? _rshiftTbl[r + k] : 0;
	return false;
}

class TownsPC98_OpnDriver;
class TownsPC98_OpnChannel {
public:
	TownsPC98_OpnChannel(TownsPC98_OpnDriver *driver, uint8 regOffs, uint8 flgs, uint8 num,
		uint8 key, uint8 prt, uint8 id);
	virtual ~TownsPC98_OpnChannel();
	virtual void init();

	typedef enum channelState {
		CHS_RECALCFREQ		=	0x01,
		CHS_KEYOFF			=	0x02,
		CHS_SSGOFF			=	0x04,
		CHS_PITCHWHEELOFF	=	0x08,
		CHS_ALL_BUT_EOT		=	0x0f,
		CHS_PROTECT			=	0x40,
		CHS_EOT				=	0x80
	} ChannelState;

	virtual void loadData(uint8 *data);
	virtual void processEvents();
	virtual void processFrequency();
	virtual bool processControlEvent(uint8 cmd);
	void writeReg(uint8 regAddress, uint8 value);

	virtual void keyOn();
	void keyOff();

	void setOutputLevel();
	virtual void fadeStep();
	virtual void reset();

	void updateEnv();
	void generateOutput(int32 &leftSample, int32 &rightSample, int32 *del, int32 *feed);

	bool _enableLeft;
	bool _enableRight;
	bool _updateEnvelopeParameters;
	const uint8 _idFlag;
	int _feedbuf[3];

protected:
	void setupPitchWheel();
	bool processPitchWheel();

	bool control_dummy(uint8 para);
	bool control_f0_setPatch(uint8 para);
	bool control_f1_presetOutputLevel(uint8 para);
	bool control_f2_setKeyOffTime(uint8 para);
	bool control_f3_setFreqLSB(uint8 para);
	bool control_f4_setOutputLevel(uint8 para);
	bool control_f5_setTempo(uint8 para);
	bool control_f6_repeatSection(uint8 para);
	bool control_f7_setupPitchWheel(uint8 para);
	bool control_f8_togglePitchWheel(uint8 para);
	bool control_fa_writeReg(uint8 para);
	virtual bool control_fb_incOutLevel(uint8 para);
	virtual bool control_fc_decOutLevel(uint8 para);
	bool control_fd_jump(uint8 para);
	virtual bool control_ff_endOfTrack(uint8 para);

	uint8 _ticksLeft;
	uint8 _algorithm;
	uint8 _instr;
	uint8 _totalLevel;
	uint8 _frqBlockMSB;
	int8 _frqLSB;
	uint8 _keyOffTime;
	bool _hold;
	uint8 *_dataPtr;
	uint8 _ptchWhlInitDelayHi;
	uint8 _ptchWhlInitDelayLo;
	int16 _ptchWhlModInitVal;
	uint8 _ptchWhlDuration;
	uint8 _ptchWhlCurDelay;
	int16 _ptchWhlModCurVal;
	uint8 _ptchWhlDurLeft;
	uint16 _frequency;
	uint8 _block;
	uint8 _regOffset;
	uint8 _flags;
	uint8 _ssgTl;
	uint8 _ssgStep;
	uint8 _ssgTicksLeft;
	uint8 _ssgTargetLvl;
	uint8 _ssgStartLvl;

	const uint8 _chanNum;
	const uint8 _keyNum;
	const uint8 _part;

	TownsPC98_OpnDriver *_drv;
	TownsPC98_OpnOperator **_opr;
	uint16 _frqTemp;

	typedef bool (TownsPC98_OpnChannel::*ControlEventFunc)(uint8 para);
	const ControlEventFunc *controlEvents;
};

class TownsPC98_OpnChannelSSG : public TownsPC98_OpnChannel {
public:
	TownsPC98_OpnChannelSSG(TownsPC98_OpnDriver *driver, uint8 regOffs,
		uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id);
	virtual ~TownsPC98_OpnChannelSSG() {}
	void init();

	virtual void loadData(uint8 *data);
	void processEvents();
	void processFrequency();
	bool processControlEvent(uint8 cmd);

	void keyOn();
	void nextShape();

	void protect();
	void restore();

	void fadeStep();

protected:
	void setOutputLevel(uint8 lvl);

	bool control_f0_setInstr(uint8 para);
	bool control_f1_setTotalLevel(uint8 para);
	bool control_f4_setAlgorithm(uint8 para);
	bool control_f9_loadCustomPatch(uint8 para);
	bool control_fb_incOutLevel(uint8 para);
	bool control_fc_decOutLevel(uint8 para);
	bool control_ff_endOfTrack(uint8 para);

	typedef bool (TownsPC98_OpnChannelSSG::*ControlEventFunc)(uint8 para);
	const ControlEventFunc *controlEvents;
};

class TownsPC98_OpnSfxChannel : public TownsPC98_OpnChannelSSG {
public:
	TownsPC98_OpnSfxChannel(TownsPC98_OpnDriver *driver, uint8 regOffs,
		uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id) :
		TownsPC98_OpnChannelSSG(driver, regOffs, flgs, num, key, prt, id) {}
	~TownsPC98_OpnSfxChannel() {}

	void loadData(uint8 *data);
};

class TownsPC98_OpnChannelPCM : public TownsPC98_OpnChannel {
public:
	TownsPC98_OpnChannelPCM(TownsPC98_OpnDriver *driver, uint8 regOffs,
		uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id);
	~TownsPC98_OpnChannelPCM() {}
	void init();

	void loadData(uint8 *data);
	void processEvents();
	bool processControlEvent(uint8 cmd);

	void reset();

private:
	bool control_f1_pcmStart(uint8 para);
	bool control_ff_endOfTrack(uint8 para);

	typedef bool (TownsPC98_OpnChannelPCM::*ControlEventFunc)(uint8 para);
	const ControlEventFunc *controlEvents;
};

class TownsPC98_OpnSquareSineSource {
public:
	TownsPC98_OpnSquareSineSource(const float rate);
	~TownsPC98_OpnSquareSineSource();

	void init(const int *rsTable, const int *rseTable);
	void reset();
	uint8 readReg(uint8 address);
	void writeReg(uint8 address, uint8 value, bool force = false);

	void nextTick(int32 *buffer, uint32 bufferSize);

private:
	void updatesRegs();

	uint8 _reg[16];
	uint8 _updateRequestBuf[32];
	int _updateRequest;
	uint8 *_regIndex;
	int _rand;

	int8 _evpTimer;
	uint32 _pReslt;
	uint8 _attack;

	bool _evpUpdate, _cont;

	int _evpUpdateCnt;
	uint8 _outN;
	int _nTick;

	int32 *_tlTable;
	int32 *_tleTable;

	const float _rate;
	const int _tickLength;
	int _timer;

	struct Channel {
		int tick;
		uint8 smp;
		uint8 out;
	} _channels[3];

	bool _ready;
};

class TownsPC98_OpnPercussionSource {
public:
	TownsPC98_OpnPercussionSource(const float rate);
	~TownsPC98_OpnPercussionSource();

	void init(const void *pcmData = 0);
	void reset();
	void writeReg(uint8 address, uint8 value);

	void nextTick(int32 *buffer, uint32 bufferSize);

private:
	struct PcmInstrument {
		const uint8 *data;

		const uint8 *start;
		const uint8 *end;
		const uint8 *pos;
		uint32 size;
		bool active;
		uint8 level;

		int8 decState;
		uint8 decStep;

		int16 samples[2];
		int out;
	};

	void recalcOuput(PcmInstrument *ins);
	void advanceInput(PcmInstrument *ins);

	PcmInstrument _pcmInstr[6];
	uint8 _regs[48];

	uint8 _totalLevel;

	const int _tickLength;
	int _timer;
	bool _ready;
};

class TownsPC98_OpnDriver : public Audio::AudioStream {
friend class TownsPC98_OpnChannel;
friend class TownsPC98_OpnChannelSSG;
friend class TownsPC98_OpnSfxChannel;
friend class TownsPC98_OpnChannelPCM;
public:
	enum OpnType {
		OD_TOWNS,
		OD_TYPE26,
		OD_TYPE86
	};

	TownsPC98_OpnDriver(Audio::Mixer *mixer, OpnType type);
	~TownsPC98_OpnDriver();

	bool init();

	void loadMusicData(uint8 *data, bool loadPaused = false);
	void loadSoundEffectData(uint8 *data, uint8 trackNum);
	void reset();
	void fadeStep();

	void pause() { _musicPlaying = false; }
	void cont() { _musicPlaying = true; }

	void callback();
	void nextTick(int32 *buffer, uint32 bufferSize);

	bool looping() { return _looping == _updateChannelsFlag ? true : false; }
	bool musicPlaying() { return _musicPlaying; }

	// AudioStream interface
	int inline readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return true; }
	bool endOfData() const { return false; }
	int getRate() const { return _mixer->getOutputRate(); }

protected:
	void generateTables();

	TownsPC98_OpnChannel **_channels;
	TownsPC98_OpnChannelSSG **_ssgChannels;
	TownsPC98_OpnSfxChannel **_sfxChannels;
	TownsPC98_OpnChannelPCM *_pcmChannel;

	TownsPC98_OpnSquareSineSource *_ssg;
	TownsPC98_OpnPercussionSource *_pcm;

	void startSoundEffect();
	void setTempo(uint8 tempo);

	void lock() { _mutex.lock(); }
	void unlock() { _mutex.unlock(); }

	Audio::Mixer *_mixer;
	Common::Mutex _mutex;
	Audio::SoundHandle _soundHandle;

	const uint8 *_opnCarrier;
	const uint8 *_opnFreqTable;
	const uint8 *_opnFreqTableSSG;
	const uint8 *_opnFxCmdLen;
	const uint8 *_opnLvlPresets;

	uint8 *_oprRates;
	uint8 *_oprRateshift;
	uint8 *_oprAttackDecay;
	uint32 *_oprFrq;
	uint32 *_oprSinTbl;
	int32 *_oprLevelOut;
	int32 *_oprDetune;

	uint8 *_musicBuffer;
	uint8 *_sfxBuffer;
	uint8 *_trackPtr;
	uint8 *_patches;
	uint8 *_ssgPatches;

	uint8 _cbCounter;
	uint8 _updateChannelsFlag;
	uint8 _updateSSGFlag;
	uint8 _updatePCMFlag;
	uint8 _finishedChannelsFlag;
	uint8 _finishedSSGFlag;
	uint8 _finishedPCMFlag;

	uint16 _tempo;
	bool _musicPlaying;
	bool _sfxPlaying;
	uint8 _fading;
	uint8 _looping;
	uint32 _musicTickCounter;

	bool _updateEnvelopeParameters;

	bool _regProtectionFlag;
	int _sfxOffs;
	uint8 *_sfxData;
	uint16 _sfxOffsets[2];

	int32 _samplesTillCallback;
	int32 _samplesTillCallbackRemainder;
	int32 _samplesPerCallback;
	int32 _samplesPerCallbackRemainder;

	const int _numChan;
	const int _numSSG;
	const bool _hasPCM;
	const bool _hasStereo;

	static const uint8 _drvTables[];
	static const uint32 _adtStat[];
	static const int _ssgTables[];

	const float _baserate;
	bool _ready;
};

TownsPC98_OpnChannel::TownsPC98_OpnChannel(TownsPC98_OpnDriver *driver, uint8 regOffs, uint8 flgs, uint8 num,
	uint8 key, uint8 prt, uint8 id) : _drv(driver), _regOffset(regOffs), _flags(flgs), _chanNum(num), _keyNum(key),
	_part(prt), _idFlag(id) {

	_ticksLeft = _algorithm = _instr = _totalLevel = _frqBlockMSB = _keyOffTime = 0;
	_ssgStartLvl = _ssgTl = _ssgStep = _ssgTicksLeft = _ssgTargetLvl = _block = 0;
	_ptchWhlInitDelayHi = _ptchWhlInitDelayLo = _ptchWhlDuration = _ptchWhlCurDelay = _ptchWhlDurLeft = 0;
	_frqLSB = 0;
	_hold = _updateEnvelopeParameters = false;
	_enableLeft = _enableRight = true;
	_dataPtr = 0;
	_ptchWhlModInitVal = _ptchWhlModCurVal = 0;
	_frequency = _frqTemp = 0;
	memset(&_feedbuf, 0, sizeof(int) * 3);
	_opr = 0;
}

TownsPC98_OpnChannel::~TownsPC98_OpnChannel() {
	if (_opr) {
		for (int i = 0; i < 4; i++)
			delete _opr[i];
		delete [] _opr;
	}
}

void TownsPC98_OpnChannel::init() {
	_opr = new TownsPC98_OpnOperator*[4];
	for (int i = 0; i < 4; i++)
		_opr[i] = new TownsPC98_OpnOperator(_drv->_baserate, _drv->_oprRates, _drv->_oprRateshift,
			_drv->_oprAttackDecay, _drv->_oprFrq, _drv->_oprSinTbl, _drv->_oprLevelOut, _drv->_oprDetune);

	#define Control(x)	&TownsPC98_OpnChannel::control_##x
	static const ControlEventFunc ctrlEvents[] = {
		Control(f0_setPatch),
		Control(f1_presetOutputLevel),
		Control(f2_setKeyOffTime),
		Control(f3_setFreqLSB),
		Control(f4_setOutputLevel),
		Control(f5_setTempo),
		Control(f6_repeatSection),
		Control(f7_setupPitchWheel),
		Control(f8_togglePitchWheel),
		Control(dummy),
		Control(fa_writeReg),
		Control(fb_incOutLevel),
		Control(fc_decOutLevel),
		Control(fd_jump),
		Control(dummy),
		Control(ff_endOfTrack)
	};
	#undef Control

	controlEvents = ctrlEvents;
}

void TownsPC98_OpnChannel::keyOff() {
	// all operators off
	uint8 value = _keyNum & 0x0f;
	uint8 regAddress = 0x28;
	writeReg(regAddress, value);
	_flags |= CHS_KEYOFF;
}

void TownsPC98_OpnChannel::keyOn() {
	// all operators on
	uint8 value = _keyNum | 0xf0;
	uint8 regAddress = 0x28;
	writeReg(regAddress, value);
}

void TownsPC98_OpnChannel::loadData(uint8 *data) {
	_flags = (_flags & ~CHS_EOT) | CHS_ALL_BUT_EOT;
	_ticksLeft = 1;
	_dataPtr = data;
	_totalLevel = 0x7F;

	uint8 *src_b = _dataPtr;
	int loop = 1;
	uint8 cmd = 0;
	while (loop) {
		if (loop == 1) {
			cmd = *src_b++;
			if (cmd < 0xf0) {
				src_b++;
				loop = 1;
			} else {
				if (cmd == 0xff) {
					loop = *src_b ? 2 : 0;
					if (READ_LE_UINT16(src_b))
						_drv->_looping |= _idFlag;
				} else if (cmd == 0xf6) {
					loop = 3;
				} else {
					loop = 2;
				}
			}
		} else if (loop == 2) {
			src_b += _drv->_opnFxCmdLen[cmd - 240];
			loop = 1;
		} else if (loop == 3) {
			src_b[0] = src_b[1];
			src_b += 4;
			loop = 1;
		}
	}
}

void TownsPC98_OpnChannel::processEvents() {
	if (_flags & CHS_EOT)
		return;

	if (_hold == false && _ticksLeft == _keyOffTime)
		keyOff();

	if (--_ticksLeft)
		return;

	if (_hold == false)
		keyOff();

	uint8 cmd = 0;
	bool loop = true;

	while (loop) {
		cmd = *_dataPtr++;
		if (cmd < 0xf0)
			loop = false;
		else if (!processControlEvent(cmd))
			return;
	}

	uint8 para = *_dataPtr++;

	if (cmd == 0x80) {
		keyOff();
		_hold = false;
	} else {
		keyOn();

		if (_hold == false || cmd != _frqBlockMSB)
			_flags |= CHS_RECALCFREQ;

		_hold = (para & 0x80) ? true : false;
		_frqBlockMSB = cmd;
	}

	_ticksLeft = para & 0x7f;
}

void TownsPC98_OpnChannel::processFrequency() {
	if (_flags & CHS_RECALCFREQ) {
		uint8 block = (_frqBlockMSB & 0x70) >> 1;
		uint16 bfreq = ((const uint16*)_drv->_opnFreqTable)[_frqBlockMSB & 0x0f];

		_frequency = (bfreq + _frqLSB) | (block << 8);
		writeReg(_regOffset + 0xa4, (_frequency >> 8));
		writeReg(_regOffset + 0xa0, (_frequency & 0xff));

		setupPitchWheel();
	}

	if (!(_flags & CHS_PITCHWHEELOFF)) {
		if (!processPitchWheel())
			return;

		writeReg(_regOffset + 0xa4, (_frequency >> 8));
		writeReg(_regOffset + 0xa0, (_frequency & 0xff));
	}
}

void TownsPC98_OpnChannel::setupPitchWheel() {
	_ptchWhlCurDelay = _ptchWhlInitDelayHi;
	if (_flags & CHS_KEYOFF) {
		_ptchWhlModCurVal = _ptchWhlModInitVal;
		_ptchWhlCurDelay += _ptchWhlInitDelayLo;
	}
	_ptchWhlDurLeft = (_ptchWhlDuration >> 1);
	_flags &= ~(CHS_KEYOFF | CHS_RECALCFREQ);
}

bool TownsPC98_OpnChannel::processPitchWheel() {
	if (--_ptchWhlCurDelay)
		return false;

	_ptchWhlCurDelay = _ptchWhlInitDelayHi;
	_frequency += _ptchWhlModCurVal;

	if(!--_ptchWhlDurLeft) {
		_ptchWhlDurLeft = _ptchWhlDuration;
		_ptchWhlModCurVal = -_ptchWhlModCurVal;
	}

	return true;
}

bool TownsPC98_OpnChannel::processControlEvent(uint8 cmd) {
	uint8 para = *_dataPtr++;
	return (this->*controlEvents[cmd & 0x0f])(para);
}

void TownsPC98_OpnChannel::setOutputLevel() {
	uint8 outopr = _drv->_opnCarrier[_algorithm];
	uint8 reg = 0x40 + _regOffset;

	for (int i = 0; i < 4; i++) {
		if (outopr & 1)
			writeReg(reg, _totalLevel);
		outopr >>= 1;
		reg += 4;
	}
}

void TownsPC98_OpnChannel::fadeStep() {
	_totalLevel += 3;
	if (_totalLevel > 0x7f)
		_totalLevel = 0x7f;
	setOutputLevel();
}

void TownsPC98_OpnChannel::reset() {
	if (_opr) {
		for (int i = 0; i < 4; i++)
			_opr[i]->reset();
	}

	_block = 0;
	_frequency = 0;
	_hold = false;
	_frqTemp = 0;
	_ssgTl = 0;
	_ssgStartLvl = 0;
	_ssgTargetLvl = 0;
	_ssgStep = 0;
	_ssgTicksLeft = 0;
	_totalLevel = 0;
	_flags |= CHS_EOT;

	_updateEnvelopeParameters = false;
	_enableLeft = _enableRight = true;
	memset(&_feedbuf, 0, sizeof(int) * 3);
}

void TownsPC98_OpnChannel::updateEnv() {
	for (int i = 0; i < 4 ; i++)
		_opr[i]->updatePhaseIncrement();
}

void TownsPC98_OpnChannel::generateOutput(int32 &leftSample, int32 &rightSample, int *del, int *feed) {
	int phbuf1, phbuf2, output;
	phbuf1 = phbuf2 = output = 0;

	switch (_algorithm) {
		case 0:
			_opr[0]->generateOutput(0, feed, phbuf1);
			_opr[2]->generateOutput(*del, 0, phbuf2);
			*del = 0;
			_opr[1]->generateOutput(phbuf1, 0, *del);
			_opr[3]->generateOutput(phbuf2, 0, output);
			break;
		case 1:
			_opr[0]->generateOutput(0, feed, phbuf1);
			_opr[2]->generateOutput(*del, 0, phbuf2);
			_opr[1]->generateOutput(0, 0, phbuf1);
			_opr[3]->generateOutput(phbuf2, 0, output);
			*del = phbuf1;
			break;
		case 2:
			_opr[0]->generateOutput(0, feed, phbuf2);
			_opr[2]->generateOutput(*del, 0, phbuf2);
			_opr[1]->generateOutput(0, 0, phbuf1);
			_opr[3]->generateOutput(phbuf2, 0, output);
			*del = phbuf1;
			break;
		case 3:
			_opr[0]->generateOutput(0, feed, phbuf2);
			_opr[2]->generateOutput(0, 0, *del);
			_opr[1]->generateOutput(phbuf2, 0, phbuf1);
			_opr[3]->generateOutput(*del, 0, output);
			*del = phbuf1;
			break;
		case 4:
			_opr[0]->generateOutput(0, feed, phbuf1);
			_opr[2]->generateOutput(0, 0, phbuf2);
			_opr[1]->generateOutput(phbuf1, 0, output);
			_opr[3]->generateOutput(phbuf2, 0, output);
			*del = 0;
			break;
		case 5:
			*del = feed[1];
			_opr[0]->generateOutput(-1, feed, phbuf1);
			_opr[2]->generateOutput(*del, 0, output);
			_opr[1]->generateOutput(*del, 0, output);
			_opr[3]->generateOutput(*del, 0, output);
			break;
		case 6:
			_opr[0]->generateOutput(0, feed, phbuf1);
			_opr[2]->generateOutput(0, 0, output);
			_opr[1]->generateOutput(phbuf1, 0, output);
			_opr[3]->generateOutput(0, 0, output);
			*del = 0;
			break;
		case 7:
			_opr[0]->generateOutput(0, feed, output);
			_opr[2]->generateOutput(0, 0, output);
			_opr[1]->generateOutput(0, 0, output);
			_opr[3]->generateOutput(0, 0, output);
			*del = 0;
			break;
		};

	int32 finOut = ((output * 7) / 2);

	if (_enableLeft)
		leftSample += finOut;

	if (_enableRight)
		rightSample += finOut;
}

void TownsPC98_OpnChannel::writeReg(uint8 regAddress, uint8 value) {
	if (_drv->_regProtectionFlag)
		return;

	uint8 h = regAddress & 0xf0;
	uint8 l = (regAddress & 0x0f);
	static const uint8 oprOrdr[] = { 0, 2, 1, 3 };
	uint8 o = oprOrdr[(l - _regOffset) >> 2];

	switch (h) {
		case 0x00:
			// ssg
			if (_drv->_ssg)
				_drv->_ssg->writeReg(regAddress, value);
			break;
		case 0x10:
			// pcm rhythm channel
			if (_drv->_pcm)
				_drv->_pcm->writeReg(regAddress - 0x10, value);
			break;
		case 0x20:
			if (l == 8) {
				// Key on/off
				for (int i = 0; i < 4; i++) {
					if ((value >> (4 + i)) & 1)
						_opr[i]->keyOn();
					else
						_opr[i]->keyOff();
				}
			} else if (l == 2) {
				// LFO
				warning("TownsPC98_OpnDriver: TRYING TO USE LFO (NOT SUPPORTED)");
			} else if (l == 7) {
				// Timers; Ch 3/6 special mode
				warning("TownsPC98_OpnDriver: TRYING TO USE CH 3/6 SPECIAL MODE (NOT SUPPORTED)");
			} else if (l == 4 || l == 5) {
				// Timer A
				warning("TownsPC98_OpnDriver: TRYING TO USE TIMER_A (NOT SUPPORTED)");
			} else if (l == 6) {
				// Timer B
				warning("TownsPC98_OpnDriver: TRYING TO USE TIMER_B (NOT SUPPORTED)");
			} else if (l == 10 || l == 11) {
				// DAC
				warning("TownsPC98_OpnDriver: TRYING TO USE DAC (NOT SUPPORTED)");
			}
			break;

		case 0x30:
			// detune, multiple
			_opr[o]->detune((value >> 4) & 7);
			_opr[o]->multiple(value & 0x0f);
			_updateEnvelopeParameters = true;
			break;

		case 0x40:
			// total level
			_opr[o]->totalLevel(value & 0x7f);
			break;

		case 0x50:
			// rate scaling, attack rate
			_opr[o]->attackRate(value & 0x1f);
			if (_opr[o]->scaleRate(value >> 6))
				_updateEnvelopeParameters = true;
			break;

		case 0x60:
			// first decay rate, amplitude modulation
			_opr[o]->decayRate(value & 0x1f);
			if (value & 0x80)
				warning("TownsPC98_OpnDriver: TRYING TO USE AMP MODULATION (NOT SUPPORTED)");
			break;

		case 0x70:
			// secondary decay rate
			_opr[o]->sustainRate(value & 0x1f);
			break;

		case 0x80:
			// secondary amplitude, release rate;
			_opr[o]->sustainLevel(value >> 4);
			_opr[o]->releaseRate(value & 0x0f);
			break;

		case 0x90:
			warning("TownsPC98_OpnDriver: UNKNOWN ADDRESS %d", regAddress);
			break;

		case 0xa0:
			// frequency
			l -= _regOffset;
			if (l == 0) {
				_frqTemp = (_frqTemp & 0xff00) | value;
				_updateEnvelopeParameters = true;
				for (int i = 0; i < 4; i++)
					_opr[i]->frequency(_frqTemp);
			} else if (l == 4) {
				_frqTemp = (_frqTemp & 0xff) | (value << 8);
			} else if (l == 8) {
				// Ch 3/6 special mode frq
				warning("TownsPC98_OpnDriver: TRYING TO USE CH 3/6 SPECIAL MODE FREQ (NOT SUPPORTED)");
			} else if (l == 12) {
				// Ch 3/6 special mode frq
				warning("TownsPC98_OpnDriver: TRYING TO USE CH 3/6 SPECIAL MODE FREQ (NOT SUPPORTED)");
			}
			break;

		case 0xb0:
			l -= _regOffset;
			if (l == 0) {
				// feedback, _algorithm
				_opr[0]->feedbackLevel((value >> 3) & 7);
				_opr[1]->feedbackLevel(0);
				_opr[2]->feedbackLevel(0);
				_opr[3]->feedbackLevel(0);
			} else if (l == 4) {
				// stereo, LFO sensitivity
				_enableLeft = value & 0x80 ? true : false;
				_enableRight = value & 0x40 ? true : false;
				uint8 ams = (value & 0x3F) >> 3;
				if (ams)
					warning("TownsPC98_OpnDriver: TRYING TO USE AMP MODULATION SENSITIVITY (NOT SUPPORTED)");
				uint8 fms = value & 3;
				if (fms)
					warning("TownsPC98_OpnDriver: TRYING TO USE FREQ MODULATION SENSITIVITY (NOT SUPPORTED)");
			}
			break;

		default:
			warning("TownsPC98_OpnDriver: UNKNOWN ADDRESS %d", regAddress);
			break;
	}
}

bool TownsPC98_OpnChannel::control_f0_setPatch(uint8 para) {
	_instr = para;
	uint8 reg = _regOffset + 0x80;

	for (int i = 0; i < 4; i++) {
		// set release rate for each operator
		writeReg(reg, 0x0f);
		reg += 4;
	}

	const uint8 *tptr = _drv->_patches + ((uint32)_instr << 5);
	reg = _regOffset + 0x30;

	// write registers 0x30 to 0x8f
	for (int i = 0; i < 6; i++) {
		writeReg(reg, tptr[0]);
		reg += 4;
		writeReg(reg, tptr[2]);
		reg += 4;
		writeReg(reg, tptr[1]);
		reg += 4;
		writeReg(reg, tptr[3]);
		reg += 4;
		tptr += 4;
	}

	reg = _regOffset + 0xB0;
	_algorithm = tptr[0] & 7;
	// set feedback and algorithm
	writeReg(reg, tptr[0]);

	setOutputLevel();
	return true;
}

bool TownsPC98_OpnChannel::control_f1_presetOutputLevel(uint8 para) {
	if (_drv->_fading)
		return true;

	_totalLevel = _drv->_opnLvlPresets[para];
	setOutputLevel();
	return true;
}

bool TownsPC98_OpnChannel::control_f2_setKeyOffTime(uint8 para) {
	_keyOffTime = para;
	return true;
}

bool TownsPC98_OpnChannel::control_f3_setFreqLSB(uint8 para) {
	_frqLSB = (int8) para;
	return true;
}

bool TownsPC98_OpnChannel::control_f4_setOutputLevel(uint8 para) {
	if (_drv->_fading)
		return true;

	_totalLevel = para;
	setOutputLevel();
	return true;
}

bool TownsPC98_OpnChannel::control_f5_setTempo(uint8 para) {
	_drv->setTempo(para);
	return true;
}

bool TownsPC98_OpnChannel::control_f6_repeatSection(uint8 para) {
	_dataPtr--;
	_dataPtr[0]--;

	if (*_dataPtr) {
		// repeat section until counter has reached zero
		_dataPtr = _drv->_trackPtr + READ_LE_UINT16(_dataPtr + 2);
	} else {
		// reset counter, advance to next section
		_dataPtr[0] = _dataPtr[1];
		_dataPtr += 4;
	}
	return true;
}

bool TownsPC98_OpnChannel::control_f7_setupPitchWheel(uint8 para) {
	_ptchWhlInitDelayHi = _dataPtr[0];
	_ptchWhlInitDelayLo = para;
	_ptchWhlModInitVal = (int16) READ_LE_UINT16(_dataPtr + 1);
	_ptchWhlDuration = _dataPtr[3];
	_dataPtr += 4;
	_flags = (_flags & ~CHS_PITCHWHEELOFF) | CHS_KEYOFF | CHS_RECALCFREQ;
	return true;
}

bool TownsPC98_OpnChannel::control_f8_togglePitchWheel(uint8 para) {
	if (para == 0x10) {
		if (*_dataPtr++) {
			_flags = (_flags & ~CHS_PITCHWHEELOFF) | CHS_KEYOFF;
		} else {
			_flags |= CHS_PITCHWHEELOFF;
		}
	} else {
		/* NOT IMPLEMENTED
		uint8 skipChannels = para / 36;
		uint8 entry = para % 36;
		TownsPC98_OpnDriver::TownsPC98_OpnChannel *t = &chan[skipChannels];
		
		t->unnamedEntries[entry] = *_dataPtr++;*/
	}
	return true;
}

bool TownsPC98_OpnChannel::control_fa_writeReg(uint8 para) {
	writeReg(para, *_dataPtr++);
	return true;
}

bool TownsPC98_OpnChannel::control_fb_incOutLevel(uint8 para) {
	_dataPtr--;
	if (_drv->_fading)
		return true;

	uint8 val = (_totalLevel + 3);
	if (val > 0x7f)
		val = 0x7f;

	_totalLevel = val;
	setOutputLevel();
	return true;
}

bool TownsPC98_OpnChannel::control_fc_decOutLevel(uint8 para) {
	_dataPtr--;
	if (_drv->_fading)
		return true;

	int8 val = (int8) (_totalLevel - 3);
	if (val < 0)
		val = 0;

	_totalLevel = (uint8) val;
	setOutputLevel();
	return true;
}

bool TownsPC98_OpnChannel::control_fd_jump(uint8 para) {
	uint8 *tmp = _drv->_trackPtr + READ_LE_UINT16(_dataPtr - 1);
	_dataPtr = (tmp[1] == 1) ? tmp : ++_dataPtr;
	return true;
}

bool TownsPC98_OpnChannel::control_dummy(uint8 para) {
	_dataPtr--;
	return true;
}

bool TownsPC98_OpnChannel::control_ff_endOfTrack(uint8 para) {
	uint16 val = READ_LE_UINT16(--_dataPtr);
	if (val) {
		// loop
		_dataPtr = _drv->_trackPtr + val;
		return true;
	} else {
		// quit parsing for active channel
		--_dataPtr;
		_flags |= CHS_EOT;
		_drv->_finishedChannelsFlag |= _idFlag;
		keyOff();
		return false;
	}
}

TownsPC98_OpnChannelSSG::TownsPC98_OpnChannelSSG(TownsPC98_OpnDriver *driver, uint8 regOffs,
		uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id) :
		TownsPC98_OpnChannel(driver, regOffs, flgs, num, key, prt, id) {
}

void TownsPC98_OpnChannelSSG::init() {
	_algorithm = 0x80;

	#define Control(x)	&TownsPC98_OpnChannelSSG::control_##x
	static const ControlEventFunc ctrlEventsSSG[] = {
		Control(f0_setInstr),
		Control(f1_setTotalLevel),
		Control(f2_setKeyOffTime),
		Control(f3_setFreqLSB),
		Control(f4_setAlgorithm),
		Control(f5_setTempo),
		Control(f6_repeatSection),
		Control(f7_setupPitchWheel),
		Control(f8_togglePitchWheel),
		Control(f9_loadCustomPatch),
		Control(fa_writeReg),
		Control(fb_incOutLevel),
		Control(fc_decOutLevel),
		Control(fd_jump),
		Control(dummy),
		Control(ff_endOfTrack)
	};
	#undef Control

	controlEvents = ctrlEventsSSG;
}

void TownsPC98_OpnChannelSSG::processEvents() {
	if (_flags & CHS_EOT)
		return;

	_drv->_regProtectionFlag = (_flags & CHS_PROTECT) ? true : false;

	if (!_hold && _ticksLeft == _keyOffTime)
		nextShape();

	if (!--_ticksLeft) {

		uint8 cmd = 0;
		bool loop = true;

		while (loop) {
			cmd = *_dataPtr++;
			if (cmd < 0xf0)
				loop = false;
			else if (!processControlEvent(cmd))
				return;
		}

		uint8 para = *_dataPtr++;

		if (cmd == 0x80) {
			nextShape();
			_hold = false;
		} else {
			if (!_hold) {
				_instr &= 0xf0;
				_ssgStep = _drv->_ssgPatches[_instr];
				_ssgTicksLeft = _drv->_ssgPatches[_instr + 1] & 0x7f;
				_ssgTargetLvl = _drv->_ssgPatches[_instr + 2];
				_ssgStartLvl = _drv->_ssgPatches[_instr + 3];
				_flags = (_flags & ~CHS_SSGOFF) | CHS_KEYOFF;
			}

			keyOn();

			if (_hold == false || cmd != _frqBlockMSB)
				_flags |= CHS_RECALCFREQ;

			_hold = (para & 0x80) ? true : false;
			_frqBlockMSB = cmd;
		}

		_ticksLeft = para & 0x7f;
	}

	if (!(_flags & CHS_SSGOFF)) {
		if (--_ssgTicksLeft) {
			if (!_drv->_fading)
				setOutputLevel(_ssgStartLvl);
			return;
		}

		_ssgTicksLeft = _drv->_ssgPatches[_instr + 1] & 0x7f;

		if (_drv->_ssgPatches[_instr + 1] & 0x80) {
			uint8 t = _ssgStartLvl - _ssgStep;

			if (_ssgStep <= _ssgStartLvl && _ssgTargetLvl < t) {
				if (!_drv->_fading)
					setOutputLevel(t);
				return;
			}
		} else {
			int t = _ssgStartLvl + _ssgStep;
			uint8 p = (uint8) (t & 0xff);

			if (t < 256 && _ssgTargetLvl > p) {
				if (!_drv->_fading)
					setOutputLevel(p);
				return;
			}
		}

		setOutputLevel(_ssgTargetLvl);
		if (_ssgStartLvl && !(_instr & 8)){
			_instr += 4;
			_ssgStep = _drv->_ssgPatches[_instr];
			_ssgTicksLeft = _drv->_ssgPatches[_instr + 1] & 0x7f;
			_ssgTargetLvl = _drv->_ssgPatches[_instr + 2];
		} else {
			_flags |= CHS_SSGOFF;
			setOutputLevel(0);
		}
	}
}

void TownsPC98_OpnChannelSSG::processFrequency() {
	if (_algorithm & 0x40)
		return;

	if (_flags & CHS_RECALCFREQ) {
		_block = _frqBlockMSB >> 4;
		_frequency = ((const uint16*)_drv->_opnFreqTableSSG)[_frqBlockMSB & 0x0f] + _frqLSB;

		uint16 f = _frequency >> _block;
		writeReg(_regOffset << 1, f & 0xff);
		writeReg((_regOffset << 1) + 1, f >> 8);

		setupPitchWheel();
	}

	if (!(_flags & (CHS_EOT | CHS_PITCHWHEELOFF | CHS_SSGOFF))) {
		if (!processPitchWheel())
			return;

		processPitchWheel();

		uint16 f = _frequency >> _block;
		writeReg(_regOffset << 1, f & 0xff);
		writeReg((_regOffset << 1) + 1, f >> 8);
	}
}

bool TownsPC98_OpnChannelSSG::processControlEvent(uint8 cmd) {
	uint8 para = *_dataPtr++;
	return (this->*controlEvents[cmd & 0x0f])(para);
}

void TownsPC98_OpnChannelSSG::nextShape() {
	_instr = (_instr & 0xf0) + 0x0c;
	_ssgStep = _drv->_ssgPatches[_instr];
	_ssgTicksLeft = _drv->_ssgPatches[_instr + 1] & 0x7f;
	_ssgTargetLvl = _drv->_ssgPatches[_instr + 2];
}

void TownsPC98_OpnChannelSSG::keyOn() {
	uint8 c = 0x7b;
	uint8 t = (_algorithm & 0xC0) << 1;
	if (_algorithm & 0x80)
		t |= 4;

	c = (c << (_regOffset + 1)) | (c >> (7 - _regOffset));
	t = (t << (_regOffset + 1)) | (t >> (7 - _regOffset));

	if (!(_algorithm & 0x80))
		writeReg(6, _algorithm & 0x7f);

	uint8 e = (_drv->_ssg->readReg(7) & c) | t;
	writeReg(7, e);
}

void TownsPC98_OpnChannelSSG::protect() {
	_flags |= CHS_PROTECT;
}

void TownsPC98_OpnChannelSSG::restore() {
	_flags &= ~CHS_PROTECT;
	keyOn();
	writeReg(8 + _regOffset, _ssgTl);
	uint16 f = _frequency >> _block;
	writeReg(_regOffset << 1, f & 0xff);
	writeReg((_regOffset << 1) + 1, f >> 8);
}

void TownsPC98_OpnChannelSSG::loadData(uint8 *data) {
	_drv->_regProtectionFlag = (_flags & CHS_PROTECT) ? true : false;
	TownsPC98_OpnChannel::loadData(data);
	setOutputLevel(0);
	_algorithm = 0x80;
}

void TownsPC98_OpnChannelSSG::setOutputLevel(uint8 lvl) {
	_ssgStartLvl = lvl;
	uint16 newTl = (((uint16)_totalLevel + 1) * (uint16)lvl) >> 8;
	if (newTl == _ssgTl)
		return;
	_ssgTl = newTl;
	writeReg(8 + _regOffset, _ssgTl);
}

void TownsPC98_OpnChannelSSG::fadeStep() {
	_totalLevel--;
	if ((int8)_totalLevel < 0)
		_totalLevel = 0;
	setOutputLevel(_ssgStartLvl);
}

bool TownsPC98_OpnChannelSSG::control_f0_setInstr(uint8 para) {
	_instr = para << 4;
	para = (para >> 3) & 0x1e;
	if (para)
		return control_f4_setAlgorithm(para | 0x40);
	return true;
}

bool TownsPC98_OpnChannelSSG::control_f1_setTotalLevel(uint8 para) {
	if (!_drv->_fading)
		_totalLevel = para;
	return true;
}

bool TownsPC98_OpnChannelSSG::control_f4_setAlgorithm(uint8 para) {
	_algorithm = para;
	return true;
}

bool TownsPC98_OpnChannelSSG::control_f9_loadCustomPatch(uint8 para) {
	_instr = (_drv->_sfxOffs + 10 + _regOffset) << 4;
	_drv->_ssgPatches[_instr] = *_dataPtr++;
	_drv->_ssgPatches[_instr + 3] = para;
	_drv->_ssgPatches[_instr + 4] = *_dataPtr++;
	_drv->_ssgPatches[_instr + 6] = *_dataPtr++;
	_drv->_ssgPatches[_instr + 8] = *_dataPtr++;
	_drv->_ssgPatches[_instr + 12] = *_dataPtr++;
	return true;
}

bool TownsPC98_OpnChannelSSG::control_fb_incOutLevel(uint8 para) {
	_dataPtr--;
	if (_drv->_fading)
		return true;

	_totalLevel--;
	if ((int8)_totalLevel < 0)
		_totalLevel = 0;

	return true;
}

bool TownsPC98_OpnChannelSSG::control_fc_decOutLevel(uint8 para) {
	_dataPtr--;
	if (_drv->_fading)
		return true;

	if(_totalLevel + 1 < 0x10)
		_totalLevel++;

	return true;
}

bool TownsPC98_OpnChannelSSG::control_ff_endOfTrack(uint8 para) {
	if (!_drv->_sfxOffs) {
		uint16 val = READ_LE_UINT16(--_dataPtr);
		if (val) {
			// loop
			_dataPtr = _drv->_trackPtr + val;
			return true;
		} else {
			// stop parsing
			if (!_drv->_fading)
				setOutputLevel(0);
			--_dataPtr;
			_flags |= CHS_EOT;
			_drv->_finishedSSGFlag |= _idFlag;
		}
	} else {
		// end of sfx track - restore ssg music channel
		_flags |= CHS_EOT;
		_drv->_ssgChannels[_chanNum]->restore();
	}

	return false;
}

void TownsPC98_OpnSfxChannel::loadData(uint8 *data) {
	_flags = CHS_ALL_BUT_EOT;
	_ticksLeft = 1;
	_dataPtr = data;
	_ssgTl = 0xff;
	_algorithm = 0x80;
}

TownsPC98_OpnChannelPCM::TownsPC98_OpnChannelPCM(TownsPC98_OpnDriver *driver, uint8 regOffs,
		uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id) :
		TownsPC98_OpnChannel(driver, regOffs, flgs, num, key, prt, id) {
}

void TownsPC98_OpnChannelPCM::init() {
	_algorithm = 0x80;

	#define Control(x)	&TownsPC98_OpnChannelPCM::control_##x
	static const ControlEventFunc ctrlEventsPCM[] = {
		Control(dummy),
		Control(f1_pcmStart),
		Control(dummy),
		Control(dummy),
		Control(dummy),
		Control(dummy),
		Control(f6_repeatSection),
		Control(dummy),
		Control(dummy),
		Control(dummy),
		Control(fa_writeReg),
		Control(dummy),
		Control(dummy),
		Control(dummy),
		Control(dummy),
		Control(ff_endOfTrack)
	};
	#undef Control

	controlEvents = ctrlEventsPCM;
}

void TownsPC98_OpnChannelPCM::loadData(uint8 *data) {
	_flags = (_flags & ~CHS_EOT) | CHS_ALL_BUT_EOT;
	_ticksLeft = 1;
	_dataPtr = data;
	_totalLevel = 0x7F;
}

void TownsPC98_OpnChannelPCM::processEvents()  {
	if (_flags & CHS_EOT)
		return;

	if (--_ticksLeft)
		return;

	uint8 cmd = 0;
	bool loop = true;

	while (loop) {
		cmd = *_dataPtr++;
		if (cmd == 0x80) {
			loop = false;
		} else if (cmd < 0xf0) {
			writeReg(0x10, cmd);
		} else if (!processControlEvent(cmd)) {
			return;
		}
	}

	_ticksLeft = *_dataPtr++;
}

bool TownsPC98_OpnChannelPCM::processControlEvent(uint8 cmd) {
	uint8 para = *_dataPtr++;
	return (this->*controlEvents[cmd & 0x0f])(para);
}

void TownsPC98_OpnChannelPCM::reset() {
	TownsPC98_OpnChannel::reset();

	if (_drv->_pcm)
		_drv->_pcm->reset();
}

bool TownsPC98_OpnChannelPCM::control_f1_pcmStart(uint8 para) {
	_totalLevel = para;
	writeReg(0x11, para);
	return true;
}

bool TownsPC98_OpnChannelPCM::control_ff_endOfTrack(uint8 para) {
	uint16 val = READ_LE_UINT16(--_dataPtr);
	if (val) {
		// loop
		_dataPtr = _drv->_trackPtr + val;
		return true;
	} else {
		// quit parsing for active channel
		--_dataPtr;
		_flags |= CHS_EOT;
		_drv->_finishedPCMFlag |= _idFlag;
		return false;
	}
}

TownsPC98_OpnSquareSineSource::TownsPC98_OpnSquareSineSource(const float rate) : _rate(rate),	_tlTable(0),
	_tleTable(0), _regIndex(_reg), _updateRequest(-1), _tickLength((int)(rate * 32768.0f * 27.0f)), _ready(0) {
	memset(_reg, 0, 16);
	memset(_channels, 0, sizeof(Channel) * 3);
	reset();
}

TownsPC98_OpnSquareSineSource::~TownsPC98_OpnSquareSineSource() {
	delete [] _tlTable;
	delete [] _tleTable;
}

void TownsPC98_OpnSquareSineSource::init(const int *rsTable, const int *rseTable) {
	if (_ready) {
		reset();
		return;
	}

	delete [] _tlTable;
	delete [] _tleTable;
	_tlTable = new int32[16];
	_tleTable = new int32[32];
	float a, b, d;
	d = 801.0f;

	for (int i = 0; i < 16; i++) {
		b = 1.0f / rsTable[i];
		a = 1.0f / d + b + 1.0f / 1000.0f;
		float v = (b / a) * 32767.0f;
		_tlTable[i] = (int32) v;

		b = 1.0f / rseTable[i];
		a = 1.0f / d + b + 1.0f / 1000.0f;
		v = (b / a) * 32767.0f;
		_tleTable[i] = (int32) v;
	}

	for (int i = 16; i < 32; i++) {
		b = 1.0f / rseTable[i];
		a = 1.0f / d + b + 1.0f / 1000.0f;
		float v = (b / a) * 32767.0f;
		_tleTable[i] = (int32) v;
	}

	_ready = true;
}

void TownsPC98_OpnSquareSineSource::reset() {
	_rand = 1;
	_outN = 1;
	_updateRequest = -1;
	_nTick = _evpUpdateCnt = 0;
	_regIndex = _reg;
	_evpTimer = 0x1f;
	_pReslt = 0x1f;
	_attack = 0;
	_cont = false;
	_evpUpdate = true;
	_timer = 0;

	for (int i = 0; i < 3; i++) {
		_channels[i].tick = 0;
		_channels[i].smp = _channels[i].out = 0;
	}

	for (int i = 0; i < 14; i++)
		writeReg(i, 0, true);

	writeReg(7, 0xbf, true);
}

uint8 TownsPC98_OpnSquareSineSource::readReg(uint8 address) {
	return _reg[address];
}

void TownsPC98_OpnSquareSineSource::writeReg(uint8 address, uint8 value, bool force) {
	_regIndex = &_reg[address];
	int o = _regIndex - _reg;
	if (!force && (o == 13 || *_regIndex != value)) {
		if (_updateRequest == 31) {
			warning("TownsPC98_OpnSquareSineSource: event buffer overflow");
			_updateRequest = -1;
		}
		_updateRequestBuf[++_updateRequest] = value;
		_updateRequestBuf[++_updateRequest] = o;
		return;
	}

	*_regIndex = value;
}

void TownsPC98_OpnSquareSineSource::nextTick(int32 *buffer, uint32 bufferSize) {
	if (!_ready)
		return;

	for (uint32 i = 0; i < bufferSize; i++) {
		_timer += _tickLength;
		while (_timer > 0x30000) {
			_timer -= 0x30000;

			if (++_nTick >= (_reg[6] & 0x1f)) {
				if ((_rand + 1) & 2)
					_outN ^= 1;

				_rand = (((_rand & 1) ^ ((_rand >> 3) & 1)) << 16) | (_rand >> 1);
				_nTick = 0;
			}

			for (int ii = 0; ii < 3; ii++) {
				if (++_channels[ii].tick >= (((_reg[ii * 2 + 1] & 0x0f) << 8) | _reg[ii * 2])) {
					_channels[ii].tick = 0;
					_channels[ii].smp ^= 1;
				}
				_channels[ii].out = (_channels[ii].smp | ((_reg[7] >> ii) & 1)) & (_outN | ((_reg[7] >> (ii + 3)) & 1));
			}

			if (_evpUpdate) {
				if (++_evpUpdateCnt >= ((_reg[12] << 8) | _reg[11])) {
					_evpUpdateCnt = 0;

					if (--_evpTimer < 0) {
						if (_cont) {
							_evpTimer &= 0x1f;
						} else {
							_evpUpdate = false;
							_evpTimer = 0;
						}
					}
				}
			}
			_pReslt = _evpTimer ^ _attack;
			updatesRegs();
		}

		int32 finOut = 0;
		for (int ii = 0; ii < 3; ii++) {
			if ((_reg[ii + 8] >> 4) & 1)
				finOut += _tleTable[_channels[ii].out ? _pReslt : 0];
			else
				finOut += _tlTable[_channels[ii].out ? (_reg[ii + 8] & 0x0f) : 0];
		}

		finOut /= 2;
		buffer[i << 1] += finOut;
		buffer[(i << 1) + 1] += finOut;
	}
}

void TownsPC98_OpnSquareSineSource::updatesRegs() {
	for (int i = 0; i < _updateRequest;) {
		uint8 b = _updateRequestBuf[i++];
		uint8 a = _updateRequestBuf[i++];
		writeReg(a, b, true);
	}
	_updateRequest = -1;
}

TownsPC98_OpnPercussionSource::TownsPC98_OpnPercussionSource(const float rate) :
	_tickLength((int)(rate * 65536.0)), _timer(0), _ready(false) {
		memset(_pcmInstr, 0, sizeof(PcmInstrument) * 6);
}

TownsPC98_OpnPercussionSource::~TownsPC98_OpnPercussionSource() {
}

void TownsPC98_OpnPercussionSource::init(const void *pcmData) {
	if (_ready) {
		reset();
		return;
	}

	const uint8 *start = (const uint8*) pcmData;
	const uint8 *pos = start;

	if (pcmData) {
		for (int i = 0; i < 6; i++) {
			_pcmInstr[i].data = start + READ_BE_UINT16(pos);
			pos += 2;
			_pcmInstr[i].size = READ_BE_UINT16(pos);
			pos += 2;
		}
		reset();
		_ready = true;
	} else {
		memset(_pcmInstr, 0, sizeof(PcmInstrument) * 6);
		_ready = false;
	}
}

void TownsPC98_OpnPercussionSource::reset() {
	_timer = 0;
	_totalLevel = 63;

	memset(_regs, 0, 48);

	for (int i = 0; i < 6; i++) {
		PcmInstrument *s = &_pcmInstr[i];
		s->pos = s->start = s->data;
		s->end = s->data + s->size;
		s->active = false;
		s->level = 0;
		s->out = 0;
		s->decStep = 1;
		s->decState = 0;
		s->samples[0] = s->samples[1] = 0;
	}
}

void TownsPC98_OpnPercussionSource::writeReg(uint8 address, uint8 value) {
	if (!_ready)
		return;

	 uint8 h = address >> 4;
	 uint8 l = address & 15;

	_regs[address] = value;

	if (address == 0) {
		if (value & 0x80) {
			//key off
			for (int i = 0; i < 6; i++) {
				if ((value >> i) & 1)
					_pcmInstr[i].active = false;
			}
		} else {
			//key on
			for (int i = 0; i < 6; i++) {
				if ((value >> i) & 1) {
					PcmInstrument *s = &_pcmInstr[i];
					s->pos = s->start;
					s->active = true;
					s->out = 0;
					s->samples[0] = s->samples[1] = 0;
					s->decStep = 1;
					s->decState = 0;
				}
			}
		}
	} else if (address == 1) {
		// total level
		_totalLevel = (value & 63) ^ 63;
		for (int i = 0; i < 6; i++)
			recalcOuput(&_pcmInstr[i]);
	} else if (!h && l & 8) {
		// instrument level
		l &= 7;
		_pcmInstr[l].level = (value & 0x1f) ^ 0x1f;
		recalcOuput(&_pcmInstr[l]);
	} else if (h & 3) {
		l &= 7;
		if (h == 1) {
			// set start offset
			_pcmInstr[l].start  = _pcmInstr[l].data + ((_regs[24 + l] * 256 + _regs[16 + l]) << 8);
		} else if (h == 2) {
			// set end offset
			_pcmInstr[l].end = _pcmInstr[l].data + ((_regs[40 + l] * 256 + _regs[32 + l]) << 8) + 255;
		}
	}
}

void TownsPC98_OpnPercussionSource::nextTick(int32 *buffer, uint32 bufferSize) {
	if (!_ready)
		return;

	for (uint32 i = 0; i < bufferSize; i++) {
		_timer += _tickLength;
		while (_timer > 0x30000) {
			_timer -= 0x30000;

			for (int ii = 0; ii < 6; ii++) {
				PcmInstrument *s = &_pcmInstr[ii];
				if (s->active) {
					recalcOuput(s);
					if (s->decStep) {
						advanceInput(s);
						if (s->pos == s->end)
							s->active = false;
					}
					s->decStep ^= 1;
				}
			}
		}

		int32 finOut = 0;

		for (int ii = 0; ii < 6; ii++) {
			if (_pcmInstr[ii].active)
				finOut += _pcmInstr[ii].out;
		}

		finOut = (finOut * 7);

		buffer[i << 1] += finOut;
		buffer[(i << 1) + 1] += finOut;
	}
}

void TownsPC98_OpnPercussionSource::recalcOuput(PcmInstrument *ins) {
	uint32 s = _totalLevel + ins->level;
	uint32 x = s > 62 ? 0 : (1 + (s >> 3));
	int32 y = s > 62 ? 0 : (15 - (s & 7));
	ins->out = ((ins->samples[ins->decStep] * y) >> x) & ~3;
}

void TownsPC98_OpnPercussionSource::advanceInput(PcmInstrument *ins) {
	static const int8 adjustIndex[] = {-1, -1, -1, -1, 2, 5, 7, 9 };

	static const int16 stepTable[] = { 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55,
		60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337,
		371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552
	};
	
	int d = ins->samples[1];
	uint8 cur = (int8) *ins->pos++;

	for (int i = 0; i < 2; i++) {
		int b = (2 * (cur & 7) + 1) * stepTable[ins->decState] / 8;
		if (cur & 8)
			b = -b;
		d += b;
		d = CLIP<int16>(d, -2048, 2047);
		ins->samples[i] = d;
		ins->decState = CLIP<int8>(ins->decState + adjustIndex[cur & 7], 0, 48);
		cur >>= 4;
	}
}

TownsPC98_OpnDriver::TownsPC98_OpnDriver(Audio::Mixer *mixer, OpnType type) :
	_mixer(mixer), _trackPtr(0), _musicPlaying(false), _sfxPlaying(false), _fading(false), _channels(0),
	_ssgChannels(0), _sfxChannels(0), _pcmChannel(0),	_looping(0), _opnCarrier(_drvTables + 76),
	_opnFreqTable(_drvTables + 84), _opnFreqTableSSG(_drvTables + 252),	_opnFxCmdLen(_drvTables + 36),
	_opnLvlPresets(_drvTables + (type == OD_TOWNS ? 52 : 228)), _oprRates(0), _oprRateshift(0), _oprAttackDecay(0),
	_oprFrq(0), _oprSinTbl(0), _oprLevelOut(0), _oprDetune(0), _cbCounter(4), _musicTickCounter(0),
	_updateChannelsFlag(type == OD_TYPE26 ? 0x07 : 0x3F), _updateSSGFlag(type == OD_TOWNS ? 0 : 7),
	_updatePCMFlag(type == OD_TYPE86 ? 1 : 0), _finishedChannelsFlag(0), _finishedSSGFlag(0),
	_finishedPCMFlag(0), _samplesTillCallback(0), _samplesTillCallbackRemainder(0),
	_sfxData(0), _ready(false), _numSSG(type == OD_TOWNS ? 0 : 3), _hasPCM(type == OD_TYPE86 ? true : false),
	_sfxOffs(0), _numChan(type == OD_TYPE26 ? 3 : 6), _hasStereo(type == OD_TYPE26 ? false : true),
	_ssgPatches(0), _ssg(0), _pcm(0), _baserate(55125.0f / (float)getRate()) {
	setTempo(84);
}

TownsPC98_OpnDriver::~TownsPC98_OpnDriver() {
	_mixer->stopHandle(_soundHandle);

	if (_channels) {
		for (int i = 0; i < _numChan; i++)
			delete _channels[i];
		delete [] _channels;
	}

	if (_ssgChannels) {
		for (int i = 0; i < _numSSG; i++)
			delete _ssgChannels[i];
		delete [] _ssgChannels;
	}

	if (_sfxChannels) {
		for (int i = 0; i < 2; i++)
			delete _sfxChannels[i];
		delete [] _sfxChannels;
	}

	if (_pcmChannel)
		delete _pcmChannel;

	delete _ssg;
	delete _pcm;

	delete [] _oprRates;
	delete [] _oprRateshift;
	delete [] _oprFrq;
	delete [] _oprAttackDecay;
	delete [] _oprSinTbl;
	delete [] _oprLevelOut;
	delete [] _oprDetune;
	delete [] _ssgPatches;
}

bool TownsPC98_OpnDriver::init() {
	if (_ready) {
		reset();
		return true;
	}

	generateTables();

	if (_channels) {
		for (int i = 0; i < _numChan; i++) {
			if (_channels[i])
				delete _channels[i];
		}
		delete [] _channels;
	}
	_channels = new TownsPC98_OpnChannel*[_numChan];
	for (int i = 0; i < _numChan; i++) {
		int ii = i * 6;
		_channels[i] = new TownsPC98_OpnChannel(this, _drvTables[ii], _drvTables[ii + 1],
			_drvTables[ii + 2],	_drvTables[ii + 3],	_drvTables[ii + 4], _drvTables[ii + 5]);
		_channels[i]->init();
	}

	if (_ssgChannels) {
		for (int i = 0; i < _numSSG; i++) {
			if (_ssgChannels[i])
				delete _ssgChannels[i];
		}
		delete [] _ssgChannels;
	}

	if (_sfxChannels) {
		for (int i = 0; i < 2; i++) {
			if (_sfxChannels[i])
				delete _sfxChannels[i];
		}
		delete [] _sfxChannels;
	}

	if (_numSSG) {
		_ssg = new TownsPC98_OpnSquareSineSource(_baserate);
		_ssg->init(&_ssgTables[0], &_ssgTables[16]);
		_ssgPatches = new uint8[256];
		memcpy(_ssgPatches, _drvTables + 244, 256);

		_ssgChannels = new TownsPC98_OpnChannelSSG*[_numSSG];
		for (int i = 0; i < _numSSG; i++) {
			int ii = i * 6;
			_ssgChannels[i] = new TownsPC98_OpnChannelSSG(this, _drvTables[ii], _drvTables[ii + 1],
				_drvTables[ii + 2],	_drvTables[ii + 3],	_drvTables[ii + 4], _drvTables[ii + 5]);
			_ssgChannels[i]->init();
		}

		_sfxChannels = new TownsPC98_OpnSfxChannel*[2];
		for (int i = 0; i < 2; i++) {
			int ii = (i + 1) * 6;
			_sfxChannels[i] = new TownsPC98_OpnSfxChannel(this, _drvTables[ii], _drvTables[ii + 1],
				_drvTables[ii + 2],	_drvTables[ii + 3],	_drvTables[ii + 4], _drvTables[ii + 5]);
			_sfxChannels[i]->init();
		}
	}

	if (_hasPCM) {
		_pcm = new TownsPC98_OpnPercussionSource(_baserate);
		_pcm->init();

		delete _pcmChannel;
		_pcmChannel = new TownsPC98_OpnChannelPCM(this, 0, 0, 0, 0, 0, 1);
		_pcmChannel->init();
	}

	_mixer->playInputStream(Audio::Mixer::kMusicSoundType,
		&_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, false, true);

	_ready = true;
	return true;
}

int inline TownsPC98_OpnDriver::readBuffer(int16 *buffer, const int numSamples) {
	memset(buffer, 0, sizeof(int16) * numSamples);
	int32 *tmp = new int32[numSamples];
	int32 *tmpStart = tmp;
	memset(tmp, 0, sizeof(int32) * numSamples);
	int32 samplesLeft = numSamples >> 1;

	while (samplesLeft) {
		if (!_samplesTillCallback) {
			callback();

			_samplesTillCallback = _samplesPerCallback;
			_samplesTillCallbackRemainder += _samplesPerCallbackRemainder;
			if (_samplesTillCallbackRemainder >= _tempo) {
				_samplesTillCallback++;
				_samplesTillCallbackRemainder -= _tempo;
			}
		}

		int32 render = MIN(samplesLeft, _samplesTillCallback);
		samplesLeft -= render;
		_samplesTillCallback -= render;

		nextTick(tmp, render);

		if (_ssg)
			_ssg->nextTick(tmp, render);
		if (_pcm)
			_pcm->nextTick(tmp, render);

		for (int i = 0; i < render; ++i) {
			int32 l = tmp[i << 1];
			if (l > 32767)
				l = 32767;
			if (l < -32767)
				l = -32767;
			buffer[i << 1] = (int16) l;

			int32 r = tmp[(i << 1) + 1];
			if (r > 32767)
				r = 32767;
			if (r < -32767)
				r = -32767;
			buffer[(i << 1) + 1] = (int16) r;
		}

		buffer += (render << 1);
		tmp += (render << 1);
	}

	delete [] tmpStart;
	return numSamples;
}

void TownsPC98_OpnDriver::loadMusicData(uint8 *data, bool loadPaused) {
	if (!_ready) {
		warning("TownsPC98_OpnDriver: Driver must be initialized before loading data");
		return;
	}

	if (!data) {
		warning("TownsPC98_OpnDriver: Invalid music file data");
		return;
	}

	reset();

	lock();

	uint8 *src_a = _trackPtr = _musicBuffer = data;

	for (uint8 i = 0; i < 3; i++) {
		_channels[i]->loadData(data + READ_LE_UINT16(src_a));
		src_a += 2;
	}

	for (int i = 0; i < _numSSG; i++) {
		_ssgChannels[i]->loadData(data + READ_LE_UINT16(src_a));
		src_a += 2;
	}

	for (uint8 i = 3; i < _numChan; i++) {
		_channels[i]->loadData(data + READ_LE_UINT16(src_a));
		src_a += 2;
	}

	if (_hasPCM) {
		_pcmChannel->loadData(data + READ_LE_UINT16(src_a));
		src_a += 2;
	}

	_regProtectionFlag = false;

	_patches = src_a + 4;
	_cbCounter = 4;
	_finishedChannelsFlag = _finishedSSGFlag = 0;

	_musicPlaying = (loadPaused ? false : true);

	unlock();
}

void TownsPC98_OpnDriver::loadSoundEffectData(uint8 *data, uint8 trackNum) {
	if (!_ready) {
		warning("TownsPC98_OpnDriver: Driver must be initialized before loading data");
		return;
	}

	if (!_sfxChannels) {
		warning("TownsPC98_OpnDriver: sound effects not supported by this configuration");
		return;
	}

	if (!data) {
		warning("TownsPC98_OpnDriver: Invalid sound effect file data");
		return;
	}

	lock();
	_sfxData = _sfxBuffer = data;
	_sfxOffsets[0] = READ_LE_UINT16(&_sfxData[(trackNum << 2)]);
	_sfxOffsets[1] = READ_LE_UINT16(&_sfxData[(trackNum << 2) + 2]);
	_sfxPlaying = true;
	unlock();
}

void TownsPC98_OpnDriver::reset() {
	lock();

	for (int i = 0; i < _numChan; i++)
		_channels[i]->reset();
	for (int i = 0; i < _numSSG; i++)
		_ssgChannels[i]->reset();

	if (_ssg) {
		for (int i = 0; i < 2; i++)
			_sfxChannels[i]->reset();

		memcpy(_ssgPatches, _drvTables + 276, 256);
		_ssg->reset();
	}

	if (_pcmChannel)
		_pcmChannel->reset();

	_musicPlaying = _sfxPlaying = _fading = false;
	_looping = 0;
	_musicTickCounter = 0;
	_sfxData = 0;

	unlock();
}

void TownsPC98_OpnDriver::fadeStep() {
	if (!_musicPlaying)
		return;

	lock();

	for (int j = 0; j < _numChan; j++) {
		if (_updateChannelsFlag & _channels[j]->_idFlag)
			_channels[j]->fadeStep();
	}
		
	for (int j = 0; j < _numSSG; j++) {
		if (_updateSSGFlag & _ssgChannels[j]->_idFlag)
			_ssgChannels[j]->fadeStep();
	}

	if (!_fading) {
		_fading = 19;
		if (_hasPCM) {
			if (_updatePCMFlag & _pcmChannel->_idFlag)
				_pcmChannel->reset();
		}
	} else {
		if (!--_fading)
			reset();
	}

	unlock();
}

void TownsPC98_OpnDriver::callback() {
	lock();

	if (_sfxChannels && _sfxPlaying) {
		if (_sfxData)
			startSoundEffect();

		_sfxOffs = 3;
		_trackPtr = _sfxBuffer;

		for (int i = 0; i < 2; i++) {
			_sfxChannels[i]->processEvents();
			_sfxChannels[i]->processFrequency();
		}

		_trackPtr = _musicBuffer;
	}

	_sfxOffs = 0;

	if (!--_cbCounter && _musicPlaying) {
		_cbCounter = 4;
		_musicTickCounter++;

		for (int i = 0; i < _numChan; i++) {
			if (_updateChannelsFlag & _channels[i]->_idFlag) {
				_channels[i]->processEvents();
				_channels[i]->processFrequency();
			}
		}

		if (_numSSG) {
			for (int i = 0; i < _numSSG; i++) {
				if (_updateSSGFlag & _ssgChannels[i]->_idFlag) {
					_ssgChannels[i]->processEvents();
					_ssgChannels[i]->processFrequency();
				}
			}
		}

		if (_hasPCM)
			if (_updatePCMFlag & _pcmChannel->_idFlag)
				_pcmChannel->processEvents();
	}

	_regProtectionFlag = false;

	if (_finishedChannelsFlag == _updateChannelsFlag && _finishedSSGFlag == _updateSSGFlag && _finishedPCMFlag == _updatePCMFlag)
		_musicPlaying = false;

	unlock();
}

void TownsPC98_OpnDriver::nextTick(int32 *buffer, uint32 bufferSize) {
	if (!_ready)
		return;

	for (int i = 0; i < _numChan; i++) {
		if (_channels[i]->_updateEnvelopeParameters) {
			_channels[i]->_updateEnvelopeParameters = false;
			_channels[i]->updateEnv();
		}

		for (uint32 ii = 0; ii < bufferSize ; ii++)
			_channels[i]->generateOutput(buffer[ii * 2],
				buffer[ii * 2 + 1],	&_channels[i]->_feedbuf[2], _channels[i]->_feedbuf);
	}
}

void TownsPC98_OpnDriver::generateTables() {
	delete [] _oprRates;
	_oprRates = new uint8[128];
	memset(_oprRates, 0x90, 32);
	uint8 *dst = (uint8*) _oprRates + 32;
	for (int i = 0; i < 48; i += 4)
		WRITE_BE_UINT32(dst + i, 0x00081018);
	dst += 48;
	for (uint8 i = 0; i < 16; i ++) {
		uint8 v = (i < 12) ? i : 12;
		*dst++ = ((4 + v) << 3);
	}
	memset(dst, 0x80, 32);

	delete [] _oprRateshift;
	_oprRateshift = new uint8[128];
	memset(_oprRateshift, 0, 128);
	dst = (uint8*) _oprRateshift + 32;
	for (int i = 11; i; i--) {
		memset(dst, i, 4);
		dst += 4;
	}

	delete [] _oprFrq;
	_oprFrq = new uint32[0x1000];
	for (uint32 i = 0; i < 0x1000; i++)
		_oprFrq[i] = (uint32)(_baserate * (float)(i << 11));

	delete [] _oprAttackDecay;
	_oprAttackDecay = new uint8[152];
	memset(_oprAttackDecay, 0, 152);
	for (int i = 0; i < 36; i++)
		WRITE_BE_UINT32(_oprAttackDecay + (i << 2), _adtStat[i]);

	delete [] _oprSinTbl;
	_oprSinTbl = new uint32[1024];
	for (int i = 0; i < 1024; i++) {
		double val = sin((double) (((i << 1) + 1) * PI / 1024.0));
		double d_dcb = log(1.0 / (double)ABS(val)) / log(2.0) * 256.0;
		int32 i_dcb = (int32)(2.0 * d_dcb);
		i_dcb = (i_dcb & 1) ? (i_dcb >> 1) + 1 : (i_dcb >> 1);
		_oprSinTbl[i] = (i_dcb << 1) + (val >= 0.0 ? 0 : 1);
	}

	delete [] _oprLevelOut;
	_oprLevelOut = new int32[0x1a00];
	for (int i = 0; i < 256; i++) {
		double val = floor(65536.0 / pow(2.0, 0.00390625 * (double)(1 + i)));
		int32 val_int = ((int32) val) >> 4;
		_oprLevelOut[i << 1] = (val_int & 1) ? ((val_int >> 1) + 1) << 2 : (val_int >> 1) << 2;
		_oprLevelOut[(i << 1) + 1] = -_oprLevelOut[i << 1];
		for (int ii = 1; ii < 13; ii++) {
			_oprLevelOut[(i << 1) + (ii << 9)] =  _oprLevelOut[i << 1] >> ii;
			_oprLevelOut[(i << 1) + (ii << 9) + 1] = -_oprLevelOut[(i << 1) + (ii << 9)];
		}
	}

	uint8 *dtt = new uint8[128];
	memset(dtt, 0, 36);
	memset(dtt + 36, 1, 8);
	memcpy(dtt + 44, _drvTables + 144, 84);

	delete [] _oprDetune;
	_oprDetune = new int32[256];
	for (int i = 0; i < 128; i++) {
		_oprDetune[i] = (int32)	((float)dtt[i] * _baserate * 64.0);
		_oprDetune[i + 128] = -_oprDetune[i];
	}

	delete [] dtt;
}

void TownsPC98_OpnDriver::startSoundEffect() {
	for (int i = 0; i < 2; i++) {
		if (_sfxOffsets[i]) {
			_ssgChannels[i + 1]->protect();
			_sfxChannels[i]->reset();
			_sfxChannels[i]->loadData(_sfxData + _sfxOffsets[i]);
		}
	}

	_sfxData = 0;
}

void TownsPC98_OpnDriver::setTempo(uint8 tempo) {
	_tempo = tempo - 0x10;
	_samplesPerCallback = _tempo ? (getRate() / _tempo) : 0;
	_samplesPerCallbackRemainder = _tempo ? (getRate() % _tempo) : 0;
}

SoundTowns::SoundTowns(KyraEngine_v1 *vm, Audio::Mixer *mixer)
	: Sound(vm, mixer), _lastTrack(-1), _currentSFX(0), _sfxFileData(0),
	_sfxFileIndex((uint)-1), _sfxWDTable(0), _sfxBTTable(0), _parser(0) {

	_driver = new Towns_EuphonyDriver(_mixer);
	int ret = open();
	if (ret != MERR_ALREADY_OPEN && ret != 0)
		error("couldn't open midi driver");
}

SoundTowns::~SoundTowns() {
	AudioCD.stop();
	haltTrack();
	delete[] _sfxFileData;

	Common::StackLock lock(_mutex);
	_driver->setTimerCallback(0, 0);
	close();

	_driver = 0;
}

bool SoundTowns::init() {
	_vm->checkCD();
	int unused = 0;
	_sfxWDTable = _vm->staticres()->loadRawData(k1TownsSFXwdTable, unused);
	_sfxBTTable = _vm->staticres()->loadRawData(k1TownsSFXbtTable, unused);

	return loadInstruments();
}

void SoundTowns::process() {
	AudioCD.updateCD();
}

void SoundTowns::playTrack(uint8 track) {
	if (track < 2)
		return;
	track -= 2;

	const int32 *const tTable = (const int32 *const) cdaData();
	int tTableIndex = 3 * track;

	int trackNum = (int) READ_LE_UINT32(&tTable[tTableIndex + 2]);
	int32 loop = (int32) READ_LE_UINT32(&tTable[tTableIndex + 1]);

	if (track == _lastTrack && _musicEnabled)
		return;

	beginFadeOut();

	if (_musicEnabled == 2 && trackNum != -1) {
		AudioCD.play(trackNum+1, loop ? -1 : 1, 0, 0);
		AudioCD.updateCD();
	} else if (_musicEnabled) {
		playEuphonyTrack(READ_LE_UINT32(&tTable[tTableIndex]), loop);
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
	}
	_driver->queue()->release();
}

void SoundTowns::loadSoundFile(uint file) {
	if (_sfxFileIndex == file)
		return;
	_sfxFileIndex = file;
	delete[] _sfxFileData;
	_sfxFileData = _vm->resource()->fileData(fileListEntry(file), 0);
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

	uint8 *fileBody = _sfxFileData + 0x01b8;
	int32 offset = (int32)READ_LE_UINT32(_sfxFileData + (track - 0x0b) * 4);
	if (offset == -1)
		return;

	uint32 *sfxHeader = (uint32*)(fileBody + offset);

	uint32 sfxHeaderID = READ_LE_UINT32(sfxHeader);
	uint32 sfxHeaderInBufferSize = READ_LE_UINT32(&sfxHeader[1]);
	uint32 sfxHeaderOutBufferSize = READ_LE_UINT32(&sfxHeader[3]);
	uint32 sfxRootNoteOffs = READ_LE_UINT32(&sfxHeader[7]);
	uint32 sfxRate = READ_LE_UINT32(&sfxHeader[6]);

	uint32 playbackBufferSize = (sfxHeaderID == 1) ? sfxHeaderInBufferSize : sfxHeaderOutBufferSize;

	uint8 *sfxPlaybackBuffer = (uint8 *)malloc(playbackBufferSize);
	memset(sfxPlaybackBuffer, 0x80, playbackBufferSize);

	uint8 *sfxBody = ((uint8 *)sfxHeader) + 0x20;

	if (!sfxHeaderID) {
		memcpy(sfxPlaybackBuffer, sfxBody, playbackBufferSize);
	} else if (sfxHeaderID == 1) {
		Screen::decodeFrame4(sfxBody, sfxPlaybackBuffer, playbackBufferSize);
	} else if (_sfxWDTable) {
		uint8 *tgt = sfxPlaybackBuffer;
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

	uint32 outputRate = uint32(11025 * calculatePhaseStep(note, sfxRootNoteOffs, sfxRate, 11025, 0x2000));

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
	uint8 *twm = _vm->resource()->fileData("twmusic.pak", 0);
	if (!twm)
		return false;
	_driver->queue()->loadDataToCurrentPosition(twm, 0x8BF0);
	_driver->loadFmInstruments(_driver->queue()->trackData() + 8);

	_driver->queue()->loadDataToCurrentPosition(twm + 0x0CA0, 0xC58A);
	_driver->loadWaveInstruments(_driver->queue()->trackData() + 8);
	delete[] twm;
	_driver->queue()->release();

	return true;
}

void SoundTowns::playEuphonyTrack(uint32 offset, int loop) {
	uint8 *twm = _vm->resource()->fileData("twmusic.pak", 0);
	Common::StackLock lock(_mutex);

	if (!_parser) {
		_parser = new Towns_EuphonyParser(_driver->queue());
		_parser->setMidiDriver(this);
		_parser->setTimerRate(getBaseTempo());
	}

	_parser->property(MidiParser::mpAutoLoop, loop);
	_parser->loadMusic(twm + 0x4b70 + offset, 0xC58A);

	delete[] twm;
}

void SoundTowns::onTimer(void *data) {
	SoundTowns *music = (SoundTowns *)data;
	Common::StackLock lock(music->_mutex);
	if (music->_parser)
		music->_parser->onTimer();
}

float SoundTowns::calculatePhaseStep(int8 semiTone, int8 semiToneRootkey,
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

SoundPC98::SoundPC98(KyraEngine_v1 *vm, Audio::Mixer *mixer) :
	Sound(vm, mixer), _musicTrackData(0), _sfxTrackData(0), _lastTrack(-1), _driver(0) {
}

SoundPC98::~SoundPC98() {
	delete[] _musicTrackData;
	delete[] _sfxTrackData;
	delete _driver;
}

bool SoundPC98::init() {
	_driver = new TownsPC98_OpnDriver(_mixer, TownsPC98_OpnDriver::OD_TYPE26);
	_sfxTrackData = _vm->resource()->fileData("se.dat", 0);
	if (!_sfxTrackData)
		return false;
	return _driver->init();
}

void SoundPC98::playTrack(uint8 track) {
	if (--track >= 56)
		track -= 55;

	if (track == _lastTrack && _musicEnabled)
		return;

	beginFadeOut();

	char musicfile[13];
	sprintf(musicfile, fileListEntry(0), track);
	delete[] _musicTrackData;
	_musicTrackData = _vm->resource()->fileData(musicfile, 0);
	if (_musicEnabled)
		_driver->loadMusicData(_musicTrackData);

	_lastTrack = track;
}

void SoundPC98::haltTrack() {
	_lastTrack = -1;
	AudioCD.stop();
	AudioCD.updateCD();
	_driver->reset();
}

void SoundPC98::beginFadeOut() {
	if (!_driver->musicPlaying())
		return;

	for (int i = 0; i < 20; i++) {
		_driver->fadeStep();
		_vm->delay(32);
	}
	haltTrack();
}

void SoundPC98::playSoundEffect(uint8 track) {
	if (!_sfxTrackData)
		return;

	//	This has been disabled for now since I don't know
	//	how to make up the correct track number. It probably
	//	needs a map.
	// _driver->loadSoundEffectData(_sfxTrackData, track);
}


//	KYRA 2

SoundTownsPC98_v2::SoundTownsPC98_v2(KyraEngine_v1 *vm, Audio::Mixer *mixer) :
	Sound(vm, mixer), _currentSFX(0), _musicTrackData(0), _sfxTrackData(0), _lastTrack(-1), _driver(0), _useFmSfx(false) {
}

SoundTownsPC98_v2::~SoundTownsPC98_v2() {
	delete[] _musicTrackData;
	delete[] _sfxTrackData;
	delete _driver;
}

bool SoundTownsPC98_v2::init() {
	_driver = new TownsPC98_OpnDriver(_mixer, _vm->gameFlags().platform == Common::kPlatformPC98 ?
		TownsPC98_OpnDriver::OD_TYPE86 : TownsPC98_OpnDriver::OD_TOWNS);
	_useFmSfx = _vm->gameFlags().platform == Common::kPlatformPC98 ? true : false;
	_vm->checkCD();
	// FIXME: While checking for 'track1.XXX(X)' looks like
	// a good idea, we should definitely not be doing this
	// here. Basically our filenaming scheme could change
	// or we could add support for other audio formats. Also
	// this misses the possibility that we play the tracks
	// right off CD. So we should find another way to
	// check if we have access to CD audio.
	if (_musicEnabled &&
		(Common::File::exists("track1.mp3") || Common::File::exists("track1.ogg") ||
		 Common::File::exists("track1.flac") || Common::File::exists("track1.fla")))
			_musicEnabled = 2;
	
	return _driver->init();
}

void SoundTownsPC98_v2::loadSoundFile(Common::String file) {
	delete [] _sfxTrackData;
	_sfxTrackData = _vm->resource()->fileData(file.c_str(), 0);
}

void SoundTownsPC98_v2::process() {
	AudioCD.updateCD();
}

void SoundTownsPC98_v2::playTrack(uint8 track) {
	if (track == _lastTrack && _musicEnabled)
		return;

	const uint16 *const cdaTracks = (const uint16 *const) cdaData();

	int trackNum = -1;
	if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
		for (int i = 0; i < cdaTrackNum(); i++) {
			if (track == (uint8) READ_LE_UINT16(&cdaTracks[i * 2])) {
				trackNum = (int) READ_LE_UINT16(&cdaTracks[i * 2 + 1]) - 1;
				break;
			}
		}
	}

	beginFadeOut();

	char musicfile[13];
	sprintf(musicfile, fileListEntry(0), track);
	delete[] _musicTrackData;

	_musicTrackData = _vm->resource()->fileData(musicfile, 0);
	_driver->loadMusicData(_musicTrackData, true);

	if (_musicEnabled == 2 && trackNum != -1) {
		AudioCD.play(trackNum+1, _driver->looping() ? -1 : 1, 0, 0);
		AudioCD.updateCD();
	} else if (_musicEnabled) {
		_driver->cont();
	}

	_lastTrack = track;
}

void SoundTownsPC98_v2::haltTrack() {
	_lastTrack = -1;
	AudioCD.stop();
	AudioCD.updateCD();
	_driver->reset();
}

void SoundTownsPC98_v2::beginFadeOut() {
	if (!_driver->musicPlaying())
		return;

	for (int i = 0; i < 20; i++) {
		_driver->fadeStep();
		_vm->delay(32);
	}

	haltTrack();
}

int32 SoundTownsPC98_v2::voicePlay(const char *file, bool) {
	static const uint16 rates[] =	{ 0x10E1, 0x0CA9, 0x0870, 0x0654, 0x0438, 0x032A, 0x021C, 0x0194 };

	int h = 0;
	if (_currentSFX) {
		while (_mixer->isSoundHandleActive(_soundChannels[h].channelHandle) && h < kNumChannelHandles)
			h++;
		if (h >= kNumChannelHandles)
			return 0;
	}

	char filename[13];
	sprintf(filename, "%s.PCM", file);

	uint8 *data = _vm->resource()->fileData(filename, 0);
	uint8 *src = data;

	uint16 sfxRate = rates[READ_LE_UINT16(src)];
	src += 2;
	bool compressed = (READ_LE_UINT16(src) & 1) ? true : false;
	src += 2;
	uint32 outsize = READ_LE_UINT32(src);
	uint8 *sfx = (uint8*) malloc(outsize);
	uint8 *dst = sfx;
	src += 4;

	if (compressed) {
		for (uint32 i = outsize; i;) {
			uint8 cnt = *src++;
			if (cnt & 0x80) {
				cnt &= 0x7F;
				memset(dst, *src++, cnt);
			} else {
				memcpy(dst, src, cnt);
				src += cnt;
			}
			dst += cnt;
			i -= cnt;
		}
	} else {
		memcpy(dst, src, outsize);
	}

	for (uint32 i = 0; i < outsize; i++) {
		uint8 cmd = sfx[i];
		if (cmd & 0x80) {
			cmd = ~cmd;
		} else {
			cmd |= 0x80;
			if (cmd == 0xff)
				cmd--;
		}
		if (cmd < 0x80)
			cmd = 0x80 - cmd;
		sfx[i] = cmd;
	}

	uint32 outputRate = uint32(11025 * SoundTowns::calculatePhaseStep(0x3c, 0x3c, sfxRate, 11025, 0x2000));

	_currentSFX = Audio::makeLinearInputStream(sfx, outsize, outputRate,
		Audio::Mixer::FLAG_UNSIGNED | Audio::Mixer::FLAG_LITTLE_ENDIAN | Audio::Mixer::FLAG_AUTOFREE, 0, 0);
	_soundChannels[h].file = file;
	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_soundChannels[h].channelHandle, _currentSFX);

	delete[] data;
	return 1;
}

void SoundTownsPC98_v2::playSoundEffect(uint8 track) {
	if (!_useFmSfx || !_sfxTrackData)
		return;

	_driver->loadSoundEffectData(_sfxTrackData, track);
}

// static resources

const uint8 TownsPC98_OpnDriver::_drvTables[] = {
	//	channel presets
	0x00, 0x80, 0x00, 0x00, 0x00, 0x01,
	0x01, 0x80, 0x01, 0x01, 0x00, 0x02,
	0x02, 0x80, 0x02, 0x02, 0x00, 0x04,
	0x00, 0x80, 0x03, 0x04, 0x01, 0x08,
	0x01, 0x80, 0x04, 0x05, 0x01, 0x10,
	0x02, 0x80, 0x05, 0x06, 0x01, 0x20,

	//	control event size
	0x01, 0x01, 0x01, 0x01,	0x01, 0x01, 0x04, 0x05,
	0x02, 0x06, 0x02, 0x00, 0x00, 0x02, 0x00, 0x02,

	//	fmt level presets
	0x54, 0x50,	0x4C, 0x48,	0x44, 0x40, 0x3C, 0x38,
	0x34, 0x30, 0x2C, 0x28, 0x24, 0x20, 0x1C, 0x18,
	0x14, 0x10, 0x0C, 0x08,	0x04, 0x90, 0x90, 0x90,

	//	carriers
	0x08, 0x08, 0x08, 0x08,	0x0C, 0x0E, 0x0E, 0x0F,

	//	frequencies
	0x6A, 0x02, 0x8F, 0x02, 0xB6, 0x02,	0xDF, 0x02,
	0x0B, 0x03, 0x39, 0x03, 0x6A, 0x03, 0x9E, 0x03,
	0xD5, 0x03,	0x10, 0x04, 0x4E, 0x04, 0x8F, 0x04,
	0x00, 0x00, 0x00, 0x00,

	//	unused
	0x01, 0x00,	0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
	0x02, 0x00,	0x00, 0x00,	0x05, 0x00, 0x00, 0x00,
	0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,

	//	detune
	0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x01, 0x01,	0x01, 0x01,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x03,	0x03, 0x03,
	0x04, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07,
	0x08, 0x08, 0x09, 0x0a,	0x0b, 0x0c, 0x0d, 0x0e,
	0x10, 0x10, 0x10, 0x10,	0x02, 0x02, 0x02, 0x02,
	0x02, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05,
	0x05, 0x06,	0x06, 0x07, 0x08, 0x08, 0x09, 0x0a,
	0x0b, 0x0c,	0x0d, 0x0e, 0x10, 0x11, 0x13, 0x14,
	0x16, 0x16, 0x16, 0x16,

	//	pc98 level presets
	0x40, 0x3B, 0x38, 0x34, 0x30, 0x2A, 0x28, 0x25,
	0x22, 0x20, 0x1D, 0x1A, 0x18, 0x15, 0x12, 0x10,
	0x0D, 0x0A, 0x08, 0x05, 0x02, 0x90, 0x90, 0x90,

	//	ssg frequencies
	0xE8, 0x0E, 0x12, 0x0E, 0x48, 0x0D, 0x89, 0x0C,
	0xD5, 0x0B, 0x2B, 0x0B, 0x8A, 0x0A, 0xF3, 0x09,
	0x64, 0x09, 0xDD, 0x08, 0x5E, 0x08, 0xE6, 0x07,

	// ssg patch data
	0x00, 0x00, 0xFF, 0xFF, 0x00, 0x81, 0x00, 0x00,
	0x00, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x37, 0x81, 0xC8, 0x00,
	0x00, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x37, 0x81, 0xC8, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xBE, 0x00,
	0x00, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xBE, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xBE, 0x00,
	0x04, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xBE, 0x00,
	0x0A, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0x01, 0x00,
	0xFF, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00,
	0xFF, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xFF, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x64, 0x01, 0xFF, 0x64, 0xFF, 0x81, 0xFF, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,

	0x02, 0x01, 0xFF, 0x28, 0xFF, 0x81, 0xF0, 0x00,
	0x00, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xC8, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x28, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0x78, 0x5F, 0x81, 0xA0, 0x00,
	0x05, 0x81, 0x00, 0x00, 0x28, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x00, 0x81, 0x00, 0x00,
	0x00, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x00, 0x81, 0x00, 0x00,
	0x00, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x00, 0x81, 0x00, 0x00,
	0x00, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00
};

const uint32 TownsPC98_OpnDriver::_adtStat[] = {
	0x00010001, 0x00010001,	0x00010001, 0x01010001,
	0x00010101, 0x00010101, 0x00010101, 0x01010101,
	0x01010101, 0x01010101, 0x01010102, 0x01010102,
	0x01020102, 0x01020102, 0x01020202, 0x01020202,
	0x02020202, 0x02020202, 0x02020204, 0x02020204,
	0x02040204, 0x02040204, 0x02040404, 0x02040404,
	0x04040404, 0x04040404, 0x04040408, 0x04040408,
	0x04080408, 0x04080408, 0x04080808, 0x04080808,
	0x08080808, 0x08080808, 0x10101010, 0x10101010
};

const int TownsPC98_OpnDriver::_ssgTables[] = {
	0x01202A, 0x0092D2, 0x006B42, 0x0053CB, 0x003DF8, 0x003053, 0x0022DA, 0x001A8C,
	0x00129B, 0x000DC1, 0x000963, 0x0006C9, 0x000463, 0x0002FA, 0x0001B6, 0x0000FB,
	0x0193B6, 0x01202A, 0x00CDB1, 0x0092D2, 0x007D7D, 0x006B42, 0x005ECD, 0x0053CB,
	0x00480F, 0x003DF8, 0x0036B9, 0x003053, 0x00290A, 0x0022DA, 0x001E6B, 0x001A8C,
	0x001639, 0x00129B, 0x000FFF, 0x000DC1, 0x000B5D, 0x000963, 0x0007FB, 0x0006C9,
	0x000575, 0x000463, 0x00039D, 0x0002FA, 0x000242, 0x0001B6, 0x00014C, 0x0000FB
};

} // end of namespace Kyra

#undef EUPHONY_FADEOUT_TICKS

