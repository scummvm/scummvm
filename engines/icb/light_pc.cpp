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
#include "engines/icb/p4.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/global_objects_psx.h"
#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/common/px_capri_maths.h"
#include "engines/icb/drawpoly_pc.h"
#include "engines/icb/light_pc.h"
#include "engines/icb/shade_pc.h"

namespace ICB {

#if _PSX_ON_PC == 1

// Globals telling you "extra" parameters about the lights
extern uint32 useLampWidth;
extern uint32 useLampBounce;
extern int32 lampWidth[3];
extern int32 lampBounce[3];

#else // #if _PSX_ON_PC == 1

// Globals telling you "extra" parameters about the lights
uint32 useLampWidth;
uint32 useLampBounce;
int32 lampWidth[3];
int32 lampBounce[3];

#endif // #if _PSX_ON_PC == 1

//------------------------------------------------------------------------
// Set the GTE constant light variables
// Sets the direciton lights colour matrix
// Sets the ambient colour
// Fills in the lightDirects matrix

// Selects the 3 brightest lamps from the lamplist
// at the actors position and puts them into the
// GTE light matrix and the lightDirects matrix
//

int prepareLightsPC(VECTOR *pos, PSXrgb *ambient, PSXLampList *lamplist, PSXShadeList *shadelist, MATRIXPC *lDirects, LampInfo *linfo) {
	// Disable the slow features as default
	useLampBounce = 0;
	useLampWidth = 0;
	// Zero out the "extra" parameters
	lampWidth[0] = 0;
	lampWidth[1] = 0;
	lampWidth[2] = 0;
	lampBounce[0] = 0;
	lampBounce[1] = 0;
	lampBounce[2] = 0;

	if (lamplist == NULL) {
		return prepareLightsGlobalPC(lDirects);
	}

	LampInfo lampInfo[MAX_NUMBER_LIGHTS + 1];

	// Turn them off
	linfo[0].intens = 0;
	linfo[1].intens = 0;
	linfo[2].intens = 0;

	uint32 brightest = MAX_NUMBER_LIGHTS;
	uint32 middle = MAX_NUMBER_LIGHTS;
	uint32 darkest = MAX_NUMBER_LIGHTS;
	lampInfo[brightest].intens = 0;
	lampInfo[brightest].index = MAX_NUMBER_LIGHTS;

	MATRIXPC lColours;

	// Zero out the matrix first off
	lDirects->m[0][0] = 0;
	lDirects->m[0][1] = 0;
	lDirects->m[0][2] = 0;
	lDirects->m[1][0] = 0;
	lDirects->m[1][1] = 0;
	lDirects->m[1][2] = 0;
	lDirects->m[2][0] = 0;
	lDirects->m[2][1] = 0;
	lDirects->m[2][2] = 0;

	lColours.m[0][0] = 0;
	lColours.m[0][1] = 0;
	lColours.m[0][2] = 0;
	lColours.m[1][0] = 0;
	lColours.m[1][1] = 0;
	lColours.m[1][2] = 0;
	lColours.m[2][0] = 0;
	lColours.m[2][1] = 0;
	lColours.m[2][2] = 0;

	uint32 num = lamplist->n;

	PSXLamp *plamp = NULL;
	PSXLampState *plampstate;

	uint32 state;
	uint32 sstate;
	VECTOR dir;
	VECTOR normdir;
	uint32 i, s;

	uint32 *pstate = lamplist->states;
	PSXLamp **pplamp = lamplist->lamps;

	uint32 ns = shadelist->n;

	PSXShade *pshade;
	ShadeQuad *pshadestate;

	uint32 *psstate;
	PSXShade **ppshade;

	uint32 inten, m;
	for (i = 0; i < num; pplamp++, pstate++, i++) {
		// Make the direction vector
		plamp = *pplamp;
		state = *pstate;

		plampstate = plamp->states + state;

		// Ignore the lamp if it is switched off
		m = plampstate->m;

		// For animating lights which are turned off - turn them off
		// after the 3 brightest lights have been selected !
		if (plamp->nStates > 1)
			m = 128;

		if (m == 0)
			continue;

		// Squared distance to the lamp
		dir.vx = pos->vx - plampstate->pos.vx;
		dir.vy = pos->vy - plampstate->pos.vy;
		dir.vz = pos->vz - plampstate->pos.vz;
		// VectorNoraml returns the sum of the squares
		// i.e. the squared distance
		// Normalize the lamp_actor vector to 1.0
		uint32 rr = VectorNormal(&dir, &normdir);

#define DEBUG_PREPARE_LIGHTS 0
#if DEBUG_PREPARE_LIGHTS
		printf("Lamp %d rr %d actor %d %d %d lamp %d %d %d\nafs2 %d afe2 %d ans2 %d ane2 %d", i, rr, pos->vx, pos->vy, pos->vz, plampstate->pos.vx, plampstate->pos.vy,
		       plampstate->pos.vz, plampstate->ans2, plampstate->ane2, plampstate->afs2, plampstate->afe2);
#endif // #if DEBUG_PREPARE_LIGHTS

		// Get the light direction
		// non-directional lamps e.g. bulbs, OMNI's
		// then the direction is vector from lamp to actor i.e. dir
		// For directional lamps, the -ve 3rd column of the matrix
		// is what we want because MAX definition is the lamp direction
		// is 0,0,-1 in the light frame

		// The attenuation law is
		//
		// Intensity
		// multiplier
		//   |
		//   v         near_end             far_start
		//                 |                   |
		// 1.0 |           *********************
		//     |          *                     *
		//     |         *                       *
		//     |        *                         *
		//     |       *                           *
		//     |      *                             *
		//     |     *                               *
		//     |    *                                 *
		// 0.0 *****                                   *************
		//         |                                   |
		//       near_start                         far_end
		//
		//                   increasing distance --->

		// Ignore the lamp if rr > atten_far_end^2
		// In pre-process of lights if atten_far_use = 0
		// then it sets atten_far_end to a very big number

		// RLP schema >= 4 squared distances are stored in the light file
		uint32 afe2 = plampstate->afe2;
		// if ( ( afe2 < ATTEN_MAX_DISTANCE*ATTEN_MAX_DISTANCE ) && ( rr > afe2 ) ) continue;
		if (rr > afe2)
			continue;

		// RLP schema >= 4 squared distances are stored in the light file
		uint32 ans2 = plampstate->ans2;

		// Ignore the lamp if rr < atten_near_start^2
		// if ( ( ans2 < ATTEN_MAX_DISTANCE*ATTEN_MAX_DISTANCE ) && ( rr < ans2 ) ) continue;
		if (rr < ans2)
			continue;

		// RLP schema >= 4 squared distances are stored in the light file
		uint32 ane2 = plampstate->ane2;
		uint32 afs2 = plampstate->afs2;

		// Are we in the increasing portion ?
		if (rr < ane2) {
			m = ((rr - ans2) * m) / (ane2 - ans2);
		}
		// else are we in the decreasing portion ?
		else if ((afs2 < ATTEN_MAX_DISTANCE * ATTEN_MAX_DISTANCE) && (rr > afs2)) {
			m = ((afe2 - rr) * m) / (afe2 - afs2);
		}
// else we are in the constant portion : i.e. do nothing

#if DEBUG_PREPARE_LIGHTS
		printf("Lamp %d rr %d m %d afe2-rr %d afe2-afs2 %d", i, rr, m, (afe2 - rr), (afe2 - afs2));
#endif // #if DEBUG_PREPARE_LIGHTS

		// Ignore black lamps caused by the attenuation laws
		if (m == 0)
			continue;

		// For conical beams i.e. spot lights
		//
		// If the light has a beam then take into account the effect on m due
		// to angle between the light->actor vector and the light's direction
		//
		// The attenuation law is
		//
		// At angles less than beam_angle/2 then multiplier = 1.0
		// At angles beyond beam_softness/2 then multiplier = 0.0
		// Between these angles have straight line law
		//
		// Intensity
		// multiplier
		//   |
		//   v         beam_angle/2
		//                 |
		// 1.0 |************
		//     |            *
		//     |             *
		//     |              *
		//     |               *
		//     |                *
		//     |                 *
		// 0.0 |                  *************
		//                        |
		//                 beam_softness/2
		//
		//     -----------------> angle between actor and the light direction
		//

		if (plamp->type == SPOT_LIGHT) {
			// ba stored in the file is cos(beam_angle/2), so it can be
			// compared directly against the cos term we compute from the
			// dot product of light direction & light-actor direction
			int32 ba = plamp->ba;
			int32 bs = plamp->bs;
			// Get cos of the angle between light direciton & light-actor direction
			// These are both normalised to 4096, so end result has 1.0 = 4096*4096
			// Light direction is -ve z column of matrix (but normdir = -(light to actor vector))
			int32 cosa = normdir.vx * plampstate->vx + normdir.vy * plampstate->vy + normdir.vz * plampstate->vz;

#if DEBUG_PREPARE_LIGHTS
			printf("actor %d %d %d lamp %d %d %d dir %d %d %d cosa %d bs %d ba %d", pos->vx, pos->vy, pos->vz, plampstate->pos.vx, plampstate->pos.vy,
			       plampstate->pos.vz, plampstate->vx, plampstate->vy, plampstate->vz, (cosa >> 12), bs, ba);
#endif // #if DEBUG_PREPARE_LIGHTS

			// is the man more than 90 deg away from the beam direction
			if (cosa <= 0)
				continue;

			// reduce its range so that 1.0=4096
			cosa = cosa >> 12;

			// is the man outside the beam ?
			if (cosa < bs)
				continue;

			// is the man inside the decreasing portion
			if (cosa < ba) {
				m = ((cosa - bs) * m) / (ba - bs);
			}
// else the man is inside the constant portion
// and no need to change m
#if DEBUG_PREPARE_LIGHTS
			printf("m %d", m);
#endif // #if DEBUG_PREPARE_LIGHTS
		}

		// For cylindrical beams i.e. direct lights
		//
		// If the light has a beam then take into account the effect on m due
		// to the perpendicular distance between the light->actor vector and the light's direction vector
		//
		// The attenuation law is
		//
		// At distances less than beam_angle then multiplier = 1.0
		// At distance beyond beam_softness then multiplier = 0.0
		// Between these distance have straight line law
		//
		// Intensity
		// multiplier
		//   |
		//   v         beam_angle
		//                 |
		// 1.0 |************
		//     |            *
		//     |             *
		//     |              *
		//     |               *
		//     |                *
		//     |                 *
		// 0.0 |                  *************
		//                        |
		//                 beam_softness
		//
		//     -----------------> perpendicular distance between actor and the light direction
		//
		//    *  l - light position
		//    |+ -
		//    | + B
		//    |  +
		// A  |   +
		//    |    +
		//    |  /  +
		//    | / P  +
		//    |/      +|
		//    *       -  n - Light direction (normalised to unit vector)
		//    r          -
		//    -
		//
		// Actor position
		//
		// By pythagorous
		//
		// perpendicular distance = P = SQRT( A*A - B*B )
		//
		// B = ( r - l ) . n
		//       -   -     -
		//
		// A = ( r - l )
		//       -   -
		//
		// => P = SQRT( h*h - ( h . n )^2 )
		//
		// => P = SQRT( h*h - ( h . n )^2 )
		//
		// => P = SQRT( rr * ( 1 -  ( normdir . n )^2 ) )

		if (plamp->type == DIRECT_LIGHT) {
			// Light direction is -ve z column of matrix (but normdir = -(light to actor vector))
			// These are both normalised to 4096, so end result has 1.0 = 4096*4096
			int32 cosa = normdir.vx * plampstate->vx + normdir.vy * plampstate->vy + normdir.vz * plampstate->vz;

			// reduce its range so that 1.0=4096
			cosa = cosa >> 12;

			if (cosa < -4096)
				cosa = -4096;
			if (cosa > 4096)
				cosa = 4096;

			// to prevent overflows but make for slower code !
			int32 r = (int32)PXsqrt((PXfloat)rr);
			int32 P = (r * (int32)PXsqrt((PXfloat)(4096 * 4096 - cosa * cosa))) >> 12;

			int32 ba = plamp->ba;
			int32 bs = plamp->bs;

#if 0
			Message("xyz %d %d %d normdir %d %d %d",
			        pos->vx, pos->vy, pos->vz, normdir.vx, normdir.vy, normdir.vz) ;
			Message("r %d P %d cosa %d m = %d", r, P, cosa, m);
#endif
			// is the man outside the beam ?
			if (P > bs) {
				continue;
			}

			// is the man inside the decreasing portion
			if (P > ba) {
				m = ((bs - P) * m) / (bs - ba);
			} else {
			}
			// else the man is inside the constant portion
			// and no need to change m

			// The light direction is fixed
			normdir.vx = plampstate->vx;
			normdir.vy = plampstate->vy;
			normdir.vz = plampstate->vz;
		}

		// Account for decay
		int decaym = 4096;

		if (plamp->decay == DECAY_INV_SQR) {
			decaym = ((4096 * RLP_DECAY_CONSTANT * RLP_DECAY_CONSTANT) / rr);
		} else if (plamp->decay == DECAY_INV) {
			int32 r = (int32)PXsqrt((PXfloat)rr);
			decaym = ((4096 * RLP_DECAY_CONSTANT) / r);
		}

		if (plamp->decay != DECAY_NONE) {
			if (decaym > 4096)
				decaym = 4096;
			m = (m * decaym) >> 12;
		}

		// Ignore black lamps caused by the attenuation laws
		if (m == 0)
			continue;

		// Change the multiplier to account for any shades
		psstate = shadelist->states;
		ppshade = shadelist->shades;
		for (s = 0; s < ns; ppshade++, psstate++, s++) {
			pshade = *ppshade;
			sstate = *psstate;

			// Only do static shades in this loop - so we don't
			// get sudden light selection change due to animating shades
			// turning on/off
			if (pshade->nStates > 1)
				continue;

			pshadestate = pshade->states + sstate;
			m = computeShadeMultiplierPC(pshadestate, pos, &plampstate->pos, m);
		}

		// make the intensity = the multiplier * "value" (as in HSV)
		//   of the lights colour
		// RLP schema >= 4 v is stored in the lamps colour
		inten = m * plampstate->c.v;

#if DEBUG_PREPARE_LIGHTS
		printf("%HLamp %d m %d rr %d v %d inten %d ans %d ane %d afs %d afe %d", i, m, rr, plampstate->c.v, inten, ans2, ane2, afs2, afe2);
#endif // #if DEBUG_PREPARE_LIGHTS

		// Ignore lamps which are too dim
		if (inten < lampInfo[darkest].intens)
			continue;

		lampInfo[i].direct.vx = normdir.vx;
		lampInfo[i].direct.vy = normdir.vy;
		lampInfo[i].direct.vz = normdir.vz;

		lampInfo[i].index = i;
		lampInfo[i].intens = inten;
		lampInfo[i].mult = m;
		lampInfo[i].bounce = plamp->b;
		lampInfo[i].width = plamp->w;
		lampInfo[i].rr = rr;

		// compare against the 3 existing intensities
		// convention is that 0 = brightest, 1 = middle, 2 = dimmest
		if (inten > lampInfo[brightest].intens) {
			// Shift the lamps down in intensity
			darkest = middle;
			middle = brightest;
			brightest = i;
		} else if (inten > lampInfo[middle].intens) {
			// Shift the lamps down in intensity
			darkest = middle;
			middle = i;
		} else { // inten must be > lampInfo[darkest].intens
			darkest = i;
		}
	}

	pstate = lamplist->states;

	// Do just the brightest lamp
	// for ( i = 0; i < 1; pintens++, pindex++, i++ )

	// Put the 3 (at most) brightest lamps
	uint32 lamp_index[3];
	uint32 *pindex = lamp_index;
	lamp_index[0] = lampInfo[brightest].index;
	lamp_index[1] = lampInfo[middle].index;
	lamp_index[2] = lampInfo[darkest].index;

	// The brightness of the light at the actor

	// Ambient is 0-255, other lights are 0-4095
	int br = (ambient->r + ambient->g + ambient->g) << 4;

	int nlights = 0;
	// If we have an ambient
	if (br != 0)
		nlights = 1;

#if 0
	Message("Lamps %d %d %d", lamp_index[0], lamp_index[1], lamp_index[2]);
#endif

	for (i = 0; i < 3; pindex++, i++) {
		uint32 index = *pindex;
		// Ignore unset lamps
		if (index >= MAX_NUMBER_LIGHTS)
			continue;

		// Another lamp
		nlights++;

		// Fill in the lighting direction matrix
		// | Lx1 Ly1 Lz1 |
		// | Lx2 Ly2 Lz2 |
		// | Lx3 Ly3 Lz3 |
		// Lxn, Lyn, Lzn is the x/y/z component of directional light n
		LampInfo *pLampInfo = &(lampInfo[index]);
		linfo[i] = *pLampInfo;
		VECTOR *padirs = &(pLampInfo->direct);
		lDirects->m[i][0] = (short)padirs->vx;
		lDirects->m[i][1] = (short)padirs->vy;
		lDirects->m[i][2] = (short)padirs->vz;

		// Fill in the lighting colour matrix
		// | Lr1 Lr2 Lr3 |
		// | Lg1 Lg2 Lg3 |
		// | Lb1 Lb2 Lb3 |
		// Lrn, Lgn, Lbn is the red/green/blue of directional light n
		// Should do this with GTE vector interpolation
		state = pstate[index];
		plamp = (lamplist->lamps[index]);
		PSXLampState *plampstat = plamp->states + state;
		m = pLampInfo->mult;

		// For animating lights which are turned off - turn them off
		// after the 3 brightest lights have been selected !
		if (plamp->nStates > 1) {
			m = (m * plampstat->m) >> 7;
		}

		// Change the multiplier to account for any shades
		psstate = shadelist->states;
		ppshade = shadelist->shades;
		for (s = 0; s < ns; ppshade++, psstate++, s++) {
			pshade = *ppshade;
			sstate = *psstate;

			// Static shades have been done in the previous loop
			// so only do animating static shades in this loop
			if (pshade->nStates == 1)
				continue;

			pshadestate = pshade->states + sstate;
			m = computeShadeMultiplierPC(pshadestate, pos, &plampstat->pos, m);
		}

		lColours.m[0][i] = (short)(((uint32)(plampstat->c.r) * m) >> 7);
		lColours.m[1][i] = (short)(((uint32)(plampstat->c.g) * m) >> 7);
		lColours.m[2][i] = (short)(((uint32)(plampstat->c.b) * m) >> 7);

		linfo[i].colour.vx = (short)lColours.m[0][i];
		linfo[i].colour.vy = (short)lColours.m[1][i];
		linfo[i].colour.vz = (short)lColours.m[2][i];

		br = br + (lColours.m[0][i] + lColours.m[1][i] + lColours.m[2][i]);

#if 0
		Message("Lamp %d %d RGB:%d %d %d dir:%d %d %d",
		        i, index,
		        lColours.m[0][i], lColours.m[1][i], lColours.m[2][i],
		        lDirects->m[i][0], lDirects->m[i][1], lDirects->m[i][2]);
#endif

		// Only do the square root when we have to
		if (pLampInfo->width > 0)
			lampWidth[i] = (ONE * pLampInfo->width) / (pLampInfo->width + (int32)PXsqrt((PXfloat)pLampInfo->rr));
		else
			lampWidth[i] = 0;

		lampBounce[i] = pLampInfo->bounce;
#if DEBUG_PREPARE_LIGHTS
		printf("%HActor %d %d %d Lamp %d %d m %d dir %d %d %d colour %d %d %d", pos->vx, pos->vy, pos->vz, i, index, m, lDirects->m[i][0], lDirects->m[i][1],
		       lDirects->m[i][2], lColours.m[0][i], lColours.m[1][i], lColours.m[2][i]);
#endif // #if DEBUG_PREPARE_LIGHTS
	}
	// Non-zero if any non-zero width's
	useLampWidth = lampWidth[0] | lampWidth[1] | lampWidth[2];
	// Non-zero if any non-zero bounces's
	useLampBounce = lampBounce[0] | lampBounce[1] | lampBounce[2];

	// Set the GTE lighting colour matrix
	gte_SetColorMatrix_pc(&lColours);

	// Set the ambient colour
	gte_SetBackColor_pc(ambient->r, ambient->g, ambient->b);

	// Average out the brightness over the number of lamps
	if (nlights > 0) {
		br = br / nlights;
	}
	return br;
#undef DEBUG_PREPARE_LIGHTS
}

//------------------------------------------------------------------------
// Set the GTE constant light variables
// Sets the direciton lights colour matrix
// Sets the ambient colour
// Fills in the lightDirects matrix
int prepareLightsGlobalPC(MATRIXPC *lightDirects) {
	// Compute the lighting matrix which is :
	// | Lx1 Ly1 Lz1 |
	// | Lx2 Ly2 Lz2 |  * local2world_matrix
	// | Lx3 Ly3 Lz3 |
	// Where : Lxn, Lyn, Lzn is the x/y/z component of directional light n
	lightDirects->m[0][0] = (short)Lights[0].vx;
	lightDirects->m[0][1] = (short)Lights[0].vy;
	lightDirects->m[0][2] = (short)Lights[0].vz;
	lightDirects->m[1][0] = (short)Lights[1].vx;
	lightDirects->m[1][1] = (short)Lights[1].vy;
	lightDirects->m[1][2] = (short)Lights[1].vz;
	lightDirects->m[2][0] = (short)Lights[2].vx;
	lightDirects->m[2][1] = (short)Lights[2].vy;
	lightDirects->m[2][2] = (short)Lights[2].vz;

	MATRIXPC lightColours;
	// Fill in the lighting colour matrix
	// | Lr1 Lr2 Lr3 |
	// | Lg1 Lg2 Lg3 |
	// | Lb1 Lb2 Lb3 |
	lightColours.m[0][0] = (short)Lights[0].r;
	lightColours.m[1][0] = (short)Lights[0].g;
	lightColours.m[2][0] = (short)Lights[0].b;
	lightColours.m[0][1] = (short)Lights[1].r;
	lightColours.m[1][1] = (short)Lights[1].g;
	lightColours.m[2][1] = (short)Lights[1].b;
	lightColours.m[0][2] = (short)Lights[2].r;
	lightColours.m[1][2] = (short)Lights[2].g;
	lightColours.m[2][2] = (short)Lights[2].b;

	// Set the GTE lighting colour matrix
	gte_SetColorMatrix_pc(&lightColours);

	// Set the ambient colour
	gte_SetBackColor_pc(Lights[3].r, Lights[3].g, Lights[3].b);

	int br;

	br = (Lights[3].r + Lights[3].g + Lights[3].b) << 4;
	br = br + Lights[0].r + Lights[0].g + Lights[0].b;
	br = br + Lights[1].r + Lights[1].g + Lights[1].b;
	br = br + Lights[2].r + Lights[2].g + Lights[2].b;

	return br;
}

} // End of namespace ICB
