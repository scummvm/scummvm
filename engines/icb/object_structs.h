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

#ifndef ICB_OBSTRUCTS
#define ICB_OBSTRUCTS

#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_game_object.h"
#include "engines/icb/common/px_route_barriers.h"
#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_array.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/animation_mega_set.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/string_vest.h"
#include "engines/icb/route_manager.h"
#include "engines/icb/breath.h"
#include "engines/icb/gfx/rlp_api.h"
#include "engines/icb/common/px_bones.h"

// depth of script logic tree for game objects - 3 is same as Broken Sword games
#define TREE_SIZE 3

namespace ICB {

// high level walk or run mode for mega/player
// used by high level routing and interaction functionality
enum __motion { __MOTION_WALK, __MOTION_RUN };

// high level armed state
// this will be used to construct the animation path name
enum __weapon {
	__NOT_ARMED,
	__GUN,
	__CROUCH_NOT_ARMED,
	__CROUCH_GUN,
	__TOTAL_WEAPONS, // used for searches
	__NOT_SET        // default - will cause error in fn-set-voxel-path
};

// Added this enum for object type field.
enum __object_type { __NO_TYPE_SET = 0, __BUTTON_OPERATED_DOOR, __RECHARGE_POINT, __ORGANIC_MEGA, __NON_ORGANIC_MEGA, __REMORA_CARRIER, __RECHARGE_POINT_WITH_EMP, __AUTO_DOOR };

#define MAX_WEAPON_NAME_LENGTH 16

extern char weapon_text[__TOTAL_WEAPONS][MAX_WEAPON_NAME_LENGTH];
extern bool8 armed_state_table[__TOTAL_WEAPONS];
extern bool8 crouch_state_table[__TOTAL_WEAPONS];

#define MAX_CHAR_NAME_LENGTH 32
#define MAX_OUTFIT_NAME_LENGTH 16
#define MAX_CUSTOM_NAME_LENGTH 16

#define IMAGE_PATH_STR_LEN 144
#define BASE_PATH_STR_LEN 144
#define ANIM_NAME_STR_LEN 144
#define PALETTE_STR_LEN ENGINE_STRING_LEN

// game objects with voxel images need one of these
// assigned at session start when objects are initialised
class _vox_image { // this is badly named - frame info or something

private:
	int8 anim_table[__TOTAL_ANIMS];

public:
	void ___init(const char *chr, const char *set, __weapon weapon);

	// temprarily store these for swapping between voxel and polygons (so we can call ___init() again...
	// when the voxels go these things can go
	char palette[PALETTE_STR_LEN];
	char temp_chr[128];
	char temp_set[128];
	__weapon temp_weapon;
	char alternativeImagePath[128]; // this stores the pcp path for animations (for getting the raj files instead of rai files...
	char alternativeBasePath[128];  // this is same as base path but is pcp (on voxels) - isn't required for polys and can go when we
                                        // ditch
	// setting functions
	bool8 Init_custom_animation(const char *anim_name);
	bool8 Find_anim_type(__mega_set_names *anim, const char *name);
	bool8 Set_texture(const char *texture_name);
	bool8 Set_palette(const char *palette_name);
	bool8 Set_mesh(const char *mesh_name);
	void Promote_non_generic();
	int8 IsAnimTable(int i);
	void MakeAnimEntry(int i);

	// those hashs in full

	uint32 anim_name_hash[__TOTAL_ANIMS];
	uint32 info_name_hash[__TOTAL_ANIMS];

	uint32 mesh_hash;        // the name of the base mesh
	uint32 shadow_mesh_hash; // the name of the base shadow mesh
	uint32 palette_hash;     // the name of the palette
	uint32 texture_hash;     // the name of the texture (which includes a default palette)
	uint32 pose_hash;        // the name of the ordinary pose/accessory mesh
	uint32 custom_pose_hash; // the name of the "custom" pose/accessory mesh

	// base path

	char base_path[BASE_PATH_STR_LEN];
	uint32 base_path_hash; // hash value of the base_path string

	// init'ed by _vox_image::_vox_image which is created by fn-set-voxel-image-path
	// path primitives - require to build path+name to non generic custom specifics
	char image_path[IMAGE_PATH_STR_LEN]; // i.e. karl\, karl_in_coat\, etc. Initially set by objects init script and then by
	                                     // interaction/logic

	bool8 has_custom_path_built; // says whether or not the current custom anim path has been built - done for preloader which builds
	                             // filename early

	// bone deformations

