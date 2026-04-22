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

#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/rail.h"
#include "mads/madsv2/core/attr.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/himem.h"
#include "mads/madsv2/core/object.h"

namespace MADS {
namespace MADSV2 {

Player player = { 0 };
Player2 player2 = { 0 };

byte player_facing_to_series[10] = { 0, 7, 4, 3, 6, 0, 2, 5, 0, 1 };
byte player_clockwise[10] = { 9, 4, 1, 2, 7, 9, 3, 8, 9, 6 };
byte player_counter_clockwise[10] = { 7, 2, 3, 6, 1, 7, 9, 4, 7, 8 };

void Player::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(walking);
	s.syncAsSint16LE(x);
	s.syncAsSint16LE(y);
	s.syncAsSint16LE(target_x);
	s.syncAsSint16LE(target_y);
	s.syncAsSint16LE(sign_x);
	s.syncAsSint16LE(sign_y);
	s.syncAsSint16LE(x_count);
	s.syncAsSint16LE(y_count);
	s.syncAsSint16LE(x_counter);
	s.syncAsSint16LE(y_counter);
	s.syncAsSint16LE(target_facing);
	s.syncAsSint16LE(special_code);
	s.syncAsSint16LE(sprite_changed);
	s.syncAsSint16LE(frame_delay);
	s.syncAsSint16LE(center_of_gravity);
	s.syncAsSint16LE(walk_freedom);
	s.syncAsSint16LE(walk_anywhere);
	s.syncAsSint16LE(walk_off_edge_to_room);
	s.syncAsSint16LE(walk_off_edge);
	s.syncAsSint16LE(need_to_walk);
	s.syncAsSint16LE(ready_to_walk);
	s.syncAsSint16LE(prepare_walk_facing);
	s.syncAsSint16LE(prepare_walk_x);
	s.syncAsSint16LE(prepare_walk_y);
	s.syncAsSint16LE(commands_allowed);
	s.syncAsSint16LE(walker_visible);
	s.syncAsSint16LE(walker_previously_visible);
	s.syncAsSint16LE(series_base);
	s.syncAsSint16LE(available[8]);
	s.syncAsSint16LE(facing);
	s.syncAsSint16LE(turn_to_facing);
	s.syncAsSint16LE(series);
	s.syncAsSint16LE(mirror);
	s.syncAsSint16LE(sprite);
	s.syncAsByte(depth);
	s.syncAsByte(scale);
	s.syncAsSint16LE(stop_walker_sequence);
	for (int i = 0; i < PLAYER_MAX_STOP; ++i)
		s.syncAsSint16LE(stop_walker_stack[i]);
	for (int i = 0; i < PLAYER_MAX_STOP; ++i)
		s.syncAsSint16LE(stop_walker_trigger[i]);
	s.syncAsSint16LE(stop_walker_pointer);
	s.syncAsSint16LE(upcoming_trigger);
	s.syncAsSint16LE(trigger);
	s.syncAsSint16LE(next_special_code);
	s.syncAsSint16LE(scaling_velocity);
	s.syncAsSint16LE(pixel_accum);
	s.syncAsSint16LE(dist_accum);
	s.syncAsUint16LE(delta_distance);
	s.syncAsUint16LE(total_distance);
	s.syncAsSint16LE(velocity);
	s.syncAsSint16LE(high_sprite);
	s.syncAsSint16LE(command_ready);
	s.syncAsSint16LE(command_error);
	s.syncAsSint16LE(command_source);
	s.syncAsSint16LE(command);
	s.syncAsSint16LE(main_object);
	s.syncAsSint16LE(second_object);
	s.syncAsSint16LE(main_object_source);
	s.syncAsSint16LE(second_object_source);
	s.syncAsSint16LE(prep);
	s.syncAsSint16LE(look_around);
	s.syncAsSint16LE(main_syntax);
	s.syncAsSint16LE(second_syntax);
	s.syncBytes((byte *)series_name, 20);
	s.syncBytes((byte *)sentence, 64);
	s.syncAsSint32LE(clock);
	s.syncAsByte(been_here_before);
	s.skip(1);

