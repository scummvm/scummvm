/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "scumm/scumm.h"
#include "sound/fmopl.h"
#include "sound/mididrv.h"
#include "scumm/imuse.h"
#include "scumm/saveload.h"
#include "scumm/sound.h"
#include "common/util.h"

/*
 * Some constants
 */
#define TICKS_PER_BEAT 480

#define SYSEX_ID 0x7D
#define PERCUSSION_CHANNEL 9

#define TRIGGER_ID 0
#define COMMAND_ID 1

#define MDPG_TAG "MDpg"
#define MDHD_TAG "MDhd"

/* Roland to General Midi patch table. Still needs some work. */
static const byte mt32_to_gmidi[128] = {
//    0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	  0,   1,   0,   2,   4,   4,   5,   3,  16,  17,  18,  16,  16,  19,  20,  21, // 0x
	  6,   6,   6,   7,   7,   7,   8, 112,  62,  62,  63,  63,  38,  38,  39,  39, // 1x
	 88,  54,  52,  98,  97,  99,  14,  54, 102,  96,  53, 102,  81, 100,  14,  80, // 2x
	 48,  48,  49,  45,  41,  40,  42,  42,  43,  46,  45,  24,  25,  28,  27, 104, // 3x
	 32,  32,  34,  33,  36,  37,  35,  35,  79,  73,  72,  72,  74,  75,  64,  65, // 4x
	 66,  67,  71,  71,  68,  69,  70,  22,  56,  59,  57,  57,  60,  60,  58,  61, // 5x
	 61,  11,  11,  98,  14,   9,  14,  13,  12, 107, 107,  77,  78,  78,  76,  76, // 6x
	 47, 117, 127, 118, 118, 116, 115, 119, 115, 112,  55, 124, 123,   0,  14, 117  // 7x
};


/* Put IMUSE specific classes here, instead of in a .h file
 * they will only be used from this file, so it will reduce
 * compile time */

class IMuseDriver;

struct Part;
struct MidiChannelAdl;
struct MidiChannelGM;
struct Instrument;


struct HookDatas {
	byte _jump, _transpose;
	byte _part_onoff[16];
	byte _part_volume[16];
	byte _part_program[16];
	byte _part_transpose[16];

	int query_param(int param, byte chan);
	int set(byte cls, byte value, byte chan);
};


struct Player {
	IMuseInternal *_se;

	Part *_parts;
	bool _active;
	bool _scanning;
	int _id;
	byte _priority;
	byte _volume;
	int8 _pan;
	int8 _transpose;
	int8 _detune;
	uint _vol_chan;
	byte _vol_eff;

	uint _song_index;
	uint _track_index;
	uint _timer_counter;
	uint _loop_to_beat;
	uint _loop_from_beat;
	uint _loop_counter;
	uint _loop_to_tick;
	uint _loop_from_tick;
	uint32 _tempo;
	uint32 _tempo_eff;						/* NoSave */
	uint32 _cur_pos;
	uint32 _next_pos;
	uint32 _song_offset;
	uint32 _timer_speed;					/* NoSave */
	uint _tick_index;
	uint _beat_index;
	uint _ticks_per_beat;
	byte _speed;									/* NoSave */
	bool _abort;

	HookDatas _hook;

	bool _mt32emulate;
	bool _isGM;

	/* Player part */
	void hook_clear();
	void clear();
	bool start_sound(int sound);
	void uninit_parts();
	byte *parse_midi(byte *s);
	void key_off(uint8 chan, byte data);
	void key_on(uint8 chan, byte data, byte velocity);
	void part_set_transpose(uint8 chan, byte relative, int8 b);
	void parse_sysex(byte *p, uint len);
	void maybe_jump (byte cmd, uint track, uint beat, uint tick);
	void maybe_set_transpose(byte *data);
	void maybe_part_onoff(byte *data);
	void maybe_set_volume(byte *data);
	void maybe_set_program(byte *data);
	void maybe_set_transpose_part(byte *data);
	uint update_actives();
	Part *get_part(uint8 part);
	void turn_off_pedals();
	int set_vol(byte vol);
	int get_param(int param, byte chan);
	int query_part_param(int param, byte chan);
	int set_transpose(byte relative, int b);
	void set_priority(int pri);
	void set_pan(int pan);
	void set_detune(int detune);
	void turn_off_parts();
	void play_active_notes();
	void cancel_volume_fade();

	static void decode_sysex_bytes(byte *src, byte *dst, int len);

	void clear_active_note(int chan, byte note);
	void set_active_note(int chan, byte note);
	void clear_active_notes();

	/* Sequencer part */
	bool set_loop(uint count, uint tobeat, uint totick, uint frombeat, uint fromtick);
	void clear_loop();
	void set_speed(byte speed);
	bool jump(uint track, uint beat, uint tick);
	void uninit_seq();
	void set_tempo(uint32 data);
	int start_seq_sound(int sound);
	void find_sustaining_notes(byte *a, byte *b, uint32 l);
	int scan(uint totrack, uint tobeat, uint totick);
	int query_param(int param);

	int fade_vol(byte vol, int time);
	bool is_fading_out();
	void sequencer_timer();
};

struct VolumeFader {
	Player *player;
	bool active;
	byte curvol;
	uint16 speed_lo_max, num_steps;
	int8 speed_hi;
	int8 direction;
	int8 speed_lo;
	uint16 speed_lo_counter;

	void initialize() {
		active = false;
	}
	void on_timer(bool probe);
	byte fading_to();
};

struct SustainingNotes {
	SustainingNotes *next;
	SustainingNotes *prev;
	Player *player;
	byte note, chan;
	uint32 off_pos;
	uint32 pos;
	uint16 counter;
};

struct CommandQueue {
	uint16 array[8];
};


struct IsNoteCmdData {
	byte chan;
	byte note;
	byte vel;
};



struct MidiChannel {
	Part *_part;
	MidiChannelAdl *adl() { return (MidiChannelAdl *)this; }
	MidiChannelGM *gm() { return (MidiChannelGM *)this; }
};


struct MidiChannelGM : MidiChannel {
	byte _chan;
	uint16 _actives[8];
};


struct Part {
	int _slot;
	IMuseDriver *_drv;
	Part *_next, *_prev;
	MidiChannel *_mc;
	Player *_player;
	int16 _pitchbend;
	byte _pitchbend_factor;
	int8 _transpose, _transpose_eff;
	byte _vol, _vol_eff;
	int8 _detune, _detune_eff;
	int8 _pan, _pan_eff;
	bool _on;
	byte _modwheel;
	bool _pedal;
	byte _program;
	int8 _pri;
	byte _pri_eff;
	byte _chan;
	byte _effect_level;
	byte _chorus;
	byte _percussion;
	byte _bank;

	void key_on(byte note, byte velocity);
	void key_off(byte note);
	void set_param(byte param, int value);
	void init(IMuseDriver * _driver);
	void setup(Player *player);
	void uninit();
	void off();
	void silence();
	void set_instrument(uint b);
	void set_instrument(Instrument * data);

	void set_transpose(int8 transpose);
	void set_vol(uint8 volume);
	void set_detune(int8 detune);
	void set_pri(int8 pri);
	void set_pan(int8 pan);
	void set_modwheel(uint value);
	void set_pedal(bool value);
	void set_pitchbend(int value);
	void release_pedal();
	void set_program(byte program);
	void set_chorus(uint chorus);
	void set_effect_level(uint level);

	int update_actives(uint16 *active);
	void set_pitchbend_factor(uint8 value);
	void set_onoff(bool on);
	void fix_after_load();

	void update_pris();

	void changed(byte what);
};

/* Abstract IMuseInternal driver class */
class IMuseDriver {
public:
	enum {
		pcMod = 1,
		pcVolume = 2,
		pcPedal = 4,
		pcModwheel = 8,
		pcPan = 16,
		pcEffectLevel = 32,
		pcProgram = 64,
		pcChorus = 128,
		pcAll = 255,
	};

	virtual void on_timer() = 0;
	virtual uint32 get_base_tempo() = 0;
	virtual byte get_hardware_type() = 0;
	virtual void init(IMuseInternal *eng, OSystem *syst) = 0;
	virtual void uninit() = 0;
	virtual void update_pris() = 0;
	virtual void set_instrument(uint slot, byte *instr) = 0;
	virtual void part_set_instrument(Part *part, Instrument * instr) = 0;
	virtual void part_key_on(Part *part, byte note, byte velocity) = 0;
	virtual void part_key_off(Part *part, byte note) = 0;
	virtual void part_off(Part *part) = 0;
	virtual void part_changed(Part *part, byte what) = 0;
	virtual void part_set_param(Part *part, byte param, int value) = 0;
	virtual int part_update_active(Part *part, uint16 *active) = 0;
};


// WARNING: This is the internal variant of the IMUSE class.
// imuse.h contains a public version of the same class.
// the public version, only contains a set of methods.
class IMuseInternal {
	friend struct Player;
private:
	IMuseDriver * _driver;

	byte **_base_sounds;

	byte _locked;
	byte _hardware_type;

private:

	bool _paused;
	bool _active_volume_faders;
	bool _initialized;
	byte _volume_fader_counter;

	int _game_tempo;

	uint _queue_end, _queue_pos, _queue_sound;
	byte _queue_adding;

	SustainingNotes *_sustain_notes_used;
	SustainingNotes *_sustain_notes_free;
	SustainingNotes *_sustain_notes_head;

	byte _queue_marker;
	byte _queue_cleared;
	byte _master_volume;					/* Master volume. 0-127 */
	byte _music_volume;						/* Global music volume. 0-128 */

	uint16 _trigger_count;

	uint16 _channel_volume[8];
	uint16 _channel_volume_eff[8];	/* NoSave */
	uint16 _volchan_table[8];

	Player _players[8];
	SustainingNotes _sustaining_notes[24];
	VolumeFader _volume_fader[8];
	Part _parts[32];

	uint16 _active_notes[128];
	CommandQueue _cmd_queue[64];

	byte *findTag(int sound, char *tag, int index);
	bool isMT32(int sound);
	bool isGM(int sound);
	int get_queue_sound_status(int sound);
	Player *allocate_player(byte priority);
	void handle_marker(uint id, byte data);
	int get_channel_volume(uint a);
	void init_players();
	void init_parts();
	void init_volume_fader();
	void init_sustaining_notes();
	void init_queue();

	void sequencer_timers();
	void expire_sustain_notes();
	void expire_volume_faders();

	Part *allocate_part(byte pri);

	int enqueue_command(int a, int b, int c, int d, int e, int f, int g);
	int enqueue_trigger(int sound, int marker);
	int query_queue(int param);
	Player *get_player_byid(int id);

	int get_volchan_entry(uint a);
	int set_volchan_entry(uint a, uint b);
	int set_channel_volume(uint chan, uint vol);
	void update_volumes();
	void reset_tick();
	VolumeFader *allocate_volume_fader();

	int set_volchan(int sound, int volchan);

	void fix_parts_after_load();
	void fix_players_after_load(Scumm *scumm);

	static int saveReference(IMuseInternal *me, byte type, void *ref);
	static void *loadReference(IMuseInternal *me, byte type, int ref);

	void lock();
	void unlock();

	int set_master_volume_intern(uint vol);

public:
	Part *parts_ptr() {
		return _parts;
	}
	IMuseDriver *driver() {
		return _driver;
	}
	
	int initialize(OSystem *syst, MidiDriver *midi, SoundMixer *mixer);

	/* Public interface */

	void on_timer();
	void pause(bool paused);
	int terminate();
	int save_or_load(Serializer *ser, Scumm *scumm);
	int set_music_volume(uint vol);
	int get_music_volume();
	int set_master_volume(uint vol);
	int get_master_volume();
	bool start_sound(int sound);
	int stop_sound(int sound);
	int stop_all_sounds();
	int get_sound_status(int sound);
	int32 do_command(int a, int b, int c, int d, int e, int f, int g, int h);
	int clear_queue();
	void setBase(byte **base);

	uint32 property(int prop, uint32 value);

	static IMuseInternal *create(OSystem *syst, MidiDriver *midi, SoundMixer *mixer);
};


/* IMuseAdlib classes */

struct Struct10 {
	byte active;
	int16 cur_val;
	int16 count;
	uint16 param;
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

struct InstrumentExtra {
	byte a, b, c, d, e, f, g, h;
};

struct Instrument {
	byte flags_1;
	byte oplvl_1;
	byte atdec_1;
	byte sustrel_1;
	byte waveform_1;
	byte flags_2;
	byte oplvl_2;
	byte atdec_2;
	byte sustrel_2;
	byte waveform_2;
	byte feedback;
	byte flags_a;
	InstrumentExtra extra_a;
	byte flags_b;
	InstrumentExtra extra_b;
	byte duration;
};

struct MidiChannelAdl : MidiChannel {
	MidiChannelAdl *_next, *_prev;
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
};

class IMuseAdlib:public IMuseDriver {
private:
	FM_OPL *_opl;
	byte *_adlib_reg_cache;
	IMuseInternal *_se;
	SoundMixer *_mixer;

	int _adlib_timer_counter;

	uint16 channel_table_2[9];
	int _midichan_index;
	int _next_tick;
	uint16 curnote_table[9];
	MidiChannelAdl _midi_channels[9];

	Instrument _part_instr[32];
	Instrument _glob_instr[32];

	void adlib_key_off(int chan);
	void adlib_note_on(int chan, byte note, int mod);
	void adlib_note_on_ex(int chan, byte note, int mod);
	int adlib_read_param(int chan, byte data);
	void adlib_setup_channel(int chan, Instrument * instr, byte vol_1, byte vol_2);
	byte adlib_read(byte port) {
		return _adlib_reg_cache[port];
	}
	void adlib_set_param(int channel, byte param, int value);
	void adlib_key_onoff(int channel);
	void adlib_write(byte port, byte value);
	void adlib_playnote(int channel, int note);

	MidiChannelAdl *allocate_midichan(byte pri);

	void reset_tick();
	void mc_off(MidiChannel * mc);

	static void link_mc(Part *part, MidiChannelAdl *mc);
	static void mc_inc_stuff(MidiChannelAdl *mc, Struct10 * s10, Struct11 * s11);
	static void mc_init_stuff(MidiChannelAdl *mc, Struct10 * s10, Struct11 * s11, byte flags,
														InstrumentExtra * ie);
	static void struct10_init(Struct10 * s10, InstrumentExtra * ie);
	static byte struct10_ontimer(Struct10 * s10, Struct11 * s11);
	static void struct10_setup(Struct10 * s10);
	static int random_nr(int a);
	void mc_key_on(MidiChannel * mc, byte note, byte velocity);

	static void premix_proc(void *param, int16 *buf, uint len);

public:
	IMuseAdlib(SoundMixer *mixer) {
		_mixer = mixer;
	}
	void uninit() {}
	void init(IMuseInternal *eng, OSystem *syst);
	void update_pris() {
	}
	void generate_samples(int16 *buf, int len);
	void on_timer();
	void set_instrument(uint slot, byte *instr);
	void part_set_instrument(Part *part, Instrument * instr);
	void part_key_on(Part *part, byte note, byte velocity);
	void part_key_off(Part *part, byte note);
	void part_set_param(Part *part, byte param, int value);
	void part_changed(Part *part, byte what);
	void part_off(Part *part);
	int part_update_active(Part *part, uint16 *active);
	void adjust_priorities() {
	}

	uint32 get_base_tempo() {
#ifdef _WIN32_WCE
		return 0x1F0000 * 2;				// Sampled down to 11 kHz
#else	//_WIN32_WCE
		return 0x1F0000;						// Was: 0x1924E0;
#endif //_WIN32_WCE
	}

	byte get_hardware_type() {
		return 1;
	}
};


/* IMuseGM classes */

class IMuseGM : public IMuseDriver {
	IMuseInternal *_se;
	OSystem *_system;
	MidiDriver *_md;
	MidiChannelGM _midi_channels[9];

	int16 _midi_pitchbend_last[16];
	uint8 _midi_volume_last[16];
	bool _midi_pedal_last[16];
	byte _midi_modwheel_last[16];
	byte _midi_effectlevel_last[16];
	byte _midi_chorus_last[16];
	int8 _midi_pan_last[16];


	void midiPitchBend(byte chan, int16 pitchbend);
	void midiVolume(byte chan, byte volume);
	void midiPedal(byte chan, bool pedal);
	void midiModWheel(byte chan, byte modwheel);
	void midiEffectLevel(byte chan, byte level);
	void midiChorus(byte chan, byte chorus);
	void midiControl0(byte chan, byte value);
	void midiProgram(byte chan, byte program, bool mt32emulate);
	void midiPan(byte chan, int8 pan);
	void midiNoteOn(byte chan, byte note, byte velocity);
	void midiNoteOff(byte chan, byte note);
	void midiSilence(byte chan);
	void midiInit();

public:
	IMuseGM(MidiDriver *midi) { _md = midi; }
	void uninit();
	void init(IMuseInternal *eng, OSystem *os);
	void update_pris();
	void part_off(Part *part);
	int part_update_active(Part *part, uint16 *active);

	void on_timer() {}
	void set_instrument(uint slot, byte *instr) {}
	void part_set_instrument(Part *part, Instrument * instr) {}
	void part_set_param(Part *part, byte param, int value) {}
	void part_key_on(Part *part, byte note, byte velocity);
	void part_key_off(Part *part, byte note);
	void part_changed(Part *part, byte what);

	static int midi_driver_thread(void *param);

	uint32 get_base_tempo() { return 0x460000; }
	byte get_hardware_type() { return 5; }
};



//*********************************
//**** IMUSE helper functions ****
//*********************************


static int clamp(int val, int min, int max)
{
	if (val < min)
		return min;
	if (val > max)
		return max;
	return val;
}

static int transpose_clamp(int a, int b, int c)
{
	if (b > a)
		a += (b - a + 11) / 12 * 12;
	if (c < a)
		a -= (a - c + 11) / 12 * 12;
	return a;
}

static uint32 get_delta_time(byte **s)
{
	byte *d = *s, b;
	uint32 time = 0;
	do {
		b = *d++;
		time = (time << 7) | (b & 0x7F);
	} while (b & 0x80);
	*s = d;
	return time;
}

static uint read_word(byte *a)
{
	return (a[0] << 8) + a[1];
}

static void skip_midi_cmd(byte **song_ptr)
{
	byte *s, code;

	const byte num_skip[] = {
		2, 2, 2, 2, 1, 1, 2
	};

	s = *song_ptr;

	code = *s++;

	if (code < 0x80) {
		s = NULL;
	} else if (code < 0xF0) {
		s += num_skip[(code & 0x70) >> 4];
	} else {
		if (code == 0xF0 || code == 0xF7 || code == 0xFF && *s++ != 0x2F) {
			s += get_delta_time(&s);
		} else {
			s = NULL;
		}
	}
	*song_ptr = s;
}

static int is_note_cmd(byte **a, IsNoteCmdData * isnote)
{
	byte *s = *a;
	byte code;

	code = *s++;

	switch (code >> 4) {
	case 8:											/* key off */
		isnote->chan = code & 0xF;
		isnote->note = *s++;
		isnote->vel = *s++;
		*a = s;
		return 1;
	case 9:											/* key on */
		isnote->chan = code & 0xF;
		isnote->note = *s++;
		isnote->vel = *s++;
		*a = s;
		if (isnote->vel)
			return 2;
		return 1;
	case 0xA:
	case 0xB:
	case 0xE:
		s++;
	case 0xC:
	case 0xD:
		s++;
		break;
	case 0xF:
		if (code == 0xF0 || code == 0xF7 || code == 0xFF && *s++ != 0x2F) {
			s += get_delta_time(&s);
			break;
		}
		return -1;
	default:
		return -1;
	}
	*a = s;
	return 0;
}

/**********************************************************************/

void IMuseInternal::lock()
{
	_locked++;
}

void IMuseInternal::unlock()
{
	_locked--;
}

byte *IMuseInternal::findTag(int sound, char *tag, int index)
{
	byte *ptr = NULL;
	int32 size, pos;

	if (_base_sounds)
		ptr = _base_sounds[sound];

	if (ptr == NULL) {
		  debug(1, "IMuseInternal::findTag completely failed finding sound %d", sound);
		return NULL;

	}

	ptr += 8;
	size = READ_BE_UINT32_UNALIGNED(ptr);
	ptr += 4;

	pos = 0;

	while (pos < size) {
		if (!memcmp(ptr + pos, tag, 4) && !index--)
			return ptr + pos + 8;
		pos += READ_BE_UINT32_UNALIGNED(ptr + pos + 4) + 8;
	}

	debug(3, "IMuseInternal::findTag failed finding sound %d", sound);
	return NULL;
}

bool IMuseInternal::isMT32(int sound)
{
	byte *ptr = NULL;
	uint32 tag;

	if (_base_sounds)
		ptr = _base_sounds[sound];

	if (ptr == NULL)
		return false;

	tag = *(((uint32 *)ptr) + 1);
	switch (tag) {
	case MKID('ADL '):
		return false;
	case MKID('ROL '):
		return true;
	case MKID('GMD '):
		return false;
	case MKID('MAC '):
		return true;
	case MKID('SPK '):
		return false;
	}

	return false;
}

bool IMuseInternal::isGM(int sound)
{
	byte *ptr = NULL;
	uint32 tag;

	if (_base_sounds)
		ptr = _base_sounds[sound];

	if (ptr == NULL)
		return false;

	tag = *(((uint32 *)ptr) + 1);
	switch (tag) {
	case MKID('ADL '):
		return false;
	case MKID('ROL '):
		return true; // Yeah... for our purposes, this is GM
	case MKID('GMD '):
		return true;
	case MKID('MIDI'):
		return true;
	case MKID('MAC '):
		return true; // I guess this one too, since it qualifies under isMT32()
	case MKID('SPK '):
		return false;
	}

	return false;
}

bool IMuseInternal::start_sound(int sound)
{
	Player *player;
	void *mdhd;

	mdhd = findTag(sound, MDHD_TAG, 0);
	if (!mdhd) {
		mdhd = findTag(sound, MDPG_TAG, 0);
		if (!mdhd) {
			warning("SE::start_sound failed: Couldn't find sound %d", sound);
			return false;
		}
	}
	player = allocate_player(128);
	if (!player)
		return false;

	player->clear();
	return player->start_sound(sound);
}


Player *IMuseInternal::allocate_player(byte priority)
{
	Player *player = _players, *best = NULL;
	int i;
	byte bestpri = 255;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (!player->_active)
			return player;
		if (player->_priority < bestpri) {
			best = player;
			bestpri = player->_priority;
		}
	}

