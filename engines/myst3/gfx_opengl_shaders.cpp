/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Matrix calculations taken from the glm library
// Which is covered by the MIT license
// And has this additional copyright note:
/* Copyright (c) 2005 - 2012 G-Truc Creation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 */

#if defined(WIN32)
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one...
#undef ARRAYSIZE
#endif

#include "common/config-manager.h"
#include "common/rect.h"
#include "common/textconsole.h"

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

#include "graphics/colormasks.h"
#include "graphics/pixelbuffer.h"
#include "graphics/surface.h"

#include "math/glmath.h"
#include "math/vector2d.h"
#include "math/rect2d.h"
#include "math/quat.h"

#include "graphics/opengles2/extensions.h"
#include "graphics/opengles2/shader.h"

#include "engines/myst3/gfx.h"
#include "engines/myst3/gfx_opengl_texture.h"
#include "engines/myst3/gfx_opengl_shaders.h"

namespace Myst3 {

Renderer *CreateGfxOpenGLShader(OSystem *system) {
	return new ShaderRenderer(system);
}

static const GLfloat boxVertices[] = {
	// XS   YT
	0.0, 0.0,
	1.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
};

void ShaderRenderer::setupQuadEBO() {
	unsigned short quadIndices[6 * 100];

	unsigned short start = 0;
	for (unsigned short *p = quadIndices; p < &quadIndices[6 * 100]; p += 6) {
		p[0] = p[3] = start++;
		p[1] = start++;
		p[2] = p[4] = start++;
		p[5] = start++;
	}

	_quadEBO = Graphics::Shader::createBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
}

Math::Vector2d ShaderRenderer::scaled(float x, float y) const {
	return Math::Vector2d(x / _currentViewport.getWidth(), y / _currentViewport.getHeight());
}

ShaderRenderer::ShaderRenderer(OSystem *system) :
		BaseRenderer(system),
		_prevText(""),
		_prevTextPosition(0,0),
		_currentViewport(Math::Vector2d(0.0, 0.0), Math::Vector2d(kOriginalWidth, kOriginalHeight)),
		_boxShader(nullptr),
		_cubeShader(nullptr),
		_rect3dShader(nullptr),
		_textShader(nullptr),
		_boxVBO(0),
		_cubeVBO(0),
		_rect3dVBO(0),
		_textVBO(0),
		_quadEBO(0) {
}

ShaderRenderer::~ShaderRenderer() {
	Graphics::Shader::freeBuffer(_boxVBO);
	Graphics::Shader::freeBuffer(_cubeVBO);
	Graphics::Shader::freeBuffer(_rect3dVBO);
	Graphics::Shader::freeBuffer(_textVBO);
	Graphics::Shader::freeBuffer(_quadEBO);

	delete _boxShader;
	delete _cubeShader;
	delete _rect3dShader;
	delete _textShader;
}

Texture *ShaderRenderer::createTexture(const Graphics::Surface *surface) {
	OpenGLTexture *texture = new OpenGLTexture(surface, true);

#if defined(USE_GLES2)
	texture->setUnpackSubImageSupport(Graphics::isExtensionSupported("GL_EXT_unpack_subimage"));
#endif

	return texture;
}

void ShaderRenderer::freeTexture(Texture *texture) {
	OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);
	delete glTexture;
}

void ShaderRenderer::init() {
	debug("Initializing OpenGL Renderer with shaders");

	bool fullscreen = ConfMan.getBool("fullscreen");
	_system->setupScreen(kOriginalWidth, kOriginalHeight, fullscreen, true);
	computeScreenViewport();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	static const char* attributes[] = { "position", "texcoord", NULL };
	_boxShader = Graphics::Shader::fromFiles("myst3_box", attributes);
	_boxVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices);
	_boxShader->enableVertexAttribute("position", _boxVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);
	_boxShader->enableVertexAttribute("texcoord", _boxVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);

	_cubeShader = Graphics::Shader::fromFiles("myst3_cube", attributes);
	_cubeVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices);
	_cubeShader->enableVertexAttribute("texcoord", _cubeVBO, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(float), 0);
	_cubeShader->enableVertexAttribute("position", _cubeVBO, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 2 * sizeof(float));

	_rect3dShader = Graphics::Shader::fromFiles("myst3_cube", attributes);
	_rect3dVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, 20 * sizeof(float), NULL);
	_rect3dShader->enableVertexAttribute("texcoord", _rect3dVBO, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(float), 0);
	_rect3dShader->enableVertexAttribute("position", _rect3dVBO, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 2 * sizeof(float));

	_textShader = Graphics::Shader::fromFiles("myst3_text", attributes);
	_textVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, 100 * 16 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	_textShader->enableVertexAttribute("texcoord", _textVBO, 2, GL_FLOAT, GL_TRUE, 4 * sizeof(float), 0);
	_textShader->enableVertexAttribute("position", _textVBO, 2, GL_FLOAT, GL_TRUE, 4 * sizeof(float), 2 * sizeof(float));

	setupQuadEBO();
}

