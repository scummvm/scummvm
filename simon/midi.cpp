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


#include "stdafx.h"
#include "scummsys.h"
#include "system.h"
#ifndef macintosh
#include "../sound/mididrv.h"
#include "../sound/mixer.h"
#else
#include "mididrv.h"
#include "mixer.h"
#endif
#include "simon.h"

void MidiPlayer::read_from_file(void *dst, uint size) {
	if (fread(dst, size, 1, _input) != 1)
		error("Midi read error");
}

byte MidiPlayer::read_byte_from_file() {
	byte num;
	read_from_file(&num, 1);
	return num;
}

uint32 MidiPlayer::read_uint32_from_file() {
	uint32 num;
	read_from_file(&num, 4);
	return READ_BE_UINT32(&num);
}

uint16 MidiPlayer::read_uint16_from_file() {
	uint16 num;
	read_from_file(&num, 2);
	return READ_BE_UINT16(&num);
}

void MidiPlayer::read_all_songs(FILE *in) {
	uint i,num;
	
	_input = in;

	_midi_cur_song_ptr = _midi_songs;

	num = read_byte_from_file();

	for(i=0; i!=num; i++) {
		read_one_song(&_midi_songs[i]);
	}
}

void MidiPlayer::read_all_songs_old(FILE *in) {
	uint i,num;
	
	_input = in;
	_midi_cur_song_ptr = _midi_songs;

	num = 1;

	for(i=0; i!=num; i++) {
		read_one_song(&_midi_songs[i]);
	}
}

void MidiPlayer::read_mthd(Song *s, bool old) {
	Track *t;
	uint i;

	if (!old) {
		if (read_uint32_from_file() != 6)
			error("Invalid 'MThd' chunk size");
		s->midi_format = read_uint16_from_file();
		s->num_tracks = read_uint16_from_file();
		s->ppqn = read_uint16_from_file();
	} else {
		s->midi_format = 0;
		s->num_tracks = 1;
		s->ppqn = 0xc0;

		read_uint16_from_file();
		read_byte_from_file();
	}

	s->tracks = t = (Track*)calloc(s->num_tracks, sizeof(Track));
	if (t == NULL)
		error("Out of memory when allocating MIDI tracks");
	
	for(i=0; i!=s->num_tracks; i++, t++) {
		if (!old) {
			if (read_uint32_from_file() != 'MTrk')
				error("Midi track has no 'MTrk'");
	
			t->data_size = read_uint32_from_file();
		} else {
			uint32 pos = ftell(_input);
			fseek(_input, 0, SEEK_END);
			uint32 end = ftell(_input);
			fseek(_input, pos, SEEK_SET);
			t->data_size = end - pos;
		}

		t->data_ptr = (byte*)calloc(t->data_size,1);
		if (t->data_ptr == NULL)
			error("Out of memory when allocating MIDI track data");

		read_from_file(t->data_ptr, t->data_size);

		t->data_cur_size = t->data_size;
		t->data_cur_ptr = t->data_ptr;

		t->a = 0;
		t->last_cmd = 0;
		t->delay = 0;
		
		if (t->data_cur_size == 0) {
			t->a |= 1;
			continue;
		}
		
		t->delay = track_read_gamma(t);
	}
}

void MidiPlayer::read_one_song(Song *s) {
	_midi_var10 = 0;
	
	s->ppqn = 0;
	s->midi_format = 0;
	s->num_tracks = 0;
	s->tracks = NULL;

	uint32 id = read_uint32_from_file();

	switch(id) {
	case 'MThd':
		read_mthd(s, false);
		break;

	case 'GMF\x1':
		warning("Old style songs not properly supported yet");
		read_mthd(s, true);
		break;

	default:
		error("Midi song has no 'MThd'");
	}

}

uint32 MidiPlayer::track_read_gamma(Track *t) {
	uint32 sum;
	byte b;

	sum = 0;
	do {
		b = track_read_byte(t);
		sum = (sum<<7) | (b & 0x7F);
	} while (b & 0x80);

	return sum;	
}

