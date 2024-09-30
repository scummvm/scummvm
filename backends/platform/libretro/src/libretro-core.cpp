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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "audio/mixer_intern.h"
#include "base/main.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/fs.h"
#include "common/error.h"
#include "streams/file_stream.h"
#include "graphics/surface.h"
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#ifndef _MSC_VER
/**
 * Include libgen.h for basename() and dirname().
 * @see http://linux.die.net/man/3/basename
 */
#include <libgen.h>
#endif

/**
 * Include base/internal_version.h to allow access to SCUMMVM_VERSION.
 * @see retro_get_system_info()
 */
#define INCLUDED_FROM_BASE_VERSION_CPP
#include "base/internal_version.h"

#include "graphics/managed_surface.h"

#include "backends/platform/libretro/include/libretro-defs.h"
#include "backends/platform/libretro/include/libretro-core.h"
#include "backends/platform/libretro/include/libretro-threads.h"
#include "backends/platform/libretro/include/libretro-core-options.h"
#include "backends/platform/libretro/include/libretro-os.h"
#include "backends/platform/libretro/include/libretro-mapper.h"

static struct retro_game_info game_buf;
static struct retro_game_info *game_buf_ptr;

retro_log_printf_t retro_log_cb = NULL;
retro_input_state_t retro_input_cb = NULL;
static retro_video_refresh_t video_cb = NULL;
static retro_audio_sample_batch_t audio_batch_cb = NULL;
static retro_environment_t environ_cb = NULL;
static retro_input_poll_t poll_cb = NULL;
static int retro_input_device = RETRO_DEVICE_JOYPAD;

// Default deadzone: 15%
static int analog_deadzone = (int)(0.15f * ANALOG_RANGE);

static float gamepad_cursor_speed = 1.0f;
static bool analog_response_is_quadratic = false;
static bool gamepad_cursor_only = false;

static float mouse_speed = 1.0f;
static float gamepad_acceleration_time = 0.2f;
static int mouse_fine_control_speed_reduction = 4;

static bool timing_inaccuracies_enabled = false;

char cmd_params[20][200];
char cmd_params_num;

static uint8 video_hw_mode = 0;

static unsigned base_width = RES_W_OVERLAY;
static unsigned base_height = RES_H_OVERLAY;
static unsigned max_width = RES_W_OVERLAY;
static unsigned max_height = RES_H_OVERLAY;

static uint32 current_frame = 0;
static uint8 frameskip_no;
static uint8 frameskip_type;
static uint8 frameskip_threshold;
static uint32 frameskip_counter = 0;

static uint8 audio_status = AUDIO_STATUS_MUTE;

static unsigned retro_audio_buff_occupancy = 0;
static uint8 retro_audio_buff_underrun_threshold = 25;

static float frame_rate = 0;
static uint16 sample_rate = 0;
static float audio_samples_per_frame   = 0.0f; // length in samples per frame
static float audio_samples_accumulator = 0.0f;

static int16 *audio_sample_buffer = NULL; // pointer to output buffer

static bool input_bitmask_supported = false;
static bool updating_variables = false;
static int opt_frameskip_threshold_display = 0;
static int opt_frameskip_no_display = 0;

#ifdef USE_OPENGL
static struct retro_hw_render_callback hw_render;

static void context_reset(void) {
	retro_log_cb(RETRO_LOG_DEBUG, "HW context reset\n");
	if (retro_emu_thread_started())
		LIBRETRO_G_SYSTEM->resetGraphicsContext();
}

static void context_destroy(void) {
	retro_log_cb(RETRO_LOG_DEBUG, "HW context destroy\n");
}

uintptr_t retro_get_hw_fb(void) {
	return hw_render.get_current_framebuffer();
}

void *retro_get_proc_address(const char *name) {
	return (void *)(hw_render.get_proc_address(name));
}
#endif

static void setup_hw_rendering(void) {

	enum retro_pixel_format pixel_fmt;
#ifdef USE_OPENGL
	if (video_hw_mode & VIDEO_GRAPHIC_MODE_REQUEST_HW) {
		pixel_fmt = RETRO_PIXEL_FORMAT_XRGB8888;
		if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixel_fmt) && retro_log_cb)
			retro_log_cb(RETRO_LOG_WARN, "RETRO_PIXEL_FORMAT_XRGB8888 not supported.\n");
		hw_render.context_reset = context_reset;
		hw_render.context_destroy = context_destroy;
		hw_render.cache_context = false;
		hw_render.bottom_left_origin = true;
#if defined(HAVE_OPENGL)
		hw_render.context_type = RETRO_HW_CONTEXT_OPENGL;
		video_hw_mode |= VIDEO_GRAPHIC_MODE_HAVE_OPENGL;
