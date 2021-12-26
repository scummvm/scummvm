/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/icb/p4.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_globalvariables.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/session.h"
#include "engines/icb/mission.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_objects_psx.h"
#include "engines/icb/debug.h"
#include "engines/icb/res_man.h"
#include "engines/icb/movie_pc.h"
#include "engines/icb/cluster_manager_pc.h"
#include "engines/icb/sound_logic.h"
#include "engines/icb/options_manager_pc.h"
#include "engines/icb/floors.h"
#include "engines/icb/remora.h"
#include "engines/icb/gfx/psx_camera.h"
#include "engines/icb/gfx/psx_pxactor.h"
#include "engines/icb/gfx/rlp_api.h"
#include "engines/icb/gfx/psx_scrn.h"
#include "engines/icb/common/px_capri_maths_pc.h"
#include "engines/icb/common/px_capri_maths.h"
#include "engines/icb/common/ptr_util.h"
#include "engines/icb/sound.h"
#include "engines/icb/tracer.h"

namespace ICB {

#define ISGRAPH(c) ((((c) >= '0') && ((c) <= '9')) || (((c) >= 'a') && ((c) <= 'z')) || (((c) >= 'A') && ((c) <= 'Z')) || ((c) == '_'))

#define ISNUMBER(c) (((c) >= '0') && ((c) <= '9'))

void Init_globals() {
	int32 len, index;
	uint8 *mem_tmp;
	char buf[ENGINE_STRING_LEN];
	char cluster[ENGINE_STRING_LEN];
	char input[256];
	char value[256];
	uint32 i;
	uint32 nVars = 0;

	sprintf(buf, GLOBAL_VAR_PATH);
	sprintf(cluster, GLOBAL_CLUSTER_PATH);
	uint32 fn_hash = HashString(buf);
	uint32 cluster_hash = HashString(cluster);

	if (rs_bg->Test_file(buf, fn_hash, cluster, cluster_hash)) {

		Tdebug("globals.txt", "found globals file - %s", (const char *)buf);

		mem_tmp = rs_bg->Res_open(buf, fn_hash, cluster, cluster_hash);

		Tdebug("globals.txt", "loaded");

		len = rs_bg->Fetch_size(buf, fn_hash, cluster, cluster_hash);

		index = 0;
		do {
			// get name
			i = 0;

			while (ISGRAPH(mem_tmp[index]))
				input[i++] = mem_tmp[index++];

			input[i] = 0; // NULL terminate

			if (index >= len) {
				Fatal_error("Index >= len %d %d whilst getting global var name %s", index, len, input);
			}

			// ship white
			while (!ISGRAPH(mem_tmp[index]))
				index++;

			if (index >= len) {
				Fatal_error("Index >= len %d %d whilst skipping to global var value %s", index, len, input);
			}

			// get value
			i = 0;

			while ((index < len) && (ISNUMBER(mem_tmp[index]))) // 0-9
				value[i++] = mem_tmp[index++];              // toupper(buf[index++]);

			value[i] = 0; // NULL terminate

			// ship white
			while ((index < len) && (!ISGRAPH(mem_tmp[index])))
				index++;

			i = atoi(value);

			Tdebug("globals.txt", "found var [%s] set to [%s, %d]", input, value, i);
			g_globalScriptVariables->InitVariable(input, i);

			nVars++;
		} while (index < (len - 1));

		rs_bg->Res_purge(buf, fn_hash, cluster, cluster_hash, 0);
	} else
		Fatal_error("no globals file - %s", (const char *)buf);

	// Right all done
	// So sort the globals so they can be searched quicker !
	g_globalScriptVariables->SortVariables();

	g_otz_offset = -5;

	Tdebug("globals.txt", "Found %d global variables", nVars);
}

void CreateGlobalObjects() {
	g_px = new c_global_switches();
	g_oTracer = new _tracer();
	g_ptrArray = new Common::Array<PointerReference>();
	g_globalScriptVariables = new CpxGlobalScriptVariables();
	g_theSequenceManager = new MovieManager();
	g_while_u_wait_SequenceManager = new MovieManager();
	g_personalSequenceManager = new MovieManager();
	g_theClusterManager = new ClusterManager();
	g_theOptionsManager = new OptionsManager();
	g_icb_mission = new _mission();
	g_icb_session = new _game_session();
	g_icb_session_floors = new _floor_world();
	g_text_bloc1 = new text_sprite();
	g_text_bloc2 = new text_sprite();
	g_av_actor = new psxActor();
	g_camera = new psxCamera();
	g_av_Light = new PSXLamp();
	for (int i = 0; i < MAX_voxel_list; i++) {
		g_megas[i] = new _mega();
		g_vox_images[i] = new _vox_image();
	}
	for (int i = 0; i < MAX_session_objects; i++)
		g_logics[i] = new _logic();
	g_stub = new _stub();
	gterot_pc = new MATRIXPC();
	gtetrans_pc = new MATRIXPC();
	gtecolour_pc = new MATRIXPC();
	gtelight_pc = new MATRIXPC();
	gterot = new MATRIX();
	gtetrans = new MATRIX();
	gtecolour = new MATRIX();
	gtelight = new MATRIX();
	for (int i = 0; i < MAX_REGISTERED_SOUNDS; i++)
		g_registeredSounds[i] = new CRegisteredSound();

	// The order of creation matters:
	g_oEventManager = new _event_manager();
	g_oLineOfSight = new _line_of_sight();
	g_oIconMenu = new _icon_menu();
	g_oIconListManager = new _icon_list_manager();
	g_oRemora = new _remora();
	g_oSoundLogicEngine = new _sound_logic();
}

void DestroyGlobalObjects() {
	delete g_px;
	g_px = nullptr;
	delete g_oTracer;
	delete g_ptrArray;
	delete g_globalScriptVariables;
	g_globalScriptVariables = nullptr;
	delete g_theSequenceManager;
	delete g_while_u_wait_SequenceManager;
	delete g_personalSequenceManager;
	delete g_theClusterManager;
	delete g_theOptionsManager;
	delete g_icb_mission;
	delete g_icb_session;
	delete g_icb_session_floors;
	delete g_text_bloc1;
	delete g_text_bloc2;
	delete g_camera;
	delete g_av_actor;
	delete g_av_Light;
	for (int i = 0; i < MAX_voxel_list; i++) {
		delete g_megas[i];
		delete g_vox_images[i];
	}
	for (int i = 0; i < MAX_session_objects; i++)
		delete g_logics[i];
	delete g_stub;
	delete gterot_pc;
	delete gtetrans_pc;
	delete gtecolour_pc;
	delete gtelight_pc;
	delete gterot;
	delete gtetrans;
	delete gtecolour;
	delete gtelight;
	for (int i = 0; i < MAX_REGISTERED_SOUNDS; i++) {
		delete g_registeredSounds[i];
		g_registeredSounds[i] = nullptr;
	}

	delete g_oEventManager;
	delete g_oLineOfSight;
	delete g_oIconMenu;
	delete g_oIconListManager;
	delete g_oRemora;
	delete g_oSoundLogicEngine;
}

} // End of namespace ICB
