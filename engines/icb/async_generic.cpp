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

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/debug.h"
#include "engines/icb/session.h"
#include "engines/icb/actor.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/res_man.h"
#include "engines/icb/mission.h"

namespace ICB {

#define MAX_async_anims_per_state 24
#define MAX_mega_async_anims_per_state 24
__mega_set_names player_generic_anim_async_table[__TOTAL_PLAYER_MODES][MAX_async_anims_per_state] = {
    // STOOD,   1
    {__STAND, __BEING_SHOT, __BEING_SHOT_DEAD, __STAND_TO_WALK, __TURN_ON_THE_SPOT_CLOCKWISE, __WALK, __PULL_OUT_WEAPON, __STAND_AND_SHOOT, __STEP_BACKWARD, __SIDESTEP_LEFT,
     __STRIKE, __WALK_TO_STAND, __LOAD_GUN, __LOAD_GUN_2,

     __WALK_TO_PULL_OUT_WEAPON,

     __CLIMB_DOWN_LADDER_LEFT, __CLIMB_DOWN_LADDER_RIGHT, __SLIDE_DOWN_LADDER, __CLIMB_UP_LADDER_LEFT, __CLIMB_UP_LADDER_RIGHT, __CLIMB_UP_LADDER_RIGHT_TO_STAND,
     __SLIDE_DOWN_LADDER_TO_STAND, __CLIMB_DOWN_LADDER_RIGHT_TO_STAND,

     __NO_ANIM},

    // WALKING, 2
    {__WALK_TO_PULL_OUT_WEAPON, __WALK_TO_STAND, __WALK_TO_OTHER_STAND_LEFT_LEG, __RUN, __BEING_SHOT, __BEING_SHOT_DEAD, __HIT_FROM_BEHIND, __STRIKE,
     __CORD_STAND_TO_CLIMB_UP_LADDER, __STAND_TO_CLIMB_DOWN_LADDER_RIGHT,

     __WALK_UPSTAIRS_LEFT, __WALK_UPSTAIRS_RIGHT, __WALK_DOWNSTAIRS_LEFT, __WALK_DOWNSTAIRS_RIGHT, __WALK_UPSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_UP,

     __CLIMB_DOWN_LADDER_LEFT, __CLIMB_DOWN_LADDER_RIGHT, __SLIDE_DOWN_LADDER, __CLIMB_UP_LADDER_LEFT, __CLIMB_UP_LADDER_RIGHT, __CLIMB_UP_LADDER_RIGHT_TO_STAND,
     __SLIDE_DOWN_LADDER_TO_STAND, __CLIMB_DOWN_LADDER_RIGHT_TO_STAND,

     __NO_ANIM},

    // CROUCH_WALK, 3
    {__STAND_CROUCHED_TO_STAND, __NO_ANIM},
    // RUNNING,     4
    {__RUN_TO_STAND, __WALK, __CORD_STAND_TO_CLIMB_UP_LADDER, __STAND_TO_CLIMB_DOWN_LADDER_RIGHT, __RUN_UPSTAIRS_LEFT, __RUN_UPSTAIRS_RIGHT, __RUN_DOWNSTAIRS_LEFT,
     __RUN_DOWNSTAIRS_RIGHT, __STRIKE, __BEING_SHOT, __BEING_SHOT_DEAD,

     __CLIMB_DOWN_LADDER_LEFT, __CLIMB_DOWN_LADDER_RIGHT, __SLIDE_DOWN_LADDER, __CLIMB_UP_LADDER_LEFT, __CLIMB_UP_LADDER_RIGHT, __CLIMB_UP_LADDER_RIGHT_TO_STAND,
     __SLIDE_DOWN_LADDER_TO_STAND, __CLIMB_DOWN_LADDER_RIGHT_TO_STAND,

     __NO_ANIM},
    // crouch to punch 5
    {__NO_ANIM},

    // CROUCH_TO_STAND_UNARMED, 6
    {__NO_ANIM},
    // CROUCH_TO_STAND_ARMED,       7
    {__NO_ANIM},

    // CROUCHING,       8
    {__STAND_CROUCHED_TO_STAND, __TURN_ON_THE_SPOT_CLOCKWISE, __NO_ANIM},

    // NEW_AIM, 9
    {__TURN_ON_THE_SPOT_CLOCKWISE, __STEP_BACKWARD, __STEP_FORWARD, __SIDESTEP_LEFT, __PULL_OUT_WEAPON, __BEING_SHOT, __BEING_SHOT_DEAD, __LOAD_GUN, __LOAD_GUN_2, __NO_ANIM},

    // CROUCH_AIM,  10
    {__STAND_CROUCHED_TO_STAND, __TURN_ON_THE_SPOT_CLOCKWISE, __STEP_BACKWARD, __SIDESTEP_LEFT, __PULL_OUT_WEAPON, __LOAD_GUN, __LOAD_GUN_CROUCH_2, __NO_ANIM},

    // PUTTING_AWAY_GUN,    11
    {__NO_ANIM},
    // PUTTING_AWAY_CROUCH_GUN, 12
    {__NO_ANIM},

    // LINKING, 13
    {__NO_ANIM},
    // GUN LINKING, 14
    {__NO_ANIM},

    // FAST_LINKING,    15
    {__NO_ANIM},
    // EASY_LINKING,    16
    {__NO_ANIM},
    // REVERSE_LINKING, 17
    {__NO_ANIM},
    // FAST_REVERSE_LINKING,    18
    {__NO_ANIM},
    // STILL_LINKING,   19
    {__NO_ANIM},
    // INVENTORY,   20
    {__NO_ANIM},
    // REMORA,  21
    {__NO_ANIM},

    // STRIKING,    22
    {__STAND, __NO_ANIM},

    // ON_STAIRS,   23
    {__WALK_UPSTAIRS_LEFT, __WALK_UPSTAIRS_RIGHT, __WALK_DOWNSTAIRS_LEFT, __WALK_DOWNSTAIRS_RIGHT,

     __WALK_UPSTAIRS_RIGHT_TO_WALK_DOWNSTAIRS_LEFT, __WALK_UPSTAIRS_LEFT_TO_WALK_DOWNSTAIRS_RIGHT, __WALK_UPSTAIRS_LEFT_TO_STOOD_ON_STAIRS_FACING_UP,
     __WALK_UPSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_UP, __WALK_DOWNSTAIRS_RIGHT_TO_WALK_UPSTAIRS_LEFT, __WALK_DOWNSTAIRS_LEFT_TO_WALK_UPSTAIRS_RIGHT,
     __WALK_DOWNSTAIRS_LEFT_TO_STOOD_ON_STAIRS_FACING_DOWN, __WALK_DOWNSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_DOWN,

     __NO_ANIM},

    // RUNNING_ON_STAIRS,   24
    {__RUN_UPSTAIRS_LEFT, __RUN_UPSTAIRS_RIGHT, __RUN_DOWNSTAIRS_LEFT, __RUN_DOWNSTAIRS_RIGHT,

     __WALK_UPSTAIRS_RIGHT_TO_WALK_DOWNSTAIRS_LEFT, __WALK_UPSTAIRS_LEFT_TO_WALK_DOWNSTAIRS_RIGHT, __WALK_UPSTAIRS_LEFT_TO_STOOD_ON_STAIRS_FACING_UP,
     __WALK_UPSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_UP, __WALK_DOWNSTAIRS_RIGHT_TO_WALK_UPSTAIRS_LEFT, __WALK_DOWNSTAIRS_LEFT_TO_WALK_UPSTAIRS_RIGHT,
     __WALK_DOWNSTAIRS_LEFT_TO_STOOD_ON_STAIRS_FACING_DOWN, __WALK_DOWNSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_DOWN,

     __NO_ANIM},

    // STOOD_ON_STAIRS, 25
    {__WALK_UPSTAIRS_LEFT, __WALK_UPSTAIRS_RIGHT, __WALK_DOWNSTAIRS_LEFT, __WALK_DOWNSTAIRS_RIGHT,

     __WALK_UPSTAIRS_RIGHT_TO_WALK_DOWNSTAIRS_LEFT, __WALK_UPSTAIRS_LEFT_TO_WALK_DOWNSTAIRS_RIGHT, __WALK_DOWNSTAIRS_RIGHT_TO_WALK_UPSTAIRS_LEFT,
     __WALK_DOWNSTAIRS_LEFT_TO_WALK_UPSTAIRS_RIGHT, __NO_ANIM},

    // REVERSE_ON_STAIRS,   26
    {__NO_ANIM},

    // ON_LADDER,   27
    {__CLIMB_UP_LADDER_LEFT, __CLIMB_UP_LADDER_RIGHT, __CLIMB_DOWN_LADDER_LEFT, __CLIMB_DOWN_LADDER_RIGHT, __CLIMB_UP_LADDER_RIGHT_TO_STAND, __SLIDE_DOWN_LADDER,
     __SLIDE_DOWN_LADDER_TO_STAND, __CLIMB_DOWN_LADDER_RIGHT_TO_STAND, __NO_ANIM},

    // BEGIN_DOWN_LADDER,   28
    {__CLIMB_DOWN_LADDER_LEFT, __CLIMB_DOWN_LADDER_RIGHT, __SLIDE_DOWN_LADDER, __CLIMB_UP_LADDER_LEFT, __CLIMB_UP_LADDER_RIGHT, __CLIMB_UP_LADDER_RIGHT_TO_STAND,
     __SLIDE_DOWN_LADDER_TO_STAND, __CLIMB_DOWN_LADDER_RIGHT_TO_STAND, __NO_ANIM},

    // LEAVE_LADDER,    29
    {__STAND, __TURN_ON_THE_SPOT_CLOCKWISE, __STAND_TO_WALK, __NO_ANIM},

    // LEAVE_LADDER_BOTTOM, 30
    {__STAND, __TURN_ON_THE_SPOT_CLOCKWISE, __STAND_TO_WALK, __NO_ANIM},

    // SLIP_SLIDIN_AWAY,    31
    {__SLIDE_DOWN_LADDER_TO_STAND, __NO_ANIM},
    // finish reloading 32
    {__NO_ANIM},

    // fin-normal-reload 33
    {__NO_ANIM},

    // fin-normal-crouch-reload
    {__NO_ANIM}};

mcodeFunctionReturnCodes fn_preload_basics(int32 &result, int32 *params) { return (MS->fn_preload_basics(result, params)); }

