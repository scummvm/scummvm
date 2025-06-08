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

#ifndef BACKENDS_GRAPHICS_OPENGL_RENDERER3D_H
#define BACKENDS_GRAPHICS_OPENGL_RENDERER3D_H

#include "common/scummsys.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

#include "graphics/opengl/texture.h"

namespace OpenGL {

class Renderer3D {
public:
	Renderer3D();
	~Renderer3D() { destroy(); }

	void initSize(uint w, uint h, int samples, bool renderToFrameBuffer);
	void resize(uint w, uint h);
	void recreate();
	void destroy();

	void leave3D();
	void enter3D();
	void presentBuffer();

	void showOverlay(uint w, uint h);
	void hideOverlay();

	const Texture &getGLTexture() const { return _texture; }
	bool hasTexture() const { return _texture.getGLTexture() != 0; }

	void setRotation(Common::RotationMode rotation) { _texture.setRotation(rotation); }

protected:
	void setup();

	int _stackLevel;
	bool _inOverlay;
	int _pendingScreenChangeWidth;
	int _pendingScreenChangeHeight;

	bool _renderToFrameBuffer;
	int _samples;
	Texture _texture;
	GLuint _renderBuffers[3];
	GLuint _frameBuffers[2];

#define CTX_STATE(gl_param) GLboolean _save ## gl_param
#define CTX_BOOLEAN(gl_param) GLboolean _save ## gl_param
#define CTX_INTEGER(gl_param, count) GLint _save ## gl_param[count]

	CTX_STATE(GL_BLEND);
	CTX_STATE(GL_CULL_FACE);
	CTX_STATE(GL_DEPTH_TEST);
	CTX_STATE(GL_DITHER);
	CTX_STATE(GL_POLYGON_OFFSET_FILL);
	CTX_STATE(GL_SCISSOR_TEST);
	CTX_STATE(GL_STENCIL_TEST);

	CTX_BOOLEAN(GL_DEPTH_WRITEMASK);

	CTX_INTEGER(GL_BLEND_SRC_RGB, 1);
	CTX_INTEGER(GL_BLEND_DST_RGB, 1);
	CTX_INTEGER(GL_BLEND_SRC_ALPHA, 1);
	CTX_INTEGER(GL_BLEND_DST_ALPHA, 1);
	CTX_INTEGER(GL_SCISSOR_BOX, 4);
	CTX_INTEGER(GL_VIEWPORT, 4);

#undef CTX_INTEGER
#undef CTX_BOOLEAN
#undef CTX_STATE
};

} // End of namespace OpenGL

#endif // defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

#endif
