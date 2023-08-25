/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "ags/lib/allegro.h"
#include "ags/plugins/ags_pal_render/raycast.h"
#include "ags/plugins/ags_pal_render/ags_pal_render.h"

namespace AGS3 {
namespace Plugins {
namespace AGSPalRender {

#define PI         (3.1415926535f)
#define S_WIDTH 320
#define S_HEIGHT 160

// Variable Declaration
bool raycastOn;
double posX = 22.0, posY = 11.5; //x and y start position
double dirX = -1.0, dirY = 0.0; //initial direction vector
double planeX = 0.0, planeY = 0.77; //the 2d raycaster version of camera plane
double moveSpeed = (1.0 / 60.0) * 3.0; //the constant value is in squares/second
double rotSpeed = (1.0 / 60.0) * 2.0; //the constant value is in radians/second
byte worldMap[MAP_WIDTH][MAP_HEIGHT];
byte lightMap[MAP_WIDTH][MAP_HEIGHT];
int ceilingMap[MAP_WIDTH][MAP_HEIGHT];
int floorMap[MAP_WIDTH][MAP_HEIGHT];
int heightMap[MAP_WIDTH][MAP_HEIGHT];
byte seenMap[MAP_WIDTH][MAP_HEIGHT];
int textureSlot;
int ambientlight;
int ambientweight = 0;
int ambientcolor = 0;
int ambientcolorAmount = 0;

Sprite sprite[numSprites] = {};
int editorMap [S_WIDTH][S_HEIGHT] = {};

unsigned char texture[MAX_TEXTURES][texWidth * texHeight];

wallType wallData[256] = {};

//arrays used to sort the sprites
int spriteOrder[numSprites];
double spriteTransformX[numSprites];
double spriteTransformY[numSprites];

unsigned char **transcolorbuffer;
unsigned char **transalphabuffer;
double **transzbuffer;
bool *transslicedrawn;
int *transwallblendmode;
double **ZBuffer;
double *distTable;
short *interactionmap;
int skybox = 0;
bool heightmapOn;

int noclip = 0;

int selectedX;
int selectedY;
unsigned char selectedColor;

void AGSPalRender::Ray_SelectTile(ScriptMethodParams &params) {
	PARAMS3(int, x, int, y, unsigned char, color);
	if (x < 0 || x >= MAP_WIDTH) selectedX = -1;
	else if (y < 0 || y >= MAP_HEIGHT) selectedY = -1;
	else {
		selectedX = x;
		selectedY = y;
		selectedColor = color;
	}
}

void AGSPalRender::Ray_HasSeenTile(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	if (x < 0 || x >= MAP_WIDTH) params._result = -1;
	else if (y < 0 || y >= MAP_HEIGHT) params._result = -1;
	else params._result = seenMap [x][y];
}

void AGSPalRender::Ray_SetNoClip(ScriptMethodParams &params) {
	PARAMS1(int, value);
	noclip = value;
}

void AGSPalRender::Ray_GetNoClip(ScriptMethodParams &params) {
	params._result = noclip;
}

void AGSPalRender::Ray_DrawTile(ScriptMethodParams &params) {
	PARAMS2(int, spr, int, tile);
	BITMAP *img = engine->GetSpriteGraphic(spr);
	uint8 *sprarray = engine->GetRawBitmapSurface(img);
	int pitch = engine->GetBitmapPitch(img);
	for (int y = 0, yy = 0; y < MAP_HEIGHT; ++y, yy += pitch)
		for (int x = 0; x < MAP_WIDTH; ++x)
			sprarray[yy + x] = texture [tile][(texWidth * y) + x];
	engine->ReleaseBitmapSurface(img);
}

void AGSPalRender::Ray_DrawOntoTile(ScriptMethodParams &params) {
	PARAMS2(int, spr, int, tile);
	BITMAP *img = engine->GetSpriteGraphic(spr);
	uint8 *sprarray = engine->GetRawBitmapSurface(img);
	int pitch = engine->GetBitmapPitch(img);
	for (int y = 0, yy = 0; y < MAP_HEIGHT; ++y, yy += pitch)
		for (int x = 0; x < MAP_WIDTH; ++x)
			texture [tile][(texWidth * y) + x] = sprarray [yy + x];
	engine->ReleaseBitmapSurface(img);
}

void AGSPalRender::Ray_GetTileX_At(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	if (x < 0 || x >= S_WIDTH  || y < 0 || y >= S_HEIGHT) params._result = -1;
	else params._result = editorMap [x][y] >> 16;
}

void AGSPalRender::Ray_GetTileY_At(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	if (x < 0 || x >= S_WIDTH  || y < 0 || y >= S_HEIGHT) params._result = -1;
	else params._result = editorMap [x][y] & 0x0000FFFF;
}

void AGSPalRender::Ray_SetWallAt(ScriptMethodParams &params) {
	PARAMS3(int, x, int, y, int, id);
	if (x < 0 || x >= MAP_WIDTH) return;
	if (y < 0 || y >= MAP_HEIGHT) return;
	worldMap [x][y] = id;
}

void AGSPalRender::Ray_GetWallAt(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	if (x < 0 || x >= MAP_WIDTH) params._result = -1;
	else if (y < 0 || y >= MAP_HEIGHT) params._result = -1;
	else params._result = worldMap [x][y];
}

void AGSPalRender::Ray_GetAmbientWeight(ScriptMethodParams &params) {
	params._result = ambientweight;
}

void AGSPalRender::Ray_SetAmbientLight(ScriptMethodParams &params) {
	PARAMS1(int, value);
	ambientlight = MIN(255, MAX(0, value));
}

void AGSPalRender::Ray_SetAmbientColor(ScriptMethodParams &params) {
	PARAMS2(int, color, int, amount);
	ambientcolor = color;
	ambientcolorAmount = amount;
}

void AGSPalRender::Ray_GetAmbientLight(ScriptMethodParams &params) {
	params._result = ambientlight;
}
double fsqrt(double y) {
	double x, z, tempf;
	unsigned long *tfptr = ((unsigned long *)&tempf) + 1;

	tempf = y;
	*tfptr = (0xbfcdd90a - *tfptr) >> 1; /* estimate of 1/sqrt(y) */
	x =  tempf;
	z =  y * 0.5;                      /* hoist out the �/2�    */
	x = (1.5 * x) - (x * x) * (x * z); /* iteration formula     */
	x = (1.5 * x) - (x * x) * (x * z);
	x = (1.5 * x) - (x * x) * (x * z);
	x = (1.5 * x) - (x * x) * (x * z);
	x = (1.5 * x) - (x * x) * (x * z);
	return x * y;
}

void AGSPalRender::Ray_SetWallHotspot(ScriptMethodParams &params) {
	PARAMS2(int, id, char, hotsp);
	wallData[id].hotspotinteract = hotsp;
}

void AGSPalRender::Ray_SetWallTextures(ScriptMethodParams &params) {
	PARAMS5(int, id, int, n, int, s, int, w, int, e);
	wallData[id].texture[0] = n;
	wallData[id].texture[1] = s;
	wallData[id].texture[2] = w;
	wallData[id].texture[3] = e;
}

void AGSPalRender::Ray_SetWallSolid(ScriptMethodParams &params) {
	PARAMS5(int, id, int, n, int, s, int, w, int, e);
	wallData[id].solid [0] = MAX(0, MIN(n, 1));
	wallData[id].solid [1] = MAX(0, MIN(s, 1));
	wallData[id].solid [2] = MAX(0, MIN(w, 1));
	wallData[id].solid [3] = MAX(0, MIN(e, 1));
}

void AGSPalRender::Ray_SetWallIgnoreLighting(ScriptMethodParams &params) {
	PARAMS5(int, id, int, n, int, s, int, w, int, e);
	wallData[id].ignorelighting [0] = MAX(0, MIN(n, 1));
	wallData[id].ignorelighting [1] = MAX(0, MIN(s, 1));
	wallData[id].ignorelighting [2] = MAX(0, MIN(w, 1));
	wallData[id].ignorelighting [3] = MAX(0, MIN(e, 1));
}

void AGSPalRender::Ray_SetWallAlpha(ScriptMethodParams &params) {
	PARAMS5(int, id, int, n, int, s, int, w, int, e);
	wallData[id].alpha [0] = MAX(0, MIN(n, 255));
	wallData[id].alpha [1] = MAX(0, MIN(s, 255));
	wallData[id].alpha [2] = MAX(0, MIN(w, 255));
	wallData[id].alpha [3] = MAX(0, MIN(e, 255));
}

void AGSPalRender::Ray_SetWallBlendType(ScriptMethodParams &params) {
	PARAMS5(int, id, int, n, int, s, int, w, int, e);
	wallData[id].blendtype [0] = MAX(0, MIN(n, 10));
	wallData[id].blendtype [1] = MAX(0, MIN(s, 10));
	wallData[id].blendtype [2] = MAX(0, MIN(w, 10));
	wallData[id].blendtype [3] = MAX(0, MIN(e, 10));
}




void AGSPalRender::Ray_GetWallHotspot(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = wallData[id].hotspotinteract;
}

void AGSPalRender::Ray_GetWallTexture(ScriptMethodParams &params) {
	PARAMS2(int, id, int, dir);
	params._result = wallData[id].texture[dir];
}

void AGSPalRender::Ray_GetWallSolid(ScriptMethodParams &params) {
	PARAMS2(int, id, int, dir);
	params._result = wallData[id].solid [dir];
}

void AGSPalRender::Ray_GetWallIgnoreLighting(ScriptMethodParams &params) {
	PARAMS2(int, id, int, dir);
	params._result = wallData[id].ignorelighting [dir];
}

void AGSPalRender::Ray_GetWallAlpha(ScriptMethodParams &params) {
	PARAMS2(int, id, int, dir);
	params._result = wallData[id].alpha [dir];
}

void AGSPalRender::Ray_GetWallBlendType(ScriptMethodParams &params) {
	PARAMS2(int, id, int, dir);
	params._result = wallData[id].blendtype [dir];
}


void AGSPalRender::Ray_GetMoveSpeed(ScriptMethodParams &params) {
	float mSpeed = (float)moveSpeed;
	params._result = PARAM_FROM_FLOAT(mSpeed);
}


void AGSPalRender::Ray_SetMoveSpeed(ScriptMethodParams &params) {
	PARAMS1(int32, speedi);
	float speed = PARAM_TO_FLOAT(speedi);
	moveSpeed = (double)speed;
}

void AGSPalRender::Ray_GetRotSpeed(ScriptMethodParams &params) {
	float rSpeed = (float)rotSpeed;
	params._result = PARAM_FROM_FLOAT(rSpeed);
}

void AGSPalRender::Ray_SetRotSpeed(ScriptMethodParams &params) {
	PARAMS1(int32, speedi);
	float speed = PARAM_TO_FLOAT(speedi);
	rotSpeed = (double)speed;
}


void AGSPalRender::Ray_GetLightAt(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	params._result = lightMap [x][y];
}

void AGSPalRender::Ray_SetLightAt(ScriptMethodParams &params) {
	PARAMS3(int, x, int, y, int, light);
	lightMap [x][y] = light;
}

void AGSPalRender::Ray_SetPlaneY(ScriptMethodParams &params) {
	PARAMS1(int32, yi);
	float y = PARAM_TO_FLOAT(yi);
	planeY = (double)y;
}

void AGSPalRender::Ray_GetPlaneY(ScriptMethodParams &params) {
	float pY = (float)planeY;
	params._result = PARAM_FROM_FLOAT(pY);
}

void AGSPalRender::Ray_SetPlayerPosition(ScriptMethodParams &params) {
	PARAMS2(int32, xi, int32, yi);
	float x = PARAM_TO_FLOAT(xi);
	float y = PARAM_TO_FLOAT(yi);
	posX = (double)x;
	posY = (double)y;
}

void AGSPalRender::Ray_GetPlayerX(ScriptMethodParams &params) {

	float x = (float)posX;
	params._result = PARAM_FROM_FLOAT(x);
}

void AGSPalRender::Ray_GetPlayerY(ScriptMethodParams &params) {
	float y = (float)posY;
	params._result = PARAM_FROM_FLOAT(y);
}

void AGSPalRender::Ray_GetPlayerAngle(ScriptMethodParams &params) {
	double bgrad = atan2(dirY, dirX);
	int bgdeg = (int)(bgrad / PI * 180.0) + 180;
	params._result = bgdeg % 360;
}

void AGSPalRender::Ray_SetPlayerAngle(ScriptMethodParams &params) {
	PARAMS1(int, angle);
	int realangle = angle % 360;
	if (realangle < 0) realangle += 360;

	ScriptMethodParams playerAngle;
	Ray_GetPlayerAngle(playerAngle);
	int anglediff = realangle - playerAngle._result;
	double radians = 0.0174533 * anglediff;
	double oldDirX = dirX;
	dirX = dirX * cos(radians) - dirY * sin(radians);
	dirY = oldDirX * sin(radians) + dirY * cos(radians);
	double oldPlaneX = planeX;
	planeX = planeX * cos(radians) - planeY * sin(radians);
	planeY = oldPlaneX * sin(radians) + planeY * cos(radians);
}


void LoadHeightMap(int heightmapSlot) {
	int tempw = engine->GetSpriteWidth(heightmapSlot);
	int temph = engine->GetSpriteHeight(heightmapSlot);
	if (tempw != MAP_WIDTH || temph != MAP_HEIGHT) engine->AbortGame("LoadHeightMap: Map sizes are mismatched!");
	BITMAP *heightmapBm = engine->GetSpriteGraphic(heightmapSlot);
	if (!heightmapBm) engine->AbortGame("LoadHeightMap: Cannot load sprite into memory.");
	uint8 *hmArray = engine->GetRawBitmapSurface(heightmapBm);
	int pitch = engine->GetBitmapPitch(heightmapBm);

	for (int i = 0; i < tempw; i++) {
		for (int j = 0; j < temph; j++) {
			heightMap[i][j] = hmArray[i * pitch + j];
		}
	}
	engine->ReleaseBitmapSurface(heightmapBm);
	heightmapOn = true;
}

void AGSPalRender::LoadMap(ScriptMethodParams &params) {
	PARAMS4(int, worldmapSlot, int, lightmapSlot, int, ceilingmapSlot, int, floormapSlot);
	int tempw = engine->GetSpriteWidth(worldmapSlot);
	int temph = engine->GetSpriteHeight(worldmapSlot);
	BITMAP *worldmapBm = nullptr;
	BITMAP *lightmapBm = nullptr;
	BITMAP *floormapBm = nullptr;
	BITMAP *ceilingmapBm = nullptr;
	uint8 *wmArray = nullptr;
	uint8 *lmArray = nullptr;
	uint8 *fmArray = nullptr;
	uint8 *cmArray = nullptr;
	int wmPitch = 0;
	int lmPitch = 0;
	int fmPitch = 0;
	int cmPitch = 0;
	worldmapBm = engine->GetSpriteGraphic(worldmapSlot);
	if (!worldmapBm) engine->AbortGame("LoadMap: Couldn't load worldmap sprite into memory.");
	wmArray = engine->GetRawBitmapSurface(worldmapBm);
	wmPitch = engine->GetBitmapPitch(worldmapBm);
	if (engine->GetSpriteWidth(lightmapSlot) != tempw || engine->GetSpriteHeight(lightmapSlot) != temph) engine->AbortGame("LoadMap: Lightmap has different dimensions to worldmap.");
	else {
		lightmapBm = engine->GetSpriteGraphic(lightmapSlot);
		if (!lightmapBm) engine->AbortGame("LoadMap: Couldn't load lightmap sprite into memory.");
		lmArray = engine->GetRawBitmapSurface(lightmapBm);
		lmPitch = engine->GetBitmapPitch(lightmapBm);
	}
	if (engine->GetSpriteWidth(ceilingmapSlot) != tempw || engine->GetSpriteHeight(ceilingmapSlot) != temph) engine->AbortGame("LoadMap: Ceilingmap has different dimensions to worldmap.");
	else {
		ceilingmapBm = engine->GetSpriteGraphic(ceilingmapSlot);
		if (!ceilingmapBm) engine->AbortGame("LoadMap: Couldn't load ceilingmap sprite into memory.");
		cmArray = engine->GetRawBitmapSurface(ceilingmapBm);
		cmPitch = engine->GetBitmapPitch(ceilingmapBm);
	}
	if (engine->GetSpriteWidth(floormapSlot) != tempw || engine->GetSpriteHeight(floormapSlot) != temph) engine->AbortGame("LoadMap: Floormap has different dimensions to worldmap.");
	else {
		floormapBm = engine->GetSpriteGraphic(floormapSlot);
		if (!floormapBm) engine->AbortGame("LoadMap: Couldn't load floormap sprite into memory.");
		fmArray = engine->GetRawBitmapSurface(floormapBm);
		fmPitch = engine->GetBitmapPitch(floormapBm);
	}
	for (int i = 0; i < tempw; i++) {
		for (int j = 0; j < temph; j++) {
			worldMap[i][j] = wmArray[i * wmPitch + j];
			lightMap[i][j] = lmArray[i * lmPitch + j];
			floorMap[i][j] = fmArray[i * fmPitch + j];
			ceilingMap[i][j] = cmArray[i * cmPitch + j];
			heightMap[i][j] = 0;
			seenMap[i][j] = 0;
		}
	}
	engine->ReleaseBitmapSurface(worldmapBm);
	engine->ReleaseBitmapSurface(lightmapBm);
	engine->ReleaseBitmapSurface(ceilingmapBm);
	engine->ReleaseBitmapSurface(floormapBm);
	//LoadHeightMap (31); //debug only
}

void AGSPalRender::Ray_GetSpriteScaleX(ScriptMethodParams &params) {
	PARAMS1(int, id);
	float scale = (float)sprite[id].uDivW;
	params._result = PARAM_FROM_FLOAT(scale);
}

void AGSPalRender::Ray_SetSpriteScaleX(ScriptMethodParams &params) {
	PARAMS2(int, id, int32, scalei);
	float scale = PARAM_TO_FLOAT(scalei);
	sprite[id].uDivW = scale;
}

void AGSPalRender::Ray_GetSpriteScaleY(ScriptMethodParams &params) {
	PARAMS1(int, id);
	float scale = (float)sprite[id].uDivH;
	params._result = PARAM_FROM_FLOAT(scale);
}

void AGSPalRender::Ray_SetSpriteScaleY(ScriptMethodParams &params) {
	PARAMS2(int, id, int32, scalei);
	float scale = PARAM_TO_FLOAT(scalei);
	sprite[id].uDivH = scale;
}

void AGSPalRender::Ray_GetSpriteAlpha(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = sprite[id].alpha;
}

void AGSPalRender::Ray_SetSpriteAlpha(ScriptMethodParams &params) {
	PARAMS2(int, id, int, alpha);
	sprite[id].alpha = alpha;
}

void AGSPalRender::Ray_GetSpritePic(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = sprite[id].texture;
}

void AGSPalRender::Ray_SetSpritePic(ScriptMethodParams &params) {
	PARAMS2(int, id, int, slot);
	sprite[id].texture = slot;
}


void AGSPalRender::Ray_GetSpriteAngle(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = sprite[id].angle;
}

void AGSPalRender::Ray_SetSpriteAngle(ScriptMethodParams &params) {
	PARAMS2(int, id, int, angle);
	sprite[id].angle = angle % 360;
}

void AGSPalRender::Ray_GetSpriteInteractObj(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = sprite[id].objectinteract;
}

void AGSPalRender::Ray_SetSpriteView(ScriptMethodParams &params) {
	PARAMS2(int, id, int, view);
	sprite[id].view = view;
}

void AGSPalRender::Ray_SetSpriteBlendType(ScriptMethodParams &params) {
	PARAMS2(int, id, int, type);
	sprite[id].blendmode = type;
}

void AGSPalRender::Ray_GetSpriteBlendType(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = sprite[id].blendmode;
}


void AGSPalRender::Ray_GetSpriteView(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = sprite[id].view;
}

void AGSPalRender::Ray_SetSpriteFrame(ScriptMethodParams &params) {
	PARAMS2(int, id, int, frame);
	sprite[id].frame = frame;
}

void AGSPalRender::Ray_GetSpriteFrame(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = sprite[id].frame;
}

void AGSPalRender::Ray_SetSpriteInteractObj(ScriptMethodParams &params) {
	PARAMS2(int, id, int, obj);
	sprite[id].objectinteract = obj;
}

void AGSPalRender::Ray_SetSpritePosition(ScriptMethodParams &params) {
	PARAMS3(int, id, int32, xi, int32, yi);
	float x = PARAM_TO_FLOAT(xi);
	float y = PARAM_TO_FLOAT(yi);
	sprite[id].x = x;
	sprite[id].y = y;
}

void AGSPalRender::Ray_SetSpriteVertOffset(ScriptMethodParams &params) {
	PARAMS2(int, id, int32, vMovei);
	float vMove = PARAM_TO_FLOAT(vMovei);
	sprite[id].vMove = vMove;
}


void AGSPalRender::Ray_GetSpriteVertOffset(ScriptMethodParams &params) {
	PARAMS1(int, id);
	float x = (float)sprite[id].vMove;
	params._result = PARAM_FROM_FLOAT(x);
}

void AGSPalRender::Ray_GetSpriteX(ScriptMethodParams &params) {
	PARAMS1(int, id);
	float x = (float)sprite[id].x;
	params._result = PARAM_FROM_FLOAT(x);
}

void AGSPalRender::Ray_GetSpriteY(ScriptMethodParams &params) {
	PARAMS1(int, id);
	float y = (float)sprite[id].y;
	params._result = PARAM_FROM_FLOAT(y);
}

void AGSPalRender::Ray_InitSprite(ScriptMethodParams &params) {
	PARAMS9(int, id, int32, xi, int32, yi, int, slot, unsigned char, alpha, int, blendmode, int32, scale_xi, int32, scale_yi, int32, vMovei);
	float x = PARAM_TO_FLOAT(xi);
	float y = PARAM_TO_FLOAT(yi);
	float scale_x = PARAM_TO_FLOAT(scale_xi);
	float scale_y = PARAM_TO_FLOAT(scale_yi);
	float vMove = PARAM_TO_FLOAT(vMovei);

	sprite[id].x = x;
	sprite[id].y = y;
	sprite[id].texture = slot;
	sprite[id].alpha = alpha;
	sprite[id].blendmode = blendmode;
	sprite[id].uDivW = scale_x;
	sprite[id].uDivH = scale_y;
	sprite[id].vMove = vMove;
}

//function used to sort the sprites
void combSort(int *order, double *dist, int amount);

void AGSPalRender::MakeTextures(ScriptMethodParams &params) {
	PARAMS1(int, slot);
	textureSlot = slot;
	int sourceWidth = engine->GetSpriteWidth(slot);
	int sourceHeight = engine->GetSpriteHeight(slot);
	int max = (sourceWidth / texWidth) * (sourceHeight / texHeight);
	if (max > MAX_TEXTURES) engine->AbortGame("MakeTextures: Source file has too many tiles to load.");
	BITMAP *texspr = engine->GetSpriteGraphic(slot);
	uint8 *texbuffer = engine->GetRawBitmapSurface(texspr);
	int texPitch = engine->GetBitmapPitch(texspr);
	int numTilesX = sourceWidth / texWidth;
	int numTilesY = sourceHeight / texHeight;
	//int totaltiles = numTilesX * numTilesY;
	for (int numX = 0; numX < numTilesX; ++numX) {
		for (int numY = 0; numY < numTilesY; ++numY) {
			for (int x = 0; x < texWidth; ++x)
				for (int y = 0; y < texHeight; ++y) {
					texture[(numY * numTilesX) + numX][(texWidth * y) + x] = texbuffer [(y + (texHeight * numY)) * texPitch + x + (texWidth * numX)];
				}
		}
	}
	engine->ReleaseBitmapSurface(texspr);

	for (int i = 0; i < 11; i++) {
		for (int j = 0; j < 4; j++) {
			wallData[i].texture[j] = i;
			if (i == 10) wallData[i].texture[j] = 11;
			if (i > 0) wallData[i].solid[j] = 1;
			else wallData[i].solid[j] = 0;
			if (i != 10) wallData[i].alpha[j] = 255;
			else {
				wallData[i].alpha[j] = 128;
				wallData[i].blendtype[j] = 0;
				wallData[i].solid[j] = 0;
			}
		}
	}
	wallData[1].texture[0] = 1;
	wallData[1].texture[1] = 2;
	wallData[1].texture[2] = 3;
	wallData[1].texture[3] = 4;
	wallData[1].solid[0] = 0;



}

//double ZBuffer[screenWidth][screenHeight];

void AGSPalRender::Ray_SetFloorAt(ScriptMethodParams &params) {
	PARAMS3(int, x, int, y, int, tex);
	if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT || tex > 511) return;
	else floorMap[x][y] = tex;
}

void AGSPalRender::Ray_SetCeilingAt(ScriptMethodParams &params) {
	PARAMS3(int, x, int, y, int, tex);
	if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT || tex > 511) return;
	else ceilingMap[x][y] = tex;
}

void AGSPalRender::Ray_GetCeilingAt(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) params._result = -1;
	else params._result = ceilingMap [x][y];
}


