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
 */
 
 #ifndef SOUND_H
 #define SOUND_H

#include "gmidi.h"		/* General Midi */

struct MP3OffsetTable {	/* Compressed Sound (.SO3) */
	int org_offset;
	int new_offset;
	int num_tags;
	int compressed_size;
};

typedef enum {			/* Mixer types */
  MIXER_STANDARD,
  MIXER_MP3,
  MIXER_MP3_CDMUSIC
} MixerType;

struct MixerChannel {	/* Mixer Channel */
	void *_sfx_sound;
	MixerType type;
	union {
	  struct {
	    uint32 _sfx_pos;
	    uint32 _sfx_size;
	    uint32 _sfx_fp_speed;
	    uint32 _sfx_fp_pos;
	  } standard;
#ifdef COMPRESSED_SOUND_FILE
	  struct {
	    struct mad_stream stream;
	    struct mad_frame frame;
	    struct mad_synth synth;
	    uint32 silence_cut;
	    uint32 pos_in_frame;
	    uint32 position;
	    uint32 size;
	  } mp3;
	  struct {
            struct mad_stream stream;
            struct mad_frame frame;
            struct mad_synth synth;
            uint32 pos_in_frame;
            uint32 position;
            uint32 size;
            uint32 buffer_size;
            mad_timer_t duration;
            bool   playing;
            FILE   *file;
          } mp3_cdmusic;
#endif
	} sound_data;
	void mix(int16 *data, uint32 len);
	void clear();
};

int clamp(int val, int min, int max);



struct FM_OPL;
struct Part;
struct MidiChannel;
struct MidiChannelAdl;
struct MidiChannelGM;
struct VolumeFader;
struct Player;
struct HookDatas;
struct SoundEngine;
struct SoundDriver;
struct Instrument;
struct AdlibSoundDriver;
struct MidiSoundDriver;

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
	byte param,flag0x40,flag0x10;
	Struct10 *s10;
};

struct InstrumentExtra {
	byte a,b,c,d,e,f,g,h;
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

struct Part {
	int _slot;
	SoundDriver *_drv;
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
	byte _percussion;
	byte _bank;

	void key_on(byte note, byte velocity);
	void key_off(byte note);
	void set_param(byte param, int value);
	void init(SoundDriver *_driver);
	void setup(Player *player);
	void uninit();
	void off();
	void silence();
	void set_instrument(uint b);
	void set_instrument(Instrument *data);

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


struct MidiChannel {
	Part *_part;
	MidiChannelAdl *adl() { return (MidiChannelAdl*)this; }
	MidiChannelGM *gm() { return (MidiChannelGM*)this; }
};

struct MidiChannelAdl : MidiChannel {
	MidiChannelAdl *_next,*_prev;
	byte _waitforpedal;
	byte _note;
	byte _channel;
	byte _twochan;
	byte _vol_1,_vol_2;
	int16 _duration;

	Struct10 _s10a;
	Struct11 _s11a;
	Struct10 _s10b;
	Struct11 _s11b;
};

struct MidiChannelGM : MidiChannel {
	byte _chan;
	uint16 _actives[8];
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

struct SoundDriver {
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
	virtual void init(SoundEngine *eng, OSystem *syst) = 0;
	virtual void update_pris() = 0;
	virtual void set_instrument(uint slot, byte *instr) = 0;
	virtual void part_set_instrument(Part *part, Instrument *instr) = 0;
	virtual void part_key_on(Part *part, byte note, byte velocity) = 0;
	virtual void part_key_off(Part *part, byte note) = 0;
	virtual void part_off(Part *part) = 0;
	virtual void part_changed(Part *part,byte what) = 0;
	virtual void part_set_param(Part *part, byte param, int value) = 0;
	virtual int part_update_active(Part *part,uint16 *active) = 0;
	virtual void generate_samples(int16 *buf, int len) = 0;
};

struct AdlibSoundDriver : SoundDriver {
private:
	FM_OPL *_opl;
	byte *_adlib_reg_cache;
	SoundEngine *_se;

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
	void adlib_setup_channel(int chan, Instrument *instr, byte vol_1, byte vol_2);
	byte adlib_read(byte port) { return _adlib_reg_cache[port]; }	
	void adlib_set_param(int channel, byte param, int value);
	void adlib_key_onoff(int channel);
	void adlib_write(byte port, byte value);
	void adlib_playnote(int channel, int note);

	MidiChannelAdl *allocate_midichan(byte pri);

	void reset_tick();
	void mc_off(MidiChannel *mc);	