	// function loads some standard anim and marker files in, designed to be used at session
	// init. this function run for a few megas (ie the ones that are on screen)
	// should ease off the preloader slightly and create less chug on psx...!
mcodeFunctionReturnCodes _game_session::fn_preload_basics(int32 &, int32 *) {
	// if this is init script, then I is not set yet, so do so now...
	I = L->voxel_info;

	rs_anims->Res_open_mini_cluster(I->base_path, I->base_path_hash, I->base_path, I->base_path_hash);

	// okay
	return (IR_CONT);
}

__mega_set_names evil_mega_always_in_memory[] = {__BEING_SHOT, __BEING_SHOT_DEAD, __HIT_FROM_BEHIND, __NO_ANIM};

__mega_set_names good_mega_always_in_memory[] = {__STAND, __NO_ANIM};

__mega_set_names mega_generic_anim_async_table[__TOTAL_WEAPONS][MAX_mega_async_anims_per_state] = {
    // stood unarmed
    {__STAND, __TURN_ON_THE_SPOT_CLOCKWISE, __WALK, __WALK_TO_STAND, __WALK_TO_OTHER_STAND_LEFT_LEG, __RUN, __BEING_SHOT, __BEING_SHOT_DEAD,

     __COWER, __COWER_TO_STAND, __HAND_HAIR,

     __STAND_TO_WALK_DOWN_STAIRS_RIGHT, __STAND_TO_WALK_UP_STAIRS_RIGHT, __WALK_UPSTAIRS_RIGHT, __WALK_UPSTAIRS_LEFT, __WALK_DOWNSTAIRS_RIGHT, __WALK_DOWNSTAIRS_LEFT,
     __STAND_TO_CLIMB_DOWN_LADDER_RIGHT, __STAND_TO_CLIMB_UP_LADDER_RIGHT, __CLIMB_UP_LADDER_LEFT, __CLIMB_UP_LADDER_RIGHT, __CLIMB_DOWN_LADDER_LEFT, __CLIMB_DOWN_LADDER_RIGHT,

     __NO_ANIM},
    // stood gun
    {__STAND, __TURN_ON_THE_SPOT_CLOCKWISE, __STAND_AND_AIM, __PULL_OUT_WEAPON, __STAND_AND_SHOOT, __BEING_SHOT, __BEING_SHOT_DEAD,

     // guard idle anims
     __LOOK_AT_WATCH, __STRETCH, __SCRATCH,

     __STAND_TO_WALK_DOWN_STAIRS_RIGHT, __STAND_TO_WALK_UP_STAIRS_RIGHT, __WALK_UPSTAIRS_RIGHT, __WALK_UPSTAIRS_LEFT, __WALK_DOWNSTAIRS_RIGHT, __WALK_DOWNSTAIRS_LEFT,

     __NO_ANIM},
    // crouched unarmed
    {__NO_ANIM},
    // crouched gun
    {__STAND, __STAND_CROUCHED_TO_STAND, __BEING_SHOT, __BEING_SHOT_DEAD,

     __NO_ANIM}};

bool8 Check_preload(__mega_set_names anim, int on_screen, _vox_image *I) {
	// anim exists so will need preloading
	if (I->IsAnimTable(anim)) {
		// check info file, otherwise return false
		if (rs_anims->Res_async_open(I->get_info_name(anim), I->info_name_hash[anim], I->base_path, I->base_path_hash) == 0)
			return FALSE8;

		// if onscreen we'll also need the anim file so check it's in...
		if ((on_screen) && (rs_anims->Res_async_open(I->get_anim_name(anim), I->anim_name_hash[anim], I->base_path, I->base_path_hash) == 0))
			return FALSE8;
	}

	// anim doesn't exist so we are okay since we won't be using it...
	// or all anims in memory so we're okay...
	return TRUE8;
}

#define ASYNC_GAME_CYCLES 3           // 1/4 second
#define TOO_MANY_DEFRAGS 2            // 2/3 per cycle
#define ENOUGH_OLD_MEMORY (10 * 1024) // 20k
#define AMOUNT_TO_GRAB (50 * 1024)    // 50k

// helps out the asyncer by perhaps turning it off on ocasion or something
void _game_session::Async_helper() {
	async_counter++;

	// triggered each second
	if (async_counter >= ASYNC_GAME_CYCLES) {

		// turn async on by default every second..
		async_off = 0;

		// too many defrags in last second
		if (rs_anims->amount_of_defrags >= TOO_MANY_DEFRAGS) {
			// enough old memory to make it worth doing a big flush, on the basis that the resources flushed out will probabily
			// not be used...
			if (rs_anims->Fetch_old_memory(ASYNC_GAME_CYCLES) > ENOUGH_OLD_MEMORY) {
				const char *flush_cluster = "mem";
				uint32 flush_cluster_hash = HashString(flush_cluster);
				uint32 flush_file_hash = 0xbabe;

				// grab the memory
				rs_anims->Res_alloc(flush_file_hash, flush_cluster, flush_cluster_hash, AMOUNT_TO_GRAB);

				// now purge it out...
				rs_anims->Res_purge(NULL, flush_file_hash, flush_cluster, flush_cluster_hash);

				rs_anims->Defrag();
			} else {
				// all the memory is being used so just switch ther asyncer off for this second...
				async_off = 1;
			}
		}

		// reset

		rs_anims->amount_of_defrags = 0;
		async_counter = 0;
	}
}

void _game_session::Service_generic_async() {
	// called for each mega servicing their generic cache ahead needs
	// done after each mega is processes as this is nearest to the initial 'time of need' yet allows some space to be bought between
	// megas; maybe
	// there are 2 groups - the player who works from state and megas that work from __weapon

	// ultimately, this is PSX only
	_player_stat prev;
	static uint32 list_pos = 0;
	static uint32 depth_pos = 0;

#define PLAYER_FILE player_generic_anim_async_table[player_stat_use][M->async_list_pos]
#define MEGA_FILE mega_generic_anim_async_table[M->weapon][M->async_list_pos]

	// someone is talking, don't async
	if (speech_info[CONV_ID].total_subscribers > 0)
		return;

	// has been turned off dont async

	if (async_off)
		return;

	if (cur_id == player.Fetch_player_id()) {
		// its the player

		// check the icons are loaded in

#define SPECIAL_PLAYER_FILE player_generic_anim_async_table[list_pos][depth_pos]
		// now service the other sets
		if (!init_asyncs) {
			// prime the players 4 async vox-images
			// this system will ensure the first 3 anims in each player state for each weapon set will be nearby
			init_asyncs = TRUE8;

			sync_set[0].___init(M->chr_name, M->anim_set, __NOT_ARMED);
			sync_set[1].___init(M->chr_name, M->anim_set, __GUN);
			sync_set[2].___init(M->chr_name, M->anim_set, __CROUCH_NOT_ARMED);
			sync_set[3].___init(M->chr_name, M->anim_set, __CROUCH_GUN);

			list_pos = 0;
			depth_pos = 0;

			cur_sync_set = 0; // start in set 0
			caching = FALSE8;

			Tdebug("special_async.txt", "reseting %s %s", M->chr_name, M->anim_set);
		}

		// check that current sync set is not the current set - which will be covered by other async code
		if (cur_sync_set == (uint32)M->Fetch_pose()) {
			cur_sync_set++;
			if (cur_sync_set == 4) {
				cur_sync_set = 0;
			}

			// make sure we're not caching (because the anim might not exist in this set...
			caching = 0;

			Tdebug("special_async.txt", "\nskipping to set %d", cur_sync_set);
		}

		if ((!caching) && (sync_set[cur_sync_set].IsAnimTable(SPECIAL_PLAYER_FILE))) { // if exists - build filename too
			caching = TRUE8;
			Tdebug("special_async.txt", "going to load list %d, depth %d", list_pos, depth_pos);
			//  has current pending loaded
		}

		if ((caching) && (rs_anims->Res_async_open(sync_set[cur_sync_set].get_info_name(SPECIAL_PLAYER_FILE), sync_set[cur_sync_set].info_name_hash[SPECIAL_PLAYER_FILE],
		                                           sync_set[cur_sync_set].base_path, sync_set[cur_sync_set].base_path_hash))) {
			if (rs_anims->Res_async_open(sync_set[cur_sync_set].get_anim_name(SPECIAL_PLAYER_FILE), sync_set[cur_sync_set].anim_name_hash[SPECIAL_PLAYER_FILE],
			                             sync_set[cur_sync_set].base_path, sync_set[cur_sync_set].base_path_hash)) {
				caching = FALSE8; // loaded it
			}
		}

		if (!caching) {
			// move the state on one too - we do all the states then go deeper on each... etc.
			do {
				// next set
				cur_sync_set++;

				// we've done all the ones here for all sets
				if (cur_sync_set == 4) {
					// reset set
					cur_sync_set = 0;

					// next anim
					list_pos++;

					// we've done all the anims in all the sets...
					if (list_pos == 12) {
						// reset...
						list_pos = 0;

						//  next depth level
						depth_pos++; // point to 1 deeper in this set
						if (depth_pos == 3)
							depth_pos = 0; // cant go too deep
					}
				}
			} while (SPECIAL_PLAYER_FILE == __NO_ANIM);

			Tdebug("special_async.txt", " moving to list %d, depth %d", list_pos, depth_pos);
		}

		// check for change of status
		if (player_stat_was != player.player_status) {
			prev = player_stat_use; // record old

			if ((player.player_status == LINKING) || (player.player_status == FAST_LINKING) || (player.player_status == EASY_LINKING) ||
			    (player.player_status == REVERSE_LINKING) || (player.player_status == FAST_REVERSE_LINKING) || (player.player_status == STILL_LINKING)) {
				player_stat_use = player.stat_after_link; // look ahead and start asycing from players next mode
			} else {
				player_stat_use = player.player_status; // use current real mode
			}

			if (prev != player_stat_use) { // dont reset when link finished and changes to real mode
				// player has changed status so we must reset the cache list and start loading new stuff
				M->async_list_pos = 0; // reset list position
				M->asyncing = 0;       // cancel 'wait' for file to load - change instantly to new list
			}
		}

		// record current status ready for check-for-change next time
		player_stat_was = player.player_status;

		// are we waiting for current file to load?
		if (M->asyncing) {
			// has current pending loaded
			if (rs_anims->Res_async_open(I->get_info_name(PLAYER_FILE), I->info_name_hash[PLAYER_FILE], I->base_path, I->base_path_hash)) {
				// Always async the players anims
				if (rs_anims->Res_async_open(I->get_anim_name(PLAYER_FILE), I->anim_name_hash[PLAYER_FILE], I->base_path, I->base_path_hash)) {
					M->async_list_pos++;  // next
					M->asyncing = FALSE8; // not currently loading
					return;               // try to even out the processor load as res-open itself is very slow
				} else {
					M->async_list_pos++;  // next
					M->asyncing = FALSE8; // not currently loading
					return;               // try to even out the processor load as res-open itself is very slow
				}
			} else
				return; // not loaded yet
		}

		// not waiting for file to load so start on the next
		if (PLAYER_FILE == __NO_ANIM) {
			M->asyncing = FALSE8; // not currently loading so dont check if loaded
			return;               // done the list for this state
		}

		// set file loading

		if (I->IsAnimTable(PLAYER_FILE)) {
			M->asyncing = TRUE8;
		} else {
			M->async_list_pos++;  // next
			M->asyncing = FALSE8; // not currently loading
		}

	} else { // normal    M E G A

		// Always async the player animations
		// but only async the mega animations if on the screen
		int on_screen = Object_visible_to_camera(cur_id);

		// first ensure we have being_shot,being_shot_dead and hit_from_behind in memory at all times

		int ani = 0;

		if (M->is_evil) {
			while (evil_mega_always_in_memory[ani] != __NO_ANIM) {
				Check_preload(evil_mega_always_in_memory[ani], on_screen, I);
				ani++;
			}
		} else {
			while (good_mega_always_in_memory[ani] != __NO_ANIM) {
				Check_preload(good_mega_always_in_memory[ani], on_screen, I);
				ani++;
			}
		}
		// index by M->weapon

		if (M->async_weapon != M->weapon) {
			// changed weapon/pose
			M->async_weapon = M->weapon;
			M->async_list_pos = 0; // reset list position
			M->asyncing = 0;       // cancel 'wait' for file to load - change instantly to new list
		}

		// are we waiting for current file to load?
		if (M->asyncing) {
			// has current pending loaded
			if (rs_anims->Res_async_open(I->get_info_name(MEGA_FILE), I->info_name_hash[MEGA_FILE], I->base_path, I->base_path_hash)) {
				// Async the mega animations if on the screen
				if (on_screen) {
					if (rs_anims->Res_async_open(I->get_anim_name(MEGA_FILE), I->anim_name_hash[MEGA_FILE], I->base_path, I->base_path_hash)) {
						M->async_list_pos++;  // next
						M->asyncing = FALSE8; // not currently loading
						return;               // try to even out the processor load as res-open itself is very slow
					}
				} else {
					M->async_list_pos++;  // next
					M->asyncing = FALSE8; // not currently loading
					return;               // try to even out the processor load as res-open itself is very slow
				}
			} else
				return; // not loaded yet
		}

		// not waiting for file to load so start on the next
		if (MEGA_FILE == __NO_ANIM) {
			M->asyncing = FALSE8; // not currently loading so dont check if loaded
			return;               // done the list for this state
		}

		// set file loading

		if (I->IsAnimTable(MEGA_FILE)) {
			M->asyncing = TRUE8;
		} else {
			M->async_list_pos++;  // next
			M->asyncing = FALSE8; // not currently loading
		}
	}
}

bool8 _game_session::Route_async_ready(bool8 run, bool8 turn) {
	// special pre-route code to ensure all required anims are in memory before we start routing

	// Always async the player animations
	// but only async the mega animations if on the screen

	int on_screen;

	// default not visable...
	on_screen = 0;

	// more complex version of visible code
	if (Object_visible_to_camera(cur_id)) {
		bool8 result = TRUE8;
		PXvector filmPosition;
		PXWorldToFilm(M->actor_xyz, set.GetCamera(), result, filmPosition);

		if (filmPosition.z < -g_actor_hither_plane) {
			VECTOR v;
			v.vx = (int32)M->actor_xyz.x;
			v.vy = (int32)M->actor_xyz.y;
			v.vz = (int32)M->actor_xyz.z;

			SVECTOR orient;
			orient.vx = 0;
			orient.vy = 0;
			orient.vz = 0;

			// finally if this is true then we are okay so on_screen is true
			if (QuickActorCull((psxCamera *)&(set.GetCamera()), &v, &orient) != 1)
				on_screen = 1;
		}
	}

	L->looping = 1;

	if ((run) && (!Check_preload(__RUN, on_screen, I)))
		return FALSE8;
	if ((run) && (!Check_preload(__STAND_TO_RUN, on_screen, I)))
		return FALSE8;
	if ((run) && (!Check_preload(__RUN_TO_STAND, on_screen, I)))
		return FALSE8;
	else if ((!run) && (!Check_preload(__WALK, on_screen, I)))
		return FALSE8;
	else if ((!run) && (!Check_preload(__WALK_TO_STAND, on_screen, I)))
		return FALSE8;
	else if ((!run) && (!Check_preload(__WALK_TO_OTHER_STAND_LEFT_LEG, on_screen, I)))
		return FALSE8;
	else if ((!run) && (!Check_preload(__STAND_TO_WALK, on_screen, I)))
		return FALSE8;
	else if ((turn) && (!Check_preload(__TURN_ON_THE_SPOT_CLOCKWISE, on_screen, I)))
		return FALSE8;

	// all in
	return TRUE8;
}

} // End of namespace ICB
