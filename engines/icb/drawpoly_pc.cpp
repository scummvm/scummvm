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
#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/gfx/psx_scrn.h"
#include "engines/icb/common/px_capri_maths.h"
#include "engines/icb/gfx/psx_poly.h"
#include "engines/icb/global_objects_psx.h"
#include "engines/icb/light_pc.h"

namespace ICB {

#if _PSX_ON_PC == 1

extern unsigned int selFace;
extern CVECTOR unlitPoly;
extern CVECTOR deadObjectColour;
extern unsigned int deadObject;

extern unsigned int _drawBface;

extern uint8 selWRed;
extern uint8 selWGreen;
extern uint8 selWBlue;

extern uint8 selPRed;
extern uint8 selPGreen;
extern uint8 selPBlue;
extern uint8 selPAlpha;

#else // #if _PSX_ON_PC == 1

unsigned int selFace = 65537;
CVECTOR unlitPoly = {255, 255, 255, 0};
CVECTOR deadObjectColour = {0, 0, 0, 0};
unsigned int deadObject = 0;

unsigned int _drawBface = 0;

uint8 selWRed = 0;
uint8 selWGreen = 255;
uint8 selWBlue = 255;

uint8 selPRed = 255;
uint8 selPGreen = 255;
uint8 selPBlue = 0;
uint8 selPAlpha = 0;

#endif // #if _PSX_ON_PC == 1

// Specialist lighting routines for polygons
// support for bounce & width
inline void LightPolygon(SVECTOR *n0, CVECTOR *rgbIn, CVECTOR *rgb0) {
	SVECTORPC lightEffect;
	// Normal line vector(local) -> light source effect
	ApplyMatrixSV_pc(&gtelight_pc, n0, &lightEffect);

	if (useLampWidth) {
		lightEffect.vx = (short)(lightEffect.vx + lampWidth[0]);
		lightEffect.vy = (short)(lightEffect.vy + lampWidth[1]);
		lightEffect.vz = (short)(lightEffect.vy + lampWidth[2]);
	}
	if (useLampBounce) {
		if (lightEffect.vx < 0) {
			lightEffect.vx = (short)-lightEffect.vx;
			if (lightEffect.vx > lampBounce[0])
				lightEffect.vx = (short)lampBounce[0];
		}
		if (lightEffect.vy < 0) {
			lightEffect.vy = (short)-lightEffect.vy;
			if (lightEffect.vy > lampBounce[1])
				lightEffect.vy = (short)lampBounce[1];
		}
		if (lightEffect.vz < 0) {
			lightEffect.vz = (short)-lightEffect.vz;
			if (lightEffect.vz > lampBounce[2])
				lightEffect.vz = (short)lampBounce[2];
		}
	} else {
		if (lightEffect.vx < 0)
			lightEffect.vx = 0;
		if (lightEffect.vy < 0)
			lightEffect.vy = 0;
		if (lightEffect.vz < 0)
			lightEffect.vz = 0;
	}

	// Light source effect -> Colour effect(local colour matrix+back colour)
	SVECTORPC colourEffect;
	ApplyMatrixSV_pc(&gtecolour_pc, &lightEffect, &colourEffect);
	if (colourEffect.vx < 0)
		colourEffect.vx = 0;
	if (colourEffect.vy < 0)
		colourEffect.vy = 0;
	if (colourEffect.vz < 0)
		colourEffect.vz = 0;

	// colourEffect is 0-4095 (2^12)
	// gteback is 0-255 (2^8)
	colourEffect.vx = (short)((colourEffect.vx >> 4) + gteback_pc[0]);
	colourEffect.vy = (short)((colourEffect.vy >> 4) + gteback_pc[1]);
	colourEffect.vz = (short)((colourEffect.vz >> 4) + gteback_pc[2]);

	// 256 = 1.0 in colourEffect
	// 128 = 1.0 in in0
	int red = (rgbIn->r * colourEffect.vx);
	int green = (rgbIn->g * colourEffect.vy);
	int blue = (rgbIn->b * colourEffect.vz);

	red = red >> 8;
	green = green >> 8;
	blue = blue >> 8;

	if (red > 255)
		red = 255;
	if (green > 255)
		green = 255;
	if (blue > 255)
		blue = 255;

	rgb0->r = (uint8)(red);
	rgb0->g = (uint8)(green);
	rgb0->b = (uint8)(blue);
}

// Specialist lighting routines for polygons
// support for bounce & width
inline void LightPolygon3(SVECTOR *n0, SVECTOR *n1, SVECTOR *n2, CVECTOR *rgbIn, CVECTOR *rgb0, CVECTOR *rgb1, CVECTOR *rgb2) {
	LightPolygon(n0, rgbIn, rgb0);
	LightPolygon(n1, rgbIn, rgb1);
	LightPolygon(n2, rgbIn, rgb2);
}

#define LIGHTPOLYGON(n0, rgbIn, rgb0)                                                                                                                                              \
	{                                                                                                                                                                          \
		if (useLampWidth || useLampBounce)                                                                                                                                 \
			LightPolygon(n0, rgbIn, rgb0);                                                                                                                             \
		else                                                                                                                                                               \
			gte_NormalColorCol_pc(n0, rgbIn, rgb0);                                                                                                                    \
	}

#define LIGHTPOLYGON3(n0, n1, n2, rgbIn, rgb0, rgb1, rgb2)                                                                                                                         \
	{                                                                                                                                                                          \
		if (useLampWidth || useLampBounce)                                                                                                                                 \
			LightPolygon3(n0, n1, n2, rgbIn, rgb0, rgb1, rgb2);                                                                                                        \
		else                                                                                                                                                               \
			gte_NormalColorCol3_pc(n0, n1, n2, rgbIn, rgb0, rgb1, rgb2);                                                                                               \
	}

//----------------------------------------------------------------

/*
gte_ll();
gte_llv0();
gte_llv1();
gte_llv2();
gte_llir();
gte_llv0tr();
gte_llv1tr();
gte_llv2tr();
gte_llirtr();
gte_llv0bk();
gte_llv1bk();
gte_llv2bk();
gte_llirbk();
gte_lc();
gte_lcv0();
gte_lcv1();
gte_lcv2();
gte_lcir();
gte_lcv0tr();
gte_lcv1tr();
gte_lcv2tr();
gte_lcirtr();
gte_lcv0bk();
gte_lcv1bk();
gte_lcv2bk();
gte_lcirbk();
*/

//------------------------------------------------------------------------

#if CD_MODE == 0

//------------------------------------------------------------------------

// Flat, Un-Textured, Self-Luminous, triangles
void drawFUS3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex) {
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	// u_int num = min( n, 10 ); // do a maximum of 10 polygons
	u_int num = n;
	int32 p, flag;
	u_int i;
	uint32 *pPoly;
	int32 z0, z1, z2;
	SVECTORPC sxy0, sxy1, sxy2, stemp;
	CVECTOR *rgbIn;
	uint32 tmp;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < num; i++) {
		// Structure is :
		// 0x20 | B | G | R
		// v1       | v0
		// pad      | v2
		rgbIn = (CVECTOR *)pPoly++;

		if (deadObject) {
			rgbIn = &deadObjectColour;
		}

		tmp = *pPoly++;
		v0 = pVertex + (tmp & 0xFFFF);
		v1 = pVertex + (tmp >> 16);

		tmp = *pPoly++;
		v2 = pVertex + (tmp & 0xFFFF);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		gte_RotTransPers_pc(v0, &sxy0, &p, &flag, &z0);
		z0 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4

		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v1, &sxy1, &p, &flag, &z1);
		z1 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v2, &sxy2, &p, &flag, &z2);
		z2 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		// Give the GTE some time to recover
		int32 avgz;
		gte_AverageZ3_pc(z0, z1, z2, &avgz);

		// Do we want to do back face removal ?
		if (_removeBackface) {
			// use winding order of the polygons
			// e.g. NormalClip which is cross product of screen vectors
			if (_useWindingOrder) {
				gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
				// Drawing back facings polys requires making them have
				// clockwise winding order as by definition they have
				// anti-clockwise winding order
				if (flag < 0) {
					if (_drawBface) {
						stemp = sxy1;
						sxy1 = sxy2;
						sxy2 = stemp;
					} else {
						// Skip this polygon
						continue;
					}
				} else if (flag == 0)
					continue; // ignore zero-area polygons
			}

			// TODO
			// dot product of viewing direction and polygon normal
			// will require generation of polygon normal from vertex normals
			// and transformation of the normal to world space
		}

		// Put wireframe triangles in list before textured ones
		// so should be in front of textured ones
		if ((_drawWfrm) || (selFace == i)) {
			LINE_F4 *line = (LINE_F4 *)drawpacket;
			setLineF4(line);
			// set XY's in the GPU packet 0-1-2-3
			line->x0 = sxy0.vx;
			line->y0 = sxy0.vy;
			line->x1 = sxy1.vx;
			line->y1 = sxy1.vy;
			line->x2 = sxy2.vx;
			line->y2 = sxy2.vy;
			line->x3 = sxy0.vx;
			line->y3 = sxy0.vy;
			if (selFace == i)
				setRGB0(line, selWRed, selWGreen, selWBlue);
			else
				setRGB0(line, (uint8)wfrmRed, (uint8)wfrmGreen, (uint8)wfrmBlue);
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(LINE_F4));
		}

		// Draw polygon version behind the wireframe
		if ((_drawPolys) || (selFace == i)) {
			// Draw untextured polygons
			POLY_F3 *poly = (POLY_F3 *)drawpacket;
			setPolyF3(poly);
			// set XY's in the GPU packet 0-1-2
			poly->x0 = sxy0.vx;
			poly->y0 = sxy0.vy;
			poly->x1 = sxy1.vx;
			poly->y1 = sxy1.vy;
			poly->x2 = sxy2.vx;
			poly->y2 = sxy2.vy;

			// Set the RGB colours
			// No light source calculation
			// So just set base colour
			if (selFace == i)
				setRGB0(poly, selPRed, selPGreen, selPBlue);
			else
				setRGB0(poly, rgbIn->r, rgbIn->g, rgbIn->b);
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(POLY_F3));
		}
	}
}

//------------------------------------------------------------------------

// Gouraud, Un-Textured, Self-Luminous, triangles
void drawGUS3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex) {
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	// u_int num = min( n, 10 ); // do a maximum of 10 polygons
	u_int num = n;
	uint32 tmp;
	int32 p, flag;
	u_int i;
	uint32 *pPoly;
	int32 z0, z1, z2;
	SVECTORPC sxy0, sxy1, sxy2, stemp;

	// Base colour for gouraud shading computation
	CVECTOR *rgb0;
	CVECTOR *rgb1;
	CVECTOR *rgb2;
	CVECTOR *rgbTemp;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < num; i++) {
		// Structure is :
		// 8-bits | 8-bits | 8-bits | 8-bits
		// 0x30   | Blue0  | Green0 | Red0
		// 0x30   | Blue1  | Green1 | Red1
		// 0x30   | Blue2  | Green2 | Red2
		// 16-bits | 8-bits | 8-bits
		// --------------------------
		//  v1              | v0
		//  pad             | v2
		rgb0 = (CVECTOR *)pPoly++;
		rgb1 = (CVECTOR *)pPoly++;
		rgb2 = (CVECTOR *)pPoly++;

		if (deadObject) {
			rgb0 = &deadObjectColour;
			rgb1 = &deadObjectColour;
			rgb2 = &deadObjectColour;
		}

		tmp = *pPoly++;
		v0 = pVertex + (tmp & 0xFFFF);
		v1 = pVertex + (tmp >> 16);

		tmp = *pPoly++;
		v2 = pVertex + (tmp & 0xFFFF);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		gte_RotTransPers_pc(v0, &sxy0, &p, &flag, &z0);
		z0 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4

		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v1, &sxy1, &p, &flag, &z1);
		z1 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v2, &sxy2, &p, &flag, &z2);
		z2 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		// Give the GTE some time to recover
		int32 avgz;
		gte_AverageZ3_pc(z0, z1, z2, &avgz);

		// Do we want to do back face removal ?
		if (_removeBackface) {
			// use winding order of the polygons
			// e.g. NormalClip which is cross product of screen vectors
			if (_useWindingOrder) {
				gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
				// Drawing back facings polys requires making them have
				// clockwise winding order as by definition they have
				// anti-clockwise winding order
				if (flag < 0) {
					if (_drawBface) {
						stemp = sxy1;
						sxy1 = sxy2;
						sxy2 = stemp;
						rgbTemp = rgb1;
						rgb1 = rgb2;
						rgb2 = rgbTemp;
					} else {
						// Skip this polygon
						continue;
					}
				} else if (flag == 0)
					continue; // ignore zero-area polygons
			}

			// TODO
			// dot product of viewing direction and polygon normal
			// will require generation of polygon normal from vertex normals
			// and transformation of the normal to world space
		}

		// Put wireframe triangles in list before textured ones
		// so should be in front of textured ones
		if ((_drawWfrm) || (selFace == i)) {
			LINE_F4 *line = (LINE_F4 *)drawpacket;
			setLineF4(line);
			if (selFace == i)
				setRGB0(line, selWRed, selWGreen, selWBlue);
			else
				setRGB0(line, (uint8)wfrmRed, (uint8)wfrmGreen, (uint8)wfrmBlue);
			// set XY's in the GPU packet 0-1-2-3
			line->x0 = sxy0.vx;
			line->y0 = sxy0.vy;
			line->x1 = sxy1.vx;
			line->y1 = sxy1.vy;
			line->x2 = sxy2.vx;
			line->y2 = sxy2.vy;
			line->x3 = sxy0.vx;
			line->y3 = sxy0.vy;
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(LINE_F4));
		}

		// Draw textured version behind the wireframe
		if ((_drawPolys) || (selFace == i)) {
			POLY_G3 *poly = (POLY_G3 *)drawpacket;
			// Draw untextured polygons
			setPolyG3(poly);
			// set XY's in the GPU packet 0-1-2
			poly->x0 = sxy0.vx;
			poly->y0 = sxy0.vy;
			poly->x1 = sxy1.vx;
			poly->y1 = sxy1.vy;
			poly->x2 = sxy2.vx;
			poly->y2 = sxy2.vy;

			// No light source calculation
			// So just set base colour
			if (selFace == i) {
				setRGB0(poly, selPRed, selPGreen, selPBlue);
				setRGB1(poly, selPRed, selPGreen, selPBlue);
				setRGB2(poly, selPRed, selPGreen, selPBlue);
			} else {
				setRGB0(poly, rgb0->r, rgb0->g, rgb0->b);
				setRGB1(poly, rgb1->r, rgb1->g, rgb1->b);
				setRGB2(poly, rgb2->r, rgb2->g, rgb2->b);
			}
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(POLY_G3));
		}
	}
}

