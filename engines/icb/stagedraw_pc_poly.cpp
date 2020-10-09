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
#include "engines/icb/global_objects.h"
#include "engines/icb/stage_draw.h"
#include "engines/icb/debug.h"
#include "engines/icb/mission.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/surface_manager.h"
#include "engines/icb/session.h"
#include "engines/icb/common/px_staticlayers.h"
#include "engines/icb/shake.h"
#include "engines/icb/global_objects_psx.h"
#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/gfx/psx_scrn.h"
#include "engines/icb/common/px_capri_maths.h"
#include "engines/icb/gfx/psx_pcgpu.h"
#include "engines/icb/gfx/gfxstub.h"
#include "engines/icb/gfx/psx_camera.h"
#include "engines/icb/gfx/psx_tman.h"
#include "engines/icb/gfx/rap_api.h"
#include "engines/icb/gfx/rab_api.h"
#include "engines/icb/gfx/psx_pchmd.h"
#include "engines/icb/actor.h"
#include "engines/icb/actor_pc.h"
#include "engines/icb/drawpoly_pc.h"
#include "engines/icb/shadow_pc.h"
#include "engines/icb/gfx/gfxstub_dutch.h"
#include "engines/icb/gfx/gfxstub_rev_dutch.h"
#include "engines/icb/common/px_bones.h"
#include "engines/icb/common/revtex_api.h"
#include "engines/icb/common/pc_props.h"
#include "engines/icb/bone.h"
#include "engines/icb/res_man.h"

