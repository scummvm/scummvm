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
	warning("STUBBED: rInitialize3DEnvironment\n");
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
	warning("STUBBED: rSetRenderMode\n");
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
		warning("%s: GL raised error %d - %s\n", when, error, explanation.c_str());
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
	warning("STUBBED: rLockSurfaceDirect\n");
	return nullptr;
}

void rUnlockSurfaceDirect(gTexture *t) {
	warning("STUBBED: rUnlockSurfaceDirect\n");
}

uint16 rRGBAToTextureFormat(unsigned int r, unsigned int g, unsigned int b, unsigned int a) {
	warning("STUBBED: rRGBAToTextureFormat\n");
	return 0;
}

bool rSetMovieFrame(MaterialPtr mat, uint32 dwCurrFrame) {
	warning("STUBBED: rSetMovieFrame\n");
	return false;
}

int rCreateSurface(unsigned int dimx, unsigned int dimy, unsigned char flags) {
	warning("STUBBED: rCreateSurface\n");
	return 0;
}

VertexBuffer rCreateVertexBuffer(unsigned int num) {
	//warning("STUBBED: rCreateVertexBuffer\n");
	return VertexBuffer();
}

bool rDeleteVertexBuffer(VertexBuffer &vb) {
	warning("STUBBED: rDeleteVertexBuffer\n");
	return false;
}

void rGetScreenInfos(unsigned int *width, unsigned int *height, unsigned int *bpp) {
	warning("STUBBED: rGetScreenInfos\n");
	*width = 800;
	*height = 600;
	*bpp = 32;
}

//D3d specific geometry trasf. functions
bool rSetProjectionMatrix(float width, float height, float fAspect,
                          float fNearPlane, float fFarPlane) {
	// Not sure if fAspect is Y here though
	glMatrixMode(GL_PROJECTION);
	auto perspectiveMatrix = Math::makePerspectiveMatrix(fAspect, width / height, fNearPlane, fFarPlane);
	glLoadMatrixf(perspectiveMatrix.getData());

	glMatrixMode(GL_MODELVIEW);
	return false;
}


void rScreenSpaceToCameraSpace(float *dx, float *dy, float *dz, float x, float y) {
	//warning("STUBBED: rScreenSpaceToCameraSpace\n");
#if 0
	D3DMATRIX   m;
	D3DVECTOR   v, d;
	unsigned int width, height, bpp;

	rGetScreenInfos(&width, &height, &bpp);

	D3DMath_MatrixInvert(m, rProjectionMatrix);

	v.x = (x - width / 2) / (width / 2);
	v.y = -(y - height / 2) / (height / 2);
	v.z = 1.0f;
	D3DMath_VectorMatrixMultiply(d, v, m);

	*dx = d.x;
	*dy = d.y;
	*dz = d.z - gNearPlane;
#endif
}

gTexture *gLoadTexture(char *TextName, unsigned int LoaderFlags) {
	warning("STUBBED gLoadTexture\n");
	return nullptr;
}

bool rGrabVideo(const char *path, char flags) {
	warning("STUBBED: rGrabVideo\n");
	return true;
}

void rReleaseAllBitmaps(unsigned int NotFlags) {
	warning("STUBBED: rReleaseAllBitmaps\n");
	return;
}

void rReleaseBitmap(int i) {
	warning("STUBBED: rReleaseBitmap\n");
	return;
}

void rReleaseAllTextures(unsigned int NotFlags) {
	warning("STUBBED: rReleaseAllTextures\n");
	return;
}

void rSetBitmapName(unsigned int id, const char *s) {
	warning("STUBBED: rSetBitmapName\n");
	return;
}

uint32 rGetMovieFrame(MaterialPtr mat) {
	warning("STUBBED: rGetMovieFrame\n");
	return 0;
}


void rPrintText(const char *s, unsigned int dst,  unsigned int src, unsigned short *FontTable, unsigned short x, unsigned short y) {
	warning("STUBBED: rPrintText\n");
	return;
}

