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

#ifndef AGS_STUBS_ALLEGRO_DIGI_H
#define AGS_STUBS_ALLEGRO_DIGI_H

#include "common/scummsys.h"
#include "ags/stubs/allegro/base.h"
#include "ags/stubs/allegro/alconfig.h"

namespace AGS3 {


#define DIGI_VOICES           64       /* Theoretical maximums: */
/* actual drivers may not be */
/* able to handle this many */

/* a sample */
struct SAMPLE {
	int bits;                           /* 8 or 16 */
	int stereo;                         /* sample type flag */
	int freq;                           /* sample frequency */
	int priority;                       /* 0-255 */
	unsigned long len;                  /* length (in samples) */
	unsigned long loop_start;           /* loop start position */
	unsigned long loop_end;             /* loop finish position */
	unsigned long param;                /* for internal use by the driver */
	void *data;                         /* sample data */
};


#define DIGI_AUTODETECT       -1       /* for passing to install_sound() */
#define DIGI_NONE             0

/* driver for playing digital sfx */
struct DIGI_DRIVER {
	int  id;                            /* driver ID code */
	AL_CONST char *name;                /* driver name */
	AL_CONST char *desc;                /* description string */
	AL_CONST char *ascii_name;          /* ASCII format name string */
	int  voices;                        /* available voices */
	int  basevoice;                     /* voice number offset */
	int  max_voices;                    /* maximum voices we can support */
	int  def_voices;                    /* default number of voices to use */

	/* setup routines */
	AL_METHOD(int, detect, (int input));
	AL_METHOD(int, init, (int input, int voices));
	AL_METHOD(void, exit, (int input));
	AL_METHOD(int, set_mixer_volume, (int volume));
	AL_METHOD(int, get_mixer_volume, (void));

	/* for use by the audiostream functions */
	AL_METHOD(void *, lock_voice, (int voice, int start, int end));
	AL_METHOD(void, unlock_voice, (int voice));
	AL_METHOD(int, buffer_size, (void));

	/* voice control functions */
	AL_METHOD(void, init_voice, (int voice, AL_CONST SAMPLE *sample));
	AL_METHOD(void, release_voice, (int voice));
	AL_METHOD(void, start_voice, (int voice));
	AL_METHOD(void, stop_voice, (int voice));
	AL_METHOD(void, loop_voice, (int voice, int playmode));

	/* position control functions */
	AL_METHOD(int, get_position, (int voice));
	AL_METHOD(void, set_position, (int voice, int position));

	/* volume control functions */
	AL_METHOD(int, get_volume, (int voice));
	AL_METHOD(void, set_volume, (int voice, int volume));
	AL_METHOD(void, ramp_volume, (int voice, int tyme, int endvol));
	AL_METHOD(void, stop_volume_ramp, (int voice));

	/* pitch control functions */
	AL_METHOD(int, get_frequency, (int voice));
	AL_METHOD(void, set_frequency, (int voice, int frequency));
	AL_METHOD(void, sweep_frequency, (int voice, int tyme, int endfreq));
	AL_METHOD(void, stop_frequency_sweep, (int voice));

	/* pan control functions */
	AL_METHOD(int, get_pan, (int voice));
	AL_METHOD(void, set_pan, (int voice, int pan));
	AL_METHOD(void, sweep_pan, (int voice, int tyme, int endpan));
	AL_METHOD(void, stop_pan_sweep, (int voice));

	/* effect control functions */
	AL_METHOD(void, set_echo, (int voice, int strength, int delay));
	AL_METHOD(void, set_tremolo, (int voice, int rate, int depth));
	AL_METHOD(void, set_vibrato, (int voice, int rate, int depth));

	/* input functions */
	int rec_cap_bits;
	int rec_cap_stereo;
	AL_METHOD(int, rec_cap_rate, (int bits, int stereo));
	AL_METHOD(int, rec_cap_parm, (int rate, int bits, int stereo));
	AL_METHOD(int, rec_source, (int source));
	AL_METHOD(int, rec_start, (int rate, int bits, int stereo));
	AL_METHOD(void, rec_stop, (void));
	AL_METHOD(int, rec_read, (void *buf));
};

AL_ARRAY(_DRIVER_INFO, _digi_driver_list);

/* macros for constructing the driver lists */
#define BEGIN_DIGI_DRIVER_LIST                                 \
	_DRIVER_INFO _digi_driver_list[] =                          \
	        {

#define END_DIGI_DRIVER_LIST                                   \
	{  0,                nullptr,             0     }        \
	};

AL_VAR(DIGI_DRIVER *, digi_driver);

AL_VAR(DIGI_DRIVER *, digi_input_driver);

AL_VAR(int, digi_card);

AL_VAR(int, digi_input_card);

AL_FUNC(int, detect_digi_driver, (int driver_id));

} // namespace AGS3

#endif
