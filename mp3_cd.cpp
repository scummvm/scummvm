/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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
#include "scumm.h"

#ifdef COMPRESSED_SOUND_FILE__
#include "mp3_cd.h"
#include <SDL.h>

int _current_mp3_cd_track;
struct xing _vbr_header;
struct mad_header _mad_header;
long _mp3_size;
FILE *_mp3_track;
MixerChannel *_mc;

// From xing.c in MAD

int xing_parse(struct xing *xing, struct mad_bitptr ptr, unsigned int bitlen)
{

	xing->flags = 0;

	if (bitlen < 64 || mad_bit_read(&ptr, 32) != XING_MAGIC)
		goto fail;

	xing->flags = mad_bit_read(&ptr, 32);
	bitlen -= 64;

	if (xing->flags & XING_FRAMES) {
		if (bitlen < 32)
			goto fail;

		xing->frames = mad_bit_read(&ptr, 32);
		bitlen -= 32;
	}

	if (xing->flags & XING_BYTES) {
		if (bitlen < 32)
			goto fail;

		xing->bytes = mad_bit_read(&ptr, 32);
		bitlen -= 32;
	}

	if (xing->flags & XING_TOC) {
		int i;

		if (bitlen < 800)
			goto fail;

		for (i = 0; i < 100; ++i)
			xing->toc[i] = (unsigned char)mad_bit_read(&ptr, 8);

		bitlen -= 800;
	}

	if (xing->flags & XING_SCALE) {
		if (bitlen < 32)
			goto fail;

		xing->scale = mad_bit_read(&ptr, 32);
		bitlen -= 32;
	}

	return 0;

fail:
	xing->flags = 0;
	return -1;
}

// Borrowed from Winamp plugin in_mad.c

bool parse_xing_vbr_tag()
{
	struct mad_stream stream;
	struct mad_frame frame;
	unsigned char buffer[8192];
	unsigned int buflen = 0;
	int count = 0, result = 0;

	_vbr_header.flags = 0;

	mad_stream_init(&stream);
	mad_frame_init(&frame);

	fseek(_mp3_track, 0, SEEK_SET);

	while (1) {
		if (buflen < sizeof(buffer)) {
			uint16 bytes;

			bytes = fread(buffer + buflen, 1, sizeof(buffer) - buflen, _mp3_track);
			if (bytes <= 0) {
				if (bytes == -1)
					result = -1;
				break;
			}

			buflen += bytes;
		}

		mad_stream_buffer(&stream, buffer, buflen);

		while (1) {
			if (mad_frame_decode(&frame, &stream) == -1) {
				if (!MAD_RECOVERABLE(stream.error))
					break;

				if (stream.error != MAD_ERROR_BADCRC)
					continue;
			}

			if (count++ ||
					xing_parse(&_vbr_header, stream.anc_ptr, stream.anc_bitlen)
					== -1)
				break;
		}

		if (count || stream.error != MAD_ERROR_BUFLEN)
			break;

		memmove(buffer, stream.next_frame,
						buflen = &buffer[buflen] - stream.next_frame);
	}

	if (count)
		memcpy(&_mad_header, &frame.header, sizeof(mad_header));
	else
		result = -1;

	mad_frame_finish(&frame);
	mad_stream_finish(&stream);

	return (result != -1);
}

uint32 calc_cd_file_offset(int start_frame)
{
	long offset;

	if (!_vbr_header.flags) {
		float frame_size;
		//mad_timer_t timer;  - recode with timer

		/* Constant bit rate - perhaps not fully accurate */
		frame_size = 144 * _mad_header.bitrate / _mad_header.samplerate;
		offset = (long)((float)start_frame / (float)CD_FPS * 1000 /
										(float)((float)1152 / (float)_mad_header.samplerate *
														1000) * (float)(frame_size + 0.5));
	} else {
		/* DOES NOT WORK AT THE MOMENT */
		/* see Xing SDK */
		long a;
		float fa, fb, fx;
		float percent = (float)start_frame / (float)CD_FPS * 1000 /
			((float)((float)1152 / (float)_mad_header.samplerate * 1000) *
			 _vbr_header.frames) * 100;

		if (percent < 0.0f)
			percent = 0.0f;
		if (percent > 100.0f)
			percent = 100.0f;

		a = (int)percent;
		if (a > 99)
			a = 99;
		fa = _vbr_header.toc[a];
		if (a < 99) {
			fb = _vbr_header.toc[a + 1];
		} else {
			fb = 256.0f;
		}

		fx = fa + (fb - fa) * (percent - a);

		offset = (int)((1.0f / 256.0f) * fx * _vbr_header.bytes);

	}

	return offset;
}

bool mp3_cd_play(Scumm *s, int track, int num_loops, int start_frame,
								 int end_frame)
{

	// See if we are already playing this track, else try to open it

	if (_current_mp3_cd_track != track) {
		char track_name[1024];

		sprintf(track_name, "%strack%d.mp3", s->_gameDataPath, track);
		_mp3_track = fopen(track_name, "rb");
		if (!_mp3_track) {
			warning("No CD and track %d not available in mp3 format", track);
			return false;
		}

		if (!parse_xing_vbr_tag()) {
			warning("Error parsing file header - ignoring file", track);
			fclose(_mp3_track);
			return false;
		}

		if (_vbr_header.flags) {
			if (!((_vbr_header.flags & XING_TOC) &&
						(_vbr_header.flags & XING_BYTES) &&
						(_vbr_header.flags & XING_FRAMES)
					)) {
				warning("Missing required part of VBR header - ignoring file");
				fclose(_mp3_track);
				_vbr_header.flags = 0;
				return false;
			}
		}
		// Allocate the music mixer if necessary

		if (!_mc) {
			_mc = s->allocateMixer();
			if (!_mc) {
				warning("No mixer channel available for MP3 music");
				return false;
			}
		}

		fseek(_mp3_track, 0, SEEK_END);
		_mp3_size = ftell(_mp3_track);
		fseek(_mp3_track, 0, SEEK_SET);

		_mc->type = MIXER_MP3_CDMUSIC;
		_mc->sound_data.mp3_cdmusic.file = _mp3_track;
		_mc->sound_data.mp3_cdmusic.playing = false;
		_mc->sound_data.mp3_cdmusic.buffer_size = 200000;
		_mc->_sfx_sound = malloc(_mc->sound_data.mp3_cdmusic.buffer_size);

		/* see if it's enough */
		mad_stream_init(&_mc->sound_data.mp3.stream);
		if (_mad_header.samplerate == 44100)
			mad_stream_options((mad_stream *) & _mc->sound_data.mp3.stream,
												 MAD_OPTION_HALFSAMPLERATE);

		mad_frame_init(&_mc->sound_data.mp3.frame);
		mad_synth_init(&_mc->sound_data.mp3.synth);

		_current_mp3_cd_track = track;
	}


	if (_current_mp3_cd_track == track) {

		uint32 where;

		// See where we want to go
		where = calc_cd_file_offset(start_frame);

		if (start_frame < 0 || end_frame < 0) {
			warning("Negative index in frame");
			return false;
		}

		mad_timer_set(&_mc->sound_data.mp3_cdmusic.duration,
									0, end_frame, CD_FPS);

		fseek(_mp3_track, where, SEEK_SET);
		_mc->sound_data.mp3_cdmusic.playing = true;

		return true;
	}

	return false;
}

#endif