// Flat, Textured, Self-Luminous Triangles
void drawFTS3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex) {
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	// u_int num = min( n, 10 ); // do a maximum of 10 polygons
	u_int num = n;
	u_short uu0, uu1, uu2;
	u_short vv0, vv1, vv2;
	uint32 tmp;
	int32 p, flag;
	u_int i;
	uint32 *pPoly;
	int32 z0, z1, z2;
	SVECTORPC sxy0, sxy1, sxy2, stemp;
	uint32 uv0, uv1, uv2;

	// Base colour for shading computation
	// Could easily be specified per polygon
	CVECTOR *rgbIn;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < num; i++) {
		// Structure is :
		// Bit 31 ----> Bit 0
		//
		// 8-bits | 8-bits | 8-bits | 8-bits
		// 0x24   | Blue   | Green  | Red
		// 16-bits | 16-bits
		// --------------------------
		// v0      | u0
		// v1      | u1
		// v2      | u2
		// --------------------------
		// pad     | v0
		//  v2     | v1
		rgbIn = (CVECTOR *)pPoly++;

		if (deadObject) {
			rgbIn = &deadObjectColour;
		}

		uv0 = *pPoly++;
		vv0 = (u_short)((uv0 >> 16) & 0xFFFF);
		uu0 = (u_short)(uv0 & 0xFFFF);

		uv1 = *pPoly++;
		vv1 = (u_short)((uv1 >> 16) & 0xFFFF);
		uu1 = (u_short)(uv1 & 0xFFFF);

		uv2 = *pPoly++;
		vv2 = (u_short)((uv2 >> 16) & 0xFFFF);
		uu2 = (u_short)(uv2 & 0xFFFF);

		tmp = *pPoly++;
		v0 = pVertex + (tmp & 0xFFFF);

		tmp = *pPoly++;
		v1 = pVertex + (uint)(tmp & 0xFFFF);
		v2 = pVertex + (uint)(tmp >> 16);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		gte_RotTransPers_pc(v0, &sxy0, &p, &flag, &z0);
		z0 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4

		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v1, &sxy1, &p, &flag, &z1);
		z1 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v2, &sxy2, &p, &flag, &z2);
		z2 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		// Give the GTE some time to recover
		int32 avgz;
		gte_AverageZ3_pc(z0, z1, z2, &avgz);

		// Do we want to do back face removal ?
		if (_removeBackface) {
			// use winding order of the polygons
			// e.g. NormalClip which is cross product of screen vectors
			if (_useWindingOrder) {
				gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
				// Drawing back facings polys requires making them have
				// clockwise winding order as by definition they have
				// anti-clockwise winding order
				if (flag < 0) {
					if (_drawBface) {
						stemp = sxy1;
						sxy1 = sxy2;
						sxy2 = stemp;
						flag = uv1;
						uv1 = uv2;
						uv2 = flag;
					} else {
						// Skip this polygon
						continue;
					}
				} else if (flag == 0)
					continue; // ignore zero-area polygons
			}

			// TODO
			// dot product of viewing direction and polygon normal
			// will require generation of polygon normal from vertex normals
			// and transformation of the normal to world space
		}

		// Put wireframe triangles in list before textured ones
		// so should be in front of textured ones
		if ((_drawWfrm) || (selFace == i)) {
			LINE_F4 *line = (LINE_F4 *)drawpacket;
			setLineF4(line);
			if (selFace == i)
				setRGB0(line, selWRed, selWGreen, selWBlue);
			else
				setRGB0(line, (uint8)wfrmRed, (uint8)wfrmGreen, (uint8)wfrmBlue);
			// set XY's in the GPU packet 0-1-2-3
			line->x0 = sxy0.vx;
			line->y0 = sxy0.vy;
			line->x1 = sxy1.vx;
			line->y1 = sxy1.vy;
			line->x2 = sxy2.vx;
			line->y2 = sxy2.vy;
			line->x3 = sxy0.vx;
			line->y3 = sxy0.vy;
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(LINE_F4));
		}

		// Draw textured version behind the wireframe
		if ((_drawPolys) || (selFace == i)) {
			if (_drawTxture) {
				POLY_FT3 *poly = (POLY_FT3 *)drawpacket;
				setPolyFT3(poly);
				// set XY's in the GPU packet 0-1-2
				poly->x0 = sxy0.vx;
				poly->y0 = sxy0.vy;
				poly->x1 = sxy1.vx;
				poly->y1 = sxy1.vy;
				poly->x2 = sxy2.vx;
				poly->y2 = sxy2.vy;

				setUV3(poly, uu0, vv0, uu1, vv1, uu2, vv2);

				// No light source calculation
				// So just set base colour
				if (selFace == i)
					setRGB0(poly, selPRed, selPGreen, selPBlue);
				else
					setRGB0(poly, rgbIn->r, rgbIn->b, rgbIn->b);

				// Put it into the global ot at the correct place
				myAddPrimClip(avgz, drawpacket);
				// advance the global packet pointer by correct amount
				myAddPacket(sizeof(POLY_FT3));
			} else {
				// Set the RGB colours
				POLY_F3 *poly = (POLY_F3 *)drawpacket;
				// Draw untextured polygons
				setPolyF3(poly);
				// set XY's in the GPU packet 0-1-2
				poly->x0 = sxy0.vx;
				poly->y0 = sxy0.vy;
				poly->x1 = sxy1.vx;
				poly->y1 = sxy1.vy;
				poly->x2 = sxy2.vx;
				poly->y2 = sxy2.vy;

				// No light source calculation
				// So just set base colour
				if (selFace == i)
					setRGB0(poly, selPRed, selPGreen, selPBlue);
				else
					setRGB0(poly, rgbIn->r, rgbIn->g, rgbIn->b);
				// Put it into the global ot at the correct place
				myAddPrimClip(avgz, drawpacket);
				// advance the global packet pointer by correct amount
				myAddPacket(sizeof(POLY_F3));
			}
		}
	}
}

// Gouraud, Textured, Self-Luminous Triangles
void drawGTS3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex) {
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	// u_int num = min( n, 10 ); // do a maximum of 10 polygons
	u_int num = n;
	u_short uu0, uu1, uu2;
	u_short vv0, vv1, vv2;
	uint32 tmp;
	int32 p, flag;
	u_int i;
	uint32 *pPoly;
	int32 z0, z1, z2;
	SVECTORPC sxy0, sxy1, sxy2, stemp;
	uint32 uv0, uv1, uv2;

	// gouraud colours
	CVECTOR *rgb0;
	CVECTOR *rgb1;
	CVECTOR *rgb2;
	CVECTOR *rgbTemp;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < num; i++) {
		// Structure is :
		// 8-bits | 8-bits | 8-bits | 8-bits
		// 0x34   | Blue0  | Green0 | Red0
		// 0x34   | Blue1  | Green1 | Red1
		// 0x34   | Blue2  | Green2 | Red2
		// 16-bits | 16-bits
		// --------------------------
		// v0      | u0
		// v1      | u1
		// v2      | u2
		// --------------------------
		// pad     | v0
		//  v2     | v1
		rgb0 = (CVECTOR *)pPoly++;
		rgb1 = (CVECTOR *)pPoly++;
		rgb2 = (CVECTOR *)pPoly++;

		if (deadObject) {
			rgb0 = &deadObjectColour;
			rgb1 = &deadObjectColour;
			rgb2 = &deadObjectColour;
		}

		uv0 = *pPoly++;
		vv0 = (u_short)((uv0 >> 16) & 0xFFFF);
		uu0 = (u_short)(uv0 & 0xFFFF);

		uv1 = *pPoly++;
		vv1 = (u_short)((uv1 >> 16) & 0xFFFF);
		uu1 = (u_short)(uv1 & 0xFFFF);

		uv2 = *pPoly++;
		vv2 = (u_short)((uv2 >> 16) & 0xFFFF);
		uu2 = (u_short)(uv2 & 0xFFFF);

		tmp = *pPoly++;
		v0 = pVertex + (tmp & 0xFFFF);

		tmp = *pPoly++;
		v1 = pVertex + (tmp & 0xFFFF);
		v2 = pVertex + (tmp >> 16);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		gte_RotTransPers_pc(v0, &sxy0, &p, &flag, &z0);
		z0 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4

		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v1, &sxy1, &p, &flag, &z1);
		z1 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v2, &sxy2, &p, &flag, &z2);
		z2 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		// Give the GTE some time to recover
		int32 avgz;
		gte_AverageZ3_pc(z0, z1, z2, &avgz);

		// Do we want to do back face removal ?
		if (_removeBackface) {
			// use winding order of the polygons
			// e.g. NormalClip which is cross product of screen vectors
			if (_useWindingOrder) {
				gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
				// Drawing back facings polys requires making them have
				// clockwise winding order as by definition they have
				// anti-clockwise winding order
				if (flag < 0) {
					if (_drawBface) {
						stemp = sxy1;
						sxy1 = sxy2;
						sxy2 = stemp;
						rgbTemp = rgb1;
						rgb1 = rgb2;
						rgb2 = rgbTemp;
						flag = uv1;
						uv1 = uv2;
						uv2 = flag;
					} else {
						// Skip this polygon
						continue;
					}
				} else if (flag == 0)
					continue; // ignore zero-area polygons
			}

			// TODO
			// dot product of viewing direction and polygon normal
			// will require generation of polygon normal from vertex normals
			// and transformation of the normal to world space
		}

		// Put wireframe triangles in list before textured ones
		// so should be in front of textured ones
		if ((_drawWfrm) || (selFace == i)) {
			LINE_F4 *line = (LINE_F4 *)drawpacket;
			setLineF4(line);
			if (selFace == i)
				setRGB0(line, selWRed, selWGreen, selWBlue);
			else
				setRGB0(line, (uint8)wfrmRed, (uint8)wfrmGreen, (uint8)wfrmBlue);
			// set XY's in the GPU packet 0-1-2-3
			line->x0 = sxy0.vx;
			line->y0 = sxy0.vy;
			line->x1 = sxy1.vx;
			line->y1 = sxy1.vy;
			line->x2 = sxy2.vx;
			line->y2 = sxy2.vy;
			line->x3 = sxy0.vx;
			line->y3 = sxy0.vy;

			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(LINE_F4));
		}

		// Draw textured version behind the wireframe
		if ((_drawPolys) || (selFace == i)) {
			if (_drawTxture) {
				// Set the RGB colours
				POLY_GT3 *poly = (POLY_GT3 *)drawpacket;
				setPolyGT3(poly);
				// set XY's in the GPU packet 0-1-2
				poly->x0 = sxy0.vx;
				poly->y0 = sxy0.vy;
				poly->x1 = sxy1.vx;
				poly->y1 = sxy1.vy;
				poly->x2 = sxy2.vx;
				poly->y2 = sxy2.vy;

				setUV3(poly, uu0, vv0, uu1, vv1, uu2, vv2);

				// No light source calculation
				// So just set base colour
				if (selFace == i) {
					setRGB0(poly, selPRed, selPGreen, selPBlue);
					setRGB1(poly, selPRed, selPGreen, selPBlue);
					setRGB2(poly, selPRed, selPGreen, selPBlue);
				} else {
					setRGB0(poly, rgb0->r, rgb0->b, rgb0->b);
					setRGB1(poly, rgb1->r, rgb1->b, rgb1->b);
					setRGB2(poly, rgb2->r, rgb2->b, rgb2->b);
				}

				// Put it into the global ot at the correct place
				myAddPrimClip(avgz, drawpacket);
				// advance the global packet pointer by correct amount
				myAddPacket(sizeof(POLY_GT3));
			} else {
				// Set the RGB colours
				POLY_G3 *poly = (POLY_G3 *)drawpacket;
				// Draw untextured polygons
				setPolyG3(poly);
				// set XY's in the GPU packet 0-1-2
				poly->x0 = sxy0.vx;
				poly->y0 = sxy0.vy;
				poly->x1 = sxy1.vx;
				poly->y1 = sxy1.vy;
				poly->x2 = sxy2.vx;
				poly->y2 = sxy2.vy;

				// No light source calculation
				// So just set base colour
				if (selFace == i) {
					setRGB0(poly, selPRed, selPGreen, selPBlue);
					setRGB1(poly, selPRed, selPGreen, selPBlue);
					setRGB2(poly, selPRed, selPGreen, selPBlue);
				} else {
					setRGB0(poly, rgb0->r, rgb0->b, rgb0->b);
					setRGB1(poly, rgb1->r, rgb1->b, rgb1->b);
					setRGB2(poly, rgb2->r, rgb2->b, rgb2->b);
				}

				// Put it into the global ot at the correct place
				myAddPrimClip(avgz, drawpacket);
				// advance the global packet pointer by correct amount
				myAddPacket(sizeof(POLY_G3));
			}
		}
	}
}