byte MidiPlayer::track_read_byte(Track *t) {
	if (t->a & 1)
		error("Trying to read byte from MIDI stream when end reached");
	
	if (!--t->data_cur_size) {
		t->a|=1;
	}

	return *t->data_cur_ptr++;
}

void MidiPlayer::initialize() {
	int res;
	int i;

	for(i=0; i!=16; i++)
		_midi_volume_table[i] = 100;
	_midi_5 = 0;

	_md->property(MidiDriver::PROP_TIMEDIV, _midi_songs[0].ppqn);

	res = _md->open(MidiDriver::MO_STREAMING);
	if (res != 0)
		error("MidiPlayer::initializer, got %s", MidiDriver::get_error_name(res));
	
	_midi_var9 = true;
}

int MidiPlayer::fill(MidiEvent *me, int num_event) {
	uint32 best,j;
	Track *best_track,*t;
	bool did_reset;
	NoteRec midi_tmp_note_rec;
	int i = 0;

	did_reset = false;
	
	for(i=0; i!=num_event;) {
		best_track = NULL;
		best = 0xFFFFFFFF;

		/* Locate which track that's next */
		t = _midi_cur_song_ptr->tracks;
		for(j=_midi_cur_song_ptr->num_tracks; j; j--,t++) {
			if (!(t->a&1)) {
				if (t->delay < best) {
					best = t->delay;
					best_track = t;
				} 
			}
		}

		if (best_track == NULL) {
			/* reset tracks if song ended? */
			if (did_reset) {
				/* exit if song ended completely */
				return i;
			}
			did_reset = true;
			reset_tracks();
			continue;
		}

		read_next_note(best_track, &midi_tmp_note_rec);
		if (fill_helper(&midi_tmp_note_rec, me + i))
			i++;

		if (midi_tmp_note_rec.sysex_data)	free(midi_tmp_note_rec.sysex_data);
	}

	return i;
}

int MidiPlayer::on_fill(void *param, MidiEvent *ev, int num) {
	MidiPlayer *mp = (MidiPlayer*)param;
	return mp->fill(ev, num);
}

bool MidiPlayer::fill_helper(NoteRec *nr, MidiEvent *me) {
	uint b;

	b = nr->delay - _midi_var10;
	_midi_var10 = nr->delay;
	
	if (nr->cmd<0xF0) {
		me->delta = b;
		me->event = nr->cmd | (nr->param_1<<8) | (nr->param_2<<16);
		
		if ((nr->cmd&0xF0) == 0xB0 && nr->param_1 == 7) {
			_midi_volume_table[nr->cmd&0xF] = nr->param_2;
			nr->param_1 = 0x76;
			me->event = nr->cmd | (nr->param_1<<8) | (nr->param_2<<16)/* | MEVT_F_CALLBACK*/;
		}
		return true;

	}
	
	if (nr->cmd==0xF0 || nr->cmd==0xF7 || nr->param_1 != 0x51) {
		return false;
	}
	
	_midi_tempo = nr->sysex_data[2] | (nr->sysex_data[1]<<8) | (nr->sysex_data[0]<<16);
	_midi_var8 = (_midi_cur_song_ptr->ppqn*60000) / _midi_tempo;

	me->delta = b;
	me->event = (MidiDriver::ME_TEMPO << 24) | _midi_tempo;

	return true;
}

void MidiPlayer::reset_tracks() {
	Track *t;
	uint i;

	_midi_var10 = 0;

	for(i=0,t=_midi_cur_song_ptr->tracks; i!=_midi_cur_song_ptr->num_tracks; i++,t++) {
		t->data_cur_size = t->data_size;
		t->data_cur_ptr = t->data_ptr;
		t->a = 0;
		t->last_cmd = 0;
		t->delay = 0;
		if (t->data_cur_size==0) {
			t->a|=1;
		} else {
			t->delay = track_read_gamma(t);
		}
	}
}

