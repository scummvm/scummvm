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

#include "engines/icb/keyboard.h"
#include "engines/icb/mouse.h"
#include "engines/icb/debug.h"
#include "engines/icb/res_man.h"
#include "engines/icb/options_manager_pc.h"
#include "engines/icb/p4.h"
#include "engines/icb/global_vars.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_bitmap.h"
#include "engines/icb/stage_draw.h"
#include "engines/icb/mission.h"
#include "engines/icb/cluster_manager_pc.h"
#include "engines/icb/configfile.h"
#include "engines/icb/icb.h"

#include "common/str.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "common/events.h"

namespace ICB {

bool gRegainedFocus = false;
bool gotTheFocus = false;

char g_characters[] = "characters\\";
char gamelanguage[ENGINE_STRING_LEN] = "english";
bool8 camera_hack;
uint32 BACKGROUND_BUFFER_SIZE;
uint32 ANIMATION_BUFFER_SIZE;
uint32 BITMAP_BUFFER_SIZE;
uint32 SONICS_BUFFER_SIZE;

// private session data (scripts+objects+walkgrids) : 200KB
#define PRIVATE_RESMAN_SIZE (400 * 1024)

uint32 getConfigValueWithDefault(const ConfigFile &config, const Common::String &section, const Common::String &key, uint32 defaultValue) {
	if (scumm_stricmp("MusicVolume", key.c_str()) == 0) {
		return ConfMan.getInt("music_volume") / 2;
	} else if (scumm_stricmp("SpeechVolume", key.c_str()) == 0) {
		return ConfMan.getInt("speech_volume") / 2;
	} else if (scumm_stricmp("SfxVolume", key.c_str()) == 0) {
		return ConfMan.getInt("sfx_volume") / 2;
	} else if (scumm_stricmp("Game Completed", key.c_str()) == 0) {
		if (ConfMan.hasKey("game_completed"))
			return ConfMan.getBool("game_completed") ? 1 : 0;
		else
			return 0;
	} else if (scumm_stricmp("Subtitles", key.c_str()) == 0) {
		if (ConfMan.hasKey("subtitles"))
			return (uint32)ConfMan.getBool("subtitles");
	} else if (scumm_stricmp("Movie Library", section.c_str()) == 0) {
		Common::String movie = Common::String("movie_") + key;
		if (ConfMan.hasKey(movie))
			return (uint32)ConfMan.getBool(movie);
		else {
			uint32 result = config.readIntSetting(section, key, defaultValue);
			ConfMan.setBool(movie, result != 0);
		}
	} else if (scumm_stricmp("Controller Settings", section.c_str()) == 0 &&
		   scumm_stricmp("Method", key.c_str()) == 0) {
		if (ConfMan.hasKey("actor_relative"))
			return ConfMan.getBool("actor_relative") ? 0 : 1;
		else {
			uint32 result = config.readIntSetting(section, key, defaultValue);
			ConfMan.setBool("actor_relative", result == 0);
		}
	}

	return config.readIntSetting(section, key, defaultValue);
}

void ReadConfigFromIniFile() {
	char configFile[1024];
	uint32 temp;

	if (g_icb->getGameType() == GType_ICB)
		Common::sprintf_s(configFile, "engine\\icb.ini");
	else if (g_icb->getGameType() == GType_ELDORADO)
		Common::sprintf_s(configFile, "engine\\eldorado.ini");
	else
		assert(false);

	ConfigFile config;
	pxString filename = configFile;
	filename.ConvertPath();
	config.readFile(filename.c_str());

	// Music volume
	SetMusicVolume(getConfigValueWithDefault(config, "Option Settings", "MusicVolume", 127));

	// Speech volume
	SetSpeechVolume(getConfigValueWithDefault(config, "Option Settings", "SpeechVolume", 127));

	// Sfx volume
	SetSfxVolume(getConfigValueWithDefault(config, "Option Settings", "SfxVolume", 127));

	// Has the game been completed previously
	temp = getConfigValueWithDefault(config, "Extras", "Game Completed", 0);
	// HACK: Enable all extras for now
	warning("Enabling all extras for development purposes");
	temp = 1;
	if (temp == 0)
		g_px->game_completed = FALSE8;
	else
		g_px->game_completed = TRUE8;

	// Subtitle switch
	temp = getConfigValueWithDefault(config, "Video Settings", "Subtitles", 1);
	if (temp == 0)
		g_px->on_screen_text = FALSE8;
	else
		g_px->on_screen_text = TRUE8;

	// Control method
	temp = getConfigValueWithDefault(config, "Controller Settings", "Method", ACTOR_RELATIVE);
	if ((__Actor_control_mode)temp == SCREEN_RELATIVE)
		g_icb_session->player.Set_control_mode(SCREEN_RELATIVE);
	else
		g_icb_session->player.Set_control_mode(ACTOR_RELATIVE);

	// Set the default keys first in case the ini file mappings are invalid somehow
	SetDefaultKeys();

	// Read the movie library settings
	for (uint32 i = 0; i < TOTAL_NUMBER_OF_MOVIES; i++) {
		temp = config.readIntSetting("Movie Library", pxVString("%X", HashString(g_movieLibrary[i].filename)), 0);
		if (temp == 0)
			g_movieLibrary[i].visible = FALSE8;
		else
			g_movieLibrary[i].visible = TRUE8;
	}
}

void Save_config_file() {
	ConfMan.setInt("music_volume", GetMusicVolume() * 2);
	ConfMan.setInt("speech_volume", GetMusicVolume() * 2);
	ConfMan.setInt("sfx_volume", GetMusicVolume() * 2);
	ConfMan.setBool("subtitles", g_px->on_screen_text != 0);
	ConfMan.setBool("game_completed", g_px->game_completed);
	ConfMan.setBool("actor_relative", g_icb_session->player.Get_control_mode() == ACTOR_RELATIVE);

	// Write the movie library settings
	for (uint32 i = 0; i < TOTAL_NUMBER_OF_MOVIES; i++) {
		// Only write a setting when it's been achieved
		if (g_movieLibrary[i].visible) {
			char temp[1024];
			Common::sprintf_s(temp, "%X", HashString(g_movieLibrary[i].filename));
			Common::String movie = Common::String("movie_") + temp;
			ConfMan.setBool(movie, true);
		}
	}

	ConfMan.flushToDisk();
}

void InitEngine(const char *lpCmdLine) {
	CreateGlobalObjects();

	// Set the character root directory to be the same as the normal root directory
	camera_hack = false; // defaults to off

	// create the surface manaager
	surface_manager = new _surface_manager;

	// Init the low level direct draw object
	surface_manager->Init_direct_draw();

	// Init the sound engine
	if (!Init_Sound_Engine()) {
		Message_box("Sound device unavailable.");
		Zdebug("Failed to start the sound engine");
		noSoundEngine = TRUE8;
	}

	// See how much memory we have in this machine
	Memory_stats();

	// Animations
	rs1 = new res_man(ANIMATION_BUFFER_SIZE);
	rs1->Set_auto_timeframe_advance();
	rs_anims = rs1;

	// Icons, Fonts and Remora
	rs3 = new res_man(BITMAP_BUFFER_SIZE);
	rs3->Set_auto_timeframe_advance();
	rs_icons = rs3;
	rs_remora = rs3;
	rs_font = rs3;

	// Stage
	rs2 = new res_man(BACKGROUND_BUFFER_SIZE);
	rs2->Set_auto_timeframe_advance();
	rs_bg = rs2;

	// Privates
	private_session_resman = new res_man(PRIVATE_RESMAN_SIZE);

	// Initalize the Revolution Render Device
	InitRevRenderDevice();

	// we run a special check for a font file - otherwise, res-open will fail triggering a con-fatal-error - not good
	sys_font_hash = HashString(SYS_FONT);
	font_cluster_hash = HashString(FONT_CLUSTER_PATH);

	// inti direct input
	Init_direct_input();

	// init global variables
	Zdebug("Init_globals");
	Init_globals();
	g_globalScriptVariables->SetVariable("missionelapsedtime", 0);
	g_globalScriptVariables->SetVariable("demo", 0);
	Zdebug("~Init_globals");

	// ok, see if the special gameScript is present
	// if so set the stub mode to GameScript mode
	if (gs.Init_game_script() && strstr(lpCmdLine, "mission") == nullptr) {
		// GameScript mode
		// unless there is a console.icb file we don't allow debugging

		// set base mode of stub to gameScript processor
		g_stub->Set_current_stub_mode(__game_script);
	} else
		g_stub->Set_current_stub_mode(__mission_and_console);

	// Initialise the runtime cluster manager
	g_theClusterManager->Initialise();

	zdebug = FALSE8; // no console so switch off debugging (zoff)

	ConfMan.setBool("actor_relative", true);
	ConfMan.setBool("subtitles", true);

	// Load settings from our ini file (must be called AFTER everything has been setup)
	ReadConfigFromIniFile();
}

void quitEngine() {
	Zdebug("\nap closed");
	Zdebug("Be Vigilant!\n");

	if (g_mission)
		g_icb_mission->___delete_mission();

	Close_Sound_Engine();

	if (surface_manager)
		delete surface_manager;

	Save_config_file(); // write user options ini file
	DestoryRevRenderDevice();

	// Shutdown the runtime cluster manager
	g_theClusterManager->Shutdown();

	DestroyGlobalObjects();
}

bool mainLoopIteration() {
	Common::Event event;

	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN: {
			// FIXME: All branches execute effectively the same code
#if 0
			// Pass ENTER and BACKSPACE KEYDOWN events to WriteKey() so the save menu in options_manager_pc.cpp can see them.
			if (event.kbd.keycode == Common::KEYCODE_RETURN) {
				WriteKey((char)event.kbd.keycode);
			} else if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
				WriteKey((char)event.kbd.keycode);
			} else {
#endif
				WriteKey(event.kbd.keycode);
			//}
			setKeyState(event.kbd.keycode, true);
			break;
		}
		case Common::EVENT_KEYUP: {
			setKeyState(event.kbd.keycode, false);
			break;
		}
		case Common::EVENT_MOUSEMOVE:
			mousex = event.mouse.x;
			mousey = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			LogMouseEvent(RD_LEFTBUTTONDOWN);
			mousex = event.mouse.x;
			mousey = event.mouse.y;
			break;
		case Common::EVENT_RBUTTONDOWN:
			LogMouseEvent(RD_RIGHTBUTTONDOWN);
			mousex = event.mouse.x;
			mousey = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONUP:
			LogMouseEvent(RD_LEFTBUTTONUP);
			mousex = event.mouse.x;
			mousey = event.mouse.y;
			break;
		case Common::EVENT_RBUTTONUP:
			LogMouseEvent(RD_RIGHTBUTTONUP);
			mousex = event.mouse.x;
			mousey = event.mouse.y;
			break;

		case Common::EVENT_QUIT:
			quitEngine();
			return false;
		default:
			break;
		}
	}
	// Used to be triggered by else if(gotTheFocus).
	g_stub->Process_stub();

	g_system->delayMillis(1);
	return true;
}

