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
#include "engines/icb/global_objects_psx.h"
#include "engines/icb/drawpoly_pc.h"
#include "engines/icb/light_pc.h"
#include "engines/icb/softskin_pc.h"
#include "engines/icb/shadow_pc.h"
#include "engines/icb/common/px_capri_maths.h"
#include "engines/icb/gfx/psx_poly.h"

#include "common/system.h"

namespace ICB {

int tutorialMode = 0;

#define MAX_LW_MATRICES 64

#if _PSX_ON_PC
#define MAX_VECTORS 4096
#else
#define MAX_VECTORS 512
#endif

int littpc;
int mattpc;
int verttpc;
int polytpc;
int hiertpc;
int shadtpc;

// Local prototypes
#if CD_MODE == 0
void drawBboxPC(SVECTOR *scrn, CVECTOR colour);
void drawOutlinePC(SVECTOR *min, SVECTOR *max, CVECTOR colour);
#endif // #if CD_MODE == 0

void DrawModel4PC(rap_API *mrap, int poseBone, MATRIXPC *lw, MATRIXPC *local2screen, int uvframe, uint debug, SVECTOR *bbox, SVECTOR *minbbox, SVECTOR *maxbbox);

// My home grown replacement for the DrawActor routine
// which uses home grown replacement to do the drawing
// and also does the skinning using the co-ordinate animation data
// using soft-skinning specific data

void DrawActor4PC(psxActor *actor, psxCamera *camera, Bone_Frame *frame, rap_API *mesh, rap_API *pose, rap_API *smesh, PSXrgb *ambient, PSXLampList *lamplist,
                  PSXShadeList *shadelist, int nShadows, SVECTORPC *p_n, int *p_d, uint32 debug, int uvframe, BoneDeformation **boneDeforms, int *brightness,
                  MATRIXPC *local2screen // filled in
                  ) {

	// The position actor->pos is the world position
	// lw is the local to world matrix
	// camera->view is the world to screen matrix (without projection)

	MATRIXPC ll, lw;
	MATRIXPC lightDirects;
	MATRIXPC bone2actor[MAX_LW_MATRICES];
	MatrixHierarchyPC skeleton[MAX_LW_MATRICES];
	MATRIXPC *lwPtr;
	BoneLink *bones = mesh->GetBonePtr();

	SVECTOR poseBox[8];
	SVECTOR poseMinBox, poseMaxBox;
	SVECTOR *shadowBox[MAX_SHADOWS] = {actor->shadowBox[0], actor->shadowBox[1], actor->shadowBox[2]};

	// Set the focal length in the GTE
	gte_SetGeomScreen_pc(camera->focLen * ZSCALE);

	int32 p, flag;

	// Prepare the gte lighting variables and make the
	// light direction matrix
	// Sets the direciton lights colour matrix

	// Sets the ambient colour
	// Fills in the lightDirects matrix
	// prepareLightsGlobal( &lightDirects );
	littpc = g_system->getMillis();
	LampInfo linfo[3];
	VECTOR mpos;
	mpos.vx = actor->truePos.x;
	mpos.vy = actor->truePos.y;
	mpos.vz = actor->truePos.z;
	*brightness = prepareLightsPC(&mpos, ambient, lamplist, shadelist, &lightDirects, linfo);

	littpc = g_system->getMillis() - littpc;
	mattpc = g_system->getMillis();

	// Loop over the bones in the hierarchy
	// pre-computing the conversion matrices
	if (mesh->nBones >= MAX_LW_MATRICES) {
		Fatal_error("ERROR Too many bones %d max %d", mesh->nBones, MAX_LW_MATRICES);
	}

	// loop over the skeleton :
	// Making the matrices
	// copy in the skeleton translations from the RAP file
	// make the rotation part from the angles in the RAB file
	MatrixHierarchyPC *bone = skeleton;
	BoneLink *skelPiece = bones;
	LinkedMatrix *angles = frame->bones;
	SVECTOR rot;
	uint b;

	for (b = 0; b < mesh->nBones; bone++, angles++, skelPiece++, b++) {
		// Make the translation part
		bone->matrix.t[0] = skelPiece->tx;
		bone->matrix.t[1] = skelPiece->ty;
		bone->matrix.t[2] = skelPiece->tz;

		// Make the rotation part
		ExpandSVECTOR(angles->crot, &rot);

		for (int d = 0; d < MAX_DEFORMABLE_BONES; d++) {
			if ((boneDeforms[d]) && ((int)b == boneDeforms[d]->boneNumber)) {
				rot.vx = (short)(rot.vx + boneDeforms[d]->boneValue.vx);
				rot.vy = (short)(rot.vy + boneDeforms[d]->boneValue.vy);
				rot.vz = (short)(rot.vz + boneDeforms[d]->boneValue.vz);
			}
		}

		RotMatrix_gte_pc(&rot, &(bone->matrix));

		// Make the parent pointer : parent == nBones means not linked
		if (skelPiece->parent != mesh->nBones) {
			bone->parent = skeleton + skelPiece->parent;
		} else
			bone->parent = 0x0;
	}

	int hasUpperBodyDeform = 0;
	SVECTOR upperBodyDeform;

	// no upper body deform
	upperBodyDeform.vx = 0;
	upperBodyDeform.vy = 0;
	upperBodyDeform.vz = 0;

	// check for any bone transformations on bone 1 which will effect the gun position...

	for (int d = 0; d < MAX_DEFORMABLE_BONES; d++) {
		// if this is the upper body twist... (bone 1)
		if ((boneDeforms[d]) && (boneDeforms[d]->boneNumber == 1)) {
			hasUpperBodyDeform = 1;
			upperBodyDeform.vx = (short)(upperBodyDeform.vx + boneDeforms[d]->boneValue.vx);
			upperBodyDeform.vy = (short)(upperBodyDeform.vy + boneDeforms[d]->boneValue.vy);
			upperBodyDeform.vz = (short)(upperBodyDeform.vz + boneDeforms[d]->boneValue.vz);
		}
	}

	// Is there an accesory to draw ?
	uint nBones = mesh->nBones;
	int pb = 0;
	if (frame->poseBone.parent != mesh->nBones) {
		pb = nBones;
		// Make the translation part
		bone->matrix.t[0] = frame->poseBone.tx;
		bone->matrix.t[1] = frame->poseBone.ty;
		bone->matrix.t[2] = frame->poseBone.tz;

		// we have an upper body deform
		if (hasUpperBodyDeform) {
// first subtract hip-upperBody

#define HIP_TO_BONE 50

			bone->matrix.t[2] -= HIP_TO_BONE;
			bone->matrix.t[0] += 20;

			MATRIXPC matrix;
			SVECTOR gun_rot;

			gun_rot.vx = upperBodyDeform.vx;
			gun_rot.vy = upperBodyDeform.vy;
			gun_rot.vz = (short)(48 * upperBodyDeform.vz / 64);

			RotMatrix_gte_pc(&gun_rot, &matrix);

			ApplyMatrixLV_pc(&matrix, (VECTOR *)&bone->matrix.t, (VECTOR *)&bone->matrix.t);

			bone->matrix.t[2] += HIP_TO_BONE;
			bone->matrix.t[0] -= 20;
		}

		// Make the rotation part
		ExpandSVECTOR(angles->crot, &rot);
		RotMatrix_gte_pc(&rot, &(bone->matrix));

		if (hasUpperBodyDeform) {
			MATRIXPC matrix;
			RotMatrix_gte_pc(&upperBodyDeform, &matrix);
			gte_MulMatrix0_pc(&matrix, &(bone->matrix), &(bone->matrix));
		}

		bone->parent = skeleton + frame->poseBone.parent;
		nBones++;
	}
	mattpc = g_system->getMillis() - mattpc;
	hiertpc = g_system->getMillis();

	// Loop over the bones in the hierarchy
	// pre-computing the conversion matrices

	lwPtr = bone2actor;
	bone = skeleton;
	MatrixHierarchyPC *pcoord;

	MATRIXPC tempWorkm0;
	MATRIXPC tempWorkm1;
	VECTOR tempTvec;

	MATRIXPC *workm0;
	MATRIXPC *workm1;
	VECTOR *tvec;

	workm0 = &tempWorkm0;
	workm1 = &tempWorkm1;
	tvec = &tempTvec;

	for (b = 0; b < nBones; bone++, lwPtr++, b++) {
		// Store the units local-world matrix in the workm variable
		// Store the local-screen matrix in the ls array in hmdAnim
		pcoord = bone->parent;

		// Calculate Local-World Matrix
		// and store it in the work matrix for this co-ordinate
		*workm0 = bone->matrix;

		// Does it have a parent co-ordinate system ?
		// If so, then apply that coord system to current matrix
		while (pcoord) {
			// parent*daughter and then store in lwPtr
			*workm1 = pcoord->matrix;

			ApplyMatrixLV_pc(workm1, (VECTOR *)&workm0->t[0], tvec);

			gte_MulMatrix0_pc(workm1, workm0, workm0);

			workm0->t[0] = tvec->vx + workm1->t[0];
			workm0->t[1] = tvec->vy + workm1->t[1];
			workm0->t[2] = tvec->vz + workm1->t[2];

			pcoord = pcoord->parent;
		}
		*lwPtr = *workm0;
		bone->parent = NULL;
		bone->matrix = *workm0;
	}

	hiertpc = g_system->getMillis() - hiertpc;

	// Make a local to screen matrix
	// local2world is actor->lw
	// Calculate Local-Screen Matrix

	// Make the rotation matrix
	MATRIXPC cam_pc, act_pc;
	const int MSCALE = ONE_PC / ONE;
	cam_pc.m[0][0] = camera->view.m[0][0] * MSCALE;
	cam_pc.m[0][1] = camera->view.m[0][1] * MSCALE;
	cam_pc.m[0][2] = camera->view.m[0][2] * MSCALE;
	cam_pc.m[1][0] = camera->view.m[1][0] * MSCALE;
	cam_pc.m[1][1] = camera->view.m[1][1] * MSCALE;
	cam_pc.m[1][2] = camera->view.m[1][2] * MSCALE;
	cam_pc.m[2][0] = camera->view.m[2][0] * MSCALE * ZSCALE;
	cam_pc.m[2][1] = camera->view.m[2][1] * MSCALE * ZSCALE;
	cam_pc.m[2][2] = camera->view.m[2][2] * MSCALE * ZSCALE;
	cam_pc.t[0] = camera->view.t[0];
	cam_pc.t[1] = camera->view.t[1];
	cam_pc.t[2] = camera->view.t[2] * ZSCALE;

	act_pc.m[0][0] = actor->lw.m[0][0] * MSCALE;
	act_pc.m[0][1] = actor->lw.m[0][1] * MSCALE;
	act_pc.m[0][2] = actor->lw.m[0][2] * MSCALE;
	act_pc.m[1][0] = actor->lw.m[1][0] * MSCALE;
	act_pc.m[1][1] = actor->lw.m[1][1] * MSCALE;
	act_pc.m[1][2] = actor->lw.m[1][2] * MSCALE;
	act_pc.m[2][0] = actor->lw.m[2][0] * MSCALE;
	act_pc.m[2][1] = actor->lw.m[2][1] * MSCALE;
	act_pc.m[2][2] = actor->lw.m[2][2] * MSCALE;
	act_pc.t[0] = actor->lw.t[0];
	act_pc.t[1] = actor->lw.t[1];
	act_pc.t[2] = actor->lw.t[2];

	gte_MulMatrix0_pc(&cam_pc, &act_pc, local2screen);

	// make the ls trans vector
	//  = world2screen * local2world_trans + world2screen_trans
	SVECTORPC sv;
	sv.vx = (short)actor->lw.t[0];
	sv.vy = (short)actor->lw.t[1];
	sv.vz = (short)actor->lw.t[2];

	ApplyMatrixLV_pc(&cam_pc, (VECTOR *)&(actor->lw.t[0]), (VECTOR *)&(local2screen->t[0]));
	local2screen->t[0] += cam_pc.t[0];
	local2screen->t[1] += cam_pc.t[1];
	local2screen->t[2] += cam_pc.t[2];

	// Make the GTE local light matrix by lightDirects * local-world
	// destroys GTE rot matrix
	//
	SVECTOR trot;
	SVECTOR hip;
	MATRIXPC hipm;
	trot = actor->rot;

	// HIP is root bone in the hierarchy
	angles = frame->bones;
	ExpandSVECTOR(angles->crot, &hip);

	// Include the orientation of the HIP in the local-world transformation
	// So that the lighting is correct for HIP rotation e.g. lying down dead, turning, turning around on stairs
	// Note: hip axis is not aligned to local axis:
	// world axis: local.vx = hip.vx + 90 deg, local.vy = hip.vz, local.vz = hip.vy
	// Ignore the hip.vy - which represents roll because the hips roll independently of the whole body movement
	// e.g. when walking along rolling the hips !

	trot.vx = (short)(hip.vx + (ONE / 4));
	trot.vy = (short)(hip.vz);
	trot.vz = 0;

	// Make the rotation matrix to account for the hips orientation relative to the actor coords
	RotMatrix_gte_pc(&trot, &hipm);

	// Make the rotation matrix to account for the actor orientation in world coords
	RotMatrix_gte_pc(&actor->rot, &lw);

	// Make the combined matrix
	gte_MulMatrix0_pc(&lw, &hipm, &lw);

	gte_MulMatrix0_pc(&lightDirects, &lw, &ll);

	// Set the GTE lighting direction matrix
	gte_SetLightMatrix_pc(&ll);

	// Do the soft-skinning drawing of the model
	// use the linkage info in mesh file and the co-ordinate data
	// from the hmd file

	// Draw the model
	DrawModel4PC(mesh, -1, bone2actor, local2screen, uvframe, debug, actor->bboxScrn, &actor->minBbox, &actor->maxBbox);

	// At most 3 shadows !
	if (nShadows > MAX_SHADOWS) {
		Fatal_error("ERROR Too many shadows %d max %d", nShadows, MAX_SHADOWS);
	}
	SVECTORPC ldirs[MAX_SHADOWS];
	CVECTOR lcolours[MAX_SHADOWS];
	int l;
	int ns = 0;
	int work;
	for (l = 0; l < nShadows; l++) {
		// Only use lights which are on and point downwards
		if ((linfo[l].intens != 0) && (linfo[l].direct.vy < 0)) {
			ldirs[ns].vx = (short)linfo[l].direct.vx;
			ldirs[ns].vy = (short)linfo[l].direct.vy;
			ldirs[ns].vz = (short)linfo[l].direct.vz;
			work = (linfo[l].colour.vx >> 4);
			if (work > 255)
				work = 255;
			lcolours[ns].r = (u_char)work;
			work = (linfo[l].colour.vy >> 4);
			if (work > 255)
				work = 255;
			lcolours[ns].g = (u_char)work;
			work = (linfo[l].colour.vz >> 4);
			if (work > 255)
				work = 255;
			lcolours[ns].b = (u_char)work;
			ns++;
		}
	}

	// In simple shadow mode just have a single top-down BLACK shadow
	if (nShadows == -1) {
		ns = 1;
		ldirs[0].vx = 0;
		ldirs[0].vy = -4096;
		ldirs[0].vz = 0;
		lcolours[0].r = 0x80;
		lcolours[0].g = 0x80;
		lcolours[0].b = 0x80;
	}

	// In complex mode add a top-down BLACK shadow if no shadows are being
	// drawn
	if ((nShadows > 0) && (ns == 0)) {
		ns = 1;
		ldirs[0].vx = 0;
		ldirs[0].vy = -4096;
		ldirs[0].vz = 0;
		lcolours[0].r = 0x80;
		lcolours[0].g = 0x80;
		lcolours[0].b = 0x80;
	}

	// draw the shadow
	if (smesh != NULL) {
		shadtpc = g_system->getMillis();
		DrawShadow1PC(smesh, -1, bone2actor, &cam_pc, &act_pc, ns, ldirs, lcolours, p_n, p_d, debug, shadowBox, actor->shadowMinBox, actor->shadowMaxBox);
		shadtpc = g_system->getMillis() - shadtpc;
		actor->nShadows = ns;
	} else {
		actor->nShadows = 0;
	}

	// Draw the accessory
	if ((pose != NULL) && (pb != 0)) {
		DrawModel4PC(pose, pb, bone2actor, local2screen, uvframe, debug, poseBox, &poseMinBox, &poseMaxBox);

		// WARNING
		// the accessory bounding box is not added onto the actors proper bounding box !
		// it is only added onto the min, max bounding box outline

		// Add the accessory bounding box onto the min, max actors bounding box
		if (poseMinBox.vx < actor->minBbox.vx)
			actor->minBbox.vx = poseMinBox.vx;
		if (poseMinBox.vy < actor->minBbox.vy)
			actor->minBbox.vy = poseMinBox.vy;
		if (poseMinBox.vz < actor->minBbox.vz)
			actor->minBbox.vz = poseMinBox.vz;

		if (poseMaxBox.vx > actor->maxBbox.vx)
			actor->maxBbox.vx = poseMaxBox.vx;
		if (poseMaxBox.vy > actor->maxBbox.vy)
			actor->maxBbox.vy = poseMaxBox.vy;
		if (poseMaxBox.vz > actor->maxBbox.vz)
			actor->maxBbox.vz = poseMaxBox.vz;
	}

	// Set the local-screen matrix in the GTE
	gte_SetRotMatrix_pc(local2screen);
	gte_SetTransMatrix_pc(local2screen);
	gte_SetScreenScaleShift_pc(0);

	// Compute the screen position of the models local origin
	// Which is the translation part of the GTE matrix
	SVECTORPC origin;
	origin.vx = origin.vy = origin.vz = 0;

	SVECTORPC sxy0;
	gte_RotTransPers_pc(&origin, &sxy0, &p, &flag, (int32 *)&(actor->sPos.vz));
	actor->sPos.vx = (short)sxy0.vx;
	actor->sPos.vy = (short)sxy0.vy;

#if CD_MODE == 0
	CVECTOR bboxColour = {(u_char)bboxRed, (u_char)bboxGreen, (u_char)bboxBlue, 0};
	if (_drawBbox)
		drawBboxPC(actor->bboxScrn, bboxColour);
	CVECTOR sbColour = {240, 240, 240, 0};
	if (_drawSolidBbox)
		drawSolidBboxPC(actor->bboxScrn, &sbColour);

	bboxColour.r = 255;
	bboxColour.g = 20;
	bboxColour.b = 180;
	if (_drawShadowBbox) {
		for (b = 0; b < actor->nShadows; b++) {
			drawBboxPC(shadowBox[b], bboxColour);
		}
	}

	// 2 LINE_F3's : e.g. a rectangle
	CVECTOR slColour;
	if (_drawSline) {
		slColour.r = (u_char)slineRed;
		slColour.g = (u_char)slineGreen;
		slColour.b = (u_char)slineBlue;
		drawOutlinePC(&actor->minBbox, &actor->maxBbox, slColour);
	}
	if (_drawShadowSline) {
		for (b = 0; b < actor->nShadows; b++) {
			slColour.r = (u_char)180;
			slColour.g = (u_char)(20 + b * 70);
			slColour.b = (u_char)255;
			drawOutlinePC(actor->shadowMinBox + b, actor->shadowMaxBox + b, slColour);
		}
	}
#endif // #if CD_MODE == 0
}

// dont bother in a cd mode
#if CD_MODE == 0
void drawOutlinePC(SVECTOR *min, SVECTOR *max, CVECTOR colour) {
	LINE_F3 *line = (LINE_F3 *)drawpacket;
	setLineF3(line);
	setRGB0(line, colour.r, colour.g, colour.b);
	// minx, miny => maxx, miny => maxx, maxy
	setXY3(line, min->vx, min->vy, max->vx, min->vy, max->vx, max->vy);
	myAddPrimClip(min->vz, drawpacket);
	myAddPacket(sizeof(LINE_F3));

	// maxx, maxy => minx, maxy => minx, miny
	line = (LINE_F3 *)drawpacket;
	setLineF3(line);
	setRGB0(line, colour.r, colour.g, colour.b);
	setXY3(line, max->vx, max->vy, min->vx, max->vy, min->vx, min->vy);
	myAddPrimClip(min->vz, drawpacket);
	myAddPacket(sizeof(LINE_F3));
}
#endif

// New version using soft-skinning specific data-files
void DrawModel4PC(rap_API *mrap, int poseBone, MATRIXPC *lw, MATRIXPC *local2screen, int uvframe, uint debug, SVECTOR *bbox, SVECTOR *minbbox, SVECTOR *maxbbox) {
	SVECTORPC local[MAX_VECTORS];
	SVECTORPC screen[MAX_VECTORS];

	int16 xminLocal = +32767;
	int16 xmaxLocal = -32767;
	int16 yminLocal = +32767;
	int16 ymaxLocal = -32767;
	int16 zminLocal = +32767;
	int16 zmaxLocal = -32767;

	verttpc = g_system->getMillis();
	int screenScale = mrap->worldScaleShift;
	int nVertices = softskinPC(mrap, poseBone, lw, local, &xminLocal, &xmaxLocal, &yminLocal, &ymaxLocal, &zminLocal, &zmaxLocal, screenScale);

	// So all the local positions have been made
	verttpc = g_system->getMillis() - verttpc;
	polytpc = g_system->getMillis();

	// step 3 : draw the polygons using the list of created screen positions
	// Need to get the pointer to HMD stored data out of HMD file

	// Put the correct rot and trans matrix in place
	// transform model from world space to screen space
	gte_SetRotMatrix_pc(local2screen);
	gte_SetTransMatrix_pc(local2screen);
	gte_SetScreenScaleShift_pc(screenScale);

	int32 flag, p;
	int i;
	SVECTORPC *v0;
	SVECTOR *pbbox;

	// Convert the bounding box from local co-ordinates into screen co-ordinates
	bbox[0].vx = xminLocal;
	bbox[0].vy = yminLocal;
	bbox[0].vz = zminLocal;

	bbox[1].vx = xminLocal;
	bbox[1].vy = yminLocal;
	bbox[1].vz = zmaxLocal;

	bbox[2].vx = xmaxLocal;
	bbox[2].vy = yminLocal;
	bbox[2].vz = zminLocal;

	bbox[3].vx = xmaxLocal;
	bbox[3].vy = yminLocal;
	bbox[3].vz = zmaxLocal;

	bbox[4].vx = xmaxLocal;
	bbox[4].vy = ymaxLocal;
	bbox[4].vz = zminLocal;

	bbox[5].vx = xmaxLocal;
	bbox[5].vy = ymaxLocal;
	bbox[5].vz = zmaxLocal;

	bbox[6].vx = xminLocal;
	bbox[6].vy = ymaxLocal;
	bbox[6].vz = zminLocal;

	bbox[7].vx = xminLocal;
	bbox[7].vy = ymaxLocal;
	bbox[7].vz = zmaxLocal;

	pbbox = bbox;
	SVECTORPC sxy0;
	for (i = 0; i < 8; i++) {
		gte_RotTransPers_pc(pbbox, &sxy0, &p, &flag, (int32 *)&(pbbox->vz));
		pbbox->vx = (short)sxy0.vx;
		pbbox->vy = (short)sxy0.vy;
		pbbox++;
	}

	// Find the minimum and maximum screen positions (plus z)
	pbbox = bbox;
	copyVector(minbbox, pbbox);
	copyVector(maxbbox, pbbox);
	pbbox++;
	for (i = 1; i < 8; i++, pbbox++) {
		if (pbbox->vx < minbbox->vx)
			minbbox->vx = pbbox->vx;
		if (pbbox->vy < minbbox->vy)
			minbbox->vy = pbbox->vy;
		if (pbbox->vz < minbbox->vz)
			minbbox->vz = pbbox->vz;
		if (pbbox->vx > maxbbox->vx)
			maxbbox->vx = pbbox->vx;
		if (pbbox->vy > maxbbox->vy)
			maxbbox->vy = pbbox->vy;
		if (pbbox->vz > maxbbox->vz)
			maxbbox->vz = pbbox->vz;
	}

	// Loop over the vector pool converting them all to screen co-ordinates
	v0 = local;
	if (debug == 0) {
		ConvertToScreenCoords(local, screen, nVertices);
	}

	// Now go and find the actual polygon data for this primitive
	uint32 *polyStart;
	uint32 nPolys;
	uint32 *pNormal = mrap->GetNormalPtr();

	nPolys = mrap->nFUS3;
	if (nPolys != 0) {
		polyStart = mrap->GetFUS3Ptr();
// Do the drawing using internal C based debugging drawing code
#if CD_MODE == 0
		if (debug)
			drawFUS3PC(polyStart, nPolys, local);
		else
#endif // #if CD_MODE == 0
			fastDrawFUS3PC(polyStart, nPolys, screen);
	}
	nPolys = mrap->nGUS3;
	if (nPolys != 0) {
		polyStart = mrap->GetGUS3Ptr();
// Do the drawing using internal C based debugging drawing code
#if CD_MODE == 0
		if (debug)
			drawGUS3PC(polyStart, nPolys, local);
		else
#endif // #if CD_MODE == 0
			fastDrawGUS3PC(polyStart, nPolys, screen);
	}
	nPolys = mrap->nFTS3;
	if (nPolys != 0) {
		polyStart = mrap->GetFTS3Ptr();
// Do the drawing using internal C based debugging drawing code
#if CD_MODE == 0
		if (debug)
			drawFTS3PC(polyStart, nPolys, local);
		else
#endif // #if CD_MODE == 0
			fastDrawFTS3PC(polyStart, nPolys, screen);
	}
	nPolys = mrap->nGTS3;
	if (nPolys != 0) {
		polyStart = mrap->GetGTS3Ptr();
// Do the drawing using internal C based debugging drawing code
#if CD_MODE == 0
		if (debug)
			drawGTS3PC(polyStart, nPolys, local);
		else
#endif // #if CD_MODE == 0
			fastDrawGTS3PC(polyStart, nPolys, screen);
	}
	nPolys = mrap->nFUL3;
	if (nPolys != 0) {
		polyStart = mrap->GetFUL3Ptr();
// Do the drawing using internal C based debugging drawing code
#if CD_MODE == 0
		if (debug)
			drawFUL3PC(polyStart, nPolys, local, (SVECTOR *)pNormal);
		else
#endif // #if CD_MODE == 0
			fastDrawFUL3PC(polyStart, nPolys, screen, (SVECTOR *)pNormal);
	}
	nPolys = mrap->nGUL3;
	if (nPolys != 0) {
		polyStart = mrap->GetGUL3Ptr();
// Do the drawing using internal C based debugging drawing code
#if CD_MODE == 0
		if (debug)
			drawGUL3PC(polyStart, nPolys, local, (SVECTOR *)pNormal);
		else
#endif // #if CD_MODE == 0
			fastDrawGUL3PC(polyStart, nPolys, screen, (SVECTOR *)pNormal);
	}
	nPolys = mrap->nFTL3;
	if (nPolys != 0) {
		polyStart = mrap->GetFTL3Ptr();
// Do the drawing using internal C based debugging drawing code
#if CD_MODE == 0
		if (debug)
			drawFTL3PC(polyStart, nPolys, local, (SVECTOR *)pNormal);
		else
#endif // #if CD_MODE == 0
			fastDrawFTL3PC(polyStart, nPolys, screen, (SVECTOR *)pNormal);
	}
	nPolys = mrap->nGTL3;
	if (nPolys != 0) {
		polyStart = mrap->GetGTL3Ptr();
// Do the drawing using internal C based debugging drawing code
#if CD_MODE == 0
		if (debug)
			drawGTL3PC(polyStart, nPolys, local, (SVECTOR *)pNormal);
		else
#endif // #if CD_MODE == 0
			fastDrawGTL3PC(polyStart, nPolys, screen, (SVECTOR *)pNormal);
	}

	// Now do the animating polygons
	uint nTypes = mrap->nAnimTypes;
	if (nTypes != 0) {
		int nFrames = mrap->nFrames;
		if (nFrames == 0)
			nFrames = 1;
		int frm = uvframe % nFrames;

		// for dead things uvframe = -1 : and in that case draw the last frame of the animation
		if (uvframe == -1) {
			frm = 0;
			deadObject = 1;
		} else {
			deadObject = 0;
		}

		uint32 *typePtr = mrap->GetAnimPolyPtr();
		polyStart = mrap->GetAnimPolyFrame(frm);
		for (uint t = 0; t < nTypes; t++) {
			switch (*typePtr++) { // ++ skips the type field
			case HMD_FUS3: {
				nPolys = *typePtr++;
#if CD_MODE == 0
				if (debug)
					drawFUS3PC(polyStart, nPolys, local);
				else
#endif // #if CD_MODE == 0
					fastDrawFUS3PC(polyStart, nPolys, screen);
				polyStart += (nPolys * HMD_FUS3_SIZE); // skip to the next polygon
				break;
			}
			case HMD_GUS3: {
				nPolys = *typePtr++;
#if CD_MODE == 0
				if (debug)
					drawGUS3PC(polyStart, nPolys, local);
				else
#endif // #if CD_MODE == 0
					fastDrawGUS3PC(polyStart, nPolys, screen);
				polyStart += (nPolys * HMD_GUS3_SIZE); // skip to the next polygon
				break;
			}
			case HMD_FTS3: {
				nPolys = *typePtr++;
#if CD_MODE == 0
				if (debug)
					drawFTS3PC(polyStart, nPolys, local);
				else
#endif // #if CD_MODE == 0
					fastDrawFTS3PC(polyStart, nPolys, screen);
				polyStart += (nPolys * HMD_FTS3_SIZE); // skip to the next polygon
				break;
			}
			case HMD_GTS3: {
				nPolys = *typePtr++;
#if CD_MODE == 0
				if (debug)
					drawGTS3PC(polyStart, nPolys, local);
				else
#endif // #if CD_MODE == 0
					fastDrawGTS3PC(polyStart, nPolys, screen);
				polyStart += (nPolys * HMD_GTS3_SIZE); // skip to the next polygon
				break;
			}
			case HMD_FUL3: {
				nPolys = *typePtr++;
#if CD_MODE == 0
				if (debug)
					drawFUL3PC(polyStart, nPolys, local, (SVECTOR *)pNormal);
				else
#endif // #if CD_MODE == 0
					fastDrawFUL3PC(polyStart, nPolys, screen, (SVECTOR *)pNormal);
				polyStart += (nPolys * HMD_FUL3_SIZE); // skip to the next polygon
				break;
			}
			case HMD_GUL3: {
				nPolys = *typePtr++;
#if CD_MODE == 0
				if (debug)
					drawGUL3PC(polyStart, nPolys, local, (SVECTOR *)pNormal);
				else
#endif // #if CD_MODE == 0
					fastDrawGUL3PC(polyStart, nPolys, screen, (SVECTOR *)pNormal);
				polyStart += (nPolys * HMD_GUL3_SIZE); // skip to the next polygon
				break;
			}
			case HMD_FTL3: {
				nPolys = *typePtr++;
#if CD_MODE == 0
				if (debug)
					drawFTL3PC(polyStart, nPolys, local, (SVECTOR *)pNormal);
				else
#endif // #if CD_MODE == 0
					fastDrawFTL3PC(polyStart, nPolys, screen, (SVECTOR *)pNormal);
				polyStart += (nPolys * HMD_FTL3_SIZE); // skip to the next polygon
				break;
			}
			case HMD_GTL3: {
				nPolys = *typePtr++;
#if CD_MODE == 0
				if (debug)
					drawGTL3PC(polyStart, nPolys, local, (SVECTOR *)pNormal);
				else
#endif // #if CD_MODE == 0
					fastDrawGTL3PC(polyStart, nPolys, screen, (SVECTOR *)pNormal);
				polyStart += (nPolys * HMD_GTL3_SIZE); // skip to the next polygon
				break;
			}
			default: { Fatal_error("ERROR Unknown animating polygon %X", *(polyStart - 1)); }
			}
		}
	}
	polytpc = g_system->getMillis() - polytpc;
}

void DrawActorTiePC(psxCamera *camera, SVECTORPC *pos, uint32 size, CVECTOR *) {
	SVECTORPC wcorners[8];
	SVECTORPC scorners[8];

	wcorners[0].vx = (short)(pos->vx - size);
	wcorners[0].vy = pos->vy;
	wcorners[0].vz = (short)(pos->vz + size / 2);

	wcorners[1].vx = (short)(pos->vx - size);
	wcorners[1].vy = pos->vy;
	wcorners[1].vz = (short)(pos->vz - size / 2);

	wcorners[2].vx = (short)(pos->vx - size / 2);
	wcorners[2].vy = pos->vy;
	wcorners[2].vz = (short)(pos->vz - size);

	wcorners[3].vx = (short)(pos->vx + size / 2);
	wcorners[3].vy = pos->vy;
	wcorners[3].vz = (short)(pos->vz - size);

	wcorners[4].vx = (short)(pos->vx + size);
	wcorners[4].vy = pos->vy;
	wcorners[4].vz = (short)(pos->vz - size / 2);

	wcorners[5].vx = (short)(pos->vx + size);
	wcorners[5].vy = pos->vy;
	wcorners[5].vz = (short)(pos->vz + size / 2);

	wcorners[6].vx = (short)(pos->vx + size / 2);
	wcorners[6].vy = pos->vy;
	wcorners[6].vz = (short)(pos->vz + size);

	wcorners[7].vx = (short)(pos->vx - size / 2);
	wcorners[7].vy = pos->vy;
	wcorners[7].vz = (short)(pos->vz + size);

	SVECTORPC *local = wcorners;
	SVECTORPC *scrn;

	// Make the rotation matrix
	MATRIXPC cam_pc;
	const int MSCALE = ONE_PC / ONE;
	cam_pc.m[0][0] = camera->view.m[0][0] * MSCALE;
	cam_pc.m[0][1] = camera->view.m[0][1] * MSCALE;
	cam_pc.m[0][2] = camera->view.m[0][2] * MSCALE;
	cam_pc.m[1][0] = camera->view.m[1][0] * MSCALE;
	cam_pc.m[1][1] = camera->view.m[1][1] * MSCALE;
	cam_pc.m[1][2] = camera->view.m[1][2] * MSCALE;
	cam_pc.m[2][0] = camera->view.m[2][0] * MSCALE * ZSCALE;
	cam_pc.m[2][1] = camera->view.m[2][1] * MSCALE * ZSCALE;
	cam_pc.m[2][2] = camera->view.m[2][2] * MSCALE * ZSCALE;

	cam_pc.t[0] = camera->view.t[0];
	cam_pc.t[1] = camera->view.t[1];
	cam_pc.t[2] = camera->view.t[2] * ZSCALE;

	int i;
	int32 p, flag, z0;
	gte_SetGeomScreen_pc(camera->focLen * ZSCALE);
	gte_SetRotMatrix_pc(&cam_pc);
	gte_SetTransMatrix_pc(&cam_pc);
	gte_SetScreenScaleShift_pc(0);

	SVECTORPC mine;
	scrn = &mine;

	gte_RotTransPers_pc(pos, scrn, &p, &flag, &z0);
	scrn->vz = (short)z0;

	scrn = scorners;

	for (i = 0; i < 8; i++, local++, scrn++) {
		gte_RotTransPers_pc(local, scrn, &p, &flag, &z0);
		scrn->vz = (short)z0;
	}

	TPOLY_G3 *tie;
	int32 inz0;

	inz0 = 0;
	for (i = 0; i < 8; i++)
		inz0 += scorners[0].vz;

	inz0 /= 8;

	for (i = 0; i < 8; i++) {
		int next = i + 1;
		if (next == 8)
			next = 0;
		tie = (TPOLY_G3 *)drawpacket;
		setTPolyG3(tie);
		setXY3(tie, scorners[i].vx, scorners[i].vy, scorners[next].vx, scorners[next].vy, mine.vx, mine.vy);
		setRGB0(tie, 0, 0, 0);
		setRGB1(tie, 0, 0, 0);
		setRGB2(tie, 96, 96, 96);
		setTSemiTrans(tie, 1);
		setTABRMode(tie, 2);

		z0 = myAddPrimClip(inz0, drawpacket);
		myAddPacket(sizeof(TPOLY_G3));
	}
}

#if CD_MODE == 0

void drawBboxPC(SVECTOR *scrn, CVECTOR colour) {
	// 6 LINE_F3's i.e. 6 pairs of lines = 12 lines in total
	// The 6 pairs of lines are the following vertex links:
	// 0->1->3
	// 2->3->5
	// 0->2->4
	// 0->6->7
	// 1->7->5
	// 6->4->5

	int32 z0;
	LINE_F3 *line = (LINE_F3 *)drawpacket;
	setLineF3(line);
	setRGB0(line, colour.r, colour.g, colour.b);
	// Pair0: 0->1->3
	setXY3(line, scrn[0].vx, scrn[0].vy, scrn[1].vx, scrn[1].vy, scrn[3].vx, scrn[3].vy);

	z0 = myAddPrimClip(scrn[3].vz, drawpacket);
	myAddPacket(sizeof(LINE_F3));

	// Pair1: 2->3->5
	line = (LINE_F3 *)drawpacket;
	setLineF3(line);
	setRGB0(line, colour.r, colour.g, colour.b);
	setXY3(line, scrn[2].vx, scrn[2].vy, scrn[3].vx, scrn[3].vy, scrn[5].vx, scrn[5].vy);
	z0 = myAddPrimClip(scrn[5].vz, drawpacket);
	myAddPacket(sizeof(LINE_F3));

	// Pair2: 0->2->4
	line = (LINE_F3 *)drawpacket;
	setLineF3(line);
	setRGB0(line, colour.r, colour.g, colour.b);
	setXY3(line, scrn[0].vx, scrn[0].vy, scrn[2].vx, scrn[2].vy, scrn[4].vx, scrn[4].vy);
	z0 = myAddPrimClip(scrn[5].vz, drawpacket);
	myAddPacket(sizeof(LINE_F3));

	// Pair3: 0->6->7
	line = (LINE_F3 *)drawpacket;
	setLineF3(line);
	setRGB0(line, colour.r, colour.g, colour.b);
	setXY3(line, scrn[0].vx, scrn[0].vy, scrn[6].vx, scrn[6].vy, scrn[7].vx, scrn[7].vy);
	z0 = myAddPrimClip(scrn[7].vz, drawpacket);
	myAddPacket(sizeof(LINE_F3));

	// Pair4: 1->7->5
	line = (LINE_F3 *)drawpacket;
	setLineF3(line);
	setRGB0(line, colour.r, colour.g, colour.b);
	setXY3(line, scrn[1].vx, scrn[1].vy, scrn[7].vx, scrn[7].vy, scrn[5].vx, scrn[5].vy);
	z0 = myAddPrimClip(scrn[5].vz, drawpacket);
	myAddPacket(sizeof(LINE_F3));

	// Pair5: 6->4->5
	line = (LINE_F3 *)drawpacket;
	setLineF3(line);
	setRGB0(line, colour.r, colour.g, colour.b);
	setXY3(line, scrn[6].vx, scrn[6].vy, scrn[4].vx, scrn[4].vy, scrn[5].vx, scrn[5].vy);
	z0 = myAddPrimClip(scrn[5].vz, drawpacket);
	myAddPacket(sizeof(LINE_F3));
}

#endif
void ConvertToScreenCoords(SVECTORPC *local, SVECTORPC *screen, int nVertices) {
	int i;
	SVECTORPC *in = local;
	SVECTORPC *out = screen;
	int32 flag, p;

	for (i = 0; i < nVertices; i++) {
		// Note, store the result back on top of itself !
		gte_RotTransPers_pc(in, out, &p, &flag, (int32 *)&(out->vz));
		out->vz <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		p = (flag & (1 << 31));
		out->pad = (short)(p >> 16);
		in++;
		out++;
	}
}

void ConvertToScreenCoords(SVECTOR *local, SVECTOR *screen, int nVertices) {
	int i;
	SVECTOR *in = local;
	SVECTOR *out = screen;
	int32 flag, p;

	for (i = 0; i < nVertices; i++) {
		// Note, store the result back on top of itself !
		gte_RotTransPers(in, (int32 *)out, &p, &flag, (int32 *)&(out->vz));
		out->vz <<= 2; // multiply by 4 to cope with AverageZ later on dividing by 4
		p = (flag & (1 << 31));
		out->pad = (short)(p >> 16);
		in++;
		out++;
	}
}

} // End of namespace ICB
