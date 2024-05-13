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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/graphics/metal/framebuffer.h"
#include "backends/graphics/metal/texture.h"

#include <Metal/Metal.hpp>

namespace Metal {

Framebuffer::Framebuffer()
	: _viewport(new MTL::Viewport()), _projectionMatrix(),
	  _clearColor(),
	  _blendState(kBlendModeDisabled), _scissorBox() {
}

void Framebuffer::activate() {
	applyViewport();
	applyProjectionMatrix();
	applyClearColor();
	applyBlendState();
	applyScissorTestState();
	applyScissorBox();

	activateInternal();
}

void Framebuffer::deactivate() {
	deactivateInternal();
}

void Framebuffer::setClearColor(float r, float g, float b, float a) {
	_clearColor[0] = r;
	_clearColor[1] = g;
	_clearColor[2] = b;
	_clearColor[3] = a;

	// Directly apply changes when we are active.
	if (isActive()) {
		applyClearColor();
	}
}

void Framebuffer::enableBlend(BlendMode mode) {
	_blendState = mode;

	// Directly apply changes when we are active.
	if (isActive()) {
		applyBlendState();
	}
}

void Framebuffer::enableScissorTest(bool enable) {
	_scissorTestState = enable;

	// Directly apply changes when we are active.
	if (isActive()) {
		applyScissorTestState();
	}
}

void Framebuffer::setScissorBox(int x, int y, int w, int h) {
	_scissorBox[0] = x;
	_scissorBox[1] = y;
	_scissorBox[2] = w;
	_scissorBox[3] = h;

	// Directly apply changes when we are active.
	if (isActive()) {
		applyScissorBox();
	}
}

void Framebuffer::setViewport(int x, int y, int w, int h) {
	_viewport->originX = x;
	_viewport->originY = y;
	_viewport->width = w;
	_viewport->height = h;

	// Directly apply changes when we are active.
	if (isActive()) {
		applyViewport();
	}
}

void Framebuffer::applyViewport() {
	//TODO: Implement
}

void Framebuffer::applyProjectionMatrix() {
	//TODO: Implement
}

void Framebuffer::applyClearColor() {
	//TODO: Implement
}

void Framebuffer::applyBlendState() {
	//TODO: Implement
}

void Framebuffer::applyScissorTestState() {
}

void Framebuffer::applyScissorBox() {
	//TODO: Implement
}

void Framebuffer::copyRenderStateFrom(const Framebuffer &other, uint copyMask) {
	if (copyMask & kCopyMaskClearColor) {
		memcpy(_clearColor, other._clearColor, sizeof(_clearColor));
	}
	if (copyMask & kCopyMaskBlendState) {
		_blendState = other._blendState;
	}
	if (copyMask & kCopyMaskScissorBox) {
		memcpy(_scissorBox, other._scissorBox, sizeof(_scissorBox));
	}

	if (isActive()) {
		applyClearColor();
		applyBlendState();
		applyScissorTestState();
		applyScissorBox();
	}
}

//
// Render to texture target implementation
//

TextureTarget::TextureTarget(MTL::Device *device) : _texture(new MetalTexture(device, MTL::PixelFormatRGBA8Unorm, (MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead))) {
}

TextureTarget::~TextureTarget() {
	delete _texture;
}

void TextureTarget::activateInternal() {
}

void TextureTarget::destroy() {
	_texture->destroy();
}

void TextureTarget::create() {
	_texture->create();
}

bool TextureTarget::setSize(uint width, uint height) {
	if (!_texture->setSize(width, height)) {
		return false;
	}

	// Set viewport dimensions.
	_viewport->originX = 0;
	_viewport->originY = 0;
	_viewport->width = width;
	_viewport->height = height;

	// Setup orthogonal projection matrix.
	_projectionMatrix(0, 0) =  2.0f / (float)width;
	_projectionMatrix(0, 1) =  0.0f;
	_projectionMatrix(0, 2) =  0.0f;
	_projectionMatrix(0, 3) =  0.0f;

	_projectionMatrix(1, 0) =  0.0f;
	_projectionMatrix(1, 1) = -2.0f / (float)height;
	_projectionMatrix(1, 2) =  0.0f;
	_projectionMatrix(1, 3) =  0.0f;

	_projectionMatrix(2, 0) =  0.0f;
	_projectionMatrix(2, 1) =  0.0f;
	_projectionMatrix(2, 2) =  0.0f;
	_projectionMatrix(2, 3) =  0.0f;

	_projectionMatrix(3, 0) = -1.0f;
	_projectionMatrix(3, 1) =  1.0f;
	_projectionMatrix(3, 2) =  0.0f;
	_projectionMatrix(3, 3) =  1.0f;

	// Directly apply changes when we are active.
	if (isActive()) {
		applyViewport();
		applyProjectionMatrix();
	}
	return true;
}

MTL::Texture *TextureTarget::getTargetTexture() {
	return _texture->getMetalTexture();
}

} // End of namespace Metal