void AGSPalRender::Ray_GetFloorAt(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) params._result = -1;
	else params._result = floorMap [x][y];
}


void AGSPalRender::Ray_GetLightingAt(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) params._result = -1;
	else {
		int lighting = 0;
		if (ceilingMap[x][y] == 0) {
			lighting = ambientlight;
			if (ambientlight < lightMap [x][y]) lighting = lightMap[x][y];
		}
		params._result = lighting;
	}
}

void AGSPalRender::Ray_SetLightingAt(ScriptMethodParams &params) {
	PARAMS3(int, x, int, y, unsigned char, lighting);
	if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) return;
	else {
		lightMap [x][y] = lighting;
	}
}

void AGSPalRender::Ray_SetSkyBox(ScriptMethodParams &params) {
	PARAMS1(int, slot);
	BITMAP *test = engine->GetSpriteGraphic(slot);
	if (test) {
		skybox = slot;
	} else engine->AbortGame("Ray_SetSkybox: No such sprite!");
}

void AGSPalRender::Ray_GetSkyBox(ScriptMethodParams &params) {
	//PARAMS1(int, slot);
	params._result = skybox;
}

void AGSPalRender::Ray_GetHotspotAt(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	if (!interactionmap) params._result = -1;
	else if (x > S_WIDTH || x < 0 || y > S_HEIGHT || y < 0) params._result = -1;
	else params._result = interactionmap [x * S_WIDTH + y] & 0x00FF;
}

