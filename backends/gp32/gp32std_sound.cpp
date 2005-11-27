/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
 * Copyright (C) 2002 Ph0x - GP32 Backend
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
 * Copyright (C) 2005 Won Star - GP32 Backend
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
 * $Header$
 *
 */

#include "stdafx.h"
#include "common/scummsys.h"

#include "gp32std.h"
#include "gp32std_sound.h"

#define GP32_TIMER_AUDIO_IDX 0

// Global variables
static volatile unsigned int frame = 0;
static volatile unsigned int *soundPos = 0;
static volatile int idx_buf;
static volatile unsigned int shiftVal = 0;
static void *buffer;
static GPSOUNDBUF soundBuf;

// This routine gets called by the timer interrupt and
// polls the current playing position within the buffer.

static void soundTimer() {
	unsigned int t = (((unsigned int)(*soundPos) - (unsigned int)buffer) >> shiftVal) >= soundBuf.samples ? 1 : 0;
	if (t != frame) {
		unsigned int offs = ((frame == 1) ? (soundBuf.samples << shiftVal) : 0);
		soundBuf.callback(soundBuf.userdata, (uint8 *)((unsigned int)buffer + offs), soundBuf.samples << shiftVal);
		frame = t;
	}
}

int gp_soundBufStart(GPSOUNDBUF *sb) {
	int bufferSize = 0;
	frame = 0;

	// Copy the structure
	memcpy(&soundBuf, sb, sizeof(GPSOUNDBUF));

	// Calculate size of a single sample in bytes
	// and a corresponding shift value
	shiftVal = 0;
	switch (soundBuf.freq) {
	case PCM_S11:
		break;
	case PCM_S22:
		break;
	case PCM_S44:
		shiftVal++;
		break;
	case PCM_M11:
		break;
	case PCM_M22:
		break;
	case PCM_M44:
		shiftVal++;
		break;
	}
	if (soundBuf.format == PCM_16BIT)
		shiftVal++;
	soundBuf.samplesize = 1 << shiftVal;

	// Allocate memory for the playing buffer
	bufferSize = soundBuf.samplesize * soundBuf.samples * 2;
	buffer = malloc(bufferSize);
	memset(buffer, 0, bufferSize);

	// Set timer interrupt
	if (GpTimerOptSet(GP32_TIMER_AUDIO_IDX, soundBuf.pollfreq, 0, soundTimer) == GPOS_ERR_ALREADY_USED) {
		NP(" Timer is already used... kill timer");
		GpTimerKill(GP32_TIMER_AUDIO_IDX);
	}
	GpTimerSet(GP32_TIMER_AUDIO_IDX);
	// Start playing
    GpPcmInit(soundBuf.freq, soundBuf.format);
	GpPcmPlay((unsigned short *)buffer, bufferSize, 1);
	GpPcmLock((unsigned short *)buffer, (int *)&idx_buf, (unsigned int *)&soundPos);

	return 0;
}

void gp_soundBufStop() {
	GpPcmStop();
	GpPcmRemove((unsigned short *)buffer);
	GpTimerKill(GP32_TIMER_AUDIO_IDX);
	free(buffer);
}
