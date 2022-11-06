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

#include "watchmaker/game.h"

#if defined(USE_OPENGL_GAME)

#include "graphics/pixelformat.h"
#include "watchmaker/3d/render/opengl_2d.h"
#include "watchmaker/3d/render/opengl_renderer.h"
#include "watchmaker/game.h"
#include "watchmaker/rect.h"
#include "watchmaker/render.h"
#include "watchmaker/renderer.h"
#include "watchmaker/sdl_wrapper.h"
#include "watchmaker/tga_util.h"
#include "watchmaker/utils.h"
#include "watchmaker/work_dirs.h"

#include "graphics/opengl/system_headers.h"

namespace Watchmaker {

// Bitmap list
unsigned int    gNumBitmapList = 0;
gTexture        gBitmapList[MAX_BITMAP_LIST];
Rect            gBlitterExtends;
int     gStencilBitDepth;

gLogo       Logos[10];
gMaterial   LogosMaterials[10];
int         NumLogosMaterials = 0, GlobalLogosDelay = 60 * 3;

unsigned int CurLoaderFlags;

//*********************************************************************************************
unsigned int gGetBitmapListPosition() {
	unsigned int pos = 1;

	while (!gBitmapList[pos].isEmpty()) {
		pos++;
	}

	if (pos > MAX_BITMAP_LIST)
		return 0;

	if (pos > gNumBitmapList)
		gNumBitmapList = pos;

	return pos;
}

unsigned int rGetBitmapDimX(unsigned int id) {
	return gBitmapList[id].DimX;
}

//************************************************************************************************************************
unsigned int rGetBitmapDimY(unsigned int id) {
	return gBitmapList[id].DimY;
}

unsigned int rGetBitmapRealDimX(unsigned int id) {
	return gBitmapList[id].RealDimX;
}

unsigned int  rGetBitmapRealDimY(unsigned int id) {
	return gBitmapList[id].RealDimY;
}

//************************************************************************************************************************
void rUpdateExtends(int x1, int y1, int x2, int y2) {
	//Update extends
	if (x1 < gBlitterExtends.left)
		gBlitterExtends.left = x1;
	if (y1 < gBlitterExtends.top)
		gBlitterExtends.top = y1;
	if (x2 > gBlitterExtends.right)
		gBlitterExtends.right = x2;
	if (y2 > gBlitterExtends.bottom)
		gBlitterExtends.bottom = y2;
}

//************************************************************************************************************************
void rGetExtends(int *x1, int *y1, int *x2, int *y2) {
	*x1 = gBlitterExtends.left;
	*y1 = gBlitterExtends.top;
	*x2 = gBlitterExtends.right;
	*y2 = gBlitterExtends.bottom;
}

//************************************************************************************************************************
void rResetExtends(void) {
	gBlitterExtends.left = 99999999;
	gBlitterExtends.top = 99999999;
	gBlitterExtends.right = -99999999;
	gBlitterExtends.bottom = -99999999;
}

// TODO: Move this to Renderer
extern Rect gBlitterViewport;
bool gClipToBlitterViewport(int *sposx, int *sposy, int *sdimx, int *sdimy,
                            int *dposx, int *dposy) {
	int dwWidth, dwHeight;

	dwWidth = (gBlitterViewport.right - gBlitterViewport.left);
	dwHeight = (gBlitterViewport.bottom - gBlitterViewport.top);

	if (((*dposx) + (*sdimx)) > dwWidth) {
		(*sdimx) = (*sdimx) - ((*dposx) + (*sdimx) - dwWidth);
	}
	if (((*dposy) + (*sdimy)) > dwHeight) {
		(*sdimy) = (*sdimy) - ((*dposy) + (*sdimy) - dwHeight);
	}

	if ((*dposx) < gBlitterViewport.left) {
		(*sposx) += gBlitterViewport.left - (*dposx);
		(*sdimx) -= gBlitterViewport.left - (*dposx);
		(*dposx) = gBlitterViewport.left;
	}
	if ((*dposy) < gBlitterViewport.top) {
		(*sposy) += gBlitterViewport.top - (*dposy);
		(*sdimy) -= gBlitterViewport.top - (*dposy);
		(*dposy) = gBlitterViewport.top;
	}

	if (((*sdimx) <= 0) || ((*sdimy) <= 0)) {
		return false;
	}

	return true;
}

void enter2Dmode(WGame &game) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	checkGlError("Exiting enter2Dmode");
}

void exit2Dmode() {
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	checkGlError("exit2Dmode");
}