void MidiPlayer::read_next_note(Track *t, NoteRec *nr) {
	byte cmd_byte;
	uint i;

	nr->delay = 0;
	nr->cmd = 0;
	nr->param_1 = 0;
	nr->param_2 = 0;
	nr->cmd_length = 0;
	nr->sysex_data = NULL;
	
	if(t->a&1 || t->data_cur_size==0)
		error("read next note when track ended");

	/* read next midi byte, but skip any pitch bends. */
	for(;;) {
		cmd_byte = track_read_byte(t);
		if ((cmd_byte&0xF0) != 0xE0)
			break;

		track_read_byte(t);
		track_read_byte(t);
	} 
	
	if (!(cmd_byte & 0x80)) {
		/* running status? */
		if (t->last_cmd==0)
			error("Last cmd = 0");
		nr->cmd = t->last_cmd;
		nr->param_1 = cmd_byte;
		cmd_byte = nr->cmd&0xF0;
		nr->cmd_length = 2;
		if (cmd_byte!=0xC0 && cmd_byte!=0xD0) {
			nr->param_2 = track_read_byte(t);
			nr->cmd_length++;
		}
	} else if ((cmd_byte&0xF0)!=0xF0) {
		nr->cmd = cmd_byte;
		t->last_cmd = cmd_byte;
		cmd_byte &= 0xF0;
		nr->cmd_length = (cmd_byte==0xC0 || cmd_byte==0xD0) ? 2 : 3;
		
		if (t->data_cur_size < nr->cmd_length-1) {
			error("read_next_note: end of stream");
		}
		
		nr->param_1 = track_read_byte(t);
		if (nr->cmd_length==3)
			nr->param_2 = track_read_byte(t);

	} else if (cmd_byte==0xF0 || cmd_byte==0xF7) {
		nr->cmd = cmd_byte;
		nr->cmd_length = track_read_gamma(t);
		if (t->data_cur_size < nr->cmd_length)
			error("read_next_note: end of stream 2");
		nr->sysex_data = (byte*)malloc(nr->cmd_length);
		if (nr->sysex_data==NULL)
			error("read_next_note: out of memory");
		for(i=0; i!=nr->cmd_length; i++)
			nr->sysex_data[i] = track_read_byte(t);
	} else if (cmd_byte==0xFF) {
		
		nr->cmd = cmd_byte;
		nr->param_1 = track_read_byte(t);
		nr->cmd_length = track_read_gamma(t);
		if (nr->cmd_length) {
			if (t->data_cur_size < nr->cmd_length)
				error("read_next_note: end of stream 3");
			nr->sysex_data = (byte*)malloc(nr->cmd_length);
			if (nr->sysex_data==NULL)
				error("read_next_note: out of memory");
			for(i=0; i!=nr->cmd_length; i++)
				nr->sysex_data[i] = track_read_byte(t);
		}
		if (nr->param_1==0x2F)
			t->a|=1;
	} else {
		error("Invalid sysex cmd");
	}

	nr->delay = t->delay;
	if (!(t->a&1)) {
		t->delay += track_read_gamma(t);
	}
}

void MidiPlayer::shutdown() {
	_md->close();
	unload();
}

void MidiPlayer::unload() {
	uint i,j;
	Song *s;
	Track *t;
	for(i=0,s=_midi_songs; i!=8; i++,s++) {
		if (s->tracks) {
			for(j=0,t=s->tracks; j!=s->num_tracks;j++,t++) {
				if (t->data_ptr)
					free(t->data_ptr);
			}
			free(s->tracks);
			s->tracks = NULL;
		}
	}
}

void MidiPlayer::play() {
	_md->pause(false);
}


void MidiPlayer::set_driver(MidiDriver *md) {
	_md = md;
	md->set_stream_callback(this, on_fill);
}


