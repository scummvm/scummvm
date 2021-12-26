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

} // namespace AGSPalRender
} // namespace Plugins
} // namespace AGS3

#endif