void AGSPalRender::Ray_GetObjectAt(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	if (!interactionmap) params._result = -1;
	else if (x > S_WIDTH || x < 0 || y > S_HEIGHT || y < 0) params._result = -1;
	else params._result = interactionmap [x * S_WIDTH + y] >> 8;
}

void AGSPalRender::Ray_GetDistanceAt(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	float falsereturn = -1.0f;
	if (!ZBuffer) {
		params._result = PARAM_FROM_FLOAT(falsereturn);
	} else if (x > S_WIDTH || x < 0 || y > S_HEIGHT || y < 0) {
		params._result = PARAM_FROM_FLOAT(falsereturn);
	} else {

		float zbuf = (float)ZBuffer[x][y];
		params._result = PARAM_FROM_FLOAT(zbuf);
	}
}

void AGSPalRender::Init_Raycaster(ScriptMethodParams &) {
	if (ZBuffer)
		return;
	//if (!worldMap) return;
	transcolorbuffer = new unsigned char *[S_WIDTH];
	transalphabuffer = new unsigned char *[S_WIDTH];
	transslicedrawn = new bool[S_WIDTH]();
	transzbuffer = new double*[S_WIDTH];
	transwallblendmode = new int [MAP_WIDTH]();
	ZBuffer = new double*[S_WIDTH];
	distTable = new double[S_HEIGHT + (S_HEIGHT >> 1)];
	interactionmap = new short[S_WIDTH * S_HEIGHT]();
	for (int y = 0; y < S_HEIGHT + (S_HEIGHT >> 1); y++) {
		distTable [y] = S_HEIGHT / (2.0 * y - S_HEIGHT);
	}
	for (int x = 0; x < S_WIDTH; x++) {
		transcolorbuffer[x] = new unsigned char [S_HEIGHT * (MAP_WIDTH)]();
		transalphabuffer[x] = new unsigned char [S_HEIGHT * (MAP_WIDTH)]();
		transzbuffer[x] = new double [S_HEIGHT * (MAP_WIDTH)]();
		ZBuffer[x] = new double [S_HEIGHT]();
		transslicedrawn [x] = false;
	}
}

