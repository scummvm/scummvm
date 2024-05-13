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

#include "backends/graphics/metal/renderer.h"
#include "backends/graphics/metal/shader.h"

namespace Metal {

Renderer::Renderer(MTL::CommandQueue *commandQueue) : _commandQueue(commandQueue), _device(commandQueue->device())
{
	buildShaders();
	buildBuffers();
}

Renderer::~Renderer()
{
	_indexBuffer->release();
	_samplerLinear->release();
	_samplerNearest->release();
	_noBlendPipeLineState->release();
	_clut8PipeLineState->release();
	_device->release();
}

void Renderer::buildShaders()
{
	MTL::VertexDescriptor *defaultVertexDescriptor = MTL::VertexDescriptor::alloc()->init();
	defaultVertexDescriptor->layouts()->object(30)->setStride(sizeof(Vertex));
	defaultVertexDescriptor->layouts()->object(30)->setStepRate(1);
	defaultVertexDescriptor->layouts()->object(30)->setStepFunction(MTL::VertexStepFunctionPerVertex);
	defaultVertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
	defaultVertexDescriptor->attributes()->object(0)->setOffset(0);
	defaultVertexDescriptor->attributes()->object(0)->setBufferIndex(30);
	defaultVertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
	defaultVertexDescriptor->attributes()->object(1)->setOffset(sizeof(simd_float2));
	defaultVertexDescriptor->attributes()->object(1)->setBufferIndex(30);

	// Default Pipeline
	MTL::RenderPipelineDescriptor *defaultPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
	defaultPipelineDescriptor->setVertexFunction(ShaderMan.query(ShaderManager::kDefaultVertexShader));
	defaultPipelineDescriptor->setFragmentFunction(ShaderMan.query(ShaderManager::kDefaultFragmentShader));
	defaultPipelineDescriptor->setVertexDescriptor(defaultVertexDescriptor);

	MTL::RenderPipelineColorAttachmentDescriptor *renderbufferAttachment = defaultPipelineDescriptor->colorAttachments()->object(0);
	renderbufferAttachment->setPixelFormat(MTL::PixelFormat::PixelFormatRGBA8Unorm);

	// CLUT8 Pipeline
	MTL::RenderPipelineDescriptor *clut8PipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
	clut8PipelineDescriptor->setVertexFunction(ShaderMan.query(ShaderManager::kDefaultVertexShader));
	clut8PipelineDescriptor->setFragmentFunction(ShaderMan.query(ShaderManager::kCLUT8LookUpFragmentShader));
	clut8PipelineDescriptor->setVertexDescriptor(defaultVertexDescriptor);

	MTL::RenderPipelineColorAttachmentDescriptor *clut8RenderbufferAttachment = clut8PipelineDescriptor->colorAttachments()->object(0);
	clut8RenderbufferAttachment->setPixelFormat(MTL::PixelFormat::PixelFormatRGBA8Unorm);

	// Alpha Blending
	renderbufferAttachment->setBlendingEnabled(true);
	renderbufferAttachment->setRgbBlendOperation(MTL::BlendOperationAdd);
	renderbufferAttachment->setAlphaBlendOperation(MTL::BlendOperationAdd);
	renderbufferAttachment->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
	renderbufferAttachment->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
	renderbufferAttachment->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
	renderbufferAttachment->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);

	// Samplers
	MTL::SamplerDescriptor *samplerDescriptor = MTL::SamplerDescriptor::alloc()->init();
	samplerDescriptor->setLabel(NS::String::string("Linear sampler", NS::UTF8StringEncoding));
	samplerDescriptor->setMagFilter(MTL::SamplerMinMagFilterLinear);
	samplerDescriptor->setMinFilter(MTL::SamplerMinMagFilterLinear);
	_samplerLinear = _device->newSamplerState(samplerDescriptor);

	samplerDescriptor->setLabel(NS::String::string("Nearest sampler", NS::UTF8StringEncoding));
	samplerDescriptor->setMagFilter(MTL::SamplerMinMagFilterNearest);
	samplerDescriptor->setMinFilter(MTL::SamplerMinMagFilterNearest);
	_samplerNearest = _device->newSamplerState(samplerDescriptor);

	NS::Error* error = nullptr;
	_noBlendPipeLineState = _device->newRenderPipelineState(defaultPipelineDescriptor, &error);
	if (!_noBlendPipeLineState)
	{
		__builtin_printf("%s", error->localizedDescription()->utf8String());
		assert(false);
	}
	_clut8PipeLineState = _device->newRenderPipelineState(clut8PipelineDescriptor, &error);
	if (!_clut8PipeLineState)
	{
		__builtin_printf("%s", error->localizedDescription()->utf8String());
		assert(false);
	}