// Flat, Un-Textured, Lit, triangles
void drawFUL3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex, SVECTOR *pNormal) {
	SVECTOR *n0;
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	// u_int num = min( n, 10 ); // do a maximum of 10 polygons
	u_int num = n;
	int32 p, flag;
	u_int i;
	uint32 *pPoly;
	int32 z0, z1, z2;
	SVECTORPC sxy0, sxy1, sxy2, stemp;
	SVECTORPC sxyn0;
	SVECTORPC sn0;
	CVECTOR *rgbIn;
	CVECTOR rgb0 = {0, 0, 0, 0};
	uint32 tmp;
	u_char ctmp;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < num; i++) {
		// Structure is :
		// 0x20 | B | G | R
		// v0       | n0
		// v2       | v1
		rgbIn = (CVECTOR *)pPoly++;

		tmp = *pPoly++;
		v0 = pVertex + (tmp >> 16);
		n0 = pNormal + (tmp & 0xFFFF);

		tmp = *pPoly++;
		v2 = pVertex + (tmp >> 16);
		v1 = pVertex + (tmp & 0xFFFF);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		gte_RotTransPers_pc(v0, &sxy0, &p, &flag, &z0);
		z0 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4

		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v1, &sxy1, &p, &flag, &z1);
		z1 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v2, &sxy2, &p, &flag, &z2);
		z2 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		// Give the GTE some time to recover
		int32 avgz;
		gte_AverageZ3_pc(z0, z1, z2, &avgz);

		// Do we want to do back face removal ?
		if (_removeBackface) {
			// use winding order of the polygons
			// e.g. NormalClip which is cross product of screen vectors
			if (_useWindingOrder) {
				gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
				// Drawing back facings polys requires making them have
				// clockwise winding order as by definition they have
				// anti-clockwise winding order
				if (flag < 0) {
					if (_drawBface) {
						stemp = sxy1;
						sxy1 = sxy2;
						sxy2 = stemp;
					} else {
						// Skip this polygon
						continue;
					}
				} else if (flag == 0)
					continue; // ignore zero-area polygons
			}

			// TODO
			// dot product of viewing direction and polygon normal
			// will require generation of polygon normal from vertex normals
			// and transformation of the normal to world space
		}

		// Set the RGB colours
		if (_drawLit) {
			// Do the flat lighting computation
			LIGHTPOLYGON(n0, rgbIn, &rgb0);
		}

		// Put normal lines in list before wireframe
		// so should be drawn in front of wireframe triangles
		if ((_drawNormals) && ((selFace == i) || (selFace > 65536))) {
			// Do coord transform on the scaled normal vectors + v0
			sn0.vx = (short)(v0->vx + (-n0->vx >> _normalScale));
			sn0.vy = (short)(v0->vy + (-n0->vy >> _normalScale));
			sn0.vz = (short)(v0->vz + (-n0->vz >> _normalScale));
			// z0 = RotTransPers( &sn0, &sxyn0, &p, &flag );
			gte_RotTransPers_pc(&sn0, &sxyn0, &p, &flag, &z0);
			// Normal 0 : scale the normal and add to vertex scrn pos
			LINE_G2 *line = (LINE_G2 *)drawpacket;
			setLineG2(line);
			line->x0 = sxy0.vx;
			line->y0 = sxy0.vy;
			line->x1 = sxyn0.vx;
			line->y1 = sxyn0.vy;
			if (selFace == i) {
				// from bright red
				setRGB0(line, 255, 0, 0);
				// to bright white
				setRGB1(line, 255, 255, 255);
			} else {
				// from bright blue
				setRGB0(line, 0, 0, 255);
				// to bright white
				setRGB1(line, 255, 255, 255);
			}
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(LINE_G2));
		}

		// Put wireframe triangles in list before textured ones
		// so should be in front of textured ones
		if ((_drawWfrm) || (selFace == i)) {
			LINE_F4 *line = (LINE_F4 *)drawpacket;
			setLineF4(line);
			// set XY's in the GPU packet 0-1-2-3
			line->x0 = sxy0.vx;
			line->y0 = sxy0.vy;
			line->x1 = sxy1.vx;
			line->y1 = sxy1.vy;
			line->x2 = sxy2.vx;
			line->y2 = sxy2.vy;
			line->x3 = sxy0.vx;
			line->y3 = sxy0.vy;
			// Draw a wireframe that responds to lighting
			if (_drawLit) {
				int cval;
				// Compute the colour scaling and clip it
				// Flat shading so give the same colour to each vertex
				if (rgbIn->r != 0) {
					cval = ((wfrmRed * rgb0.r) / rgbIn->r);
					if (cval > 255)
						cval = 255;
					ctmp = (uint8)cval;
				} else
					ctmp = 0;
				line->r0 = ctmp;

				if (rgbIn->g != 0) {
					cval = ((wfrmGreen * rgb0.g) / rgbIn->g);
					if (cval > 255)
						cval = 255;
					ctmp = (uint8)cval;
				} else
					ctmp = 0;
				line->g0 = ctmp;

				if (rgbIn->b != 0) {
					cval = ((wfrmBlue * rgb0.b) / rgbIn->b);
					if (cval > 255)
						cval = 255;
					ctmp = (uint8)cval;
				} else
					ctmp = 0;
				line->b0 = ctmp;
			} else {
				setRGB0(line, (uint8)wfrmRed, (uint8)wfrmGreen, (uint8)wfrmBlue);
			}
			if (selFace == i)
				setRGB0(line, selWRed, selWGreen, selWBlue);

			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(LINE_F4));
		}

		// Draw polygon version behind the wireframe
		if ((_drawPolys) || (selFace == i)) {
			// Draw untextured polygons
			POLY_F3 *poly = (POLY_F3 *)drawpacket;
			setPolyF3(poly);
			// set XY's in the GPU packet 0-1-2
			poly->x0 = sxy0.vx;
			poly->y0 = sxy0.vy;
			poly->x1 = sxy1.vx;
			poly->y1 = sxy1.vy;
			poly->x2 = sxy2.vx;
			poly->y2 = sxy2.vy;

			// Set the RGB colours
			if (_drawLit) {
				// Flat shaded so give each vertex the same colour
				setRGB0(poly, rgb0.r, rgb0.g, rgb0.b);
			} else {
				// No light source calculation
				// So just set base colour
				setRGB0(poly, rgbIn->r, rgbIn->g, rgbIn->b);
			}
			if (selFace == i) {
				setRGB0(poly, selPRed, selPGreen, selPBlue);
			}
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(POLY_F3));
		}
	}
}

// Gouraud, Un-Textured, Lit, triangles
void drawGUL3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex, SVECTOR *pNormal) {
	SVECTOR *n0;
	SVECTOR *n1;
	SVECTOR *n2;
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;
	SVECTOR *ntemp;

	// u_int num = min( n, 10 ); // do a maximum of 10 polygons
	u_int num = n;
	uint32 tmp;
	u_char ctmp;
	int32 p, flag;
	u_int i;
	uint32 *pPoly;
	int32 z0, z1, z2;
	SVECTORPC sxy0, sxy1, sxy2, stemp;
	SVECTORPC sxyn0, sxyn1, sxyn2;
	SVECTORPC sn0, sn1, sn2;

	// Base colour for gouraud shading computation
	// Could easily be specified per polygon
	CVECTOR rgbIn;
	CVECTOR rgb0 = {0, 0, 0, 0};
	CVECTOR rgb1 = {0, 0, 0, 0};
	CVECTOR rgb2 = {0, 0, 0, 0};

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < num; i++) {
		// Structure is :
		// 8-bits | 8-bits | 8-bits | 8-bits
		// 0x20   | Blue   | Green  | Red
		// 16-bits | 8-bits | 8-bits
		// --------------------------
		//  v0              | n0
		//  v1              | n1
		//  v2              | n2
		tmp = *pPoly++;
		// code0 = tmp >> 24;
		rgbIn.r = (uint8)(tmp & 0xFF);
		rgbIn.g = (uint8)((tmp >> 8) & 0xFF);
		rgbIn.b = (uint8)((tmp >> 16) & 0xFF);

		tmp = *pPoly++;
		v0 = pVertex + (tmp >> 16);
		n0 = pNormal + (tmp & 0xFFFF);

		tmp = *pPoly++;
		v1 = pVertex + (tmp >> 16);
		n1 = pNormal + (tmp & 0xFFFF);

		tmp = *pPoly++;
		v2 = pVertex + (tmp >> 16);
		n2 = pNormal + (tmp & 0xFFFF);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		gte_RotTransPers_pc(v0, &sxy0, &p, &flag, &z0);
		z0 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4

		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v1, &sxy1, &p, &flag, &z1);
		z1 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v2, &sxy2, &p, &flag, &z2);
		z2 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		// Give the GTE some time to recover
		int32 avgz;
		gte_AverageZ3_pc(z0, z1, z2, &avgz);

		// Do we want to do back face removal ?
		if (_removeBackface) {
			// use winding order of the polygons
			// e.g. NormalClip which is cross product of screen vectors
			if (_useWindingOrder) {
				gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
				// Drawing back facings polys requires making them have
				// clockwise winding order as by definition they have
				// anti-clockwise winding order
				if (flag < 0) {
					if (_drawBface) {
						stemp = sxy1;
						sxy1 = sxy2;
						sxy2 = stemp;
						ntemp = n1;
						n1 = n2;
						n2 = ntemp;
					} else {
						// Skip this polygon
						continue;
					}
				} else if (flag == 0)
					continue; // ignore zero-area polygons
			}

			// TODO
			// dot product of viewing direction and polygon normal
			// will require generation of polygon normal from vertex normals
			// and transformation of the normal to world space
		}

		// Set the RGB colours
		if (_drawLit) {
			// Do the full gouraud computation
			// NormalColorCol3( n0, n1, n2, &rgbIn, &rgb0, &rgb1, &rgb2 );
			LIGHTPOLYGON3(n0, n1, n2, &rgbIn, &rgb0, &rgb1, &rgb2);

			int cval;

			if (!_drawGouraud) {
				// For flat shading mode take the average RGB for the face
				// Could/should use something like AVSZ3
				// but hell whats a divide by 3 between friends (~30 cycles?)
				cval = ((rgb0.r + rgb1.r + rgb2.r) / 3);
				if (cval > 255)
					cval = 255;
				rgb0.r = (uint8)cval;

				cval = ((rgb0.g + rgb1.g + rgb2.g) / 3);
				if (cval > 255)
					cval = 255;
				rgb0.g = (uint8)cval;

				cval = ((rgb0.b + rgb1.b + rgb2.b) / 3);
				if (cval > 255)
					cval = 255;
				rgb0.b = (uint8)cval;
			}
		} else {
			rgbIn.r = unlitPoly.r;
			rgbIn.g = unlitPoly.g;
			rgbIn.b = unlitPoly.b;
		}

		// Put normal lines in list before wireframe
		// so should be drawn in front of wireframe triangles
		if ((_drawNormals) && ((selFace == i) || (selFace > 65536))) {
			// Do coord transform on the scaled normal vectors + v0
			sn0.vx = (short)(v0->vx + (-n0->vx >> _normalScale));
			sn0.vy = (short)(v0->vy + (-n0->vy >> _normalScale));
			sn0.vz = (short)(v0->vz + (-n0->vz >> _normalScale));
			sn1.vx = (short)(v1->vx + (-n1->vx >> _normalScale));
			sn1.vy = (short)(v1->vy + (-n1->vy >> _normalScale));
			sn1.vz = (short)(v1->vz + (-n1->vz >> _normalScale));
			sn2.vx = (short)(v2->vx + (-n2->vx >> _normalScale));
			sn2.vy = (short)(v2->vy + (-n2->vy >> _normalScale));
			sn2.vz = (short)(v2->vz + (-n2->vz >> _normalScale));

			// z0 = RotTransPers3( &sn0, &sn1, &sn2,
			//                    &sxyn0, &sxyn1, &sxyn2, &p, &flag );
			gte_RotTransPers3_pc(&sn0, &sn1, &sn2, &sxyn0, &sxyn1, &sxyn2, &p, &flag, &z0);
			// Normal 0
			LINE_G2 *line = (LINE_G2 *)drawpacket;
			setLineG2(line);
			line->x0 = sxy0.vx;
			line->y0 = sxy0.vy;
			line->x1 = sxy1.vx;
			line->y1 = sxy1.vy;
			if (selFace == i) {
				// Bright red
				setRGB0(line, 255, 0, 0);
				// Bright white
				setRGB1(line, 255, 255, 255);
			} else {
				// Bright blue
				setRGB0(line, 0, 0, 255);
				// Bright white
				setRGB1(line, 255, 255, 255);
			}
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(LINE_G2));

			// Normal 1
			line = (LINE_G2 *)drawpacket;
			setLineG2(line);
			line->x0 = sxy0.vx;
			line->y0 = sxy0.vy;
			line->x1 = sxyn1.vx;
			line->y1 = sxyn1.vy;
			if (selFace == i) {
				// Bright red
				setRGB0(line, 255, 0, 0);
				// Bright white
				setRGB1(line, 255, 255, 255);
			} else {
				// Bright blue
				setRGB0(line, 0, 0, 255);
				// Bright white
				setRGB1(line, 255, 255, 255);
			}
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(LINE_G2));

			// Normal 2
			line = (LINE_G2 *)drawpacket;
			setLineG2(line);
			line->x0 = sxy0.vx;
			line->y0 = sxy0.vy;
			line->x1 = sxyn2.vx;
			line->y1 = sxyn2.vy;
			if (selFace == i) {
				// Bright red
				setRGB0(line, 255, 0, 0);
				// Bright white
				setRGB1(line, 255, 255, 255);
			} else {
				// Bright blue
				setRGB0(line, 0, 0, 255);
				// Bright white
				setRGB1(line, 255, 255, 255);
			}
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(LINE_G2));
		}

		// Put wireframe triangles in list before textured ones
		// so should be in front of textured ones
		if ((_drawWfrm) || (selFace == i)) {
			// Draw a wireframe that responds to lighting
			if (_drawLit) {
				LINE_G4 *line = (LINE_G4 *)drawpacket;
				setLineG4(line);
				int cval;
				if (_drawGouraud) {
					// Give each vertex a seperate colour
					// Compute the colour scaling and clip it
					if (rgbIn.r != 0) {
						cval = ((wfrmRed * rgb0.r) / rgbIn.r);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->r0 = ctmp;
						line->r3 = ctmp;

						cval = ((wfrmRed * rgb1.r) / rgbIn.r);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->r1 = ctmp;

						cval = ((wfrmRed * rgb2.r) / rgbIn.r);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->r2 = ctmp;
					} else {
						line->r0 = 0;
						line->r3 = 0;
						line->r1 = 0;
						line->r2 = 0;
					}

					if (rgbIn.g != 0) {
						cval = ((wfrmGreen * rgb0.g) / rgbIn.g);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->g0 = ctmp;
						line->g3 = ctmp;

						cval = ((wfrmGreen * rgb1.g) / rgbIn.g);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->g1 = ctmp;

						cval = ((wfrmGreen * rgb2.g) / rgbIn.g);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->g2 = ctmp;
					} else {
						line->g0 = 0;
						line->g3 = 0;
						line->g1 = 0;
						line->g2 = 0;
					}
					if (rgbIn.b != 0) {
						cval = ((wfrmBlue * rgb0.b) / rgbIn.b);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->b0 = ctmp;
						line->b3 = ctmp;

						cval = ((wfrmBlue * rgb1.b) / rgbIn.b);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->b1 = ctmp;

						cval = ((wfrmBlue * rgb2.b) / rgbIn.b);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->b2 = ctmp;
					} else {
						line->b0 = 0;
						line->b3 = 0;
						line->b1 = 0;
						line->b2 = 0;
					}
				} else {
					// Compute the colour scaling and clip it
					// Flat shading so give the same colour to each vertex
					if (rgbIn.r != 0) {
						cval = ((wfrmRed * rgb0.r) / rgbIn.r);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
					} else
						ctmp = 0;
					line->r0 = ctmp;
					line->r1 = ctmp;
					line->r2 = ctmp;
					line->r3 = ctmp;

					if (rgbIn.g != 0) {
						cval = ((wfrmGreen * rgb0.g) / rgbIn.g);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
					} else
						ctmp = 0;
					line->g0 = ctmp;
					line->g1 = ctmp;
					line->g2 = ctmp;
					line->g3 = ctmp;

					if (rgbIn.b != 0) {
						cval = ((wfrmBlue * rgb0.b) / rgbIn.b);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
					} else
						ctmp = 0;

					line->b0 = ctmp;
					line->b1 = ctmp;
					line->b2 = ctmp;
					line->b3 = ctmp;
				}
				if (selFace == i) {
					setRGB0(line, selWRed, selWGreen, selWBlue);
					setRGB1(line, selWRed, selWGreen, selWBlue);
					setRGB2(line, selWRed, selWGreen, selWBlue);
					setRGB3(line, selWRed, selWGreen, selWBlue);
				}

				// set XY's in the GPU packet 0-1-2-3
				line->x0 = sxy0.vx;
				line->y0 = sxy0.vy;
				line->x1 = sxy1.vx;
				line->y1 = sxy1.vy;
				line->x2 = sxy2.vx;
				line->y2 = sxy2.vy;
				line->x3 = sxy0.vx;
				line->y3 = sxy0.vy;
				// Put it into the global ot at the correct place
				myAddPrimClip(avgz, drawpacket);
				// advance the global packet pointer by correct amount
				myAddPacket(sizeof(LINE_G4));
			} else {
				LINE_F4 *line = (LINE_F4 *)drawpacket;
				setLineF4(line);
				if (selFace == i)
					setRGB0(line, selWRed, selWGreen, selWBlue);
				else
					setRGB0(line, (uint8)wfrmRed, (uint8)wfrmGreen, (uint8)wfrmBlue);
				// set XY's in the GPU packet 0-1-2-3
				line->x0 = sxy0.vx;
				line->y0 = sxy0.vy;
				line->x1 = sxy1.vx;
				line->y1 = sxy1.vy;
				line->x2 = sxy2.vx;
				line->y2 = sxy2.vy;
				line->x3 = sxy0.vx;
				line->y3 = sxy0.vy;
				// Put it into the global ot at the correct place
				myAddPrimClip(avgz, drawpacket);
				// advance the global packet pointer by correct amount
				myAddPacket(sizeof(LINE_F4));
			}
		}

		// Draw textured version behind the wireframe
		if ((_drawPolys) || (selFace == i)) {
			// Set the RGB colours
			if (_drawLit) {
				if (_drawGouraud) {
					POLY_G3 *poly = (POLY_G3 *)drawpacket;
					// Draw untextured polygons
					setPolyG3(poly);
					// set XY's in the GPU packet 0-1-2
					poly->x0 = sxy0.vx;
					poly->y0 = sxy0.vy;
					poly->x1 = sxy1.vx;
					poly->y1 = sxy1.vy;
					poly->x2 = sxy2.vx;
					poly->y2 = sxy2.vy;

					if (selFace == i) {
						setRGB0(poly, selPRed, selPGreen, selPBlue);
						setRGB1(poly, selPRed, selPGreen, selPBlue);
						setRGB2(poly, selPRed, selPGreen, selPBlue);
					} else {
						setRGB0(poly, rgb0.r, rgb0.g, rgb0.b);
						setRGB1(poly, rgb1.r, rgb1.g, rgb1.b);
						setRGB2(poly, rgb2.r, rgb2.g, rgb2.b);
					}

					// Put it into the global ot at the correct place
					myAddPrimClip(avgz, drawpacket);
					// advance the global packet pointer by correct amount
					myAddPacket(sizeof(POLY_G3));
				} else {
					POLY_F3 *poly = (POLY_F3 *)drawpacket;
					// Draw untextured polygons
					setPolyF3(poly);
					// set XY's in the GPU packet 0-1-2
					poly->x0 = sxy0.vx;
					poly->y0 = sxy0.vy;
					poly->x1 = sxy1.vx;
					poly->y1 = sxy1.vy;
					poly->x2 = sxy2.vx;
					poly->y2 = sxy2.vy;

					// Flat shaded so give each vertex the same colour
					setRGB0(poly, rgb0.r, rgb0.g, rgb0.b);

					// Put it into the global ot at the correct place
					myAddPrimClip(avgz, drawpacket);
					// advance the global packet pointer by correct amount
					myAddPacket(sizeof(POLY_F3));
				}
			} else {
				POLY_F3 *poly = (POLY_F3 *)drawpacket;
				// Draw untextured polygons
				setPolyF3(poly);
				// set XY's in the GPU packet 0-1-2
				poly->x0 = sxy0.vx;
				poly->y0 = sxy0.vy;
				poly->x1 = sxy1.vx;
				poly->y1 = sxy1.vy;
				poly->x2 = sxy2.vx;
				poly->y2 = sxy2.vy;

				// No light source calculation
				// So just set base colour
				setRGB0(poly, rgbIn.r, rgbIn.g, rgbIn.b);
				// Put it into the global ot at the correct place
				myAddPrimClip(avgz, drawpacket);
				// advance the global packet pointer by correct amount
				myAddPacket(sizeof(POLY_F3));
			}
		}
	}
}

