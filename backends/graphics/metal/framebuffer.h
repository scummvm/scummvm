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

#ifndef BACKENDS_GRAPHICS_METAL_FRAMEBUFFER_H
#define BACKENDS_GRAPHICS_METAL_FRAMEBUFFER_H

#include "math/matrix4.h"

namespace MTL {
class Device;
class Texture;
struct Viewport;
}

namespace Metal {

/**
 * Object describing a framebuffer Metal can render to.
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
		 * Requires the image data being drawn to have its color values pre-multipled
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
	void setClearColor(float r, float g, float b, float a);

	/**
	 * Enable/disable blending.
	 */
	void enableBlend(BlendMode mode);

	/**
	 * Enable/disable scissoring
	 */
	void enableScissorTest(bool enable);

	/**
	 * Set scissor box dimensions.
	 */
	void setScissorBox(int x, int y, int w, int h);

	/**
	 * Set viewport.
	 */
	void setViewport(int x, int y, int w, int h);

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
	bool isActive() const { return false; }

	MTL::Viewport *_viewport;
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
	virtual bool setSize(uint width, uint height) = 0;

	/**
	 * Accessor to activate framebuffer for pipeline.
	 */
	void activate();

	/**
	 * Accessor to deactivate framebuffer from pipeline.
	 */
	void deactivate();

	virtual MTL::Texture *getTargetTexture() = 0;

protected:
	float _clearColor[4];

private:
	void applyClearColor();

	BlendMode _blendState;
	void applyBlendState();

	bool _scissorTestState;
	void applyScissorTestState();

	int _scissorBox[4];
	void applyScissorBox();
};

class MetalTexture;

/**
 * Render to texture framebuffer implementation.
 *
 * This target allows to render to a texture, which can then be used for
 * further rendering.
 */
class TextureTarget : public Framebuffer {
public:
	TextureTarget(MTL::Device *device);
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
	bool setSize(uint width, uint height) override;

	/**
	 * Query pointer to underlying Metal texture.
	 */
	MetalTexture *getTexture() const { return _texture; }

	MTL::Texture *getTargetTexture() override;


protected:
	void activateInternal() override;

private:
	MetalTexture *_texture;
};

} // End of namespace Metal

#endif