#elif defined(HAVE_OPENGLES2)
		hw_render.context_type = RETRO_HW_CONTEXT_OPENGLES2;
		video_hw_mode |= VIDEO_GRAPHIC_MODE_HAVE_OPENGLES2;
#endif
		if (!environ_cb(RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render)) {
			retro_log_cb(RETRO_LOG_WARN, "Failed to set up hardware rendering, falling back to software.\n");
			retro_osd_notification("Failed to set up HW rendering.");
			video_hw_mode = VIDEO_GRAPHIC_MODE_REQUEST_SW;
		}
	}
#endif
	if ((video_hw_mode & VIDEO_GRAPHIC_MODE_REQUEST_SW) || !video_hw_mode) {
		pixel_fmt = RETRO_PIXEL_FORMAT_RGB565;
		if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixel_fmt) && retro_log_cb)
			retro_log_cb(RETRO_LOG_WARN, "RETRO_PIXEL_FORMAT_RGB565 not supported.\n");
	}
}

uint8 retro_get_video_hw_mode(void) {
	return video_hw_mode;
}

void process_key_event_wrapper(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers) {
	LIBRETRO_G_SYSTEM->processKeyEvent(down, keycode, character, key_modifiers);
}

static void log_scummvm_exit_code(void) {
	if (retro_get_scummvm_res() == Common::kNoError)
		retro_log_cb(RETRO_LOG_INFO, "ScummVM exited successfully.\n");
	else if (retro_get_scummvm_res() < Common::kNoError)
		retro_log_cb(RETRO_LOG_WARN, "Unknown ScummVM exit code.\n");
	else
		retro_log_cb(RETRO_LOG_ERROR, "ScummVM exited with error %d.\n", retro_get_scummvm_res());
}

static void audio_buffer_init(uint16 sample_rate, uint16 frame_rate) {
	audio_samples_accumulator = 0.0f;
	audio_samples_per_frame   = (float)sample_rate / (float)frame_rate;
	uint32 audio_sample_buffer_size  = ((uint32)retro_setting_get_audio_samples_buffer_size()) * 2 * sizeof(int16);
	audio_sample_buffer       = audio_sample_buffer ? (int16 *)realloc(audio_sample_buffer, audio_sample_buffer_size) : (int16 *)malloc(audio_sample_buffer_size);

	if (audio_sample_buffer)
		memset(audio_sample_buffer, 0, audio_sample_buffer_size);
	else
		retro_log_cb(RETRO_LOG_ERROR, "audio_buffer_init error.\n");
}

static void audio_run(void) {
	int16 *audio_buffer_ptr;
	uint32 samples_to_read;
	uint32 samples_produced;

	/* Audio_samples_per_frame is decimal;
	 * get integer component */
	samples_to_read = (uint32)audio_samples_per_frame;

	/* Account for fractional component */
	audio_samples_accumulator += audio_samples_per_frame - (float)samples_to_read;

	if (audio_samples_accumulator >= 1.0f) {
		samples_to_read++;
		audio_samples_accumulator -= 1.0f;
	}

	samples_produced = ((Audio::MixerImpl *)g_system->getMixer())->mixCallback((byte *) audio_sample_buffer, samples_to_read * 2 * sizeof(int16));

	/* Workaround as currently there's no way to detect silence-only buffers from the mixer */
	if (samples_produced) {
		int i = 0;
		for (; i < samples_produced; i += 2)
			/* SID streams constant crap */
			if (READ_UINT16(audio_sample_buffer + i) > 32)
				break;
		samples_produced = i >= samples_produced ? 0 : samples_produced;
	}

	if (samples_produced)
		audio_status &= ~AUDIO_STATUS_MUTE;
	else {
		audio_status |= AUDIO_STATUS_MUTE;
		return;
	}

	/* Workaround for a RetroArch audio driver
	 * limitation: a maximum of 1024 frames
	 * can be written per call of audio_batch_cb(),
	 * so we have to send samples in chunks */
	audio_buffer_ptr = audio_sample_buffer;
	while (samples_produced > 0) {
		uint32 samples_to_write = (samples_produced > AUDIO_BATCH_FRAMES_MAX) ? AUDIO_BATCH_FRAMES_MAX : samples_produced;

		audio_batch_cb(audio_buffer_ptr, samples_to_write);

		samples_produced -= samples_to_write;
		audio_buffer_ptr += samples_to_write << 1;
	}
}

