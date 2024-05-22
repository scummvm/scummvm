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

#ifndef BACKENDS_PLATFORM_IOS7_IOS7_VIDEO_METAL_H
#define BACKENDS_PLATFORM_IOS7_IOS7_VIDEO_METAL_H

#include "backends/platform/ios7/ios7_video.h"

#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

API_AVAILABLE(ios(11.0))
@interface iPhoneViewMetal : iPhoneView {
	// OpenGL ES context
	EAGLContext *_mainContext;
	EAGLContext *_openGLContext;

	// Metal context
	CAMetalLayer *_metalLayer;

	uint _drawableWidth;
	uint _drawableHeight;

	// Shared CoreVideo OpenGL ES context
	CVOpenGLESTextureCacheRef _openGLTextureCache;
	CVOpenGLESTextureRef _openGLTextureRef;

	// Shared pixel buffer
	CVPixelBufferRef _sharedPixelBuffer;

	// Shared CoreVideo Metal context
	CVMetalTextureCacheRef _metalTextureCache;
	CVMetalTextureRef _metalTextureRef;
}

// Metal properties
@property (readonly, nonnull, nonatomic) id<MTLDevice> metalDevice;
@property (readonly, nonnull, nonatomic) id<MTLCommandQueue> commandQueue;
@property (readonly, nonnull, nonatomic) id<MTLTexture> metalTexture;
@property (readonly, nonnull, nonatomic) id<MTLLibrary> metalLibrary;
@property (readonly, nonnull, nonatomic) id<MTLFunction> kernelFunctionFlipY;
@property (readonly, nonnull, nonatomic) id<MTLFunction> kernelFunctionBlit;

- (id)initWithFrame:(struct CGRect)frame andMetalDevice:(id<MTLDevice>_Nonnull) device;

- (uint)createOpenGLContext;
- (void)destroyOpenGLContext;
- (void)refreshScreen;
- (int)getScreenWidth;
- (int)getScreenHeight;

- (void)initSurface;

@end

#endif