	BoneDeformation neckBone;
	BoneDeformation jawBone;
	BoneDeformation lookBone;
	SVECTOR
	scriptedLookBoneTarget; // this is the one the scripts set - if this is not <0,0,0> then these values override the look targets

	uint8 padding2;
	uint8 padding3;

	// on the pc we store these strings
	char anim_name[__TOTAL_ANIMS][ANIM_NAME_STR_LEN];
	char info_name[__TOTAL_ANIMS][ANIM_NAME_STR_LEN];
	char mesh_name[ANIM_NAME_STR_LEN];
	char shadow_mesh_name[ANIM_NAME_STR_LEN];
	char texture_name[PALETTE_STR_LEN];
	char palette_name[PALETTE_STR_LEN];
	char pose_name[ANIM_NAME_STR_LEN];
	char custom_pose_name[ANIM_NAME_STR_LEN];

	// and so the access functions return the
	// actual thing

	char *get_anim_name(int i) { return anim_name[i]; }
	char *get_info_name(int i) { return info_name[i]; }
	char *get_mesh_name() { return mesh_name; }
	char *get_shadow_mesh_name() { return shadow_mesh_name; }
	char *get_texture_name() { return texture_name; }
	char *get_palette_name() { return palette_name; }
	char *get_pose_name() { return pose_name; }
	char *get_custom_pose_name() { return custom_pose_name; }

};

inline int8 _vox_image::IsAnimTable(int i) {
	// If the table has not been set yet : make the anim entry which will set anim_table entry to correct value
	if (anim_table[i] == (int8)-1)
		MakeAnimEntry(i);
	return anim_table[i];
}

#define MAX_bars 96

// Handy constants for accesing the viewState member variable
// <last_cycle_state>_<this_cycle_state>
// The <last_cycle_state> (1 or 0) is in bit 0
// The <this_cycle_state> (1 or 0) is in bit 1
enum CameraStateEnum { OFF_CAMERA = 0x0, ON_CAMERA = 0x1 };

#define THIS_CYCLE_SHIFT 1
#define LAST_CYCLE_SHIFT 0
#define LAST_CYCLE_MASK ((1 << THIS_CYCLE_SHIFT) - 1)

#define MAKE_VIEW_STATE(last, this) (last | (this << THIS_CYCLE_SHIFT))

// off camera last cycle and this cycle : 0x0
#define OFF_OFF_CAMERA MAKE_VIEW_STATE(OFF_CAMERA, OFF_CAMERA)

// on camera last cycle and off this cycle : 0x1
#define ON_OFF_CAMERA MAKE_VIEW_STATE(ON_CAMERA, OFF_CAMERA)

// off camera last cycle and on this cycle : 0x2
#define OFF_ON_CAMERA MAKE_VIEW_STATE(OFF_CAMERA, ON_CAMERA)

// on camera last cycle and on this cycle
#define ON_ON_CAMERA MAKE_VIEW_STATE(ON_CAMERA, ON_CAMERA)

class _mega { // mega logic specific
      public:
	_parent_box *cur_parent; // our owner parent box
	uint32 par_number;       // for players abar list update
	uint32 cur_slice;        // for speedups

	uint32 barrier_list[MAX_bars]; // including animating ones
	uint32 nudge_list[MAX_bars];   // only player builds this - i cant be bothered with putting it in the player struct however

	uint32 number_of_barriers;  // kept for safety
	uint32 number_of_nudge;     // kept for safety
	uint32 number_of_animating; // kept for safety

	__mega_set_names next_anim_type; // queued anim to play after current link anim

	// turn on spot pan stuff
	PXfloat target_pan;        // actual relative distance to turn
	PXfloat actual_target_pan; // actual target pan to clip to if necessary

	PXfloat auto_target_pan; // target pan for auto engine pans - i.e. when routing\bumping

	PXfloat looking_pan;

	uint32 target_id;    // hurray! Its back again!! Id of target object
	PXvector target_xyz; // target coordinate - used for position correcting, etc

	__weapon weapon; // what weapon the mega is carrying
	__motion motion; // walking, running, used by high level anim functions

	// To maintain allignment, the sizes of these arrays must be a multiple of 4.
	char chr_name[MAX_CHAR_NAME_LENGTH];     // i.e. cord
	char anim_set[MAX_OUTFIT_NAME_LENGTH];   // i.e. casual_wear
	char custom_set[MAX_CUSTOM_NAME_LENGTH]; // i.e. mine