	if (bestpri < priority)
		return best;

	debug(1, "Denying player request");
	return NULL;
}

void IMuseInternal::init_players()
{
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		player->_active = false;
		player->_se = this;
	}
}

void IMuseInternal::init_sustaining_notes()
{
	SustainingNotes *next = NULL, *sn = _sustaining_notes;
	int i;

	_sustain_notes_used = NULL;
	_sustain_notes_head = NULL;

	for (i = ARRAYSIZE(_sustaining_notes); i != 0; i--, sn++) {
		sn->next = next;
		next = sn;
	}
	_sustain_notes_free = next;
}

void IMuseInternal::init_volume_fader()
{
	VolumeFader *vf = _volume_fader;
	int i;

	for (i = ARRAYSIZE(_volume_fader); i != 0; i--, vf++)
		vf->initialize();

	_active_volume_faders = false;
}

void IMuseInternal::init_parts()
{
	Part *part;
	int i;

	for (i = 0, part = _parts; i != ARRAYSIZE(_parts); i++, part++) {
		part->init(_driver);
		part->_slot = i;
	}
}

int IMuseInternal::stop_sound(int sound)
{
	Player *player = _players;
	int i;
	int r = -1;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->_active && player->_id == sound) {
			player->clear();
			r = 0;
		}
	}
	return r;
}

int IMuseInternal::stop_all_sounds()
{
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->_active)
			player->clear();
	}
	return 0;
}

void IMuseInternal::on_timer()
{
	if (_locked || _paused)
		return;

	lock();

	sequencer_timers();
	expire_sustain_notes();
	expire_volume_faders();
	_driver->on_timer();

	unlock();
}

void IMuseInternal::sequencer_timers()
{
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->_active)
			player->sequencer_timer();
	}
}

void IMuseInternal::handle_marker(uint id, byte data)
{
	uint16 *p;
	uint pos;

	pos = _queue_end;
	if (pos == _queue_pos)
		return;

	if (_queue_adding && _queue_sound == id && data == _queue_marker)
		return;

	p = _cmd_queue[pos].array;

	if (p[0] != TRIGGER_ID || p[1] != id || p[2] != data)
		return;

	_trigger_count--;
	_queue_cleared = false;
	do {
		pos = (pos + 1) & (ARRAYSIZE(_cmd_queue) - 1);
		if (_queue_pos == pos)
			break;
		p = _cmd_queue[pos].array;
		if (*p++ != COMMAND_ID)
			break;
		_queue_end = pos;

		do_command(p[0], p[1], p[2], p[3], p[4], p[5], p[6], 0);

		if (_queue_cleared)
			return;
		pos = _queue_end;
	} while (1);

	_queue_end = pos;
}

int IMuseInternal::get_channel_volume(uint a)
{
	if (a < 8)
		return _channel_volume_eff[a];
	return _master_volume;
}

Part *IMuseInternal::allocate_part(byte pri)
{
	Part *part, *best = NULL;
	int i;

	for (i = ARRAYSIZE(_parts), part = _parts; i != 0; i--, part++) {
		if (!part->_player)
			return part;
		if (pri >= part->_pri_eff) {
			pri = part->_pri_eff;
			best = part;
		}
	}

	if (best)
		best->uninit();
	else
		debug(1, "Denying part request");
	return best;
}

void IMuseInternal::expire_sustain_notes()
{
	SustainingNotes *sn, *next;
	Player *player;
	uint32 counter;

	for (sn = _sustain_notes_head; sn; sn = next) {
		next = sn->next;
		player = sn->player;

		counter = sn->counter + player->_timer_speed;
		sn->pos += counter >> 16;
		sn->counter = (unsigned short)counter & 0xFFFF;

		if (sn->pos >= sn->off_pos) {
			player->key_off(sn->chan, sn->note);

			/* Unlink the node */
			if (next)
				next->prev = sn->prev;
			if (sn->prev)
				sn->prev->next = next;
			else
				_sustain_notes_head = next;

			/* And put it in the free list */
			sn->next = _sustain_notes_free;
			_sustain_notes_free = sn;
		}
	}
}

void IMuseInternal::expire_volume_faders()
{
	VolumeFader *vf;
	int i;

	if (++_volume_fader_counter & 7)
		return;

	if (!_active_volume_faders)
		return;

	_active_volume_faders = false;
	vf = _volume_fader;
	for (i = ARRAYSIZE(_volume_fader); i != 0; i--, vf++) {
		if (vf->active) {
			_active_volume_faders = true;
			vf->on_timer(false);
		}
	}
}

void VolumeFader::on_timer(bool probe)
{
	byte newvol;

	newvol = curvol + speed_hi;
	speed_lo_counter += speed_lo;

	if (speed_lo_counter >= speed_lo_max) {
		speed_lo_counter -= speed_lo_max;
		newvol += direction;
	}

	if (curvol != newvol) {
		curvol = newvol;
		if (!newvol) {
			if (!probe)
				player->clear();
			active = false;
			return;
		}
		if (!probe)
			player->set_vol(newvol);
	}

	if (!--num_steps) {
		active = false;
	}
}

byte VolumeFader::fading_to()
{
	byte newvol;
	byte orig_curvol;
	uint16 orig_speed_lo_counter, orig_num_steps;

	if (!active)
		return 127;

	// It would be so much easier to just store the fade-to volume in a
	// variable, but then we'd have to break savegame compatibility. So
	// instead we do a "dry run" fade.

	orig_speed_lo_counter = speed_lo_counter;
	orig_num_steps = num_steps;
	orig_curvol = curvol;

	while (active)
		on_timer(true);

	active = true;
	newvol = curvol;

	speed_lo_counter = orig_speed_lo_counter;
	num_steps = orig_num_steps;
	curvol = orig_curvol;

	return newvol;
}

int IMuseInternal::get_sound_status(int sound)
{
	int i;
	Player *player;

	for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
		if (player->_active && player->_id == (uint16)sound) {
			// Assume that anyone asking for the sound status is
			// really asking "is it ok if I start playing this
			// sound now?" So if the sound is about to fade out,
			// shut it down and pretend it wasn't playing.
			if (player->is_fading_out()) {
				player->clear();
				continue;
			}
			return 1;
		}
	}
	return get_queue_sound_status(sound);
}

int IMuseInternal::get_queue_sound_status(int sound)
{
	uint16 *a;
	int i, j;

	j = _queue_pos;
	i = _queue_end;

	while (i != j) {
		a = _cmd_queue[i].array;
		if (a[0] == COMMAND_ID && a[1] == 8 && a[2] == (uint16)sound)
			return 2;
		i = (i + 1) & (ARRAYSIZE(_cmd_queue) - 1);
	}
	return 0;
}

int IMuseInternal::set_volchan(int sound, int volchan)
{
	int r;
	int i;
	int num;
	Player *player, *best, *sameid;

	r = get_volchan_entry(volchan);
	if (r == -1)
		return -1;

	if (r >= 8) {
		for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
			if (player->_active && player->_id == (uint16)sound && player->_vol_chan != (uint16)volchan) {
				player->_vol_chan = volchan;
				player->set_vol(player->_volume);
				return 0;
			}
		}
		return -1;
	} else {
		best = NULL;
		num = 0;
		sameid = NULL;
		for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
			if (player->_active) {
				if (player->_vol_chan == (uint16)volchan) {
					num++;
					if (!best || player->_priority <= best->_priority)
						best = player;
				} else if (player->_id == (uint16)sound) {
					sameid = player;
				}
			}
		}
		if (sameid == NULL)
			return -1;
		if (num >= r)
			best->clear();
		player->_vol_chan = volchan;
		player->set_vol(player->_volume);
		return 0;
	}
}

int IMuseInternal::clear_queue()
{
	_queue_adding = false;
	_queue_cleared = true;
	_queue_pos = 0;
	_queue_end = 0;
	_trigger_count = 0;
	return 0;
}

int IMuseInternal::enqueue_command(int a, int b, int c, int d, int e, int f, int g)
{
	uint16 *p;
	uint i;

	i = _queue_pos;

	if (i == _queue_end)
		return -1;

	if (a == -1) {
		_queue_adding = false;
		_trigger_count++;
		return 0;
	}

	p = _cmd_queue[_queue_pos].array;
	p[0] = COMMAND_ID;
	p[1] = a;
	p[2] = b;
	p[3] = c;
	p[4] = d;
	p[5] = e;
	p[6] = f;
	p[7] = g;

	i = (i + 1) & (ARRAYSIZE(_cmd_queue) - 1);

	if (_queue_end != i) {
		_queue_pos = i;
		return 0;
	} else {
		_queue_pos = (i - 1) & (ARRAYSIZE(_cmd_queue) - 1);
		return -1;
	}
}

int IMuseInternal::query_queue(int param)
{
	switch (param) {
	case 0:											/* get trigger count */
		return _trigger_count;
	case 1:											/* get trigger type */
		if (_queue_end == _queue_pos)
			return -1;
		return _cmd_queue[_queue_end].array[1];
	case 2:											/* get trigger sound */
		if (_queue_end == _queue_pos)
			return 0xFF;
		return _cmd_queue[_queue_end].array[2];
	default:
		return -1;
	}
}

int IMuseInternal::get_music_volume()
{
	return _music_volume * 2;
}

int IMuseInternal::set_music_volume(uint vol)
{
	if (vol > 256)
		vol = 256;
	else if (vol < 0)
		vol = 0;

	_music_volume = vol / 2;
	return 0;
}

int IMuseInternal::set_master_volume_intern(uint vol)
{
	if (vol > 127)
		return -1;

	vol = vol * _music_volume / 128;

	_master_volume = vol;
	for (int i = 0; i != 8; i++)
		_channel_volume_eff[i] = (_channel_volume[i] + 1) * vol >> 7;
	update_volumes();

	return 0;
}

int IMuseInternal::set_master_volume(uint vol)
{
	// recalibrate from 0-256 range
	vol = vol * 127 / 256;

	return set_master_volume_intern(vol);
}

int IMuseInternal::get_master_volume()
{
	// recalibrate to 0-256 range
	return _master_volume * 256 / 127;
}

int IMuseInternal::terminate()
{
	if (_driver) {
		_driver->uninit();
		delete _driver;
		_driver = NULL;
	}
	return 0;
	/* not implemented */
}


int IMuseInternal::enqueue_trigger(int sound, int marker)
{
	uint16 *p;
	uint pos;

	pos = _queue_pos;

	p = _cmd_queue[pos].array;
	p[0] = TRIGGER_ID;
	p[1] = sound;
	p[2] = marker;

	pos = (pos + 1) & (ARRAYSIZE(_cmd_queue) - 1);
	if (_queue_end == pos) {
		_queue_pos = (pos - 1) & (ARRAYSIZE(_cmd_queue) - 1);
		return -1;
	}

	_queue_pos = pos;
	_queue_adding = true;
	_queue_sound = sound;
	_queue_marker = marker;
	return 0;
}

int32 IMuseInternal::do_command(int a, int b, int c, int d, int e, int f, int g, int h)
{
	byte cmd = a & 0xFF;
	byte param = a >> 8;
	Player *player = NULL;

	if (!_initialized && (cmd || param))
		return -1;

	if (param == 0) {
		switch (cmd) {
		case 6:
			return set_master_volume_intern(b);
		case 7:
			return _master_volume;
		case 8:
			return start_sound(b) ? 0 : -1;
		case 9:
			return stop_sound(b);
		case 10: // FIXME: Sam and Max - Not sure if this is correct
			return stop_all_sounds();
		case 11:
			return stop_all_sounds();
		case 13:
			return get_sound_status(b);
		case 14:{ // Sam and Max: Volume Fader?
				int i;
				Player *player;

				for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
					if (player->_active && player->_id == (uint16)b) {
						player->fade_vol(e, f);
						return 0;
					}
				}
			}
		case 15:{ // Sam and Max: Unconditional Jump?
				int i;									//      Something to do with position?
				Player *player;
				for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
					if (player->_active && player->_id == (uint16)b) {
						player->jump(player->_track_index + 1, 0, 0);
						return 0;
					}
				}
			}

		case 16:
			return set_volchan(b, c);
		case 17:
			return set_channel_volume(b, c);
		case 18:
			return set_volchan_entry(b, c);
		case 19:
			return stop_sound(b);
		case 20: // FIXME: Deferred command system? - Sam and Max
			return 0;
		case 2:
		case 3:
			return 0;
		default:
			warning("IMuseInternal::do_command invalid command %d", cmd);
		}
	} else if (param == 1) {

		if ((1 << cmd) & (0x783FFF)) {
			player = get_player_byid(b);
			if (!player)
				return -1;
			if ((1 << cmd) & (1 << 11 | 1 << 22)) {
				assert(c >= 0 && c <= 15);
				player = (Player *)player->get_part(c);
				if (!player)
					return -1;
			}
		}

		switch (cmd) {
		case 0:
			return player->get_param(c, d);
		case 1:
			player->set_priority(c);
			return 0;
		case 2:
			return player->set_vol(c);
		case 3:
			player->set_pan(c);
			return 0;
		case 4:
			return player->set_transpose(c, d);
		case 5:
			player->set_detune(c);
			return 0;
		case 6:
			player->set_speed(c);
			return 0;
		case 7:
			return player->jump(c, d, e) ? 0 : -1;
		case 8:
			return player->scan(c, d, e);
		case 9:
			return player->set_loop(c, d, e, f, g) ? 0 : -1;
		case 10:
			player->clear_loop();
			return 0;
		case 11:
			((Part *)player)->set_onoff(d != 0);
			return 0;
		case 12:
			return player->_hook.set(c, d, e);
		case 13:
			return player->fade_vol(c, d);
		case 14:
			return enqueue_trigger(b, c);
		case 15:
			return enqueue_command(b, c, d, e, f, g, h);
		case 16:
			return clear_queue();
		case 19:
			return player->get_param(c, d);
		case 20:
			return player->_hook.set(c, d, e);
		case 21:
			return -1;
		case 22:
			((Part *)player)->set_vol(d);
			return 0;
		case 23:
			return query_queue(b);
		case 24:
			return 0;
		default:
			warning("IMuseInternal::do_command default midi command %d", cmd);
			return -1;
		}
	}

	return -1;
}

int IMuseInternal::set_channel_volume(uint chan, uint vol)
{
	if (chan >= 8 || vol > 127)
		return -1;

	_channel_volume[chan] = vol;
	_channel_volume_eff[chan] = _master_volume * (vol + 1) >> 7;
	update_volumes();
	return 0;
}

void IMuseInternal::update_volumes()
{
	Player *player;
	int i;

	for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
		if (player->_active)
			player->set_vol(player->_volume);
	}
}

int IMuseInternal::set_volchan_entry(uint a, uint b)
{
	if (a >= 8)
		return -1;
	_volchan_table[a] = b;
	return 0;
}

int HookDatas::query_param(int param, byte chan)
{
	switch (param) {
	case 18:
		return _jump;
	case 19:
		return _transpose;
	case 20:
		return _part_onoff[chan];
	case 21:
		return _part_volume[chan];
	case 22:
		return _part_program[chan];
	case 23:
		return _part_transpose[chan];
	default:
		return -1;
	}
}

int HookDatas::set(byte cls, byte value, byte chan)
{
	switch (cls) {
	case 0:
		_jump = value;
		break;
	case 1:
		_transpose = value;
		break;
	case 2:
		if (chan < 16)
			_part_onoff[chan] = value;
		else if (chan == 16)
			memset(_part_onoff, value, 16);
		break;
	case 3:
		if (chan < 16)
			_part_volume[chan] = value;
		else if (chan == 16)
			memset(_part_volume, value, 16);
		break;
	case 4:
		if (chan < 16)
			_part_program[chan] = value;
		else if (chan == 16)
			memset(_part_program, value, 16);
		break;
	case 5:
		if (chan < 16)
			_part_transpose[chan] = value;
		else if (chan == 16)
			memset(_part_transpose, value, 16);
		break;
	default:
		return -1;
	}
	return 0;
}


VolumeFader *IMuseInternal::allocate_volume_fader()
{
	VolumeFader *vf;
	int i;

	vf = _volume_fader;
	for (i = ARRAYSIZE(_volume_fader); vf->active;) {
		vf++;
		if (!--i)
			return NULL;
	}

	vf->active = true;
	_active_volume_faders = true;
	return vf;
}

Player *IMuseInternal::get_player_byid(int id)
{
	int i;
	Player *player, *found = NULL;

	for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
		if (player->_active && player->_id == (uint16)id) {
			if (found)
				return NULL;
			found = player;
		}
	}
	return found;
}

int IMuseInternal::get_volchan_entry(uint a)
{
	if (a < 8)
		return _volchan_table[a];
	return -1;
}

uint32 IMuseInternal::property(int prop, uint32 value)
{
	switch (prop) {
	case IMuse::PROP_TEMPO_BASE:
		_game_tempo = value;
		break;
	}
	return 0;
}

void IMuseInternal::setBase(byte **base)
{
	_base_sounds = base;
}


IMuseInternal *IMuseInternal::create(OSystem *syst, MidiDriver *midi, SoundMixer *mixer)
{
	IMuseInternal *i = new IMuseInternal;
	i->initialize(syst, midi, mixer);
	return i;
}


int IMuseInternal::initialize(OSystem *syst, MidiDriver *midi, SoundMixer *mixer)
{
	int i;

	IMuseDriver *driv;

	if (midi == NULL) {
		driv = new IMuseAdlib(mixer);
	} else {
		driv = new IMuseGM(midi);
	}

	_driver = driv;
	_hardware_type = driv->get_hardware_type();
	_game_tempo = driv->get_base_tempo();

	driv->init(this, syst);

	_master_volume = 127;
	if (_music_volume < 1)
		_music_volume = kDefaultMusicVolume;

	for (i = 0; i != 8; i++)
		_channel_volume[i] = _channel_volume_eff[i] = _volchan_table[i] = 127;

	init_players();
	init_sustaining_notes();
	init_volume_fader();
	init_queue();
	init_parts();

	_initialized = true;

	return 0;
}

void IMuseInternal::init_queue()
{
	_queue_adding = false;
	_queue_pos = 0;
	_queue_end = 0;
	_trigger_count = 0;
}

void IMuseInternal::pause(bool paused)
{
	lock();

	int i;
	Part *part;

	for (i = ARRAYSIZE(_parts), part = _parts; i != 0; i--, part++) {
		if (part->_player) {
			if (paused) {
				part->_vol_eff = 0;
			} else {
				part->set_vol(part->_vol);
			}
			part->changed(IMuseDriver::pcVolume);
		}
	}

	_paused = paused;

	unlock();
}


/*************************************************************************/

