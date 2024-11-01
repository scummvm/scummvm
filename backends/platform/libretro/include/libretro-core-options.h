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

#ifndef LIBRETRO_CORE_OPTIONS_H__
#define LIBRETRO_CORE_OPTIONS_H__

#include <stdlib.h>
#include <string.h>

#include <libretro.h>
#include <retro_inline.h>

#ifndef HAVE_NO_LANGEXTRA
#include "libretro-core-options-intl.h"
#endif

/*
 ********************************
 * VERSION: 2.0
 ********************************
 *
 * - 2.0: Add support for core options v2 interface
 * - 1.3: Move translations to libretro_core_options_intl.h
 *        - libretro_core_options_intl.h includes BOM and utf-8
 *          fix for MSVC 2010-2013
 *        - Added HAVE_NO_LANGEXTRA flag to disable translations
 *          on platforms/compilers without BOM support
 * - 1.2: Use core options v1 interface when
 *        RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION is >= 1
 *        (previously required RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION == 1)
 * - 1.1: Support generation of core options v0 retro_core_option_value
 *        arrays containing options with a single value
 * - 1.0: First commit
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 ********************************
 * Core Option Definitions
 ********************************
 */

/* RETRO_LANGUAGE_ENGLISH */

/* Default language:
 * - All other languages must include the same keys and values
 * - Will be used as a fallback in the event that frontend language
 *   is not available
 * - Will be used as a fallback for any missing entries in
 *   frontend language definition */


struct retro_core_option_v2_category option_cats_us[] = {
	{
		"video",
		"Video settings",
		"Configure video settings"
	},
	{
		"cursor",
		"Cursor Movement",
		"Configure cursor movement settings"
	},
	{
		"frameskip",
		"Frameskip",
		"Configure frameskip settings"
	},
	{
		"timing",
		"Timing",
		"Configure timing settings"
	},
	{
		"retropad",
		"RetroPad mapping",
		"Configure RetroPad mapping"
	},
	{ NULL, NULL, NULL },
};

