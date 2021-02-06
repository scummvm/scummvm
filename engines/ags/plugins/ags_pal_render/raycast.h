#ifndef __PALGORITHMS_RAYCAST_H
#define __PALGORITHMS_RAYCAST_H

#include "palrender.h"

#define mapWidth 64
#define mapHeight 64

#if defined(BUILTIN_PLUGINS)
namespace agspalrender {
#endif

struct Sprite {
	double x;
	double y;
	int texture;
	unsigned char alpha;
	int blendmode;
	double uDivW;
	double uDivH;
	double vMove;
	double hMove;
	char objectinteract;
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
	unsigned char hotspotinteract;
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
extern unsigned char worldMap[64][64];
extern unsigned char lightMap[64][64];
extern int ceilingMap[64][64];
extern int floorMap[64][64];
extern int heightMap[64][64];
extern unsigned char seenMap[64][64];
//extern int mapWidth;
//extern int mapHeight;
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



void MakeTextures(int slot);
void Raycast_Render(int slot);
void MoveForward();
void MoveBackward();
void RotateLeft();
void RotateRight();
void Init_Raycaster();
void QuitCleanup();
void LoadMap(int worldmapSlot, int lightmapSlot, int ceilingmapSlot, int floormapSlot);
void Ray_InitSprite(int id, SCRIPT_FLOAT(x), SCRIPT_FLOAT(y), int slot, unsigned char alpha, int blendmode, SCRIPT_FLOAT(scale_x), SCRIPT_FLOAT(scale_y), SCRIPT_FLOAT(vMove));
void Ray_SetPlayerPosition(SCRIPT_FLOAT(x), SCRIPT_FLOAT(y));
FLOAT_RETURN_TYPE Ray_GetPlayerX();
FLOAT_RETURN_TYPE Ray_GetPlayerY();
int Ray_GetPlayerAngle();
void Ray_SetPlayerAngle(int angle);

int Ray_GetWallHotspot(int id);
int Ray_GetWallTexture(int id, int dir);
int Ray_GetWallSolid(int id, int dir);
int Ray_GetWallIgnoreLighting(int id, int dir);
int Ray_GetWallAlpha(int id, int dir);
int Ray_GetWallBlendType(int id, int dir);

void Ray_SelectTile(int x, int y, unsigned char color);

int Ray_GetHotspotAt(int x, int y);
int Ray_GetObjectAt(int x, int y);

void Ray_DrawTile(int spr, int tile);
void Ray_DrawOntoTile(int spr, int tile);
void Ray_SetNoClip(int value);
int Ray_GetNoClip();
void Ray_SetSpriteInteractObj(int id, int obj);
int  Ray_GetSpriteInteractObj(int id);
void Ray_SetSpritePosition(int id, SCRIPT_FLOAT(x), SCRIPT_FLOAT(y));
void Ray_SetSpriteVertOffset(int id, SCRIPT_FLOAT(vMove));
FLOAT_RETURN_TYPE Ray_GetSpriteVertOffset(int id);
FLOAT_RETURN_TYPE Ray_GetSpriteX(int id);
FLOAT_RETURN_TYPE Ray_GetSpriteY(int id);

void Ray_SetWallHotspot(int id, char hotsp);
void Ray_SetWallTextures(int id, int n, int s, int w, int e);
void Ray_SetWallSolid(int id, int n, int s, int w, int e);
void Ray_SetWallIgnoreLighting(int id, int n, int s, int w, int e);
void Ray_SetWallAlpha(int id, int n, int s, int w, int e);
void Ray_SetWallBlendType(int id, int n, int s, int w, int e);

FLOAT_RETURN_TYPE Ray_GetMoveSpeed();
void Ray_SetMoveSpeed(SCRIPT_FLOAT(speed));
FLOAT_RETURN_TYPE Ray_GetRotSpeed();
void Ray_SetRotSpeed(SCRIPT_FLOAT(speed));
int Ray_GetWallAt(int x, int y);
int Ray_GetLightAt(int x, int y);
void Ray_SetLightAt(int x, int y, int light);
void Ray_SetWallAt(int x, int y, int id);
void Ray_SetPlaneY(SCRIPT_FLOAT(y));
FLOAT_RETURN_TYPE Ray_GetDistanceAt(int x, int y);
int Ray_GetSpriteAngle(int id);
void Ray_SetSpriteAngle(int id, int angle);
void Ray_SetSpriteView(int id, int view);
int Ray_GetSpriteView(int id);
void Ray_SetSpriteFrame(int id, int frame);
int Ray_GetSpriteFrame(int id);

int Ray_GetTileX_At(int x, int y);
int Ray_GetTileY_At(int x, int y);

void Ray_SetSkyBox(int slot);
int Ray_GetSkyBox(int slot);

void Ray_SetAmbientLight(int value);
int Ray_GetAmbientLight();
void Ray_SetAmbientColor(int color, int amount);


int Ray_GetSpriteAlpha(int id);
void Ray_SetSpriteAlpha(int id, int alpha);
int Ray_GetSpritePic(int id);
void Ray_SetSpritePic(int id, int slot);

FLOAT_RETURN_TYPE Ray_GetSpriteScaleX(int id);
void Ray_SetSpriteScaleX(int id, SCRIPT_FLOAT(scale));
FLOAT_RETURN_TYPE Ray_GetSpriteScaleY(int id);
void Ray_SetSpriteScaleY(int id, SCRIPT_FLOAT(scale));

void Ray_SetSpriteBlendType(int id, int type);
int Ray_GetSpriteBlendType(int id);

void Ray_SetFloorAt(int x, int y, int tex);
void Ray_SetCeilingAt(int x, int y, int tex);
int Ray_GetCeilingAt(int x, int y);
int Ray_GetFloorAt(int x, int y);
int Ray_GetLightingAt(int x, int y);
void Ray_SetLightingAt(int x, int y, unsigned char lighting);
int Ray_GetAmbientWeight();

int Ray_HasSeenTile(int x, int y);

#if defined(BUILTIN_PLUGINS)
} // namespace agspalrender
#endif

#endif