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

#ifndef ICB_MISSION
#define ICB_MISSION

#include "engines/icb/common/px_common.h"
#include "engines/icb/session.h"
#include "engines/icb/string_vest.h"

namespace ICB {

#define SR_VERSION 17

// save_game_max is :
// 8192 (1 memory card block) - 256 (PSX system data) - 8 (PSX engine data)
#define SAVE_GAME_MAX_SIZE (8192 - 256 - 8)

// total number of sessions allowed per mission
#define MAX_sessions 5
#define MAX_fvars (MAX_voxel_list * 4)
#define MAX_lvars 30

class _micro_object {
public:
	int32 lvar_value[MAX_lvars];
	uint32 total_lvars; // worth doing to reduce save game size

	_object_status status_flag;
};

class _micro_session {
public:
	char session__name[ENGINE_STRING_LEN]; // eg land_train

	uint32 number_of_micro_objects;
	_micro_object micro_objects[MAX_session_objects];

	int32 fvars[MAX_fvars];
};

enum __load_result { __NO_SUCH_FILE, __WRONG_VERSION, __GLOBAL_MISMATCH, __LOAD_OK };

class _mission {

public:
	_mission();
	~_mission();

	void ___init_mission(const char *mission, const char *session);
	void ___delete_mission();

	void End_mission();

	inline const char *Fetch_mission_name();
	inline const char *Fetch_h_mission_name();
	inline const char *Fetch_tiny_mission_name();
	inline const char *Fetch_tiny_session_name();
	uint32 Game_cycle();
	void Create_display();
	void Set_new_session_name(const char *ascii);
	void Set_init_nico_name(const char *ascii);
	bool8 Is_there_init_nico();
	const char *Return_init_nico_name();
	void Reset_init_nico();
	void Save_micro_session();
	void Restore_micro_session_coords(bool8 from_disk);
	void Restore_micro_session_vars();
	void Save_game_position(const char *filename, const char *slot_label, uint32 timeplayed);
	void Restore_micro_session_from_save_game(Common::SeekableReadStream *stream);
	void SavePlatformSpecific(Common::WriteStream *stream);
	void LoadPlatformSpecific(Common::SeekableReadStream *stream);

	_game_session *session; // pointer to current game session object

	// camera stuff
	uint32 camera_follow_id_overide; // id of a character to watch - if 0 defaults to player
	// here because we want it to hold across session boundaries
	// This flag keeps track of whether or not the Remora was active when an fn_set_watch()
	// was issued, so it can be put back how it was.
	int32 remora_save_mode;

	uint32 ad_time;           // actor draw timer flags used for debugging
	uint32 lt_time;           // actor draw timer flags used for debugging
	uint32 set_time;          // setdraw timer flags used for debugging
	uint32 flip_time;         // back buffer blit timer used for debugging
	uint32 cycle_time;        // The length of time the last cycle took
	uint32 logic_time;        // Time spent in logic
	uint32 resman_logic_time; // Time spent in logic
	uint32 los_time;
	uint32 event_time;
	uint32 sound_time;
	uint32 xtra_mega_time;
	uint32 nActorsDrawn;
	uint32 nActorsConsidered;

	// points to the name of a new session
	char new_session_name[TINY_NAME_LEN];
	char init_nico_name[TINY_NAME_LEN];
	int32 old_hits_value; // we carry across the lvars from the previous session - this is getting tweeky

	bool8 chi_following; // is a chi object following the player? Used for session changes

	// player requires these to cross sessions
	uint32 num_bullets;
	uint32 num_clips;
	uint32 num_medi;

	bool8 inited_globals; // yes or no - used to init the globals

private:
	char mission_name[ENGINE_STRING_LEN];  // full path
	char tiny_mission_name[TINY_NAME_LEN]; // eg land_train
	char tiny_session_name[TINY_NAME_LEN]; // eg car_5

	uint32 mission_terminate; //! 0 means the mission will end
	char h_mission_name[8];   // filename of the hash'ed version of mission_name

	uint32 mission_status; // 0 ok
	// 1 terminate the mission - mission has finished

	uint32 number_sessions_saved;
	_micro_session micro_sessions[MAX_sessions];

	bool8 new_session; // yes new session
	bool8 init_nico;   // yes new session nico
};

bool8 Setup_new_mission(const char *mission_name, const char *session_name);
bool8 Setup_camtest_mission();
__load_result Load_game(const char *filename);

inline const char *_mission::Fetch_mission_name() { return (mission_name); }

inline const char *_mission::Fetch_tiny_mission_name() { return (tiny_mission_name); }

inline const char *_mission::Fetch_tiny_session_name() { return (tiny_session_name); }

inline const char *_mission::Fetch_h_mission_name() { return (h_mission_name); }

} // End of namespace ICB

#endif
