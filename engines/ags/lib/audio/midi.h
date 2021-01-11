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

#ifndef AGS_LIB_AUDIO_MIDI_H
#define AGS_LIB_AUDIO_MIDI_H

#include "common/scummsys.h"
#include "common/array.h"
#include "ags/lib/allegro/base.h"
#include "ags/lib/allegro/alconfig.h"

namespace AGS3 {

/* a midi file */
typedef Common::Array<byte> MIDI;


#define MIDI_AUTODETECT       -1
#define MIDI_NONE             0
#define MIDI_DIGMID           AL_ID('D','I','G','I')

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


AL_VAR(int, midi_card);

AL_VAR(int, midi_input_card);

AL_VAR(volatile long, midi_pos);       /* current position in the midi file, in beats */
AL_VAR(volatile long, midi_time);      /* current position in the midi file, in seconds */

AL_VAR(long, midi_loop_start);         /* where to loop back to at EOF */
AL_VAR(long, midi_loop_end);           /* loop when we hit this position */


AL_FUNC(int, detect_midi_driver, (int driver_id));

extern void stop_midi();
extern void destroy_midi(MIDI *tune);
extern int play_midi(MIDI *tune, bool repeat);
extern size_t get_midi_length(MIDI *tune);
extern void midi_seek(int target);
extern void midi_pause();
extern void midi_resume();
extern int load_midi_patches();

} // namespace AGS3

#endif
