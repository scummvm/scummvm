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

#include "engines/icb/global_switches.h"
#include "engines/icb/session.h"
#include "engines/icb/animation_mega_set.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/debug.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/mission.h"
#include "engines/icb/p4.h"
#include "engines/icb/player.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/floors.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/common/px_floor_map.h"
#include "engines/icb/common/px_features.h"
#include "engines/icb/res_man.h"

#include "common/str.h"
#include "common/util.h"

namespace ICB {

// this is the master animation declaration table.
_an_anim_entry master_anim_name_table[__TOTAL_ANIMS] = {
	{"walk", __WALK},
	{"walk_to_stand", __WALK_TO_STAND},
	{"walk_to_other_stand_(left_leg)", __WALK_TO_OTHER_STAND_LEFT_LEG},
	{"walk_to_pull_out_weapon", __WALK_TO_PULL_OUT_WEAPON},

	{"stand", __STAND},
	{"stand_to_walk", __STAND_TO_WALK},
	{"stand_to_walk_upstairs_right", __STAND_TO_WALK_UP_STAIRS_RIGHT},
	{"stand_to_walk_downstairs_right", __STAND_TO_WALK_DOWN_STAIRS_RIGHT},
	{"stand_to_run", __STAND_TO_RUN},
	{"stand_to_step_backward", __STAND_TO_STEP_BACKWARD},
	{"stand_crouch_to_stand", __STAND_CROUCHED_TO_STAND},
	{"stand_crouch_to_walk_crouched", __STAND_CROUCHED_TO_WALK_CROUCHED},

	{"run", __RUN},
	{"run_to_stand", __RUN_TO_STAND},
	{"run_to_pull_out_weapon", __RUN_TO_PULL_OUT_WEAPON},

	{"use_card_on_slot", __USE_CARD_ON_SLOT},
	{"pick_up_object_from_floor", __PICK_UP_OBJECT_FROM_FLOOR},
	{"push_button", __PUSH_BUTTON},
	{"being_shot", __BEING_SHOT},
	{"being_shot_dead", __BEING_SHOT_DEAD},

	{"sidestep_left", __SIDESTEP_LEFT},
	{"step_backward", __STEP_BACKWARD},
	{"step_forward", __STEP_FORWARD},
	{"step_backward_to_stand", __STEP_BACKWARD_TO_STAND},
	{"step_backward_to_other_stand_(left_leg)", __STEP_BACKWARD_TO_OTHER_STAND_LEFT},

	{"stand_and_aim", __STAND_AND_AIM},
	{"stand_and_shoot", __STAND_AND_SHOOT},
	{"pull_out_weapon", __PULL_OUT_WEAPON},
	{"cord_strike", __STRIKE},
	{"low_strike", __LOW_STRIKE},
	{"hit_from_behind", __HIT_FROM_BEHIND},

	{"turn_on_the_spot_cw", __TURN_ON_THE_SPOT_CLOCKWISE},

	{"walk_upstairs_left", __WALK_UPSTAIRS_LEFT},
	{"walk_upstairs_right", __WALK_UPSTAIRS_RIGHT},
	{"walk_downstairs_left", __WALK_DOWNSTAIRS_LEFT},
	{"walk_downstairs_right", __WALK_DOWNSTAIRS_RIGHT},

	{"walk_upstairs_left_to_stood_on_stairs_facing_up", __WALK_UPSTAIRS_LEFT_TO_STOOD_ON_STAIRS_FACING_UP},
	{"walk_upstairs_right_to_stood_on_stairs_facing_up", __WALK_UPSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_UP},
	{"walk_downstairs_left_to_stood_on_stairs_facing_down", __WALK_DOWNSTAIRS_LEFT_TO_STOOD_ON_STAIRS_FACING_DOWN},
	{"walk_downstairs_right_to_stood_on_stairs_facing_down", __WALK_DOWNSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_DOWN},

	{"walk_upstairs_left_to_walk_downstairs_right", __WALK_UPSTAIRS_LEFT_TO_WALK_DOWNSTAIRS_RIGHT},
	{"walk_upstairs_right_to_walk_downstairs_left", __WALK_UPSTAIRS_RIGHT_TO_WALK_DOWNSTAIRS_LEFT},
	{"walk_downstairs_left_to_walk_upstairs_right", __WALK_DOWNSTAIRS_LEFT_TO_WALK_UPSTAIRS_RIGHT},
	{"walk_downstairs_right_to_walk_upstairs_left", __WALK_DOWNSTAIRS_RIGHT_TO_WALK_UPSTAIRS_LEFT},

	{"run_up_stairs_left", __RUN_UPSTAIRS_LEFT},
	{"run_up_stairs_right", __RUN_UPSTAIRS_RIGHT},
	{"run_down_stairs_left", __RUN_DOWNSTAIRS_LEFT},
	{"run_down_stairs_right", __RUN_DOWNSTAIRS_RIGHT},

	{"climb_up_ladder_left", __CLIMB_UP_LADDER_LEFT},
	{"climb_up_ladder_right", __CLIMB_UP_LADDER_RIGHT},
	{"climb_down_ladder_left", __CLIMB_DOWN_LADDER_LEFT},
	{"climb_down_ladder_right", __CLIMB_DOWN_LADDER_RIGHT},

	{"climb_up_ladder_right_to_stand", __CLIMB_UP_LADDER_RIGHT_TO_STAND},
	{"climb_down_ladder_right_to_stand", __CLIMB_DOWN_LADDER_RIGHT_TO_STAND},

	{"cord_stand_to_climb_up_ladder", __CORD_STAND_TO_CLIMB_UP_LADDER},

	{"stand_to_climb_up_ladder_right", __STAND_TO_CLIMB_UP_LADDER_RIGHT},
	{"stand_to_climb_down_ladder_right", __STAND_TO_CLIMB_DOWN_LADDER_RIGHT},

	{"climb_down_ladder_left_to_slide_down_ladder", __CLIMB_DOWN_LADDER_LEFT_TO_SLIDE_DOWN_LADDER},
	{"slide_down_ladder", __SLIDE_DOWN_LADDER},
	{"slide_down_ladder_to_stand", __SLIDE_DOWN_LADDER_TO_STAND},

	{"load_gun", __LOAD_GUN},
	{"load_gun_2", __LOAD_GUN_2},
	{"load_gun2", __LOAD_GUN_CROUCH_2},

	{"cowering", __COWER},
	{"cowering_to_stand", __COWER_TO_STAND},
	{"run_hand_through_hair", __HAND_HAIR},
	{"shrug", __SHRUG},
	{"look_at_watch", __LOOK_AT_WATCH},
	{"stretch", __STRETCH},
	{"scratching_backside", __SCRATCH},
};

// these names must be in same order as __weapon
char weapon_text[__TOTAL_WEAPONS][MAX_WEAPON_NAME_LENGTH] = {"unarmed", "gun", "crouched", "crouched_gun"};

bool8 armed_state_table[__TOTAL_WEAPONS] = {FALSE8, TRUE8, FALSE8, TRUE8};

_player_stat player_stat_table[__TOTAL_WEAPONS] = {STOOD, NEW_AIM, CROUCHING, CROUCH_AIM};

bool8 crouch_state_table[__TOTAL_WEAPONS] = {FALSE8, FALSE8, TRUE8, TRUE8};

void _vox_image::___init(const char *chr, const char *set, __weapon weapon) {
	// Clear the override pose hash value
	Cancel_override_pose();

	// store these things temporarily so we can recall this function when swapping voxel -> polygon and vice verse...
	Common::strcpy_s(temp_chr, chr);
	Common::strcpy_s(temp_set, set);
	temp_weapon = weapon;

	// constructor for mega-set-caps class
	// resolve the path of the voxel image directory

	// we are passed the character name AND the graphic set
	// for example, cord, wetsuit

	// this object is created by _game_session::fn_set_voxel_image_path

	// where chr='cord'
	// set='casual_wear'

	int32 k, len;

	// check for no weapon being set
	if (weapon == __NOT_SET)
		Fatal_error("WARNING %s does not have a weapon type", MS->Fetch_object_name(MS->Fetch_cur_id()));

	palette_hash = NULL_HASH;

// get base path
	// Make hash filename of the character
	char chr_hash[8];
	HashFile(chr, chr_hash);

	// Make hash filename of the outfit
	char set_hash[8];
	HashFile(set, set_hash);

	// Make the cluster name "\c\<#character>\<#outfit>\outfit.clu"
	len = Common::sprintf_s(base_path, CHR_PATH, chr_hash, set_hash);
	if (len > BASE_PATH_STR_LEN)
		Fatal_error("_vox_image::___init base_path string too long");
	base_path_hash = NULL_HASH;

	Zdebug("make base path == %s from %s %s\n", (const char *)base_path, chr, set);

	// In the clustered version the image path is the path inside the cluster

	len = Common::sprintf_s(image_path, "%s\\", weapon_text[weapon]);
	if (len > IMAGE_PATH_STR_LEN)
		Fatal_error("_vox_image::___init image_path [%s] string too long", image_path);

	len = Common::sprintf_s(shadow_mesh_name, "%s", "mesh_shadow.rap");
	if (len > IMAGE_PATH_STR_LEN)
		Fatal_error("_vox_image::___init shadow_mesh_name [%s] string too long", shadow_mesh_name);

	len = Common::sprintf_s(pose_name, "%s\\pose.rap", weapon_text[weapon]);
	if (len > IMAGE_PATH_STR_LEN)
		Fatal_error("_vox_image::___init pose_name [%s] string too long", pose_name);

	pose_hash = HashString(pose_name);
	shadow_mesh_hash = HashString(shadow_mesh_name);

	// Make the hash value for this cluster name
	base_path_hash = HashString(base_path);

	// Make the hash value for this cluster name
	base_path_hash = HashString(base_path);

	Zdebug("image path == %s\n", (const char *)image_path);
	Zdebug("base path == %s\n", (const char *)base_path);

	for (k = 0; k < __NON_GENERIC; k++) {
		anim_table[k] = (int8)-1;
	}

	if (((g_mission) && (g_mission->session)) && (MS->Fetch_cur_id() != 999)) {
		MS->logic_structs[MS->Fetch_cur_id()]->cur_anim_type = __STAND;
		MS->logic_structs[MS->Fetch_cur_id()]->anim_pc = 0;
	}

	has_custom_path_built = FALSE8; // no custom animation exists

	Zdebug("\n-------------------------------------------------------------------------------\n");
}

void _vox_image::MakeAnimEntry(int32 i) {
	Common::String strName;

	strName = Common::String::format("%s%s.rab", (const char *)image_path, (const char *)master_anim_name_table[i].name);

	if (strName.size() > ANIM_NAME_STR_LEN) {
		Fatal_error("_vox_image::___init [%s] string too long", strName.c_str());
	}
	Common::strcpy_s(anim_name[i], strName.c_str());

	anim_name_hash[i] = HashString(anim_name[i]);

	strName = Common::String::format("%s%s.raj", (const char *)image_path, (const char *)master_anim_name_table[i].name);

	if (strName.size() > ANIM_NAME_STR_LEN) {
		Fatal_error("_vox_image::___init [%s] string too long", strName.c_str());
	}
	Common::strcpy_s(info_name[i], strName.c_str());

	info_name_hash[i] = HashString(info_name[i]);

	// do the test file
	anim_table[i] = (int8)(rs_anims->Test_file(get_anim_name(i), anim_name_hash[i], base_path, base_path_hash));
}

bool8 _vox_image::Init_custom_animation(const char *anim) {
	// init a non generic animation in its special __NON_GENERIC slot
	// this does not ensure the anim exists
	bool8 custom;

	char custom_image_path_rav[128];
	char custom_image_path_rai[128];

	uint32 len;

	len = strlen(anim);
	uint32 j;
	for (j = 0; j < len; j++)
		if (Common::isUpper(*(anim + j)))
			Fatal_error("Init_custom_animation finds [%s] has upper case letters - implementor must edit the script", anim);

	// has anyone been and built the path before?
	if (has_custom_path_built) {
		has_custom_path_built = FALSE8; // remove it
		return (TRUE8);                 // carry on
	}

	custom = MS->Fetch_custom();

	// check for no weapon being set
	if (custom == FALSE8 /*__NONE*/) {
		// custom must be in the current weapon set - bah, shouldn't have done it like this - it's daft
		
		// rav (or equivalent) always come from pcp directory...
		len = Common::sprintf_s(custom_image_path_rav, "%s\\", weapon_text[MS->Fetch_cur_megas_pose()]);

		// rai (or equivalent) always come from base path...
		len = Common::sprintf_s(custom_image_path_rai, "%s\\", weapon_text[MS->Fetch_cur_megas_pose()]);

		// pose mesh name
		len = Common::sprintf_s(custom_pose_name, "%s\\pose.rap", weapon_text[MS->Fetch_cur_megas_pose()]);

		custom_pose_hash = HashString(custom_pose_name);
	} else {
		// we have specified a custom type - i.e. the anim is not part of the current weapon set, but
		// instead sits parallel to weapon directory
		len = Common::sprintf_s(custom_image_path_rav, "%s\\", MS->Fetch_cur_megas_custom_text());
		len = Common::sprintf_s(custom_image_path_rai, "%s\\", MS->Fetch_cur_megas_custom_text());
		len = Common::sprintf_s(custom_pose_name, "%s\\pose.rap", MS->Fetch_cur_megas_custom_text());

		if (len > 128)
			Fatal_error("Init_custom_animation string error");
		custom_pose_hash = HashString(custom_pose_name);

	}

	len = Common::sprintf_s(anim_name[__NON_GENERIC], "%s%s.rab", (const char *)custom_image_path_rav, (const char *)anim);

	if (len > ANIM_NAME_STR_LEN)
		Fatal_error("Init_custom_animation string error");
	anim_name_hash[__NON_GENERIC] = HashString(anim_name[__NON_GENERIC]);

	len = Common::sprintf_s(info_name[__NON_GENERIC], "%s%s.raj", (const char *)custom_image_path_rai, (const char *)anim);
	if (len > ANIM_NAME_STR_LEN)
		Fatal_error("Init_custom_animation string error");
	info_name_hash[__NON_GENERIC] = HashString(info_name[__NON_GENERIC]);

	anim_table[__NON_GENERIC] = 1;

	if (!rs_anims->Test_file(get_anim_name(__NON_GENERIC), anim_name_hash[__NON_GENERIC], base_path, base_path_hash)) {
		Fatal_error("custom anim [%s,%08x] not found in cluster %s", (const char *)anim_name[__NON_GENERIC], anim_name_hash[__NON_GENERIC], base_path);
	}

	Zdebug(" created [%s]", (const char *)anim_name[__NON_GENERIC]);

	return (TRUE8);
}

void _vox_image::Promote_non_generic() {
	// copy non-generic path to safe __PROMOTED_NON_GENERIC slot
	// this is all for psx asyncing

	memcpy(&anim_name[__PROMOTED_NON_GENERIC], &anim_name[__NON_GENERIC], ANIM_NAME_STR_LEN);
	memcpy(&info_name[__PROMOTED_NON_GENERIC], &info_name[__NON_GENERIC], ANIM_NAME_STR_LEN);

	info_name_hash[__PROMOTED_NON_GENERIC] = info_name_hash[__NON_GENERIC];
	anim_name_hash[__PROMOTED_NON_GENERIC] = anim_name_hash[__NON_GENERIC];

	anim_table[__PROMOTED_NON_GENERIC] = 1; // hack this
}

bool8 _vox_image::Find_anim_type(__mega_set_names *anim, const char *name) {
	uint32 k;

	for (k = 0; k < __TOTAL_ANIMS; k++) {
		// we must search the table

		if (!strcmp(name, master_anim_name_table[k].name)) {
			*(anim) = master_anim_name_table[k].ref;
			return (TRUE8);
		}
	}

	return (FALSE8);
}

bool8 _game_session::Start_generic_ascii_anim(const char *ascii_name) {
	// search for named anim and setup if found
	uint32 k;

	// search for the named generic anim - can't use __ANIM_NAME from script unfortunately
	for (k = 0; k < __TOTAL_ANIMS; k++) {
		// we must search the table

		if (!strcmp(const_cast<char *>(ascii_name), master_anim_name_table[k].name)) {
			Zdebug("  Start_generic_ascii_anim found [%s]", ascii_name);

			L->cur_anim_type = master_anim_name_table[k].ref;
			L->anim_pc = 0;

			// Check to see if this anim exists on the hard drive
			// Note this will also make the name entry correctly if the name hasn't already
			// been made
			if (I->IsAnimTable(L->cur_anim_type) == (int8)-1)
				return (FALSE8);

			return (TRUE8);
		}
	}

	Zdebug("nightmare!");

	return (FALSE8);
}

__mega_set_names _game_session::Fetch_generic_anim_from_ascii(const char *ascii_name) {
	// pass name of a generic anim and return the type
	uint32 k;

	// search for the named generic anim - can't use __ANIM_NAME from script unfortunately
	for (k = 0; k < __TOTAL_ANIMS; k++) {
		if (!strcmp(const_cast<char *>(ascii_name), master_anim_name_table[k].name)) {
			// found!
			if (I->IsAnimTable(L->cur_anim_type) == (int8)-1)
				Fatal_error("Fetch_generic_anim_from_ascii can't find on drive %s", ascii_name);
			return (master_anim_name_table[k].ref);
		}
	}

	Fatal_error("Fetch_generic_anim_from_ascii can't find %s", ascii_name);
	return __NON_GENERIC;
}

void PreRegisterTexture(const char *, uint32, const char *, uint32, const char *, uint32);

bool8 _vox_image::Set_texture(const char *tex_name) {
	int32 len;

	len = Common::sprintf_s(texture_name, "%s.revtex", tex_name);

	if (len > IMAGE_PATH_STR_LEN)
		Fatal_error("_vox_image::Set_texture [%s] string too long", tex_name);

	texture_hash = HashString(texture_name);

	// set palette to be same as texture

	Common::strcpy_s(palette_name, texture_name);
	palette_hash = texture_hash;

	// okay preload the texture/palette combo
	PreRegisterTexture(texture_name, texture_hash, palette_name, palette_hash, base_path, base_path_hash);

	return TRUE8;
}

bool8 _vox_image::Set_mesh(const char *m_name) {
	char name[32];
	int32 len;

	Common::strcpy_s(name, m_name);
	Common::strcat_s(name, ".rap");

	len = Common::sprintf_s(mesh_name, "%s", name);
	if (len > IMAGE_PATH_STR_LEN)
		Fatal_error("_vox_image::___init mesh_name [%s] string too long", mesh_name);

	mesh_hash = HashString(mesh_name);
	return TRUE8;
}

bool8 _vox_image::Set_palette(const char *pal_name) {
	// Ignore the default_palette : this is yucky and hacky but it is the end of the project so tough luck
	if (strcmp(pal_name, "default") == 0) {
		palette_hash = NULL_HASH;
		return TRUE8;
	}

	int32 len;

	len = Common::sprintf_s(palette_name, "%s.revtex", pal_name);

	if (len > IMAGE_PATH_STR_LEN)
		Fatal_error("_vox_image::Set_palette [%s] string too long", pal_name);

	palette_hash = HashString(palette_name);

	// okay preload the texture/palette combo
	PreRegisterTexture(texture_name, texture_hash, palette_name, palette_hash, base_path, base_path_hash);

	return TRUE8;
}

bool8 _vox_image::Set_override_pose(const char *p_name) {
	int len = Common::sprintf_s(override_pose_name, "%s\\pose.rap", p_name);

	if (len > IMAGE_PATH_STR_LEN)
		Fatal_error("_vox_image::Set_override_pose [%s] string too long", override_pose_name);

	override_pose_hash = HashString(override_pose_name);

	return TRUE8;
}

bool8 _vox_image::Cancel_override_pose() {
	override_pose_hash = NULL_HASH;
	override_pose_name[0] = '\0';

	return TRUE8;
}

// Async load a file from a character cluster into rs_anims
// Return 0 - file is not in memory
// Return 1 - file is in memory
int _vox_image::Preload_file(const char *file) {
	char file_name[ENGINE_STRING_LEN];
	int len = Common::sprintf_s(file_name, "%s", file);

	if (len > IMAGE_PATH_STR_LEN)
		Fatal_error("_vox_image::Preload_file [%s] string too long", file_name);

	uint32 fileHash = NULL_HASH;

	// The PC version does not use async loading
	if (rs_anims->Res_open(file_name, fileHash, base_path, base_path_hash))
		return 1;

	return 0;
}

} // End of namespace ICB