// Flat, Textured, Lit Triangles
void drawFTL3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex, SVECTOR *pNormal) {
	SVECTOR *n0;
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	// u_int num = min( n, 10 ); // do a maximum of 10 polygons
	u_int num = n;
	u_short uu0, uu1, uu2;
	u_short vv0, vv1, vv2;
	uint32 tmp;
	u_char ctmp;
	int32 p, flag;
	u_int i;
	uint32 *pPoly;
	int32 z0, z1, z2;
	SVECTORPC sxy0, sxy1, sxy2, stemp;
	SVECTORPC sxyn0;
	SVECTORPC sn0;
	uint32 uv0, uv1, uv2;

	// Base colour for shading computation
	// Could easily be specified per polygon
	CVECTOR rgbIn = {128, 128, 128, 0};
	CVECTOR rgb0 = {0, 0, 0, 0};

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < num; i++) {
		// Structure is :
		// Bit 31 ----> Bit 0
		//
		// 16-bits | 16-bits
		// --------------------------
		// v0      | u0
		// v1      | u1
		// v2      | u2
		// --------------------------
		//  v0              | n0
		//  v2              | v1
		uv0 = *pPoly++;
		vv0 = (u_short)((uv0 >> 16) & 0xFFFF);
		uu0 = (u_short)(uv0 & 0xFFFF);

		uv1 = *pPoly++;
		vv1 = (u_short)((uv1 >> 16) & 0xFFFF);
		uu1 = (u_short)(uv1 & 0xFFFF);

		uv2 = *pPoly++;
		vv2 = (u_short)((uv2 >> 16) & 0xFFFF);
		uu2 = (u_short)(uv2 & 0xFFFF);

		tmp = *pPoly++;
		v0 = pVertex + (tmp >> 16);
		n0 = pNormal + (tmp & 0xFFFF);

		tmp = *pPoly++;
		v1 = pVertex + (tmp & 0xFFFF);
		v2 = pVertex + (tmp >> 16);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		gte_RotTransPers_pc(v0, &sxy0, &p, &flag, &z0);
		z0 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4

		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v1, &sxy1, &p, &flag, &z1);
		z1 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v2, &sxy2, &p, &flag, &z2);
		z2 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		// Give the GTE some time to recover
		int32 avgz;
		gte_AverageZ3_pc(z0, z1, z2, &avgz);

		// Do we want to do back face removal ?
		if (_removeBackface) {
			// use winding order of the polygons
			// e.g. NormalClip which is cross product of screen vectors
			if (_useWindingOrder) {
				gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
				// Drawing back facings polys requires making them have
				// clockwise winding order as by definition they have
				// anti-clockwise winding order
				if (flag < 0) {
					if (_drawBface) {
						stemp = sxy1;
						sxy1 = sxy2;
						sxy2 = stemp;
						flag = uv1;
						uv1 = uv2;
						uv2 = flag;
					} else {
						// Skip this polygon
						continue;
					}
				} else if (flag == 0)
					continue; // ignore zero-area polygons
			}

			// TODO
			// dot product of viewing direction and polygon normal
			// will require generation of polygon normal from vertex normals
			// and transformation of the normal to world space
		}

		// Set the RGB colours
		if (_drawLit) {
			// Do the full gouraud computation
			// NormalColorCol( n0, &rgbIn, &rgb0 );
			LIGHTPOLYGON(n0, &rgbIn, &rgb0);
		}

		// Put normal lines in list before wireframe
		// so should be drawn in front of wireframe triangles
		if ((_drawNormals) && ((selFace == i) || (selFace > 65536))) {
			// Do coord transform on the scaled normal vectors + v0
			sn0.vx = (short)(v0->vx + (-n0->vx >> _normalScale));
			sn0.vy = (short)(v0->vy + (-n0->vy >> _normalScale));
			sn0.vz = (short)(v0->vz + (-n0->vz >> _normalScale));
			// z0 = RotTransPers( &sn0, &sxyn0, &p, &flag );
			gte_RotTransPers_pc(&sn0, &sxyn0, &p, &flag, &z0);
			// Normal 0
			LINE_G2 *line = (LINE_G2 *)drawpacket;
			setLineG2(line);
			line->x0 = sxy0.vx;
			line->y0 = sxy0.vy;
			line->x1 = sxyn0.vx;
			line->y1 = sxyn0.vy;
			if (selFace == i) {
				// Bright red
				setRGB0(line, 255, 0, 0);
				// Bright white
				setRGB1(line, 255, 255, 255);
			} else {
				// Bright blue
				setRGB0(line, 0, 0, 255);
				// Bright white
				setRGB1(line, 255, 255, 255);
			}
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(LINE_G2));
		}

		// Put wireframe triangles in list before textured ones
		// so should be in front of textured ones
		if ((_drawWfrm) || (selFace == i)) {
			// Draw a wireframe that responds to lighting
			if (_drawLit) {
				int cval;
				LINE_G4 *line = (LINE_G4 *)drawpacket;
				setLineG4(line);
				// Compute the colour scaling and clip it
				// Flat shading so give the same colour to each vertex
				cval = ((wfrmRed * rgb0.r) / rgbIn.r);
				if (cval > 255)
					cval = 255;
				ctmp = (uint8)cval;
				line->r0 = ctmp;
				line->r1 = ctmp;
				line->r2 = ctmp;
				line->r3 = ctmp;
				cval = ((wfrmGreen * rgb0.g) / rgbIn.g);
				if (cval > 255)
					cval = 255;
				ctmp = (uint8)cval;
				line->g0 = ctmp;
				line->g1 = ctmp;
				line->g2 = ctmp;
				line->g3 = ctmp;
				cval = ((wfrmBlue * rgb0.b) / rgbIn.b);
				if (cval > 255)
					cval = 255;
				ctmp = (uint8)cval;
				line->b0 = ctmp;
				line->b1 = ctmp;
				line->b2 = ctmp;
				line->b3 = ctmp;

				if (selFace == i) {
					setRGB0(line, selWRed, selWGreen, selWBlue);
					setRGB1(line, selWRed, selWGreen, selWBlue);
					setRGB2(line, selWRed, selWGreen, selWBlue);
					setRGB3(line, selWRed, selWGreen, selWBlue);
				}
				// set XY's in the GPU packet 0-1-2-3
				line->x0 = sxy0.vx;
				line->y0 = sxy0.vy;
				line->x1 = sxy1.vx;
				line->y1 = sxy1.vy;
				line->x2 = sxy2.vx;
				line->y2 = sxy2.vy;
				line->x3 = sxy0.vx;
				line->y3 = sxy0.vy;
				// Put it into the global ot at the correct place
				myAddPrimClip(avgz, drawpacket);
				// advance the global packet pointer by correct amount
				myAddPacket(sizeof(LINE_G4));
			} else {
				LINE_F4 *line = (LINE_F4 *)drawpacket;
				setLineF4(line);
				if (selFace == i)
					setRGB0(line, selWRed, selWGreen, selWBlue);
				else
					setRGB0(line, (uint8)wfrmRed, (uint8)wfrmGreen, (uint8)wfrmBlue);
				// set XY's in the GPU packet 0-1-2-3
				line->x0 = sxy0.vx;
				line->y0 = sxy0.vy;
				line->x1 = sxy1.vx;
				line->y1 = sxy1.vy;
				line->x2 = sxy2.vx;
				line->y2 = sxy2.vy;
				line->x3 = sxy0.vx;
				line->y3 = sxy0.vy;
				// Put it into the global ot at the correct place
				myAddPrimClip(avgz, drawpacket);
				// advance the global packet pointer by correct amount
				myAddPacket(sizeof(LINE_F4));
			}
		}

		// Draw textured version behind the wireframe
		if ((_drawPolys) || (selFace == i)) {
			if (_drawTxture) {
				POLY_FT3 *poly = (POLY_FT3 *)drawpacket;
				setPolyFT3(poly);
				// set XY's in the GPU packet 0-1-2
				poly->x0 = sxy0.vx;
				poly->y0 = sxy0.vy;
				poly->x1 = sxy1.vx;
				poly->y1 = sxy1.vy;
				poly->x2 = sxy2.vx;
				poly->y2 = sxy2.vy;

				setUV3(poly, uu0, vv0, uu1, vv1, uu2, vv2);

				if (_drawLit) {
					setRGB0(poly, rgb0.r, rgb0.g, rgb0.b);
				} else {
					// No light source calculation
					// So just set base colour
					setRGB0(poly, rgbIn.r, rgbIn.b, rgbIn.b);
				}
				if (selFace == i) {
					setRGB0(poly, selPRed, selPGreen, selPBlue);
				}

				// Put it into the global ot at the correct place
				myAddPrimClip(avgz, drawpacket);
				// advance the global packet pointer by correct amount
				myAddPacket(sizeof(POLY_FT3));
			} else {
				// Set the RGB colours
				if (_drawLit) {
					POLY_F3 *poly = (POLY_F3 *)drawpacket;
					// Draw untextured polygons
					setPolyF3(poly);
					// set XY's in the GPU packet 0-1-2
					poly->x0 = sxy0.vx;
					poly->y0 = sxy0.vy;
					poly->x1 = sxy1.vx;
					poly->y1 = sxy1.vy;
					poly->x2 = sxy2.vx;
					poly->y2 = sxy2.vy;

					// Flat shaded so give each vertex the same colour
					if (selFace == i) {
						setRGB0(poly, selPRed, selPGreen, selPBlue);
					} else {
						setRGB0(poly, rgb0.r, rgb0.g, rgb0.b);
					}

					// Put it into the global ot at the correct place
					myAddPrimClip(avgz, drawpacket);
					// advance the global packet pointer by correct amount
					myAddPacket(sizeof(POLY_F3));
				} else {
					POLY_F3 *poly = (POLY_F3 *)drawpacket;
					// Draw untextured polygons
					setPolyF3(poly);
					// set XY's in the GPU packet 0-1-2
					poly->x0 = sxy0.vx;
					poly->y0 = sxy0.vy;
					poly->x1 = sxy1.vx;
					poly->y1 = sxy1.vy;
					poly->x2 = sxy2.vx;
					poly->y2 = sxy2.vy;

					// No light source calculation
					// So just set base colour
					setRGB0(poly, rgbIn.r, rgbIn.g, rgbIn.b);
					// Put it into the global ot at the correct place
					myAddPrimClip(avgz, drawpacket);
					// advance the global packet pointer by correct amount
					myAddPacket(sizeof(POLY_F3));
				}
			}
		}
	}
}

