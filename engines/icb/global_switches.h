/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_GLOBAL_SWITCHES_H
#define ICB_GLOBAL_SWITCHES_H

#include "engines/icb/common/px_common.h"

namespace ICB {

enum _display_mode {
	THREED,      // full stage draw with voxels
	NETHACK,     // top down plan view
	TEMP_NETHACK // net hack that swaps back to THREED when a set can be displayed - i.e. as soon as possible
};

class c_global_switches {
public:
	c_global_switches();

	uint32 px_fps; // 0  normal/default
	//! 0 24fps/alternate fps scheme

	_display_mode display_mode; // nethack or full voxel

	int actorShadows; // -1 = top down shadows only, 0 - no actors shadows
	int current_cd;
	int antialiasedActorLevel; // 0 = none, 1 = edges only, 2 = edge + actor blend
	int missionTimeout;        // how many game cycles to run a mission for ( <=0 - means forever)

	bool8 nicos_displayed; // yes or no
	bool8 frag_help;       // display set fragments
	bool8 lighting;        // actor lighting manual overide
	bool8 cross_hair;      // development

	bool8 los_enabled; // turn los on/off
	bool8 shadow_los;  // Line of sight- players in shadows
	bool8 debugging_and_console;
	bool8 recordingVideo; // true each frame is recorded, false no recording is done

	bool8 on_screen_text; // text on screen or not
	bool8 mega_hilite;
	bool8 texturedActors;
	bool8 litActors;
	bool8 polyActors;
	bool8 wireframeActors;

	bool8 speechLineNumbers;
	bool8 pc_full_intall;
	bool8 joystickVibration;
	bool8 socket_watch;

	bool8 semitransparencies; // On or off
	bool8 mega_timer;
	bool8 logic_timing; // so we know when to exlude resman
	bool8 prop_hilite;

	bool8 game_completed; // Set by gamescript marker - unlocks title screen bonuses
};

extern c_global_switches px;

} // End of namespace ICB

#endif