void ShaderRenderer::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ShaderRenderer::setupCameraOrtho2D(bool noScaling) {
	if (noScaling) {
		glViewport(0, 0, _system->getWidth(), _system->getHeight());
		_currentViewport = Math::Rect2d(Math::Vector2d(0, 0), Math::Vector2d(_system->getWidth(), _system->getHeight()));
	} else {
		glViewport(_screenViewport.left, _screenViewport.top, _screenViewport.width(), _screenViewport.height());
		_currentViewport = Math::Rect2d(Math::Vector2d(0, 0), Math::Vector2d(kOriginalWidth, kOriginalHeight));
	}
}

void ShaderRenderer::setupCameraPerspective(float pitch, float heading, float fov) {
	BaseRenderer::setupCameraPerspective(pitch, heading, fov);

	Common::Rect frame = frameViewport();
	glViewport(frame.left, frame.top, frame.width(), frame.height());
}

void ShaderRenderer::drawRect2D(const Common::Rect &rect, uint32 color) {
	uint8 a, r, g, b;
	Graphics::colorToARGB< Graphics::ColorMasks<8888> >(color, a, r, g, b);

	_boxShader->use();
	_boxShader->setUniform("textured", false);
	_boxShader->setUniform("color", Math::Vector4d(r / 255.0, g / 255.0, b / 255.0, a / 255.0));
	_boxShader->setUniform("verOffsetXY", scaled(rect.left, rect.top));
	_boxShader->setUniform("verSizeWH", scaled(rect.width(), rect.height()));

	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);

	if (a != 255) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void ShaderRenderer::drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect,
		Texture *texture, float transparency, bool additiveBlending) {

	OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);

	const float tLeft = textureRect.left / (float)glTexture->internalWidth;
	const float tWidth = textureRect.width() / (float)glTexture->internalWidth;
	const float tTop = textureRect.top / (float)glTexture->internalHeight;
	const float tHeight = textureRect.height() / (float)glTexture->internalHeight;

	const float sLeft = screenRect.left;
	const float sTop = screenRect.top;
	const float sWidth = screenRect.width();
	const float sHeight = screenRect.height();

	if (transparency >= 0.0) {
		if (additiveBlending) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		} else {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		glEnable(GL_BLEND);
	} else {
		transparency = 1.0;
	}

	_boxShader->use();
	_boxShader->setUniform("textured", true);
	_boxShader->setUniform("color", Math::Vector4d(1.0f, 1.0f, 1.0f, transparency));
	_boxShader->setUniform("verOffsetXY", scaled(sLeft, sTop));
	_boxShader->setUniform("verSizeWH", scaled(sWidth, sHeight));
	_boxShader->setUniform("texOffsetXY", Math::Vector2d(tLeft, tTop));
	_boxShader->setUniform("texSizeWH", Math::Vector2d(tWidth, tHeight));

	glEnable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);

	glBindTexture(GL_TEXTURE_2D, glTexture->id);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void ShaderRenderer::draw2DText(const Common::String &text, const Common::Point &position) {
	OpenGLTexture *glFont = static_cast<OpenGLTexture *>(_font);

	// The font only has uppercase letters
	Common::String textToDraw = text;
	textToDraw.toUppercase();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	if (_prevText != textToDraw || _prevTextPosition != position) {
		_prevText = textToDraw;
		_prevTextPosition = position;

		float x = position.x / _currentViewport.getWidth();
		float y = position.y / _currentViewport.getHeight();

		float *bufData = new float[16 * textToDraw.size()];
		float *cur = bufData;

		for (uint i = 0; i < textToDraw.size(); i++) {
			Common::Rect textureRect = getFontCharacterRect(textToDraw[i]);
			float w = textureRect.width() / _currentViewport.getWidth();
			float h = textureRect.height() / _currentViewport.getHeight();

			float cw = textureRect.width() / (float)glFont->internalWidth;
			float ch = textureRect.height() / (float)glFont->internalHeight;
			float cx = textureRect.left / (float)glFont->internalWidth;
			float cy = textureRect.top / (float)glFont->internalHeight;

			const float charData[] = {
				cx,      cy + ch, x,     y,    
				cx + cw, cy + ch, x + w, y,    
				cx + cw, cy,      x + w, y + h,
				cx,      cy,      x,     y + h,
			};

			memcpy(cur, charData, 16 * sizeof(float));
			cur += 16;

			x += (textureRect.width() - 3) / _currentViewport.getWidth();
		}

		glBindBuffer(GL_ARRAY_BUFFER, _textVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, textToDraw.size() * 16 * sizeof(float), bufData);
		delete[] bufData;
	}

	_textShader->use();
	glBindTexture(GL_TEXTURE_2D, glFont->id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadEBO);
	glDrawElements(GL_TRIANGLES, 6 * textToDraw.size(), GL_UNSIGNED_SHORT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

void ShaderRenderer::drawCube(Texture **textures) {
	OpenGLTexture *texture0 = static_cast<OpenGLTexture *>(textures[0]);

	glEnable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);

	_cubeShader->use();
	_cubeShader->setUniform1f("texScale", texture0->width / (float) texture0->internalWidth);
	_cubeShader->setUniform("mvpMatrix", _mvpMatrix);

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[0])->id);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[1])->id);
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[2])->id);
	glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[3])->id);
	glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[4])->id);
	glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[5])->id);
	glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);

	glDepthMask(GL_TRUE);
}

