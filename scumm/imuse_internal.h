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

#ifndef DEFINED_IMUSE_INTERNAL
#define DEFINED_IMUSE_INTERNAL

#include "common/scummsys.h"
#include "scumm/instrument.h"
#include "sound/mididrv.h"

// Unremark this statement to activate some of
// the most common iMuse diagnostic messages.
// #define IMUSE_DEBUG

struct HookDatas;
struct ParameterFader;
struct DeferredCommand;
struct ImTrigger;
struct SustainingNotes;
struct CommandQueue;
struct IsNoteCmdData;
class  Player;
struct Part;
class  IMuseInternal;

// Some entities also referenced
class MidiParser;
class Scumm;
class OSystem;
class SoundMixer;



//////////////////////////////////////////////////
//
// Some constants
//
//////////////////////////////////////////////////

#define TICKS_PER_BEAT 480

#define IMUSE_SYSEX_ID 0x7D
#define ROLAND_SYSEX_ID 0x41
#define PERCUSSION_CHANNEL 9

#define TRIGGER_ID 0
#define COMMAND_ID 1

#define MDPG_TAG "MDpg"
#define MDHD_TAG "MDhd"



////////////////////////////////////////
//
//  Helper functions
//
////////////////////////////////////////

inline int clamp(int val, int min, int max) {
	if(val < min)
		return min;
	if(val > max)
		return max;
	return val;
}

inline int transpose_clamp(int a, int b, int c) {
	if(b > a)
		a += (b - a + 11) / 12 * 12;
	if(c < a)
		a -= (a - c + 11) / 12 * 12;
	return a;
}



//////////////////////////////////////////////////
//
// Entity declarations
//
//////////////////////////////////////////////////

struct HookDatas {
	byte _jump[2];
	byte _transpose;
	byte _part_onoff[16];
	byte _part_volume[16];
	byte _part_program[16];
	byte _part_transpose[16];

	int query_param(int param, byte chan);
	int set(byte cls, byte value, byte chan);
	HookDatas() { memset(this, 0, sizeof(HookDatas)); }
};

struct ParameterFader {
	enum {
		pfVolume = 1,
		pfTranspose = 3,
		pfSpeed = 4
	};

	int param;
	int start;
	int end;
	uint32 total_time;
	uint32 current_time;

	ParameterFader() { param = 0; }
	void init() { param = 0; }
};

struct DeferredCommand {
	uint32 time_left;
	int a, b, c, d, e, f;
	DeferredCommand() { memset(this, 0, sizeof(DeferredCommand)); }
};

struct ImTrigger {
	int sound;
	byte id;
	uint16 expire;
	int command [8];
	ImTrigger() { memset(this, 0, sizeof(ImTrigger)); }
};

struct CommandQueue {
	uint16 array[8];
	CommandQueue() { memset(this, 0, sizeof(CommandQueue)); }
};

class Player : public MidiDriver {
protected:
	MidiDriver *_midi;
	MidiParser *_parser;

	Part *_parts;
	bool _active;
	bool _scanning;
	int _id;
	byte _priority;
	byte _volume;
	int8 _pan;
	int8 _transpose;
	int8 _detune;
	byte _vol_eff;

	uint _track_index;
	uint _loop_to_beat;
	uint _loop_from_beat;
	uint _loop_counter;
	uint _loop_to_tick;
	uint _loop_from_tick;
	byte _speed;
	bool _abort;

	// This does not get used by us! It is only
	// here for save/load purposes, and gets
	// passed on to the MidiParser during
	// fixAfterLoad().
	uint32 _music_tick;

	HookDatas _hook;
	ParameterFader _parameterFaders[4];

	bool _isMT32;
	bool _isGM;

protected:
	// Player part
	void hook_clear();
	void uninit_parts();
	byte *parse_midi(byte *s);
	void key_off(uint8 chan, byte data);
	void key_on(uint8 chan, byte data, byte velocity);
	void part_set_transpose(uint8 chan, byte relative, int8 b);
	void parse_sysex(byte *p, uint len);
	void maybe_jump(byte cmd, uint track, uint beat, uint tick);
	void maybe_set_transpose(byte *data);
	void maybe_part_onoff(byte *data);
	void maybe_set_volume(byte *data);
	void maybe_set_program(byte *data);
	void maybe_set_transpose_part(byte *data);
	uint update_actives();
	void turn_off_pedals();
	int  query_part_param(int param, byte chan);
	void turn_off_parts();
	void play_active_notes();

	void transitionParameters();

	static void decode_sysex_bytes(const byte *src, byte *dst, int len);

	void clear_active_note(int chan, byte note);
	void set_active_note(int chan, byte note);
	void clear_active_notes();

	// Sequencer part
	int start_seq_sound(int sound, bool reset_vars = true);
	int query_param(int param);

public:
	IMuseInternal *_se;
	uint _vol_chan;

public:
	Player();
	virtual ~Player();