namespace ICB {

void AddDynamicLight(PSXLampList &lamplist, _logic *log);

typedef struct {
	uint16 x0, y0, z0;
	uint16 x1, y1, z1;
//	uint16 map[0];
} _px_prop_zmap_;
/* Local Defines */
#define ACTORSORTBUFFERSIZE (256 * 1024)
#define MAXACTORQTY MAXIMUM_POTENTIAL_ON_SCREEN_ACTOR_QUANTITY

RevRenderDevice revRen;
int RGBWidth = SCREEN_WIDTH;                // width
int RGBHeight = SCREEN_DEPTH;               // height
int RGBBytesPerPixel = 4;                   // 32 bit
int RGBPitch = RGBWidth * RGBBytesPerPixel; // pitch
int ZBytesPerPixel = 2;                     // 16bit z-buffer
int ZPitch = ZBytesPerPixel * SCREEN_WIDTH; // z-pitch
char *pActorBuffer = NULL;                  // buffer for drawing actors
char *pRGB = NULL;                          // buffer for RGB data
char *pZa = NULL;                           // buffer for actor z data
char *pZfx = NULL;                          // buffer for fx z data
char *pZ = NULL;                            // Current z buffer being used by the renderer
int mip_map_level = 0;
TextureManager the_tman;
#define ZBUFFERSIZE (2 * SCREEN_WIDTH * SCREEN_DEPTH)

// Stage draw composition table ... keeps track of the tiles which need drawing
uint32 *pUsedTiles = NULL;

// Init the dutch dll thing render device etc...
void InitRevRenderDevice() {
	InitDrawing();
	tman = &the_tman;
	tman->Init(0, 0, 1024, 512);

	if (!pActorBuffer) {
		// Allocate the RGB and Z maps together ( two z maps acotr + effects )
		pActorBuffer = new char[2 * ZBUFFERSIZE + (RGBPitch * RGBHeight)];

		// Setup the effects z buffer pointer
		pZfx = pActorBuffer;

		// Setup the actor z buffer pointer
		pZ = pZa = pZfx + ZBUFFERSIZE;

		// Setup the rgb buffer pointer
		pRGB = pZa + ZBUFFERSIZE;
	}

	memset(pZa, 0xff, ZPitch * RGBHeight);
	memset(pZfx, 0xff, ZPitch * RGBHeight);
	memset(pRGB, 0, RGBPitch * RGBHeight);

	revRen.width = RGBWidth;
	revRen.stride = RGBWidth * RGBBytesPerPixel;
	revRen.height = RGBHeight;
	revRen.RGBdata = (u_char *)pRGB;
	revRen.Zdata = (u_short *)pZa;
	SetRenderDevice(&revRen);

	if (pUsedTiles == NULL)
		pUsedTiles = new uint32[TILE_COUNT];
	memset(pUsedTiles, 0, TILE_COUNT << 2);
}

void DestoryRevRenderDevice() {
	if (pActorBuffer)
		delete[] pActorBuffer;

	pActorBuffer = NULL;
	pRGB = NULL;
	pZ = NULL;
	pZa = NULL;
	pZfx = NULL;

	if (pUsedTiles)
		delete[] pUsedTiles;
}

#define MAX_NUM_TEX_HANS (256)
#define TEX 0
#define PAL 1
TextureHandle *texHans[MAX_NUM_TEX_HANS];
uint32 texHanHashs[MAX_NUM_TEX_HANS][2];
uint32 texHanBaseHashs[MAX_NUM_TEX_HANS];
int numTexHans = 0;

void ClearTextures() {
	int i;

	// remove all textures
	for (i = 0; i < numTexHans; i++) {
		texHanHashs[i][TEX] = 0; // no hash so don't think we have this texture
		texHanHashs[i][PAL] = 0; // no hash so don't think we have this texture
		texHanBaseHashs[i] = 0;  // ditto

		UnregisterTexture(texHans[i]);
	}

	numTexHans = 0;
}

#define BM_ID 0x4d42

// open a new texture, add it to position numTexHans of texHans list...
void OpenTexture(const char *tex_name, uint32 tex_hash, const char *pal_name, uint32 pal_hash, const char *base, uint32 base_hash) {
	// Load the texture
	revtex_API *rTexAPI = (revtex_API *)rs_anims->Res_open(tex_name, tex_hash, base, base_hash);

	// Check the texture file is correct ID & schema
	if ((*(uint32 *)rTexAPI->id) != (*(uint32 *)const_cast<char *>(REVTEX_API_ID)))
		Fatal_error("Invalid revtex_API id file %s API %s in file %s", rTexAPI->id, REVTEX_API_ID, tex_name);

	if (rTexAPI->schema != REVTEX_API_SCHEMA)
		Fatal_error("Invalid revtex_API file schema file %d API %d in file %s", rTexAPI->schema, REVTEX_API_SCHEMA, tex_name);

	// Load the palette (it might be the same file !)
	revtex_API *rPalAPI = (revtex_API *)rs_anims->Res_open(pal_name, pal_hash, base, base_hash);

	// Is the palette different ?
	if (rPalAPI != rTexAPI) {
		// It's different !
		if ((*(uint32 *)rPalAPI->id) != (*(uint32 *)const_cast<char *>(REVTEX_API_ID)))
			Fatal_error("Invalid revtex_API id file %s API %s in file %s", rTexAPI->id, REVTEX_API_ID, pal_name);

		if (rPalAPI->schema != REVTEX_API_SCHEMA)
			Fatal_error("Invalid revtex_API file schema file %d API %d in file %s", rTexAPI->schema, REVTEX_API_SCHEMA, pal_name);

		// Copy the palette into the texture
		memcpy(rTexAPI->palette, rPalAPI->palette, 256 * sizeof(uint32));
	}

	// Set up RevTexture structure
	RevTexture revTex;
	revTex.palette = rTexAPI->palette;
	revTex.width = rTexAPI->width;
	revTex.height = rTexAPI->height;
	for (int i = 0; i < 9; i++) {
		revTex.level[i] = (uint8 *)rTexAPI + rTexAPI->levelOffset[i];
	}

	// Register the texture
	texHans[numTexHans] = RegisterTexture(&revTex);
	texHanHashs[numTexHans][TEX] = tex_hash;
	texHanHashs[numTexHans][PAL] = pal_hash;
	texHanBaseHashs[numTexHans] = base_hash;

	// increase number of texHans
	numTexHans++;

	// Purge the texture out of resman to avoid us ever trying to remap an already mapped texture
	rs_anims->Res_purge(tex_name, tex_hash, base, base_hash, 0);
}

// gets a handle to a texture, loading it in if necesary
TextureHandle *GetRegisteredTexture(const char *tex_name, uint32 tex_hash, const char *pal_name, uint32 pal_hash, const char *base, uint32 base_hash) {
	int i;

	// Need to ensure the hash values are set
	if (tex_hash == NULL_HASH)
		tex_hash = HashString(tex_name);
	if (base_hash == NULL_HASH)
		base_hash = HashString(base);
	if (pal_hash == NULL_HASH)
		pal_hash = HashString(pal_name);

	// if already registered then return it...
	for (i = 0; i < numTexHans; i++)
		if ((texHanHashs[i][TEX] == tex_hash) && (texHanHashs[i][PAL] == pal_hash) && (texHanBaseHashs[i] == base_hash))
			return texHans[i];

	// okay add it
	OpenTexture(tex_name, tex_hash, pal_name, pal_hash, base, base_hash);

	// it will always be at position numTexHans-1 because it has just been added
	return texHans[numTexHans - 1];
}

// preload a texture, does a GetRegisteredTexture but doesn't return it...
void PreRegisterTexture(const char *tex_name, uint32 tex_hash, const char *pal_name, uint32 pal_hash, const char *base, uint32 base_hash) {
	TextureHandle *th = GetRegisteredTexture(tex_name, tex_hash, pal_name, pal_hash, base, base_hash);
	if (!th)
		Fatal_error("Cant open texture:palette %s:%s\n", tex_name, pal_name);
}

void drawObjects(SDactor &act, PSXLampList &lamplist, PSXrgb *pAmbient, PSXShadeList &shadelist, MATRIXPC *local2screen, int *brightnessReturn) {
	psxActor &actor = act.psx_actor;
	rap_API *mesh;
	rap_API *smesh;
	rap_API *pose;
	rab_API *bones;

	_vox_image *&vox = act.log->voxel_info;
	_mega *&mega = act.log->mega;

	// Load the actor files
	TextureHandle *texHan = GetRegisteredTexture(vox->texture_name, vox->texture_hash, vox->palette_name, vox->palette_hash, vox->base_path, vox->base_path_hash);

	// Set the POSE name
	char *poseName;
	uint32 poseHash;

	if ((act.log->cur_anim_type == __PROMOTED_NON_GENERIC) || (act.log->cur_anim_type == __NON_GENERIC)) {
		// For special custom animation is it
		// vox->custom_pose_name, vox->custom_pose_hash
		poseName = vox->custom_pose_name;
		poseHash = vox->custom_pose_hash;
	} else {
		// For ordinary animations it is
		// vox->pose_name, vox->pose_hash
		poseName = vox->pose_name;
		poseHash = vox->pose_hash;
	}

	// Load the POSE
	pose = (rap_API *)rs_anims->Res_open(poseName, poseHash, vox->base_path, vox->base_path_hash);
	ConvertRAP(pose);
	// check the pose data
	if (*(int *)pose->id != *(int *)const_cast<char *>(RAP_API_ID))
		Fatal_error("Pose Invalid rap ID rap_api.h %s file:%s file:%s", pose->id, RAP_API_ID, poseName);
	if (pose->schema != RAP_API_SCHEMA)
		Fatal_error("Wrong rap schema value file %d api %d file:%s", pose->schema, RAP_API_SCHEMA, poseName);

	// Load the MESH
	mesh = (rap_API *)rs_anims->Res_open(vox->mesh_name, vox->mesh_hash, vox->base_path, vox->base_path_hash);
	ConvertRAP(mesh);
	// check the mesh data
	if (*(int *)mesh->id != *(int *)const_cast<char *>(RAP_API_ID))
		Fatal_error("Mesh Invalid rap ID rap_api.h %s file:%s file:%s", mesh->id, RAP_API_ID, vox->mesh_name);
	if (mesh->schema != RAP_API_SCHEMA)
		Fatal_error("Mesh rap schema value file %d api %d file:%s", mesh->schema, RAP_API_SCHEMA, vox->mesh_name);

	// Load the SHADOW MESH
	smesh = (rap_API *)rs_anims->Res_open(vox->shadow_mesh_name, vox->shadow_mesh_hash, vox->base_path, vox->base_path_hash);
	ConvertRAP(smesh);
	// check the shadow mesh data
	if (*(int *)smesh->id != *(int *)const_cast<char *>(RAP_API_ID))
		Fatal_error("Mesh Invalid rap ID rap_api.h %s file:%s file:%s", smesh->id, RAP_API_ID, vox->shadow_mesh_name);
	if (smesh->schema != RAP_API_SCHEMA)
		Fatal_error("Mesh rap schema value file %d api %d file:%s", smesh->schema, RAP_API_SCHEMA, vox->shadow_mesh_name);

	// Load the BONES
	bones = (rab_API *)rs_anims->Res_open(vox->anim_name[act.log->cur_anim_type], vox->anim_name_hash[act.log->cur_anim_type], vox->base_path, vox->base_path_hash);
	// check the bones...
	if (*(int *)bones->id != *(int *)const_cast<char *>(RAB_API_ID))
		Fatal_error("Bones Invalid rab ID rab_api.h %s file:%s file:%s", bones->id, RAB_API_ID, vox->anim_name[act.log->cur_anim_type]);
	if (bones->schema != RAB_API_SCHEMA)
		Fatal_error("Bones rab schema value file %d api %d file:%s", bones->schema, RAB_API_SCHEMA, vox->anim_name[act.log->cur_anim_type]);

	// Check the data is all in still and resman has settled.
	if (*(int *)mesh->id != *(int *)const_cast<char *>(RAP_API_ID)) {
		mesh = (rap_API *)rs_anims->Res_open(vox->mesh_name, vox->mesh_hash, vox->base_path, vox->base_path_hash);
		ConvertRAP(mesh);
	}

	if (*(int *)smesh->id != *(int *)const_cast<char *>(RAP_API_ID)) {
		smesh = (rap_API *)rs_anims->Res_open(vox->shadow_mesh_name, vox->shadow_mesh_hash, vox->base_path, vox->base_path_hash);
		ConvertRAP(smesh);
	}

	if (*(int *)bones->id != *(int *)const_cast<char *>(RAB_API_ID)) {
		bones = (rab_API *)rs_anims->Res_open(vox->anim_name[act.log->cur_anim_type], vox->anim_name_hash[act.log->cur_anim_type], vox->base_path, vox->base_path_hash);
	}

	if (*(int *)pose->id != *(int *)const_cast<char *>(RAP_API_ID)) {
		pose = (rap_API *)rs_anims->Res_open(poseName, poseHash, vox->base_path, vox->base_path_hash);
		ConvertRAP(pose);
	}

	if (*(int *)mesh->id != *(int *)const_cast<char *>(RAP_API_ID)) {
		mesh = (rap_API *)rs_anims->Res_open(vox->mesh_name, vox->mesh_hash, vox->base_path, vox->base_path_hash);
		ConvertRAP(mesh);
	}

	if (*(int *)smesh->id != *(int *)const_cast<char *>(RAP_API_ID)) {
		smesh = (rap_API *)rs_anims->Res_open(vox->shadow_mesh_name, vox->shadow_mesh_hash, vox->base_path, vox->base_path_hash);
		ConvertRAP(smesh);
	}

	if (*(int *)bones->id != *(int *)const_cast<char *>(RAB_API_ID)) {
		bones = (rab_API *)rs_anims->Res_open(vox->anim_name[act.log->cur_anim_type], vox->anim_name_hash[act.log->cur_anim_type], vox->base_path, vox->base_path_hash);
	}

	if (*(int *)pose->id != *(int *)const_cast<char *>(RAP_API_ID)) {
		pose = (rap_API *)rs_anims->Res_open(poseName, poseHash, vox->base_path, vox->base_path_hash);
		ConvertRAP(pose);
	}

	// FINAL CHECKS
	if (*(int *)mesh->id != *(int *)const_cast<char *>(RAP_API_ID))
		Fatal_error("Failed to get MESH %s settled within Resman", vox->mesh_name);
	if (*(int *)smesh->id != *(int *)const_cast<char *>(RAP_API_ID))
		Fatal_error("Failed to get SHADOW MESH %s settled within Resman", vox->shadow_mesh_name);
	if (*(int *)bones->id != *(int *)const_cast<char *>(RAB_API_ID))
		Fatal_error("Failed to get BONES %s settled within Resman", vox->anim_name[act.log->cur_anim_type]);
	if (*(int *)pose->id != *(int *)const_cast<char *>(RAP_API_ID))
		Fatal_error("Failed to get POSE %s settled within Resman", poseName);

	// FINISHED LOADING
	// Turn this code back after the demo
	if (act.frame >= bones->nFrames) {
		Fatal_error("stagedraw_pc_poly Illegal frame %d bones %d anim %s", act.frame, bones->nFrames, vox->anim_name[act.log->cur_anim_type]);
	}

	int f = act.frame;
	if ((f < 0) || (f >= (int)bones->nFrames))
		f = (bones->nFrames - 1);

	Bone_Frame *frame;

	frame = bones->GetFrame(f);

	selFace = 65535;

	_drawLit = 1;
	_drawTxture = 1;
	_drawWfrm = 0;
	_drawGouraud = 1;
	_drawDebugMesh = 0;

	if (px.texturedActors) {
		ChooseTexture(texHan);
		_drawTxture = 1;
	} else {
		_drawTxture = 0;
		_drawDebugMesh = 1;
	}

	if (px.litActors) {
		_drawLit = 1;
	} else {
		_drawLit = 0;
	}

	if (px.polyActors) {
		_drawPolys = 1;
	} else {
		_drawPolys = 0;
		_drawDebugMesh = 1;
	}

	if (px.wireframeActors) {
		_drawWfrm = 1;
		_drawDebugMesh = 1;
	} else {
		_drawWfrm = 0;
	}

	int nShadows = 0;
	if (mega->drawShadow) {
		// Complex shadows have up to nShadows of them
		// Note: a top-down BLACK will get added if NO shadows are cast
		nShadows = px.actorShadows;
	}

	SVECTORPC p_n[MAX_SHADOWS];
	int p_d[MAX_SHADOWS];

	for (int i = 0; i < MAX_SHADOWS; i++) {
		p_n[i].vx = 0;
		p_n[i].vy = 1;
		p_n[i].vz = 0;
		p_d[i] = (int)mega->actor_xyz.y; // At the actor's feet
	}

	// head movement

	// bone deformation
	BoneDeformation *myBones[MAX_DEFORMABLE_BONES];

	// setup bone pointers
	myBones[NECK_DEFORMATION] = &(vox->neckBone);
	myBones[JAW_DEFORMATION] = &(vox->jawBone);
	myBones[LOOK_DEFORMATION] = &(vox->lookBone);
	myBones[SPARE_DEFORMATION] = NULL; // no 4th deformation (unless player...)

	// if player then update player bones
	if (MS->player.log == act.log)
		myBones[SPARE_DEFORMATION] = &(MS->player.shotDeformation);

	int uvframe;

	// For dead things do not draw the animating polygons
	if (g_mission->session->objects == NULL || strcmp(act.log->GetName(), "StageView") == 0) {
		uvframe = gameCycle;
	} else {
		c_game_object *ob = (c_game_object *)MS->objects->Fetch_item_by_name(act.log->GetName());

		int32 ret = ob->GetVariable("state");

		// The snow_suit in Car_5 does not have a state flag
		// so can't test ret == -1
		// Basically if you don't have a state flag -
		// you are assumed to be dead

		uvframe = -1;
		if (ret != -1) {
			ret = ob->GetIntegerVariable(ret);
			// which means ALIVE
			if (ret == 0)
				uvframe = gameCycle;
		}
	}

	// No muzzle flash being drawn at the moment
	DrawActor4PC(&actor, (psxCamera *)&MS->GetCamera(), frame, mesh, pose, smesh, pAmbient, &lamplist, &shadelist, nShadows, p_n, p_d, _drawDebugMesh, uvframe, myBones,
	             brightnessReturn, local2screen);

	// A white light is 4096+4096+4096
	const int FullWhiteLight = (4096 * 3);
	int inShadePercentage;

	inShadePercentage = (int)(mega->inShadePercentage); // 0-100

	int inShadeBrightness = ((FullWhiteLight * inShadePercentage) / 100);

	if ((*brightnessReturn) < inShadeBrightness)
		mega->in_shade = TRUE8;
	else
		mega->in_shade = FALSE8;
}

int drawSpecialObjects(SDactor &actor, MATRIXPC *local2screen, int brightness, SVECTOR *minBBox, SVECTOR *maxBBox) {
	// Shooting ?
	int mflash = 0;
	SVECTOR mpos;

	_mega *&mega = actor.log->mega;
	_vox_image *&vox = actor.log->voxel_info;

	if (mega->is_shooting) {
		// Try to find the interaction marker !
		PXanim *pAnim =
		    (PXanim *)rs_anims->Res_open(vox->info_name[actor.log->cur_anim_type], vox->info_name_hash[actor.log->cur_anim_type], vox->base_path, vox->base_path_hash);

		PXframe *frm = PXFrameEnOfAnim(actor.frame, pAnim);
		if (frm->marker_qty > INT_POS) {
			PXmarker *marker = &(frm->markers[INT_POS]);
			uint8 mtype = (uint8)marker->GetType();
			if ((INT0_TYPE == mtype) || (INT_TYPE == mtype)) {
				// The interact marker exists
				PXfloat mx, my, mz;
				marker->GetXYZ(&mx, &my, &mz);

				PXfloat ox, oy, oz;
				marker = &(frm->markers[ORG_POS]);
				marker->GetXYZ(&ox, &oy, &oz);

				// Yeah - found a muzzle flash
				mflash = 1;

				// Muzzle flash only lasts for 1 cycle
				mega->is_shooting = 0;

				mpos.vx = (short)mx;
				mpos.vz = (short)mz;

				// Subtract the hip_height off the marker position
				mpos.vy = (short)(my - oy);

				// inititalise the cartridge case...
				mega->InitCartridgeCase(&mpos, (short)0); // oy);
			}
		}
	}

	SVECTOR local_mf_pos;

	if (vox->lookBone.boneNumber == 1) {
		MATRIX mf_matrix;
		SVECTOR rotation;

		rotation.vx = vox->lookBone.boneValue.vx;
		rotation.vy = vox->lookBone.boneValue.vz;
		rotation.vz = vox->lookBone.boneValue.vy;

		RotMatrix_gte(&rotation, &mf_matrix);

		ApplyMatrixSV(&mf_matrix, &mpos, &local_mf_pos);
	} else
		local_mf_pos = mpos;

	int ret = DrawActorSpecialEffectsPC(mflash, &local_mf_pos, 35, 5, mega->bulletOn, &(mega->bulletPos), (int)(mega->bulletColour), &(mega->breath), local2screen, brightness,
	                                    minBBox, maxBBox);

	return ret;
}

void StageDrawPoly(SDactor *actors, uint32 actorQty) {
	_set *set = &MS->set;

	MATRIXPC local2screen[MAXACTORQTY]; // the local>screen matrices for each actor...
	int brightnessValues[MAXACTORQTY];
	uint32 j;
	SVECTOR effectsMin[MAXACTORQTY];
	SVECTOR effectsMax[MAXACTORQTY];
	int effectsDrawn = 0;

	// Reset the renderers z range thingy
	ResetZRange();

	// Refresh the background
	set->RefreshBackdrop();

	// Call Set Draw to update all the props
	set->Set_draw(px.frag_help);

	// PSX LIGHT RIG SETUP
	ConvertRLP(set->GetPRig());
	// check files
	if (*(int *)set->GetPRig()->id != *(int *)const_cast<char *>(RLP_API_ID)) {
		Fatal_error("Invalid rlp ID file %s rlp_api.h %s", set->GetPRig()->id, RLP_API_ID);
	}
	if (set->GetPRig()->schema != RLP_API_SCHEMA) {
		Fatal_error("Invalid rlp schema file %d rlp_api.h %d", set->GetPRig()->schema, RLP_API_SCHEMA);
	}

	uint32 nl = set->GetPRig()->nLamps;
	if ((nl == 0) || (nl > MAX_NUMBER_LIGHTS)) {
		Fatal_error("Illegal number of lights %d in rlp file", nl);
	}

	// setup lamp list from rlp
	PSXLampList lamplist;
	PSXrgb *pAmbient;
	PSXrgb megaAmbient;
	PSXLamp **ppLamp = lamplist.lamps;
	PSXLamp *pLamp;
	uint32 *pState = lamplist.states;
	uint32 state;
	lamplist.n = nl;

	// Set the ambient
	pAmbient = &megaAmbient;

	// For each Lamp, set its current frame (state)
	for (uint32 l = 0; l < nl; ppLamp++, pState++, l++) {
		// Set the pointer to this lamp
		pLamp = (PSXLamp *)set->GetPRig()->GetLamp(l);
		*ppLamp = pLamp;

		// If the lamp has more than one state work out which
		// state to use by the state of the associated prop
		state = pLamp->nStates - 1;
		if (state > 0) {
			state = MS->Fetch_prop_state(pLamp->prop_name);
		}
		if (state >= pLamp->nStates) {
			state = pLamp->nStates - 1;
		}
		Tdebug("lights.txt", "cam %s Lamp %d State %d Prop_name %s", set->GetSetName(), l, state, (const char *)pLamp->prop_name);
		// Set which state to use for this lamp
		*pState = state;
	}

	// setup shade list from rlp
	PSXShadeList shadelist;
	PSXShade **ppShade = shadelist.shades;
	PSXShade *pShade;
	pState = shadelist.states;

	uint32 ns = set->GetPRig()->nShades;
	shadelist.n = ns;

	for (uint32 s = 0; s < ns; ppShade++, pState++, s++) {
		// Set the pointer to this lamp
		pShade = (PSXShade *)set->GetPRig()->GetShade(s);
		*ppShade = pShade;

		// If the lamp has more than one state work out which
		// state to use by the state of the associated prop
		state = pShade->nStates - 1;
		if (state > 0) {
			state = MS->Fetch_prop_state(pShade->prop_name);
		}
		if (state >= pShade->nStates) {
			state = pShade->nStates - 1;
		}
		Tdebug("lights.txt", "cam %s Shade %d State %d Prop_name %s", set->GetSetName(), s, state, (const char *)pShade->prop_name);
		// Set which state to use for this lamp
		*pState = state;
	}

	// Point the renderer at the actor z map
	pZ = pZa;
	revRen.Zdata = (u_short *)pZ;
	SetRenderDevice(&revRen);

	// Render all the actors
	for (j = 0; j < actorQty; j++) {
		// Finally Lighting Setup
		AddDynamicLight(lamplist, actors[j].log);

		// Set the mega's ambient light level
		megaAmbient = set->GetPRig()->ambient;

		megaAmbient.r = (uint16)(megaAmbient.r + actors[j].r);
		megaAmbient.g = (uint16)(megaAmbient.g + actors[j].g);
		megaAmbient.b = (uint16)(megaAmbient.b + actors[j].b);

		if (megaAmbient.r > 255)
			megaAmbient.r = 255;
		if (megaAmbient.g > 255)
			megaAmbient.g = 255;
		if (megaAmbient.b > 255)
			megaAmbient.b = 255;

		// Build the packet list for the actor
		drawObjects(actors[j], lamplist, pAmbient, shadelist, &(local2screen[j]), &(brightnessValues[j]));

		// Draw the effects
		effectsDrawn += drawSpecialObjects(actors[j], &(local2screen[j]), brightnessValues[j], &effectsMin[j], &effectsMax[j]);
	}

	drawOTList();

	// Load the prop file
	pcPropFile *propFile = set->GetProps();
	// Load the static file
	pcStaticLayers *sceneZ = set->GetStaticLayers();

	// Compensate for shake screen in the actor z map offset
	uint32 screenShakeOffset = 0;
	uint32 *safe_ad = (uint32 *)surface_manager->Lock_surface(working_buffer_id);
	uint32 pitch = surface_manager->Get_pitch(working_buffer_id);
	//uint32 bytesPerPixel= surface_manager->Get_BytesPP(working_buffer_id);
	uint32 *source = (uint32 *)pRGB;
	uint16 *zActor = (uint16 *)pZa;
	screenShakeOffset += ((GetShakeY() * pitch >> 2) + GetShakeX());

	RGBPitch = RGBWidth * RGBBytesPerPixel;

	{
		/*try*/ {
			// Fill in the table of used tiles
			for (j = 0; j < actorQty; j++) {
				// Get all the actors shadow tiles
				for (uint32 s = 0; s < actors[j].psx_actor.nShadows; s++) {
					int x0 = actors[j].psx_actor.shadowMinBox[s].vx + 320;
					int x1 = actors[j].psx_actor.shadowMaxBox[s].vx + 321;
					int y0 = actors[j].psx_actor.shadowMinBox[s].vy + 239;
					int y1 = actors[j].psx_actor.shadowMaxBox[s].vy + 240;

					if (x1 < 0)
						continue; // Completely off screen
					if (x0 > 640)
						continue; // Completely off screen
					if (y1 < 0)
						continue;
					if (y0 > 479)
						continue; // Completely off screen
					if (x0 < 0)       // Left hand clip
						x0 = 0;
					if (x1 > 640) // Right hand clip
						x1 = 640;
					if (y0 < 0) // Top edge clip
						y0 = 0;
					if (y1 > 479) // Botom edge clip
						y1 = 479;

					// Include shake screen offset in clipping
					int xShake = GetShakeX();
					if (xShake) {
						x0 += xShake;
						x1 += xShake;

						if (x1 < 0)
							continue; // Completely off screen
						if (x0 > 640)
							continue; // Completely off screen
						// Left edge clipping
						if (x0 < 0)
							x0 = 0;
						// Right edge clipping
						if (x1 > 640)
							x1 = 640;
						x0 -= xShake;
						x1 -= xShake;
					}

					int yShake = GetShakeY();
					if (yShake) {
						y0 += yShake;
						y1 += yShake;

						if (y1 < 0)
							continue; // Completely off screen
						if (y0 > 479)
							continue; // Completely off screen
						// Top edge clipping
						if (y0 < 0)
							y0 = 0;
						// Bottom edge clipping
						if (y1 > 479)
							y1 = 479;

						y0 -= yShake;
						y1 -= yShake;
					}

					// Mark these tiles as
					int zTys = y0 / TILE_HEIGHT;
					int zTxs = x0 / TILE_WIDTH;
					int tileOffset = (zTys * TILES_WIDE) + zTxs;
					int zTw = ((x1 + (TILE_WIDTH - 1)) / TILE_WIDTH) - zTxs;
					int zTh = ((y1 + (TILE_HEIGHT - 1)) / TILE_HEIGHT) - zTys;

					// Now compose this actor
					int rowStep = TILES_WIDE - zTw;
					for (int zTy = zTh; zTy; --zTy) {
						for (int zTx = zTw; zTx; --zTx) {
							++pUsedTiles[tileOffset];
							tileOffset++;
						}
						tileOffset += rowStep;
					}
				}

				// And the actor him/her self
				int x0 = actors[j].psx_actor.minBbox.vx + 320;
				int x1 = actors[j].psx_actor.maxBbox.vx + 321;
				int y0 = actors[j].psx_actor.minBbox.vy + 239;
				int y1 = actors[j].psx_actor.maxBbox.vy + 240;

				// Check if actor is completely of screen
				if (x1 < 0)
					continue; // Completely off screen
				if (x0 > 640)
					continue; // Completely off screen
				if (y1 < 0)
					continue; // Completely off screen
				if (y0 > 479)
					continue; // Completely off screen
				// Left hand clip
				if (x0 < 0)
					x0 = 0;
				// Right hand clip
				if (x1 > 640)
					x1 = 640;
				// Top edge clip
				if (y0 < 0)
					y0 = 0;
				// Botom edge clip
				if (y1 > 479)
					y1 = 479;
				// Include shake screen offset in clipping
				int xShake = GetShakeX();
				if (xShake) {
					x0 += xShake;
					x1 += xShake;

					if (x1 < 0)
						continue; // Completely off screen
					if (x0 > 640)
						continue; // Completely off screen
					// Left edge clipping
					if (x0 < 0)
						x0 = 0;
					// Right edge clipping
					if (x1 > 640)
						x1 = 640;
					x0 -= xShake;
					x1 -= xShake;
				}

				int yShake = GetShakeY();
				if (yShake) {
					y0 += yShake;
					y1 += yShake;

					if (y1 < 0)
						continue; // Completely off screen
					if (y0 > 479)
						continue; // Completely off screen
					// Top edge clipping
					if (y0 < 0)
						y0 = 0;
					// Bottom edge clipping
					if (y1 > 479)
						y1 = 479;
					y0 -= yShake;
					y1 -= yShake;
				}
				// Mark these tiles as
				int zTys = y0 / TILE_HEIGHT;
				int zTxs = x0 / TILE_WIDTH;
				int tileOffset = (zTys * TILES_WIDE) + zTxs;
				int zTw = ((x1 + (TILE_WIDTH - 1)) / TILE_WIDTH) - zTxs;
				int zTh = ((y1 + (TILE_HEIGHT - 1)) / TILE_HEIGHT) - zTys;

				// Now compose this actor
				int rowStep = TILES_WIDE - zTw;
				for (int zTy = zTh; zTy; --zTy) {
					for (int zTx = zTw; zTx; --zTx) {
						++pUsedTiles[tileOffset];
						tileOffset++;
					}
					tileOffset += rowStep;
				}
			}

			// Now composit the whole kit a cabudal
			uint32 tileOffset = 0;
			uint32 rowOffset = 0;
			uint32 pitchedRowOffset = 0;
			for (int zTy = TILES_HIGH; zTy; --zTy) {
				uint32 offset = rowOffset;
				uint32 pitchedOffset = pitchedRowOffset;

				// Go through all the tiles the actor is occupying
				for (int zTx = TILES_WIDE; zTx; --zTx) {
					if (pUsedTiles[tileOffset]) {
						// This tile is used, so Get the prop quantity
						uint32 propQty = propFile->GetPropQty();
						for (uint32 p = 0; p < propQty; p++) {
							// Get this prop
							pcPropRGB *pProp = propFile->GetProp(p);
							// Get the state for this prop
							uint32 stat = g_mission->session->Fetch_prop_state(const_cast<char *>(pProp->GetName()));
							// Get the state
							pcPropRGBState *pStat = pProp->GetState(stat);
							// Get a pointer to the z table
							uint16 *zTable = pStat->GetZTileTable(tileOffset);

							if (zTable != NULL) {
								uint16 *zPtr = zTable;
								uint16 *rowAd = zActor + offset;

								for (int y = 0; y < TILE_HEIGHT; y++) {
									uint16 *ptr = rowAd;
									for (int x = 0; x < TILE_WIDTH;) {
										// read the rle counters (trans/solid)
										int trans = ((int)*zPtr) & 0xff;
										int solid = ((int)*zPtr) >> 8;
										zPtr++;

										x += trans + solid;
										ptr += trans;

										while (solid--) {
											if (*ptr > *zPtr)
												*ptr = 0xffff;
											++ptr;
											++zPtr;
										}
									}
									rowAd += (SCREEN_WIDTH);
								}
							}

							// Now do the semi transparencies for this prop tile
							uint16 *tilePtrs = pStat->GetSemiTileTable(tileOffset); // pStat->GetSemiTileTable(tileOffset);
							if ((tilePtrs != NULL) && (px.semitransparencies == TRUE8)) {
								uint16 *tPtr = tilePtrs;
								uint32 *bufRGB = source + offset;
								uint16 *bufZ = zActor + offset;
#if 1
								for (int y = 0; y < TILE_HEIGHT; y++) {
									uint32 *aRGB = bufRGB;
									uint16 *aZ = bufZ;
									for (int x = 0; x < TILE_WIDTH;) {
										// read the rle counters (trans/solid)
										int trans = ((int)*tPtr) & 0xff;
										int solid = ((int)*tPtr) >> 8;
										tPtr++;
										aRGB += trans;
										aZ += trans;
										x += trans + solid;
										while (solid--) {
											if (*tPtr++ <= *aZ) {
												uint8 *pix = (uint8 *)aRGB;
												uint8 *t = (uint8 *)tPtr;
												for (int i = 0; i < 3; i++) {
													pix[i] = MIN(255, ((pix[i] * t[i]) >> 8) + t[i + 3]);
												}
											}
											tPtr += 3;
											aRGB++;
											aZ++;
										}
									}
									bufRGB += SCREEN_WIDTH;
									bufZ += SCREEN_WIDTH;
								}
#else
								uint32 RGB_ROW_STEP = RGBPitch - (TILE_WIDTH * RGBBytesPerPixel);
								uint32 Z_ROW_STEP = ZPitch - (TILE_WIDTH * ZBytesPerPixel);
								_asm {
									; // Load the counters and pointers
									mov  ecx, (TILE_HEIGHT*TILE_WIDTH)
									mov  esi, tPtr
									mov  edx, bufZ
									mov  edi, bufRGB
									; // Clear an MMX register to use with the unpack instruction
									psrlw MM0, 16
									; // Set up a mask for getting the multiplier and addition bit from the semi data.
									mov eax, 0x00ffffff
									xor ebx, ebx
									movd mm7, eax
									neg eax
									movd mm6, eax
									xor eax, eax
									prop_semi_actor_loop:
									; // Load the transparent count
									xor eax, eax
									mov al, byte ptr [esi]
									; // Step along the RLE stream
									inc esi
									; // subtract it from the counter
									sub ecx, eax
									; // move the actor z map pointer along
									shl eax, 1
									add edx, eax
									; // move the actor rgb pointer along
									shl eax, 1
									add edi, eax
									; // Load the solid count
									xor eax, eax
									mov al, byte ptr [esi]
									; // remove this from the count
									sub ecx, eax
									inc eax
									; // Move along in the rle stream
									inc esi
									prop_semi_solid_loop:
									; // Decrement the solid counter
									dec eax
									; // Have we finished ?
									jz end_prop_solid_loop
									; // Load the actor z
									mov bx, word ptr [edx]
									; // Load the semi transparencies z
									sub bx, word ptr [esi]
									; // Is the semitransparency closer ?
									jna prop_semi_skip
									; // Load the multiplier and colour data in to mm1
									movq MM1, [esi]
									; // Load the actor rgb into mm3
									movd MM3, dword ptr [edi]
									; // Copy the colour/multiplier into mm2
									movq  MM2, MM1
									; // Save the top byte of the actor colour (to preserve blending type)
									movq MM4, MM3
									; // Move the colour data into the write bit
									psrlq MM1, 40
									; // Save the blend type from the actor colour
									pand MM4, MM6
									; // Shift the multiplier down
									psrlq MM2, 16
									; // Mask of the unwanted bits from the multiplier
									pand MM2, MM7
									; // Unpack the actor rgb
									punpcklbw MM3, MM0
									; // Unpack the multiplier
									punpcklbw MM2, MM0
									; // multiply the actor
									pmullw MM3, MM2
									; // Scale it back
									psrlw MM3, 8
									; // Re pack the colour back to bytes
									packuswb MM3, MM0
									; // Add the colour data
									paddusb MM3, MM1
									; // Put the blend type back
									por MM3, MM4
									; // Now store the blended actor data
									movd dword ptr [edi], MM3
									prop_semi_skip:
									; // mov the pointers along
									add esi, 8
									add edx, 2
									add edi, 4
									; // And around again
									jmp prop_semi_solid_loop
									end_prop_solid_loop:
									; // on to the next span
									mov eax, ecx
									and eax, 63
									jnz prop_semi_actor_loop
									; // end of the row update the pointers
									add edx, Z_ROW_STEP
									add edi, RGB_ROW_STEP
									cmp ecx, 0
									jnz prop_semi_actor_loop
									emms;
								}
#endif
							}
						}

						// Now add the static semitransparencies
						uint16 *tilePtrs = sceneZ->GetSemiTileTable(tileOffset);
						if ((tilePtrs != NULL) && (px.semitransparencies == TRUE8)) {
							uint16 *tPtr = tilePtrs;
							uint32 *bufRGB = source + offset;
							uint16 *bufZ = zActor + offset;
#if 1
							for (int y = 0; y < TILE_HEIGHT; y++) {
								uint32 *aRGB = bufRGB;
								uint16 *aZ = bufZ;
								for (int x = 0; x < TILE_WIDTH;) {
									// read the rle counters (trans/solid)
									int trans = ((int)*tPtr) & 0xff;
									int solid = ((int)*tPtr) >> 8;
									tPtr++;
									aRGB += trans;
									aZ += trans;
									x += trans + solid;
									while (solid--) {
										if (*tPtr++ <= *aZ) {
											uint8 *pix = (uint8 *)aRGB;
											uint8 *t = (uint8 *)tPtr;
											for (int i = 0; i < 3; i++) {
												pix[i] = MIN(255, ((pix[i] * t[i]) >> 8) + t[i + 3]);
											}
										}
										tPtr += 3;
										aRGB++;
										aZ++;
									}
								}
								bufRGB += SCREEN_WIDTH;
								bufZ += SCREEN_WIDTH;
							}
#else
							uint32 RGB_ROW_STEP = RGBPitch - (TILE_WIDTH * RGBBytesPerPixel);
							uint32 Z_ROW_STEP = ZPitch - (TILE_WIDTH * ZBytesPerPixel);
							_asm {
								; // Load the counters and pointers
								mov  ecx, (TILE_HEIGHT*TILE_WIDTH)
								mov  esi, tPtr
								mov  edx, bufZ
								mov  edi, bufRGB
								; // Clear an MMX register to use with the unpack instruction
								psrlw MM0, 16
								; // Set up a mask for getting the multiplier and addition bit from the semi data.
								mov eax, 0x00ffffff
								xor ebx, ebx
								movd mm7, eax
								neg eax
								movd mm6, eax
								xor eax, eax
								static_semi_actor_loop:
								; // Load the transparent count
								xor eax, eax
								mov al, byte ptr [esi]
								; // Step along the RLE stream
								inc esi
								; // subtract it from the counter
								sub ecx, eax
								; // move the actor z map pointer along
								shl eax, 1
								add edx, eax
								; // move the actor rgb pointer along
								shl eax, 1
								add edi, eax
								; // Load the solid count
								xor eax, eax
								mov al, byte ptr [esi]
								; // remove this from the count
								sub ecx, eax
								inc eax
								; // Move along in the rle stream
								inc esi
								static_semi_solid_loop:
								; // Decrement the solid counter
								dec eax
								; // Have we finished ?
								jz end_static_solid_loop
								; // Load the actor z
								mov bx, word ptr [edx]
								; // Load the semi transparencies z
								sub bx, word ptr [esi]
								; // Is the semitransparency closer ?
								jna static_semi_skip
								; // Load the multiplier and colour data in to mm1
								movq MM1, [esi]
								; // Load the actor rgb into mm3
								movd MM3, dword ptr [edi]
								; // Copy the colour/multiplier into mm2
								movq MM2, MM1
								; // Save the top byte of the actor colour (to preserve blending type)
								movq MM4, MM3
								; // Move the colour data into the write bit
								psrlq MM1, 40
								; // Save the blend type from the actor colour
								pand MM4, MM6
								; // Shift the multiplier down
								psrlq MM2, 16
								; // Mask of the unwanted bits from the multiplier
								pand MM2, MM7
								; // Unpack the actor rgb
								punpcklbw MM3, MM0
								; // Unpack the multiplier
								punpcklbw MM2, MM0
								; // multiply the actor
								pmullw MM3, MM2
								; // Scale it back
								psrlw MM3, 8
								; // Re pack the colour back to bytes
								packuswb MM3, MM0
								; // Add the colour data
								paddusb MM3, MM1
								; // Put the blend type back
								por MM3, MM4
								; // Now store the blended actor data
								movd dword ptr [edi], MM3
								static_semi_skip:
								; // mov the pointers along
								add esi, 8
								add edx, 2
								add edi, 4
								; // And around again
								jmp static_semi_solid_loop
								end_static_solid_loop:
								; // on to the next span
								mov eax, ecx
								and eax, 63
								jnz static_semi_actor_loop
								; // end of the row update the pointers
								add edx, Z_ROW_STEP
								add edi, RGB_ROW_STEP
								cmp ecx, 0
								jnz static_semi_actor_loop
								emms;
							}
#endif
						}

						/* Composite the actor into the back buffer */
						// Get a pointer to the scene z table
						tilePtrs = sceneZ->GetTileTable(tileOffset);
						uint16 *zA = zActor + offset;
						uint32 *rgbA32 = source + offset;
						uint32 *rgbS32 = safe_ad + pitchedOffset + screenShakeOffset;
						if (tilePtrs != NULL) {
							uint16 *zPtr = tilePtrs;
							for (int y = 0; y < TILE_HEIGHT; y++) {
								uint16 *zDst = zA;
								uint32 *rgbSrc = rgbA32;
								uint32 *rgbDst = rgbS32;
								for (int x = 0; x < TILE_WIDTH;) {
									// read the rle counters (trans/solid)
									int trans = ((int)*zPtr) & 0xff;
									int solid = ((int)*zPtr) >> 8;
									zPtr++;

									x += trans + solid;

									while (trans--) {
										if (*zDst != 0xffff) {
											switch ((*rgbSrc) >> 30) {
											case 0:
												*rgbDst = *rgbSrc;
												break;
											case 1:
#if 1
											{
												// 32-bit BGR pixel
												uint8 *pixel = (uint8 *)rgbDst;
												uint8 *src = (uint8 *)rgbSrc;
												// Add from RGB components
												for (int i = 0; i < 3; i++) {
													pixel[i] = MIN(255, pixel[i] + src[i]);
												}
											}
#else
												_asm {
													mov esi, rgbSrc
													mov edi, rgbDst
													movd mm0, dword ptr [edi]
													movd mm1, dword ptr [esi]
													paddusb mm0, mm1
													movd dword ptr [edi], mm0
													emms;
												}
#endif
											break;
											case 2:
#if 1
											{
												// 32-bit BGR pixel
												uint8 *pixel = (uint8 *)rgbDst;
												uint8 *src = (uint8 *)rgbSrc;
												// Sub from RGB components
												for (int i = 0; i < 3; i++) {
													pixel[i] = MAX(0, pixel[i] - src[i]);
												}
											}
#else
												_asm {
													mov esi, rgbSrc
													mov edi, rgbDst
													movd mm0, dword ptr [edi]
													movd mm1, dword ptr [esi]
													psubusb mm0, mm1
													movd dword ptr [edi], mm0
													emms;
												}
#endif
											break;
											case 3:
#if 1
											{
												// 32-bit BGR pixel
												uint8 *pixel = (uint8 *)rgbDst;
												uint8 *src = (uint8 *)rgbSrc;
												// Add from RGB components
												for (int i = 0; i < 3; i++) {
													pixel[i] = (pixel[i] + src[i]) >> 1;
												}
											}
#else
												_asm {
													mov esi, rgbSrc
													mov edi, rgbDst
													psrlw mm7, 16
													movd mm0, dword ptr [edi]
													movd mm1, dword ptr [esi]
													punpcklbw mm0, mm7
													punpcklbw mm1, mm7
													paddusw mm0, mm1
													psrlw  mm0, 1
													packuswb mm0, mm7
													movd dword ptr [edi], mm0
													emms;
												}
												break;
#endif
											}
										}

										++zDst;
										++rgbDst;
										++rgbSrc;
									}

									while (solid--) {
										if (*zDst < *zPtr) {
											switch ((*rgbSrc) >> 30) {
											case 0:
												*rgbDst = *rgbSrc;
												break;
											case 1:
#if 1
											{
												// 32-bit BGR pixel
												uint8 *pixel = (uint8 *)rgbDst;
												uint8 *src = (uint8 *)rgbSrc;
												// Add from RGB components
												for (int i = 0; i < 3; i++) {
													pixel[i] = MIN(255, pixel[i] + src[i]);
												}
											}
#else
												_asm {
													mov esi, rgbSrc
													mov edi, rgbDst
													movd mm0, dword ptr [edi]
													movd mm1, dword ptr [esi]
													paddusb mm0, mm1
													movd dword ptr [edi], mm0
													emms;
												}
#endif
											break;
											case 2:
#if 1
											{
												// 32-bit BGR pixel
												uint8 *pixel = (uint8 *)rgbDst;
												uint8 *src = (uint8 *)rgbSrc;
												// Sub from RGB components
												for (int i = 0; i < 3; i++) {
													pixel[i] = MAX(0, pixel[i] - src[i]);
												}
											}
#else
												_asm {
													mov esi, rgbSrc
													mov edi, rgbDst
													movd mm0, dword ptr [edi]
													movd mm1, dword ptr [esi]
													psubusb mm0, mm1
													movd dword ptr [edi], mm0
													emms;
												}
#endif
											break;
											case 3:
#if 1
											{
												// 32-bit BGR pixel
												uint8 *pixel = (uint8 *)rgbDst;
												uint8 *src = (uint8 *)rgbSrc;
												// Add from RGB components
												for (int i = 0; i < 3; i++) {
													pixel[i] = (pixel[i] + src[i]) >> 1;
												}
											}
#else
												_asm {
													mov esi, rgbSrc
													mov edi, rgbDst
													psrlw mm7, 16
													movd mm0, dword ptr [edi]
													movd mm1, dword ptr [esi]
													punpcklbw mm0, mm7
													punpcklbw mm1, mm7
													paddusw mm0, mm1
													psrlw  mm0, 1
													packuswb mm0, mm7
													movd dword ptr [edi], mm0
													emms;
												}
												break;
#endif
											}
										}
										++zDst;
										++zPtr;
										++rgbDst;
										++rgbSrc;
									}
								}
								zA += SCREEN_WIDTH;
								rgbA32 += SCREEN_WIDTH;
								rgbS32 += pitch >> 2;
							}
						} else {
							for (int y = TILE_HEIGHT; y; --y) {
								uint16 *zDst = zA;
								uint32 *rgbSrc = rgbA32;
								uint32 *rgbDst = rgbS32;
								for (int x = TILE_WIDTH; x; --x) {
									if (*zDst != 0xffff) {
										switch ((*rgbSrc) >> 30) {
										case 0:
											*rgbDst = *rgbSrc;
											break;
										case 1:
#if 1
										{
											// 32-bit BGR pixel
											uint8 *pixel = (uint8 *)rgbDst;
											uint8 *src = (uint8 *)rgbSrc;
											// Add from RGB components
											for (int i = 0; i < 3; i++) {
												pixel[i] = MIN(255, pixel[i] + src[i]);
											}
										}
#else
											_asm {
												mov esi, rgbSrc
												mov edi, rgbDst
												movd mm0, dword ptr [edi]
												movd mm1, dword ptr [esi]
												paddusb mm0, mm1
												movd dword ptr [edi], mm0
												emms;
											}
#endif
										break;
										case 2:
#if 1
										{
											// 32-bit BGR pixel
											uint8 *pixel = (uint8 *)rgbDst;
											uint8 *src = (uint8 *)rgbSrc;
											// Sub from RGB components
											for (int i = 0; i < 3; i++) {
												pixel[i] = MAX(0, pixel[i] - src[i]);
											}
										}
#else
											_asm {
												mov esi, rgbSrc
												mov edi, rgbDst
												movd mm0, dword ptr [edi]
												movd mm1, dword ptr [esi]
												psubusb mm0, mm1
												movd dword ptr [edi], mm0
												emms;
											}
#endif
										break;
										case 3:
#if 1
										{
											// 32-bit BGR pixel
											uint8 *pixel = (uint8 *)rgbDst;
											uint8 *src = (uint8 *)rgbSrc;
											// Add from RGB components
											for (int i = 0; i < 3; i++) {
												pixel[i] = (pixel[i] + src[i]) >> 1;
											}
										}
#else
											_asm {
												mov esi, rgbSrc
												mov edi, rgbDst
												psrlw mm7, 16
												movd mm0, dword ptr [edi]
												movd mm1, dword ptr [esi]
												punpcklbw mm0, mm7
												punpcklbw mm1, mm7
												paddusw mm0, mm1
												psrlw  mm0, 1
												packuswb mm0, mm7
												movd dword ptr [edi], mm0
												emms;
											}
#endif
										break;
										}
									}
									++zDst;
									++rgbDst;
									++rgbSrc;
								}
								zA += SCREEN_WIDTH;
								rgbA32 += SCREEN_WIDTH;
								rgbS32 += pitch >> 2;
							}
						}
					}
					tileOffset++;
					offset += TILE_WIDTH;
					pitchedOffset += TILE_WIDTH;
				}
				rowOffset += TILE_HEIGHT * SCREEN_WIDTH;
				pitchedRowOffset += (TILE_HEIGHT * pitch >> 2);
			}
		}

		// Now Draw/Compose the special effects (Don't bother if they don't have MMX)
		if (surface_manager->HasMMX()) {
			/*try*/ {
				if (effectsDrawn) {
					// Point the renderer at the effects z map
					pZ = pZfx;
					revRen.Zdata = (u_short *)pZ;
					SetRenderDevice(&revRen);
					drawOTList();

					for (j = 0; j < actorQty; j++) {
						int x0 = effectsMin[j].vx + 320;
						int x1 = effectsMax[j].vx + 321;
						int y0 = effectsMin[j].vy + 239;
						int y1 = effectsMax[j].vy + 240;
						//						int z0 = effectsMin[j].vz + 0;
						//						int z1 = effectsMax[j].vz + 1;

						if (x1 < x0)
							continue; // Not a valid BBox

						if (y1 < y0)
							continue; // Not a valid BBox

						// Check if actor is completely of screen
						if (x1 < 0)
							continue; // Completely off screen
						if (x0 > 640)
							continue; // Completely off screen
						if (y1 < 0)
							continue; // Completely off screen
						if (y0 > 479)
							continue; // Completely off screen

						// Left hand clip
						if (x0 < 0)
							x0 = 0;

						// Right hand clip
						if (x1 > 640)
							x1 = 640;

						// Top edge clip
						if (y0 < 0)
							y0 = 0;

						// Botom edge clip
						if (y1 > 479)
							y1 = 479;

						// Include shake screen offset in clipping
						int xShake = GetShakeX();
						if (xShake) {
							x0 += xShake;
							x1 += xShake;

							if (x1 < 0)
								continue; // Completely off screen
							if (x0 > 640)
								continue; // Completely off screen

							// Left edge clipping
							if (x0 < 0)
								x0 = 0;

							// Right edge clipping
							if (x1 > 640)
								x1 = 640;

							x0 -= xShake;
							x1 -= xShake;
						}

						int yShake = GetShakeY();
						if (yShake) {
							y0 += yShake;
							y1 += yShake;

							if (y1 < 0)
								continue; // Completely off screen
							if (y0 > 479)
								continue; // Completely off screen

							// Top edge clipping
							if (y0 < 0)
								y0 = 0;

							// Bottom edge clipping
							if (y1 > 479)
								y1 = 479;

							y0 -= yShake;
							y1 -= yShake;
						}

						// Find which tiles z tiles the actor is at
						int zTys = y0 / TILE_HEIGHT;
						int zTxs = x0 / TILE_WIDTH;
						int tileOffset = (zTys * TILES_WIDE) + zTxs;
						//						int tileStart = tileOffset;
						int zTw = ((x1 + (TILE_WIDTH - 1)) / TILE_WIDTH) - zTxs;
						int zTh = ((y1 + (TILE_HEIGHT - 1)) / TILE_HEIGHT) - zTys;
						uint32 tileRowOffset = (TILE_HEIGHT * zTys * SCREEN_WIDTH) + (zTxs * TILE_WIDTH);
						uint32 pitchedTileRowOffset = (TILE_HEIGHT * zTys * pitch >> 2) + (zTxs * TILE_WIDTH);

						// Now composite the effects
						for (int zTy = 0; zTy < zTh; zTy++) {
							// Go through all the tiles the actor is occupying
							uint32 offset = tileRowOffset;
							uint32 pitchedOffset = pitchedTileRowOffset;

							for (int zTx = 0; zTx < zTw; zTx++) {
								// Get a pointer to the scene z table
								uint16 *tilePtrs = sceneZ->GetTileTable(tileOffset);
								uint16 *zA = (uint16 *)pZfx + offset;
								uint32 *rgbA32 = source + offset;
								uint32 *rgbS32 = safe_ad + pitchedOffset + screenShakeOffset;

								if (tilePtrs != NULL) {
									uint16 *zPtr = tilePtrs;
									for (int y = 0; y < TILE_HEIGHT; y++) {
										uint16 *zDst = zA;
										uint32 *rgbSrc = rgbA32;
										uint32 *rgbDst = rgbS32;
										for (int x = 0; x < TILE_WIDTH;) {
											// read the rle counters (trans/solid)
											int trans = ((int)*zPtr) & 0xff;
											int solid = ((int)*zPtr) >> 8;
											zPtr++;

											x += trans + solid;

											while (trans--) {
												if (*zDst < *(zDst + (ZBUFFERSIZE >> 1))) {
													switch ((*rgbSrc) >> 30) {
													case 0:
														*rgbDst = *rgbSrc;
														break;
													case 1:
#if 1
													{
														// 32-bit BGR pixel
														uint8 *pixel = (uint8 *)rgbDst;
														uint8 *src = (uint8 *)rgbSrc;
														// Add from RGB components
														for (int i = 0; i < 3; i++) {
															pixel[i] = MIN(255, pixel[i] + src[i]);
														}
													}
#else
														_asm {
															mov esi, rgbSrc
															mov edi, rgbDst
															movd mm0, dword ptr [edi]
															movd mm1, dword ptr [esi]
															paddusb mm0, mm1
															movd dword ptr [edi], mm0
															emms;
														}
#endif
													break;
													case 2:
#if 1
													{
														// 32-bit BGR pixel
														uint8 *pixel = (uint8 *)rgbDst;
														uint8 *src = (uint8 *)rgbSrc;
														// Sub from RGB components
														for (int i = 0; i < 3; i++) {
															pixel[i] = MAX(0, pixel[i] - src[i]);
														}
													}
#else
														_asm {
															mov esi, rgbSrc
															mov edi, rgbDst
															movd mm0, dword ptr [edi]
															movd mm1, dword ptr [esi]
															psubusb mm0, mm1
															movd dword ptr [edi], mm0
															emms;
														}
#endif
													break;
													case 3:
#if 1
													{
														// 32-bit BGR pixel
														uint8 *pixel = (uint8 *)rgbDst;
														uint8 *src = (uint8 *)rgbSrc;
														// Add from RGB components
														for (int i = 0; i < 3; i++) {
															pixel[i] = (pixel[i] + src[i]) >> 1;
														}
													}
#else
														_asm {
															mov esi, rgbSrc
															mov edi, rgbDst
															psrlw mm7, 16
															movd mm0, dword ptr [edi]
															movd mm1, dword ptr [esi]
															punpcklbw mm0, mm7
															punpcklbw mm1, mm7
															paddusw mm0, mm1
															psrlw  mm0, 1
															packuswb mm0, mm7
															movd dword ptr [edi], mm0
															emms;
														}
														break;
#endif
													}
												}

												++zDst;
												++rgbDst;
												++rgbSrc;
											}

											while (solid--) {
												if ((*zDst < *zPtr) && (*zDst < *(zDst + (ZBUFFERSIZE >> 1)))) {
													switch ((*rgbSrc) >> 30) {
													case 0:
														*rgbDst = *rgbSrc;
														break;
													case 1:
#if 1
													{
														// 32-bit BGR pixel
														uint8 *pixel = (uint8 *)rgbDst;
														uint8 *src = (uint8 *)rgbSrc;
														// Add from RGB components
														for (int i = 0; i < 3; i++) {
															pixel[i] = MIN(255, pixel[i] + src[i]);
														}
													}
#else
														_asm {
															mov esi, rgbSrc
															mov edi, rgbDst
															movd mm0, dword ptr [edi]
															movd mm1, dword ptr [esi]
															paddusb mm0, mm1
															movd dword ptr [edi], mm0
															emms;
														}
#endif
													break;
													case 2:
#if 1
													{
														// 32-bit BGR pixel
														uint8 *pixel = (uint8 *)rgbDst;
														uint8 *src = (uint8 *)rgbSrc;
														// Sub from RGB components
														for (int i = 0; i < 3; i++) {
															pixel[i] = MAX(0, pixel[i] - src[i]);
														}
													}
#else
														_asm {
															mov esi, rgbSrc
															mov edi, rgbDst
															movd mm0, dword ptr [edi]
															movd mm1, dword ptr [esi]
															psubusb mm0, mm1
															movd dword ptr [edi], mm0
															emms;
														}
#endif
													break;
													case 3:
#if 1
													{
														// 32-bit BGR pixel
														uint8 *pixel = (uint8 *)rgbDst;
														uint8 *src = (uint8 *)rgbSrc;
														// Add from RGB components
														for (int i = 0; i < 3; i++) {
															pixel[i] = (pixel[i] + src[i]) >> 1;
														}
													}
#else
														_asm {
															mov esi, rgbSrc
															mov edi, rgbDst
															psrlw mm7, 16
															movd mm0, dword ptr [edi]
															movd mm1, dword ptr [esi]
															punpcklbw mm0, mm7
															punpcklbw mm1, mm7
															paddusw mm0, mm1
															psrlw  mm0, 1
															packuswb mm0, mm7
															movd dword ptr [edi], mm0
															emms;
														}
#endif
													break;
													}
												}
												++zDst;
												++zPtr;
												++rgbDst;
												++rgbSrc;
											}
										}
										zA += SCREEN_WIDTH;
										rgbA32 += SCREEN_WIDTH;
										rgbS32 += pitch >> 2;
									}
								} else {
									for (int y = TILE_HEIGHT; y; --y) {
										uint16 *zDst = zA;
										uint32 *rgbSrc = rgbA32;
										uint32 *rgbDst = rgbS32;
										for (int x = TILE_WIDTH; x; --x) {
											if (*zDst < *(zDst + (ZBUFFERSIZE >> 1))) {
												switch ((*rgbSrc) >> 30) {
												case 0:
													*rgbDst = *rgbSrc;
													break;
												case 1:
#if 1
												{
													// 32-bit BGR pixel
													uint8 *pixel = (uint8 *)rgbDst;
													uint8 *src = (uint8 *)rgbSrc;
													// Add from RGB components
													for (int i = 0; i < 3; i++) {
														pixel[i] = MIN(255, pixel[i] + src[i]);
													}
												}
#else
													_asm {
														mov esi, rgbSrc
														mov edi, rgbDst
														movd mm0, dword ptr[edi]
														movd mm1, dword ptr[esi]
														paddusb mm0, mm1
														movd dword ptr[edi], mm0
														emms;
													}
#endif
												break;
												case 2:
#if 1
												{
													// 32-bit BGR pixel
													uint8 *pixel = (uint8 *)rgbDst;
													uint8 *src = (uint8 *)rgbSrc;
													// Sub from RGB components
													for (int i = 0; i < 3; i++) {
														pixel[i] = MAX(0, pixel[i] - src[i]);
													}
												}
#else
													_asm {
														mov esi, rgbSrc
														mov edi, rgbDst
														movd mm0, dword ptr[edi]
														movd mm1, dword ptr[esi]
														psubusb mm0, mm1
														movd dword ptr[edi], mm0
														emms;
													}
#endif
												break;
												case 3:
#if 1
												{
													// 32-bit BGR pixel
													uint8 *pixel = (uint8 *)rgbDst;
													uint8 *src = (uint8 *)rgbSrc;
													// Add from RGB components
													for (int i = 0; i < 3; i++) {
														pixel[i] = (pixel[i] + src[i]) >> 1;
													}
												}
#else
													_asm {
														mov esi, rgbSrc
														mov edi, rgbDst
														psrlw mm7, 16
														movd mm0, dword ptr[edi]
														movd mm1, dword ptr[esi]
														punpcklbw mm0, mm7
														punpcklbw mm1, mm7
														paddusw mm0, mm1
														psrlw  mm0, 1
														packuswb mm0, mm7
														movd dword ptr[edi], mm0
														emms;
													}
#endif
												break;
												}
											}
											++zDst;
											++rgbDst;
											++rgbSrc;
										}
										zA += SCREEN_WIDTH;
										rgbA32 += SCREEN_WIDTH;
										rgbS32 += pitch >> 2;
									}
								}
								tileOffset++;
								offset += TILE_WIDTH;
								pitchedOffset += TILE_WIDTH;
							}
							tileOffset += 10 - zTw;
							tileRowOffset += TILE_HEIGHT * SCREEN_WIDTH;
							pitchedTileRowOffset += (TILE_HEIGHT * pitch >> 2);
						}
					}
				}
			}
		}

		// Reset the actor z map
		uint32 *ptiles = pUsedTiles;
		char *pZC = pZa;
		for (int zTy = TILES_HIGH; zTy; zTy--) {
			// Go through all the tiles the actor is occupying
			for (int zTx = TILES_WIDE; zTx; zTx--) {
				if (*ptiles) {
					char *pCZ = pZC;
					for (int i = TILE_HEIGHT; i; i--) {
						memset(pCZ, 0xff, TILE_WIDTH << 1);
						pCZ += ZPitch;
					}
					*ptiles = 0;
				}
				ptiles++;
				pZC += TILE_WIDTH << 1;
			}
			pZC += (TILE_HEIGHT * ZPitch) - ZPitch;
		}
	}
	// Unlock the dx surface
	surface_manager->Unlock_surface(working_buffer_id);

	// Draw weather effects
	set->DrawWeather();

	// Draw the screen effects (widescreen fades ect)
	surface_manager->DrawEffects(working_buffer_id);
}

} // End of namespace ICB
