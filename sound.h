/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Change Log:
 * $Log$
 * Revision 1.1  2001/11/14 18:37:38  strigeus
 * music support,
 * fixed timing bugs
 *
 */

struct Part;
struct MidiChannel;
struct VolumeFader;
struct Player;
struct HookDatas;
struct SoundEngine;

struct Part {
	SoundEngine *_se;
	Part *_next, *_prev;
	MidiChannel *_mc;
	Player *_player;
	int16 _pitchbend;
	byte _pitchbend_factor;
	int8 _transpose,_transpose_eff;
	byte _vol,_vol_eff;
	int8 _detune,_detune_eff;
	int8 _pan,_pan_eff;
	bool _on;
	byte _modwheel;
	bool _pedal;
	byte _program;
	int8 _pri;
	byte _pri_eff;
	byte _chan;
	byte _effect_level;
	byte _chorus;
	byte _gmidi_5;
	byte _gmidi_1;

	void key_on(byte note, byte velocity);
	void key_off(byte note);
	void set_param(int b, byte c) {}
	void init(SoundEngine *se);
	void setup(Player *player);
	void uninit();
	void off();
	void silence();
	void set_instrument(uint b);
	void set_instrument(byte *data) {}

	void set_transpose(int8 transpose);
	void set_vol(uint8 volume);
	void set_detune(int8 detune);
	void set_pri(int8 pri, bool recalc);
	void set_pan(int8 pan);
	void set_modwheel(uint value);
	void set_pedal(bool value);
	void set_pitchbend(int value);
	void release_pedal();
	void set_program(byte program);
	void set_chorus(uint chorus);
	void set_effect_level(uint level);
	void set_chan_param(int b, int c) {}
	void mod_changed();
	void vol_changed();
	void pedal_changed();
	void modwheel_changed();
	void pan_changed();
	void effect_level_changed();
	void program_changed();
	void chorus_changed();
	int update_actives(uint16 *active);
	void set_pitchbend_factor(uint8 value);
	void set_onoff(bool on);

	void fix_after_load();
};

struct MidiChannel {
	Part *_part;
	byte _chan;
	uint16 _actives[8];

	void init(byte chan);
};

struct VolumeFader {
	Player *player;
	bool active;
	byte curvol;
	uint16 speed_lo_max,num_steps;
	int8 speed_hi;
	int8 direction;
	int8 speed_lo;
	uint16 speed_lo_counter;
	
	void initialize() { active = false; }
	void on_timer();
};

struct HookDatas {
	byte _jump,_transpose;
	byte _part_onoff[16];
	byte _part_volume[16];
	byte _part_program[16];
	byte _part_transpose[16];

	int query_param(int param, byte chan);
	int set(byte cls, byte value, byte chan);
};

struct Player {
	SoundEngine *_se;

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
	uint32 _tempo_eff; /* NoSave */
	uint32 _cur_pos;
	uint32 _next_pos;
	uint32 _song_offset;
	uint32 _timer_speed; /* NoSave */
	uint _tick_index;
	uint _beat_index;
	uint _ticks_per_beat;
	byte _speed; /* NoSave */
	bool _abort;

	HookDatas _hook;

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
	void maybe_jump(byte *data);
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
	void silence_parts();
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
	void sequencer_timer();
};


struct SustainingNotes {
	SustainingNotes *next;
	SustainingNotes *prev;
	Player *player;
	byte note,chan;
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

struct SoundEngine {
	void *_mo; /* midi out */

	byte **_base_sounds;

	Scumm *_s;
	
	byte _locked;

	bool _paused;
	bool _active_volume_faders;
	bool _initialized;
	byte _volume_fader_counter;

	uint _queue_end, _queue_pos, _queue_sound;
	byte _queue_adding;

	SustainingNotes *_sustain_notes_used;
	SustainingNotes *_sustain_notes_free;
	SustainingNotes *_sustain_notes_head;

	uint16 _timer_counter_1;

	byte _queue_marker;
	byte _queue_cleared;
	byte _master_volume;

	uint16 _trigger_count;
	
	uint16 _channel_volume[8];
	uint16 _channel_volume_eff[8]; /* NoSave */
	uint16 _volchan_table[8];
	
	Player _players[8];
	SustainingNotes _sustaining_notes[24];
	VolumeFader _volume_fader[8];
	Part _parts[32];
	MidiChannel _midi_channels[9];
	uint16 _active_notes[128];
	CommandQueue _cmd_queue[64];

	int16 _midi_pitchbend_last[16];
	uint8 _midi_volume_last[16];
	bool _midi_pedal_last[16];
	byte _midi_modwheel_last[16];
	byte _midi_effectlevel_last[16];
	byte _midi_chorus_last[16];
	int8 _midi_pan_last[16];

	byte *findTag(int sound, char *tag, int index);
	int initialize(Scumm *scumm);
	int terminate();
	int save_or_load(Serializer *ser);
	int set_master_volume(uint vol);
	int get_master_volume();
	bool start_sound(int sound);
	int stop_sound(int sound);
	int stop_all_sounds();
	int get_sound_status(int sound);
	int get_queue_sound_status(int sound);
	Player *allocate_player(byte priority);
	void handle_marker(uint id, byte data);
	int get_channel_volume(uint a);
	void init_players();
	void init_parts();
	void init_volume_fader();
	void init_sustaining_notes();
	void init_queue();

	void on_timer();
	void sequencer_timers();
	void expire_sustain_notes();
	void expire_volume_faders();

	void set_instrument(uint slot, byte *data) {}
	
	int32 do_command(int a, int b, int c, int d, int e, int f, int g, int h);
	Part *allocate_part(byte pri);

	int clear_queue();
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

	void midiPitchBend(byte chan, int16 pitchbend);
	void midiVolume(byte chan, byte volume);
	void midiPedal(byte chan, bool pedal);
	void midiModWheel(byte chan, byte modwheel);
	void midiEffectLevel(byte chan, byte level);
	void midiChorus(byte chan, byte chorus);
	void midiControl0(byte chan, byte value);
	void midiProgram(byte chan, byte program);
	void midiPan(byte chan, int8 pan);
	void midiNoteOn(byte chan, byte note, byte velocity);
	void midiNoteOff(byte chan, byte note);
	void midiSilence(byte chan);
	void midiInit();

	void adjust_priorities();

	void fix_parts_after_load();
	void fix_players_after_load();

	static int saveReference(SoundEngine *me, byte type, void *ref);
	static void *loadReference(SoundEngine *me, byte type, int ref);

	void lock();
	void unlock();

	void pause(bool paused);
};
