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

#ifndef BACKENDS_GRAPHICS_METAL_RENDERER_H
#define BACKENDS_GRAPHICS_METAL_RENDERER_H

#include "backends/graphics/metal/texture.h"
#include <Metal/Metal.hpp>

#include <simd/simd.h>

namespace Metal {

class MetalTexture;

class Renderer
{
public:
	struct Vertex {
		// Positions in pixel space. A value of 100 indicates 100 pixels from the origin/center.
		simd_float2 position;
		// 2D texture coordinate
		simd_float2 texCoord;
	};

	Renderer(MTL::CommandQueue *commandQueue);
	~Renderer();
	void buildShaders();
	void buildBuffers();
	void draw2dTexture(const MTL::Texture *outTexture, MTL::Texture *inTexture, const Vertex vertices[4], const matrix_float4x4 &projectionMatrix, MTL::Viewport &viewport, MTL::LoadAction loadAction, bool scissorTestState, const MTL::ScissorRect &scissorBox, const MTL::ClearColor &clearColor, const MTL::SamplerMinMagFilter filter);
	void draw2dTextureWithPalette(const MTL::Texture *outTexture, const MTL::Texture *paletteTexture, MTL::Texture *inTexture, const Vertex vertices[4], const matrix_float4x4 &projectionMatrix, MTL::Viewport &viewport);
	
private:
	MTL::Device *_device;
	MTL::CommandQueue *_commandQueue;
	MTL::RenderPipelineState *_noBlendPipeLineState;
	MTL::RenderPipelineState *_clut8PipeLineState;
	MTL::Buffer* _indexBuffer;
	MTL::SamplerState *_samplerNearest;
	MTL::SamplerState *_samplerLinear;
};

} // end namespace Metal

#endif // BACKENDS_GRAPHICS_METAL_RENDERER_H