void ShaderRenderer::drawTexturedRect3D(const Math::Vector3d &topLeft, const Math::Vector3d &bottomLeft,
		const Math::Vector3d &topRight, const Math::Vector3d &bottomRight, Texture *texture) {
	OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);

	const float w = glTexture->width / (float)glTexture->internalWidth;
	const float h = glTexture->height / (float)glTexture->internalHeight;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	glBindTexture(GL_TEXTURE_2D, glTexture->id);

	const GLfloat vertices[] = {
		// S   T         X                 Y                 Z
		   0,  0,  -topLeft.x(),      topLeft.y(),      topLeft.z(),
		   0,  h,  -bottomLeft.x(),   bottomLeft.y(),   bottomLeft.z(),
		   w,  0,  -topRight.x(),     topRight.y(),     topRight.z(),
		   w,  h,  -bottomRight.x(),  bottomRight.y(),  bottomRight.z(),
	};

	_rect3dShader->use();
	_rect3dShader->setUniform1f("texScale", 1.0f);
	_rect3dShader->setUniform("mvpMatrix", _mvpMatrix);
	glBindBuffer(GL_ARRAY_BUFFER, _rect3dVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 20 * sizeof(float), vertices);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

Graphics::Surface *ShaderRenderer::getScreenshot() {
	Common::Rect screen = viewport();

	Graphics::Surface *s = new Graphics::Surface();
	s->create(screen.width(), screen.height(), Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

#if defined(USE_GLES2)
	GLenum format = GL_UNSIGNED_BYTE;
#else
	GLenum format = GL_UNSIGNED_INT_8_8_8_8_REV;
#endif

	glReadPixels(screen.left, screen.top, screen.width(), screen.height(), GL_RGBA, format, s->getPixels());

#if defined(USE_GLES2) && defined(SCUMM_BIG_ENDIAN)
	// OpenGL ES does not support the GL_UNSIGNED_INT_8_8_8_8_REV texture format, we need to byteswap the surface
	OpenGLTexture::byteswapSurface(s);
#endif

	flipVertical(s);

	return s;
}

} // End of namespace Myst3

#endif
