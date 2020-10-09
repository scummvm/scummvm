/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#if defined (SDL_BACKEND) && defined (ENABLE_OPENGL)

#include "engines/icb/gfx/gfxstub.h"
#include "engines/icb/gfx/gfxstub_rev_dutch.h"
#include "engines/icb/surface_manager.h"

#include <SDL.h>
#include <SDL_opengl.h>

namespace ICB {

// 256*256 RGBA temporary texture
#define TEMP_TEXTURE_WIDTH 256
#define TEMP_TEXTURE_HEIGHT 256
#define BITS_PER_COLOUR 4
u_char pcRGBA[TEMP_TEXTURE_WIDTH * TEMP_TEXTURE_HEIGHT * BITS_PER_COLOUR];

typedef struct {
	char *pRGB;
	int RGBPitch;
	int RGBBytesPerPixel;
	u_int width, height;

	char *pZ;
	int ZPitch;
	int ZBytesPerPixel;
} MyRenderDevice;

MyRenderDevice myRenDev = {0};
RevRenderDevice *lastRevRenDev = NULL;

int SetRenderDevice(RevRenderDevice *renderDev) {
	lastRevRenDev = NULL;
	if (renderDev->RGBdata == NULL)
		return 1;
	if (renderDev->Zdata == NULL)
		return 1;
	if (renderDev->width <= 0)
		return 1;
	if (renderDev->width > 2048)
		return 1;
	if (renderDev->height <= 0)
		return 1;
	if (renderDev->height > 2048)
		return 1;

	myRenDev.pRGB = (char *)(renderDev->RGBdata);
	myRenDev.pZ = (char *)(renderDev->Zdata);
	myRenDev.RGBBytesPerPixel = 4;
	myRenDev.RGBPitch = renderDev->stride;
	myRenDev.ZBytesPerPixel = 2;
	myRenDev.ZPitch = renderDev->width * myRenDev.ZBytesPerPixel;
	myRenDev.width = renderDev->width;
	myRenDev.height = renderDev->height;
	lastRevRenDev = renderDev;
	return 0;
}

int UnregisterTexture(TextureHandle *texture) {
	int i;

	for (i = 0; i < 9; i++)
		if ((texture->pRGBA[i]) != NULL)
			delete[](texture->pRGBA[i]);

	if (texture->palette != NULL)
		delete[](texture->palette);

	glDeleteTextures(1, &texture->textureId);

	delete texture;

	// always return zero
	return 0;
}

TextureHandle *RegisterTexture(const RevTexture *revInput) {
	int i;
	TextureHandle *th = new TextureHandle();

	th->w = revInput->width;
	th->h = revInput->height;

	for (i = 0; i < 9; i++)
		th->pRGBA[i] = NULL;

	if (revInput->palette[0] == 0xDEADBEAF) {
		th->bpp = 4;
		th->palette = NULL;
		th->pRGBA[0] = revInput->level[0];
	} else {
		// Complain if width or height > 256 < 1
		if ((th->w < 1) || (th->w > 256)) {
			delete th;
			return NULL;
		}
		if ((th->h < 1) || (th->h > 256)) {
			delete th;
			return NULL;
		}

		// Complain if the width or height are not powers of 2
		for (i = 0; i < 8; i++) {
			// ERROR
			if (((th->w >> i) << i) != th->w) {
				if ((th->w >> i) != 0) {
					delete th;
					return NULL;
				}
			}
			// ERROR
			if (((th->h >> i) << i) != th->h) {
				if ((th->h >> i) != 0) {
					delete th;
					return NULL;
				}
			}
		}

		th->bpp = 4;
		th->palette = new u_int[256];
		for (i = 0; i < 256; i++)
			th->palette[i] = revInput->palette[i];

		int size = th->w * th->h * th->bpp;
		th->pRGBA[0] = new u_char[size];
		u_char *src = revInput->level[0];
		u_char *dst = th->pRGBA[0];
		for (int l = 0; l < size; l += 4) {
			u_char index = src[l / 4];
			u_int colour = th->palette[index];
			dst[l + 0 /*b*/] = ((colour >> 0) & 0xFF);  // b
			dst[l + 1 /*g*/] = ((colour >> 8) & 0xFF);  // g
			dst[l + 2 /*r*/] = ((colour >> 16) & 0xFF); // r
			dst[l + 3 /*a*/] = ((colour >> 24) & 0xFF); // a
		}
#if 0
		SDL_Surface *s = SDL_CreateRGBSurfaceFrom(dst, th->w, th->h, 32, th->w * 4,
		                 0xFF0000, 0xFF00, 0xFF, 0x0);
		if (SDL_SaveBMP(s, "texture.bmp") < 0)
			printf("Failed to create the output file");
		SDL_FreeSurface(s);
#endif
	}
	glGenTextures(1, &th->textureId);
	glBindTexture(GL_TEXTURE_2D, th->textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, BITS_PER_COLOUR, th->w, th->h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, th->pRGBA[0]);

	return th;
}

int c = 0;
int l = 0;
int t = 0;

void startDrawing(void) {
	// Save settings which we are going to override
	c = glIsEnabled(GL_CULL_FACE);
	l = glIsEnabled(GL_LIGHTING);
	t = glIsEnabled(GL_TEXTURE_2D);

	// Save MODELVIEW matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	// Save PROJECTION matrix
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBindFramebuffer(GL_FRAMEBUFFER, g_RGBFrameBuffer);
	glViewport(0, 0, myRenDev.width, myRenDev.height);

	glOrtho(-320, 320, -240, 240, -32767, 32767);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_ALWAYS);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3f(1.0, 1.0, 1.0);
}

