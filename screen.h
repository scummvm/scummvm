// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <SDL_opengl.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SCREEN_BLOCK_WIDTH 16
#define SCREEN_BLOCK_HEIGHT 16

#define SCREEN_BLOCK_SIZE (SCREEN_BLOCK_WIDTH*SCREEN_BLOCK_HEIGHT)

// Yaz: warning, SCREEN_WIDTH must be divisible by SCREEN_BLOCK_WIDTH and SCREEN_HEIGHT by SCREEN_BLOCK_HEIGH
// maybe we should check it with the precompiler...
#define NUM_SCREEN_BLOCK_WIDTH (SCREEN_WIDTH / SCREEN_BLOCK_WIDTH)
#define NUM_SCREEN_BLOCK_HEIGHT (SCREEN_HEIGHT / SCREEN_BLOCK_HEIGHT)

#define NUM_SCREEN_BLOCKS (NUM_SCREEN_BLOCK_WIDTH * NUM_SCREEN_BLOCK_HEIGHT)

struct screenBlockDataStruct {
	bool isDirty;
	float depth;
};

void screenBlocksReset();
void screenBlocksInit(char* zbuffer);
void screenBlocksInitEmpty();
void screenBlocksAddRectangle( int top, int right, int left, int bottom, float depth );
void screenBlocksDrawDebug();
void screenBlocksBlitDirtyBlocks();

#endif // _SCREEN_H_