static void retro_audio_buff_status_cb(bool active, unsigned occupancy, bool underrun_likely) {
	if (active)
		audio_status |= AUDIO_STATUS_BUFFER_ACTIVE;
	else
		audio_status &= ~AUDIO_STATUS_BUFFER_ACTIVE;

	if (occupancy < retro_audio_buff_underrun_threshold)
		audio_status |= AUDIO_STATUS_BUFFER_UNDERRUN;
	else if (occupancy > (retro_audio_buff_underrun_threshold << 2))
		audio_status &= ~AUDIO_STATUS_BUFFER_UNDERRUN;

	retro_audio_buff_occupancy = occupancy;
}

void retro_osd_notification(const char *msg) {
	if (!msg || *msg == '\0')
		return;
	struct retro_message_ext retro_msg;
	retro_msg.type = RETRO_MESSAGE_TYPE_NOTIFICATION;
	retro_msg.target = RETRO_MESSAGE_TARGET_OSD;
	retro_msg.duration = 3000;
	retro_msg.msg = msg;
	environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE_EXT, &retro_msg);
}

static void update_variables(void) {
	struct retro_variable var;
	updating_variables = true;

	var.key = "scummvm_gamepad_cursor_only";
	var.value = NULL;
	gamepad_cursor_only = false;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		if (strcmp(var.value, "enabled") == 0)
			gamepad_cursor_only = true;
	}

	var.key = "scummvm_gamepad_cursor_speed";
	var.value = NULL;
	gamepad_cursor_speed = 1.0f;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		gamepad_cursor_speed = (float)atof(var.value);
	}

	var.key = "scummvm_gamepad_cursor_acceleration_time";
	var.value = NULL;
	gamepad_acceleration_time = 0.2f;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		gamepad_acceleration_time = (float)atof(var.value);
	}

	var.key = "scummvm_analog_response";
	var.value = NULL;
	analog_response_is_quadratic = false;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		if (strcmp(var.value, "quadratic") == 0)
			analog_response_is_quadratic = true;
	}

	var.key = "scummvm_analog_deadzone";
	var.value = NULL;
	analog_deadzone = (int)(0.15f * ANALOG_RANGE);
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		analog_deadzone = (int)(atoi(var.value) * 0.01f * ANALOG_RANGE);
	}

	var.key = "scummvm_mouse_speed";
	var.value = NULL;
	mouse_speed = 1.0f;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mouse_speed = (float)atof(var.value);
	}

	var.key = "scummvm_mouse_fine_control_speed_reduction";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mouse_fine_control_speed_reduction = (int)atoi(var.value);
	}

	var.key = "scummvm_allow_timing_inaccuracies";
	var.value = NULL;
	timing_inaccuracies_enabled = false;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		if (strcmp(var.value, "enabled") == 0)
			timing_inaccuracies_enabled = true;
	}

	var.key = "scummvm_framerate";
	var.value = NULL;
	float old_frame_rate = frame_rate;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		if (strcmp(var.value, "disabled") == 0)
			frame_rate = environ_cb(RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE, &frame_rate) ? frame_rate : DEFAULT_REFRESH_RATE;
		else {
			char frame_rate_var[3] = {0};
			strncpy(frame_rate_var, var.value, 2);
			frame_rate = (float)atof(frame_rate_var);
		}
	} else
		frame_rate = DEFAULT_REFRESH_RATE;

	var.key = "scummvm_samplerate";
	var.value = NULL;
	uint16 old_sample_rate = sample_rate;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		char sample_rate_var[6] = {0};
		strncpy(sample_rate_var, var.value, 5);
		sample_rate = atoi(sample_rate_var);
	} else
		sample_rate = DEFAULT_SAMPLE_RATE;

	var.key = "scummvm_frameskip_threshold";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		frameskip_threshold = (uint8)strtol(var.value, NULL, 10);
	}

	var.key = "scummvm_frameskip_no";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		frameskip_no = (uint8)strtol(var.value, NULL, 10) + 1;
	}

	var.key = "scummvm_frameskip_type";
	var.value = NULL;
	uint8 old_frameskip_type = frameskip_type;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		opt_frameskip_threshold_display = 0;
		opt_frameskip_no_display = 0;

		if (strcmp(var.value, "disabled") == 0)
			frameskip_type = 0;
		else if (strcmp(var.value, "fixed") == 0) {
			frameskip_type = 1;
			opt_frameskip_no_display = 1;
		} else if (strcmp(var.value, "auto") == 0)
			frameskip_type = 2;
		else if (strcmp(var.value, "manual") == 0) {
			frameskip_type = 3;
			opt_frameskip_threshold_display = 1;
		}
	}

	var.key = "scummvm_mapper_up";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_UP, var.value);
	}

	var.key = "scummvm_mapper_down";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_DOWN, var.value);
	}

	var.key = "scummvm_mapper_left";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_LEFT, var.value);
	}

	var.key = "scummvm_mapper_right";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_RIGHT, var.value);
	}

	var.key = "scummvm_mapper_a";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_A, var.value);
	}

	var.key = "scummvm_mapper_b";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_B, var.value);
	}

	var.key = "scummvm_mapper_x";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_X, var.value);
	}

	var.key = "scummvm_mapper_y";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_Y, var.value);
	}

	var.key = "scummvm_mapper_select";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_SELECT, var.value);
	}

	var.key = "scummvm_mapper_start";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_START, var.value);
	}

	var.key = "scummvm_mapper_l";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_L, var.value);
	}

	var.key = "scummvm_mapper_r";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_R, var.value);
	}

	var.key = "scummvm_mapper_l2";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_L2, var.value);
	}

	var.key = "scummvm_mapper_r2";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_R2, var.value);
	}

	var.key = "scummvm_mapper_l3";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_L3, var.value);
	}

	var.key = "scummvm_mapper_r3";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_R3, var.value);
	}

	var.key = "scummvm_mapper_lu";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_LU, var.value);
	}

	var.key = "scummvm_mapper_ld";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_LD, var.value);
	}

	var.key = "scummvm_mapper_ll";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_LL, var.value);
	}

	var.key = "scummvm_mapper_lr";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_LR, var.value);
	}

	var.key = "scummvm_mapper_ru";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_RU, var.value);
	}

	var.key = "scummvm_mapper_rd";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_RD, var.value);
	}

	var.key = "scummvm_mapper_rl";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_RL, var.value);
	}

	var.key = "scummvm_mapper_rr";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		mapper_set_device_keys(RETRO_DEVICE_ID_JOYPAD_RR, var.value);
	}

	var.key = "scummvm_video_hw_acceleration";
	var.value = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
		if (strcmp(var.value, "enabled") == 0) {
			if (video_hw_mode & VIDEO_GRAPHIC_MODE_REQUEST_SW) {
				// video_hw_mode = VIDEO_GRAPHIC_MODE_REQUEST_HW;
				video_hw_mode |= VIDEO_GRAPHIC_MODE_RESET_PENDING;
			} else if (!video_hw_mode)
				video_hw_mode = VIDEO_GRAPHIC_MODE_REQUEST_HW;
		} else {
			if (video_hw_mode & VIDEO_GRAPHIC_MODE_REQUEST_HW) {
				// video_hw_mode = VIDEO_GRAPHIC_MODE_REQUEST_SW;
				video_hw_mode |= VIDEO_GRAPHIC_MODE_RESET_PENDING;
			} else if (!video_hw_mode)
				video_hw_mode = VIDEO_GRAPHIC_MODE_REQUEST_SW;
		}
	}

	if (!(audio_status & AUDIO_STATUS_BUFFER_SUPPORT)) {
		if (frameskip_type > 1) {
			retro_log_cb(RETRO_LOG_WARN, "Selected frameskip mode not available.\n");
			retro_osd_notification("Selected frameskip mode not available");
			frameskip_type = 0;
		}
	}

	if (old_frameskip_type != frameskip_type || old_frame_rate != frame_rate || old_sample_rate != sample_rate) {
		audio_status |= AUDIO_STATUS_UPDATE_LATENCY;
		if (old_frame_rate != frame_rate || old_sample_rate != sample_rate) {
			audio_buffer_init(sample_rate, (uint16) frame_rate);
			if (g_system)
				audio_status |= (AUDIO_STATUS_UPDATE_AV_INFO & AUDIO_STATUS_RESET_PENDING);
		}
	}

	if (video_hw_mode & VIDEO_GRAPHIC_MODE_RESET_PENDING) {
		/* TODO: evaluate if setting can be applied on the fly
		setup_hw_rendering();
		LIBRETRO_G_SYSTEM->resetGraphicsManager();
		retro_reset(); */
		retro_osd_notification("Core reload is needed to apply HW acceleration setting change.");
		video_hw_mode &= ~VIDEO_GRAPHIC_MODE_RESET_PENDING;
	}

	updating_variables = false;
}