void endDrawing() {
	glReadPixels(0, 0, myRenDev.width, myRenDev.height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, myRenDev.pRGB);
	glReadPixels(0, 0, myRenDev.width, myRenDev.height, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, myRenDev.pZ);

#if 0
	SDL_Surface *s;
	s = SDL_CreateRGBSurfaceFrom(myRenDev.pRGB, myRenDev.width, myRenDev.height, 32,
	                             myRenDev.width * 4, 0xFF0000, 0xFF00, 0xFF, 0x00);
	if (SDL_SaveBMP(s, "renderbuffer_ogl.bmp") < 0)
		printf("Failed to create the output file");
	SDL_FreeSurface(s);

	s = SDL_CreateRGBSurfaceFrom(myRenDev.pZ, myRenDev.width, myRenDev.height, 16,
	                             myRenDev.width * 2, 0x0000f800, 0x000007e0, 0x0000001f, 0x00);
	if (SDL_SaveBMP(s, "zbuffer_ogl.bmp") < 0)
		printf("Failed to create the output file");
	SDL_FreeSurface(s);
#endif

	// Restore the previous settings
	if (l)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);

	if (c)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	if (t)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);

	// Restore MODELVIEW matrix
	glPopMatrix();
	// Restore PROJECTION matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Real graphic routines
