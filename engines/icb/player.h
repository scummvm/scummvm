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

#ifndef ICB_PLAYER
#define ICB_PLAYER

#include "engines/icb/common/px_common.h"
#include "engines/icb/debug.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/common/px_route_barriers.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/animation_mega_set.h"
#include "engines/icb/fn_routines.h"
#include "engines/icb/bone.h"

namespace ICB {

extern int armedChangesMode;
extern int animToggles;
extern PXreal REPEL_TURN;
extern PXreal REPULSE_DISTANCE;
extern int CAMERA_SMOOTH_CYCLES;

// How deep the player control mode stack is !
#define CONTROL_MODE_STACK_DEPTH 4

// The maximum number of ammo clips the player can carry
static const uint MAX_AMMO_CLIPS = 5;

// The maximum number of hit points the player has
static const uint MAX_HITS = 10;

// the player service is not tied to the session but exists as a global engine service
enum _player_stat {
	STOOD, // 1
	WALKING, // 2
	CROUCH_WALK, // 3
	RUNNING, // 4
	CROUCH_TO_PUNCH, // 5
	CROUCH_TO_STAND_UNARMED, // 6
	CROUCH_TO_STAND_ARMED, // 7
	CROUCHING, // 8
	NEW_AIM, // 9
	CROUCH_AIM, // 10
	PUTTING_AWAY_GUN, // 11
	PUTTING_AWAY_CROUCH_GUN, // 12
	LINKING, // 13
	GUN_LINKING, // 14
	FAST_LINKING, // 15
	EASY_LINKING, // 16
	REVERSE_LINKING, // 17
	FAST_REVERSE_LINKING, // 18
	STILL_LINKING, // 19
	INVENTORY, // 20
	REMORA, // 21
	STRIKING, // 22
	ON_STAIRS, // 23
	RUNNING_ON_STAIRS, // 24
	STOOD_ON_STAIRS, // 25
	REVERSE_ON_STAIRS, // 26
	ON_LADDER, // 27
	BEGIN_DOWN_LADDER, // 28
	LEAVE_LADDER, // 29
	LEAVE_LADDER_BOTTOM, // 30
	SLIP_SLIDIN_AWAY, // 31
	FINISHED_RELOADING, // 32
	FIN_NORMAL_RELOAD, // 33
	FIN_NORMAL_CROUCH_RELOAD, // 34
	STILL_REVERSE_LINKING, // 35

	__TOTAL_PLAYER_MODES
};

// routines called from gateway return one of these
enum __mode_return { __FINISHED_THIS_CYCLE, __MORE_THIS_CYCLE };

// these are the raw input device states that the player control interface is derived from
enum __player_momentum {
	__STILL,
	__BACKWARD_1,
	__FORWARD_1,
	__FORWARD_2, // only analogue device

	__FORWARD_3
};

enum __player_turn {
	__NO_TURN,
	__LEFT,
	__RIGHT,

	//	psx and pc analogue devices only
	__HARD_LEFT, // analogue controllers only - will stop forward momentum and instigate a turn on spot
	__HARD_RIGHT // as above
};

enum __player_button {
	// PlayStation              PC Default          Use

	__BUTTON_1, // X                            ctrl                    fire
	__BUTTON_2, // O                            space                   interact
	__BUTTON_3, //                              Rshift              unused
	__BUTTON_4, // /\                           enter                   inventory
	__BUTTON_5, //                              delete              unused
	__BUTTON_6, // R2                           Lshift              remora
	__BUTTON_7, // L1                           Ralt                    sidestep
	__NO_BUTTON
};

// Powers of two bit-flag settings
enum ButtonEnums {
	__INTERACT = 1,
	__ATTACK = 2, // fire or punch
	__INVENTORY = 4,
	__ARMUNARM = 8,
	__REMORA = 16,
	__CROUCH = 32,
	__SIDESTEP = 64, // this is a movement modifier button
	__WALKBACK = 64, // this is a movement modifier button
	__JOG = 128,     // RUN !
	__UNUSEDBUTTON = 256
}; // Note, __EXAMINE was never used

struct _input {
	__player_momentum momentum;
	__player_turn turn;

	int bitflag;