struct retro_core_option_v2_definition option_defs_us[] = {
	{
		"scummvm_gamepad_cursor_only",
		"Cursor > Exclusive cursor control with RetroPad",
		"Exclusive cursor control with RetroPad",
		"Allows the use of RetroPad only to control mouse cursor, excluding the other inputs (e.g. physical mouse, touch screen).",
		NULL,
		"cursor",
		{
			{"disabled", NULL},
			{"enabled", NULL},
			{NULL, NULL},
		},
		"disabled"
	},
	{
		"scummvm_gamepad_cursor_speed",
		"Cursor > Gamepad Cursor Speed",
		"Gamepad Cursor Speed",
		"Sets the mouse cursor speed multiplier when moving the cursor with the RetroPad left analog stick or D-Pad. The default value of '1.0' is optimised for games that have a native resolution of '320x200' or '320x240'. When running 'high definition' games with a resolution of '640x400' or '640x480', a Gamepad Cursor Speed of '2.0' is recommended.",
		NULL,
		"cursor",
		{
			{"0.25", NULL},
			{"0.5", NULL},
			{"0.75", NULL},
			{"1.0", NULL},
			{"1.5", NULL},
			{"2.0", NULL},
			{"2.5", NULL},
			{"3.0", NULL},
			{NULL, NULL},
		},
		"1.0"
	},
	{
		"scummvm_gamepad_cursor_acceleration_time",
		"Cursor > Gamepad Cursor Acceleration",
		"Gamepad Cursor Acceleration",
		"The amount of time (In seconds) it takes for the cursor to reach full speed",
		NULL,
		"cursor",
		{
			{"off", NULL},
			{"0.1", NULL},
			{"0.2", NULL},
			{"0.3", NULL},
			{"0.4", NULL},
			{"0.5", NULL},
			{"0.6", NULL},
			{"0.7", NULL},
			{"0.8", NULL},
			{"0.9", NULL},
			{"1.0", NULL},
			{NULL, NULL},
		},
		"0.2"
	},
	{
		"scummvm_analog_response",
		"Cursor > Analog Cursor Response",
		"Analog Cursor Response",
		"Determines how the speed of the cursor varies when tilting the RetroPad left analog stick. 'Linear': Speed is directly proportional to analog stick displacement. This is standard behaviour with which most users will be familiar. 'Quadratic': Speed increases quadratically with analog stick displacement. This allows for greater precision when making small movements without sacrificing maximum speed at full analog range. This mode may require practice for effective use.",
		NULL,
		"cursor",
		{
			{"linear", "Linear"},
			{"quadratic", "Quadratic"},
			{NULL, NULL},
		},
		"linear"
	},
	{
		"scummvm_analog_deadzone",
		"Cursor > Analog Deadzone",
		"Analog Deadzone",
		"Sets the deadzone in percentage of the RetroPad analog sticks. Used to eliminate cursor drift/unwanted input.",
		NULL,
		"cursor",
		{
			{"0", "0 %"},
			{"5", "5 %"},
			{"10", "10 %"},
			{"15", "15 %"},
			{"20", "20 %"},
			{"25", "25 %"},
			{"30", "30 %"},
			{NULL, NULL},
		},
		"15"
	},
	{
		"scummvm_mouse_speed",
		"Cursor > Mouse Speed",
		"Mouse Speed",
		"Sets the mouse cursor speed multiplier when moving the cursor with the RetroMouse.",
		NULL,
		"cursor",
		{
			{"0.05", NULL},
			{"0.1", NULL},
			{"0.15", NULL},
			{"0.2", NULL},
			{"0.25", NULL},
			{"0.3", NULL},
			{"0.35", NULL},
			{"0.4", NULL},
			{"0.45", NULL},
			{"0.5", NULL},
			{"0.6", NULL},
			{"0.7", NULL},
			{"0.8", NULL},
			{"0.9", NULL},
			{"1.0", NULL},
			{"1.25", NULL},
			{"1.5", NULL},
			{"1.75", NULL},
			{"2.0", NULL},
			{"2.5", NULL},
			{"3.0", NULL},
			{NULL, NULL},
		},
		"1.0"
	},
	{
		"scummvm_mouse_fine_control_speed_reduction",
		"Cursor > Mouse Fine Control Speed Reduction",
		"Mouse Fine Control Speed Reduction",
		"Sets the mouse cursor speed reduction as percentage of normal speed when fine control is activated.",
		NULL,
		"cursor",
		{
			{"2", "50 %"},
			{"4", "20 %"},
			{"10", "10 %"},
			{NULL, NULL},
		},
		"4"
	},
	{
		"scummvm_frameskip_type",
		"Frameskip > Frameskip Mode",
		"Frameskip Mode",
		"Skip frames to avoid audio buffer under-run (crackling). Improves performance at the expense of visual smoothness. 'Auto' skips frames when advised by the frontend. 'Threshold' uses the 'Frameskip Threshold (%)' setting. 'Fixed' uses the 'Fixed Frameskip' setting.",
		NULL,
		"frameskip",
		{
			{ "disabled", NULL },
			{ "fixed", "Fixed" },
			{ "auto", "Auto" },
			{ "manual", "Threshold" },
			{ NULL, NULL },
		},
		"auto"
	},
	{
		"scummvm_frameskip_threshold",
		"Frameskip > Frameskip Threshold (%)",
		"Frameskip Threshold (%)",
		"When 'Frameskip' is set to 'Threshold', specifies the audio buffer occupancy threshold (percentage) below which frames will be skipped. Higher values reduce the risk of crackling by causing frames to be dropped more frequently.",
		NULL,
		"frameskip",
		{
			{ "15", NULL },
			{ "18", NULL },
			{ "21", NULL },
			{ "24", NULL },
			{ "27", NULL },
			{ "30", NULL },
			{ "33", NULL },
			{ "36", NULL },
			{ "39", NULL },
			{ "42", NULL },
			{ "45", NULL },
			{ "48", NULL },
			{ "51", NULL },
			{ "54", NULL },
			{ "57", NULL },
			{ "60", NULL },
			{ NULL, NULL },
		},
		"33"
	},
	{
		"scummvm_frameskip_no",
		"Frameskip > Fixed Frameskip",
		"Fixed Frameskip",
		"When 'Frameskip' is set to 'Fixed', or if the frontend doesn't support the alternative 'Frameskip' mode, skip rendering at a fixed rate of X frames out of X+1",
		NULL,
		"frameskip",
		{
			{ "0", "No skipping" },
			{ "1", "Skip rendering of 1 frames out of 2" },
			{ "2", "Skip rendering of 2 frames out of 3" },
			{ "3", "Skip rendering of 3 frames out of 4" },
			{ "4", "Skip rendering of 4 frames out of 5" },
			{ "5", "Skip rendering of 5 frames out of 6" },
			{ NULL, NULL },
		},
		"0"
	},
	{
		"scummvm_allow_timing_inaccuracies",
		"Timing > Allow Timing Inaccuracies",
		"Allow Timing Inaccuracies",
		"Allow timing inaccuracies that reduces CPU requirements. Though most timing deviations are imperceptible, in some cases it may introduce audio sync/timing issues, hence this option should be enabled only if full speed cannot be reached otherwise.",
		NULL,
		"timing",
		{
			{"disabled", NULL},
			{"enabled", NULL},
			{NULL, NULL},
		},
		"disabled"
	},
	{
		"scummvm_framerate",
		"Timing > Frame rate cap",
		"Frame rate cap",
		"Set core frame rate upper limit. Changing this setting will reset the core.",
		NULL,
		"timing",
		{
			{ "disabled", NULL },
			{ "60 Hz", NULL },
			{ "50 Hz", NULL },
			{ "30 Hz", NULL },
			{ "25 Hz", NULL },
			{ NULL, NULL },
		},
		"disabled"
	},
	{
		"scummvm_samplerate",
		"Timing > Sample rate",
		"Sample rate",
		"Set core sample rate. Changing this setting will reset the core.",
		NULL,
		"timing",
		{
			{ "48000 Hz", NULL },
			{ "44100 Hz", NULL },
			{ NULL, NULL },
		},
		"48000 Hz"
	},
	/* Button mappings */
	{
		"scummvm_mapper_up",
		"RetroPad > Up",
		"Up",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROKE_UP"
	},
	{
		"scummvm_mapper_down",
		"RetroPad > Down",
		"Down",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROKE_DOWN"
	},
	{
		"scummvm_mapper_left",
		"RetroPad > Left",
		"Left",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROKE_LEFT"
	},
	{
		"scummvm_mapper_right",
		"RetroPad > Right",
		"Right",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROKE_RIGHT"
	},
	{
		"scummvm_mapper_a",
		"RetroPad > A",
		"A",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROK_SPACE"
	},
	{
		"scummvm_mapper_b",
		"RetroPad > B",
		"B",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROK_RETURN"
	},
	{
		"scummvm_mapper_x",
		"RetroPad > X",
		"X",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROK_F5"
	},
	{
		"scummvm_mapper_y",
		"RetroPad > Y",
		"Y",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROK_ESCAPE"
	},
	{
		"scummvm_mapper_select",
		"RetroPad > Select",
		"Select",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROKE_VKBD"
	},
	{
		"scummvm_mapper_start",
		"RetroPad > Start",
		"Start",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROKE_SCUMMVM_GUI"
	},
	{
		"scummvm_mapper_l",
		"RetroPad > L",
		"L",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROKE_LEFT_BUTTON"
	},
	{
		"scummvm_mapper_r",
		"RetroPad > R",
		"R",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROKE_RIGHT_BUTTON"
	},
	{
		"scummvm_mapper_l2",
		"RetroPad > L2",
		"L2",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"---"
	},
	{
		"scummvm_mapper_r2",
		"RetroPad > R2",
		"R2",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROKE_FINE_CONTROL"
	},
	{
		"scummvm_mapper_l3",
		"RetroPad > L3",
		"L3",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"---"
	},
	{
		"scummvm_mapper_r3",
		"RetroPad > R3",
		"R3",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"---"
	},
	/* Left Stick */
	{
		"scummvm_mapper_lu",
		"RetroPad > Left Analog > Up",
		"Left Analog > Up",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROKE_UP"
	},
	{
		"scummvm_mapper_ld",
		"RetroPad > Left Analog > Down",
		"Left Analog > Down",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROKE_DOWN"
	},
	{
		"scummvm_mapper_ll",
		"RetroPad > Left Analog > Left",
		"Left Analog > Left",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROKE_LEFT"
	},
	{
		"scummvm_mapper_lr",
		"RetroPad > Left Analog > Right",
		"Left Analog > Right",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROKE_RIGHT"
	},
	/* Right Stick */
	{
		"scummvm_mapper_ru",
		"RetroPad > Right Analog > Up",
		"Right Analog > Up",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROK_UP"
	},
	{
		"scummvm_mapper_rd",
		"RetroPad > Right Analog > Down",
		"Right Analog > Down",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROK_DOWN"
	},
	{
		"scummvm_mapper_rl",
		"RetroPad > Right Analog > Left",
		"Right Analog > Left",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROK_LEFT"
	},
	{
		"scummvm_mapper_rr",
		"RetroPad > Right Analog > Right",
		"Right Analog > Right",
		NULL,
		NULL,
		"retropad",
		{
			{ NULL, NULL }
		},
		"RETROK_RIGHT"
	},
	{
		"scummvm_video_hw_acceleration",
		"Video > Hardware acceleration",
		"Hardware acceleration",
		"Request video hardware acceleration (OpenGL or OpenGLES2) to the frontend if supported. It is needed to reload the core to apply this setting.",
		NULL,
		"video",
		{
			{"disabled", NULL},
#ifdef USE_OPENGL
			{"enabled", NULL},
#endif
			{NULL, NULL},		},
#ifdef USE_OPENGL
		"enabled"
#else
		"disabled"
#endif
	},
#ifdef USE_HIGHRES
	{
		"scummvm_gui_aspect_ratio",
		"Video > GUI aspect ratio",
		"ScummVM Launcher aspect ratio",
		"Set ScummVM Launcher aspect ratio.",
		NULL,
		"video",
		{
			{"0", "4:3"},
			{"1", "16:9"},
			{NULL, NULL},		},
		"1"
	},
	{
		"scummvm_gui_h_res",
		"Video > GUI resolution",
		"ScummVM Launcher resolution",
		"Set ScummVM Launcher aspect ratio.",
		NULL,
		"video",
		{
			{"240", "LD"},
			{"480", "SD"},
			{"720", "HD"},
			{"1080", "FHD"},
			{NULL, NULL},		},
		"720"
	},
#endif
	{ NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};

struct retro_core_options_v2 options_us = {
	option_cats_us,
	option_defs_us
};

/* List has been reduced to fit RETRO_NUM_CORE_OPTION_VALUES_MAX.
 * Latest element {NULL, NULL} has been omitted in this case as the array is exactly sized
 * RETRO_NUM_CORE_OPTION_VALUES_MAX; in case the array size will be reduced, {NULL, NULL}
 * element to be uncommented.
 */

struct retro_core_option_value retro_keys_label_us [] = {
	{"---", "---"},
	{"RETROKE_VKBD", "Toggle Virtual Keyboard"},
	{"RETROKE_LEFT", "Mouse Cursor Left"},
	{"RETROKE_UP", "Mouse Cursor Up"},
	{"RETROKE_DOWN", "Mouse Cursor Down"},
	{"RETROKE_RIGHT", "Mouse Cursor Right"},
	{"RETROKE_LEFT_BUTTON", "Mouse Left Button"},
	{"RETROKE_RIGHT_BUTTON", "Mouse Right Button"},
	{"RETROKE_FINE_CONTROL", "Mouse Cursor Fine Control"},
	{"RETROKE_SCUMMVM_GUI", "ScummVM GUI"},
	{"RETROKE_SHIFT_MOD", "Keyboard Shift (Modifier)"},
	{"RETROKE_CTRL_MOD", "Keyboard Control (Modifier)"},
	{"RETROKE_ALT_MOD", "Keyboard Alt (Modifier)"},
	{"RETROK_BACKSPACE", "Keyboard Backspace"},
	{"RETROK_TAB", "Keyboard Tab"},
	{"RETROK_CLEAR", "Keyboard Clear"},
	{"RETROK_RETURN", "Keyboard Return"},
	{"RETROK_PAUSE", "Keyboard Pause"},
	{"RETROK_ESCAPE", "Keyboard Escape"},
	{"RETROK_SPACE", "Keyboard Space"},
	{"RETROK_EXCLAIM", "Keyboard !"},
	{"RETROK_QUOTEDBL", "Keyboard \""},
	{"RETROK_HASH", "Keyboard #"},
	{"RETROK_DOLLAR", "Keyboard $"},
	{"RETROK_AMPERSAND", "Keyboard &"},
	{"RETROK_QUOTE", "Keyboard \'"},
	{"RETROK_LEFTPAREN", "Keyboard ("},
	{"RETROK_RIGHTPAREN", "Keyboard )"},
	{"RETROK_ASTERISK", "Keyboard *"},
	{"RETROK_PLUS", "Keyboard +"},
	{"RETROK_COMMA", "Keyboard ,"},
	{"RETROK_MINUS", "Keyboard -"},
	{"RETROK_PERIOD", "Keyboard ."},
	{"RETROK_SLASH", "Keyboard /"},
	{"RETROK_0", "Keyboard 0"},
	{"RETROK_1", "Keyboard 1"},
	{"RETROK_2", "Keyboard 2"},
	{"RETROK_3", "Keyboard 3"},
	{"RETROK_4", "Keyboard 4"},
	{"RETROK_5", "Keyboard 5"},
	{"RETROK_6", "Keyboard 6"},
	{"RETROK_7", "Keyboard 7"},
	{"RETROK_8", "Keyboard 8"},
	{"RETROK_9", "Keyboard 9"},
	{"RETROK_COLON", "Keyboard :"},
	{"RETROK_SEMICOLON", "Keyboard ;"},
	{"RETROK_LESS", "Keyboard <"},
	{"RETROK_EQUALS", "Keyboard ="},
	{"RETROK_GREATER", "Keyboard >"},
	{"RETROK_QUESTION", "Keyboard ?"},
	{"RETROK_AT", "Keyboard @"},
	{"RETROK_LEFTBRACKET", "Keyboard ["},
	{"RETROK_BACKSLASH", "Keyboard \\"},
	{"RETROK_RIGHTBRACKET", "Keyboard ]"},
	{"RETROK_CARET", "Keyboard ^"},
	{"RETROK_UNDERSCORE", "Keyboard _"},
	{"RETROK_BACKQUOTE", "Keyboard `"},
	{"RETROK_a", "Keyboard a"},
	{"RETROK_b", "Keyboard b"},
	{"RETROK_c", "Keyboard c"},
	{"RETROK_d", "Keyboard d"},
	{"RETROK_e", "Keyboard e"},
	{"RETROK_f", "Keyboard f"},
	{"RETROK_g", "Keyboard g"},
	{"RETROK_h", "Keyboard h"},
	{"RETROK_i", "Keyboard i"},
	{"RETROK_j", "Keyboard j"},
	{"RETROK_k", "Keyboard k"},
	{"RETROK_l", "Keyboard l"},
	{"RETROK_m", "Keyboard m"},
	{"RETROK_n", "Keyboard n"},
	{"RETROK_o", "Keyboard o"},
	{"RETROK_p", "Keyboard p"},
	{"RETROK_q", "Keyboard q"},
	{"RETROK_r", "Keyboard r"},
	{"RETROK_s", "Keyboard s"},
	{"RETROK_t", "Keyboard t"},
	{"RETROK_u", "Keyboard u"},
	{"RETROK_v", "Keyboard v"},
	{"RETROK_w", "Keyboard w"},
	{"RETROK_x", "Keyboard x"},
	{"RETROK_y", "Keyboard y"},
	{"RETROK_z", "Keyboard z"},
	{"RETROK_KP0", "Keyboard Numpad 0"},
	{"RETROK_KP1", "Keyboard Numpad 1"},
	{"RETROK_KP2", "Keyboard Numpad 2"},
	{"RETROK_KP3", "Keyboard Numpad 3"},
	{"RETROK_KP4", "Keyboard Numpad 4"},
	{"RETROK_KP5", "Keyboard Numpad 5"},
	{"RETROK_KP6", "Keyboard Numpad 6"},
	{"RETROK_KP7", "Keyboard Numpad 7"},
	{"RETROK_KP8", "Keyboard Numpad 8"},
	{"RETROK_KP9", "Keyboard Numpad 9"},
	{"RETROK_KP_PERIOD", "Keyboard Numpad ."},
	{"RETROK_KP_DIVIDE", "Keyboard Numpad /"},
	{"RETROK_KP_MULTIPLY", "Keyboard Numpad *"},
	{"RETROK_KP_MINUS", "Keyboard Numpad -"},
	{"RETROK_KP_PLUS", "Keyboard Numpad +"},
	{"RETROK_KP_ENTER", "Keyboard Numpad Enter"},
	{"RETROK_KP_EQUALS", "Keyboard Numpad ="},
	{"RETROK_UP", "Keyboard Up"},
	{"RETROK_DOWN", "Keyboard Down"},
	{"RETROK_LEFT", "Keyboard Left"},
	{"RETROK_RIGHT", "Keyboard Right"},
	{"RETROK_INSERT", "Keyboard Insert"},
	{"RETROK_DELETE", "Keyboard Delete"},
	{"RETROK_HOME", "Keyboard Home"},
	{"RETROK_END", "Keyboard End"},
	{"RETROK_PAGEUP", "Keyboard PageUp"},
	{"RETROK_PAGEDOWN", "Keyboard PageDown"},
	{"RETROK_F1", "Keyboard F1"},
	{"RETROK_F2", "Keyboard F2"},
	{"RETROK_F3", "Keyboard F3"},
	{"RETROK_F4", "Keyboard F4"},
	{"RETROK_F5", "Keyboard F5"},
	{"RETROK_F6", "Keyboard F6"},
	{"RETROK_F7", "Keyboard F7"},
	{"RETROK_F8", "Keyboard F8"},
	{"RETROK_F9", "Keyboard F9"},
	{"RETROK_F10", "Keyboard F10"},
	{"RETROK_F11", "Keyboard F11"},
	{"RETROK_F12", "Keyboard F12"},
//{"RETROK_F13","Keyboard F13"},
//{"RETROK_F14","Keyboard F14"},
//{"RETROK_F15","Keyboard F15"},
//{"RETROK_NUMLOCK","Keyboard NumLock"},
//{"RETROK_CAPSLOCK","Keyboard Caps Lock"},
//{"RETROK_SCROLLOCK","Keyboard Scroll Lock"},
	{"RETROK_LSHIFT", "Keyboard Left Shift"},
	{"RETROK_RSHIFT", "Keyboard Right Shift"},
	{"RETROK_LCTRL", "Keyboard Left Control"},
	{"RETROK_RCTRL", "Keyboard Right Control"},
	{"RETROK_LALT", "Keyboard Left Alt"},
	{"RETROK_RALT", "Keyboard Right Alt"},
//{"RETROK_LMETA","Keyboard Left Meta"},
//{"RETROK_RMETA","Keyboard Right Meta"},
//{"RETROK_LSUPER","Keyboard Left Super"},
//{"RETROK_RSUPER","Keyboard Right Super"},
//{"RETROK_MODE","Keyboard Mode"},
//{"RETROK_COMPOSE","Keyboard Compose"},
//{"RETROK_HELP","Keyboard Help"},
//{"RETROK_PRINT","Keyboard Print"},
//{"RETROK_SYSREQ","Keyboard SysReq"},
//{"RETROK_BREAK","Keyboard Break"},
//{"RETROK_MENU","Keyboard Menu"},
//{"RETROK_POWER","Keyboard Power"},
//{"RETROK_EURO","Keyboard Euro"},
//{"RETROK_UNDO","Keyboard Undo"},
//{NULL, NULL}
};

/*
 ********************************
 * Language Mapping
 ********************************
 */

#ifndef HAVE_NO_LANGEXTRA
struct retro_core_options_v2 *options_intl[RETRO_LANGUAGE_LAST] = {
	&options_us, /* RETRO_LANGUAGE_ENGLISH */
	NULL,           /* RETRO_LANGUAGE_JAPANESE */
	NULL,           /* RETRO_LANGUAGE_FRENCH */
	NULL,           /* RETRO_LANGUAGE_SPANISH */
	NULL,           /* RETRO_LANGUAGE_GERMAN */
	&options_it, /* RETRO_LANGUAGE_ITALIAN */
	NULL,           /* RETRO_LANGUAGE_DUTCH */
	NULL,           /* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */
	NULL,           /* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */
	NULL,           /* RETRO_LANGUAGE_RUSSIAN */
	NULL,           /* RETRO_LANGUAGE_KOREAN */
	NULL,           /* RETRO_LANGUAGE_CHINESE_TRADITIONAL */
	NULL,           /* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */
	NULL,           /* RETRO_LANGUAGE_ESPERANTO */
	NULL,           /* RETRO_LANGUAGE_POLISH */
	NULL,           /* RETRO_LANGUAGE_VIETNAMESE */
	NULL,           /* RETRO_LANGUAGE_ARABIC */
	NULL,           /* RETRO_LANGUAGE_GREEK */
	NULL,           /* RETRO_LANGUAGE_TURKISH */
};

struct retro_core_option_value *retro_keys_labels_intl[RETRO_LANGUAGE_LAST] = {
	retro_keys_label_us, /* RETRO_LANGUAGE_ENGLISH */
	NULL,           /* RETRO_LANGUAGE_JAPANESE */
	NULL,           /* RETRO_LANGUAGE_FRENCH */
	NULL,           /* RETRO_LANGUAGE_SPANISH */
	NULL,           /* RETRO_LANGUAGE_GERMAN */
	retro_keys_label_it, /* RETRO_LANGUAGE_ITALIAN */
	NULL,           /* RETRO_LANGUAGE_DUTCH */
	NULL,           /* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */
	NULL,           /* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */
	NULL,           /* RETRO_LANGUAGE_RUSSIAN */
	NULL,           /* RETRO_LANGUAGE_KOREAN */
	NULL,           /* RETRO_LANGUAGE_CHINESE_TRADITIONAL */
	NULL,           /* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */
	NULL,           /* RETRO_LANGUAGE_ESPERANTO */
	NULL,           /* RETRO_LANGUAGE_POLISH */
	NULL,           /* RETRO_LANGUAGE_VIETNAMESE */
	NULL,           /* RETRO_LANGUAGE_ARABIC */
	NULL,           /* RETRO_LANGUAGE_GREEK */
	NULL,           /* RETRO_LANGUAGE_TURKISH */
};
#endif

/*
 ********************************
 * Functions
 ********************************
 */

static INLINE void libretro_fill_options_mapper_data(retro_environment_t environ_cb) {
	/* Fill in the values for all the mappers */
	int m = 0;
	int n = 0;
	while (option_defs_us[m].key) {
		if (strstr(option_defs_us[m].key, "scummvm_mapper_")) {
			n = 0;
			while (retro_keys_label_us[n].label && n < RETRO_NUM_CORE_OPTION_VALUES_MAX - 1) {
				option_defs_us[m].values[n].value = retro_keys_label_us[n].value;
				option_defs_us[m].values[n].label = retro_keys_label_us[n].label;
				++n;
			}
			option_defs_us[m].values[n].value = NULL;
			option_defs_us[m].values[n].label = NULL;
		}
		++m;
	}
#ifndef HAVE_NO_LANGEXTRA
	struct retro_core_option_value *retro_keys_label_intl = NULL;
	unsigned language = 0;

	if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
	        (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH))
		retro_keys_label_intl = retro_keys_labels_intl[language];

