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

#include "common/str.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "common/events.h"

namespace ICB {

bool gRegainedFocus = false;
bool gotTheFocus = false;

pxString font_cluster = FONT_CLUSTER_PATH;

char g_characters[] = "characters\\";
char gamelanguage[ENGINE_STRING_LEN] = "english";
bool8 camera_hack;
uint32 BACKGROUND_BUFFER_SIZE;
uint32 ANIMATION_BUFFER_SIZE;
uint32 BITMAP_BUFFER_SIZE;
uint32 SONICS_BUFFER_SIZE;

// private session data (scripts+objects+walkgrids) : 200KB
#define PRIVATE_RESMAN_SIZE (400 * 1024)

// local prototypes
void Mission_and_console();

uint32 getConfigValueWithDefault(const Common::String &key, uint32 defaultValue) {
	uint32 result = defaultValue;
	if (ConfMan.hasKey(key)) {
		result = ConfMan.getInt(key);
	}
	return result;
}

void ReadConfigFromIniFile() {
	char configFile[1024];
	uint32 temp;
	const uint32 k128 = 128;
	const uint32 k0 = 0;

	sprintf(configFile, CONFIG_INI_FILENAME);

	ConfigFile config;
	pxString filename = configFile;
	filename.ConvertPath();
	config.readFile(filename.c_str());

	// Music volume
	temp = getConfigValueWithDefault("music_volume", 255); // TODO: Fix the internal volume settings to use 0-255 instead of 0-128
	temp = MIN(MAX(k0, temp), k128);

	SetMusicVolume(temp);

	// Speech volume
	temp = getConfigValueWithDefault("speech_volume", 255);
	temp = MIN(MAX(k0, temp), k128);
	SetSpeechVolume(temp);

	// Sfx volume
	temp = getConfigValueWithDefault("sfx_volume", 255);
	temp = MIN(MAX(k0, temp), k128);
	SetSfxVolume(temp);

	// Has the game been completed previously
	temp = config.readIntSetting("Extras", pxVString("%X", HashString("Game Completed")), 0);

	// HACK: Enable all extras for now
	temp = 1;
	warning("Enabling all extras for development purposes");

	if (temp == 0)
		px.game_completed = FALSE8;
	else
		px.game_completed = TRUE8;

	// Let command line switch take priority for this setting (for Daves translations work)
	// ie only look at ini value if command line hasn't set it already
	// Subtitle switch
	temp = false;
	if (ConfMan.hasKey("subtitles")) {
		temp = ConfMan.getBool("subtitles");
	}
	if (temp == 0)
		px.on_screen_text = FALSE8;
	else
		px.on_screen_text = TRUE8;

	// Semitransparency switch
	temp = config.readIntSetting("Video Settings", "Semitransparency", 0);

	if (temp == 0)
		px.semitransparencies = FALSE8;
	else
		px.semitransparencies = TRUE8;

	// Control device
	temp = config.readIntSetting("Controller Settings", "Device", 0);

	if (temp == 0)
		currentJoystick = NO_JOYSTICK;
	else
		currentJoystick = ANALOGUE_JOYSTICK;

	// A call to this will validate the above setting
	Poll_direct_input();

	// Control method
	temp = config.readIntSetting("Controller Settings", "Method", 0);

	if ((__Actor_control_mode)temp == SCREEN_RELATIVE)
		g_icb_session->player.Set_control_mode(SCREEN_RELATIVE);
	else
		g_icb_session->player.Set_control_mode(ACTOR_RELATIVE);

	// Set the default keys first in case the ini file mappings are invalid somehow
	SetDefaultKeys();

	// Keyboard mappings
	temp = config.readIntSetting("Keyboard Mappings", "Up", 0);
	if (GetKeyName((uint8)temp))
		up_key = (uint8)temp;
	temp = config.readIntSetting("Keyboard Mappings", "Down", 0);
	if (GetKeyName((uint8)temp))
		down_key = (uint8)temp;
	temp = config.readIntSetting("Keyboard Mappings", "Left", 0);
	if (GetKeyName((uint8)temp))
		left_key = (uint8)temp;
	temp = config.readIntSetting("Keyboard Mappings", "Right", 0);
	if (GetKeyName((uint8)temp))
		right_key = (uint8)temp;
	temp = config.readIntSetting("Keyboard Mappings", "Sidestep", 0);
	if (GetKeyName((uint8)temp))
		sidestep_key = (uint8)temp;
	temp = config.readIntSetting("Keyboard Mappings", "Run", 0);
	if (GetKeyName((uint8)temp))
		run_key = (uint8)temp;
	temp = config.readIntSetting("Keyboard Mappings", "Crouch", 0);
	if (GetKeyName((uint8)temp))
		crouch_key = (uint8)temp;
	temp = config.readIntSetting("Keyboard Mappings", "Interact", 0);
	if (GetKeyName((uint8)temp))
		interact_key = (uint8)temp;
	temp = config.readIntSetting("Keyboard Mappings", "Arm", 0);
	if (GetKeyName((uint8)temp))
		arm_key = (uint8)temp;
	temp = config.readIntSetting("Keyboard Mappings", "Attack", 0);
	if (GetKeyName((uint8)temp))
		fire_key = (uint8)temp;
	temp = config.readIntSetting("Keyboard Mappings", "Inventory", 0);
	if (GetKeyName((uint8)temp))
		inventory_key = (uint8)temp;
	temp = config.readIntSetting("Keyboard Mappings", "Remora", 0);
	if (GetKeyName((uint8)temp))
		remora_key = (uint8)temp;
	temp = config.readIntSetting("Keyboard Mappings", "Pause", 0);
	if (GetKeyName((uint8)temp))
		pause_key = (uint8)temp;

	// Unset all joystick
	UnsetJoystickConfig();

	// Joystick mappings
	temp = config.readIntSetting("Joystick Mappings", "Up", 0);
	if (GetAxisName((uint8)temp))
		up_joy = (uint8)temp;
	temp = config.readIntSetting("Joystick Mappings", "Down", 0);
	if (GetAxisName((uint8)temp))
		down_joy = (uint8)temp;
	temp = config.readIntSetting("Joystick Mappings", "Left", 0);
	if (GetAxisName((uint8)temp))
		left_joy = (uint8)temp;
	temp = config.readIntSetting("Joystick Mappings", "Right", 0);
	if (GetAxisName((uint8)temp))
		right_joy = (uint8)temp;

	temp = config.readIntSetting("Joystick Mappings", "Sidestep", 0);
	if (GetButtonName((uint8)temp))
		sidestep_button = (uint8)temp;
	temp = config.readIntSetting("Joystick Mappings", "Run", 0);
	if (GetButtonName((uint8)temp))
		run_button = (uint8)temp;
	temp = config.readIntSetting("Joystick Mappings", "Crouch", 0);
	if (GetButtonName((uint8)temp))
		crouch_button = (uint8)temp;
	temp = config.readIntSetting("Joystick Mappings", "Interact", 0);
	if (GetButtonName((uint8)temp))
		interact_button = (uint8)temp;
	temp = config.readIntSetting("Joystick Mappings", "Arm", 0);
	if (GetButtonName((uint8)temp))
		arm_button = (uint8)temp;
	temp = config.readIntSetting("Joystick Mappings", "Attack", 0);
	if (GetButtonName((uint8)temp))
		fire_button = (uint8)temp;
	temp = config.readIntSetting("Joystick Mappings", "Inventory", 0);
	if (GetButtonName((uint8)temp))
		inventory_button = (uint8)temp;
	temp = config.readIntSetting("Joystick Mappings", "Remora", 0);
	if (GetButtonName((uint8)temp))
		remora_button = (uint8)temp;
	temp = config.readIntSetting("Joystick Mappings", "Pause", 0);
	if (GetButtonName((uint8)temp))
		pause_button = (uint8)temp;

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
	// Whip some bytes off the stack
	char configFile[1024];
	char tempBuff[1024];

	sprintf(configFile, CONFIG_INI_FILENAME);
	// Should really delete the existing ini file before writing the current settings...
	remove(configFile);

	ConfigFile config;

	// Update ini file settings when we shutdown correctly
	sprintf(tempBuff, "%d", GetMusicVolume());
	config.writeSetting("Option Settings", "MusicVolume", tempBuff);
	sprintf(tempBuff, "%d", GetSpeechVolume());
	config.writeSetting("Option Settings", "SpeechVolume", tempBuff);
	sprintf(tempBuff, "%d", GetSfxVolume());
	config.writeSetting("Option Settings", "SfxVolume", tempBuff);

	sprintf(tempBuff, "%d", px.on_screen_text);
	config.writeSetting("Video Settings", "Subtitles", tempBuff);
	sprintf(tempBuff, "%d", px.semitransparencies);
	config.writeSetting("Video Settings", "Semitransparency", tempBuff);
	sprintf(tempBuff, "%d", px.actorShadows);

	sprintf(tempBuff, "%d", currentJoystick);
	config.writeSetting("Controller Settings", "Device", pxVString("%d", currentJoystick));
	sprintf(tempBuff, "%d", g_icb_session->player.Get_control_mode());
	config.writeSetting("Controller Settings", "Method", pxVString("%d", g_icb_session->player.Get_control_mode()));

	sprintf(tempBuff, "%d", up_key);
	config.writeSetting("Keyboard Mappings", "Up", tempBuff);
	sprintf(tempBuff, "%d", down_key);
	config.writeSetting("Keyboard Mappings", "Down", tempBuff);
	sprintf(tempBuff, "%d", left_key);
	config.writeSetting("Keyboard Mappings", "Left", tempBuff);
	sprintf(tempBuff, "%d", right_key);
	config.writeSetting("Keyboard Mappings", "Right", tempBuff);
	sprintf(tempBuff, "%d", sidestep_key);
	config.writeSetting("Keyboard Mappings", "Sidestep", tempBuff);
	sprintf(tempBuff, "%d", run_key);
	config.writeSetting("Keyboard Mappings", "Run", tempBuff);
	sprintf(tempBuff, "%d", crouch_key);
	config.writeSetting("Keyboard Mappings", "Crouch", tempBuff);
	sprintf(tempBuff, "%d", interact_key);
	config.writeSetting("Keyboard Mappings", "Interact", tempBuff);
	sprintf(tempBuff, "%d", arm_key);
	config.writeSetting("Keyboard Mappings", "Arm", tempBuff);
	sprintf(tempBuff, "%d", fire_key);
	config.writeSetting("Keyboard Mappings", "Attack", tempBuff);
	sprintf(tempBuff, "%d", inventory_key);
	config.writeSetting("Keyboard Mappings", "Inventory", tempBuff);
	sprintf(tempBuff, "%d", remora_key);
	config.writeSetting("Keyboard Mappings", "Remora", tempBuff);
	sprintf(tempBuff, "%d", pause_key);
	config.writeSetting("Keyboard Mappings", "Pause", tempBuff);

	sprintf(tempBuff, "%d", up_joy);
	config.writeSetting("Joystick Mappings", "Up", tempBuff);
	sprintf(tempBuff, "%d", down_joy);
	config.writeSetting("Joystick Mappings", "Down", tempBuff);
	sprintf(tempBuff, "%d", left_joy);
	config.writeSetting("Joystick Mappings", "Left", tempBuff);
	sprintf(tempBuff, "%d", right_joy);
	config.writeSetting("Joystick Mappings", "Right", tempBuff);
	sprintf(tempBuff, "%d", sidestep_button);
	config.writeSetting("Joystick Mappings", "Sidestep", tempBuff);
	sprintf(tempBuff, "%d", run_button);
	config.writeSetting("Joystick Mappings", "Run", tempBuff);
	sprintf(tempBuff, "%d", crouch_button);
	config.writeSetting("Joystick Mappings", "Crouch", tempBuff);
	sprintf(tempBuff, "%d", interact_button);
	config.writeSetting("Joystick Mappings", "Interact", tempBuff);
	sprintf(tempBuff, "%d", arm_button);
	config.writeSetting("Joystick Mappings", "Arm", tempBuff);
	sprintf(tempBuff, "%d", fire_button);
	config.writeSetting("Joystick Mappings", "Attack", tempBuff);
	sprintf(tempBuff, "%d", inventory_button);
	config.writeSetting("Joystick Mappings", "Inventory", tempBuff);
	sprintf(tempBuff, "%d", remora_button);
	config.writeSetting("Joystick Mappings", "Remora", tempBuff);
	sprintf(tempBuff, "%d", pause_button);
	config.writeSetting("Joystick Mappings", "Pause", tempBuff);

	char temp[1024];

	// Has the game been completed
	if (px.game_completed) {
		sprintf(temp, "%X", HashString("Game Completed"));

		config.writeSetting("Extras", temp, "1");
	}

	// Write the movie library settings
	for (uint32 i = 0; i < TOTAL_NUMBER_OF_MOVIES; i++) {
		// Only write a setting when it's been achieved
		if (g_movieLibrary[i].visible) {
			sprintf(temp, "%X", HashString(g_movieLibrary[i].filename));
			sprintf(tempBuff, "%d", g_movieLibrary[i].visible);

			config.writeSetting("Movie Library", temp, tempBuff);
		}
	}
	pxString filename = configFile;
	filename.ConvertPath();
	config.writeFile(filename.c_str());
}

void InitEngine(const char *lpCmdLine) {
	CreateGlobalObjects();

	// Set the character root directory to be the same as the normal root directory
	camera_hack = false; // defaults to off

#ifdef USE_SDL_DIRECTLY
	sdl_window = SDL_CreateWindow("In Cold Blood (C)2000 Revolution Software Ltd", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, sdl_flags);
#endif
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
	rs1 = new res_man(ANIMATION_BUFFER_SIZE, false); // TODO: Should be true, but threads are broken atm.
	rs1->Set_auto_timeframe_advance();
	rs_anims = rs1;

	// Icons, Fonts and Remora
	rs3 = new res_man(BITMAP_BUFFER_SIZE, false);
	rs3->Set_auto_timeframe_advance();
	rs_icons = rs3;
	rs_remora = rs3;
	rs_font = rs3;

	// Stage
	rs2 = new res_man(BACKGROUND_BUFFER_SIZE, false);
	rs2->Set_auto_timeframe_advance();
	rs_bg = rs2;

	// Privates
	private_session_resman = new res_man(PRIVATE_RESMAN_SIZE, false); // size, no ASYNC

	// Initalize the Revolution Render Device
	InitRevRenderDevice();

	// we run a special check for a font file - otherwise, res-open will fail triggering a con-fatal-error - not good
	sys_font_hash = HashString(SYS_FONT);
	font_cluster_hash = HashString(font_cluster);

	// inti direct input
	Init_direct_input();

	// init global variables
	Zdebug("Init_globals");
	Init_globals();
	g_globalScriptVariables.SetVariable("missionelapsedtime", 0);
	g_globalScriptVariables.SetVariable("demo", 0);
	Zdebug("~Init_globals");

	// ok, see if the special gameScript is present
	// if so set the stub mode to GameScript mode
	if (gs.Init_game_script() && strstr(lpCmdLine, "mission") == NULL) {
		// GameScript mode
		// unless there is a console.icb file we dont allow debugging

		// set base mode of stub to gameScript processor
		stub.Set_current_stub_mode(__game_script);

		// are all resources present?
		Common::SeekableReadStream *stream = openDiskFileForBinaryStreamRead("full.icb");
		if (stream) {
			px.pc_full_intall = TRUE8;
			delete stream;
		}
	} else
		stub.Set_current_stub_mode(__mission_and_console);

	// Initialise the runtime cluster manager
	g_theClusterManager->Initialise();

	zdebug = FALSE8; // no console so switch off debugging (zoff)

	// Load settings from our ini file (must be called AFTER everything has been setup)
	ReadConfigFromIniFile();
}

bool mainLoopIteration() {
	Common::Event event;

	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		/*
		case SDL_TEXTINPUT:
			if (event.text.text[0] && event.text.text[1] == 0) {
				uint8 key = event.text.text[0];
					WriteKey(key);
			}
			break;
		*/
		case Common::EVENT_KEYDOWN: {
			// Pass ENTER and BACKSPACE KEYDOWN events to WriteKey() so the save menu in options_manager_pc.cpp can see them.
			if (event.kbd.keycode == Common::KEYCODE_RETURN) {
				WriteKey((char)event.kbd.keycode);
			} else if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
				WriteKey((char)event.kbd.keycode);
			} else {
				WriteKey(event.kbd.keycode);
			}
			setKeyState(event.kbd.keycode, true);
		} break;
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
		/*case SDL_WINDOWEVENT:
			switch (event.window.event) {
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				SDL_ShowCursor(SDL_FALSE);
				gRegainedFocus = TRUE8;
				gotTheFocus = TRUE8;
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				SDL_ShowCursor(SDL_TRUE);
				gRegainedFocus = FALSE8;
				gotTheFocus = FALSE8;
				break;
			case SDL_WINDOWEVENT_EXPOSED:
				stub.Process_stub();
				break;
			case SDL_WINDOWEVENT_CLOSE:
				break;
			}
			break;*/