	int    addParameterFader(int param, int target, int time);
	void   clear();
	void   clearLoop();
	void   fixAfterLoad();
	Part * getActivePart(uint8 part);
	uint   getBeatIndex();
	int8   getDetune() { return _detune; }
	byte   getEffectiveVolume() { return _vol_eff; }
	int    getID() { return _id; }
	MidiDriver *getMidiDriver() { return _midi; }
	int    getParam(int param, byte chan);
	int8   getPan() { return _pan; }
	Part * getPart(uint8 part);
	byte   getPriority() { return _priority; }
	uint   getTicksPerBeat() { return TICKS_PER_BEAT; }
	int8   getTranspose() { return _transpose; }
	byte   getVolume() { return _volume; }
	bool   isActive() { return _active; }
	bool   isFadingOut();
	bool   isGM() { return _isGM; }
	bool   isMT32() { return _isMT32; }
	bool   jump(uint track, uint beat, uint tick);
	void   onTimer();
	void   removePart(Part *part);
	int    scan(uint totrack, uint tobeat, uint totick);
	int    save_or_load(Serializer *ser);
	int    setHook(byte cls, byte value, byte chan) { return _hook.set(cls, value, chan); }
	void   setDetune(int detune);
	bool   setLoop(uint count, uint tobeat, uint totick, uint frombeat, uint fromtick);
	void   setPan(int pan);
	void   setPriority(int pri);
	void   setSpeed(byte speed);
	int    setTranspose(byte relative, int b);
	int    setVolume(byte vol);
	bool   startSound(int sound, MidiDriver *midi);
	int    getMusicTimer();

public:
	// MidiDriver interface
	int open() { return 0; }
	void close() { }
	void send(uint32 b);
	const char *getErrorName(int error_code) { return "Unknown"; }
	void sysEx(byte *msg, uint16 length);
	void metaEvent(byte type, byte *data, uint16 length);
	void setTimerCallback(void *timer_param, void(*timer_proc)(void *)) { }
	uint32 getBaseTempo();
	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }
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
	void load_global_instrument(byte b);

	void set_transpose(int8 transpose);
	void setVolume(uint8 volume);
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

// WARNING: This is the internal variant of the IMUSE class.
// imuse.h contains a public version of the same class.
// the public version, only contains a set of methods.
class IMuseInternal {
	friend class Player;

protected:
	bool _old_adlib_instruments;
	bool _enable_multi_midi;
	MidiDriver *_midi_adlib;
	MidiDriver *_midi_native;

	byte **_base_sounds;
	
	SoundMixer *_mixer;

protected:
	bool _paused;
	bool _initialized;

	int _tempoFactor;

	int  _player_limit;    // Limits how many simultaneous music tracks are played
	bool _recycle_players; // Can we stop a player in order to start another one?

	uint _queue_end, _queue_pos, _queue_sound;
	byte _queue_adding;

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
	Part _parts[32];

	uint16 _active_notes[128];
	Instrument _global_adlib_instruments[32];
	CommandQueue _cmd_queue[64];
	DeferredCommand _deferredCommands[4];

protected:
	byte *findStartOfSound(int sound);
	bool isMT32(int sound);
	bool isGM(int sound);
	int get_queue_sound_status(int sound);
	void handle_marker(uint id, byte data);
	int get_channel_volume(uint a);
	void initMidiDriver(MidiDriver *midi);
	void initMT32(MidiDriver *midi);
	void init_players();
	void init_parts();
	void init_queue();

	void sequencer_timers(MidiDriver *midi);

	MidiDriver *getBestMidiDriver(int sound);
	Player *allocate_player(byte priority);
	Part *allocate_part(byte pri, MidiDriver *midi);

	int32 ImSetTrigger(int sound, int id, int a, int b, int c, int d, int e, int f, int g, int h);
	int32 ImClearTrigger(int sound, int id);
	int32 ImFireAllTriggers(int sound);

	void addDeferredCommand(int time, int a, int b, int c, int d, int e, int f);
	void handleDeferredCommands(MidiDriver *midi);

	int enqueue_command(int a, int b, int c, int d, int e, int f, int g);
	int enqueue_trigger(int sound, int marker);
	int query_queue(int param);
	Player *findActivePlayer(int id);

	int get_volchan_entry(uint a);
	int set_volchan_entry(uint a, uint b);
	int set_channel_volume(uint chan, uint vol);
	void update_volumes();
	void reset_tick();

	int set_volchan(int sound, int volchan);

	void fix_parts_after_load();
	void fix_players_after_load(Scumm *scumm);

	static int saveReference(void *me_ref, byte type, void *ref);
	static void *loadReference(void *me_ref, byte type, int ref);

	static void midiTimerCallback(void *data);

public:
	IMuseInternal();
	~IMuseInternal();

	int initialize(OSystem *syst, SoundMixer *mixer, MidiDriver *midi);
	void reallocateMidiChannels(MidiDriver *midi);
	void setGlobalAdlibInstrument(byte slot, byte *data);
	void copyGlobalAdlibInstrument(byte slot, Instrument *dest);

	// IMuse interface

	void on_timer(MidiDriver *midi);
	void pause(bool paused);
	int terminate();
	int save_or_load(Serializer *ser, Scumm *scumm);
	int set_music_volume(uint vol);
	int get_music_volume();
	int set_master_volume(uint vol);
	int get_master_volume();
	bool startSound(int sound);
	int stopSound(int sound);
	int stopAllSounds();
	int getSoundStatus(int sound, bool ignoreFadeouts = true);
	int getMusicTimer();
	int32 doCommand (int a, int b, int c, int d, int e, int f, int g, int h);
	int32 doCommand (int numargs, int args[]);
	int clear_queue();
	void setBase(byte **base);
	uint32 property(int prop, uint32 value);

	static IMuseInternal *create(OSystem *syst, SoundMixer *mixer, MidiDriver *midi);
};

#endif