	defaultPipelineDescriptor->release();
	clut8PipelineDescriptor->release();
	samplerDescriptor->release();
}

void Renderer::buildBuffers()
{
	unsigned short indices[] = {
		0, 1, 2,
		0, 2, 3
	};
	
	_indexBuffer = _device->newBuffer(indices, sizeof(indices), MTL::ResourceStorageModeShared);
}

void Renderer::draw2dTexture(const MTL::Texture *outTexture, MTL::Texture *inTexture, const Vertex vertices[4], const matrix_float4x4 &projectionMatrix, MTL::Viewport &viewport, MTL::LoadAction loadAction, bool scissorTestState, const MTL::ScissorRect &scissorBox, const MTL::ClearColor &clearColor, const MTL::SamplerMinMagFilter filter)
{
	NS::AutoreleasePool *autoreleasePool = NS::AutoreleasePool::alloc()->init();
	
	MTL::CommandBuffer *commandBuffer = _commandQueue->commandBuffer();
	
	MTL::RenderPassDescriptor *renderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();

	MTL::RenderPassColorAttachmentDescriptor *attachment = renderPassDescriptor->colorAttachments()->object(0);
	attachment->setClearColor(clearColor);
	attachment->setLoadAction(loadAction);
	attachment->setStoreAction(MTL::StoreActionStore);
	attachment->setTexture(outTexture);

	MTL::RenderCommandEncoder *encoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);

	encoder->setRenderPipelineState(_noBlendPipeLineState);
	encoder->setViewport(viewport);

	if (scissorTestState)
		encoder->setScissorRect(scissorBox);

	// reference to the layout buffer in vertexDescriptor
	const int kCoordinatesSize = 4 * sizeof(Vertex);
	encoder->setVertexBytes(vertices, kCoordinatesSize, 30);

	encoder->setVertexBytes(&projectionMatrix, sizeof(projectionMatrix), 0);

	// Texture is set to be referred as attribute [[texture(0)]] in a shader function’s parameter list.
	encoder->setFragmentTexture(inTexture, 0);
	encoder->setFragmentSamplerState(filter == MTL::SamplerMinMagFilterLinear ? _samplerLinear : _samplerNearest, 0);
	encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, 6, MTL::IndexTypeUInt16, _indexBuffer, 0);

	encoder->endEncoding();
	commandBuffer->commit();
	renderPassDescriptor->release();
	autoreleasePool->release();
}

void Renderer::draw2dTextureWithPalette(const MTL::Texture *outTexture, const MTL::Texture *paletteTexture, MTL::Texture *inTexture, const Vertex vertices[4], const matrix_float4x4 &projectionMatrix, MTL::Viewport &viewport) {
	NS::AutoreleasePool *autoreleasePool = NS::AutoreleasePool::alloc()->init();
	MTL::CommandBuffer *commandBuffer = _commandQueue->commandBuffer();

	MTL::RenderPassDescriptor *renderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();

	MTL::RenderPassColorAttachmentDescriptor *attachment = renderPassDescriptor->colorAttachments()->object(0);
	attachment->setClearColor(MTL::ClearColor(0, 0, 0, 1));
	attachment->setLoadAction(MTL::LoadActionClear);
	attachment->setStoreAction(MTL::StoreActionStore);
	attachment->setTexture(outTexture);

	MTL::RenderCommandEncoder *encoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);

	encoder->setRenderPipelineState(_clut8PipeLineState);
	encoder->setViewport(viewport);

	// reference to the layout buffer in vertexDescriptor
	const int kCoordinatesSize = 4 * sizeof(Vertex);
	encoder->setVertexBytes(vertices, kCoordinatesSize, 30);

	encoder->setVertexBytes(&projectionMatrix, sizeof(projectionMatrix), 0);

	// Texture is set to be referred as attribute [[texture(0)]] in a shader function’s parameter list.
	encoder->setFragmentTexture(inTexture, 0);
	// Texture is set to be referred as attribute [[texture(1)]] in a shader function’s parameter list.
	encoder->setFragmentTexture(paletteTexture, 1);
	encoder->setFragmentSamplerState(_samplerNearest, 0);
	encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, 6, MTL::IndexTypeUInt16, _indexBuffer, 0);

	encoder->endEncoding();
	commandBuffer->commit();
	renderPassDescriptor->release();
	autoreleasePool->release();
}

} // end namespace Metal