int Player::fade_vol(byte vol, int time)
{
	VolumeFader *vf;
	int i;

	cancel_volume_fade();
	if (time == 0) {
		set_vol(vol);
		return 0;
	}

	vf = _se->allocate_volume_fader();
	if (vf == NULL)
		return -1;

	vf->player = this;
	vf->num_steps = vf->speed_lo_max = time;
	vf->curvol = _volume;
	i = (vol - vf->curvol);
	vf->speed_hi = i / time;
	if (i < 0) {
		i = -i;
		vf->direction = -1;
	} else {
		vf->direction = 1;
	}
	vf->speed_lo = i % time;
	vf->speed_lo_counter = 0;
	return 0;
}

bool Player::is_fading_out()
{
	VolumeFader *vf = _se->_volume_fader;
	int i;

	for (i = 0; i < 8; i++, vf++) {
		if (vf->active && vf->direction < 0 && vf->player == this && vf->fading_to() == 0)
			return true;
	}
	return false;
}

void Player::clear()
{
	uninit_seq();
	cancel_volume_fade();
	uninit_parts();
	_active = false;
	_ticks_per_beat = TICKS_PER_BEAT;
}

bool Player::start_sound(int sound)
{
	void *mdhd;

	mdhd = _se->findTag(sound, MDHD_TAG, 0);
	if (mdhd == NULL) {
		mdhd = _se->findTag(sound, MDPG_TAG, 0);
		if (mdhd == NULL) {
				warning("P::start_sound failed: Couldn't find %s", MDHD_TAG);
				return false;
		}
	}

	_mt32emulate = _se->isMT32(sound);
	_isGM = _se->isGM(sound);

	_parts = NULL;
	_active = true;
	_id = sound;
	_priority = 0x80;
	_volume = 0x7F;
	_vol_chan = 0xFFFF;

	_vol_eff = (_se->get_channel_volume(0xFFFF) << 7) >> 7;

	_pan = 0;
	_transpose = 0;
	_detune = 0;

	hook_clear();
	if (start_seq_sound(sound) != 0) {
		_active = false;
		return false;
	}
	return true;
}

void Player::hook_clear()
{
	memset(&_hook, 0, sizeof(_hook));
}

int Player::start_seq_sound(int sound)
{
	byte *ptr, *track_ptr;

	_song_index = sound;
	_timer_counter = 0;
	_loop_to_beat = 1;
	_loop_from_beat = 1;
	_track_index = 0;
	_loop_counter = 0;
	_loop_to_tick = 0;
	_loop_from_tick = 0;

	set_tempo(500000);
	set_speed(128);
	ptr = _se->findTag(sound, "MTrk", _track_index);
	if (ptr == NULL)
		return -1;

	track_ptr = ptr;
	_cur_pos = _next_pos = get_delta_time(&track_ptr);
	_song_offset = track_ptr - ptr;

	_tick_index = _cur_pos;
	_beat_index = 1;

	if (_tick_index >= _ticks_per_beat) {
		_beat_index += _tick_index / _ticks_per_beat;
		_tick_index %= _ticks_per_beat;
	}

	return 0;
}

void Player::set_tempo(uint32 b)
{
	uint32 i, j;

	i = _se->_game_tempo;

	j = _tempo = b;

	while (i & 0xFFFF0000 || j & 0xFFFF0000) {
		i >>= 1;
		j >>= 1;
	}

	_tempo_eff = (i << 16) / j;

	set_speed(_speed);
}

void Player::cancel_volume_fade()
{
	VolumeFader *vf = _se->_volume_fader;
	int i;

	for (i = 0; i < 8; i++, vf++) {
		if (vf->active && vf->player == this)
			vf->active = false;
	}
}

void Player::uninit_parts()
{
	if (_parts && _parts->_player != this)
		error("asd");
	while (_parts)
		_parts->uninit();
}

void Player::uninit_seq()
{
	_abort = true;
}

void Player::set_speed(byte speed)
{
	_speed = speed;
	_timer_speed = (_tempo_eff * speed >> 7);
}

byte *Player::parse_midi(byte *s)
{
	byte cmd, chan, note, velocity, control;
	uint value;
	Part *part;

	cmd = *s++;

	chan = cmd & 0xF;

	switch (cmd >> 4) {
	case 0x8:										/* key off */
		note = *s++;
		if (!_scanning) {
			key_off(chan, note);
		} else {
			clear_active_note(chan, note);
		}
		s++;												/* skip velocity */
		break;

	case 0x9:										/* key on */
		note = *s++;
		velocity = *s++;
		if (velocity) {
			if (!_scanning)
				key_on(chan, note, velocity);
			else
				set_active_note(chan, note);
		} else {
			if (!_scanning)
				key_off(chan, note);
			else
				clear_active_note(chan, note);
		}
		break;

	case 0xA:										/* aftertouch */
		s += 2;
		break;

	case 0xB:										/* control change */
		control = *s++;
		value = *s++;
		part = get_part(chan);
		if (!part)
			break;

		switch (control) {
		case 1:										/* modulation wheel */
			part->set_modwheel(value);
			break;
		case 7:										/* volume */
			part->set_vol(value);
			break;
		case 10:										/* pan position */
			part->set_pan(value - 0x40);
			break;
		case 16:										/* pitchbend factor */
			part->set_pitchbend_factor(value);
			break;
		case 17:										/* gp slider 2 */
			part->set_detune(value - 0x40);
			break;
		case 18:										/* gp slider 3 */
			part->set_pri(value - 0x40);
			_se->_driver->update_pris();
			break;
		case 64:										/* hold pedal */
			part->set_pedal(value != 0);
			break;
		case 91:										/* effects level */
			part->set_effect_level(value);
			break;
		case 93:										/* chorus */
			part->set_chorus(value);
			break;
		default:
			warning("parse_midi: invalid control %d", control);
		}
		break;

	case 0xC:										/* program change */
		value = *s++;
		debug (2, "Player::parse_midi - Setting channel %2d to program %d", chan, value); // Jamieson630: Helps to build the GM-to-FM mapping
		part = get_part(chan);
		if (part)
			part->set_program(value);
		break;

	case 0xD:										/* channel pressure */
		s++;
		break;

	case 0xE:										/* pitch bend */
		part = get_part(chan);
		if (part)
			part->set_pitchbend(((s[1] - 0x40) << 7) | s[0]);
		s += 2;
		break;

	case 0xF:
		if (chan == 0) {
			uint size = get_delta_time(&s);
			if (*s == SYSEX_ID)
				parse_sysex(s, size);
			s += size;
		} else if (chan == 0xF) {
			cmd = *s++;
			if (cmd == 47)
				goto Error;							/* end of song */
			if (cmd == 81) {
				set_tempo((s[1] << 16) | (s[2] << 8) | s[3]);
				s += 4;
				break;
			}
			s += get_delta_time(&s);
		} else if (chan == 0x7) {
			s += get_delta_time(&s);
		} else {
			goto Error;
		}
		break;

	default:
	Error:;
		if (!_scanning)
			clear();
		return NULL;
	}
	return s;
}

void Player::parse_sysex(byte *p, uint len)
{
	byte code;
	byte a;
	uint b;
	byte buf[128];
	Part *part;

	/* too big? */
	if (len >= sizeof(buf) * 2)
		return;

	/* skip sysex manufacturer */
	p++;
	len -= 2;

	switch (code = *p++) {
	case 0:
		if (len > 2) {
			// Part on/off?
			// This seems to do the right thing for Monkey 2, at least.
			a = *p++ & 0x0F;
			part = get_part(a);
			if (part) {
				debug(2, "%d => turning %s part %d", p[1], (p[1] == 2) ? "OFF" : "ON", a);
				part->set_onoff(p[1] != 2);
			}
		} // end if

		// Jamieson630: Sam & Max uses this for something entirely different.
		// The data is much shorter, hence the len > 2 check above.
		break;

	case 1:
		// This SysEx is used in Sam & Max to provide loop (and
		// possibly marker) information. Presently, only the
		// loop information is implemented.
		if (_scanning)
			break;
		maybe_jump (p[0], p[1] - 1, (read_word (p + 2) - 1) * 4 + p[4], ((p[5] * _ticks_per_beat) >> 2) + p[6]);
		break;
		
	case 16:											/* set instrument in part */
		a = *p++ & 0x0F;
		if (_se->_hardware_type != *p++)
			break;
		decode_sysex_bytes(p, buf, len - 3);
		part = get_part(a);
		if (part)
			part->set_instrument((Instrument *) buf);
		break;

	case 17:											/* set global instrument */
		p++;
		if (_se->_hardware_type != *p++)
			break;
		a = *p++;
		decode_sysex_bytes(p, buf, len - 4);
		_se->_driver->set_instrument(a, buf);
		break;

	case 33:											/* param adjust */
		a = *p++ & 0x0F;
		if (_se->_hardware_type != *p++)
			break;
		decode_sysex_bytes(p, buf, len - 3);
		part = get_part(a);
		if (part)
			part->set_param(read_word(buf), read_word(buf + 2));
		break;

	case 48:											/* hook - jump */
		if (_scanning)
			break;
		decode_sysex_bytes(p + 1, buf, len - 2);
		maybe_jump (buf[0], read_word (buf + 1), read_word (buf + 3), read_word (buf + 5));
		break;

	case 49:											/* hook - global transpose */
		decode_sysex_bytes(p + 1, buf, len - 2);
		maybe_set_transpose(buf);
		break;

	case 50:											/* hook - part on/off */
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p, buf + 1, len - 2);
		maybe_part_onoff(buf);
		break;

	case 51:											/* hook - set volume */
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p, buf + 1, len - 2);
		maybe_set_volume(buf);
		break;

	case 52:											/* hook - set program */
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p, buf + 1, len - 2);
		maybe_set_program(buf);
		break;

	case 53:											/* hook - set transpose */
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p, buf + 1, len - 2);
		maybe_set_transpose_part(buf);
		break;

	case 64:											/* marker */
		p++;
		len -= 2;
		while (len--) {
			_se->handle_marker(_id, *p++);
		}
		break;

	case 80:											/* loop */
		decode_sysex_bytes(p + 1, buf, len - 2);
		set_loop(read_word(buf),
						 read_word(buf + 2), read_word(buf + 4), read_word(buf + 6), read_word(buf + 8)
			);
		break;

	case 81:											/* end loop */
		clear_loop();
		break;

	case 96:											/* set instrument */
		part = get_part(p[0] & 0x0F);
		b = (p[1] & 0x0F) << 12 | (p[2] & 0x0F) << 8 | (p[4] & 0x0F) << 4 | (p[4] & 0x0F);
		if (part)
			part->set_instrument(b);
		break;

	default:
		debug(6, "unknown sysex %d", code);
	}
}

void Player::decode_sysex_bytes(byte *src, byte *dst, int len)
{
	while (len >= 0) {
		*dst++ = (src[0] << 4) | (src[1] & 0xF);
		src += 2;
		len -= 2;
	}
}

void Player::maybe_jump (byte cmd, uint track, uint beat, uint tick)
{
	/* is this the hook i'm waiting for? */
	if (cmd && _hook._jump != cmd)
		return;

	/* reset hook? */
	if (cmd != 0 && cmd < 0x80)
		_hook._jump = 0;

	jump (track, beat, tick);
}

void Player::maybe_set_transpose(byte *data)
{
	byte cmd;

	cmd = data[0];

	/* is this the hook i'm waiting for? */
	if (cmd && _hook._transpose != cmd)
		return;

	/* reset hook? */
	if (cmd != 0 && cmd < 0x80)
		_hook._transpose = 0;

	set_transpose(data[1], (int8)data[2]);
}

void Player::maybe_part_onoff(byte *data)
{
	byte cmd, *p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	p = &_hook._part_onoff[chan];

	/* is this the hook i'm waiting for? */
	if (cmd && *p != cmd)
		return;

	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part = get_part(chan);
	if (part)
		part->set_onoff(data[2] != 0);
}

void Player::maybe_set_volume(byte *data)
{
	byte cmd;
	byte *p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	p = &_hook._part_volume[chan];

	/* is this the hook i'm waiting for? */
	if (cmd && *p != cmd)
		return;

	/* reset hook? */
	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part = get_part(chan);
	if (part)
		part->set_vol(data[2]);
}

void Player::maybe_set_program(byte *data)
{
	byte cmd;
	byte *p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	/* is this the hook i'm waiting for? */
	p = &_hook._part_program[chan];

	if (cmd && *p != cmd)
		return;

	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part = get_part(chan);
	if (part)
		part->set_program(data[2]);
}

void Player::maybe_set_transpose_part(byte *data)
{
	byte cmd;
	byte *p;
	uint chan;

	cmd = data[1];
	chan = data[0];

	/* is this the hook i'm waiting for? */
	p = &_hook._part_transpose[chan];

	if (cmd && *p != cmd)
		return;

	/* reset hook? */
	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part_set_transpose(chan, data[2], (int8)data[3]);
}

int Player::set_transpose(byte relative, int b)
{
	Part *part;

	if (b > 24 || b < -24 || relative > 1)
		return -1;
	if (relative)
		b = transpose_clamp(_transpose + b, -7, 7);

	_transpose = b;

	for (part = _parts; part; part = part->_next) {
		part->set_transpose(part->_transpose);
	}

	return 0;
}

void Player::clear_active_notes()
{
	memset(_se->_active_notes, 0, sizeof(_se->_active_notes));
}

void Player::clear_active_note(int chan, byte note)
{
	_se->_active_notes[note] &= ~(1 << chan);
}

void Player::set_active_note(int chan, byte note)
{
	_se->_active_notes[note] |= (1 << chan);
}

void Player::part_set_transpose(uint8 chan, byte relative, int8 b)
{
	Part *part;

	if (b > 24 || b < -24)
		return;

	part = get_part(chan);
	if (!part)
		return;
	if (relative)
		b = transpose_clamp(b + part->_transpose, -7, 7);
	part->set_transpose(b);
}

void Player::key_on(uint8 chan, uint8 note, uint8 velocity)
{
	Part *part;

	part = get_part(chan);
	if (!part || !part->_on)
		return;

	part->key_on(note, velocity);
}

void Player::key_off(uint8 chan, uint8 note)
{
	Part *part;

	for (part = _parts; part; part = part->_next) {
		if (part->_chan == (byte)chan && part->_on)
			part->key_off(note);
	}
}

bool Player::jump(uint track, uint beat, uint tick)
{
	byte *mtrk, *cur_mtrk, *scanpos;
	uint32 topos, curpos, track_offs;

	if (!_active)
		return false;

	mtrk = _se->findTag(_song_index, "MTrk", track);
	if (!mtrk)
		return false;

	cur_mtrk = _se->findTag(_song_index, "MTrk", _track_index);
	if (!cur_mtrk)
		return false;

	_se->lock();

	if (beat == 0)
		beat = 1;

	topos = (beat - 1) * _ticks_per_beat + tick;

	if (track == _track_index && topos >= _cur_pos) {
		scanpos = _song_offset + mtrk;
		curpos = _next_pos;
	} else {
		scanpos = mtrk;
		curpos = get_delta_time(&scanpos);
	}

	while (curpos < topos) {
		skip_midi_cmd(&scanpos);
		if (!scanpos) {
			_se->unlock();
			return false;
		}
		curpos += get_delta_time(&scanpos);
	}

	track_offs = scanpos - mtrk;

	turn_off_pedals();

	find_sustaining_notes(cur_mtrk + _song_offset, mtrk + track_offs, curpos - topos);

	_beat_index = beat;
	_tick_index = tick;
	_cur_pos = topos;
	_next_pos = curpos;
	_timer_counter = 0;
	_song_offset = track_offs;
	if (track != _track_index) {
		_track_index = track;
		_loop_counter = 0;
	}
	_abort = true;
	_se->unlock();
	return true;
}

bool Player::set_loop(uint count, uint tobeat, uint totick, uint frombeat, uint fromtick)
{
	if (tobeat + 1 >= frombeat)
		return false;

	if (tobeat == 0)
		tobeat = 1;

	_loop_counter = 0;						/* because of possible interrupts */
	_loop_to_beat = tobeat;
	_loop_to_tick = totick;
	_loop_from_beat = frombeat;
	_loop_from_tick = fromtick;
	_loop_counter = count;

	return true;
}

void Player::clear_loop()
{
	_loop_counter = 0;
}

void Player::turn_off_pedals()
{
	Part *part;

	for (part = _parts; part; part = part->_next) {
		if (part->_pedal)
			part->set_pedal(false);
	}
}

void Player::find_sustaining_notes(byte *a, byte *b, uint32 l)
{
	uint32 pos;
	uint16 mask;
	uint16 *bitlist_ptr;
	SustainingNotes *sn, *next;
	IsNoteCmdData isnote;
	int j;
	uint num_active;
	uint max_off_pos;

	num_active = update_actives();

	/* pos contains number of ticks since current position */
	pos = _next_pos - _cur_pos;
	if ((int32)pos < 0)
		pos = 0;

	/* locate the positions where the notes are turned off.
	 * remember each note that was turned off
	 */
	while (num_active != 0) {
		/* is note off? */
		j = is_note_cmd(&a, &isnote);
		if (j == -1)
			break;
		if (j == 1) {
			mask = 1 << isnote.chan;
			bitlist_ptr = _se->_active_notes + isnote.note;
			if (*bitlist_ptr & mask) {
				*bitlist_ptr &= ~mask;
				num_active--;
				/* Get a node from the free list */
				if ((sn = _se->_sustain_notes_free) == NULL)
					return;
				_se->_sustain_notes_free = sn->next;

				/* Insert it in the beginning of the used list */
				sn->next = _se->_sustain_notes_used;
				_se->_sustain_notes_used = sn;
				sn->prev = NULL;
				if (sn->next)
					sn->next->prev = sn;

				sn->note = isnote.note;
				sn->chan = isnote.chan;
				sn->player = this;
				sn->off_pos = pos;
				sn->pos = 0;
				sn->counter = 0;
			}
		}
		pos += get_delta_time(&a);
	}

	/* find the maximum position where a note was turned off */
	max_off_pos = 0;
	for (sn = _se->_sustain_notes_used; sn; sn = sn->next) {
		_se->_active_notes[sn->note] |= (1 << sn->chan);
		if (sn->off_pos > max_off_pos) {
			max_off_pos = sn->off_pos;
		}
	}

	/* locate positions where notes are turned on */
	pos = l;
	while (pos < max_off_pos) {
		j = is_note_cmd(&b, &isnote);
		if (j == -1)
			break;
		if (j == 2) {
			mask = 1 << isnote.chan;
			bitlist_ptr = _se->_active_notes + isnote.note;

			if (*bitlist_ptr & mask) {
				sn = _se->_sustain_notes_used;
				while (sn) {
					next = sn->next;
					if (sn->note == isnote.note && sn->chan == isnote.chan && pos < sn->off_pos) {
						*bitlist_ptr &= ~mask;
						/* Unlink from the sustain list */
						if (next)
							next->prev = sn->prev;
						if (sn->prev)
							sn->prev->next = next;
						else
							_se->_sustain_notes_used = next;
						/* Insert into the free list */
						sn->next = _se->_sustain_notes_free;
						_se->_sustain_notes_free = sn;
					}
					sn = next;
				}
			}
		}
		pos += get_delta_time(&b);
	}

	/* Concatenate head and used list */
	if (!_se->_sustain_notes_head) {
		_se->_sustain_notes_head = _se->_sustain_notes_used;
		_se->_sustain_notes_used = NULL;
		return;
	}
	sn = _se->_sustain_notes_head;
	while (sn->next)
		sn = sn->next;
	sn->next = _se->_sustain_notes_used;
	_se->_sustain_notes_used = NULL;
	if (sn->next)
		sn->next->prev = sn;
}

Part *Player::get_part(uint8 chan)
{
	Part *part;

	part = _parts;
	while (part) {
		if (part->_chan == chan)
			return part;
		part = part->_next;
	}

	part = _se->allocate_part(_priority);
	if (!part) {
		warning("no parts available");
		return NULL;
	}

	part->_chan = chan;
	part->setup(this);

	return part;
}

uint Player::update_actives()
{
	Part *part;
	uint16 *active;
	int count = 0;

	clear_active_notes();
	active = _se->_active_notes;
	for (part = _parts; part; part = part->_next) {
		if (part->_mc)
			count += part->update_actives(active);
	}
	return count;
}

void Player::set_priority(int pri)
{
	Part *part;

	_priority = pri;
	for (part = _parts; part; part = part->_next) {
		part->set_pri(part->_pri);
	}
	_se->_driver->update_pris();
}

void Player::set_pan(int pan)
{
	Part *part;

	_pan = pan;
	for (part = _parts; part; part = part->_next) {
		part->set_pan(part->_pan);
	}
}

void Player::set_detune(int detune)
{
	Part *part;

	_detune = detune;
	for (part = _parts; part; part = part->_next) {
		part->set_detune(part->_detune);
	}
}

