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

#include "common/scummsys.h"

#if defined(USE_OPENGL_GAME)

#include "graphics/opengl/system_headers.h"

#include "common/system.h"
#include "math/glmath.h"
#include "watchmaker/fonts.h"
#include "watchmaker/game.h"
#include "watchmaker/globvar.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/rect.h"
#include "watchmaker/render.h"
#include "watchmaker/renderer.h"
#include "watchmaker/windows_hacks.h"

namespace Watchmaker {

Renderer::Renderer(WGame *game, sdl_wrapper *wrapper) : sdl(wrapper), _game(game) {
	_workDirs = &game->workDirs;
}

// blitter info
Rect            gBlitterViewport;
gTexture        gScreenBuffer;

void Renderer::initGL() {
	auto windowInfo = this->getScreenInfos();
	// Cut'n paste from https://www.libsdl.org/release/SDL-1.2.15/docs/html/guidevideoopengl.html
	float ratio = (float)windowInfo.width / windowInfo.height;

	glShadeModel(GL_SMOOTH);

	/* Culling. */
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	//glEnable(GL_CULL_FACE); // TODO

	glClearColor(0, 0, 0, 0);
	glViewport(0, 0, windowInfo.width, windowInfo.height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	auto perspectiveMatrix = Math::makePerspectiveMatrix(60.0, ratio, 1.0, 1024.0);;
	glLoadMatrixf(perspectiveMatrix.getData());
}

void Renderer::setVirtualScreen(unsigned int dimX, unsigned int dimY) {
	unsigned int width, height, bpp;

	//calculate aspect ratio
	getScreenInfos(width, height, bpp);

	gAspectX = 1.0f / ((float)dimX / (float)width);
	gAspectY = 1.0f / ((float)dimY / (float)height);
	gInvAspectX = (float)dimX / (float)width;
	gInvAspectY = (float)dimY / (float)height;
}

void Renderer::getScreenInfos(unsigned int &width, unsigned int &height, unsigned int &bpp) const {
	sdl->getWindowSize(width, height);
	bpp = sdl->getBitDepth();
}

WindowInfo Renderer::getScreenInfos() const {
	WindowInfo info;
	getScreenInfos(info.width, info.height, info.bpp);
	return info;
}

bool Renderer::createScreenBuffer() {
	auto windowSize = getScreenInfos();
	_viewport.left = _viewport.top = 0;
	_viewport.right = 800;
	_viewport.bottom = 600;
	warning("TODO: Implement createScreenBuffer");
	return false;
}

bool Renderer::initBlitterViewPort() {
	auto info = getScreenInfos();
	Rect viewport{0, 0, (int)info.width, (int)info.height};

	// This is just leftover, as the original viewport-setting would default to the screenbuffer-size.
	if ((viewport.top == 0) && (viewport.bottom == 0) && (viewport.left == 0) && (viewport.right == 0)) {
		gBlitterViewport.top = 0;
		gBlitterViewport.bottom = gScreenBuffer.DimY;
		gBlitterViewport.left = 0;
		gBlitterViewport.right = gScreenBuffer.DimX;
	} else {
		gBlitterViewport = viewport;
	}
}

void Renderer::setCurCameraViewport(t3dF32 fov, uint8 sup) {

}

void Renderer::showFrame() {
	g_system->updateScreen();
}

int Renderer::rFitX(int x) {
	return (int)floor((float)x * gAspectX);
}
int Renderer::rFitY(int y) {
	return (int)floor((float)y * gAspectY);
}
int Renderer::rInvFitX(int x) {
	return (int)ceil((float)x * gInvAspectX);
}
int Renderer::rInvFitY(int y) {
	return (int)ceil(((float)(y)) * gInvAspectY);
}

void gPrintText(WGame &game, const char *s, uint32 d, uint32 src, uint16 *FontTable, short x, short y) {
	int16 	i=0,nextx,nexty;
	unsigned char c;

	uint32 AddFlag=0,NumRetries=0;

	nextx = nexty = 0;
	while ( (c=s[i])!=0) {
		i++;
		int16 posx, posy, dimx, dimy;
		posx = FontTable[c*4+0];
		posy = FontTable[c*4+1];
		dimx = FontTable[c*4+2];
		dimy = FontTable[c*4+3];

		rBlitter(game, d, src, x + nextx,y + nexty, posx, posy, dimx, dimy);

		nextx += dimx;
	}
}

void Renderer::printText(const char *s, unsigned int dst, FontKind font, FontColor color, uint16 x, uint16 y) {
	auto f = _fonts->fontForKind(font);
	uint32 src = f->color[color];

	if (dst==0)
		gPrintText(*_game, s, NULL, src, f->table, x, y);
	else
		gPrintText(*_game, s, dst, src, f->table, x, y);
}

} // End of namespace Watchmaker

#endif // USE_OPENGL_GAME