static void retro_set_options_display(void) {
	struct retro_core_option_display option_display;

	option_display.visible = opt_frameskip_threshold_display;
	option_display.key = "scummvm_frameskip_threshold";
	environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY, &option_display);

	option_display.visible = opt_frameskip_no_display;
	option_display.key = "scummvm_frameskip_no";
	environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY, &option_display);
}

static bool retro_update_options_display(void) {
	if (updating_variables)
		return false;

	/* Core options */
	bool updated = false;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated) {
		update_variables();
		LIBRETRO_G_SYSTEM->refreshRetroSettings();
		retro_set_options_display();
	}
	return updated;
}

bool retro_setting_get_timing_inaccuracies_enabled() {
	return timing_inaccuracies_enabled;
}

bool retro_setting_get_gamepad_cursor_only(void) {
	return gamepad_cursor_only;
}

int retro_setting_get_analog_deadzone(void) {
	return analog_deadzone;
}

float retro_setting_get_gamepad_cursor_speed(void) {
	return gamepad_cursor_speed;
}

bool retro_setting_get_analog_response_is_quadratic(void) {
	return analog_response_is_quadratic;
}

float retro_setting_get_mouse_speed(void) {
	return mouse_speed;
}

int retro_setting_get_mouse_fine_control_speed_reduction(void) {
	return mouse_fine_control_speed_reduction;
}

