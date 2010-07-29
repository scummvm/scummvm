/******************************************************************************/
/* This file is part of Broken Sword 2.5                                      */
/* Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer     */
/*                                                                            */
/* Broken Sword 2.5 is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* Broken Sword 2.5 is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with Broken Sword 2.5; if not, write to the Free Software            */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA   */
/******************************************************************************/

/* --------------------------------------------------------------------------
   INCLUDES
   -------------------------------------------------------------------------- */

#include <malloc.h>

#include "glinclude.h"
#include "core.h"
#include "util.h"
#include "sprite.h"
#include "sprite_tiled.h"

/* -------------------------------------------------------------------------- */

static GLS_Result Delete(GLS_Sprite *);
static GLS_Result Blit(GLS_Sprite *, const GLS_Rect *);
static GLS_Result SetData(GLS_Sprite *, GLS_UInt32, GLS_UInt32, const void *, GLS_UInt32);
static GLS_spriteFunctionTable tiledFunctionTable = { ST_TILED, Delete, Blit, SetData };

/* -------------------------------------------------------------------------- */

#define MIN_TEXTURE_SIZE_LOG2 6
#define MIN_TEXTURE_SIZE (1 << MIN_TEXTURE_SIZE_LOG2)

#define MAX_POSSIBLE_TEXTURE_SIZE_LOG2 31
#define MAX_POSSIBLE_TEXTURE_SIZE (1 << MAX_POSSIBLE_TEXTURE_SIZE_LOG2)

/* -------------------------------------------------------------------------- */

typedef struct
{
	GLint	textureID;
	GLfloat	x;
	GLfloat	y;
	GLfloat	width;
	GLfloat	height;
} tile;

/* -------------------------------------------------------------------------- */

typedef struct
{
	/* These first three members have to be present at the start of each sprite type. */
	GLS_spriteFunctionTable * functionTable;
	GLS_UInt32	width;
	GLS_UInt32	height;

	GLS_UInt32	widthSubdivisionCount;
	GLS_UInt32	heightSubdivisionCount;
	tile		tiles[1]; /* Has to be last member. */
} spriteTiled;

/* -------------------------------------------------------------------------- */

static GLS_UInt32 CalculateSubdivisions(GLS_UInt32 value, GLS_UInt32 maxTextureSize, GLS_UInt32 maxTextureSizeLog2,
										GLS_UInt32 subdivisions[MAX_POSSIBLE_TEXTURE_SIZE_LOG2 + 1])
{
	/* This function takes a value and determines how it best can be divided up into pieces of power of 2 length.
	   The resulting parts are at least MIN_TEXTURE_SIZE long and at most maxTextureSize long.
	   If we do this for both width and height of a sprite, we can determine how this sprite can be split up into
	   smaller power of 2 textures using a simple two dimensional loop.

	   This returns the number of subdivisions as the return value.
	   The sizes of the different pieces are encoded in the subdivisions array. The indicies into the array determine the
	   power of 2 and the values in the array the number of pieces with that size.
	   E.g. if the array contains the value 3 at index 4 this means you have 3 pieces of size 16 (2^4 = 16).
	*/

	GLS_UInt32 subdivisionCount = 0;
	GLS_UInt32 maxSizeCount;
	GLS_UInt32 counter = 0;

	/* First determine how many pieces of maximal size we can fill and write this into the result array.
	   Update the value and the subdivision count accordingly. */
	maxSizeCount = value / maxTextureSize;
	subdivisions[maxTextureSizeLog2] = maxSizeCount;
	subdivisionCount += maxSizeCount;
	value -= maxSizeCount * maxTextureSize;

	/* The remaining value is rounded up to a multiple of the minimal texture size. */
	value = ((value + MIN_TEXTURE_SIZE - 1) / MIN_TEXTURE_SIZE) * MIN_TEXTURE_SIZE;

	/* Determine the remaining pieces by using the binary representation of the value.
	   If a bit is set, we need to add a piece of the according size. */
	while (value)
	{
		if (value & 1)
		{
			++subdivisions[counter];
			++subdivisionCount;
		}

		++counter;
		value >>= 1;
	}

	return subdivisionCount;
}

/* -------------------------------------------------------------------------- */

