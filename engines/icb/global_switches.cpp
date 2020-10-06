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

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/global_switches.h"

namespace ICB {

// projects x's global flags
c_global_switches px;

c_global_switches::c_global_switches() {
	px_fps = 1;               // 12
	display_mode = THREED;    // default display mode
	nicos_displayed = FALSE8; // 3d nico display
	frag_help = FALSE8;       // set draw displays fragments
	lighting = TRUE8;         // set lighting manual overide

	// have BASIC top-down shadows ON by default for the both platforms
	actorShadows = -1; // = -1 for automatic top-down shadow
	cross_hair = FALSE8;
	prop_hilite = FALSE8;
	mega_hilite = FALSE8;
	game_completed = FALSE8;
	los_enabled = TRUE8;
	debugging_and_console = TRUE8; // head up switcher, headup info and console allowed - i.e. no in final game
	shadow_los = TRUE8;            // actors can stand in shadows and LOS takes account
	recordingVideo = FALSE8;       // true each frame is recorded, false no recording is done
	on_screen_text = TRUE8;
	texturedActors = TRUE8;
	litActors = TRUE8;
	polyActors = TRUE8;
	wireframeActors = FALSE8;
	speechLineNumbers = FALSE8;
	current_cd = 1; // cd 1, 2, 3
	missionTimeout = -1;
	pc_full_intall = FALSE8;
	joystickVibration = FALSE8;
	socket_watch = FALSE8;
	semitransparencies = TRUE8;
	mega_timer = FALSE8;
	antialiasedActorLevel = 0;
	logic_timing = 0;
}

} // End of namespace ICB