	s.syncAsSint16LE( num_rooms_been_in);
	for (int i = 0; i < PLAYER_MAX_ROOMS; ++i)
		s.syncAsSint16LE(rooms_been_in[i]);

	s.syncAsSint16LE(num_series);
	s.syncAsByte(walker_loads_first);
	s.syncAsByte(walker_loaded_first);
	s.syncAsByte(walker_is_loaded);
	s.syncAsByte(walker_must_reload);
	s.syncAsSint16LE(walker_been_visible);
	s.syncAsByte(force_series);
	s.skip(1);
	s.syncAsSint16LE(walk_trigger);
	s.syncAsByte(walk_trigger_dest);
	s.skip(1);

	for (int i = 0; i < 3; ++i)
		s.syncAsSint16LE(walk_trigger_words[i]);

	s.syncAsSint16LE(enable_at_target);
}

void Player2::synchronize(Common::Serializer &s) {
	for (int i = 0; i < 3; ++i)
		s.syncAsSint16LE(words[i]);
}

/**
 * Returns the scaling factor for the player walker, based on the
 * player's current position
 */
static int player_scaling_factor(int y) {
	int bound, scale;

	bound = y - room->back_y;

	if (kernel_room_bound_dif != 0) {
		scale = room->back_scale + ((kernel_room_scale_dif * bound) / kernel_room_bound_dif);
	} else {
		scale = room->front_scale;
	}

	scale = MIN(100, scale);

	return (scale);
}

/**
 * Loads up the proper frame rate for the current walker series.
 */
static void player_set_base_frame_rate() {
	player.frame_delay = series_list[player.series_base + player.series]->walker->frame_rate;
	if (!player.frame_delay) player.frame_delay = 6;
}

void player_new_stop_walker() {
	int id;
	int abs_stop;
	WalkerInfoPtr walker;

	id = player.series_base + player.series;
	walker = series_list[id]->walker;

	if (!walker->num_secondary) {
		player.sprite = 1;
		goto done;
	}

	player.stop_walker_sequence = player.stop_walker_stack[player.stop_walker_pointer];

	if (!player.walker_visible) {
		player.upcoming_trigger = 0;
	} else {
		player.upcoming_trigger = player.stop_walker_trigger[player.stop_walker_pointer];
		if (player.stop_walker_pointer) player.stop_walker_pointer--;
	}

	abs_stop = ABS(player.stop_walker_sequence);
	if (player.stop_walker_sequence >= 0) {
		player.sprite = walker->sequence_start[abs_stop];
	} else {
		player.sprite = walker->sequence_stop[abs_stop];
	}

	if (abs_stop == 0) {
		player_set_base_frame_rate();
	} else {
		player.frame_delay = walker->sequence_chance[abs_stop];
	}

done:
	player.sprite_changed = true;
}

void player_select_series() {
	int id;

	player_clear_stop_walkers();

	player.mirror = 0;
	player.series = player_facing_to_series[player.facing];

	if (!player.available[player.series]) {
		player.series -= 4;
		player.mirror = MIRROR_MASK;
	}

	id = player.series_base + player.series;

	player.velocity = MAX<int>(series_list[id]->walker->velocity, 100);

	player_set_base_frame_rate();

	player.high_sprite = series_list[id]->walker->num_primary;
	if (player.high_sprite == 0) player.high_sprite = series_list[id]->num_sprites;

	player.center_of_gravity = series_list[id]->walker->center_of_gravity;

	if ((player.sprite <= 0) || (player.sprite > player.high_sprite)) {
		player.sprite = 1;
	}

	player.sprite_changed = true;
}

static void player_activate_trigger() {
	int count;

	player.commands_allowed |= player.enable_at_target;
	player.enable_at_target = false;

	if (player.walk_trigger) {
		kernel.trigger = player.walk_trigger;
		kernel.trigger_mode = player.walk_trigger_dest;
		if (kernel.trigger_mode != KERNEL_TRIGGER_DAEMON) {
			for (count = 0; count < 3; count++) {
				player2.words[count] = player.walk_trigger_words[count];
			}
		}
		player.walk_trigger = 0;
	}
}

/**
 * When the player needs to turn to face a different direction
 * than his current facing, this routine rotates him one place
 * in the right direction.
 */
static void player_keep_turning() {
	int clockwise_count = 0;
	int clockwise_sum = 0;
	int counter_clockwise_count = 0;
	int counter_clockwise_sum = 0;
	int temp_facing;
	int decision;

	player.sprite++;

	for (temp_facing = player.facing;
		temp_facing != player.turn_to_facing;
		temp_facing = player_clockwise[temp_facing]) {
		clockwise_count++;
		clockwise_sum += temp_facing;
		assert(clockwise_count < 100);
	}

	for (temp_facing = player.facing;
		temp_facing != player.turn_to_facing;
		temp_facing = player_counter_clockwise[temp_facing]) {
		counter_clockwise_count++;
		counter_clockwise_sum += temp_facing;
	}

	decision = (clockwise_count - counter_clockwise_count);
	if (!decision) decision = clockwise_sum - counter_clockwise_sum;

	if (decision < 0) {
		player.facing = player_clockwise[player.facing];
	} else {
		player.facing = player_counter_clockwise[player.facing];
	}

	player_select_series();

	if (player.facing == player.turn_to_facing) {
		if (!player.walking) {
			player_new_stop_walker();
			player_activate_trigger();
		}
	}

	player.clock++;  // Add 1 to player clock for slightly slower turn
}

void player_stationary_update() {
	int id;
	int abs_stop;
	int sgn_stop;
	WalkerInfoPtr walker;

	if (player.facing != player.turn_to_facing) {
		player_keep_turning();
		goto done;
	}

	id = player.series_base + player.series;
	walker = series_list[id]->walker;

	if (walker->num_secondary > 0) {
		abs_stop = ABS(player.stop_walker_sequence);
		sgn_stop = sgn(player.stop_walker_sequence);
		if (sgn_stop == 0) sgn_stop = 1;

		if (abs_stop < walker->num_secondary) {

			player.sprite += sgn_stop;
			player.sprite_changed = true;

			if (player.sprite > walker->sequence_stop[abs_stop]) {
				player.trigger = player.upcoming_trigger;
				player_new_stop_walker();
			}
			if (player.sprite < walker->sequence_start[abs_stop]) {
				player.trigger = player.upcoming_trigger;
				player_new_stop_walker();
			}
		} else {
			player.stop_walker_sequence = 0;
		}
	}

done:
	;
}

void player_set_facing() {
	int dx, dy, ds, sy;
	int mode;
	int s1, s2;
	int angle_factor;
	word ddx, ddy, dddx;
	word ssy;
	word pixels;

	dx = (player.target_x - player.x);
	dy = (player.target_y - player.y);

	s1 = player_scaling_factor(player.target_y);
	s2 = player_scaling_factor(player.y);

	s1 = MIN(s1, 100);
	s2 = MIN(s2, 100);

	player.sign_x = sgn(dx);
	player.sign_y = sgn(dy);

	dx = ABS(dx);
	dy = ABS(dy);
	ds = ABS(s2 - s1);

	ddx = ((word)dx) * 100;
	ddy = ((word)dy) * 100;

	dddx = ((word)dx) * 33;

	angle_factor = 100 + (player.scaling_velocity ? (ds * 3) : 0);

	sy = (dy * angle_factor) / 100;
	ssy = ((word)sy) * 100;

	if (dx == 0) {
		mode = PLAYER_VERTICAL;
	} else if (dy == 0) {
		mode = PLAYER_HORIZONTAL;
	} else if ((dx > sy) && ((dddx / (word)sy) >= PLAYER_DIAGONAL_THRESHOLD)) {
		mode = PLAYER_HORIZONTAL;
	} else {
		if ((dy > dx) && ((ssy / (word)dx) >= PLAYER_DIAGONAL_THRESHOLD)) {
			mode = PLAYER_VERTICAL;
		} else {
			mode = PLAYER_DIAGONAL;
		}
	}

	switch (mode) {
	case PLAYER_VERTICAL:
		player.turn_to_facing = (player.sign_y > 0) ? 2 : 8;
		break;
	case PLAYER_HORIZONTAL:
		player.turn_to_facing = (player.sign_x > 0) ? 6 : 4;
		break;
	case PLAYER_DIAGONAL:
		player.turn_to_facing = (player.sign_y > 0) ? 3 : 9;
		player.turn_to_facing -= (player.sign_x > 0) ? 0 : 2;
		break;
	}

	player.total_distance = imath_hypot(dx, dy);
	player.x_count = dx + 1;
	player.y_count = dy + 1;
	player.x_counter = dx;
	player.y_counter = dy;

	pixels = MAX(player.x_counter, player.y_counter);
	if (pixels > 0) {
		player.delta_distance = (player.total_distance * 100) / pixels;
	} else {
		player.delta_distance = 0;
	}

	if (player.x <= player.target_x) {
		player.pixel_accum = 0;
	} else {
		player.pixel_accum = MIN(dx, dy);
	}

	player.dist_accum = 0 - (int)player.delta_distance;
}

void player_set_final_facing() {
	if (player.target_facing != 5) {
		player.turn_to_facing = player.target_facing;
	}
}

void player_set_sprite() {
	if (player.walking) {
		player.sprite += 1;
		if (player.sprite > player.high_sprite) player.sprite = 1;
		player.sprite_changed = true;
	} else {
		if (!player.sprite_changed) player_stationary_update();
	}
}

void player_keep_walking() {
	int at_x, at_y;
	int walk_code;
	int id;
	int new_facing = false;
	int temp_velocity;
	int angle_scale;
	int angle_range;

	while (player.walking && !player.walk_off_edge && (player.x == player.target_x) && (player.y == player.target_y)) {
		if (rail_solution_stack_pointer == 0) {
			if (player.walk_off_edge_to_room) {
				player.walk_off_edge = player.walk_off_edge_to_room;
				player.walk_anywhere = true;
				player.walk_off_edge_to_room = 0;
				player.commands_allowed = false;
				new_facing = false;
			} else {
				player.walking = false;
				player_set_final_facing();
				new_facing = true;
			}
		} else {
			id = rail_solution_stack[--rail_solution_stack_pointer];
			player.target_x = room->rail[id].x;
			player.target_y = room->rail[id].y;
			new_facing = true;
		}
	}

	if (new_facing) {
		if (player.walking) player_set_facing();
	}

	if (player.facing != player.turn_to_facing) {
		player_keep_turning();
	} else {
		if (!player.walking) {
			player_new_stop_walker();
			player_activate_trigger();
		}
	}

	temp_velocity = player.velocity;

	if (player.scaling_velocity && (player.total_distance > 0)) {
		angle_range = 100 - player.scale;
		angle_scale = player.scale + ((angle_range * (player.x_count - 1)) / player.total_distance);
		temp_velocity = (int)(((long)temp_velocity * ((long)player.scale * (long)angle_scale)) / 10000L);
		temp_velocity = MAX(temp_velocity, 1);
	}

	if (player.walking && (player.facing == player.turn_to_facing)) {
		at_x = player.x;
		at_y = player.y;
		walk_code = false;
		player.special_code = 0;

		if (player.dist_accum < temp_velocity) {

			do {
				if (player.pixel_accum < player.x_count) {
					player.pixel_accum += player.y_count;
				}
				if (player.pixel_accum >= player.x_count) {
					if ((player.y_counter > 0) || player.walk_off_edge) at_y += player.sign_y;
					player.y_counter--;
					player.pixel_accum -= player.x_count;
				}
				if (player.pixel_accum < player.x_count) {
					if ((player.x_counter > 0) || player.walk_off_edge) at_x += player.sign_x;
					player.x_counter--;
				}

				if (!player.walk_anywhere && !(player.walk_off_edge || player.walk_off_edge_to_room)) {
					walk_code |= attr_walk(&scr_walk, at_x, at_y);
					if (!player.special_code) {
						player.special_code = attr_special(&scr_special, at_x, at_y);
					}
				}

				player.dist_accum += player.delta_distance;
			} while ((player.dist_accum < temp_velocity) && (!walk_code) &&
				((player.x_counter > 0) || (player.y_counter > 0) || (player.walk_off_edge)));

		}

		player.dist_accum -= temp_velocity;

		if (walk_code) {
			player_cancel_command();
		} else {
			if (!player.walk_off_edge) {
				if (player.x_counter <= 0) at_x = player.target_x;
				if (player.y_counter <= 0) at_y = player.target_y;
			}
			player.x = at_x;
			player.y = at_y;
		}
	}
}

int player_search_image() {
	int number = -1;
	int count;

	for (count = 0; (count < (int)image_marker) && (number < 0); count++) {
		if ((image_list[count].segment_id == KERNEL_SEGMENT_PLAYER) &&
			(image_list[count].flags >= 0)) {
			number = count;
		}
	}
	return number;
}

void player_set_image() {
	int count;
	int scale;
	int old_image_number;
	int xs, ys, x, y;
	int effective_y;
	byte depth;
	Image new_image;

	if (!player.sprite_changed && (player.walker_visible == player.walker_previously_visible)) goto done;

	old_image_number = player_search_image();
	if (old_image_number >= 0) {
		image_list[old_image_number].flags = IMAGE_ERASE;
	}

	depth = 1;
	effective_y = player.y;
	// effective_y = MIN (effective_y, display_y - 1);
	for (count = 1; count < 15; count++) {
		if (effective_y <= room->depth_table[depth]) {
			depth = (byte)(count + 1);
		}
	}

	scale = player_scaling_factor(player.y);

	player.depth = (byte)depth;
	player.scale = (byte)MIN(100, scale);

	if (player.walker_visible) {
		new_image.flags = IMAGE_UPDATE;
		new_image.segment_id = KERNEL_SEGMENT_PLAYER;
		new_image.series_id = (byte)(player.series_base + player.series);
		new_image.sprite_id = player.sprite | player.mirror;
		new_image.x = player.x;
		new_image.y = player.y + ((player.center_of_gravity * scale) / 100);
		new_image.depth = depth;
		new_image.scale = (byte)scale;

		if ((old_image_number >= 0) && memcmp(&new_image.segment_id, &image_list[old_image_number].segment_id,
			sizeof(Image) - sizeof(int)) == 0) {
			image_list[old_image_number].flags = 0;
		} else {
			image_list[image_marker] = new_image;
			image_marker++;
		}

		if (player.walk_off_edge) {
			xs = (series_list[new_image.series_id]->index[player.sprite - 1].xs * scale) / 200;
			ys = (series_list[new_image.series_id]->index[player.sprite - 1].ys * scale) / 100;
			x = new_image.x;
			y = new_image.y;

			if ((((x + xs) < 0) || ((x - xs) >= picture_map.total_x_size)) ||
				((y < 0) || ((y - ys) >= picture_map.total_y_size))) {
				if (room_id == player.walk_off_edge) {
					kernel.force_restart = true;
				}
				new_room = player.walk_off_edge;
				player.walk_off_edge = 0;
				player.walk_anywhere = player.walk_freedom;
			}
		}
	}

done:
	player.walker_previously_visible = player.walker_visible;
	player.walker_been_visible |= player.walker_visible;
	player.sprite_changed = false;
}

int player_load_series(const char *name) {
	int error_flag = true;
	int count;
	int handle;
	char temp_buf[80];
	char *mark;
	char load_buf[8] = { '8', '9', '6', '3', '2', '7', '4', '1' };

	if (name == NULL) {
		name = player.series_name;
	} else {
		Common::strcpy_s(player.series_name, name);
	}

	player.num_series = 0;

	if (!strlen(player.series_name)) {
		for (count = 0; count < 8; count++) {
			player.available[count] = false;
		}
		player.walker_visible = false;
		player.walker_is_loaded = false;
		player.walker_must_reload = false;
		goto done;
	}

	Common::strcpy_s(temp_buf, "*");
	Common::strcat_s(temp_buf, name);
	Common::strcat_s(temp_buf, "_0.SS");

	mark = strrchr(temp_buf, '_') + 1;

	for (count = 0; count < 8; count++) {
		if (count >= 5) kernel_ok_to_fail_load = true;
		*mark = load_buf[count];
		player.available[count] = true;
		handle = kernel_load_series(temp_buf, SPRITE_LOAD_WALKER_INFO);
		if (handle < 0) {
			if (count < 5) {
				goto done;
			} else {
				player.available[count] = false;
			}
		} else {
			player.num_series++;
		}
		if (count == 0) player.series_base = handle;
	}

	error_flag = false;
	player.walker_is_loaded = true;
	player.walker_must_reload = false;

done:
	kernel_ok_to_fail_load = false;
	return(error_flag);
}

void player_dump_walker() {
	int high;
	int count;

	if (player.walker_is_loaded && (player.num_series > 0)) {
		high = player.series_base + player.num_series - 1;
		for (count = high; count >= player.series_base; count--) {
			series_user[count] = 1;
			matte_deallocate_series(count, true);
		}
	}

	player.walker_is_loaded = false;
	player.walker_must_reload = true;
}

void player_preserve_palette() {
	int  count;
	dword flag_mask;
	dword flag;

	flag_mask = PAL_RESERVED;

	if (player.walker_is_loaded) {
		for (count = 0; count < player.num_series; count++) {
			flag = series_list[player.series_base + count]->color_handle;
			flag = (1L << flag);
			flag_mask |= flag;
		}
	}

	for (count = 0; count < 256; count++) {
		color_status[count] &= flag_mask;
	}

	palette_locked = false;

	for (count = 2; count < PAL_MAXFLAGS; count++) {
		flag = (1L << (dword)count);
		if (!(flag_mask & flag)) {
			flag_used[count] = false;
		} else {
			flag_used[count] = true;
		}
	}
}

void player_himem_preload(const char *name, int level) {
	char temp_buf[80];
	int count;

	for (count = 1; count <= 9; count++) {
		if (count != 5) {
			Common::strcpy_s(temp_buf, name);
			Common::strcat_s(temp_buf, "_0");
			temp_buf[strlen(temp_buf) - 1] += count;
			himem_preload_series(kernel_full_name(0, 0, -1, temp_buf, KERNEL_SS), level);
		}
	}
}

/**
 * Puts player on course to mouse click location, if any.
 */
static void player_walk_directly(int walk_form) {
	if (inter_point_established && ((walk_form == WALK_DIRECT_2) || (player.command < 0))) {
		player.need_to_walk = true;
		player.prepare_walk_x = inter_point_x;
		player.prepare_walk_y = inter_point_y;
	}
}

void player_new_command() {
	int count;
	int walk_spot;

	player_cancel_command();

	player.command_ready = true;
	player.command_error = false;

	player.command = inter_command;
	player.main_object = inter_main_object;
	player.second_object = inter_second_object;
	player.prep = inter_prep;
	player.command_source = inter_command_source;
	player.main_object_source = inter_main_object_source;
	player.second_object_source = inter_second_object_source;
	player.look_around = inter_look_around;

	player.main_syntax = inter_main_syntax;
	player.second_syntax = inter_second_syntax;

	for (count = 0; count < 3; count++) {
		player2.words[count] = inter_words[count];
	}

	Common::strcpy_s(player.sentence, inter_sentence);

	walk_spot = -1;
	player.need_to_walk = false;

	if (!player.look_around && (inter_input_mode != INTER_CONVERSATION)) {
		if (player.main_object_source == STROKE_INTERFACE) {
			walk_spot = player.main_object;
		} else if (player.second_object_source == STROKE_INTERFACE) {
			walk_spot = player.second_object;
		}
		if (walk_spot >= room_num_spots) {
			walk_spot = kernel_dynamic_consecutive(walk_spot - room_num_spots);
			if (walk_spot >= 0) {
				if ((kernel_dynamic_hot[walk_spot].feet_x == WALK_DIRECT) ||
					(kernel_dynamic_hot[walk_spot].feet_x == WALK_DIRECT_2)) {
					player_walk_directly(kernel_dynamic_hot[walk_spot].feet_x);
				} else if (kernel_dynamic_hot[walk_spot].feet_x >= 0) {
					player.need_to_walk = true;
					player.prepare_walk_x = kernel_dynamic_hot[walk_spot].feet_x;
					player.prepare_walk_y = kernel_dynamic_hot[walk_spot].feet_y;
				}
				player.prepare_walk_facing = kernel_dynamic_hot[walk_spot].facing;
			}
			walk_spot = -1;
		}
	}

	if (walk_spot >= 0) {
		if ((room_spots[walk_spot].feet_x == WALK_DIRECT) ||
			(room_spots[walk_spot].feet_x == WALK_DIRECT_2)) {
			player_walk_directly(room_spots[walk_spot].feet_x);
		} else if (room_spots[walk_spot].feet_x == WALK_NONE) {
			;
		} else if (room_spots[walk_spot].feet_x >= 0) {
			player.need_to_walk = true;
			player.prepare_walk_x = room_spots[walk_spot].feet_x;
			player.prepare_walk_y = room_spots[walk_spot].feet_y;
		}

		player.prepare_walk_facing = room_spots[walk_spot].facing;
	}

	player.ready_to_walk = player.need_to_walk;

	// Be sure player moves immediately even if in the middle of a long
	// stop-walker frame.
	player.clock = MIN(player.clock, kernel.clock + series_list[player.series_base + player.series]->walker->frame_rate);
}

void player_new_walk() {
	if (player.need_to_walk && player.ready_to_walk) {
		player_start_walking(player.prepare_walk_x, player.prepare_walk_y, player.prepare_walk_facing);
		player.need_to_walk = false;
	}
}

int player_parse(int vocab_item, ...) {
	va_list marker;
	int my_word = vocab_item;
	int result = true;
	int matched;
	int count;

	if (inter_input_mode == INTER_CONVERSATION) {
		result = false;
		goto done;
	}

	va_start(marker, vocab_item);
	while (my_word > 0) {
		matched = false;
		for (count = 0; count < 3; count++) {
			if (my_word == player2.words[count]) {
				matched = true;
			}
		}
		result &= matched;

		my_word = va_arg(marker, int);
	}

done:
	return result;
}

void player_cancel_walk() {
	player.target_x = player.x;
	player.target_y = player.y;

	player.target_facing = 5;
	player.turn_to_facing = player.facing;

	player.need_to_walk = false;
	player.ready_to_walk = false;
	player.walking = false;

	player.walk_anywhere = player.walk_freedom;
	player.walk_off_edge = 0;
	player.walk_off_edge_to_room = 0;

	player.next_special_code = 0;

	player.walk_trigger = 0;
	player.commands_allowed |= player.enable_at_target;
	player.enable_at_target = false;

	rail_solution_stack_pointer = 0;
}

void player_cancel_command() {
	player_cancel_walk();
	player.command_ready = false;
}

int player_has_been_in_room(int id) {
	int been_there = false;
	int count;

	for (count = 0; count < player.num_rooms_been_in; count++) {
		if (id == player.rooms_been_in[count]) {
			been_there = true;
		}
	}

	return been_there;
}

void player_discover_room(int id) {
	player.been_here_before = (byte)player_has_been_in_room(id);

	if (!player.been_here_before) {
		if (player.num_rooms_been_in >= PLAYER_MAX_ROOMS) {
			error_report(ERROR_BEEN_IN_TOO_MANY_ROOMS, WARNING, MODULE_PLAYER, PLAYER_MAX_ROOMS, id);
		} else {
			player.rooms_been_in[player.num_rooms_been_in++] = id;
		}
	}
}

int player_has(int object_id) {
	int has_flag = false;

	if (object_id < 0) goto done;

	if (object[object_id].location != PLAYER) goto done;

	has_flag = true;

done:
	return has_flag;
}

void player_clear_stop_walkers() {
	player.stop_walker_stack[0] = 0;
	player.stop_walker_trigger[0] = 0;
	player.stop_walker_pointer = 0;
	player.upcoming_trigger = 0;
	player.trigger = 0;
}

void player_init() {
	player.x = video_x >> 1;
	player.y = display_y >> 1;
	player.center_of_gravity = 0;
	player.walking = false;
	player.need_to_walk = false;
	player.sprite_changed = true;
	player.frame_delay = 3;
	player.commands_allowed = true;
	player.walker_visible = true;
	player.walk_freedom = false;

	// jacked up to 5 from 0 so inventory sprites can be loaded before walker
	player.series_base = 5;
	player.command_ready = false;
	player.num_rooms_been_in = 0;
	player.special_code = 0;
	player.next_special_code = 0;
	player.scaling_velocity = false;
	player.walker_is_loaded = false;
	player.walker_must_reload = true;
	player.walker_loads_first = true;

	player.walk_trigger = 0;
	player.enable_at_target = false;

	player_clear_stop_walkers();
}

void player_start_walking(int walk_x, int walk_y, int walk_facing) {
	int allow_one_illegal;
	int from_x, from_y;
	int unto_x, unto_y;
	int temp_stack_pointer;
#ifdef show_rails
	Buffer scr_live = { video_y, video_x, mcga_video };
#endif

	player_clear_stop_walkers();
	player_set_base_frame_rate();

	player.walking = true;
	player.target_facing = walk_facing;

	rail_add_node(rail_num_nodes - 2, player.x, player.y);
	rail_add_node(rail_num_nodes - 1, walk_x, walk_y);

	allow_one_illegal = attr_walk(&scr_walk, walk_x, walk_y);

	rail_check_path(allow_one_illegal);

#ifdef show_rails
	from_x = player.x;
	from_y = player.y;
	temp_stack_pointer = rail_solution_stack_pointer;
	while (temp_stack_pointer) {
		temp_stack_pointer--;
		unto_x = room->rail[rail_solution_stack[temp_stack_pointer]].x;
		unto_y = room->rail[rail_solution_stack[temp_stack_pointer]].y;
		buffer_line(scr_live, from_x, from_y, unto_x, unto_y, 3);
		from_x = unto_x;
		from_y = unto_y;
		if (kernel.frame_by_frame) keys_get();
	}
#endif

	from_x = player.x;
	from_y = player.y;
	player.next_special_code = 0;
	temp_stack_pointer = rail_solution_stack_pointer;
	while (temp_stack_pointer && !player.next_special_code) {
		temp_stack_pointer--;
		unto_x = room->rail[rail_solution_stack[temp_stack_pointer]].x;
		unto_y = room->rail[rail_solution_stack[temp_stack_pointer]].y;
		player.next_special_code = (buffer_legal(scr_special, room->xs,
			from_x, from_y,
			unto_x, unto_y) != LEGAL);
		from_x = unto_x;
		from_y = unto_y;
	}
}

int player_add_stop_walker(int walker, int trigger) {
	int error_flag = true;

	if (walker >= series_list[player.series_base + player.series]->walker->num_secondary) goto done;

	if (player.stop_walker_pointer < (PLAYER_MAX_STOP - 1)) {
		player.stop_walker_pointer++;
		player.stop_walker_stack[player.stop_walker_pointer] = walker;
		player.stop_walker_trigger[player.stop_walker_pointer] = trigger;
		error_flag = false;
	}

done:
	return error_flag;
}

void player_walk(int x, int y, int facing) {
	player_cancel_walk();
	player.need_to_walk = true;
	player.ready_to_walk = true;
	player.prepare_walk_x = x;
	player.prepare_walk_y = y;
	player.prepare_walk_facing = facing;
}

void player_first_walk(int from_x, int from_y, int from_facing,
	int to_x, int to_y, int to_facing,
	int enable_at_target) {
	player.x = from_x;
	player.y = from_y;
	player.facing = from_facing;

	player_walk(to_x, to_y, to_facing);

	player.walk_anywhere = true;

	player.commands_allowed = false;
	player.enable_at_target = enable_at_target;
}

void player_demand_facing(int facing) {
	player.facing = facing;
	player.turn_to_facing = facing;
	player_select_series();
}

void player_demand_location(int x, int y) {
	player_cancel_walk();
	player.x = player.target_x = x;
	player.y = player.target_y = y;
}

void player_walk_trigger(int trigger) {
	int count;

	player.walk_trigger = trigger;
	player.walk_trigger_dest = (byte)kernel.trigger_setup_mode;
	for (count = 0; count < 3; count++) {
		player.walk_trigger_words[count] = player2.words[count];
	}
}

} // namespace MADSV2
} // namespace MADS
