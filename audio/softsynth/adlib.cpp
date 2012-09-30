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
 */

#include "audio/softsynth/emumidi.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/types.h"
#include "common/util.h"
#include "audio/fmopl.h"
#include "audio/musicplugin.h"
#include "common/translation.h"

#ifdef DEBUG_ADLIB
static int g_tick;
#endif

class MidiDriver_ADLIB;
struct AdLibVoice;

// We use packing for the following two structs, because the code
// does simply copy them over from byte streams, without any
// serialization. Check AdLibPart::sysEx_customInstrument for an
// example of this.
//
// It might be very well possible, that none of the compilers we support
// add any padding bytes at all, since the structs contain only variables
// of the type 'byte'. But better safe than sorry.
#include "common/pack-start.h"
struct InstrumentExtra {
	byte a, b, c, d, e, f, g, h;
} PACKED_STRUCT;

struct AdLibInstrument {
	byte modCharacteristic;
	byte modScalingOutputLevel;
	byte modAttackDecay;
	byte modSustainRelease;
	byte modWaveformSelect;
	byte carCharacteristic;
	byte carScalingOutputLevel;
	byte carAttackDecay;
	byte carSustainRelease;
	byte carWaveformSelect;
	byte feedback;
	byte flagsA;
	InstrumentExtra extraA;
	byte flagsB;
	InstrumentExtra extraB;
	byte duration;
} PACKED_STRUCT;
#include "common/pack-end.h"

class AdLibPart : public MidiChannel {
	friend class MidiDriver_ADLIB;

protected:
//	AdLibPart *_prev, *_next;
	AdLibVoice *_voice;
	int16 _pitchBend;
	byte _pitchBendFactor;
	int8 _transposeEff;
	byte _volEff;
	int8 _detuneEff;
	byte _modWheel;
	bool _pedal;
	byte _program;
	byte _priEff;
	AdLibInstrument _partInstr;

protected:
	MidiDriver_ADLIB *_owner;
	bool _allocated;
	byte _channel;

	void init(MidiDriver_ADLIB *owner, byte channel);
	void allocate() { _allocated = true; }

public:
	AdLibPart() {
		_voice = 0;
		_pitchBend = 0;
		_pitchBendFactor = 2;
		_transposeEff = 0;
		_volEff = 0;
		_detuneEff = 0;
		_modWheel = 0;
		_pedal = 0;
		_program = 0;
		_priEff = 0;

		_owner = 0;
		_allocated = false;
		_channel = 0;

		memset(&_partInstr, 0, sizeof(_partInstr));
	}

	MidiDriver *device();
	byte getNumber() { return _channel; }
	void release() { _allocated = false; }

	void send(uint32 b);

	// Regular messages
	void noteOff(byte note);
	void noteOn(byte note, byte velocity);
	void programChange(byte program);
	void pitchBend(int16 bend);

	// Control Change messages
	void controlChange(byte control, byte value);
	void modulationWheel(byte value);
	void volume(byte value);
	void panPosition(byte value) { return; } // Not supported
	void pitchBendFactor(byte value);
	void detune(byte value);
	void priority(byte value);
	void sustain(bool value);
	void effectLevel(byte value) { return; } // Not supported
	void chorusLevel(byte value) { return; } // Not supported
	void allNotesOff();

	// SysEx messages
	void sysEx_customInstrument(uint32 type, const byte *instr);
};

// FYI (Jamieson630)
// It is assumed that any invocation to AdLibPercussionChannel
// will be done through the MidiChannel base class as opposed to the
// AdLibPart base class. If this were NOT the case, all the functions
// listed below would need to be virtual in AdLibPart as well as MidiChannel.
class AdLibPercussionChannel : public AdLibPart {
	friend class MidiDriver_ADLIB;

protected:
	void init(MidiDriver_ADLIB *owner, byte channel);

public:
	~AdLibPercussionChannel();

	void noteOff(byte note);
	void noteOn(byte note, byte velocity);
	void programChange(byte program) { }
	void pitchBend(int16 bend) { }

	// Control Change messages
	void modulationWheel(byte value) { }
	void pitchBendFactor(byte value) { }
	void detune(byte value) { }
	void priority(byte value) { }
	void sustain(bool value) { }

	// SysEx messages
	void sysEx_customInstrument(uint32 type, const byte *instr);

private:
	byte _notes[256];
	AdLibInstrument *_customInstruments[256];
};

struct Struct10 {
	byte active;
	int16 curVal;
	int16 count;
	uint16 maxValue;
	int16 startValue;
	byte loop;
	byte tableA[4];
	byte tableB[4];
	int8 unk3;
	int8 modWheel;
	int8 modWheelLast;
	uint16 speedLoMax;
	uint16 numSteps;
	int16 speedHi;
	int8 direction;
	uint16 speedLo;
	uint16 speedLoCounter;
};

struct Struct11 {
	int16 modifyVal;
	byte param, flag0x40, flag0x10;
	Struct10 *s10;
};

struct AdLibVoice {
	AdLibPart *_part;
	AdLibVoice *_next, *_prev;
	byte _waitForPedal;
	byte _note;
	byte _channel;
	byte _twoChan;
	byte _vol1, _vol2;
	int16 _duration;

	Struct10 _s10a;
	Struct11 _s11a;
	Struct10 _s10b;
	Struct11 _s11b;

	AdLibVoice() { memset(this, 0, sizeof(AdLibVoice)); }
};

struct AdLibSetParams {
	byte a, b, c, d;
};

static const byte g_operator1Offsets[9] = {
	0, 1, 2, 8,
	9, 10, 16, 17,
	18
};

static const byte g_operator2Offsets[9] = {
	3, 4, 5, 11,
	12, 13, 19, 20,
	21
};

static const AdLibSetParams g_setParamTable[] = {
	{0x40, 0, 63, 63},  // level
	{0xE0, 2, 0, 0},    // unused
	{0x40, 6, 192, 0},  // level key scaling
	{0x20, 0, 15, 0},   // modulator frequency multiple
	{0x60, 4, 240, 15}, // attack rate
	{0x60, 0, 15, 15},  // decay rate
	{0x80, 4, 240, 15}, // sustain level
	{0x80, 0, 15, 15},  // release rate
	{0xE0, 0, 3, 0},    // waveformSelect select
	{0x20, 7, 128, 0},  // amp mod
	{0x20, 6, 64, 0},   // vib
	{0x20, 5, 32, 0},   // eg typ
	{0x20, 4, 16, 0},   // ksr
	{0xC0, 0, 1, 0},    // decay alg
	{0xC0, 1, 14, 0}    // feedback
};

static const byte g_paramTable1[16] = {
	29, 28, 27, 0,
	3, 4, 7, 8,
	13, 16, 17, 20,
	21, 30, 31, 0
};

static const uint16 g_maxValTable[16] = {
	0x2FF, 0x1F, 0x7, 0x3F,
	0x0F, 0x0F, 0x0F, 0x3,
	0x3F, 0x0F, 0x0F, 0x0F,
	0x3, 0x3E, 0x1F, 0
};

static const uint16 g_numStepsTable[] = {
	1, 2, 4, 5,
	6, 7, 8, 9,
	10, 12, 14, 16,
	18, 21, 24, 30,
	36, 50, 64, 82,
	100, 136, 160, 192,
	240, 276, 340, 460,
	600, 860, 1200, 1600
};

static const byte g_noteFrequencies[] = {
	90, 91, 92, 92, 93, 94, 94, 95,
	96, 96, 97, 98, 98, 99, 100, 101,
	101, 102, 103, 104, 104, 105, 106, 107,
	107, 108, 109, 110, 111, 111, 112, 113,
	114, 115, 115, 116, 117, 118, 119, 120,
	121, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 132, 133, 134,
	135, 136, 137, 138, 139, 140, 141, 142,
	143, 145, 146, 147, 148, 149, 150, 151,
	152, 153, 154, 155, 157, 158, 159, 160,
	161, 162, 163, 165, 166, 167, 168, 169,
	171, 172, 173, 174, 176, 177, 178, 180,
	181, 182, 184, 185, 186, 188, 189, 190,
	192, 193, 194, 196, 197, 199, 200, 202,
	203, 205, 206, 208, 209, 211, 212, 214,
	215, 217, 218, 220, 222, 223, 225, 226,
	228, 230, 231, 233, 235, 236, 238, 240,
	242, 243, 245, 247, 249, 251, 252, 254
};