int DrawSprite(int x0, int y0, short w, short h, uint8 r0, uint8 g0, uint8 b0, u_short u0, u_short v0, uint8 alpha, u_short z, void *tex) {
	float u0f, v0f, u1f, v1f;
	short x1, y1;
	TextureHandle *pthan = NULL;

	if (tex == NULL) {
		return 0;
	} else {
		pthan = (TextureHandle *)tex;
	}

	z = 32767 - z;
	u0f = (float)(u0 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v0f = (float)(v0 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);
	u1f = u0f + (float)(w >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v1f = v0f + (float)(h >> 8) / (TEMP_TEXTURE_HEIGHT - 1);
	x1 = (short)(x0 + w);
	y1 = (short)(y0 + h);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pthan->textureId);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	// Note: 128 = normal colour scaling
	glColor4f(r0 / 128.0f, g0 / 128.0f, b0 / 128.0f, alpha / 128.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(u0f, v0f);
	glVertex3i(x0, y0, z);
	glTexCoord2f(u1f, v0f);
	glVertex3i(x1, y0, z);
	glTexCoord2f(u1f, v1f);
	glVertex3i(x1, y1, z);
	glTexCoord2f(u0f, v1f);
	glVertex3i(x0, y1, z);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	return 1;
}

// Single coloured rectangle
int DrawTile(int x0, int y0, short w, short h, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, u_short z) {
	z = 32767 - z;
	glColor4f(r0 / 255.0f, g0 / 255.0f, b0 / 255.0f, alpha / 255.0f);
	glBegin(GL_POLYGON);
	glVertex3i(x0, y0, z);
	glVertex3i(x0 + w, y0, z);
	glVertex3i(x0 + w, y0 + h, z);
	glVertex3i(x0, y0 + h, z);
	glEnd();
	return 1;
}

// Single flat coloured line : 2 points, 1 colour
int DrawLineF2(int x0, int y0, int x1, int y1, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, u_short z) {
	z = 32767 - z;
	glColor4f(r0 / 255.0f, g0 / 255.0f, b0 / 255.0f, alpha / 255.0f);

	glBegin(GL_LINES);

	glVertex3i(x0, y0, z);

	glVertex3i(x1, y1, z);

	glEnd();
	return 1;
}

// two connected lines flat coloured : 3 points, 1 colour
int DrawLineF3(int x0, int y0, int x1, int y1, int x2, int y2, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, u_short z) {
	z = 32767 - z;
	glColor4f(r0 / 255.0f, g0 / 255.0f, b0 / 255.0f, alpha / 255.0f);

	glBegin(GL_LINE_STRIP);

	glVertex3i(x0, y0, z);

	glVertex3i(x1, y1, z);

	glVertex3i(x2, y2, z);

	glEnd();
	return 1;
}

// three connected lines flat coloured : 4 points, 1 colour
int DrawLineF4(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, u_short z) {
	z = 32767 - z;
	glColor4f(r0 / 255.0f, g0 / 255.0f, b0 / 255.0f, alpha / 255.0f);

	glBegin(GL_LINE_STRIP);

	glVertex3i(x0, y0, z);

	glVertex3i(x1, y1, z);

	glVertex3i(x2, y2, z);

	glVertex3i(x3, y3, z);

	glEnd();
	return 1;
}

// Single gouraud coloured line : 2 points, 2 colours
int DrawLineG2(int x0, int y0, int x1, int y1, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 alpha, u_short z) {
	z = 32767 - z;
	glBegin(GL_LINES);

	glColor4f(r0 / 255.0f, g0 / 255.0f, b0 / 255.0f, alpha / 255.0f);
	glVertex3i(x0, y0, z);

	glColor4f(r1 / 255.0f, g1 / 255.0f, b1 / 255.0f, alpha / 255.0f);
	glVertex3i(x1, y1, z);

	glEnd();
	return 1;
}

// two connected lines gouraud coloured : 3 points, 3 colours
int DrawLineG3(int x0, int y0, int x1, int y1, int x2, int y2, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2, uint8 alpha, u_short z) {
	z = 32767 - z;
	glBegin(GL_LINES);

	glColor4f(r0 / 255.0f, g0 / 255.0f, b0 / 255.0f, alpha / 255.0f);
	glVertex3i(x0, y0, z);

	glColor4f(r1 / 255.0f, g1 / 255.0f, b1 / 255.0f, alpha / 255.0f);
	glVertex3i(x1, y1, z);

	glColor4f(r2 / 255.0f, g2 / 255.0f, b2 / 255.0f, alpha / 255.0f);
	glVertex3i(x2, y2, z);

	glEnd();
	return 1;
}

// three connected lines gouraud coloured : 4 points, 4 colours
int DrawLineG4(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2, uint8 r3,
               uint8 g3, uint8 b3, uint8 alpha, u_short z) {
	z = 32767 - z;
	glBegin(GL_LINES);

	glColor4f(r0 / 255.0f, g0 / 255.0f, b0 / 255.0f, alpha / 255.0f);
	glVertex3i(x0, y0, z);

	glColor4f(r1 / 255.0f, g1 / 255.0f, b1 / 255.0f, alpha / 255.0f);
	glVertex3i(x1, y1, z);

	glColor4f(r2 / 255.0f, g2 / 255.0f, b2 / 255.0f, alpha / 255.0f);
	glVertex3i(x2, y2, z);

	glColor4f(r3 / 255.0f, g3 / 255.0f, b3 / 255.0f, alpha / 255.0f);
	glVertex3i(x3, y3, z);

	glEnd();
	return 1;
}

// Simple flat coloured triangle : 3 points, 1 colour
int DrawFlatTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, u_short z) {
	z = 32767 - z;
	glBegin(GL_TRIANGLES);

	glColor4f(r0 / 255.0f, g0 / 255.0f, b0 / 255.0f, alpha / 255.0f);

	glVertex3i(x0, y0, z);

	glVertex3i(x1, y1, z);

	glVertex3i(x2, y2, z);

	glEnd();
	return 1;
}

// Simple flat coloured quad : 4 points, 1 colour
int DrawFlatQuad(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, u_short z) {
	z = 32767 - z;
	glBegin(GL_TRIANGLE_STRIP);

	glColor4f(r0 / 255.0f, g0 / 255.0f, b0 / 255.0f, alpha / 255.0f);

	glVertex3i(x0, y0, z);

	glVertex3i(x1, y1, z);

	glVertex3i(x2, y2, z);

	glVertex3i(x3, y3, z);

	glEnd();
	return 1;
}

// Simple gouraud coloured triangle : 3 points, 3 colours
int DrawGouraudTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2, uint8 alpha,
                        u_short z) {
	z = 32767 - z;
	glBegin(GL_TRIANGLES);

	glColor4f(r0 / 255.0f, g0 / 255.0f, b0 / 255.0f, alpha / 255.0f);
	glVertex3i(x0, y0, z);

	glColor4f(r1 / 255.0f, g1 / 255.0f, b1 / 255.0f, alpha / 255.0f);
	glVertex3i(x1, y1, z);

	glColor4f(r2 / 255.0f, g2 / 255.0f, b2 / 255.0f, alpha / 255.0f);
	glVertex3i(x2, y2, z);

	glEnd();
	return 1;
}

