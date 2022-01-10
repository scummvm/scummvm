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
#include "backends/graphics3d/opengl/texture.h"

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
	glDepthMask(GL_FALSE);
}

void FixedSurfaceRenderer::render(const TextureGL *tex, const Math::Rect2d &dest) {
	float texcropX = tex->getWidth() / float(tex->getTexWidth());
	float texcropY = tex->getHeight() / float(tex->getTexHeight());
	float texTop    = _flipY ? 0.0f : texcropY;
	float texBottom = _flipY ? texcropY : 0.0f;

	float offsetX = dest.getTopLeft().getX();
	float offsetY = _flipY ? dest.getTopLeft().getY() : 1.0f - dest.getTopLeft().getY() - dest.getHeight();
	float sizeX   = fabsf(dest.getWidth());
	float sizeY   = fabsf(dest.getHeight());

	glColor4f(1.0, 1.0, 1.0, 1.0);

	glBindTexture(GL_TEXTURE_2D, tex->getTextureName());
	glBegin(GL_QUADS);
		glTexCoord2f(0, texTop);
		glVertex2f(offsetX, offsetY);
		glTexCoord2f(texcropX, texTop);
		glVertex2f(offsetX + sizeX, offsetY);
		glTexCoord2f(texcropX, texBottom);
		glVertex2f(offsetX + sizeX, offsetY + sizeY);
		glTexCoord2f(0.0, texBottom);
		glVertex2f(offsetX, offsetY + sizeY);
	glEnd();
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

ShaderSurfaceRenderer::ShaderSurfaceRenderer() {
	const GLfloat vertices[] = {
			0.0, 0.0,
			1.0, 0.0,
			0.0, 1.0,
			1.0, 1.0,
	};

	// Setup the box shader used to render the overlay
	const char *attributes[] = { "position", "texcoord", nullptr };
	_boxShader = ShaderGL::fromStrings("box", BuiltinShaders::boxVertex, BuiltinShaders::boxFragment, attributes);
	_boxVerticesVBO = ShaderGL::createBuffer(GL_ARRAY_BUFFER, sizeof(vertices), vertices);
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
	glGetIntegerv(GL_BLEND_SRC_ALPHA, &_prevStateBlendFunc);
	glGetIntegerv(GL_VIEWPORT, _prevStateViewport);
}

void ShaderSurfaceRenderer::render(const TextureGL *tex, const Math::Rect2d &dest) {
	glBindTexture(GL_TEXTURE_2D, tex->getTextureName());

	float texcropX = tex->getWidth() / float(tex->getTexWidth());
	float texcropY = tex->getHeight() / float(tex->getTexHeight());
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
	_prevStateBlend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	glBlendFunc(GL_BLEND_SRC_ALPHA, _prevStateBlendFunc);
	glViewport(_prevStateViewport[0], _prevStateViewport[1], _prevStateViewport[2], _prevStateViewport[3]);

	_flipY = false;
	_alphaBlending = false;

	_boxShader->unbind();
}

ShaderSurfaceRenderer::~ShaderSurfaceRenderer() {
	ShaderGL::freeBuffer(_boxVerticesVBO);

	delete _boxShader;
}

#endif

} // End of namespace OpenGL

#endif
