/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * $Header$
 *
 */

#ifndef SIMON_MIDI_H
#define SIMON_MIDI_H

class MidiDriver;
struct MidiEvent;

class MidiPlayer {
public:
	void read_all_songs(FILE *in);
	void read_all_songs_old(FILE *in);
	void initialize();
	void shutdown();
	void play();
	void set_driver(MidiDriver *md);

private:
	struct Track {
		uint32 a;
		uint32 data_size;
		uint32 data_cur_size;
		byte *data_ptr;
		byte *data_cur_ptr;
		uint32 delay;
		byte last_cmd;
	};

	struct Song {
		uint ppqn;
		uint midi_format;
		uint num_tracks;
		Track *tracks;
	};

	struct NoteRec {
		uint32 delay;
		byte cmd;
		byte param_1;
		byte param_2;
		uint cmd_length;
		byte *sysex_data;
	};

	MidiDriver *_md;

	FILE *_input;

	uint _midi_var10, _midi_5;
	bool _midi_var9;
	byte _midi_var1;
	bool _shutting_down;
	uint _midi_var8;

	uint _midi_var11;

	uint32 _midi_tempo;

	Track *_midi_tick_track_ptr;
	Track *_midi_track_ptr;
	int16 _midi_song_id;
	int16 _midi_song_id_2;
	int16 _midi_var2;

	Song *_midi_cur_song_ptr;

	uint32 _midi_volume_table[16];

	Song _midi_songs[8];

	void read_mthd(Song *s, bool old);

	void read_from_file(void *dst, uint size);
	void read_one_song(Song *s);
	byte read_byte_from_file();
	uint32 read_uint32_from_file();
	uint16 read_uint16_from_file();

	static uint32 track_read_gamma(Track *t);
	static byte track_read_byte(Track *t);

	int fill(MidiEvent *me, int num_event);
	bool fill_helper(NoteRec *nr, MidiEvent *me);

	void reset_tracks();
	void read_next_note(Track *t, NoteRec *nr);

	void unload();

	static int on_fill(void *param, MidiEvent *ev, int num);

};

#endif
