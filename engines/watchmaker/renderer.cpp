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
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/fonts.h"
#include "watchmaker/game.h"
#include "watchmaker/globvar.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/rect.h"
#include "watchmaker/render.h"
#include "watchmaker/renderer.h"
#include "watchmaker/windows_hacks.h"

namespace Watchmaker {

Renderer::Renderer(WGame *game, sdl_wrapper *wrapper) : _2dStuff(this), sdl(wrapper), _game(game) {
	_workDirs = &game->workDirs;
}

// blitter info
Common::Rect    gBlitterViewport;
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
	unsigned int width, height;

	//calculate aspect ratio
	getScreenInfos(width, height);

	gAspectX = 1.0f / ((float)dimX / (float)width);
	gAspectY = 1.0f / ((float)dimY / (float)height);
	gInvAspectX = (float)dimX / (float)width;
	gInvAspectY = (float)dimY / (float)height;
}

void Renderer::getScreenInfos(unsigned int &width, unsigned int &height) const {
	sdl->getWindowSize(width, height);
}

WindowInfo Renderer::getScreenInfos() const {
	WindowInfo info;
	getScreenInfos(info.width, info.height);
	return info;
}

bool Renderer::createScreenBuffer() {
	//auto windowSize = getScreenInfos();
	_viewport.left = _viewport.top = 0;
	_viewport.right = 800;
	_viewport.bottom = 600;
	warning("TODO: Implement createScreenBuffer");
	return false;
}

void Renderer::initBlitterViewPort() {
	auto info = getScreenInfos();
	Common::Rect viewport{0, 0, (int16)info.width, (int16)info.height};

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
	auto windowInfo = getScreenInfos();
	int32 cx = windowInfo.width / 2;
	int32 cy = windowInfo.height / 2;
	int32 sx = windowInfo.width;
	int32 sy = windowInfo.height;

	t3dF32 SuperView = 50.0f * (sup ^ 1);

	t3dCurCamera->Center.x = (t3dF32)(cx);
	t3dCurCamera->Center.y = (t3dF32)(cy);

	t3dCurCamera->NearClipPlane = fov;
	t3dCurCamera->FarClipPlane = 89000.0f;

	warning("TODO: Set projection matrix");

	setProjectionMatrix((float)(sx),
	                    (float)(sy),
	                    fov,
	                    10.0f + SuperView, 90000.0f);

	//Set Clipplanes
	t3dV3F c0;
	t3dVectFill(&c0, 0.0f);
	t3dV3F v1(screenSpaceToCameraSpace(0.0f,  0.0f));
	t3dV3F v2(screenSpaceToCameraSpace((t3dF32)sx, 0.0f));
	t3dV3F v3(screenSpaceToCameraSpace(0.0f, (t3dF32)sy));
	t3dV3F v4(screenSpaceToCameraSpace((t3dF32)sx, (t3dF32)sy));

#if 0
	t3dPlaneNormal(&ClipPlanes[LEFTCLIP],   &c0, &v1, &v3);
	t3dPlaneNormal(&ClipPlanes[RIGHTCLIP],  &c0, &v4, &v2);
	t3dPlaneNormal(&ClipPlanes[TOPCLIP],    &c0, &v2, &v1);
	t3dPlaneNormal(&ClipPlanes[BOTTOMCLIP], &c0, &v3, &v4);
#endif
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
	int16   i = 0, nextx, nexty;
	unsigned char c;

	nextx = nexty = 0;
	while ((c = s[i]) != 0) {
		i++;
		int16 posx, posy, dimx, dimy;
		posx = FontTable[c * 4 + 0];
		posy = FontTable[c * 4 + 1];
		dimx = FontTable[c * 4 + 2];
		dimy = FontTable[c * 4 + 3];

		rBlitter(game, d, src, x + nextx, y + nexty, posx, posy, dimx, dimy);

		nextx += dimx;
	}
}

void Renderer::printText(const char *s, unsigned int dst, FontKind font, FontColor color, uint16 x, uint16 y) {
	auto f = _fonts->fontForKind(font);
	uint32 src = f->color[color];

	gPrintText(*_game, s, dst, src, f->table, x, y);
}

bool Renderer::setProjectionMatrix(float width, float height, float fAspect, float fNearPlane, float fFarPlane) {
	// Not sure if fAspect is Y here though
	glMatrixMode(GL_PROJECTION);
	_nearPlane = fNearPlane;
	_projectionMatrix = Math::makePerspectiveMatrix(fAspect, width / height, fNearPlane, fFarPlane);
	glLoadMatrixf(_projectionMatrix.getData());

	glMatrixMode(GL_MODELVIEW);
	return false;
}

Math::Vector3d vector3Matrix4Mult(Math::Vector3d &vec, const Math::Matrix4 &m) {
	// Since the query functions return a 4x4 Matrix, but we only really care
	// about the 3D vector, we make our own version of the Vector-Matrix mult.
	// In practice we could perhaps introduce an operator* in Vector4d, but
	// since the w component is un-interesting that would be 4 unnecessary mults.
	const float *d = m.getData();
	return Math::Vector3d(vec.x() * d[0] + vec.y() * d[3] + vec.z() * d[6],
	                      vec.x() * d[1] + vec.y() * d[4] + vec.z() * d[7],
	                      vec.x() * d[2] + vec.y() * d[5] + vec.z() * d[8]);
}

Math::Vector3d Renderer::screenSpaceToCameraSpace(float x, float y) {
	unsigned int width, height, bpp;

	rGetScreenInfos(&width, &height, &bpp);

	auto matrix = _projectionMatrix;
	matrix.inverse();

	Math::Vector3d v;
	v.x() = (x - width / 2) / (width / 2);
	v.y() = -(y - height / 2) / (height / 2);
	v.z() = 1.0f;
	Math::Vector3d d = vector3Matrix4Mult(v, matrix);

	return Math::Vector3d(d.x(), d.y(), d.z() - _nearPlane);
}


} // End of namespace Watchmaker

#endif // USE_OPENGL_GAME
