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
#include "simon.h"

/* This is win32 only code at the moment */
#ifdef WIN32

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
	return swap32(num);
}

uint16 MidiPlayer::read_uint16_from_file() {
	uint16 num;
	read_from_file(&num, 2);
	return swap16(num);
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
#ifdef WIN32
	uint i;
	MyMidiHdr *mmh;
	MIDIPROPTIMEDIV mptd;
	uint x;

	if (_midi_stream_handle == NULL) {
		_midi_device_id = 0;
		check_error(midiStreamOpen(&_midi_stream_handle, &_midi_device_id, 1, 
			(uint32)midi_callback, (uint32)this, CALLBACK_FUNCTION));
	}

	for(i=0,mmh=_prepared_headers; i!=NumPreparedHeaders; i++,mmh++) {
		mmh->hdr.dwBufferLength = 0x400;
		mmh->hdr.lpData = (LPSTR)calloc(0x400,1);
		if (mmh->hdr.lpData == NULL)
			error("Out of memory for prepared header");
	}

	for(i=0; i!=16; i++)
		_midi_volume_table[i] = 100;
	
	mptd.cbStruct = sizeof(mptd);
	mptd.dwTimeDiv = _midi_songs[0].ppqn;

	check_error(midiStreamProperty(_midi_stream_handle, (byte*)&mptd, 
		MIDIPROP_SET | MIDIPROP_TIMEDIV));

	_midi_5 = 0;
	x = 1;
	
	for(i=0,mmh=_prepared_headers; i!=NumPreparedHeaders; i++,mmh++) {
		
		fill(x, mmh);

		mmh->hdr.dwBytesRecorded = mmh->b;
		
		if (!_midi_var9) {
			check_error(midiOutPrepareHeader((HMIDIOUT)_midi_stream_handle, 
				&mmh->hdr, sizeof(mmh->hdr)));
		}
		
		check_error(midiStreamOut(_midi_stream_handle, 
			&mmh->hdr, sizeof(mmh->hdr)));

		x = 0;
	}

	_midi_var9 = true;
#endif
}

int MidiPlayer::fill(uint x, MyMidiHdr *mmh) {
	uint32 best,i;
	Track *best_track,*t;
	bool did_reset;

	mmh->a = 0;
	mmh->size = 0x200;
	mmh->c = 0;
	mmh->d = 0;
	mmh->b = 0;

	did_reset = false;
	
	for(;;) {
		if (mmh->size - mmh->b < 12)
			return 1;
		
		best_track = NULL;
		best = 0xFFFFFFFF;

		/* Locate which track that's next */
		t = _midi_cur_song_ptr->tracks;
		for(i=0; i!=_midi_cur_song_ptr->num_tracks; i++,t++) {
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
				return 0;
			}
			did_reset = true;
			reset_tracks();
			continue;
		}

		read_next_note(best_track, &_midi_tmp_note_rec);
//		if ((_midi_tmp_note_rec.cmd&0xF)==3) {
//			printf("%4d: %2X %d\n", _midi_tmp_note_rec.delay,
//				_midi_tmp_note_rec.cmd, _midi_tmp_note_rec.param_1);
			fill_helper(&_midi_tmp_note_rec, mmh);
//		}

		if (_midi_num_sysex) {
			free(_midi_tmp_note_rec.sysex_data);
			_midi_num_sysex--;
		}
	}
}

