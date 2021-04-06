/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGS_PAL_RENDER_RAYCAST_H
#define AGS_PLUGINS_AGS_PAL_RENDER_RAYCAST_H

#include "ags/plugins/ags_pal_render/pal_render.h"

namespace AGS3 {
namespace Plugins {
namespace AGSPalRender {

#define MAP_WIDTH 64
#define MAP_HEIGHT 64

struct Sprite {
	double x;
	double y;
	int texture;
	byte alpha;
	int blendmode;
	double uDivW;
	double uDivH;
	double vMove;
	double hMove;
	int8 objectinteract;
	int view;
	int frame;
	int angle;
};

struct wallType {
	int texture[4];
	int solid[4];
	int ignorelighting[4];
	int alpha[4];
	int blendtype[4];
	int mask[4];
	byte hotspotinteract;
};


extern bool raycastOn;
extern double posX;
extern double posY; //x and y start position
extern double dirX;
extern double dirY; //initial direction vector
extern double planeX;
extern double planeY; //the 2d raycaster version of camera plane
extern double moveSpeed; //the constant value is in squares/second
extern double rotSpeed; //the constant value is in radians/second
extern unsigned char worldMap[MAP_WIDTH][MAP_HEIGHT];
extern unsigned char lightMap[MAP_WIDTH][MAP_HEIGHT];
extern int ceilingMap[MAP_WIDTH][MAP_HEIGHT];
extern int floorMap[MAP_WIDTH][MAP_HEIGHT];
extern int heightMap[MAP_WIDTH][MAP_HEIGHT];
extern unsigned char seenMap[MAP_WIDTH][MAP_HEIGHT];
extern int textureSlot;
extern int ambientlight;

#define numSprites 256
extern Sprite sprite[numSprites];


#define texWidth 64
#define texHeight 64
#define MAX_TEXTURES 512
extern unsigned char texture[][texWidth * texHeight];

extern bool heightmapOn;


extern wallType wallData[256];

//arrays used to sort the sprites
extern unsigned char **transcolorbuffer;
extern unsigned char **transalphabuffer;
extern double **transzbuffer;
extern bool *transslicedrawn;
extern int *transwallblendmode;
extern double **ZBuffer;
extern double *distTable;
extern short *interactionmap;
extern int skybox;



void MakeTextures(ScriptMethodParams &params);
void Raycast_Render(ScriptMethodParams &params);
void MoveForward(ScriptMethodParams &params);
void MoveBackward(ScriptMethodParams &params);
void RotateLeft(ScriptMethodParams &params);
void RotateRight(ScriptMethodParams &params);
void Init_Raycaster(ScriptMethodParams &params);
void QuitCleanup(ScriptMethodParams &params);
void LoadMap(ScriptMethodParams &params);
void Ray_InitSprite(ScriptMethodParams &params);
void Ray_SetPlayerPosition(ScriptMethodParams &params);
void Ray_GetPlayerX(ScriptMethodParams &params);
void Ray_GetPlayerY(ScriptMethodParams &params);
void Ray_GetPlayerAngle(ScriptMethodParams &params);
void Ray_SetPlayerAngle(ScriptMethodParams &params);

void Ray_GetWallHotspot(ScriptMethodParams &params);
void Ray_GetWallTexture(ScriptMethodParams &params);
void Ray_GetWallSolid(ScriptMethodParams &params);
void Ray_GetWallIgnoreLighting(ScriptMethodParams &params);
void Ray_GetWallAlpha(ScriptMethodParams &params);
void Ray_GetWallBlendType(ScriptMethodParams &params);

void Ray_SelectTile(ScriptMethodParams &params);

void Ray_GetHotspotAt(ScriptMethodParams &params);
void Ray_GetObjectAt(ScriptMethodParams &params);

void Ray_DrawTile(ScriptMethodParams &params);
void Ray_DrawOntoTile(ScriptMethodParams &params);
void Ray_SetNoClip(ScriptMethodParams &params);
void Ray_GetNoClip(ScriptMethodParams &params);
void Ray_SetSpriteInteractObj(ScriptMethodParams &params);
void  Ray_GetSpriteInteractObj(ScriptMethodParams &params);
void Ray_SetSpritePosition(ScriptMethodParams &params);
void Ray_SetSpriteVertOffset(ScriptMethodParams &params);
void Ray_GetSpriteVertOffset(ScriptMethodParams &params);
void Ray_GetSpriteX(ScriptMethodParams &params);
void Ray_GetSpriteY(ScriptMethodParams &params);

void Ray_SetWallHotspot(ScriptMethodParams &params);
void Ray_SetWallTextures(ScriptMethodParams &params);
void Ray_SetWallSolid(ScriptMethodParams &params);
void Ray_SetWallIgnoreLighting(ScriptMethodParams &params);
void Ray_SetWallAlpha(ScriptMethodParams &params);
void Ray_SetWallBlendType(ScriptMethodParams &params);

void Ray_GetMoveSpeed(ScriptMethodParams &params);
void Ray_SetMoveSpeed(ScriptMethodParams &params);
void Ray_GetRotSpeed(ScriptMethodParams &params);
void Ray_SetRotSpeed(ScriptMethodParams &params);
void Ray_GetWallAt(ScriptMethodParams &params);
void Ray_GetLightAt(ScriptMethodParams &params);
void Ray_SetLightAt(ScriptMethodParams &params);
void Ray_SetWallAt(ScriptMethodParams &params);
void Ray_SetPlaneY(ScriptMethodParams &params);
void Ray_GetDistanceAt(ScriptMethodParams &params);
void Ray_GetSpriteAngle(ScriptMethodParams &params);
void Ray_SetSpriteAngle(ScriptMethodParams &params);
void Ray_SetSpriteView(ScriptMethodParams &params);
void Ray_GetSpriteView(ScriptMethodParams &params);
void Ray_SetSpriteFrame(ScriptMethodParams &params);
void Ray_GetSpriteFrame(ScriptMethodParams &params);

void Ray_GetTileX_At(ScriptMethodParams &params);
void Ray_GetTileY_At(ScriptMethodParams &params);

void Ray_SetSkyBox(ScriptMethodParams &params);
void Ray_GetSkyBox(ScriptMethodParams &params);

void Ray_SetAmbientLight(ScriptMethodParams &params);
void Ray_GetAmbientLight(ScriptMethodParams &params);
void Ray_SetAmbientColor(ScriptMethodParams &params);


void Ray_GetSpriteAlpha(ScriptMethodParams &params);
void Ray_SetSpriteAlpha(ScriptMethodParams &params);
void Ray_GetSpritePic(ScriptMethodParams &params);
void Ray_SetSpritePic(ScriptMethodParams &params);

void Ray_GetSpriteScaleX(ScriptMethodParams &params);
void Ray_SetSpriteScaleX(ScriptMethodParams &params);
void Ray_GetSpriteScaleY(ScriptMethodParams &params);
void Ray_SetSpriteScaleY(ScriptMethodParams &params);

void Ray_SetSpriteBlendType(ScriptMethodParams &params);
void Ray_GetSpriteBlendType(ScriptMethodParams &params);

void Ray_SetFloorAt(ScriptMethodParams &params);
void Ray_SetCeilingAt(ScriptMethodParams &params);
void Ray_GetCeilingAt(ScriptMethodParams &params);
void Ray_GetFloorAt(ScriptMethodParams &params);
void Ray_GetLightingAt(ScriptMethodParams &params);
void Ray_SetLightingAt(ScriptMethodParams &params);
void Ray_GetAmbientWeight(ScriptMethodParams &params);

void Ray_HasSeenTile(ScriptMethodParams &params);

} // namespace AGSPalRender
} // namespace Plugins
} // namespace AGS3

#endif
