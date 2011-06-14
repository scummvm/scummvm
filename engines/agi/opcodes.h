/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGI_OPCODES_H
#define AGI_OPCODES_H

#include <string.h>

namespace Agi {

struct AgiInstruction {
	const char *name;
	const char *args;
	AgiCommand func;

	int argumentsLength() { return strlen(args); }
};

extern AgiInstruction logicNamesTest[];
extern AgiInstruction logicNamesCmd[];

void cmd_increment(AgiGame *state, uint8 *p);
void cmd_decrement(AgiGame *state, uint8 *p);
void cmd_assignn(AgiGame *state, uint8 *p);
void cmd_assignv(AgiGame *state, uint8 *p);
void cmd_addn(AgiGame *state, uint8 *p);
void cmd_addv(AgiGame *state, uint8 *p);
void cmd_subn(AgiGame *state, uint8 *p);
void cmd_subv(AgiGame *state, uint8 *p);	// 0x08
void cmd_lindirectv(AgiGame *state, uint8 *p);
void cmd_rindirect(AgiGame *state, uint8 *p);
void cmd_lindirectn(AgiGame *state, uint8 *p);
void cmd_set(AgiGame *state, uint8 *p);
void cmd_reset(AgiGame *state, uint8 *p);
void cmd_toggle(AgiGame *state, uint8 *p);
void cmd_set_v(AgiGame *state, uint8 *p);
void cmd_reset_v(AgiGame *state, uint8 *p);	// 0x10
void cmd_toggle_v(AgiGame *state, uint8 *p);
void cmd_new_room(AgiGame *state, uint8 *p);
void cmd_new_room_f(AgiGame *state, uint8 *p);
void cmd_load_logic(AgiGame *state, uint8 *p);
void cmd_load_logic_f(AgiGame *state, uint8 *p);
void cmd_call(AgiGame *state, uint8 *p);
void cmd_call_f(AgiGame *state, uint8 *p);
void cmd_load_pic(AgiGame *state, uint8 *p);	// 0x18
void cmd_draw_pic(AgiGame *state, uint8 *p);
void cmd_show_pic(AgiGame *state, uint8 *p);
void cmd_discard_pic(AgiGame *state, uint8 *p);
void cmd_overlay_pic(AgiGame *state, uint8 *p);
void cmd_show_pri_screen(AgiGame *state, uint8 *p);
void cmd_load_view(AgiGame *state, uint8 *p);
void cmd_load_view_f(AgiGame *state, uint8 *p);
void cmd_discard_view(AgiGame *state, uint8 *p);	// 0x20
void cmd_animate_obj(AgiGame *state, uint8 *p);
void cmd_unanimate_all(AgiGame *state, uint8 *p);
void cmd_draw(AgiGame *state, uint8 *p);
void cmd_erase(AgiGame *state, uint8 *p);
void cmd_position(AgiGame *state, uint8 *p);
void cmd_position_f(AgiGame *state, uint8 *p);
void cmd_get_posn(AgiGame *state, uint8 *p);
void cmd_reposition(AgiGame *state, uint8 *p);	// 0x28
void cmd_set_view(AgiGame *state, uint8 *p);
void cmd_set_view_f(AgiGame *state, uint8 *p);
void cmd_set_loop(AgiGame *state, uint8 *p);
void cmd_set_loop_f(AgiGame *state, uint8 *p);
void cmd_fix_loop(AgiGame *state, uint8 *p);
void cmd_release_loop(AgiGame *state, uint8 *p);
void cmd_set_cel(AgiGame *state, uint8 *p);
void cmd_set_cel_f(AgiGame *state, uint8 *p);	// 0x30
void cmd_last_cel(AgiGame *state, uint8 *p);
void cmd_current_cel(AgiGame *state, uint8 *p);
void cmd_current_loop(AgiGame *state, uint8 *p);
void cmd_current_view(AgiGame *state, uint8 *p);
void cmd_number_of_loops(AgiGame *state, uint8 *p);
void cmd_set_priority(AgiGame *state, uint8 *p);
void cmd_set_priority_f(AgiGame *state, uint8 *p);
void cmd_release_priority(AgiGame *state, uint8 *p);	// 0x38
void cmd_get_priority(AgiGame *state, uint8 *p);
void cmd_stop_update(AgiGame *state, uint8 *p);
void cmd_start_update(AgiGame *state, uint8 *p);
void cmd_force_update(AgiGame *state, uint8 *p);
void cmd_ignore_horizon(AgiGame *state, uint8 *p);
void cmd_observe_horizon(AgiGame *state, uint8 *p);
void cmd_set_horizon(AgiGame *state, uint8 *p);
void cmd_object_on_water(AgiGame *state, uint8 *p);	// 0x40
void cmd_object_on_land(AgiGame *state, uint8 *p);
void cmd_object_on_anything(AgiGame *state, uint8 *p);
void cmd_ignore_objs(AgiGame *state, uint8 *p);
void cmd_observe_objs(AgiGame *state, uint8 *p);
void cmd_distance(AgiGame *state, uint8 *p);
void cmd_stop_cycling(AgiGame *state, uint8 *p);
void cmd_start_cycling(AgiGame *state, uint8 *p);
void cmd_normal_cycle(AgiGame *state, uint8 *p);	// 0x48
void cmd_end_of_loop(AgiGame *state, uint8 *p);
void cmd_reverse_cycle(AgiGame *state, uint8 *p);
void cmd_reverse_loop(AgiGame *state, uint8 *p);
void cmd_cycle_time(AgiGame *state, uint8 *p);
void cmd_stop_motion(AgiGame *state, uint8 *p);
void cmd_start_motion(AgiGame *state, uint8 *p);
void cmd_step_size(AgiGame *state, uint8 *p);
void cmd_step_time(AgiGame *state, uint8 *p);	// 0x50
void cmd_move_obj(AgiGame *state, uint8 *p);
void cmd_move_obj_f(AgiGame *state, uint8 *p);
void cmd_follow_ego(AgiGame *state, uint8 *p);
void cmd_wander(AgiGame *state, uint8 *p);
void cmd_normal_motion(AgiGame *state, uint8 *p);
void cmd_set_dir(AgiGame *state, uint8 *p);
void cmd_get_dir(AgiGame *state, uint8 *p);
void cmd_ignore_blocks(AgiGame *state, uint8 *p);	// 0x58
void cmd_observe_blocks(AgiGame *state, uint8 *p);
void cmd_block(AgiGame *state, uint8 *p);
void cmd_unblock(AgiGame *state, uint8 *p);
void cmd_get(AgiGame *state, uint8 *p);
void cmd_get_f(AgiGame *state, uint8 *p);
void cmd_drop(AgiGame *state, uint8 *p);
void cmd_put(AgiGame *state, uint8 *p);
void cmd_put_f(AgiGame *state, uint8 *p);	// 0x60
void cmd_get_room_f(AgiGame *state, uint8 *p);
void cmd_load_sound(AgiGame *state, uint8 *p);
void cmd_sound(AgiGame *state, uint8 *p);
void cmd_stop_sound(AgiGame *state, uint8 *p);
void cmd_print(AgiGame *state, uint8 *p);
void cmd_print_f(AgiGame *state, uint8 *p);
void cmd_display(AgiGame *state, uint8 *p);
void cmd_display_f(AgiGame *state, uint8 *p);	// 0x68
void cmd_clear_lines(AgiGame *state, uint8 *p);
void cmd_text_screen(AgiGame *state, uint8 *p);
void cmd_graphics(AgiGame *state, uint8 *p);
void cmd_set_cursor_char(AgiGame *state, uint8 *p);
void cmd_set_text_attribute(AgiGame *state, uint8 *p);
void cmd_shake_screen(AgiGame *state, uint8 *p);
void cmd_configure_screen(AgiGame *state, uint8 *p);
void cmd_status_line_on(AgiGame *state, uint8 *p);	// 0x70
void cmd_status_line_off(AgiGame *state, uint8 *p);
void cmd_set_string(AgiGame *state, uint8 *p);
void cmd_get_string(AgiGame *state, uint8 *p);
void cmd_word_to_string(AgiGame *state, uint8 *p);
void cmd_parse(AgiGame *state, uint8 *p);
void cmd_get_num(AgiGame *state, uint8 *p);
void cmd_prevent_input(AgiGame *state, uint8 *p);
void cmd_accept_input(AgiGame *state, uint8 *p);	// 0x78
void cmd_set_key(AgiGame *state, uint8 *p);
void cmd_add_to_pic(AgiGame *state, uint8 *p);
void cmd_add_to_pic_f(AgiGame *state, uint8 *p);
void cmd_status(AgiGame *state, uint8 *p);
void cmd_save_game(AgiGame *state, uint8 *p);
void cmd_load_game(AgiGame *state, uint8 *p);
void cmd_init_disk(AgiGame *state, uint8 *p);
void cmd_restart_game(AgiGame *state, uint8 *p);	// 0x80
void cmd_show_obj(AgiGame *state, uint8 *p);
void cmd_random(AgiGame *state, uint8 *p);
void cmd_program_control(AgiGame *state, uint8 *p);
void cmd_player_control(AgiGame *state, uint8 *p);
void cmd_obj_status_f(AgiGame *state, uint8 *p);
void cmd_quit(AgiGame *state, uint8 *p);
void cmd_show_mem(AgiGame *state, uint8 *p);
void cmd_pause(AgiGame *state, uint8 *p);	// 0x88
void cmd_echo_line(AgiGame *state, uint8 *p);
void cmd_cancel_line(AgiGame *state, uint8 *p);
void cmd_init_joy(AgiGame *state, uint8 *p);
void cmd_toggle_monitor(AgiGame *state, uint8 *p);
void cmd_version(AgiGame *state, uint8 *p);
void cmd_script_size(AgiGame *state, uint8 *p);
void cmd_set_game_id(AgiGame *state, uint8 *p);
void cmd_log(AgiGame *state, uint8 *p);	// 0x90
void cmd_set_scan_start(AgiGame *state, uint8 *p);
void cmd_reset_scan_start(AgiGame *state, uint8 *p);
void cmd_reposition_to(AgiGame *state, uint8 *p);
void cmd_reposition_to_f(AgiGame *state, uint8 *p);
void cmd_trace_on(AgiGame *state, uint8 *p);
void cmd_trace_info(AgiGame *state, uint8 *p);
void cmd_print_at(AgiGame *state, uint8 *p);
void cmd_print_at_v(AgiGame *state, uint8 *p);	// 0x98
//void cmd_discard_view(AgiGame *state, uint8 *p);	// Opcode repeated from 0x20 ?
void cmd_clear_text_rect(AgiGame *state, uint8 *p);
void cmd_set_upper_left(AgiGame *state, uint8 *p);
void cmd_set_menu(AgiGame *state, uint8 *p);
void cmd_set_menu_item(AgiGame *state, uint8 *p);
void cmd_submit_menu(AgiGame *state, uint8 *p);
void cmd_enable_item(AgiGame *state, uint8 *p);
void cmd_disable_item(AgiGame *state, uint8 *p);	// 0xa0
void cmd_menu_input(AgiGame *state, uint8 *p);
void cmd_show_obj_v(AgiGame *state, uint8 *p);
void cmd_open_dialogue(AgiGame *state, uint8 *p);
void cmd_close_dialogue(AgiGame *state, uint8 *p);
void cmd_mul_n(AgiGame *state, uint8 *p);
void cmd_mul_v(AgiGame *state, uint8 *p);
void cmd_div_n(AgiGame *state, uint8 *p);
void cmd_div_v(AgiGame *state, uint8 *p);	// 0xa8
void cmd_close_window(AgiGame *state, uint8 *p);
void cmd_set_simple(AgiGame *state, uint8 *p);
void cmd_push_script(AgiGame *state, uint8 *p);
void cmd_pop_script(AgiGame *state, uint8 *p);
void cmd_hold_key(AgiGame *state, uint8 *p);
void cmd_set_pri_base(AgiGame *state, uint8 *p);
void cmd_discard_sound(AgiGame *state, uint8 *p);
void cmd_hide_mouse(AgiGame *state, uint8 *p);	// 0xb0
void cmd_allow_menu(AgiGame *state, uint8 *p);
void cmd_show_mouse(AgiGame *state, uint8 *p);
void cmd_fence_mouse(AgiGame *state, uint8 *p);
void cmd_mouse_posn(AgiGame *state, uint8 *p);
void cmd_release_key(AgiGame *state, uint8 *p);
void cmd_adj_ego_move_to_x_y(AgiGame *state, uint8 *p);

void cond_equal(AgiGame *state, uint8 *p);
void cond_equalv(AgiGame *state, uint8 *p);
void cond_less(AgiGame *state, uint8 *p);
void cond_lessv(AgiGame *state, uint8 *p);
void cond_greater(AgiGame *state, uint8 *p);
void cond_greaterv(AgiGame *state, uint8 *p);
void cond_isset(AgiGame *state, uint8 *p);
void cond_issetv(AgiGame *state, uint8 *p);
void cond_has(AgiGame *state, uint8 *p);
void cond_obj_in_room(AgiGame *state, uint8 *p);
void cond_posn(AgiGame *state, uint8 *p);
void cond_controller(AgiGame *state, uint8 *p);
void cond_have_key(AgiGame *state, uint8 *p);
void cond_said(AgiGame *state, uint8 *p);
void cond_compare_strings(AgiGame *state, uint8 *p);
void cond_obj_in_box(AgiGame *state, uint8 *p);
void cond_center_posn(AgiGame *state, uint8 *p);
void cond_right_posn(AgiGame *state, uint8 *p);
void cond_unknown_13(AgiGame *state, uint8 *p);
void cond_unknown(AgiGame *state, uint8 *p);

} // End of namespace Agi

#endif /* AGI_OPCODES_H */