float retro_setting_get_gamepad_acceleration_time(void) {
	return gamepad_acceleration_time;
}

float retro_setting_get_frame_rate(void) {
	return frame_rate;
}

bool retro_get_input_bitmask_supported(void) {
	return input_bitmask_supported;
}

uint16 retro_setting_get_sample_rate(void) {
	return sample_rate;
}

uint16 retro_setting_get_audio_samples_buffer_size(void) {
	/* ScummVM audio buffer size is normally between 512 and 8192, but the value
	must be one of: 256, 512, 1024, 2048, 4096, 8192, 16384, or 32768. */
	uint16 v = audio_samples_per_frame--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;

	return ++v;
}

void init_command_params(void) {
	memset(cmd_params, 0, sizeof(cmd_params));
	cmd_params_num = 1;
	strcpy(cmd_params[0], "scummvm\0");
}

int retro_get_input_device(void) {
	return retro_input_device;
}

void parse_command_params(char *cmdline) {
	int j = 0;
	int cmdlen = strlen(cmdline);
	bool quotes = false;

	if (!cmdlen) return;

	/* Append a new line to the end of the command to signify it's finished. */
	cmdline[cmdlen] = '\n';
	cmdline[++cmdlen] = '\0';

	/* parse command line into array of arguments */
	for (int i = 0; i < cmdlen; i++) {
		switch (cmdline[i]) {
		case '\"':
			if (quotes) {
				cmdline[i] = '\0';
				strcpy(cmd_params[cmd_params_num], cmdline + j);
				cmd_params_num++;
				quotes = false;
			} else
				quotes = true;
			j = i + 1;
			break;
		case ' ':
		case '\n':
			if (!quotes) {
				if (i != j && !quotes) {
					cmdline[i] = '\0';
					strcpy(cmd_params[cmd_params_num], cmdline + j);
					cmd_params_num++;
				}
				j = i + 1;
			}
			break;
		}
	}
}

static void exit_to_frontend(void) {
	environ_cb(RETRO_ENVIRONMENT_SHUTDOWN, NULL);
}

static void close_emu_thread(void) {
	while (retro_emu_thread_started() && !retro_emu_thread_exited()) {
		LIBRETRO_G_SYSTEM->requestQuit();
		retro_switch_to_emu_thread();
	}
	retro_deinit_emu_thread();
}

#if defined(WIIU) || defined(__SWITCH__) || defined(_MSC_VER) || defined(_3DS)
#include <stdio.h>
#include <string.h>
char *dirname(char *path) {
	char *p;
	if (path == NULL || *path == '\0')
		return ".";
	p = path + strlen(path) - 1;
	while (*p == '/') {
		if (p == path)
			return path;
		*p-- = '\0';
	}
	while (p >= path && *p != '/')
		p--;
	return p < path ? "." : p == path ? "/" : (*p = '\0', path);
}
#endif

#if (defined(GEKKO) && !defined(WIIU)) || defined(__CELLOS_LV2__)
int access(const char *path, int amode) {
	RFILE *f;
	int mode;

	switch (amode) {
	// we don't really care if a file exists but isn't readable
	case F_OK:
	case R_OK:
		mode = RETRO_VFS_FILE_ACCESS_READ;
		break;

	case W_OK:
		mode = RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING;
		break;

	default:
		return -1;
	}

	f = filestream_open(path, RETRO_VFS_FILE_ACCESS_READ, RETRO_VFS_FILE_ACCESS_HINT_NONE);

	if (f) {
		filestream_close(f);
		return 0;
	}

	return -1;
}
#endif

void retro_set_video_refresh(retro_video_refresh_t cb) {
	video_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb) {}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {
	audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb) {
	poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb) {
	retro_input_cb = cb;
}

