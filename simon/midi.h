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
class File;
struct MidiEvent;

class MidiPlayer {
public:
	MidiPlayer();

	void read_all_songs (File *in, uint music);
	void read_all_songs_old (File *in, uint music);
	void initialize();
	void shutdown();
	void play();
	void pause (bool b);
	int  get_volume();
	void set_volume (int volume);
	void set_driver (MidiDriver *md);

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

	MidiDriver *_midiDriver;
	uint _lastDelay;
	Song *_currentSong;
	Song _songs[8];
	byte _volumeTable[16]; // 0-127
	byte _masterVolume;    // 0-255
	bool _paused;

	void read_mthd(File *in, Song *s, bool old, uint music);

	void read_one_song(File *in, Song *s, uint music);

	static uint32 track_read_gamma(Track *t);
	static byte track_read_byte(Track *t);

	int fill (MidiEvent *me, int num_event);
	bool fill_helper (NoteRec *nr, MidiEvent *me);

	void reset_tracks();
	void read_next_note(Track *t, NoteRec *nr);

	void unload();

	static int on_fill(void *param, MidiEvent *ev, int num);

};

#endif