// Simple gouraud coloured quad : 4 points, 4 colours
int DrawGouraudQuad(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2,
                    uint8 r3, uint8 g3, uint8 b3, uint8 alpha, u_short z) {
	z = 32767 - z;
	glBegin(GL_TRIANGLE_STRIP);

	glColor4f(r0 / 255.0f, g0 / 255.0f, b0 / 255.0f, alpha / 255.0f);
	glVertex3i(x0, y0, z);

	glColor4f(r1 / 255.0f, g1 / 255.0f, b1 / 255.0f, alpha / 255.0f);
	glVertex3i(x1, y1, z);

	glColor4f(r2 / 255.0f, g2 / 255.0f, b2 / 255.0f, alpha / 255.0f);
	glVertex3i(x2, y2, z);

	glColor4f(r3 / 255.0f, g3 / 255.0f, b3 / 255.0f, alpha / 255.0f);
	glVertex3i(x3, y3, z);

	glEnd();
	return 1;
}

// Simple flat coloured triangle : 3 points, 1 colour, 3 UV's
int DrawFlatTriangleTextured(int x0, int y0, int x1, int y1, int x2, int y2, uint8 r0, uint8 g0, uint8 b0, u_short u0, u_short v0, u_short u1, u_short v1, u_short u2, u_short v2,
                             uint8 alpha, u_short z, void *tex) {
	float u0f, v0f, u1f, v1f, u2f, v2f;
	TextureHandle *pthan = NULL;

	if (tex == NULL) {
		return 0;
	} else {
		pthan = (TextureHandle *)tex;
	}

	z = 32767 - z;
	u0f = (float)(u0 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v0f = (float)(v0 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);
	u1f = (float)(u1 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v1f = (float)(v1 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);
	u2f = (float)(u2 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v2f = (float)(v2 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pthan->textureId);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Note: 128 = normal colour scaling
	glColor4f(r0 / 128.0f, g0 / 128.0f, b0 / 128.0f, alpha / 128.0f);

	glBegin(GL_TRIANGLES);

	glTexCoord2f(u0f, v0f);
	glVertex3i(x0, y0, z);

	glTexCoord2f(u1f, v1f);
	glVertex3i(x1, y1, z);

	glTexCoord2f(u2f, v2f);
	glVertex3i(x2, y2, z);

	glEnd();
	glDisable(GL_TEXTURE_2D);

	return 1;
}

// Simple flat coloured quad : 4 points, 1 colour, 4 UV's
int DrawFlatQuadTextured(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint8 r0, uint8 g0, uint8 b0, u_short u0, u_short v0, u_short u1, u_short v1, u_short u2,
                         u_short v2, u_short u3, u_short v3, uint8 alpha, u_short z, void *tex) {
	float u0f, v0f, u1f, v1f, u2f, v2f, u3f, v3f;
	TextureHandle *pthan = NULL;

	if (tex == NULL) {
		return 0;
	} else {
		pthan = (TextureHandle *)tex;
	}

	z = 32767 - z;
	u0f = (float)(u0 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v0f = (float)(v0 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);
	u1f = (float)(u1 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v1f = (float)(v1 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);
	u2f = (float)(u2 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v2f = (float)(v2 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);
	u3f = (float)(u3 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v3f = (float)(v3 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pthan->textureId);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Note: 128 = normal colour scaling
	glColor4f(r0 / 128.0f, g0 / 128.0f, b0 / 128.0f, alpha / 128.0f);

	glBegin(GL_TRIANGLE_STRIP);

	glTexCoord2f(u0f, v0f);
	glVertex3i(x0, y0, z);

	glTexCoord2f(u1f, v1f);
	glVertex3i(x1, y1, z);

	glTexCoord2f(u2f, v2f);
	glVertex3i(x2, y2, z);

	glTexCoord2f(u3f, v3f);
	glVertex3i(x3, y3, z);

	glEnd();
	glDisable(GL_TEXTURE_2D);

	return 1;
}

// Simple gouraud coloured triangle : 3 points, 3 colours
int DrawGouraudTriangleTextured(int x0, int y0, int x1, int y1, int x2, int y2, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2,
                                u_short u0, u_short v0, u_short u1, u_short v1, u_short u2, u_short v2, uint8 alpha, u_short z, void *tex) {
	float u0f, v0f, u1f, v1f, u2f, v2f;
	TextureHandle *pthan = NULL;

	if (tex == NULL) {
		return 0;
	} else {
		pthan = (TextureHandle *)tex;
	}

	z = 32767 - z;
	u0f = (float)(u0 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v0f = (float)(v0 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);
	u1f = (float)(u1 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v1f = (float)(v1 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);
	u2f = (float)(u2 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v2f = (float)(v2 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pthan->textureId);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBegin(GL_TRIANGLES);

	// Note: 128 = normal colour scaling
	glColor4f(r0 / 128.0f, g0 / 128.0f, b0 / 128.0f, alpha / 128.0f);
	glTexCoord2f(u0f, v0f);
	glVertex3i(x0, y0, z);

	// Note: 128 = normal colour scaling
	glColor3f(r1 / 128.0f, g1 / 128.0f, b1 / 128.0f);
	glTexCoord2f(u1f, v1f);
	glVertex3i(x1, y1, z);

	// Note: 128 = normal colour scaling
	glColor3f(r2 / 128.0f, g2 / 128.0f, b2 / 128.0f);
	glTexCoord2f(u2f, v2f);
	glVertex3i(x2, y2, z);

	glEnd();
	glDisable(GL_TEXTURE_2D);

	return 1;
}

// Simple gouraud coloured quad : 4 points, 4 colours
int DrawGouraudQuadTextured(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2,
                            uint8 b2, uint8 r3, uint8 g3, uint8 b3, u_short u0, u_short v0, u_short u1, u_short v1, u_short u2, u_short v2, u_short u3, u_short v3, uint8 alpha,
                            u_short z, void *tex) {
	float u0f, v0f, u1f, v1f, u2f, v2f, u3f, v3f;
	TextureHandle *pthan = NULL;

	if (tex == NULL) {
		return 0;
	} else {
		pthan = (TextureHandle *)tex;
	}

	z = 32767 - z;
	u0f = (float)(u0 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v0f = (float)(v0 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);
	u1f = (float)(u1 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v1f = (float)(v1 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);
	u2f = (float)(u2 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v2f = (float)(v2 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);
	u3f = (float)(u3 >> 8) / (TEMP_TEXTURE_WIDTH - 1);
	v3f = (float)(v3 >> 8) / (TEMP_TEXTURE_HEIGHT - 1);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pthan->textureId);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBegin(GL_TRIANGLE_STRIP);

	// Note: 128 = normal colour scaling
	glColor4f(r0 / 128.0f, g0 / 128.0f, b0 / 128.0f, alpha / 128.0f);
	glTexCoord2f(u0f, v0f);
	glVertex3i(x0, y0, z);

	// Note: 128 = normal colour scaling
	glColor4f(r1 / 128.0f, g1 / 128.0f, b1 / 128.0f, alpha / 128.0f);
	glTexCoord2f(u1f, v1f);
	glVertex3i(x1, y1, z);

	// Note: 128 = normal colour scaling
	glColor4f(r2 / 128.0f, g2 / 128.0f, b2 / 128.0f, alpha / 128.0f);
	glTexCoord2f(u2f, v2f);
	glVertex3i(x2, y2, z);

	// Note: 128 = normal colour scaling
	glColor4f(r3 / 128.0f, g3 / 128.0f, b3 / 128.0f, alpha / 128.0f);
	glTexCoord2f(u3f, v3f);
	glVertex3i(x3, y3, z);

	glEnd();
	glDisable(GL_TEXTURE_2D);

	return 1;
}

} // End of namespace ICB

#endif
