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

#include "ags/lib/allegro.h"
#include "ags/plugins/ags_pal_render/raycast.h"

namespace AGS3 {
namespace Plugins {
namespace AGSPalRender {

#define PI         (3.1415926535f)

//Variable Declaration
bool raycastOn;
double posX = 22.0, posY = 11.5; //x and y start position
double dirX = -1.0, dirY = 0.0; //initial direction vector
double planeX = 0.0, planeY = 0.77; //the 2d raycaster version of camera plane
double moveSpeed = (1.0 / 60.0) * 3.0; //the constant value is in squares/second
double rotSpeed = (1.0 / 60.0) * 2.0; //the constant value is in radians/second
unsigned char worldMap[64][64];
unsigned char lightMap[64][64];
int ceilingMap[64][64];
int floorMap[64][64];
int heightMap[64][64];
unsigned char seenMap[64][64];
//int mapWidth;
//int mapHeight;
#define mapWidth 64
#define mapHeight 64
int textureSlot;
int ambientlight;
int ambientweight = 0;
int ambientcolor = 0;
int ambientcolorAmount = 0;

Sprite sprite[numSprites] = {};


#define sWidth 320
#define sHeight 160

int editorMap [sWidth][sHeight] = {};

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

void Ray_SelectTile(int x, int y, unsigned char color) {
	if (x < 0 || x > mapWidth) selectedX = -1;
	else if (y < 0 || y > mapWidth) selectedY = -1;
	else {
		selectedX = x;
		selectedY = y;
		selectedColor = color;
	}
}

int Ray_HasSeenTile(int x, int y) {
	if (x < 0 || x > mapWidth) return -1;
	else if (y < 0 || y > mapWidth) return -1;
	return seenMap [x][y];
}

void Ray_SetNoClip(int value) {
	noclip = value;
}

int Ray_GetNoClip() {
	return noclip;
}

void Ray_DrawTile(int spr, int tile) {
	BITMAP *sprite = engine->GetSpriteGraphic(spr);
	unsigned char **sprarray = engine->GetRawBitmapSurface(sprite);
	for (int y = 0; y < 64; ++y)
		for (int x = 0; x < 64; ++x)
			sprarray [y][x] = texture [tile][(texWidth * y) + x];
	engine->ReleaseBitmapSurface(sprite);
}

void Ray_DrawOntoTile(int spr, int tile) {
	BITMAP *sprite = engine->GetSpriteGraphic(spr);
	unsigned char **sprarray = engine->GetRawBitmapSurface(sprite);
	for (int y = 0; y < 64; ++y)
		for (int x = 0; x < 64; ++x)
			texture [tile][(texWidth * y) + x] = sprarray [y][x];
	engine->ReleaseBitmapSurface(sprite);
}

int Ray_GetTileX_At(int x, int y) {
	if (x < 0 || x > 319  || y < 0 || y > 159) return -1;
	else return editorMap [x][y] >> 16;
}

int Ray_GetTileY_At(int x, int y) {
	if (x < 0 || x > 319  || y < 0 || y > 159) return -1;
	else return editorMap [x][y] & 0x0000FFFF;
}

void Ray_SetWallAt(int x, int y, int id) {
	if (x < 0 || x >= mapWidth) return;
	if (y < 0 || y >= mapHeight) return;
	worldMap [x][y] = id;
}

int Ray_GetWallAt(int x, int y) {
	if (x < 0 || x >= mapWidth) return -1;
	if (y < 0 || y >= mapHeight) return -1;
	return worldMap [x][y];
}

int Ray_GetAmbientWeight() {
	return ambientweight;
}

void Ray_SetAmbientLight(int value) {
	ambientlight = MIN(255, MAX(0, value));
}

void Ray_SetAmbientColor(int color, int amount) {
	ambientcolor = color;
	ambientcolorAmount = amount;
}

int Ray_GetAmbientLight() {
	return ambientlight;
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

void Ray_SetWallHotspot(int id, char hotsp) {
	wallData[id].hotspotinteract = hotsp;
}

void Ray_SetWallTextures(int id, int n, int s, int w, int e) {
	wallData[id].texture[0] = n;
	wallData[id].texture[1] = s;
	wallData[id].texture[2] = w;
	wallData[id].texture[3] = e;
}

void Ray_SetWallSolid(int id, int n, int s, int w, int e) {
	wallData[id].solid [0] = MAX(0, MIN(n, 1));
	wallData[id].solid [1] = MAX(0, MIN(s, 1));
	wallData[id].solid [2] = MAX(0, MIN(w, 1));
	wallData[id].solid [3] = MAX(0, MIN(e, 1));
}

void Ray_SetWallIgnoreLighting(int id, int n, int s, int w, int e) {
	wallData[id].ignorelighting [0] = MAX(0, MIN(n, 1));
	wallData[id].ignorelighting [1] = MAX(0, MIN(s, 1));
	wallData[id].ignorelighting [2] = MAX(0, MIN(w, 1));
	wallData[id].ignorelighting [3] = MAX(0, MIN(e, 1));
}

void Ray_SetWallAlpha(int id, int n, int s, int w, int e) {
	wallData[id].alpha [0] = MAX(0, MIN(n, 255));
	wallData[id].alpha [1] = MAX(0, MIN(s, 255));
	wallData[id].alpha [2] = MAX(0, MIN(w, 255));
	wallData[id].alpha [3] = MAX(0, MIN(e, 255));
}

void Ray_SetWallBlendType(int id, int n, int s, int w, int e) {
	wallData[id].blendtype [0] = MAX(0, MIN(n, 10));
	wallData[id].blendtype [1] = MAX(0, MIN(s, 10));
	wallData[id].blendtype [2] = MAX(0, MIN(w, 10));
	wallData[id].blendtype [3] = MAX(0, MIN(e, 10));
}




int Ray_GetWallHotspot(int id) {
	return wallData[id].hotspotinteract;
}

int Ray_GetWallTexture(int id, int dir) {
	return wallData[id].texture[dir];
}

int Ray_GetWallSolid(int id, int dir) {
	return wallData[id].solid [dir];
}

int Ray_GetWallIgnoreLighting(int id, int dir) {
	return wallData[id].ignorelighting [dir];
}

int Ray_GetWallAlpha(int id, int dir) {
	return wallData[id].alpha [dir];
}

int Ray_GetWallBlendType(int id, int dir) {
	return wallData[id].blendtype [dir];
}





FLOAT_RETURN_TYPE Ray_GetMoveSpeed() {
	float mSpeed = (float)moveSpeed;
	RETURN_FLOAT(mSpeed);
}


void Ray_SetMoveSpeed(SCRIPT_FLOAT(speed)) {
	INIT_SCRIPT_FLOAT(speed);
	moveSpeed = (double)speed;
}

FLOAT_RETURN_TYPE Ray_GetRotSpeed() {
	float rSpeed = (float)rotSpeed;
	RETURN_FLOAT(rSpeed);
}

void Ray_SetRotSpeed(SCRIPT_FLOAT(speed)) {
	INIT_SCRIPT_FLOAT(speed);
	rotSpeed = (double)speed;
}


int Ray_GetLightAt(int x, int y) {
	return lightMap [x][y];
}

void Ray_SetLightAt(int x, int y, int light) {
	lightMap [x][y] = light;
}

void Ray_SetPlaneY(SCRIPT_FLOAT(y)) {
	INIT_SCRIPT_FLOAT(y);
	planeY = (double)y;
}

FLOAT_RETURN_TYPE Ray_GetPlaneY() {
	float pY = (float)planeY;
	RETURN_FLOAT(pY);
}

void Ray_SetPlayerPosition(SCRIPT_FLOAT(x), SCRIPT_FLOAT(y)) {
	INIT_SCRIPT_FLOAT(x);
	INIT_SCRIPT_FLOAT(y);
	posX = (double)x;
	posY = (double)y;
}

FLOAT_RETURN_TYPE Ray_GetPlayerX() {

	float x = (float)posX;
	RETURN_FLOAT(x);
}

FLOAT_RETURN_TYPE Ray_GetPlayerY() {
	float y = (float)posY;
	RETURN_FLOAT(y);
}

int Ray_GetPlayerAngle() {
	double bgrad = atan2(dirY, dirX);
	int bgdeg = (int)(bgrad / PI * 180.0) + 180;
	return bgdeg % 360;
}

void Ray_SetPlayerAngle(int angle) {
	int realangle = angle % 360;
	if (realangle < 0) realangle += 360;

	int anglediff = realangle - Ray_GetPlayerAngle();
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
	if (tempw != mapWidth || temph != mapHeight) engine->AbortGame("LoadHeightMap: Map sizes are mismatched!");
	BITMAP *heightmapBm = engine->GetSpriteGraphic(heightmapSlot);
	if (!heightmapBm) engine->AbortGame("LoadHeightMap: Cannot load sprite into memory.");
	unsigned char **hmArray = engine->GetRawBitmapSurface(heightmapBm);

	for (int i = 0; i < tempw; i++) {
		for (int j = 0; j < temph; j++) {
			heightMap[i][j] = hmArray[i][j];
		}
	}
	engine->ReleaseBitmapSurface(heightmapBm);
	heightmapOn = true;
}

void LoadMap(int worldmapSlot, int lightmapSlot, int ceilingmapSlot, int floormapSlot) {
	int tempw = engine->GetSpriteWidth(worldmapSlot);
	int temph = engine->GetSpriteHeight(worldmapSlot);
	BITMAP *worldmapBm;
	BITMAP *lightmapBm;
	BITMAP *floormapBm;
	BITMAP *ceilingmapBm;
	unsigned char **wmArray;
	unsigned char **lmArray;
	unsigned char **fmArray;
	unsigned char **cmArray;
	worldmapBm = engine->GetSpriteGraphic(worldmapSlot);
	if (!worldmapBm) engine->AbortGame("LoadMap: Couldn't load worldmap sprite into memory.");
	wmArray = engine->GetRawBitmapSurface(worldmapBm);
	if (engine->GetSpriteWidth(lightmapSlot) != tempw || engine->GetSpriteHeight(lightmapSlot) != temph) engine->AbortGame("LoadMap: Lightmap has different dimensions to worldmap.");
	else {
		lightmapBm = engine->GetSpriteGraphic(lightmapSlot);
		if (!lightmapBm) engine->AbortGame("LoadMap: Couldn't load lightmap sprite into memory.");
		lmArray = engine->GetRawBitmapSurface(lightmapBm);
	}
	if (engine->GetSpriteWidth(ceilingmapSlot) != tempw || engine->GetSpriteHeight(ceilingmapSlot) != temph) engine->AbortGame("LoadMap: Ceilingmap has different dimensions to worldmap.");
	else {
		ceilingmapBm = engine->GetSpriteGraphic(ceilingmapSlot);
		if (!ceilingmapBm) engine->AbortGame("LoadMap: Couldn't load ceilingmap sprite into memory.");
		cmArray = engine->GetRawBitmapSurface(ceilingmapBm);
	}
	if (engine->GetSpriteWidth(floormapSlot) != tempw || engine->GetSpriteHeight(floormapSlot) != temph) engine->AbortGame("LoadMap: Floormap has different dimensions to worldmap.");
	else {
		floormapBm = engine->GetSpriteGraphic(floormapSlot);
		if (!floormapBm) engine->AbortGame("LoadMap: Couldn't load floormap sprite into memory.");
		fmArray = engine->GetRawBitmapSurface(floormapBm);
	}
	for (int i = 0; i < tempw; i++) {
		for (int j = 0; j < temph; j++) {
			worldMap[i][j] = wmArray[i][j];
			lightMap[i][j] = lmArray[i][j];
			floorMap[i][j] = fmArray[i][j];
			ceilingMap[i][j] = cmArray[i][j];
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

FLOAT_RETURN_TYPE Ray_GetSpriteScaleX(int id) {
	float scale = (float)sprite[id].uDivW;
	RETURN_FLOAT(scale);
}

void Ray_SetSpriteScaleX(int id, SCRIPT_FLOAT(scale)) {
	INIT_SCRIPT_FLOAT(scale);
	sprite[id].uDivW = scale;
}

FLOAT_RETURN_TYPE Ray_GetSpriteScaleY(int id) {
	float scale = (float)sprite[id].uDivH;
	RETURN_FLOAT(scale);
}

void Ray_SetSpriteScaleY(int id, SCRIPT_FLOAT(scale)) {
	INIT_SCRIPT_FLOAT(scale);
	sprite[id].uDivH = scale;
}

int Ray_GetSpriteAlpha(int id) {
	return sprite[id].alpha;
}

void Ray_SetSpriteAlpha(int id, int alpha) {
	sprite[id].alpha = alpha;
}

int Ray_GetSpritePic(int id) {
	return sprite[id].texture;
}

void Ray_SetSpritePic(int id, int slot) {
	sprite[id].texture = slot;
}



int Ray_GetSpriteAngle(int id) {
	return sprite[id].angle;
}

void Ray_SetSpriteAngle(int id, int angle) {
	sprite[id].angle = angle % 360;
}

int Ray_GetSpriteInteractObj(int id) {
	return sprite[id].objectinteract;
}

void Ray_SetSpriteView(int id, int view) {
	sprite[id].view = view;
}

void Ray_SetSpriteBlendType(int id, int type) {
	sprite[id].blendmode = type;
}

int Ray_GetSpriteBlendType(int id) {
	return sprite[id].blendmode;
}


int Ray_GetSpriteView(int id) {
	return sprite[id].view;
}

void Ray_SetSpriteFrame(int id, int frame) {
	sprite[id].frame = frame;
}

int Ray_GetSpriteFrame(int id) {
	return sprite[id].frame;
}

void Ray_SetSpriteInteractObj(int id, int obj) {
	sprite[id].objectinteract = obj;
}

void Ray_SetSpritePosition(int id, SCRIPT_FLOAT(x), SCRIPT_FLOAT(y)) {
	INIT_SCRIPT_FLOAT(x);
	INIT_SCRIPT_FLOAT(y);
	sprite[id].x = x;
	sprite[id].y = y;
}

void Ray_SetSpriteVertOffset(int id, SCRIPT_FLOAT(vMove)) {
	INIT_SCRIPT_FLOAT(vMove);
	sprite[id].vMove = vMove;
}


FLOAT_RETURN_TYPE Ray_GetSpriteVertOffset(int id) {
	float x = (float)sprite[id].vMove;
	RETURN_FLOAT(x);
}

FLOAT_RETURN_TYPE Ray_GetSpriteX(int id) {
	float x = (float)sprite[id].x;
	RETURN_FLOAT(x);
}

FLOAT_RETURN_TYPE Ray_GetSpriteY(int id) {
	float y = (float)sprite[id].y;
	RETURN_FLOAT(y);
}

void Ray_InitSprite(int id, SCRIPT_FLOAT(x), SCRIPT_FLOAT(y), int slot, unsigned char alpha, int blendmode, SCRIPT_FLOAT(scale_x), SCRIPT_FLOAT(scale_y), SCRIPT_FLOAT(vMove)) {
	INIT_SCRIPT_FLOAT(x);
	INIT_SCRIPT_FLOAT(y);
	INIT_SCRIPT_FLOAT(scale_x);
	INIT_SCRIPT_FLOAT(scale_y);
	INIT_SCRIPT_FLOAT(vMove);
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

void MakeTextures(int slot) {
	textureSlot = slot;
	int sourceWidth = engine->GetSpriteWidth(slot);
	int sourceHeight = engine->GetSpriteHeight(slot);
	int max = (sourceWidth / texWidth) * (sourceHeight / texHeight);
	if (max > MAX_TEXTURES) engine->AbortGame("MakeTextures: Source file has too many tiles to load.");
	BITMAP *texspr = engine->GetSpriteGraphic(slot);
	unsigned char **texbuffer = engine->GetRawBitmapSurface(texspr);
	int numTilesX = sourceWidth / texWidth;
	int numTilesY = sourceHeight / texHeight;
	int totaltiles = numTilesX * numTilesY;
	for (int numX = 0; numX < numTilesX; ++numX) {
		for (int numY = 0; numY < numTilesY; ++numY) {
			for (int x = 0; x < texWidth; ++x)
				for (int y = 0; y < texHeight; ++y) {
					texture[(numY * numTilesX) + numX][(texWidth * y) + x] = texbuffer [y + (texHeight * numY)][x + (texWidth * numX)];
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

void Ray_SetFloorAt(int x, int y, int tex) {
	if (x < 0 || x > mapWidth || y < 0 || y > mapHeight || tex > 511) return;
	else floorMap[x][y] = tex;
}

void Ray_SetCeilingAt(int x, int y, int tex) {
	if (x < 0 || x > mapWidth || y < 0 || y > mapHeight || tex > 511) return;
	else ceilingMap[x][y] = tex;
}

int Ray_GetCeilingAt(int x, int y) {
	if (x < 0 || x > mapWidth || y < 0 || y > mapHeight) return -1;
	else return ceilingMap [x][y];
}


int Ray_GetFloorAt(int x, int y) {
	if (x < 0 || x > mapWidth || y < 0 || y > mapHeight) return -1;
	else return floorMap [x][y];
}


int Ray_GetLightingAt(int x, int y) {
	if (x < 0 || x > mapWidth || y < 0 || y > mapHeight) return -1;
	else {
		int lighting = 0;
		if (ceilingMap[x][y] == 0) {
			lighting = ambientlight;
			if (ambientlight < lightMap [x][y]) lighting = lightMap[x][y];
		}
		return lighting;
	}
}

void Ray_SetLightingAt(int x, int y, unsigned char lighting) {
	if (x < 0 || x > mapWidth || y < 0 || y > mapHeight) return;
	else {
		lightMap [x][y] = lighting;
	}
}

void Ray_SetSkyBox(int slot) {
	BITMAP *test = engine->GetSpriteGraphic(slot);
	if (test) {
		skybox = slot;
	} else engine->AbortGame("Ray_SetSkybox: No such sprite!");
}

int Ray_GetSkyBox(int slot) {
	return skybox;
}

int Ray_GetHotspotAt(int x, int y) {
	if (!interactionmap) return -1;
	else if (x > sWidth || x < 0 || y > sHeight || y < 0) return -1;
	else return interactionmap [x * sWidth + y] & 0x00FF;
}

int Ray_GetObjectAt(int x, int y) {
	if (!interactionmap) return -1;
	else if (x > sWidth || x < 0 || y > sHeight || y < 0) return -1;
	else return interactionmap [x * sWidth + y] >> 8;
}

FLOAT_RETURN_TYPE Ray_GetDistanceAt(int x, int y) {
	float falsereturn = -1.0f;
	if (!ZBuffer) {
		RETURN_FLOAT(falsereturn);
	} else if (x > sWidth || x < 0 || y > sHeight || y < 0) {
		RETURN_FLOAT(falsereturn);
	} else {

		float zbuf = (float)ZBuffer[x][y];
		RETURN_FLOAT(zbuf);
	}
}

void Init_Raycaster() {
	if (ZBuffer) return;
	if (!worldMap) return;
	transcolorbuffer = new unsigned char *[sWidth];
	transalphabuffer = new unsigned char *[sWidth];
	transslicedrawn = new bool[sWidth]();
	transzbuffer = new double*[sWidth];
	transwallblendmode = new int [mapWidth]();
	ZBuffer = new double*[sWidth];
	distTable = new double[sHeight + (sHeight >> 1)];
	interactionmap = new short[sWidth * sHeight]();
	for (int y = 0; y < sHeight + (sHeight >> 1); y++) {
		distTable [y] = sHeight / (2.0 * y - sHeight);
	}
	for (int x = 0; x < sWidth; x++) {
		transcolorbuffer[x] = new unsigned char [sHeight * (mapWidth)]();
		transalphabuffer[x] = new unsigned char [sHeight * (mapWidth)]();
		transzbuffer[x] = new double [sHeight * (mapWidth)]();
		ZBuffer[x] = new double [sHeight]();
		transslicedrawn [x] = false;
	}
}

bool rendering;
void Raycast_Render(int slot) {
	ambientweight = 0;
	raycastOn = true;
	double playerrad = atan2(dirY, dirX) + (2.0 * PI);
	rendering = true;
	int32 w = sWidth, h = sHeight;
	BITMAP *screen = engine->GetSpriteGraphic(slot);
	if (!screen) engine->AbortGame("Raycast_Render: No valid sprite to draw on.");
	engine->GetBitmapDimensions(screen, &w, &h, nullptr);
	BITMAP *sbBm = engine->GetSpriteGraphic(skybox);
	if (!sbBm) engine->AbortGame("Raycast_Render: No valid skybox sprite.");
	if (skybox > 0) {
		int bgdeg = (int)((playerrad / PI) * 180.0) + 180;
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
	unsigned char **buffer = engine->GetRawBitmapSurface(screen);
	for (int x = 0; x < w; x++) {
		transslicedrawn [x] = false;
		for (int y = 0; y < h; y++) {
			ZBuffer[x][y] = 0;
		}
	}
	int multiplier = mapWidth;
	memset(interactionmap, 0, sizeof(short) * (sHeight * sWidth));
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
		double perpWallDist;

		//what direction to step in x or y-direction (either +1 or -1)
		int stepX;
		int stepY;
		int prevmapX = 0;
		int prevmapY = 0;
		int hit = 0; //was there a wall hit?
		int side; //was a NS or a EW wall hit?

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
		//perform DDA
		bool deeper = true;
		bool opposite = true;
		bool oppositedrawn = false;
		double wallX; //where exactly the wall was hit
		int drawStart;
		int drawEnd;
		while (hit == 0 && deeper == true) {
			if (opposite) {
				rayDirX = -rayDirX;
				rayDirY = -rayDirY;
				stepX = -stepX;
				stepY = -stepY;
				if (sideDistX < sideDistY) side = 0;
				else side = 1;
			} else if (sideDistX < sideDistY) { //jump to next map square, OR in x-direction, OR in y-direction
				sideDistX += deltaDistX;
				mapX += stepX;
				mapX = abs(mapX) % mapHeight;
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
				mapY = abs(mapY) % mapHeight;
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
			int texside;
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
						wall_light = lightMap [(int)mapX - 1 % mapWidth][(int)mapY] << 5;
						if (ceilingMap [(int)mapX - 1 % mapWidth][(int)mapY] <= 1) do_ambient = true;
						else if (texture[ceilingMap [(int)mapX - 1 % mapWidth][(int)mapY] - 1][(texWidth * (63 - texX)) + 63] == 0) do_ambient = true;
					}
					if (rayDirX < 0 && side == 0) {
						wall_light = lightMap [(int)mapX + 1 % mapWidth][(int)mapY] << 5;
						if (ceilingMap [(int)mapX + 1 % mapWidth][(int)mapY] <= 1) do_ambient = true;
						else if (texture[ceilingMap [(int)mapX + 1 % mapWidth][(int)mapY] - 1][(texWidth * texX) + 0] == 0) do_ambient = true;

					}
					if (rayDirY > 0 && side == 1) {
						wall_light = lightMap [(int)mapX][(int)mapY - 1 % mapHeight] << 5;
						if (ceilingMap [(int)mapX][(int)mapY - 1 % mapHeight] <= 1) do_ambient = true;
						else if (texture[ceilingMap [(int)mapX][(int)mapY - 1 % mapHeight] - 1][(texWidth * 63) + texX] == 0) do_ambient = true;
					}
					if (rayDirY < 0 && side == 1) {
						wall_light = lightMap [(int)mapX][(int)mapY + 1 % mapHeight] << 5;
						if (ceilingMap [(int)mapX][(int)mapY + 1 % mapHeight] <= 1) do_ambient = true;
						else if (texture[ceilingMap [(int)mapX][(int)mapY + 1 % mapHeight] - 1][(texWidth * 0) + 63 - texX] == 0) do_ambient = true;
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
								buffer[y][x] = color;
								if (ambientpixels) ambientweight++;
								//SET THE ZBUFFER FOR THE SPRITE CASTING
								ZBuffer[x][y] = perpWallDist; //perpendicular distance is used
								interactionmap [x * sWidth + y] = wallData[worldMap[mapX][mapY]].hotspotinteract;
								editorMap [x][y] = ((short)mapX) << 16 | ((short)mapY);
							} else {
								if (transslicedrawn[x] == false) {
									memset(transcolorbuffer[x], 0, sizeof(unsigned char) * (sHeight * mapWidth));
									memset(transalphabuffer[x], 0, sizeof(unsigned char) * (sHeight * mapWidth));
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
								buffer[y][x] = selectedColor;
								ZBuffer [x][y] = perpWallDist;
							}
						}

					}
				}
				if (alphastripe) {
					if (transwallcount < mapWidth) {
						transwallcount++;
					}
					alphastripe = false;
				}
				if (opposite) {
					if (mapX == 0 || mapX == mapWidth || mapY == 0 || mapY == mapHeight) {
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
		if (drawEnd < 0) drawEnd = h - 1; //becomes < 0 when the integer overflows
		//draw the floor from drawEnd to the bottom of the screen
		int drawdist = h;
		int expandeddraw = h >> 1;
		for (int y = drawEnd; y < drawdist + expandeddraw; y++) {
			//currentDist = h / (2.0 * y - h); //you could make a small lookup table for this instead
			currentDist = distTable[y];
			if (y > h - 1) {
				if (!heightMap) break;
				double weight = (currentDist - distPlayer) / (distWall - distPlayer);

				double currentFloorX = weight * floorXWall + (1.0 - weight) * posX;
				double currentFloorY = weight * floorYWall + (1.0 - weight) * posY;

				int floorTexX, floorTexY;
				int cmapX = (int)currentFloorX;
				if (cmapX > mapWidth - 1) cmapX = mapWidth - 1;
				if (cmapX < 0) cmapX = 0;
				int cmapY = (int)currentFloorY;
				if (cmapY > mapHeight - 1) cmapY = mapHeight - 1;
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

				if (heightMap && floorcolor > 0) {
					if (heightMap[cmapX][cmapY] - 1 > 0) {
						int raisedfloorstart = y - (int)(texture[heightMap[cmapX][cmapY] - 1][texpos] / currentDist);
						if (raisedfloorstart > h - 1) continue;
						if (raisedfloorstart < 0) raisedfloorstart = 0;
						for (int ny = raisedfloorstart; ny < y; ny++) {
							if (ny < h && (ZBuffer[x][ny] > currentDist || ZBuffer[x][ny] == 0)) {
								ZBuffer[x][ny] = currentDist; //perpendicular distance is used
								buffer[ny][x] = floorcolor;
								interactionmap [x * sWidth + ny] = 0;
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
				int cmapX = (int)currentFloorX % mapWidth;
				if (cmapX < 0) cmapX = 0;
				int cmapY = (int)currentFloorY % mapHeight;
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

				if (heightMap && floorcolor > 0 && (currentDist < ZBuffer[x][y] || ZBuffer[x][y] == 0)) {
					if (heightMap[cmapX][cmapY] - 1 > 0) {
						int raisedfloorstart = y - (int)(texture[heightMap[cmapX][cmapY] - 1][texWidth * floorTexY + floorTexX] / currentDist);
						if (raisedfloorstart > h - 1) continue;
						if (raisedfloorstart < 0) raisedfloorstart = 0;
						for (int ny = raisedfloorstart; ny < y; ny++) {
							if (ZBuffer[x][ny] > currentDist || ZBuffer[x][ny] == 0) {
								ZBuffer[x][ny] = currentDist; //perpendicular distance is used
								buffer[ny][x] = floorcolor;
								interactionmap [x * sWidth + ny] = 0;
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
					buffer[y][x] = floorcolor;
					editorMap [x][y] = ((short)cmapX) << 16 | ((short)cmapY);
				} else ZBuffer[x][y] = 9999999999999.0;
				if (currentDist < ZBuffer[x][h - y] || ZBuffer[x][h - y] == 0) {
					if (ceilingcolor > 0) {
						ZBuffer[x][h - y] = currentDist; //perpendicular distance is used
						buffer[h - y][x] = ceilingcolor;
					} else ZBuffer[x][h - y] = 999999999999.0;
					editorMap [x][h - y] = ((short)cmapX) << 16 | ((short)cmapY);
				}
				interactionmap [x * sWidth + y] = 0;
				interactionmap [x * sWidth + (h - y)] = 0;
				if ((int)cmapX == selectedX && (int)cmapY == selectedY) {
					if (floorTexX == 0 || floorTexX == 63 || floorTexY == 0 || floorTexY == 63) {
						buffer[y][x] = selectedColor;
						ZBuffer [x][y] = perpWallDist;
						buffer[h - y][x] = selectedColor;
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
						if (transwallblendmode[transwalldrawn] == 0) buffer[y][x] = Mix::MixColorAlpha(color, buffer[y][x], transalphabuffer[x][transwalloffset + y]); //paint pixel if it isn't black, black is the invisible color
						else if (transwallblendmode[transwalldrawn] == 1) buffer[y][x] = Mix::MixColorAdditive(color, buffer[y][x], transalphabuffer[x][transwalloffset + y]);
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
				int (*sfGetGameParameter)(int, int, int, int);
				sfGetGameParameter = ((int(*)(int, int, int, int)) engine->GetScriptFunctionAddress("GetGameParameter"));
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
		double spriteX = sprite[spriteOrder[i]].x - posX;
		double spriteY = sprite[spriteOrder[i]].y - posY;
		//double transformX = invDet * (dirY * spriteX - dirX * spriteY);
		//double transformY = invDet * (-planeY * spriteX + planeX * spriteY); //this is actually the depth inside the screen, that what Z is in 3D

		int spriteScreenX = int((w / 2) * (1 + spriteTransformX[i] / spriteTransformY[i]));

		//parameters for scaling and moving the sprites
		BITMAP *spritetexbm = engine->GetSpriteGraphic(sprite[spriteOrder[i]].texture);
		unsigned char **spritetex = engine->GetRawBitmapSurface(spritetexbm);
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
						unsigned char color = spritetex[texY][texX]; //get current color from the texture
						if (color != 0) {
							if (sprite[spriteOrder[i]].alpha < 255) {
								if (sprite[spriteOrder[i]].blendmode == 0) color = Mix::MixColorAlpha(color, buffer[y][stripe], sprite[spriteOrder[i]].alpha);
								if (sprite[spriteOrder[i]].blendmode == 1) color = Mix::MixColorAdditive(color, buffer[y][stripe], sprite[spriteOrder[i]].alpha);
							}
							color = Mix::MixColorLightLevel(color, spr_light);
							if (transzbuffer[stripe][transwalldraw * h + y] < spriteTransformY[i] && transzbuffer[stripe][transwalldraw * h + y] != 0 && transslicedrawn[stripe] && transcolorbuffer[stripe][transwalldraw * h + y] > 0 && transalphabuffer[stripe][transwalldraw * h + y] > 0) {
								if (transwallblendmode[transwalldraw] == 0) color = Mix::MixColorAlpha(color, transcolorbuffer[stripe][transwalldraw * h + y], transalphabuffer[stripe][transwalldraw * h + y]);
								else if (transwallblendmode[transwalldraw] == 1) color = Mix::MixColorAdditive(color, transcolorbuffer[stripe][transwalldraw * h + y], transalphabuffer[stripe][transwalldraw * h + y]);
								buffer[y][stripe] = color;
								ZBuffer[stripe][y] = transzbuffer[stripe][transwalldraw * h + y];
							} else {
								buffer[y][stripe] = color; //paint pixel if it isn't black, black is the invisible color
								ZBuffer[stripe][y] = spriteTransformY[i]; //put the sprite on the zbuffer so we can draw around it.
							}
							interactionmap [stripe * sWidth + y] = sprite[spriteOrder[i]].objectinteract << 8;
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

void QuitCleanup() {
	if (!rendering) {
		for (int i = 0; i < sWidth; ++i) {
			if (transcolorbuffer[i])delete [] transcolorbuffer[i];
			if (transalphabuffer[i])delete [] transalphabuffer[i];
			if (transzbuffer[i])delete [] transzbuffer[i];
			if (ZBuffer[i]) delete [] ZBuffer[i];
		}
		if (transcolorbuffer) delete [] transcolorbuffer;
		if (transalphabuffer) delete [] transalphabuffer;
		if (transzbuffer) delete [] transzbuffer;
		if (ZBuffer) delete [] ZBuffer;
		if (transwallblendmode) delete [] transwallblendmode;
		if (interactionmap) delete [] interactionmap;
	}
}

void MoveForward() {
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
		if (wallData[worldMap[int(newposx)][int(posY)]].solid[texsidex] == false && wallData[worldMap[int(posX)][int(posY)]].solid[inside_texsidex] == false && int(newposx) > -1 && int(newposx) < mapWidth) posX += dirX * moveSpeed;
		if (wallData[worldMap[int(posX)][int(newposy)]].solid[texsidey] == false && wallData[worldMap[int(posX)][int(posY)]].solid[inside_texsidey] == false && int(newposy) > -1 && int(newposy) < mapHeight) posY += dirY * moveSpeed;
	} else if (!noclip && inside) {
		posX += dirX * moveSpeed;
		posY += dirY * moveSpeed;
	} else {
		if (int(newposx) > -1 && int(newposx) < mapWidth) posX += dirX * moveSpeed;
		if (int(newposy) > -1 && int(newposy) < mapHeight) posY += dirY * moveSpeed;
	}
}

void MoveBackward() {
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
		if (wallData[worldMap[int(newposx)][int(posY)]].solid[texsidex] == false && wallData[worldMap[int(posX)][int(posY)]].solid[inside_texsidex] == false && int(newposx) > -1 && int(newposx) < mapWidth) posX -= dirX * moveSpeed;
		if (wallData[worldMap[int(posX)][int(newposy)]].solid[texsidey] == false && wallData[worldMap[int(posX)][int(posY)]].solid[inside_texsidey] == false && int(newposy) > -1 && int(newposy) < mapHeight) posY -= dirY * moveSpeed;
	} else if (!noclip && inside) {
		posX -= dirX * moveSpeed;
		posY -= dirY * moveSpeed;
	} else {
		if (int(newposx) > -1 && int(newposx) < mapWidth) posX -= dirX * moveSpeed;
		if (int(newposy) > -1 && int(newposy) < mapHeight) posY -= dirY * moveSpeed;
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

void RotateLeft() {
	double oldDirX = dirX;
	dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
	dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
	double oldPlaneX = planeX;
	planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
	planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
}

void RotateRight() {
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

} // namespace AGSBlend
} // namespace Plugins
} // namespace AGS3

