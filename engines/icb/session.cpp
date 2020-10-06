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

#include "engines/icb/p4.h"
#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/debug.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/res_man.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_game_object.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/common/px_prop_anims.h"
#include "engines/icb/common/px_walkarea_integer.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/session.h"
#include "engines/icb/mission.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/floors.h"
#include "engines/icb/barriers.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/remora.h"
#include "engines/icb/sound_logic.h"
#include "engines/icb/loadscrn.h"
#include "engines/icb/sound.h"
#include "engines/icb/sound_lowlevel.h"

namespace ICB {


// Translation tweaks

_linked_data_file *LoadTranslatedFile(cstr session, cstr mission);


// prototypes
int32 Fetch_token_value(uint8 *file, uint32 length, uint8 *token);

void ClearTextures();

void _game_session::___init(const char *mission, const char *new_session_name) {
	// session object constructor
	// set up a game_session object from a session name

	uint32 buf_hash;

	// begin with no set object
	// a camera will be choosen after the first logic cycle based upon the player objects position
	set.Reset();

	// no special footsteps set

	numFloorFootSfx = 0;
	specialFootSfx = 0;
	ladderFootSfx = 0;
	defaultFootSfx = 0;

	// setup speech text block pointer
	text_bloc = g_text_bloc1;
	text_speech_bloc = g_text_bloc2; // pc has second bloc

	// If you die when you have an unread email and restart the mission, the email icon continues
	// to flash.  The Remora should reset this when it initialises but since that happens after all
	// the objects have been loaded, I decided it would be safest to reset the flag here.  This is
	// very much an 11th-hour hack.
	g_oRemora->MarkEmailRead();

	// first clear out private_session_resman
	private_session_resman->Reset();
	// tell it that the resources cannot be moved about
	private_session_resman->Set_to_no_defrag();

	ClearTextures();

	if (camera_hack == TRUE8) {
		total_objects = 0;
		return;
	}

	// Make the filename equivalent of the hash'ed version of session name
	HashFile(new_session_name, session_h_name);

	// Put the hash mission and hash session filenames together
	char h_mission_name[8];
	HashFile(mission, h_mission_name);

	sprintf(speech_font_one, FONT_PATH, "font.pcfont");
	sprintf(remora_font, FONT_PATH, "futura.pcfont");

	if (sprintf(session_name, "%s\\%s\\", mission, new_session_name) > ENGINE_STRING_LEN)
		Fatal_error("_game_session::_game_session [%s] string overflow", session_name);

	if (sprintf(h_session_name, "%s\\%s", h_mission_name, session_h_name) > ENGINE_STRING_LEN)
		Fatal_error("_game_session::_game_session [%s] string overflow", h_session_name);

	if (sprintf(session_cluster, SESSION_CLUSTER_PATH, h_mission_name, session_h_name) > ENGINE_STRING_LEN)
		Fatal_error("_game_session::_game_session [%s] string overflow", session_cluster);

	session_cluster_hash = HashString(session_cluster);
	speech_font_one_hash = HashString(speech_font_one);
	remora_font_hash = HashString(remora_font);
	Zdebug("_game_session %s", (const char *)session_name);

// now setup the session
// load all the fixed name files

//	mission<
//		session<
//			objects.linked
//			scripts.linked
//			walkgrid.grid
//			session.RVanims
//			camera-name<

	// Jake : so PSX can have nice session loading screen and details (for timing and to stop player getting bored)
	StartLoading(new_session_name);

	LoadMsg("Session Cluster");

	// right, on the psx for the between session sound we need to make sure the
	// resman has the mission sound stuff in memory BEFORE we do the
	// StartLoading...

	LoadMsg("Session Sound");

	// setup sound data cluster on psx...

	LoadSessionSounds(session_cluster);


	// initialise the session game objects
	// we can assume all of these in here will be of the game object class!

	// When clustered the session files have the base stripped
	strcpy(temp_buf, "objects");

	// so PSX can have nice session loading screen and details (for timing and to stop player getting bored)
	LoadMsg("Session Objects");

	// Make Res_open compute the hash value
	buf_hash = NULL_HASH;
	objects = (_linked_data_file *)private_session_resman->Res_open(temp_buf, buf_hash, session_cluster, session_cluster_hash);

	//	set this for convenience
	total_objects = objects->Fetch_number_of_items();
	Zdebug("total objects %d", total_objects);

	if (total_objects >= MAX_session_objects)
		Fatal_error("too many objects! max available %d", MAX_session_objects);

	// create the prop table
	// each object has a prop state - though only objects that are linked to background props will use them
	// an objects number maps to the state table
	//	prop_state_table = new   uint32[total_objects+10];
	uint32 j;
	for (j = 0; j < total_objects; j++)
		prop_state_table[j] = 0;

	// inititialise the session scripts

	// When clustered the session files have the base stripped
	strcpy(temp_buf, "scripts");

	// so PSX can have nice session loading screen and details (for timing and to stop player getting bored)
	LoadMsg("Session Scripts");
	buf_hash = NULL_HASH;
	scripts = (_linked_data_file *)private_session_resman->Res_open(temp_buf, buf_hash, session_cluster, session_cluster_hash);

	// display script version info
	// also available on console
	Script_version_check();

	// initialise prop animation file

	// When clustered the session files have the base stripped
	strcpy(temp_buf, PX_FILENAME_PROPANIMS);

	// so PSX can have nice session loading screen and details (for timing and to stop player getting bored)
	LoadMsg("Session PropAnims");
	buf_hash = NULL_HASH;
	prop_anims = (_linked_data_file *)private_session_resman->Res_open(temp_buf, buf_hash, session_cluster, session_cluster_hash);

	// Check file version is correct.
	if (prop_anims->GetHeaderVersion() != VERSION_PXWGPROPANIMS)
		Fatal_error("%s version check failed (file has %d, engine has %d)", temp_buf, prop_anims->GetHeaderVersion(), VERSION_PXWGPROPANIMS);

	// init features file
	// we stick this in the private cache so it hangs around and later in-game references wont cause a main pool reload

	// When clustered the session files have the base stripped
	strcpy(temp_buf, "pxwgfeatures");

	// so PSX can have nice session loading screen and details (for timing and to stop player getting bored)
	LoadMsg("Session Features");
	buf_hash = NULL_HASH;
	features = (_linked_data_file *)private_session_resman->Res_open(temp_buf, buf_hash, session_cluster, session_cluster_hash);

	// engine knows no set/camera chosen
	Reset_camera_director();
	camera_lock = FALSE8; // move to camera director

	// reset the route manager service
	Reset_route_manager();

	text_bloc->please_render = FALSE8;
	text_speech_bloc->please_render = FALSE8;
	conv_focus = 0; // no conversation is in focus
	total_convs = 0; // no conversations

	Tdebug("text_lines.txt", "\n\n---Text Lines---\n");

	// text
	text = NULL; // game can exist with this file

	char textFileName[100];

	// When clustered the session files have the base stripped
	strcpy(temp_buf, "text");

	buf_hash = HashString(temp_buf);
	if (private_session_resman->Test_file(temp_buf, buf_hash, session_cluster, session_cluster_hash)) {
		// Jake : so PSX can have nice session loading screen and details (for timing and to stop player getting bored)
		LoadMsg("Session Text");

		// Special text loading code so the translators can test their stuff

		if (tt) {
			// Ok, translators mode has been activated
			text = LoadTranslatedFile(mission, session_name);
		} else
			text = (_linked_data_file *)private_session_resman->Res_open(temp_buf, buf_hash, session_cluster, session_cluster_hash);
	} else
		Fatal_error("Missing Text File \"%s\"", temp_buf);

	Tdebug("session.txt", "text lines END");

	// A copy of the code above to open the global text file.  Feel free to edit this if I've ballsed up
	// anywhere.
	global_text = NULL;

	char global_cluster[ENGINE_STRING_LEN];

	strcpy(global_cluster, GLOBAL_CLUSTER_PATH);

	uint32 global_cluster_hash = HashString(global_cluster);

	sprintf(textFileName, GLOBAL_TEXT_FILE);

	buf_hash = HashString(textFileName);

	if (private_session_resman->Test_file(textFileName, buf_hash, global_cluster, global_cluster_hash)) {
		LoadMsg(temp_buf);

		if (tt) {
			// Ok, translators mode has been activated
			global_text = LoadTranslatedFile("global", "global\\global\\");
		} else
			global_text = (_linked_data_file *)private_session_resman->Res_open(textFileName, buf_hash, global_cluster, global_cluster_hash);

	} else {
		Fatal_error("Failed to find global text file [%s][%s]", textFileName, global_cluster);
	}

	// The surface manager needs to know what colour to use for transparency.  This comes from a fixed
	// reference file which is opened here so the global reference can be set.
	g_oIconMenu->SetTransparencyColourKey();

	// Initialise the remora
	g_oRemora->InitialiseRemora();

	// Set the default colour for voice over text.
	voice_over_red = VOICE_OVER_DEFAULT_RED;
	voice_over_green = VOICE_OVER_DEFAULT_GREEN;
	voice_over_blue = VOICE_OVER_DEFAULT_BLUE;

	Tdebug("session.txt", "CHI START");
	// chi
	// players movement history
	cur_history = 0;
	chi_think_mode = __NOTHING;

	is_there_a_chi = FALSE8;

	Tdebug("session.txt", "walkareas START");

	// setup walkareas
	total_was = 0;

	// When clustered the session files have the base stripped
	strcpy(temp_buf, "walkarea");

	buf_hash = HashString(temp_buf);

	// Jake : so PSX can have nice session loading screen and details (for timing and to stop player getting bored)
	LoadMsg("Session Walkareas");
	uint32 len = private_session_resman->Check_file_size(temp_buf, buf_hash, session_cluster, session_cluster_hash);

	if (len) {
		walk_areas = (_linked_data_file *)private_session_resman->Res_open(temp_buf, buf_hash, session_cluster, session_cluster_hash);
		Tdebug("walkareas.txt", "%d top level walkareas\n", walk_areas->Fetch_number_of_items());

		int nMissing = 0;
		for (uint32 k = 0; k < walk_areas->Fetch_number_of_items(); k++) {
			INTEGER_WalkAreaFile *inner_wa;
			inner_wa = (INTEGER_WalkAreaFile *)walk_areas->Fetch_item_by_number(k);

			Tdebug("walkareas.txt", "\nclump %d has %d inner items", k, inner_wa->GetNoAreas());

			for (j = 0; j < inner_wa->GetNoAreas(); j++) {
				const __aWalkArea *wa;

				wa = inner_wa->GetWalkArea(j);

				wa_list[total_was++] = wa; // write entry to individual item list

				if (total_was == MAX_was)
					Fatal_error("total number of walk-areas exceeded - %d", MAX_was);
			}
		}
		if (nMissing > 0)
			Fatal_error("%d missing cameras : Game must terminate", nMissing);

		Tdebug("walkareas.txt", "\n%d individual walk areas found", total_was);

	} else
		Tdebug("walkareas.txt", "no walkarea file");

	number_of_missing_objects = 0; // start with no missing objects

	// structure assignment counters - see fn_create_mega
	num_megas = 0;
	num_vox_images = 0;

	// init conveyors
	for (j = 0; j < MAX_conveyors; j++)
		conveyors[j].moving = FALSE8;

	// init auto interact
	for (j = 0; j < MAX_auto_interact; j++)
		auto_interact_list[j] = 0; // no entry

	// stairs
	num_stairs = 0;

	// lifts
	num_lifts = 0;

	// turn off health bar
	health_time = 0;

	// setup generic asyncer
	player_stat_was = __TOTAL_PLAYER_MODES; // for 'prev' check
	player_stat_use = __TOTAL_PLAYER_MODES; // for 'prev' check
	async_counter = 0; // counts up each frame
	async_off = 0;     // on by default

	// first cycle indicator
	first_session_cycle = TRUE8;

	Tdebug("session.txt", "session constructor END");
}

void _game_session::Script_version_check() {
	if (FN_ROUTINES_DATA_VERSION != scripts->GetHeaderVersion())
		Fatal_error("WARNING! SCRIPTS AND ENGINE ARE NOT SAME VERSION");
}

void _game_session::___destruct() {
	// session object deconstructor
	// kills the current session and removes its resources

	Zdebug("*session destructing*");

	// turn off all sounds
	StopAllSoundsNow();

	Zdebug("sounds stopped");

	// camview mode
	if (camera_hack == TRUE8) {
		SetReset();
		return;
	}

	// trash resources
	private_session_resman->Reset();

	// remove diag bars that have been newed
	for (uint32 j = 0; j < total_objects; j++)
		if (logic_structs[j]->mega)
			logic_structs[j]->mega->m_main_route.___init(); // delete diag bars

	// delete current set view
	SetReset();

	// delete player object
	//	delete   player;

	// speech
	//	if   (text_bloc)
	//		delete  text_bloc;  //in-case quit during speech
}

void _game_session::Initialise_set(const char *name, const char *cluster_name) {
	// initialise a set object for the current session
	// stick us into TEMP_NETHACK mode if the set does not physically exist

	// place a split point/boundary into the resource loading
	rs_bg->Advance_time_stamp();

	// init the set
	set.Init(name, cluster_name);

	// decide which props to sleep and which to wake
	Setup_prop_sleep_states();
}

void _game_session::Setup_prop_sleep_states() {
	// initialise the new set object

	for (uint32 j = 0; j < total_objects; j++)
		if (!logic_structs[j]->mega) { // props
			logic_structs[j]->prop_on_this_screen = set.DoesPropExist((const char *)logic_structs[j]->GetName());

			if (logic_structs[j]->hold_mode == prop_camera_hold) {
				if (!logic_structs[j]->prop_on_this_screen) {
					logic_structs[j]->camera_held = TRUE8; // not on screen
					logic_structs[j]->cycle_time = 0; // accurate for displays
				} else {
					logic_structs[j]->camera_held = FALSE8; // on screen
				}
			} else { // not an auto sleep item, but...
				//				check for sleeping items that are now on screen
				if ((logic_structs[j]->camera_held) && (logic_structs[j]->prop_on_this_screen)) { // held AND on screen (a door)
					logic_structs[j]->camera_held = FALSE8; // on screen and not a camera_hold_mode item so wake it up - i.e. doors
				}
			}
		}
}

void _game_session::Awaken_doors() {
	// called to release doors when entering remora mode

	for (uint32 j = 0; j < total_objects; j++)
		if ((logic_structs[j]->big_mode == __CUSTOM_BUTTON_OPERATED_DOOR) || (logic_structs[j]->big_mode == __CUSTOM_AUTO_DOOR)) {
			logic_structs[j]->camera_held = FALSE8; // awake!
			logic_structs[j]->prop_on_this_screen = TRUE8;
		}
}

__mega_set_names player_startup_anims[] = {__STAND, __STAND_TO_WALK, __WALK, __WALK_TO_STAND, __TURN_ON_THE_SPOT_CLOCKWISE};
#define NUMBER_player_startup_anims 5

void _game_session::Init_objects() {
	char buf[ENGINE_STRING_LEN];
	uint32 j, id;

	if (!g_mission->inited_globals) {
		// init local globals
		// only do this at start of mission - never again afterward - i.e. not when returning to first session from another
		uint32 script_hash;

		id = objects->Fetch_item_number_by_name("player"); // returns -1 if object not in existence
		if (id == 0xffffffff)
			Fatal_error("Init_objects cant find 'player'");
		script_hash = HashString("player::globals");
		const char *pc = (const char *)scripts->Try_fetch_item_by_hash(script_hash);
		if (pc) {
			object = (c_game_object *)objects->Fetch_item_by_number(id);
			Tdebug("objects_init.txt", " initialising globals", (const char *)buf);
			RunScript(pc, object);
		}

		g_mission->inited_globals = TRUE8;
	}

	Zdebug("\nInitialise_objects");

	// so PSX can have nice session loading screen and details (for timing and to stop player getting bored)
	// this stuff moved here when ammo, bullets etc moved into player struct
	InitMsg("Player");
	// create a player object
	player.___init();

	// Now run the InitScript for each object.  This has to be done after the line-of-sight end
	// event manager have been initialised in case calls get made to these services in any of the
	// objects' InitScripts.
	for (j = 0; ((j < total_objects)); j++) {
		object = (c_game_object *)objects->Fetch_item_by_number(j);
		Tdebug("objects_init.txt", "\n\n---------------------------------------------------\n%d  initialising object '%s'", j, object->GetName());
		Zdebug("\n\n---------------------------------------------------\n%d  initialising object '%s'", j, object->GetName());

		Zdebug("[%d]", num_megas);

		// fast reference for engine functions
		// needed incase structs are referenced by FN_ functions
		cur_id = j;

		// set L for FN_ functions that may be called
		L = logic_structs[j];
		I = L->voxel_info;
		M = L->mega;

		// possibly run the init OR logic context script to kick in a base script ready for running in the logic loop
		// the init script is always script 0 for the object
		// the init script may or may not be overiden
		// get the address of the script we want to run
		const char *pc = (const char *)scripts->Try_fetch_item_by_hash(object->GetScriptNameFullHash(OB_INIT_SCRIPT)); // run init script

		if (pc) {
			RunScript(pc, object);

			strcpy(buf, object->GetName());
			strcat(buf, "::local_init");

			uint32 script_hash;

			script_hash = HashString(buf);

			// Jso PSX can have nice session loading screen and details (for timing and to stop player getting bored)
			InitMsg(object->GetName());

			Tdebug("objects_init.txt", "search for [%s]", (const char *)buf);

			pc = (const char *)scripts->Try_fetch_item_by_hash(script_hash);

			if (pc) {
				//              set M and I for FN_ functions that may be called

				I = L->voxel_info;
				M = L->mega;

				Tdebug("objects_init.txt", " running optional = [%s]", (const char *)buf);
				RunScript(pc, object);
			} else
				Tdebug("objects_init.txt", " no [%s] found", (const char *)buf);

			//          setup logic context
			//          set to start on level 0
			logic_structs[j]->logic_level = 0;

			//          set base logic to logic context script
			logic_structs[j]->logic[0] = (char *)scripts->Try_fetch_item_by_hash(object->GetScriptNameFullHash(OB_LOGIC_CONTEXT));
			//          **note, we dont need to set up the script reference (logic_ref) for level 0
		} else
			Shut_down_object("by initialise - no init script");

		L = logic_structs[j];
		I = L->voxel_info;
		if (L->image_type == VOXEL) {
			for (uint i = 0; i < __NON_GENERIC; i++) {
				if (I->IsAnimTable(i)) {
#ifdef PRELOAD
					rs_anims->Res_open(I->info_name[i], I->info_name_hash[i], I->base_path, I->base_path_hash);
#endif
				}
			}
		}

	}

	Tdebug("objects_init.txt", "\n\n\ncreating mega list");
	// create voxel id list
	number_of_voxel_ids = 0;
	for (j = 0; j < total_objects; j++) { // object 0 is used
		//		object must be alive and interactable and a mega
		if ((logic_structs[j]->image_type == VOXEL) && (logic_structs[j]->ob_status != OB_STATUS_HELD)) { // not if the object has been manually switched out
			Tdebug("objects_init.txt", "%s", (const char *)logic_structs[j]->GetName());
			voxel_id_list[number_of_voxel_ids++] = (uint8)j;
		}
	}

	if (number_of_voxel_ids >= MAX_voxel_list)
		Fatal_error("Initialise_objects, the voxel id list is too small");

	Tdebug("objects_init.txt", "\n\nfound %d voxel characters", number_of_voxel_ids);

	// init the player object number
	// get id
	id = objects->Fetch_item_number_by_name("player"); // returns -1 if object not in existence

	if (id != 0xffffffff) {
		L = logic_structs[id]; // fetch logic struct for player object
		I = L->voxel_info;
		M = L->mega;

		object = (c_game_object *)objects->Fetch_item_by_number(id);

		//		not if this object has been shut-down - for not having a map marker for example
		if (L->ob_status != OB_STATUS_HELD)
			player.Set_player_id(id);

		// Preload the player animation to make PSX jerking better
		for (uint i = 0; i < NUMBER_player_startup_anims; i++)
			rs_anims->Res_open(I->get_anim_name(player_startup_anims[i]), I->anim_name_hash[player_startup_anims[i]], I->base_path, I->base_path_hash);
	}

	// done
	Zdebug("Init session finished\n");

	// so PSX can have nice session loading screen and details (for timing and to stop player getting bored)
	EndLoading();
}

void _game_session::Pre_initialise_objects() {
	// prepare gameworld and objects but dont run init scripts yet

	// so PSX can have nice session loading screen and details (for timing and to stop player getting bored)
	StartInit(total_objects + 6); // +6 because also floors, barriers, markers, camera_table, plan_view, player

	Zdebug("\nPre_Initialise_objects");

	Zdebug("[%d]", num_megas);

	// so PSX can have nice session loading screen and details (for timing and to stop player getting bored)
	InitMsg("Floors");

	// initialise the floor area definition file
	// uses the private resman so mission->session-> need to be initialised so this cant be on session contructor
	floor_def = g_icb_session_floors;
	g_icb_session_floors->___init();

	// so PSX can have nice session loading screen and details (for timing and to stop player getting bored)
	InitMsg("Barriers");

	// initialise the route barriers
	session_barriers = &g_icb_session_barriers;
	g_icb_session_barriers.___init();

	Zdebug("A[%d]", num_megas);

	// so PSX can have nice session loading screen and details (for timing and to stop player getting bored)
	InitMsg("Markers");

	// init engine markers
	markers.___init();

	// so PSX can have nice session loading screen and details (for timing and to stop player getting bored)
	InitMsg("Cameras");

	// setup the camera system
	Build_camera_table();

	// First set up the logic structures for the objects.  Each begins with a NULL pointer for its vox image.
	uint32 j;
	for (j = 0; ((j < total_objects)); ++j) {

		Zdebug("%d -[%d]", j, num_megas);

		object = (c_game_object *)objects->Fetch_item_by_number(j);

		logic_structs[j] = &logics[j];
		logic_structs[j]->___init((const char *)object->GetName());
	}

	// Set up the event manager for this session.  This has to be done after the barrier handler
	// has been set up and after the objects have been set up because it relies on information
	// from both.
	PXTRY

	g_oEventManager->Initialise();

	PXCATCH

	Fatal_error("Exception in _event_manager::Initialise()");

	PXENDCATCH

	// And set a duty cycle for the line-of-sight manager.  Just set default for now.

	PXTRY

	Zdebug("duty");
	g_oLineOfSight->SetDutyCycle(1);
	Zdebug("~duty");

	PXCATCH

	Fatal_error("Exception in g_oLineOfSight->SetDutyCycle()");

	PXENDCATCH

	// Initialise the sound logic engine.
	g_oSoundLogicEngine->Initialise();

	player.has_weapon = TRUE8; // called before players init script
}

extern int32 john_number_traces;
extern int32 john_total_traces;

extern int32 fnTimer;
int32 logicTimer;
uint32 script_cycleTimer;

void _game_session::One_logic_cycle() {
	// process all the game objects

	uint32 j;

	// Wind the line-of-sight engine on one position.  Note that there is a variable in the line-of-sight engine
	// which can be used to make this call return without doing anything a certain percentage of the time.

	john_number_traces = 0;
	john_total_traces = 0;

	uint32 time = GetMicroTimer();
	PXTRY
	g_oLineOfSight->DutyCycle();
	PXCATCH
	Fatal_error("Exception in g_oLineOfSight->DutyCycle()");
	PXENDCATCH
	time = GetMicroTimer() - time;
	g_mission->los_time = time;

	time = GetMicroTimer();
	PXTRY
	g_oSoundLogicEngine->Cycle();
	PXCATCH
	Fatal_error("Exception in g_oSoundLogicEngine->Cycle()");
	PXENDCATCH
	time = GetMicroTimer() - time;
	g_mission->sound_time = time;

	time = GetMicroTimer();

	// service the speech driver
	Service_speech();

	// reset route manager
	Start_new_router_game_cycle();

	// Tell the event manager to process its event timers if it is currently maintaining any.
	g_oEventManager->CycleEventManager();

	// If the icon menu currently has a holding icon, service its logic.
	if (g_oIconListManager->IsHolding())
		g_oIconListManager->CycleHoldingLogic();

	// If the icon menu is currently flashing added medipacks or clips run the logic for it.
	if (g_oIconMenu->IsAdding())
		g_oIconMenu->CycleAddingLogic();

	time = GetMicroTimer() - time;
	g_mission->event_time = time;

	// run through all the objects calling their logic
	for (j = 0; j < total_objects; j++) { // object 0 is used

		//		fetch the engine created logic structure for this object
		L = logic_structs[j];

		if ((L->ob_status != OB_STATUS_HELD) && (!L->camera_held)) { // not if the object has been manually switched out
			I = L->voxel_info;
			M = L->mega;
			cur_id = j; // fast reference for engine functions
			// fetch the object that is our current object
			// 'object' needed as logic code may ask it for the objects name, etc.
			object = (c_game_object *)objects->Fetch_item_by_number(j);

			// run appropriate logic
			switch (L->big_mode) {
			case __SCRIPT: // just running full scripts
				if (px.mega_timer)
					script_cycleTimer = GetMicroTimer();
				Pre_logic_event_check();
				Script_cycle();
				if (px.mega_timer) {
					script_cycleTimer = GetMicroTimer() - script_cycleTimer;
					L->cycle_time = script_cycleTimer;
				}
				break;

			case __NO_LOGIC: // do nothing
				break;

			case __CUSTOM_SIMPLE_ANIMATE: // special simple animator logic
				if (px.mega_timer)
					script_cycleTimer = GetMicroTimer();
				Custom_simple_animator();
				if (px.mega_timer) {
					script_cycleTimer = GetMicroTimer() - script_cycleTimer;
					L->cycle_time = script_cycleTimer;
				}

				break;

			case __CUSTOM_BUTTON_OPERATED_DOOR: // special button operated door
				if (px.mega_timer)
					script_cycleTimer = GetMicroTimer();
				Custom_button_operated_door();
				if (px.mega_timer) {
					script_cycleTimer = GetMicroTimer() - script_cycleTimer;
					L->cycle_time = script_cycleTimer;
				}

				break;

			case __CUSTOM_AUTO_DOOR:
				if (px.mega_timer)
					script_cycleTimer = GetMicroTimer();
				Custom_auto_door();
				if (px.mega_timer) {
					script_cycleTimer = GetMicroTimer() - script_cycleTimer;
					L->cycle_time = script_cycleTimer;
				}
				break;

			case __MEGA_SLICE_HELD:
				if (M->on_players_floor) {
					L->big_mode = __SCRIPT; // in view - alive
					g_oEventManager->ClearAllEventsForObject(cur_id);
					g_oSoundLogicEngine->ClearHeardFlag(cur_id);
					Script_cycle();
				}

				if (PXfabs(M->actor_xyz.y - logic_structs[player.Fetch_player_id()]->mega->actor_xyz.y) < (int)(M->slice_hold_tolerance)) {
					L->big_mode = __SCRIPT;
					g_oEventManager->ClearAllEventsForObject(cur_id);
					g_oSoundLogicEngine->ClearHeardFlag(cur_id);
					Script_cycle();
				}
				break;
			case __MEGA_PLAYER_FLOOR_HELD:
			case __MEGA_INITIAL_FLOOR_HELD:
				//					waiting for player - release when on our floor
				if (M->on_players_floor) {
					L->big_mode = __SCRIPT;
					g_oEventManager->ClearAllEventsForObject(cur_id);
					g_oSoundLogicEngine->ClearHeardFlag(cur_id);
					Script_cycle();
				} else if (first_session_cycle)
					Script_cycle(); // no camera yet
				break;
			}

			// if the character is voxel based then add it to the list of voxel characters that is used by stage draw
			if ((L->image_type == VOXEL) && ((L->ob_status != OB_STATUS_HELD))) {
				time = GetMicroTimer();

				// update some mega related stuff if mega is not sleeping
				if (L->big_mode == __SCRIPT) {
					//              process outstanding auto pan remaining
					if (L->auto_panning)
						Advance_auto_pan();

					Idle_manager(); // heh heh, check for mega just stood around

					UpdateFootstep();
					UpdateMegaFX();
				}

				// set floor rect value - used by stage draw to find indexed camera name
				floor_def->Set_floor_rect_flag(L);

				// check player floor status
				if (!first_session_cycle)
					Process_player_floor_status(); // sends events when object gains same floor as player

				// async
				if (!M->dead)
					Service_generic_async(); // servive live megas

				// process hold mode options
				// has on-camera-only mode and is not on screen - then sleep again
				if ((L->hold_mode == mega_player_floor_hold) && (!M->on_players_floor))
					L->big_mode = __MEGA_PLAYER_FLOOR_HELD;

				// has slice hold mode and is now off camera - check for shut off tolerance
				if ((L->hold_mode == mega_slice_hold) && (!M->on_players_floor)) {
					if (PXfabs(M->actor_xyz.y - logic_structs[player.Fetch_player_id()]->mega->actor_xyz.y) > (int)(M->slice_hold_tolerance))
						L->big_mode = __MEGA_SLICE_HELD;
				}

				// if on screen and stood then lets maybe do a scratch anim

				time = GetMicroTimer() - time;
				g_mission->xtra_mega_time += time;
			}
		}
	}

	// set not first cycle
	first_session_cycle = FALSE8;
}

void _game_session::Pre_logic_event_check() {
	//  Check if there any events pending for the object.
	// if  so we re-run the logic context which may or may not decide to change the current script

	if (((L->do_not_disturb == 1)) || (L->do_not_disturb == 2))
		return; // 1 or 2 == fn-do-not-disturb + speech

	if (L->do_not_disturb == 3) { // socket_force_new_logic
		// clear events
		g_oEventManager->ClearAllEventsForObject(cur_id);

		L->do_not_disturb = 0; // reset for next cycle
		return;
	}

	// Added call to the sound logic engine to see if any sound events are pending.
	if (L->context_request || g_oEventManager->HasEventPending(cur_id) || g_oSoundLogicEngine->SoundEventPendingForID(cur_id)) {
		//      Yes, the object has an event pending, so rerun its logic context.
		if (L->context_request)
			Zdebug("[%s] internal request to rerun logic context", object->GetName());

		else
			Zdebug("[%s] event means rerun logic context", object->GetName());

		if ((L->image_type == VOXEL) && (M->interacting)) { // check for megas who are interacting
			// interacting, so ignoring LOS event
			Zdebug("interacting, so ignoring LOS event");
		} else {
			L->logic[0] = (char *)scripts->Try_fetch_item_by_hash((object->GetScriptNameFullHash(OB_LOGIC_CONTEXT)));

			// run script - context chooser MAY pick a new L 1 logic
			// we call this now so the new script will be setup and ready to run
			RunScript(const_cast<const char *&>(L->logic[0]), object);

			// reset the crude switch
			L->context_request = FALSE8;
		}
	}
}

void _game_session::Script_cycle() {
	int32 ret;
	c_game_object *script_owner;
	uint32 inner_cycles;

	inner_cycles = 0; // to catch infnite_loops

	// inner logic loop
	do {
		// sort out which object we should run the script with
		// this can change within a cycle
		if ((L->image_type == VOXEL) && (M->interacting)) { // check for megas who are interacting
			// object is running someone elses interaction script
			// so get their object and pass to interpretter so that local vars can be accessed correctly
			script_owner = (c_game_object *)objects->Fetch_item_by_number(M->target_id);
		} else {
			script_owner = object; // object running its own script
		}

		ret = RunScript(const_cast<const char *&>(L->logic[L->logic_level]), script_owner);

		// ret is:
		// 0 done enough this cycle
		// 1 current script has finished and hit closing brace
		// 2 FN_ returned an IR_TERMINATE to interpretter so we just go around - new script or gosub

		if (ret == IR_RET_SCRIPT_FINISHED) { // script has finished so drop down a level
			if (L->logic_level) { // not on base, so we can just drop down to the script below
				L->logic_level--;

				// in-case we were running an interaction script then cancel interaction
				if (L->image_type == VOXEL) {
					M->target_id = 0;
					M->interacting = FALSE8;
					L->looping = L->old_looping; // safe for return
				}
			}

			if (!L->logic_level) { // restart the object
				L->logic_ref[1] = 0; // completely reset L 1 so that context choose will select ok
				// it is acceptable to choose the logic that had previously been running

				// temp reset PC the hard way
				L->logic[0] = (char *)scripts->Try_fetch_item_by_hash(object->GetScriptNameFullHash(OB_LOGIC_CONTEXT));

				// run script - context chooser will pick a new L 1 logic
				RunScript(const_cast<const char *&>(L->logic[0]), object);

				// if still on base then nothing chosen - shut down the object and log a warning somewhere
				if (!L->logic_level) {
					Shut_down_object("by One_logic_cycle - logic context failed to choose");
					ret = 0;
				}
			}
		}

		//		catch infinite loops
		inner_cycles++;

		// I upped this from 5 to 1000 because the Remora relies on fn_new_script to jump around
		// it's menu structure and this was tripping the limit.  The player will have to spend ages
		// in the Remora's menu's now for it to trip this limit, and genuine infinite loops will
		// still be caught.
		if (inner_cycles == 1000)
			Fatal_error("object [%s] is in an infinite script loop!", object->GetName());

	} while (ret); // ret==0 means quit for this object
}

uint32 _game_session::Fetch_prop_state(char *prop_name) {
	// return a props state
	// if the prop object doesnt exist we create a dummy - the system continues regardless - which is nice

	uint32 prop_number;
	uint32 j;

	if (camera_hack == FALSE8) {
		prop_number = objects->Fetch_item_number_by_name(prop_name);

		if (prop_number != 0xffffffff)
			return (prop_state_table[prop_number]); // get prop state (pc)
	}

	// prop does not have a owner object - so we create a dummy for it if we havent already
	// so, have we already created a dummy?
	// search for dummy

	// is our object already here?
	j = 0;
	while ((j < number_of_missing_objects) && (strcmp(missing_obs[j], prop_name)))
		++j;

	// didnt find the object
	if (j == number_of_missing_objects) {
		// create entry for the object
		if (strcmp(prop_name, "not a prop") && (camera_hack == FALSE8)) // dont report dummy lights
			Message_box("object missing for prop [%s]", prop_name);

		Set_string(prop_name, missing_obs[number_of_missing_objects], MAX_missing_object_name_length);

		Tdebug("missing_objects.txt", "%d [%s]", number_of_missing_objects, missing_obs[number_of_missing_objects]);
		missing_ob_prop_states[number_of_missing_objects++] = 0;

		return (0);
	}

	// dummy did exist so return the current dummy prop value
	return (missing_ob_prop_states[j]);
}

void _game_session::Set_prop_state(char *prop_name, uint32 value) {
	// set a prop state
	// if the prop doesnt exist we skip it - and assume it will soon be built
	// there is no scope checking

	uint32 prop_number;
	uint32 j;

	if (camera_hack == FALSE8) {
		prop_number = objects->Fetch_item_number_by_name(prop_name);

		if (prop_number != 0xffffffff)
			prop_state_table[prop_number] = value; // set prop state (pc)
	}

	// have we already created a dummy?
	// search for dummy

	// is our object already here?
	j = 0;
	while ((j < number_of_missing_objects) && (strcmp(missing_obs[j], prop_name)))
		++j;

	// didnt find the object
	if (j == number_of_missing_objects)
		return;

	// found the dummy so set its value
	missing_ob_prop_states[j] = (uint8)value;
}

uint32 _game_session::Fetch_named_objects_id(const char *name) const {
	uint32 i;

	for (i = 0; i < total_objects; ++i) {
		if (strcmp(name, logic_structs[i]->GetName()) == 0)
			return (i);
	}

	// The object wasn't found.
	Fatal_error("Object %s not found in _game_session::Fetch_named_objects_id()", name);

	// This stops a compiler error.
	return (0xffffffff);
}

void _game_session::Process_player_floor_status() {
	// work out if this object
	bool8 result = FALSE8;
	uint32 num_extra, j, cam, player_floor;

	player_floor = logic_structs[player.Fetch_player_id()]->owner_floor_rect;

	// dont need to tell the player he's on the players floor
	if (player.Fetch_player_id() == cur_id)
		return;

	if (floor_to_camera_index[L->owner_floor_rect] == cur_camera_number)
		result = TRUE8; //
	else { // no exact same
		// ok, but is our floor linked to theirs?
		cam = floor_to_camera_index[player_floor];
		num_extra = cam_floor_list[cam].num_extra_floors;

		for (j = 0; j < num_extra; j++)
			if (cam_floor_list[cam].extra_floors[j] == L->owner_floor_rect) { // our floor one of players extras?
				result = TRUE8; // yes - the floors are linked
				break;
			}
	}

	if ((!M->on_players_floor) && (result)) {
		g_oEventManager->PostNamedEventToObject("on_floor", cur_id, player.Fetch_player_id()); // send as if from player
	}

	M->on_players_floor = result; // set to current state
}

void _game_session::Idle_manager() {
	// megas only
	// is the character idling?
	// if so and for a int32 time gosub a script that will do something

	uint32 k;
	char *ad;
	uint32 script_hash;

	if ((L->pause) && (L->cur_anim_type == __STAND) && (L->conversation_uid == NO_SPEECH_REQUEST) && (!M->Is_crouched()) && (Object_visible_to_camera(cur_id))) {

		M->idle_count++;

		if ((M->idle_count > 24) && (L->logic_level == 1)) {
			M->idle_count = 0;

			script_hash = HashString("idle");

			// try and find a script with the passed extention i.e. ???::looping
			for (k = 0; k < object->GetNoScripts(); k++) {
				if (script_hash == object->GetScriptNamePartHash(k)) {
					//          script k is the one to run
					//          get the address of the script we want to run
					ad = (char *)scripts->Try_fetch_item_by_hash(object->GetScriptNameFullHash(k));

					//          write actual offset
					L->logic[2] = ad;

					L->logic_level = 2; //

					L->old_looping = L->looping; // safe for return

					L->looping = 0; // reset to 0 for new logics

					M->custom = FALSE8; // reset

					return; // done it
				}
			}
		}
	}
}

void _game_session::Set_init_voxel_floors() {
	// set all mega characters floors - called after game restore because logics may begin by checking the floor number but it wont be set until end of first cycle
	uint32 j;

	for (j = 0; j < number_of_voxel_ids; j++)
		floor_def->Set_floor_rect_flag(logic_structs[voxel_id_list[j]]);

	// setup the players route barriers

	MS->M = MS->logic_structs[MS->player.Fetch_player_id()]->mega;
	MS->L = MS->logic_structs[MS->player.Fetch_player_id()];
	Prepare_megas_route_barriers(TRUE8); // update barriers
}

_linked_data_file *LoadTranslatedFile(cstr mission, cstr session) {
	// Get the actual session name
	cstr sessionstart = session + strlen(mission) + 1;
	pxString actsession;
	actsession.SetString(sessionstart, strlen(sessionstart) - 1);

	// Make up the name for the file to be loaded up
	pxString fname = pxVString("%s\\data\\%s%s.ttrans", tt_text, mission, (const char *)actsession);

	if (!checkFileExists(fname))
		Fatal_error("Unable to load file %s", (const char *)fname);

	// Load in this file
	Common::SeekableReadStream *stream = openDiskFileForBinaryStreamRead(fname.c_str());

	if (stream == NULL) // if it could not be opened
		Fatal_error("Unable to load file %s", (const char *)fname);

	uint len = stream->size();

	//      make space for file
	char *mem = new char[len + 1];

	//      read it in
	stream->read(mem, len);
	delete stream; // close the file

	//      0 terminate the string
	mem[len] = 0;

	return ((_linked_data_file *)mem);
}

} // End of namespace ICB