// Gouraud, Textured, Lit Triangles
void drawGTL3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex, SVECTOR *pNormal) {
	SVECTOR *n0;
	SVECTOR *n1;
	SVECTOR *n2;
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;
	SVECTOR *ntemp;

	// u_int num = min( n, 10 ); // do a maximum of 10 polygons
	u_int num = n;
	u_short uu0, uu1, uu2;
	u_short vv0, vv1, vv2;
	uint32 tmp;
	u_char ctmp;
	int32 p, flag;
	u_int i;
	uint32 *pPoly;
	int32 z0, z1, z2;
	SVECTORPC sxy0, sxy1, sxy2, stemp;
	SVECTORPC sxyn0, sxyn1, sxyn2;
	SVECTORPC sn0, sn1, sn2;
	uint32 uv0, uv1, uv2;

	// Base colour for gouraud shading computation
	// Could easily be specified per polygon
	CVECTOR rgbIn = {128, 128, 128, 0};
	CVECTOR rgb0 = {0, 0, 0, 0};
	CVECTOR rgb1 = {0, 0, 0, 0};
	CVECTOR rgb2 = {0, 0, 0, 0};

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < num; i++) {
		// Structure is :
		// v0, u0
		// v1, u1
		// v2, u2
		// v0     | n0
		// v1     | n1
		// v2     | n2
		uv0 = *pPoly++;
		vv0 = (u_short)((uv0 >> 16) & 0xFFFF);
		uu0 = (u_short)(uv0 & 0xFFFF);

		uv1 = *pPoly++;
		vv1 = (u_short)((uv1 >> 16) & 0xFFFF);
		uu1 = (u_short)(uv1 & 0xFFFF);

		uv2 = *pPoly++;
		vv2 = (u_short)((uv2 >> 16) & 0xFFFF);
		uu2 = (u_short)(uv2 & 0xFFFF);

		tmp = *pPoly++;
		v0 = pVertex + (tmp >> 16);
		n0 = pNormal + (tmp & 0xFFFF);

		tmp = *pPoly++;
		v1 = pVertex + (tmp >> 16);
		n1 = pNormal + (tmp & 0xFFFF);

		tmp = *pPoly++;
		v2 = pVertex + (tmp >> 16);
		n2 = pNormal + (tmp & 0xFFFF);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		gte_RotTransPers_pc(v0, &sxy0, &p, &flag, &z0);
		z0 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4

		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v1, &sxy1, &p, &flag, &z1);
		z1 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v2, &sxy2, &p, &flag, &z2);
		z2 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		// Give the GTE some time to recover
		int32 avgz;
		gte_AverageZ3_pc(z0, z1, z2, &avgz);

		// Do we want to do back face removal ?
		if (_removeBackface) {
			// use winding order of the polygons
			// e.g. NormalClip which is cross product of screen vectors
			if (_useWindingOrder) {
				gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
				// Drawing back facings polys requires making them have
				// clockwise winding order as by definition they have
				// anti-clockwise winding order
				if (flag < 0) {
					if (_drawBface) {
						stemp = sxy1;
						sxy1 = sxy2;
						sxy2 = stemp;
						ntemp = n1;
						n1 = n2;
						n2 = ntemp;
						flag = uv1;
						uv1 = uv2;
						uv2 = flag;
					} else {
						// Skip this polygon
						continue;
					}
				} else if (flag == 0)
					continue; // ignore zero-area polygons
			}

			// TODO
			// dot product of viewing direction and polygon normal
			// will require generation of polygon normal from vertex normals
			// and transformation of the normal to world space
		}

		// Set the RGB colours
		if (_drawLit) {
			// Do the full gouraud computation
			// NormalColorCol3( n0, n1, n2, &rgbIn, &rgb0, &rgb1, &rgb2 );
			LIGHTPOLYGON3(n0, n1, n2, &rgbIn, &rgb0, &rgb1, &rgb2);
			int cval;

			if (!_drawGouraud) {
				// For flat shading mode take the average RGB for the face
				// Could/should use something like AVSZ3
				// but hell whats a divide by 3 between friends (~30 cycles?)
				cval = ((rgb0.r + rgb1.r + rgb2.r) / 3);
				if (cval > 255)
					cval = 255;
				rgb0.r = (uint8)cval;

				cval = ((rgb0.g + rgb1.g + rgb2.g) / 3);
				if (cval > 255)
					cval = 255;
				rgb0.g = (uint8)cval;

				cval = ((rgb0.b + rgb1.b + rgb2.b) / 3);
				if (cval > 255)
					cval = 255;
				rgb0.b = (uint8)cval;
			}
		}

		// Put normal lines in list before wireframe
		// so should be drawn in front of wireframe triangles
		if ((_drawNormals) && ((selFace == i) || (selFace > 65536))) {
			// Do coord transform on the scaled normal vectors + v0
			sn0.vx = (short)(v0->vx + (-n0->vx >> _normalScale));
			sn0.vy = (short)(v0->vy + (-n0->vy >> _normalScale));
			sn0.vz = (short)(v0->vz + (-n0->vz >> _normalScale));
			sn1.vx = (short)(v1->vx + (-n1->vx >> _normalScale));
			sn1.vy = (short)(v1->vy + (-n1->vy >> _normalScale));
			sn1.vz = (short)(v1->vz + (-n1->vz >> _normalScale));
			sn2.vx = (short)(v2->vx + (-n2->vx >> _normalScale));
			sn2.vy = (short)(v2->vy + (-n2->vy >> _normalScale));
			sn2.vz = (short)(v2->vz + (-n2->vz >> _normalScale));
			// z0 = RotTransPers3( &sn0, &sn1, &sn2,
			//                    &sxyn0, &sxyn1, &sxyn2, &p, &flag );
			gte_RotTransPers3_pc(&sn0, &sn1, &sn2, &sxyn0, &sxyn1, &sxyn2, &p, &flag, &z0);
			// Normal 0
			LINE_G2 *line = (LINE_G2 *)drawpacket;
			setLineG2(line);
			line->x0 = sxy0.vx;
			line->y0 = sxy0.vy;
			line->x1 = sxyn0.vx;
			line->y1 = sxyn0.vy;
			if (selFace == i) {
				// Bright red
				setRGB0(line, 255, 0, 0);
				// Bright white
				setRGB1(line, 255, 255, 255);
			} else {
				// Bright blue
				setRGB0(line, 0, 0, 255);
				// Bright white
				setRGB1(line, 255, 255, 255);
			}
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(LINE_G2));

			// Normal 1
			line = (LINE_G2 *)drawpacket;
			setLineG2(line);
			line->x0 = sxy0.vx;
			line->y0 = sxy0.vy;
			line->x1 = sxyn1.vx;
			line->y1 = sxyn1.vy;
			if (selFace == i) {
				// Bright red
				setRGB0(line, 255, 0, 0);
				// Bright white
				setRGB1(line, 255, 255, 255);
			} else {
				// Bright blue
				setRGB0(line, 0, 0, 255);
				// Bright white
				setRGB1(line, 255, 255, 255);
			}
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(LINE_G2));

			// Normal 2
			line = (LINE_G2 *)drawpacket;
			setLineG2(line);
			line->x0 = sxy0.vx;
			line->y0 = sxy0.vy;
			line->x1 = sxyn2.vx;
			line->y1 = sxyn2.vy;
			if (selFace == i) {
				// Bright red
				setRGB0(line, 255, 0, 0);
				// Bright white
				setRGB1(line, 255, 255, 255);
			} else {
				// Bright blue
				setRGB0(line, 0, 0, 255);
				// Bright white
				setRGB1(line, 255, 255, 255);
			}
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(LINE_G2));
		}

		// Put wireframe triangles in list before textured ones
		// so should be in front of textured ones
		if ((_drawWfrm) || (selFace == i)) {
			// Draw a wireframe that responds to lighting
			if (_drawLit) {
				LINE_G4 *line = (LINE_G4 *)drawpacket;
				setLineG4(line);
				int cval;
				if (_drawGouraud) {
					// Give each vertex a seperate colour
					// Compute the colour scaling and clip it
					if (rgbIn.r != 0) {
						cval = ((wfrmRed * rgb0.r) / rgbIn.r);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->r0 = ctmp;
						line->r3 = ctmp;

						cval = ((wfrmRed * rgb1.r) / rgbIn.r);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->r1 = ctmp;

						cval = ((wfrmRed * rgb2.r) / rgbIn.r);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->r2 = ctmp;
					} else {
						line->r0 = 0;
						line->r3 = 0;
						line->r1 = 0;
						line->r2 = 0;
					}

					if (rgbIn.g != 0) {
						cval = ((wfrmGreen * rgb0.g) / rgbIn.g);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->g0 = ctmp;
						line->g3 = ctmp;
						cval = ((wfrmGreen * rgb1.g) / rgbIn.g);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->g1 = ctmp;
						cval = ((wfrmGreen * rgb2.g) / rgbIn.g);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->g2 = ctmp;
					} else {
						line->g0 = 0;
						line->g3 = 0;
						line->g1 = 0;
						line->g2 = 0;
					}
					if (rgbIn.b != 0) {
						cval = ((wfrmBlue * rgb0.b) / rgbIn.b);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->b0 = ctmp;
						line->b3 = ctmp;
						cval = ((wfrmBlue * rgb1.b) / rgbIn.b);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->b1 = ctmp;
						cval = ((wfrmBlue * rgb2.b) / rgbIn.b);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
						line->b2 = ctmp;
					} else {
						line->b0 = 0;
						line->b3 = 0;
						line->b1 = 0;
						line->b2 = 0;
					}
				} else {
					// Compute the colour scaling and clip it
					// Flat shading so give the same colour to each vertex
					if (rgbIn.r != 0) {
						cval = ((wfrmRed * rgb0.r) / rgbIn.r);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
					} else
						ctmp = 0;
					line->r0 = ctmp;
					line->r1 = ctmp;
					line->r2 = ctmp;
					line->r3 = ctmp;

					if (rgbIn.g != 0) {
						cval = ((wfrmGreen * rgb0.g) / rgbIn.g);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
					} else
						ctmp = 0;
					line->g0 = ctmp;
					line->g1 = ctmp;
					line->g2 = ctmp;
					line->g3 = ctmp;

					if (rgbIn.b != 0) {
						cval = ((wfrmBlue * rgb0.b) / rgbIn.b);
						if (cval > 255)
							cval = 255;
						ctmp = (uint8)cval;
					} else
						ctmp = 0;

					line->b0 = ctmp;
					line->b1 = ctmp;
					line->b2 = ctmp;
					line->b3 = ctmp;
				}

				if (selFace == i) {
					setRGB0(line, selWRed, selWGreen, selWBlue);
					setRGB1(line, selWRed, selWGreen, selWBlue);
					setRGB2(line, selWRed, selWGreen, selWBlue);
					setRGB3(line, selWRed, selWGreen, selWBlue);
				}
				// set XY's in the GPU packet 0-1-2-3
				line->x0 = sxy0.vx;
				line->y0 = sxy0.vy;
				line->x1 = sxy1.vx;
				line->y1 = sxy1.vy;
				line->x2 = sxy2.vx;
				line->y2 = sxy2.vy;
				line->x3 = sxy0.vx;
				line->y3 = sxy0.vy;
				// Put it into the global ot at the correct place
				myAddPrimClip(avgz, drawpacket);
				// advance the global packet pointer by correct amount
				myAddPacket(sizeof(LINE_G4));
			} else {
				LINE_F4 *line = (LINE_F4 *)drawpacket;
				setLineF4(line);
				if (selFace == i)
					setRGB0(line, selWRed, selWGreen, selWBlue);
				else
					setRGB0(line, (uint8)wfrmRed, (uint8)wfrmGreen, (uint8)wfrmBlue);
				// set XY's in the GPU packet 0-1-2-3
				line->x0 = sxy0.vx;
				line->y0 = sxy0.vy;
				line->x1 = sxy1.vx;
				line->y1 = sxy1.vy;
				line->x2 = sxy2.vx;
				line->y2 = sxy2.vy;
				line->x3 = sxy0.vx;
				line->y3 = sxy0.vy;
				// Put it into the global ot at the correct place
				myAddPrimClip(avgz, drawpacket);
				// advance the global packet pointer by correct amount
				myAddPacket(sizeof(LINE_F4));
			}
		}

		// Draw textured version behind the wireframe
		if ((_drawPolys) || (selFace == i)) {
			if (_drawTxture) {
				// Set the RGB colours
				if ((_drawLit) && (_drawGouraud)) {
					POLY_GT3 *poly = (POLY_GT3 *)drawpacket;
					setPolyGT3(poly);
					// set XY's in the GPU packet 0-1-2
					poly->x0 = sxy0.vx;
					poly->y0 = sxy0.vy;
					poly->x1 = sxy1.vx;
					poly->y1 = sxy1.vy;
					poly->x2 = sxy2.vx;
					poly->y2 = sxy2.vy;

					setUV3(poly, uu0, vv0, uu1, vv1, uu2, vv2);
					if (selFace == i) {
						setRGB0(poly, selPRed, selPGreen, selPBlue);
						setRGB1(poly, selPRed, selPGreen, selPBlue);
						setRGB2(poly, selPRed, selPGreen, selPBlue);
					} else {
						setRGB0(poly, rgb0.r, rgb0.g, rgb0.b);
						setRGB1(poly, rgb1.r, rgb1.g, rgb1.b);
						setRGB2(poly, rgb2.r, rgb2.g, rgb2.b);
					}

					// Put it into the global ot at the correct place
					myAddPrimClip(avgz, drawpacket);
					// advance the global packet pointer by correct amount
					myAddPacket(sizeof(POLY_GT3));
				} else {
					POLY_FT3 *poly = (POLY_FT3 *)drawpacket;
					setPolyFT3(poly);
					// set XY's in the GPU packet 0-1-2
					poly->x0 = sxy0.vx;
					poly->y0 = sxy0.vy;
					poly->x1 = sxy1.vx;
					poly->y1 = sxy1.vy;
					poly->x2 = sxy2.vx;
					poly->y2 = sxy2.vy;

					setUV3(poly, uu0, vv0, uu1, vv1, uu2, vv2);

					if (_drawLit) {
						setRGB0(poly, rgb0.r, rgb0.g, rgb0.b);
					} else {
						// No light source calculation
						// So just set base colour
						setRGB0(poly, rgbIn.r, rgbIn.g, rgbIn.b);
					}
					if (selFace == i) {
						setRGB0(poly, selPRed, selPGreen, selPBlue);
					}

					// Put it into the global ot at the correct place
					myAddPrimClip(avgz, drawpacket);
					// advance the global packet pointer by correct amount
					myAddPacket(sizeof(POLY_FT3));
				}
			} else {
				// Set the RGB colours
				if (_drawLit) {
					if (_drawGouraud) {
						POLY_G3 *poly = (POLY_G3 *)drawpacket;
						// Draw untextured polygons
						setPolyG3(poly);
						// set XY's in the GPU packet 0-1-2
						poly->x0 = sxy0.vx;
						poly->y0 = sxy0.vy;
						poly->x1 = sxy1.vx;
						poly->y1 = sxy1.vy;
						poly->x2 = sxy2.vx;
						poly->y2 = sxy2.vy;

						if (selFace == i) {
							setRGB0(poly, selPRed, selPGreen, selPBlue);
							setRGB1(poly, selPRed, selPGreen, selPBlue);
							setRGB2(poly, selPRed, selPGreen, selPBlue);
						} else {
							setRGB0(poly, rgb0.r, rgb0.g, rgb0.b);
							setRGB1(poly, rgb1.r, rgb1.g, rgb1.b);
							setRGB2(poly, rgb2.r, rgb2.g, rgb2.b);
						}

						// Put it into the global ot at the correct place
						myAddPrimClip(avgz, drawpacket);
						// advance the global packet pointer by correct amount
						myAddPacket(sizeof(POLY_G3));
					} else {
						POLY_F3 *poly = (POLY_F3 *)drawpacket;
						// Draw untextured polygons
						setPolyF3(poly);
						// set XY's in the GPU packet 0-1-2
						poly->x0 = sxy0.vx;
						poly->y0 = sxy0.vy;
						poly->x1 = sxy1.vx;
						poly->y1 = sxy1.vy;
						poly->x2 = sxy2.vx;
						poly->y2 = sxy2.vy;

						// Flat shaded so give each vertex the same colour
						if (selFace == i) {
							setRGB0(poly, selPRed, selPGreen, selPBlue);
						} else {
							setRGB0(poly, rgb0.r, rgb0.g, rgb0.b);
						}

						// Put it into the global ot at the correct place
						myAddPrimClip(avgz, drawpacket);
						// advance the global packet pointer by correct amount
						myAddPacket(sizeof(POLY_F3));
					}
				} else {
					POLY_F3 *poly = (POLY_F3 *)drawpacket;
					// Draw untextured polygons
					setPolyF3(poly);
					// set XY's in the GPU packet 0-1-2
					poly->x0 = sxy0.vx;
					poly->y0 = sxy0.vy;
					poly->x1 = sxy1.vx;
					poly->y1 = sxy1.vy;
					poly->x2 = sxy2.vx;
					poly->y2 = sxy2.vy;

					// No light source calculation
					// So just set base colour
					if (selFace == i) {
						setRGB0(poly, selPRed, selPGreen, selPBlue);
					} else {
						setRGB0(poly, unlitPoly.r, unlitPoly.g, unlitPoly.b);
						poly->code = unlitPoly.cd;
					}
					// Put it into the global ot at the correct place
					myAddPrimClip(avgz, drawpacket);
					// advance the global packet pointer by correct amount
					myAddPacket(sizeof(POLY_F3));
				}
			}
		}
	}
}