	if (! options_intl[language] || ! retro_keys_label_intl)
		return;

	m = 0;
	n = 0;
	while (options_intl[language]->definitions[m].key) {
		if (strstr(options_intl[language]->definitions[m].key, "scummvm_mapper_")) {
			n = 0;
			while ((retro_keys_label_intl + n)->label && n < RETRO_NUM_CORE_OPTION_VALUES_MAX - 1) {
				options_intl[language]->definitions[m].values[n].value = (retro_keys_label_intl + n)->value;
				options_intl[language]->definitions[m].values[n].label = (retro_keys_label_intl + n)->label;
				++n;
			}
			options_intl[language]->definitions[m].values[n].value = NULL;
			options_intl[language]->definitions[m].values[n].label = NULL;
		}
		++m;
	}
#endif
}

/* Handles configuration/setting of core options.
 * Should be called as early as possible - ideally inside
 * retro_set_environment(), and no later than retro_load_game()
 * > We place the function body in the header to avoid the
 *   necessity of adding more .c files (i.e. want this to
 *   be as painless as possible for core devs)
 */

static INLINE void libretro_set_core_options(retro_environment_t environ_cb,
        bool *categories_supported) {
	unsigned version  = 0;
#ifndef HAVE_NO_LANGEXTRA
	unsigned language = 0;
#endif

	if (!environ_cb || !categories_supported)
		return;

	*categories_supported = false;

	if (!environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version))
		version = 0;

	if (version >= 2) {
#ifndef HAVE_NO_LANGEXTRA
		struct retro_core_options_v2_intl core_options_intl;

		core_options_intl.us    = &options_us;
		core_options_intl.local = NULL;

		if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
		        (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH))
			core_options_intl.local = options_intl[language];

		*categories_supported = environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL,
		                                   &core_options_intl);