		// TODO: Fix sound
		/*case SDL_USEREVENT:
			if (g_TimerOn) {
				UpdateSounds10Hz();
				Poll_Sound_Engine();
			}
			break;*/

		case Common::EVENT_QUIT:
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
			return false;
		default:
			break;
		}
	}
	// Used to be triggered by else if(gotTheFocus).
	stub.Process_stub();

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
		c_game_object *ob = (c_game_object *)MS->objects->Fetch_item_by_number(MS->player.Fetch_player_id());
		int32 ret = ob->GetVariable("state");
		if (ob->GetIntegerVariable(ret)) {
			// Return to avoid deleting the mission
			stub.Push_stub_mode(__gameover_menu);
			return;
		}

		g_icb_mission->___delete_mission();

		if (!gs.Running_from_gamescript())
			Fatal_error("Thank you for playing In Cold Blood");

		else
			stub.Pop_stub_mode(); // back to game script server
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

int WhichCD(const char * /* mission */) { return 1; }

bool HasMMXTechnology() { return true; }

void _mission::LoadPlatformSpecific(Common::SeekableReadStream * /* fh */) {
	// No PC specific stuff in here (at the moment)
}

void _mission::SavePlatformSpecific(Common::WriteStream * /* fh */) {
	// No PC specific stuff in here (at the moment)
}

} // End of namespace ICB
