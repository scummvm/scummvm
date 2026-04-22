/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/rooms/section5.h"
#include "mads/madsv2/phantom/rooms/room505.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_505_init() {
	int id;

	kernel.disable_fastwalk = true;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->they_parted = false;
		local->leave_room = false;
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
		local->made_it_past_106 = false;
	}

	/* ==================== Load conversation ==================== */

	conv_get(CONV_COFFIN);

	kernel_flip_hotspot_loc(words_lid, false, COVER_AIR_X, COVER_AIR_Y);
	kernel_flip_hotspot(words_Christine, false);


	/* ===================== Load Sprite Series ================== */

	ss[fx_lid] = kernel_load_series(kernel_name('x', 6), false);
	ss[fx_pusher] = kernel_load_series(kernel_name('a', 1), false);
	ss[fx_skull_0] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_skull_1] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_skull_2] = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_skull_3] = kernel_load_series(kernel_name('x', 3), false);
	ss[fx_skull_4] = kernel_load_series(kernel_name('x', 4), false);
	ss[fx_skull_5] = kernel_load_series(kernel_name('x', 5), false);
	ss[fx_take_9] = kernel_load_series(kernel_name('a', 4), false);

	/* ========================= Previous Rooms ================== */


	if (previous_room == KERNEL_RESTORING_GAME) {

		if (conv_restore_running == CONV_COFFIN) {

			kernel_flip_hotspot(words_lid, false);
			kernel_flip_hotspot_loc(words_lid, true, COVER_AIR_X, COVER_AIR_Y);
			seq[fx_lid] = kernel_seq_stamp(ss[fx_lid], false, 12);
			kernel_seq_depth(seq[fx_lid], 1);

			aa[1] = kernel_run_animation(kernel_name('c', 1), ROOM_505_AFTER_RESCUE);
			kernel_reset_animation(aa[1], 109);
			local->anim_1_running = true;
			player.walker_visible = false;
			player.commands_allowed = false;
			local->both_action = BOTH_SHUT_UP;

			conv_run(CONV_COFFIN);
			conv_export_pointer(&global[player_score]);

		} else if (local->they_parted && (global[fight_status] == FIGHT_NOT_HAPPENED)) {
			kernel_flip_hotspot(words_lid, false);
			kernel_flip_hotspot_loc(words_lid, true, COVER_AIR_X, COVER_AIR_Y);
			seq[fx_lid] = kernel_seq_stamp(ss[fx_lid], false, 12);
			kernel_seq_depth(seq[fx_lid], 1);

			local->anim_2_running = true;
			local->both_action = BOTH_SHUT_UP;
			aa[2] = kernel_run_animation(kernel_name('b', 1), 0);
			id = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
				0, 0, 0, 0);
			kernel_dynamic_hot[id].prep = PREP_ON;
			kernel_dynamic_walk(id, DYNAMIC_CHR_WALK_TO_X, DYNAMIC_CHR_WALK_TO_Y, FACING_NORTHWEST);
			kernel_reset_animation(aa[2], 89);
			kernel_dynamic_anim(id, aa[2], 3);
			kernel_dynamic_anim(id, aa[2], 4);
			kernel_dynamic_anim(id, aa[2], 5);
			kernel_dynamic_anim(id, aa[2], 6);
			kernel_dynamic_anim(id, aa[2], 7);

		} else if (global[fight_status]) {
			kernel_flip_hotspot(words_lid, false);
			kernel_flip_hotspot_loc(words_lid, true, COVER_AIR_X, COVER_AIR_Y);
			seq[fx_lid] = kernel_seq_stamp(ss[fx_lid], false, 12);
			kernel_seq_depth(seq[fx_lid], 1);

		} else {
			aa[1] = kernel_run_animation(kernel_name('c', 1), ROOM_505_AFTER_RESCUE);
			local->anim_1_running = true;
			local->both_action = HIT_GLASS;
			kernel_flip_hotspot(words_Christine, true);
		}
	}

	if ((previous_room == 504) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x = PLAYER_X_FROM_504;
		player.y = PLAYER_Y_FROM_504;
		player.facing = FACING_EAST;
		player.commands_allowed = false;
		player_walk(WALK_TO_X_FROM_504, WALK_TO_Y_FROM_504, FACING_SOUTHEAST);
		if (global[coffin_status] != COFFIN_OPEN) {
			player_walk_trigger(ROOM_505_START_CONV);
			local->anim_1_running = true;
			local->both_action = HIT_GLASS;
			kernel_flip_hotspot(words_Christine, true);
			aa[1] = kernel_run_animation(kernel_name('c', 1), ROOM_505_AFTER_RESCUE);

		} else {
			kernel_flip_hotspot(words_lid, false);
			kernel_flip_hotspot_loc(words_lid, true, COVER_AIR_X, COVER_AIR_Y);
			seq[fx_lid] = kernel_seq_stamp(ss[fx_lid], false, 12);
			kernel_seq_depth(seq[fx_lid], 1);
			player.commands_allowed = true;
		}
	}

	section_5_music();
}

