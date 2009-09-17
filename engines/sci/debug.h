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

#include "sci/engine/vm_types.h"	// for StackPtr
#include "sci/engine/vm.h"			// for ExecStack

namespace Sci {

enum DebugSeeking {
	kDebugSeekNothing = 0,
	kDebugSeekCallk = 1,        // Step forward until callk is found
	kDebugSeekLevelRet = 2,     // Step forward until returned from this level
	kDebugSeekSpecialCallk = 3, // Step forward until a /special/ callk is found
	kDebugSeekSO = 4,           // Step forward until specified PC (after the send command) and stack depth
	kDebugSeekGlobal = 5        // Step forward until one specified global variable is modified
};

struct DebugState {
	bool debugging;
	bool stopOnEvent;
	DebugSeeking seeking;		// Stepping forward until some special condition is met
	int runningStep;			// Set to > 0 to allow multiple stepping
	int seekLevel;				// Used for seekers that want to check their exec stack depth
	int seekSpecial;			// Used for special seeks
	int old_pc_offset;
	StackPtr old_sp;
};

// Various global variables used for debugging are declared here
extern int g_debug_sleeptime_factor;
extern int g_debug_simulated_key;
extern bool g_debug_track_mouse_clicks;
extern bool g_debug_weak_validations;
extern DebugState g_debugState;

} // End of namespace Sci

#endif
