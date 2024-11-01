/* Copyright (C) 2023 Giovanni Cascione <ing.cascione@gmail.com>
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

#ifndef LIBRETRO_CORE_H
#define LIBRETRO_CORE_H

#include <libretro.h>

extern retro_log_printf_t retro_log_cb;
extern retro_input_state_t retro_input_cb;

bool retro_get_input_bitmask_supported(void);
void retro_osd_notification(const char* msg);
int retro_get_input_device(void);
const char * retro_get_core_dir(void);
const char * retro_get_system_dir(void);
const char * retro_get_save_dir(void);
const char * retro_get_playlist_dir(void);

bool retro_setting_get_timing_inaccuracies_enabled(void);
float retro_setting_get_frame_rate(void);
uint16 retro_setting_get_sample_rate(void);
uint16 retro_setting_get_audio_samples_buffer_size(void);
int retro_setting_get_analog_deadzone(void);
bool retro_setting_get_analog_response_is_quadratic(void);
float retro_setting_get_mouse_speed(void);
int retro_setting_get_mouse_fine_control_speed_reduction(void);
bool retro_setting_get_gamepad_cursor_only(void);
float retro_setting_get_gamepad_cursor_speed(void);
float retro_setting_get_gamepad_acceleration_time(void);
int retro_setting_get_gui_res_w(void);
int retro_setting_get_gui_res_h(void);

void retro_set_size(unsigned width, unsigned height);

uint8 retro_get_video_hw_mode(void);
#ifdef USE_OPENGL
uintptr_t retro_get_hw_fb(void);
void * retro_get_proc_address(const char * name);
#endif
#endif // LIBRETRO_CORE_H