static void process_conversation_coffin() {
	int you_trig_flag = false;
	int me_trig_flag = false;

	switch (player_verb) {
	case conv020_where_a:
		me_trig_flag = true;
		you_trig_flag = true;
		if (!kernel.trigger) {
			conv_hold();
			player_walk(COFFIN_X, COFFIN_Y, FACING_SOUTHWEST);
			player_walk_trigger(ROOM_505_START_CONV + 1);
		}
		break;

	case conv020_exit_b_b:
		me_trig_flag = true;
		you_trig_flag = true;
		if (!kernel.trigger) {
			conv_hold();
			local->raoul_action = CONV20_RAOUL_END;
		}
		break;

	case conv020_bye_b_b:
		me_trig_flag = true;
		you_trig_flag = true;
		local->both_action = PART;
		break;

	case conv020_dialogue_b_b:
		me_trig_flag = true;
		you_trig_flag = true;
		if (!local->made_it_past_106) {
			conv_hold();
		}
		break;

	case conv020_story_b_b:
		me_trig_flag = true;
		you_trig_flag = true;
		if (!kernel.trigger) {
			conv_hold();
			local->both_action = GIVE_RING;
		}
		break;
	}

	switch (kernel.trigger) {
	case ROOM_505_START_CONV + 1:
		aa[0] = kernel_run_animation(kernel_name('r', 1), ROOM_505_END_CONV);
		local->anim_0_running = true;
		local->raoul_action = CONV20_RAOUL_TALK;
		local->raoul_talk_count = 0;
		player.walker_visible = false;
		break;

	case ROOM_505_END_CONV:
		player.walker_visible = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
		kernel_timing_trigger(10, ROOM_505_END_CONV + 1);
		break;

	case ROOM_505_END_CONV + 1:
		conv_release();
		break;

	case ROOM_505_START_CONV:
		conv_release();
		break;

	case ROOM_505_ME_TALK:
		if ((local->both_action != PART) &&
			(local->both_action != HIT_GLASS)) {
			local->both_action = HE_TALK;
		}
		break;

	case ROOM_505_YOU_TALK:
		if ((local->both_action != PART) &&
			(local->both_action != HIT_GLASS)) {
			local->both_action = SHE_TALK;
		}
		break;

	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_505_ME_TALK);
	} /* if me_trig_flag == true, then a me trigger is called from above, not here. */

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_505_YOU_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	local->both_talk_count = 0;
}