bool rendering;
void AGSPalRender::Raycast_Render(ScriptMethodParams &params) {
	PARAMS1(int, slot);
	ambientweight = 0;
	raycastOn = true;
	double playerrad = atan2(dirY, dirX) + (2.0 * PI);
	rendering = true;
	int32 w = S_WIDTH, h = S_HEIGHT;
	BITMAP *screen = engine->GetSpriteGraphic(slot);
	if (!screen) engine->AbortGame("Raycast_Render: No valid sprite to draw on.");
	engine->GetBitmapDimensions(screen, &w, &h, nullptr);
	BITMAP *sbBm = engine->GetSpriteGraphic(skybox);
	if (!sbBm) engine->AbortGame("Raycast_Render: No valid skybox sprite.");
	if (skybox > 0) {
		//int bgdeg = (int)((playerrad / PI) * 180.0) + 180;
		int xoffset = (int)(playerrad * 320.0);
		BITMAP *virtsc = engine->GetVirtualScreen();
		engine->SetVirtualScreen(screen);
		xoffset = abs(xoffset % w);
		if (xoffset > 0) {
			engine->BlitBitmap(xoffset - 320, 1, sbBm, false);
		}
		engine->BlitBitmap(xoffset, 1, sbBm, false);
		engine->SetVirtualScreen(virtsc);
	}
	int transwallcount = 0;
	uint8 *buffer = engine->GetRawBitmapSurface(screen);
	int bufferPitch = engine->GetBitmapPitch(screen);
	for (int x = 0; x < w; x++) {
		transslicedrawn [x] = false;
		for (int y = 0; y < h; y++) {
			ZBuffer[x][y] = 0;
		}
	}
	//int multiplier = mapWidth;
	memset(interactionmap, 0, sizeof(short) * (S_HEIGHT * S_WIDTH));
	//start the main loop
	for (int x = 0; x < w; x++) {
		transwallcount = 0;
		//calculate ray position and direction
		double cameraX = 2 * x / double(w) - 1; //x-coordinate in camera space
		double rayPosX = posX;
		double rayPosY = posY;
		double rayDirX = dirX + planeX * cameraX;
		double rayDirY = dirY + planeY * cameraX;

		//which box of the map we're in
		int mapX = int(rayPosX);
		int mapY = int(rayPosY);

		//length of ray from current position to next x or y-side
		double sideDistX;
		double sideDistY;

		//length of ray from one x or y-side to next x or y-side
		double deltaDistX = fsqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
		double deltaDistY = fsqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
		double perpWallDist = 0.0;

		//what direction to step in x or y-direction (either +1 or -1)
		int stepX;
		int stepY;
		int prevmapX = 0;
		int prevmapY = 0;
		int hit = 0; //was there a wall hit?
		int side = 0; //was a NS or a EW wall hit?

		//calculate step and initial sideDist
		if (rayDirX < 0) {
			stepX = -1;
			sideDistX = (rayPosX - mapX) * deltaDistX;
		} else {
			stepX = 1;
			sideDistX = (mapX + 1.0 - rayPosX) * deltaDistX;
		}
		if (rayDirY < 0) {
			stepY = -1;
			sideDistY = (rayPosY - mapY) * deltaDistY;
		} else {
			stepY = 1;
			sideDistY = (mapY + 1.0 - rayPosY) * deltaDistY;
		}
		// Perform DDA
		bool deeper = true;
		bool opposite = true;
		bool oppositedrawn = false;
		double wallX = 0; // Where exactly the wall was hit
		int drawStart;
		int drawEnd = 0;
		while (hit == 0 && deeper == true) {
			if (opposite) {
				rayDirX = -rayDirX;
				rayDirY = -rayDirY;
				stepX = -stepX;
				stepY = -stepY;
				if (sideDistX < sideDistY) side = 0;
				else side = 1;
			} else if (sideDistX < sideDistY) { // jump to next map square, OR in x-direction, OR in y-direction
				sideDistX += deltaDistX;
				mapX += stepX;
				mapX = abs(mapX) % MAP_HEIGHT;
				side = 0;
				if (oppositedrawn && worldMap[mapX][mapY] > 8) {
					opposite = true;
					oppositedrawn = false;
					rayDirX = -rayDirX;
					rayDirY = -rayDirY;
					stepX = -stepX;
					stepY = -stepY;
					if (sideDistX < sideDistY) side = 0;
					else side = 1;
				} else {
					oppositedrawn = false;
					opposite = false;
				}
			} else {
				sideDistY += deltaDistY;
				mapY += stepY;
				mapY = abs(mapY) % MAP_HEIGHT;
				side = 1;
				if (oppositedrawn && worldMap[mapX][mapY] > 8) {
					opposite = true;
					oppositedrawn = false;
					rayDirX = -rayDirX;
					rayDirY = -rayDirY;
					stepX = -stepX;
					stepY = -stepY;
					if (sideDistX < sideDistY) side = 0;
					else side = 1;
				} else {
					oppositedrawn = false;
					opposite = false;
				}
			}
			int texside = 0;
			if (rayDirX > 0 && side == 0) texside = 0;
			if (rayDirX < 0 && side == 0) texside = 1;
			if (rayDirY > 0 && side == 1) texside = 2;
			if (rayDirY < 0 && side == 1) texside = 3;

			//set this tile as seen.
			seenMap[mapX][mapY] = 1;
			//Check if ray has hit a wall
			if (wallData[worldMap[mapX][mapY]].texture[texside]) {
				bool ambientpixels = false;
				hit = 1; //Set this to true so that by default, it's impossible to hang the engine.
				deeper = false; //Set this to false so that we don't go deeper than we need to.

				//Calculate distance of perpendicular ray (oblique distance will give fisheye effect!)
				if (side == 0) perpWallDist = fabs((mapX - rayPosX + (1 - stepX) / 2) / rayDirX);
				else       perpWallDist = fabs((mapY - rayPosY + (1 - stepY) / 2) / rayDirY);
				//Calculate height of line to draw on screen
				int lineHeight = abs(int(h / perpWallDist));

				//calculate lowest and highest pixel to fill in current stripe
				drawStart = -lineHeight / 2 + h / 2;
				if (drawStart < 0) drawStart = 0;
				drawEnd = lineHeight / 2 + h / 2;
				if (drawEnd >= h) drawEnd = h;
				//texturing calculations
				int texNum = wallData[worldMap[mapX][mapY]].texture[texside] - 1; //1 subtracted from it so that texture 0 can be used!
				if (!opposite) {
					//calculate value of wallX
					if (side == 1) wallX = rayPosX + ((mapY - rayPosY + (1 - stepY) / 2) / rayDirY) * rayDirX;
					else       wallX = rayPosY + ((mapX - rayPosX + (1 - stepX) / 2) / rayDirX) * rayDirY;
				} else {
					if (side == 1) wallX = rayPosX + ((mapY - rayPosY + (1 - stepY) / 2) / -rayDirY) * -rayDirX;
					else       wallX = rayPosY + ((mapX - rayPosX + (1 - stepX) / 2) / -rayDirX) * -rayDirY;
				}
				wallX -= floor((wallX));

				//x coordinate on the texture
				int wall_light = 255;
				int texX = int(wallX * double(texWidth));
				if (side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
				if (side == 1 && rayDirY < 0) texX = texWidth - texX - 1;
				bool do_ambient = false;
				if (!opposite) {
					if (rayDirX > 0 && side == 0) {
						wall_light = lightMap [(int)mapX - 1 % MAP_WIDTH][(int)mapY] << 5;
						if (ceilingMap [(int)mapX - 1 % MAP_WIDTH][(int)mapY] <= 1) do_ambient = true;
						else if (texture[ceilingMap [(int)mapX - 1 % MAP_WIDTH][(int)mapY] - 1][(texWidth * (63 - texX)) + 63] == 0) do_ambient = true;
					}
					if (rayDirX < 0 && side == 0) {
						wall_light = lightMap [(int)mapX + 1 % MAP_WIDTH][(int)mapY] << 5;
						if (ceilingMap [(int)mapX + 1 % MAP_WIDTH][(int)mapY] <= 1) do_ambient = true;
						else if (texture[ceilingMap [(int)mapX + 1 % MAP_WIDTH][(int)mapY] - 1][(texWidth * texX) + 0] == 0) do_ambient = true;

					}
					if (rayDirY > 0 && side == 1) {
						wall_light = lightMap [(int)mapX][(int)mapY - 1 % MAP_HEIGHT] << 5;
						if (ceilingMap [(int)mapX][(int)mapY - 1 % MAP_HEIGHT] <= 1) do_ambient = true;
						else if (texture[ceilingMap [(int)mapX][(int)mapY - 1 % MAP_HEIGHT] - 1][(texWidth * 63) + texX] == 0) do_ambient = true;
					}
					if (rayDirY < 0 && side == 1) {
						wall_light = lightMap [(int)mapX][(int)mapY + 1 % MAP_HEIGHT] << 5;
						if (ceilingMap [(int)mapX][(int)mapY + 1 % MAP_HEIGHT] <= 1) do_ambient = true;
						else if (texture[ceilingMap [(int)mapX][(int)mapY + 1 % MAP_HEIGHT] - 1][(texWidth * 0) + 63 - texX] == 0) do_ambient = true;
					}
				} else if (opposite) {
					wall_light = lightMap [(int)mapX][(int)mapY] << 5;
					if (ceilingMap [(int)mapX][(int)mapY] <= 1) do_ambient = true;
					if (rayDirX > 0 && side == 0) {
						if (texture[ceilingMap [(int)mapX][(int)mapY] - 1][(texWidth * (63 - texX)) + 63] == 0) do_ambient = true;
					}
					if (rayDirX < 0 && side == 0) {
						if (texture[ceilingMap [(int)mapX][(int)mapY] - 1][(texWidth * texX) + 63] == 0) do_ambient = true;
					}
					if (rayDirY > 0 && side == 1) {
						if (texture[ceilingMap [(int)mapX][(int)mapY] - 1][(texWidth * 0) + (63 - texX)] == 0) do_ambient = true;
					}
					if (rayDirY < 0 && side == 1) {
						if (texture[ceilingMap [(int)mapX][(int)mapY] - 1][(texWidth * 63) + texX] == 0) do_ambient = true;
					}
				}
				if (do_ambient) {
					ambientpixels = true;
					wall_light = MAX(wall_light, ambientlight);
				}
				wall_light = MIN(255, MAX(0, wall_light));
				bool alphastripe = false;
				for (int y = drawStart; y < drawEnd; y++) {
					if (ZBuffer[x][y] > perpWallDist || ZBuffer[x][y] == 0) { //We can draw.
						int d = y * 256 - h * 128 + lineHeight * 128; //256 and 128 factors to avoid floats
						int texY = ((d * texHeight) / lineHeight) / 256;
						int color = texture[texNum][texWidth * texY + texX];
						if (color > 0) {
							if (ambientpixels && ambientcolor) color = Mix::MixColorMultiply(ambientcolor, color, ambientcolorAmount, 1);
							if (!wallData[worldMap[mapX][mapY]].ignorelighting[texside] && wall_light < 255) color = Mix::MixColorLightLevel(color, wall_light);
							if (wallData[worldMap[mapX][mapY]].alpha[texside] == 255 && wallData[worldMap[mapX][mapY]].mask[texside] == 0) {
								buffer[y * bufferPitch + x] = color;
								if (ambientpixels) ambientweight++;
								//SET THE ZBUFFER FOR THE SPRITE CASTING
								ZBuffer[x][y] = perpWallDist; //perpendicular distance is used
								interactionmap [x * S_WIDTH + y] = wallData[worldMap[mapX][mapY]].hotspotinteract;
								editorMap [x][y] = ((short)mapX) << 16 | ((short)mapY);
							} else {
								if (transslicedrawn[x] == false) {
									memset(transcolorbuffer[x], 0, sizeof(unsigned char) * (S_HEIGHT * MAP_WIDTH));
									memset(transalphabuffer[x], 0, sizeof(unsigned char) * (S_HEIGHT * MAP_WIDTH));
									//memset (transzbuffer[x],0,sizeof(double)*(sHeight*mapWidth));
									transslicedrawn[x] = true;
								}
								transwallblendmode[transwallcount] = wallData[worldMap[mapX][mapY]].blendtype[texside];
								int transwalloffset = transwallcount * h;
								transcolorbuffer[x][transwalloffset + y] = color;
								if (ambientpixels) ambientweight++;
								if (wallData[worldMap[mapX][mapY]].mask[texside] == 0) transalphabuffer[x][transwalloffset + y] = wallData[worldMap[mapX][mapY]].alpha[texside];
								else {
									transalphabuffer[x][transwalloffset + y] = (wallData[worldMap[mapX][mapY]].alpha[texside] + texture[wallData[worldMap[mapX][mapY]].mask[texside]][texWidth * texY + texX]) >> 1;
								}
								transzbuffer[x][transwalloffset + y] = perpWallDist;
								hit = 0;
								deeper = true;
								alphastripe = true;
							}
						} else {
							//We found transparency, we have to draw deeper.
							deeper = true;
							hit = 0;
						}
						if ((int)mapX == selectedX && (int)mapY == selectedY) {
							if (texX == 0 || texX == 63 || texY == 0 || texY == 63) {
								buffer[y * bufferPitch + x] = selectedColor;
								ZBuffer [x][y] = perpWallDist;
							}
						}

					}
				}
				if (alphastripe) {
					if (transwallcount < MAP_WIDTH) {
						transwallcount++;
					}
					alphastripe = false;
				}
				if (opposite) {
					if (mapX == 0 || mapX == MAP_WIDTH || mapY == 0 || mapY == MAP_HEIGHT) {
						deeper = false;
						hit = 0;
					}
					oppositedrawn = true;
					if (deeper) opposite = false;
					rayDirX = -rayDirX;
					rayDirY = -rayDirY;
					stepX = -stepX;
					stepY = -stepY;
				} else if (!opposite && deeper) {
					opposite = true;
					prevmapX = mapX;
					prevmapY = mapY;
				}
				//End of wall drawing functions.
			} else if (opposite) {
				opposite = false;
				//oppositedrawn = false;
				rayDirX = -rayDirX;
				rayDirY = -rayDirY;
				stepX = -stepX;
				stepY = -stepY;
			} else if (!opposite && deeper) {
				opposite = true;
				prevmapX = mapX;
				prevmapY = mapY;
			}
			//End of loop.
		}

		// Unused variables
		(void)prevmapX;
		(void)prevmapY;

		//FLOOR CASTING

		double floorXWall, floorYWall; //x, y position of the floor texel at the bottom of the wall
		//4 different wall directions possible
		if (side == 0 && rayDirX > 0) {
			floorXWall = mapX;
			floorYWall = mapY + wallX;
			if (opposite) floorXWall = floorXWall + 1.0;
		} else if (side == 0 && rayDirX < 0) {
			floorXWall = mapX + 1.0;
			floorYWall = mapY + wallX;
			if (opposite) floorXWall = floorXWall - 1.0;
		} else if (side == 1 && rayDirY > 0) {
			floorXWall = mapX + wallX;
			floorYWall = mapY;
			if (opposite) floorYWall = floorYWall + 1.0;
		} else {
			floorXWall = mapX + wallX;
			floorYWall = mapY + 1.0;
			if (opposite) floorYWall = floorYWall - 1.0;
		}

		double distWall, distPlayer, currentDist;

		distWall = perpWallDist;
		distPlayer = 0.0;
		if (drawEnd < 0)
			drawEnd = h - 1; //becomes < 0 when the integer overflows
		//draw the floor from drawEnd to the bottom of the screen
		int drawdist = h;
		int expandeddraw = h >> 1;
		for (int y = drawEnd; y < drawdist + expandeddraw; y++) {
			//currentDist = h / (2.0 * y - h); //you could make a small lookup table for this instead
			currentDist = distTable[y];
			if (y > h - 1) {
				//if (!heightMap) break;
				double weight = (currentDist - distPlayer) / (distWall - distPlayer);

				double currentFloorX = weight * floorXWall + (1.0 - weight) * posX;
				double currentFloorY = weight * floorYWall + (1.0 - weight) * posY;

				int floorTexX, floorTexY;
				int cmapX = (int)currentFloorX;
				if (cmapX > MAP_WIDTH - 1) cmapX = MAP_WIDTH - 1;
				if (cmapX < 0) cmapX = 0;
				int cmapY = (int)currentFloorY;
				if (cmapY > MAP_HEIGHT - 1) cmapY = MAP_HEIGHT - 1;
				if (cmapY < 0) cmapY = 0;
				if (heightMap[cmapX][cmapY] - 1 < 1) continue;
				int lighting = lightMap [cmapX][cmapY] << 5;
				lighting = MIN(255, MAX(0, lighting));
				floorTexX = int(currentFloorX * texWidth) % texWidth;
				floorTexY = int(currentFloorY * texHeight) % texHeight;
				int floorcolor = 0;
				int ceilingcolor = 0;
				int texpos = texWidth * floorTexY + floorTexX;
				if (ceilingMap[cmapX][cmapY] - 1 > 0) {
					ceilingcolor = texture[ceilingMap[cmapX][cmapY] - 1][texWidth * floorTexY + floorTexX];
				}
				if (floorMap[cmapX][cmapY] - 1 > 0) {
					floorcolor = texture[floorMap[cmapX][cmapY] - 1][texpos];
				} else continue;
				if (ceilingcolor == 0) {
					lighting = MAX(lighting, ambientlight);
					ambientweight ++;
				}
				if (lighting < 255) {
					if (floorcolor) floorcolor = Mix::MixColorLightLevel(floorcolor, lighting);
				}

				if (/*heightMap &&*/ floorcolor > 0) {
					if (heightMap[cmapX][cmapY] - 1 > 0) {
						int raisedfloorstart = y - (int)(texture[heightMap[cmapX][cmapY] - 1][texpos] / currentDist);
						if (raisedfloorstart > h - 1) continue;
						if (raisedfloorstart < 0) raisedfloorstart = 0;
						for (int ny = raisedfloorstart; ny < y; ny++) {
							if (ny < h && (ZBuffer[x][ny] > currentDist || ZBuffer[x][ny] == 0)) {
								ZBuffer[x][ny] = currentDist; //perpendicular distance is used
								buffer[ny * bufferPitch + x] = floorcolor;
								interactionmap [x * S_WIDTH + ny] = 0;
								editorMap [x][ny] = ((short)mapX) << 16 | ((short)mapY);
							}
						}
						if (raisedfloorstart < y && y == h - 1 && y == h + expandeddraw) expandeddraw ++;
					}
				}
			} else {
				double weight = (currentDist - distPlayer) / (distWall - distPlayer);

				double currentFloorX = weight * floorXWall + (1.0 - weight) * posX;
				double currentFloorY = weight * floorYWall + (1.0 - weight) * posY;

				int floorTexX, floorTexY;
				int cmapX = (int)currentFloorX % MAP_WIDTH;
				if (cmapX < 0) cmapX = 0;
				int cmapY = (int)currentFloorY % MAP_HEIGHT;
				if (cmapY < 0) cmapY = 0;
				int lighting = lightMap [cmapX][cmapY] << 5;
				lighting = MIN(255, MAX(0, lighting));
				floorTexX = int(currentFloorX * texWidth) % texWidth;
				floorTexY = int(currentFloorY * texHeight) % texHeight;
				int floorcolor = 0;
				int ceilingcolor = 0;
				if (floorMap[cmapX][cmapY] - 1 > 0) {
					floorcolor = texture[floorMap[cmapX][cmapY] - 1][texWidth * floorTexY + floorTexX];
				}
				if (ceilingMap[cmapX][cmapY] - 1 > 0) {
					ceilingcolor = texture[ceilingMap[cmapX][cmapY] - 1][texWidth * floorTexY + floorTexX];
				}
				if (ceilingcolor == 0) {
					lighting = MAX(lighting, ambientlight);
					ambientweight++;
				}
				if (lighting < 255) {
					if (floorcolor) floorcolor = Mix::MixColorLightLevel(floorcolor, lighting);
					if (ceilingcolor) ceilingcolor = Mix::MixColorLightLevel(ceilingcolor, lighting);
				}

				if (/*heightMap &&*/ floorcolor > 0 && (currentDist < ZBuffer[x][y] || ZBuffer[x][y] == 0)) {
					if (heightMap[cmapX][cmapY] - 1 > 0) {
						int raisedfloorstart = y - (int)(texture[heightMap[cmapX][cmapY] - 1][texWidth * floorTexY + floorTexX] / currentDist);
						if (raisedfloorstart > h - 1) continue;
						if (raisedfloorstart < 0) raisedfloorstart = 0;
						for (int ny = raisedfloorstart; ny < y; ny++) {
							if (ZBuffer[x][ny] > currentDist || ZBuffer[x][ny] == 0) {
								ZBuffer[x][ny] = currentDist; //perpendicular distance is used
								buffer[ny * bufferPitch + x] = floorcolor;
								interactionmap [x * S_WIDTH + ny] = 0;
								editorMap [x][ny] = ((short)cmapX) << 16 | ((short)cmapY);
							}
						}
						if (raisedfloorstart < y && y == h - 1 && y == h + expandeddraw) expandeddraw ++;
					}
				}
				//floor
				//ceiling (symmetrical!)
				//SET THE ZBUFFER FOR THE SPRITE CASTING
				if (floorcolor > 0 && (currentDist < ZBuffer[x][y] || ZBuffer[x][y] == 0)) {
					ZBuffer[x][y] = currentDist; //perpendicular distance is used
					buffer[y * bufferPitch + x] = floorcolor;
					editorMap [x][y] = ((short)cmapX) << 16 | ((short)cmapY);
				} else ZBuffer[x][y] = 9999999999999.0;
				if (currentDist < ZBuffer[x][h - y] || ZBuffer[x][h - y] == 0) {
					if (ceilingcolor > 0) {
						ZBuffer[x][h - y] = currentDist; //perpendicular distance is used
						buffer[(h - y) * bufferPitch + x] = ceilingcolor;
					} else ZBuffer[x][h - y] = 999999999999.0;
					editorMap [x][h - y] = ((short)cmapX) << 16 | ((short)cmapY);
				}
				interactionmap [x * S_WIDTH + y] = 0;
				interactionmap [x * S_WIDTH + (h - y)] = 0;
				if ((int)cmapX == selectedX && (int)cmapY == selectedY) {
					if (floorTexX == 0 || floorTexX == 63 || floorTexY == 0 || floorTexY == 63) {
						buffer[y * bufferPitch + x] = selectedColor;
						ZBuffer [x][y] = perpWallDist;
						buffer[(h - y) * bufferPitch + x] = selectedColor;
						ZBuffer [x][h - y] = perpWallDist;
					}
				}


			}
		}
		//TRANSLUCENT WALL RENDERING
		if (transslicedrawn[x] == true) {
			for (int y = 0; y < h; y++) {
				for (int transwalldrawn = 0; transwalldrawn < transwallcount; transwalldrawn++) {
					int transwalloffset = transwalldrawn * h;
					int color = transcolorbuffer[x][transwalloffset + y];
					if (color != 0) {
						if (transwallblendmode[transwalldrawn] == 0) buffer[y * bufferPitch + x] = Mix::MixColorAlpha(color, buffer[y * bufferPitch + x], transalphabuffer[x][transwalloffset + y]); //paint pixel if it isn't black, black is the invisible color
						else if (transwallblendmode[transwalldrawn] == 1) buffer[y * bufferPitch + x] = Mix::MixColorAdditive(color, buffer[y * bufferPitch + x], transalphabuffer[x][transwalloffset + y]);
						//if (ZBuffer[x][y] > transzbuffer[transwalldrawn*h+y]) ZBuffer[x][y] = transzbuffer[transwalldrawn*h+y]; //put the sprite on the zbuffer so we can draw around it.
					}
				}
			}
		}
		//End of wall loop.
	}


	//SPRITE CASTING
	//sort sprites from far to close

	//Initialize Sprites for casting.
	double invDet = 1.0 / (planeX * dirY - dirX * planeY);
	for (int i = 0; i < numSprites; i++) {
		spriteOrder[i] = i;
		spriteTransformY[i] = ((posX - sprite[i].x) * (posX - sprite[i].x) + (posY - sprite[i].y) * (posY - sprite[i].y));
	}
	combSort(spriteOrder, spriteTransformY, numSprites);
	for (int i = 0; i < numSprites; i++) {
		double spriteX = sprite[spriteOrder[i]].x - posX;
		double spriteY = sprite[spriteOrder[i]].y - posY;
		spriteTransformX[i] = invDet * (dirY * spriteX - dirX * spriteY);
		spriteTransformY[i] = invDet * (-planeY * spriteX + planeX * spriteY);
	}

	int transwalldraw = 0;
	//after sorting the sprites, do the projection and draw them
	for (int i = 0; i < numSprites; i++) {
		int flipped = 0;
		if (sprite[spriteOrder[i]].view != 0) {

			double sprrad = atan2(sprite[spriteOrder[i]].y - posY, sprite[spriteOrder[i]].x - posX);
			int sprdeg = (int)(sprrad / 3.1415 * 180.0);
			sprdeg = ((sprdeg + 180) + sprite[spriteOrder[i]].angle) % 360;
			int loop = 0;
			if (sprdeg > 336 || sprdeg < 23) loop = 0;
			else if (sprdeg > 22 && sprdeg < 68) loop = 6;
			else if (sprdeg > 67 && sprdeg < 113) loop = 1;
			else if (sprdeg > 112 && sprdeg < 158) loop = 7;
			else if (sprdeg > 157 && sprdeg < 203) loop = 3;
			else if (sprdeg > 202 && sprdeg < 248) loop = 5;
			else if (sprdeg > 247 && sprdeg < 293) loop = 2;
			else if (sprdeg > 292 && sprdeg < 337) loop = 4;
			AGSViewFrame *vf = engine->GetViewFrame(sprite[spriteOrder[i]].view, loop, sprite[spriteOrder[i]].frame);
			if (vf == nullptr) engine->AbortGame("Raycast_Render: Unable to load viewframe of sprite.");
			else {
				sprite[spriteOrder[i]].texture = vf->pic;
				PluginMethod sfGetGameParameter = engine->GetScriptFunctionAddress("GetGameParameter");
				flipped = sfGetGameParameter(13, sprite[spriteOrder[i]].view, loop, sprite[spriteOrder[i]].frame);
			}
		}
		//translate sprite position to relative to camera
		//double spriteX = sprite[spriteOrder[i]].x - posX;
		//double spriteY = sprite[spriteOrder[i]].y - posY;

		//transform sprite with the inverse camera matrix
		// [ planeX   dirX ] -1                                       [ dirY      -dirX ]
		// [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
		// [ planeY   dirY ]                                          [ -planeY  planeX ]

		//double invDet = 1.0 / (planeX * dirY - dirX * planeY); //required for correct matrix multiplication
		//double spriteX = sprite[spriteOrder[i]].x - posX;
		//double spriteY = sprite[spriteOrder[i]].y - posY;
		//double transformX = invDet * (dirY * spriteX - dirX * spriteY);
		//double transformY = invDet * (-planeY * spriteX + planeX * spriteY); //this is actually the depth inside the screen, that what Z is in 3D

		int spriteScreenX = int((w / 2) * (1 + spriteTransformX[i] / spriteTransformY[i]));

		//parameters for scaling and moving the sprites
		BITMAP *spritetexbm = engine->GetSpriteGraphic(sprite[spriteOrder[i]].texture);
		uint8 *spritetex = engine->GetRawBitmapSurface(spritetexbm);
		int spritetexPitch = engine->GetBitmapPitch(spritetexbm);
		int sprw = engine->GetSpriteWidth(sprite[spriteOrder[i]].texture);
		int sprh = engine->GetSpriteHeight(sprite[spriteOrder[i]].texture);

		double uDiv = ((double)sprw / (double)texWidth) + sprite[spriteOrder[i]].uDivW;
		double vDiv = ((double)sprh / (double)texHeight) + sprite[spriteOrder[i]].uDivH;
		double vMove = sprite[spriteOrder[i]].vMove + ((double)texHeight - (double)sprh) * 1.6;
		double hMove = sprite[spriteOrder[i]].hMove;
		int vMoveScreen = int(vMove / spriteTransformY[i]);
		int hMoveScreen = int(hMove / spriteTransformY[i]);
		//calculate height of the sprite on screen
		int spriteHeight = abs(int(h / (spriteTransformY[i]) * vDiv)) ; //using "transformY" instead of the real distance prevents fisheye
		//calculate lowest and highest pixel to fill in current stripe
		int drawStartY = -spriteHeight / 2 + h / 2 + vMoveScreen;
		if (drawStartY < 0) drawStartY = 0;
		int drawEndY = spriteHeight / 2 + h / 2 + vMoveScreen;
		if (drawEndY >= h) drawEndY = h - 1;

		//calculate width of the sprite
		int spriteWidth = abs(int (h / (spriteTransformY[i]) * uDiv)) ;
		int drawStartX = -spriteWidth / 2 + spriteScreenX + hMoveScreen;
		if (drawStartX < 0) drawStartX = 0;
		int drawEndX = spriteWidth / 2 + spriteScreenX + hMoveScreen;
		if (drawEndX >= w) drawEndX = w - 1;
		int spr_light = lightMap [(int)sprite[spriteOrder[i]].x][(int)sprite[spriteOrder[i]].y] << 5;
		spr_light = MIN(255, MAX(0, spr_light));
		int floorTexX = int(sprite[spriteOrder[i]].x * texWidth) % texWidth;
		int floorTexY = int(sprite[spriteOrder[i]].y * texHeight) % texHeight;
		if (ceilingMap [(int)sprite[spriteOrder[i]].x][(int)sprite[spriteOrder[i]].y] == 0) {
			spr_light = MAX(spr_light, ambientlight);
		} else if (texture[ceilingMap [(int)sprite[spriteOrder[i]].x][(int)sprite[spriteOrder[i]].y] - 1][texWidth * floorTexY + floorTexX] == 0) spr_light = MAX(spr_light, ambientlight);
		//loop through every vertical stripe of the sprite on screen


		for (int stripe = drawStartX; stripe < drawEndX; stripe++) {
			transwalldraw = 0;
			//int texX = int(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * texWidth / spriteWidth) / 256;
			int texX = int(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) *  sprw / spriteWidth) / 256;
			if (texX >= sprw || texX < 0) continue;
			if (flipped) texX = sprw - texX;
			//the conditions in the if are:
			//1) it's in front of camera plane so you don't see things behind you
			//2) it's on the screen (left)
			//3) it's on the screen (right)
			//4) ZBuffer, with perpendicular distance
			if (spriteTransformY[i] > 0 && stripe > 0 && stripe < w)
				for (int y = drawStartY; y < drawEndY; y++) { //for every pixel of the current stripe
					if (spriteTransformY[i] < ZBuffer[stripe][y]) {
						if (transslicedrawn[stripe]) while ((transzbuffer[stripe][transwalldraw * h + y] > spriteTransformY[i] && transzbuffer[stripe][transwalldraw * h + y] != 0) && (transwalldraw < transwallcount)) transwalldraw++;
						int d = (y - vMoveScreen) * 256 - h * 128 + spriteHeight * 128; //256 and 128 factors to avoid floats
						//int texY = ((d * texHeight) / spriteHeight) / 256;
						int texY = ((d * sprh) / spriteHeight) / 256;
						if (texY >= sprh || texY < 0) continue;
						//unsigned char color = texture[sprite[spriteOrder[i]].texture][texWidth * texY + texX]; //get current color from the texture
						unsigned char color = spritetex[texY * spritetexPitch + texX]; //get current color from the texture
						if (color != 0) {
							if (sprite[spriteOrder[i]].alpha < 255) {
								if (sprite[spriteOrder[i]].blendmode == 0) color = Mix::MixColorAlpha(color, buffer[y * bufferPitch + stripe], sprite[spriteOrder[i]].alpha);
								if (sprite[spriteOrder[i]].blendmode == 1) color = Mix::MixColorAdditive(color, buffer[y * bufferPitch + stripe], sprite[spriteOrder[i]].alpha);
							}
							color = Mix::MixColorLightLevel(color, spr_light);
							if (transzbuffer[stripe][transwalldraw * h + y] < spriteTransformY[i] && transzbuffer[stripe][transwalldraw * h + y] != 0 && transslicedrawn[stripe] && transcolorbuffer[stripe][transwalldraw * h + y] > 0 && transalphabuffer[stripe][transwalldraw * h + y] > 0) {
								if (transwallblendmode[transwalldraw] == 0) color = Mix::MixColorAlpha(color, transcolorbuffer[stripe][transwalldraw * h + y], transalphabuffer[stripe][transwalldraw * h + y]);
								else if (transwallblendmode[transwalldraw] == 1) color = Mix::MixColorAdditive(color, transcolorbuffer[stripe][transwalldraw * h + y], transalphabuffer[stripe][transwalldraw * h + y]);
								buffer[y * bufferPitch + stripe] = color;
								ZBuffer[stripe][y] = transzbuffer[stripe][transwalldraw * h + y];
							} else {
								buffer[y * bufferPitch + stripe] = color; //paint pixel if it isn't black, black is the invisible color
								ZBuffer[stripe][y] = spriteTransformY[i]; //put the sprite on the zbuffer so we can draw around it.
							}
							interactionmap [stripe * S_WIDTH + y] = sprite[spriteOrder[i]].objectinteract << 8;
						}
					}
				}
		}
		engine->ReleaseBitmapSurface(spritetexbm);
	}
	engine->ReleaseBitmapSurface(screen);
	engine->NotifySpriteUpdated(slot);
	rendering = false;

}

