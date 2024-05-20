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

#ifndef BACKENDS_GRAPHICS_IOS_METAL_RENDERBUFFER_H
#define BACKENDS_GRAPHICS_IOS_METAL_RENDERBUFFER_H

#include "backends/graphics/metal/framebuffer.h"


namespace Metal {

/**
 * Render to renderbuffer framebuffer implementation.
 *
 * This target allows to render to a renderbuffer, which can then be used as
 * a rendering source like expected on iOS.
 */
class MetalRenderbufferTarget : public Framebuffer {
public:
	MetalRenderbufferTarget(MTL::Texture *targetTexture);
	~MetalRenderbufferTarget() override;
	
	/**
	 * Set size of the render target.
	 */
	bool setSize(uint width, uint height) override;
	/**
	 * Updates the render target.
	 */
	void updateRenderBuffer(MTL::Texture* newTargetTexture);

	MTL::Texture *getTargetTexture() override;
	
protected:
	void activateInternal() override {};
	void deactivateInternal() override {};
	
private:
	MTL::Texture *_targetTexture;
};

} // End of namespace Metal

#endif

