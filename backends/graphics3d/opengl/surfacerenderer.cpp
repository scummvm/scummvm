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

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

#include "backends/graphics3d/opengl/surfacerenderer.h"
#include "graphics/opengl/texture.h"

#include "graphics/opengl/context.h"

#if defined(USE_OPENGL_SHADERS)
#include "graphics/opengl/shader.h"
#endif

namespace OpenGL {

SurfaceRenderer *createBestSurfaceRenderer() {
#if defined(USE_OPENGL_SHADERS)
	if (OpenGLContext.shadersSupported) {
		return new ShaderSurfaceRenderer();
	}
#endif

#if defined(USE_OPENGL_GAME)
	return new FixedSurfaceRenderer();
#else
	error("Could not create an appropriate surface renderer for the current OpenGL context");
#endif
}

struct SurfaceVertex {
	float x;
	float y;
	float u;
	float v;
};

SurfaceRenderer::SurfaceRenderer() :
		_flipY(false),
		_alphaBlending(false) {
}

SurfaceRenderer::~SurfaceRenderer() {
}

void SurfaceRenderer::setFlipY(bool flipY) {
	_flipY = flipY;
}

void SurfaceRenderer::enableAlphaBlending(bool enable) {
	if (_alphaBlending != enable) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}
	_alphaBlending = enable;
}

#if defined(USE_OPENGL_GAME)

FixedSurfaceRenderer::~FixedSurfaceRenderer() {
}

void FixedSurfaceRenderer::prepareState() {
	// Save current state
	glPushAttrib(GL_TRANSFORM_BIT | GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_SCISSOR_BIT | GL_PIXEL_MODE_BIT | GL_TEXTURE_BIT);

	// prepare view
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1.0, 1.0, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDepthMask(GL_FALSE);
}

void FixedSurfaceRenderer::render(const Texture *tex, const Math::Rect2d &dest) {
	float texcropX = tex->getLogicalWidth() / float(tex->getWidth());
	float texcropY = tex->getLogicalHeight() / float(tex->getHeight());
	float texTop    = _flipY ? 0.0f : texcropY;
	float texBottom = _flipY ? texcropY : 0.0f;

	float offsetX = dest.getTopLeft().getX();
	float offsetY = _flipY ? dest.getTopLeft().getY() : 1.0f - dest.getTopLeft().getY() - dest.getHeight();
	float sizeX   = fabsf(dest.getWidth());
	float sizeY   = fabsf(dest.getHeight());

	SurfaceVertex vertices[4];
	vertices[0].x = offsetX;
	vertices[0].y = offsetY;
	vertices[0].u = 0.0f;
	vertices[0].v = texTop;
	vertices[1].x = offsetX + sizeX;
	vertices[1].y = offsetY;
	vertices[1].u = texcropX;
	vertices[1].v = texTop;
	vertices[2].x = offsetX + sizeX;
	vertices[2].y = offsetY + sizeY;
	vertices[2].u = texcropX;
	vertices[2].v = texBottom;
	vertices[3].x = offsetX;
	vertices[3].y = offsetY + sizeY;
	vertices[3].u = 0.0f;
	vertices[3].v = texBottom;

	glColor4f(1.0, 1.0, 1.0, 1.0);

	tex->bind();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, sizeof(SurfaceVertex), &vertices[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SurfaceVertex), &vertices[0].u);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void FixedSurfaceRenderer::restorePreviousState() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glPopAttrib();

	_flipY = false;
	_alphaBlending = false;
}

#endif

#if defined(USE_OPENGL_SHADERS)

static const char *const boxVertex =
	"attribute vec2 position;\n"
	"attribute vec2 texcoord;\n"
	"uniform vec2 offsetXY;\n"
	"uniform vec2 sizeWH;\n"
	"uniform vec2 texcrop;\n"
// OGLES2 on AmigaOS doesn't support uniform booleans
#if defined(__amigaos4__)
	"uniform mediump int flipY;\n"
#else
	"uniform bool flipY;\n"