void AGSPalRender::QuitCleanup(ScriptMethodParams &) {
	if (!rendering) {
		for (int i = 0; i < S_WIDTH; ++i) {
			if (transcolorbuffer[i])delete[] transcolorbuffer[i];
			if (transalphabuffer[i])delete[] transalphabuffer[i];
			if (transzbuffer[i])delete[] transzbuffer[i];
			if (ZBuffer[i]) delete[] ZBuffer[i];
		}
		if (transcolorbuffer) delete[] transcolorbuffer;
		if (transalphabuffer) delete[] transalphabuffer;
		if (transzbuffer) delete[] transzbuffer;
		if (ZBuffer) delete[] ZBuffer;
		if (transwallblendmode) delete[] transwallblendmode;
		if (interactionmap) delete[] interactionmap;
	}
}

void AGSPalRender::MoveForward(ScriptMethodParams &) {
	double newposx = 0;
	if (dirX > 0) newposx = 0.1 + posX + dirX * moveSpeed;
	else newposx = -0.1 + posX + dirX * moveSpeed;
	int texsidex = 0;
	int inside_texsidex = 0;
	double newposy = 0;
	if (dirY > 0) newposy = 0.1 + posY + dirY * moveSpeed;
	else newposy = -0.1 + posY + dirY * moveSpeed;
	int texsidey = 0;
	int inside_texsidey = 0;
	bool inside = false;
	if ((int)newposx == (int)posX && (int)newposy == (int)posY) inside = true;
	if (dirX > 0 && !inside) {
		texsidex = 0;
		inside_texsidex = 1;
	} else if (dirX > 0) {
		texsidex = 1;
		inside_texsidex = 0;
	}
	if (dirX < 0 && !inside) {
		texsidex = 1;
		inside_texsidex = 0;
	} else if (dirX < 0) {
		texsidex = 0;
		inside_texsidex = 1;
	}

	if (dirY > 0 && !inside) {
		texsidey = 2;
		inside_texsidey = 3;
	} else if (dirY > 0) {
		texsidey = 3;
		inside_texsidey = 2;
	}
	if (dirY < 0 && !inside) {
		texsidey = 3;
		inside_texsidey = 2;
	} else if (dirY < 0) {
		texsidey = 2;
		inside_texsidey = 3;
	}

	if (!noclip && !inside) {
		if (wallData[worldMap[int(newposx)][int(posY)]].solid[texsidex] == false && wallData[worldMap[int(posX)][int(posY)]].solid[inside_texsidex] == false && int(newposx) > -1 && int(newposx) < MAP_WIDTH) posX += dirX * moveSpeed;
		if (wallData[worldMap[int(posX)][int(newposy)]].solid[texsidey] == false && wallData[worldMap[int(posX)][int(posY)]].solid[inside_texsidey] == false && int(newposy) > -1 && int(newposy) < MAP_HEIGHT) posY += dirY * moveSpeed;
	} else if (!noclip && inside) {
		posX += dirX * moveSpeed;
		posY += dirY * moveSpeed;
	} else {
		if (int(newposx) > -1 && int(newposx) < MAP_WIDTH) posX += dirX * moveSpeed;
		if (int(newposy) > -1 && int(newposy) < MAP_HEIGHT) posY += dirY * moveSpeed;
	}
}

