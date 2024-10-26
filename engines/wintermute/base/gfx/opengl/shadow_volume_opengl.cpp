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

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/dcgf.h"

#include "graphics/opengl/system_headers.h"

#if defined(USE_OPENGL_GAME)

#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/shadow_volume_opengl.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
ShadowVolumeOpenGL::ShadowVolumeOpenGL(BaseGame *inGame) : ShadowVolume(inGame) {
}

//////////////////////////////////////////////////////////////////////////
ShadowVolumeOpenGL::~ShadowVolumeOpenGL() {
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolumeOpenGL::render() {
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, _vertices.data());
	glDrawArrays(GL_TRIANGLES, 0, _vertices.size());
	glDisableClientState(GL_VERTEX_ARRAY);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolumeOpenGL::renderToStencilBuffer() {
	// Disable z-buffer writes (note: z-testing still occurs), and enable the
	// stencil-buffer
	glDepthMask(GL_FALSE);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_CULL_FACE);

	// Set up stencil compare fuction, reference value, and masks.
	// Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true.
	// Note: since we set up the stencil-test to always pass, the STENCILFAIL
	// renderstate is really not needed.
	glStencilFunc(GL_ALWAYS, 0x1, 0xFFFFFFFF);

	glShadeModel(GL_FLAT);
	// Make sure that no pixels get drawn to the frame buffer
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_ONE);

	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

	// Draw back-side of shadow volume in stencil/z only
	glFrontFace(GL_CCW);
	render();

	// Decrement stencil buffer value
	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);

	// Draw front-side of shadow volume in stencil/z only
	glFrontFace(GL_CW);
	render();

	// Restore render states
	glEnable(GL_LIGHTING);
	glFrontFace(GL_CCW);
	glShadeModel(GL_SMOOTH);
	glDepthMask(GL_TRUE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolumeOpenGL::renderToScene() {
	initMask();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Only write where stencil val >= 1 (count indicates # of shadows that overlap that pixel)
	glStencilFunc(GL_LEQUAL, 0x1, 0xFFFFFFFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glDisable(GL_ALPHA_TEST);

	_gameRef->_renderer3D->setProjection2D();

	glBindTexture(GL_TEXTURE_2D, 0);

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	// Draw a big, gray square
	glVertexPointer(3, GL_FLOAT, sizeof(ShadowVertex), &_shadowMask[0].x);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ShadowVertex), &_shadowMask[0].r);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	// Restore render states
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	_gameRef->_renderer3D->setup3D(nullptr, true);

	// clear stencil buffer
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolumeOpenGL::initMask() {
	DXViewport viewport = _gameRef->_renderer3D->getViewPort();

	_shadowMask[0].x = viewport._x;
	_shadowMask[0].y = viewport._height;
	_shadowMask[0].z = 0.0f;

	_shadowMask[1].x = viewport._x;
	_shadowMask[1].y = viewport._y;
	_shadowMask[1].z = 0.0f;

	_shadowMask[2].x = viewport._width;
	_shadowMask[2].y = viewport._height;
	_shadowMask[2].z = 0.0f;

	_shadowMask[3].x = viewport._width;
	_shadowMask[3].y = viewport._y;
	_shadowMask[3].z = 0.0f;

	byte a = RGBCOLGetA(_color);
	byte r = RGBCOLGetR(_color);
	byte g = RGBCOLGetG(_color);
	byte b = RGBCOLGetB(_color);

	for (int i = 0; i < 4; ++i) {
		_shadowMask[i].r = r;
		_shadowMask[i].g = g;
		_shadowMask[i].b = b;
		_shadowMask[i].a = a;
	}

	return true;
}

} // namespace Wintermute

#endif // defined(USE_OPENGL_GAME)