	void SetButton(const enum ButtonEnums b) { bitflag |= b; }
	void UnSetButton(const enum ButtonEnums b) { bitflag &= ~b; }
	uint IsButtonSet(const enum ButtonEnums b) const {
		return (bitflag & b);
	}
};

typedef struct {
	PXreal x, z;
	bool8 stepped_on_step;
} _step_sample;

#define MAX_stair_length 40
#define TOP_stair_num (MAX_stair_length - 1)

enum __pc_input { __KEYS, __OTHER_DI_DEVICE };

// What type of control system is being by the player
enum __Actor_control_mode { ACTOR_RELATIVE, SCREEN_RELATIVE };

class _player {
public:
	void ___init();

	void Set_player_id(uint32 id);
	uint32 Fetch_player_id();
	mcodeFunctionReturnCodes Gateway();
	void Update_input_state();

	void DrawCompass();

	void Find_current_player_interact_object();
	void Render_crude_interact_highlight();

	uint32 Fetch_player_interact_id();
	bool8 Fetch_player_interact_status();

	bool8 Player_exists();

	void Reset_player();

	_input *Fetch_input_state();

	uint32 GetBulletsPerClip();
	uint32 GetMaxClips();

	int32 GetNoBullets();
	void SetBullets(uint32 num);
	void UseBullets(uint32 num);

	int32 GetNoAmmoClips();
	void AddAmmoClips(uint32 num, bool8 bFlashIcons);
	void UseAmmoClips(uint32 num);

	int32 GetNoMediPacks();
	void AddMediPacks(uint32 num, bool8 bFlashIcons);
	void UseMediPacks(uint32 num);

	__mode_return Player_walking();
	__mode_return Player_crouch_walk();
	__mode_return Player_running();
	__mode_return Player_stood();
	__mode_return Player_crouching();
	__mode_return Player_aiming();
	__mode_return Player_new_aim();
	__mode_return Player_crouch_aim();
	__mode_return Process_strike();
	__mode_return Player_stairs();
	__mode_return Player_running_on_stairs();
	__mode_return Player_stood_on_stairs();
	__mode_return Player_ladder();
	__mode_return Player_slide_on_ladder();

	void Leave_stair();

	__mode_return Process_link();
	__mode_return Process_fast_link();

	__mode_return Process_reverse_link();

	__mode_return Process_easy_link();
	__mode_return Process_still_link();
	__mode_return Process_reverse_still_link();

	__mode_return Player_interact();
	__mode_return Player_press_fire_button();
	__mode_return Player_press_inv_button();
	__mode_return Player_press_strike_button();
	__mode_return Player_press_remora_button();

	void Add_to_interact_history();

	void Set_to_first_frame(__mega_set_names opt_link);
	void Set_to_last_frame(__mega_set_names opt_link);

	void Start_new_mode(_player_stat new_mode);

	void Soft_start_new_mode_no_link(_player_stat new_mode, __mega_set_names type);

	void Soft_start_new_mode(_player_stat new_mode, __mega_set_names opt_link);
	void Soft_start_new_mode(_player_stat new_mode, __mega_set_names opt_link, __mega_set_names opt_link2);

	void Hard_start_new_mode(_player_stat new_mode, __mega_set_names opt_link);
	void Fast_hard_start_new_mode(_player_stat new_mode, __mega_set_names opt_link);

	void Hard_start_reverse_new_mode(_player_stat new_mode, __mega_set_names opt_link);

	void Easy_start_new_mode(_player_stat new_mode, __mega_set_names opt_link);

	void Still_start_new_mode(_player_stat new_mode, __mega_set_names link);
	void Still_reverse_start_new_mode(_player_stat new_mode, __mega_set_names link);

	bool8 Advance_frame_motion_and_pan(__mega_set_names anim_type);
	bool8 Reverse_frame_motion_and_pan(__mega_set_names anim_type);

	void Set_player_status(_player_stat new_mode);

	inline void Push_control_mode(__Actor_control_mode newMode);
	inline void Pop_control_mode();
	inline void Set_control_mode(__Actor_control_mode newMode);
	inline __Actor_control_mode Get_control_mode();

	void Push_player_stat();
	void Pop_player_stat();

	// the master high level switch
	uint32 player_id; // number of object file of the player object - set this for fast access to structs in sub-modules
	_logic *log; // for the objects logic structure

	// high level master mode switch
	_player_stat player_status;