void renderTexture(WGame &game, gTexture &bitmap, Rect srcRect, Rect dstRect) {
	checkGlError("Entering renderTexture");
	glClearColor(0, 0, 1, 0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	bitmap._texture->bind();
	glLoadIdentity();
	glTranslatef(0, 0, 0.0);

	float bottomSrc = ((float)srcRect.bottom) / bitmap.RealDimY;
	float topSrc = ((float)srcRect.top) / bitmap.RealDimY;
	float leftSrc = ((float)srcRect.left) / bitmap.RealDimX;
	float rightSrc = ((float)srcRect.right) / bitmap.RealDimX;

	Rect viewport = game._renderer->_viewport;
	float bottomDst = 1.0 - ((dstRect.bottom == 0 ? 0 : ((double)dstRect.bottom) / viewport.height()) * 2.0);
	float topDst = 1.0 - ((dstRect.top == 0 ? 0 : ((double)dstRect.top) / viewport.height()) * 2.0);
	float leftDst = ((dstRect.left == 0 ? 0 : ((double)dstRect.left) / viewport.width()) * 2.0) - 1.0;
	float rightDst = ((dstRect.right == 0 ? 0 : ((double)dstRect.right) / viewport.width()) * 2.0) - 1.0;

	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0);

	glTexCoord2f(leftSrc, bottomSrc); // Bottom Left
	glVertex3f(leftDst, bottomDst, 0.0f);

	glTexCoord2f(rightSrc, bottomSrc); // Bottom Right
	glVertex3f(rightDst, bottomDst, 0.0f);

	glTexCoord2f(rightSrc, topSrc); // Top Right
	glVertex3f(rightDst, topDst, 0.0f);

	glTexCoord2f(leftSrc, topSrc); // Top Left
	glVertex3f(leftDst, topDst, 0.0f);

	glEnd();
	glFlush();
	checkGlError("Exiting renderTexture");
}

void gTexture::render(WGame &game, Rect src, Rect dst) {
	// Render self
	if (_texture) {
		renderTexture(game, *this, src, dst);
	}
	for (int i = 0; i < _blitsOnTop.size(); i++) {
		_blitsOnTop[i].texture->render(game, _blitsOnTop[i].src, _blitsOnTop[i].dst);
	}
}

void rBlitScreenBuffer(WGame &game) { // Should probably go to opengl_2d
	checkGlError("Entering rBlitScreenBuffer");
	enter2Dmode(game);
	gBitmapList[BACK_BUFFER].render(game, game._renderer->_viewport, game._renderer->_viewport);
	exit2Dmode();
	checkGlError("Exiting rBlitScreenBuffer");
}

void rClear(int dst, int dposx, int dposy, int sdimx, int sdimy, unsigned char r, unsigned char g, unsigned char b) {
	warning("STUBBED: rClear(%d, %d, %d, %d, %d", dst, dposx, dposy, sdimx, sdimy);
	gBitmapList[dst].clear();
}

//************************************************************************************************************************
void rBlitter(WGame &game, int dst, int src, int dposx, int dposy,
              int sposx, int sposy, int sdimx, int sdimy) {
	// TODO: This currently gets called a bit too much.
	warning("TODO: Stubbed rBlitter(%s, %d, %d, %d, %d, %d, %d, %d, %d)", gBitmapList[src].name.c_str(), dst, src, dposx, dposy, sposx, sposy, sdimx, sdimy);
	auto &bitmap = gBitmapList[src];

	assert(dst == 0);
	auto &dstBitmap = gBitmapList[dst];

	checkGlError("rBlitter Start");

	glEnable(GL_TEXTURE_2D);
	int dwWidth, dwHeight;

	dwWidth = game._renderer->_viewport.width();
	dwHeight = game._renderer->_viewport.height();;

	if ((sdimx <= 0)) {
		sdimx = gBitmapList[src].DimX;
	}
	if ((sdimy <= 0)) {
		sdimy = gBitmapList[src].DimY;
	}

	if ((dposx >= dwWidth) || (dposy >= dwHeight) || (sposx >= dwWidth) || (sposy >= dwHeight) ||
	        ((dposx + sdimx) <= 0) || ((dposy + sdimy) <= 0) || ((sposx + sdimx) <= 0) || ((sposy + sdimy) <= 0)) {
		return;
	}

	if (dst == 0) {
		if (!gClipToBlitterViewport(&sposx, &sposy, &sdimx, &sdimy, &dposx, &dposy)) {
			error("gClipToBlitterViewport report an error");
			return;
		}

		rUpdateExtends(dposx, dposy, dposx + sdimx, dposy + sdimy);
	}

	if ((sdimx == 0) && (sdimy == 0)) {
		sdimx = gBitmapList[src].DimX;
		sdimy = gBitmapList[src].DimY;
	}

	{
		Rect srcRect;
		// Source rect
		srcRect.top = sposy;
		srcRect.left = sposx;
		srcRect.right = sposx + sdimx;
		srcRect.bottom = sposy + sdimy;

		Rect dstRect;
		// Destination rect
		// Convention in dpos is that 0,0 is upper left hand corner, increasing down the y-axis.
		dstRect.top = dposy;
		dstRect.left = dposx;
		dstRect.right = dposx + sdimx;
		dstRect.bottom = dposy + sdimy;
		if (((dstRect.bottom - dstRect.top) <= 0) || ((dstRect.right - dstRect.left) <= 0) || ((srcRect.bottom - srcRect.top) <= 0) || ((srcRect.right - srcRect.left) <= 0) ||
		        (dstRect.right <= 0) || (srcRect.right <= 0) || (dstRect.bottom < 0) || (srcRect.bottom < 0)) {
//	     DebugLogWindow("gBlitter: blit not needed: dimx:%d dimy:%d", ( sr.top-sr.bottom ),( sr.left-sr.right ));
			return;
		}
		dstBitmap.blitInto(&bitmap, srcRect, dstRect);
	}
	checkGlError("rBlitter End");

//	DebugLogFile("gBlitter(%d %d)",dst,src);
	//gBlitter(d, s, sposx, sposy, sdimx, sdimy, dposx, dposy, 0);
//#endif
}