static const AdLibInstrument g_gmInstruments[128] = {
	// 0x00
	{ 0xC2, 0xC5, 0x2B, 0x99, 0x58, 0xC2, 0x1F, 0x1E, 0xC8, 0x7C, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x23 },
	{ 0x22, 0x53, 0x0E, 0x8A, 0x30, 0x14, 0x06, 0x1D, 0x7A, 0x5C, 0x06, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x06, 0x00, 0x1C, 0x79, 0x40, 0x02, 0x00, 0x4B, 0x79, 0x58, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xC2, 0x89, 0x2A, 0x89, 0x49, 0xC2, 0x16, 0x1C, 0xB8, 0x7C, 0x04, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x23 },
	{ 0xC2, 0x17, 0x3D, 0x6A, 0x00, 0xC4, 0x2E, 0x2D, 0xC9, 0x20, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x06, 0x1E, 0x1C, 0x99, 0x00, 0x02, 0x3A, 0x4C, 0x79, 0x00, 0x0C, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x84, 0x40, 0x3B, 0x5A, 0x6F, 0x81, 0x0E, 0x3B, 0x5A, 0x7F, 0x0B, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x84, 0x40, 0x3B, 0x5A, 0x63, 0x81, 0x00, 0x3B, 0x5A, 0x7F, 0x01, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x8C, 0x80, 0x05, 0xEA, 0x59, 0x82, 0x0A, 0x3C, 0xAA, 0x64, 0x07, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x85, 0x40, 0x0D, 0xEC, 0x71, 0x84, 0x58, 0x3E, 0xCB, 0x7C, 0x01, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x8A, 0xC0, 0x0C, 0xDC, 0x50, 0x88, 0x58, 0x3D, 0xDA, 0x7C, 0x01, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xC9, 0x40, 0x2B, 0x78, 0x42, 0xC2, 0x04, 0x4C, 0x8A, 0x7C, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x1A },
	{ 0x2A, 0x0E, 0x17, 0x89, 0x28, 0x22, 0x0C, 0x1B, 0x09, 0x70, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE7, 0x9B, 0x08, 0x08, 0x26, 0xE2, 0x06, 0x0A, 0x08, 0x70, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xC5, 0x05, 0x00, 0xFC, 0x40, 0x84, 0x00, 0x00, 0xDC, 0x50, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x86, 0x40, 0x5D, 0x5A, 0x41, 0x81, 0x00, 0x0B, 0x5A, 0x7F, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	// 0x10
	{ 0xED, 0x00, 0x7B, 0xC8, 0x40, 0xE1, 0x99, 0x4A, 0xE9, 0x7E, 0x07, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE8, 0x4F, 0x3A, 0xD7, 0x7C, 0xE2, 0x97, 0x49, 0xF9, 0x7D, 0x05, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE1, 0x10, 0x2F, 0xF7, 0x7D, 0xF3, 0x45, 0x8F, 0xC7, 0x62, 0x07, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x01, 0x8C, 0x9F, 0xDA, 0x70, 0xE4, 0x50, 0x9F, 0xDA, 0x6A, 0x09, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x08, 0xD5, 0x9D, 0xA5, 0x45, 0xE2, 0x3F, 0x9F, 0xD6, 0x49, 0x07, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE5, 0x0F, 0x7D, 0xB8, 0x2E, 0xA2, 0x0F, 0x7C, 0xC7, 0x61, 0x04, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xF2, 0x2A, 0x9F, 0xDB, 0x01, 0xE1, 0x04, 0x8F, 0xD7, 0x62, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE4, 0x88, 0x9C, 0x50, 0x64, 0xE2, 0x18, 0x70, 0xC4, 0x7C, 0x0B, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x02, 0xA3, 0x0D, 0xDA, 0x01, 0xC2, 0x35, 0x5D, 0x58, 0x00, 0x06, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x18 },
	{ 0x42, 0x55, 0x3E, 0xEB, 0x24, 0xD4, 0x08, 0x0D, 0xA9, 0x71, 0x04, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x18 },
	{ 0xC2, 0x00, 0x2B, 0x17, 0x51, 0xC2, 0x1E, 0x4D, 0x97, 0x7C, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x19 },
	{ 0xC6, 0x01, 0x2D, 0xA7, 0x44, 0xC2, 0x06, 0x0E, 0xA7, 0x79, 0x06, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xC2, 0x0C, 0x06, 0x06, 0x55, 0xC2, 0x3F, 0x09, 0x86, 0x7D, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x0A },
	{ 0xC2, 0x2E, 0x4F, 0x77, 0x00, 0xC4, 0x08, 0x0E, 0x98, 0x59, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xC2, 0x30, 0x4F, 0xCA, 0x01, 0xC4, 0x0D, 0x0E, 0xB8, 0x7F, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xC4, 0x29, 0x4F, 0xCA, 0x03, 0xC8, 0x0D, 0x0C, 0xB7, 0x7D, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x0B },
	// 0x20
	{ 0xC2, 0x40, 0x3C, 0x96, 0x58, 0xC4, 0xDE, 0x0E, 0xC7, 0x7C, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x20 },
	{ 0x31, 0x13, 0x2D, 0xD7, 0x3C, 0xE2, 0x18, 0x2E, 0xB8, 0x7C, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x22, 0x86, 0x0D, 0xD7, 0x50, 0xE4, 0x18, 0x5E, 0xB8, 0x7C, 0x06, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x28 },
	{ 0xF2, 0x0A, 0x0D, 0xD7, 0x40, 0xE4, 0x1F, 0x5E, 0xB8, 0x7C, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xF2, 0x09, 0x4B, 0xD6, 0x48, 0xE4, 0x1F, 0x1C, 0xB8, 0x7C, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x28 },
	{ 0x62, 0x11, 0x0C, 0xE6, 0x3C, 0xE4, 0x1F, 0x0C, 0xC8, 0x7C, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE2, 0x12, 0x3D, 0xE6, 0x34, 0xE4, 0x1F, 0x7D, 0xB8, 0x7C, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE2, 0x13, 0x3D, 0xE6, 0x34, 0xE4, 0x1F, 0x5D, 0xB8, 0x7D, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xA2, 0x40, 0x5D, 0xBA, 0x3F, 0xE2, 0x00, 0x8F, 0xD8, 0x79, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE2, 0x40, 0x3D, 0xDA, 0x3B, 0xE1, 0x00, 0x7E, 0xD8, 0x7A, 0x04, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x62, 0x00, 0x6D, 0xFA, 0x5D, 0xE2, 0x00, 0x8F, 0xC8, 0x79, 0x04, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE1, 0x00, 0x4E, 0xDB, 0x4A, 0xE3, 0x18, 0x6F, 0xE9, 0x7E, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE1, 0x00, 0x4E, 0xDB, 0x66, 0xE2, 0x00, 0x7F, 0xE9, 0x7E, 0x06, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x02, 0x0F, 0x66, 0xAA, 0x51, 0x02, 0x64, 0x29, 0xF9, 0x7C, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x04 },
	{ 0x16, 0x4A, 0x04, 0xBA, 0x39, 0xC2, 0x58, 0x2D, 0xCA, 0x7C, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x03 },
	{ 0x02, 0x00, 0x01, 0x7A, 0x79, 0x02, 0x3F, 0x28, 0xEA, 0x7C, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	// 0x30
	{ 0x62, 0x53, 0x9C, 0xBA, 0x31, 0x62, 0x5B, 0xAD, 0xC9, 0x55, 0x04, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xF2, 0x40, 0x6E, 0xDA, 0x49, 0xE2, 0x13, 0x8F, 0xF9, 0x7D, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE2, 0x40, 0x8F, 0xFA, 0x50, 0xF2, 0x04, 0x7F, 0xFA, 0x7D, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE4, 0xA0, 0xCE, 0x5B, 0x02, 0xE2, 0x32, 0x7F, 0xFB, 0x3D, 0x04, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE6, 0x80, 0x9C, 0x99, 0x42, 0xE2, 0x04, 0x7D, 0x78, 0x60, 0x04, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xEA, 0xA0, 0xAC, 0x67, 0x02, 0xE2, 0x00, 0x7C, 0x7A, 0x7C, 0x06, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE7, 0x94, 0xAD, 0xB7, 0x03, 0xE2, 0x00, 0x7C, 0xBA, 0x7C, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xC3, 0x3F, 0x4B, 0xE9, 0x7E, 0xC1, 0x3F, 0x9B, 0xF9, 0x7F, 0x0B, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x06 },
	{ 0xB2, 0x20, 0xAD, 0xE9, 0x00, 0x62, 0x05, 0x8F, 0xC8, 0x68, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xF2, 0x00, 0x8F, 0xFB, 0x50, 0xF6, 0x47, 0x8F, 0xE9, 0x68, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xF2, 0x00, 0xAF, 0x88, 0x58, 0xF2, 0x54, 0x6E, 0xC9, 0x7C, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xF2, 0x2A, 0x9F, 0x98, 0x01, 0xE2, 0x84, 0x4E, 0x78, 0x6C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE2, 0x02, 0x9F, 0xB8, 0x48, 0x22, 0x89, 0x9F, 0xE8, 0x7C, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE2, 0x2A, 0x7F, 0xB8, 0x01, 0xE4, 0x00, 0x0D, 0xC5, 0x7C, 0x0C, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE4, 0x28, 0x8E, 0xE8, 0x01, 0xF2, 0x00, 0x4D, 0xD6, 0x7D, 0x0C, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x62, 0x23, 0x8F, 0xEA, 0x00, 0xF2, 0x00, 0x5E, 0xD9, 0x7C, 0x0C, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	// 0x40
	{ 0xB4, 0x26, 0x6E, 0x98, 0x01, 0x62, 0x00, 0x7D, 0xC8, 0x7D, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE2, 0x2E, 0x20, 0xD9, 0x01, 0xF2, 0x0F, 0x90, 0xF8, 0x78, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE4, 0x28, 0x7E, 0xF8, 0x01, 0xE2, 0x23, 0x8E, 0xE8, 0x7D, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xB8, 0x28, 0x9E, 0x98, 0x01, 0x62, 0x00, 0x3D, 0xC8, 0x7D, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x62, 0x00, 0x8E, 0xC9, 0x3D, 0xE6, 0x00, 0x7E, 0xD8, 0x68, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE2, 0x00, 0x5F, 0xF9, 0x48, 0xE6, 0x98, 0x8F, 0xF8, 0x7D, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x62, 0x0C, 0x6E, 0xD8, 0x3D, 0x2A, 0x06, 0x7D, 0xD8, 0x58, 0x04, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE4, 0x00, 0x7E, 0x89, 0x38, 0xE6, 0x84, 0x80, 0xF8, 0x68, 0x0C, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE4, 0x80, 0x6C, 0xD9, 0x30, 0xE2, 0x00, 0x8D, 0xC8, 0x7C, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE2, 0x80, 0x88, 0x48, 0x40, 0xE2, 0x0A, 0x7D, 0xA8, 0x7C, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE4, 0x00, 0x77, 0xC5, 0x54, 0xE2, 0x00, 0x9E, 0xD7, 0x70, 0x06, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE4, 0x80, 0x86, 0xB9, 0x64, 0xE2, 0x05, 0x9F, 0xD7, 0x78, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE2, 0x00, 0x68, 0x68, 0x56, 0xE2, 0x08, 0x9B, 0xB3, 0x7C, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE4, 0x00, 0xA6, 0x87, 0x41, 0xE2, 0x0A, 0x7E, 0xC9, 0x7C, 0x06, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE4, 0x80, 0x9A, 0xB8, 0x48, 0xE2, 0x00, 0x9E, 0xF9, 0x60, 0x09, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE2, 0x80, 0x8E, 0x64, 0x68, 0xE2, 0x28, 0x6F, 0x73, 0x7C, 0x01, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	// 0x50
	{ 0xE8, 0x00, 0x7D, 0x99, 0x54, 0xE6, 0x80, 0x80, 0xF8, 0x7C, 0x0C, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE6, 0x00, 0x9F, 0xB9, 0x6D, 0xE1, 0x00, 0x8F, 0xC8, 0x7D, 0x02, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE4, 0x00, 0x09, 0x68, 0x4A, 0xE2, 0x2B, 0x9E, 0xF3, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xC4, 0x00, 0x99, 0xE8, 0x3B, 0xE2, 0x25, 0x6F, 0x93, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE6, 0x00, 0x6F, 0xDA, 0x69, 0xE2, 0x05, 0x2F, 0xD8, 0x6A, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xEC, 0x60, 0x9D, 0xC7, 0x00, 0xE2, 0x21, 0x7F, 0xC9, 0x7C, 0x06, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE3, 0x00, 0x0F, 0xF7, 0x7D, 0xE1, 0x3F, 0x0F, 0xA7, 0x01, 0x0D, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE4, 0xA9, 0x0F, 0xA8, 0x02, 0xE2, 0x3C, 0x5F, 0xDA, 0x3C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE8, 0x40, 0x0D, 0x89, 0x7D, 0xE2, 0x17, 0x7E, 0xD9, 0x7C, 0x07, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE1, 0x00, 0xDF, 0x8A, 0x56, 0xE2, 0x5E, 0xCF, 0xBA, 0x7E, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE2, 0x00, 0x0B, 0x68, 0x60, 0xE2, 0x01, 0x9E, 0xB8, 0x7C, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xEA, 0x00, 0xAE, 0xAB, 0x49, 0xE2, 0x00, 0xAE, 0xBA, 0x6C, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xEB, 0x80, 0x8C, 0xCB, 0x3A, 0xE2, 0x86, 0xAF, 0xCA, 0x7C, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE5, 0x40, 0xDB, 0x3B, 0x3C, 0xE2, 0x80, 0xBE, 0xCA, 0x71, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE4, 0x00, 0x9E, 0xAA, 0x3D, 0xE1, 0x43, 0x0F, 0xBA, 0x7E, 0x04, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE7, 0x40, 0xEC, 0xCA, 0x44, 0xE2, 0x03, 0xBF, 0xBA, 0x66, 0x02, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	// 0x60
	{ 0xEA, 0x00, 0x68, 0xB8, 0x48, 0xE2, 0x0A, 0x8E, 0xB8, 0x7C, 0x0C, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x61, 0x00, 0xBE, 0x99, 0x7E, 0xE3, 0x40, 0xCF, 0xCA, 0x7D, 0x09, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xCD, 0x00, 0x0B, 0x00, 0x48, 0xC2, 0x58, 0x0C, 0x00, 0x7C, 0x0C, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x1C },
	{ 0xE2, 0x00, 0x0E, 0x00, 0x52, 0xE2, 0x58, 0x5F, 0xD0, 0x7D, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xCC, 0x00, 0x7D, 0xDA, 0x40, 0xC2, 0x00, 0x5E, 0x9B, 0x58, 0x0C, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE9, 0xC0, 0xEE, 0xD8, 0x43, 0xE2, 0x05, 0xDD, 0xAA, 0x70, 0x06, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xDA, 0x00, 0x8F, 0xAC, 0x4A, 0x22, 0x05, 0x8D, 0x8A, 0x75, 0x02, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x62, 0x8A, 0xCB, 0x7A, 0x74, 0xE6, 0x56, 0xAF, 0xDB, 0x70, 0x02, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xC2, 0x41, 0xAC, 0x5B, 0x5B, 0xC2, 0x80, 0x0D, 0xCB, 0x7D, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x12 },
	{ 0x75, 0x00, 0x0E, 0xCB, 0x5A, 0xE2, 0x1E, 0x0A, 0xC9, 0x7D, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x10 },
	{ 0x41, 0x00, 0x0E, 0xEA, 0x53, 0xC2, 0x00, 0x08, 0xCA, 0x7C, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x07 },
	{ 0xC1, 0x40, 0x0C, 0x59, 0x6A, 0xC2, 0x80, 0x3C, 0xAB, 0x7C, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x0D },
	{ 0x4B, 0x00, 0x0A, 0xF5, 0x61, 0xC2, 0x19, 0x0C, 0xE9, 0x7C, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x07 },
	{ 0x62, 0x00, 0x7F, 0xD8, 0x54, 0xEA, 0x00, 0x8F, 0xD8, 0x7D, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE1, 0x00, 0x7F, 0xD9, 0x56, 0xE1, 0x00, 0x8F, 0xD8, 0x7E, 0x06, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0xE1, 0x00, 0x7F, 0xD9, 0x56, 0xE1, 0x00, 0x8F, 0xD8, 0x7E, 0x06, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	// 0x70
	{ 0xCF, 0x40, 0x09, 0xEA, 0x54, 0xC4, 0x00, 0x0C, 0xDB, 0x64, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0xCF, 0x40, 0x0C, 0xAA, 0x54, 0xC4, 0x00, 0x18, 0xF9, 0x64, 0x0C, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0xC9, 0x0E, 0x88, 0xD9, 0x3E, 0xC2, 0x08, 0x1A, 0xEA, 0x6C, 0x0C, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x05 },
	{ 0x03, 0x00, 0x15, 0x00, 0x64, 0x02, 0x00, 0x08, 0x00, 0x7C, 0x09, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0x01, 0x00, 0x47, 0xD7, 0x6C, 0x01, 0x3F, 0x0C, 0xFB, 0x7C, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x04 },
	{ 0x00, 0x00, 0x36, 0x67, 0x7C, 0x01, 0x3F, 0x0E, 0xFA, 0x7C, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x05 },
	{ 0x02, 0x00, 0x36, 0x68, 0x7C, 0x01, 0x3F, 0x0E, 0xFA, 0x7C, 0x00, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x05 },
	{ 0xCB, 0x00, 0xAF, 0x00, 0x7E, 0xC0, 0x00, 0xC0, 0x06, 0x7F, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x0F },
	{ 0x05, 0x0D, 0x80, 0xA6, 0x7F, 0x0B, 0x38, 0xA9, 0xD8, 0x00, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x04 },
	{ 0x0F, 0x00, 0x90, 0xFA, 0x68, 0x06, 0x00, 0xA7, 0x39, 0x54, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x06 },
	{ 0xC9, 0x15, 0xDD, 0xFF, 0x7C, 0x00, 0x00, 0xE7, 0xFC, 0x6C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x38 },
	{ 0x48, 0x3C, 0x30, 0xF6, 0x03, 0x0A, 0x38, 0x97, 0xE8, 0x00, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x04 },
	{ 0x07, 0x80, 0x0B, 0xC8, 0x65, 0x02, 0x3F, 0x0C, 0xEA, 0x7C, 0x0F, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x05 },
	{ 0x00, 0x21, 0x66, 0x40, 0x03, 0x00, 0x3F, 0x47, 0x00, 0x00, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0x08, 0x00, 0x0B, 0x3C, 0x7C, 0x08, 0x3F, 0x06, 0xF3, 0x00, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0x00, 0x3F, 0x4C, 0xFB, 0x00, 0x00, 0x3F, 0x0A, 0xE9, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x05 }
};

static AdLibInstrument g_gmPercussionInstruments[39] = {
	{ 0x1A, 0x3F, 0x15, 0x05, 0x7C, 0x02, 0x21, 0x2B, 0xE4, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x06 },
	{ 0x11, 0x12, 0x04, 0x07, 0x7C, 0x02, 0x23, 0x0B, 0xE5, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x05 },
	{ 0x0A, 0x3F, 0x0B, 0x01, 0x7C, 0x1F, 0x1C, 0x46, 0xD0, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x01 },
	{ 0x00, 0x3F, 0x0F, 0x00, 0x7C, 0x10, 0x12, 0x07, 0x00, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0x0F, 0x3F, 0x0B, 0x00, 0x7C, 0x1F, 0x0F, 0x19, 0xD0, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0x00, 0x3F, 0x1F, 0x00, 0x7E, 0x1F, 0x16, 0x07, 0x00, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x03 },
	{ 0x12, 0x3F, 0x05, 0x06, 0x7C, 0x03, 0x1F, 0x4A, 0xD9, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x03 },
	{ 0xCF, 0x7F, 0x08, 0xFF, 0x7E, 0x00, 0xC7, 0x2D, 0xF7, 0x73, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0x12, 0x3F, 0x05, 0x06, 0x7C, 0x43, 0x21, 0x0C, 0xE9, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x03 },
	{ 0xCF, 0x7F, 0x08, 0xCF, 0x7E, 0x00, 0x45, 0x2A, 0xF8, 0x4B, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x0C },
	{ 0x12, 0x3F, 0x06, 0x17, 0x7C, 0x03, 0x27, 0x0B, 0xE9, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x03 },
	{ 0xCF, 0x7F, 0x08, 0xCD, 0x7E, 0x00, 0x40, 0x1A, 0x69, 0x63, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x0C },
	{ 0x13, 0x3F, 0x05, 0x06, 0x7C, 0x03, 0x17, 0x0A, 0xD9, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x03 },
	{ 0x15, 0x3F, 0x05, 0x06, 0x7C, 0x03, 0x21, 0x0C, 0xE9, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x03 },
	{ 0xCF, 0x3F, 0x2B, 0xFB, 0x7E, 0xC0, 0x1E, 0x1A, 0xCA, 0x7F, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x10 },
	{ 0x17, 0x3F, 0x04, 0x09, 0x7C, 0x03, 0x22, 0x0D, 0xE9, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x03 },
	{ 0xCF, 0x3F, 0x0F, 0x5E, 0x7C, 0xC6, 0x13, 0x00, 0xCA, 0x7F, 0x04, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x03 },
	{ 0xCF, 0x3F, 0x7E, 0x9D, 0x7C, 0xC8, 0xC0, 0x0A, 0xBA, 0x74, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x06 },
	{ 0xCF, 0x3F, 0x4D, 0x9F, 0x7C, 0xC6, 0x00, 0x08, 0xDA, 0x5B, 0x04, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x04 },
	{ 0xCF, 0x3F, 0x5D, 0xAA, 0x7A, 0xC0, 0xA4, 0x67, 0x99, 0x7C, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0xCF, 0x3F, 0x4A, 0xFD, 0x7C, 0xCF, 0x00, 0x59, 0xEA, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0x0F, 0x18, 0x0A, 0xFA, 0x57, 0x06, 0x07, 0x06, 0x39, 0x7C, 0x0A, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0xCF, 0x3F, 0x2B, 0xFC, 0x7C, 0xCC, 0xC6, 0x0B, 0xEA, 0x7F, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x10 },
	{ 0x05, 0x1A, 0x04, 0x00, 0x7C, 0x12, 0x10, 0x0C, 0xEA, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x07 },
	{ 0x04, 0x19, 0x04, 0x00, 0x7C, 0x12, 0x10, 0x2C, 0xEA, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x04 },
	{ 0x04, 0x0A, 0x04, 0x00, 0x6C, 0x01, 0x07, 0x0D, 0xFA, 0x74, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x07 },
	{ 0x15, 0x14, 0x05, 0x00, 0x7D, 0x01, 0x07, 0x5C, 0xE9, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x05 },
	{ 0x10, 0x10, 0x05, 0x08, 0x7C, 0x01, 0x08, 0x0D, 0xEA, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x05 },
	{ 0x11, 0x00, 0x06, 0x87, 0x7F, 0x02, 0x40, 0x09, 0x59, 0x68, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x08 },
	{ 0x13, 0x26, 0x04, 0x6A, 0x7F, 0x01, 0x00, 0x08, 0x5A, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x08 },
	{ 0xCF, 0x4E, 0x0C, 0xAA, 0x50, 0xC4, 0x00, 0x18, 0xF9, 0x54, 0x04, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0xCF, 0x4E, 0x0C, 0xAA, 0x50, 0xC3, 0x00, 0x18, 0xF8, 0x54, 0x04, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0xCB, 0x3F, 0x8F, 0x00, 0x7E, 0xC5, 0x00, 0x98, 0xD6, 0x5F, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x0D },
	{ 0x0C, 0x18, 0x87, 0xB3, 0x7F, 0x19, 0x10, 0x55, 0x75, 0x7C, 0x0E, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0x05, 0x11, 0x15, 0x00, 0x64, 0x02, 0x08, 0x08, 0x00, 0x5C, 0x09, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0x04, 0x08, 0x15, 0x00, 0x48, 0x01, 0x08, 0x08, 0x00, 0x60, 0x08, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x02 },
	{ 0xDA, 0x00, 0x53, 0x30, 0x68, 0x07, 0x1E, 0x49, 0xC4, 0x7E, 0x03, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 },
	{ 0x1C, 0x00, 0x07, 0xBC, 0x6C, 0x0C, 0x14, 0x0B, 0x6A, 0x7E, 0x0B, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x03 },
	{ 0x0A, 0x0E, 0x7F, 0x00, 0x7D, 0x13, 0x20, 0x28, 0x03, 0x7C, 0x06, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0x00 }
};

static const byte g_gmPercussionInstrumentMap[128] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C,
	0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0xFF, 0xFF, 0x17, 0x18, 0x19, 0x1A,
	0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x21, 0x22, 0x23, 0xFF, 0xFF,
	0x24, 0x25, 0xFF, 0xFF, 0xFF, 0x26, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static byte g_volumeLookupTable[64][32];

static const byte g_volumeTable[] = {
	0, 4, 7, 11,
	13, 16, 18, 20,
	22, 24, 26, 27,
	29, 30, 31, 33,
	34, 35, 36, 37,
	38, 39, 40, 41,
	42, 43, 44, 44,
	45, 46, 47, 47,
	48, 49, 49, 50,
	51, 51, 52, 53,
	53, 54, 54, 55,
	55, 56, 56, 57,
	57, 58, 58, 59,
	59, 60, 60, 60,
	61, 61, 62, 62,
	62, 63, 63, 63
};

static int lookupVolume(int a, int b) {
	if (b == 0)
		return 0;

	if (b == 31)
		return a;

	if (a < -63 || a > 63) {
		return b * (a + 1) >> 5;
	}

	if (b < 0) {
		if (a < 0) {
			return g_volumeLookupTable[-a][-b];
		} else {
			return -g_volumeLookupTable[a][-b];
		}
	} else {
		if (a < 0) {
			return -g_volumeLookupTable[-a][b];
		} else {
			return g_volumeLookupTable[a][b];
		}
	}
}

static void createLookupTable() {
	int i, j;
	int sum;

	for (i = 0; i < 64; i++) {
		sum = i;
		for (j = 0; j < 32; j++) {
			g_volumeLookupTable[i][j] = sum >> 5;
			sum += i;
		}
	}
	for (i = 0; i < 64; i++)
		g_volumeLookupTable[i][0] = 0;
}

////////////////////////////////////////
//
// AdLib MIDI driver
//
////////////////////////////////////////

class MidiDriver_ADLIB : public MidiDriver_Emulated {
	friend class AdLibPart;
	friend class AdLibPercussionChannel;

public:
	MidiDriver_ADLIB(Audio::Mixer *mixer);

	int open();
	void close();
	void send(uint32 b);
	void send(byte channel, uint32 b); // Supports higher than channel 15
	uint32 property(int prop, uint32 param);

	void setPitchBendRange(byte channel, uint range);
	void sysEx_customInstrument(byte channel, uint32 type, const byte *instr);

	MidiChannel *allocateChannel();
	MidiChannel *getPercussionChannel() { return &_percussion; } // Percussion partially supported


	// AudioStream API
	bool isStereo() const { return false; }
	int getRate() const { return _mixer->getOutputRate(); }

private:
	bool _scummSmallHeader; // FIXME: This flag controls a special mode for SCUMM V3 games

	FM_OPL *_opl;
	byte *_regCache;

	int _timerCounter;

	uint16 _channelTable2[9];
	int _voiceIndex;
	int _timerIncrease;
	int _timerThreshold;
	uint16 _curNotTable[9];
	AdLibVoice _voices[9];
	AdLibPart _parts[32];
	AdLibPercussionChannel _percussion;

	void generateSamples(int16 *buf, int len);
	void onTimer();
	void partKeyOn(AdLibPart *part, AdLibInstrument *instr, byte note, byte velocity);
	void partKeyOff(AdLibPart *part, byte note);

	void adlibKeyOff(int chan);
	void adlibNoteOn(int chan, byte note, int mod);
	void adlibNoteOnEx(int chan, byte note, int mod);
	int adlibGetRegValueParam(int chan, byte data);
	void adlibSetupChannel(int chan, AdLibInstrument *instr, byte vol1, byte vol2);
	byte adlibGetRegValue(byte reg) {
		return _regCache[reg];
	}
	void adlibSetParam(int channel, byte param, int value);
	void adlibKeyOnOff(int channel);
	void adlibWrite(byte reg, byte value);
	void adlibPlayNote(int channel, int note);

	AdLibVoice *allocateVoice(byte pri);

	void mcOff(AdLibVoice *voice);

	static void linkMc(AdLibPart *part, AdLibVoice *voice);
	void mcIncStuff(AdLibVoice *voice, Struct10 *s10, Struct11 *s11);
	void mcInitStuff(AdLibVoice *voice, Struct10 *s10, Struct11 *s11, byte flags,
					   InstrumentExtra *ie);

	void struct10Init(Struct10 *s10, InstrumentExtra *ie);
	static byte struct10OnTimer(Struct10 *s10, Struct11 *s11);
	static void struct10Setup(Struct10 *s10);
	static int randomNr(int a);
	void mcKeyOn(AdLibVoice *voice, AdLibInstrument *instr, byte note, byte velocity);
};

// MidiChannel method implementations

void AdLibPart::init(MidiDriver_ADLIB *owner, byte channel) {
	_owner = owner;
	_channel = channel;
	_priEff = 127;
	programChange(0);
}

MidiDriver *AdLibPart::device() {
	return _owner;
}

void AdLibPart::send(uint32 b) {
	_owner->send(_channel, b);
}

void AdLibPart::noteOff(byte note) {
#ifdef DEBUG_ADLIB
	debug(6, "%10d: noteOff(%d)", g_tick, note);
#endif
	_owner->partKeyOff(this, note);
}

void AdLibPart::noteOn(byte note, byte velocity) {
#ifdef DEBUG_ADLIB
	debug(6, "%10d: noteOn(%d,%d)", g_tick, note, velocity);
#endif
	_owner->partKeyOn(this, &_partInstr, note, velocity);
}

void AdLibPart::programChange(byte program) {
	if (program > 127)
		return;

	/*
	uint i;
	uint count = 0;
	for (i = 0; i < ARRAYSIZE(g_gmInstruments[0]); ++i)
		count += g_gmInstruments[program][i];
	if (!count)
		warning("No AdLib instrument defined for GM program %d", (int)program);
	*/
	_program = program;
	memcpy(&_partInstr, &g_gmInstruments[program], sizeof(AdLibInstrument));
}

void AdLibPart::pitchBend(int16 bend) {
	AdLibVoice *voice;

	_pitchBend = bend;
	for (voice = _voice; voice; voice = voice->_next) {
		_owner->adlibNoteOn(voice->_channel, voice->_note + _transposeEff,
							  (_pitchBend * _pitchBendFactor >> 6) + _detuneEff);
	}
}

void AdLibPart::controlChange(byte control, byte value) {
	switch (control) {
	case 0:
	case 32:
		// Bank select. Not supported
		break;
	case 1:
		modulationWheel(value);
		break;
	case 7:
		volume(value);
		break;
	case 10:
		// Pan position. Not supported.
		break;
	case 16:
		pitchBendFactor(value);
		break;
	case 17:
		detune(value);
		break;
	case 18:
		priority(value);
		break;
	case 64:
		sustain(value > 0);
		break;
	case 91:
		// Effects level. Not supported.
		break;
	case 93:
		// Chorus level. Not supported.
		break;
	case 119:
		// Unknown, used in Simon the Sorcerer 2
		break;
	case 121:
		// reset all controllers
		modulationWheel(0);
		pitchBendFactor(0);
		detune(0);
		sustain(0);
		break;
	case 123:
		allNotesOff();
		break;
	default:
		warning("AdLib: Unknown control change message %d (%d)", (int)control, (int)value);
	}
}

void AdLibPart::modulationWheel(byte value) {
	AdLibVoice *voice;

	_modWheel = value;
	for (voice = _voice; voice; voice = voice->_next) {
		if (voice->_s10a.active && voice->_s11a.flag0x40)
			voice->_s10a.modWheel = _modWheel >> 2;
		if (voice->_s10b.active && voice->_s11b.flag0x40)
			voice->_s10b.modWheel = _modWheel >> 2;
	}
}

void AdLibPart::volume(byte value) {
	AdLibVoice *voice;

	_volEff = value;
	for (voice = _voice; voice; voice = voice->_next) {
		_owner->adlibSetParam(voice->_channel, 0, g_volumeTable[g_volumeLookupTable[voice->_vol2][_volEff >> 2]]);
		if (voice->_twoChan) {
			_owner->adlibSetParam(voice->_channel, 13, g_volumeTable[g_volumeLookupTable[voice->_vol1][_volEff >> 2]]);
		}
	}
}

void AdLibPart::pitchBendFactor(byte value) {
	AdLibVoice *voice;

	_pitchBendFactor = value;
	for (voice = _voice; voice; voice = voice->_next) {
		_owner->adlibNoteOn(voice->_channel, voice->_note + _transposeEff,
							  (_pitchBend * _pitchBendFactor >> 6) + _detuneEff);
	}
}

void AdLibPart::detune(byte value) {
	AdLibVoice *voice;

	_detuneEff = value;
	for (voice = _voice; voice; voice = voice->_next) {
		_owner->adlibNoteOn(voice->_channel, voice->_note + _transposeEff,
							  (_pitchBend * _pitchBendFactor >> 6) + _detuneEff);
	}
}

void AdLibPart::priority(byte value) {
	_priEff = value;
}

void AdLibPart::sustain(bool value) {
	AdLibVoice *voice;

	_pedal = value;
	if (!value) {
		for (voice = _voice; voice; voice = voice->_next) {
			if (voice->_waitForPedal)
				_owner->mcOff(voice);
		}
	}
}

void AdLibPart::allNotesOff() {
	while (_voice)
		_owner->mcOff(_voice);
}

void AdLibPart::sysEx_customInstrument(uint32 type, const byte *instr) {
	if (type == 'ADL ') {
		memcpy(&_partInstr, instr, sizeof(AdLibInstrument));
	}
}

// MidiChannel method implementations for percussion

AdLibPercussionChannel::~AdLibPercussionChannel() {
	for (int i = 0; i < ARRAYSIZE(_customInstruments); ++i) {
		delete _customInstruments[i];
	}
}

void AdLibPercussionChannel::init(MidiDriver_ADLIB *owner, byte channel) {
	AdLibPart::init(owner, channel);
	_priEff = 0;
	_volEff = 127;

	// Initialize the custom instruments data
	memset(_notes, 0, sizeof(_notes));
	memset(_customInstruments, 0, sizeof(_customInstruments));
}

void AdLibPercussionChannel::noteOff(byte note) {
	// Jamieson630: Unless I run into a specific instrument that
	// may require a key off, I'm going to ignore this message.
	// The rationale is that a percussion instrument should
	// fade out of its own accord, and the AdLib instrument
	// definitions used should follow this rule. Since
	// percussion voices are allocated at the lowest priority
	// anyway, we know that "hanging" percussion sounds will
	// not prevent later musical instruments (or even other
	// percussion sounds) from playing.
	/*
	 _owner->partKeyOff(this, note);
	*/
}

void AdLibPercussionChannel::noteOn(byte note, byte velocity) {
	AdLibInstrument *inst = NULL;

	// The custom instruments have priority over the default mapping
	inst = _customInstruments[note];
	if (inst)
		note = _notes[note];

	if (!inst) {
		// Use the default GM to FM mapping as a fallback
		byte key = g_gmPercussionInstrumentMap[note];
		if (key != 0xFF)
			inst = &g_gmPercussionInstruments[key];
	}

	if (!inst) {
		debug(2, "No instrument FM definition for GM percussion key %d", (int)note);
		return;
	}

	_owner->partKeyOn(this, inst, note, velocity);
}

void AdLibPercussionChannel::sysEx_customInstrument(uint32 type, const byte *instr) {
	if (type == 'ADLP') {
		byte note = instr[0];
		_notes[note] = instr[1];

		// Allocate memory for the new instruments
		if (!_customInstruments[note]) {
			_customInstruments[note] = new AdLibInstrument;
			memset(_customInstruments[note], 0, sizeof(AdLibInstrument));
		}

		// Save the new instrument data
		_customInstruments[note]->modCharacteristic     = instr[2];
		_customInstruments[note]->modScalingOutputLevel = instr[3];
		_customInstruments[note]->modAttackDecay        = instr[4];
		_customInstruments[note]->modSustainRelease     = instr[5];
		_customInstruments[note]->modWaveformSelect     = instr[6];
		_customInstruments[note]->carCharacteristic     = instr[7];
		_customInstruments[note]->carScalingOutputLevel = instr[8];
		_customInstruments[note]->carAttackDecay        = instr[9];
		_customInstruments[note]->carSustainRelease     = instr[10];
		_customInstruments[note]->carWaveformSelect     = instr[11];
		_customInstruments[note]->feedback               = instr[12];
	}
}

// MidiDriver method implementations

MidiDriver_ADLIB::MidiDriver_ADLIB(Audio::Mixer *mixer)
	: MidiDriver_Emulated(mixer) {
	uint i;

	_scummSmallHeader = false;

	_regCache = 0;

	_timerCounter = 0;
	_voiceIndex = 0;
	for (i = 0; i < ARRAYSIZE(_curNotTable); ++i) {
		_curNotTable[i] = 0;
	}

	for (i = 0; i < ARRAYSIZE(_parts); ++i) {
		_parts[i].init(this, i + ((i >= 9) ? 1 : 0));
	}
	_percussion.init(this, 9);
	_timerIncrease = 0xD69;
	_timerThreshold = 0x411B;
}

int MidiDriver_ADLIB::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	MidiDriver_Emulated::open();

	int i;
	AdLibVoice *voice;

	for (i = 0, voice = _voices; i != ARRAYSIZE(_voices); i++, voice++) {
		voice->_channel = i;
		voice->_s11a.s10 = &voice->_s10b;
		voice->_s11b.s10 = &voice->_s10a;
	}

	_regCache = (byte *)calloc(256, 1);

	_opl = makeAdLibOPL(getRate());

	adlibWrite(1, 0x20);
	adlibWrite(8, 0x40);
	adlibWrite(0xBD, 0x00);
	createLookupTable();

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	return 0;
}

void MidiDriver_ADLIB::close() {
	if (!_isOpen)
		return;
	_isOpen = false;

	_mixer->stopHandle(_mixerSoundHandle);

	uint i;
	for (i = 0; i < ARRAYSIZE(_voices); ++i) {
		if (_voices[i]._part)
			mcOff(&_voices[i]);
	}

	// Turn off the OPL emulation
	OPLDestroy(_opl);
//	YM3812Shutdown();

	free(_regCache);
}

void MidiDriver_ADLIB::send(uint32 b) {
	send(b & 0xF, b & 0xFFFFFFF0);
}

void MidiDriver_ADLIB::send(byte chan, uint32 b) {
	//byte param3 = (byte) ((b >> 24) & 0xFF);
	byte param2 = (byte)((b >> 16) & 0xFF);
	byte param1 = (byte)((b >>  8) & 0xFF);
	byte cmd    = (byte)(b & 0xF0);

	AdLibPart *part;
	if (chan == 9)
		part = &_percussion;
	else
		part = &_parts[chan];

	switch (cmd) {
	case 0x80:// Note Off
		part->noteOff(param1);
		break;
	case 0x90: // Note On
		part->noteOn(param1, param2);
		break;
	case 0xA0: // Aftertouch
		break; // Not supported.
	case 0xB0: // Control Change
		part->controlChange(param1, param2);
		break;
	case 0xC0: // Program Change
		part->programChange(param1);
		break;
	case 0xD0: // Channel Pressure
		break; // Not supported.
	case 0xE0: // Pitch Bend
		part->pitchBend((param1 | (param2 << 7)) - 0x2000);
		break;
	case 0xF0: // SysEx
		// We should never get here! SysEx information has to be
		// sent via high-level semantic methods.
		warning("MidiDriver_ADLIB: Receiving SysEx command on a send() call");
		break;

	default:
		warning("MidiDriver_ADLIB: Unknown send() command 0x%02X", cmd);
	}
}

uint32 MidiDriver_ADLIB::property(int prop, uint32 param) {
	switch (prop) {
	case PROP_OLD_ADLIB: // Older games used a different operator volume algorithm
		_scummSmallHeader = (param > 0);
		if (_scummSmallHeader) {
			_timerIncrease = 473;
			_timerThreshold = 1000;
		} else {
			_timerIncrease = 0xD69;
			_timerThreshold = 0x411B;
		}
		return 1;
	}

	return 0;
}

void MidiDriver_ADLIB::setPitchBendRange(byte channel, uint range) {
	AdLibVoice *voice;
	AdLibPart *part = &_parts[channel];

	part->_pitchBendFactor = range;
	for (voice = part->_voice; voice; voice = voice->_next) {
		adlibNoteOn(voice->_channel, voice->_note + part->_transposeEff,
					  (part->_pitchBend * part->_pitchBendFactor >> 6) + part->_detuneEff);
	}
}

void MidiDriver_ADLIB::sysEx_customInstrument(byte channel, uint32 type, const byte *instr) {
	_parts[channel].sysEx_customInstrument(type, instr);
}

MidiChannel *MidiDriver_ADLIB::allocateChannel() {
	AdLibPart *part;
	uint i;

	for (i = 0; i < ARRAYSIZE(_parts); ++i) {
		part = &_parts[i];
		if (!part->_allocated) {
			part->allocate();
			return part;
		}
	}
	return NULL;
}

// All the code brought over from IMuseAdLib

void MidiDriver_ADLIB::adlibWrite(byte reg, byte value) {
	if (_regCache[reg] == value)
		return;
#ifdef DEBUG_ADLIB
	debug(6, "%10d: adlibWrite[%x] = %x", g_tick, reg, value);
#endif
	_regCache[reg] = value;

	OPLWriteReg(_opl, reg, value);
}

void MidiDriver_ADLIB::generateSamples(int16 *data, int len) {
	memset(data, 0, sizeof(int16) * len);
	YM3812UpdateOne(_opl, data, len);
}

void MidiDriver_ADLIB::onTimer() {
	AdLibVoice *voice;
	int i;

	_timerCounter += _timerIncrease;
	while (_timerCounter >= _timerThreshold) {
		_timerCounter -= _timerThreshold;
#ifdef DEBUG_ADLIB
		g_tick++;
#endif
		voice = _voices;
		for (i = 0; i != ARRAYSIZE(_voices); i++, voice++) {
			if (!voice->_part)
				continue;
			if (voice->_duration && (voice->_duration -= 0x11) <= 0) {
				mcOff(voice);
				return;
			}
			if (voice->_s10a.active) {
				mcIncStuff(voice, &voice->_s10a, &voice->_s11a);
			}
			if (voice->_s10b.active) {
				mcIncStuff(voice, &voice->_s10b, &voice->_s11b);
			}
		}
	}
}

void MidiDriver_ADLIB::mcOff(AdLibVoice *voice) {
	AdLibVoice *tmp;

	adlibKeyOff(voice->_channel);

	tmp = voice->_prev;

	if (voice->_next)
		voice->_next->_prev = tmp;
	if (tmp)
		tmp->_next = voice->_next;
	else
		voice->_part->_voice = voice->_next;
	voice->_part = NULL;
}

void MidiDriver_ADLIB::mcIncStuff(AdLibVoice *voice, Struct10 *s10, Struct11 *s11) {
	byte code;
	AdLibPart *part = voice->_part;

	code = struct10OnTimer(s10, s11);

	if (code & 1) {
		switch (s11->param) {
		case 0:
			voice->_vol2 = s10->startValue + s11->modifyVal;
			if (!_scummSmallHeader) {
				adlibSetParam(voice->_channel, 0,
								g_volumeTable[g_volumeLookupTable[voice->_vol2]
											  [part->_volEff >> 2]]);
			} else {
				adlibSetParam(voice->_channel, 0, voice->_vol2);
			}
			break;
		case 13:
			voice->_vol1 = s10->startValue + s11->modifyVal;
			if (voice->_twoChan && !_scummSmallHeader) {
				adlibSetParam(voice->_channel, 13,
								g_volumeTable[g_volumeLookupTable[voice->_vol1]
											  [part->_volEff >> 2]]);
			} else {
				adlibSetParam(voice->_channel, 13, voice->_vol1);
			}
			break;
		case 30:
			s11->s10->modWheel = (char)s11->modifyVal;
			break;
		case 31:
			s11->s10->unk3 = (char)s11->modifyVal;
			break;
		default:
			adlibSetParam(voice->_channel, s11->param,
							s10->startValue + s11->modifyVal);
			break;
		}
	}

	if (code & 2 && s11->flag0x10)
		adlibKeyOnOff(voice->_channel);
}

void MidiDriver_ADLIB::adlibKeyOff(int chan) {
	byte reg = chan + 0xB0;
	adlibWrite(reg, adlibGetRegValue(reg) & ~0x20);
}

byte MidiDriver_ADLIB::struct10OnTimer(Struct10 *s10, Struct11 *s11) {
	byte result = 0;
	int i;

	if (s10->count && (s10->count -= 17) <= 0) {
		s10->active = 0;
		return 0;
	}

	i = s10->curVal + s10->speedHi;
	s10->speedLoCounter += s10->speedLo;
	if (s10->speedLoCounter >= s10->speedLoMax) {
		s10->speedLoCounter -= s10->speedLoMax;
		i += s10->direction;
	}
	if (s10->curVal != i || s10->modWheel != s10->modWheelLast) {
		s10->curVal = i;
		s10->modWheelLast = s10->modWheel;
		i = lookupVolume(i, s10->modWheelLast);
		if (i != s11->modifyVal) {
			s11->modifyVal = i;
			result = 1;
		}
	}

	if (!--s10->numSteps) {
		s10->active++;
		if (s10->active > 4) {
			if (s10->loop) {
				s10->active = 1;
				result |= 2;
				struct10Setup(s10);
			} else {
				s10->active = 0;
			}
		} else {
			struct10Setup(s10);
		}
	}

	return result;
}

void MidiDriver_ADLIB::adlibSetParam(int channel, byte param, int value) {
	const AdLibSetParams *as;
	byte reg;

	assert(channel >= 0 && channel < 9);

	if (param <= 12) {
		reg = g_operator2Offsets[channel];
	} else if (param <= 25) {
		param -= 13;
		reg = g_operator1Offsets[channel];
	} else if (param <= 27) {
		param -= 13;
		reg = channel;
	} else if (param == 28 || param == 29) {
		if (param == 28)
			value -= 15;
		else
			value -= 383;
		value <<= 4;
		_channelTable2[channel] = value;
		adlibPlayNote(channel, _curNotTable[channel] + value);
		return;
	} else {
		return;
	}

	as = &g_setParamTable[param];
	if (as->d)
		value = as->d - value;
	reg += as->a;
	adlibWrite(reg, (adlibGetRegValue(reg) & ~as->c) | (((byte)value) << as->b));
}

void MidiDriver_ADLIB::adlibKeyOnOff(int channel) {
	byte val;
	byte reg = channel + 0xB0;
	assert(channel >= 0 && channel < 9);

	val = adlibGetRegValue(reg);
	adlibWrite(reg, val & ~0x20);
	adlibWrite(reg, val | 0x20);
}

void MidiDriver_ADLIB::struct10Setup(Struct10 *s10) {
	int b, c, d, e, f, g, h;
	byte t;

	b = s10->unk3;
	f = s10->active - 1;

	t = s10->tableA[f];
	e = g_numStepsTable[g_volumeLookupTable[t & 0x7F][b]];
	if (t & 0x80) {
		e = randomNr(e);
	}
	if (e == 0)
		e++;

	s10->numSteps = s10->speedLoMax = e;

	if (f != 2) {
		c = s10->maxValue;
		g = s10->startValue;
		t = s10->tableB[f];
		d = lookupVolume(c, (t & 0x7F) - 31);
		if (t & 0x80) {
			d = randomNr(d);
		}
		if (d + g > c) {
			h = c - g;
		} else {
			h = d;
			if (d + g < 0)
				h = -g;
		}
		h -= s10->curVal;
	} else {
		h = 0;
	}

	s10->speedHi = h / e;
	if (h < 0) {
		h = -h;
		s10->direction = -1;
	} else {
		s10->direction = 1;
	}

	s10->speedLo = h % e;
	s10->speedLoCounter = 0;
}

void MidiDriver_ADLIB::adlibPlayNote(int channel, int note) {
	byte old, oct, notex;
	int note2;
	int i;

	note2 = (note >> 7) - 4;
	note2 = (note2 < 128) ? note2 : 0;

	oct = (note2 / 12);
	if (oct > 7)
		oct = 7 << 2;
	else
		oct <<= 2;
	notex = note2 % 12 + 3;

	old = adlibGetRegValue(channel + 0xB0);
	if (old & 0x20) {
		old &= ~0x20;
		if (oct > old) {
			if (notex < 6) {
				notex += 12;
				oct -= 4;
			}
		} else if (oct < old) {
			if (notex > 11) {
				notex -= 12;
				oct += 4;
			}
		}
	}

	i = (notex << 3) + ((note >> 4) & 0x7);
	adlibWrite(channel + 0xA0, g_noteFrequencies[i]);
	adlibWrite(channel + 0xB0, oct | 0x20);
}

int MidiDriver_ADLIB::randomNr(int a) {
	static byte _randSeed = 1;
	if (_randSeed & 1) {
		_randSeed >>= 1;
		_randSeed ^= 0xB8;
	} else {
		_randSeed >>= 1;
	}
	return _randSeed * a >> 8;
}

void MidiDriver_ADLIB::partKeyOff(AdLibPart *part, byte note) {
	AdLibVoice *voice;

	for (voice = part->_voice; voice; voice = voice->_next) {
		if (voice->_note == note) {
			if (part->_pedal)
				voice->_waitForPedal = true;
			else
				mcOff(voice);
		}
	}
}

void MidiDriver_ADLIB::partKeyOn(AdLibPart *part, AdLibInstrument *instr, byte note, byte velocity) {
	AdLibVoice *voice;

	voice = allocateVoice(part->_priEff);
	if (!voice)
		return;

	linkMc(part, voice);
	mcKeyOn(voice, instr, note, velocity);
}

AdLibVoice *MidiDriver_ADLIB::allocateVoice(byte pri) {
	AdLibVoice *ac, *best = NULL;
	int i;

	for (i = 0; i < 9; i++) {
		if (++_voiceIndex >= 9)
			_voiceIndex = 0;
		ac = &_voices[_voiceIndex];
		if (!ac->_part)
			return ac;
		if (!ac->_next) {
			if (ac->_part->_priEff <= pri) {
				pri = ac->_part->_priEff;
				best = ac;
			}
		}
	}

	/* SCUMM V3 games don't have note priorities, first comes wins. */
	if (_scummSmallHeader)
		return NULL;

	if (best)
		mcOff(best);
	return best;
}

void MidiDriver_ADLIB::linkMc(AdLibPart *part, AdLibVoice *voice) {
	voice->_part = part;
	voice->_next = (AdLibVoice *)part->_voice;
	part->_voice = voice;
	voice->_prev = NULL;

	if (voice->_next)
		voice->_next->_prev = voice;
}

void MidiDriver_ADLIB::mcKeyOn(AdLibVoice *voice, AdLibInstrument *instr, byte note, byte velocity) {
	AdLibPart *part = voice->_part;
	int c;
	byte vol1, vol2;

	voice->_twoChan = instr->feedback & 1;
	voice->_note = note;
	voice->_waitForPedal = false;
	voice->_duration = instr->duration;
	if (voice->_duration != 0)
		voice->_duration *= 63;

	if (!_scummSmallHeader)
		vol1 = (instr->modScalingOutputLevel & 0x3F) + g_volumeLookupTable[velocity >> 1][instr->modWaveformSelect >> 2];
	else
		vol1 = 0x3f - (instr->modScalingOutputLevel & 0x3F);
	if (vol1 > 0x3F)
		vol1 = 0x3F;
	voice->_vol1 = vol1;

	if (!_scummSmallHeader)
		vol2 = (instr->carScalingOutputLevel & 0x3F) + g_volumeLookupTable[velocity >> 1][instr->carWaveformSelect >> 2];
	else
		vol2 = 0x3f - (instr->carScalingOutputLevel & 0x3F);
	if (vol2 > 0x3F)
		vol2 = 0x3F;
	voice->_vol2 = vol2;

	c = part->_volEff >> 2;

	if (!_scummSmallHeader) {
		vol2 = g_volumeTable[g_volumeLookupTable[vol2][c]];
		if (voice->_twoChan)
			vol1 = g_volumeTable[g_volumeLookupTable[vol1][c]];
	}

	adlibSetupChannel(voice->_channel, instr, vol1, vol2);
	adlibNoteOnEx(voice->_channel, part->_transposeEff + note, part->_detuneEff + (part->_pitchBend * part->_pitchBendFactor >> 6));

	if (instr->flagsA & 0x80) {
		mcInitStuff(voice, &voice->_s10a, &voice->_s11a, instr->flagsA, &instr->extraA);
	} else {
		voice->_s10a.active = 0;
	}

	if (instr->flagsB & 0x80) {
		mcInitStuff(voice, &voice->_s10b, &voice->_s11b, instr->flagsB, &instr->extraB);
	} else {
		voice->_s10b.active = 0;
	}
}

void MidiDriver_ADLIB::adlibSetupChannel(int chan, AdLibInstrument *instr, byte vol1, byte vol2) {
	byte channel;

	assert(chan >= 0 && chan < 9);

	channel = g_operator1Offsets[chan];
	adlibWrite(channel + 0x20, instr->modCharacteristic);
	adlibWrite(channel + 0x40, (instr->modScalingOutputLevel | 0x3F) - vol1);
	adlibWrite(channel + 0x60, 0xff & (~instr->modAttackDecay));
	adlibWrite(channel + 0x80, 0xff & (~instr->modSustainRelease));
	adlibWrite(channel + 0xE0, instr->modWaveformSelect);

	channel = g_operator2Offsets[chan];
	adlibWrite(channel + 0x20, instr->carCharacteristic);
	adlibWrite(channel + 0x40, (instr->carScalingOutputLevel | 0x3F) - vol2);
	adlibWrite(channel + 0x60, 0xff & (~instr->carAttackDecay));
	adlibWrite(channel + 0x80, 0xff & (~instr->carSustainRelease));
	adlibWrite(channel + 0xE0, instr->carWaveformSelect);

	adlibWrite((byte)chan + 0xC0, instr->feedback);
}

void MidiDriver_ADLIB::adlibNoteOnEx(int chan, byte note, int mod) {
	int code;
	assert(chan >= 0 && chan < 9);
	code = (note << 7) + mod;
	_curNotTable[chan] = code;
	_channelTable2[chan] = 0;
	adlibPlayNote(chan, code);
}

void MidiDriver_ADLIB::mcInitStuff(AdLibVoice *voice, Struct10 *s10,
									 Struct11 *s11, byte flags, InstrumentExtra *ie) {
	AdLibPart *part = voice->_part;
	s11->modifyVal = 0;
	s11->flag0x40 = flags & 0x40;
	s10->loop = flags & 0x20;
	s11->flag0x10 = flags & 0x10;
	s11->param = g_paramTable1[flags & 0xF];
	s10->maxValue = g_maxValTable[flags & 0xF];
	s10->unk3 = 31;
	if (s11->flag0x40) {
		s10->modWheel = part->_modWheel >> 2;
	} else {
		s10->modWheel = 31;
	}

	switch (s11->param) {
	case 0:
		s10->startValue = voice->_vol2;
		break;
	case 13:
		s10->startValue = voice->_vol1;
		break;
	case 30:
		s10->startValue = 31;
		s11->s10->modWheel = 0;
		break;
	case 31:
		s10->startValue = 0;
		s11->s10->unk3 = 0;
		break;
	default:
		s10->startValue = adlibGetRegValueParam(voice->_channel, s11->param);
	}

	struct10Init(s10, ie);
}

void MidiDriver_ADLIB::struct10Init(Struct10 *s10, InstrumentExtra *ie) {
	s10->active = 1;
	if (!_scummSmallHeader) {
		s10->curVal = 0;
	} else {
		s10->curVal = s10->startValue;
		s10->startValue = 0;
	}
	s10->modWheelLast = 31;
	s10->count = ie->a;
	if (s10->count)
		s10->count *= 63;
	s10->tableA[0] = ie->b;
	s10->tableA[1] = ie->d;
	s10->tableA[2] = ie->f;
	s10->tableA[3] = ie->g;

	s10->tableB[0] = ie->c;
	s10->tableB[1] = ie->e;
	s10->tableB[2] = 0;
	s10->tableB[3] = ie->h;

	struct10Setup(s10);
}

int MidiDriver_ADLIB::adlibGetRegValueParam(int chan, byte param) {
	const AdLibSetParams *as;
	byte val;
	byte channel;

	assert(chan >= 0 && chan < 9);

	if (param <= 12) {
		channel = g_operator2Offsets[chan];
	} else if (param <= 25) {
		param -= 13;
		channel = g_operator1Offsets[chan];
	} else if (param <= 27) {
		param -= 13;
		channel = chan;
	} else if (param == 28) {
		return 0xF;
	} else if (param == 29) {
		return 0x17F;
	} else {
		return 0;
	}

	as = &g_setParamTable[param];
	val = adlibGetRegValue(channel + as->a);
	val &= as->c;
	val >>= as->b;
	if (as->d)
		val = as->d - val;

	return val;
}

void MidiDriver_ADLIB::adlibNoteOn(int chan, byte note, int mod) {
	int code;
	assert(chan >= 0 && chan < 9);
	code = (note << 7) + mod;
	_curNotTable[chan] = code;
	adlibPlayNote(chan, (int16)_channelTable2[chan] + code);
}


// Plugin interface

class AdLibEmuMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return _s("AdLib Emulator");
	}

	const char *getId() const {
		return "adlib";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices AdLibEmuMusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_ADLIB));
	return devices;
}

Common::Error AdLibEmuMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_ADLIB(g_system->getMixer());

	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(ADLIB)
	//REGISTER_PLUGIN_DYNAMIC(ADLIB, PLUGIN_TYPE_MUSIC, AdLibEmuMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(ADLIB, PLUGIN_TYPE_MUSIC, AdLibEmuMusicPlugin);
//#endif
