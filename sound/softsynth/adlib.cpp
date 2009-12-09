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
 * $URL$
 * $Id$
 */

#include "sound/softsynth/emumidi.h"
#include "common/debug.h"
#include "common/util.h"
#include "sound/fmopl.h"
#include "sound/musicplugin.h"

#ifdef DEBUG_ADLIB
static int tick;
#endif

class MidiDriver_ADLIB;
struct AdlibVoice;

// We use packing for the following two structs, because the code
// does simply copy them over from byte streams, without any
// serialization. Check AdlibPart::sysEx_customInstrument for an
// example of this.
//
// It might be very well possible, that none of the compilers we support
// add any padding bytes at all, since the structs contain only variables
// of the type 'byte'. But better safe than sorry.
#include "common/pack-start.h"
struct InstrumentExtra {
	byte a, b, c, d, e, f, g, h;
} PACKED_STRUCT;

struct AdlibInstrument {
	byte mod_characteristic;
	byte mod_scalingOutputLevel;
	byte mod_attackDecay;
	byte mod_sustainRelease;
	byte mod_waveformSelect;
	byte car_characteristic;
	byte car_scalingOutputLevel;
	byte car_attackDecay;
	byte car_sustainRelease;
	byte car_waveformSelect;
	byte feedback;
	byte flags_a;
	InstrumentExtra extra_a;
	byte flags_b;
	InstrumentExtra extra_b;
	byte duration;

	AdlibInstrument() { memset(this, 0, sizeof(AdlibInstrument)); }
} PACKED_STRUCT;
#include "common/pack-end.h"

class AdlibPart : public MidiChannel {
	friend class MidiDriver_ADLIB;

protected:
//	AdlibPart *_prev, *_next;
	AdlibVoice *_voice;
	int16 _pitchbend;
	byte _pitchbend_factor;
	int8 _transpose_eff;
	byte _vol_eff;
	int8 _detune_eff;
	byte _modwheel;
	bool _pedal;
	byte _program;
	byte _pri_eff;
	AdlibInstrument _part_instr;

protected:
	MidiDriver_ADLIB *_owner;
	bool _allocated;
	byte _channel;

	void init(MidiDriver_ADLIB *owner, byte channel);
	void allocate() { _allocated = true; }

public:
	AdlibPart() {
		_voice = 0;
		_pitchbend = 0;
		_pitchbend_factor = 2;
		_transpose_eff = 0;
		_vol_eff = 0;
		_detune_eff = 0;
		_modwheel = 0;
		_pedal = 0;
		_program = 0;
		_pri_eff = 0;

		_owner = 0;
		_allocated = false;
		_channel = 0;
	}

	MidiDriver *device();
	byte getNumber() { return _channel; }
	void release() { _allocated = false; }

	void send (uint32 b);

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
// It is assumed that any invocation to AdlibPercussionChannel
// will be done through the MidiChannel base class as opposed to the
// AdlibPart base class. If this were NOT the case, all the functions
// listed below would need to be virtual in AdlibPart as well as MidiChannel.
class AdlibPercussionChannel : public AdlibPart {
	friend class MidiDriver_ADLIB;

protected:
	void init(MidiDriver_ADLIB *owner, byte channel);

public:
	~AdlibPercussionChannel();

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
	AdlibInstrument *_customInstruments[256];
};

struct Struct10 {
	byte active;
	int16 cur_val;
	int16 count;
	uint16 max_value;
	int16 start_value;
	byte loop;
	byte table_a[4];
	byte table_b[4];
	int8 unk3;
	int8 modwheel;
	int8 modwheel_last;
	uint16 speed_lo_max;
	uint16 num_steps;
	int16 speed_hi;
	int8 direction;
	uint16 speed_lo;
	uint16 speed_lo_counter;
};

struct Struct11 {
	int16 modify_val;
	byte param, flag0x40, flag0x10;
	Struct10 *s10;
};

struct AdlibVoice {
	AdlibPart *_part;
	AdlibVoice *_next, *_prev;
	byte _waitforpedal;
	byte _note;
	byte _channel;
	byte _twochan;
	byte _vol_1, _vol_2;
	int16 _duration;

	Struct10 _s10a;
	Struct11 _s11a;
	Struct10 _s10b;
	Struct11 _s11b;