void rGetTextDim(const char *s, unsigned short *FontTable, int *x, int *y) {
	warning("STUBBED: rGetTextDim\n");
	return;
}


void rResetPipeline() {
	//warning("STUBBED: rResetPipeline\n");
	return;
}

void rClear(int dst, int dposx, int dposy, int sdimx, int sdimy, unsigned char r, unsigned char g, unsigned char b) {
	//warning("STUBBED: rClear(%d, %d, %d, %d, %d\n", dst, dposx, dposy, sdimx, sdimy);
}


void rBlitScreenBuffer(void) { // Should probably go to opengl_2d
	//warning("STUBBED: rBlitScreenBuffer\n");
}

bool gMovie_SetFrame(gMaterial *mat, WORD newFrame) {
	Common::SharedPtr<gMovie> mv = mat->Movie;

	if (mv->curFrame == newFrame) return TRUE;

	//do we have to replace the whole frame or do we have to built it?
	char rebuildFrame = 1;
	if (mv->curFrame == 0xFFFF) rebuildFrame = 0;
	else if (!(newFrame % mv->keyFrame)) rebuildFrame = 0;        //it's a keyframe
#if 0
	DDSURFACEDESC2 ddsd2;
	ddsd2.dwSize = sizeof(DDSURFACEDESC2);

	if ((mv->surf->Lock(NULL, &ddsd2, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL))) {                  // Lock and fill with the dds
		DebugLogFile("gMovie_SetFrame: Can't lock surface DDS");
		return NULL;
	}

	if (!rebuildFrame) {
		gMovie_LoadThisFrameData(mv, newFrame);
		memcpy(ddsd2.lpSurface, mv->buffer, ddsd2.dwLinearSize);
	} else {
		if ((mv->curFrame + 1) != newFrame) { //we can't directly build this frame because the current frame is not his previous
			WORD startFrame;
			WORD prevKey = (newFrame / mv->keyFrame) * mv->keyFrame;

			if ((mv->curFrame > newFrame) || (mv->curFrame < prevKey)) {
				gMovie_LoadThisFrameData(mv, prevKey);
				memcpy(ddsd2.lpSurface, mv->buffer, ddsd2.dwLinearSize);
				startFrame = prevKey + 1;
			} else startFrame = mv->curFrame + 1;

			for (WORD i = startFrame; i < newFrame; i++) {
				gMovie_BuildNewFrame((BYTE *)ddsd2.lpSurface, mv, i);
			}

		}

		gMovie_BuildNewFrame((BYTE *)ddsd2.lpSurface, mv, newFrame);

	}

	if ((mv->surf->Unlock(NULL))) {
		DebugLogFile("gMovie_SetFrame: Can't unlock surface DDS");
		return NULL;
	}

	if (mat->Texture->lpDDSurface->Blt(NULL, mv->surf, NULL, DDBLT_WAIT, NULL) != DD_OK) {
		DebugLogFile("gMovie_SetFrame: Can't Blit DDS texture");
		return NULL;
	}

	mv->curFrame = newFrame;

	return TRUE;
#endif
}

//*********************************************************************************************
bool gUpdateMovie(gMaterial *mat) {
	WORD newFrame = 0;
	DWORD curTime;

	if (mat->Flags & T3D_MATERIAL_MOVIEPAUSED)
		return TRUE;

    Common::SharedPtr<gMovie> mv = mat->Movie;

	if ((mv->curFrame == 0xFFFF) || (!mv->startTime)) {
		mv->startTime = timeGetTime();
		newFrame = 0;
	} else {
		// Use the time to find which frame we should be drawing
		curTime = timeGetTime();
		DWORD elapsedTime = curTime - mv->startTime;
		newFrame = (WORD)((float)elapsedTime / (1000.f / (float)mv->frameRate));

		if (newFrame >= mv->numFrames) {
			mv->startTime = curTime;
			newFrame = 0;
		}
	}

	bool retv = gMovie_SetFrame(mat, newFrame);
	return (retv);
}

} // End of namespace Watchmaker

#endif // USE_OPENGL_GAME
