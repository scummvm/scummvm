/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 */

#ifndef SOUND_SOFTSYNTH_Y2612_H
#define SOUND_SOFTSYNTH_Y2612_H

#include "common/sys.h"

#include "sound/softsynth/emumidi.h"

////////////////////////////////////////
//
// Class declarations
//
////////////////////////////////////////

class Voice2612;
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
	void sysEx_customInstrument(uint32 type, const byte *instr);
};

class MidiDriver_YM2612 : public MidiDriver_Emulated {
protected:
	MidiChannel_YM2612 *_channel[16];

	int _next_voice;
	int _volume;

protected:
	void nextTick(int16 *buf1, int buflen);
	int volume(int val = -1) { if (val >= 0) _volume = val; return _volume; }
	void rate(uint16 r);

	void generateSamples(int16 *buf, int len);

public:
	MidiDriver_YM2612(Audio::Mixer *mixer);
	virtual ~MidiDriver_YM2612();

	static void createLookupTables();
	static void removeLookupTables();

	int open();
	void close();
	void send(uint32 b);
	void send(byte channel, uint32 b); // Supports higher than channel 15
	uint32 property(int prop, uint32 param) { return 0; }

	void setPitchBendRange(byte channel, uint range) { }
	void sysEx(const byte *msg, uint16 length);

	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }


	// AudioStream API
	bool isStereo() const { return true; }
	int getRate() const { return _mixer->getOutputRate(); }
};

#endif

