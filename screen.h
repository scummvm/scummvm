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

struct screenBlockDataStruct
{
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