	static void link_mc(Part *part, MidiChannelAdl *mc);
	static void mc_inc_stuff(MidiChannelAdl *mc, Struct10 *s10, Struct11 *s11);
	static void mc_init_stuff(MidiChannelAdl *mc, Struct10 *s10, Struct11 *s11, byte flags, InstrumentExtra *ie);
	static void struct10_init(Struct10 *s10, InstrumentExtra *ie);
	static byte struct10_ontimer(Struct10 *s10, Struct11 *s11);
	static void struct10_setup(Struct10 *s10);
	static int random_nr(int a);
	void mc_key_on(MidiChannel *mc, byte note, byte velocity);

public:
	void uninit();
	void init(SoundEngine *eng, OSystem *syst);
	void update_pris() { }
	void generate_samples(int16 *buf, int len);	
	void on_timer();	
	void set_instrument(uint slot, byte *instr);
	void part_set_instrument(Part *part, Instrument *instr);
	void part_key_on(Part *part, byte note, byte velocity);
	void part_key_off(Part *part, byte note);
	void part_set_param(Part *part, byte param, int value);
	void part_changed(Part *part,byte what);
	void part_off(Part *part);
	int part_update_active(Part *part,uint16 *active);
	void adjust_priorities() {}

	uint32 get_base_tempo() { 
#ifdef _WIN32_WCE
		return 0x1F0000 * 2;	// Sampled down to 11 kHz
#else //_WIN32_WCE
		return 0x1924E0;
#endif //_WIN32_WCE
	}

	byte get_hardware_type() { return 1; }
};

#if 0
struct MidiDriver {
	bool MidiInitialized;
	int DeviceType;
	int SeqDevice;
	void *_mo; /* midi out */
	
	void midiInit();
	void midiInitTimidity();
	void midiInitSeq();
	void midiInitWindows();
	void midiInitNull();
	void midiInitQuicktime();

	void MidiOut(int b);
	void MidiOutSeq(void *a, int b);
	void MidiOutWindows(void *a, int b);
	void MidiOutQuicktime(void *a, int b);
	void MidiOutMorphOS(void *a, int b);

	int connect_to_timidity(int port);
	int open_sequencer_device();
};
#endif

struct MidiSoundDriver : SoundDriver {	
	SoundEngine *_se;
	OSystem *_system;

	MidiChannelGM _midi_channels[9];

	int16 _midi_pitchbend_last[16];
	uint8 _midi_volume_last[16];
	bool _midi_pedal_last[16];
	byte _midi_modwheel_last[16];
	byte _midi_effectlevel_last[16];
	byte _midi_chorus_last[16];
	int8 _midi_pan_last[16];

	MidiDriver *_md;
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

public:
	void uninit();
	void init(SoundEngine *eng, OSystem *os);
	void update_pris();
	void part_off(Part *part);
	int part_update_active(Part *part,uint16 *active);

	void generate_samples(int16 *buf, int len) {}
	void on_timer() {}
	void set_instrument(uint slot, byte *instr) {}
	void part_set_instrument(Part *part, Instrument *instr) {}
	void part_set_param(Part *part, byte param, int value) {}
	void part_key_on(Part *part, byte note, byte velocity);
	void part_key_off(Part *part, byte note);
	void part_changed(Part *part,byte what);
	void midiSetDriver(MidiDriver *driver);

	static int midi_driver_thread(void *param);

	uint32 get_base_tempo() { return 0x400000; }
	byte get_hardware_type() { return 5; }
};

struct SoundEngine {
friend struct Player;
private:
	SoundDriver *_driver;

	byte **_base_sounds;

	Scumm *_s;
	
	byte _locked;
	byte _hardware_type;

	bool _paused;
	bool _active_volume_faders;
	bool _initialized;	
	byte _volume_fader_counter;

	uint _queue_end, _queue_pos, _queue_sound;
	byte _queue_adding;

	SustainingNotes *_sustain_notes_used;
	SustainingNotes *_sustain_notes_free;
	SustainingNotes *_sustain_notes_head;

	byte _queue_marker;
	byte _queue_cleared;
	byte _master_volume;
	byte _music_volume;	/* Global music volume. Percantage */

	uint16 _trigger_count;
	
	uint16 _channel_volume[8];
	uint16 _channel_volume_eff[8]; /* NoSave */
	uint16 _volchan_table[8];
	
	Player _players[8];
	SustainingNotes _sustaining_notes[24];
	VolumeFader _volume_fader[8];
	Part _parts[32];
	
	uint16 _active_notes[128];
	CommandQueue _cmd_queue[64];

	byte *findTag(int sound, char *tag, int index);
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
	void fix_players_after_load();

	static int saveReference(SoundEngine *me, byte type, void *ref);
	static void *loadReference(SoundEngine *me, byte type, int ref);

	void lock();
	void unlock();

public:
	void on_timer();
	Part *parts_ptr() { return _parts; }
	void pause(bool paused);
	int initialize(Scumm *scumm, SoundDriver *driver);
	int terminate();
	int save_or_load(Serializer *ser);
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
	void setBase(byte **base) { _base_sounds = base; }	

	SoundDriver *driver() { return _driver; }
	bool _mt32emulate;	
};


#endif