#else
		*categories_supported = environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2,
		                                   &options_us);
#endif
	} else {
		size_t i, j;
		size_t option_index              = 0;
		size_t num_options               = 0;
		struct retro_core_option_definition
			*option_v1_defs_us         = NULL;
#ifndef HAVE_NO_LANGEXTRA
		size_t num_options_intl          = 0;
		struct retro_core_option_v2_definition
			*option_defs_intl          = NULL;
		struct retro_core_option_definition
			*option_v1_defs_intl       = NULL;
		struct retro_core_options_intl
			core_options_v1_intl;
#endif
		struct retro_variable *variables = NULL;
		char **values_buf                = NULL;

		/* Determine total number of options */
		while (true) {
			if (option_defs_us[num_options].key)
				num_options++;
			else
				break;
		}

		if (version >= 1) {
			/* Allocate US array */
			option_v1_defs_us = (struct retro_core_option_definition *)
			                    calloc(num_options + 1, sizeof(struct retro_core_option_definition));

			/* Copy parameters from option_defs_us array */
			for (i = 0; i < num_options; i++) {
				struct retro_core_option_v2_definition *option_def_us = &option_defs_us[i];
				struct retro_core_option_value *option_values         = option_def_us->values;
				struct retro_core_option_definition *option_v1_def_us = &option_v1_defs_us[i];
				struct retro_core_option_value *option_v1_values      = option_v1_def_us->values;

				option_v1_def_us->key           = option_def_us->key;
				option_v1_def_us->desc          = option_def_us->desc;
				option_v1_def_us->info          = option_def_us->info;
				option_v1_def_us->default_value = option_def_us->default_value;

				/* Values must be copied individually... */
				while (option_values->value) {
					option_v1_values->value = option_values->value;
					option_v1_values->label = option_values->label;

					option_values++;
					option_v1_values++;
				}
			}

#ifndef HAVE_NO_LANGEXTRA
			if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
			        (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH) &&
			        options_intl[language])
				option_defs_intl = options_intl[language]->definitions;

			if (option_defs_intl) {
				/* Determine number of intl options */
				while (true) {
					if (option_defs_intl[num_options_intl].key)
						num_options_intl++;
					else
						break;
				}

				/* Allocate intl array */
				option_v1_defs_intl = (struct retro_core_option_definition *)
				                      calloc(num_options_intl + 1, sizeof(struct retro_core_option_definition));

				/* Copy parameters from option_defs_intl array */
				for (i = 0; i < num_options_intl; i++) {
					struct retro_core_option_v2_definition *option_def_intl = &option_defs_intl[i];
					struct retro_core_option_value *option_values           = option_def_intl->values;
					struct retro_core_option_definition *option_v1_def_intl = &option_v1_defs_intl[i];
					struct retro_core_option_value *option_v1_values        = option_v1_def_intl->values;

					option_v1_def_intl->key           = option_def_intl->key;
					option_v1_def_intl->desc          = option_def_intl->desc;
					option_v1_def_intl->info          = option_def_intl->info;
					option_v1_def_intl->default_value = option_def_intl->default_value;

					/* Values must be copied individually... */
					while (option_values->value) {
						option_v1_values->value = option_values->value;
						option_v1_values->label = option_values->label;

						option_values++;
						option_v1_values++;
					}
				}
			}

			core_options_v1_intl.us    = option_v1_defs_us;
			core_options_v1_intl.local = option_v1_defs_intl;

			environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL, &core_options_v1_intl);
