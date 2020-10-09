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

#include "engines/icb/common/px_common.h"
#include "engines/icb/debug.h"
#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/actor_pc.h"
#include "engines/icb/gfx/psx_scrn.h"
#include "engines/icb/gfx/psx_pchmd.h"
#include "engines/icb/gfx/psx_poly.h"
#include "engines/icb/global_objects_psx.h"
#include "engines/icb/drawpoly_pc.h"
#include "engines/icb/icb.h"
#include "engines/icb/common/px_capri_maths.h"

namespace ICB {

void DrawMuzzleFlashPC(SVECTOR *mfpos, int mfh, int mfw);
void DrawCartridgeCasePC(SVECTOR *bulletOffset, int col);
void DrawBreathingPC(Breath *breath); // test function

#if _PSX_ON_PC == 1

extern int minX, maxX, minY, maxY, minZ, maxZ;

#else // #if _PSX_ON_PC == 1

int minX, maxX, minY, maxY, minZ, maxZ;

#endif // #if _PSX_ON_PC == 1

#define VERY_BRIGHT 9000
#define NOT_VERY_BRIGHT 3000

// draw the special effects
int DrawActorSpecialEffectsPC(int mflash, SVECTOR *mfpos, int mfh, int mfw, int bullet, SVECTOR *bulletPos, int bulletCol, Breath *breath, MATRIXPC *local2screen, int brightness,
                              SVECTOR *minBBox, SVECTOR *maxBBox) {
	// Put the correct rot and trans matrix in place
	// transform model from world space to screen space
	gte_SetRotMatrix_pc(local2screen);
	gte_SetTransMatrix_pc(local2screen);

	minX = SCREEN_WIDTH / 2;
	maxX = -SCREEN_WIDTH / 2;
	minY = SCREEN_DEPTH / 2;
	maxY = -SCREEN_DEPTH / 2;
	minZ = 0x7fff;
	maxZ = 0;

	// Draw muzzle flash if need be
	if (mflash) {
		DrawMuzzleFlashPC(mfpos, mfh, mfw);
	}

	if (bullet) {
		int col;
		int b;

		// get difference between actual brightness and the min level
		b = brightness - NOT_VERY_BRIGHT;

		// limit it, no point having negative, anything below min is clamped
		if (b < 0)
			b = 0;
		// also limit at top range, anything above is clamped to full
		else if (b > (VERY_BRIGHT - NOT_VERY_BRIGHT))
			b = (VERY_BRIGHT - NOT_VERY_BRIGHT);

		// now use the brightness slider to do proportion of bullet col which has come in
		col = bulletCol * b / (VERY_BRIGHT - NOT_VERY_BRIGHT);

		DrawCartridgeCasePC(bulletPos, bulletCol);
	}

	if ((breath) && (breath->on)) {
		DrawBreathingPC(breath);
	}

	if ((minBBox) && (maxBBox)) {
		minBBox->vx = (short)minX;
		minBBox->vy = (short)minY;
		minBBox->vz = (short)minZ;

		maxBBox->vx = (short)maxX;
		maxBBox->vy = (short)maxY;
		maxBBox->vz = (short)maxZ;
	}

	if ((minX < maxX) && (minY < maxY))
		return 1;

	return 0;
}

#define CIRCLE_SEGMENTS 12

void DrawBreathParticlePC(short x, short y, int z, uint8 col, short w, short h, int *rands) {
	int angle = 0;
	int da = 4096 / CIRCLE_SEGMENTS;
	int cx, cy;
	int nx, ny;
	int i;
	TPOLY_G3 *poly;

#define getCircleX(ang) ((int)(PXcos((float)ang / 4096.0f) * w))
#define getCircleY(ang) ((int)(PXsin((float)ang / 4096.0f) * h))

	// get top point
	nx = (int)getCircleX(angle);
	ny = (int)getCircleY(angle);

	int randPointer = (x & 7); // 0-7

	// for every other point
	for (i = 0; i < CIRCLE_SEGMENTS; i++) {
		cx = nx;
		cy = ny;

		angle = (angle + da) & 4095;

		nx = (int)(getCircleX(angle) + rands[randPointer]);
		randPointer = (randPointer + 1) & 7;

		ny = (int)(getCircleY(angle) + rands[randPointer]);
		randPointer = (randPointer + 1) & 7;

		poly = (TPOLY_G3 *)drawpacket;
		setTPolyG3(poly);
		setTSemiTrans(poly, 1);
		setTABRMode(poly, 1);

		setXY3(poly, (short)x, (short)y, (short)(cx + x), (short)(cy + y), (short)(nx + x), (short)(ny + y));

		setRGB0(poly, col, col, col);
		setRGB1(poly, col, col, col);
		setRGB2(poly, col, col, col);

		myAddPacket(sizeof(TPOLY_G3));
		myAddPrimClip(z, poly);
	}
}

#define MAX_DRAW_BREATH 4
#define MAX_DRAW_SMOKE 8

// breathing test....
void DrawBreathingPC(Breath *breath) {
	int i;
	uint8 col;
	// to get z value etc
	SVECTORPC local;
	// unused
	SVECTORPC out;
	int32 p, flag;
	int32 z0;
	int32 size;
	int max;

	// set maximum number of polys...
	if (breath->on == BREATH_SMOKE)
		max = MAX_DRAW_SMOKE;
	else
		max = MAX_DRAW_BREATH;

	int rands[8];

	for (i = 0; i < 8; i++)
		rands[i] = g_icb->getRandomSource()->getRandomNumber(5 - 1) - 2;

	// draw the polys
	for (i = 0; i < max; i++) {
		if ((int)(breath->breathColour[i]) > 0) {

			local.vx = breath->position.vx;
			local.vy = (short)(breath->position.vy + breath->breathY[i]);
			local.vz = (short)(breath->position.vz + breath->breathZ[i]);

			gte_RotTransPers_pc(&local, &out, &p, &flag, &z0);

			size = (128 * breath->breathSize[i]) / z0;
			if (size) {

				short x, y;

				x = (short)(out.vx);
				y = (short)(out.vy);

				col = (uint8)(2 * breath->breathColour[i]);

				short w, h;

				w = (short)((size * 3) >> 1);
				h = (short)size;

				DrawBreathParticlePC(x, y, z0, col, w, h, rands);

				// check bounding box...
				if (x < minX)
					minX = x;

				if (y < minY)
					minY = y;

				if (y + w > maxX)
					maxX = (short)(x + w);

				if (y + h > maxY)
					maxY = (short)(y + h);

				if (z0 < minZ)
					minZ = (short)z0;

				if (z0 > maxZ)
					maxZ = (short)z0;
			}
		}
	}
}

// number of points (8 for cube) and number of faces (6 for cube)...
#define BULLET_POINTS 8
#define BULLET_QUADS 6

// dimensions of bullet
#define BULLET_LENGTH 4
#define BULLET_RADIUS 1

// 8 points on cubic shape, length by 2*radius (width) by 2*radius (height)...
const short bulletOffsets[BULLET_POINTS][3] = {{-BULLET_RADIUS, BULLET_RADIUS, 0},
                                               {BULLET_RADIUS, BULLET_RADIUS, 0},
                                               {BULLET_RADIUS, -BULLET_RADIUS, 0},
                                               {-BULLET_RADIUS, -BULLET_RADIUS, 0},
                                               {-BULLET_RADIUS, BULLET_RADIUS, BULLET_LENGTH},
                                               {BULLET_RADIUS, BULLET_RADIUS, BULLET_LENGTH},
                                               {BULLET_RADIUS, -BULLET_RADIUS, BULLET_LENGTH},
                                               {-BULLET_RADIUS, -BULLET_RADIUS, BULLET_LENGTH}};

// 6 sides for cube....
const int bulletQuads[BULLET_QUADS][4] = {{1, 0, 2, 3}, {0, 1, 4, 5}, {1, 2, 5, 6}, {2, 3, 6, 7}, {3, 0, 7, 4}, {4, 5, 7, 6}};

#define BULLET_RED_1 255
#define BULLET_GREEN_1 255
#define BULLET_BLUE_1 255

#define BULLET_RED_2 100
#define BULLET_GREEN_2 100
#define BULLET_BLUE_2 100

void DrawCartridgeCasePC(SVECTOR *bulletOffset, int col) {
	SVECTORPC sxy[BULLET_POINTS];
	int32 p, flag, z = 0;
	SVECTORPC pos;

	int i;

	for (i = 0; i < BULLET_POINTS; i++) {
		pos.vx = (short)(bulletOffset->vx + bulletOffsets[i][0]);
		pos.vy = (short)(bulletOffset->vy + bulletOffsets[i][1]);
		pos.vz = (short)(bulletOffset->vz + bulletOffsets[i][2]);

		gte_RotTransPers_pc(&pos, &sxy[i], &p, &flag, &z);
	}

	// draw polygon...
	for (i = 0; i < BULLET_QUADS; i++) {
		POLY_G4 *poly;
		poly = (POLY_G4 *)drawpacket;

		setPolyG4(poly);

		poly->x0 = sxy[bulletQuads[i][0]].vx;
		poly->y0 = sxy[bulletQuads[i][0]].vy;
		poly->x1 = sxy[bulletQuads[i][1]].vx;
		poly->y1 = sxy[bulletQuads[i][1]].vy;
		poly->x2 = sxy[bulletQuads[i][2]].vx;
		poly->y2 = sxy[bulletQuads[i][2]].vy;
		poly->x3 = sxy[bulletQuads[i][3]].vx;
		poly->y3 = sxy[bulletQuads[i][3]].vy;

		// check bounding box...

		// minX

		if (poly->x0 < minX)
			minX = poly->x0;

		if (poly->x1 < minX)
			minX = poly->x1;

		if (poly->x2 < minX)
			minX = poly->x2;

		if (poly->x3 < minX)
			minX = poly->x3;

		// minY

		if (poly->y0 < minY)
			minY = poly->y0;

		if (poly->y1 < minY)
			minY = poly->y1;

		if (poly->y2 < minY)
			minY = poly->y2;

		if (poly->y3 < minY)
			minY = poly->y3;

		// minX

		if (poly->x0 > maxX)
			maxX = poly->x0;

		if (poly->x1 > maxX)
			maxX = poly->x1;

		if (poly->x2 > maxX)
			maxX = poly->x2;

		if (poly->x3 > maxX)
			maxX = poly->x3;

		// minY

		if (poly->y0 > maxY)
			maxY = poly->y0;

		if (poly->y1 > maxY)
			maxY = poly->y1;

		if (poly->y2 > maxY)
			maxY = poly->y2;

		if (poly->y3 > maxY)
			maxY = poly->y3;

		// z

		if (z < minZ)
			minZ = (short)z;

		if (z > maxZ)
			maxZ = (short)z;

		// set colours (remember dark at back and light at front

		if (bulletQuads[i][0] < 4)
			setRGB0(poly, (uint8)((col * BULLET_RED_1) / 256), (uint8)((col * BULLET_GREEN_1) / 256), (uint8)((col * BULLET_BLUE_1) / 256));
		else
			setRGB0(poly, (uint8)((col * BULLET_RED_2) / 256), (uint8)((col * BULLET_GREEN_2) / 256), (uint8)((col * BULLET_BLUE_2) / 256));

		if (bulletQuads[i][1] < 4)
			setRGB1(poly, (uint8)((col * BULLET_RED_1) / 256), (uint8)((col * BULLET_GREEN_1) / 256), (uint8)((col * BULLET_BLUE_1) / 256));
		else
			setRGB1(poly, (uint8)((col * BULLET_RED_2) / 256), (uint8)((col * BULLET_GREEN_2) / 256), (uint8)((col * BULLET_BLUE_2) / 256));

		if (bulletQuads[i][2] < 4)
			setRGB2(poly, (uint8)((col * BULLET_RED_1) / 256), (uint8)((col * BULLET_GREEN_1) / 256), (uint8)((col * BULLET_BLUE_1) / 256));
		else
			setRGB2(poly, (uint8)((col * BULLET_RED_2) / 256), (uint8)((col * BULLET_GREEN_2) / 256), (uint8)((col * BULLET_BLUE_2) / 256));

		if (bulletQuads[i][3] < 4)
			setRGB3(poly, (uint8)((col * BULLET_RED_1) / 256), (uint8)((col * BULLET_GREEN_1) / 256), (uint8)((col * BULLET_BLUE_1) / 256));
		else
			setRGB3(poly, (uint8)((col * BULLET_RED_2) / 256), (uint8)((col * BULLET_GREEN_2) / 256), (uint8)((col * BULLET_BLUE_2) / 256));

		// draw it

		myAddPrimClip(z, poly);
		myAddPacket(sizeof(POLY_G4));
	}
}

#define MUZZLE_COLOUR_0 0x201008
#define MUZZLE_COLOUR_1 0xc08020

#define setRGB0Colour(prim, col) setRGB0(prim, (col >> 16) & 255, (col >> 8) & 255, col & 255)
#define setRGB1Colour(prim, col) setRGB1(prim, (col >> 16) & 255, (col >> 8) & 255, col & 255)
#define setRGB2Colour(prim, col) setRGB2(prim, (col >> 16) & 255, (col >> 8) & 255, col & 255)

void DrawMuzzleFlashPC(SVECTOR *mfpos, int mfh, int mfw) {
	// X left of character
	// Y upwards
	// Z forwards
	int h = mfh;
	int w = mfw;
	int mz = 20;
	int32 p, flag, z;
	SVECTORPC origin;
	SVECTORPC points[18];
	SVECTORPC sxy0;
	int32 z0;
	SVECTORPC spoints[18];
	int i;

	// The centre point
	short mfx = mfpos->vx;
	short mfy = mfpos->vy;
	short mfz = mfpos->vz;
	origin.vx = mfx;
	origin.vy = mfy;
	origin.vz = mfz;

	gte_RotTransPers_pc(&origin, &sxy0, &p, &flag, &z0);

	i = 0;

	// slightly out... (OUT MORE FOR TRIS)
	points[i].vx = (short)(mfx + 0);
	points[i].vy = (short)(mfy + w);
	points[i].vz = (short)(mfz + 2 * mz);

	// far...
	i++;
	points[i].vx = (short)(mfx + 0);
	points[i].vy = (short)(mfy + 0);
	points[i].vz = (short)(mfz + 2 * mz);

	// slightly out... (OUT MORE FOR TRIS)
	i++;
	points[i].vx = (short)(mfx + 0);
	points[i].vy = (short)(mfy - w);
	points[i].vz = (short)(mfz + 2 * mz);

	// slightly out... (OUT MORE FOR TRIS)
	i++;
	points[i].vx = (short)(mfx + w);
	points[i].vy = (short)(mfy + 0);
	points[i].vz = (short)(mfz + 2 * mz);

	// far...
	i++;
	points[i].vx = (short)(mfx + 0);
	points[i].vy = (short)(mfy + 0);
	points[i].vz = (short)(mfz + 2 * mz);

	// slightly out... (OUT MORE FOR TRIS)
	i++;
	points[i].vx = (short)(mfx - w);
	points[i].vy = (short)(mfy + 0);
	points[i].vz = (short)(mfz + 2 * mz);

	// The top-left corner
	i++;
	points[i].vx = (short)(mfx - w);
	points[i].vy = (short)(mfy + h);
	points[i].vz = (short)(mfz + mz);

	// The top-middle corner
	i++;
	points[i].vx = (short)(mfx + 0);
	points[i].vy = (short)(mfy + h + w);
	points[i].vz = (short)(mfz + mz);

	// The top-right corner
	i++;
	points[i].vx = (short)(mfx + w);
	points[i].vy = (short)(mfy + h);
	points[i].vz = (short)(mfz + mz);

	// The left-bottom corner
	i++;
	points[i].vx = (short)(mfx - h);
	points[i].vy = (short)(mfy - w);
	points[i].vz = (short)(mfz + mz);

	// The left-middle corner
	i++;
	points[i].vx = (short)(mfx - h - w);
	points[i].vy = (short)(mfy + 0);
	points[i].vz = (short)(mfz + mz);

	// The left-top corner
	i++;
	points[i].vx = (short)(mfx - h);
	points[i].vy = (short)(mfy + w);
	points[i].vz = (short)(mfz + mz);

	// The bottom-left corner
	i++;
	points[i].vx = (short)(mfx - w);
	points[i].vy = (short)(mfy - h);
	points[i].vz = (short)(mfz + mz);

	// The bottom-middle corner
	i++;
	points[i].vx = (short)(mfx + 0);
	points[i].vy = (short)(mfy - h - w);
	points[i].vz = (short)(mfz + mz);

	// The bottom-right corner
	i++;
	points[i].vx = (short)(mfx + w);
	points[i].vy = (short)(mfy - h);
	points[i].vz = (short)(mfz + mz);

	// The right-top corner
	i++;
	points[i].vx = (short)(mfx + h);
	points[i].vy = (short)(mfy + w);
	points[i].vz = (short)(mfz + mz);

	// The right-middle corner
	i++;
	points[i].vx = (short)(mfx + h + w);
	points[i].vy = (short)(mfy - 0);
	points[i].vz = (short)(mfz + mz);

	// The right-bottom corner
	i++;
	points[i].vx = (short)(mfx + h);
	points[i].vy = (short)(mfy - w);
	points[i].vz = (short)(mfz + mz);

	SVECTORPC *ppoints = points;
	SVECTORPC *pspoints = spoints;
	for (i = 0; i < 18; i++) {
		gte_RotTransPers_pc(ppoints, pspoints, &p, &flag, &z);
		ppoints++;
		pspoints++;
	}

	TPOLY_G3 *poly;

	// Draw the four polygons
	int j = 0;
	for (i = 0; i < 6; i++) {
		poly = (TPOLY_G3 *)drawpacket;

		setTPolyG3(poly);
		setTABRMode(poly, 1);
		setTSemiTrans(poly, 1);

		SVECTORPC sxy1 = spoints[j];
		SVECTORPC sxy2 = spoints[j + 1];
		SVECTORPC sxy3 = spoints[j + 2];
		j += 3;

		poly->x0 = sxy0.vx;
		poly->y0 = sxy0.vy;
		setRGB0Colour(poly, MUZZLE_COLOUR_1);

		gte_NormalClip_pc(&sxy1, &sxy2, &sxy0, &flag);

		// Back facing
		if (flag < 0) {
			poly->x1 = sxy3.vx;
			poly->y1 = sxy3.vy;
			poly->x2 = sxy1.vx;
			poly->y2 = sxy1.vy;
			// *(int32*)&(poly->x3) = sxy3;
			setRGB1Colour(poly, MUZZLE_COLOUR_0);
			setRGB2Colour(poly, MUZZLE_COLOUR_0);
		} else {
			poly->x1 = sxy1.vx;
			poly->y1 = sxy1.vy;
			poly->x2 = sxy3.vx;
			poly->y2 = sxy3.vy;
			// *(int32*)&(poly->x3) = sxy3;
			setRGB1Colour(poly, MUZZLE_COLOUR_0);
			setRGB2Colour(poly, MUZZLE_COLOUR_0);
		}
		// check bounding box...

		// minX

		if (poly->x0 < minX)
			minX = poly->x0;

		if (poly->x1 < minX)
			minX = poly->x1;

		if (poly->x2 < minX)
			minX = poly->x2;

		// minY

		if (poly->y0 < minY)
			minY = poly->y0;

		if (poly->y1 < minY)
			minY = poly->y1;

		if (poly->y2 < minY)
			minY = poly->y2;

		// minX

		if (poly->x0 > maxX)
			maxX = poly->x0;

		if (poly->x1 > maxX)
			maxX = poly->x1;

		if (poly->x2 > maxX)
			maxX = poly->x2;

		// minY

		if (poly->y0 > maxY)
			maxY = poly->y0;

		if (poly->y1 > maxY)
			maxY = poly->y1;

		if (poly->y2 > maxY)
			maxY = poly->y2;

		// z

		if (z0 < minZ)
			minZ = (short)z0;

		if (z0 > maxZ)
			maxZ = (short)z0;

		// draw it

		myAddPrimClip(z0, poly);

		myAddPacket(sizeof(TPOLY_G3));
	}
}

} // End of namespace ICB