// Simple flat untextured triangles
// draw backfacing triangles as well
// the colour is set by a global variable

void drawTRI3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex) {
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	// u_int num = min( n, 10 ); // do a maximum of 10 polygons
	u_int num = n;
	int32 p, flag;
	u_int i;
	uint32 *pPoly;
	int32 z0, z1, z2;
	SVECTORPC sxy0, sxy1, sxy2, stemp;
	uint32 tmp;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < num; i++) {
		// Each polygon is 2 32-bit WORDS
		// Bit 31 ----> Bit 0
		//
		// 16-bits | 16-bits
		// --------------------------
		//  v1     | v0
		//  pad    | v2

		tmp = *pPoly++;
		v0 = pVertex + (tmp & 0xFFFF);
		v1 = pVertex + (tmp >> 16);

		tmp = *pPoly++;
		v2 = pVertex + (tmp & 0xFFFF);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		gte_RotTransPers_pc(v0, &sxy0, &p, &flag, &z0);
		z0 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4

		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v1, &sxy1, &p, &flag, &z1);
		z1 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		gte_RotTransPers_pc(v2, &sxy2, &p, &flag, &z2);
		z2 <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		// test the return value of flag
		if (flag & 0x80000000)
			continue;

		// Give the GTE some time to recover
		int32 avgz;
		gte_AverageZ3_pc(z0, z1, z2, &avgz);

		// See what winding order this polygon is ?
		gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);

		// Drawing back facing polys requires making them have
		// clockwise winding order as by definition they have
		// anti-clockwise winding order
		if (flag < 0) {
			stemp = sxy1;
			sxy1 = sxy2;
			sxy2 = stemp;
		}

		// Put wireframe triangles in list before textured ones
		// so should be in front of textured ones
		if ((_drawWfrm) || (selFace == i)) {
			LINE_F4 *line = (LINE_F4 *)drawpacket;
			setLineF4(line);
			// set XY's in the GPU packet 0-1-2-3
			line->x0 = sxy0.vx;
			line->y0 = sxy0.vy;
			line->x1 = sxy1.vx;
			line->y1 = sxy1.vy;
			line->x2 = sxy2.vx;
			line->y2 = sxy2.vy;
			line->x3 = sxy0.vx;
			line->y3 = sxy0.vy;
			if (selFace == i)
				setRGB0(line, selWRed, selWGreen, selWBlue);
			else
				setRGB0(line, (uint8)wfrmRed, (uint8)wfrmGreen, (uint8)wfrmBlue);
			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(LINE_F4));
		}

		// Draw polygon version behind the wireframe
		if ((_drawPolys) || (selFace == i)) {
			// Draw untextured polygons
			TPOLY_F3 *poly = (TPOLY_F3 *)drawpacket;
			setTPolyF3(poly);
			setTABRMode(poly, 2);   // 2 = back - front
			setTSemiTrans(poly, 1); // 1 = enable semi-transparency

			// set XY's in the GPU packet 0-1-2
			poly->x0 = sxy0.vx;
			poly->y0 = sxy0.vy;
			poly->x1 = sxy1.vx;
			poly->y1 = sxy1.vy;
			poly->x2 = sxy2.vx;
			poly->y2 = sxy2.vy;

			// Set the RGB colours
			// No light source calculation
			// So just set base colour
			if (selFace == i) {
				setRGB0(poly, selPRed, selPGreen, selPBlue);
				poly->code = selPAlpha;
			} else {
				setRGB0(poly, unlitPoly.r, unlitPoly.g, unlitPoly.b);
				poly->code = unlitPoly.cd;
			}

			// Put it into the global ot at the correct place
			myAddPrimClip(avgz, drawpacket);
			// advance the global packet pointer by correct amount
			myAddPacket(sizeof(TPOLY_F3));
		}
	}
}

#endif // #if CD_MODE == 0

// Flat, Un-Textured, Self-Luminous, triangles
void fastDrawFUS3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex) {
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	int32 flag;
	u_int i;
	uint32 *pPoly;
	int32 z0;
	SVECTORPC sxy0, sxy1, sxy2;
	CVECTOR *rgbIn;
	uint32 tmp;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < n; i++) {
		// Structure is :
		// 0x20 | B | G | R
		// v1       | v0
		// pad      | v2
		rgbIn = (CVECTOR *)pPoly++;

		if (deadObject) {
			rgbIn = &deadObjectColour;
		}

		tmp = *pPoly++;
		v0 = pVertex + (tmp & 0xFFFF);
		v1 = pVertex + (tmp >> 16);

		tmp = *pPoly++;
		v2 = pVertex + (tmp & 0xFFFF);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		// gte_RotTransPers3_pc(v0,v1,v2,&sxy0,&sxy1,&sxy2,&p,&flag,&z0);

		// test the return value of flag
		// The vertices have been pre-transformed into screen space and stored in the vertex pool
		// The flag value is set to 0x8000 or 0 in the pad structure
		if (v0->pad != 0)
			continue;
		if (v1->pad != 0)
			continue;
		if (v2->pad != 0)
			continue;

		// use winding order of the polygons
		// e.g. NormalClip which is cross product of screen vectors
		sxy0.vx = v0->vx;
		sxy0.vy = v0->vy;
		sxy1.vx = v1->vx;
		sxy1.vy = v1->vy;
		sxy2.vx = v2->vx;
		sxy2.vy = v2->vy;

		gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
		if (flag <= 0)
			continue;

		// Draw untextured polygons
		POLY_F3 *poly = (POLY_F3 *)drawpacket;
		setPolyF3(poly);
		// set XY's in the GPU packet 0-1-2
		poly->x0 = sxy0.vx;
		poly->y0 = sxy0.vy;
		poly->x1 = sxy1.vx;
		poly->y1 = sxy1.vy;
		poly->x2 = sxy2.vx;
		poly->y2 = sxy2.vy;

		// Set the RGB colours
		// No light source calculation
		// So just set base colour
		setRGB0(poly, rgbIn->r, rgbIn->g, rgbIn->b);

		// Give the GTE some time to recover
		gte_AverageZ3_pc(v0->vz, v1->vz, v2->vz, &z0);

		// Put it into the global ot at the correct place
		myAddPrimClip(z0, drawpacket);
		// advance the global packet pointer by correct amount
		myAddPacket(sizeof(POLY_F3));
	}
}

// Fast: no options, Gouraud, Un-Textured, Self-Luminous, triangles
void fastDrawGUS3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex) {
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	uint32 tmp;
	int32 flag;
	u_int i;
	uint32 *pPoly;
	int32 z0;
	SVECTORPC sxy0, sxy1, sxy2;

	// Base colour for gouraud shading computation
	// Could easily be specified per polygon
	CVECTOR *rgb0;
	CVECTOR *rgb1;
	CVECTOR *rgb2;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < n; i++) {
		// Structure is :
		// 8-bits | 8-bits | 8-bits | 8-bits
		// 0x30   | Blue0  | Green0 | Red0
		// 0x30   | Blue1  | Green1 | Red1
		// 0x30   | Blue2  | Green2 | Red2
		// 16-bits | 8-bits | 8-bits
		// --------------------------
		//  v1              | v0
		//  pad             | v2
		rgb0 = (CVECTOR *)pPoly++;
		rgb1 = (CVECTOR *)pPoly++;
		rgb2 = (CVECTOR *)pPoly++;

		if (deadObject) {
			rgb0 = &deadObjectColour;
			rgb1 = &deadObjectColour;
			rgb2 = &deadObjectColour;
		}

		tmp = *pPoly++;
		v0 = pVertex + (tmp & 0xFFFF);
		v1 = pVertex + (tmp >> 16);

		tmp = *pPoly++;
		v2 = pVertex + (tmp & 0xFFFF);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		// gte_RotTransPers3(v0,v1,v2,&sxy0,&sxy1,&sxy2,&p,&flag,&z0);

		// test the return value of flag
		// The vertices have been pre-transformed into screen space and stored in the vertex pool
		// The flag value is set to 0x8000 or 0 in the pad structure
		if (v0->pad != 0)
			continue;
		if (v1->pad != 0)
			continue;
		if (v2->pad != 0)
			continue;

		// use winding order of the polygons
		// e.g. NormalClip which is cross product of screen vectors
		sxy0.vx = v0->vx;
		sxy0.vy = v0->vy;
		sxy1.vx = v1->vx;
		sxy1.vy = v1->vy;
		sxy2.vx = v2->vx;
		sxy2.vy = v2->vy;

		gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
		if (flag <= 0)
			continue;

		POLY_G3 *poly = (POLY_G3 *)drawpacket;
		// Draw untextured polygons
		setPolyG3(poly);
		// set XY's in the GPU packet 0-1-2
		poly->x0 = sxy0.vx;
		poly->y0 = sxy0.vy;
		poly->x1 = sxy1.vx;
		poly->y1 = sxy1.vy;
		poly->x2 = sxy2.vx;
		poly->y2 = sxy2.vy;

		setRGB0(poly, rgb0->r, rgb0->g, rgb0->b);
		setRGB1(poly, rgb1->r, rgb1->g, rgb1->b);
		setRGB2(poly, rgb2->r, rgb2->g, rgb2->b);

		// Give the GTE some time to recover
		gte_AverageZ3_pc(v0->vz, v1->vz, v2->vz, &z0);

		// Put it into the global ot at the correct place
		myAddPrimClip(z0, drawpacket);
		// advance the global packet pointer by correct amount
		myAddPacket(sizeof(POLY_G3));
	}
}

// Fast: no options, Flat, Textured, Self-Luminous Triangles
void fastDrawFTS3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex) {
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	u_short uu0, uu1, uu2;
	u_short vv0, vv1, vv2;
	uint32 tmp;
	int32 flag;
	u_int i;
	uint32 *pPoly;
	int32 z0;
	SVECTORPC sxy0, sxy1, sxy2;
	uint32 uv0, uv1, uv2;

	// Base colour for shading computation
	// Could easily be specified per polygon
	CVECTOR *rgbIn;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < n; i++) {
		// Structure is :
		// Bit 31 ----> Bit 0
		//
		// 8-bits | 8-bits | 8-bits | 8-bits
		// 0x24   | Blue   | Green  | Red
		// 16-bits | 16-bits
		// --------------------------
		// v0      | u0
		// v1      | u1
		// v2      | u2
		// --------------------------
		// pad     | v0
		//  v2     | v1
		rgbIn = (CVECTOR *)pPoly++;

		if (deadObject == 1) {
			rgbIn = &deadObjectColour;
		}

		uv0 = *pPoly++;
		vv0 = (u_short)((uv0 >> 16) & 0xFFFF);
		uu0 = (u_short)(uv0 & 0xFFFF);

		uv1 = *pPoly++;
		vv1 = (u_short)((uv1 >> 16) & 0xFFFF);
		uu1 = (u_short)(uv1 & 0xFFFF);

		uv2 = *pPoly++;
		vv2 = (u_short)((uv2 >> 16) & 0xFFFF);
		uu2 = (u_short)(uv2 & 0xFFFF);

		tmp = *pPoly++;
		v0 = pVertex + (tmp & 0xFFFF);

		tmp = *pPoly++;
		v1 = pVertex + (uint)(tmp & 0xFFFF);
		v2 = pVertex + (uint)(tmp >> 16);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		// gte_RotTransPers3_pc(v0,v1,v2,&sxy0,&sxy1,&sxy2,&p,&flag,&z0);

		// test the return value of flag
		// The vertices have been pre-transformed into screen space and stored in the vertex pool
		// The flag value is set to 0x8000 or 0 in the pad structure
		if (v0->pad != 0)
			continue;
		if (v1->pad != 0)
			continue;
		if (v2->pad != 0)
			continue;

		// use winding order of the polygons
		// e.g. NormalClip which is cross product of screen vectors
		sxy0.vx = v0->vx;
		sxy0.vy = v0->vy;
		sxy1.vx = v1->vx;
		sxy1.vy = v1->vy;
		sxy2.vx = v2->vx;
		sxy2.vy = v2->vy;

		gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
		if (flag <= 0)
			continue;

		POLY_FT3 *poly = (POLY_FT3 *)drawpacket;
		setPolyFT3(poly);
		// set XY's in the GPU packet 0-1-2
		poly->x0 = sxy0.vx;
		poly->y0 = sxy0.vy;
		poly->x1 = sxy1.vx;
		poly->y1 = sxy1.vy;
		poly->x2 = sxy2.vx;
		poly->y2 = sxy2.vy;

		setUV3(poly, uu0, vv0, uu1, vv1, uu2, vv2);

		// No light source calculation so just set base colour
		setRGB0(poly, rgbIn->r, rgbIn->b, rgbIn->b);

		// Give the GTE some time to recover
		gte_AverageZ3_pc(v0->vz, v1->vz, v2->vz, &z0);

		// Put it into the global ot at the correct place
		myAddPrimClip(z0, drawpacket);
		// advance the global packet pointer by correct amount
		myAddPacket(sizeof(POLY_FT3));
	}
}

