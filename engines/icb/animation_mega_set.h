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

#ifndef ICB_ANIMATION_MEGA_SET
#define ICB_ANIMATION_MEGA_SET

#include "engines/icb/common/px_string.h"
#include "engines/icb/common/px_rccommon.h"

namespace ICB {

#define MAX_ANIM_NAME_LENGTH 64

enum __mega_set_names {
	__WALK,
	__WALK_TO_STAND,
	__WALK_TO_OTHER_STAND_LEFT_LEG,
	__WALK_TO_PULL_OUT_WEAPON,

	__STAND,
	__STAND_TO_WALK,
	__STAND_TO_WALK_UP_STAIRS_RIGHT,
	__STAND_TO_WALK_DOWN_STAIRS_RIGHT,
	__STAND_TO_RUN,
	__STAND_TO_STEP_BACKWARD,
	__STAND_CROUCHED_TO_STAND,
	__STAND_CROUCHED_TO_WALK_CROUCHED,

	__RUN,
	__RUN_TO_STAND,
	__RUN_TO_PULL_OUT_WEAPON,

	__USE_CARD_ON_SLOT,
	__PICK_UP_OBJECT_FROM_FLOOR,
	__PUSH_BUTTON,
	__BEING_SHOT,
	__BEING_SHOT_DEAD,

	__SIDESTEP_LEFT,
	__STEP_BACKWARD,
	__STEP_FORWARD,
	__STEP_BACKWARD_TO_STAND,
	__STEP_BACKWARD_TO_OTHER_STAND_LEFT,

	__STAND_AND_AIM,
	__STAND_AND_SHOOT,
	__PULL_OUT_WEAPON,
	__STRIKE,
	__LOW_STRIKE,
	__HIT_FROM_BEHIND,

	__TURN_ON_THE_SPOT_CLOCKWISE,

	__WALK_UPSTAIRS_LEFT,
	__WALK_UPSTAIRS_RIGHT,
	__WALK_DOWNSTAIRS_LEFT,
	__WALK_DOWNSTAIRS_RIGHT,

	__WALK_UPSTAIRS_LEFT_TO_STOOD_ON_STAIRS_FACING_UP,
	__WALK_UPSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_UP,
	__WALK_DOWNSTAIRS_LEFT_TO_STOOD_ON_STAIRS_FACING_DOWN,
	__WALK_DOWNSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_DOWN,

	__WALK_UPSTAIRS_LEFT_TO_WALK_DOWNSTAIRS_RIGHT,
	__WALK_UPSTAIRS_RIGHT_TO_WALK_DOWNSTAIRS_LEFT,
	__WALK_DOWNSTAIRS_LEFT_TO_WALK_UPSTAIRS_RIGHT,
	__WALK_DOWNSTAIRS_RIGHT_TO_WALK_UPSTAIRS_LEFT,

	__RUN_UPSTAIRS_LEFT,
	__RUN_UPSTAIRS_RIGHT,
	__RUN_DOWNSTAIRS_LEFT,
	__RUN_DOWNSTAIRS_RIGHT,

	__CLIMB_UP_LADDER_LEFT,
	__CLIMB_UP_LADDER_RIGHT,
	__CLIMB_DOWN_LADDER_LEFT,
	__CLIMB_DOWN_LADDER_RIGHT,

	__CLIMB_UP_LADDER_RIGHT_TO_STAND,
	__CLIMB_DOWN_LADDER_RIGHT_TO_STAND,

	__CORD_STAND_TO_CLIMB_UP_LADDER,

	__STAND_TO_CLIMB_UP_LADDER_RIGHT,
	__STAND_TO_CLIMB_DOWN_LADDER_RIGHT,

	__CLIMB_DOWN_LADDER_LEFT_TO_SLIDE_DOWN_LADDER,
	__SLIDE_DOWN_LADDER,
	__SLIDE_DOWN_LADDER_TO_STAND,

	__LOAD_GUN,
	__LOAD_GUN_2,
	__LOAD_GUN_CROUCH_2,

	__COWER,
	__COWER_TO_STAND,
	__HAND_HAIR,
	__SHRUG,
	__LOOK_AT_WATCH,
	__STRETCH,
	__SCRATCH,

	__NON_GENERIC,          // a non generic specific - initialised at run time
	__PROMOTED_NON_GENERIC, // a generic gets promoted here where it wilk be safe from new generic paths being formed - done for psx
	                        // asyncing

	__NO_LINK, // can be used by _player::Start_new_mode
	__NO_ANIM,
	__TOTAL_ANIMS
};

typedef struct {
	char name[MAX_ANIM_NAME_LENGTH]; // the name minus .RAV & .RAI
	__mega_set_names ref;
} _an_anim_entry;

extern _an_anim_entry master_anim_name_table[__TOTAL_ANIMS];

} // End of namespace ICB

#endif
