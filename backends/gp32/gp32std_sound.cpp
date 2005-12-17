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
	unsigned int sampleshiftVal = soundBuf.samples << shiftVal;
	unsigned int t = (((unsigned int)(*soundPos) - (unsigned int)buffer) >> shiftVal) >= soundBuf.samples ? 1 : 0;
	if (t != frame) {
		unsigned int offs = ((frame == 1) ? (sampleshiftVal) : 0);
		//memset((uint8 *)buffer + offs, 0, sampleshiftVal);
		soundBuf.callback(soundBuf.userdata, (uint8 *)((unsigned int)buffer + offs), sampleshiftVal);
		frame = t;
		{
			// Play silence
			register uint16 *d = (uint16 *)((uint8 *)buffer + offs); // alignment-safe
			register uint32 max = (uint32)((uint8 *)buffer + offs + sampleshiftVal);
			do {
				*d++ ^= 0x8000; // 1
				*d++ ^= 0x8000; // 2
				*d++ ^= 0x8000; // 3
				*d++ ^= 0x8000; // 4
				*d++ ^= 0x8000; // 5
				*d++ ^= 0x8000; // 6
				*d++ ^= 0x8000; // 7
				*d++ ^= 0x8000; // 8
				*d++ ^= 0x8000; // 9
				*d++ ^= 0x8000; // 10
				*d++ ^= 0x8000; // 11
				*d++ ^= 0x8000; // 12
				*d++ ^= 0x8000; // 13
				*d++ ^= 0x8000; // 14
				*d++ ^= 0x8000; // 15
				*d++ ^= 0x8000; // 16
				*d++ ^= 0x8000; // 17
				*d++ ^= 0x8000; // 18
				*d++ ^= 0x8000; // 19
				*d++ ^= 0x8000; // 20
				*d++ ^= 0x8000; // 21
				*d++ ^= 0x8000; // 22
				*d++ ^= 0x8000; // 23
				*d++ ^= 0x8000; // 24
				*d++ ^= 0x8000; // 25
				*d++ ^= 0x8000; // 26
				*d++ ^= 0x8000; // 27
				*d++ ^= 0x8000; // 28
				*d++ ^= 0x8000; // 29
				*d++ ^= 0x8000; // 30
				*d++ ^= 0x8000; // 31
				*d++ ^= 0x8000; // 32
			} while ((uint32)d < max);
		}
	}
}

int gp_soundBufStart(GPSOUNDBUF *sb) {
	int bufferSize = 0;

	PCM_SR gpFreq = PCM_S11;
	PCM_BIT gpFormat = PCM_16BIT;

	frame = 0;

	// Copy the structure
	memcpy(&soundBuf, sb, sizeof(GPSOUNDBUF));

	// Calculate size of a single sample in bytes
	// and a corresponding shift value
	shiftVal = 0;

	switch (soundBuf.format) {
	case 8:
		gpFormat = PCM_8BIT;
		break;
	case 16:
		gpFormat = PCM_16BIT;
		shiftVal++;
		break;
	}

	switch (soundBuf.freq) {
	case 11025:
		if (soundBuf.channels == 2) {
			gpFreq = PCM_S11;
			shiftVal++;
		} else
			gpFreq = PCM_M11;
		break;
	case 22050:
		if (soundBuf.channels == 2) {
			gpFreq = PCM_S22;
			shiftVal++;
		} else
			gpFreq = PCM_M22;
		break;
	case 44100:
		if (soundBuf.channels == 2) {
			gpFreq = PCM_S44;
			shiftVal++;
		} else
			gpFreq = PCM_M44;
		break;
	}

	soundBuf.samplesize = 1 << shiftVal;
	
	// Allocate memory for the playing buffer
	bufferSize = soundBuf.samplesize * soundBuf.samples * 2;
	buffer = malloc(bufferSize);

	// Clear the buffer
	uint16 *tmpBuf = (uint16 *)buffer;
	for (int i = 0; i < bufferSize / 2; i++)
		tmpBuf[i] = 0x8000;

	// Frequency of the timer interrupt which polls the playing position
	soundBuf.pollfreq = 4 * (2 * soundBuf.freq) / soundBuf.samples;

	// Set timer interrupt
	if (GpTimerOptSet(GP32_TIMER_AUDIO_IDX, soundBuf.pollfreq, 0, soundTimer) == GPOS_ERR_ALREADY_USED) {
		GPDEBUG(" Timer is already used... kill timer");
		GpTimerKill(GP32_TIMER_AUDIO_IDX);
	}
	GpTimerSet(GP32_TIMER_AUDIO_IDX);

	GpPcmInit(gpFreq, gpFormat);
	GpPcmPlay((unsigned short *)buffer, bufferSize, 1);
	GpPcmLock((unsigned short *)buffer, (int *)&idx_buf, (unsigned int *)&soundPos);

	return 0;
}

void gp_soundBufStop() {
	GpTimerKill(GP32_TIMER_AUDIO_IDX);
	GpPcmStop();
	GpPcmRemove((unsigned short *)buffer);
	free(buffer);
}
