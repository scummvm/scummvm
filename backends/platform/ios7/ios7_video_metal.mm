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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/ios7/ios7_video_metal.h"

#include <CoreVideo/CoreVideo.h>

extern void printError(const char *error_message);
#define printOpenGLError() printOglError(__FILE__, __LINE__)
extern void printOglError(const char *file, int line);

const char* kernelFunction = R"(
	#include <metal_stdlib>
	#include <simd/simd.h>
	using namespace metal;

	kernel void flip_y(
	texture2d<float, access::write> dst [[texture(0)]],
	texture2d<float, access::read> src [[texture(1)]],
	uint2 gid [[thread_position_in_grid]])
	{
		float4 flipColor = src.read(uint2(gid.x, src.get_height() - gid.y));
		dst.write(flipColor, gid);
	}
)";

@implementation iPhoneViewMetal

static inline void execute_on_main_thread(void (^block)(void)) {
	if ([NSThread currentThread] == [NSThread mainThread]) {
		block();
	}
	else {
		dispatch_sync(dispatch_get_main_queue(), block);
	}
}

+ (Class)layerClass {
	return [CAMetalLayer class];
}

// According to Apple doc layoutSublayersOfLayer: is supported from iOS 10.0.
// This doesn't seem to be correct since the instance method layoutSublayers,
// supported from iOS 2.0, default calls the layoutSublayersOfLayer: method
// of the layer’s delegate object. It's been verified that this function is
// called in at least iOS 9.3.5.
- (void)layoutSublayersOfLayer:(CAMetalLayer *)layer {
	if (layer == self.layer) {
		[self addEvent:InternalEvent(kInputScreenChanged, 0, 0)];
	}
	[super layoutSublayersOfLayer:layer];
}

- (id)initWithFrame:(struct CGRect)frame andMetalDevice:(id<MTLDevice>_Nonnull)device {
	self = [super initWithFrame:frame];

	_metalDevice = device;

	// Initialise the Metal context
	[self createMetalContext];

	// Initialize the OpenGL ES context
	[self createOpenGLESContext];

	return self;
}

- (void)createMetalContext {
	_metalLayer = (CAMetalLayer *)self.layer;
	_metalLayer.framebufferOnly = NO;
	_metalLayer.pixelFormat = MTLPixelFormatRGBA8Unorm;

	_commandQueue = [_metalDevice newCommandQueue];
	[_metalLayer setDevice:_metalDevice];
	[_metalLayer setDrawableSize:CGSizeMake(self.frame.size.width * self.contentScaleFactor, self.frame.size.height * self.contentScaleFactor)];

	_metalLibrary = [_metalDevice newLibraryWithSource:[NSString stringWithUTF8String:kernelFunction] options:nil error:nil];
	_kernelFunctionFlipY = [_metalLibrary newFunctionWithName:@"flip_y"];
}

- (void)createOpenGLESContext {
	_mainContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

	// In case creating the OpenGL ES context failed, we will error out here.
	if (_mainContext == nil) {
		printError("Could not create OpenGL ES context.");
		abort();
	}

	// main thread will always use _mainContext
	[EAGLContext setCurrentContext:_mainContext];
}

- (uint)createOpenGLContext {
	// Create OpenGL context with the sharegroup from the context
	// connected to the Apple Core Animation layer
	if (!_openGLContext && _mainContext) {
		_openGLContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup:_mainContext.sharegroup];

		if (_openGLContext == nil) {
			printError("Could not create OpenGL ES context using sharegroup");
			abort();
		}
		// background thread will always use _openGLContext
		if ([EAGLContext setCurrentContext:_openGLContext]) {
			[self setupRenderBuffer];
		}
	}
	return CVOpenGLESTextureGetName(_openGLTextureRef);
}

- (void)destroyOpenGLContext {
	[_openGLContext release];
	_openGLContext = nil;
}