	// this is the real game-world coordinate
	PXvector actor_xyz;
	PXvector pushed_actor_xyz;

	uint32 m_phase;
	_route_description m_main_route;

	Breath breath;

	// the dynamic lamp (with one state only) and a switch as to whether it is on or off (if >0 then it is on for dynLightOn cycles, if
	// 0 then off, if -1 on until explicitly turned off)
	PSXLamp dynLight;                      // the lamp, filled in partly by logic and partly by stagedraw (rotating with character)
	int32 dynLightOn;                      // if 0-off, >0-on for n cycles (count down), -1 - constant until turned off
	int16 dynLightX, dynLightY, dynLightZ; // cm from centre of character (hip) (hip hurray!)

	SVECTOR bulletPos;
	int16 bulletDX;
	int16 bulletDY;
	int16 bulletInitialHeight;

	uint8 bulletBounced, bulletColour;

	bool8 bulletOn;

	// NOTE THAT THE FOLLOWING DATA MEMBERS HAVE BEEN ARRANGED TO MAKE 4-BYTE ALLIGNMENT CLEAR.

	// A status flag containing info if the MEGA was in view last cycle,
	// and is in view this cycle, a change in view state causes an event
	// to be sent to the MEGA saying "oncamera" "offcamera"
	bool8 pushed;
	uint8 viewState;
	bool8 interacting; // yes or no to avoid problems interacting with id=0 !!!!
	bool8 turn_dir;    // 0 right       1 left
	bool8 custom;
	bool8 has_exclusive_coords; // if yes then this object will not have coordinates saved to micro-session - player and chi

	bool8 is_evil;          // true if guard or robot, or some other horrible creature, false if lovely chi or some sort of scientist or whatever.
	bool8 make_remora_beep; // true for robots so the remora picks them up

	uint8 inShadePercentage; // how much light to be in shadow
	bool8 in_shade;          // Set to TRUE8 when mega is in shade, FALSE8 otherwise.
	bool8 use_strike_script; // call a script instead of hitting object
	bool8 use_fire_script;   // call a script instead of shooting object

	bool8 on_players_floor;
	uint8 speech_red;
	uint8 speech_green;
	uint8 speech_blue;

	uint8 anim_speed;    // 1 is normal
	bool8 reverse_route; // routing backwards
	bool8 is_shooting;   // are we firing the gun ?
	bool8 drawShadow;    // sometimes turned off

	// generic async
	bool8 asyncing;        // is loading a generic anim file
	uint8 async_list_pos;  // position in current file list/queue
	__weapon async_weapon; // current set used by asyncer

	// footsteps
	int8 footstep_status;   // current foot left/right status...
	uint8 footstep_weight;  // weight of footstep (100 is Cord) 255 maximum...
	bool8 footstep_special; // whether special or not...

	bool8 display_me; // high level character draw clip

	// generic stair-----------------------------------------------------------
	// stuff for shadow correction on stairs
	bool8 on_stairs;
	PXreal bottom_x, bottom_z;
	PXreal top_x, top_z;
	PXfloat stair_pan;

	uint8 extrap_size;           // route barrier extrapolation size
	bool8 dead;                  // dead or not?
	uint32 slice_hold_tolerance; // y distance to stray before slice hold kicks back in
	uint8 idle_count;            // how int32 just been stood

	// camera control
	bool8 y_locked;
	PXreal y_lock;

	void ___init();

	void InitCartridgeCase(SVECTOR *initPos, short initialHeight);

	bool8 Fetch_armed_status();
	bool8 Fetch_custom();
	__weapon Fetch_pose();
	bool8 Is_crouched();

	// Move this cycle view state flag into last cycle view state
	void ShiftViewState();
	// Set the view state for this cycle
	void SetThisViewState(enum CameraStateEnum status);

	void InitDynamicLight();

	// SetDynamicLight(cycles,r,g,b,x,y,z,falloff);
	// where cycles is number of cycles to stay on (-1 for constant, 0 for off)
	// falloff is maximum extent of light, falloff starts 10% of the way from it...
	void SetDynamicLight(int32 in_cycles, int32 in_r, int32 in_g, int32 in_b, int32 in_x, int32 in_y, int32 in_z, int32 falloff);

