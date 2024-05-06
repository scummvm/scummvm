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

extern void printError(const char *error_message);
#define printOpenGLError() printOglError(__FILE__, __LINE__)
extern void printOglError(const char *file, int line);

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
	return _offScreenRenderbuffer;
}

- (void)destroyOpenGLContext {
	[_openGLContext release];
	_openGLContext = nil;
}

- (void)refreshScreen {
	// TODO: Implement drawing in Metal drawable
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

		// TODO: Setup interoperable pixel buffers
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
