/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

#if defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/shadow_volume_opengl_shader.h"

namespace Wintermute {

#include "common/pack-start.h"

struct ShadowVertexShader {
	float x;
	float y;
} PACKED_STRUCT;

#include "common/pack-end.h"

//////////////////////////////////////////////////////////////////////////
ShadowVolumeOpenGLShader::ShadowVolumeOpenGLShader(BaseGame *inGame, OpenGL::ShaderGL *volumeShader, OpenGL::ShaderGL *maskShader)
	: ShadowVolume(inGame), _color(0x7f000000), _volumeShader(volumeShader), _maskShader(maskShader) {
	ShadowVertexShader shadowMask[4];
	Rect32 viewport = _gameRef->_renderer->getViewPort();

	shadowMask[0].x = viewport.left;
	shadowMask[0].y = viewport.bottom;

	shadowMask[1].x = viewport.left;
	shadowMask[1].y = viewport.top;

	shadowMask[2].x = viewport.right;
	shadowMask[2].y = viewport.bottom;

	shadowMask[3].x = viewport.right;
	shadowMask[3].y = viewport.top;

	glGenBuffers(1, &_shadowMaskVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _shadowMaskVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(ShadowVertexShader), shadowMask, GL_DYNAMIC_DRAW);
}

//////////////////////////////////////////////////////////////////////////
ShadowVolumeOpenGLShader::~ShadowVolumeOpenGLShader() {
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolumeOpenGLShader::render() {
	glBindTexture(GL_TEXTURE_2D, 0);
	glDrawArrays(GL_TRIANGLES, 0, _vertices.size());

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolumeOpenGLShader::renderToStencilBuffer() {
	// since the vertex count of the volume might change,
	// we just create a new buffer per frame
	// we might as well use the number of vertices of the mesh as an upper bound
	// or get rid of this completely by moving everything onto the gpu
	glDeleteBuffers(1, &_shadowVolumeVertexBuffer);
	glGenBuffers(1, &_shadowVolumeVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _shadowVolumeVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 12 * _vertices.size(), _vertices.data(), GL_STATIC_DRAW);

	_volumeShader->enableVertexAttribute("position", _shadowVolumeVertexBuffer, 3, GL_FLOAT, false, 12, 0);
	_volumeShader->use(true);

	// Disable z-buffer writes (note: z-testing still occurs), and enable the
	// stencil-buffer
	glDepthMask(GL_FALSE);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_CULL_FACE);

	// Set up stencil compare fuction, reference value, and masks.
	// Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true.
	// Note: since we set up the stencil-test to always pass, the STENCILFAIL
	// renderstate is really not needed.
	glStencilFunc(GL_ALWAYS, 0x1, 0xFFFFFFFF);

	// Make sure that no pixels get drawn to the frame buffer
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_ONE);

	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

	// Draw back-side of shadow volume in stencil/z only
	glCullFace(GL_FRONT);
	render();

	// Decrement stencil buffer value
	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);

	// Draw front-side of shadow volume in stencil/z only
	glCullFace(GL_BACK);
	render();

	// Restore render states
	glFrontFace(GL_CCW);
	glDepthMask(GL_TRUE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolumeOpenGLShader::renderToScene() {
	initMask();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Only write where stencil val >= 1 (count indicates # of shadows that overlap that pixel)
	glStencilFunc(GL_LEQUAL, 0x1, 0xFFFFFFFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	_gameRef->_renderer3D->setProjection2D();

	glBindTexture(GL_TEXTURE_2D, 0);

	_maskShader->enableVertexAttribute("position", _shadowMaskVertexBuffer, 2, GL_FLOAT, false, 8, 0);
	_maskShader->use(true);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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
bool ShadowVolumeOpenGLShader::initMask() {
	Rect32 viewport = _gameRef->_renderer->getViewPort();

	ShadowVertexShader shadowMask[4];

	shadowMask[0].x = viewport.left;
	shadowMask[0].y = viewport.bottom;

	shadowMask[1].x = viewport.left;
	shadowMask[1].y = viewport.top;

	shadowMask[2].x = viewport.right;
	shadowMask[2].y = viewport.bottom;

	shadowMask[3].x = viewport.right;
	shadowMask[3].y = viewport.top;

	glBindBuffer(GL_ARRAY_BUFFER, _shadowMaskVertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(ShadowVertexShader), shadowMask);

	Math::Vector4d color;

	color.x() = RGBCOLGetR(_color) / 255.0f;
	color.y() = RGBCOLGetG(_color) / 255.0f;
	color.z() = RGBCOLGetB(_color) / 255.0f;
	color.w() = RGBCOLGetA(_color) / 255.0f;

	_maskShader->use();
	_maskShader->setUniform("color", color);

	return true;
}

} // namespace Wintermute

#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