void room_505_pre_parser() {
	if (player_said_2(unlock, skull_face) ||
		player_said_2(unlock, sarcophagus) ||
		player_said_3(put, key, skull_face) ||
		player_said_2(unlock, lid)) {
		if (global[coffin_status] == COFFIN_CLOSED) {
			if (player_said_1(skull_face)) {
				player_walk(LOCK_X, LOCK_Y, FACING_SOUTHWEST);
			} else if (global[looked_at_skull_face]) {
				player_walk(LOCK_X, LOCK_Y, FACING_SOUTHWEST);
			}
		}
	}

	if (player_said_1(skull_face)) {
		if (player_said_1(look) || player_said_1(look_at)) {
			player_walk(LOCK_X, LOCK_Y, FACING_SOUTHWEST);
		}
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(curtain)) {
			player.need_to_walk = true;
		}
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(skull)) {
			player.need_to_walk = true;
		}
	}
}

void room_505_parser() {
	int id;
	int skull_seq;

	if (kernel.trigger == ROOM_505_HOLD_HANDS) {
		local->both_action = GET_OUT_B;
		goto handled;
	}

	if (conv_control.running == CONV_COFFIN) {
		process_conversation_coffin();
		goto handled;
	}

	if (player_said_2(talk_to, Christine)) {
		if (global[coffin_status] != COFFIN_OPEN) {
			text_show(text_505_36);
		} else {
			conv_run(CONV_COFFIN);
			conv_export_pointer(&global[player_score]);
			local->part_action = SHE_WAIT_TALK;
			local->part_talk_count = 0;
		}
		goto handled;
	}

	if ((player_said_2(unlock, sarcophagus) || player_said_2(unlock, lid)) && !global[looked_at_skull_face]) {
		text_show(text_505_39);
		goto handled;
	}

	if ((player_said_2(unlock, skull_face) || player_said_3(put, key, skull_face)) ||
		((player_said_2(unlock, sarcophagus) || player_said_2(unlock, lid)) && global[looked_at_skull_face])) {
		if (global[coffin_status] == COFFIN_CLOSED) {
			switch (kernel.trigger) {
			case (0):
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], false,
					5, 0, 0, 2);
				kernel_seq_range(seq[fx_take_9], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_player(seq[fx_take_9], true);
				kernel_seq_trigger(seq[fx_take_9],
					KERNEL_TRIGGER_SPRITE, 9, ROOM_505_UNLOCK);
				kernel_seq_trigger(seq[fx_take_9],
					KERNEL_TRIGGER_EXPIRE, 0, ROOM_505_UNLOCK + 1);
				goto handled;
				break;

			case ROOM_505_UNLOCK:
				sound_play(N_CoffinUnlocks);
				text_show(text_505_28);
				goto handled;
				break;

			case ROOM_505_UNLOCK + 1:
				player.walker_visible = true;
				player.commands_allowed = true;
				global[coffin_status] = COFFIN_UNLOCKED;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
				goto handled;
				break;
			}

		} else {
			text_show(text_505_34);
			goto handled;
		}
	}

	if (player_said_2(push, skull) && (inter_point_x >= 19)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			seq[fx_pusher] = kernel_seq_pingpong(ss[fx_pusher], false,
				5, 0, 0, 2);
			kernel_seq_range(seq[fx_pusher], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_player(seq[fx_pusher], true);
			kernel_seq_trigger(seq[fx_pusher],
				KERNEL_TRIGGER_SPRITE, 6, 1);
			kernel_seq_trigger(seq[fx_pusher],
				KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			sound_play(N_PushSkull);
			if (inter_point_x <= 44) {
				id = ss[fx_skull_3];
			} else if (inter_point_x <= 58) {
				id = ss[fx_skull_2];
			} else if (inter_point_x <= 71) {
				id = ss[fx_skull_1];
			} else if (inter_point_x <= 84) {
				id = ss[fx_skull_0];
				if (global[coffin_status] == COFFIN_UNLOCKED) {
					local->both_action = GET_OUT_A;
					kernel_flip_hotspot(words_lid, false);
					kernel_flip_hotspot(words_Christine, false);
					kernel_flip_hotspot_loc(words_lid, true, COVER_AIR_X, COVER_AIR_Y);
					kernel_load_variant(1);
				}
			} else if (inter_point_x <= 100) {
				id = ss[fx_skull_4];
			} else {
				id = ss[fx_skull_5];
			}

			skull_seq = kernel_seq_pingpong(id, false, 5, 0, 0, 2);
			kernel_seq_range(skull_seq, KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_depth(skull_seq, 1);
			break;

		case 2:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_pusher]);
			player.walker_visible = true;
			player.commands_allowed = true;
			if (local->both_action == GET_OUT_A) {
				player_walk(HOLD_HANDS_X, HOLD_HANDS_Y, FACING_EAST);
				player_walk_trigger(ROOM_505_HOLD_HANDS);
				player.commands_allowed = false;
			}
			break;
		}
		goto handled;
	}

	if (player_said_2(walk_through, door)) {
		if (local->anim_2_running) {
			local->leave_room = true;
			player.commands_allowed = false;
		} else {
			global[chris_left_505] = true;
			new_room = 504;
		}
		goto handled;
	}

	if (player.look_around) {
		text_show(text_505_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(floor)) {
			text_show(text_505_11);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_505_12);
			goto handled;
		}

		if (player_said_1(sarcophagus)) {
			if (global[coffin_status] <= COFFIN_UNLOCKED) {
				text_show(text_505_13);
			} else {
				text_show(text_505_14);
			}
			goto handled;
		}

		if (player_said_1(skull_face)) {
			global[looked_at_skull_face] = true;
			text_show(text_505_29);
			goto handled;
		}

		if (player_said_1(door)) {
			text_show(text_505_19);
			goto handled;
		}

		if (player_said_1(skull)) {
			if (inter_point_x < 19) {
				text_show(text_505_21);
			} else {
				text_show(text_505_20);
			}
			goto handled;
		}

		if (player_said_1(skulls)) {
			text_show(text_505_21);
			goto handled;
		}

		if (player_said_1(totem)) {
			text_show(text_505_22);
			goto handled;
		}

		if (player_said_1(desk)) {
			text_show(text_505_23);
			goto handled;
		}

		if (player_said_1(pole)) {
			text_show(text_505_24);
			goto handled;
		}

		if (player_said_1(curtain)) {
			text_show(text_505_25);
			goto handled;
		}

		if (player_said_1(Christine)) {
			if (global[coffin_status] == COFFIN_OPEN) {
				text_show(text_505_30);
			} else {
				text_show(text_505_37);
			}
			goto handled;
		}

		if (player_said_1(lid)) {
			if (global[coffin_status] == COFFIN_OPEN) {
				text_show(text_505_31);
			} else {
				text_show(text_505_32);
			}
			goto handled;
		}
	}

	if (player_said_2(lock, sarcophagus) ||
		player_said_2(lock, lid) ||
		player_said_2(lock, skull_face)) {
		text_show(text_505_35);
		goto handled;
	}

	if (player_said_2(open, sarcophagus) ||
		player_said_2(open, lid)) {
		if (global[coffin_status] == COFFIN_OPEN) {
			text_show(text_505_33);
		} else if (global[coffin_status] == COFFIN_UNLOCKED) {
			text_show(text_505_18);
		} else {
			text_show(text_505_15);
		}
		goto handled;
	}

	if (player_said_2(take, Christine)) {
		if (global[coffin_status] != COFFIN_OPEN) {
			text_show(text_505_38);
			goto handled;
		}
	}

	if (player_said_2(take, Christine)) {
		if (global[coffin_status] <= COFFIN_UNLOCKED) {
			text_show(text_505_38);
		} else {
			text_show(text_505_40);
		}
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

static void handle_animation_raoul() {
	int raoul_reset_frame;

	if (kernel_anim[aa[0]].frame != local->raoul_frame) {
		local->raoul_frame = kernel_anim[aa[0]].frame;
		raoul_reset_frame = -1;

		switch (local->raoul_frame) {
		case 3:
			conv_release();
			break;

		case 4:   /* end of freeze and talk 1 */
		case 5:   /* end of talk 2            */
		case 6:   /* end of talk 2            */
			if (local->raoul_action == CONV20_RAOUL_TALK) {
				raoul_reset_frame = imath_random(3, 5);
				++local->raoul_talk_count;
				if (local->raoul_talk_count > 20) {
					local->raoul_action = CONV20_RAOUL_SHUT_UP;
					raoul_reset_frame = 3;
				}
				break;
			}

			if (local->raoul_action == CONV20_RAOUL_SHUT_UP) {
				raoul_reset_frame = 3;
			}

			if (local->raoul_action == CONV20_RAOUL_END) {
				raoul_reset_frame = 6;
			}
			break;
		}

		if (raoul_reset_frame >= 0) {
			kernel_reset_animation(aa[0], raoul_reset_frame);
			local->raoul_frame = raoul_reset_frame;
		}
	}
}

static void handle_animation_both() {
	int both_reset_frame;

	if (kernel_anim[aa[1]].frame != local->both_frame) {
		local->both_frame = kernel_anim[aa[1]].frame;
		both_reset_frame = -1;

		switch (local->both_frame) {
		case 41:
			sound_play(N_AngelMus505);
			break;

		case 51:  /* lid is up in air */
			seq[fx_lid] = kernel_seq_stamp(ss[fx_lid], false, 12);
			kernel_seq_depth(seq[fx_lid], 1);
			global[coffin_status] = COFFIN_OPEN;
			break;

		case 66:
			player.commands_allowed = false;
			conv_run(CONV_COFFIN);
			conv_export_pointer(&global[player_score]);
			break;

		case 67:
			if (local->both_action == GET_OUT_A) {
				both_reset_frame = 66;
			}
			break;

		case 68:  /* make walker vanish */
			player.walker_visible = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
			break;

		case 32: /* almost end of hit glass once */
			if (imath_random(1, 2) == 1) {
				both_reset_frame = 28;
			}
			break;

		case 14: /* almost end of hit glass twice */
			if (imath_random(1, 3) == 1) {
				both_reset_frame = 8;
			}
			break;

		case 1:  /* end of freeze */
		case 20: /* end of hit glass 1 */
		case 39: /* end of hot glass 2 */

			if (local->both_action == HIT_GLASS) {
				if (imath_random(1, 35) == 1) {
					if (imath_random(1, 2) == 1) {
						both_reset_frame = 1;  /* bang on glass 1 */
					} else {
						both_reset_frame = 20; /* bang on glass 2 */
					}

				} else {
					both_reset_frame = 0;
				}

			} else if (local->both_action == GET_OUT_A) {
				both_reset_frame = 39;

			} else {
				both_reset_frame = 0;
			}


			break;

		case 106:
			local->made_it_past_106 = true;
			conv_release();
			break;

		case 109:
		case 130:
			conv_release();
			break;

		case 127:
			sound_play(N_TakeObjectSnd);
			inter_give_to_player(wedding_ring);
			object_examine(wedding_ring, text_008_21, 0);
			/* show player the ring */
			break;

		case 110:  /* end of he talk 1  */
		case 111:  /* end of he talk 2  */
		case 112:  /* end of he talk 3  */
		case 113:  /* end of she talk 1 */
		case 114:  /* end of she talk 2 */
		case 115:  /* end of she talk 3 */
		case 131:  /* end of give ring  */

			switch (local->both_action) {
			case HE_TALK:
				both_reset_frame = imath_random(109, 111);
				++local->both_talk_count;
				if (local->both_talk_count > 20) {
					local->both_action = BOTH_SHUT_UP;
					both_reset_frame = 109;
				}  /* He will talk */
				break;

			case SHE_TALK:
				both_reset_frame = imath_random(112, 114);
				++local->both_talk_count;
				if (local->both_talk_count > 20) {
					local->both_action = BOTH_SHUT_UP;
					both_reset_frame = 109;
				}  /* She will talk */
				break;

			case GIVE_RING:
				both_reset_frame = 115;  /* She will give ring to Him */
				local->both_action = BOTH_SHUT_UP;
				break;

			case PART:
				both_reset_frame = 131;
				local->both_action = SHE_WAIT;
				player.commands_allowed = false;
				break;  /* She will go over to door, he will walk a couple of feet */

			default:
				both_reset_frame = 109;
				break;  /* freeze them holding hands */
			}
			break;
		}

		if (both_reset_frame >= 0) {
			kernel_reset_animation(aa[1], both_reset_frame);
			local->both_frame = both_reset_frame;
		}
	}
}

static void handle_animation_part() {
	int part_reset_frame;

	if (kernel_anim[aa[2]].frame != local->part_frame) {
		local->part_frame = kernel_anim[aa[2]].frame;
		part_reset_frame = -1;

		switch (local->part_frame) {
		case 20:
			sound_play(N_BackgroundMus);
			break;

		case 90:
			if (local->part_action == SHE_WAIT_TALK) {
				part_reset_frame = 146;
			} else if (!local->leave_room) {
				part_reset_frame = 89;
			}
			break;

		case 25:
			player.x = RAOUL_END_X;
			player.y = RAOUL_END_Y;
			player_demand_facing(FACING_WEST);
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[2]);
			break;

		case 70:
			player.commands_allowed = true;
			break;

		case 145:
			new_room = 504;
			break;

		case 147:
		case 148:
		case 149:
			part_reset_frame = imath_random(146, 148);
			++local->part_talk_count;
			if (local->part_talk_count > 10) {
				part_reset_frame = 89;
				local->part_action = SHE_WAIT;
			}
			break;
		}

		if (part_reset_frame >= 0) {
			kernel_reset_animation(aa[2], part_reset_frame);
			local->part_frame = part_reset_frame;
		}
	}
}

