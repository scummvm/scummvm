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
 * VERSION: 1.3
 ********************************
 *
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

struct retro_core_option_definition option_defs_us[] = {
	{
		"scummvm_gamepad_cursor_speed",
		"Gamepad Cursor Speed",
		"Sets the mouse cursor speed multiplier when moving the cursor with the RetroPad left analog stick or D-Pad. The default value of '1.0' is optimised for games that have a native resolution of '320x200' or '320x240'. When running 'high definition' games with a resolution of '640x400' or '640x480', a Gamepad Cursor Speed of '2.0' is recommended.",
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
		"Gamepad Cursor Acceleration",
		"The amount of time (In seconds) it takes for the cursor to reach full speed",
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
		"Analog Cursor Response",
		"Determines how the speed of the cursor varies when tilting the RetroPad left analog stick. 'Linear': Speed is directly proportional to analog stick displacement. This is standard behaviour with which most users will be familiar. 'Quadratic': Speed increases quadratically with analog stick displacement. This allows for greater precision when making small movements without sacrificing maximum speed at full analog range. This mode may require practice for effective use.",
		{
			{"linear", "Linear"},
			{"quadratic", "Quadratic"},
			{NULL, NULL},
		},
		"linear"
	},
	{
		"scummvm_analog_deadzone",
		"Analog Deadzone (Percent)",
		"Sets the deadzone of the RetroPad analog sticks. Used to eliminate cursor drift/unwanted input.",
		{
			{"0", NULL},
			{"5", NULL},
			{"10", NULL},
			{"15", NULL},
			{"20", NULL},
			{"25", NULL},
			{"30", NULL},
			{NULL, NULL},
		},
		"15"
	},
	{
		"scummvm_mouse_speed",
		"Mouse Speed",
		"Sets the mouse cursor speed multiplier when moving the cursor with the RetroMouse.",
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
		"scummvm_speed_hack",
		"Speed Hack (Restart)",
		"Enables a speed hack that significantly reduces CPU requirements by allowing subtle timing inaccuracies. This hack is considered 'safe' - it should cause no errors, and most timing deviations are imperceptible. It remains a hack, though, and users of desktop-class machines are advised to keep it disabled. On low power hardware (weak Android devices, single board computers), this hack is essential for full speed operation of the core.",
		{
			{"disabled", NULL},
			{"enabled", NULL},
			{NULL, NULL},
		},
#if defined(ANDROID) || defined(DINGUX) || defined(_3DS)
		"enabled"
#else
		"disabled"
#endif
	},
	{NULL, NULL, NULL, {{0}}, NULL},
};

/*
 ********************************
 * Language Mapping
 ********************************
 */

#ifndef HAVE_NO_LANGEXTRA
struct retro_core_option_definition *option_defs_intl[RETRO_LANGUAGE_LAST] = {
	option_defs_us, /* RETRO_LANGUAGE_ENGLISH */
	NULL,           /* RETRO_LANGUAGE_JAPANESE */
	NULL,           /* RETRO_LANGUAGE_FRENCH */
	NULL,           /* RETRO_LANGUAGE_SPANISH */
	NULL,           /* RETRO_LANGUAGE_GERMAN */
	option_defs_it, /* RETRO_LANGUAGE_ITALIAN */
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

/* Handles configuration/setting of core options.
 * Should be called as early as possible - ideally inside
 * retro_set_environment(), and no later than retro_load_game()
 * > We place the function body in the header to avoid the
 *   necessity of adding more .c files (i.e. want this to
 *   be as painless as possible for core devs)
 */

static INLINE void libretro_set_core_options(retro_environment_t environ_cb) {
	unsigned version = 0;

	if (!environ_cb)
		return;

	if (environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version) && (version >= 1)) {
#ifndef HAVE_NO_LANGEXTRA
		struct retro_core_options_intl core_options_intl;
		unsigned language = 0;

		core_options_intl.us = option_defs_us;
		core_options_intl.local = NULL;

		if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) && (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH))
			core_options_intl.local = option_defs_intl[language];

		environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL, &core_options_intl);
#else
		environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS, &option_defs_us);
#endif
	} else {
		size_t i;
		size_t num_options = 0;
		struct retro_variable *variables = NULL;
		char **values_buf = NULL;

		/* Determine number of options */
		while (true) {
			if (option_defs_us[num_options].key)
				num_options++;
			else
				break;
		}

		/* Allocate arrays */
		variables = (struct retro_variable *)calloc(num_options + 1, sizeof(struct retro_variable));
		values_buf = (char **)calloc(num_options, sizeof(char *));

		if (!variables || !values_buf)
			goto error;

		/* Copy parameters from option_defs_us array */
		for (i = 0; i < num_options; i++) {
			const char *key = option_defs_us[i].key;
			const char *desc = option_defs_us[i].desc;
			const char *default_value = option_defs_us[i].default_value;
			struct retro_core_option_value *values = option_defs_us[i].values;
			size_t buf_len = 3;
			size_t default_index = 0;

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
					size_t j;

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

			variables[i].key = key;
			variables[i].value = values_buf[i];
		}

		/* Set variables */
		environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);

error:

		/* Clean up */
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
