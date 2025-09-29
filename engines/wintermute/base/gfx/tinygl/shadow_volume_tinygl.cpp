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

#if defined(USE_TINYGL)

#include "engines/wintermute/base/gfx/tinygl/base_render_tinygl.h"
#include "engines/wintermute/base/gfx/tinygl/shadow_volume_tinygl.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
ShadowVolumeTinyGL::ShadowVolumeTinyGL(BaseGame *inGame) : ShadowVolume(inGame) {
}

//////////////////////////////////////////////////////////////////////////
ShadowVolumeTinyGL::~ShadowVolumeTinyGL() {
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolumeTinyGL::render() {
	tglBindTexture(TGL_TEXTURE_2D, 0);
	tglDisable(TGL_TEXTURE_2D);
	_game->_renderer3D->_lastTexture = nullptr;

	tglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	tglEnableClientState(TGL_VERTEX_ARRAY);
	tglVertexPointer(3, TGL_FLOAT, 0, _vertices.getData());
	tglDrawArrays(TGL_TRIANGLES, 0, _vertices.getSize());
	tglDisableClientState(TGL_VERTEX_ARRAY);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolumeTinyGL::renderToStencilBuffer() {
	// Disable z-buffer writes (note: z-testing still occurs), and enable the
	// stencil-buffer
	tglDepthMask(TGL_FALSE);
	tglDisable(TGL_TEXTURE_2D);
	tglDisable(TGL_LIGHTING);
	tglEnable(TGL_STENCIL_TEST);
	tglEnable(TGL_CULL_FACE);

	// Set up stencil compare fuction, reference value, and masks.
	// Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true.
	// Note: since we set up the stencil-test to always pass, the STENCILFAIL
	// renderstate is really not needed.
	tglStencilFunc(TGL_ALWAYS, 0x1, 0xFFFFFFFF);

	tglShadeModel(TGL_FLAT);
	// Make sure that no pixels get drawn to the frame buffer
	tglEnable(TGL_BLEND);
	tglBlendFunc(TGL_ZERO, TGL_ONE);

	tglStencilOp(TGL_KEEP, TGL_KEEP, TGL_INCR);

	// Draw back-side of shadow volume in stencil/z only
	tglFrontFace(TGL_CCW);
	render();

	// Decrement stencil buffer value
	tglStencilOp(TGL_KEEP, TGL_KEEP, TGL_DECR);

	// Draw front-side of shadow volume in stencil/z only
	tglFrontFace(TGL_CW);
	render();

	// Restore render states
	tglEnable(TGL_LIGHTING);
	tglFrontFace(TGL_CCW);
	tglShadeModel(TGL_SMOOTH);
	tglDepthMask(TGL_TRUE);
	tglDisable(TGL_STENCIL_TEST);
	tglDisable(TGL_BLEND);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolumeTinyGL::renderToScene() {
	initMask();

	tglDisable(TGL_DEPTH_TEST);
	tglEnable(TGL_STENCIL_TEST);
	tglEnable(TGL_BLEND);
	tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);

	// Only write where stencil val >= 1 (count indicates # of shadows that overlap that pixel)
	tglStencilFunc(TGL_LEQUAL, 0x1, 0xFFFFFFFF);
	tglStencilOp(TGL_KEEP, TGL_KEEP, TGL_KEEP);

	tglDisable(TGL_FOG);
	tglDisable(TGL_LIGHTING);
	tglDisable(TGL_ALPHA_TEST);

	tglBindTexture(TGL_TEXTURE_2D, 0);

	BaseRenderTinyGL *renderer = dynamic_cast<BaseRenderTinyGL *>(_game->_renderer3D);
	renderer->setProjection2D();

	tglFrontFace(TGL_CW);

	tglEnableClientState(TGL_COLOR_ARRAY);
	tglEnableClientState(TGL_VERTEX_ARRAY);

	// Draw a big, gray square
	tglVertexPointer(3, TGL_FLOAT, sizeof(ShadowVertex), &_shadowMask[0].x);
	tglColorPointer(4, TGL_UNSIGNED_BYTE, sizeof(ShadowVertex), &_shadowMask[0].r);

	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);

	tglDisableClientState(TGL_COLOR_ARRAY);
	tglDisableClientState(TGL_VERTEX_ARRAY);

	// Restore render states
	tglEnable(TGL_DEPTH_TEST);
	tglDisable(TGL_STENCIL_TEST);

	_game->_renderer3D->setup3D(nullptr, true);

	// clear stencil buffer
	tglClearStencil(0);
	tglClear(TGL_STENCIL_BUFFER_BIT);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolumeTinyGL::initMask() {
	auto *rend = _game->_renderer3D;

	// bottom left
	_shadowMask[0].x = 0.0f;
	_shadowMask[0].y = rend->getHeight();
	_shadowMask[0].z = 1.0f;

	// top left
	_shadowMask[1].x = 0.0f;
	_shadowMask[1].y = 0.0f;
	_shadowMask[1].z = 1.0f;

	// bottom right
	_shadowMask[2].x = rend->getWidth();
	_shadowMask[2].y = rend->getHeight();
	_shadowMask[2].z = 1.0f;

	// top right
	_shadowMask[3].x = rend->getWidth();
	_shadowMask[3].y = 0.0f;
	_shadowMask[3].z = 1.0f;

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

#endif // defined(USE_TINYGL)
