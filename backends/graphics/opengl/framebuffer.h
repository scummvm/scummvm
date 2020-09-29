/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BACKENDS_GRAPHICS_OPENGL_FRAMEBUFFER_H
#define BACKENDS_GRAPHICS_OPENGL_FRAMEBUFFER_H

#include "backends/graphics/opengl/opengl-sys.h"

namespace OpenGL {

/**
 * Object describing a framebuffer OpenGL can render to.
 */
class Framebuffer {
	friend class Pipeline;
public:
	Framebuffer();
	virtual ~Framebuffer() {};

public:
	enum BlendMode {
		/**
		 * Newly drawn pixels overwrite the existing contents of the framebuffer
		 * without mixing with them
		 */
		kBlendModeDisabled,

		/**
		 * Newly drawn pixels mix with the framebuffer based on their alpha value
		 * for transparency.
		 */
		kBlendModeTraditionalTransparency,

		/**
		 * Newly drawn pixels mix with the framebuffer based on their alpha value
		 * for transparency.
		 *
		 * Requires the image data being drawn to have its color values pre-multipled
		 * with the alpha value.
		 */
		kBlendModePremultipliedTransparency
	};

	/**
	 * Set the clear color of the framebuffer.
	 */
	void setClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

	/**
	 * Enable/disable GL_BLEND.
	 */
	void enableBlend(BlendMode mode);

	/**
	 * Enable/disable GL_SCISSOR_TEST.
	 */
	void enableScissorTest(bool enable);

	/**
	 * Set scissor box dimensions.
	 */
	void setScissorBox(GLint x, GLint y, GLsizei w, GLsizei h);

	/**
	 * Obtain projection matrix of the framebuffer.
	 */
	const GLfloat *getProjectionMatrix() const { return _projectionMatrix; }
protected:
	bool isActive() const { return _isActive; }

	GLint _viewport[4];
	void applyViewport();

	GLfloat _projectionMatrix[4*4];
	void applyProjectionMatrix();

	/**
	 * Activate framebuffer.
	 *
	 * This is supposed to set all state associated with the framebuffer.
	 */
	virtual void activateInternal() = 0;

	/**
	 * Deactivate framebuffer.
	 *
	 * This is supposed to make any cleanup required when unbinding the
	 * framebuffer.
	 */
	virtual void deactivateInternal() {}

private:
	/**
	 * Accessor to activate framebuffer for pipeline.
	 */
	void activate();

	/**
	 * Accessor to deactivate framebuffer from pipeline.
	 */
	void deactivate();

private:
	bool _isActive;

	GLfloat _clearColor[4];
	void applyClearColor();

	BlendMode _blendState;
	void applyBlendState();

	bool _scissorTestState;
	void applyScissorTestState();

	GLint _scissorBox[4];
	void applyScissorBox();
};

/**
 * Default back buffer implementation.
 */
class Backbuffer : public Framebuffer {
public:
	/**
	 * Set the dimensions (a.k.a. size) of the back buffer.
	 */
	void setDimensions(uint width, uint height);

protected:
	virtual void activateInternal();
};

#if !USE_FORCED_GLES
class GLTexture;

/**
 * Render to texture framebuffer implementation.
 *
 * This target allows to render to a texture, which can then be used for
 * further rendering.
 */
class TextureTarget : public Framebuffer {
public:
	TextureTarget();
	virtual ~TextureTarget();

	/**
	 * Notify that the GL context is about to be destroyed.
	 */
	void destroy();

	/**
	 * Notify that the GL context has been created.
	 */
	void create();

	/**
	 * Set size of the texture target.
	 */
	void setSize(uint width, uint height);

	/**
	 * Query pointer to underlying GL texture.
	 */
	GLTexture *getTexture() const { return _texture; }

protected:
	virtual void activateInternal();

private:
	GLTexture *_texture;
	GLuint _glFBO;
	bool _needUpdate;
};
#endif

} // End of namespace OpenGL

#endif
