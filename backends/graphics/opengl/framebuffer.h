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

#ifndef BACKENDS_GRAPHICS_OPENGL_FRAMEBUFFER_H
#define BACKENDS_GRAPHICS_OPENGL_FRAMEBUFFER_H

#include "graphics/opengl/system_headers.h"

#include "math/matrix4.h"

#include "common/rotationmode.h"

namespace OpenGL {

class Pipeline;

/**
 * Object describing a framebuffer OpenGL can render to.
 */
class Framebuffer {
public:
	Framebuffer();
	virtual ~Framebuffer() {};

public:
	enum BlendMode {
		/**
		 * Newly drawn pixels overwrite the existing contents of the framebuffer
		 * without mixing with them.
		 */
		kBlendModeDisabled,

		/**
		 * Newly drawn pixels overwrite the existing contents of the framebuffer
		 * without mixing with them. Alpha channel is discarded.
		 */
		kBlendModeOpaque,

		/**
		 * Newly drawn pixels mix with the framebuffer based on their alpha value
		 * for transparency.
		 */
		kBlendModeTraditionalTransparency,

		/**
		 * Newly drawn pixels mix with the framebuffer based on their alpha value
		 * for transparency.
		 *
		 * Requires the image data being drawn to have its color values pre-multiplied
		 * with the alpha value.
		 */
		kBlendModePremultipliedTransparency,

		/**
		 * Newly drawn pixels add to the destination value.
		 */
		kBlendModeAdditive,

		/**
		 * Newly drawn pixels mask out existing pixels based on the alpha value and
		 * add inversions of the pixels based on the color.
		 */
		kBlendModeMaskAlphaAndInvertByColor,
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
	const Math::Matrix4 &getProjectionMatrix() const { return _projectionMatrix; }

	enum CopyMask {
		kCopyMaskClearColor   = (1 << 0),
		kCopyMaskBlendState   = (1 << 1),
		kCopyMaskScissorState = (1 << 2),
		kCopyMaskScissorBox   = (1 << 4),

		kCopyMaskAll          = kCopyMaskClearColor | kCopyMaskBlendState |
		                        kCopyMaskScissorState | kCopyMaskScissorBox,
	};

	/**
	 * Copy rendering state from another framebuffer
	 */
	void copyRenderStateFrom(const Framebuffer &other, uint copyMask);

protected:
	bool isActive() const { return _pipeline != nullptr; }

	GLint _viewport[4];
	void applyViewport();

	Math::Matrix4 _projectionMatrix;
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

public:
	/**
	 * Set the size of the target buffer.
	 */
	virtual bool setSize(uint width, uint height, Common::RotationMode rotation) = 0;

	/**
	 * Accessor to activate framebuffer for pipeline.
	 */
	void activate(Pipeline *pipeline);

	/**
	 * Accessor to deactivate framebuffer from pipeline.
	 */
	void deactivate();

private:
	Pipeline *_pipeline;

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
	 * Set the size of the back buffer.
	 */
	bool setSize(uint width, uint height, Common::RotationMode rotation) override;

protected:
	void activateInternal() override;
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
	~TextureTarget() override;

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
	bool setSize(uint width, uint height, Common::RotationMode rotation) override;

	/**
	 * Query pointer to underlying GL texture.
	 */
	GLTexture *getTexture() const { return _texture; }

protected:
	void activateInternal() override;

private:
	GLTexture *_texture;
	GLuint _glFBO;
	bool _needUpdate;
};
#endif

} // End of namespace OpenGL

#endif
