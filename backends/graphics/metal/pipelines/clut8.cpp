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

#include "backends/graphics/metal/pipelines/clut8.h"
#include "backends/graphics/metal/shader.h"
#include "backends/graphics/metal/framebuffer.h"

namespace Metal {

CLUT8LookUpPipeline::CLUT8LookUpPipeline(Renderer *renderer)
	: ShaderPipeline(renderer, ShaderMan.query(ShaderManager::kCLUT8LookUpFragmentShader)) {
}

void CLUT8LookUpPipeline::drawTextureInternal(const MetalTexture &texture, const float *coordinates, const float *texcoords) {
	assert(isActive());

	const Renderer::Vertex vertices[] = {
		{{coordinates[0], coordinates[1]}, {texcoords[0], texcoords[1]}}, // Vertex 0
		{{coordinates[2], coordinates[3]}, {texcoords[2], texcoords[3]}}, // Vertex 1
		{{coordinates[4], coordinates[5]}, {texcoords[4], texcoords[5]}}, // Vertex 2
		{{coordinates[6], coordinates[7]}, {texcoords[6], texcoords[7]}}  // Vertex 3
	};
	
	_renderer->draw2dTextureWithPalette(_activeFramebuffer->getTargetTexture(), _paletteTexture->getMetalTexture(), texture.getMetalTexture(), vertices, _projectionMatrix, _viewport);
}

} // End of namespace Metal
