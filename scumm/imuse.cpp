/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
#include "sound/mididrv.h"
#include "scumm/imuse.h"
#include "scumm/instrument.h"
#include "scumm/saveload.h"
#include "common/util.h"

// Unremark this statement to activate some of
// the most common iMuse diagnostic messages.
// #define IMUSE_DEBUG

//
// Some constants
//
#define TICKS_PER_BEAT 480

#define IMUSE_SYSEX_ID 0x7D
#define ROLAND_SYSEX_ID 0x41
#define PERCUSSION_CHANNEL 9

#define TRIGGER_ID 0
#define COMMAND_ID 1

#define MDPG_TAG "MDpg"
#define MDHD_TAG "MDhd"

// Put IMUSE specific classes here, instead of in a .h file
// they will only be used from this file, so it will reduce
// compile time.

struct Part;

struct HookDatas {
	byte _jump[2];
	byte _transpose;
	byte _part_onoff[16];
	byte _part_volume[16];
	byte _part_program[16];
	byte _part_transpose[16];

	int query_param(int param, byte chan);
	int set(byte cls, byte value, byte chan);
};


struct Player {
	IMuseInternal *_se;
	MidiDriver *_midi;

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
	uint32 _tempo_eff; // No Save
	uint32 _cur_pos;
	uint32 _next_pos;
	uint32 _song_offset;
	uint32 _timer_speed; // No Save
	uint _tick_index;
	uint _beat_index;
	uint _ticks_per_beat;
	byte _speed; // No Save
	bool _abort;

	HookDatas _hook;

	bool _mt32emulate;
	bool _isGM;

	// Player part
	void hook_clear();
	void clear();
	bool startSound (int sound, MidiDriver *midi);
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

	// Sequencer part
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

	Player() {
		memset(this,0,sizeof(Player));	// palmos
	}

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
	
	VolumeFader() {
		memset(this,0,sizeof(VolumeFader));	//palmos
	}
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

struct Part {
	int _slot;
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
	int8 _pri;
	byte _pri_eff;
	byte _chan;
	byte _effect_level;
	byte _chorus;
	byte _percussion;
	byte _bank;

	// New abstract instrument definition
	Instrument _instrument;
	bool _unassigned_instrument; // For diagnostic reporting purposes only

	// Used to be in MidiDriver
	uint16 _actives[8];

	void key_on(byte note, byte velocity);
	void key_off(byte note);
	void set_param(byte param, int value) { }
	void init();
	void setup(Player *player);
	void uninit();
	void off();
	void silence();
	void set_instrument(uint b);
	void set_instrument(byte *data);
	void load_global_instrument (byte b);

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

	void sendAll();
	bool clearToTransmit();
	
	Part() {
		memset(this,0,sizeof(Part));
	}
};

struct ImTrigger {
	int sound;
	byte id;
	uint16 expire;
	byte command [4];
};

// WARNING: This is the internal variant of the IMUSE class.
// imuse.h contains a public version of the same class.
// the public version, only contains a set of methods.
class IMuseInternal {
	friend struct Player;

private:
	bool _enable_multi_midi;
	MidiDriver *_midi_adlib;
	MidiDriver *_midi_native;

	byte **_base_sounds;

private:
	bool _paused;
	bool _active_volume_faders;
	bool _initialized;
	byte _volume_fader_counter;

	int _tempoFactor;

	uint _queue_end, _queue_pos, _queue_sound;
	byte _queue_adding;

	SustainingNotes *_sustain_notes_used;
	SustainingNotes *_sustain_notes_free;
	SustainingNotes *_sustain_notes_head;

	byte _queue_marker;
	byte _queue_cleared;
	byte _master_volume; // Master volume. 0-255
	byte _music_volume; // Global music volume. 0-255

	uint16 _trigger_count;
	ImTrigger _snm_triggers[16]; // Sam & Max triggers
	uint16 _snm_trigger_index;

	uint16 _channel_volume[8];
	uint16 _channel_volume_eff[8]; // No Save
	uint16 _volchan_table[8];

	Player _players[8];
	SustainingNotes _sustaining_notes[24];
	VolumeFader _volume_fader[8];
	Part _parts[32];

	uint16 _active_notes[128];
	Instrument _global_adlib_instruments[32];
	CommandQueue _cmd_queue[64];

	byte *findTag(int sound, char *tag, int index);
	bool isMT32(int sound);
	bool isGM(int sound);
	int get_queue_sound_status(int sound);
	void handle_marker(uint id, byte data);
	int get_channel_volume(uint a);
	void initMidiDriver (MidiDriver *midi);
	void init_players();
	void init_parts();
	void init_volume_fader();
	void init_sustaining_notes();
	void init_queue();

	void sequencer_timers (MidiDriver *midi);
	void expire_sustain_notes (MidiDriver *midi);
	void expire_volume_faders (MidiDriver *midi);

	MidiDriver *getBestMidiDriver (int sound);
	Player *allocate_player(byte priority);
	Part *allocate_part(byte pri, MidiDriver *midi);

	int32 ImSetTrigger (int sound, int id, int a, int b, int c, int d);
	int32 ImClearTrigger (int sound, int id);
	int32 ImFireAllTriggers (int sound);

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

	static int saveReference(void *me_ref, byte type, void *ref);
	static void *loadReference(void *me_ref, byte type, int ref);

	static void midiTimerCallback (void *data);

public:
	IMuseInternal() {
		memset(this,0,sizeof(IMuseInternal));	// palmos
	}
	~IMuseInternal();

	int initialize(OSystem *syst, MidiDriver *midi);
	void reallocateMidiChannels (MidiDriver *midi);
	void setGlobalAdlibInstrument (byte slot, byte *data);
	void copyGlobalAdlibInstrument (byte slot, Instrument *dest);

	// IMuse interface

	void on_timer (MidiDriver *midi);
	void pause(bool paused);
	int terminate();
	int save_or_load(Serializer *ser, Scumm *scumm);
	int set_music_volume(uint vol);
	int get_music_volume();
	int set_master_volume(uint vol);
	int get_master_volume();
	bool startSound(int sound);
	int stopSound(int sound);
	int stop_all_sounds();
	int getSoundStatus(int sound);
	bool get_sound_active(int sound);
	int32 doCommand(int a, int b, int c, int d, int e, int f, int g, int h);
	int clear_queue();
	void setBase(byte **base);

	uint32 property(int prop, uint32 value);

