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
#include "util.h"
#include "core.h"
#include "sprite.h"
#include "sprite_rectangle.h"
#include "sprite_pow2.h"
#include "sprite_tiled.h"

/* -------------------------------------------------------------------------- */

typedef struct  
{
	/* These first three members are present at the start of each sprite type.
	   Therefore it is safe to cast all sprite to this. */
	GLS_spriteFunctionTable * functionTable;
	GLS_UInt32		width;
	GLS_UInt32		height;
} spriteBase;

/* -------------------------------------------------------------------------- */

#define MAX_WASTED_TEXTURE_MEMORY_PERCENT 30

/* -------------------------------------------------------------------------- */

GLS_Result GLS_NewSprite(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool useAlphachannel, void * data, GLS_Sprite ** pSprite)
{
	//* The sprite needs to be at least 1x1 pixel in size */
	if (width == 0 || height == 0) return GLS_INVALID_SPRITE_DIMENSIONS;

	/* If both dimensions are powers of 2 a power of 2 texture will be used for the sprite */
	if (GLS_IsPowerOf2(width) && GLS_IsPowerOf2(height)) return GLS_NewSpritePow2(width, height, useAlphachannel, data, pSprite);

	/* Use TEXTURE_RECTANGLE_ARB for non power of 2 dimensions whenever possible */
	if (GLS_TheOGLCaps.textureRectanglesSupported &&
		GLS_TheOGLCaps.maxTextureRectangleSize >= width &&
		GLS_TheOGLCaps.maxTextureRectangleSize >= height)
		return GLS_NewSpriteRectangle(width, height, useAlphachannel, data, pSprite);

	/* Use a power of 2 texture if TEXTURE_RECTANGLE_ARB is not possible, the image is small enough and the amount of wasted texture memory 
	   is not too big */
	if ((width <= GLS_TheOGLCaps.maxTextureSize && height <= GLS_TheOGLCaps.maxTextureSize) &&
		(GLS_NextPowerOf2(width) * GLS_NextPowerOf2(height) * 100) / (width * height) <= MAX_WASTED_TEXTURE_MEMORY_PERCENT)
		return GLS_NewSpritePow2(width, height, useAlphachannel, data, pSprite);

	/* Otherwise use tiled power of 2 textures */
	return GLS_NewSpriteTiled(width, height, useAlphachannel, data, pSprite);
}


/* -------------------------------------------------------------------------- */

GLS_Result GLS_DeleteSprite(GLS_Sprite * sprite)
{
	/* Poor mans polymorphism. */
	spriteBase * s = (spriteBase *) sprite;
	return s->functionTable->Delete(sprite);
}


/* -------------------------------------------------------------------------- */

GLS_Result GLS_Blit(GLS_Sprite * sprite,
					GLS_SInt32 x, GLS_SInt32 y,
					const GLS_Rect * subImage,
					const GLS_Color * color,
					GLS_Bool flipH, GLS_Bool flipV,
					GLS_Float scaleX, GLS_Float scaleY)
{
	spriteBase * s = (spriteBase *) sprite;
	GLS_Rect localSubImage;
	const GLS_Rect * subImagePtr;
	GLS_Result result;

	/* Check parameters for validity. */
	if (subImage && (subImage->x2 > s->width || subImage->y2 > s->height)) return GLS_INVALID_SUB_IMAGE;

	/* If no subimage was supplied, create one that covers the whole sprite. */
	if (subImage)
		subImagePtr = subImage;
	else
	{
		localSubImage.x1 = 0;
		localSubImage.y1 = 0;
		localSubImage.x2 = s->width;
		localSubImage.y2 = s->height;
		subImagePtr = &localSubImage;
	}

	/* Modify the Modelview-Matrix to reflect the sprites position, flip settings, and scale. */
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef((GLfloat) x, (GLfloat) y, 0);
	if (flipH == GLS_True || flipV == GLS_True || scaleX != 1.0f || scaleY != 1.0f)
	{
		glScalef(scaleX * (flipH == GLS_True ? -1.0f : 1.0f), scaleY * (flipV == GLS_True ? -1.0f : 1.0f), 1.0f);
		glTranslatef(flipH == GLS_True ? (subImagePtr->x2 - subImagePtr->x1) * -1.0f : 0.0f,
					 flipV == GLS_True ? (subImagePtr->y2 - subImagePtr->y1) * -1.0f : 0.0f, 0.0f);
	}

	/* Set the color to be used. Use white if no color was provided. */
	if (color)
		glColor4ub(color->r, color->g, color->b, color->a);
	else
		glColor4ub(255, 255, 255, 255);

	/* Display the sprite by calling the Blit() function of the specific sprite type. */
	result = s->functionTable->Blit(sprite, subImagePtr);

	/* Restore the old matrix. */
	glPopMatrix();

	return result;
}


/* -------------------------------------------------------------------------- */

GLS_Result GLS_SetSpriteData(GLS_Sprite sprite, GLS_UInt32 width, GLS_UInt32 height, const void * data, GLS_UInt32 stride)
{
	spriteBase * s = (spriteBase *) sprite;

	/* Check parameters for validity. */
	if (width == 0 || height == 0 || width > s->width || height > s->height) return GLS_INVALID_DATA_DIMENSIONS;
	if (data == 0) return GLS_INVALID_DATA_POINTER;

	/* Poor mans polymorphism. */
	return s->functionTable->SetData(sprite, width, height, data, stride);
}
