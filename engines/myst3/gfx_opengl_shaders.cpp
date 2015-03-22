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

#include "graphics/opengles2/shader.h"

#include "engines/myst3/gfx.h"
#include "engines/myst3/gfx_opengl_texture.h"
#include "engines/myst3/gfx_opengl_shaders.h"

namespace Myst3 {

Renderer *CreateGfxOpenGLShader(OSystem *system) {
	return new ShaderRenderer(system);
}

static const GLfloat box_vertices[] = {
	// XS   YT
	0.0, 0.0,
	1.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
};

static const GLfloat cube_vertices[] = {
	// S     T      X      Y      Z
	0.0f, 1.0f, -1.0f, -1.0f,  1.0f,
	1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
	0.0f, 0.0f, -1.0f,  1.0f,  1.0f,
	1.0f, 0.0f, -1.0f,  1.0f, -1.0f,
	0.0f, 1.0f,  1.0f, -1.0f, -1.0f,
	1.0f, 1.0f,  1.0f, -1.0f,  1.0f,
	0.0f, 0.0f,  1.0f,  1.0f, -1.0f,
	1.0f, 0.0f,  1.0f,  1.0f,  1.0f,
	0.0f, 1.0f,  1.0f, -1.0f, -1.0f,
	1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
	0.0f, 0.0f,  1.0f, -1.0f,  1.0f,
	1.0f, 0.0f, -1.0f, -1.0f,  1.0f,
	0.0f, 1.0f,  1.0f,  1.0f,  1.0f,
	1.0f, 1.0f, -1.0f,  1.0f,  1.0f,
	0.0f, 0.0f,  1.0f,  1.0f, -1.0f,
	1.0f, 0.0f, -1.0f,  1.0f, -1.0f,
	0.0f, 1.0f, -1.0f, -1.0f, -1.0f,
	1.0f, 1.0f,  1.0f, -1.0f, -1.0f,
	0.0f, 0.0f, -1.0f,  1.0f, -1.0f,
	1.0f, 0.0f,  1.0f,  1.0f, -1.0f,
	0.0f, 1.0f,  1.0f, -1.0f,  1.0f,
	1.0f, 1.0f, -1.0f, -1.0f,  1.0f,
	0.0f, 0.0f,  1.0f,  1.0f,  1.0f,
	1.0f, 0.0f, -1.0f,  1.0f,  1.0f,
};

void ShaderRenderer::setupQuadEBO() {
	unsigned short quad_indices[6 * 100];

	unsigned short start = 0;
	for (unsigned short *p = quad_indices; p < &quad_indices[6 * 100]; p += 6) {
		p[0] = p[3] = start++;
		p[1] = start++;
		p[2] = p[4] = start++;
		p[5] = start++;
	}

	_quadEBO = Graphics::Shader::createBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);
}

Math::Vector2d ShaderRenderer::scaled(float x, float y) const {
	return Math::Vector2d(x / _currentViewport.getWidth(), y / _currentViewport.getHeight());
}

ShaderRenderer::ShaderRenderer(OSystem *system) :
		BaseRenderer(system),
		_prevText(""),
		_prevTextPosition(0,0),
		_currentViewport(Math::Vector2d(0.0, 0.0), Math::Vector2d(kOriginalWidth, kOriginalHeight)),
		_box_shader(nullptr),
		_cube_shader(nullptr),
		_rect3d_shader(nullptr),
		_text_shader(nullptr),
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

	delete _box_shader;
	delete _cube_shader;
	delete _rect3d_shader;
	delete _text_shader;
}

Texture *ShaderRenderer::createTexture(const Graphics::Surface *surface) {
	return new OpenGLTexture(surface);
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
	_box_shader = Graphics::Shader::fromFiles("myst3_box", attributes);
	_boxVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(box_vertices), box_vertices);
	_box_shader->enableVertexAttribute("position", _boxVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);
	_box_shader->enableVertexAttribute("texcoord", _boxVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);

	_cube_shader = Graphics::Shader::fromFiles("myst3_cube", attributes);
	_cubeVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices);
	_cube_shader->enableVertexAttribute("texcoord", _cubeVBO, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(float), 0);
	_cube_shader->enableVertexAttribute("position", _cubeVBO, 3, GL_FLOAT, GL_TRUE, 5 * sizeof(float), 2 * sizeof(float));

	_rect3d_shader = Graphics::Shader::fromFiles("myst3_cube", attributes);
	_rect3dVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, 20 * sizeof(float), NULL);
	_rect3d_shader->enableVertexAttribute("texcoord", _rect3dVBO, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(float), 0);
	_rect3d_shader->enableVertexAttribute("position", _rect3dVBO, 3, GL_FLOAT, GL_TRUE, 5 * sizeof(float), 2 * sizeof(float));

	_text_shader = Graphics::Shader::fromFiles("myst3_text", attributes);
	_textVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, 100 * 16 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	_text_shader->enableVertexAttribute("texcoord", _textVBO, 2, GL_FLOAT, GL_TRUE, 4 * sizeof(float), 0);
	_text_shader->enableVertexAttribute("position", _textVBO, 2, GL_FLOAT, GL_TRUE, 4 * sizeof(float), 2 * sizeof(float));

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

	_box_shader->use();
	_box_shader->setUniform("textured", false);
	_box_shader->setUniform("color", Math::Vector4d(r / 255.0, g / 255.0, b / 255.0, a / 255.0));
	_box_shader->setUniform("verOffsetXY", scaled(rect.left, rect.top));
	_box_shader->setUniform("verSizeWH", scaled(rect.width(), rect.height()));

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

	_box_shader->use();
	_box_shader->setUniform("textured", true);
	_box_shader->setUniform("color", Math::Vector4d(1.0f, 1.0f, 1.0f, transparency));
	_box_shader->setUniform("verOffsetXY", scaled(sLeft, sTop));
	_box_shader->setUniform("verSizeWH", scaled(sWidth, sHeight));
	_box_shader->setUniform("texOffsetXY", Math::Vector2d(tLeft, tTop));
	_box_shader->setUniform("texSizeWH", Math::Vector2d(tWidth, tHeight));

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

	_text_shader->use();
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

	_cube_shader->use();
	_cube_shader->setUniform1f("verScale", 256.0f);
	_cube_shader->setUniform1f("texScale", texture0->width / (float) texture0->internalWidth);
	_cube_shader->setUniform1f("texClamp", (texture0->width - 1) / (float) texture0->internalWidth);
	_cube_shader->setUniform("mvpMatrix", _mvpMatrix);

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[4])->id);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[3])->id);
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[1])->id);
	glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[5])->id);
	glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[0])->id);
	glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[2])->id);
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

	_rect3d_shader->use();
	_rect3d_shader->setUniform1f("verScale", 1.0f);
	_rect3d_shader->setUniform1f("texScale", 1.0f);
	_rect3d_shader->setUniform1f("texClamp", 1.0f);
	_rect3d_shader->setUniform("mvpMatrix", _mvpMatrix);
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

	glReadPixels(screen.left, screen.top, screen.width(), screen.height(), GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, s->getPixels());

	flipVertical(s);

	return s;
}

} // End of namespace Myst3

#endif
