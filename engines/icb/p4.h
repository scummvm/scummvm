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

#ifndef ICB_P4_H
#define ICB_P4_H

// Silly codewarrior doesn't understand relative paths
#include "engines/icb/common/px_common.h"
#include "engines/icb/string_vest.h"

namespace ICB {

// Handy #define's
#define MS g_mission->session
#define MSS g_mission->session->set

// functions required externally
void Terminate_ap(); // call this to end the game and return to windows

// variables
extern int gameCycle; // for head-up display
extern char gamelanguage[];
extern bool8 camera_hack;
extern uint32 font_cluster_hash; // Res_open will compute the hash value and store it
extern uint32 sys_font_hash;     // Res_open will compute the hash value and store it

#define RESOURCE_IS_COMPRESSED 1

extern bool gRegainedFocus; // Set when we regain the focus.  Cleared by the graphics reloading functions

} // End of namespace ICB (To avoid including inside the namespace

#include "engines/icb/common/px_string.h"

namespace ICB {
extern char g_characters[];

#define REMORA_GRAPHICS_PATH "remora\\pc\\"
#define FONT_PATH "fonts\\pc\\%s"
#define REMORA_CLUSTER_PATH "A\\2DART"

// Using psx style cluster paths
#define ICON_CLUSTER_PATH "A\\2DART"                  // icon cluster path
#define FONT_CLUSTER_PATH "A\\2DART"                  // font cluster path
#define GLOBAL_CLUSTER_PATH "G\\G"                    // global cluster path
#define SESSION_TEST_PATH "m\\%s\\%s.sex"             // File to test for a sessions existence
#define SESSION_CLUSTER_PATH "M\\%s\\%s.SEX"          // Path to the session cluster
#define CHR_PATH "C\\%s\\%s.OFT"
#define SET_PATH "M\\%s\\%s.CAM" // path to the set

#define SYS_FONT "fonts\\font.tdw"       // default font name
#define ICON_PATH "inventory_icon\\pc\\" // path for inventory objects
#define GAMESCRIPT_PATH "gamescript"
#define GLOBAL_VAR_PATH "globals" // globals file name
#define GLOBAL_TEXT_FILE "%s.ttpc", gamelanguage

// These are for the Remora's text system (and any other voice-over text we might need).
// This defines the name of the resource files that are searched for Remora text.
#define FILENAME_VOTEXT "remora.ctf"
#define GLOBAL_VOTEXT_NAME "remora.ctf"
#define ICON_LABELS_FILENAME "iconlabels.ctf"
#define CONFIG_INI_FILENAME "engine\\icb.ini"

// Resource Manager Sizes -- Initalize in Memory_stats
extern uint32 BACKGROUND_BUFFER_SIZE;
extern uint32 ANIMATION_BUFFER_SIZE;
extern uint32 BITMAP_BUFFER_SIZE;
extern uint32 SONICS_BUFFER_SIZE;

// globals for the font cluster name and hash value
extern pxString font_cluster;

#define ANIM_CHECK(a)                                                                                                                                                              \
	if (!I->IsAnimTable(a))                                                                                                                                                    \
		Fatal_error("engine finds anim [%s] '%s' %X missing for object [%s]", (const char *)master_anim_name_table[a].name, I->get_info_name(a), I->info_name_hash[a],     \
		            object->GetName());

enum __stub_modes {
	__mission_and_console,
	__game_script,
	__floors,
	__shift_mode,
	__troute,
	__vox_view,
	__set_test,
	__console_prompt,
	__stage_view,
	__sequence,
	__options_menu,
	__font_test,
	__toe_on_door,
	__pause_menu,
	__load_save_menu,
	__credits,
	__scrolling_text,
	__gameover_menu,
	__no_stub_mode
};
#define TOTAL_STUBS 8
#define TARGET_TIME 70

class _stub {
	public:
	_stub();
	~_stub();

	void Reset(__stub_modes new_mode);
	void Set_current_stub_mode(__stub_modes new_mode);
	void Push_stub_mode(__stub_modes new_mode);
	void Pop_stub_mode();
	void Process_stub();
	__stub_modes Return_current_stub();

	void Reset_timer();
	void Fix_time();
	void Timer_off();
	void Timer_on();
	bool8 Return_timer_status();
	void Update_screen();

	__stub_modes mode[TOTAL_STUBS];
	int32 stub; // stub level number
	int cycle_speed;
private:
	uint32 stub_timer_time;
	bool8 timer; // on off
	uint8 padding1;
	uint8 padding2;
	uint8 padding3;
};

// this is one of the stub modes
void Mission_and_console();

// returns the CD number for the specified mission
int WhichCD(const char *mission);

} // End of namespace ICB

#endif
