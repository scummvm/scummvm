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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_DEBUG_H
#define SCI_DEBUG_H

namespace Sci {

// Various global variables used for debugging are declared here

extern int g_stop_on_event;

extern int g_debugstate_valid;
extern int g_debug_seeking;
extern int g_debug_step_running;

extern int g_debug_sleeptime_factor;
extern int g_debug_simulated_key;
extern bool g_debug_track_mouse_clicks;
extern bool g_debug_weak_validations;
// Script related variables
extern int g_debug_seeking;
extern int g_debug_seek_special;
extern int g_debug_seek_level;

struct DebugState {
	reg_t *p_pc;
	StackPtr *p_sp;
	StackPtr *p_pp;
	reg_t *p_objp;
	int *p_restadjust;
	SegmentId *p_var_segs;
	reg_t **p_vars;
	reg_t **p_var_base;
	int *p_var_max; // May be NULL even in valid state!
};

} // End of namespace Sci

#endif
