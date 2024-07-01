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

#include "backends/graphics/metal/pipelines/shader.h"
#include "backends/graphics/metal/shader.h"
#include "backends/graphics/metal/framebuffer.h"
#include "backends/graphics/metal/renderer.h"

namespace Metal {

ShaderPipeline::ShaderPipeline(Renderer *renderer, MTL::Function *shader)
	: _renderer(renderer), _activeShader(shader) {
}

ShaderPipeline::~ShaderPipeline() {
	_renderer = nullptr;
}

void ShaderPipeline::activateInternal() {
	Pipeline::activateInternal();
}

void ShaderPipeline::deactivateInternal() {
	Pipeline::deactivateInternal();
}

void ShaderPipeline::setColor(float r, float g, float b, float a) {
	float *dst = _colorAttributes;
	for (uint i = 0; i < 4; ++i) {
		*dst++ = r;
		*dst++ = g;
		*dst++ = b;
		*dst++ = a;
	}
}

void ShaderPipeline::drawTextureInternal(const MetalTexture &texture, const float *coordinates, const float *texcoords) {
	assert(isActive());
	
	const Renderer::Vertex vertices[] = {
		{{coordinates[0], coordinates[1]}, {texcoords[0], texcoords[1]}}, // Vertex 0
		{{coordinates[2], coordinates[3]}, {texcoords[2], texcoords[3]}}, // Vertex 1
		{{coordinates[4], coordinates[5]}, {texcoords[4], texcoords[5]}}, // Vertex 2
		{{coordinates[6], coordinates[7]}, {texcoords[6], texcoords[7]}}  // Vertex 3
	};

	_renderer->draw2dTexture(_activeFramebuffer->getTargetTexture(), texture.getMetalTexture(), vertices, _projectionMatrix, _viewport, (MTL::LoadAction)_loadAction, _scissorTestState, _scissorBox, _clearColor, texture.isLinearFilteringEnabled() ? MTL::SamplerMinMagFilterLinear : MTL::SamplerMinMagFilterNearest);
}

void ShaderPipeline::setProjectionMatrix(const Math::Matrix4 &projectionMatrix) {
	assert(isActive());
	_projectionMatrix = { {
		{projectionMatrix(0, 0), projectionMatrix(0, 1), projectionMatrix(0, 2), projectionMatrix(0, 3)},
		{projectionMatrix(1, 0), projectionMatrix(1, 1), projectionMatrix(1, 2), projectionMatrix(1, 3)},
		{projectionMatrix(2, 0), projectionMatrix(2, 1), projectionMatrix(2, 2), projectionMatrix(2, 3)},
		{projectionMatrix(3, 0), projectionMatrix(3, 1), projectionMatrix(3, 2), projectionMatrix(3, 3)}
	} };
}

} // End of namespace Metal
