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
#include "common/system.h"
#include "common/file.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"
#include "simon/simon.h"

// FIXME: This is a horrible place to put this, but for now....
#include "sound/midistreamer.cpp"

void MidiPlayer::read_all_songs(File *in, uint music)
{
	uint i, num;

	_currentSong = _songs;

	num = in->readByte();

	for (i = 0; i != num; i++) {
		read_one_song(in, &_songs[i], music);
	}
}

void MidiPlayer::read_all_songs_old(File *in, uint music)
{
	uint i, num;

	_currentSong = _songs;

	num = 1;

	for (i = 0; i != num; i++) {
		read_one_song(in, &_songs[i], music);
	}
}

void MidiPlayer::read_mthd(File *in, Song *s, bool old, uint music)
{
	Track *t;
	uint i;

	if (!old) {
		if (in->readUint32BE() != 6)
			error("Invalid 'MThd' chunk size");
		s->midi_format = in->readUint16BE();
		s->num_tracks = in->readUint16BE();
		s->ppqn = in->readUint16BE();
	} else {
		s->midi_format = 0;
		s->num_tracks = 1;
		s->ppqn = 0xc0;

		in->readUint16BE();
		in->readByte();
	}

	s->tracks = t = (Track *)calloc(s->num_tracks, sizeof(Track));
	if (t == NULL)
		error("Out of memory when allocating MIDI tracks");

	for (i = 0; i != s->num_tracks; i++, t++) {
		if (!old) {
			if (in->readUint32BE() != 'MTrk')
				error("Midi track has no 'MTrk'");

			t->data_size = in->readUint32BE();
		} else {
			//FIXME We currently don't know how to find out music track size for GMF midi format
			// So we use music files sizes minues header for now to allow looping
			int music_data_size[] = {8900, 12166, 2848, 3442, 4034, 4508, 7064, 9730, 6014, 4742, 3138,
						6570, 5384, 8909, 6457, 16321, 2742, 8968, 4804, 8442, 7717,
						9444, 5800, 1381, 5660, 6684, 2456, 4744, 2455, 1177, 1232,
						17256, 5103, 8794, 4884, 16};
			t->data_size = music_data_size[music] - 8;
		}

		t->data_ptr = (byte *)calloc(t->data_size, 1);
		if (t->data_ptr == NULL)
			error("Out of memory when allocating MIDI track data (%d)", t->data_size);

		in->read(t->data_ptr, t->data_size);

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

void MidiPlayer::read_one_song(File *in, Song *s, uint music)
{
	_lastDelay = 0;

	s->ppqn = 0;
	s->midi_format = 0;
	s->num_tracks = 0;
	s->tracks = NULL;

	uint32 id = in->readUint32BE();

	switch (id) {
	case 'MThd':
		read_mthd(in, s, false, music);
		break;

	case 'GMF\x1':
		warning("Old style songs not properly supported yet");
		read_mthd(in, s, true, music);
		break;

	default:
		error("Midi song has no 'MThd'");
	}

}

uint32 MidiPlayer::track_read_gamma(Track *t)
{
	uint32 sum;
	byte b;

	sum = 0;
	do {
		b = track_read_byte(t);
		sum = (sum << 7) | (b & 0x7F);
	} while (b & 0x80);

	return sum;
}

byte MidiPlayer::track_read_byte(Track *t)
{
	if (t->a & 1)
		error("Trying to read byte from MIDI stream when end reached");

	if (!--t->data_cur_size) {
		t->a |= 1;
	}

	return *t->data_cur_ptr++;
}

void MidiPlayer::initialize()
{
	int res;
	int i;

	for (i = 0; i != 16; i++)
		_volumeTable[i] = 100;

	_midiDriver->property(MidiDriver::PROP_TIMEDIV, _songs[0].ppqn);

	res = _midiDriver->open(MidiDriver::MO_STREAMING);
	if (res == MidiDriver::MERR_STREAMING_NOT_AVAILABLE) {
		// No direct streaming, slap a front-end on.
		_midiDriver = new MidiStreamer (_midiDriver);
		_midiDriver->property (MidiDriver::PROP_TIMEDIV, _songs[0].ppqn);
		_midiDriver->set_stream_callback (this, on_fill);
		res = _midiDriver->open (MidiDriver::MO_STREAMING);
	}

	if (res != 0)
		error("MidiPlayer::initializer, got %s", MidiDriver::get_error_name(res));
}

int MidiPlayer::fill(MidiEvent *me, int num_event)
{
	uint32 best, j;
	Track *best_track, *t;
	bool did_reset;
	NoteRec midi_tmp_note_rec;
	int i = 0;

	did_reset = false;

	for (i = 0; i != num_event;) {
		best_track = NULL;
		best = 0xFFFFFFFF;

		/* Locate which track that's next */
		t = _currentSong->tracks;
		for (j = _currentSong->num_tracks; j; j--, t++) {
			if (!(t->a & 1)) {
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

		if (midi_tmp_note_rec.sysex_data)
			free(midi_tmp_note_rec.sysex_data);
	}

	return i;
}

int MidiPlayer::on_fill(void *param, MidiEvent *ev, int num)
{
	MidiPlayer *mp = (MidiPlayer *) param;
	return mp->fill(ev, num);
}

bool MidiPlayer::fill_helper(NoteRec *nr, MidiEvent *me)
{
	uint b;

	b = nr->delay - _lastDelay;
	_lastDelay = nr->delay;

	if (nr->cmd < 0xF0) {
		me->delta = b;
		me->event = nr->cmd | (nr->param_1 << 8) | (nr->param_2 << 16);

		if ((nr->cmd & 0xF0) == 0xB0 && nr->param_1 == 7) {
			_volumeTable[nr->cmd & 0xF] = nr->param_2;
			nr->param_1 = 0x76;
			me->event = nr->cmd | (nr->param_1 << 8) | (nr->param_2 << 16) /* | MEVT_F_CALLBACK */ ;
		}
		return true;

	}

	if (nr->cmd == 0xF0 || nr->cmd == 0xF7 || nr->param_1 != 0x51) {
		return false;
	}

	int tempo = nr->sysex_data[2] | (nr->sysex_data[1] << 8) | (nr->sysex_data[0] << 16);
//	_midi_var8 = (_currentSong->ppqn * 60000) / tempo;

	me->delta = b;
	me->event = (MidiDriver::ME_TEMPO << 24) | tempo;

	return true;
}

void MidiPlayer::reset_tracks()
{
	Track *t;
	uint i;

	_lastDelay = 0;

	for (i = 0, t = _currentSong->tracks; i != _currentSong->num_tracks; i++, t++) {
		t->data_cur_size = t->data_size;
		t->data_cur_ptr = t->data_ptr;
		t->a = 0;
		t->last_cmd = 0;
		t->delay = 0;
		if (t->data_cur_size == 0) {
			t->a |= 1;
		} else {
			t->delay = track_read_gamma(t);
		}
	}
}

void MidiPlayer::read_next_note(Track *t, NoteRec *nr)
{
	byte cmd_byte;
	uint i;

	nr->delay = 0;
	nr->cmd = 0;
	nr->param_1 = 0;
	nr->param_2 = 0;
	nr->cmd_length = 0;
	nr->sysex_data = NULL;

	if (t->a & 1 || t->data_cur_size == 0)
		error("read next note when track ended");

	/* read next midi byte, but skip any pitch bends. */
	for (;;) {
		cmd_byte = track_read_byte(t);
		if ((cmd_byte & 0xF0) != 0xE0)
			break;

		track_read_byte(t);
		track_read_byte(t);
	}

	if (!(cmd_byte & 0x80)) {
		/* running status? */
		if (t->last_cmd == 0)
			error("Last cmd = 0");
		nr->cmd = t->last_cmd;
		nr->param_1 = cmd_byte;
		cmd_byte = nr->cmd & 0xF0;
		nr->cmd_length = 2;
		if (cmd_byte != 0xC0 && cmd_byte != 0xD0) {
			nr->param_2 = track_read_byte(t);
			nr->cmd_length++;
		}
	} else if ((cmd_byte & 0xF0) != 0xF0) {
		nr->cmd = cmd_byte;
		t->last_cmd = cmd_byte;
		cmd_byte &= 0xF0;
		nr->cmd_length = (cmd_byte == 0xC0 || cmd_byte == 0xD0) ? 2 : 3;

		if (t->data_cur_size < nr->cmd_length - 1) {
			error("read_next_note: end of stream");
		}

		nr->param_1 = track_read_byte(t);
		if (nr->cmd_length == 3)
			nr->param_2 = track_read_byte(t);

	} else if (cmd_byte == 0xF0 || cmd_byte == 0xF7) {
		nr->cmd = cmd_byte;
		nr->cmd_length = track_read_gamma(t);
		if (t->data_cur_size < nr->cmd_length)
			error("read_next_note: end of stream 2");
		nr->sysex_data = (byte *)malloc(nr->cmd_length);
		if (nr->sysex_data == NULL)
			error("read_next_note: out of memory");
		for (i = 0; i != nr->cmd_length; i++)
			nr->sysex_data[i] = track_read_byte(t);
	} else if (cmd_byte == 0xFF) {

		nr->cmd = cmd_byte;
		nr->param_1 = track_read_byte(t);
		nr->cmd_length = track_read_gamma(t);
		if (nr->cmd_length) {
			if (t->data_cur_size < nr->cmd_length)
				error("read_next_note: end of stream 3");
			nr->sysex_data = (byte *)malloc(nr->cmd_length);
			if (nr->sysex_data == NULL)
				error("read_next_note: out of memory");
			for (i = 0; i != nr->cmd_length; i++)
				nr->sysex_data[i] = track_read_byte(t);
		}
		if (nr->param_1 == 0x2F)
			t->a |= 1;
	} else {
		error("Invalid sysex cmd");
	}

	nr->delay = t->delay;
	if (!(t->a & 1)) {
		t->delay += track_read_gamma(t);
	}
}

void MidiPlayer::shutdown()
{
	_midiDriver->close();
	unload();
}

void MidiPlayer::unload()
{
	uint i, j;
	Song *s;
	Track *t;
	for (i = 0, s = _songs; i != 8; i++, s++) {
		if (s->tracks) {
			for (j = 0, t = s->tracks; j != s->num_tracks; j++, t++) {
				if (t->data_ptr)
					free(t->data_ptr);
			}
			free(s->tracks);
			s->tracks = NULL;
		}
	}
}

void MidiPlayer::play()
{
	_midiDriver->pause(false);
}

void MidiPlayer::pause(bool b)
{
	_midiDriver->pause(b);
}

uint MidiPlayer::get_volume()
{
	// TODO: implement me 
	return 0;
}

void MidiPlayer::set_volume(uint volume)
{
	// TODO: implement me 
}

void MidiPlayer::set_driver(MidiDriver *md)
{
	_midiDriver = md;
	_midiDriver->set_stream_callback(this, on_fill);
}
