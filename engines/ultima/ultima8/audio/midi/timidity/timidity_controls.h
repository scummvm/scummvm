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

#ifdef USE_TIMIDITY_MIDI

#ifndef ULTIMA8_AUDIO_MIDI_TIMIDITY_TIMIDITY_CONTROLS_H
#define ULTIMA8_AUDIO_MIDI_TIMIDITY_TIMIDITY_CONTROLS_H

#include "timidity.h"

namespace Ultima {
namespace Ultima8 {

#ifdef NS_TIMIDITY
namespace NS_TIMIDITY {
#endif

/* Return values for ControlMode.read */

#define TM_RC_ERROR -1
#define TM_RC_NONE 0
#define TM_RC_QUIT 1
#define TM_RC_NEXT 2
#define TM_RC_PREVIOUS 3 /* Restart this song at beginning, or the previous
             song if we're less than a second into this one. */
#define TM_RC_FORWARD 4
#define TM_RC_BACK 5
#define TM_RC_JUMP 6
#define TM_RC_TOGGLE_PAUSE 7 /* Pause/continue */
#define TM_RC_RESTART 8 /* Restart song at beginning */

#define TM_RC_PAUSE 9 /* Really pause playing */
#define TM_RC_CONTINUE 10 /* Continue if paused */
#define TM_RC_REALLY_PREVIOUS 11 /* Really go to the previous song */
#define TM_RC_CHANGE_VOLUME 12
#define TM_RC_LOAD_FILE 13      /* Load a new midifile */
#define TM_RC_TUNE_END 14       /* The tune is over, play it again sam? */

#define CMSG_INFO   0
#define CMSG_WARNING    1
#define CMSG_ERROR  2
#define CMSG_FATAL  3
#define CMSG_TRACE  4
#define CMSG_TIME   5
#define CMSG_TOTAL  6
#define CMSG_FILE   7
#define CMSG_TEXT   8

#define VERB_NORMAL 0
#define VERB_VERBOSE    1
#define VERB_NOISY  2
#define VERB_DEBUG  3
#define VERB_DEBUG_SILLY    4

struct ControlMode {
	const char *id_name, id_character;
	int verbosity, trace_playing, opened;

	int (*open)(int using_stdin, int using_stdout);
	void (*pass_playing_list)(int number_of_files, char *list_of_files[]);
	void (*close)(void);
	int (*read)(int32 *valp);
	int (*cmsg)(int type, int verbosity_level, const char *fmt, ...);

	void (*refresh)(void);
	void (*reset)(void);
	void (*file_name)(char *name);
	void (*total_time)(int tt);
	void (*current_time)(int ct);

	void (*note)(int v);
	void (*master_volume)(int mv);
	void (*program)(int channel, int val); /* val<0 means drum set -val */
	void (*volume)(int channel, int val);
	void (*expression)(int channel, int val);
	void (*panning)(int channel, int val);
	void (*sustain)(int channel, int val);
	void (*pitch_bend)(int channel, int val);

};

extern ControlMode *ctl_list[], *ctl;
extern char timidity_error[];

#ifdef NS_TIMIDITY
};
#endif

} // End of namespace Ultima8
} // End of namespace Ultima

#endif

#endif
