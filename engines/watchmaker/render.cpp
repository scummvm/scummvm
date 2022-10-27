/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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

#include "watchmaker/sdl_wrapper.h"
#include "watchmaker/render.h"
#include "watchmaker/utils.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/tga_util.h"
#include "common/stream.h"
#include "watchmaker/game.h"
#include "watchmaker/rect.h"
#include "watchmaker/renderer.h"

#if defined(USE_OPENGL_GAME)

#include "graphics/opengl/system_headers.h"
#include "math/glmath.h"

#define MAXTEXTURES     2000
#define MAX_BITMAP_LIST 1024

namespace Watchmaker {

bool rInitialize3DEnvironment(WGame &game, char *cfg) {
	warning("STUBBED: rInitialize3DEnvironment");
	game.sdl->initWindow();
	game._renderer->initGL();
	return true;
}

#if 0
//*********************************************************************************************
unsigned int gGetTextureListPosition() {
	unsigned int pos = 1;

	while (gTextureList[pos].surface != NULL) {
		pos++;
	}

	if (pos > MAXTEXTURES)
		return 0;

	if (pos > gNumTextureList)
		gNumTextureList = pos;

	return pos;
}
#endif

bool rSetRenderMode(int state) {
	warning("STUBBED: rSetRenderMode");
}

bool checkGlError(const char *when) {
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		Common::String explanation = "";
		switch (error) {
			case GL_INVALID_ENUM: explanation = "GL_INVALID_ENUM"; break;
			case GL_INVALID_VALUE: explanation = "GL_INVALID_VALUE"; break;
			case GL_INVALID_OPERATION: explanation = "GL_INVALID_OPERATION"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: explanation = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
			case GL_OUT_OF_MEMORY: explanation = "GL_OUT_OF_MEMORY"; break;
			case GL_STACK_UNDERFLOW: explanation = "GL_STACK_UNDERFLOW"; break;
			case GL_STACK_OVERFLOW: explanation = "GL_STACK_OVERFLOW"; break;
			default: break;
		}
		warning("%s: GL raised error %d - %s", when, error, explanation.c_str());
		return false;
	}
	return true;
}

bool rClearBuffers(char flags) {
	bool clearStencil = rGetStencilBitDepth() != 0;
	bool clearDepth = flags & rCLEARZBUFFER;
	bool clearBack = flags & rCLEARBACKBUFFER;
	bool clearScreen = flags & rCLEARSCREENBUFFER;

	// Picking green to make the clear color easy to spot.
	glClearColor(0, 1, 0, 1);
	GLbitfield mask = (clearStencil ? GL_STENCIL_BUFFER_BIT : 0) | (clearDepth ? GL_DEPTH_BUFFER_BIT : 0) | (clearScreen ? GL_COLOR_BUFFER_BIT : 0);
	glClear(mask);
	return checkGlError("rClearBuffers");
}

unsigned char *rLockSurfaceDirect(gTexture *t, unsigned int *pitch) {
	warning("STUBBED: rLockSurfaceDirect");
	return nullptr;
}

void rUnlockSurfaceDirect(gTexture *t) {
	warning("STUBBED: rUnlockSurfaceDirect");
}

uint16 rRGBAToTextureFormat(unsigned int r, unsigned int g, unsigned int b, unsigned int a) {
	warning("STUBBED: rRGBAToTextureFormat");
	return 0;
}

bool rSetMovieFrame(MaterialPtr mat, uint32 dwCurrFrame) {
	warning("STUBBED: rSetMovieFrame");
	return false;
}

int rCreateSurface(unsigned int dimx, unsigned int dimy, unsigned char flags) {
	warning("STUBBED: rCreateSurface");
	return 0;
}

VertexBuffer rCreateVertexBuffer(unsigned int num) {
	//warning("STUBBED: rCreateVertexBuffer");
	return VertexBuffer();
}

bool rDeleteVertexBuffer(VertexBuffer &vb) {
	warning("STUBBED: rDeleteVertexBuffer");
	return false;
}

void rGetScreenInfos(unsigned int *width, unsigned int *height, unsigned int *bpp) {
	warning("STUBBED: rGetScreenInfos");
	*width = 800;
	*height = 600;
	*bpp = 32;
}

gTexture *gLoadTexture(char *TextName, unsigned int LoaderFlags) {
	warning("STUBBED gLoadTexture");
	return nullptr;
}

bool rGrabVideo(const char *path, char flags) {
	warning("STUBBED: rGrabVideo");
	return true;
}

void rReleaseAllBitmaps(unsigned int NotFlags) {
	warning("STUBBED: rReleaseAllBitmaps");
	return;
}

void rReleaseBitmap(int i) {
	warning("STUBBED: rReleaseBitmap");
	return;
}

void rReleaseAllTextures(unsigned int NotFlags) {
	warning("STUBBED: rReleaseAllTextures");
	return;
}

void rSetBitmapName(unsigned int id, const char *s) {
	warning("STUBBED: rSetBitmapName");
	return;
}

uint32 rGetMovieFrame(MaterialPtr mat) {
	warning("STUBBED: rGetMovieFrame");
	return 0;
}


void rPrintText(const char *s, unsigned int dst,  unsigned int src, unsigned short *FontTable, unsigned short x, unsigned short y) {
	warning("STUBBED: rPrintText");
	return;
}

void rGetTextDim(const char *s, unsigned short *FontTable, int *x, int *y) {
	warning("STUBBED: rGetTextDim");
	return;
}

void rClear(int dst, int dposx, int dposy, int sdimx, int sdimy, unsigned char r, unsigned char g, unsigned char b) {
	//warning("STUBBED: rClear(%d, %d, %d, %d, %d", dst, dposx, dposy, sdimx, sdimy);
}


void rBlitScreenBuffer(void) { // Should probably go to opengl_2d
	//warning("STUBBED: rBlitScreenBuffer");
}

} // End of namespace Watchmaker

#endif // USE_OPENGL_GAME