void AGSPalRender::MoveBackward(ScriptMethodParams &) {
	double newposx = 0;
	if (dirX > 0) newposx = -0.1 + posX - dirX * moveSpeed;
	else newposx = 0.1 + posX - dirX * moveSpeed;
	int texsidex = 0;
	int inside_texsidex = 0;
	double newposy = 0;
	if (dirY > 0) newposy = -0.1 + posY - dirY * moveSpeed;
	else newposy = 0.1 + posY - dirY * moveSpeed;
	int texsidey = 0;
	int inside_texsidey = 0;
	bool inside = false;
	if ((int)newposx == (int)posX && (int)newposy == (int)posY) inside = true;
	if (dirX > 0 && !inside) {
		texsidex = 1;
		inside_texsidex = 0;
	} else if (dirX > 0) {
		texsidex = 0;
		inside_texsidex = 1;
	}
	if (dirX < 0 && !inside) {
		texsidex = 0;
		inside_texsidex = 1;
	} else if (dirX < 0) {
		texsidex = 1;
		inside_texsidex = 0;
	}

	if (dirY > 0 && !inside) {
		texsidey = 3;
		inside_texsidey = 2;
	} else if (dirY > 0) {
		texsidey = 2;
		inside_texsidey = 3;
	}
	if (dirY < 0 && !inside) {
		texsidey = 2;
		inside_texsidey = 3;
	} else if (dirY < 0) {
		texsidey = 3;
		inside_texsidey = 2;
	}

	if ((int)posX == (int)newposy && (int)posY == (int)newposy) inside = true;
	if (!noclip && !inside) {
		if (wallData[worldMap[int(newposx)][int(posY)]].solid[texsidex] == false && wallData[worldMap[int(posX)][int(posY)]].solid[inside_texsidex] == false && int(newposx) > -1 && int(newposx) < MAP_WIDTH) posX -= dirX * moveSpeed;
		if (wallData[worldMap[int(posX)][int(newposy)]].solid[texsidey] == false && wallData[worldMap[int(posX)][int(posY)]].solid[inside_texsidey] == false && int(newposy) > -1 && int(newposy) < MAP_HEIGHT) posY -= dirY * moveSpeed;
	} else if (!noclip && inside) {
		posX -= dirX * moveSpeed;
		posY -= dirY * moveSpeed;
	} else {
		if (int(newposx) > -1 && int(newposx) < MAP_WIDTH) posX -= dirX * moveSpeed;
		if (int(newposy) > -1 && int(newposy) < MAP_HEIGHT) posY -= dirY * moveSpeed;
	}
}



