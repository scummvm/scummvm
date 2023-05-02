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

#ifndef BACKENDS_LIBRETRO_OS_H
#define BACKENDS_LIBRETRO_OS_H

#define SAMPLE_RATE     48000
#define REFRESH_RATE    60
#define FRAMESKIP_MAX   REFRESH_RATE / 2

// Audio status
#define AUDIO_STATUS_MUTE               (1 << 0)
#define AUDIO_STATUS_BUFFER_SUPPORT     (1 << 1)
#define AUDIO_STATUS_BUFFER_ACTIVE      (1 << 2)
#define AUDIO_STATUS_BUFFER_UNDERRUN    (1 << 3)
#define AUDIO_STATUS_UPDATE_LATENCY     (1 << 4)

// Performance switcher
#define PERF_SWITCH_FRAMESKIP_EVENTS                    REFRESH_RATE / 2
#define PERF_SWITCH_ON                                  (1 << 0)
#define PERF_SWITCH_ENABLE_TIMING_INACCURACIES          (1 << 1)
#define PERF_SWITCH_DISABLE_CONSECUTIVE_SCREEN_UPDATES  (1 << 2)
#define PERF_SWITCH_OVER                                (1 << 3)
#define PERF_SWITCH_RESET_THRESHOLD                     60
#define PERF_SWITCH_RESET_REST                          REFRESH_RATE * 30

// Thread switch caller
#define THREAD_SWITCH_POLL              (1 << 0)
#define THREAD_SWITCH_DELAY             (1 << 1)
#define THREAD_SWITCH_UPDATE            (1 << 2)

// Preliminary scan results
#define TEST_GAME_OK_TARGET_FOUND        0
#define TEST_GAME_OK_ID_FOUND            1
#define TEST_GAME_OK_ID_AUTODETECTED     2
#define TEST_GAME_KO_NOT_FOUND           3
#define TEST_GAME_KO_MULTIPLE_RESULTS    4

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "libretro.h"

#include "base/main.h"
#include "common/system.h"

#ifndef F_OK
#define F_OK 0
#endif

#ifndef W_OK
#define W_OK 2
#endif

#ifndef R_OK
#define R_OK 4
#endif

extern char cmd_params[20][200];
extern char cmd_params_num;

#if (defined(GEKKO) && !defined(WIIU)) || defined(__CELLOS_LV2__)
extern int access(const char *path, int amode);
#endif

OSystem *retroBuildOS();
const Graphics::Surface &getScreen();

void retroProcessMouse(retro_input_state_t aCallback, int device, float gamepad_cursor_speed, float gamepad_acceleration_time, bool analog_response_is_quadratic, int analog_deadzone, float mouse_speed);
void retroQuit(void);
void retroReset(void);
int retroTestGame(const char *game_id, bool autodetect);

void retroSetSystemDir(const char *aPath);
void retroSetSaveDir(const char *aPath);

void retroKeyEvent(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers);

uint8 getThreadSwitchCaller(void);

void retroDestroy(void);

#endif
