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

#include "engines/icb/common/px_common.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/p4.h"
#include "engines/icb/remora.h"
#include "engines/icb/icon_list_manager.h"
#include "engines/icb/sound.h"
#include "engines/icb/string_vest.h"
#include "engines/icb/sound_logic.h"
#include "engines/icb/timer_func.h"
#include "engines/icb/mission_functions.h"
#include "engines/icb/common/datapacker.h"
#include "engines/icb/res_man.h"
#include "engines/icb/options_manager_pc.h"
#include "engines/icb/cluster_manager_pc.h"

#include "common/util.h"
#include "common/textconsole.h"

namespace ICB {

// Short term global until we stop supporting old save game format
uint32 packData;

bool8 Setup_camtest_mission() {
	// create a null mission and session for camtest viewer mode

	return FALSE8;
}

void _mission::Set_new_session_name(const char *ascii) {
	// record the name of the next session

/*	// name to lower
	for (uint32 j = 0; j < strlen(ascii); j++)
		if (Common::isUpper(*(ascii + j)))
			*(ascii + j) = tolower(*(ascii + j));*/

	Set_string(const_cast<char *>(ascii), new_session_name, TINY_NAME_LEN);

	new_session = TRUE8; // trigger a new session
}

void _mission::Set_init_nico_name(const char *ascii) {
	// record the name of the init nico

	Set_string(const_cast<char *>(ascii), init_nico_name, TINY_NAME_LEN);

	init_nico = TRUE8; // trigger a new session
}

bool8 _mission::Is_there_init_nico() {
	// return init nico status

	return (init_nico);
}

void _mission::Reset_init_nico() {
	// remove the name of the init nico

	init_nico = FALSE8; // reset
}

const char *_mission::Return_init_nico_name() { return ((const char *)init_nico_name); }

bool8 Setup_new_mission(const char *mission_name, const char *session_name) {
	// Just to be tidy

	// Stop any previous sounds
	StopAllSoundsNow();

	// Pause the sound engine
	PauseSounds();

	// create a new mission object deleting any current one

	// fix names to lowercase on the PC ONLY
/*	uint32 j;
	for (j = 0; j < strlen(mission_name); j++)
		if (Common::isUpper(*(mission_name + j)))
			*(mission_name + j) = tolower(*(mission_name + j));

	for (j = 0; j < strlen(session_name); j++)
		if (Common::isUpper(*(session_name + j)))
			*(session_name + j) = tolower(*(session_name + j));*/

	// check if session exists

	// If we are using the straight paths check for the cluster
	// Otherwise make up the path to the psx name format cluster

	// Make the filename equivalent of the hash'ed version of mission name
	char h_mission_name[8];
	HashFile(mission_name, h_mission_name);
	char h_session_name[8];
	HashFile(session_name, h_session_name);

	// convert hashes to lower-case for FS operations
	for (int i = 0; i < 8; i++) {
		h_mission_name[i] = (char)tolower(h_mission_name[i]);
		h_session_name[i] = (char)tolower(h_session_name[i]);
	}

	sprintf(temp_buf, SESSION_TEST_PATH, h_mission_name, h_session_name);

#if 1 // was #ifdef FROM_PC_CD
	// Need the mission data present on hard-disk for it to destruct properly
	// so do this here for the pc before we shunt stuff about
	if (g_mission)
		g_icb_mission->___delete_mission();

	// Call the runtime cluster manager to install mission data on minimum install

	// Need a mission id number fist of all
	MISSION_ID m = (MISSION_ID)FindMissionNumber(mission_name);

	// Initialise the install for this mission
	// This function also checks that the correct CD is in the drive too
	if (g_theClusterManager->StartMissionInstall(m)) {
		// Keep calling this until it's done.  I know this prevents windows
		// from doing anything but that's a useful thing for installing stuff.
		while (g_theClusterManager->InstallMission())
			;
	}
#endif

	if (rs_bg->Test_file(temp_buf)) {
		if (g_mission)
			g_icb_mission->___delete_mission();

		g_icb_mission->___init_mission(mission_name, session_name);
		MS->Pre_initialise_objects();
		MS->Init_objects();

		return (TRUE8);
	}

	return (FALSE8);
}

void _mission::___init_mission(const char *new_mission_name, const char *session_name) {
	// mission object constructor
	// creates a session too

	Zdebug("___init_mission %s %s", new_mission_name, session_name);

	// camview mode
	if (camera_hack)
		return;

	chi_following = FALSE8; // store a mission wide session independent record of whether or not chi has been initialised
	num_bullets = 0;
	num_clips = 0;
	num_medi = 0;
	inited_globals = FALSE8;

	if (new_mission_name == NULL)
		Fatal_error("new mission no mission name");
	if (session_name == NULL)
		Fatal_error("new mission no session name");

	// Work out which CD we should be using
	px.current_cd = WhichCD(new_mission_name);

	// Need a mission id number fist of all
	MISSION_ID m = (MISSION_ID)FindMissionNumber(new_mission_name);

	// Initialise the install for this mission
	// This function also checks that the correct CD is in the drive too
	if (g_theClusterManager->StartMissionInstall(m)) {
		// Keep calling this until it's done.  I know this prevents windows
		// from doing anything but that's a useful thing for installing stuff.
		while (g_theClusterManager->InstallMission())
			;
	}

	// When using clusters keep items withouth the root so the correct hashing
	// start point can be maintained
	strcpy(mission_name, new_mission_name);

	Set_string(new_mission_name, tiny_mission_name, TINY_NAME_LEN);
	Set_string(session_name, tiny_session_name, TINY_NAME_LEN);

	// Make the filename equivalent of the hash'ed version of mission name
	HashFile(new_mission_name, h_mission_name);

	Zdebug("-- Begin new mission - %s", (const char *)new_mission_name);

	//  setup mission sound

	LoadMissionSounds(new_mission_name);

	// Reset the widescreen and fade effects
	surface_manager->Reset_Effects();
	// set pointer :this is so things inside session->__init can use MS e.g. remora & inventory initialisation
	g_mission = g_icb_mission;

	// set sessions private resource manager to have no defragging - will fail as soon as a file cannot fit
	private_session_resman->Set_to_no_defrag();

	// setup the starting session
	// note - objects will need their initialise-objects call making outside of this because mission->session-> must be inited
	session = g_icb_session;
	session->___init(mission_name, session_name);

	// set mission status to ok - a script can reset to !0 to end the entire mission
	mission_status = 0;

	// camera will follow the player when this is 0 - change to another id to follow that instead
	camera_follow_id_overide = 0;

	// Don't think this matters but better be safe.
	remora_save_mode = -1;

	// total micro sessions saved
	number_sessions_saved = 0;

	// reset new session flag
	new_session = FALSE8;
	init_nico = FALSE8;

	Zdebug("~___init_mission %s %s", new_mission_name, session_name);
}

void _mission::___delete_mission() {
	// mission object deconstructor

	Zdebug("deleting mission");

	// kill the pointer which doubles as an inited yes/no flag
	g_mission = NULL;

	session->___destruct();

	// Close any existing menu
	g_oIconMenu->CloseDownIconMenu();

	// This kills any lists that are lying about (that script writers may have created but forgotten to destroy).  It
	// won't kill the inventory though, because that has GAME_WIDE scope and we are only killing to MISSION_WIDE.
	g_oIconListManager->ResetToScopeLevel(MISSION_WIDE);

	// This is the call that removes all the icons in the inventory list WITHOUT destroying the list itself.
	g_oIconListManager->ResetList(ICON_LIST_INVENTORY);

	// Turn the in-game timer off
	StopTimer();
}

void _mission::End_mission() {
	// cause the mission to end and shutdown
	mission_status = 1;
}

_mission::_mission()
		: session(NULL), camera_follow_id_overide(0), remora_save_mode(0), ad_time(0), lt_time(0), set_time(0), flip_time(0), cycle_time(0), logic_time(0), resman_logic_time(0),
		los_time(0), event_time(0), sound_time(0), xtra_mega_time(0), nActorsDrawn(0), nActorsConsidered(0), old_hits_value(0), chi_following(0), num_bullets(0), num_clips(0),
		num_medi(0), inited_globals(FALSE8), mission_terminate(0), mission_status(0), number_sessions_saved(0), new_session(FALSE8), init_nico(FALSE8) {
	memset(new_session_name, '\0', TINY_NAME_LEN);
	memset(init_nico_name, '\0', TINY_NAME_LEN);
	memset(mission_name, '\0', ENGINE_STRING_LEN);
	memset(tiny_mission_name, '\0', TINY_NAME_LEN);
	memset(tiny_session_name, '\0', TINY_NAME_LEN);
	memset(h_mission_name, '\0', 8);
	for (int i = 0; i < MAX_sessions; i++) {
		memset(micro_sessions[i].session__name, '\0', ENGINE_STRING_LEN);
		micro_sessions[i].number_of_micro_objects = 0;
		for (int j = 0; j < MAX_session_objects; j++) {
			memset(micro_sessions[i].micro_objects[j].lvar_value, 0, MAX_lvars * sizeof(int32));
			micro_sessions[i].micro_objects[j].total_lvars = 0;
			micro_sessions[i].micro_objects[j].status_flag = OB_STATUS_NOT_HELD;
		}
		memset(micro_sessions[i].fvars, 0, MAX_fvars * sizeof(int32));
	}
}

_mission::~_mission() { Zdebug("*mission destructing*"); }

uint32 _mission::Game_cycle() {
	// run a cycle for the current session in the current mission

	// returns   0 ok
	//				1 finish the mission

	// safety check for no session
	if (session == NULL)
		Fatal_error("no session");

	if (new_session == TRUE8) { // a new session has been requested
		PauseSounds();

		// save the session data to a micro session
		Save_micro_session();

		// remove existing session - taking the view with it
		session->___destruct();

		// set sessions private resource manager to have no defragging - will fail as soon as a file cannot fit
		private_session_resman->Set_to_no_defrag();

		// initialise the new one
		session->___init((const char *)mission_name, (const char *)new_session_name);

		// set the tiny session name
		Set_string(new_session_name, tiny_session_name, TINY_NAME_LEN);

		// create gameworld and structs
		session->Pre_initialise_objects();

		// reload object values if the session has been saved
		Restore_micro_session_vars();

		// init the objects
		session->Init_objects();

		// reload object coordinates
		Restore_micro_session_coords(FALSE8);

		// must reset to NULL
		new_session = FALSE8;

		// cancel init nico
		g_mission->Reset_init_nico();

		camera_follow_id_overide = 0; // cancel previous watch

		remora_save_mode = -1;

		UnpauseSounds();
	}

	// help out rs_anims

	rs_anims->Garbage_removal();

	// check async
	rs1->async_checkArray();

	// update the sound engine
	UpdateHearableSounds();

	// now do a loop of logic
	// get start time
	MS->prev_save_state = MS->Can_save(); // get previous state - used by lifts to see if player is active
	MS->Set_can_save(FALSE8);             // cant save as default - this is reversed by states that allow save this game cycle

	px.logic_timing = TRUE8;
	logic_time = GetMicroTimer();
	resman_logic_time = 0; // reset
	xtra_mega_time = 0;
	session->One_logic_cycle();
	// work out overall time
	logic_time = GetMicroTimer() - logic_time;
	px.logic_timing = FALSE8;

	// cancel SAVE if watching another mega
	if (g_mission->camera_follow_id_overide)
		MS->Set_can_save(FALSE8);

	session->Process_conveyors(); // conveyor belts

#if defined(DEBUG_AUTOSAVE)
	RAM_save_and_restore();
#endif

	// Update global timer by a tick
	g_globalScriptVariables.SetVariable("missionelapsedtime", g_globalScriptVariables.GetVariable("missionelapsedtime") + 1);

	// call the camera director which will pick a view depending upon the player objects position
	session->Camera_director();

	return (mission_status);
}

void _mission::Save_micro_session() {
	// save all object lvars to a micro session
	c_game_object *object;
	uint32 j = 0;
	uint32 i, k;
	uint32 total_fvars = 0;

	Tdebug("micro_session.txt", "\n\nSAVING session %s", Fetch_tiny_session_name());

	// see if this session has ever been saved
	for (j = 0; j < number_sessions_saved; j++) {
		if (!strcmp((const char *)(micro_sessions[j].session__name), Fetch_tiny_session_name())) {
			Tdebug("micro_session.txt", " session found - slot %d", j);
			break;
		}
	}

	// are we creating a new one
	if (j == number_sessions_saved) {
		number_sessions_saved++;
	}

	// j is slot number
	Set_string(Fetch_tiny_session_name(), micro_sessions[j].session__name);

	Tdebug("micro_session.txt", " saving in slot %d", j);

	// save number of em
	micro_sessions[j].number_of_micro_objects = session->Fetch_number_of_objects();

	for (i = 0; i < session->Fetch_number_of_objects(); i++) {
		object = (c_game_object *)session->objects->Fetch_item_by_number(i);

		Tdebug("micro_session.txt", "\n  object %d  %s, %d vars - status %d", i, object->GetName(), object->GetNoLvars(), session->Fetch_object_status(i));
		micro_sessions[j].micro_objects[i].status_flag = session->Fetch_object_status(i);

		// if mega then save coord
		if (session->logic_structs[i]->image_type == VOXEL) {
			if (!session->logic_structs[i]->mega->pushed) {
				Tdebug("micro_session.txt", "  mega");
				micro_sessions[j].fvars[total_fvars++] = ((int32)session->logic_structs[i]->mega->actor_xyz.x);
				micro_sessions[j].fvars[total_fvars++] = ((int32)session->logic_structs[i]->mega->actor_xyz.y);
				micro_sessions[j].fvars[total_fvars++] = ((int32)session->logic_structs[i]->mega->actor_xyz.z);
			} else {
				Tdebug("micro_session.txt", "  mega   *pushed*");
				micro_sessions[j].fvars[total_fvars++] = ((int32)session->logic_structs[i]->mega->pushed_actor_xyz.x);
				micro_sessions[j].fvars[total_fvars++] = ((int32)session->logic_structs[i]->mega->pushed_actor_xyz.y);
				micro_sessions[j].fvars[total_fvars++] = ((int32)session->logic_structs[i]->mega->pushed_actor_xyz.z);
			}
			micro_sessions[j].fvars[total_fvars++] = ((int32)(session->logic_structs[i]->pan * PAN_SCALE_FACTOR));
		}

		micro_sessions[j].micro_objects[i].total_lvars = 0;

		if (object->GetNoLvars() > MAX_lvars)
			Fatal_error("object [%s] has too many lvars - has %d, only %d allowed", object->GetName(), object->GetNoLvars(), MAX_lvars);

		for (k = 0; k < object->GetNoLvars(); k++) {
			if (!object->IsVariableString(k)) {
				Tdebug("micro_session.txt", "   saving lvar %d %s value %d", k, object->GetScriptVariableName(k), object->GetIntegerVariable(k));

				int value = object->GetIntegerVariable(k);

				// Using 14-bits to pack lvar's
				int packMin = -(1 << 13);
				int packMax = +((1 << 13) - 1);

				if ((value < packMin) || (value > packMax)) {
					// Don't do a message box for a CD build of the game!
					Message_box("Object '%s' lvar %d '%s' is too big to pack please try and reduce %d range is %d->%d", object->GetName(), k,
					            object->GetScriptVariableName(k), value, packMin, packMax);
					packData = 0;
				}

				// Using 16-bits as a maximum to store lvar's
				int lvarMin = -(1 << 15);
				int lvarMax = +((1 << 15) - 1);

				if ((value < lvarMin) || (value > lvarMax)) {
					Fatal_error("Object '%s' lvar %d '%s' is too big to save %d range is %d->%d", object->GetName(), k, object->GetScriptVariableName(k), value,
					            packMin, packMax);
				}

				micro_sessions[j].micro_objects[i].lvar_value[micro_sessions[j].micro_objects[i].total_lvars++] = value;
			}
		}
	}
}

void _mission::Restore_micro_session_vars() {
	// reload all object lvars from a micro session
	c_game_object *object;
	uint32 j = 0;
	uint32 i, k;
	uint32 lvar;

	Tdebug("micro_session.txt", "\n\nRestore_micro_session_vars session %s", Fetch_tiny_session_name());

	// see if this session has ever been saved
	for (j = 0; j < number_sessions_saved; j++)
		if (!strcmp((const char *)(micro_sessions[j].session__name), Fetch_tiny_session_name())) {
			Tdebug("micro_session.txt", " session found - slot %d", j);

			// restore lvars
			for (i = 0; i < session->Fetch_number_of_objects(); i++) {
				object = (c_game_object *)session->objects->Fetch_item_by_number(i);

				Tdebug("micro_session.txt", "\n  object %d  %s, %d vars - status %d", i, object->GetName(), object->GetNoLvars(),
				       micro_sessions[j].micro_objects[i].status_flag);
				session->Set_object_status(i, micro_sessions[j].micro_objects[i].status_flag);

				lvar = 0;
				for (k = 0; k < object->GetNoLvars(); k++) {
					if (!object->IsVariableString(k)) {
						Tdebug("micro_session.txt", "   restoring lvar %d %s to %d", k, object->GetScriptVariableName(k),
						       micro_sessions[j].micro_objects[i].lvar_value[lvar]);
						// reset lvar value
						object->SetIntegerVariable(k, micro_sessions[j].micro_objects[i].lvar_value[lvar++]);
					}
				}
			}

			return;
		}

	Tdebug("micro_session.txt", " session NOT found", j);
}

void _mission::Restore_micro_session_coords(bool8 from_disk) {
	// reload all object lvars from a micro session
	c_game_object *object;
	uint32 j = 0;
	uint32 i;
	uint32 index = 0;

	Tdebug("micro_session.txt", "\n\nRestore_micro_session_coords session %s", Fetch_tiny_session_name());

	// see if this session has ever been saved
	for (j = 0; j < number_sessions_saved; j++)
		if (!strcmp((const char *)(micro_sessions[j].session__name), Fetch_tiny_session_name())) {
			Tdebug("micro_session.txt", " session found - slot %d", j);

			// restore lvars
			for (i = 0; i < session->Fetch_number_of_objects(); i++) {
				object = (c_game_object *)session->objects->Fetch_item_by_number(i);

				Tdebug("micro_session.txt", "\n  object %d  %s, %d vars - status %d", i, object->GetName(), object->GetNoLvars(),
				       micro_sessions[j].micro_objects[i].status_flag);
				session->Set_object_status(i, micro_sessions[j].micro_objects[i].status_flag);

				// if mega then restore coord
				if (session->logic_structs[i]->image_type == VOXEL) {
					// from disk, or doesnt have exclusives or (does have exclusives) but is chi and chi is not
					// following
					if ((from_disk) || (!session->logic_structs[i]->mega->has_exclusive_coords) ||
					    ((session->chi_id == i) && (session->chi_think_mode != __FOLLOWING))) {
						session->logic_structs[i]->mega->actor_xyz.x = (PXreal)micro_sessions[j].fvars[index++];
						session->logic_structs[i]->mega->actor_xyz.y = (PXreal)micro_sessions[j].fvars[index++];
						session->logic_structs[i]->mega->actor_xyz.z = (PXreal)micro_sessions[j].fvars[index++];
						session->logic_structs[i]->pan = (PXreal)(micro_sessions[j].fvars[index++] / (PXreal)PAN_SCALE_FACTOR);
					} else { // skip as the data IS saved regardless
						index += 4;
					}
				}
			}
			return;
		}

	Tdebug("micro_session.txt", " session NOT found", j);
}

void _mission::Save_game_position(const char *filename, const char *slot_label, uint32 timeplayed) {
	// save the current game position to disk

	//	save mission/session names
	//			globals
	//			micro-sessions

	uint32 i, j, k;
	int32 avalue, fval;
	int16 lval;
	uint8 atinyvalue;
	int32 l;
	uint32 numIcons;
	char *iconnames[ICON_LIST_MAX_ICONS];
	uint32 iconHashes[ICON_LIST_MAX_ICONS];
	uint32 iconCounts[ICON_LIST_MAX_ICONS];

	Tdebug("save_restore.txt", "\nsaving file [%s]", filename);
	Tdebug("save_restore.txt", " mission [%s]", Fetch_tiny_mission_name());

	// As a default we are trying to pack the data
	packData = 1;

	// save the CURRENT session data to a micro session
	Save_micro_session();

	// Check the lvar's to see if we can pack them or not
	uint nlvars = 0;
	for (j = 0; j < number_sessions_saved; j++) {
		for (i = 0; i < micro_sessions[j].number_of_micro_objects; i++) {
			nlvars = (uint8)micro_sessions[j].micro_objects[i].total_lvars;

			for (k = 0; k < nlvars; k++) {
				lval = (int16)micro_sessions[j].micro_objects[i].lvar_value[k];

				// Using 14-bits to pack lvar's
				int packMin = -(1 << 13);
				int packMax = +((1 << 13) - 1);

				if ((lval < packMin) || (lval > packMax)) {
					Message_box("Jake says : packData = 0");
					packData = 0;
				}
			}
		}
	}

	error("TODO: Fix savegame-path");
	/*
	#ifdef _WIN32
	        mkdir("saves");
	#else
	        mkdir("saves", 0755);
	#endif
	*/

	// first save the index file which contains the session name and mission name that we're currently running - and hence want to
	// restore to later
	Common::WriteStream *stream = openDiskWriteStream(filename); // attempt to open the file for writing

#if defined(DEBUG_AUTOSAVE)

	// NULL is only an error if the filename is something other than the name of
	// the debug autosave file.
	if (strcmp(filename, AUTOSAVE_FILENAME) != 0) {
		if (fh == NULL)
			Fatal_error("Save_game_position cannot *OPEN* [%s]", (const char *)filename);
	}

#else

	if (stream == NULL)
		Fatal_error("Save_game_position cannot *OPEN* [%s]", (const char *)filename);

#endif

	// specific stuff for pc save game menu
	stream->write(slot_label, MAX_LABEL_LENGTH);
	stream->writeSint32LE(timeplayed);

	atinyvalue = SR_VERSION;
	stream->writeByte(atinyvalue);

	avalue = strlen(Fetch_tiny_mission_name()) + 1;
	stream->writeSint32LE(avalue);
	stream->write((const char *)Fetch_tiny_mission_name(), strlen(Fetch_tiny_mission_name()) + 1); // TODO: Refactor to string

	avalue = strlen(Fetch_tiny_session_name()) + 1;
	stream->writeSint32LE(avalue);
	stream->write((const char *)Fetch_tiny_session_name(), strlen(Fetch_tiny_session_name()) + 1); // TODO: Refactor to string

	// now write the globals out
	atinyvalue = (uint8)g_globalScriptVariables.GetNoItems();
	Tdebug("save_restore.txt", " %d globals", atinyvalue);
	stream->writeByte(atinyvalue);

	for (j = 0; j < atinyvalue; j++) {
		avalue = (int32)g_globalScriptVariables.GetVariable(g_globalScriptVariables[j].hash, 0, 0);
		stream->writeSint32LE(avalue);
		Tdebug("save_restore.txt", "  %d 0x%08x = %d", j, g_globalScriptVariables[j].hash, avalue);
	}

	// get the icon information
	numIcons = g_oIconListManager->GetList(ICON_LIST_INVENTORY, iconnames, iconHashes, iconCounts);

	stream->writeUint32LE(numIcons);
	Tdebug("save_restore.txt", "  %d unique icons", numIcons);

	for (j = 0; j < numIcons; j++) {
		// write length of icon name then the string
		avalue = strlen(iconnames[j]) + 1;
		stream->writeSint32LE(avalue);
		stream->write((const void *)iconnames[j], avalue); // TODO: Refactor to string?

		// write the hash value
		avalue = (uint32)iconHashes[j];
		stream->writeUint32LE(avalue);

		// write the count value
		atinyvalue = (uint8)iconCounts[j];
		stream->writeByte(atinyvalue);
	}

	// save the micro-sessions

	// save number of micro-sessions
	avalue = number_sessions_saved;
	stream->writeSint32LE(avalue);
	Tdebug("save_restore.txt", " %d sessions", avalue);

	// The DataPacker object
	DataPacker dpack;

	for (j = 0; j < number_sessions_saved; j++) {
		// write the session name out
		Tdebug("save_restore.txt", "  save session [%s]", (const char *)micro_sessions[j].session__name);
		avalue = strlen(micro_sessions[j].session__name) + 1;
		stream->writeSint32LE(avalue);
		Tdebug("save_restore.txt", "  name len %d", avalue);
		stream->write((const void *)micro_sessions[j].session__name, strlen(micro_sessions[j].session__name) + 1); // TODO: Refactor to string?

		for (l = 0; l < MAX_fvars; l++) {
			fval = micro_sessions[j].fvars[l];
			stream->writeSint32LE(fval);
			Tdebug("save_restore.txt", "   %d", fval);
		}

		stream->writeSint32LE(micro_sessions[j].number_of_micro_objects);
		Tdebug("save_restore.txt", "  %d objects in session", avalue);

		for (i = 0; i < micro_sessions[j].number_of_micro_objects; i++) {
			Tdebug("save_restore.txt", "  \nobject %d", i);

			// first write state flag
			atinyvalue = (uint8)micro_sessions[j].micro_objects[i].status_flag;
			Tdebug("save_restore.txt", "   status %d", atinyvalue);
			stream->writeByte(atinyvalue);

			// save out lvars
			atinyvalue = (uint8)micro_sessions[j].micro_objects[i].total_lvars;
			stream->writeByte(atinyvalue);
			Tdebug("save_restore.txt", "   %d lvars", atinyvalue);
		}

		// Pack the lvar data using the data packer object
		// Initialise the data packer object for writing
		DataPacker::ReturnCodes dret = DataPacker::OK;

		// Are we storing the raw data or using the packer ?
		DataPacker::PackModeEnum packMode = DataPacker::PACK;
		atinyvalue = 0;
		if (packData == 1) {
			packMode = DataPacker::PACK;
			atinyvalue = 1;
		} else {
			packMode = DataPacker::DONT_PACK;
			atinyvalue = 0;
		}

		stream->writeByte(atinyvalue);

		dret = dpack.open(DataPacker::WRITE, packMode);

		if (dret != DataPacker::OK) {
			Real_Fatal_error("DataPacker::Open failed dret %d", (int)dret);
		}
		for (i = 0; i < micro_sessions[j].number_of_micro_objects; i++) {
			nlvars = micro_sessions[j].micro_objects[i].total_lvars;
			for (k = 0; k < nlvars; k++) {
				lval = (int16)micro_sessions[j].micro_objects[i].lvar_value[k];
				Tdebug("save_restore.txt", "   lvar %d = %d", k, lval);

				dret = dpack.put(lval, stream);
				if (dret != DataPacker::OK) {
					Real_Fatal_error("DataPacker::Put failed dret %d", (int)dret);
				}
			}
		}
		dret = dpack.close(stream);
		if (dret != DataPacker::OK) {
			Real_Fatal_error("DataPacker::Close failed dret %d", (int)dret);
		}
	}

	// now save special chi follow information

	if ((session->is_there_a_chi) && (session->chi_think_mode != __NOTHING) && (session->chi_history != session->cur_history)) {
		// there is a chi and she is following and not on our floor

		atinyvalue = 1; // yes
		Tdebug("save_restore.txt", "\nsaving CHI data");
		stream->writeByte(atinyvalue);

		// save x,y and z to restart chi @
		fval = (uint32)session->hist_pin_x;
		stream->writeSint32LE(fval);
		Tdebug("save_restore.txt", " %d", fval);

		fval = (uint32)session->hist_pin_y;
		stream->writeSint32LE(fval);
		Tdebug("save_restore.txt", " %d", fval);

		fval = (uint32)session->hist_pin_z;
		stream->writeSint32LE(fval);
		Tdebug("save_restore.txt", " %d", fval);
	} else {
		// no chi
		atinyvalue = 0; // no
		Tdebug("save_restore.txt", "\nNOT saving CHI data");
		stream->writeByte(atinyvalue);
	}

	// save ammo, bullets, medis
	atinyvalue = (uint8)g_mission->num_bullets;
	stream->writeByte(atinyvalue);
	atinyvalue = (uint8)g_mission->num_clips;
	stream->writeByte(atinyvalue);
	atinyvalue = (uint8)g_mission->num_medi;
	stream->writeByte(atinyvalue);

	// manual cameras
	if (session->manual_camera) {
		atinyvalue = 1; // yes
		Tdebug("save_restore.txt", "\nsaving manual camera");
		stream->writeByte(atinyvalue);

		// save name
		stream->write(session->manual_camera_name, ENGINE_STRING_LEN); // TODO: Refactor to string?
		// cam number
		stream->writeUint32LE(session->cur_camera_number);
	} else {
		// no manual camera
		atinyvalue = 0; // no
		Tdebug("save_restore.txt", "\nNOT saving manual camera");
		stream->writeByte(atinyvalue);
	}

	// game script pc
	avalue = gs.pc;
	stream->writeSint32LE(avalue);
	Tdebug("save_restore.txt", " gamescript pc %d", avalue);

	// save timed events
	g_oEventManager->Save(stream);

	// Save the Remora's locations-visited information.
	g_oRemora->Save(stream);

	// Need to guard against NULL pointer in use by debug autosave.
#if 0 // TODO: Do we actually need to do this? We don't have a max size...
	if (stream) {
		fseek(fh, 0, SEEK_END);    //get size of file
		if (ftell(fh) > SAVE_GAME_MAX_SIZE)
			Message_box("Warning : save game is greater than %d bytes (is %d bytes)\nthis will not work on the PSX please tell Jake",
			            SAVE_GAME_MAX_SIZE, ftell(fh));
	}
#endif

	// save gfx init info for initing a set...
	MSS.SaveGFXInfo(stream);
	SavePlatformSpecific(stream);

	delete stream;
}

void _mission::Restore_micro_session_from_save_game(Common::SeekableReadStream *stream) {
	// restore the micro-sessions for the mission
	uint32 i, j, k;
	int32 avalue;
	uint8 atinyvalue;
	char buf[256];
	uint32 total_sessions;
	uint8 total_lvars = 0;

	total_sessions = stream->readUint32LE();
	Tdebug("save_restore.txt", " %d sessions", total_sessions);

	// set the total saved flag
	number_sessions_saved = total_sessions;

	// The DataPacker object
	DataPacker dpack;

	for (j = 0; j < total_sessions; j++) {
		// read the session name
		// length of name
		avalue = stream->readSint32LE();

		// name
		stream->read(buf, avalue);

		Set_string(buf, micro_sessions[j].session__name);
		Tdebug("save_restore.txt", "  load session [%s]", (const char *)micro_sessions[j].session__name);

		for (k = 0; k < MAX_fvars; k++) {
			avalue = stream->readSint32LE();
			Tdebug("save_restore.txt", "   %d", avalue);
			micro_sessions[j].fvars[k] = avalue;
		}

		micro_sessions[j].number_of_micro_objects = stream->readUint32LE();

		Tdebug("save_restore.txt", "  %d objects in session", micro_sessions[j].number_of_micro_objects);

		DataPacker::PackModeEnum packMode = DataPacker::PACK;

		// The new way
		for (i = 0; i < micro_sessions[j].number_of_micro_objects; i++) {
			Tdebug("save_restore.txt", "  \nobject %d", i);

			// read in state flag
			atinyvalue = stream->readByte();
			micro_sessions[j].micro_objects[i].status_flag = (_object_status)atinyvalue;
			Tdebug("save_restore.txt", "   status %d", atinyvalue);

			// read in lvars
			total_lvars = stream->readByte();

			Tdebug("save_restore.txt", "   %d lvars", total_lvars);
			micro_sessions[j].micro_objects[i].total_lvars = (uint32)total_lvars;
		}

		atinyvalue = stream->readByte();
		if (atinyvalue == 1) {
			packMode = DataPacker::PACK;
		} else {
			packMode = DataPacker::DONT_PACK;
		}
		// read in info for all objects

		// Unpack the lvar data using the data packer object
		// Initialise the data packer object for reading
		DataPacker::ReturnCodes dret = DataPacker::OK;

		dret = dpack.open(DataPacker::READ, packMode);

		if (dret != DataPacker::OK) {
			Real_Fatal_error("DataPacker::Open failed dret %d", (int)dret);
		}

		for (i = 0; i < micro_sessions[j].number_of_micro_objects; i++) {
			total_lvars = (uint8)micro_sessions[j].micro_objects[i].total_lvars;
			for (k = 0; k < total_lvars; k++) {
				dret = dpack.Get(avalue, stream);
				if (dret != DataPacker::OK) {
					Real_Fatal_error("DataPacker::Get failed dret %d", (int)dret);
				}
				micro_sessions[j].micro_objects[i].lvar_value[k] = avalue;
				Tdebug("save_restore.txt", "   lvar %d = %d", k, avalue);
			}
		}

		dret = dpack.close(stream);
		if (dret != DataPacker::OK) {
			Real_Fatal_error("DataPacker::Close failed dret %d", (int)dret);
		}
	}
}

__load_result Load_game(const char *filename) {
	// load a save game
	Common::SeekableReadStream *stream = 0; // file pointer
	uint32 avalue;
	uint8 atinyvalue;
	char mission_name[64];
	char session_name[64];
	char icon_name[MAXLEN_ICON_NAME];
	uint32 icon_hash;
	uint32 j;
	uint32 num_icons;
	int32 fvar;

	Tdebug("save_restore.txt", "\nLoad game [%s]", filename);

	// open the index file
	stream = openDiskFileForBinaryStreamRead(filename); // attempt to open the file for reading

	if (stream == NULL)
		return __NO_SUCH_FILE;

	char label[MAX_LABEL_LENGTH];           // load into here cause i'm too thick to know how to skip it (tw)
	fvar = stream->readUint32LE();          // load and discard time played
	stream->read(&label, MAX_LABEL_LENGTH); // load and discard user label name

	// load schema and check
	atinyvalue = stream->readByte();

	if (atinyvalue != SR_VERSION) {
		delete stream;
		Real_Message_box("Old version save games are not supported file:%d code:%d", atinyvalue, SR_VERSION);
		return __WRONG_VERSION;
	}

	// read the mission name
	// length of name
	avalue = stream->readUint32LE();
	// name
	stream->read(mission_name, avalue);

	// read the session name
	// length of name
	avalue = stream->readUint32LE();
	// name
	stream->read(session_name, avalue);

	Tdebug("save_restore.txt", "mission [%s] session [%s]", mission_name, session_name);

	// number of globals
	atinyvalue = stream->readByte();
	Tdebug("save_restore.txt", " %d globals", atinyvalue);
	if (atinyvalue != (uint8)g_globalScriptVariables.GetNoItems()) {
		Tdebug("save_restore.txt", " globals mismatch");
		return __GLOBAL_MISMATCH;
	}

	for (j = 0; j < atinyvalue; j++) {
		avalue = stream->readSint32LE();
		g_globalScriptVariables.SetVariable(g_globalScriptVariables[j].hash, avalue);
		Tdebug("save_restore.txt", "  %d = %d", j, avalue);
	}

	// delete any existing mission
	if (g_mission)
		g_icb_mission->___delete_mission();
	g_icb_mission->___init_mission(mission_name, session_name);

	// read number of icons
	num_icons = stream->readUint32LE();
	Tdebug("save_restore.txt", "  %d icons", num_icons);

	for (j = 0; j < num_icons; j++) {
		// string length
		avalue = stream->readSint32LE();
		stream->read(icon_name, avalue);

		// read hash value
		icon_hash = stream->readUint32LE();

		// read count
		atinyvalue = stream->readByte();

		Tdebug("save_restore.txt", "  [%s] hash %X %d", icon_name, icon_hash, atinyvalue);

		while (atinyvalue--)
			g_oIconListManager->AddIconToList(ICON_LIST_INVENTORY, icon_name, icon_hash);
	}

	// setup the major session components
	MS->Pre_initialise_objects();

	// rebuild the micro-session
	g_mission->Restore_micro_session_from_save_game(stream);

	Tdebug("save_restore.txt", "restored micro session");

	// reload object values if the session has been saved
	g_mission->Restore_micro_session_vars();

	Tdebug("save_restore.txt", "restored vars");

	// run the init scripts
	MS->Init_objects();

	Tdebug("save_restore.txt", "initialised objects");

	// reload object coordinates
	g_mission->Restore_micro_session_coords(TRUE8);

	MS->Set_init_voxel_floors(); // v important for some logics that will begin by checking a floor number
	// setup player history to new restored coordinate/floor
	MS->pre_interact_floor = MS->logic_structs[MS->player.Fetch_player_id()]->owner_floor_rect;

	// restore chi if she was following
	atinyvalue = stream->readByte();
	if (atinyvalue) {
		Tdebug("save_restore.txt", "\nrestore CHI");

		// chi x
		fvar = stream->readSint32LE();
		Tdebug("save_restore.txt", " %d", fvar);
		MS->logic_structs[MS->chi_id]->mega->actor_xyz.x = (PXreal)fvar;

		// y
		fvar = stream->readSint32LE();
		Tdebug("save_restore.txt", " %d", fvar);
		MS->logic_structs[MS->chi_id]->mega->actor_xyz.y = (PXreal)fvar;

		// z
		fvar = stream->readSint32LE();
		Tdebug("save_restore.txt", " %d", fvar);
		MS->logic_structs[MS->chi_id]->mega->actor_xyz.z = (PXreal)fvar;

		g_mission->chi_following = TRUE8; // she was following so set the master flag - may or may not be required but lets go for safety

		Tdebug("save_restore.txt", " %3.1f %3.1f %3.1f", MS->logic_structs[MS->chi_id]->mega->actor_xyz.x, MS->logic_structs[MS->chi_id]->mega->actor_xyz.y,
		       MS->logic_structs[MS->chi_id]->mega->actor_xyz.z);
	} else {
		Tdebug("save_restore.txt", "\nNOT restoring CHI");
	}

	// save ammo, bullets, medis
	atinyvalue = stream->readByte();
	g_mission->num_bullets = (uint32)atinyvalue;

	atinyvalue = stream->readByte();
	g_mission->num_clips = (uint32)atinyvalue;

	atinyvalue = stream->readByte();
	g_mission->num_medi = (uint32)atinyvalue;

	// manual camera?
	atinyvalue = stream->readByte();
	if (atinyvalue) {
		Tdebug("save_restore.txt", "\nrestoring manual camera");

		// save name
		stream->read(MS->manual_camera_name, ENGINE_STRING_LEN);
		// cam number
		MS->cur_camera_number = stream->readUint32LE();

		MS->manual_camera = TRUE8;

		char h_buf[8];
		HashFile(MS->manual_camera_name, h_buf);
		MS->Initialise_set(MS->manual_camera_name, h_buf);
	} else {
		Tdebug("save_restore.txt", "\nNOT restoring manual camera");
	}

	// gamescript pc
	gs.pc = stream->readUint32LE();

	// timed events
	g_oEventManager->Restore(stream);

	// Restore the Remora's knowledge about where the player has been.
	g_oRemora->Restore(stream);

	// load gfx init info for initing a set...
	MSS.LoadGFXInfo(stream);
	g_mission->LoadPlatformSpecific(stream);

	delete stream;

	// run a logic cycle to get those anims caching!
	MS->One_logic_cycle();
	rs_anims->async_flush();
	MS->Camera_director();
	MS->One_logic_cycle();
	rs_anims->async_flush();

	return __LOAD_OK;
}

void _mission::Create_display() {
	switch (px.display_mode) {
	case THREED:
		// Need this for development safey - but is redundant in final (console-less) game
		if (!session->SetOK()) {
			px.display_mode = TEMP_NETHACK; // so we can bounce out again if a real camera/set is found

			// Save the actor's control mode
			session->player.Push_control_mode(ACTOR_RELATIVE);
		} else {
			// Check if we have just regain the focus after task switching
			if (gRegainedFocus) {
				session->set.ReInit();

				if (g_oIconMenu->IsActive())
					g_oIconMenu->ReActivate();

				gRegainedFocus = false;
			}
			// To get on,off camera events in REMORA mode
			// this function is less accurate than stage_draw computation of same events
			session->UpdateOnOffCamera();

			// The Remora has a function which gets called every cycle when it is active.  This is because game
			// logic continues to run when the Remora is up.
			// But, note the background is NOT drawn whilst in REMORA mode
			if (g_oRemora->IsActive()) {
				g_oRemora->DrawRemora();
			} else {
//  full 3d stage draw NOT in REMORA mode
				session->Stage_draw_poly();

				// Only render speech when not in REMORA mode
				// (as REMORA uses speech system to draw its own text)
				if (px.on_screen_text) {
					session->Render_speech(session->text_speech_bloc);

					// If there is currently a SFX subtitle active then display it.
					if (g_oSoundLogicEngine->SubtitleActive())
						g_oSoundLogicEngine->DrawSubtitle();
				}
			}

			// draw a timer if we have one...
			DrawTimer();

			// If the icon menu is active, draw it.
			if (g_oIconMenu->IsActive()) {
				g_oIconMenu->DrawIconMenu();

				// If not in the REMORA then draw the armed menu & health bar as well
				if ((g_oRemora->IsActive() == FALSE8) && (session->logic_structs[session->player.Fetch_player_id()]->mega->Fetch_armed_status())) {
					int nBullets = session->player.GetNoBullets();
					int nClips = session->player.GetNoAmmoClips();
					int maxBullets = session->player.GetBulletsPerClip();
					int maxClips = session->player.GetMaxClips();
					g_oIconMenu->DrawArmedMenu(nBullets, maxBullets, nClips, maxClips);

					session->Draw_health_bar();
					session->health_time = 0; // cancel the health bar timer
				}
			} else if (session->logic_structs[session->player.Fetch_player_id()]->mega->Fetch_armed_status()) { // if player armed
				session->Draw_health_bar();
				session->health_time = 0; // cancel the health bar timer

				int nBullets = session->player.GetNoBullets();
				int nClips = session->player.GetNoAmmoClips();
				int maxBullets = session->player.GetBulletsPerClip();
				int maxClips = session->player.GetMaxClips();
				g_oIconMenu->DrawArmedMenu(nBullets, maxBullets, nClips, maxClips);
			} else if (session->health_time) {
				// Draw the health bar if unarmed and recently taken damage
				session->health_time--;
				session->Draw_health_bar();
			}

			// If the icon menu is currently flashing added medipacks or clips draw it (but not in Remora).
			if (!g_oRemora->IsActive() && g_oIconMenu->IsAdding())
				g_oIconMenu->DrawAdding();

			if (px.mega_timer)
				session->Display_mega_times();

			// Crude Interaction Highlight
			session->player.Render_crude_interact_highlight();
			session->Show_lit_unlit_diagnostics();
			session->player.DrawCompass();
		}
		break;

	default:
		Fatal_error("unknown game display mode");
		break;
	}
}

} // End of namespace ICB