void retro_set_environment(retro_environment_t cb) {
	environ_cb = cb;
	bool tmp = true;
	bool has_categories;
	environ_cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &tmp);
	libretro_fill_options_mapper_data(environ_cb);
	libretro_set_core_options(environ_cb, &has_categories);

	/* Core option display callback */
	struct retro_core_options_update_display_callback update_display_callback = {retro_update_options_display};
	environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK, &update_display_callback);
}

unsigned retro_api_version(void) {
	return RETRO_API_VERSION;
}

void retro_get_system_info(struct retro_system_info *info) {
	info->library_name = CORE_NAME;
#if defined GIT_TAG
#define __GIT_VERSION GIT_TAG
#elif defined GIT_HASH
#define __GIT_VERSION GIT_HASH "-" SCUMMVM_VERSION
#else
#define __GIT_VERSION ""
#endif
	info->library_version = __GIT_VERSION;
	info->valid_extensions = "scummvm";
	info->need_fullpath = true;
	info->block_extract = false;
}

void retro_set_size(unsigned width, unsigned height) {
	if (base_width == width && base_height == height) {
		return;
	} else if (width > max_width || height > max_height) {
		max_width = width;
		max_height = height;
		audio_status |= AUDIO_STATUS_UPDATE_AV_INFO;
	} else
		audio_status |= AUDIO_STATUS_UPDATE_GEOMETRY;

	base_width = width;
	base_height = height;
}

void retro_get_system_av_info(struct retro_system_av_info *info) {
	info->geometry.base_width = base_width;
	info->geometry.base_height = base_height;
	info->geometry.max_width = max_width;
	info->geometry.max_height = max_height;
	info->geometry.aspect_ratio = (float)base_width / (float)base_height;
	info->timing.fps = frame_rate;
	info->timing.sample_rate = sample_rate;
}

const char *retro_get_core_dir(void) {
	const char *coredir = NULL;

	environ_cb(RETRO_ENVIRONMENT_GET_LIBRETRO_PATH, &coredir);

	return coredir;
}

const char *retro_get_system_dir(void) {
	const char *sysdir = NULL;

	environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &sysdir);

	return sysdir;
}

const char *retro_get_save_dir(void) {
	const char *savedir = NULL;

	environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &savedir);

	return savedir;
}

const char *retro_get_playlist_dir(void) {
	const char *playlistdir = NULL;

	environ_cb(RETRO_ENVIRONMENT_GET_PLAYLIST_DIRECTORY, &playlistdir);

	return playlistdir;
}

void retro_init(void) {
	struct retro_log_callback log;
	if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
		retro_log_cb = log.log;
	else
		retro_log_cb = NULL;

	retro_log_cb(RETRO_LOG_DEBUG, "ScummVM core version: %s\n", __GIT_VERSION);

	struct retro_audio_buffer_status_callback buf_status_cb;
	buf_status_cb.callback = retro_audio_buff_status_cb;
	audio_status = environ_cb(RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK, &buf_status_cb) ? (audio_status | AUDIO_STATUS_BUFFER_SUPPORT) : (audio_status & ~AUDIO_STATUS_BUFFER_SUPPORT);

	update_variables();

	retro_set_options_display();

	init_command_params();

	setup_hw_rendering();

	environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, retro_input_desc);

	environ_cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void *)retro_controller_lists);

	retro_keyboard_callback cb = {process_key_event_wrapper};
	environ_cb(RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK, &cb);

	if (environ_cb(RETRO_ENVIRONMENT_GET_INPUT_BITMASKS, NULL))
		input_bitmask_supported = true;

	g_system = new OSystem_libretro();
}

void retro_deinit(void) {
	LIBRETRO_G_SYSTEM->destroy();

	if (audio_sample_buffer)
		free(audio_sample_buffer);

	audio_sample_buffer       = NULL;
	audio_samples_per_frame   = 0.0f;
	audio_samples_accumulator = 0.0f;
	log_scummvm_exit_code();
}

void retro_set_controller_port_device(unsigned port, unsigned device) {
	if (port != 0) {
		if (retro_log_cb)
			retro_log_cb(RETRO_LOG_WARN, "Invalid controller port %d, expected port 0 (#1)\n", port);
		return;
	}

	switch (device) {
	case RETRO_DEVICE_JOYPAD:
	case RETRO_DEVICE_MOUSE:
	case RETRO_DEVICE_KEYBOARD:
	case RETRO_DEVICE_ANALOG:
	case RETRO_DEVICE_POINTER:
		retro_input_device = device;
		break;
	default:
		if (retro_log_cb)
			retro_log_cb(RETRO_LOG_WARN, "Invalid controller device class %d.\n", device);
		break;
	}
}