int Player::scan(uint totrack, uint tobeat, uint totick)
{
	byte *mtrk, *scanptr;
	uint32 curpos, topos;
	uint32 pos;

	assert(totrack >= 0 && tobeat >= 0 && totick >= 0);

	if (!_active)
		return -1;

	mtrk = _se->findTag(_song_index, "MTrk", totrack);
	if (!mtrk)
		return -1;

	_se->lock();
	if (tobeat == 0)
		tobeat++;

	turn_off_parts();
	clear_active_notes();
	scanptr = mtrk;
	curpos = get_delta_time(&scanptr);
	_scanning = true;

	topos = (tobeat - 1) * _ticks_per_beat + totick;

	while (curpos < topos) {
		scanptr = parse_midi(scanptr);
		if (!scanptr) {
			_scanning = false;
			_se->unlock();
			return -1;
		}
		curpos += get_delta_time(&scanptr);
	}
	pos = scanptr - mtrk;

	_scanning = false;
	_se->driver()->update_pris();
	play_active_notes();
	_beat_index = tobeat;
	_tick_index = totick;
	_cur_pos = topos;
	_next_pos = curpos;
	_timer_counter = 0;
	_song_offset = pos;
	if (_track_index != totrack) {
		_track_index = totrack;
		_loop_counter = 0;
	}
	_se->unlock();
	return 0;
}

void Player::turn_off_parts()
{
	Part *part;

	for (part = _parts; part; part = part->_next)
		part->off();
}

void Player::play_active_notes()
{
	int i, j;
	uint mask;

	for (i = 0; i != 128; i++) {
		mask = _se->_active_notes[i];
		for (j = 0; j != 16; j++, mask >>= 1) {
			if (mask & 1) {
				key_on(j, i, 80);
			}
		}
	}
}

int Player::set_vol(byte vol)
{
	Part *part;

	if (vol > 127)
		return -1;

	_volume = vol;
	_vol_eff = _se->get_channel_volume(_vol_chan) * (vol + 1) >> 7;

	for (part = _parts; part; part = part->_next) {
		part->set_vol(part->_vol);
	}

	return 0;
}

int Player::get_param(int param, byte chan)
{
	switch (param) {
	case 0:
		return (byte)_priority;
	case 1:
		return (byte)_volume;
	case 2:
		return (byte)_pan;
	case 3:
		return (byte)_transpose;
	case 4:
		return (byte)_detune;
	case 5:
		return _speed;
	case 6:
		return _track_index;
	case 7:
		return _beat_index;
	case 8:
		return _tick_index;
	case 9:
		return _loop_counter;
	case 10:
		return _loop_to_beat;
	case 11:
		return _loop_to_tick;
	case 12:
		return _loop_from_beat;
	case 13:
		return _loop_from_tick;
	case 14:
	case 15:
	case 16:
	case 17:
		return query_part_param(param, chan);
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
		return _hook.query_param(param, chan);
	default:
		return -1;
	}
}

int Player::query_part_param(int param, byte chan)
{
	Part *part;

	part = _parts;
	while (part) {
		if (part->_chan == chan) {
			switch (param) {
			case 14:
				return part->_on;
			case 15:
				return part->_vol;
			case 16:
				return part->_program;
			case 17:
				return part->_transpose;
			default:
				return -1;
			}
		}
		part = part->_next;
	}
	return 129;
}

void Player::sequencer_timer()
{
	byte *mtrk;
	uint32 counter;
	byte *song_ptr;

	counter = _timer_counter + _timer_speed;
	_timer_counter = counter & 0xFFFF;
	_cur_pos += counter >> 16;
	_tick_index += counter >> 16;

	if (_tick_index >= _ticks_per_beat) {
		_beat_index += _tick_index / _ticks_per_beat;
		_tick_index %= _ticks_per_beat;
	}
	if (_loop_counter && _beat_index >= _loop_from_beat && _tick_index >= _loop_from_tick) {
		_loop_counter--;
		jump(_track_index, _loop_to_beat, _loop_to_tick);
	}
	if (_next_pos <= _cur_pos) {
		mtrk = _se->findTag(_song_index, "MTrk", _track_index);
		if (!mtrk) {
			warning("Sound %d was unloaded while active", _song_index);
			clear();
		} else {
			song_ptr = mtrk + _song_offset;
			_abort = false;

			while (_next_pos <= _cur_pos) {
				song_ptr = parse_midi(song_ptr);
				if (!song_ptr || _abort)
					return;
				_next_pos += get_delta_time(&song_ptr);
			}
			_song_offset = song_ptr - mtrk;
		}
	}
}

/*******************************************************************/

enum {
	TYPE_PART = 1,
	TYPE_PLAYER = 2,
};

int IMuseInternal::saveReference(IMuseInternal *me, byte type, void *ref)
{
	switch (type) {
	case TYPE_PART:
		return (Part *)ref - me->_parts;
	case TYPE_PLAYER:
		return (Player *)ref - me->_players;
	default:
		error("saveReference: invalid type");
	}
}

void *IMuseInternal::loadReference(IMuseInternal *me, byte type, int ref)
{
	switch (type) {
	case TYPE_PART:
		return &me->_parts[ref];
	case TYPE_PLAYER:
		return &me->_players[ref];
	default:
		error("loadReference: invalid type");
	}
}

int IMuseInternal::save_or_load(Serializer *ser, Scumm *scumm)
{
	const SaveLoadEntry mainEntries[] = {
		MKLINE(IMuseInternal, _queue_end, sleUint8),
		MKLINE(IMuseInternal, _queue_pos, sleUint8),
		MKLINE(IMuseInternal, _queue_sound, sleUint16),
		MKLINE(IMuseInternal, _queue_adding, sleByte),
		MKLINE(IMuseInternal, _queue_marker, sleByte),
		MKLINE(IMuseInternal, _queue_cleared, sleByte),
		MKLINE(IMuseInternal, _master_volume, sleByte),
		MKLINE(IMuseInternal, _trigger_count, sleUint16),
		MKARRAY(IMuseInternal, _channel_volume[0], sleUint16, 8),
		MKARRAY(IMuseInternal, _volchan_table[0], sleUint16, 8),
		MKEND()
	};

	const SaveLoadEntry playerEntries[] = {
		MKREF(Player, _parts, TYPE_PART),
		MKLINE(Player, _active, sleByte),
		MKLINE(Player, _id, sleUint16),
		MKLINE(Player, _priority, sleByte),
		MKLINE(Player, _volume, sleByte),
		MKLINE(Player, _pan, sleInt8),
		MKLINE(Player, _transpose, sleByte),
		MKLINE(Player, _detune, sleInt8),
		MKLINE(Player, _vol_chan, sleUint16),
		MKLINE(Player, _vol_eff, sleByte),
		MKLINE(Player, _speed, sleByte),
		MKLINE(Player, _song_index, sleUint16),
		MKLINE(Player, _track_index, sleUint16),
		MKLINE(Player, _timer_counter, sleUint16),
		MKLINE(Player, _loop_to_beat, sleUint16),
		MKLINE(Player, _loop_from_beat, sleUint16),
		MKLINE(Player, _loop_counter, sleUint16),
		MKLINE(Player, _loop_to_tick, sleUint16),
		MKLINE(Player, _loop_from_tick, sleUint16),
		MKLINE(Player, _tempo, sleUint32),
		MKLINE(Player, _cur_pos, sleUint32),
		MKLINE(Player, _next_pos, sleUint32),
		MKLINE(Player, _song_offset, sleUint32),
		MKLINE(Player, _tick_index, sleUint16),
		MKLINE(Player, _beat_index, sleUint16),
		MKLINE(Player, _ticks_per_beat, sleUint16),
		MKLINE(Player, _hook._jump, sleByte),
		MKLINE(Player, _hook._transpose, sleByte),
		MKARRAY(Player, _hook._part_onoff[0], sleByte, 16),
		MKARRAY(Player, _hook._part_volume[0], sleByte, 16),
		MKARRAY(Player, _hook._part_program[0], sleByte, 16),
		MKARRAY(Player, _hook._part_transpose[0], sleByte, 16),
		MKEND()
	};

	const SaveLoadEntry volumeFaderEntries[] = {
		MKREF(VolumeFader, player, TYPE_PLAYER),
		MKLINE(VolumeFader, active, sleUint8),
		MKLINE(VolumeFader, curvol, sleUint8),
		MKLINE(VolumeFader, speed_lo_max, sleUint16),
		MKLINE(VolumeFader, num_steps, sleUint16),
		MKLINE(VolumeFader, speed_hi, sleInt8),
		MKLINE(VolumeFader, direction, sleInt8),
		MKLINE(VolumeFader, speed_lo, sleInt8),
		MKLINE(VolumeFader, speed_lo_counter, sleUint16),
		MKEND()
	};

	const SaveLoadEntry partEntries[] = {
		MKREF(Part, _next, TYPE_PART),
		MKREF(Part, _prev, TYPE_PART),
		MKREF(Part, _player, TYPE_PLAYER),
		MKLINE(Part, _pitchbend, sleInt16),
		MKLINE(Part, _pitchbend_factor, sleUint8),
		MKLINE(Part, _transpose, sleInt8),
		MKLINE(Part, _vol, sleUint8),
		MKLINE(Part, _detune, sleInt8),
		MKLINE(Part, _pan, sleInt8),
		MKLINE(Part, _on, sleUint8),
		MKLINE(Part, _modwheel, sleUint8),
		MKLINE(Part, _pedal, sleUint8),
		MKLINE(Part, _program, sleUint8),
		MKLINE(Part, _pri, sleUint8),
		MKLINE(Part, _chan, sleUint8),
		MKLINE(Part, _effect_level, sleUint8),
		MKLINE(Part, _chorus, sleUint8),
		MKLINE(Part, _percussion, sleUint8),
		MKLINE(Part, _bank, sleUint8),
		MKEND()
	};

	if (!ser->isSaving()) {
		stop_all_sounds();
	}
#ifdef _WIN32_WCE								// Don't break savegames made with andys' build
	if (!ser->isSaving() && ser->checkEOFLoadStream())
		return 0;
#endif

	ser->_ref_me = this;
	ser->_saveload_ref = ser->isSaving()? ((void *)&saveReference) : ((void *)&loadReference);

	ser->saveLoadEntries(this, mainEntries);
	ser->saveLoadArrayOf(_players, ARRAYSIZE(_players), sizeof(_players[0]), playerEntries);
	ser->saveLoadArrayOf(_parts, ARRAYSIZE(_parts), sizeof(_parts[0]), partEntries);
	ser->saveLoadArrayOf(_volume_fader, ARRAYSIZE(_volume_fader),
											 sizeof(_volume_fader[0]), volumeFaderEntries);

	if (!ser->isSaving()) {
		/* Load all sounds that we need */
		fix_players_after_load(scumm);
		init_sustaining_notes();
		_active_volume_faders = true;
		fix_parts_after_load();
		_driver->update_pris();
	}

	return 0;
}

#undef MKLINE
#undef MKEND

void IMuseInternal::fix_parts_after_load()
{
	Part *part;
	int i;

	for (i = ARRAYSIZE(_parts), part = _parts; i != 0; i--, part++) {
		if (part->_player)
			part->fix_after_load();
	}
}

/* Only call this routine from the main thread,
 * since it uses getResourceAddress */
void IMuseInternal::fix_players_after_load(Scumm *scumm)
{
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->_active) {
			player->set_tempo(player->_tempo);
			scumm->getResourceAddress(rtSound, player->_id);
			player->_mt32emulate = isMT32(player->_id);
			player->_isGM = isGM(player->_id);
			if (scumm->_use_adlib) {
				// FIXME - This should make sure the right
				// instruments are loaded, but it does not
				// even try to move to the right position in
				// the track. Using scan() gives a marginally
				// better result, but not good enough.
				//
				// The correct fix is probably to store the
				// Adlib instruments, or information on where
				// to find them, in the savegame.
				player->jump(player->_track_index, 0, 0);
			}
		}
	}
}

void Part::set_detune(int8 detune)
{
	_detune_eff = clamp((_detune = detune) + _player->_detune, -128, 127);
	changed(IMuseDriver::pcMod);
}

void Part::set_pitchbend(int value)
{
	_pitchbend = value;
	changed(IMuseDriver::pcMod);
}

void Part::set_vol(uint8 vol)
{
	_vol_eff = ((_vol = vol) + 1) * _player->_vol_eff >> 7;
	changed(IMuseDriver::pcVolume);
}

void Part::set_pri(int8 pri)
{
	_pri_eff = clamp((_pri = pri) + _player->_priority, 0, 255);
}

void Part::set_pan(int8 pan)
{
	_pan_eff = clamp((_pan = pan) + _player->_pan, -64, 63);
	changed(IMuseDriver::pcPan);
}

void Part::set_transpose(int8 transpose)
{
	_transpose_eff = transpose_clamp((_transpose = transpose) + _player->_transpose, -12, 12);
	changed(IMuseDriver::pcMod);
}

void Part::set_pedal(bool value)
{
	_pedal = value;
	changed(IMuseDriver::pcPedal);
}

void Part::set_modwheel(uint value)
{
	_modwheel = value;
	changed(IMuseDriver::pcModwheel);
}

void Part::set_chorus(uint chorus)
{
	_chorus = chorus;
	changed(IMuseDriver::pcChorus);
}

void Part::set_effect_level(uint level)
{
	_effect_level = level;
	changed(IMuseDriver::pcEffectLevel);
}

void Part::fix_after_load()
{
	set_transpose(_transpose);
	set_vol(_vol);
	set_detune(_detune);
	set_pri(_pri);
	set_pan(_pan);
}

void Part::set_pitchbend_factor(uint8 value)
{
	if (value > 12)
		return;
	set_pitchbend(0);
	_pitchbend_factor = value;
}

void Part::set_onoff(bool on)
{
	if (_on != on) {
		_on = on;
		if (!on)
			off();
		if (!_percussion)
			update_pris();
	}
}

void Part::set_instrument(Instrument * data)
{
	_drv->part_set_instrument(this, data);
}

void Part::key_on(byte note, byte velocity)
{
	_drv->part_key_on(this, note, velocity);
}

void Part::key_off(byte note)
{
	_drv->part_key_off(this, note);
}

void Part::init(IMuseDriver * driver)
{
	_drv = driver;
	_player = NULL;
	_next = NULL;
	_prev = NULL;
	_mc = NULL;
}

void Part::setup(Player *player)
{
	_player = player;

	/* Insert first into player's list */
	_prev = NULL;
	_next = player->_parts;
	if (player->_parts)
		player->_parts->_prev = this;
	player->_parts = this;

	_percussion = true;
	_on = true;
	_pri_eff = player->_priority;
	_pri = 0;
	_vol = 127;
	_vol_eff = player->_vol_eff;
	_pan = clamp(player->_pan, -64, 63);
	_transpose_eff = player->_transpose;
	_transpose = 0;
	_detune = 0;
	_detune_eff = player->_detune;
	_pitchbend_factor = 2;
	_pitchbend = 0;
	_effect_level = 64;
	_program = 255;
	_chorus = 0;
	_modwheel = 0;
	_bank = 0;
	_pedal = false;
	_mc = NULL;
}

void Part::uninit()
{
	if (!_player)
		return;
	off();

	/* unlink */
	if (_next)
		_next->_prev = _prev;
	if (_prev)
		_prev->_next = _next;
	else
		_player->_parts = _next;
	_player = NULL;
	_next = NULL;
	_prev = NULL;
}

void Part::off()
{
	_drv->part_off(this);
}

void Part::changed(byte what)
{
	_drv->part_changed(this, what);
}

void Part::set_param(byte param, int value)
{
	_drv->part_set_param(this, param, value);
}

void Part::update_pris()
{
	_drv->update_pris();
}

int Part::update_actives(uint16 *active)
{
	return _drv->part_update_active(this, active);
}

void Part::set_program(byte program)
{
	if (_program != program || _bank != 0) {
		_program = program;
		_bank = 0;
		changed(IMuseDriver::pcProgram);
	}
}

void Part::set_instrument(uint b)
{
	_bank = (byte)(b >> 8);
	_program = (byte)b;
	changed(IMuseDriver::pcProgram);
}


//********************************************
//***** ADLIB PART OF IMUSE STARTS HERE ******
//********************************************


