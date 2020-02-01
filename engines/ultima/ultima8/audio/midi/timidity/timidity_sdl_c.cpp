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

#include "ultima/ultima8/misc/pent_include.h"

#ifdef USE_TIMIDITY_MIDI

#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include "timidity.h"
#include "timidity_common.h"
#include "timidity_output.h"
#include "timidity_controls.h"
#include "timidity_instrum.h"
#include "timidity_playmidi.h"

namespace Ultima8 {

#ifdef NS_TIMIDITY
namespace NS_TIMIDITY {
#endif

static void ctl_refresh(void);
static void ctl_total_time(int tt);
static void ctl_master_volume(int mv);
static void ctl_file_name(char *name);
static void ctl_current_time(int ct);
static void ctl_note(int v);
static void ctl_program(int ch, int val);
static void ctl_volume(int channel, int val);
static void ctl_expression(int channel, int val);
static void ctl_panning(int channel, int val);
static void ctl_sustain(int channel, int val);
static void ctl_pitch_bend(int channel, int val);
static void ctl_reset(void);
static int ctl_open(int using_stdin, int using_stdout);
static void ctl_close(void);
static int ctl_read(int32 *valp);
static int cmsg(int type, int verbosity_level, const char *fmt, ...);

/**********************************/
/* export the interface functions */

#define ctl sdl_control_mode

ControlMode ctl = {
	"SDL interface", 's',
	OF_NORMAL, 0, 0,
	ctl_open, NULL, ctl_close, ctl_read, cmsg,
	ctl_refresh, ctl_reset, ctl_file_name, ctl_total_time, ctl_current_time,
	ctl_note,
	ctl_master_volume, ctl_program, ctl_volume,
	ctl_expression, ctl_panning, ctl_sustain, ctl_pitch_bend
};

static int ctl_open(int using_stdin, int using_stdout) {
	ctl.opened = 1;
	return 0;
}

static void ctl_close(void) {
	ctl.opened = 0;
}

static int ctl_read(int32 *valp) {
	return TM_RC_NONE;
}

static int cmsg(int type, int verbosity_level, const char *fmt, ...) {
	va_list ap;
	if ((type == CMSG_TEXT || type == CMSG_INFO || type == CMSG_WARNING) &&
	        ctl.verbosity < verbosity_level)
		return 0;
	va_start(ap, fmt);
	vsprintf(timidity_error, fmt, ap);
	va_end(ap);
	//perr.Print ("%s\n", timidity_error);
	return 0;
}

static void ctl_refresh(void) { }

static void ctl_total_time(int tt) {}

static void ctl_master_volume(int mv) {}

static void ctl_file_name(char *name) {}

static void ctl_current_time(int ct) {}

static void ctl_note(int v) {}

static void ctl_program(int ch, int val) {}

static void ctl_volume(int channel, int val) {}

static void ctl_expression(int channel, int val) {}

static void ctl_panning(int channel, int val) {}

static void ctl_sustain(int channel, int val) {}

static void ctl_pitch_bend(int channel, int val) {}

static void ctl_reset(void) {}

#ifdef NS_TIMIDITY
};
#endif

} // End of namespace Ultima8

#endif
