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

#include "engines/icb/common/px_common.h"
#include "engines/icb/p4.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/debug.h"
#include "engines/icb/mission.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_objects_psx.h"
#include "engines/icb/p4.h"
#include "engines/icb/actor.h"
#include "engines/icb/actor_pc.h"
#include "engines/icb/drawpoly_pc.h"
#include "engines/icb/common/px_capri_maths.h"
#include "engines/icb/gfx/gfxstub.h"
#include "engines/icb/gfx/gfxstub_dutch.h"
#include "engines/icb/gfx/gfxstub_rev_dutch.h"
#include "engines/icb/actor_view_pc.h"
#include "engines/icb/res_man.h"

#include "common/keyboard.h"

namespace ICB {

// Cute little puppies
extern char *pRGB;
extern char *pZa;
extern int32 RGBWidth;
extern int32 RGBHeight;
extern int32 RGBPitch;
extern int32 RGBBytesPerPixel;

RevRenderDevice renderingDevice;

extern int32 texturesUsedThisCycle;

// This controls autoanimation (0 = None, 1 = Backward, 2 = Forward)
uint32 auto_anim = 2;

// Camera and animation structures
PXanim *pxanim;
SVECTOR rot;   // Actor rotation
SVECTOR _crot; // Camera rotation
int32 uvframe = 0;

// Global filename stuff
char cluster_name[32];
uint32 cluster_name_hash;
char raj_name[128];
uint32 raj_name_hash;

// Pointers to useful strings for the current actor
char *character_name;
char *anim_name;
const char *weapon_name;
char *outfit_name;

int32 framenum;
int32 g_repeats;

// Do we allow keyboard input to affect the actor viewing
bool8 g_av_userControlled = FALSE8;

// Lighting structure and coordinates, colour components
int16 av_LightX;
int16 av_LightY;
int16 av_LightZ;
int32 av_LightR;
int32 av_LightG;
int32 av_LightB;
int32 av_LightA;
bool8 av_LightDir;
bool8 av_autoR;
bool8 av_autoG;
bool8 av_autoB;

// Render coordinates
int16 av_x, av_y, av_z;

// Prototypes for functions used in this module
TextureHandle *GetRegisteredTexture(const char *, uint32, const char *, uint32, const char *, uint32);
void DrawFrame(const int32 frame);
void MakeCameraView();
void ResetCamera();
void ResetActor();
void InitLight();
void SetLight(int32 falloff);
void AutoCycleLight();

#define LIGHT_DISTANCE_FROM_ACTOR 100
#define LIGHT_HEIGHT_LIMIT 200

void InitActorView(const char *name, const char *outfit, const char *weapon, const char *anim, int16 ix, int16 iy, int16 iz) {
	// Store initial offset coordinates
	av_x = ix;
	av_y = iy;
	av_z = iz;

	// Make hash filename of the character
	char h_character[8];
	HashFile(name, h_character);
	// Make hash filename of the outfit
	char h_outfit[8];
	HashFile(outfit, h_outfit);
	// Make the cluster name
	Common::sprintf_s(cluster_name, "\\C\\%s\\%s.OFT", h_character, h_outfit);
	// Hash value for this cluster name
	cluster_name_hash = NULL_HASH;

	ResetCamera();
	ResetActor();

	raj_name_hash = NULL_HASH;

	Common::sprintf_s(raj_name, "%s\\%s.raj", weapon, anim);

	anim_name = const_cast<char *>(anim);
	weapon_name = const_cast<char *>(weapon);
	outfit_name = const_cast<char *>(outfit);
	character_name = const_cast<char *>(name);

	// Animate the shape from frame to frame, looping
	framenum = 0;
	g_repeats = 0;
	auto_anim = 2;

	// Initialise a light to use
	InitLight();

	// Start the psx poly drawing ganky bit
	InitDrawing();

	// Load and select the appropriate texture
	char texture_name[128];
	uint32 texture_name_hash = NULL_HASH;

	Common::sprintf_s(texture_name, "material.revtex");

	TextureHandle *texHan = GetRegisteredTexture(texture_name, texture_name_hash, texture_name, texture_name_hash, cluster_name, cluster_name_hash);

	ChooseTexture(texHan);
}

void ChangeAnimPlaying(const char *pose, const char *anim, bool8 forwards, int32 repeats, int16 ix, int16 iy, int16 iz) {
	// Set pose
	if (pose) {
		// New pose
		weapon_name = const_cast<char *>(pose);
	} else {
		// Default is unarmed
		weapon_name = "unarmed";
	}

	// Require anim parameter
	if (anim == nullptr)
		Fatal_error("ChangeAnimPlaying() cannot set active animation to NULL!");

	// Remake raj filename
	raj_name_hash = NULL_HASH;

	Common::sprintf_s(raj_name, "%s\\%s.raj", weapon_name, anim);

	// Change animation to use
	anim_name = const_cast<char *>(anim);

	// Set direction to run the anim
	if (forwards)
		auto_anim = 2;
	else
		auto_anim = 1;

	pxanim = (PXanim *)rs_anims->Res_open(raj_name, raj_name_hash, cluster_name, cluster_name_hash);

	// Set to the starting frame of this anim
	if (forwards)
		framenum = 0;
	else
		framenum = pxanim->frame_qty - 2;

	g_repeats = repeats;

	// Store initial offset coordinates
	av_x = ix;
	av_y = iy;
	av_z = iz;

	ResetCamera();
}

int32 ActorViewDraw() {
	// Return value
	int32 returnStatus = MID_ANIMATION;

	// Alters the light nicely
	AutoCycleLight();
	// Call this each cycle to update our light
	SetLight(500);

	// This does most of the work
	DrawFrame(framenum);

	// This code acts upon user input to alter the actor's rotation and anim frame
	// from the initialised defaults.
	if (g_av_userControlled) {
		// Increment in degrees
		int32 dang = 5;

		// Actor rotation
		if (Read_DI_keys(Common::KEYCODE_LEFT)) {
			rot.vy = (int16)(rot.vy + 4096 * dang / 360);
		}
		if (Read_DI_keys(Common::KEYCODE_DOWN)) {
			rot.vy = (int16)(rot.vy - 4096 * dang / 360);
		}
		if (Read_DI_keys(Common::KEYCODE_UP)) {
			rot.vx = (int16)(rot.vx + 4096 * dang / 360);
		}
		if (Read_DI_keys(Common::KEYCODE_DOWN)) {
			rot.vx = (int16)(rot.vx - 4096 * dang / 360);
		}
		if (Read_DI_keys(Common::KEYCODE_PAGEUP)) {
			rot.vz = (int16)(rot.vz + 4096 * dang / 360);
		}
		if (Read_DI_keys(Common::KEYCODE_PAGEDOWN)) {
			rot.vz = (int16)(rot.vz - 4096 * dang / 360);
		}

		if (rot.vx > 4096)
			rot.vx -= 4096;
		if (rot.vy > 4096)
			rot.vy -= 4096;
		if (rot.vz > 4096)
			rot.vz -= 4096;

		if (rot.vx < -4096)
			rot.vx += 4096;
		if (rot.vy < -4096)
			rot.vy += 4096;
		if (rot.vz < -4096)
			rot.vz += 4096;

		// Set animation playing forwards
		if (Read_DI_keys(Common::KEYCODE_1)) {
			auto_anim = 2;
		}
		// Set animation playing backwards
		if (Read_DI_keys(Common::KEYCODE_2)) {
			auto_anim = 1;
		}
		// Previous frame
		if (Read_DI_once_keys(Common::KEYCODE_MINUS)) {
			auto_anim = 0;
			framenum--;
		}
		// Next frame
		if (Read_DI_once_keys(Common::KEYCODE_EQUALS)) {
			auto_anim = 0;
			framenum++;
		}
	}

	// Auto animating backward
	if (auto_anim == 1) {
		framenum--;
	}
	// Auto forwards
	else if (auto_anim) {
		framenum++;
	}

	// Catch the loop for doing -1 in unsigned decimal
	if (framenum > pxanim->frame_qty)
		framenum = pxanim->frame_qty - 2;
	if (framenum < 0) {
		if (g_repeats > 0) {
			g_repeats--;
		} else {
			if (auto_anim == 1)
				returnStatus = ANIMATION_END;
		}

		framenum = pxanim->frame_qty - 2;
	}

	// Catch the loop for going past end of the animation
	if (framenum > (pxanim->frame_qty - 2)) {
		if (g_repeats > 0) {
			g_repeats--;
		} else {
			if (auto_anim == 2)
				returnStatus = ANIMATION_END;
		}

		framenum = 0;
	}

	// Catch illegal animations
	if (framenum < 0)
		framenum = 0;

	// Draw the display list
	drawOTList();

	// Now copy the sucker to the screen
	uint32 *address = (uint32 *)surface_manager->Lock_surface(working_buffer_id);
	uint32 pitch = surface_manager->Get_pitch(working_buffer_id);
	uint32 *source = (uint32 *)pRGB;
	uint16 *zActor = (uint16 *)pZa;

	uint32 *safe_ad = address;

	for (int32 y = SCREEN_DEPTH; y; y--) {
		uint32 *ad = safe_ad;
		for (int32 x = SCREEN_WIDTH; x; x--) {
			// If the z-map for this pixel is FFFF then this pixel doesn't contain actor
			if (*zActor != 0xFFFF) {
				*ad = *source;
				*zActor = 0xFFFF;
			}

			++zActor;
			++ad;
			++source;
		}
		safe_ad += pitch / 4;
	}

	// Unlock the surface
	surface_manager->Unlock_surface(working_buffer_id);

	return returnStatus;
}

void DrawFrame(const int32 frame) {
	// These structures are needed for the drawing code to accept our light
	PSXLampList the_lights;
	PSXShadeList the_shades;
	the_lights.n = 1;
	the_lights.states[0] = 0;
	the_lights.lamps[0] = g_av_Light;
	the_shades.n = 0;

	// Open the animation file
	char bone_name[128];
	char pose_name[128];
	char mesh_name[128];
	char smesh_name[128];
	PSXrgb ambient;
	ambient.r = 128;
	ambient.g = 128;
	ambient.b = 128;

	pxanim = (PXanim *)rs_anims->Res_open(raj_name, raj_name_hash, cluster_name, cluster_name_hash);
	PXFrameEnOfAnim(framenum, pxanim);

	// Make the actors orientation matrix
	g_av_actor->rot = rot;
	g_av_actor->rot.vy = (int16)(g_av_actor->rot.vy);

	// Make the root local-world matrix
	RotMatrix_gte(&g_av_actor->rot, &g_av_actor->lw);

	// Need to use marker to get correct actor height (making crouch look correct)
	PXframe *frm = PXFrameEnOfAnim(framenum, pxanim);
	PXmarker &marker = frm->markers[ORG_POS];
	float mposx, mposy, mposz;
	PXmarker_PSX_Object::GetXYZ(&marker, &mposx, &mposy, &mposz);
	int32 dy = (int32)mposy;

	g_av_actor->lw.t[0] = 0;
	g_av_actor->lw.t[1] = dy - 112;
	g_av_actor->lw.t[2] = 0;

	// Set the true rotation & position values from the ORG marker
	g_av_actor->truePos.x = 0;
	g_av_actor->truePos.y = dy - 112;
	g_av_actor->truePos.z = 0;
	g_av_actor->trueRot = g_av_actor->rot;

	Common::sprintf_s(pose_name, "%s\\pose.rap", weapon_name);
	Common::sprintf_s(bone_name, "%s\\%s.rab", weapon_name, anim_name);
	Common::sprintf_s(mesh_name, "mesh.rap");
	Common::sprintf_s(smesh_name, "mesh_shadow.rap");

	uint32 mesh_hash = HashString(mesh_name);
	RapAPI *mesh = (RapAPI *)rs_anims->Res_open(mesh_name, mesh_hash, cluster_name, cluster_name_hash);
	uint32 smesh_hash = HashString(smesh_name);
	RapAPI *smesh = (RapAPI *)rs_anims->Res_open(smesh_name, smesh_hash, cluster_name, cluster_name_hash);
	uint32 pose_hash = HashString(pose_name);
	RapAPI *pose = (RapAPI *)rs_anims->Res_open(pose_name, pose_hash, cluster_name, cluster_name_hash);
	uint32 bone_hash = HashString(bone_name);
	RabAPI *rab = (RabAPI *)rs_anims->Res_open(bone_name, bone_hash, cluster_name, cluster_name_hash);

	ConvertRAP(pose);
	ConvertRAP(smesh);
	ConvertRAP(mesh);

	// Some error checking
	if (READ_LE_INT32((int32 *)mesh->id) != *(int32 *)const_cast<char *>(RAP_API_ID)) {
		Fatal_error("Wrong rap id value file %d api %d file:%s", mesh->id, RAP_API_ID, mesh_name);
	}
	if (FROM_LE_32(mesh->schema) != RAP_API_SCHEMA) {
		Fatal_error("Wrong rap schema value file %d rap_api %d file:%s", mesh->schema, RAP_API_SCHEMA, mesh_name);
	}
	if (READ_LE_INT32((int32 *)pose->id) != *(int32 *)const_cast<char *>(RAP_API_ID)) {
		Fatal_error("Wrong rap id value file %d api %d file:%s", pose->id, RAP_API_ID, pose_name);
	}
	if (FROM_LE_32(pose->schema) != RAP_API_SCHEMA) {
		Fatal_error("Wrong rap schema value file %d rap_api %d file:%s", pose->schema, RAP_API_SCHEMA, pose_name);
	}
	if (READ_LE_INT32((int32 *)rab->id) != *(int32 *)const_cast<char *>(RAB_API_ID)) {
		Fatal_error("Wrong rab id value file %d rab_api %d file:%s", rab->id, RAB_API_ID, bone_name);
	}
	if (FROM_LE_32(rab->schema) != RAB_API_SCHEMA) {
		Fatal_error("Wrong rab schema value file %d rab_api %d file:%s", rab->schema, RAB_API_SCHEMA, bone_name);
	}
	if (FROM_LE_16(mesh->nBones) != rab->nBones) {
		Fatal_error("mesh nBones != animation nBones %d != %d", mesh->nBones, rab->nBones);
	}

	// Pass in the linkage file and the bones file
	Bone_Frame *bone_frame = RabAPIObject::GetFrame(rab, frame);
	int32 brightness;

	int32 debug = 1;

	BoneDeformation *myBones[MAX_DEFORMABLE_BONES];

	for (int32 i = 0; i < MAX_DEFORMABLE_BONES; i++) {
		myBones[i] = nullptr;
	}

	// Shadow stuff to play with
	int32 nShadows = 0;
	SVECTORPC p_n[3];
	int32 p_d[3];

	p_n[0].vx = 0;
	p_n[0].vy = -1;
	p_n[0].vz = 0;
	p_d[0] = -118;

	MATRIXPC local2screen; // not really bothered about this...

	// Drawing finally
	DrawActor4PC(g_av_actor, g_camera, bone_frame, mesh, pose, smesh, &ambient, &the_lights, &the_shades, nShadows, p_n, p_d, debug, uvframe, myBones, &brightness,
	             &local2screen);

	uvframe++;
}

void MakeCameraView() {
	RotMatrix_gte(&_crot, &g_camera->view);

	// Include the x,y,z scalings
	g_camera->view.m[0][0] = (int16)(g_camera->view.m[0][0] * 1);
	g_camera->view.m[0][1] = (int16)(g_camera->view.m[0][1] * 1);
	g_camera->view.m[0][2] = (int16)(g_camera->view.m[0][2] * 1);
	g_camera->view.m[1][0] = (int16)(g_camera->view.m[1][0] * 1);
	g_camera->view.m[1][1] = (int16)(g_camera->view.m[1][1] * 1);
	g_camera->view.m[1][2] = (int16)(g_camera->view.m[1][2] * 1);
	g_camera->view.m[2][0] = (int16)(g_camera->view.m[2][0] * 4);
	g_camera->view.m[2][1] = (int16)(g_camera->view.m[2][1] * 4);
	g_camera->view.m[2][2] = (int16)(g_camera->view.m[2][2] * 4);
}

void ResetCamera() {
	_crot.vx = (4096 * 180) / 360;
	_crot.vy = (4096 * -30) / 360;
	_crot.vz = 0;
	g_camera->view.t[0] = 170 + av_x;
	g_camera->view.t[1] = 0 + av_y;
	g_camera->view.t[2] = 1800 + av_z;
	g_camera->focLen = 619 * 4;
	MakeCameraView();
}

void ResetActor() {
	// Set up av_actor rotation
	rot.vx = 0;
	rot.vy = 0;
	rot.vz = 0;
}

void InitLight() {
	g_av_Light->nStates = 1;       // One state
	g_av_Light->w = 0;             // Zero width
	g_av_Light->b = 0;             // Zero bounce
	g_av_Light->anu = 0;           // Don't use it
	g_av_Light->type = OMNI_LIGHT; // OMNI
	g_av_Light->ba = 0;            // Means nothing for an OMNI
	g_av_Light->bs = 0;            // Means nothing for an OMNI

	// Don't think these things are used...
	g_av_Light->states[0].ans2 = 0;
	g_av_Light->states[0].ane2 = (100 * 1) * (100 * 1);

	// No shade...
	g_av_Light->states[0].m = 128;

	// Direction doesn't matter; it's an OMNI light
	g_av_Light->states[0].vx = 4096; // Ignored for an OMNI light
	g_av_Light->states[0].vy = 0;    // Ignored for an OMNI light
	g_av_Light->states[0].vz = 0;    // Ignored for an OMNI light

	// Initial angle
	av_LightA = 0;
	av_LightDir = TRUE8;

	// Initial position
	av_LightX = 0;
	av_LightY = 0;
	av_LightZ = LIGHT_DISTANCE_FROM_ACTOR;

	// Initial colour (RED)
	av_LightR = 255;
	av_LightG = 0;
	av_LightB = 0;

	// Auto flags
	av_autoR = FALSE8;
	av_autoG = FALSE8;
	av_autoB = FALSE8;
}

void AutoCycleLight() {
	// Increase angle by 10 degrees
	av_LightA += 10;
	if (av_LightA >= 360)
		av_LightA = 0;

	// Convert to radians
	double radians = (av_LightA * M_PI) / 180.0f;

	// Now calculate z and x coordinates from this angle
	av_LightX = (int16)(sin(radians) * LIGHT_DISTANCE_FROM_ACTOR);
	av_LightZ = (int16)(cos(radians) * LIGHT_DISTANCE_FROM_ACTOR);

	// Now bouce the light height between two fixed limits
	if (av_LightDir) {
		av_LightY = (int16)(av_LightY + 10);

		if (av_LightY > LIGHT_HEIGHT_LIMIT) {
			av_LightY = LIGHT_HEIGHT_LIMIT;
			av_LightDir = FALSE8;
		}
	} else {
		av_LightY = (int16)(av_LightY - 10);

		if (av_LightY < -LIGHT_HEIGHT_LIMIT) {
			av_LightY = -LIGHT_HEIGHT_LIMIT;
			av_LightDir = TRUE8;
		}
	}

	// Red component
	if (av_autoR) {
		av_LightR += 3;

		if (av_LightR > 255) {
			av_LightR = 255;
			av_autoR = FALSE8;
		}
	} else {
		av_LightR -= 2;

		if (av_LightR < 0) {
			av_LightR = 0;
			av_autoR = TRUE8;
		}
	}

	// Green component
	if (av_autoG) {
		av_LightG += 2;

		if (av_LightG > 255) {
			av_LightG = 255;
			av_autoG = FALSE8;
		}
	} else {
		av_LightG -= 3;

		if (av_LightG < 0) {
			av_LightG = 0;
			av_autoG = TRUE8;
		}
	}

	// Blue component
	if (av_autoB) {
		av_LightB += 7;

		if (av_LightB > 255) {
			av_LightB = 255;
			av_autoB = FALSE8;
		}
	} else {
		av_LightB -= 5;

		if (av_LightB < 0) {
			av_LightB = 0;
			av_autoB = TRUE8;
		}
	}
}

void SetLight(int32 falloff) {
	// Check colours are 0-255
	if ((av_LightR > 255) || (av_LightR < 0) || (av_LightG > 255) || (av_LightG < 0) || (av_LightB > 255) || (av_LightB < 0))
		Fatal_error("ActorView light rgb %d,%d,%d out of range (0-255)", av_LightR, av_LightG, av_LightB);

	// Set colours (scale 0-255 to 0-4095)
	g_av_Light->states[0].c.r = (int16)((av_LightR * 4096) / 256);
	g_av_Light->states[0].c.g = (int16)((av_LightG * 4096) / 256);
	g_av_Light->states[0].c.b = (int16)((av_LightB * 4096) / 256);

	// Set the v field of colour to be the maximum of r,g,b
	g_av_Light->states[0].c.v = g_av_Light->states[0].c.r;         // Start at red
	if (g_av_Light->states[0].c.g > g_av_Light->states[0].c.v)     // If green bigger
		g_av_Light->states[0].c.v = g_av_Light->states[0].c.g; // Set to green
	if (g_av_Light->states[0].c.b > g_av_Light->states[0].c.v)     // If blue bigger
		g_av_Light->states[0].c.v = g_av_Light->states[0].c.b; // Set to blue

	g_av_Light->states[0].pos.vx = (int32)av_LightX;
	g_av_Light->states[0].pos.vy = (int32)av_LightY;
	g_av_Light->states[0].pos.vz = (int32)av_LightZ;

	// And add the players position
	g_av_Light->states[0].pos.vx += (int32)g_av_actor->truePos.x;
	g_av_Light->states[0].pos.vy += (int32)g_av_actor->truePos.y;
	g_av_Light->states[0].pos.vz += (int32)g_av_actor->truePos.z;

	// Falloff
	if (falloff == 0) {
		g_av_Light->afu = 0; // Don't use it
	} else {
		g_av_Light->states[0].afs2 = (falloff * falloff) / 100; // (d/10)^2     = (d*d)/100
		g_av_Light->states[0].afe2 = falloff * falloff;         // d^2          = (d*d)
		g_av_Light->afu = 1;                                    // Use it
	}
}

int32 my_sprintf(char *buf, const char *format...) {
	char lbuf[256];

	// Process the variable arguments
	va_list arglist;
	va_start(arglist, format);

	int32 slen = vsnprintf(lbuf, 256, const_cast<char *>(format), arglist);

	strncpy(buf, lbuf, slen);
	buf[slen] = '\0';

	va_end(arglist);

	return slen;
}

} // End of namespace ICB
