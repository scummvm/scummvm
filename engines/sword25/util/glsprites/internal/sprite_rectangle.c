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
#include "sprite.h"
#include "sprite_rectangle.h"

/* -------------------------------------------------------------------------- */

static GLS_Result Delete(GLS_Sprite *);
static GLS_Result Blit(GLS_Sprite *, const GLS_Rect *);
static GLS_Result SetData(GLS_Sprite *, GLS_UInt32, GLS_UInt32, const void *, GLS_UInt32);
static GLS_spriteFunctionTable rectangleFunctionTable = { ST_RECTANGLE, Delete, Blit, SetData };

/* -------------------------------------------------------------------------- */

typedef struct
{
	/* These first three members have to be present at the start of each sprite type. */
	GLS_spriteFunctionTable * functionTable;
	GLS_UInt32		width;
	GLS_UInt32		height;

	GLint			textureID;
} spriteRectangle;


/* -------------------------------------------------------------------------- */

GLS_Result GLS_NewSpriteRectangle(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool useAlphachannel, void * data, GLS_Sprite ** pSprite)
{
	/* Allocate memory for sprite object */
	spriteRectangle * sprite = (spriteRectangle *) malloc(sizeof(spriteRectangle));
	if (!sprite) return GLS_OUT_OF_MEMORY;

	/* Initialize sprite object */
	sprite->functionTable = &rectangleFunctionTable;
	sprite->textureID = 0;
	sprite->width = width;
	sprite->height = height;

	/* Create OpenGL texture */
	glGenTextures(1, &(sprite->textureID));
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, sprite->textureID);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, useAlphachannel ? GL_RGBA8 : GL_RGB8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	*pSprite = (GLS_Sprite) sprite;

	/* The unbind here is not really necessary but some GL drivers (e.g. GeForce 4 488 Go) screw up subsequent GL_TEXTURE_2D
	   render operations if a GL_TEXTURE_RECTANGLE_ARB texture is still bound. */
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

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
	spriteRectangle * s = (spriteRectangle *) sprite;

	if (s->textureID) glDeleteTextures(1, &s->textureID);
	free(s);

	return GLS_OK;
}


/* -------------------------------------------------------------------------- */

static GLS_Result Blit(GLS_Sprite * sprite,
					   const GLS_Rect * subImage)
{
	GLfloat width;
	GLfloat height;

	spriteRectangle * s = (spriteRectangle *) sprite;

	width = (float)(subImage->x2 - subImage->x1);
	height = (float)(subImage->y2 - subImage->y1);
	
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, s->textureID);

	glEnable(GL_TEXTURE_RECTANGLE_ARB);

	glBegin(GL_QUADS);
		glTexCoord2i(subImage->x1, subImage->y1);
		glVertex2f(0.0f, 0.0f);

		glTexCoord2i(subImage->x2, subImage->y1);
		glVertex2f(width, 0.0f);

		glTexCoord2i(subImage->x2, subImage->y2);
		glVertex2f(width, height);

		glTexCoord2i(subImage->x1, subImage->y2);
		glVertex2f(0.0f, height);
	glEnd();

	/* The unbind here is not really necessary but some GL drivers (e.g. GeForce 4 488 Go) screw up subsequent GL_TEXTURE_2D
	   render operations if a GL_TEXTURE_RECTANGLE_ARB texture is still bound. */
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

	glDisable(GL_TEXTURE_RECTANGLE_ARB);

	return glGetError() ? GLS_OPENGL_ERROR : GLS_OK;
}


/* -------------------------------------------------------------------------- */

static GLS_Result SetData(GLS_Sprite * sprite, GLS_UInt32 width, GLS_UInt32 height, const void * data, GLS_UInt32 stride)
{
	spriteRectangle * s = (spriteRectangle *) sprite;
	
	glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, width + stride);

	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, s->textureID);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

	/* The unbind here is not really necessary but some GL drivers (e.g. GeForce 4 488 Go) screw up subsequent GL_TEXTURE_2D
	   render operations if a GL_TEXTURE_RECTANGLE_ARB texture is still bound. */
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

	glPopClientAttrib();

	return glGetError() ? GLS_OPENGL_ERROR : GLS_OK;
}