bool retro_load_game(const struct retro_game_info *game) {
	if (!g_system) {
		if (retro_log_cb)
			retro_log_cb(RETRO_LOG_ERROR, "[scummvm] Failed to initialize ScummVM.\n");
		return false;
	}

#ifdef LIBRETRO_DEBUG
	char debug_buf [20];
	sprintf(debug_buf, "--debuglevel=11");
	parse_command_params(debug_buf);
#endif

	if (game) {
		game_buf_ptr = &game_buf;
		memcpy(game_buf_ptr, game, sizeof(retro_game_info));
		// Retrieve the game path.
		Common::FSNode detect_target = Common::FSNode(game->path);
		Common::FSNode parent_dir = detect_target.getParent();
		char target_id[400] = {0};
		char buffer[400] = {0};
		int test_game_status = TEST_GAME_KO_NOT_FOUND;

		const char *target_file_ext = ".scummvm";
		int target_file_ext_pos = strlen(game->path) - strlen(target_file_ext);

		// See if we are loading a .scummvm file.
		if (!(target_file_ext_pos < 0) && strstr(game->path + target_file_ext_pos, target_file_ext) != NULL) {
			// Open the file.
			RFILE *gamefile = filestream_open(game->path, RETRO_VFS_FILE_ACCESS_READ, RETRO_VFS_FILE_ACCESS_HINT_NONE);
			if (!gamefile) {
				retro_log_cb(RETRO_LOG_ERROR, "[scummvm] Failed to load given game file '%s'.\n", game->path);
				return false;
			}

			// Load the file data.
			if (filestream_gets(gamefile, target_id, sizeof(target_id)) == NULL) {
				filestream_close(gamefile);
				retro_log_cb(RETRO_LOG_ERROR, "[scummvm] Failed to load contents of game file '%s'.\n", game->path);
				return false;
			}
			filestream_close(gamefile);

			Common::String tmp = target_id;
			tmp.trim();
			strcpy(target_id, tmp.c_str());

			if (strlen(target_id) == 0) {
				retro_log_cb(RETRO_LOG_ERROR, "[scummvm] Game file '%s' does not contain any target id.\n", game->path);
				return false;
			}

			test_game_status = LIBRETRO_G_SYSTEM->testGame(target_id, false);
		} else {
			if (detect_target.isDirectory()) {
				parent_dir = detect_target;
			} else {
				// If this node has no parent node, then it returns a duplicate of this node.
				if (detect_target.getPath().equals(parent_dir.getPath())) {
					retro_log_cb(RETRO_LOG_ERROR, "[scummvm] Autodetect not possible. No parent directory detected in '%s'.\n", game->path);
					return false;
				}
			}

			test_game_status = LIBRETRO_G_SYSTEM->testGame(parent_dir.getPath().toString().c_str(), true);
		}

		// Preliminary game scan results
		switch (test_game_status) {
		case TEST_GAME_OK_ID_FOUND:
			sprintf(buffer, "-p \"%s\" %s", parent_dir.getPath().toString().c_str(), target_id);
			retro_log_cb(RETRO_LOG_DEBUG, "[scummvm] launch via target id and game dir\n");
			break;
		case TEST_GAME_OK_TARGET_FOUND:
			sprintf(buffer, "%s", target_id);
			retro_log_cb(RETRO_LOG_DEBUG, "[scummvm] launch via target id and scummvm.ini\n");
			break;
		case TEST_GAME_OK_ID_AUTODETECTED:
			sprintf(buffer, "-p \"%s\" --auto-detect", parent_dir.getPath().toString().c_str());
			retro_log_cb(RETRO_LOG_DEBUG, "[scummvm] launch via autodetect\n");
			break;
		case TEST_GAME_KO_MULTIPLE_RESULTS:
			retro_log_cb(RETRO_LOG_WARN, "[scummvm] Multiple targets found for '%s' in scummvm.ini\n", target_id);
			retro_osd_notification("Multiple targets found");
			break;
		case TEST_GAME_KO_NOT_FOUND:
		default:
			retro_log_cb(RETRO_LOG_WARN, "[scummvm] Game not found. Check path and content of '%s'\n", game->path);
			retro_osd_notification("Game not found");
		}

		parse_command_params(buffer);
	} else {
		game_buf_ptr = NULL;
	}

	if (!retro_init_emu_thread()) {
		if (retro_log_cb)
			retro_log_cb(RETRO_LOG_ERROR, "[scummvm] Failed to initialize emulation thread!\n");
		return false;
	}
	return true;
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info) {
	return false;
}

