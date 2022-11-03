/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/gfx/gfxstub.h"
#include "engines/icb/gfx/psx_scrn.h"

namespace ICB {

int32 _selFace;

// Number of GPU packets to reserve
#define PACKETMAX 2900 * 4
// the size of a POLY_GT3 packet
#define PACKETSIZE sizeof(POLY_GT3)
// a full-up DR_LOAD is 17 int32s (68 bytes) and is the biggest type of packet
#define PACKET_MAX_SIZE sizeof(DR_LOAD)
#define PACKETMEM (PACKETMAX * PACKETSIZE)

// Global place to create GPU packets
// Use a ring system, so when get to end just start again from the beginning
GPUPACKET packets[PACKETMEM]; /* GPU PACKETS AREA */
GPUPACKET *drawpacket;
GPUPACKET *drawpacketStart;
GPUPACKET *drawpacketEnd;

// Global double buffer index
int32 drawBuf = 0;

// Global ot arrays and stuff
OT_tag *otarray[2];
OT_tag *drawot;
OT_tag otlist[2][OT_SIZE];

// The min & max places to put Z data into the OT list
int32 minZOTpos = 5;
int32 maxZOTpos = OT_SIZE - 5;
int32 nearClip = 0;
int32 minUsedZpos = 20000;
int32 maxUsedZpos = 0;

int32 g_otz_shift = 0; // 1cm accuracy
int32 g_otz_offset;

// The zones for otz_shift computation
#define OTZ_ZONE1 32000  // 1cm
#define OTZ_ZONE2 64000  // 2cm
#define OTZ_ZONE4 128000 // 4cm
#define OTZ_ZONE8 256000 // 8cm

void InitDrawing(void) {
	// sort out the GPU packet memory
	drawpacketStart = packets;
	drawpacketEnd = packets + PACKETMEM - PACKET_MAX_SIZE * 2;
	drawpacket = drawpacketStart;

	otarray[0] = &(otlist[0][0]);
	otarray[1] = &(otlist[1][0]);
	drawBuf = 0;
	drawot = otarray[drawBuf];

	CLEAROTLIST(drawot, OT_SIZE);
	minZOTpos = 5;
	maxZOTpos = OT_SIZE - 5;
	nearClip = 0;
	minUsedZpos = 20000;
	maxUsedZpos = 0;

	g_otz_shift = 0; // 1cm accuracy
	g_otz_offset = ((nearClip >> g_otz_shift) - minZOTpos);
}

void drawOTList(void) {
	startDrawing();
	DrawOTag(drawot + OT_FIRST);
	endDrawing();
	CLEAROTLIST(drawot, OT_SIZE);
}

void recoverFromOTcrash(void) {
	endDrawing();
	CLEAROTLIST(drawot, OT_SIZE);
}

void ResetZRange(void) {
	// Reset the z-sorting values
	nearClip = minUsedZpos - 100; // last used value -1m
	int32 dz = maxUsedZpos - nearClip;
	// Simple zones for otz_shift
	if (dz < OTZ_ZONE1) // 1cm
		g_otz_shift = 0;
	else if (dz < OTZ_ZONE2) // 2cm
		g_otz_shift = 1;
	else if (dz < OTZ_ZONE4) // 4cm
		g_otz_shift = 2;
	else if (dz < OTZ_ZONE8) // 8cm
		g_otz_shift = 3;
	else
		g_otz_shift = 4; // 16cm - yuck

	g_otz_offset = ((nearClip >> g_otz_shift) - minZOTpos);

	minUsedZpos = 20000;
	maxUsedZpos = 0;
}

} // End of namespace ICB
