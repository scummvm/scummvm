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
uint16          *gFonts[10];
unsigned int    gNumFonts = 0;
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

	while (gBitmapList[pos].surface != nullptr) {
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

//************************************************************************************************************************
void rBlitter(WGame &game, int dst, int src, int dposx, int dposy,
              int sposx, int sposy, int sdimx, int sdimy) {
	// TODO: This currently gets called a bit too much.
	//warning("TODO: Stubbed rBlitter(%s, %d, %d, %d, %d, %d, %d, %d, %d)", gBitmapList[src].Name, dst, src, dposx, dposy, sposx, sposy, sdimx, sdimy);
	auto &bitmap = gBitmapList[src];

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
		((dposx + sdimx) <= 0) || ((dposy + sdimy) <= 0) || ((sposx + sdimx) <= 0) || ((sposy + sdimy) <= 0))
		return;

	if (dst == 0) {
#if 0
		if (!gClipToBlitterViewport(&sposx, &sposy, &sdimx, &sdimy, &dposx, &dposy)) {
//			DebugLogFile("gClipToBlitterViewport report an error");
			return;
		}
		d = gScreenBuffer.lpDDSurface;
#endif
		rUpdateExtends(dposx, dposy, dposx + sdimx, dposy + sdimy);
#if 0
		/*      //Update extends
				if (dposx<gBlitterExtends.left)
					gBlitterExtends.left=dposx;
				if (dposy<gBlitterExtends.top)
					gBlitterExtends.top=dposy;
				if ((dposx+sdimx)>gBlitterExtends.right)
					gBlitterExtends.right=dposx+sdimx;
				if ((dposy+sdimy)>gBlitterExtends.bottom)
					gBlitterExtends.bottom=dposy+sdimy;*/
	} else
		d = gBitmapList[dst].lpDDSurface;

	if (src == 0) {
		DebugLogFile("rBlitter error: src is an invalid surface");
		return;
	} else
		s = gBitmapList[src].lpDDSurface;
#endif
	}
	if ((sdimx == 0) && (sdimy == 0)) {
		sdimx = gBitmapList[src].DimX;
		sdimy = gBitmapList[src].DimY;
	}

	{
		Rect srcRect;
		// Source rect
		srcRect.top=sposy;
		srcRect.left=sposx;
		srcRect.right= sposx + sdimx;
		srcRect.bottom= sposy + sdimy;

		Rect dstRect;
		// Destination rect
		// Convention in dpos is that 0,0 is upper left hand corner, increasing down the y-axis.
		dstRect.top=dposy;
		dstRect.left=dposx;
		dstRect.right= dposx + sdimx;
		dstRect.bottom= dposy + sdimy;
		if(((dstRect.bottom - dstRect.top) <= 0) || ((dstRect.right - dstRect.left ) <= 0) || ((srcRect.bottom - srcRect.top) <= 0) || ((srcRect.right - srcRect.left ) <= 0) ||
		   (dstRect.right <= 0) || (srcRect.right <= 0) || (dstRect.bottom < 0) || (srcRect.bottom < 0) )
		{
//	   	DebugLogWindow("gBlitter: blit not needed: dimx:%d dimy:%d", ( sr.top-sr.bottom ),( sr.left-sr.right ));
			return;
		}
		glClearColor(0,0,1,0);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, bitmap.texId);
		glLoadIdentity();
		glTranslatef(0, 0, -1.0);
		//glTranslatef((2.0 / dposx) - 1.0, (2.0 / dposy) - 1.0, 0.0f);

		//glClear(GL_COLOR_BUFFER_BIT);


		float bottomSrc = ((float)srcRect.bottom) / bitmap.RealDimY;
		float topSrc =    ((float)srcRect.top) / bitmap.RealDimY;
		float leftSrc =   ((float)srcRect.left) / bitmap.RealDimX;
		float rightSrc =  ((float)srcRect.right) / bitmap.RealDimX;

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
	Texture->surface = ReadTgaImage(TextName, *stream, RGBA8888, Texture->Flags);
	applyColorKey(*Texture->surface, 0, 0, 0, false);
	Texture->texId = createTextureFromSurface(*Texture->surface, GL_RGBA);
	Texture->name = TextName;

	if (flags & rSURFACESTRETCH) { // Also rSURFACEFLIP
		static bool warned = false;
		if (!warned) {
			warning("TODO: rSURFACESTRETCH");
			warned = true;
		}
		// HACK: Just set a dimension at all:
		Texture->DimX = Texture->surface->w;
		Texture->DimY = Texture->surface->h;
	} else {
		Texture->DimX = Texture->surface->w;
		Texture->DimY = Texture->surface->h;
	}

	Texture->RealDimX = Texture->surface->w;
	Texture->RealDimY = Texture->surface->h;
	// TODO: Colour-keying
	return pos;
}

void rSetLoaderFlags(unsigned int NewLoaderFlags) {
	CurLoaderFlags = NewLoaderFlags;
}

bool gUpdateMovie(gMaterial &mat) {
	WORD newFrame = 0;
	DWORD curTime;

	if (mat.Flags & T3D_MATERIAL_MOVIEPAUSED)
		return TRUE;

	auto mv = mat.Movie;

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
#if 0
	bool retv=gMovie_SetFrame(mat,newFrame);

	return(retv);
#endif
	return true;
}

} // End of namespace Watchmaker

#endif // USE_OPENGL_GAME