GLS_Result GLS_NewSpriteTiled(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool useAlphachannel, void * data, GLS_Sprite ** pSprite)
{
	GLS_UInt32 maxTextureSize, maxTextureSizeLog2;
	GLS_UInt32 widthSubdivisions[MAX_POSSIBLE_TEXTURE_SIZE_LOG2 + 1] = { 0 };
	GLS_UInt32 widthSubdivisionCount;
	GLS_UInt32 heightSubdivisions[MAX_POSSIBLE_TEXTURE_SIZE_LOG2 + 1] = { 0 };
	GLS_UInt32 heightSubdivisionCount;
	GLS_UInt32 subdivisonX, subdivisionY, pixelX = 0, pixelY = 0, i;
	spriteTiled * sprite;
	tile * curTile;

	/* Determine the maximal texture size to be used and make sure it is a power of two. */
	maxTextureSize = GLS_TheOGLCaps.maxTextureSize > MAX_POSSIBLE_TEXTURE_SIZE ? MAX_POSSIBLE_TEXTURE_SIZE : GLS_TheOGLCaps.maxTextureSize;
	maxTextureSize = GLS_IsPowerOf2(maxTextureSize) ? maxTextureSize : GLS_NextPowerOf2(maxTextureSize >> 1);

	/* Determine the log2 of the maximal texture size. */
	maxTextureSizeLog2 = GLS_Log2(maxTextureSize);

	/* Determine the subdivisions along the width and the height. */
	widthSubdivisionCount = CalculateSubdivisions(width, maxTextureSize, maxTextureSizeLog2, widthSubdivisions);
	heightSubdivisionCount = CalculateSubdivisions(height, maxTextureSize, maxTextureSizeLog2, heightSubdivisions);

	/* Allocate memory for sprite object */
	sprite = (spriteTiled *) calloc(sizeof(spriteTiled) + sizeof(tile) * (widthSubdivisionCount * heightSubdivisionCount - 1), 1);
	if (!sprite) return GLS_OUT_OF_MEMORY;

	/* Initialize sprite object. */
	sprite->functionTable = &tiledFunctionTable;
	sprite->width = width;
	sprite->height = height;
	sprite->widthSubdivisionCount = widthSubdivisionCount;
	sprite->heightSubdivisionCount = heightSubdivisionCount;

	/* Create the textures. */
	curTile = &sprite->tiles[0];
	for (subdivisionY = MAX_POSSIBLE_TEXTURE_SIZE_LOG2;; --subdivisionY)
	{
		while (heightSubdivisions[subdivisionY]--)
		{
			for (subdivisonX = MAX_POSSIBLE_TEXTURE_SIZE_LOG2;; --subdivisonX)
			{
				for (i = 0; i < widthSubdivisions[subdivisonX]; ++i)
				{
					/* Initialize tile. Store as GLfloat to avoid unnecessary integer conversions when drawing. */
					curTile->x = (GLfloat) pixelX;
					curTile->y = (GLfloat) pixelY;
					curTile->width = (GLfloat) (1 << subdivisonX);
					curTile->height = (GLfloat) (1 << subdivisionY);

					/* Create OpenGL texture */
					glGenTextures(1, &(curTile->textureID));
					glBindTexture(GL_TEXTURE_2D, curTile->textureID);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

					glTexImage2D(GL_TEXTURE_2D, 0, useAlphachannel ? GL_RGBA8 : GL_RGB8, 1 << subdivisonX, 1 << subdivisionY, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

					/* Advance to the next empty tile. */
					pixelX += 1 << subdivisonX;
					++curTile;
				}

				/* Test for loop end. Prevents underflow of unsigned integer. */
				if (subdivisonX == 0) break;
			}

			/* Enter next row. */
			pixelX = 0;
			pixelY += 1 << subdivisionY;
		}

		/* Test for loop end. Prevents underflow of unsigned integer. */
		if (subdivisionY == 0) break;
	}

	*pSprite = (GLS_Sprite) sprite;

	/* Intialize the sprite with the supplied pixel data (if any). */
	if (data) SetData(*pSprite, width, height, data, 0);

	/* If an error occured the sprite is deleted and the function returns with an error. */
	if (glGetError())
	{
		Delete(*pSprite);
		return GLS_TEXTURE_CREATION_FAILED;
	}

	return GLS_OK;
}


/* -------------------------------------------------------------------------- */

static GLS_Result Delete(GLS_Sprite * sprite)
{
	spriteTiled * s = (spriteTiled *) sprite;
	GLS_UInt32 tileCount = s->widthSubdivisionCount * s->heightSubdivisionCount;
	GLS_UInt32 i;

	/* Delete each texture. */
	for (i = 0; i < tileCount; ++i)
	{
		if (s->tiles[i].textureID) glDeleteTextures(1, &s->tiles[i].textureID);
	}

	/* Free the memory used by the sprite. */
	free(s);

	return GLS_OK;
}


/* -------------------------------------------------------------------------- */

static GLS_Result Blit(GLS_Sprite * sprite,
					   const GLS_Rect * subImage)
{
	GLfloat width, height;
	GLfloat texX1, texY1, texX2, texY2;
	GLS_UInt32 tileIndex = 0;
	GLS_UInt32 tileX, tileY;
	GLfloat tileRectX1, tileRectY1;
	GLfloat tileRectX2, tileRectY2;
	GLfloat tileWidth, tileHeight;
	GLfloat lastHeight = 0;
	GLS_GLfloatRect tileRect, resultRect;
	GLfloat x = 0.0f, y = 0.0f;
	GLS_GLfloatRect subImageF = { (GLfloat) subImage->x1, (GLfloat) subImage->y1, (GLfloat) subImage->x2, (GLfloat) subImage->y2 };

	spriteTiled * s = (spriteTiled *) sprite;

	/* Determine width and height of the subimage to display. */
	width = (float)(subImage->x2 - subImage->x1);
	height = (float)(subImage->y2 - subImage->y1);

	glEnable(GL_TEXTURE_2D);

	/* Draw all required tiles. */
	for (tileY = 0; tileY < s->heightSubdivisionCount; ++tileY)
	{
		for (tileX = 0; tileX < s->widthSubdivisionCount; ++tileX)
		{
			tileRect.x1 = s->tiles[tileIndex].x;	
			tileRect.y1 = s->tiles[tileIndex].y;
			tileRect.x2 = tileRect.x1 + s->tiles[tileIndex].width;
			tileRect.y2 = tileRect.y1 + s->tiles[tileIndex].height;

			if (GLS_IntersectGLfloatRects(&tileRect, &subImageF, &resultRect))
			{
				tileRectX1 = resultRect.x1;
				tileRectY1 = resultRect.y1;
				tileRectX2 = resultRect.x2;
				tileRectY2 = resultRect.y2;

				glBindTexture(GL_TEXTURE_2D, s->tiles[tileIndex].textureID);

				texX1 = (tileRectX1 - s->tiles[tileIndex].x) / (GLfloat) s->tiles[tileIndex].width;
				texY1 = (tileRectY1 - s->tiles[tileIndex].y) / (GLfloat) s->tiles[tileIndex].height;
				texX2 = (GLfloat) (tileRectX2 - s->tiles[tileIndex].x) / (GLfloat) s->tiles[tileIndex].width;
				texY2 = (GLfloat) (tileRectY2 - s->tiles[tileIndex].y) / (GLfloat) s->tiles[tileIndex].height;

				tileWidth = resultRect.x2 - resultRect.x1;
				tileHeight = resultRect.y2 - resultRect.y1;

				glBegin(GL_QUADS);
					glTexCoord2f(texX1, texY1);
					glVertex2f(x, y);

					glTexCoord2f(texX2, texY1);
					glVertex2f(x + tileWidth, y);

					glTexCoord2f(texX2, texY2);
					glVertex2f(x + tileWidth, y + tileHeight);

					glTexCoord2f(texX1, texY2);
					glVertex2f(x, y + tileHeight);
				glEnd();

				x += tileWidth;
				lastHeight = tileHeight;
			}
			
			++tileIndex;
		}

		x = 0;
		y += lastHeight;
	}

	glDisable(GL_TEXTURE_2D);

	return glGetError() ? GLS_OPENGL_ERROR : GLS_OK;
}


/* -------------------------------------------------------------------------- */

static GLS_Result SetData(GLS_Sprite * sprite, GLS_UInt32 width, GLS_UInt32 height, const void * data, GLS_UInt32 stride)
{
	spriteTiled * s = (spriteTiled *) sprite;
	GLS_UInt32 x = 0, y = 0, tileIndex = 0;
	GLS_UInt32 tileX, tileY;
	GLS_UInt32 copyWidth, copyHeight;
	const GLS_UInt32 * pixelData = (const GLS_UInt32 *) data;

	/* Set the source image row length and push the original value that it can be restored later. */
	glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width + stride);

	/* Loop through all tiles width first. */
	for (tileY = 0; tileY < s->heightSubdivisionCount; ++tileY)
	{
		/* Is there enough pixel data to reach the next tile? */
		if (height > y)
		{
			for (tileX = 0; tileX < s->widthSubdivisionCount; ++tileX)
			{
				/* Is the enough pixel data to reach the next tile? */
				if (width > x)
				{
					/* Determine the amount of pixels to be copied to the current tile.
					   They might either cover the whole tile, or just part of the tile if not enough
					   pixel data is left. */
					copyWidth = GLS_MIN(width - x, (GLS_UInt32) s->tiles[tileIndex].width);
					copyHeight = GLS_MIN(height - y, (GLS_UInt32) s->tiles[tileIndex].height);

					/* Copy the pixels. */
					glBindTexture(GL_TEXTURE_2D, s->tiles[tileIndex].textureID);

					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, copyWidth, copyHeight, GL_RGBA, GL_UNSIGNED_BYTE,
									&pixelData[x + y * (width + stride)]);
				}

				/* Update the x position inside the pixel data and advance to the next tile. */
				x += (GLS_UInt32) s->tiles[tileIndex].width;
				++tileIndex;
			}
		}

		/* Restet the x position inside the pixel data (we have reached another row) and update the y position. */
		x = 0;
		y += (GLS_UInt32) s->tiles[tileIndex - 1].height;
	}

	glPopClientAttrib();

	return glGetError() ? GLS_OPENGL_ERROR : GLS_OK;
}