static byte lookup_table[64][32];
const byte volume_table[] = {
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

int lookup_volume(int a, int b)
{
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

void create_lookup_table()
{
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

MidiChannelAdl *IMuseAdlib::allocate_midichan(byte pri)
{
	MidiChannelAdl *ac, *best = NULL;
	int i;

	for (i = 0; i < 9; i++) {
		if (++_midichan_index >= 9)
			_midichan_index = 0;
		ac = &_midi_channels[_midichan_index];
		if (!ac->_part)
			return ac;
		if (!ac->_next) {
			if (ac->_part->_pri_eff <= pri) {
				pri = ac->_part->_pri_eff;
				best = ac;
			}
		}
	}

	if (best)
		mc_off(best);
	else;													//debug(1, "Denying adlib channel request");
	return best;
}

void IMuseAdlib::premix_proc(void *param, int16 *buf, uint len)
{
	((IMuseAdlib *) param)->generate_samples(buf, len);
}

void IMuseAdlib::init(IMuseInternal *eng, OSystem *syst)
{
	int i;
	MidiChannelAdl *mc;

	_se = eng;

	for (i = 0, mc = _midi_channels; i != ARRAYSIZE(_midi_channels); i++, mc++) {
		mc->_channel = i;
		mc->_s11a.s10 = &mc->_s10b;
		mc->_s11b.s10 = &mc->_s10a;
	}

	_adlib_reg_cache = (byte *)calloc(256, 1);

	_opl = OPLCreate(OPL_TYPE_YM3812, 3579545, syst->property(OSystem::PROP_GET_SAMPLE_RATE, 0));

	adlib_write(1, 0x20);
	adlib_write(8, 0x40);
	adlib_write(0xBD, 0x00);
	create_lookup_table();

	_mixer->setupPremix(this, premix_proc);
}

void IMuseAdlib::adlib_write(byte port, byte value)
{
	if (_adlib_reg_cache[port] == value)
		return;
	_adlib_reg_cache[port] = value;

	OPLWriteReg(_opl, port, value);
}

void IMuseAdlib::adlib_key_off(int chan)
{
	byte port = chan + 0xB0;
	adlib_write(port, adlib_read(port) & ~0x20);
}

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
	{0x40, 0, 63, 63},						/* level */
	{0xE0, 2, 0, 0},							/* unused */
	{0x40, 6, 192, 0},						/* level key scaling */
	{0x20, 0, 15, 0},							/* modulator frequency multiple */
	{0x60, 4, 240, 15},						/* attack rate */
	{0x60, 0, 15, 15},						/* decay rate */
	{0x80, 4, 240, 15},						/* sustain level */
	{0x80, 0, 15, 15},						/* release rate */
	{0xE0, 0, 3, 0},							/* waveform select */
	{0x20, 7, 128, 0},						/* amp mod */
	{0x20, 6, 64, 0},							/* vib */
	{0x20, 5, 32, 0},							/* eg typ */
	{0x20, 4, 16, 0},							/* ksr */
	{0xC0, 0, 1, 0},							/* decay alg */
	{0xC0, 1, 14, 0}							/* feedback */
};

void IMuseAdlib::adlib_set_param(int channel, byte param, int value)
{
	const AdlibSetParams *as;
	byte port;

	assert(channel >= 0 && channel < 9);

	if (param <= 12) {
		port = channel_mappings_2[channel];
	} else if (param <= 25) {
		param -= 13;
		port = channel_mappings[channel];
	} else if (param <= 27) {
		param -= 13;
		port = channel;
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
	port += as->a;
	adlib_write(port, (adlib_read(port) & ~as->c) | (((byte)value) << as->b));
}

static const byte octave_numbers[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7
};

static const byte note_numbers[] = {
	3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14,
	3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14,
	3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14,
	3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14,
	3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14,
	3, 4, 5, 6, 7, 8, 9, 10
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
	242, 243, 245, 247, 249, 251, 252, 254,
};

void IMuseAdlib::adlib_playnote(int channel, int note)
{
	byte old, oct, notex;
	int note2;
	int i;

	note2 = (note >> 7) - 4;

	oct = octave_numbers[note2] << 2;
	notex = note_numbers[note2];

	old = adlib_read(channel + 0xB0);
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

void IMuseAdlib::adlib_note_on(int chan, byte note, int mod)
{
	int code;
	assert(chan >= 0 && chan < 9);
	code = (note << 7) + mod;
	curnote_table[chan] = code;
	adlib_playnote(chan, channel_table_2[chan] + code);
}

void IMuseAdlib::adlib_note_on_ex(int chan, byte note, int mod)
{
	int code;
	assert(chan >= 0 && chan < 9);
	code = (note << 7) + mod;
	curnote_table[chan] = code;
	channel_table_2[chan] = 0;
	adlib_playnote(chan, code);
}

void IMuseAdlib::adlib_key_onoff(int channel)
{
	byte val;
	byte port = channel + 0xB0;
	assert(channel >= 0 && channel < 9);

	val = adlib_read(port);
	adlib_write(port, val & ~0x20);
	adlib_write(port, val | 0x20);
}

void IMuseAdlib::adlib_setup_channel(int chan, Instrument * instr, byte vol_1, byte vol_2)
{
	byte port;

	assert(chan >= 0 && chan < 9);

	port = channel_mappings[chan];
	adlib_write(port + 0x20, instr->flags_1);

        if (!(g_scumm->_features & GF_SMALL_HEADER) || (instr->feedback & 1))
		adlib_write(port + 0x40, (instr->oplvl_1 | 0x3F) - vol_1 );
	else
		adlib_write(port + 0x40, instr->oplvl_1);

	adlib_write(port + 0x60, 0xff & (~instr->atdec_1));
	adlib_write(port + 0x80, 0xff & (~instr->sustrel_1));
	adlib_write(port + 0xE0, instr->waveform_1);

	port = channel_mappings_2[chan];
	adlib_write(port + 0x20, instr->flags_2);
	adlib_write(port + 0x40, (instr->oplvl_2 | 0x3F) - vol_2 );
	adlib_write(port + 0x60, 0xff & (~instr->atdec_2));
	adlib_write(port + 0x80, 0xff & (~instr->sustrel_2));
	adlib_write(port + 0xE0, instr->waveform_2);

	adlib_write((byte)chan + 0xC0, instr->feedback);
}

int IMuseAdlib::adlib_read_param(int chan, byte param)
{
	const AdlibSetParams *as;
	byte val;
	byte port;

	assert(chan >= 0 && chan < 9);

	if (param <= 12) {
		port = channel_mappings_2[chan];
	} else if (param <= 25) {
		param -= 13;
		port = channel_mappings[chan];
	} else if (param <= 27) {
		param -= 13;
		port = chan;
	} else if (param == 28) {
		return 0xF;
	} else if (param == 29) {
		return 0x17F;
	} else {
		return 0;
	}

	as = &adlib_setparam_table[param];
	val = adlib_read(port + as->a);
	val &= as->c;
	val >>= as->b;
	if (as->d)
		val = as->d - val;

	return val;
}

void IMuseAdlib::generate_samples(int16 *data, int len)
{
	int step;

	if (!_opl) {
		memset(data, 0, len * sizeof(int16));
		return;
	}

	do {
		step = len;
		if (step > _next_tick)
			step = _next_tick;
		YM3812UpdateOne(_opl, data, step);

		if (!(_next_tick -= step)) {
			_se->on_timer();
			reset_tick();
		}
		data += step;
	} while (len -= step);
}


void IMuseAdlib::reset_tick()
{
	_next_tick = 88;
}

void IMuseAdlib::on_timer()
{
	MidiChannelAdl *mc;
	int i;

	_adlib_timer_counter += 0xD69;
	while (_adlib_timer_counter >= 0x411B) {
		_adlib_timer_counter -= 0x411B;
		mc = _midi_channels;
		for (i = 0; i != ARRAYSIZE(_midi_channels); i++, mc++) {
			if (!mc->_part)
				continue;
			if (mc->_duration && (mc->_duration -= 0x11) <= 0) {
				mc_off(mc);
				return;
			}
			if (mc->_s10a.active) {
				mc_inc_stuff(mc, &mc->_s10a, &mc->_s11a);
			}
			if (mc->_s10b.active) {
				mc_inc_stuff(mc, &mc->_s10b, &mc->_s11b);
			}
		}
	}
}

const byte param_table_1[16] = {
	29, 28, 27, 0,
	3, 4, 7, 8,
	13, 16, 17, 20,
	21, 30, 31, 0
};

const uint16 param_table_2[16] = {
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

int IMuseAdlib::random_nr(int a)
{
	static byte _rand_seed = 1;
	if (_rand_seed & 1) {
		_rand_seed >>= 1;
		_rand_seed ^= 0xB8;
	} else {
		_rand_seed >>= 1;
	}
	return _rand_seed * a >> 8;
}

void IMuseAdlib::struct10_setup(Struct10 * s10)
{
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
		c = s10->param;
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

byte IMuseAdlib::struct10_ontimer(Struct10 * s10, Struct11 * s11)
{
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

void IMuseAdlib::struct10_init(Struct10 * s10, InstrumentExtra * ie)
{
	s10->active = 1;
	s10->cur_val = 0;
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

void IMuseAdlib::mc_init_stuff(MidiChannelAdl *mc, Struct10 * s10,
															 Struct11 * s11, byte flags, InstrumentExtra * ie)
{
	Part *part = mc->_part;

	s11->modify_val = 0;
	s11->flag0x40 = flags & 0x40;
	s10->loop = flags & 0x20;
	s11->flag0x10 = flags & 0x10;
	s11->param = param_table_1[flags & 0xF];
	s10->param = param_table_2[flags & 0xF];
	s10->unk3 = 31;
	if (s11->flag0x40) {
		s10->modwheel = part->_modwheel >> 2;
	} else {
		s10->modwheel = 31;
	}

	switch (s11->param) {
	case 0:
		s10->start_value = mc->_vol_2;
		break;
	case 13:
		s10->start_value = mc->_vol_1;
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
		s10->start_value = ((IMuseAdlib *) part->_drv)->adlib_read_param(mc->_channel, s11->param);
	}

	struct10_init(s10, ie);
}

void IMuseAdlib::mc_inc_stuff(MidiChannelAdl *mc, Struct10 * s10, Struct11 * s11)
{
	byte code;
	Part *part = mc->_part;

	code = struct10_ontimer(s10, s11);

	if (code & 1) {
		switch (s11->param) {
		case 0:
			mc->_vol_2 = s10->start_value + s11->modify_val;
			((IMuseAdlib *) part->_drv)->adlib_set_param(mc->_channel, 0,
																									 volume_table[lookup_table[mc->_vol_2]
																																[part->_vol_eff >> 2]]);
			break;
		case 13:
			mc->_vol_1 = s10->start_value + s11->modify_val;
			if (mc->_twochan) {
				((IMuseAdlib *) part->_drv)->adlib_set_param(mc->_channel, 13,
																										 volume_table[lookup_table[mc->_vol_1]
																																	[part->_vol_eff >> 2]]);
			} else {
				((IMuseAdlib *) part->_drv)->adlib_set_param(mc->_channel, 13, mc->_vol_1);
			}
			break;
		case 30:
			s11->s10->modwheel = (char)s11->modify_val;
			break;
		case 31:
			s11->s10->unk3 = (char)s11->modify_val;
			break;
		default:
			((IMuseAdlib *) part->_drv)->adlib_set_param(mc->_channel, s11->param,
																									 s10->start_value + s11->modify_val);
			break;
		}
	}

	if (code & 2 && s11->flag0x10)
		((IMuseAdlib *) part->_drv)->adlib_key_onoff(mc->_channel);
}

static byte map_gm_to_fm [128][30] = {
// DERIVED FROM DAY OF THE TENTACLE
{ 0x2F, 0x0B, 0x08, 0x78, 0x16, 0x24, 0x22, 0x0B, 0x9A, 0x34, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x2F, 0x0B, 0x08, 0x78, 0x16, 0x24, 0x22, 0x0B, 0x9A, 0x34, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Extrapolated
{ 0x84, 0x40, 0x3B, 0x5A, 0x63, 0x81, 0x00, 0x3B, 0x5A, 0x7F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x2F, 0x0B, 0x08, 0x78, 0x16, 0x24, 0x22, 0x0B, 0x9A, 0x34, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Extrapolated
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x84, 0x40, 0x3B, 0x5A, 0x63, 0x81, 0x00, 0x3B, 0x5A, 0x7F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x85, 0x80, 0x05, 0xEA, 0x3D, 0x84, 0x18, 0x3C, 0xAA, 0x7C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xE2, 0x00, 0x6C, 0x77, 0x7D, 0xE4, 0x40, 0x7D, 0xA7, 0x66, 0x07, 0xA1, 0x00, 0x02, 0x1E, 0x01, 0x20, 0x01, 0x01, 0x1F, 0xAC, 0x00, 0x04, 0x24, 0x02, 0x21, 0x02, 0x02, 0x21, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xC9, 0x40, 0x3A, 0x78, 0x5E, 0xC2, 0x00, 0x4D, 0x9A, 0x7C, 0x00, 0xA1, 0xA0, 0x08, 0x1F, 0x05, 0x1E, 0x02, 0x05, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x28, 0x1D, 0x19, 0x68, 0x02, 0x22, 0x35, 0x09, 0x08, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Extrapolated
{ 0x28, 0x1D, 0x19, 0x68, 0x02, 0x22, 0x35, 0x09, 0x08, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xC6, 0x40, 0x3B, 0x78, 0x5E, 0xC2, 0x00, 0x4D, 0x9A, 0x7C, 0x00, 0xA1, 0xA0, 0x08, 0x1F, 0x05, 0x1E, 0x02, 0x05, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Extrapolated
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xE2, 0x00, 0x6C, 0x77, 0x7D, 0xE4, 0x40, 0x7D, 0xA7, 0x66, 0x07, 0xA1, 0x00, 0x02, 0x1E, 0x01, 0x20, 0x01, 0x01, 0x1F, 0xAC, 0x00, 0x04, 0x24, 0x02, 0x21, 0x02, 0x02, 0x21, 0x00 },
{ 0xE2, 0x28, 0x38, 0xE8, 0x02, 0xE6, 0x33, 0x0B, 0xF9, 0x00, 0x08, 0xA1, 0x00, 0x02, 0x1E, 0x02, 0x20, 0x00, 0x02, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Extrapolated
{ 0xE2, 0x28, 0x38, 0xE8, 0x02, 0xE6, 0x33, 0x0B, 0xF9, 0x00, 0x08, 0xA1, 0x00, 0x02, 0x1E, 0x02, 0x20, 0x00, 0x02, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xE2, 0x00, 0x6C, 0x77, 0x7D, 0xE4, 0x40, 0x7D, 0xA7, 0x66, 0x07, 0xA1, 0x00, 0x02, 0x1E, 0x01, 0x20, 0x01, 0x01, 0x1F, 0xAC, 0x00, 0x04, 0x24, 0x02, 0x21, 0x02, 0x02, 0x21, 0x00 },
{ 0x85, 0x80, 0x05, 0xEA, 0x3D, 0x84, 0x18, 0x3C, 0xAA, 0x7C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Extrapolated
{ 0x85, 0x80, 0x05, 0xEA, 0x3D, 0x84, 0x18, 0x3C, 0xAA, 0x7C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Extrapolated
{ 0x85, 0x80, 0x05, 0xEA, 0x3D, 0x84, 0x18, 0x3C, 0xAA, 0x7C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xC6, 0x00, 0x2E, 0xC7, 0x59, 0xC2, 0x06, 0x0E, 0xA7, 0x7D, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Extrapolated
{ 0xC6, 0x00, 0x2E, 0xC7, 0x59, 0xC2, 0x06, 0x0E, 0xA7, 0x7D, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Extrapolated
{ 0xC6, 0x00, 0x2E, 0xC7, 0x59, 0xC2, 0x06, 0x0E, 0xA7, 0x7D, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xC6, 0x00, 0x2E, 0xC7, 0x59, 0xC2, 0x06, 0x0E, 0xA7, 0x7D, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x84, 0x40, 0x3B, 0x5A, 0x63, 0x81, 0x00, 0x3B, 0x5A, 0x7F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xC6, 0x00, 0x2E, 0xC7, 0x59, 0xC2, 0x06, 0x0E, 0xA7, 0x7D, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x31, 0x0C, 0x2D, 0xD7, 0x40, 0x62, 0x18, 0x2E, 0xB8, 0x7C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0x00, 0x01, 0x1C, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00 },
{ 0x22, 0x19, 0x79, 0x67, 0x00, 0x22, 0x3F, 0x2A, 0xC6, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x31, 0x0C, 0x2D, 0xD7, 0x40, 0x62, 0x18, 0x2E, 0xB8, 0x7C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0x00, 0x01, 0x1C, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00 },
{ 0x31, 0x0C, 0x2D, 0xD7, 0x40, 0x62, 0x18, 0x2E, 0xB8, 0x7C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0x00, 0x01, 0x1C, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x31, 0x0C, 0x2D, 0xD7, 0x40, 0x62, 0x18, 0x2E, 0xB8, 0x7C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0x00, 0x01, 0x1C, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xE2, 0x15, 0x7B, 0xB5, 0x02, 0xE1, 0x33, 0xAF, 0xF4, 0x36, 0x08, 0xA1, 0x00, 0x04, 0x1E, 0x04, 0x1F, 0x00, 0x04, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xC6, 0x00, 0x2E, 0xC7, 0x59, 0xC2, 0x06, 0x0E, 0xA7, 0x7D, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xF4, 0x40, 0x9F, 0xFA, 0x61, 0xE2, 0x13, 0x7F, 0xFA, 0x7D, 0x02, 0x21, 0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x35, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x02, 0x00, 0x67, 0xAA, 0x65, 0x02, 0x64, 0x28, 0xF9, 0x7C, 0x08, 0x81, 0x00, 0x04, 0x1D, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x02, 0x40, 0x04, 0x9A, 0x55, 0xC2, 0x4B, 0x2B, 0xCB, 0x7C, 0x06, 0x41, 0x00, 0x00, 0x20, 0x06, 0x1C, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x02, 0x0C, 0x03, 0x6A, 0x7D, 0x02, 0x00, 0x23, 0xEA, 0x7C, 0x02, 0x81, 0x00, 0x02, 0x20, 0x01, 0x1F, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xF4, 0x40, 0x9F, 0xFA, 0x61, 0xE2, 0x13, 0x7F, 0xFA, 0x7D, 0x02, 0x21, 0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x35, 0x00 },
{ 0xF4, 0x40, 0x9F, 0xFA, 0x61, 0xE2, 0x13, 0x7F, 0xFA, 0x7D, 0x02, 0x21, 0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x35, 0x00 },
{ 0xF4, 0x40, 0x9F, 0xFA, 0x61, 0xE2, 0x13, 0x7F, 0xFA, 0x7D, 0x02, 0x21, 0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x35, 0x00 },
{ 0x02, 0x00, 0x67, 0xAA, 0x65, 0x02, 0x64, 0x28, 0xF9, 0x7C, 0x08, 0x81, 0x00, 0x04, 0x1D, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xE1, 0x00, 0xCE, 0xD9, 0x4E, 0xE2, 0x00, 0x8F, 0x99, 0x65, 0x0E, 0x01, 0x00, 0x01, 0x1F, 0x00, 0x1E, 0x01, 0x01, 0x20, 0x01, 0x00, 0x00, 0x1F, 0x06, 0x1E, 0x00, 0x06, 0x1F, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xE2, 0x00, 0xCE, 0xD9, 0x4C, 0xE2, 0x00, 0x8F, 0x99, 0x64, 0x0E, 0x81, 0x10, 0x00, 0x1E, 0x05, 0x1F, 0x12, 0x00, 0x1F, 0x01, 0x00, 0x00, 0x1F, 0x06, 0x1E, 0x00, 0x06, 0x1F, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xB2, 0x25, 0xAD, 0xE9, 0x00, 0x62, 0x00, 0x8F, 0xC8, 0x7C, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xF2, 0x00, 0xAF, 0xFA, 0x5C, 0xF2, 0x56, 0x9F, 0xEA, 0x7C, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xF2, 0x00, 0xAF, 0xFA, 0x5C, 0xF2, 0x56, 0x9F, 0xEA, 0x7C, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xB2, 0x25, 0xAD, 0xE9, 0x00, 0x62, 0x00, 0x8F, 0xC8, 0x7C, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xE2, 0x02, 0x9F, 0xB8, 0x48, 0x22, 0x89, 0x9F, 0xE8, 0x7C, 0x00, 0x81, 0x00, 0x01, 0x1E, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xE4, 0x28, 0x7E, 0xF8, 0x01, 0xE2, 0x23, 0x8E, 0xE8, 0x7D, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xE4, 0x28, 0x7E, 0xF8, 0x01, 0xE2, 0x23, 0x8E, 0xE8, 0x7D, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x31, 0x0C, 0x2D, 0xD7, 0x40, 0x62, 0x18, 0x2E, 0xB8, 0x7C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0x00, 0x01, 0x1C, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00 },
{ 0x22, 0x10, 0x7E, 0xD8, 0x35, 0x2A, 0x2E, 0x8E, 0xD8, 0x7C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xE4, 0x28, 0x7E, 0xF8, 0x01, 0xE2, 0x23, 0x8E, 0xE8, 0x7D, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x22, 0x10, 0x7E, 0xD8, 0x35, 0x2A, 0x2E, 0x8E, 0xD8, 0x7C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xE4, 0x08, 0x7E, 0x99, 0x28, 0xE6, 0x16, 0x80, 0xF8, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xE4, 0x23, 0x8F, 0xF9, 0x7C, 0xE2, 0x18, 0x9F, 0x88, 0x7C, 0x01, 0x01, 0x04, 0x00, 0x1E, 0x00, 0x1F, 0x06, 0x05, 0x1F, 0x03, 0x29, 0x01, 0x1F, 0x02, 0x21, 0x01, 0x02, 0x1F, 0x00 },
{ 0xE4, 0x23, 0x8F, 0xF9, 0x7C, 0xE2, 0x18, 0x9F, 0x88, 0x7C, 0x01, 0x01, 0x04, 0x00, 0x1E, 0x00, 0x1F, 0x06, 0x05, 0x1F, 0x03, 0x29, 0x01, 0x1F, 0x02, 0x21, 0x01, 0x02, 0x1F, 0x00 },
{ 0xE4, 0x23, 0x8F, 0xF9, 0x7C, 0xE2, 0x18, 0x9F, 0x88, 0x7C, 0x01, 0x01, 0x04, 0x00, 0x1E, 0x00, 0x1F, 0x06, 0x05, 0x1F, 0x03, 0x29, 0x01, 0x1F, 0x02, 0x21, 0x01, 0x02, 0x1F, 0x00 },
{ 0x2A, 0x1E, 0x98, 0xA9, 0x00, 0x62, 0x00, 0x9F, 0xB9, 0x7C, 0x00, 0x01, 0x00, 0x01, 0x1E, 0x00, 0x1F, 0x01, 0x01, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x2A, 0x1E, 0x98, 0xA9, 0x00, 0x62, 0x00, 0x9F, 0xB9, 0x7C, 0x00, 0x01, 0x00, 0x01, 0x1E, 0x00, 0x1F, 0x01, 0x01, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x62, 0xA8, 0x9D, 0x84, 0x44, 0x62, 0x23, 0x7F, 0xD5, 0x4C, 0x03, 0xA1, 0x00, 0x01, 0x1E, 0x02, 0x21, 0x01, 0x01, 0x20, 0xA8, 0x00, 0x01, 0x24, 0x06, 0x20, 0x04, 0x03, 0x24, 0x00 },
{ 0xE4, 0x28, 0x7E, 0xF8, 0x01, 0xE2, 0x23, 0x8E, 0xE8, 0x7D, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xE4, 0x08, 0x7E, 0x99, 0x28, 0xE6, 0x16, 0x80, 0xF8, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xE4, 0x28, 0x7E, 0xF8, 0x01, 0xE2, 0x23, 0x8E, 0xE8, 0x7D, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x22, 0x10, 0x7E, 0xD8, 0x35, 0x2A, 0x2E, 0x8E, 0xD8, 0x7C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xE4, 0x07, 0x05, 0xAA, 0x7C, 0xE2, 0x50, 0xBE, 0xC8, 0x7D, 0x07, 0x01, 0x00, 0x03, 0x1E, 0x01, 0x1E, 0x00, 0x00, 0x1E, 0xA8, 0x00, 0x01, 0x20, 0x06, 0x23, 0x04, 0x03, 0x20, 0x00 },
{ 0x85, 0x80, 0x05, 0xEA, 0x3D, 0x84, 0x18, 0x3C, 0xAA, 0x7C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xB2, 0x25, 0xAD, 0xE9, 0x00, 0x62, 0x00, 0x8F, 0xC8, 0x7C, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xF2, 0x00, 0xAF, 0xFA, 0x5C, 0xF2, 0x56, 0x9F, 0xEA, 0x7C, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xE2, 0x02, 0x9F, 0xB8, 0x48, 0x22, 0x89, 0x9F, 0xE8, 0x7C, 0x00, 0x81, 0x00, 0x01, 0x1E, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xB2, 0x25, 0xAD, 0xE9, 0x00, 0x62, 0x00, 0x8F, 0xC8, 0x7C, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xE2, 0x00, 0xCE, 0xD9, 0x4C, 0xE2, 0x00, 0x8F, 0x99, 0x64, 0x0E, 0x81, 0x10, 0x00, 0x1E, 0x05, 0x1F, 0x12, 0x00, 0x1F, 0x01, 0x00, 0x00, 0x1F, 0x06, 0x1E, 0x00, 0x06, 0x1F, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xC9, 0x40, 0x3A, 0x38, 0x5E, 0xC2, 0x00, 0x4C, 0xAA, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xC9, 0x40, 0x3A, 0x78, 0x5E, 0xC2, 0x00, 0x4D, 0x9A, 0x7C, 0x00, 0xA1, 0xA0, 0x08, 0x1F, 0x05, 0x1E, 0x02, 0x05, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xC9, 0x40, 0x3A, 0x38, 0x5E, 0xC2, 0x00, 0x4C, 0xAA, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Extrapolated
{ 0xC9, 0x40, 0x3A, 0x78, 0x5E, 0xC2, 0x00, 0x4D, 0x9A, 0x7C, 0x00, 0xA1, 0xA0, 0x08, 0x1F, 0x05, 0x1E, 0x02, 0x05, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Extrapolated
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xC6, 0x00, 0x2E, 0xC7, 0x59, 0xC2, 0x06, 0x0E, 0xA7, 0x7D, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x02, 0x0C, 0x03, 0x6A, 0x7D, 0x02, 0x00, 0x23, 0xEA, 0x7C, 0x02, 0x81, 0x00, 0x02, 0x20, 0x01, 0x1F, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xCF, 0x3B, 0x2A, 0xFE, 0x7E, 0xC0, 0xC0, 0x0C, 0xEB, 0x63, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10 }, // Extrapolated
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x02, 0x0C, 0x03, 0x6A, 0x7D, 0x02, 0x00, 0x23, 0xEA, 0x7C, 0x02, 0x81, 0x00, 0x02, 0x20, 0x01, 0x1F, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x0F, 0x10, 0x10, 0x09, 0x49, 0x02, 0x12, 0x07, 0x9A, 0x7C, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xCF, 0x3B, 0x2A, 0xFE, 0x7E, 0xC0, 0xC0, 0x0C, 0xEB, 0x63, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0xCF, 0x40, 0x0A, 0x30, 0x5C, 0xCF, 0x00, 0x0D, 0x80, 0x7C, 0x00, 0xA0, 0x00, 0x0F, 0x1E, 0x0F, 0x20, 0x00, 0x0B, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Unknown
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // Unknown
};


void IMuseAdlib::part_changed(Part *part, byte what)
{
	MidiChannelAdl *mc;

	if (what & pcProgram) {
               if (part->_player ? part->_player->_isGM : false)
                       part_set_instrument(part, (Instrument *) &map_gm_to_fm[part->_program]);
               else if (part->_program < 32)
                        part_set_instrument(part, &_glob_instr[part->_program]);
	}

	if (what & pcMod) {
		for (mc = part->_mc->adl(); mc; mc = mc->_next) {
			adlib_note_on(mc->_channel, mc->_note + part->_transpose_eff,
			              (part->_pitchbend * part->_pitchbend_factor >> 6) + part->_detune_eff);
		}
	}

	if (what & pcVolume) {
		for (mc = part->_mc->adl(); mc; mc = mc->_next) {
			adlib_set_param(mc->_channel, 0, volume_table[lookup_table[mc->_vol_2]
																										[part->_vol_eff >> 2]]);
			if (mc->_twochan) {
				adlib_set_param(mc->_channel, 13, volume_table[lookup_table[mc->_vol_1]
																											 [part->_vol_eff >> 2]]);
			}
		}
	}

	if (what & pcPedal) {
		if (!part->_pedal) {
			for (mc = (MidiChannelAdl *)part->_mc; mc; mc = mc->_next) {
				if (mc->_waitforpedal)
					mc_off(mc);
			}
		}
	}

	if (what & pcModwheel) {
		for (mc = (MidiChannelAdl *)part->_mc; mc; mc = mc->_next) {
			if (mc->_s10a.active && mc->_s11a.flag0x40)
				mc->_s10a.modwheel = part->_modwheel >> 2;
			if (mc->_s10b.active && mc->_s11b.flag0x40)
				mc->_s10b.modwheel = part->_modwheel >> 2;
		}
	}
}

void IMuseAdlib::mc_key_on(MidiChannel * mc2, byte note, byte velocity)
{
	MidiChannelAdl *mc = (MidiChannelAdl *)mc2;
	Part *part = mc->_part;
	Instrument *instr = &_part_instr[part->_slot];
	int c;
	byte vol_1, vol_2;

	mc->_twochan = instr->feedback & 1;
	mc->_note = note;
	mc->_waitforpedal = false;
	mc->_duration = instr->duration;
	if (mc->_duration != 0)
		mc->_duration *= 63;

	vol_1 = (instr->oplvl_1 & 0x3F) + lookup_table[velocity >> 1][instr->waveform_1 >> 2];
	if (vol_1 > 0x3F)
		vol_1 = 0x3F;
	mc->_vol_1 = vol_1;

	vol_2 = (instr->oplvl_2 & 0x3F) + lookup_table[velocity >> 1][instr->waveform_2 >> 2];
	if (vol_2 > 0x3F)
		vol_2 = 0x3F;
	mc->_vol_2 = vol_2;

	c = part->_vol_eff >> 2;

	vol_2 = volume_table[lookup_table[vol_2][c]];
	if (mc->_twochan)
		vol_1 = volume_table[lookup_table[vol_1][c]];

	adlib_setup_channel(mc->_channel, instr, vol_1, vol_2);
	adlib_note_on_ex(mc->_channel, part->_transpose_eff + note, part->_detune_eff + (part->_pitchbend * part->_pitchbend_factor >> 6));

	if (instr->flags_a & 0x80) {
		mc_init_stuff(mc, &mc->_s10a, &mc->_s11a, instr->flags_a, &instr->extra_a);
	} else {
		mc->_s10a.active = 0;
	}

	if (instr->flags_b & 0x80) {
		mc_init_stuff(mc, &mc->_s10b, &mc->_s11b, instr->flags_b, &instr->extra_b);
	} else {
		mc->_s10b.active = 0;
	}
}

void IMuseAdlib::set_instrument(uint slot, byte *data)
{
	if (slot < 32)
		memcpy(&_glob_instr[slot], data, sizeof(Instrument));
}


void IMuseAdlib::link_mc(Part *part, MidiChannelAdl *mc)
{
	mc->_part = part;
	mc->_next = (MidiChannelAdl *)part->_mc;
	part->_mc = mc;
	mc->_prev = NULL;

	if (mc->_next)
		mc->_next->_prev = mc;
}

void IMuseAdlib::part_key_on(Part *part, byte note, byte velocity)
{
	MidiChannelAdl *mc;

	if ((part->_player ? part->_player->_isGM : false) && part->_chan == 9)
		return; // Jamieson630: GM percussion, not implemented for FM
	mc = allocate_midichan(part->_pri_eff);
	if (!mc)
		return;

	link_mc(part, mc);
	mc_key_on(mc, note, velocity);
}

void IMuseAdlib::part_key_off(Part *part, byte note)
{
	MidiChannelAdl *mc;

	if ((part->_player ? part->_player->_isGM : false) && part->_chan == 9)
		return; // Jamieson630: GM percussion, not implemented for FM
	for (mc = (MidiChannelAdl *)part->_mc; mc; mc = mc->_next) {
		if (mc->_note == note) {
			if (part->_pedal)
				mc->_waitforpedal = true;
			else
				mc_off(mc);
		}
	}
}

struct AdlibInstrSetParams {
	byte param;
	byte shl;
	byte mask;
};

#define MKLINE(_a_,_b_,_c_) { (int)&((Instrument*)0)->_a_, _b_, ((1<<(_c_))-1)<<(_b_) }
static const AdlibInstrSetParams adlib_instr_params[69] = {
	MKLINE(oplvl_2, 0, 6),
	MKLINE(waveform_2, 2, 5),
	MKLINE(oplvl_2, 6, 2),
	MKLINE(flags_2, 0, 4),
	MKLINE(atdec_2, 4, 4),
	MKLINE(atdec_2, 0, 4),
	MKLINE(sustrel_2, 4, 4),
	MKLINE(sustrel_2, 0, 4),
	MKLINE(waveform_2, 0, 2),
	MKLINE(flags_2, 7, 1),
	MKLINE(flags_2, 6, 1),
	MKLINE(flags_2, 5, 1),
	MKLINE(flags_2, 4, 1),

	MKLINE(oplvl_1, 0, 6),
	MKLINE(waveform_1, 2, 5),
	MKLINE(oplvl_1, 6, 2),
	MKLINE(flags_1, 0, 4),
	MKLINE(atdec_1, 4, 4),
	MKLINE(atdec_1, 0, 4),
	MKLINE(sustrel_1, 4, 4),
	MKLINE(sustrel_1, 0, 4),
	MKLINE(waveform_1, 0, 2),
	MKLINE(flags_1, 7, 1),
	MKLINE(flags_1, 6, 1),
	MKLINE(flags_1, 5, 1),
	MKLINE(flags_1, 4, 1),

	MKLINE(feedback, 0, 1),
	MKLINE(feedback, 1, 3),

	MKLINE(flags_a, 7, 1),
	MKLINE(flags_a, 6, 1),
	MKLINE(flags_a, 5, 1),
	MKLINE(flags_a, 4, 1),
	MKLINE(flags_a, 0, 4),
	MKLINE(extra_a.a, 0, 8),
	MKLINE(extra_a.b, 0, 7),
	MKLINE(extra_a.c, 0, 7),
	MKLINE(extra_a.d, 0, 7),
	MKLINE(extra_a.e, 0, 7),
	MKLINE(extra_a.f, 0, 7),
	MKLINE(extra_a.g, 0, 7),
	MKLINE(extra_a.h, 0, 7),
	MKLINE(extra_a.b, 7, 1),
	MKLINE(extra_a.c, 7, 1),
	MKLINE(extra_a.d, 7, 1),
	MKLINE(extra_a.e, 7, 1),
	MKLINE(extra_a.f, 7, 1),
	MKLINE(extra_a.g, 7, 1),
	MKLINE(extra_a.h, 7, 1),

	MKLINE(flags_b, 7, 1),
	MKLINE(flags_b, 6, 1),
	MKLINE(flags_b, 5, 1),
	MKLINE(flags_b, 4, 1),
	MKLINE(flags_b, 0, 4),
	MKLINE(extra_b.a, 0, 8),
	MKLINE(extra_b.b, 0, 7),
	MKLINE(extra_b.c, 0, 7),
	MKLINE(extra_b.d, 0, 7),
	MKLINE(extra_b.e, 0, 7),
	MKLINE(extra_b.f, 0, 7),
	MKLINE(extra_b.g, 0, 7),
	MKLINE(extra_b.h, 0, 7),
	MKLINE(extra_b.b, 7, 1),
	MKLINE(extra_b.c, 7, 1),
	MKLINE(extra_b.d, 7, 1),
	MKLINE(extra_b.e, 7, 1),
	MKLINE(extra_b.f, 7, 1),
	MKLINE(extra_b.g, 7, 1),
	MKLINE(extra_b.h, 7, 1),

	MKLINE(duration, 0, 8),
};
#undef MKLINE

void IMuseAdlib::part_set_param(Part *part, byte param, int value)
{
	const AdlibInstrSetParams *sp = &adlib_instr_params[param];
	byte *p = (byte *)&_part_instr[part->_slot] + sp->param;
	*p = (*p & ~sp->mask) | (value << sp->shl);

	if (param < 28) {
		MidiChannelAdl *mc;

		for (mc = (MidiChannelAdl *)part->_mc; mc; mc = mc->_next) {
			adlib_set_param(mc->_channel, param, value);
		}
	}
}

void IMuseAdlib::part_off(Part *part)
{
	MidiChannelAdl *mc = (MidiChannelAdl *)part->_mc;
	part->_mc = NULL;
	for (; mc; mc = mc->_next) {
		mc_off(mc);
	}
}

void IMuseAdlib::mc_off(MidiChannel * mc2)
{
	MidiChannelAdl *mc = (MidiChannelAdl *)mc2, *tmp;

	adlib_key_off(mc->_channel);

	tmp = mc->_prev;

	if (mc->_next)
		mc->_next->_prev = tmp;
	if (tmp)
		tmp->_next = mc->_next;
	else
		mc->_part->_mc = mc->_next;
	mc->_part = NULL;
}

void IMuseAdlib::part_set_instrument(Part *part, Instrument * instr)
{
	Instrument *i = &_part_instr[part->_slot];
	memcpy(i, instr, sizeof(Instrument));
}

int IMuseAdlib::part_update_active(Part *part, uint16 *active)
{
	uint16 bits;
	int count = 0;
	MidiChannelAdl *mc;

	bits = 1 << part->_chan;

	for (mc = part->_mc->adl(); mc; mc = mc->_next) {
		if (!(active[mc->_note] & bits)) {
			active[mc->_note] |= bits;
			count++;
		}
	}
	return count;
}

//********************************************
//** GENERAL MIDI PART OF IMUSE STARTS HERE **
//********************************************

void IMuseGM::midiPitchBend(byte chan, int16 pitchbend)
{
	uint16 tmp;

	if (_midi_pitchbend_last[chan] != pitchbend) {
		_midi_pitchbend_last[chan] = pitchbend;
		tmp = pitchbend + 0x2000;
		_md->send(((tmp >> 7) & 0x7F) << 16 | (tmp & 0x7F) << 8 | 0xE0 | chan);
	}
}

void IMuseGM::midiVolume(byte chan, byte volume)
{
	if (_midi_volume_last[chan] != volume) {
		_midi_volume_last[chan] = volume;
		_md->send(volume << 16 | 7 << 8 | 0xB0 | chan);
	}
}
void IMuseGM::midiPedal(byte chan, bool pedal)
{
	if (_midi_pedal_last[chan] != pedal) {
		_midi_pedal_last[chan] = pedal;
		_md->send(pedal << 16 | 64 << 8 | 0xB0 | chan);
	}
}

void IMuseGM::midiModWheel(byte chan, byte modwheel)
{
	if (_midi_modwheel_last[chan] != modwheel) {
		_midi_modwheel_last[chan] = modwheel;
		_md->send(modwheel << 16 | 1 << 8 | 0xB0 | chan);
	}
}

void IMuseGM::midiEffectLevel(byte chan, byte level)
{
	if (_midi_effectlevel_last[chan] != level) {
		_midi_effectlevel_last[chan] = level;
		_md->send(level << 16 | 91 << 8 | 0xB0 | chan);
	}
}

void IMuseGM::midiChorus(byte chan, byte chorus)
{
	if (_midi_chorus_last[chan] != chorus) {
		_midi_chorus_last[chan] = chorus;
		_md->send(chorus << 16 | 93 << 8 | 0xB0 | chan);
	}
}

void IMuseGM::midiControl0(byte chan, byte value)
{
	_md->send(value << 16 | 0 << 8 | 0xB0 | chan);
}


void IMuseGM::midiProgram(byte chan, byte program, bool mt32emulate)
{
	if (mt32emulate) {	/* Don't convert the percussion channel, it is the same in GM and MT32 */
		if (chan != PERCUSSION_CHANNEL)
			program = mt32_to_gmidi[program];
	}

	_md->send(program << 8 | 0xC0 | chan);
}

void IMuseGM::midiPan(byte chan, int8 pan)
{
	if (_midi_pan_last[chan] != pan) {
		_midi_pan_last[chan] = pan;
		_md->send(((pan - 64) & 0x7F) << 16 | 10 << 8 | 0xB0 | chan);
	}
}

void IMuseGM::midiNoteOn(byte chan, byte note, byte velocity)
{
	_md->send(velocity << 16 | note << 8 | 0x90 | chan);
}

void IMuseGM::midiNoteOff(byte chan, byte note)
{
	_md->send(note << 8 | 0x80 | chan);
}

void IMuseGM::midiSilence(byte chan)
{
	_md->send((64 << 8) | 0xB0 | chan);
	_md->send((123 << 8) | 0xB0 | chan);
}


void IMuseGM::part_key_on(Part *part, byte note, byte velocity)
{
	MidiChannelGM *mc = part->_mc->gm();

	if (mc) {
		mc->_actives[note >> 4] |= (1 << (note & 0xF));
		midiNoteOn(mc->_chan, note, velocity);
	} else if (part->_percussion) {
		midiVolume(PERCUSSION_CHANNEL, part->_vol_eff);
		midiProgram(PERCUSSION_CHANNEL, part->_bank, part->_player->_mt32emulate);
		midiNoteOn(PERCUSSION_CHANNEL, note, velocity);
	}
}

void IMuseGM::part_key_off(Part *part, byte note)
{
	MidiChannelGM *mc = part->_mc->gm();

	if (mc) {
		mc->_actives[note >> 4] &= ~(1 << (note & 0xF));
		midiNoteOff(mc->_chan, note);
	} else if (part->_percussion) {
		midiNoteOff(PERCUSSION_CHANNEL, note);
	}
}

#if !defined(__MORPHOS__)
int IMuseGM::midi_driver_thread(void *param)
{
	IMuseGM *mid = (IMuseGM *) param;
	int old_time, cur_time;

	old_time = mid->_system->get_msecs();

	for (;;) {
		mid->_system->delay_msecs(10);

		cur_time = mid->_system->get_msecs();
		while (old_time < cur_time) {
			old_time += 10;
			mid->_se->on_timer();
		}
	}
}
#else
#include <proto/exec.h>
#include <proto/dos.h>
#include "morphos.h"
#include "morphos_sound.h"
int IMuseGM::midi_driver_thread(void *param)
{
	IMuseGM *mid = (IMuseGM *) param;
	int old_time, cur_time;
	MsgPort *music_timer_port = NULL;
	timerequest *music_timer_request = NULL;

	ObtainSemaphore(&ScummMusicThreadRunning);

	if (!OSystem_MorphOS::OpenATimer(&music_timer_port, (IORequest **) &music_timer_request, UNIT_MICROHZ, false)) {
		warning("Could not open a timer - music will not play");
		Wait(SIGBREAKF_CTRL_C);
	}
	else {
		old_time = mid->_system->get_msecs();

		for (;;) {
			music_timer_request->tr_node.io_Command = TR_ADDREQUEST;
			music_timer_request->tr_time.tv_secs = 0;
			music_timer_request->tr_time.tv_micro = 10000;
			DoIO((struct IORequest *)music_timer_request);

			if (CheckSignal(SIGBREAKF_CTRL_C))
				break;

			cur_time = mid->_system->get_msecs();
			while (old_time < cur_time) {
				old_time += 10;
				mid->_se->on_timer();
			}
		}
	}

	ReleaseSemaphore(&ScummMusicThreadRunning);
	RemTask(NULL);
	return 0;
}
#endif

void IMuseGM::init(IMuseInternal *eng, OSystem *syst)
{
	int i;
	MidiChannelGM *mc;

	_system = syst;

	/* open midi driver */
	int result = _md->open(MidiDriver::MO_SIMPLE);
	if (result)
		error("IMuseGM::error = %s", MidiDriver::get_error_name(result));

	/* Install the on_timer thread */
	_se = eng;
	syst->create_thread(midi_driver_thread, this);

	for (i = 0, mc = _midi_channels; i != ARRAYSIZE(_midi_channels); i++, mc++)
		mc->_chan = i;
}

void IMuseGM::uninit()
{
	_md->close();
}

void IMuseGM::update_pris()
{
	Part *part, *hipart;
	int i;
	byte hipri, lopri;
	MidiChannelGM *mc, *lomc;

	while (true) {
		hipri = 0;
		hipart = NULL;
		for (i = 32, part = _se->parts_ptr(); i; i--, part++) {
			if (part->_player && !part->_percussion && part->_on && !part->_mc && part->_pri_eff >= hipri) {
				hipri = part->_pri_eff;
				hipart = part;
			}
		}

		if (!hipart)
			return;

		lopri = 255;
		lomc = NULL;
		for (i = ARRAYSIZE(_midi_channels), mc = _midi_channels;; mc++) {
			if (!mc->_part) {
				lomc = mc;
				break;
			}
			if (mc->_part->_pri_eff <= lopri) {
				lopri = mc->_part->_pri_eff;
				lomc = mc;
			}

			if (!--i) {
				if (lopri >= hipri)
					return;
				lomc->_part->off();
				break;
			}
		}

		hipart->_mc = lomc;
		lomc->_part = hipart;
		hipart->changed(pcAll);
	}
}

int IMuseGM::part_update_active(Part *part, uint16 *active)
{
	int i, j;
	uint16 *act, mask, bits;
	int count = 0;

	bits = 1 << part->_chan;

	act = part->_mc->gm()->_actives;

	for (i = 8; i; i--) {
		mask = *act++;
		if (mask) {
			for (j = 16; j; j--, mask >>= 1, active++) {
				if (mask & 1 && !(*active & bits)) {
					*active |= bits;
					count++;
				}
			}
		} else {
			active += 16;
		}
	}
	return count;
}

void IMuseGM::part_changed(Part *part, byte what)
{
	MidiChannelGM *mc;

	/* Mark for re-schedule if program changed when in pre-state */
	if (what & pcProgram && part->_percussion) {
		part->_percussion = false;
		update_pris();
	}

	if (!(mc = part->_mc->gm()))
		return;

	if (part->_player == NULL) {	/* No player, so dump phantom channel */
		part->_mc = NULL;
		mc->_part = NULL;
		memset(mc->_actives, 0, sizeof(mc->_actives));
		return;
	}

	if (what & pcMod)
		midiPitchBend(mc->_chan,
		              clamp(part->_pitchbend +
					        (part->_detune_eff * 64 / 12) +
		                    (part->_transpose_eff * 8192 / 12), -8192, 8191));

	if (what & pcVolume)
		midiVolume(mc->_chan, part->_vol_eff);

	if (what & pcPedal)
		midiPedal(mc->_chan, part->_pedal);

	if (what & pcModwheel)
		midiModWheel(mc->_chan, part->_modwheel);

	if (what & pcPan)
		midiPan(mc->_chan, part->_pan_eff);

	if (what & pcEffectLevel)
		midiEffectLevel(mc->_chan, part->_effect_level);

	if (what & pcProgram) {
		if (part->_bank) {
			midiControl0(mc->_chan, part->_bank);
			midiProgram(mc->_chan, part->_program, part->_player->_mt32emulate);
			midiControl0(mc->_chan, 0);
		} else {
			midiProgram(mc->_chan, part->_program, part->_player->_mt32emulate);
		}
	}

	if (what & pcChorus)
		midiChorus(mc->_chan, part->_effect_level);
}


void IMuseGM::part_off(Part *part)
{
	MidiChannelGM *mc = part->_mc->gm();
	if (mc) {
		part->_mc = NULL;
		mc->_part = NULL;
		memset(mc->_actives, 0, sizeof(mc->_actives));
		midiSilence(mc->_chan);
	}
}



/*
 * Implementation of the dummy IMuse class that acts as a proxy for
 * our real IMuseInternal class. This way we reduce the compile time
 * and inter source dependencies.
 */
IMuse::IMuse():_imuse(NULL)
{
}

IMuse::~IMuse()
{
	if (_imuse) {
		_imuse->terminate();
		delete _imuse;
	}
}

void IMuse::on_timer()
{
	_imuse->on_timer();
}

void IMuse::pause(bool paused)
{
	_imuse->pause(paused);
}

int IMuse::save_or_load(Serializer *ser, Scumm *scumm)
{
	return _imuse->save_or_load(ser, scumm);
}

int IMuse::set_music_volume(uint vol)
{
	return _imuse->set_music_volume(vol);
}

int IMuse::get_music_volume()
{
	return _imuse->get_music_volume();
}

int IMuse::set_master_volume(uint vol)
{
	return _imuse->set_master_volume(vol);
}

int IMuse::get_master_volume()
{
	return _imuse->get_master_volume();
}

bool IMuse::start_sound(int sound)
{
	return _imuse->start_sound(sound);
}

int IMuse::stop_sound(int sound)
{
	return _imuse->stop_sound(sound);
}

int IMuse::stop_all_sounds()
{
	return _imuse->stop_all_sounds();
}

int IMuse::get_sound_status(int sound)
{
	return _imuse->get_sound_status(sound);
}

int32 IMuse::do_command(int a, int b, int c, int d, int e, int f, int g, int h)
{
	return _imuse->do_command(a, b, c, d, e, f, g, h);
}

int IMuse::clear_queue()
{
	return _imuse->clear_queue();
}

void IMuse::setBase(byte **base)
{
	_imuse->setBase(base);
}

uint32 IMuse::property(int prop, uint32 value)
{
	return _imuse->property(prop, value);
}

IMuse *IMuse::create(OSystem *syst, MidiDriver *midi, SoundMixer *mixer)
{
	IMuse *i = new IMuse;
	i->_imuse = IMuseInternal::create(syst, midi, mixer);
	return i;
}

static void imus_digital_handler(void * engine) {
	g_scumm->_imuseDigital->handler();
}

IMuseDigital::IMuseDigital(Scumm *scumm) {
	memset(_channel, 0, sizeof(channel) * MAX_DIGITAL_CHANNELS);
	_scumm = scumm;
	_scumm->_timer->installProcedure(imus_digital_handler, 200);
}

IMuseDigital::~IMuseDigital() {
	_scumm->_timer->releaseProcedure(imus_digital_handler);
}

struct imuse_music_table {
	int16 index;
	char name[30];
	char title[30];
	char filename[15];
};

struct imuse_music_map {
	int16 room;
	int16 table_index;
	int16 unk1;
	int16 unk2;
	int16 unk3;
	int16 unk4;
};

static const imuse_music_map _digStateMusicMap[] = {
	{0,		0,	0,	0,	0,	0	},
	{1,		0,	0,	0,	0,	0	},
	{2,		2,	0,	0,	0,	0	},
	{3,		47,	0,	0,	0,	0	},
	{4,		3,	0,	0,	0,	0	},
	{5,		3,	0,	0,	0,	0	},
	{6,		3,	0,	0,	0,	0	},
	{7,		3,	0,	0,	0,	0	},
	{8,		4,	0,	0,	0,	0	},
	{9,		5,	0,	0,	0,	0	},
	{10,	4,	0,	0,	0,	0	},
	{11,	44,	0,	0,	0,	0	},
	{12,	5,	0,	0,	0,	0	},
	{13,	1,	0,	0,	0,	0	},
	{14,	5,	0,	0,	0,	0	},
	{15,	6,	29,	7,	0,	0	},
	{16,	8,	0,	0,	0,	0	},
	{17,	1,	0,	0,	0,	0	},
	{18,	9,	0,	0,	0,	0	},
	{19,	9,	0,	0,	0,	0	},
	{20,	6,	0,	0,	0,	0	},
	{21,	6,	0,	0,	0,	0	},
	{22,	44,	0,	0,	0,	0	},
	{23,	10,	7,	0,	0,	0	},
	{24,	26,	0,	0,	0,	0	},
	{25,	17,	0,	0,	0,	0	},
	{26,	17,	0,	0,	0,	0	},
	{27,	18,	0,	0,	0,	0	},
	{28,	1,	0,	0,	0,	0	},
	{29,	20,	0,	0,	0,	0	},
	{30,	22,	0,	0,	0,	0	},
	{31,	23,	0,	0,	0,	0	},
	{32,	22,	0,	0,	0,	0	},
	{33,	26,	0,	0,	0,	0	},
	{34,	24,	0,	0,	0,	0	},
	{35,	1,	0,	0,	0,	0	},
	{36,	1,	0,	0,	0,	0	},
	{37,	42,	0,	0,	0,	0	},
	{38,	43,	0,	0,	0,	0	},
	{39,	44,	0,	0,	0,	0	},
	{40,	1,	0,	0,	0,	0	},
	{41,	43,	0,	0,	0,	0	},
	{42,	44,	0,	0,	0,	0	},
	{43,	43,	0,	0,	0,	0	},
	{44,	45,	0,	0,	0,	0	},
	{45,	1,	0,	0,	0,	0	},
	{46,	33,	6,	35,	5,	34},
	{47,	1,	117,45,	114,46},
	{48,	43,	0,	0,	0,	0	},
	{49,	44,	0,	0,	0,	0	},
	{50,	1,	0,	0,	0,	0	},
	{51,	1,	0,	0,	0,	0	},
	{52,	0,	0,	0,	0,	0	},
	{53,	28,	0,	0,	0,	0	},
	{54,	28,	0,	0,	0,	0	},
	{55,	29,	0,	0,	0,	0	},
	{56,	29,	0,	0,	0,	0	},
	{57,	29,	0,	0,	0,	0	},
	{58,	31,	0,	0,	0,	0	},
	{59,	1,	0,	0,	0,	0	},
	{60,	37,	0,	0,	0,	0	},
	{61,	39,	0,	0,	0,	0	},
	{62,	38,	0,	0,	0,	0	},
	{63,	39,	0,	0,	0,	0	},
	{64,	39,	0,	0,	0,	0	},
	{65,	40,	0,	0,	0,	0	},
	{66,	1,	0,	0,	0,	0	},
	{67,	40,	0,	0,	0,	0	},
	{68,	39,	0,	0,	0,	0	},
	{69,	1,	0,	0,	0,	0	},
	{70,	49,	0,	0,	0,	0	},
	{71,	1,	0,	0,	0,	0	},
	{72,	1,	0,	0,	0,	0	},
	{73,	50,	0,	0,	0,	0	},
	{74,	1,	0,	0,	0,	0	},
	{75,	51,	0,	0,	0,	0	},
	{76,	1,	0,	0,	0,	0	},
	{77,	52,	7,	0,	0,	0	},
	{78,	63,	0,	0,	0,	0	},
	{79,	1,	0,	0,	0,	0	},
	{80,	41,	0,	0,	0,	0	},
	{81,	48,	0,	0,	0,	0	},
	{82,	21,	0,	0,	0,	0	},
	{83,	27,	0,	0,	0,	0	},
	{84,	1,	0,	0,	0,	0	},
	{85,	1,	0,	0,	0,	0	},
	{86,	0,	0,	0,	0,	0	},
	{87,	1,	0,	0,	0,	0	},
	{88,	32,	0,	0,	0,	0	},
	{89,	33,	6,	35,	5,	34},
	{90,	16,	0,	0,	0,	0	},
	{91,	57,	0,	0,	0,	0	},
	{92,	25,	0,	0,	0,	0	},
	{93,	0,	0,	0,	0,	0	},
	{94,	36,	0,	0,	0,	0	},
	{95,	19,	0,	0,	0,	0	},
	{96,	13,	0,	0,	0,	0	},
	{97,	14,	0,	0,	0,	0	},
	{98,	11,	0,	0,	0,	0	},
	{99,	15,	0,	0,	0,	0	},
	{100,	17,	0,	0,	0,	0	},
	{101,	38,	0,	0,	0,	0	},
	{102,	1,	0,	0,	0,	0	},
	{103,	0,	0,	0,	0,	0	},
	{104,	0,	0,	0,	0,	0	},
	{105,	30, 128,29,	0,	0	},
	{106,	0,	0,	0,	0,	0	},
	{107,	1,	0,	0,	0,	0	},
	{108,	1,	0,	0,	0,	0	},
	{109,	1,	0,	0,	0,	0	},
	{110,	2,	0,	0,	0,	0	},
	{111,	1,	0,	0,	0,	0	},
	{-1,	1,	0,	0,	0,	0	},
};

static const imuse_music_table _digStateMusicTable[] = {
	{0,		"STATE_NULL",						"",												""						},
	{1,		"stateNoChange",				"",												""						},
	{2,		"stateAstShip",					"Asteroid (amb-ship)",		"ASTERO~1.IMU"},
	{3,		"stateAstClose",				"Asteroid (amb-close)",		"ASTERO~2.IMU"},
	{4,		"stateAstInside",				"Asteroid (inside)",			"ASTERO~3.IMU"},
	{5,		"stateAstCore",					"Asteroid (core)",				"ASTERO~4.IMU"},
	{6,		"stateCanyonClose",			"Canyon (close)",					"CANYON~1.IMU"},
	{7,		"stateCanyonClose_m",		"Canyon (close-m)",				"CANYON~2.IMU"},
	{8,		"stateCanyonOver",			"Canyon (over)",					"CANYON~3.IMU"},
	{9,		"stateCanyonWreck",			"Canyon (wreck)",					"CANYON~4.IMU"},
	{10,	"stateNexusCanyon",			"Nexus (plan)",						"NEXUS(~1.IMU"},
	{11,	"stateNexusPlan",				"Nexus (plan)",						"NEXUS(~1.IMU"},
	{12,	"stateNexusRamp",				"Nexus (ramp)",						"NEXUS(~2.IMU"},
	{13,	"stateNexusMuseum",			"Nexus (museum)",					"NEXUS(~3.IMU"},
	{14,	"stateNexusMap",				"Nexus (map)",						"NEXUS(~4.IMU"},
	{15,	"stateNexusTomb",				"Nexus (tomb)",						"NE3706~5.IMU"},
	{16,	"stateNexusCath",				"Nexus (cath)",						"NE3305~5.IMU"},
	{17,	"stateNexusAirlock",		"Nexus (airlock)",				"NE2D3A~5.IMU"},
	{18,	"stateNexusPowerOff",		"Nexus (power)",					"NE8522~5.IMU"},
	{19,	"stateMuseumTramNear",	"Tram (mu-near)",					"TRAM(M~1.IMU"},
	{20,	"stateMuseumTramFar",		"Tram (mu-far)",					"TRAM(M~2.IMU"},
	{21,	"stateMuseumLockup",		"Museum (lockup)",				"MUSEUM~1.IMU"},
	{22,	"stateMuseumPool",			"Museum (amb-pool)",			"MUSEUM~2.IMU"},
	{23,	"stateMuseumSpire",			"Museum (amb-spire)",			"MUSEUM~3.IMU"},
	{24,	"stateMuseumMuseum",		"Museum (amb-mu)",				"MUSEUM~4.IMU"},
	{25,	"stateMuseumLibrary",		"Museum (library)",				"MUB575~5.IMU"},
	{26,	"stateMuseumCavern",		"Museum (cavern)",				"MUF9BE~5.IMU"},
	{27,	"stateTombTramNear",		"Tram (tomb-near)",				"TRAM(T~1.IMU"},
	{28,	"stateTombBase",				"Tomb (amb-base)",				"TOMB(A~1.IMU"},
	{29,	"stateTombSpire",				"Tomb (amb-spire)",				"TOMB(A~2.IMU"},
	{30,	"stateTombCave",				"Tomb (amb-cave)",				"TOMB(A~3.IMU"},
	{31,	"stateTombCrypt",				"Tomb (crypt)",						"TOMB(C~1.IMU"},
	{32,	"stateTombGuards",			"Tomb (crypt-guards)",		"TOMB(C~2.IMU"},
	{33,	"stateTombInner",				"Tomb (inner)",						"TOMB(I~1.IMU"},
	{34,	"stateTombCreator1",		"Tomb (creator 1)",				"TOMB(C~3.IMU"},
	{35,	"stateTombCreator2",		"Tomb (creator 2)",				"TOMB(C~4.IMU"},
	{36,	"statePlanTramNear",		"Tram (plan-near)",				"TRAM(P~1.IMU"},
	{37,	"statePlanTramFar",			"Tram (plan-far)",				"TRAM(P~2.IMU"},
	{38,	"statePlanBase",				"Plan (amb-base)",				"PLAN(A~1.IMU"},
	{39,	"statePlanSpire",				"Plan (amb-spire)",				"PLAN(A~2.IMU"},
	{40,	"statePlanDome",				"Plan (dome)",						"PLAN(D~1.IMU"},
	{41,	"stateMapTramNear",			"Tram (map-near)",				"TRAM(M~3.IMU"},
	{42,	"stateMapTramFar",			"Tram (map-far)",					"TRAM(M~4.IMU"},
	{43,	"stateMapCanyon",				"Map (amb-canyon)",				"MAP(AM~1.IMU"},
	{44,	"stateMapExposed",			"Map (amb-exposed)",			"MAP(AM~2.IMU"},
	{45,	"stateMapNestEmpty",		"Map (amb-nest)",					"MAP(AM~4.IMU"},
	{46,	"stateMapNestMonster",	"Map (monster)",					"MAP(MO~1.IMU"},
	{47,	"stateMapKlein",				"Map (klein)",						"MAP(KL~1.IMU"},
	{48,	"stateCathTramNear",		"Tram (cath-near)",				"TRAM(C~1.IMU"},
	{49,	"stateCathTramFar",			"Tram (cath-far)",				"TRAM(C~2.IMU"},
	{50,	"stateCathLab",					"Cath (amb-inside)",			"CATH(A~1.IMU"},
	{51,	"stateCathOutside",			"Cath (amb-outside)",			"CATH(A~2.IMU"},
	{52,	"stateWorldMuseum",			"World (museum)",					"WORLD(~1.IMU"},
	{53,	"stateWorldPlan",				"World (plan)",						"WORLD(~2.IMU"},
	{54,	"stateWorldTomb",				"World (tomb)",						"WORLD(~3.IMU"},
	{55,	"stateWorldMap",				"World (map)",						"WORLD(~4.IMU"},
	{56,	"stateWorldCath",				"World (cath)",						"WO3227~5.IMU"},
	{57,	"stateEye1",						"Eye 1",									"EYE1~1.IMU"	},
	{58,	"stateEye2",						"Eye 2",									"EYE2~1.IMU"	},
	{59,	"stateEye3",						"Eye 3",									"EYE3~1.IMU"	},
	{60,	"stateEye4",						"Eye 4",									"EYE4~1.IMU"	},
	{61,	"stateEye5",						"Eye 5",									"EYE5~1.IMU"	},
	{62,	"stateEye6",						"Eye 6",									"EYE6~1.IMU"	},
	{63,	"stateEye7",						"Eye 7",									"EYE7~1.IMU"	},
	{-1,	"",											"",												""						},
};

static const imuse_music_table _digSeqMusicTable[] = {
	{0,		"SEQ_NULL",							"",												""						},
	{1,		"seqLogo",							"",												""						},
	{2,		"seqIntro",							"",												""						},
	{3,		"seqExplosion1b",				"",												""						},
	{4,		"seqAstTunnel1a",				"Seq (ast tunnel 1a)",		"SEQ(AS~1.IMU"},
	{5,		"seqAstTunnel2b",				"",												""						},
	{6,		"seqAstTunnel3a",				"Seq (ast tunnel 3a)",		"SEQ(AS~2.IMU"},
	{7,		"seqToPlanet1b",				"",												""						},
	{8,		"seqArgBegin",					"Seq (arg begin)",				"SEQ(AR~1.IMU"},
	{9,		"seqArgEnd",						"Seq (arg end)",					"SEQ(AR~2.IMU"},
	{10,	"seqWreckGhost",				"Seq (ghost-wreck)",			"SEQ(GH~1.IMU"},
	{11,	"seqCanyonGhost",				"Seq (ghost-canyon)",			"SEQ(GH~2.IMU"},
	{12,	"seqBrinkFall",					"",												""						},
	{13,	"seqPanUpCanyon",				"Seq (pan up canyon)",		"SEQ(PA~1.IMU"},
	{14,	"seqAirlockTunnel1b",		"",												""						},
	{15,	"seqTramToMu",					"",												""						},
	{16,	"seqTramFromMu",				"",												""						},
	{17,	"seqTramToTomb",				"",												""						},
	{18,	"seqTramFromTomb",			"",												""						},
	{19,	"seqTramToPlan",				"",												""						},
	{20,	"seqTramFromPlan",			"",												""						},
	{21,	"seqTramToMap",					"",												""						},
	{22,	"seqTramFromMap",				"",												""						},
	{23,	"seqTramToCath",				"",												""						},
	{24,	"seqTramFromCath",			"",												""						},
	{25,	"seqMuseumGhost",				"",												""						},
	{26,	"seqSerpentAppears",		"",												""						},
	{27,	"seqSerpentEats",				"",												""						},
	{28,	"seqBrinkRes1b",				"",												""						},
	{29,	"seqBrinkRes2a",				"Seq (brink's madness)",	"SEQ(BR~1.IMU"},
	{30,	"seqLockupEntry",				"Seq (brink's madness)",	"SEQ(BR~1.IMU"},
	{31,	"seqSerpentExplodes",		"",												""						},
	{32,	"seqSwimUnderwater",		"Seq (descent)",					"SEQ(DE~1.IMU"},
	{33,	"seqWavesPlunge",				"Seq (plunge)",						"SEQ(PL~1.IMU"},
	{34,	"seqCryptOpens",				"",												""						},
	{35,	"seqGuardsFight",				"",												""						},
	{36,	"seqCreatorRes1.1a",		"Seq (creator res 1.1a)",	"SEQ(CR~1.IMU"},
	{37,	"seqCreatorRes1.2b",		"",												""						},
	{38,	"seqMaggieCapture1b",		"",												""						},
	{39,	"seqStealCrystals",			"Seq (brink's madness)",	"SEQ(BR~1.IMU"},
	{40,	"seqGetByMonster",			"",												""						},
	{41,	"seqKillMonster1b",			"",												""						},
	{42,	"seqCreatorRes2.1a",		"Seq (creator res 2.1a)",	"SEQ(CR~2.IMU"},
	{43,	"seqCreatorRes2.2b",		"",												""						},
	{44,	"seqCreatorRes2.3a",		"Seq (creator res 2.3a)",	"SEQ(CR~3.IMU"},
	{45,	"seqMaggieInsists",			"",												""						},
	{46,	"seqBrinkHelpCall",			"",												""						},
	{47,	"seqBrinkCrevice1a",		"Seq (brink crevice 1a)",	"SEQ(BR~2.IMU"},
	{48,	"seqBrinkCrevice2a",		"Seq (brink crevice 2a)",	"SEQ(BR~3.IMU"},
	{49,	"seqCathAccess1b",			"",												""						},
	{50,	"seqCathAccess2a",			"Seq (cath access 2a)",		"SEQ(CA~1.IMU"},
	{51,	"seqBrinkAtGenerator",	"Seq (brink's madness)",	"SEQ(BR~1.IMU"},
	{52,	"seqFightBrink1b",			"",												""						},
	{53,	"seqMaggieDies1b",			"",												""						},
	{54,	"seqMaggieRes1b",				"",												""						},
	{55,	"seqMaggieRes2a",				"Seq (maggie res 2a)",		"SEQ(MA~1.IMU"},
	{56,	"seqCreatureFalls",			"",												""						},
	{57,	"seqFinale1b",					"",												""						},
	{58,	"seqFinale2a",					"Seq (finale 2a)",				"SEQ(FI~1.IMU"},
	{59,	"seqFinale3b1",					"",												""						},
	{60,	"seqFinale3b2",					"",												""						},
	{61,	"seqFinale4a",					"Seq (finale 4a)",				"SEQ(FI~2.IMU"},
	{62,	"seqFinale5a",					"Seq (finale 5a)",				"SEQ(FI~3.IMU"},
	{63,	"seqFinale6a",					"Seq (finale 6a)",				"SEQ(FI~4.IMU"},
	{64,	"seqFinale7a",					"Seq (finale 7a)",				"SE3D2B~5.IMU"},
	{65,	"seqFinale8b",					"",												""						},
	{66,	"seqFinale9a",					"Seq (finale 9a)",				"SE313B~5.IMU"},
	{-1,	"",											"",												""						},
};

void IMuseDigital::handler() {
	bool new_mixer;
	uint32 l = 0, i = 0;

	for (l = 0; l < MAX_DIGITAL_CHANNELS;l ++) {
		if (_channel[l]._used) {
			if (_channel[l]._toBeRemoved == true) {
				_channel[l]._used = false;
				free(_channel[l]._data);
				memset(&_channel[l], 0, sizeof(channel));
				continue;
			}

			if (_channel[l]._delay > 0) {
				_channel[l]._delay--;
				continue;
			}

			if (_channel[l]._volumeFade != -1) {
				if (_channel[l]._volumeFadeStep < 0) {
					if (_channel[l]._volume > _channel[l]._volumeFade) {
						_channel[l]._volume += _channel[l]._volumeFadeStep;
						_channel[l]._volumeRight += _channel[l]._volumeFadeStep;
						if (_channel[l]._volume < _channel[l]._volumeFade) {
							_channel[l]._volume = _channel[l]._volumeFade;
						}
						if (_channel[l]._volumeRight < _channel[l]._volumeFade) {
							_channel[l]._volumeRight = _channel[l]._volumeFade;
						}
						if ((_channel[l]._volume == 0) && (_channel[l]._volumeRight == 0)) {
							_channel[l]._toBeRemoved = true;
						}
					}
				} else if (_channel[l]._volumeFadeStep > 0) {
					if (_channel[l]._volume < _channel[l]._volumeFade) {
						_channel[l]._volume += _channel[l]._volumeFadeStep;
						_channel[l]._volumeRight += _channel[l]._volumeFadeStep;
						if (_channel[l]._volume > _channel[l]._volumeFade) {
							_channel[l]._volume = _channel[l]._volumeFade;
						}
						if (_channel[l]._volumeRight > _channel[l]._volumeFade) {
							_channel[l]._volumeRight = _channel[l]._volumeFade;
						}
					}
				}
			}

			if ((_channel[l]._jump[0]._numLoops == 0) && (_channel[l]._isJump == true)) {
				_channel[l]._isJump = false;
			}

			uint32 new_size = _channel[l]._mixerSize;
			uint32 mixer_size = new_size;

			if (_channel[l]._mixerTrack == -1) {
				new_mixer = true;
				mixer_size *= 2;
				new_size *= 2;
			} else {
				new_mixer = false;
			}

			if (_channel[l]._isJump == false) {
				if (_channel[l]._offset + mixer_size > _channel[l]._size) {
					new_size = _channel[l]._size - _channel[l]._offset;
					if (_channel[l]._numLoops == 0) {
						_channel[l]._toBeRemoved = true;
						mixer_size = new_size;
					}
				}
			} else if (_channel[l]._isJump == true) {
				if (_channel[l]._jump[0]._numLoops != 500) {
					_channel[l]._jump[0]._numLoops--;
				}
				if (_channel[l]._offset + mixer_size >= _channel[l]._jump[0]._offset) {
					new_size = _channel[l]._jump[0]._offset - _channel[l]._offset;
				}
			}

			byte *buf = (byte*)malloc(mixer_size);
			
			memcpy(buf, _channel[l]._data + _channel[l]._offset, new_size);
			if ((new_size != mixer_size) && (_channel[l]._isJump == true)) {
				memcpy(buf + new_size, _channel[l]._data + _channel[l]._jump[0]._dest, mixer_size - new_size);
				_channel[l]._offset = _channel[l]._jump[0]._dest + (mixer_size - new_size);
			} else if ((_channel[l]._numLoops > 0) && (new_size != mixer_size)) {
				memcpy(buf + new_size, _channel[l]._data, mixer_size - new_size);
				_channel[l]._offset = mixer_size - new_size;
			} else {
				_channel[l]._offset += mixer_size;
			}

			if (_channel[l]._bits == 12) {
				for(i = 0; i < (mixer_size / 4); i++) {
					byte sample1 = buf[i * 4 + 0];
					byte sample2 = buf[i * 4 + 1];
					byte sample3 = buf[i * 4 + 2];
					byte sample4 = buf[i * 4 + 3];
					uint16 sample_a = (uint16)(((int16)((sample1 << 8) | sample2) * _channel[l]._volumeRight) >> 8);
					uint16 sample_b = (uint16)(((int16)((sample3 << 8) | sample4) * _channel[l]._volume) >> 8);
					buf[i * 4 + 0] = (byte)(sample_a >> 8);
					buf[i * 4 + 1] = (byte)(sample_a & 0xff);
					buf[i * 4 + 2] = (byte)(sample_b >> 8);
					buf[i * 4 + 3] = (byte)(sample_b & 0xff);
				}
			} else if (_channel[l]._bits == 8) {
				for(i = 0; i < (mixer_size / 2); i++) {
					buf[i * 2 + 0] = (byte)(((int8)(buf[i * 2 + 0] ^ 0x80) * _channel[l]._volumeRight) >> 8) ^ 0x80;
					buf[i * 2 + 1] = (byte)(((int8)(buf[i * 2 + 1] ^ 0x80) * _channel[l]._volume) >> 8) ^ 0x80;
				}
			}

			if (new_mixer) {
				_channel[l]._mixerTrack = _scumm->_mixer->playStream(NULL, -1, buf, mixer_size,
																				 _channel[l]._freq, _channel[l]._mixerFlags);
			} else {
				_scumm->_mixer->append(_channel[l]._mixerTrack, buf, mixer_size,
															 _channel[l]._freq, _channel[l]._mixerFlags);
			}
		}
	}
}

void IMuseDigital::startSound(int sound) {
	debug(2, "IMuseDigital::startSound(%d)", sound);
	int32 l;

	for(l = 0; l < MAX_DIGITAL_CHANNELS; l++) {
		if(_channel[l]._used == false) {
			byte *ptr = _scumm->getResourceAddress(rtSound, sound);
			byte *s_ptr = ptr;
			if(ptr == NULL) {
				warning("IMuseDigital::startSound(%d) NULL resource pointer", sound);
				return;
			}
			_channel[l]._idSound = sound;
			_channel[l]._offset = 0;
			_channel[l]._numRegions = 0;
			_channel[l]._numJumps = 0;
			_channel[l]._volumeRight = 127;
			_channel[l]._volume = 127;
			_channel[l]._volumeFade = -1;
			_channel[l]._volumeFadeParam = 0;
			_channel[l]._delay = 1;

			uint32 tag, size = 0, r, t;

			if (READ_UINT32_UNALIGNED(ptr) == MKID('Crea')) {
				_channel[l]._bits = 8;
				_channel[l]._channels = 2;
				_channel[l]._mixerTrack = -1;
				_channel[l]._mixerSize = (22050 / 5) * 2;
				_channel[l]._mixerFlags = SoundMixer::FLAG_AUTOFREE | SoundMixer::FLAG_STEREO | SoundMixer::FLAG_REVERSE_STEREO | SoundMixer::FLAG_UNSIGNED;
				byte * t_ptr= _scumm->_sound->readCreativeVocFile(ptr, size, _channel[l]._freq, _channel[l]._numLoops);

				if (_channel[l]._freq == 22222) {
					_channel[l]._freq = 22050;
				} else if (_channel[l]._freq == 10989) {
					_channel[l]._freq = 11025;
				}
				
				if (_channel[l]._freq == 11025) {
					_channel[l]._mixerSize /= 2;
				}
				size *= 2;
				_channel[l]._data = (byte *)malloc(size);
				for (t = 0; t < size / 2; t++) {
					*(_channel[l]._data + t * 2 + 0) = *(t_ptr + t);
					*(_channel[l]._data + t * 2 + 1) = *(t_ptr + t);
				}
				free(t_ptr);
				_channel[l]._size = size;
			} else if (READ_UINT32_UNALIGNED(ptr) == MKID('iMUS')) {
				ptr += 16;
				for (;;) {
					tag = READ_BE_UINT32(ptr); ptr += 4;
					switch(tag) {
						case MKID_BE('FRMT'):
							ptr += 12;
							_channel[l]._bits = READ_BE_UINT32(ptr); ptr += 4;
							_channel[l]._freq = READ_BE_UINT32(ptr); ptr += 4;
							_channel[l]._channels = READ_BE_UINT32(ptr); ptr += 4;
						break;
						case MKID_BE('TEXT'):
							size = READ_BE_UINT32(ptr); ptr += size + 4;
						break;
						case MKID_BE('REGN'):
							ptr += 4;
							if (_channel[l]._numRegions >= MAX_IMUSE_REGIONS) {
								warning("IMuseDigital::startSound(%d) Not enough space for Region");
								ptr += 8;
								break;
							}
							_channel[l]._region[_channel[l]._numRegions]._offset = READ_BE_UINT32(ptr); ptr += 4;
							_channel[l]._region[_channel[l]._numRegions]._length = READ_BE_UINT32(ptr); ptr += 4;
							_channel[l]._numRegions++;
						break;
						case MKID_BE('STOP'):
							ptr += 4;
							_channel[l]._offsetStop = READ_BE_UINT32(ptr); ptr += 4;
						break;
						case MKID_BE('JUMP'):
							ptr += 4;
							if (_channel[l]._numJumps >= MAX_IMUSE_JUMPS) {
								warning("IMuseDigital::startSound(%d) Not enough space for Jump");
								ptr += 16;
								break;
							}
							_channel[l]._jump[_channel[l]._numJumps]._offset = READ_BE_UINT32(ptr); ptr += 4;
							_channel[l]._jump[_channel[l]._numJumps]._dest = READ_BE_UINT32(ptr); ptr += 4;
							_channel[l]._jump[_channel[l]._numJumps]._id = READ_BE_UINT32(ptr); ptr += 4;
							_channel[l]._jump[_channel[l]._numJumps]._numLoops = READ_BE_UINT32(ptr); ptr += 4;
							_channel[l]._isJump = true;
							_channel[l]._numJumps++;
						break;
						case MKID_BE('DATA'):
							size = READ_BE_UINT32(ptr); ptr += 4;
						break;
						default:
							error("IMuseDigital::startSound(%d) Unknown sfx header %c%c%c%c", tag>>24, tag>>16, tag>>8, tag);
					}
					if (tag == MKID_BE('DATA')) break;
				}

				uint32 header_size = ptr - s_ptr;
				_channel[l]._offsetStop -= header_size;
				if (_channel[l]._bits == 12) {
					_channel[l]._offsetStop = (_channel[l]._offsetStop / 3) * 4;
				}
				for (r = 0; r < _channel[l]._numRegions; r++) {
					_channel[l]._region[r]._offset -= header_size;
					if (_channel[l]._bits == 12) {
						_channel[l]._region[r]._offset = (_channel[l]._region[r]._offset / 3) * 4;
						_channel[l]._region[r]._length = (_channel[l]._region[r]._length / 3) * 4;
					}
				}
				if (_channel[l]._numJumps > 0) {
					for (r = 0; r < _channel[l]._numJumps; r++) {
						_channel[l]._jump[r]._offset -= header_size;
						_channel[l]._jump[r]._dest -= header_size;
						if (_channel[l]._bits == 12) {
							_channel[l]._jump[r]._offset = (_channel[l]._jump[r]._offset / 3) * 4;
							_channel[l]._jump[r]._dest = (_channel[l]._jump[r]._dest / 3) * 4;
						}
					}
				}
				_channel[l]._mixerTrack = -1;
				_channel[l]._mixerSize = (22050 / 5) * 2;
				_channel[l]._mixerFlags = SoundMixer::FLAG_AUTOFREE | SoundMixer::FLAG_STEREO | SoundMixer::FLAG_REVERSE_STEREO;
				if (_channel[l]._bits == 12) {
					_channel[l]._mixerSize *= 2;
					_channel[l]._mixerFlags |= SoundMixer::FLAG_16BITS;
					_channel[l]._size = _scumm->_sound->decode12BitsSample(ptr, &_channel[l]._data, size, (_channel[l]._channels == 2) ? false : true);
				}
				if (_channel[l]._bits == 8) {
					_channel[l]._mixerFlags |= SoundMixer::FLAG_UNSIGNED;
					if (_channel[l]._channels == 1) {
						size *= 2;
						_channel[l]._channels = 2;
						_channel[l]._data = (byte *)malloc(size);
						for (t = 0; t < size / 2; t++) {
							*(_channel[l]._data + t * 2 + 0) = *(ptr + t);
							*(_channel[l]._data + t * 2 + 1) = *(ptr + t);
						}
					} else {
						_channel[l]._data = (byte *)malloc(size);
						memcpy(_channel[l]._data, ptr, size);
					}
					_channel[l]._size = size;
				}
				if (_channel[l]._freq == 11025) {
					_channel[l]._mixerSize /= 2;
				}
			}
			_channel[l]._toBeRemoved = false;
			_channel[l]._used = true;
			break;
		}
	}
}

void IMuseDigital::stopSound(int sound) {
	debug(2, "IMuseDigital::stopSound(%d)", sound);
	for (int32 l = 0; l < MAX_DIGITAL_CHANNELS; l++) {
		if ((_channel[l]._idSound == sound) && (_channel[l]._used == true)) {
			_channel[l]._toBeRemoved = true;
		}
	}
}

void IMuseDigital::stopAll() {
	for (int32 l = 0; l < MAX_DIGITAL_CHANNELS; l++) {
		if (_channel[l]._used == true) {
			_channel[l]._toBeRemoved = true;
		}
	}
}

int32 IMuseDigital::doCommand(int a, int b, int c, int d, int e, int f, int g, int h) {
	byte cmd = a & 0xFF;
	byte param = a >> 8;
	int32 sample = b;
	byte sub_cmd = c >> 8;
	int8 channel = -1, l;
	int8 tmp;

	if (!(cmd || param))
		return 1;

	if (param == 0) {
		switch (cmd) {
		case 12:
			switch (sub_cmd) {
			case 5:
				debug(2, "IMuseDigital::doCommand 12,5 sample(%d), param(%d)", sample, d);
				return 0;
			case 6: // volume control (0-127)
				debug(2, "IMuseDigital::doCommand setting volume sample(%d), volume(%d)", sample, d);
				for (l = 0; l < MAX_DIGITAL_CHANNELS; l++) {
					if ((_channel[l]._idSound == sample) && (_channel[l]._used == true)) {
						channel = l;
						break;
					}
				}
				if (channel == -1) {
					warning("IMuseDigital::doCommand 12,6 sample(%d) not exist in channels", sample);
					return 1;
				}
				_channel[channel]._volume = d;
				_channel[channel]._volumeRight = d;
				if (_channel[channel]._volumeFade != -1) {
					tmp = ((_channel[channel]._volumeFade - _channel[channel]._volume) * 2) / _channel[channel]._volumeFadeParam;
					if ((tmp < 0) && (tmp > -2)) {
						tmp = -1;
					} else if ((tmp > 0) && (tmp < 2)) {
						tmp = 1;
					} else {
						tmp /= 2;
					}
				_channel[channel]._volumeFadeStep = tmp;
				}
				return 0;
			case 7: // right volume control (0-127)
				debug(2, "IMuseDigital::doCommand setting right volume sample(%d),volume(%d)", sample, d);
				for (l = 0; l < MAX_DIGITAL_CHANNELS; l++) {
					if ((_channel[l]._idSound == sample) && (_channel[l]._used == true)) {
						channel = l;
						break;
					}
				}
				if (channel == -1) {
					warning("IMuseDigital::doCommand 12,7 sample(%d) not exist in channels", sample);
					return 1;
				}
				_channel[channel]._volumeRight = d;
				return 0;
			default:
				warning("IMuseDigital::doCommand 12 DEFAULT sub command %d", sub_cmd);
				return 1;
			}
		case 14:
			switch (sub_cmd) {
			case 6: // fade volume control
				debug(2, "IMuseDigital::doCommand fading volume sample(%d),fade(%d, %d)", sample, d, e);
				for (l = 0; l < MAX_DIGITAL_CHANNELS; l++) {
					if ((_channel[l]._idSound == sample) && (_channel[l]._used == true)) {
						channel = l;
						break;
					}
				}
				if (channel == -1) {
						warning("IMuseDigital::doCommand 14,6 sample %d not exist in channels", sample);
					return 1;
				}
				_channel[channel]._volumeFade = d;
				_channel[channel]._volumeFadeParam = e;
				tmp = ((_channel[channel]._volumeFade - _channel[channel]._volume) * 2) / _channel[channel]._volumeFadeParam;
				if ((tmp < 0) && (tmp > -2)) {
					tmp = -1;
				} else if ((tmp > 0) && (tmp < 2)) {
					tmp = 1;
				} else {
					tmp /= 2;
				}
				_channel[channel]._volumeFadeStep = tmp;
				return 0;
			default:
				warning("IMuseDigital::doCommand 14 DEFAULT sub command %d", sub_cmd);
				return 1;
			}
		default:
			warning("IMuseDigital::doCommand DEFAULT command %d", cmd);
			return 1;
		}
	} else if (param == 16) {
		switch (cmd) {
		case 0: // play music (state)
			debug(1, "IMuseDigital::doCommand 0x1000 (%d)", b);
			if (_scumm->_gameId == GID_FT)
				return 0;
			for(l = 0;; l++) {
				if (_digStateMusicMap[l].room == -1) {
					return 1;
				}
				if ((_digStateMusicMap[l].room == b)) {
					int16 music = _digStateMusicMap[l].table_index;
					debug(1, "Play imuse music: %s, %s, %s", _digStateMusicTable[music].name, _digStateMusicTable[music].title, _digStateMusicTable[music].filename);
					if (_digStateMusicTable[music].filename[0] != 0) {
						_scumm->_sound->playBundleMusic((char*)&_digStateMusicTable[music].filename);
					}
					return 0;
				}
			}
			return 0;
		case 1: // play music (seq)
			debug(1, "IMuseDigital::doCommand 0x1001 (%d)", b);
			return 0;
		case 2: // dummy in DIG and CMI
			debug(2, "IMuseDigital::doCommand 0x1002 (%d)", b);
			return 0;
		case 3: // ??? (stream related)
			debug(2, "IMuseDigital::doCommand 0x1003 (%d,%d)", b, c);
			return 0;
		default:
			warning("IMuseDigital::doCommand (0x1xxx) DEFAULT command %d", cmd);
			return 1;
		}
	}

	return 1;
}

int IMuseDigital::getSoundStatus(int sound) {
	debug(2, "IMuseDigital::getSoundStatus(%d)", sound);
	for (int32 l = 0; l < MAX_DIGITAL_CHANNELS; l++) {
		if ((_channel[l]._idSound == sound) && (_channel[l]._used == true)) {
			return 1;
		}
	}

	return 0;
}