int MidiPlayer::fill_helper(NoteRec *nr, MyMidiHdr *mmh) {
#ifdef WIN32
	byte *lpdata;
	uint b;

	lpdata = (byte*)mmh->hdr.lpData + mmh->a + mmh->b;

	b = nr->delay - _midi_var10;
	_midi_var10 = nr->delay;
	
	if (nr->cmd<0xF0) {
		((MIDIEVENT*)lpdata)->dwDeltaTime = b;
		((MIDIEVENT*)lpdata)->dwStreamID = 0;
		((MIDIEVENT*)lpdata)->dwEvent = nr->cmd | (nr->param_1<<8) | (nr->param_2<<16);
		
		if ((nr->cmd&0xF0) == 0xB0 && nr->param_1 == 7) {
			_midi_volume_table[nr->cmd&0xF] = nr->param_2;

			nr->param_1 = 0x76;

			((MIDIEVENT*)lpdata)->dwEvent = nr->cmd | 
				(nr->param_1<<8) | (nr->param_2<<16) | MEVT_F_CALLBACK;
		}

		mmh->b += 12;
	} else if (nr->cmd==0xF0 || nr->cmd==0xF7) {
	} else if (nr->param_1 != 0x51) {
		return -105;
	} else {
		((MIDIEVENT*)lpdata)->dwDeltaTime = b;
		((MIDIEVENT*)lpdata)->dwStreamID = 0;

		_midi_tempo = nr->sysex_data[2] | 
			(nr->sysex_data[1]<<8) | (nr->sysex_data[0]<<16);
		
		((MIDIEVENT*)lpdata)->dwEvent = _midi_tempo | (MEVT_TEMPO<<24);

		_midi_var8 = (_midi_cur_song_ptr->ppqn*60000) / _midi_tempo;
		
		if(_midi_num_sysex) {
			free(nr->sysex_data);
			_midi_num_sysex--;
		}
	
		mmh->b += 12;
	}
#endif
	return 0;
}

#if 0
int MidiPlayer::fill(uint x, MyMidiHdr *mmh) {
	Track *t;
	uint i;
	uint32 best;
	Track *best_track;
	int result;

	mmh->b = 0;

	if (x&1) {
		NoteRec *nr = &_midi_tmp_note_rec;
		
		_midi_var1 = 0;
		nr->delay = 0;
		nr->big_cmd = 0;
		nr->cmd_length = 0;
		nr->sysex_data = NULL;

		_midi_track_ptr = NULL;
		_midi_tick_track_ptr = NULL;
	}

	if (_midi_var1 & 1) {
		if (_midi_var2 == 0)
			error("MidiPlayer::fill: Return -103");
		reset_tracks();
		_midi_var1 = 0;
	} else if (_midi_var1 & 2) {
		error("MidiPlayer::fill: Return -102");
	} else if (_midi_var1 & 4) {
		_midi_var1 ^= 4;
		
		if (_midi_tmp_note_rec.cmd==0xFF && _midi_tmp_note_rec.param_1==0x2F) {
			if (_midi_num_sysex) {
				free(_midi_tmp_note_rec.sysex_data);
				_midi_num_sysex--;
			}
		} else {
			result = fill_helper(&_midi_tmp_note_rec, mmh);
			if (result==-104) {
				_midi_var1 |= 4;
				return 0;
			}
		}
	}

	/* find_next_track_to_run */
	for(;;) {
		best_track = NULL;
		best = 0xFFFFFFFF;

		/* Locate which track that's next */
		t = _midi_cur_song_ptr->tracks;
		for(i=0; i!=_midi_cur_song_ptr->num_tracks; i++,t++) {
			if (!(t->a&1)) {
				if (t->delay < best) {
					best = t->delay;
					best_track = t;
				}
			}
		}

		if (best_track == NULL) {
			_midi_var1 |= 1;
			return 0;
		}

		read_next_note(best_track, &_midi_tmp_note_rec);

		if (_midi_tmp_note_rec.cmd==0xFF && _midi_tmp_note_rec.param_1==0x2F) {
			if (_midi_num_sysex) {
				free(_midi_tmp_note_rec.sysex_data);
				_midi_num_sysex--;
			}
			continue;
		}

		result = fill_helper(&_midi_tmp_note_rec, mmh);
		if (result==-104) {
			_midi_var1 |= 4;
			return 0;
		}
	}
}

