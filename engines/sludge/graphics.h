/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
#ifndef SLUDGE_GRAPHICS_H
#define SLUDGE_GRAPHICS_H

#include "graphics/surface.h"

namespace Sludge {

struct texture {
#if 0
	GLubyte *data;
	GLuint name;
	double texW, texH;
#endif
	Graphics::Surface surface;
};

#if 0
struct shaders {
	GLuint paste;
	GLuint smartScaler;
	GLuint yuv;
	GLuint texture;
	GLuint color;
};
#endif

struct textureList {
#if 0
	GLuint name;
	GLsizei width;
	GLsizei height;
#endif
	struct textureList *next;
};

#if 0
// From Backdrop.cpp, but they're here anyway
extern GLubyte *backdropTexture;
extern GLfloat backdropTexW, backdropTexH;
#endif

extern unsigned int winWidth, winHeight;
extern int viewportHeight, viewportWidth;
extern int viewportOffsetX, viewportOffsetY;
extern int realWinWidth, realWinHeight;

extern bool NPOT_textures;

#if 0
extern shaders shader;
extern GLfloat aPMVMatrix[];

void setPrimaryColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void setSecondaryColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

void drawQuad(GLint program, const GLfloat *vertices, int numTexCoords, ...);

void setPMVMatrix(GLint program);
#endif

void setPixelCoords(bool pixels);
void setGraphicsWindow(bool fullscreen, bool restoreGraphics = true,
		bool resize = false);

void setupOpenGLStuff();

int getNextPOT(int n);

#if 0
void saveTexture(GLuint tex, GLubyte *data);

void dcopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border, GLuint name, const char *file, int line);
void dcopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height, GLuint name, const char *file, int line);
void dtexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *data, GLuint name, const char *file, int line);
void dtexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *data, GLuint name, const char *file, int line);

#define copyTexImage2D(target, level, internalformat, x, y,  width, height, border, name) dcopyTexImage2D(target,  level,  internalformat,  x,  y,  width,height, border, name, __FILE__, __LINE__)

#define copyTexSubImage2D(target,  level,  xoffset,yoffset, x,  y,   width,   height, name) dcopyTexSubImage2D(target,  level,  xoffset,  yoffset,  x,  y,   width,  height, name, __FILE__, __LINE__)

#define texImage2D(target,  level,  internalformat,  width,  height,  border,  format, type,  data,name) dtexImage2D( target,   level,  internalformat, width, height, border,  format,  type,  data, name, __FILE__, __LINE__)

#define texSubImage2D( target,  level,   xoffset,   yoffset,   width,  height, format,  type,   data,name) dtexSubImage2D( target, level,   xoffset,  yoffset,  width,  height, format,  type,  data,  name, __FILE__, __LINE__)

void deleteTextures(GLsizei n, const GLuint *textures);

void getTextureDimensions(GLuint name, GLint *width, GLint *height);

int printOglError(const char *file, int line);
#define printOpenGLError() printOglError(__FILE__, __LINE__)
#endif

}
 // End of namespace Sludge

#endif