	static IMuseInternal *create(OSystem *syst, MidiDriver *midi);
};

////////////////////////////////////////
//
//  IMUSE helper functions
//
////////////////////////////////////////

static int clamp(int val, int min, int max) {
	if (val < min)
		return min;
	if (val > max)
		return max;
	return val;
}

static int transpose_clamp(int a, int b, int c) {
	if (b > a)
		a += (b - a + 11) / 12 * 12;
	if (c < a)
		a -= (a - c + 11) / 12 * 12;
	return a;
}

static uint32 get_delta_time(byte **s) {
	byte *d = *s, b;
	uint32 time = 0;
	do {
		b = *d++;
		time = (time << 7) | (b & 0x7F);
	} while (b & 0x80);
	*s = d;
	return time;
}

static uint read_word(byte *a) {
	return (a[0] << 8) + a[1];
}

static void skip_midi_cmd(byte **song_ptr) {
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

static int is_note_cmd(byte **a, IsNoteCmdData * isnote) {
	byte *s = *a;
	byte code;

	code = *s++;

	switch (code >> 4) {
	case 8: // Key Off
		isnote->chan = code & 0xF;
		isnote->note = *s++;
		isnote->vel = *s++;
		*a = s;
		return 1;
	case 9: // Key On
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

////////////////////////////////////////
//
//  IMuseInternal implementation
//
////////////////////////////////////////

IMuseInternal::~IMuseInternal() {
	terminate();
}

byte *IMuseInternal::findTag(int sound, char *tag, int index) {
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

bool IMuseInternal::isMT32(int sound) {
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

bool IMuseInternal::isGM(int sound) {
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

MidiDriver *IMuseInternal::getBestMidiDriver (int sound) {
	MidiDriver *driver = NULL;

	if (isGM (sound)) {
		if (_midi_native) {
			driver = _midi_native;
#if !defined(__PALM_OS__) // Adlib not supported on PalmOS
		} else {
			// Route it through Adlib anyway.
			if (!_midi_adlib) {
				_midi_adlib = MidiDriver_ADLIB_create();
				initMidiDriver (_midi_adlib);
			}
			driver = _midi_adlib;
#endif
		}
#if !defined(__PALM_OS__) // Adlib not supported on PalmOS
	} else {
		if (!_midi_adlib && (_enable_multi_midi || !_midi_native)) {
			_midi_adlib = MidiDriver_ADLIB_create();
			initMidiDriver (_midi_adlib);
		}
		driver = _midi_adlib;
#endif
	}
	return driver;
}

bool IMuseInternal::startSound(int sound) {
	Player *player;
	void *mdhd;

	// Do not start a sound if it is already set to
	// start on an ImTrigger event. This fixes carnival
	// music problems where a sound has been set to trigger
	// at the right time, but then is started up immediately
	// anyway, only to be restarted later when the trigger
	// occurs.
	int i;
	ImTrigger *trigger = _snm_triggers;
	for (i = ARRAYSIZE (_snm_triggers); i; --i, ++trigger) {
		if (trigger->sound && trigger->id && trigger->command[0] == 8 && trigger->command[1] == sound)
			return false;
	}

	mdhd = findTag(sound, MDHD_TAG, 0);
	if (!mdhd) {
		mdhd = findTag(sound, MDPG_TAG, 0);
		if (!mdhd) {
			debug (2, "SE::startSound failed: Couldn't find sound %d", sound);
			return false;
		}
	}

	// Check which MIDI driver this track should use.
	// If it's NULL, it ain't something we can play.
	MidiDriver *driver = getBestMidiDriver (sound);
	if (!driver)
		return false;

	// If the requested sound is already playing, start it over
	// from scratch. This was originally a hack to prevent Sam & Max
	// iMuse messiness while upgrading the iMuse engine, but it
	// is apparently necessary to deal with fade-and-restart
	// race conditions that were observed in MI2. Reference
	// Bug #590511 and Patch #607175 (which was reversed to fix
	// an FOA regression: Bug #622606).
	for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
		if (player->_active && player->_id == sound)
			break;
	}

	if (!i)
		player = allocate_player(128);
	if (!player)
		return false;

	player->clear();
	return player->startSound (sound, driver);
}


Player *IMuseInternal::allocate_player(byte priority) {
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

void IMuseInternal::init_players() {
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		player->_active = false;
		player->_se = this;
	}
}

void IMuseInternal::init_sustaining_notes() {
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

void IMuseInternal::init_volume_fader() {
	VolumeFader *vf = _volume_fader;
	int i;

	for (i = ARRAYSIZE(_volume_fader); i != 0; i--, vf++)
		vf->initialize();

	_active_volume_faders = false;
}

void IMuseInternal::init_parts() {
	Part *part;
	int i;

	for (i = 0, part = _parts; i != ARRAYSIZE(_parts); i++, part++) {
		part->init();
		part->_slot = i;
	}
}

int IMuseInternal::stopSound(int sound) {
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

int IMuseInternal::stop_all_sounds() {
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->_active)
			player->clear();
	}
	return 0;
}

void IMuseInternal::on_timer (MidiDriver *midi) {
	if (_paused)
		return;

	sequencer_timers (midi);
	expire_sustain_notes (midi);
	expire_volume_faders (midi);
}

void IMuseInternal::sequencer_timers (MidiDriver *midi) {
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->_active && player->_midi == midi) {
			player->sequencer_timer();
		}
	}
}

void IMuseInternal::handle_marker(uint id, byte data) {
	uint16 *p = 0;
	uint pos;

	if (_queue_adding && _queue_sound == id && data == _queue_marker)
		return;

	// Fix for bug #733401: It would seem that sometimes the
	// queue read position gets out of sync (possibly just
	// reset to zero). Therefore, the read position should
	// skip over any empty (i.e. all zeros) queue entries
	// until it finds a legit entry to review.
	pos = _queue_end;
	while (pos != _queue_pos) {
		p = _cmd_queue[pos].array;
		if ((p[0] | p[1] | p[2] | p[3] | p[4] | p[5] | p[6] | p[7]) != 0)
			break;
		warning ("Skipping empty command queue entry at position %d", pos);
		pos = (pos + 1) & (ARRAYSIZE(_cmd_queue) - 1);
	}

	if (pos == _queue_pos)
		return;

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

		doCommand(p[0], p[1], p[2], p[3], p[4], p[5], p[6], 0);

		if (_queue_cleared)
			return;
		pos = _queue_end;
	} while (1);

	_queue_end = pos;
}

int IMuseInternal::get_channel_volume(uint a) {
	if (a < 8)
		return _channel_volume_eff[a];
	return (_master_volume * _music_volume / 255) >> 1;
}

Part *IMuseInternal::allocate_part (byte pri, MidiDriver *midi) {
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

	if (best) {
		best->uninit();
		reallocateMidiChannels (midi);
	} else {
		debug(1, "Denying part request");
	}
	return best;
}

void IMuseInternal::expire_sustain_notes (MidiDriver *midi) {
	SustainingNotes *sn, *next;
	Player *player;
	uint32 counter;

	for (sn = _sustain_notes_head; sn; sn = next) {
		next = sn->next;
		player = sn->player;
		if (player->_midi != midi) continue;

		counter = sn->counter + player->_timer_speed;
		sn->pos += counter >> 16;
		sn->counter = (unsigned short)counter & 0xFFFF;

		if (sn->pos >= sn->off_pos) {
			player->key_off(sn->chan, sn->note);

			// Unlink the node
			if (next)
				next->prev = sn->prev;
			if (sn->prev)
				sn->prev->next = next;
			else
				_sustain_notes_head = next;

			// And put it in the free list
			sn->next = _sustain_notes_free;
			_sustain_notes_free = sn;
		}
	}
}

void IMuseInternal::expire_volume_faders (MidiDriver *midi) {
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
			if (vf->player->_midi == midi)
				vf->on_timer(false);
		}
	}
}

void VolumeFader::on_timer(bool probe) {
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

byte VolumeFader::fading_to() {
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

int IMuseInternal::getSoundStatus(int sound) {
	int i;
	Player *player;
	for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
		if (player->_active && player->_id == (uint16)sound) {
			// Assume that anyone asking for the sound status is
			// really asking "is it ok if I start playing this
			// sound now?" So if the sound is about to fade out,
			// pretend it's not playing.
			if (player->is_fading_out())
				continue;
			return 1;
		}
	}
	return get_queue_sound_status(sound);
}

// This is exactly the same as getSoundStatus except that
// it treats sounds that are fading out just the same as
// other sounds. This is the method to use when determining
// what resources to expire from memory.
bool IMuseInternal::get_sound_active(int sound) {
	int i;
	Player *player;
	for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
		if (player->_active && player->_id == (uint16)sound)
			return 1;
	}
	return (get_queue_sound_status(sound) != 0);
}

int IMuseInternal::get_queue_sound_status(int sound) {
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

int IMuseInternal::set_volchan(int sound, int volchan) {
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

int IMuseInternal::clear_queue() {
	_queue_adding = false;
	_queue_cleared = true;
	_queue_pos = 0;
	_queue_end = 0;
	_trigger_count = 0;
	return 0;
}

int IMuseInternal::enqueue_command(int a, int b, int c, int d, int e, int f, int g) {
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

int IMuseInternal::query_queue(int param) {
	switch (param) {
	case 0: // Get trigger count
		return _trigger_count;
	case 1: // Get trigger type
		if (_queue_end == _queue_pos)
			return -1;
		return _cmd_queue[_queue_end].array[1];
	case 2: // Get trigger sound
		if (_queue_end == _queue_pos)
			return 0xFF;
		return _cmd_queue[_queue_end].array[2];
	default:
		return -1;
	}
}

int IMuseInternal::get_music_volume() {
	return _music_volume;
}

int IMuseInternal::set_music_volume(uint vol) {
	if (vol > 255)
		vol = 255;
	else if (vol < 0)
		vol = 0;

	if (_music_volume == vol)
		return 0;
	_music_volume = vol;
	vol = vol * _master_volume / 255;
	for (uint i = 0; i < ARRAYSIZE (_channel_volume); i++) {
		_channel_volume_eff[i] = _channel_volume[i] * vol / 255;
	}
	if (!_paused)
		update_volumes();
	return 0;
}

int IMuseInternal::set_master_volume (uint vol) {
	if (vol > 255)
		vol = 255;
	else if (vol < 0)
		vol = 0;
	if (_master_volume == vol)
		return 0;

	_master_volume = vol;
	vol = vol * _music_volume / 255;
	for (uint i = 0; i < ARRAYSIZE (_channel_volume); i++) {
		_channel_volume_eff[i] = _channel_volume[i] * vol / 255;
	}
	if (!_paused)
		update_volumes();
	return 0;
}

int IMuseInternal::get_master_volume() {
	return _master_volume;
}

int IMuseInternal::terminate() {
	if (_midi_adlib) {
		_midi_adlib->close();
		delete _midi_adlib;
		_midi_adlib = 0;
	}

	if (_midi_native) {
		_midi_native->close();
		delete _midi_native;
		_midi_native = 0;
	}

	return 0;
}

int IMuseInternal::enqueue_trigger(int sound, int marker) {
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

int32 IMuseInternal::doCommand(int a, int b, int c, int d, int e, int f, int g, int h) {
	int i;
	byte cmd = a & 0xFF;
	byte param = a >> 8;
	Player *player = NULL;

	if (!_initialized && (cmd || param))
		return -1;

#ifdef IMUSE_DEBUG
	debug (0, "doCommand - %d (%d/%d), %d, %d, %d, %d, %d, %d, %d", a, (int) param, (int) cmd, b, c, d, e, f, g, h);
#endif

	if (param == 0) {
		switch (cmd) {
		case 6:
			if (b > 127)
				return -1;
			else
				return set_master_volume ((b << 1) | (b ? 0 : 1)); // Convert b from 0-127 to 0-255
		case 7:
			return _master_volume >> 1; // Convert from 0-255 to 0-127
		case 8:
			return startSound(b) ? 0 : -1;
		case 9:
			return stopSound(b);
		case 10: // FIXME: Sam and Max - Not sure if this is correct
			return stop_all_sounds();
		case 11:
			return stop_all_sounds();
		case 12:
			// Sam & Max: Player-scope commands
			for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
				if (player->_active && player->_id == (uint16)b)
					break;
			}
			if (!i)
				return -1;

			switch (d) {
			case 6:
				// Set player volume.
				return player->set_vol (e);
			default:
				warning("IMuseInternal::doCommand (6) unsupported sub-command %d", d);
			}
			return -1;
		case 13:
			return getSoundStatus(b);
		case 14:
			// Sam and Max: Volume Fader?
			// Prevent instantaneous volume fades.
			// Fixes a Ball of Twine issue, but might not be the right long-term solution.
			if (f != 0) {
				for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
					if (player->_active && player->_id == (uint16)b) {
						player->fade_vol(e, f);
						return 0;
					}
				}
			}
			return -1;
		case 15:
			// Sam & Max: Set hook for a "maybe" jump
			for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
				if (player->_active && player->_id == (uint16)b) {
					player->_hook.set (0, d, 0);
					return 0;
				}
			}
			return -1;
		case 16:
			return set_volchan(b, c);
		case 17:
			if (g_scumm->_gameId != GID_SAMNMAX) {
				return set_channel_volume(b, c);
			} else {
				if (e || f || g || h) 
					return ImSetTrigger (b, d, e, f, g, h);
				else
					return ImClearTrigger (b, d);
			}
		case 18:
			if (g_scumm->_gameId != GID_SAMNMAX) {
				return set_volchan_entry(b, c);
			} else {
				// Sam & Max: ImCheckTrigger.
				// According to Mike's notes to Ender,
				// this function returns the number of triggers
				// associated with a particular player ID and
				// trigger ID.
				a = 0;
				for (i = 0; i < 16; ++i) {
					if (_snm_triggers [i].sound == b && _snm_triggers [i].id &&
					    (d == -1 || _snm_triggers [i].id == d))
					{
						++a;
					}
				}
				return a;
			}
		case 19:
			// Sam & Max: ImClearTrigger
			// This should clear a trigger that's been set up
			// with ImSetTrigger (cmd == 17). Seems to work....
			return ImClearTrigger (b, d);
		case 20:
			// Sam & Max: Deferred Command
			// FIXME: Right now this acts as an immediate command.
			// The significance of parameter b is unknown.
			warning ("Incomplete support for iMuse::doCommand(20)");
			return doCommand (c, d, e, f, g, h, 0, 0);
		case 2:
		case 3:
			return 0;
		default:
			warning("doCommand (%d [%d/%d], %d, %d, %d, %d, %d, %d, %d) unsupported", a, param, cmd, b, c, d, e, f, g, h);
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
			if (g_scumm->_gameId == GID_SAMNMAX) {
				if (d == 1) // Measure number
					return ((player->_beat_index - 1) >> 2) + 1;
				else if (d == 2) // Beat number
					return player->_beat_index;
				return -1;
			} else {
				return player->get_param(c, d);
			}
		case 1:
			if (g_scumm->_gameId == GID_SAMNMAX)
				player->jump (d - 1, (e - 1) * 4 + f, ((g * player->_ticks_per_beat) >> 2) + h);
			else
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
			warning("doCommand (%d [%d/%d], %d, %d, %d, %d, %d, %d, %d) unsupported", a, param, cmd, b, c, d, e, f, g, h);
			return -1;
		}
	}

	return -1;
}

int32 IMuseInternal::ImSetTrigger (int sound, int id, int a, int b, int c, int d) {
	// Sam & Max: ImSetTrigger.
	// Sets a trigger for a particular player and
	// marker ID, along with doCommand parameters
	// to invoke at the marker. The marker is
	// represented by MIDI SysEx block 00 xx (F7)
	// where "xx" is the marker ID.
	uint16 oldest_trigger = 0;
	ImTrigger *oldest_ptr = NULL;

	int i;
	ImTrigger *trig = _snm_triggers;
	for (i = ARRAYSIZE (_snm_triggers); i; --i, ++trig) {
		if (!trig->id)
			break;
		if (trig->id == id && trig->sound == sound)
			break;

		uint16 diff;
		if (trig->expire <= _snm_trigger_index)
			diff = _snm_trigger_index - trig->expire;
		else
			diff = 0x10000 - trig->expire + _snm_trigger_index;

		if (!oldest_ptr || oldest_trigger < diff) {
			oldest_ptr = trig;
			oldest_trigger = diff;
		}
	}

	// If we didn't find a trigger, see if we can expire one.
	if (!i) {
		if (!oldest_ptr)
			return -1;
		trig = oldest_ptr;
	}

	trig->id = id;
	trig->sound = sound;
	trig->expire = (++_snm_trigger_index & 0xFFFF);
	trig->command [0] = a;
	trig->command [1] = b;
	trig->command [2] = c;
	trig->command [3] = d;

	// If the command is to start a sound, stop that sound if it's already playing.
	// This fixes some carnival music problems.
	if (trig->command [0] == 8 && getSoundStatus (trig->command [1]))
		stopSound (trig->command [1]);
	return 0;
}

int32 IMuseInternal::ImClearTrigger (int sound, int id) {
	int count = 0;
	int i;
	for (i = 0; i < 16; ++i) {
		if (_snm_triggers [i].sound == sound && _snm_triggers [i].id &&
			(id == -1 || _snm_triggers [i].id == id))
		{
			_snm_triggers [i].sound = _snm_triggers [i].id = 0;
			++count;
		}
	}
	return (count > 0) ? 0 : -1;
}

int32 IMuseInternal::ImFireAllTriggers (int sound) {
	if (!sound) return 0;
	int count = 0;
	int i;
	for (i = 0; i < 16; ++i) {
		if (_snm_triggers [i].sound == sound)
		{
			_snm_triggers [i].sound = _snm_triggers [i].id = 0;
			doCommand (_snm_triggers [i].command [0],
			           _snm_triggers [i].command [1],
			           _snm_triggers [i].command [2],
			           _snm_triggers [i].command [3],
			           0, 0, 0, 0);
			++count;
		}
	}
	return (count > 0) ? 0 : -1;
}

int IMuseInternal::set_channel_volume(uint chan, uint vol)
{
	if (chan >= 8 || vol > 127)
		return -1;

	_channel_volume[chan] = vol;
	_channel_volume_eff[chan] = _master_volume * _music_volume * vol / 255 / 255;
	update_volumes();
	return 0;
}

void IMuseInternal::update_volumes() {
	Player *player;
	int i;

	for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
		if (player->_active)
			player->set_vol(player->_volume);
	}
}

int IMuseInternal::set_volchan_entry(uint a, uint b) {
	if (a >= 8)
		return -1;
	_volchan_table[a] = b;
	return 0;
}

int HookDatas::query_param(int param, byte chan) {
	switch (param) {
	case 18:
		return _jump[0];
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

int HookDatas::set(byte cls, byte value, byte chan) {
	switch (cls) {
	case 0:
		_jump[1] = _jump[0];
		_jump[0] = value;
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

VolumeFader *IMuseInternal::allocate_volume_fader() {
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

Player *IMuseInternal::get_player_byid(int id) {
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

int IMuseInternal::get_volchan_entry(uint a) {
	if (a < 8)
		return _volchan_table[a];
	return -1;
}

uint32 IMuseInternal::property(int prop, uint32 value) {
	switch (prop) {
	case IMuse::PROP_TEMPO_BASE:
		// This is a specified as a percentage of normal
		// music speed. The number must be an integer
		// ranging from 50 to 200 (for 50% to 200% normal speed).
		if (value >= 50 && value <= 200)
			_tempoFactor = value;
		break;

	case IMuse::PROP_NATIVE_MT32:
		Instrument::nativeMT32 (value > 0);
		break;

	case IMuse::PROP_MULTI_MIDI:
		_enable_multi_midi = (value > 0);
		break;
	}
	return 0;
}

void IMuseInternal::setBase(byte **base) {
	_base_sounds = base;
}

IMuseInternal *IMuseInternal::create (OSystem *syst, MidiDriver *native_midi) {
	IMuseInternal *i = new IMuseInternal;
	i->initialize(syst, native_midi);
	return i;
}

int IMuseInternal::initialize(OSystem *syst, MidiDriver *native_midi) {
	int i;

	_midi_native = native_midi;
	_midi_adlib = NULL;
	if (native_midi)
		initMidiDriver (_midi_native);

	if (!_tempoFactor) _tempoFactor = 100;
	_master_volume = 255;
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

void IMuseInternal::initMidiDriver (MidiDriver *midi) {
	// Open MIDI driver
	int result = midi->open();
	if (result)
		error("IMuse initialization - ", MidiDriver::getErrorName(result));

	// Connect to the driver's timer
	midi->setTimerCallback (midi, &IMuseInternal::midiTimerCallback);
}

void IMuseInternal::init_queue() {
	_queue_adding = false;
	_queue_pos = 0;
	_queue_end = 0;
	_trigger_count = 0;
}

void IMuseInternal::pause(bool paused) {
	int vol = _music_volume;
	if (paused)
		_music_volume = 0;
	update_volumes();
	_music_volume = vol;

	_paused = paused;
}

////////////////////////////////////////
//
//  Player implementation
//
////////////////////////////////////////

int Player::fade_vol(byte vol, int time) {
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

bool Player::is_fading_out() {
	VolumeFader *vf = _se->_volume_fader;
	int i;

	for (i = 0; i < 8; i++, vf++) {
		if (vf->active && vf->direction < 0 && vf->player == this && vf->fading_to() == 0)
			return true;
	}
	return false;
}

void Player::clear() {
	uninit_seq();
	cancel_volume_fade();
	uninit_parts();
	_se->ImFireAllTriggers (_id);
	_active = false;
	_ticks_per_beat = TICKS_PER_BEAT;
	_midi = NULL;
}

bool Player::startSound (int sound, MidiDriver *midi) {
	void *mdhd;

	mdhd = _se->findTag(sound, MDHD_TAG, 0);
	if (mdhd == NULL) {
		mdhd = _se->findTag(sound, MDPG_TAG, 0);
		if (mdhd == NULL) {
				warning("P::startSound failed: Couldn't find %s", MDHD_TAG);
				return false;
		}
	}

	_mt32emulate = _se->isMT32(sound);
	_isGM = _se->isGM(sound);

	_parts = NULL;
	_active = true;
	_midi = midi;
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
		_midi = NULL;
		return false;
	}
	return true;
}

void Player::hook_clear() {
	memset(&_hook, 0, sizeof(_hook));
}

int Player::start_seq_sound(int sound) {
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

void Player::set_tempo(uint32 b) {
	uint32 i, j;

	i = _midi->getBaseTempo();

	j = _tempo = b;
	j = j * 100 / _se->_tempoFactor;

	while (i & 0xFFFF0000 || j & 0xFFFF0000) {
		i >>= 1;
		j >>= 1;
	}

	_tempo_eff = (i << 16) / j;

	set_speed(_speed);
}

void Player::cancel_volume_fade() {
	VolumeFader *vf = _se->_volume_fader;
	int i;

	for (i = 0; i < 8; i++, vf++) {
		if (vf->active && vf->player == this)
			vf->active = false;
	}
}

void Player::uninit_parts() {
	if (_parts && _parts->_player != this)
		error("asd");
	while (_parts)
		_parts->uninit();

	// In case another player is waiting to allocate parts
	if (_midi)
		_se->reallocateMidiChannels (_midi);
}

void Player::uninit_seq() {
	_abort = true;
}

void Player::set_speed(byte speed) {
	_speed = speed;
	_timer_speed = (_tempo_eff * speed >> 7);
}

byte *Player::parse_midi(byte *s) {
	byte cmd, chan, note, velocity, control;
	uint value;
	Part *part;

	cmd = *s++;

	chan = cmd & 0xF;

	switch (cmd >> 4) {
	case 0x8: // Key Off
		note = *s++;
		if (!_scanning) {
			key_off(chan, note);
		} else {
			clear_active_note(chan, note);
		}
		s++; // Skip velocity
		break;

	case 0x9: // Key On
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

	case 0xA: // Aftertouch
		s += 2;
		break;

	case 0xB: // Control Change
		control = *s++;
		value = *s++;
		part = get_part(chan);
		if (!part)
			break;

		switch (control) {
		case 1: // Modulation Wheel
			part->set_modwheel(value);
			break;
		case 7: // Volume
			part->set_vol(value);
			break;
		case 10: // Pan Position
			part->set_pan(value - 0x40);
			break;
		case 16: // Pitchbend Factor (non-standard)
			part->set_pitchbend_factor(value);
			break;
		case 17: // GP Slider 2
			part->set_detune(value - 0x40);
			break;
		case 18: // GP Slider 3
			part->set_pri(value - 0x40);
			_se->reallocateMidiChannels (_midi);
			break;
		case 64: // Sustain Pedal
			part->set_pedal(value != 0);
			break;
		case 91: // Effects Level
			part->set_effect_level(value);
			break;
		case 93: // Chorus Level
			part->set_chorus(value);
			break;
		default:
			warning("parse_midi: invalid control %d", control);
		}
		break;

	case 0xC: // Program Change
		value = *s++;
		part = get_part(chan);
		if (part) {
			if (_isGM) {
				if (value < 128)
					part->set_program(value);
			} else {
				if (value < 32)
					part->load_global_instrument(value);
			}
		}
		break;

	case 0xD: // Channel Pressure
		s++;
		break;

	case 0xE: // Pitch Bend
		part = get_part(chan);
		if (part)
			part->set_pitchbend(((s[1] << 7) | s[0]) - 0x2000);
		s += 2;
		break;

	case 0xF:
		if (chan == 0) {
			uint size = get_delta_time(&s);
			parse_sysex(s, size);
			s += size;
		} else if (chan == 0xF) {
			cmd = *s++;
			if (cmd == 47)
				goto Error; // End of song
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

void Player::parse_sysex(byte *p, uint len) {
	byte code;
	byte a;
	uint b;
	byte buf[128];
	Part *part;

	// Check SysEx manufacturer.
	// Roland is 0x41
	a = *p++;
	--len;
	if (a != IMUSE_SYSEX_ID) {
		if (a == ROLAND_SYSEX_ID) {
			// Roland custom instrument definition.
			part = get_part (p[0] & 0x0F);
			if (part) {
				part->_instrument.roland (p - 1);
				if (part->clearToTransmit())
					part->_instrument.send (part->_mc);
			}
		} else {
			warning ("Unknown SysEx manufacturer 0x%02X", (int) a);
		}
		return;
	}
	--len;

	// Too big?
	if (len >= sizeof(buf) * 2)
		return;

#ifdef IMUSE_DEBUG
	for (a = 0; a < len + 1 && a < 20; ++a) {
		sprintf ((char *)&buf[a*3], " %02X", p[a]);
	} // next for
	if (a < len + 1) {
		buf[a*3] = buf[a*3+1] = buf[a*3+2] = '.';
		++a;
	} // end if
	buf[a*3] = '\0';
	debug (0, "SysEx:%s", buf);
#endif

	switch (code = *p++) {
	case 0:
		if (g_scumm->_gameId != GID_SAMNMAX) {
			// There are 17 bytes of useful information beyond
			// what we've read so far. All we know about them is
			// as follows:
			//   BYTE 00: Channel #
			//   BYTE 02: BIT 01 (0x01): Part on? (1 = yes)
			//   BYTE 05: Volume (upper 4 bits) [guessing]
			//   BYTE 06: Volume (lower 4 bits) [guessing]
			//   BYTE 09: BIT 04 (0x08): Percussion? (1 = yes)
			//   BYTE 15: Program (upper 4 bits)
			//   BYTE 16: Program (lower 4 bits)
			part = get_part (p[0] & 0x0F);
			if (part) {
				part->set_onoff (p[2] & 0x01);
				part->set_vol ((p[5] & 0x0F) << 4 | (p[6] & 0x0F));
				part->_percussion = _isGM ? ((p[9] & 0x08) > 0) : false;
				if (part->_percussion) {
					if (part->_mc) {
						part->off();
						_se->reallocateMidiChannels (_midi);
					}
				} else {
					// Even in cases where a program does not seem to be specified,
					// i.e. bytes 15 and 16 are 0, we send a program change because
					// 0 is a valid program number. MI2 tests show that in such
					// cases, a regular program change message always seems to follow
					// anyway.
					part->_instrument.program ((p[15] & 0x0F) << 4 | (p[16] & 0x0F), _mt32emulate);
					part->sendAll();
				}
			}
		} else {
			// Sam & Max: Trigger Event
			// Triggers are set by doCommand (ImSetTrigger).
			// When a SysEx marker is encountered whose sound
			// ID and marker ID match what was set by ImSetTrigger,
			// something magical is supposed to happen....
			for (a = 0; a < 16; ++a) {
				if (_se->_snm_triggers [a].sound == _id &&
				    _se->_snm_triggers [a].id == *p)
				{
					_se->_snm_triggers [a].sound = _se->_snm_triggers [a].id = 0;
					_se->doCommand (_se->_snm_triggers [a].command [0],
					                 _se->_snm_triggers [a].command [1],
					                 _se->_snm_triggers [a].command [2],
					                 _se->_snm_triggers [a].command [3],
					                 0, 0, 0, 0);
					break;
				}
			}
		} // end if
		break;

	case 1:
		// This SysEx is used in Sam & Max for maybe_jump.
		if (_scanning)
			break;
		maybe_jump (p[0], p[1] - 1, (read_word (p + 2) - 1) * 4 + p[4], ((p[5] * _ticks_per_beat) >> 2) + p[6]);
		break;

	case 2: // Start of song. Ignore for now.
		break;

	case 16: // Adlib instrument definition (Part)
		a = *p++ & 0x0F;
		++p; // Skip hardware type
		part = get_part(a);
		if (part) {
			if (len == 63) {
				decode_sysex_bytes(p, buf, len - 3);
				part->set_instrument((byte *) buf);
			} else {
				// SPK tracks have len == 49 here, and are not supported
				part->set_program (254); // Must be invalid, but not 255 (which is reserved)
			}
		}
		break;

	case 17: // Adlib instrument definition (Global)
		p += 2; // Skip hardware type and... whatever came right before it
		a = *p++;
		decode_sysex_bytes(p, buf, len - 4);
		_se->setGlobalAdlibInstrument (a, buf);
		break;

	case 33: // Parameter adjust
		a = *p++ & 0x0F;
		++p; // Skip hardware type
		decode_sysex_bytes(p, buf, len - 3);
		part = get_part(a);
		if (part)
			part->set_param(read_word(buf), read_word(buf + 2));
		break;

	case 48: // Hook - jump
		if (_scanning)
			break;
		decode_sysex_bytes(p + 1, buf, len - 2);
		maybe_jump (buf[0], read_word (buf + 1), read_word (buf + 3), read_word (buf + 5));
		break;

	case 49: // Hook - global transpose
		decode_sysex_bytes(p + 1, buf, len - 2);
		maybe_set_transpose(buf);
		break;

	case 50: // Hook - part on/off
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p, buf + 1, len - 2);
		maybe_part_onoff(buf);
		break;

	case 51: // Hook - set volume
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p, buf + 1, len - 2);
		maybe_set_volume(buf);
		break;

	case 52: // Hook - set program
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p, buf + 1, len - 2);
		maybe_set_program(buf);
		break;

	case 53: // Hook - set transpose
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p, buf + 1, len - 2);
		maybe_set_transpose_part(buf);
		break;

	case 64: // Marker
		p++;
		len -= 2;
		while (len--) {
			_se->handle_marker(_id, *p++);
		}
		break;

	case 80: // Loop
		decode_sysex_bytes(p + 1, buf, len - 2);
		set_loop(read_word(buf),
						 read_word(buf + 2), read_word(buf + 4), read_word(buf + 6), read_word(buf + 8)
			);
		break;

	case 81: // End loop
		clear_loop();
		break;

	case 96: // Set instrument
		part = get_part(p[0] & 0x0F);
		b = (p[1] & 0x0F) << 12 | (p[2] & 0x0F) << 8 | (p[4] & 0x0F) << 4 | (p[4] & 0x0F);
		if (part)
			part->set_instrument(b);
		break;

	default:
		warning ("Unknown SysEx command %d", (int) code);
	}
}

void Player::decode_sysex_bytes(byte *src, byte *dst, int len) {
	while (len >= 0) {
		*dst++ = (src[0] << 4) | (src[1] & 0xF);
		src += 2;
		len -= 2;
	}
}

void Player::maybe_jump (byte cmd, uint track, uint beat, uint tick) {
	// Is this the hook I'm waiting for?
	if (cmd && _hook._jump[0] != cmd)
		return;

	// Reset hook?
	if (cmd != 0 && cmd < 0x80) {
		_hook._jump[0] = _hook._jump[1];
		_hook._jump[1] = 0;
	}

	jump (track, beat, tick);
}

void Player::maybe_set_transpose(byte *data) {
	byte cmd;

	cmd = data[0];

	// Is this the hook I'm waiting for?
	if (cmd && _hook._transpose != cmd)
		return;

	// Reset hook?
	if (cmd != 0 && cmd < 0x80)
		_hook._transpose = 0;

	set_transpose(data[1], (int8)data[2]);
}

void Player::maybe_part_onoff(byte *data) {
	byte cmd, *p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	p = &_hook._part_onoff[chan];

	// Is this the hook I'm waiting for?
	if (cmd && *p != cmd)
		return;

	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part = get_part(chan);
	if (part)
		part->set_onoff(data[2] != 0);
}

void Player::maybe_set_volume(byte *data) {
	byte cmd;
	byte *p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	p = &_hook._part_volume[chan];

	// Is this the hook I'm waiting for?
	if (cmd && *p != cmd)
		return;

	// Reset hook?
	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part = get_part(chan);
	if (part)
		part->set_vol(data[2]);
}

void Player::maybe_set_program(byte *data) {
	byte cmd;
	byte *p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	// Is this the hook I'm waiting for?
	p = &_hook._part_program[chan];

	if (cmd && *p != cmd)
		return;

	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part = get_part(chan);
	if (part)
		part->set_program(data[2]);
}

void Player::maybe_set_transpose_part(byte *data) {
	byte cmd;
	byte *p;
	uint chan;

	cmd = data[1];
	chan = data[0];

	// Is this the hook I'm waiting for?
	p = &_hook._part_transpose[chan];

	if (cmd && *p != cmd)
		return;

	// Reset hook?
	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part_set_transpose(chan, data[2], (int8)data[3]);
}

int Player::set_transpose(byte relative, int b) {
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

void Player::clear_active_notes() {
	memset(_se->_active_notes, 0, sizeof(_se->_active_notes));
}

void Player::clear_active_note(int chan, byte note) {
	_se->_active_notes[note] &= ~(1 << chan);
}

void Player::set_active_note(int chan, byte note) {
	_se->_active_notes[note] |= (1 << chan);
}

void Player::part_set_transpose(uint8 chan, byte relative, int8 b) {
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

void Player::key_on(uint8 chan, uint8 note, uint8 velocity) {
	Part *part;

	part = get_part(chan);
	if (!part || !part->_on)
		return;

	part->key_on(note, velocity);
}

void Player::key_off(uint8 chan, uint8 note) {
	Part *part;

	for (part = _parts; part; part = part->_next) {
		if (part->_chan == (byte)chan && part->_on)
			part->key_off(note);
	}
}

bool Player::jump(uint track, uint beat, uint tick) {
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

	if (beat == 0)
		beat = 1;

	topos = (beat - 1) * _ticks_per_beat + tick;

	if (track == _track_index && topos >= _next_pos) {
		scanpos = _song_offset + mtrk;
		curpos = _next_pos;
	} else {
		scanpos = mtrk;
		curpos = get_delta_time(&scanpos);
	}

	while (curpos < topos) {
		skip_midi_cmd(&scanpos);
		if (!scanpos)
			return false;
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
	return true;
}

bool Player::set_loop(uint count, uint tobeat, uint totick, uint frombeat, uint fromtick) {
	if (tobeat + 1 >= frombeat)
		return false;

	if (tobeat == 0)
		tobeat = 1;

	_loop_counter = 0; // Because of possible interrupts
	_loop_to_beat = tobeat;
	_loop_to_tick = totick;
	_loop_from_beat = frombeat;
	_loop_from_tick = fromtick;
	_loop_counter = count;

	return true;
}

void Player::clear_loop() {
	_loop_counter = 0;
}

void Player::turn_off_pedals() {
	Part *part;

	for (part = _parts; part; part = part->_next) {
		if (part->_pedal)
			part->set_pedal(false);
	}
}

void Player::find_sustaining_notes(byte *a, byte *b, uint32 l) {
	uint32 pos;
	uint16 mask;
	uint16 *bitlist_ptr;
	SustainingNotes *sn, *next;
	IsNoteCmdData isnote;
	int j;
	uint num_active;
	uint max_off_pos;

	num_active = update_actives();

	// pos contains number of ticks since current position
	pos = _next_pos - _cur_pos;
	if ((int32)pos < 0)
		pos = 0;

	// Locate the positions where the notes are turned off.
	// Remember each note that was turned off.
	while (num_active != 0) {
		// Is note off?
		j = is_note_cmd(&a, &isnote);
		if (j == -1)
			break;
		if (j == 1) {
			mask = 1 << isnote.chan;
			bitlist_ptr = _se->_active_notes + isnote.note;
			if (*bitlist_ptr & mask) {
				*bitlist_ptr &= ~mask;
				num_active--;
				// Get a node from the free list
				if ((sn = _se->_sustain_notes_free) == NULL)
					return;
				_se->_sustain_notes_free = sn->next;

				// Insert it in the beginning of the used list
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

	// Find the maximum position where a note was turned off
	max_off_pos = 0;
	for (sn = _se->_sustain_notes_used; sn; sn = sn->next) {
		_se->_active_notes[sn->note] |= (1 << sn->chan);
		if (sn->off_pos > max_off_pos) {
			max_off_pos = sn->off_pos;
		}
	}

	// locate positions where notes are turned on
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
						// Unlink from the sustain list
						if (next)
							next->prev = sn->prev;
						if (sn->prev)
							sn->prev->next = next;
						else
							_se->_sustain_notes_used = next;
						// Insert into the free list
						sn->next = _se->_sustain_notes_free;
						_se->_sustain_notes_free = sn;
					}
					sn = next;
				}
			}
		}
		pos += get_delta_time(&b);
	}

	// Concatenate head and used list
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

Part *Player::get_part(uint8 chan) {
	Part *part;

	part = _parts;
	while (part) {
		if (part->_chan == chan)
			return part;
		part = part->_next;
	}

	part = _se->allocate_part (_priority, _midi);
	if (!part) {
		warning("no parts available");
		return NULL;
	}

	part->_chan = chan;
	part->setup(this);

	return part;
}

uint Player::update_actives() {
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

void Player::set_priority(int pri) {
	Part *part;

	_priority = pri;
	for (part = _parts; part; part = part->_next) {
		part->set_pri(part->_pri);
	}
	_se->reallocateMidiChannels (_midi);
}

void Player::set_pan(int pan) {
	Part *part;

	_pan = pan;
	for (part = _parts; part; part = part->_next) {
		part->set_pan(part->_pan);
	}
}

void Player::set_detune(int detune) {
	Part *part;

	_detune = detune;
	for (part = _parts; part; part = part->_next) {
		part->set_detune(part->_detune);
	}
}

int Player::scan(uint totrack, uint tobeat, uint totick) {
	byte *mtrk, *scanptr;
	uint32 curpos, topos;
	uint32 pos;

	assert(totrack >= 0 && tobeat >= 0 && totick >= 0);

	if (!_active)
		return -1;

	mtrk = _se->findTag(_song_index, "MTrk", totrack);
	if (!mtrk)
		return -1;

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
			return -1;
		}
		curpos += get_delta_time(&scanptr);
	}
	pos = scanptr - mtrk;

	_scanning = false;
	_se->reallocateMidiChannels (_midi);
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
	return 0;
}

void Player::turn_off_parts() {
	Part *part;

	for (part = _parts; part; part = part->_next)
		part->off();
	_se->reallocateMidiChannels (_midi);
}

void Player::play_active_notes() {
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

int Player::set_vol(byte vol) {
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

int Player::get_param(int param, byte chan) {
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

int Player::query_part_param(int param, byte chan) {
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
				return (int) part->_instrument;
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

void Player::sequencer_timer() {
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
				_song_offset = song_ptr - mtrk;
			}
		}
	}
}

////////////////////////////////////////////////////////////

enum {
	TYPE_PART = 1,
	TYPE_PLAYER = 2
};

int IMuseInternal::saveReference(void *me_ref, byte type, void *ref) {
	IMuseInternal *me = (IMuseInternal *)me_ref;
	switch (type) {
	case TYPE_PART:
		return (Part *)ref - me->_parts;
	case TYPE_PLAYER:
		return (Player *)ref - me->_players;
	default:
		error("saveReference: invalid type");
	}
}

void *IMuseInternal::loadReference(void *me_ref, byte type, int ref) {
	IMuseInternal *me = (IMuseInternal *)me_ref;
	switch (type) {
	case TYPE_PART:
		return &me->_parts[ref];
	case TYPE_PLAYER:
		return &me->_players[ref];
	default:
		error("loadReference: invalid type");
	}
}

int IMuseInternal::save_or_load(Serializer *ser, Scumm *scumm) {
	const SaveLoadEntry mainEntries[] = {
		MKLINE(IMuseInternal, _queue_end, sleUint8, VER_V8),
		MKLINE(IMuseInternal, _queue_pos, sleUint8, VER_V8),
		MKLINE(IMuseInternal, _queue_sound, sleUint16, VER_V8),
		MKLINE(IMuseInternal, _queue_adding, sleByte, VER_V8),
		MKLINE(IMuseInternal, _queue_marker, sleByte, VER_V8),
		MKLINE(IMuseInternal, _queue_cleared, sleByte, VER_V8),
		MKLINE(IMuseInternal, _master_volume, sleByte, VER_V8),
		MKLINE(IMuseInternal, _trigger_count, sleUint16, VER_V8),
		MKARRAY(IMuseInternal, _channel_volume[0], sleUint16, 8, VER_V8),
		MKARRAY(IMuseInternal, _volchan_table[0], sleUint16, 8, VER_V8),
		// TODO: Add _cmd_queue in here
		MKEND()
	};

	const SaveLoadEntry playerEntries[] = {
		MKREF(Player, _parts, TYPE_PART, VER_V8),
		MKLINE(Player, _active, sleByte, VER_V8),
		MKLINE(Player, _id, sleUint16, VER_V8),
		MKLINE(Player, _priority, sleByte, VER_V8),
		MKLINE(Player, _volume, sleByte, VER_V8),
		MKLINE(Player, _pan, sleInt8, VER_V8),
		MKLINE(Player, _transpose, sleByte, VER_V8),
		MKLINE(Player, _detune, sleInt8, VER_V8),
		MKLINE(Player, _vol_chan, sleUint16, VER_V8),
		MKLINE(Player, _vol_eff, sleByte, VER_V8),
		MKLINE(Player, _speed, sleByte, VER_V8),
		MKLINE(Player, _song_index, sleUint16, VER_V8),
		MKLINE(Player, _track_index, sleUint16, VER_V8),
		MKLINE(Player, _timer_counter, sleUint16, VER_V8),
		MKLINE(Player, _loop_to_beat, sleUint16, VER_V8),
		MKLINE(Player, _loop_from_beat, sleUint16, VER_V8),
		MKLINE(Player, _loop_counter, sleUint16, VER_V8),
		MKLINE(Player, _loop_to_tick, sleUint16, VER_V8),
		MKLINE(Player, _loop_from_tick, sleUint16, VER_V8),
		MKLINE(Player, _tempo, sleUint32, VER_V8),
		MKLINE(Player, _cur_pos, sleUint32, VER_V8),
		MKLINE(Player, _next_pos, sleUint32, VER_V8),
		MKLINE(Player, _song_offset, sleUint32, VER_V8),
		MKLINE(Player, _tick_index, sleUint16, VER_V8),
		MKLINE(Player, _beat_index, sleUint16, VER_V8),
		MKLINE(Player, _ticks_per_beat, sleUint16, VER_V8),
		MKLINE(Player, _hook._jump[0], sleByte, VER_V8),
		MKLINE(Player, _hook._transpose, sleByte, VER_V8),
		MKARRAY(Player, _hook._part_onoff[0], sleByte, 16, VER_V8),
		MKARRAY(Player, _hook._part_volume[0], sleByte, 16, VER_V8),
		MKARRAY(Player, _hook._part_program[0], sleByte, 16, VER_V8),
		MKARRAY(Player, _hook._part_transpose[0], sleByte, 16, VER_V8),
		MKEND()
	};

	const SaveLoadEntry volumeFaderEntries[] = {
		MKREF(VolumeFader, player, TYPE_PLAYER, VER_V8),
		MKLINE(VolumeFader, active, sleUint8, VER_V8),
		MKLINE(VolumeFader, curvol, sleUint8, VER_V8),
		MKLINE(VolumeFader, speed_lo_max, sleUint16, VER_V8),
		MKLINE(VolumeFader, num_steps, sleUint16, VER_V8),
		MKLINE(VolumeFader, speed_hi, sleInt8, VER_V8),
		MKLINE(VolumeFader, direction, sleInt8, VER_V8),
		MKLINE(VolumeFader, speed_lo, sleInt8, VER_V8),
		MKLINE(VolumeFader, speed_lo_counter, sleUint16, VER_V8),
		MKEND()
	};

	const SaveLoadEntry partEntries[] = {
		MKREF(Part, _next, TYPE_PART, VER_V8),
		MKREF(Part, _prev, TYPE_PART, VER_V8),
		MKREF(Part, _player, TYPE_PLAYER, VER_V8),
		MKLINE(Part, _pitchbend, sleInt16, VER_V8),
		MKLINE(Part, _pitchbend_factor, sleUint8, VER_V8),
		MKLINE(Part, _transpose, sleInt8, VER_V8),
		MKLINE(Part, _vol, sleUint8, VER_V8),
		MKLINE(Part, _detune, sleInt8, VER_V8),
		MKLINE(Part, _pan, sleInt8, VER_V8),
		MKLINE(Part, _on, sleUint8, VER_V8),
		MKLINE(Part, _modwheel, sleUint8, VER_V8),
		MKLINE(Part, _pedal, sleUint8, VER_V8),
		MK_OBSOLETE(Part, _program, sleUint8, VER_V8, VER_V16),
		MKLINE(Part, _pri, sleUint8, VER_V8),
		MKLINE(Part, _chan, sleUint8, VER_V8),
		MKLINE(Part, _effect_level, sleUint8, VER_V8),
		MKLINE(Part, _chorus, sleUint8, VER_V8),
		MKLINE(Part, _percussion, sleUint8, VER_V8),
		MKLINE(Part, _bank, sleUint8, VER_V8),
		MKEND()
	};

#ifdef _WIN32_WCE // Don't break savegames made with andys' build
	if (!ser->isSaving() && ser->checkEOFLoadStream())
		return 0;
#elif defined(__PALM_OS__) //	previous PalmOS ver. without imuse implementation or not saved(Oopps...forgot it !), is this really working ? will we have sound with old saved game ?
	if (!ser->isSaving() && ser->checkEOFLoadStream())
		return 0;	//palmfixme

#endif

	ser->_ref_me = this;
	ser->_save_ref = saveReference;
	ser->_load_ref = loadReference;

	ser->saveLoadEntries(this, mainEntries);
	ser->saveLoadArrayOf(_players, ARRAYSIZE(_players), sizeof(_players[0]), playerEntries);
	ser->saveLoadArrayOf(_parts, ARRAYSIZE(_parts), sizeof(_parts[0]), partEntries);

	{ // Load/save the instrument definitions, which were revamped with V11.
		int i;
		Part *part = &_parts[0];
		if (ser->getVersion() >= VER_V11) {
			for (i = ARRAYSIZE(_parts); i; --i, ++part) {
				part->_instrument.saveOrLoad (ser);
			}
		} else {
			for (i = ARRAYSIZE(_parts); i; --i, ++part)
				part->_instrument.clear();
		}
	}

	ser->saveLoadArrayOf(_volume_fader, ARRAYSIZE(_volume_fader),
		                 sizeof(_volume_fader[0]), volumeFaderEntries);

	if (!ser->isSaving()) {
		// Load all sounds that we need
		fix_players_after_load(scumm);
		init_sustaining_notes();
		_active_volume_faders = true;
		fix_parts_after_load();
		set_master_volume (_master_volume);

		if (_midi_native)
			reallocateMidiChannels (_midi_native);
		if (_midi_adlib)
			reallocateMidiChannels (_midi_adlib);
	}

	return 0;
}

#undef MKLINE
#undef MKEND

void IMuseInternal::fix_parts_after_load() {
	Part *part;
	int i;

	for (i = ARRAYSIZE(_parts), part = _parts; i != 0; i--, part++) {
		if (part->_player)
			part->fix_after_load();
	}
}

// Only call this routine from the main thread,
// since it uses getResourceAddress
void IMuseInternal::fix_players_after_load(Scumm *scumm) {
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->_active) {
			scumm->getResourceAddress(rtSound, player->_id);
			player->_midi = getBestMidiDriver (player->_id);
			if (player->_midi == NULL) {
				player->clear();
			} else {
				player->set_tempo(player->_tempo);
				player->_mt32emulate = isMT32(player->_id);
				player->_isGM = isGM(player->_id);
			}
		}
	}
}

void Part::set_detune(int8 detune) {
	_detune_eff = clamp((_detune = detune) + _player->_detune, -128, 127);
	if (_mc) {
		_mc->pitchBend (clamp(_pitchbend +
						(_detune_eff * 64 / 12) +
						(_transpose_eff * 8192 / 12), -8192, 8191));
	}
}

void Part::set_pitchbend(int value) {
	_pitchbend = value;
	if (_mc) {
		_mc->pitchBend (clamp(_pitchbend +
						(_detune_eff * 64 / 12) +
						(_transpose_eff * 8192 / 12), -8192, 8191));
	}
}

void Part::set_vol(uint8 vol) {
	_vol_eff = ((_vol = vol) + 1) * _player->_vol_eff >> 7;
	if (_mc)
		_mc->volume (_vol_eff);
}

void Part::set_pri(int8 pri) {
	_pri_eff = clamp((_pri = pri) + _player->_priority, 0, 255);
	if (_mc)
		_mc->priority (_pri_eff);
}

void Part::set_pan(int8 pan) {
	_pan_eff = clamp((_pan = pan) + _player->_pan, -64, 63);
	if (_mc)
		_mc->panPosition (_pan_eff + 0x40);
}

void Part::set_transpose(int8 transpose) {
	_transpose_eff = transpose_clamp((_transpose = transpose) + _player->_transpose, -12, 12);
	if (_mc) {
		_mc->pitchBend (clamp(_pitchbend +
						(_detune_eff * 64 / 12) +
						(_transpose_eff * 8192 / 12), -8192, 8191));
	}
}

void Part::set_pedal(bool value) {
	_pedal = value;
	if (_mc)
		_mc->sustain (_pedal);
}

void Part::set_modwheel(uint value) {
	_modwheel = value;
	if (_mc)
		_mc->modulationWheel (_modwheel);
}

void Part::set_chorus(uint chorus) {
	_chorus = chorus;
	if (_mc)
		_mc->chorusLevel (_effect_level);
}

void Part::set_effect_level(uint level)
{
	_effect_level = level;
	if (_mc)
		_mc->effectLevel (_effect_level);
}

void Part::fix_after_load() {
	set_transpose(_transpose);
	set_vol(_vol);
	set_detune(_detune);
	set_pri(_pri);
	set_pan(_pan);
	sendAll();
}

void Part::set_pitchbend_factor(uint8 value) {
	if (value > 12)
		return;
	set_pitchbend(0);
	_pitchbend_factor = value;
	if (_mc)
		_mc->pitchBendFactor (_pitchbend_factor);
}

void Part::set_onoff(bool on) {
	if (_on != on) {
		_on = on;
		if (!on)
			off();
		if (!_percussion)
			_player->_se->reallocateMidiChannels (_player->_midi);
	}
}

void Part::set_instrument(byte * data) {
	_instrument.adlib (data);
	if (clearToTransmit())
		_instrument.send (_mc);
}

void Part::load_global_instrument (byte slot) {
	_player->_se->copyGlobalAdlibInstrument (slot, &_instrument);
	if (clearToTransmit())
		_instrument.send (_mc);
}

void Part::key_on(byte note, byte velocity) {
	MidiChannel *mc = _mc;
	_actives[note >> 4] |= (1 << (note & 0xF));

	// DEBUG
	if (_unassigned_instrument && !_percussion) {
		_unassigned_instrument = false;
		if (!_instrument.isValid()) {
			warning ("[%02d] No instrument specified", (int) _chan);
			return;
		}
	}

	if (mc && _instrument.isValid()) {
		mc->noteOn (note, velocity);
	} else if (_percussion) {
		mc = _player->_midi->getPercussionChannel();
		if (!mc)
			return;
		mc->volume (_vol_eff);
		mc->programChange (_bank);
		mc->noteOn (note, velocity);
	}
}

void Part::key_off(byte note) {
	MidiChannel *mc = _mc;
	_actives[note >> 4] &= ~(1 << (note & 0xF));
	if (mc) {
		mc->noteOff (note);
	} else if (_percussion) {
		mc = _player->_midi->getPercussionChannel();
		if (mc)
			mc->noteOff (note);
	}
}

void Part::init() {
	_player = NULL;
	_next = NULL;
	_prev = NULL;
	_mc = NULL;
	memset(_actives, 0, sizeof (_actives));
}

void Part::setup(Player *player) {
	_player = player;

	// Insert first into player's list
	_prev = NULL;
	_next = player->_parts;
	if (player->_parts)
		player->_parts->_prev = this;
	player->_parts = this;

	_percussion = (player->_isGM && _chan == 9); // true;
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
	_instrument.clear();
	_unassigned_instrument = true;
	_chorus = 0;
	_modwheel = 0;
	_bank = 0;
	_pedal = false;
	_mc = NULL;
}

void Part::uninit() {
	if (!_player)
		return;
	off();

	// Unlink
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

void Part::off() {
	if (_mc) {
		_mc->allNotesOff();
		_mc->release();
		_mc = NULL;
	}
	memset (_actives, 0, sizeof(_actives));
}

bool Part::clearToTransmit() {
	if (_mc) return true;
	if (_instrument.isValid()) _player->_se->reallocateMidiChannels (_player->_midi);
	return false;
}

void Part::sendAll() {
	if (!clearToTransmit()) return;
	_mc->pitchBendFactor (_pitchbend_factor);
	_mc->pitchBend (clamp(_pitchbend +
	                (_detune_eff * 64 / 12) +
	                (_transpose_eff * 8192 / 12), -8192, 8191));
	_mc->volume (_vol_eff);
	_mc->sustain (_pedal);
	_mc->modulationWheel (_modwheel);
	_mc->panPosition (_pan_eff + 0x40);
	_mc->effectLevel (_effect_level);
	if (_instrument.isValid())
		_instrument.send (_mc);
	_mc->chorusLevel (_effect_level);
	_mc->priority (_pri_eff);
}

int Part::update_actives(uint16 *active) {
	int i, j;
	uint16 *act, mask, bits;
	int count = 0;

	bits = 1 << _chan;
	act = _actives;

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

void Part::set_program(byte program) {
	_bank = 0;
	_instrument.program (program, _player->_mt32emulate);
	if (clearToTransmit())
		_instrument.send (_mc);
}

void Part::set_instrument(uint b) {
	_bank = (byte)(b >> 8);
	_instrument.program ((byte) b, _player->_mt32emulate);
	if (clearToTransmit())
		_instrument.send (_mc);
}

////////////////////////////////////////
//
// Some more IMuseInternal stuff
//
////////////////////////////////////////

void IMuseInternal::midiTimerCallback (void *data) {
	MidiDriver *driver = (MidiDriver *) data;
	if (g_scumm->_imuse)
		g_scumm->_imuse->on_timer (driver);
}

void IMuseInternal::reallocateMidiChannels (MidiDriver *midi) {
	Part *part, *hipart;
	int i;
	byte hipri, lopri;
	Part *lopart;

	while (true) {
		hipri = 0;
		hipart = NULL;
		for (i = 32, part = _parts; i; i--, part++) {
			if (part->_player && part->_player->_midi == midi &&
			    !part->_percussion && part->_on &&
				!part->_mc && part->_pri_eff >= hipri)
			{
				hipri = part->_pri_eff;
				hipart = part;
			}
		}

		if (!hipart)
			return;

		if ((hipart->_mc = midi->allocateChannel()) == NULL) {
			lopri = 255;
			lopart = NULL;
			for (i = 32, part = _parts; i; i--, part++) {
				if (part->_mc && part->_mc->device() == midi && part->_pri_eff <= lopri) {
					lopri = part->_pri_eff;
					lopart = part;
				}
			}

			if (lopart == NULL || lopri >= hipri)
				return;
			lopart->off();

			if ((hipart->_mc = midi->allocateChannel()) == NULL)
				return;
		}
		hipart->sendAll();
	}
}

void IMuseInternal::setGlobalAdlibInstrument (byte slot, byte *data) {
	if (slot < 32) {
		_global_adlib_instruments[slot].adlib (data);
	}
}

void IMuseInternal::copyGlobalAdlibInstrument (byte slot, Instrument *dest) {
	if (slot >= 32)
		return;
	_global_adlib_instruments[slot].copy_to (dest);
}

////////////////////////////////////////////////////////////
//
// IMuse implementation
//
// IMuse actually serves as a concurency monitor front-end
// to IMuseInternal and ensures that only one thread
// accesses the object at a time. This is necessary to
// prevent scripts and the MIDI parser from yanking objects
// out from underneath each other.
//
////////////////////////////////////////////////////////////

IMuse::IMuse (OSystem *system, IMuseInternal *target) : _system (system), _target (target) { _mutex = system->create_mutex(); }
IMuse::~IMuse() { if (_mutex) _system->delete_mutex (_mutex); if (_target) delete _target; }
inline void IMuse::in() { _system->lock_mutex (_mutex); }
inline void IMuse::out() { _system->unlock_mutex (_mutex); }

void IMuse::on_timer (MidiDriver *midi) { in(); _target->on_timer (midi); out(); }
void IMuse::pause(bool paused) { in(); _target->pause (paused); out(); }
int IMuse::save_or_load(Serializer *ser, Scumm *scumm) { in(); int ret = _target->save_or_load (ser, scumm); out(); return ret; }
int IMuse::set_music_volume(uint vol) { in(); int ret = _target->set_music_volume (vol); out(); return ret; }
int IMuse::get_music_volume() { in(); int ret = _target->get_music_volume(); out(); return ret; }
int IMuse::set_master_volume(uint vol) { in(); int ret = _target->set_master_volume (vol); out(); return ret; }
int IMuse::get_master_volume() { in(); int ret = _target->get_master_volume(); out(); return ret; }
bool IMuse::startSound(int sound) { in(); bool ret = _target->startSound (sound); out(); return ret; }
int IMuse::stopSound(int sound) { in(); int ret = _target->stopSound (sound); out(); return ret; }
int IMuse::stop_all_sounds() { in(); int ret = _target->stop_all_sounds(); out(); return ret; }
int IMuse::getSoundStatus(int sound) { in(); int ret = _target->getSoundStatus (sound); out(); return ret; }
bool IMuse::get_sound_active(int sound) { in(); bool ret = _target->get_sound_active (sound); out(); return ret; }
int32 IMuse::doCommand(int a, int b, int c, int d, int e, int f, int g, int h) { in(); int32 ret = _target->doCommand (a,b,c,d,e,f,g,h); out(); return ret; }
int IMuse::clear_queue() { in(); int ret = _target->clear_queue(); out(); return ret; }
void IMuse::setBase(byte **base) { in(); _target->setBase (base); out(); }
uint32 IMuse::property(int prop, uint32 value) { in(); uint32 ret = _target->property (prop, value); out(); return ret; }

// The IMuse::create method provides a front-end factory
// for creating IMuseInternal without exposing that class
// to the client.
IMuse *IMuse::create (OSystem *syst, MidiDriver *midi) {
	IMuseInternal *engine = IMuseInternal::create (syst, midi);
	if (midi)
		midi->property (MidiDriver::PROP_SMALLHEADER, (g_scumm->_features & GF_SMALL_HEADER) ? 1 : 0);
	return new IMuse (syst, engine);
}