	AdlibVoice() { memset(this, 0, sizeof(AdlibVoice)); }
};

struct AdlibSetParams {
	byte a, b, c, d;
};

static const byte channel_mappings[9] = {
	0, 1, 2, 8,
	9, 10, 16, 17,
	18
};

static const byte channel_mappings_2[9] = {
	3, 4, 5, 11,
	12, 13, 19, 20,
	21
};

static const AdlibSetParams adlib_setparam_table[] = {
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

static const byte param_table_1[16] = {
	29, 28, 27, 0,
	3, 4, 7, 8,
	13, 16, 17, 20,
	21, 30, 31, 0
};

static const uint16 maxval_table[16] = {
	0x2FF, 0x1F, 0x7, 0x3F,
	0x0F, 0x0F, 0x0F, 0x3,
	0x3F, 0x0F, 0x0F, 0x0F,
	0x3, 0x3E, 0x1F, 0
};

static const uint16 num_steps_table[] = {
	1, 2, 4, 5,
	6, 7, 8, 9,
	10, 12, 14, 16,
	18, 21, 24, 30,
	36, 50, 64, 82,
	100, 136, 160, 192,
	240, 276, 340, 460,
	600, 860, 1200, 1600
};

static const byte note_to_f_num[] = {
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

static const byte map_gm_to_fm[128][30] = {
	// 0x00
{ 0xC2, 0xC5, 0x2B, 0x99, 0x58, 0xC2, 0x1F, 0x1E, 0xC8, 0x7C, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x23 },
{ 0x22, 0x53, 0x0E, 0x8A, 0x30, 0x14, 0x06, 0x1D, 0x7A, 0x5C, 0x06, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x06, 0x00, 0x1C, 0x79, 0x40, 0x02, 0x00, 0x4B, 0x79, 0x58, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xC2, 0x89, 0x2A, 0x89, 0x49, 0xC2, 0x16, 0x1C, 0xB8, 0x7C, 0x04, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x23 },
{ 0xC2, 0x17, 0x3D, 0x6A, 0x00, 0xC4, 0x2E, 0x2D, 0xC9, 0x20, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x06, 0x1E, 0x1C, 0x99, 0x00, 0x02, 0x3A, 0x4C, 0x79, 0x00, 0x0C, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x84, 0x40, 0x3B, 0x5A, 0x6F, 0x81, 0x0E, 0x3B, 0x5A, 0x7F, 0x0B, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x84, 0x40, 0x3B, 0x5A, 0x63, 0x81, 0x00, 0x3B, 0x5A, 0x7F, 0x01, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x8C, 0x80, 0x05, 0xEA, 0x59, 0x82, 0x0A, 0x3C, 0xAA, 0x64, 0x07, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x85, 0x40, 0x0D, 0xEC, 0x71, 0x84, 0x58, 0x3E, 0xCB, 0x7C, 0x01, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x8A, 0xC0, 0x0C, 0xDC, 0x50, 0x88, 0x58, 0x3D, 0xDA, 0x7C, 0x01, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xC9, 0x40, 0x2B, 0x78, 0x42, 0xC2, 0x04, 0x4C, 0x8A, 0x7C, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x1A },
{ 0x2A, 0x0E, 0x17, 0x89, 0x28, 0x22, 0x0C, 0x1B, 0x09, 0x70, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE7, 0x9B, 0x08, 0x08, 0x26, 0xE2, 0x06, 0x0A, 0x08, 0x70, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xC5, 0x05, 0x00, 0xFC, 0x40, 0x84, 0x00, 0x00, 0xDC, 0x50, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x86, 0x40, 0x5D, 0x5A, 0x41, 0x81, 0x00, 0x0B, 0x5A, 0x7F, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
	// 0x10
{ 0xED, 0x00, 0x7B, 0xC8, 0x40, 0xE1, 0x99, 0x4A, 0xE9, 0x7E, 0x07, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE8, 0x4F, 0x3A, 0xD7, 0x7C, 0xE2, 0x97, 0x49, 0xF9, 0x7D, 0x05, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE1, 0x10, 0x2F, 0xF7, 0x7D, 0xF3, 0x45, 0x8F, 0xC7, 0x62, 0x07, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x01, 0x8C, 0x9F, 0xDA, 0x70, 0xE4, 0x50, 0x9F, 0xDA, 0x6A, 0x09, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x08, 0xD5, 0x9D, 0xA5, 0x45, 0xE2, 0x3F, 0x9F, 0xD6, 0x49, 0x07, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE5, 0x0F, 0x7D, 0xB8, 0x2E, 0xA2, 0x0F, 0x7C, 0xC7, 0x61, 0x04, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xF2, 0x2A, 0x9F, 0xDB, 0x01, 0xE1, 0x04, 0x8F, 0xD7, 0x62, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE4, 0x88, 0x9C, 0x50, 0x64, 0xE2, 0x18, 0x70, 0xC4, 0x7C, 0x0B, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x02, 0xA3, 0x0D, 0xDA, 0x01, 0xC2, 0x35, 0x5D, 0x58, 0x00, 0x06, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x18 },
{ 0x42, 0x55, 0x3E, 0xEB, 0x24, 0xD4, 0x08, 0x0D, 0xA9, 0x71, 0x04, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x18 },
{ 0xC2, 0x00, 0x2B, 0x17, 0x51, 0xC2, 0x1E, 0x4D, 0x97, 0x7C, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x19 },
{ 0xC6, 0x01, 0x2D, 0xA7, 0x44, 0xC2, 0x06, 0x0E, 0xA7, 0x79, 0x06, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xC2, 0x0C, 0x06, 0x06, 0x55, 0xC2, 0x3F, 0x09, 0x86, 0x7D, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x0A },
{ 0xC2, 0x2E, 0x4F, 0x77, 0x00, 0xC4, 0x08, 0x0E, 0x98, 0x59, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xC2, 0x30, 0x4F, 0xCA, 0x01, 0xC4, 0x0D, 0x0E, 0xB8, 0x7F, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xC4, 0x29, 0x4F, 0xCA, 0x03, 0xC8, 0x0D, 0x0C, 0xB7, 0x7D, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x0B },
	// 0x20
{ 0xC2, 0x40, 0x3C, 0x96, 0x58, 0xC4, 0xDE, 0x0E, 0xC7, 0x7C, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x20 },
{ 0x31, 0x13, 0x2D, 0xD7, 0x3C, 0xE2, 0x18, 0x2E, 0xB8, 0x7C, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x22, 0x86, 0x0D, 0xD7, 0x50, 0xE4, 0x18, 0x5E, 0xB8, 0x7C, 0x06, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x28 },
{ 0xF2, 0x0A, 0x0D, 0xD7, 0x40, 0xE4, 0x1F, 0x5E, 0xB8, 0x7C, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xF2, 0x09, 0x4B, 0xD6, 0x48, 0xE4, 0x1F, 0x1C, 0xB8, 0x7C, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x28 },
{ 0x62, 0x11, 0x0C, 0xE6, 0x3C, 0xE4, 0x1F, 0x0C, 0xC8, 0x7C, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE2, 0x12, 0x3D, 0xE6, 0x34, 0xE4, 0x1F, 0x7D, 0xB8, 0x7C, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE2, 0x13, 0x3D, 0xE6, 0x34, 0xE4, 0x1F, 0x5D, 0xB8, 0x7D, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xA2, 0x40, 0x5D, 0xBA, 0x3F, 0xE2, 0x00, 0x8F, 0xD8, 0x79, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE2, 0x40, 0x3D, 0xDA, 0x3B, 0xE1, 0x00, 0x7E, 0xD8, 0x7A, 0x04, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x62, 0x00, 0x6D, 0xFA, 0x5D, 0xE2, 0x00, 0x8F, 0xC8, 0x79, 0x04, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE1, 0x00, 0x4E, 0xDB, 0x4A, 0xE3, 0x18, 0x6F, 0xE9, 0x7E, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE1, 0x00, 0x4E, 0xDB, 0x66, 0xE2, 0x00, 0x7F, 0xE9, 0x7E, 0x06, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x02, 0x0F, 0x66, 0xAA, 0x51, 0x02, 0x64, 0x29, 0xF9, 0x7C, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x04 },
{ 0x16, 0x4A, 0x04, 0xBA, 0x39, 0xC2, 0x58, 0x2D, 0xCA, 0x7C, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x03 },
{ 0x02, 0x00, 0x01, 0x7A, 0x79, 0x02, 0x3F, 0x28, 0xEA, 0x7C, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
	// 0x30
{ 0x62, 0x53, 0x9C, 0xBA, 0x31, 0x62, 0x5B, 0xAD, 0xC9, 0x55, 0x04, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xF2, 0x40, 0x6E, 0xDA, 0x49, 0xE2, 0x13, 0x8F, 0xF9, 0x7D, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE2, 0x40, 0x8F, 0xFA, 0x50, 0xF2, 0x04, 0x7F, 0xFA, 0x7D, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE4, 0xA0, 0xCE, 0x5B, 0x02, 0xE2, 0x32, 0x7F, 0xFB, 0x3D, 0x04, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE6, 0x80, 0x9C, 0x99, 0x42, 0xE2, 0x04, 0x7D, 0x78, 0x60, 0x04, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xEA, 0xA0, 0xAC, 0x67, 0x02, 0xE2, 0x00, 0x7C, 0x7A, 0x7C, 0x06, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE7, 0x94, 0xAD, 0xB7, 0x03, 0xE2, 0x00, 0x7C, 0xBA, 0x7C, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xC3, 0x3F, 0x4B, 0xE9, 0x7E, 0xC1, 0x3F, 0x9B, 0xF9, 0x7F, 0x0B, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x06 },
{ 0xB2, 0x20, 0xAD, 0xE9, 0x00, 0x62, 0x05, 0x8F, 0xC8, 0x68, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xF2, 0x00, 0x8F, 0xFB, 0x50, 0xF6, 0x47, 0x8F, 0xE9, 0x68, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xF2, 0x00, 0xAF, 0x88, 0x58, 0xF2, 0x54, 0x6E, 0xC9, 0x7C, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xF2, 0x2A, 0x9F, 0x98, 0x01, 0xE2, 0x84, 0x4E, 0x78, 0x6C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE2, 0x02, 0x9F, 0xB8, 0x48, 0x22, 0x89, 0x9F, 0xE8, 0x7C, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE2, 0x2A, 0x7F, 0xB8, 0x01, 0xE4, 0x00, 0x0D, 0xC5, 0x7C, 0x0C, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE4, 0x28, 0x8E, 0xE8, 0x01, 0xF2, 0x00, 0x4D, 0xD6, 0x7D, 0x0C, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x62, 0x23, 0x8F, 0xEA, 0x00, 0xF2, 0x00, 0x5E, 0xD9, 0x7C, 0x0C, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
	// 0x40
{ 0xB4, 0x26, 0x6E, 0x98, 0x01, 0x62, 0x00, 0x7D, 0xC8, 0x7D, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE2, 0x2E, 0x20, 0xD9, 0x01, 0xF2, 0x0F, 0x90, 0xF8, 0x78, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE4, 0x28, 0x7E, 0xF8, 0x01, 0xE2, 0x23, 0x8E, 0xE8, 0x7D, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xB8, 0x28, 0x9E, 0x98, 0x01, 0x62, 0x00, 0x3D, 0xC8, 0x7D, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x62, 0x00, 0x8E, 0xC9, 0x3D, 0xE6, 0x00, 0x7E, 0xD8, 0x68, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE2, 0x00, 0x5F, 0xF9, 0x48, 0xE6, 0x98, 0x8F, 0xF8, 0x7D, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x62, 0x0C, 0x6E, 0xD8, 0x3D, 0x2A, 0x06, 0x7D, 0xD8, 0x58, 0x04, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE4, 0x00, 0x7E, 0x89, 0x38, 0xE6, 0x84, 0x80, 0xF8, 0x68, 0x0C, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE4, 0x80, 0x6C, 0xD9, 0x30, 0xE2, 0x00, 0x8D, 0xC8, 0x7C, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE2, 0x80, 0x88, 0x48, 0x40, 0xE2, 0x0A, 0x7D, 0xA8, 0x7C, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE4, 0x00, 0x77, 0xC5, 0x54, 0xE2, 0x00, 0x9E, 0xD7, 0x70, 0x06, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE4, 0x80, 0x86, 0xB9, 0x64, 0xE2, 0x05, 0x9F, 0xD7, 0x78, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE2, 0x00, 0x68, 0x68, 0x56, 0xE2, 0x08, 0x9B, 0xB3, 0x7C, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE4, 0x00, 0xA6, 0x87, 0x41, 0xE2, 0x0A, 0x7E, 0xC9, 0x7C, 0x06, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE4, 0x80, 0x9A, 0xB8, 0x48, 0xE2, 0x00, 0x9E, 0xF9, 0x60, 0x09, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE2, 0x80, 0x8E, 0x64, 0x68, 0xE2, 0x28, 0x6F, 0x73, 0x7C, 0x01, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
	// 0x50
{ 0xE8, 0x00, 0x7D, 0x99, 0x54, 0xE6, 0x80, 0x80, 0xF8, 0x7C, 0x0C, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE6, 0x00, 0x9F, 0xB9, 0x6D, 0xE1, 0x00, 0x8F, 0xC8, 0x7D, 0x02, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE4, 0x00, 0x09, 0x68, 0x4A, 0xE2, 0x2B, 0x9E, 0xF3, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xC4, 0x00, 0x99, 0xE8, 0x3B, 0xE2, 0x25, 0x6F, 0x93, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE6, 0x00, 0x6F, 0xDA, 0x69, 0xE2, 0x05, 0x2F, 0xD8, 0x6A, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xEC, 0x60, 0x9D, 0xC7, 0x00, 0xE2, 0x21, 0x7F, 0xC9, 0x7C, 0x06, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE3, 0x00, 0x0F, 0xF7, 0x7D, 0xE1, 0x3F, 0x0F, 0xA7, 0x01, 0x0D, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE4, 0xA9, 0x0F, 0xA8, 0x02, 0xE2, 0x3C, 0x5F, 0xDA, 0x3C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE8, 0x40, 0x0D, 0x89, 0x7D, 0xE2, 0x17, 0x7E, 0xD9, 0x7C, 0x07, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE1, 0x00, 0xDF, 0x8A, 0x56, 0xE2, 0x5E, 0xCF, 0xBA, 0x7E, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE2, 0x00, 0x0B, 0x68, 0x60, 0xE2, 0x01, 0x9E, 0xB8, 0x7C, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xEA, 0x00, 0xAE, 0xAB, 0x49, 0xE2, 0x00, 0xAE, 0xBA, 0x6C, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xEB, 0x80, 0x8C, 0xCB, 0x3A, 0xE2, 0x86, 0xAF, 0xCA, 0x7C, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE5, 0x40, 0xDB, 0x3B, 0x3C, 0xE2, 0x80, 0xBE, 0xCA, 0x71, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE4, 0x00, 0x9E, 0xAA, 0x3D, 0xE1, 0x43, 0x0F, 0xBA, 0x7E, 0x04, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE7, 0x40, 0xEC, 0xCA, 0x44, 0xE2, 0x03, 0xBF, 0xBA, 0x66, 0x02, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
	// 0x60
{ 0xEA, 0x00, 0x68, 0xB8, 0x48, 0xE2, 0x0A, 0x8E, 0xB8, 0x7C, 0x0C, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x61, 0x00, 0xBE, 0x99, 0x7E, 0xE3, 0x40, 0xCF, 0xCA, 0x7D, 0x09, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xCD, 0x00, 0x0B, 0x00, 0x48, 0xC2, 0x58, 0x0C, 0x00, 0x7C, 0x0C, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x1C },
{ 0xE2, 0x00, 0x0E, 0x00, 0x52, 0xE2, 0x58, 0x5F, 0xD0, 0x7D, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xCC, 0x00, 0x7D, 0xDA, 0x40, 0xC2, 0x00, 0x5E, 0x9B, 0x58, 0x0C, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE9, 0xC0, 0xEE, 0xD8, 0x43, 0xE2, 0x05, 0xDD, 0xAA, 0x70, 0x06, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xDA, 0x00, 0x8F, 0xAC, 0x4A, 0x22, 0x05, 0x8D, 0x8A, 0x75, 0x02, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x62, 0x8A, 0xCB, 0x7A, 0x74, 0xE6, 0x56, 0xAF, 0xDB, 0x70, 0x02, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xC2, 0x41, 0xAC, 0x5B, 0x5B, 0xC2, 0x80, 0x0D, 0xCB, 0x7D, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x12 },
{ 0x75, 0x00, 0x0E, 0xCB, 0x5A, 0xE2, 0x1E, 0x0A, 0xC9, 0x7D, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x10 },
{ 0x41, 0x00, 0x0E, 0xEA, 0x53, 0xC2, 0x00, 0x08, 0xCA, 0x7C, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x07 },
{ 0xC1, 0x40, 0x0C, 0x59, 0x6A, 0xC2, 0x80, 0x3C, 0xAB, 0x7C, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x0D },
{ 0x4B, 0x00, 0x0A, 0xF5, 0x61, 0xC2, 0x19, 0x0C, 0xE9, 0x7C, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x07 },
{ 0x62, 0x00, 0x7F, 0xD8, 0x54, 0xEA, 0x00, 0x8F, 0xD8, 0x7D, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE1, 0x00, 0x7F, 0xD9, 0x56, 0xE1, 0x00, 0x8F, 0xD8, 0x7E, 0x06, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0xE1, 0x00, 0x7F, 0xD9, 0x56, 0xE1, 0x00, 0x8F, 0xD8, 0x7E, 0x06, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
	// 0x70
{ 0xCF, 0x40, 0x09, 0xEA, 0x54, 0xC4, 0x00, 0x0C, 0xDB, 0x64, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0xCF, 0x40, 0x0C, 0xAA, 0x54, 0xC4, 0x00, 0x18, 0xF9, 0x64, 0x0C, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0xC9, 0x0E, 0x88, 0xD9, 0x3E, 0xC2, 0x08, 0x1A, 0xEA, 0x6C, 0x0C, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x05 },
{ 0x03, 0x00, 0x15, 0x00, 0x64, 0x02, 0x00, 0x08, 0x00, 0x7C, 0x09, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0x01, 0x00, 0x47, 0xD7, 0x6C, 0x01, 0x3F, 0x0C, 0xFB, 0x7C, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x04 },
{ 0x00, 0x00, 0x36, 0x67, 0x7C, 0x01, 0x3F, 0x0E, 0xFA, 0x7C, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x05 },
{ 0x02, 0x00, 0x36, 0x68, 0x7C, 0x01, 0x3F, 0x0E, 0xFA, 0x7C, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x05 },
{ 0xCB, 0x00, 0xAF, 0x00, 0x7E, 0xC0, 0x00, 0xC0, 0x06, 0x7F, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x0F },
{ 0x05, 0x0D, 0x80, 0xA6, 0x7F, 0x0B, 0x38, 0xA9, 0xD8, 0x00, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x04 },
{ 0x0F, 0x00, 0x90, 0xFA, 0x68, 0x06, 0x00, 0xA7, 0x39, 0x54, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x06 },
{ 0xC9, 0x15, 0xDD, 0xFF, 0x7C, 0x00, 0x00, 0xE7, 0xFC, 0x6C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x38 },
{ 0x48, 0x3C, 0x30, 0xF6, 0x03, 0x0A, 0x38, 0x97, 0xE8, 0x00, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x04 },
{ 0x07, 0x80, 0x0B, 0xC8, 0x65, 0x02, 0x3F, 0x0C, 0xEA, 0x7C, 0x0F, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x05 },
{ 0x00, 0x21, 0x66, 0x40, 0x03, 0x00, 0x3F, 0x47, 0x00, 0x00, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0x08, 0x00, 0x0B, 0x3C, 0x7C, 0x08, 0x3F, 0x06, 0xF3, 0x00, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0x00, 0x3F, 0x4C, 0xFB, 0x00, 0x00, 0x3F, 0x0A, 0xE9, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x05 }
};

static byte gm_percussion_to_fm[39][30] = {
{ 0x1A, 0x3F, 0x15, 0x05, 0x7C, 0x02, 0x21, 0x2B, 0xE4, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x06 },
{ 0x11, 0x12, 0x04, 0x07, 0x7C, 0x02, 0x23, 0x0B, 0xE5, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x05 },
{ 0x0A, 0x3F, 0x0B, 0x01, 0x7C, 0x1F, 0x1C, 0x46, 0xD0, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x01 },
{ 0x00, 0x3F, 0x0F, 0x00, 0x7C, 0x10, 0x12, 0x07, 0x00, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0x0F, 0x3F, 0x0B, 0x00, 0x7C, 0x1F, 0x0F, 0x19, 0xD0, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0x00, 0x3F, 0x1F, 0x00, 0x7E, 0x1F, 0x16, 0x07, 0x00, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x03 },
{ 0x12, 0x3F, 0x05, 0x06, 0x7C, 0x03, 0x1F, 0x4A, 0xD9, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x03 },
{ 0xCF, 0x7F, 0x08, 0xFF, 0x7E, 0x00, 0xC7, 0x2D, 0xF7, 0x73, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0x12, 0x3F, 0x05, 0x06, 0x7C, 0x43, 0x21, 0x0C, 0xE9, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x03 },
{ 0xCF, 0x7F, 0x08, 0xCF, 0x7E, 0x00, 0x45, 0x2A, 0xF8, 0x4B, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x0C },
{ 0x12, 0x3F, 0x06, 0x17, 0x7C, 0x03, 0x27, 0x0B, 0xE9, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x03 },
{ 0xCF, 0x7F, 0x08, 0xCD, 0x7E, 0x00, 0x40, 0x1A, 0x69, 0x63, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x0C },
{ 0x13, 0x3F, 0x05, 0x06, 0x7C, 0x03, 0x17, 0x0A, 0xD9, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x03 },
{ 0x15, 0x3F, 0x05, 0x06, 0x7C, 0x03, 0x21, 0x0C, 0xE9, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x03 },
{ 0xCF, 0x3F, 0x2B, 0xFB, 0x7E, 0xC0, 0x1E, 0x1A, 0xCA, 0x7F, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x10 },
{ 0x17, 0x3F, 0x04, 0x09, 0x7C, 0x03, 0x22, 0x0D, 0xE9, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x03 },
{ 0xCF, 0x3F, 0x0F, 0x5E, 0x7C, 0xC6, 0x13, 0x00, 0xCA, 0x7F, 0x04, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x03 },
{ 0xCF, 0x3F, 0x7E, 0x9D, 0x7C, 0xC8, 0xC0, 0x0A, 0xBA, 0x74, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x06 },
{ 0xCF, 0x3F, 0x4D, 0x9F, 0x7C, 0xC6, 0x00, 0x08, 0xDA, 0x5B, 0x04, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x04 },
{ 0xCF, 0x3F, 0x5D, 0xAA, 0x7A, 0xC0, 0xA4, 0x67, 0x99, 0x7C, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0xCF, 0x3F, 0x4A, 0xFD, 0x7C, 0xCF, 0x00, 0x59, 0xEA, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0x0F, 0x18, 0x0A, 0xFA, 0x57, 0x06, 0x07, 0x06, 0x39, 0x7C, 0x0A, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0xCF, 0x3F, 0x2B, 0xFC, 0x7C, 0xCC, 0xC6, 0x0B, 0xEA, 0x7F, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x10 },
{ 0x05, 0x1A, 0x04, 0x00, 0x7C, 0x12, 0x10, 0x0C, 0xEA, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x07 },
{ 0x04, 0x19, 0x04, 0x00, 0x7C, 0x12, 0x10, 0x2C, 0xEA, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x04 },
{ 0x04, 0x0A, 0x04, 0x00, 0x6C, 0x01, 0x07, 0x0D, 0xFA, 0x74, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x07 },
{ 0x15, 0x14, 0x05, 0x00, 0x7D, 0x01, 0x07, 0x5C, 0xE9, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x05 },
{ 0x10, 0x10, 0x05, 0x08, 0x7C, 0x01, 0x08, 0x0D, 0xEA, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x05 },
{ 0x11, 0x00, 0x06, 0x87, 0x7F, 0x02, 0x40, 0x09, 0x59, 0x68, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x08 },
{ 0x13, 0x26, 0x04, 0x6A, 0x7F, 0x01, 0x00, 0x08, 0x5A, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x08 },
{ 0xCF, 0x4E, 0x0C, 0xAA, 0x50, 0xC4, 0x00, 0x18, 0xF9, 0x54, 0x04, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0xCF, 0x4E, 0x0C, 0xAA, 0x50, 0xC3, 0x00, 0x18, 0xF8, 0x54, 0x04, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0xCB, 0x3F, 0x8F, 0x00, 0x7E, 0xC5, 0x00, 0x98, 0xD6, 0x5F, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x0D },
{ 0x0C, 0x18, 0x87, 0xB3, 0x7F, 0x19, 0x10, 0x55, 0x75, 0x7C, 0x0E, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0x05, 0x11, 0x15, 0x00, 0x64, 0x02, 0x08, 0x08, 0x00, 0x5C, 0x09, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0x04, 0x08, 0x15, 0x00, 0x48, 0x01, 0x08, 0x08, 0x00, 0x60, 0x08, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x02 },
{ 0xDA, 0x00, 0x53, 0x30, 0x68, 0x07, 0x1E, 0x49, 0xC4, 0x7E, 0x03, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 },
{ 0x1C, 0x00, 0x07, 0xBC, 0x6C, 0x0C, 0x14, 0x0B, 0x6A, 0x7E, 0x0B, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x03 },
{ 0x0A, 0x0E, 0x7F, 0x00, 0x7D, 0x13, 0x20, 0x28, 0x03, 0x7C, 0x06, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x00 }
};

static const byte gm_percussion_lookup[128] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C,
	0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0xFF, 0xFF, 0x17, 0x18, 0x19, 0x1A,
	0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x21, 0x22, 0x23, 0xFF, 0xFF,
	0x24, 0x25, 0xFF, 0xFF, 0xFF, 0x26, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static byte lookup_table[64][32];

static const byte volume_table[] = {
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

static int lookup_volume(int a, int b) {
	if (b == 0)
		return 0;

	if (b == 31)
		return a;

	if (a < -63 || a > 63) {
		return b * (a + 1) >> 5;
	}

	if (b < 0) {
		if (a < 0) {
			return lookup_table[-a][-b];
		} else {
			return -lookup_table[a][-b];
		}
	} else {
		if (a < 0) {
			return -lookup_table[-a][b];
		} else {
			return lookup_table[a][b];
		}
	}
}

static void create_lookup_table() {
	int i, j;
	int sum;

	for (i = 0; i < 64; i++) {
		sum = i;
		for (j = 0; j < 32; j++) {
			lookup_table[i][j] = sum >> 5;
			sum += i;
		}
	}
	for (i = 0; i < 64; i++)
		lookup_table[i][0] = 0;
}

////////////////////////////////////////
//
// Adlib MIDI driver
//
////////////////////////////////////////

class MidiDriver_ADLIB : public MidiDriver_Emulated {
	friend class AdlibPart;
	friend class AdlibPercussionChannel;

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
	bool _scummSmallHeader;	// FIXME: This flag controls a special mode for SCUMM V3 games

	FM_OPL *_opl;
	byte *_adlib_reg_cache;

	int _adlib_timer_counter;

	uint16 channel_table_2[9];
	int _voice_index;
	int _timer_p;
	int _timer_q;
	uint16 curnote_table[9];
	AdlibVoice _voices[9];
	AdlibPart _parts[32];
	AdlibPercussionChannel _percussion;

	void generateSamples(int16 *buf, int len);
	void onTimer();
	void part_key_on(AdlibPart *part, AdlibInstrument *instr, byte note, byte velocity);
	void part_key_off(AdlibPart *part, byte note);

	void adlib_key_off(int chan);
	void adlib_note_on(int chan, byte note, int mod);
	void adlib_note_on_ex(int chan, byte note, int mod);
	int adlib_get_reg_value_param(int chan, byte data);
	void adlib_setup_channel(int chan, AdlibInstrument * instr, byte vol_1, byte vol_2);
	byte adlib_get_reg_value(byte reg) {
		return _adlib_reg_cache[reg];
	}
	void adlib_set_param(int channel, byte param, int value);
	void adlib_key_onoff(int channel);
	void adlib_write(byte reg, byte value);
	void adlib_playnote(int channel, int note);

	AdlibVoice *allocate_voice(byte pri);

	void mc_off(AdlibVoice * voice);

	static void link_mc(AdlibPart *part, AdlibVoice *voice);
	void mc_inc_stuff(AdlibVoice *voice, Struct10 * s10, Struct11 * s11);
	void mc_init_stuff(AdlibVoice *voice, Struct10 * s10, Struct11 * s11, byte flags,
						InstrumentExtra * ie);

	void struct10_init(Struct10 * s10, InstrumentExtra * ie);
	static byte struct10_ontimer(Struct10 * s10, Struct11 * s11);
	static void struct10_setup(Struct10 * s10);
	static int random_nr(int a);
	void mc_key_on(AdlibVoice *voice, AdlibInstrument *instr, byte note, byte velocity);
};

// MidiChannel method implementations

void AdlibPart::init(MidiDriver_ADLIB *owner, byte channel) {
	_owner = owner;
	_channel = channel;
	_pri_eff = 127;
	programChange(0);
}

MidiDriver *AdlibPart::device() {
	return _owner;
}

void AdlibPart::send(uint32 b) {
	_owner->send(_channel, b);
}

void AdlibPart::noteOff(byte note) {
#ifdef DEBUG_ADLIB
	debug(6, "%10d: noteOff(%d)", tick, note);
#endif
	_owner->part_key_off(this, note);
}

void AdlibPart::noteOn(byte note, byte velocity) {
#ifdef DEBUG_ADLIB
	debug(6, "%10d: noteOn(%d,%d)", tick, note, velocity);
#endif
	_owner->part_key_on(this, &_part_instr, note, velocity);
}

void AdlibPart::programChange(byte program) {
	if (program > 127)
		return;

	uint i;
	uint count = 0;
	for (i = 0; i < ARRAYSIZE(map_gm_to_fm[0]); ++i)
		count += map_gm_to_fm[program][i];
	if (!count)
		warning("No Adlib instrument defined for GM program %d", (int) program);
	_program = program;
	memcpy(&_part_instr, &map_gm_to_fm[program], sizeof(AdlibInstrument));
}

void AdlibPart::pitchBend(int16 bend) {
	AdlibVoice *voice;

	_pitchbend = bend;
	for (voice = _voice; voice; voice = voice->_next) {
		_owner->adlib_note_on(voice->_channel, voice->_note + _transpose_eff,
						(_pitchbend * _pitchbend_factor >> 6) + _detune_eff);
	}
}

void AdlibPart::controlChange(byte control, byte value) {
	switch (control) {
	case 0:
	case 32:
		break; // Bank select. Not supported
	case 1:   modulationWheel(value); break;
	case 7:   volume(value); break;
	case 10:  break; // Pan position. Not supported.
	case 16:  pitchBendFactor(value); break;
	case 17:  detune(value); break;
	case 18:  priority(value); break;
	case 64:  sustain(value > 0); break;
	case 91:  break; // Effects level. Not supported.
	case 93:  break; // Chorus level. Not supported.
	case 119: break; // Unknown, used in Simon the Sorcerer 2
	case 121: // reset all controllers
		modulationWheel(0);
		pitchBendFactor(0);
		detune(0);
		sustain(0);
		break;
	case 123: allNotesOff(); break;
	default:
		warning("Adlib: Unknown control change message %d (%d)", (int) control, (int)value);
	}
}

void AdlibPart::modulationWheel(byte value) {
	AdlibVoice *voice;

	_modwheel = value;
	for (voice = _voice; voice; voice = voice->_next) {
		if (voice->_s10a.active && voice->_s11a.flag0x40)
			voice->_s10a.modwheel = _modwheel >> 2;
		if (voice->_s10b.active && voice->_s11b.flag0x40)
			voice->_s10b.modwheel = _modwheel >> 2;
	}
}

void AdlibPart::volume(byte value) {
	AdlibVoice *voice;

	_vol_eff = value;
	for (voice = _voice; voice; voice = voice->_next) {
		_owner->adlib_set_param(voice->_channel, 0, volume_table[lookup_table[voice->_vol_2][_vol_eff >> 2]]);
		if (voice->_twochan) {
			_owner->adlib_set_param(voice->_channel, 13, volume_table[lookup_table[voice->_vol_1][_vol_eff >> 2]]);
		}
	}
}

void AdlibPart::pitchBendFactor(byte value) {
	AdlibVoice *voice;

	_pitchbend_factor = value;
	for (voice = _voice; voice; voice = voice->_next) {
		_owner->adlib_note_on(voice->_channel, voice->_note + _transpose_eff,
							(_pitchbend * _pitchbend_factor >> 6) + _detune_eff);
	}
}

void AdlibPart::detune(byte value) {
	AdlibVoice *voice;

	_detune_eff = value;
	for (voice = _voice; voice; voice = voice->_next) {
		_owner->adlib_note_on(voice->_channel, voice->_note + _transpose_eff,
						(_pitchbend * _pitchbend_factor >> 6) + _detune_eff);
	}
}

void AdlibPart::priority(byte value) {
	_pri_eff = value;
}

void AdlibPart::sustain(bool value) {
	AdlibVoice *voice;

	_pedal = value;
	if (!value) {
		for (voice = _voice; voice; voice = voice->_next) {
			if (voice->_waitforpedal)
				_owner->mc_off(voice);
		}
	}
}

void AdlibPart::allNotesOff() {
	while (_voice)
		_owner->mc_off(_voice);
}

void AdlibPart::sysEx_customInstrument(uint32 type, const byte *instr) {
	if (type == 'ADL ') {
		AdlibInstrument *i = &_part_instr;
		memcpy(i, instr, sizeof(AdlibInstrument));
	}
}

// MidiChannel method implementations for percussion

AdlibPercussionChannel::~AdlibPercussionChannel() {
	for (int i = 0; i < ARRAYSIZE(_customInstruments); ++i) {
		delete _customInstruments[i];
	}
}

void AdlibPercussionChannel::init(MidiDriver_ADLIB *owner, byte channel) {
	AdlibPart::init(owner, channel);
	_pri_eff = 0;
	_vol_eff = 127;

	// Initialize the custom instruments data
	memset(_notes, 0, sizeof(_notes));
	memset(_customInstruments, 0, sizeof(_customInstruments));
}

void AdlibPercussionChannel::noteOff(byte note) {
	// Jamieson630: Unless I run into a specific instrument that
	// may require a key off, I'm going to ignore this message.
	// The rationale is that a percussion instrument should
	// fade out of its own accord, and the Adlib instrument
	// definitions used should follow this rule. Since
	// percussion voices are allocated at the lowest priority
	// anyway, we know that "hanging" percussion sounds will
	// not prevent later musical instruments (or even other
	// percussion sounds) from playing.
/*
	_owner->part_key_off(this, note);
*/
}

void AdlibPercussionChannel::noteOn(byte note, byte velocity) {
	AdlibInstrument *inst = NULL;

	// The custom instruments have priority over the default mapping
	inst = _customInstruments[note];
	if (inst)
		note = _notes[note];

	if (!inst) {
		// Use the default GM to FM mapping as a fallback as a fallback
		byte key = gm_percussion_lookup[note];
		if (key != 0xFF)
			inst = (AdlibInstrument *)&gm_percussion_to_fm[key];
	}

	if (!inst) {
		debug(2, "No instrument FM definition for GM percussion key %d", (int)note);
		return;
	}

	_owner->part_key_on(this, inst, note, velocity);
}

void AdlibPercussionChannel::sysEx_customInstrument(uint32 type, const byte *instr) {
	if (type == 'ADLP') {
		byte note = instr[0];
		_notes[note] = instr[1];

		// Allocate memory for the new instruments
		if (!_customInstruments[note]) {
			_customInstruments[note] = new AdlibInstrument;
		}

		// Save the new instrument data
		_customInstruments[note]->mod_characteristic     = instr[2];
		_customInstruments[note]->mod_scalingOutputLevel = instr[3];
		_customInstruments[note]->mod_attackDecay        = instr[4];
		_customInstruments[note]->mod_sustainRelease     = instr[5];
		_customInstruments[note]->mod_waveformSelect     = instr[6];
		_customInstruments[note]->car_characteristic     = instr[7];
		_customInstruments[note]->car_scalingOutputLevel = instr[8];
		_customInstruments[note]->car_attackDecay        = instr[9];
		_customInstruments[note]->car_sustainRelease     = instr[10];
		_customInstruments[note]->car_waveformSelect     = instr[11];
		_customInstruments[note]->feedback               = instr[12];
	}
}

// MidiDriver method implementations

MidiDriver_ADLIB::MidiDriver_ADLIB(Audio::Mixer *mixer)
	: MidiDriver_Emulated(mixer) {
	uint i;

	_scummSmallHeader = false;

	_adlib_reg_cache = 0;

	_adlib_timer_counter = 0;
	_voice_index = 0;
	for (i = 0; i < ARRAYSIZE(curnote_table); ++i) {
		curnote_table[i] = 0;
	}

	for (i = 0; i < ARRAYSIZE(_parts); ++i) {
		_parts[i].init(this, i + ((i >= 9) ? 1 : 0));
	}
	_percussion.init(this, 9);
	_timer_p = 0xD69;
	_timer_q = 0x411B;
}

int MidiDriver_ADLIB::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	MidiDriver_Emulated::open();

	int i;
	AdlibVoice *voice;

	for (i = 0, voice = _voices; i != ARRAYSIZE(_voices); i++, voice++) {
		voice->_channel = i;
		voice->_s11a.s10 = &voice->_s10b;
		voice->_s11b.s10 = &voice->_s10a;
	}

	_adlib_reg_cache = (byte *)calloc(256, 1);

	_opl = makeAdlibOPL(getRate());

	adlib_write(1, 0x20);
	adlib_write(8, 0x40);
	adlib_write(0xBD, 0x00);
	create_lookup_table();

	_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, false, true);

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
			mc_off(&_voices[i]);
	}

	// Turn off the OPL emulation
	OPLDestroy(_opl);
//	YM3812Shutdown();

	free(_adlib_reg_cache);
}

void MidiDriver_ADLIB::send(uint32 b) {
	send(b & 0xF, b & 0xFFFFFFF0);
}

void MidiDriver_ADLIB::send(byte chan, uint32 b) {
	//byte param3 = (byte) ((b >> 24) & 0xFF);
	byte param2 = (byte) ((b >> 16) & 0xFF);
	byte param1 = (byte) ((b >>  8) & 0xFF);
	byte cmd    = (byte) (b & 0xF0);

	AdlibPart *part;
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
				_timer_p = 473;
				_timer_q = 1000;
			} else {
				_timer_p = 0xD69;
				_timer_q = 0x411B;
			}
			return 1;
	}

	return 0;
}

void MidiDriver_ADLIB::setPitchBendRange(byte channel, uint range) {
	AdlibVoice *voice;
	AdlibPart *part = &_parts[channel];

	part->_pitchbend_factor = range;
	for (voice = part->_voice; voice; voice = voice->_next) {
		adlib_note_on(voice->_channel, voice->_note + part->_transpose_eff,
					(part->_pitchbend * part->_pitchbend_factor >> 6) + part->_detune_eff);
	}
}

void MidiDriver_ADLIB::sysEx_customInstrument(byte channel, uint32 type, const byte *instr) {
	_parts[channel].sysEx_customInstrument(type, instr);
}

MidiChannel *MidiDriver_ADLIB::allocateChannel() {
	AdlibPart *part;
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

// All the code brought over from IMuseAdlib

void MidiDriver_ADLIB::adlib_write(byte reg, byte value) {
	if (_adlib_reg_cache[reg] == value)
		return;
#ifdef DEBUG_ADLIB
	debug(6, "%10d: adlib_write[%x] = %x", tick, reg, value);
#endif
	_adlib_reg_cache[reg] = value;

	OPLWriteReg(_opl, reg, value);
}

void MidiDriver_ADLIB::generateSamples(int16 *data, int len) {
	memset(data, 0, sizeof(int16) * len);
	YM3812UpdateOne(_opl, data, len);
}

void MidiDriver_ADLIB::onTimer() {
	AdlibVoice *voice;
	int i;

	_adlib_timer_counter += _timer_p;
	while (_adlib_timer_counter >= _timer_q) {
		_adlib_timer_counter -= _timer_q;
#ifdef DEBUG_ADLIB
		tick++;
#endif
		voice = _voices;
		for (i = 0; i != ARRAYSIZE(_voices); i++, voice++) {
			if (!voice->_part)
				continue;
			if (voice->_duration && (voice->_duration -= 0x11) <= 0) {
				mc_off(voice);
				return;
			}
			if (voice->_s10a.active) {
				mc_inc_stuff(voice, &voice->_s10a, &voice->_s11a);
			}
			if (voice->_s10b.active) {
				mc_inc_stuff(voice, &voice->_s10b, &voice->_s11b);
			}
		}
	}
}

void MidiDriver_ADLIB::mc_off(AdlibVoice *voice) {
	AdlibVoice *tmp;

	adlib_key_off(voice->_channel);

	tmp = voice->_prev;

	if (voice->_next)
		voice->_next->_prev = tmp;
	if (tmp)
		tmp->_next = voice->_next;
	else
		voice->_part->_voice = voice->_next;
	voice->_part = NULL;
}

void MidiDriver_ADLIB::mc_inc_stuff(AdlibVoice *voice, Struct10 *s10, Struct11 *s11) {
	byte code;
	AdlibPart *part = voice->_part;

	code = struct10_ontimer(s10, s11);

	if (code & 1) {
		switch (s11->param) {
		case 0:
			voice->_vol_2 = s10->start_value + s11->modify_val;
			if (!_scummSmallHeader) {
				adlib_set_param(voice->_channel, 0,
												volume_table[lookup_table[voice->_vol_2]
																		 [part->_vol_eff >> 2]]);
			} else {
				adlib_set_param(voice->_channel, 0, voice->_vol_2);
			}
			break;
		case 13:
			voice->_vol_1 = s10->start_value + s11->modify_val;
			if (voice->_twochan && !_scummSmallHeader) {
				adlib_set_param(voice->_channel, 13,
												volume_table[lookup_table[voice->_vol_1]
												[part->_vol_eff >> 2]]);
			} else {
				adlib_set_param(voice->_channel, 13, voice->_vol_1);
			}
			break;
		case 30:
			s11->s10->modwheel = (char)s11->modify_val;
			break;
		case 31:
			s11->s10->unk3 = (char)s11->modify_val;
			break;
		default:
			adlib_set_param(voice->_channel, s11->param,
							s10->start_value + s11->modify_val);
			break;
		}
	}

	if (code & 2 && s11->flag0x10)
		adlib_key_onoff(voice->_channel);
}

void MidiDriver_ADLIB::adlib_key_off(int chan){
	byte reg = chan + 0xB0;
	adlib_write(reg, adlib_get_reg_value(reg) & ~0x20);
}

byte MidiDriver_ADLIB::struct10_ontimer(Struct10 *s10, Struct11 *s11) {
	byte result = 0;
	int i;

	if (s10->count && (s10->count -= 17) <= 0) {
		s10->active = 0;
		return 0;
	}

	i = s10->cur_val + s10->speed_hi;
	s10->speed_lo_counter += s10->speed_lo;
	if (s10->speed_lo_counter >= s10->speed_lo_max) {
		s10->speed_lo_counter -= s10->speed_lo_max;
		i += s10->direction;
	}
	if (s10->cur_val != i || s10->modwheel != s10->modwheel_last) {
		s10->cur_val = i;
		s10->modwheel_last = s10->modwheel;
		i = lookup_volume(i, s10->modwheel_last);
		if (i != s11->modify_val) {
			s11->modify_val = i;
			result = 1;
		}
	}

	if (!--s10->num_steps) {
		s10->active++;
		if (s10->active > 4) {
			if (s10->loop) {
				s10->active = 1;
				result |= 2;
				struct10_setup(s10);
			} else {
				s10->active = 0;
			}
		} else {
			struct10_setup(s10);
		}
	}

	return result;
}

void MidiDriver_ADLIB::adlib_set_param(int channel, byte param, int value) {
	const AdlibSetParams *as;
	byte reg;

	assert(channel >= 0 && channel < 9);

	if (param <= 12) {
		reg = channel_mappings_2[channel];
	} else if (param <= 25) {
		param -= 13;
		reg = channel_mappings[channel];
	} else if (param <= 27) {
		param -= 13;
		reg = channel;
	} else if (param == 28 || param == 29) {
		if (param == 28)
			value -= 15;
		else
			value -= 383;
		value <<= 4;
		channel_table_2[channel] = value;
		adlib_playnote(channel, curnote_table[channel] + value);
		return;
	} else {
		return;
	}

	as = &adlib_setparam_table[param];
	if (as->d)
		value = as->d - value;
	reg += as->a;
	adlib_write(reg, (adlib_get_reg_value(reg) & ~as->c) | (((byte)value) << as->b));
}

void MidiDriver_ADLIB::adlib_key_onoff(int channel) {
	byte val;
	byte reg = channel + 0xB0;
	assert(channel >= 0 && channel < 9);

	val = adlib_get_reg_value(reg);
	adlib_write(reg, val & ~0x20);
	adlib_write(reg, val | 0x20);
}

void MidiDriver_ADLIB::struct10_setup(Struct10 *s10) {
	int b, c, d, e, f, g, h;
	byte t;

	b = s10->unk3;
	f = s10->active - 1;

	t = s10->table_a[f];
	e = num_steps_table[lookup_table[t & 0x7F][b]];
	if (t & 0x80) {
		e = random_nr(e);
	}
	if (e == 0)
		e++;

	s10->num_steps = s10->speed_lo_max = e;

	if (f != 2) {
		c = s10->max_value;
		g = s10->start_value;
		t = s10->table_b[f];
		d = lookup_volume(c, (t & 0x7F) - 31);
		if (t & 0x80) {
			d = random_nr(d);
		}
		if (d + g > c) {
			h = c - g;
		} else {
			h = d;
			if (d + g < 0)
				h = -g;
		}
		h -= s10->cur_val;
	} else {
		h = 0;
	}

	s10->speed_hi = h / e;
	if (h < 0) {
		h = -h;
		s10->direction = -1;
	} else {
		s10->direction = 1;
	}

	s10->speed_lo = h % e;
	s10->speed_lo_counter = 0;
}

void MidiDriver_ADLIB::adlib_playnote(int channel, int note) {
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

	old = adlib_get_reg_value(channel + 0xB0);
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
	adlib_write(channel + 0xA0, note_to_f_num[i]);
	adlib_write(channel + 0xB0, oct | 0x20);
}

int MidiDriver_ADLIB::random_nr(int a) {
	static byte _rand_seed = 1;
	if (_rand_seed & 1) {
		_rand_seed >>= 1;
		_rand_seed ^= 0xB8;
	} else {
		_rand_seed >>= 1;
	}
	return _rand_seed * a >> 8;
}

void MidiDriver_ADLIB::part_key_off(AdlibPart *part, byte note) {
	AdlibVoice *voice;

	for (voice = part->_voice; voice; voice = voice->_next) {
		if (voice->_note == note) {
			if (part->_pedal)
				voice->_waitforpedal = true;
			else
				mc_off(voice);
		}
	}
}

void MidiDriver_ADLIB::part_key_on(AdlibPart *part, AdlibInstrument *instr, byte note, byte velocity) {
	AdlibVoice *voice;

	voice = allocate_voice(part->_pri_eff);
	if (!voice)
		return;

	link_mc(part, voice);
	mc_key_on(voice, instr, note, velocity);
}

AdlibVoice *MidiDriver_ADLIB::allocate_voice(byte pri) {
	AdlibVoice *ac, *best = NULL;
	int i;

	for (i = 0; i < 9; i++) {
		if (++_voice_index >= 9)
			_voice_index = 0;
		ac = &_voices[_voice_index];
		if (!ac->_part)
			return ac;
		if (!ac->_next) {
			if (ac->_part->_pri_eff <= pri) {
				pri = ac->_part->_pri_eff;
				best = ac;
			}
		}
	}

	/* SCUMM V3 games don't have note priorities, first comes wins. */
	if (_scummSmallHeader)
		return NULL;

	if (best)
		mc_off(best);
	return best;
}

void MidiDriver_ADLIB::link_mc(AdlibPart *part, AdlibVoice *voice) {
	voice->_part = part;
	voice->_next = (AdlibVoice *)part->_voice;
	part->_voice = voice;
	voice->_prev = NULL;

	if (voice->_next)
		voice->_next->_prev = voice;
}

void MidiDriver_ADLIB::mc_key_on(AdlibVoice *voice, AdlibInstrument *instr, byte note, byte velocity) {
	AdlibPart *part = voice->_part;
	int c;
	byte vol_1, vol_2;

	voice->_twochan = instr->feedback & 1;
	voice->_note = note;
	voice->_waitforpedal = false;
	voice->_duration = instr->duration;
	if (voice->_duration != 0)
		voice->_duration *= 63;

	if (!_scummSmallHeader)
		vol_1 = (instr->mod_scalingOutputLevel & 0x3F) + lookup_table[velocity >> 1][instr->mod_waveformSelect >> 2];
	else
		vol_1 = 0x3f - (instr->mod_scalingOutputLevel & 0x3F);
	if (vol_1 > 0x3F)
		vol_1 = 0x3F;
	voice->_vol_1 = vol_1;

	if (!_scummSmallHeader)
		vol_2 = (instr->car_scalingOutputLevel & 0x3F) + lookup_table[velocity >> 1][instr->car_waveformSelect >> 2];
	else
		vol_2 = 0x3f - (instr->car_scalingOutputLevel & 0x3F);
	if (vol_2 > 0x3F)
		vol_2 = 0x3F;
	voice->_vol_2 = vol_2;

	c = part->_vol_eff >> 2;

	if (!_scummSmallHeader) {
		vol_2 = volume_table[lookup_table[vol_2][c]];
		if (voice->_twochan)
			vol_1 = volume_table[lookup_table[vol_1][c]];
	}

	adlib_setup_channel(voice->_channel, instr, vol_1, vol_2);
	adlib_note_on_ex(voice->_channel, part->_transpose_eff + note, part->_detune_eff + (part->_pitchbend * part->_pitchbend_factor >> 6));

	if (instr->flags_a & 0x80) {
		mc_init_stuff(voice, &voice->_s10a, &voice->_s11a, instr->flags_a, &instr->extra_a);
	} else {
		voice->_s10a.active = 0;
	}

	if (instr->flags_b & 0x80) {
		mc_init_stuff(voice, &voice->_s10b, &voice->_s11b, instr->flags_b, &instr->extra_b);
	} else {
		voice->_s10b.active = 0;
	}
}

void MidiDriver_ADLIB::adlib_setup_channel(int chan, AdlibInstrument *instr, byte vol_1, byte vol_2) {
	byte channel;

	assert(chan >= 0 && chan < 9);

	channel = channel_mappings[chan];
	adlib_write(channel + 0x20, instr->mod_characteristic);
	adlib_write(channel + 0x40, (instr->mod_scalingOutputLevel | 0x3F) - vol_1 );
	adlib_write(channel + 0x60, 0xff & (~instr->mod_attackDecay));
	adlib_write(channel + 0x80, 0xff & (~instr->mod_sustainRelease));
	adlib_write(channel + 0xE0, instr->mod_waveformSelect);

	channel = channel_mappings_2[chan];
	adlib_write(channel + 0x20, instr->car_characteristic);
	adlib_write(channel + 0x40, (instr->car_scalingOutputLevel | 0x3F) - vol_2 );
	adlib_write(channel + 0x60, 0xff & (~instr->car_attackDecay));
	adlib_write(channel + 0x80, 0xff & (~instr->car_sustainRelease));
	adlib_write(channel + 0xE0, instr->car_waveformSelect);

	adlib_write((byte)chan + 0xC0, instr->feedback);
}

void MidiDriver_ADLIB::adlib_note_on_ex(int chan, byte note, int mod)
{
	int code;
	assert(chan >= 0 && chan < 9);
	code = (note << 7) + mod;
	curnote_table[chan] = code;
	channel_table_2[chan] = 0;
	adlib_playnote(chan, code);
}

void MidiDriver_ADLIB::mc_init_stuff(AdlibVoice *voice, Struct10 * s10,
									Struct11 * s11, byte flags, InstrumentExtra * ie) {
	AdlibPart *part = voice->_part;
	s11->modify_val = 0;
	s11->flag0x40 = flags & 0x40;
	s10->loop = flags & 0x20;
	s11->flag0x10 = flags & 0x10;
	s11->param = param_table_1[flags & 0xF];
	s10->max_value = maxval_table[flags & 0xF];
	s10->unk3 = 31;
	if (s11->flag0x40) {
		s10->modwheel = part->_modwheel >> 2;
	} else {
		s10->modwheel = 31;
	}

	switch (s11->param) {
	case 0:
		s10->start_value = voice->_vol_2;
		break;
	case 13:
		s10->start_value = voice->_vol_1;
		break;
	case 30:
		s10->start_value = 31;
		s11->s10->modwheel = 0;
		break;
	case 31:
		s10->start_value = 0;
		s11->s10->unk3 = 0;
		break;
	default:
		s10->start_value = adlib_get_reg_value_param(voice->_channel, s11->param);
	}

	struct10_init(s10, ie);
}

void MidiDriver_ADLIB::struct10_init(Struct10 *s10, InstrumentExtra *ie) {
	s10->active = 1;
	if (!_scummSmallHeader) {
		s10->cur_val = 0;
	} else {
		s10->cur_val = s10->start_value;
		s10->start_value = 0;
	}
	s10->modwheel_last = 31;
	s10->count = ie->a;
	if (s10->count)
		s10->count *= 63;
	s10->table_a[0] = ie->b;
	s10->table_a[1] = ie->d;
	s10->table_a[2] = ie->f;
	s10->table_a[3] = ie->g;

	s10->table_b[0] = ie->c;
	s10->table_b[1] = ie->e;
	s10->table_b[2] = 0;
	s10->table_b[3] = ie->h;

	struct10_setup(s10);
}

int MidiDriver_ADLIB::adlib_get_reg_value_param(int chan, byte param) {
	const AdlibSetParams *as;
	byte val;
	byte channel;

	assert(chan >= 0 && chan < 9);

	if (param <= 12) {
		channel = channel_mappings_2[chan];
	} else if (param <= 25) {
		param -= 13;
		channel = channel_mappings[chan];
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

	as = &adlib_setparam_table[param];
	val = adlib_get_reg_value(channel + as->a);
	val &= as->c;
	val >>= as->b;
	if (as->d)
		val = as->d - val;

	return val;
}

void MidiDriver_ADLIB::adlib_note_on(int chan, byte note, int mod) {
	int code;
	assert(chan >= 0 && chan < 9);
	code = (note << 7) + mod;
	curnote_table[chan] = code;
	adlib_playnote(chan, (int16) channel_table_2[chan] + code);
}


// Plugin interface

class AdlibEmuMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "AdLib Emulator";
	}

	const char *getId() const {
		return "adlib";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver) const;
};

MusicDevices AdlibEmuMusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_ADLIB));
	return devices;
}

Common::Error AdlibEmuMusicPlugin::createInstance(MidiDriver **mididriver) const {
	*mididriver = new MidiDriver_ADLIB(g_system->getMixer());

	return Common::kNoError;
}

MidiDriver *MidiDriver_ADLIB_create() {
	MidiDriver *mididriver;

	AdlibEmuMusicPlugin p;
	p.createInstance(&mididriver);

	return mididriver;
}

//#if PLUGIN_ENABLED_DYNAMIC(ADLIB)
	//REGISTER_PLUGIN_DYNAMIC(ADLIB, PLUGIN_TYPE_MUSIC, AdlibEmuMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(ADLIB, PLUGIN_TYPE_MUSIC, AdlibEmuMusicPlugin);
//#endif