#endif
	"varying vec2 Texcoord;\n"
	"void main() {\n"
		"Texcoord = texcoord * texcrop;\n"
		"vec2 pos = offsetXY + position * sizeWH;\n"
		"pos.x = pos.x * 2.0 - 1.0;\n"
		"pos.y = pos.y * 2.0 - 1.0;\n"
#if defined(__amigaos4__)
		"if (flipY != 0)\n"
#else
		"if (flipY)\n"
#endif
			"pos.y *= -1.0;\n"
		"gl_Position = vec4(pos, 0.0, 1.0);\n"
	"}\n";

static const char *const boxFragment =
	"#ifdef GL_ES\n"
		"#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
			"precision highp float;\n"
		"#else\n"
			"precision mediump float;\n"
		"#endif\n"
	"#endif\n"
	"varying vec2 Texcoord;\n"
	"uniform sampler2D tex;\n"
	"void main() {\n"
		"gl_FragColor = texture2D(tex, Texcoord);\n"
	"}\n";

ShaderSurfaceRenderer::ShaderSurfaceRenderer() {
	const GLfloat vertices[] = {
		0.0, 0.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
	};

	// Setup the box shader used to render the overlay
	const char *attributes[] = { "position", "texcoord", nullptr };
	_boxShader = Shader::fromStrings("box", boxVertex, boxFragment, attributes);
	_boxVerticesVBO = Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(vertices), vertices);
	_boxShader->enableVertexAttribute("position", _boxVerticesVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);
	_boxShader->enableVertexAttribute("texcoord", _boxVerticesVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);
}

void ShaderSurfaceRenderer::prepareState() {
	_boxShader->use();

	_prevStateDepthTest = glIsEnabled(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);
	glGetBooleanv(GL_DEPTH_WRITEMASK, &_prevStateDepthWriteMask);
	glDepthMask(GL_FALSE);
	_prevStateBlend = glIsEnabled(GL_BLEND);
	glGetIntegerv(GL_BLEND_SRC_RGB, &_prevStateBlendSrcRGB);
	glGetIntegerv(GL_BLEND_DST_RGB, &_prevStateBlendDstRGB);
	glGetIntegerv(GL_BLEND_SRC_ALPHA, &_prevStateBlendSrcAlpha);
	glGetIntegerv(GL_BLEND_DST_ALPHA, &_prevStateBlendDstAlpha);
	glGetIntegerv(GL_VIEWPORT, _prevStateViewport);
	_prevStateScissorTest = glIsEnabled(GL_SCISSOR_TEST);
	glDisable(GL_SCISSOR_TEST);
}

void ShaderSurfaceRenderer::render(const Texture *tex, const Math::Rect2d &dest) {
	tex->bind();

	float texcropX = tex->getLogicalWidth() / float(tex->getWidth());
	float texcropY = tex->getLogicalHeight() / float(tex->getHeight());
	_boxShader->setUniform("texcrop", Math::Vector2d(texcropX, texcropY));
	_boxShader->setUniform("flipY", _flipY);

	_boxShader->setUniform("offsetXY", dest.getTopLeft());
	_boxShader->setUniform("sizeWH", Math::Vector2d(fabsf(dest.getWidth()), fabsf(dest.getHeight())));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ShaderSurfaceRenderer::restorePreviousState() {
	_prevStateDepthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	glDepthMask(_prevStateDepthWriteMask);
	_prevStateScissorTest ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);
	_prevStateBlend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	glBlendFuncSeparate(_prevStateBlendSrcRGB, _prevStateBlendDstRGB,
		_prevStateBlendSrcAlpha, _prevStateBlendDstAlpha);
	glViewport(_prevStateViewport[0], _prevStateViewport[1], _prevStateViewport[2], _prevStateViewport[3]);

	_flipY = false;
	_alphaBlending = false;

	_boxShader->unbind();
}

ShaderSurfaceRenderer::~ShaderSurfaceRenderer() {
	Shader::freeBuffer(_boxVerticesVBO);

	delete _boxShader;
}

#endif

} // End of namespace OpenGL

#endif
