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
#define AUDIO_STATUS_UPDATE_AV_INFO	    (1 << 5)
#define AUDIO_STATUS_RESET_PENDING	    (1 << 6)
#define AUDIO_STATUS_UPDATE_GEOMETRY    (1 << 7)

// Video status
#define VIDEO_GRAPHIC_MODE_REQUEST_SW     (1 << 0)
#define VIDEO_GRAPHIC_MODE_REQUEST_HW     (1 << 1)
#define VIDEO_GRAPHIC_MODE_HAVE_OPENGL    (1 << 2)
#define VIDEO_GRAPHIC_MODE_HAVE_OPENGLES2 (1 << 3)
#define VIDEO_GRAPHIC_MODE_RESET_PENDING  (1 << 4)

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