void room_505_daemon() {
	int id;

	if (local->anim_0_running) {
		handle_animation_raoul();
	}

	if (local->anim_1_running) {
		handle_animation_both();
	}

	if (local->anim_2_running) {
		handle_animation_part();
	}

	if (kernel.trigger == ROOM_505_AFTER_RESCUE) {
		kernel_abort_animation(aa[1]);
		sound_play(N_AllFade);
		local->they_parted = true;
		local->anim_2_running = true;
		local->anim_1_running = false;
		/* player.commands_allowed = false; */
		aa[2] = kernel_run_animation(kernel_name('b', 1), 0);
		id = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
			0, 0, 0, 0);
		kernel_dynamic_walk(id, DYNAMIC_CHR_WALK_TO_X, DYNAMIC_CHR_WALK_TO_Y, FACING_NORTHWEST);
		kernel_dynamic_hot[id].prep = PREP_ON;

		kernel_dynamic_anim(id, aa[2], 3);
		kernel_dynamic_anim(id, aa[2], 4);
		kernel_dynamic_anim(id, aa[2], 5);
		kernel_dynamic_anim(id, aa[2], 6);
		kernel_dynamic_anim(id, aa[2], 7);
	}

	if (kernel.trigger == ROOM_505_START_CONV) {
		player.commands_allowed = true;
		if (!player.been_here_before) {
			conv_run(CONV_COFFIN);
			conv_export_pointer(&global[player_score]);
		}
	}
}

void room_505_preload() {
	room_init_code_pointer = room_505_init;
	room_pre_parser_code_pointer = room_505_pre_parser;
	room_parser_code_pointer = room_505_parser;
	room_daemon_code_pointer = room_505_daemon;

	section_5_walker();
	section_5_interface();

	if ((global[coffin_status] == COFFIN_OPEN) && (!global[chris_left_505])) {
		kernel_initial_variant = 1;
	}

	vocab_make_active(words_Christine);
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