void Mission_and_console() {
	// run the mission with console facility
	// this mode will run forever

	uint32 mission_ret;

	gameCycle += 1; // tick up the game cycle flag

	// logic
	mission_ret = g_mission->Game_cycle();

	if (mission_ret) {
		// the mission has terminated of its own accord - as apposed to a user quit

		// if the player died then we bring up a restart/continue menu here
		CGame *ob = (CGame *)LinkedDataObject::Fetch_item_by_number(MS->objects, MS->player.Fetch_player_id());
		int32 ret = CGameObject::GetVariable(ob, "state");
		if (CGameObject::GetIntegerVariable(ob, ret)) {
			// Return to avoid deleting the mission
			g_stub->Push_stub_mode(__gameover_menu);
			return;
		}

		g_icb_mission->___delete_mission();

		if (!gs.Running_from_gamescript())
			Fatal_error("Thank you for playing In Cold Blood");

		else
			g_stub->Pop_stub_mode(); // back to game script server
	} else
		g_mission->Create_display();
}

void Terminate_ap() {
	// can be called from any service loop to exit the program and return to windows

	Fatal_error("Terminate Ap");
}

void _stub::Fix_time() {
	// wait until target time has elapsed
	// presumably 1/12 second - pc==80ms
	uint32 targetTime = (TARGET_TIME * 100) / cycle_speed;
	do {
		g_system->delayMillis(1);
	} while ((g_system->getMillis() - stub_timer_time) < targetTime);
}

int32 WhichCD(const char * /* mission */) { return 1; }

bool HasMMXTechnology() { return true; }

void _mission::LoadPlatformSpecific(Common::SeekableReadStream * /* fh */) {
	// No PC specific stuff in here (at the moment)
}

void _mission::SavePlatformSpecific(Common::WriteStream * /* fh */) {
	// No PC specific stuff in here (at the moment)
}

} // End of namespace ICB
