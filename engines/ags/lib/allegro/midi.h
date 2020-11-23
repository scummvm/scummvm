/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGS_STUBS_ALLEGRO_MIDI_H
#define AGS_STUBS_ALLEGRO_MIDI_H

#include "common/scummsys.h"
#include "ags/lib/allegro/base.h"
#include "ags/lib/allegro/alconfig.h"

namespace AGS3 {

/* Theoretical maximums: */
#define MIDI_VOICES           64       /* actual drivers may not be */
#define MIDI_TRACKS           32       /* able to handle this many */

/* a midi file */
struct MIDI {
	int divisions;                      /* number of ticks per quarter note */
	struct {
		unsigned char *data;             /* MIDI message stream */
		int len;                         /* length of the track data */
	} track[MIDI_TRACKS];
};


#define MIDI_AUTODETECT       -1
#define MIDI_NONE             0
#define MIDI_DIGMID           AL_ID('D','I','G','I')

/* driver for playing midi music */
struct MIDI_DRIVER {
	int  id;                            /* driver ID code */
	AL_CONST char *name;                /* driver name */
	AL_CONST char *desc;                /* description string */
	AL_CONST char *ascii_name;          /* ASCII format name string */
	int  voices;                        /* available voices */
	int  basevoice;                     /* voice number offset */
	int  max_voices;                    /* maximum voices we can support */
	int  def_voices;                    /* default number of voices to use */
	int  xmin, xmax;                    /* reserved voice range */

	/* setup routines */
	AL_METHOD(int, detect, (int input));
	AL_METHOD(int, init, (int input, int voices));
	AL_METHOD(void, exit, (int input));
	AL_METHOD(int, set_mixer_volume, (int volume));
	AL_METHOD(int, get_mixer_volume, (void));

	/* raw MIDI output to MPU-401, etc. */
	AL_METHOD(void, raw_midi, (int data));

	/* dynamic patch loading routines */
	AL_METHOD(int, load_patches, (AL_CONST char *patches, AL_CONST char *drums));
	AL_METHOD(void, adjust_patches, (AL_CONST char *patches, AL_CONST char *drums));

	/* note control functions */
	AL_METHOD(void, key_on, (int inst, int note, int bend, int vol, int pan));
	AL_METHOD(void, key_off, (int voice));
	AL_METHOD(void, set_volume, (int voice, int vol));
	AL_METHOD(void, set_pitch, (int voice, int note, int bend));
	AL_METHOD(void, set_pan, (int voice, int pan));
	AL_METHOD(void, set_vibrato, (int voice, int amount));
};


AL_VAR(MIDI_DRIVER, midi_digmid);

AL_ARRAY(_DRIVER_INFO, _midi_driver_list);


/* macros for constructing the driver lists */
#define BEGIN_MIDI_DRIVER_LIST                                 \
	_DRIVER_INFO _midi_driver_list[] =                          \
	        {

#define END_MIDI_DRIVER_LIST                                   \
	{  0,                NULL,                0     }        \
	};

#define MIDI_DRIVER_DIGMID                                     \
	{  MIDI_DIGMID,      &midi_digmid,        TRUE  },


AL_VAR(MIDI_DRIVER *, midi_driver);

AL_VAR(MIDI_DRIVER *, midi_input_driver);

AL_VAR(int, midi_card);

AL_VAR(int, midi_input_card);

AL_VAR(volatile long, midi_pos);       /* current position in the midi file, in beats */
AL_VAR(volatile long, midi_time);      /* current position in the midi file, in seconds */

AL_VAR(long, midi_loop_start);         /* where to loop back to at EOF */
AL_VAR(long, midi_loop_end);           /* loop when we hit this position */


AL_FUNC(int, detect_midi_driver, (int driver_id));

} // namespace AGS3

#endif
