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
#include "sprite.h"
#include "sprite_pow2.h"

/* -------------------------------------------------------------------------- */

static GLS_Result Delete(GLS_Sprite *);
static GLS_Result Blit(GLS_Sprite *, const GLS_Rect *);
static GLS_Result SetData(GLS_Sprite *, GLS_UInt32, GLS_UInt32, const void *, GLS_UInt32);
static GLS_spriteFunctionTable pow2FunctionTable = { ST_POW2, Delete, Blit, SetData };

/* -------------------------------------------------------------------------- */

typedef struct
{
	/* These first three members have to be present at the start of each sprite type. */
	GLS_spriteFunctionTable * functionTable;
	GLS_UInt32		width;
	GLS_UInt32		height;

	GLS_UInt32		widthPow2;
	GLS_UInt32		heightPow2;
	GLint			textureID;
} spritePow2;


/* -------------------------------------------------------------------------- */

GLS_Result GLS_NewSpritePow2(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool useAlphachannel, void * data, GLS_Sprite ** pSprite)
{
	GLS_UInt32 widthPow2 = GLS_NextPowerOf2(width);
	GLS_UInt32 heightPow2 = GLS_NextPowerOf2(height);

	/* Allocate memory for sprite object */
	spritePow2 * sprite = (spritePow2 *) malloc(sizeof(spritePow2));
	if (!sprite) return GLS_OUT_OF_MEMORY;

	/* Initialize sprite object */
	sprite->functionTable = &pow2FunctionTable;
	sprite->textureID = 0;
	sprite->width = width;
	sprite->height = height;
	sprite->widthPow2 = widthPow2;
	sprite->heightPow2 = heightPow2;

	/* Create OpenGL texture */
	glGenTextures(1, &(sprite->textureID));
	glBindTexture(GL_TEXTURE_2D, sprite->textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, useAlphachannel ? GL_RGBA8 : GL_RGB8, widthPow2, heightPow2, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	/* Fill the texture with the supplied pixel data.
	   We don't use glTexImage2D for this, because the created sprite might not have power of 2 dimensions and therefore the
	   supplied pixeldata wouldn't cover the whole texture. */
	if (data) glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

	*pSprite = (GLS_Sprite) sprite;

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
	spritePow2 * s = (spritePow2 *) sprite;

	if (s->textureID) glDeleteTextures(1, &s->textureID);
	free(s);

	return GLS_OK;
}


/* -------------------------------------------------------------------------- */

static GLS_Result Blit(GLS_Sprite * sprite,
					   const GLS_Rect * subImage)
{
	GLfloat width, height;
	GLfloat texX1, texY1, texX2, texY2;
	
	spritePow2 * s = (spritePow2 *) sprite;

	width = (float) (subImage->x2 - subImage->x1);
	height = (float) (subImage->y2 - subImage->y1);

	texX1 = (GLfloat) subImage->x1 / (GLfloat) s->widthPow2;
	texY1 = (GLfloat) subImage->y1 / (GLfloat) s->heightPow2;
	texX2 = (GLfloat) subImage->x2 / (GLfloat) s->widthPow2;
	texY2 = (GLfloat) subImage->y2 / (GLfloat) s->heightPow2;

	glBindTexture(GL_TEXTURE_2D, s->textureID);

	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
		glTexCoord2f(texX1, texY1);
		glVertex2f(0.0f, 0.0f);

		glTexCoord2f(texX2, texY1);
		glVertex2f(width, 0.0f);

		glTexCoord2f(texX2, texY2);
		glVertex2f(width, height);

		glTexCoord2f(texX1, texY2);
		glVertex2f(0.0f, height);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	return glGetError() ? GLS_OPENGL_ERROR : GLS_OK;
}


/* -------------------------------------------------------------------------- */

static GLS_Result SetData(GLS_Sprite * sprite, GLS_UInt32 width, GLS_UInt32 height, const void * data, GLS_UInt32 stride)
{
	spritePow2 * s = (spritePow2 *) sprite;

	glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, width + stride);

	glBindTexture(GL_TEXTURE_2D, s->textureID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glPopClientAttrib();

	return glGetError() ? GLS_OPENGL_ERROR : GLS_OK;
}