#else
			environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS, option_v1_defs_us);
#endif
		} else {
			/* Allocate arrays */
			variables  = (struct retro_variable *)calloc(num_options + 1,
			             sizeof(struct retro_variable));
			values_buf = (char **)calloc(num_options, sizeof(char *));

			if (!variables || !values_buf)
				goto error;

			/* Copy parameters from option_defs_us array */
			for (i = 0; i < num_options; i++) {
				const char *key                        = option_defs_us[i].key;
				const char *desc                       = option_defs_us[i].desc;
				const char *default_value              = option_defs_us[i].default_value;
				struct retro_core_option_value *values = option_defs_us[i].values;
				size_t buf_len                         = 3;
				size_t default_index                   = 0;

				values_buf[i] = NULL;

				if (desc) {
					size_t num_values = 0;

					/* Determine number of values */
					while (true) {
						if (values[num_values].value) {
							/* Check if this is the default value */
							if (default_value)
								if (strcmp(values[num_values].value, default_value) == 0)
									default_index = num_values;

							buf_len += strlen(values[num_values].value);
							num_values++;
						} else
							break;
					}

					/* Build values string */
					if (num_values > 0) {
						buf_len += num_values - 1;
						buf_len += strlen(desc);

						values_buf[i] = (char *)calloc(buf_len, sizeof(char));
						if (!values_buf[i])
							goto error;

						strcpy(values_buf[i], desc);
						strcat(values_buf[i], "; ");

						/* Default value goes first */
						strcat(values_buf[i], values[default_index].value);

						/* Add remaining values */
						for (j = 0; j < num_values; j++) {
							if (j != default_index) {
								strcat(values_buf[i], "|");
								strcat(values_buf[i], values[j].value);
							}
						}
					}
				}

				variables[option_index].key   = key;
				variables[option_index].value = values_buf[i];
				option_index++;
			}

			/* Set variables */
			environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
		}

error:
		/* Clean up */

		if (option_v1_defs_us) {
			free(option_v1_defs_us);
			option_v1_defs_us = NULL;
		}

#ifndef HAVE_NO_LANGEXTRA
		if (option_v1_defs_intl) {
			free(option_v1_defs_intl);
			option_v1_defs_intl = NULL;
		}
#endif

		if (values_buf) {
			for (i = 0; i < num_options; i++) {
				if (values_buf[i]) {
					free(values_buf[i]);
					values_buf[i] = NULL;
				}
			}

			free(values_buf);
			values_buf = NULL;
		}

		if (variables) {
			free(variables);
			variables = NULL;
		}
	}
}

#ifdef __cplusplus
}
#endif

#endif