// Fast: no options, Gouraud, Textured, Self-Luminous Triangles
void fastDrawGTS3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex) {
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	u_short uu0, uu1, uu2;
	u_short vv0, vv1, vv2;
	uint32 tmp;
	int32 flag;
	u_int i;
	uint32 *pPoly;
	int32 z0;
	SVECTORPC sxy0, sxy1, sxy2;
	uint32 uv0, uv1, uv2;

	// gouraud colours
	CVECTOR *rgb0;
	CVECTOR *rgb1;
	CVECTOR *rgb2;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < n; i++) {
		// Structure is :
		// 8-bits | 8-bits | 8-bits | 8-bits
		// 0x34   | Blue0  | Green0 | Red0
		// 0x34   | Blue1  | Green1 | Red1
		// 0x34   | Blue2  | Green2 | Red2
		// 16-bits | 16-bits
		// --------------------------
		// v0      | u0
		// v1      | u1
		// v2      | u2
		// --------------------------
		// pad     | v0
		//  v2     | v1
		rgb0 = (CVECTOR *)pPoly++;
		rgb1 = (CVECTOR *)pPoly++;
		rgb2 = (CVECTOR *)pPoly++;

		if (deadObject) {
			rgb0 = &deadObjectColour;
			rgb1 = &deadObjectColour;
			rgb2 = &deadObjectColour;
		}

		uv0 = *pPoly++;
		vv0 = (u_short)((uv0 >> 16) & 0xFFFF);
		uu0 = (u_short)(uv0 & 0xFFFF);

		uv1 = *pPoly++;
		vv1 = (u_short)((uv1 >> 16) & 0xFFFF);
		uu1 = (u_short)(uv1 & 0xFFFF);

		uv2 = *pPoly++;
		vv2 = (u_short)((uv2 >> 16) & 0xFFFF);
		uu2 = (u_short)(uv2 & 0xFFFF);

		tmp = *pPoly++;
		v0 = pVertex + (tmp & 0xFFFF);

		tmp = *pPoly++;
		v1 = pVertex + (tmp & 0xFFFF);
		v2 = pVertex + (tmp >> 16);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		// gte_RotTransPers3_pc(v0,v1,v2,&sxy0,&sxy1,&sxy2,&p,&flag,&z0);

		// test the return value of flag
		// The vertices have been pre-transformed into screen space and stored in the vertex pool
		// The flag value is set to 0x8000 or 0 in the pad structure
		if (v0->pad != 0)
			continue;
		if (v1->pad != 0)
			continue;
		if (v2->pad != 0)
			continue;

		// use winding order of the polygons
		// e.g. NormalClip which is cross product of screen vectors
		sxy0.vx = v0->vx;
		sxy0.vy = v0->vy;
		sxy1.vx = v1->vx;
		sxy1.vy = v1->vy;
		sxy2.vx = v2->vx;
		sxy2.vy = v2->vy;

		// use winding order of the polygons
		// e.g. NormalClip which is cross product of screen vectors
		gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
		if (flag <= 0)
			continue;

		// Set the RGB colours
		POLY_GT3 *poly = (POLY_GT3 *)drawpacket;
		setPolyGT3(poly);
		// set XY's in the GPU packet 0-1-2
		poly->x0 = sxy0.vx;
		poly->y0 = sxy0.vy;
		poly->x1 = sxy1.vx;
		poly->y1 = sxy1.vy;
		poly->x2 = sxy2.vx;
		poly->y2 = sxy2.vy;

		setUV3(poly, uu0, vv0, uu1, vv1, uu2, vv2);

		setRGB0(poly, rgb0->r, rgb0->b, rgb0->b);
		setRGB1(poly, rgb1->r, rgb1->b, rgb1->b);
		setRGB2(poly, rgb2->r, rgb2->b, rgb2->b);

		// Give the GTE some time to recover
		gte_AverageZ3_pc(v0->vz, v1->vz, v2->vz, &z0);

		// Put it into the global ot at the correct place
		myAddPrimClip(z0, drawpacket);

		// advance the global packet pointer by correct amount
		myAddPacket(sizeof(POLY_GT3));
	}
}

// Fast: no options, Flat, Un-Textured, Lit, triangles
void fastDrawFUL3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex, SVECTOR *pNormal) {
	SVECTOR *n0;
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	int32 flag;
	u_int i;
	uint32 *pPoly;
	int32 z0;
	SVECTORPC sxy0, sxy1, sxy2;
	CVECTOR *rgbIn;
	CVECTOR rgb0;
	uint32 tmp;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < n; i++) {
		// Structure is :
		// 0x20 | B | G | R
		// v0       | n0
		// v2       | v1
		rgbIn = (CVECTOR *)pPoly++;

		tmp = *pPoly++;
		v0 = pVertex + (tmp >> 16);
		n0 = pNormal + (tmp & 0xFFFF);

		tmp = *pPoly++;
		v2 = pVertex + (tmp >> 16);
		v1 = pVertex + (tmp & 0xFFFF);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		// gte_RotTransPers3_pc(v0,v1,v2,&sxy0,&sxy1,&sxy2,&p,&flag,&z0);

		// test the return value of flag
		// The vertices have been pre-transformed into screen space and stored in the vertex pool
		// The flag value is set to 0x8000 or 0 in the pad structure
		if (v0->pad != 0)
			continue;
		if (v1->pad != 0)
			continue;
		if (v2->pad != 0)
			continue;

		// use winding order of the polygons
		// e.g. NormalClip which is cross product of screen vectors
		sxy0.vx = v0->vx;
		sxy0.vy = v0->vy;
		sxy1.vx = v1->vx;
		sxy1.vy = v1->vy;
		sxy2.vx = v2->vx;
		sxy2.vy = v2->vy;

		gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
		if (flag <= 0)
			continue;

		// Do the flat lighting computation
		LIGHTPOLYGON(n0, rgbIn, &rgb0);

		// Draw untextured polygons
		POLY_F3 *poly = (POLY_F3 *)drawpacket;
		setPolyF3(poly);

		// set XY's in the GPU packet 0-1-2
		poly->x0 = sxy0.vx;
		poly->y0 = sxy0.vy;
		poly->x1 = sxy1.vx;
		poly->y1 = sxy1.vy;
		poly->x2 = sxy2.vx;
		poly->y2 = sxy2.vy;

		// Set the RGB colours
		// Flat shaded so give each vertex the same colour
		setRGB0(poly, rgb0.r, rgb0.g, rgb0.b);

		// Give the GTE some time to recover
		gte_AverageZ3_pc(v0->vz, v1->vz, v2->vz, &z0);

		// Put it into the global ot at the correct place
		myAddPrimClip(z0, drawpacket);

		// advance the global packet pointer by correct amount
		myAddPacket(sizeof(POLY_F3));
	}
}

// Fast: no options, Gouraud, Un-Textured, Lit, triangles
void fastDrawGUL3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex, SVECTOR *pNormal) {
	SVECTOR *n0;
	SVECTOR *n1;
	SVECTOR *n2;
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	uint32 tmp;
	int32 flag;
	u_int i;
	uint32 *pPoly;
	int32 z0;
	SVECTORPC sxy0, sxy1, sxy2;

	// Base colour for gouraud shading computation
	// Could easily be specified per polygon
	CVECTOR rgbIn = {128, 128, 128, 0};
	CVECTOR rgb0, rgb1, rgb2;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < n; i++) {
		// Structure is :
		// 8-bits | 8-bits | 8-bits | 8-bits
		// 0x20   | Blue   | Green  | Red
		// 16-bits | 8-bits | 8-bits
		// --------------------------
		//  v0              | n0
		//  v1              | n1
		//  v2              | n2
		tmp = *pPoly++;
		// code0 = tmp >> 24;
		rgbIn.r = (uint8)(tmp & 0xFF);
		rgbIn.g = (uint8)((tmp >> 8) & 0xFF);
		rgbIn.b = (uint8)((tmp >> 16) & 0xFF);

		tmp = *pPoly++;
		v0 = pVertex + (tmp >> 16);
		n0 = pNormal + (tmp & 0xFFFF);

		tmp = *pPoly++;
		v1 = pVertex + (tmp >> 16);
		n1 = pNormal + (tmp & 0xFFFF);

		tmp = *pPoly++;
		v2 = pVertex + (tmp >> 16);
		n2 = pNormal + (tmp & 0xFFFF);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		// gte_RotTransPers3_pc(v0,v1,v2,&sxy0,&sxy1,&sxy2,&p,&flag,&z0);

		// test the return value of flag
		// The vertices have been pre-transformed into screen space and stored in the vertex pool
		// The flag value is set to 0x8000 or 0 in the pad structure
		if (v0->pad != 0)
			continue;
		if (v1->pad != 0)
			continue;
		if (v2->pad != 0)
			continue;

		// use winding order of the polygons
		// e.g. NormalClip which is cross product of screen vectors
		sxy0.vx = v0->vx;
		sxy0.vy = v0->vy;
		sxy1.vx = v1->vx;
		sxy1.vy = v1->vy;
		sxy2.vx = v2->vx;
		sxy2.vy = v2->vy;

		gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
		if (flag <= 0)
			continue;

		// Do the full gouraud computation
		// NormalColorCol3( n0, n1, n2, &rgbIn, &rgb0, &rgb1, &rgb2 );
		LIGHTPOLYGON3(n0, n1, n2, &rgbIn, &rgb0, &rgb1, &rgb2);

		POLY_G3 *poly = (POLY_G3 *)drawpacket;
		// Draw untextured polygons
		setPolyG3(poly);
		// set XY's in the GPU packet 0-1-2
		poly->x0 = sxy0.vx;
		poly->y0 = sxy0.vy;
		poly->x1 = sxy1.vx;
		poly->y1 = sxy1.vy;
		poly->x2 = sxy2.vx;
		poly->y2 = sxy2.vy;

		setRGB0(poly, rgb0.r, rgb0.g, rgb0.b);
		setRGB1(poly, rgb1.r, rgb1.g, rgb1.b);
		setRGB2(poly, rgb2.r, rgb2.g, rgb2.b);

		// Give the GTE some time to recover
		gte_AverageZ3_pc(v0->vz, v1->vz, v2->vz, &z0);

		// Put it into the global ot at the correct place
		myAddPrimClip(z0, drawpacket);
		// advance the global packet pointer by correct amount
		myAddPacket(sizeof(POLY_G3));
	}
}

//------------------------------------------------------------------------

// Fast: no options, Flat, Textured, Lit Triangles
void fastDrawFTL3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex, SVECTOR *pNormal) {
	SVECTOR *n0;
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	u_short uu0, uu1, uu2;
	u_short vv0, vv1, vv2;
	uint32 tmp;
	int32 flag;
	u_int i;
	uint32 *pPoly;
	int32 z0;
	SVECTORPC sxy0, sxy1, sxy2;
	uint32 uv0, uv1, uv2;

	// Base colour for shading computation
	// Could easily be specified per polygon
	CVECTOR rgbIn = {128, 128, 128, 0};
	CVECTOR rgb0;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < n; i++) {
		// Structure is :
		// Bit 31 ----> Bit 0
		//
		// 16-bits | 16-bits
		// --------------------------
		// v0      | u0
		// v1      | u1
		// v2      | u2
		// --------------------------
		//  v0     | n0
		//  v2     | v1
		uv0 = *pPoly++;
		vv0 = (u_short)((uv0 >> 16) & 0xFFFF);
		uu0 = (u_short)(uv0 & 0xFFFF);

		uv1 = *pPoly++;
		vv1 = (u_short)((uv1 >> 16) & 0xFFFF);
		uu1 = (u_short)(uv1 & 0xFFFF);

		uv2 = *pPoly++;
		vv2 = (u_short)((uv2 >> 16) & 0xFFFF);
		uu2 = (u_short)(uv2 & 0xFFFF);

		tmp = *pPoly++;
		v0 = pVertex + (tmp >> 16);
		n0 = pNormal + (tmp & 0xFFFF);

		tmp = *pPoly++;
		v1 = pVertex + (tmp & 0xFFFF);
		v2 = pVertex + (tmp >> 16);

		// Now do RotTransPers3 on the vectors
		// z0 = RotTransPers3( v0, v1, v2, &sxy0, &sxy1, &sxy2, &p, &flag );
		// gte_RotTransPers3_pc(v0,v1,v2,&sxy0,&sxy1,&sxy2,&p,&flag,&z0);

		// test the return value of flag
		// The vertices have been pre-transformed into screen space and stored in the vertex pool
		// The flag value is set to 0x8000 or 0 in the pad structure
		if (v0->pad != 0)
			continue;
		if (v1->pad != 0)
			continue;
		if (v2->pad != 0)
			continue;

		// use winding order of the polygons
		// e.g. NormalClip which is cross product of screen vectors
		sxy0.vx = v0->vx;
		sxy0.vy = v0->vy;
		sxy1.vx = v1->vx;
		sxy1.vy = v1->vy;
		sxy2.vx = v2->vx;
		sxy2.vy = v2->vy;

		gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);
		if (flag <= 0)
			continue;

		// Do the full gouraud computation
		// NormalColorCol( n0, &rgbIn, &rgb0 );
		LIGHTPOLYGON(n0, &rgbIn, &rgb0);

		POLY_FT3 *poly = (POLY_FT3 *)drawpacket;
		setPolyFT3(poly);
		// set XY's in the GPU packet 0-1-2
		poly->x0 = sxy0.vx;
		poly->y0 = sxy0.vy;
		poly->x1 = sxy1.vx;
		poly->y1 = sxy1.vy;
		poly->x2 = sxy2.vx;
		poly->y2 = sxy2.vy;

		setUV3(poly, uu0, vv0, uu1, vv1, uu2, vv2);
		setRGB0(poly, rgb0.r, rgb0.g, rgb0.b);

		// Give the GTE some time to recover
		gte_AverageZ3_pc(v0->vz, v1->vz, v2->vz, &z0);

		// Put it into the global ot at the correct place
		myAddPrimClip(z0, drawpacket);
		// advance the global packet pointer by correct amount
		myAddPacket(sizeof(POLY_FT3));
	}
}