/*
void MoveBackward ()
{
	  double newposx;
	  if (dirX > 0) newposx = -0.2 + posX - dirX * moveSpeed;
	  else newposx = 0.2 + posX - dirX * moveSpeed;
	  double newposy;
	  if (dirY > 0) newposy = -0.2 + posY - dirY * moveSpeed;
	  else newposy = 0.2 + posY - dirY * moveSpeed;
	  if(worldMap[int(newposx)][int(posY)] == false || worldMap[int(newposx)][int(posY)] > 8) posX -= dirX * moveSpeed;
	  if(worldMap[int(posX)][int(newposy)] == false || worldMap[int(posX)][int(newposy)] > 8) posY -= dirY * moveSpeed;
}
*/

void AGSPalRender::RotateLeft(ScriptMethodParams &) {
	double oldDirX = dirX;
	dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
	dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
	double oldPlaneX = planeX;
	planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
	planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
}

void AGSPalRender::RotateRight(ScriptMethodParams &) {
	double oldDirX = dirX;
	dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
	dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
	double oldPlaneX = planeX;
	planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
	planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
}

//sort algorithm
void combSort(int *order, double *dist, int amount) {
	int gap = amount;
	bool swapped = false;
	while (gap > 1 || swapped) {
		//shrink factor 1.3
		gap = (gap * 10) / 13;
		if (gap == 9 || gap == 10) gap = 11;
		if (gap < 1) gap = 1;
		swapped = false;
		for (int i = 0; i < amount - gap; i++) {
			int j = i + gap;
			if (dist[i] < dist[j]) {
				SWAP(dist[i], dist[j]);
				SWAP(order[i], order[j]);
				swapped = true;
			}
		}
	}
}

} // namespace AGSPalRender
} // namespace Plugins
} // namespace AGS3