	// player status to switch to after link animation has finished
	_player_stat stat_after_link;

	// pushed stat
	_player_stat previous_stat;

	// input device state calculated at start of each cycle
	_input cur_state;

	uint32 inv_cycle_count;
	uint32 cur_interact_id; // id number of nico - which has same name as game object
	uint32 look_at_id; // when no interact id exists we may look-at a more distant prop

	//   __Actor_control_mode control_mode_stack[CONTROL_MODE_STACK_DEPTH];
	__Actor_control_mode focus_mode;
	__Actor_control_mode master_mode;

	PXfloat lastCameraPan;
	PXfloat deltaCameraPan;
	PXfloat scrnPan;

	PXfloat aim_turn_amount;        // ANGLE_UNITS_PER_GAME_CYCLE
	PXfloat stood_turn_amount;      // ANGLE_UNITS_PER_GAME_CYCLE
	PXfloat stood_fast_turn_amount; // ANGLE_UNITS_PER_GAME_CYCLE
	PXfloat walk_turn_amount;       // ANGLE_UNITS_PER_GAME_CYCLE
	PXfloat run_turn_amount;        // ANGLE_UNITS_PER_GAME_CYCLE
	PXfloat crouch_turn_amount;     // ANGLE_UNITS_PER_GAME_CYCLE

	// yes, its that stair drift correction system
	_step_sample step_samples[MAX_stair_length];

	BoneDeformation shotDeformation;

	// Note these are at the end of the structure to keep DWORD alignment
	uint8 panCycle;
	bool8 crouch;
	bool8 backward_lock; // set to TRUE to stop repeating backward steps
	bool8 forward_lock;

	bool8 remora_lock;
	bool8 interact_lock;
	bool8 fire_lock;
	bool8 inv_lock;

	bool8 has_weapon;
	bool8 interact_selected; // is there a current player interact object?
	bool8 look_at_selected; // sometimes when there is no interact
	bool8 dead_mega; // is the target dead or not - if a mega

	// set to true when
	bool8 player_exists;
	bool8 haveCamera;
	bool8 hunting;
	// stairway stuff
	uint8 stair_num; // stair number we're on

	uint8 stair_dir; // 1 up, 0 down
	uint8 stair_unit; // how many cycles into stair are we
	uint8 left_right; // 1 left / 0 right
	uint8 was_climbing; // 1 yes, 0 no - used to see if first stand

	uint8 begun_at_bottom; // used when deciding to write history
	int8 being_shot;
	int8 shot_by_id;
	int step_sample_num;

	int8 walk_count; // counts up as you walk - used to alert guards of non creeping player
	bool8 stood_on_lift; // are we stood on a lift
	// armed menu
	uint8 padding[2];

	// SORTED PADDING AGAIN.  MAYBE IT'S NOT IMPORTANT, 'COS EVERY TIME I GO TO WELL-USED CLASSES
	// LIKE PLAYER, THE ALLIGNMENT IS ALL OVER THE PLACE.
};

inline bool8 _player::Player_exists() {
	// return id of player object

	return (player_exists);
}

inline uint32 _player::Fetch_player_id() {
	// return id of player object

	if (!player_exists)
		Fatal_error("no live player - must stop");

	return (player_id);
}

inline uint32 _player::Fetch_player_interact_id() {
	// return id of player object

	return (cur_interact_id);
}

inline bool8 _player::Fetch_player_interact_status() {
	// return id of player object

	return (interact_selected);
}

inline _input *_player::Fetch_input_state() {
	// return id of player object

	return (&cur_state);
}

inline void _player::Push_control_mode(__Actor_control_mode newMode) { focus_mode = newMode; }

inline void _player::Set_control_mode(__Actor_control_mode newMode) {
	focus_mode = newMode;
	master_mode = newMode;
}

inline void _player::Pop_control_mode() {}

inline __Actor_control_mode _player::Get_control_mode() { return focus_mode; }


extern uint32 fire_key;
extern uint32 interact_key;
extern uint32 inventory_key;
extern uint32 arm_key;
extern uint32 remora_key;
extern uint32 crouch_key;
extern uint32 sidestep_key;
extern uint32 run_key;
extern uint32 pause_key;
extern uint32 up_key;
extern uint32 down_key;
extern uint32 left_key;
extern uint32 right_key;


} // End of namespace ICB

#endif
