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

#ifndef LIBRETRO_DEFS_H
#define LIBRETRO_DEFS_H

/* Workaround for a RetroArch audio driver
 * limitation: a maximum of 1024 frames
 * can be written per call of audio_batch_cb() */
#define AUDIO_BATCH_FRAMES_MAX 1024

// System analog stick range is -0x8000 to 0x8000
#define ANALOG_RANGE 0x8000

/* TODO: remove the following definition when libretro-common
will be updated to include it */
#ifndef RETRO_ENVIRONMENT_GET_PLAYLIST_DIRECTORY
#define RETRO_ENVIRONMENT_GET_PLAYLIST_DIRECTORY 79
#endif

#define DEFAULT_SAMPLE_RATE     	48000
#define DEFAULT_REFRESH_RATE    	60
#define FRAMESKIP_MAX           	DEFAULT_REFRESH_RATE / 2
#define DEFAULT_SOUNDFONT_FILENAME	"Roland_SC-55.sf2"

// Audio status
#define AUDIO_STATUS_MUTE               (1 << 0)
#define AUDIO_STATUS_BUFFER_SUPPORT     (1 << 1)
#define AUDIO_STATUS_BUFFER_ACTIVE      (1 << 2)
#define AUDIO_STATUS_BUFFER_UNDERRUN    (1 << 3)
#define AUDIO_STATUS_UPDATE_LATENCY     (1 << 4)
#define AUDIO_STATUS_UPDATE_AV_INFO	(1 << 5)

// Performance switcher
#define PERF_SWITCH_FRAMESKIP_EVENTS                    DEFAULT_REFRESH_RATE / 2
#define PERF_SWITCH_ON                                  (1 << 0)
#define PERF_SWITCH_ENABLE_TIMING_INACCURACIES          (1 << 1)
#define PERF_SWITCH_OVER                                (1 << 2)
#define PERF_SWITCH_RESET_THRESHOLD                     60
#define PERF_SWITCH_RESET_REST                          DEFAULT_REFRESH_RATE * 30

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

#ifndef F_OK
#define F_OK 0
#endif

#ifndef W_OK
#define W_OK 2
#endif

#ifndef R_OK
#define R_OK 4
#endif

#endif
