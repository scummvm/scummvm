/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ICB_PSX_SCRN_H
#define ICB_PSX_SCRN_H

#include "engines/icb/gfx/psx_pcgpu.h"
#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/common/px_assert.h"
#include "engines/icb/common/px_maths.h"
#include "engines/icb/gfx/psx_ot.h"

namespace ICB {

typedef struct DoubleBuffer {
	DRAWENV draw; /* drawing environment */
	DISPENV disp; /* display environment */
} DoubleBuffer;

#define SCREEN_W 640
#define SCREEN_H 480

#define MIN_SCREEN_X (0)
#define MAX_SCREEN_X (SCREEN_W - 1)
#define MIN_SCREEN_Y (0)
#define MAX_SCREEN_Y (SCREEN_H - 1)
#define MSG_X 20
#define MSG_Y (SCREEN_H - 40)

#define MSG_W SCREEN_W
#define MSG_H 10

// Length of the OT
#define OT_LENGTH 16 // bit length of OT

#define OT_SIZE (1 << OT_LENGTH)

// The PSX Data-cache which is 1KB big
#define D_CACHE getScratchAddr(0)
#define D_CACHE_SIZE 1024

// Number of DR_LOAD primitives to allocate
// #define MAX_DRLOADS 2048

// Screen position for 0,0
extern int scrn_ox;
extern int scrn_oy;

// Global double buffer and drawing functions
void SwapDoubleBuffer(void);
void SwapBufferIndex(void);
void DrawDisplayList(int buf);
void Init_display(int w, int h, int x1, int y1, int x2, int y2, int ox, int oy, int clear);
void RenderCycle(void);

// Global double buffer and drawing variables

extern OT_tag *otarray[2];
extern OT_tag *drawot;

extern int drawBuf;
extern int dont_set_dispbuf;
extern DoubleBuffer db[2];
extern DoubleBuffer *pdb;
extern u_int reloadFont;
extern int global_use_otlist;

// A global array place in which to create GPU packets
typedef u_char GPUPACKET;
extern GPUPACKET *drawpacket;
extern GPUPACKET *drawpacketStart;
extern GPUPACKET *drawpacketEnd;

// For tracking the maximum number of packets used
extern int globalPacketMax;
extern int packetsUsed;

// How much to shift & then offset the z values from gte to
// put them into the otlist
extern int otz_shift;
extern int otz_offset;

// Enable/disable updating of the auto-sliding & scaling min,max z position
extern int update_minmaxzpos;

// Global graphics options for z-clipping and camera scalings
extern int minZOTpos;
extern int maxZOTpos;
extern int minUsedZpos;
extern int maxUsedZpos;
extern int nearClip;
extern int delayValue;
extern int scale[3];
extern int zscale;

#if (_PSX_ON_PC == 0) && (_PSX == 1)

// Number of GPU packets to reserve
#define PACKETMAX 1500
// the size of a POLY_GT3 packet
#define PACKETSIZE sizeof(POLY_GT3)
// a full-up DR_LOAD is 17 int32s (68 bytes) and is the biggest type of packet
#define PACKET_MAX_SIZE sizeof(DR_LOAD)
#define PACKETMEM (PACKETMAX * PACKETSIZE)

// this is the nick pelling speed-up bit

#define set3(_r0, _r1) __asm__ volatile("swl %1, 2( %0 )" : : "r"(_r0), "r"(_r1) : "memory")

static inline uint32 get3(void *_r0) {
	register uint32 t;
	__asm__ volatile("lwl %0, 2( %1 )" : "=r"(t) : "r"(_r0) : "memory");
	return t;
}

#ifdef setaddr
#undef setaddr
#undef getaddr
#undef addPrim
#undef addPrims
#endif

#define setaddr(_p0, _p1) set3((_p0), ((uint32)(_p1)) << 8)
#define getaddr(_p) (get3(_p) >> 8)

static inline void addPrim(uint32 *ot, void *p) {
	uint32 tmp = get3(ot); // lwl
	setaddr(ot, p);        // sll, swl
	set3(p, tmp);          // swl
}

static inline void addPrims(uint32 *ot, void *p0, void *p1) {
	uint32 tmp = get3(ot); // lwl
	setaddr(ot, p0);       // sll, swl
	set3(p1, tmp);         // swl
}

#endif // #if (_PSX_ON_PC == 0) && (_PSX==1)

// Cheers to Nick Pelling for this excellent mySetDrawLoad
// This one is a straight forward inline version of SetDrawLoad
// and adds the DR_FLUSH onto the end
static inline void mySetDrawLoad(DR_LOAD *p, RECT16 *r, int length) {
	p->code[0] = (uint32)(0xA0 << 24); // 0xA0000000
	p->code[1] = *((uint32 *)r);
	p->code[2] = *((uint32 *)r + 1);
	setlen(p, (1 + 2 + 1 + length)); // code=1 RECT=2 DR_FLUSH=1 data=length
	p->p[length] = 0x01 << 24;       // DR_FLUSH
}

// This one is just like SetDrawLoad but does not add the DR_FLUSH on the end
static inline void mySetDrawLoadNoFlush(DR_LOAD *p, RECT16 *r, int length) {
	p->code[0] = (uint32)(0xA0 << 24); // 0xA0000000
	p->code[1] = *((uint32 *)r);
	p->code[2] = *((uint32 *)r + 1);
	setlen(p, (1 + 2 + length)); // code=1 RECT=2 data=length
	p->p[length] = 0x01 << 24;   // DR_FLUSH
}

// Handy function for using the packets global array
static inline void myAddPacket(int len) {
	// Advance to next spot :
	// can still fail e.g. add a small packet then try to add a large packet
	drawpacket += len;
	if (drawpacket >= drawpacketEnd) {
		drawpacket = drawpacketStart;
	}
}

// Handy function for putting a DR_LOAD into the packets global array
static inline void myAddDRLOAD(RECT16 *r, uint32 *pot, int length) {
	mySetDrawLoad((DR_LOAD *)drawpacket, r, length);
	addPrim(pot, drawpacket);
	myAddPacket(((5 + length) << 2));
}

// Handy function for putting a DR_LOAD into the packets global array
// but without adding a DR_FLUSH onto the end
static inline void myAddDRLOADNoFlush(RECT16 *r, uint32 *pot, int length) {
	mySetDrawLoadNoFlush((DR_LOAD *)drawpacket, r, length);
	addPrim(pot, drawpacket);
	myAddPacket(((4 + length) << 2));
}

// Little to convert a z-value into an OT position
static inline int32 myMakeOTPosition(int32 z0) {
	int32 z1 = (z0 >> otz_shift) - otz_offset;

	minUsedZpos = PXmin(z0, minUsedZpos);
	maxUsedZpos = PXmax(z0, maxUsedZpos);

	z1 = PXmax(minZOTpos, z1);
	z1 = PXmin(maxZOTpos, z1);

	return z1;
}

// My own add prim function which automatically computes the correct
// index in the OT and also performs simple near,far z clipping
static inline int32 myAddPrimClip(int32 z0, void *primitive) {
	int32 otpos = myMakeOTPosition(z0);
	if (otpos == -1)
		return -1; // ignore out of clipping range

#if (_PSX_ON_PC == 1) || (_PSX == 0)
	z0 = z0 >> 2; // Divide z by 4 so it matches background units
	addPrimZUsr(drawot + otpos, primitive, z0, OTusrData);
#else
	addPrim(drawot + otpos, primitive);
#endif
	return otpos;
}

} // End of namespace ICB

#endif // #ifndef PSX_SCRN_H