	void RemoveDynamicLight() { dynLightOn = 0; }
};

// Move this cycle view state flag into last cycle view state
inline void _mega::ShiftViewState() { viewState = (uint8)((viewState >> THIS_CYCLE_SHIFT) & (LAST_CYCLE_MASK)); }

// Set the view state for this cycle
// status is ON_CAMERA or OFF_CAMERA
inline void _mega::SetThisViewState(enum CameraStateEnum status) { viewState = (uint8)((viewState & LAST_CYCLE_MASK) | ((int)status << THIS_CYCLE_SHIFT)); }

enum _object_image_type { PROP, VOXEL };

enum _hold_mode {
	prop_camera_hold = 1,
	mega_player_floor_hold,  // hold when not on player floor
	mega_initial_floor_hold, // hold until on player floor then release forever
	mega_slice_hold,         // hold if not on plyer y then release after a set y distance
	none
};

enum _big_mode {
	__SCRIPT,
	__NO_LOGIC,
	__MEGA_SLICE_HELD,
	__MEGA_PLAYER_FLOOR_HELD,
	__MEGA_INITIAL_FLOOR_HELD,
	__CUSTOM_SIMPLE_ANIMATE,
	__CUSTOM_BUTTON_OPERATED_DOOR,
	__CUSTOM_AUTO_DOOR
};

#define MAX_list 10

#define THREE_SIXTY_INTERACT 2
#define PROP_CROUCH_INTERACT 1

// game object logic structure
class _logic {
public:
	uint32 ms_timer; // timer for this object DEBUGGING
	uint32 looping;
	uint32 old_looping;       // logic controller for script functions - such as animators
	uint32 pause;             // logic pause value
	uint32 anim_direction;    // 0 forward, else backward
	_object_status ob_status; // low level internal stuff - see enum _object_status   //held/not held...
	_big_mode big_mode;
	uint32 owner_floor_rect;       // current floor rect number
	uint32 total_list;             // number of items in list
	uint32 list[MAX_list];         // a list available to script logic
	uint32 list_result;            // tempory holding place for the result of list checking functions - that must return true/false
	uint32 sfxVars[3];             // up to 3 different sound effect modifiers per type of object (gunshot,ricochet,tinkle) (door open,door close)
	uint32 logic_level;            // what level?                  0 is the context switch, 1 is the main logic, 2+ are gosubs from level 1
	char *logic[TREE_SIZE];        // pointers to the raw compiled script data
	char *logic_ref[TREE_SIZE];    // holds the initial pointers
	_object_image_type image_type; // object is PROP or VOXEL based
	__object_type object_type;
	uint32 conversation_uid;       // id of conversation we may be in
	// type of current animation - use to index into voxel_image->anim_name;
	__mega_set_names cur_anim_type;
	uint32 anim_pc;         // pc of current animation - i.e. position in track\voxel frame file
	_vox_image *voxel_info; // extra graphic info
	_mega *mega;            // extra mega info
	// prop xyz coord derived from nico
	PXvector prop_xyz;
	PXfloat prop_interact_pan;
	PXfloat pan;        // engine pan
	PXfloat pan_adjust; // pan neutraliser for frames that include a pan from the grab - i.e. turn on spot
	// this is a distance to look up when looking at the object, only useful:
	int32 look_height; // -1 means not used so don't do...
	// auto panning stuff
	PXfloat auto_display_pan;
	//  timers
	uint32 cycle_time;
	uint32 slowest_cycle_time;
	_hold_mode hold_mode;

	int16 sparkleX;
	int16 sparkleY;

	int16 sparkleZ;
	bool8 player_can_interact; // interactable true or false
	bool8 context_request;     // crude switch to force context script rerun

	bool8 prop_coords_set;      // has a prop object been asigned coordinates - for development safety
	uint8 three_sixty_interact; // true then overides the normal tight pan approach to interaction - like a mega in-fact
	bool8 auto_panning;
	bool8 do_not_disturb; // If true, object will not rerun logic context.

	bool8 camera_held;
	bool8 prop_on_this_screen; // actually on this camera - used by closed doors
	bool8 sparkleOn;           // whether or not sparkle is on
	bool8 padding;             // Pad the structure

private:
	char ob_name[ENGINE_STRING_LEN];

public:
	void ___init(const char *name);
	void ___destruct();

	const char *GetName() const { return ob_name; }

	void GetPosition(PXreal &x, PXreal &y, PXreal &z) const {
		bool8 isActor = (bool8)(image_type == VOXEL);

		if (isActor) {
			x = mega->actor_xyz.x;
			y = mega->actor_xyz.y;
			z = mega->actor_xyz.z;
		} else {
			x = prop_xyz.x;
			y = prop_xyz.y;
			z = prop_xyz.z;
		}
	}
};

} // End of namespace ICB

#endif
