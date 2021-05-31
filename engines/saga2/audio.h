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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_AUDIO_H
#define SAGA2_AUDIO_H

/* ===================================================================== *
   the library(s) must be recompiled if you change these settings
 * ===================================================================== */

#define DEBUG_AUDIO     2
#define STATUS_MESSAGES DEBUG
#define QUEUES_EXTERNAL_ALLOCATION 1
#define USE_REAL_WAIL 1

// TODO: FIXME. STUB
typedef int HDIGDRIVER;
typedef int HTIMER;
typedef int HMDIDRIVER;
typedef int HSAMPLE;
typedef int HSEQUENCE;

// AIL stubs
#define SMP_DONE          0x0002
#define SMP_STOPPED       0x0008

void  AIL_init_sample(HSAMPLE S);
HSAMPLE AIL_allocate_sample_handle(HDIGDRIVER dig);
void AIL_release_sample_handle(HSAMPLE S);
HSEQUENCE AIL_allocate_sequence_handle(HMDIDRIVER mdi);
void AIL_end_sample(HSAMPLE S);
void AIL_set_sample_type(HSAMPLE S, int32 format, uint32 flags);
uint32 AIL_sample_status(HSAMPLE S);
void AIL_set_sample_address(HSAMPLE S, void *start, uint32 len);
int32 AIL_sample_buffer_ready(HSAMPLE S);
void AIL_end_sequence(HSEQUENCE S);
void AIL_load_sample_buffer(HSAMPLE S, uint32 buff_num, void *buffer, uint32 len);
void AIL_start_sample(HSAMPLE S);
uint32 AIL_sequence_status(HSEQUENCE S);
void AIL_set_sample_loop_count (HSAMPLE S, int32 loop_count);
void AIL_set_sequence_loop_count(HSEQUENCE S, int32 loop_count);
void  AIL_start_sequence(HSEQUENCE S);
void AIL_set_sample_volume(HSAMPLE S, int32 volume);
void AIL_set_sample_playback_rate(HSAMPLE S, int32 playback_rate);
void AIL_lock(void);
void AIL_unlock(void);
void AIL_release_sequence_handle(HSEQUENCE S);
int32 AIL_init_sequence(HSEQUENCE S, void *start, int32 sequence_num);
int32 AIL_sample_volume(HSAMPLE S);
int32 AIL_sequence_volume(HSEQUENCE S);
void AIL_set_sequence_volume(HSEQUENCE S, int32 volume, int32 milliseconds);

inline void audioFatal(char *msg) {
	error("Sound error %s", msg);
}

#endif