// Straight from Wintermute:
void applyColorKey(Graphics::Surface &surf, byte ckRed, byte ckGreen, byte ckBlue, bool replaceAlpha) {
	// this is taken from Graphics::TransparentSurface
	// only difference is that we set the pixel
	// color to transparent black, like D3DX,
	// if it matches the color key
	for (int y = 0; y < surf.h; y++) {
		for (int x = 0; x < surf.w; x++) {
			uint32 pix = ((uint32 *)surf.getPixels())[y * surf.w + x];
			uint8 r, g, b, a;
			surf.format.colorToARGB(pix, a, r, g, b);
			if (r == ckRed && g == ckGreen && b == ckBlue) {
				a = 0;
				r = 0;
				g = 0;
				b = 0;
				((uint32 *)surf.getPixels())[y * surf.w + x] = surf.format.ARGBToColor(a, r, g, b);
			} else if (replaceAlpha) {
				a = 255;
				((uint32 *)surf.getPixels())[y * surf.w + x] = surf.format.ARGBToColor(a, r, g, b);
			}
		}
	}
}

int rLoadBitmapImage(WGame &game, const char *TextName, unsigned char flags) {
	WorkDirs &workDirs = game.workDirs;
	if (flags & rTEXTURESURFACE) {
		warning("TODO: support texture surface loading");
		//  return ((int) gLoadTexture(TextName, flags));
	}

	assert(TextName);
	auto stream = workDirs.resolveFile(TextName);
	if (!stream) {
		warning("gLoadBitmapImage: Cannot find %s.", TextName);
		return -1;
	}

	Graphics::PixelFormat RGBA8888(4, 8, 8, 8, 8, 0, 8, 16, 24);

	unsigned int pos = gGetBitmapListPosition();
	if (pos == 0) {
		warning("rLoadBitmap: Can't create more bitmaps");
		return -1;
	}
	gTexture *Texture = &gBitmapList[pos];
	*Texture = gTexture();
	Texture->Flags = CurLoaderFlags;
	auto surface = ReadTgaImage(TextName, *stream, RGBA8888, Texture->Flags);
	applyColorKey(*surface, 0, 0, 0, false);
	auto texData = createTextureFromSurface(*surface, GL_RGBA);
	Texture->_texture = createGLTexture();
	Texture->_texture->assignData(*texData);
	Texture->name = TextName;

	if (flags & rSURFACESTRETCH) { // Also rSURFACEFLIP
		static bool warned = false;
		if (!warned) {
			warning("TODO: rSURFACESTRETCH");
			warned = true;
		}
		// HACK: Just set a dimension at all:
		Texture->DimX = surface->w;
		Texture->DimY = surface->h;
	} else {
		Texture->DimX = surface->w;
		Texture->DimY = surface->h;
	}

	Texture->RealDimX = surface->w;
	Texture->RealDimY = surface->h;
	// TODO: Colour-keying
	return pos;
}

void rSetLoaderFlags(unsigned int NewLoaderFlags) {
	CurLoaderFlags = NewLoaderFlags;
}

} // End of namespace Watchmaker

#endif // USE_OPENGL_GAME
