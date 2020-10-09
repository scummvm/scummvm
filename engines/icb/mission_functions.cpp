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

#define FORBIDDEN_SYMBOL_EXCEPTION_strcasecmp

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_vars.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/mission.h"
#include "engines/icb/mission_functions.h"
#include "engines/icb/res_man.h"

#include "common/textconsole.h"

namespace ICB {

int LoadMission(int m, void *usr) {
	int demo = g_globalScriptVariables.GetVariable("demo");
	Init_globals(); // reload the global vars for the new mission
	g_globalScriptVariables.SetVariable("missionelapsedtime", 0);

	// On mission 8 (m=7?) then set a global variable to say we are on mission 8 and not on mission 9
	if (m == 7) {
		g_globalScriptVariables.SetVariable("mission9", 0);
	}
	// On mission 9 (m=8?) then set a global variable to say we are on mission 9 and not on mission 8
	if (m == 8) {
		g_globalScriptVariables.SetVariable("mission9", 1);
	}

	// update the demo flag status
	g_globalScriptVariables.SetVariable("demo", demo);

	// Purge the res_man's to prevent them getting confused
	rs_anims->Res_purge_all();
	rs_bg->Res_purge_all();

	// To remove compiler warning
	usr = NULL;

	warning("rs_anims %d files %dKB rs_bg %d files %dKB", rs_anims->Fetch_files_open(), (rs_anims->Fetch_mem_used() / 1024), rs_bg->Fetch_files_open(),
	        (rs_bg->Fetch_mem_used() / 1024));

	px.current_cd = WhichCD(g_mission_names[m - 1]);

	// Load in the session (mission_name, session_name)
	if (Setup_new_mission(g_mission_startup_names[(m - 1) * 2], g_mission_startup_names[(m - 1) * 2 + 1])) {
	} else {
		return 0;
	}


	// Go straight into mission not the console

	zdebug = FALSE8;

	return 1;
}

void RestartMission(void) {
	if (g_mission == NULL) {
		Fatal_error("Can't restart a deleted mission");
	}

	// Get the mission name for the current mission
	const char *mission_name;

	mission_name = g_mission->Fetch_tiny_mission_name();

	// Find which mission number the current mission is
	int m = FindMissionNumber(mission_name);
	if (m == -1) {
		Fatal_error("Couldn't find the mission '%s'", mission_name);
	}

	// Change the mission number from 0-8 to 1-9
	m++;

	// Right : so just Load that mission
	LoadMission(m, NULL);
}

int FindMissionNumber(const char *mission) {
	int m;

	// Find which mission number this mission is
	for (m = 0; m < NUMBER_OF_MISSIONS; m++) {
		if (scumm_stricmp(g_mission_names[m], mission) == 0) {
			// Mission 8-9 special case check
			if (m == 6) {
				if (g_globalScriptVariables.GetVariable("mission9") == 1)
					m = 7;
			}

			break;
		}
	}

	if (m == NUMBER_OF_MISSIONS) {
		m = -1;
	}

	return m;
}

} // End of namespace ICB