- (void)setupOpenGLTextureCache {
	if (_openGLTextureCache == nullptr) {
		CVOpenGLESTextureCacheCreate(
			kCFAllocatorDefault,
			NULL, _openGLContext,
			NULL, &_openGLTextureCache);
	}

	CFDictionaryRef emptyDict = CFDictionaryCreate(
		kCFAllocatorDefault,
		NULL, NULL, 0,
		&kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);

	CFMutableDictionaryRef attributes = CFDictionaryCreateMutable(
		kCFAllocatorDefault, 1,
		&kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);

	CFDictionarySetValue(
		attributes,
		kCVPixelBufferIOSurfacePropertiesKey, // This creates a shared IOSSurface
		emptyDict);

	if (_sharedPixelBuffer != nullptr) {
		CFRelease(IOSurfaceRef(_sharedPixelBuffer));
		_sharedPixelBuffer = nullptr;
	}

	CVPixelBufferCreate(
		kCFAllocatorDefault,
		_drawableWidth, _drawableHeight,
		kCVPixelFormatType_32BGRA,
		attributes,
		&_sharedPixelBuffer);

	CFRelease(attributes);
	CFRelease(emptyDict);

	if (_openGLTextureRef != nullptr) {
		CVBufferRelease(_openGLTextureRef);
	}
	CVOpenGLESTextureCacheFlush(_openGLTextureCache, 0);

	CVOpenGLESTextureCacheCreateTextureFromImage(
		kCFAllocatorDefault,
		_openGLTextureCache,
		_sharedPixelBuffer,
		NULL,
		GL_TEXTURE_2D,
		GL_RGBA,
		_drawableWidth,
		_drawableHeight,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		0,
		&_openGLTextureRef);
}

- (void)setupMetalTextureCache {
	if (_metalTextureCache == nullptr) {
		CVMetalTextureCacheCreate(
			kCFAllocatorDefault,
			nil,
			_metalDevice,
			nil,
			&_metalTextureCache);
	}

	if (_metalTextureRef != nullptr) {
		CVBufferRelease(_metalTextureRef);
	}
	CVMetalTextureCacheFlush(_metalTextureCache, 0);

	CVMetalTextureCacheCreateTextureFromImage(
		kCFAllocatorDefault,
		_metalTextureCache,
		_sharedPixelBuffer,
		NULL,
		MTLPixelFormatRGBA8Unorm,
		_drawableWidth, _drawableHeight,
		0,
		&_metalTextureRef);

	_metalTexture = CVMetalTextureGetTexture(_metalTextureRef);
}

- (void)refreshScreen {
	NSError *error = NULL;

	glFinish();

	@autoreleasepool {
		id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
		id<CAMetalDrawable> drawable = [_metalLayer nextDrawable];
		id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoder];

		[encoder setComputePipelineState:[_metalDevice newComputePipelineStateWithFunction:_kernelFunctionFlipY error:&error]];
		// dst
		[encoder setTexture:[drawable texture] atIndex:0];
		// src
		[encoder setTexture:_metalTexture atIndex:1];

		MTLSize threadgroupSize = MTLSizeMake(16, 16, 1);
		MTLSize threadgroupCount;
		threadgroupCount.width = (_metalTexture.width + threadgroupSize.width - 1) / threadgroupSize.width;
		threadgroupCount.height = (_metalTexture.height + threadgroupSize.height - 1) / threadgroupSize.height;
		threadgroupCount.depth = 1;
		[encoder dispatchThreadgroups:threadgroupCount threadsPerThreadgroup:threadgroupSize];

		[encoder endEncoding];
		[commandBuffer presentDrawable:drawable];
		[commandBuffer commit];
	}
}

- (int)getScreenWidth {
	return _drawableWidth;
}

- (int)getScreenHeight {
	return _drawableHeight;
}

- (void)setupRenderBuffer {
	execute_on_main_thread(^{
		_drawableWidth = _metalLayer.drawableSize.width;
		_drawableHeight = _metalLayer.drawableSize.height;

		[self setupOpenGLTextureCache];
		[self setupMetalTextureCache];
	});
}

- (void)initSurface {
	[self setupRenderBuffer];

	[super initSurface];
}

- (void)adjustViewFrameForSafeArea {
	[super adjustViewFrameForSafeArea];

	[_metalLayer setDrawableSize:CGSizeMake(self.frame.size.width * self.contentScaleFactor, self.frame.size.height * self.contentScaleFactor)];
}

@end