int MidiPlayer::fill_helper(NoteRec *nr, MyMidiHdr *mmh) {
	byte *lpdata;
	uint a,b;

	lpdata = (byte*)mmh->hdr.lpData + mmh->a + mmh->b;

	if (mmh->b == 0) {
		mmh->c = _midi_var10;
	}

	if (_midi_var10 - mmh->c > _midi_var8) {
		if (mmh->d!=0) {
			mmh->d = 0;
			return -104;
		}
		mmh->d = 1;
	}

	a = _midi_var10;
	b = nr->delay - _midi_var10;
	_midi_var10 = nr->delay;
	
	if (nr->cmd<0xF0) {
		if (mmh->size - mmh->b < 12)
			return -104;

		((MIDIEVENT*)lpdata)->dwDeltaTime = b;
		((MIDIEVENT*)lpdata)->dwStreamID = 0;
		((MIDIEVENT*)lpdata)->dwEvent = nr->cmd | (nr->param_1<<8) | (nr->param_2<<16);
		
		if ((nr->cmd&0xF0) == 0xB0 && nr->param_1 == 7) {
			_midi_volume_table[nr->cmd&0xF] = nr->param_2;

			nr->param_1 = 0x76;

			((MIDIEVENT*)lpdata)->dwEvent = nr->cmd | 
				(nr->param_1<<8) | (nr->param_2<<16) | MEVT_F_CALLBACK;
		}

		mmh->b += 12;
	} else if (nr->cmd==0xF0 || nr->cmd==0xF7) {
		if(_midi_num_sysex) {
			free(nr->sysex_data);
			_midi_num_sysex--;
		}
	} else if (nr->param_1 != 0x51) {
		if(_midi_num_sysex) {
			free(nr->sysex_data);
			_midi_num_sysex--;
		}

		return -105;
	} else if (mmh->size - mmh->b < 12) {
		if(_midi_num_sysex) {
			free(nr->sysex_data);
			_midi_num_sysex--;
		}
		return -104;
	} else {
		((MIDIEVENT*)lpdata)->dwDeltaTime = b;
		((MIDIEVENT*)lpdata)->dwStreamID = 0;

		_midi_tempo = nr->sysex_data[2] | 
			(nr->sysex_data[1]<<8) | (nr->sysex_data[0]<<16);
		
		((MIDIEVENT*)lpdata)->dwEvent = _midi_tempo | (MEVT_TEMPO<<24);

		_midi_var8 = (_midi_cur_song_ptr->ppqn*60000) / _midi_tempo;
		
		if(_midi_num_sysex) {
			free(nr->sysex_data);
			_midi_num_sysex--;
		}
	
		mmh->b += 12;
	}

	return 0;
}
#endif

void MidiPlayer::add_finished_hdrs() {
	uint i;
	MyMidiHdr *mmh = _prepared_headers;

	for(i=0; i!=NumPreparedHeaders; i++,mmh++) {
		if (!(mmh->hdr.dwFlags & MHDR_INQUEUE)) {
			fill(0, mmh);
			if (mmh->b == 0)
				break;
			mmh->hdr.dwBytesRecorded = mmh->b;
			check_error(midiStreamOut(_midi_stream_handle, &mmh->hdr, sizeof(mmh->hdr)));
		}
	}
}

void CALLBACK MidiPlayer::midi_callback(HMIDIOUT hmo, UINT wMsg,
	DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {

	switch(wMsg) {
	case MM_MOM_DONE:{
		MidiPlayer *mp = ((MidiPlayer*)dwInstance);
		if (!mp->_shutting_down)
			mp->add_finished_hdrs();
		break;
		}
	}
}

void MidiPlayer::check_error(MMRESULT result) {
	char buf[200];
	if (result != MMSYSERR_NOERROR) {
		midiOutGetErrorText(result, buf, 200);
		error("MM System Error '%s'", buf);
	}
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
	nr->big_cmd = 0;
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
		_midi_num_sysex++;
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
			_midi_num_sysex++;
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
	
	if (_midi_stream_handle != NULL) {
		_shutting_down = true;
		
		check_error(midiStreamStop(_midi_stream_handle));
		check_error(midiOutReset((HMIDIOUT)_midi_stream_handle));

		unload();
		unprepare();

		check_error(midiStreamClose(_midi_stream_handle));
		_midi_stream_handle = NULL;

		_shutting_down = false;
	}
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

void MidiPlayer::unprepare() {
	uint i;
	MyMidiHdr *mmh = _prepared_headers;

	for(i=0; i!=NumPreparedHeaders; i++,mmh++) {
		check_error(midiOutUnprepareHeader(
			(HMIDIOUT)_midi_stream_handle, &mmh->hdr, sizeof(mmh->hdr)));
		free(mmh->hdr.lpData);
		mmh->hdr.lpData = NULL;
	}

	_midi_var9 = false;
}

void MidiPlayer::play() {
	check_error(midiStreamRestart(_midi_stream_handle));
}

#else

/* Dummy midiplayer for unix */
void MidiPlayer::shutdown() {}
void MidiPlayer::read_all_songs(FILE *in) {}
void MidiPlayer::read_all_songs_old(FILE *in) {}
void MidiPlayer::initialize() {}
void MidiPlayer::play() {}

#endif