void retro_run(void) {
	/* Settings change is covered by RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK
	except in case of core options reset to defaults, for which the following call is needed*/
	retro_update_options_display();

	if (audio_status & (AUDIO_STATUS_UPDATE_AV_INFO | AUDIO_STATUS_UPDATE_GEOMETRY)) {
		struct retro_system_av_info info;
		retro_get_system_av_info(&info);
		if (audio_status & AUDIO_STATUS_UPDATE_GEOMETRY) {
			environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &info);
			audio_status &= ~AUDIO_STATUS_UPDATE_GEOMETRY;
		} else {
			environ_cb(RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO, &info);
			audio_status &= ~AUDIO_STATUS_UPDATE_AV_INFO;
		}
#ifdef USE_OPENGL
			context_reset();
#endif
	}

	if (audio_status & AUDIO_STATUS_UPDATE_LATENCY) {
		uint32 audio_latency;
		float frame_time_msec = 1000.0f / frame_rate;

		audio_latency = (uint32)((8.0f * frame_time_msec) + 0.5f);
		audio_latency = (audio_latency + 0x1F) & ~0x1F;

		retro_audio_buff_underrun_threshold = frame_time_msec * 100 / audio_latency;

		/* This can only be called from within retro_run() */
		environ_cb(RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY, &audio_latency);
		audio_status &= ~AUDIO_STATUS_UPDATE_LATENCY;
	}

	if (audio_status & AUDIO_STATUS_RESET_PENDING) {
		audio_status &= ~AUDIO_STATUS_RESET_PENDING;
		retro_reset();
		return;
	}

	/* Setting RA's video or audio driver to null will disable video/audio bits */
	int audio_video_enable = 0;
	environ_cb(RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE, &audio_video_enable);

	bool skip_frame = false;

	if (g_system) {

		/* Determine frameskip need based on settings */
		if (frameskip_type == 2)
			skip_frame = ((audio_status & (AUDIO_STATUS_BUFFER_UNDERRUN | AUDIO_STATUS_BUFFER_ACTIVE)) == (AUDIO_STATUS_BUFFER_UNDERRUN | AUDIO_STATUS_BUFFER_ACTIVE));
		else if (frameskip_type == 1)
			skip_frame = !(current_frame % frameskip_no == 0);
		else if (frameskip_type == 3)
			skip_frame = (retro_audio_buff_occupancy < frameskip_threshold);

		/* No frame skipping if
		- no incoming audio (e.g. GUI)
		- doing a THREAD_SWITCH_UPDATE loop */
		skip_frame = skip_frame && !(audio_status & AUDIO_STATUS_MUTE);

		/* Reset frameskip counter if not flagged */
		if ((!skip_frame && frameskip_counter) || frameskip_counter >= FRAMESKIP_MAX) {
			retro_log_cb(RETRO_LOG_DEBUG, "%d frame(s) skipped (%ld)\n", frameskip_counter, current_frame);
			skip_frame = false;
			frameskip_counter = 0;
			/* Keep on skipping frames if flagged */
		} else if (skip_frame)
			frameskip_counter++;

		/* Switch to ScummVM thread */
		retro_switch_to_emu_thread();

		if (retro_emu_thread_exited()) {
			exit_to_frontend();
			return;
		}

		/* Retrieve audio */
		if (audio_video_enable & 2)
			audio_run();

		/* Retrieve video */
		if (!skip_frame && (audio_video_enable & 1)) {
			if (video_hw_mode & VIDEO_GRAPHIC_MODE_REQUEST_SW) {
				const Graphics::ManagedSurface *screen;
				LIBRETRO_G_SYSTEM->getScreen(screen);
				video_cb(screen->getPixels(), screen->w, screen->h, screen->pitch);
			} else
				video_cb(RETRO_HW_FRAME_BUFFER_VALID, LIBRETRO_G_SYSTEM->getScreenWidth(),  LIBRETRO_G_SYSTEM->getScreenHeight(), 0);

		}
		current_frame++;

		poll_cb();
		LIBRETRO_G_SYSTEM->processInputs();
	}
}

void retro_unload_game(void) {
	close_emu_thread();
}

void retro_reset(void) {
	close_emu_thread();
	init_command_params();
	retro_load_game(game_buf_ptr);
	LIBRETRO_G_SYSTEM->resetQuit();
}

// Stubs
void *retro_get_memory_data(unsigned type) {
	return NULL;
}
size_t retro_get_memory_size(unsigned type) {
	return 0;
}
size_t retro_serialize_size(void) {
	return 0;
}
bool retro_serialize(void *data, size_t size) {
	return false;
}
bool retro_unserialize(const void *data, size_t size) {
	return false;
}
void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned unused, bool unused1, const char *unused2) {}

unsigned retro_get_region(void) {
	return RETRO_REGION_NTSC;
}