// Fast : no options : Gouraud, Textured, Lit Triangles
void fastDrawGTL3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex, SVECTOR *pNormal) {
	SVECTOR *n0;
	SVECTOR *n1;
	SVECTOR *n2;
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	u_short uu0;
	u_short uu1;
	u_short uu2;
	u_short vv0;
	u_short vv1;
	u_short vv2;
	int32 flag;
	u_int i;
	int32 z0;
	uint32 uv0;
	uint32 uv1;
	uint32 uv2;
	uint32 vt0;
	uint32 vt1;
	uint32 vt2;
	SVECTORPC sxy0;
	SVECTORPC sxy1;
	SVECTORPC sxy2;
	int pad0;
	int pad1;
	int pad2;

	// Base colour for gouraud shading computation
	// Could easily be specified per polygon
	CVECTOR rgbIn = {128, 128, 128, 0};
	CVECTOR rgb0, rgb1, rgb2;

	int my_minUsedZpos;
	int my_maxUsedZpos;

	my_minUsedZpos = minUsedZpos;
	my_maxUsedZpos = maxUsedZpos;

	POLY_GT3 *poly;
	uint32 *pPoly;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < n; i++) {
		// Structure is :
		// v0, u0
		// v1, u1
		// v2, u2
		// v0     | n0
		// v1     | n1
		// v2     | n2

		uv0 = *(pPoly++);
		vv0 = (u_short)((uv0 >> 16) & 0xFFFF);
		uu0 = (u_short)(uv0 & 0xFFFF);

		uv1 = *(pPoly++);
		vv1 = (u_short)((uv1 >> 16) & 0xFFFF);
		uu1 = (u_short)(uv1 & 0xFFFF);

		uv2 = *(pPoly++);
		vv2 = (u_short)((uv2 >> 16) & 0xFFFF);
		uu2 = (u_short)(uv2 & 0xFFFF);

		vt0 = *(pPoly++);
		v0 = (pVertex + (vt0 >> 16));
		n0 = (pNormal + (vt0 & 0xFFFF));

		vt1 = *(pPoly++);
		v1 = (pVertex + (vt1 >> 16));
		n1 = (pNormal + (vt1 & 0xFFFF));

		vt2 = *(pPoly++);
		v2 = (pVertex + (vt2 >> 16));
		n2 = (pNormal + (vt2 & 0xFFFF));

		// The vertices have been pre-transformed into screen space and stored in the vertex pool
		// The flag value is set to 0x8000 or 0 in the pad structure
		pad0 = v0->pad;
		pad1 = v1->pad;
		pad2 = v2->pad;
		flag = pad0 | pad1 | pad2;

		if (flag != 0)
			continue;

		// use winding order of the polygons
		// e.g. NormalClip which is cross product of screen vectors
		sxy0.vx = v0->vx;
		sxy0.vy = v0->vy;
		sxy1.vx = v1->vx;
		sxy1.vy = v1->vy;
		sxy2.vx = v2->vx;
		sxy2.vy = v2->vy;

		gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);

		// Give the GTE some time to recover

		// Ignore polygons which are too small
		if (flag <= 0)
			continue;

		// Do the full gouraud computation
		LIGHTPOLYGON3(n0, n1, n2, &rgbIn, &rgb0, &rgb1, &rgb2);

		// Try to overlap this with the LIGHTPOLYGON3 !
		poly = (POLY_GT3 *)drawpacket;
		setPolyGT3(poly);
		// set XY's in the GPU packet 0-1-2
		poly->x0 = sxy0.vx;
		poly->y0 = sxy0.vy;
		poly->x1 = sxy1.vx;
		poly->y1 = sxy1.vy;
		poly->x2 = sxy2.vx;
		poly->y2 = sxy2.vy;

		// Give the GTE some time to recover
		gte_AverageZ3_pc(v0->vz, v1->vz, v2->vz, &z0);

		setUV3(poly, uu0, vv0, uu1, vv1, uu2, vv2);

		setRGB0(poly, rgb0.r, rgb0.g, rgb0.b);
		setRGB1(poly, rgb1.r, rgb1.g, rgb1.b);
		setRGB2(poly, rgb2.r, rgb2.g, rgb2.b);

		// Put it into the global ot at the correct place
		myAddPrimClip(z0, drawpacket);

		// advance the global packet pointer by correct amount
		myAddPacket(sizeof(POLY_GT3));
	}
}

// For mucking about with lighting
// Draw a solid bounding box around the actor (and so obscuring him)
// but then have the box respond to the lights e.g. give it normals
// and do flat & goraud shading
// Assume the GTE lighting registers to be pre-prepared

#if CD_MODE == 0

void drawSolidBboxPC(SVECTOR *scrn, CVECTOR *rgbIn) {
	// 6 POLY_F4's i.e. 6 faces
	// The 8 verticies are:
	// 0 { xmin, ymin, zmin },
	// 1 { xmin, ymin, zmax },
	// 2 { xmax, ymin, zmin },
	// 3 { xmax, ymin, zmax },
	// 4 { xmax, ymax, zmin },
	// 5 { xmax, ymax, zmax },
	// 6 { xmin, ymax, zmin },
	// 7 { xmin, ymax, zmax }

	// The 6 faces are the following vertex links:
	// ymin face : 0->1->3->2
	// xmax face : 2->3->5->4
	// zmin face : 0->2->4->6
	// xmin face : 0->6->7->1
	// zmax face : 1->7->5->3
	// ymax face : 6->4->5->7

	// The 6 face normals are (normal points into front side of face on PSX)
	// ymin face : 0,+4096,0 +ve y
	// xmax face : -4096,0,0 -ve x
	// zmin face : 0,0,+4096 +ve z
	// xmin face : +4096,0,0 +ve x
	// zmax face : 0,0,-4096 -ve z
	// ymax face : 0,-4096,0 -ve y

	int32 z0, flag;
	SVECTOR normal;
	CVECTOR rgb;
	POLY_F4 *face = NULL;

	gte_NormalClip_pc(&(scrn[0]), &(scrn[1]), &(scrn[2]), &flag);
	if (flag > 0) {
		face = (POLY_F4 *)drawpacket;
		setPolyF4(face);
		// ymin face normal : 0,+4096,0 +ve y
		normal.vx = 0;
		normal.vy = 4096;
		normal.vz = 0;
		if (_drawLit) {
			LIGHTPOLYGON(&normal, rgbIn, &rgb);
			setRGB0(face, rgb.r, rgb.g, rgb.b);
		} else {
			setRGB0(face, rgbIn->r, rgbIn->g, rgbIn->b);
		}
		// ymin face : 0->1->3->2 : note PSX is 0-1-3-2 for a quad
		setXY4(face, scrn[0].vx, scrn[0].vy, scrn[1].vx, scrn[1].vy, scrn[2].vx, scrn[2].vy, scrn[3].vx, scrn[3].vy);

		z0 = (scrn[0].vz + scrn[1].vz + scrn[2].vz + scrn[3].vz) / 4;
		z0 = myAddPrimClip(z0, drawpacket);
		myAddPacket(sizeof(POLY_F4));
	}

	gte_NormalClip_pc(&(scrn[2]), &(scrn[3]), &(scrn[4]), &flag);
	if (flag > 0) {
		// xmax face : 2->3->5->4 : note PSX is 0-1-3-2 for a quad
		face = (POLY_F4 *)drawpacket;
		setPolyF4(face);
		// xmax face : -4096,0,0 -ve x
		normal.vx = -4096;
		normal.vy = 0;
		normal.vz = 0;
		if (_drawLit) {
			LIGHTPOLYGON(&normal, rgbIn, &rgb);
			setRGB0(face, rgb.r, rgb.g, rgb.b);
		} else {
			setRGB0(face, rgbIn->r, rgbIn->g, rgbIn->b);
		}
		setXY4(face, scrn[2].vx, scrn[2].vy, scrn[3].vx, scrn[3].vy, scrn[4].vx, scrn[4].vy, scrn[5].vx, scrn[5].vy);
		z0 = (scrn[2].vz + scrn[3].vz + scrn[4].vz + scrn[5].vz) / 4;
		z0 = myAddPrimClip(z0, drawpacket);
		myAddPacket(sizeof(POLY_F4));
	}

	gte_NormalClip_pc(&(scrn[0]), &(scrn[2]), &(scrn[4]), &flag);
	if (flag > 0) {
		// zmin face : 0->2->4->6 : note PSX is 0-1-3-2 for a quad
		face = (POLY_F4 *)drawpacket;
		setPolyF4(face);
		// zmin face : 0,0,+4096 +ve z
		normal.vx = 0;
		normal.vy = 0;
		normal.vz = 4096;
		if (_drawLit) {
			LIGHTPOLYGON(&normal, rgbIn, &rgb);
			setRGB0(face, rgb.r, rgb.g, rgb.b);
		} else {
			setRGB0(face, rgbIn->r, rgbIn->g, rgbIn->b);
		}
		setXY4(face, scrn[0].vx, scrn[0].vy, scrn[2].vx, scrn[2].vy, scrn[6].vx, scrn[6].vy, scrn[4].vx, scrn[4].vy);
		z0 = (scrn[0].vz + scrn[2].vz + scrn[4].vz + scrn[6].vz) / 4;
		z0 = myAddPrimClip(z0, drawpacket);
		myAddPacket(sizeof(POLY_F4));
	}

	gte_NormalClip_pc(&(scrn[0]), &(scrn[6]), &(scrn[7]), &flag);
	if (flag > 0) {
		// xmin face : 0->6->7->1 : note PSX is 0-1-3-2 for a quad
		face = (POLY_F4 *)drawpacket;
		setPolyF4(face);
		// xmin face : +4096,0,0 +ve x
		normal.vx = 0;
		normal.vy = 0;
		normal.vz = 4096;
		if (_drawLit) {
			LIGHTPOLYGON(&normal, rgbIn, &rgb);
			setRGB0(face, rgb.r, rgb.g, rgb.b);
		} else {
			setRGB0(face, rgbIn->r, rgbIn->g, rgbIn->b);
		}
		setXY4(face, scrn[0].vx, scrn[0].vy, scrn[6].vx, scrn[6].vy, scrn[1].vx, scrn[1].vy, scrn[7].vx, scrn[7].vy);
		z0 = myAddPrimClip(scrn[7].vz, drawpacket);
		myAddPacket(sizeof(POLY_F4));
	}

	gte_NormalClip_pc(&(scrn[1]), &(scrn[7]), &(scrn[5]), &flag);
	if (flag > 0) {
		// zmax face : 1->7->5->3 : note PSX is 0-1-3-2 for a quad
		face = (POLY_F4 *)drawpacket;
		setPolyF4(face);
		// zmax face : 0,0,-4096 -ve z
		normal.vx = 0;
		normal.vy = 0;
		normal.vz = -4096;
		if (_drawLit) {
			LIGHTPOLYGON(&normal, rgbIn, &rgb);
			setRGB0(face, rgb.r, rgb.g, rgb.b);
		} else {
			setRGB0(face, rgbIn->r, rgbIn->g, rgbIn->b);
		}
		setXY4(face, scrn[1].vx, scrn[1].vy, scrn[7].vx, scrn[7].vy, scrn[3].vx, scrn[3].vy, scrn[5].vx, scrn[5].vy);
		z0 = (scrn[1].vz + scrn[3].vz + scrn[5].vz + scrn[7].vz) / 4;
		z0 = myAddPrimClip(z0, drawpacket);
		myAddPacket(sizeof(POLY_F4));
	}

	gte_NormalClip_pc(&(scrn[6]), &(scrn[4]), &(scrn[5]), &flag);
	if (flag > 0) {
		// ymax face : 6->4->5->7 : note PSX is 0-1-3-2 for a quad
		face = (POLY_F4 *)drawpacket;
		setPolyF4(face);
		// ymax face : 0,-4096,0 -ve y
		normal.vx = 0;
		normal.vy = -4096;
		normal.vz = 0;
		if (_drawLit) {
			LIGHTPOLYGON(&normal, rgbIn, &rgb);
			setRGB0(face, rgb.r, rgb.g, rgb.b);
		} else {
			setRGB0(face, rgbIn->r, rgbIn->g, rgbIn->b);
		}
		setXY4(face, scrn[6].vx, scrn[6].vy, scrn[4].vx, scrn[4].vy, scrn[7].vx, scrn[7].vy, scrn[5].vx, scrn[5].vy);
		z0 = (scrn[4].vz + scrn[5].vz + scrn[6].vz + scrn[7].vz) / 4;
		z0 = myAddPrimClip(z0, drawpacket);
		myAddPacket(sizeof(POLY_F4));
	}
}

#endif // #if CD_MODE == 0

// Simple flat untextured triangles
// draw backfacing triangles as well
// the colour is set by a global variable
void fastDrawTRI3PC(uint32 *polyStart, const u_int n, SVECTORPC *pVertex) {
	SVECTORPC *v0;
	SVECTORPC *v1;
	SVECTORPC *v2;

	int32 flag;
	u_int i;
	uint32 *pPoly;
	int32 z0;
	SVECTORPC sxy0, sxy1, sxy2, stemp;
	uint32 tmp;

	pPoly = polyStart;
	// Loop over each polygon
	for (i = 0; i < n; i++) {
		// Each polygon is 2 32-bit WORDS
		// Bit 31 ----> Bit 0
		//
		// 16-bits | 16-bits
		// --------------------------
		//  v1     | v0
		//  pad    | v2

		tmp = *pPoly++;
		v0 = pVertex + (tmp & 0xFFFF);
		v1 = pVertex + (tmp >> 16);

		tmp = *pPoly++;
		v2 = pVertex + (tmp & 0xFFFF);

		flag = (v0->pad) | (v1->pad) | (v2->pad);
		if (flag != 0)
			continue;

		// use winding order of the polygons
		// e.g. NormalClip which is cross product of screen vectors
		sxy0.vx = v0->vx;
		sxy0.vy = v0->vy;
		sxy1.vx = v1->vx;
		sxy1.vy = v1->vy;
		sxy2.vx = v2->vx;
		sxy2.vy = v2->vy;

		// See what winding order this polygon is ?
		gte_NormalClip_pc(&sxy0, &sxy1, &sxy2, &flag);

		// Drawing back facing polys requires making them have
		// clockwise winding order as by definition they have
		// anti-clockwise winding order
		if (flag < 0) {
			stemp = sxy1;
			sxy1 = sxy2;
			sxy2 = stemp;
		}

		// Give the GTE some time to recover
		gte_AverageZ3_pc(v0->vz, v1->vz, v2->vz, &z0);

		// Draw untextured polygons
		TPOLY_F3 *poly = (TPOLY_F3 *)drawpacket;
		setTPolyF3(poly);
		setTABRMode(poly, 2);   // 2 = back - front
		setTSemiTrans(poly, 1); // 1 = enable semi-transparency

		// set XY's in the GPU packet 0-1-2
		poly->x0 = sxy0.vx;
		poly->y0 = sxy0.vy;
		poly->x1 = sxy1.vx;
		poly->y1 = sxy1.vy;
		poly->x2 = sxy2.vx;
		poly->y2 = sxy2.vy;

		// Set the RGB colours
		// No light source calculation
		// So just set base colour
		setRGB0(poly, unlitPoly.r, unlitPoly.g, unlitPoly.b);
		poly->code = unlitPoly.cd;

		myAddPrimClip(z0, drawpacket);

		// advance the global packet pointer by correct amount
		myAddPacket(sizeof(TPOLY_F3));
	}
}

} // End of namespace ICB
