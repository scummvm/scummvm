/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "iphone_video.h"

static iPhoneView *sharedInstance = nil;
static int _fullWidth;
static int _fullHeight;
static CGRect _gameScreenRect;

static char *_gameScreenTextureBuffer = 0;
static int _gameScreenTextureWidth = 0;
static int _gameScreenTextureHeight = 0;

static char *_overlayTexBuffer = 0;
static int _overlayTexWidth = 0;
static int _overlayTexHeight = 0;
static CGRect _overlayRect;

static int _needsScreenUpdate = 0;
static int _overlayIsEnabled = 0;

static UITouch *_firstTouch = NULL;
static UITouch *_secondTouch = NULL;

static unsigned short *_mouseCursor = NULL;

static GLint _renderBufferWidth;
static GLint _renderBufferHeight;

static int _scaledShakeOffsetY;

static VideoContext _videoContext;

#if 0
static long lastTick = 0;
static int frames = 0;
#endif

#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(const char *file, int line) {
	int retCode = 0;

	// returns 1 if an OpenGL error occurred, 0 otherwise.
	GLenum glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		fprintf(stderr, "glError: %u (%s: %d)\n", glErr, file, line);
		retCode = 1;
		glErr = glGetError();
	}
	return retCode;
}

void iPhone_setGraphicsMode(GraphicsModes mode) {
	_videoContext.graphicsMode = mode;

	[sharedInstance performSelectorOnMainThread:@selector(setGraphicsMode) withObject:nil waitUntilDone: YES];
}

void iPhone_showCursor(int state) {
	_videoContext.mouseIsVisible = state;
}

void iPhone_setMouseCursor(unsigned short *buffer, int width, int height, int hotspotX, int hotspotY) {
	_mouseCursor = buffer;

	_videoContext.mouseWidth = width;
	_videoContext.mouseHeight = height;

	_videoContext.mouseHotspotX = hotspotX;
	_videoContext.mouseHotspotY = hotspotY;

	[sharedInstance performSelectorOnMainThread:@selector(updateMouseCursor) withObject:nil waitUntilDone: YES];
}

void iPhone_enableOverlay(int state) {
	_overlayIsEnabled = state;

	[sharedInstance performSelectorOnMainThread:@selector(clearColorBuffer) withObject:nil waitUntilDone: YES];
}

int iPhone_getScreenHeight() {
	return _videoContext.overlayHeight;
}

int iPhone_getScreenWidth() {
	return _videoContext.overlayWidth;
}

bool iPhone_isHighResDevice() {
	return _fullHeight > 480;
}

void iPhone_updateScreen(int mouseX, int mouseY) {
	//printf("Mouse: (%i, %i)\n", mouseX, mouseY);

	_videoContext.mouseX = mouseX;
	_videoContext.mouseY = mouseY;

	if (!_needsScreenUpdate) {
		_needsScreenUpdate = 1;
		[sharedInstance performSelectorOnMainThread:@selector(updateSurface) withObject:nil waitUntilDone: NO];
	}
}

void iPhone_updateScreenRect(unsigned short *screen, int x1, int y1, int x2, int y2) {
	for (int y = y1; y < y2; ++y)
		memcpy(&_gameScreenTextureBuffer[(y * _gameScreenTextureWidth + x1) * 2], &screen[y * _videoContext.screenWidth + x1], (x2 - x1) * 2);
}

void iPhone_updateOverlayRect(unsigned short *screen, int x1, int y1, int x2, int y2) {
	//printf("Overlaywidth: %u, fullwidth %u\n", _videoContext.overlayWidth, _fullWidth);
	for (int y = y1; y < y2; ++y)
		memcpy(&_overlayTexBuffer[(y * _overlayTexWidth + x1) * 2], &screen[y * _videoContext.overlayWidth + x1], (x2 - x1) * 2);
}

void iPhone_initSurface(int width, int height) {
	_videoContext.screenWidth = width;
	_videoContext.screenHeight = height;
	_videoContext.shakeOffsetY = 0;
	[sharedInstance performSelectorOnMainThread:@selector(initSurface) withObject:nil waitUntilDone: YES];
}

void iPhone_setShakeOffset(int offset) {
	_videoContext.shakeOffsetY = offset;
	[sharedInstance performSelectorOnMainThread:@selector(setViewTransformation) withObject:nil waitUntilDone: YES];
}

bool iPhone_fetchEvent(int *outEvent, int *outX, int *outY) {
	id event = [sharedInstance getEvent];
	if (event == nil) {
		return false;
	}

	id type = [event objectForKey:@"type"];

	if (type == nil) {
		printf("fetchEvent says: No type!\n");
		return false;
	}

	*outEvent = [type intValue];
	*outX = [[event objectForKey:@"x"] intValue];
	*outY = [[event objectForKey:@"y"] intValue];
	return true;
}

uint getSizeNextPOT(uint size) {
	if ((size & (size - 1)) || !size) {
		int log = 0;

		while (size >>= 1)
			++log;

		size = (2 << log);
	}

	return size;
}

const char *iPhone_getDocumentsDir() {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	return [documentsDirectory UTF8String];
}

/**
 * Converts portrait mode coordinates into rotated mode coordinates.
 */
static bool convertToRotatedCoords(UIDeviceOrientation orientation, CGPoint point, CGPoint *result) {
	switch (orientation) {
	case UIDeviceOrientationLandscapeLeft:
		result->x = point.y;
		result->y = _renderBufferWidth - point.x;
		return true;

	case UIDeviceOrientationLandscapeRight:
		result->x = _renderBufferHeight - point.y;
		result->y = point.x;
		return true;

	case UIDeviceOrientationPortrait:
		result->x = point.x;
		result->y = point.y;
		return true;

	default:
		return false;
	}
}

static bool getMouseCoords(UIDeviceOrientation orientation, CGPoint point, int *x, int *y) {
	if (!convertToRotatedCoords(orientation, point, &point))
		return false;

	CGRect *area;
	int width, height, offsetY;
	if (_overlayIsEnabled) {
		area = &_overlayRect;
		width = _videoContext.overlayWidth;
		height = _videoContext.overlayHeight;
		offsetY = _scaledShakeOffsetY;
	} else {
		area = &_gameScreenRect;
		width = _videoContext.screenWidth;
		height = _videoContext.screenHeight;
		offsetY = _videoContext.shakeOffsetY;
	}

	point.x = (point.x - CGRectGetMinX(*area)) / CGRectGetWidth(*area);
	point.y = (point.y - CGRectGetMinY(*area)) / CGRectGetHeight(*area);

	*x = (int)(point.x * width);
	// offsetY describes the translation of the screen in the upward direction,
	// thus we need to add it here.
	*y = (int)(point.y * height + offsetY);

	// Clip coordinates
	if (*x < 0 || *x > CGRectGetWidth(*area) || *y < 0 || *y > CGRectGetHeight(*area))
		return false;

	return true;
}

@implementation iPhoneView

+ (Class)layerClass {
	return [CAEAGLLayer class];
}

- (void)createContext {
	CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

	eaglLayer.opaque = YES;
	eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
	                                [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGB565, kEAGLDrawablePropertyColorFormat, nil];

	_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];

	// In case creating the OpenGL ES context failed, we will error out here.
	if (_context == nil) {
		fprintf(stderr, "Could not create OpenGL ES context\n");
		exit(-1);
	}

	if ([EAGLContext setCurrentContext:_context]) {
		glGenFramebuffersOES(1, &_viewFramebuffer); printOpenGLError();
		glGenRenderbuffersOES(1, &_viewRenderbuffer); printOpenGLError();

		glBindFramebufferOES(GL_FRAMEBUFFER_OES, _viewFramebuffer); printOpenGLError();
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer); printOpenGLError();
		[_context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(id<EAGLDrawable>)self.layer];

		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, _viewRenderbuffer); printOpenGLError();

		// Retrieve the render buffer size. This *should* match the frame size,
		// i.e. _fullWidth and _fullHeight.
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &_renderBufferWidth); printOpenGLError();
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &_renderBufferHeight); printOpenGLError();

		if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
			NSLog(@"Failed to make complete framebuffer object %x.", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
			return;
		}

		_videoContext.overlayHeight = _renderBufferWidth;
		_videoContext.overlayWidth = _renderBufferHeight;
		_overlayTexWidth = getSizeNextPOT(_videoContext.overlayHeight);
		_overlayTexHeight = getSizeNextPOT(_videoContext.overlayWidth);

		// Since the overlay size won't change the whole run, we can
		// precalculate the texture coordinates for the overlay texture here
		// and just use it later on.
		_overlayTexCoords[2] = _overlayTexCoords[6] = _videoContext.overlayWidth / (GLfloat)_overlayTexWidth;
		_overlayTexCoords[5] = _overlayTexCoords[7] = _videoContext.overlayHeight / (GLfloat)_overlayTexHeight;

		int textureSize = _overlayTexWidth * _overlayTexHeight * 2;
		_overlayTexBuffer = (char *)malloc(textureSize);
		memset(_overlayTexBuffer, 0, textureSize);

		glViewport(0, 0, _renderBufferWidth, _renderBufferHeight); printOpenGLError();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); printOpenGLError();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_TEXTURE_2D); printOpenGLError();
		glEnableClientState(GL_TEXTURE_COORD_ARRAY); printOpenGLError();
		glEnableClientState(GL_VERTEX_ARRAY); printOpenGLError();
	}
}

- (id)initWithFrame:(struct CGRect)frame {
	self = [super initWithFrame: frame];

	if ([[UIScreen mainScreen] respondsToSelector: NSSelectorFromString(@"scale")]) {
		if ([self respondsToSelector: NSSelectorFromString(@"contentScaleFactor")]) {
			//self.contentScaleFactor = [[UIScreen mainScreen] scale];
		}
	}

	_fullWidth = (int)frame.size.width;
	_fullHeight = (int)frame.size.height;

	sharedInstance = self;

	_keyboardView = nil;
	_screenTexture = 0;
	_overlayTexture = 0;
	_mouseCursorTexture = 0;

	_videoContext.graphicsMode = kGraphicsModeLinear;

	_gameScreenVertCoords[0] = _gameScreenVertCoords[1] =
	    _gameScreenVertCoords[2] = _gameScreenVertCoords[3] =
	    _gameScreenVertCoords[4] = _gameScreenVertCoords[5] =
	    _gameScreenVertCoords[6] = _gameScreenVertCoords[7] = 0;

	_gameScreenTexCoords[0] = _gameScreenTexCoords[1] =
	    _gameScreenTexCoords[2] = _gameScreenTexCoords[3] =
	    _gameScreenTexCoords[4] = _gameScreenTexCoords[5] =
	    _gameScreenTexCoords[6] = _gameScreenTexCoords[7] = 0;

	_overlayVertCoords[0] = _overlayVertCoords[1] =
	    _overlayVertCoords[2] = _overlayVertCoords[3] =
	    _overlayVertCoords[4] = _overlayVertCoords[5] =
	    _overlayVertCoords[6] = _overlayVertCoords[7] = 0;

	_overlayTexCoords[0] = _overlayTexCoords[1] =
	    _overlayTexCoords[2] = _overlayTexCoords[3] =
	    _overlayTexCoords[4] = _overlayTexCoords[5] =
	    _overlayTexCoords[6] = _overlayTexCoords[7] = 0;

	// Initialize the OpenGL ES context
	[self createContext];

	return self;
}

- (void)dealloc {
	[super dealloc];

	if (_keyboardView != nil) {
		[_keyboardView dealloc];
	}

	free(_gameScreenTextureBuffer);
	free(_overlayTexBuffer);
}

- (void)drawRect:(CGRect)frame {
#if 0
	if (lastTick == 0) {
		lastTick = time(0);
	}

	frames++;
	if (time(0) > lastTick) {
		lastTick = time(0);
		printf("FPS: %i\n", frames);
		frames = 0;
	}
#endif
}

- (void)setFilterModeForTexture:(GLuint)tex {
	if (!tex)
		return;

	glBindTexture(GL_TEXTURE_2D, tex); printOpenGLError();

	GLint filter = GL_LINEAR;

	switch (_videoContext.graphicsMode) {
	case kGraphicsModeLinear:
		filter = GL_LINEAR;
		break;

	case kGraphicsModeNone:
		filter = GL_NEAREST;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter); printOpenGLError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter); printOpenGLError();
}

- (void)setGraphicsMode {
	[self setFilterModeForTexture:_screenTexture];
	[self setFilterModeForTexture:_overlayTexture];
	[self setFilterModeForTexture:_mouseCursorTexture];
}

- (void)updateSurface {
	if (!_needsScreenUpdate) {
		return;
	}
	_needsScreenUpdate = 0;

	glClear(GL_COLOR_BUFFER_BIT); printOpenGLError();

	[self updateMainSurface];

	if (_overlayIsEnabled)
		[self updateOverlaySurface];

	if (_videoContext.mouseIsVisible)
		[self updateMouseSurface];

	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer); printOpenGLError();
	[_context presentRenderbuffer:GL_RENDERBUFFER_OES];

}

- (void)updateMouseCursor {
	if (_mouseCursorTexture == 0) {
		glGenTextures(1, &_mouseCursorTexture); printOpenGLError();
		[self setFilterModeForTexture:_mouseCursorTexture];
	}

	glBindTexture(GL_TEXTURE_2D, _mouseCursorTexture); printOpenGLError();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getSizeNextPOT(_videoContext.mouseWidth), getSizeNextPOT(_videoContext.mouseHeight), 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, _mouseCursor); printOpenGLError();

	free(_mouseCursor);
	_mouseCursor = NULL;
}

- (void)updateMainSurface {
	glVertexPointer(2, GL_FLOAT, 0, _gameScreenVertCoords); printOpenGLError();
	glTexCoordPointer(2, GL_FLOAT, 0, _gameScreenTexCoords); printOpenGLError();

	glBindTexture(GL_TEXTURE_2D, _screenTexture); printOpenGLError();

	// Unfortunately we have to update the whole texture every frame, since glTexSubImage2D is actually slower in all cases
	// due to the iPhone internals having to convert the whole texture back from its internal format when used.
	// In the future we could use several tiled textures instead.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _gameScreenTextureWidth, _gameScreenTextureHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, _gameScreenTextureBuffer); printOpenGLError();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); printOpenGLError();
}

- (void)updateOverlaySurface {
	glVertexPointer(2, GL_FLOAT, 0, _overlayVertCoords); printOpenGLError();
	glTexCoordPointer(2, GL_FLOAT, 0, _overlayTexCoords); printOpenGLError();

	glBindTexture(GL_TEXTURE_2D, _overlayTexture); printOpenGLError();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _overlayTexWidth, _overlayTexHeight, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, _overlayTexBuffer); printOpenGLError();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); printOpenGLError();
}

- (void)updateMouseSurface {
	int width = _videoContext.mouseWidth;
	int height = _videoContext.mouseHeight;

	int mouseX = _videoContext.mouseX;
	int mouseY = _videoContext.mouseY;

	int hotspotX = _videoContext.mouseHotspotX;
	int hotspotY = _videoContext.mouseHotspotY;

	CGRect *rect;
	int maxWidth, maxHeight;

	if (!_overlayIsEnabled) {
		rect = &_gameScreenRect;
		maxWidth = _videoContext.screenWidth;
		maxHeight = _videoContext.screenHeight;
	} else {
		rect = &_overlayRect;
		maxWidth = _videoContext.overlayWidth;
		maxHeight = _videoContext.overlayHeight;
	}

	const GLfloat scaleX = CGRectGetWidth(*rect) / (GLfloat)maxWidth;
	const GLfloat scaleY = CGRectGetHeight(*rect) / (GLfloat)maxHeight;

	mouseX = (int)(mouseX * scaleX);
	mouseY = (int)(mouseY * scaleY);
	hotspotX = (int)(hotspotX * scaleX);
	hotspotY = (int)(hotspotY * scaleY);
	width = (int)(width * scaleX);
	height = (int)(height * scaleY);

	mouseX -= hotspotX;
	mouseY -= hotspotY;

	mouseX += (int)CGRectGetMinX(*rect);
	mouseY += (int)CGRectGetMinY(*rect);

	GLfloat vertices[] = {
		// Top left
		mouseX        , mouseY,
		// Top right
		mouseX + width, mouseY,
		// Bottom left
		mouseX        , mouseY + height,
		// Bottom right
		mouseX + width, mouseY + height
	};

	//printf("Cursor: width %u height %u\n", _videoContext.mouseWidth, _videoContext.mouseHeight);

	float texWidth = _videoContext.mouseWidth / (float)getSizeNextPOT(_videoContext.mouseWidth);
	float texHeight = _videoContext.mouseHeight / (float)getSizeNextPOT(_videoContext.mouseHeight);

	const GLfloat texCoords[] = {
		// Top left
		0       , 0,
		// Top right
		texWidth, 0,
		// Bottom left
		0       , texHeight,
		// Bottom right
		texWidth, texHeight
	};

	glVertexPointer(2, GL_FLOAT, 0, vertices); printOpenGLError();
	glTexCoordPointer(2, GL_FLOAT, 0, texCoords); printOpenGLError();

	glBindTexture(GL_TEXTURE_2D, _mouseCursorTexture); printOpenGLError();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); printOpenGLError();
}

- (void)setUpOrientation:(UIDeviceOrientation)orientation width:(int *)width height:(int *)height {
	_orientation = orientation;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// We always force the origin (0,0) to be in the upper left corner.
	switch (_orientation) {
	case UIDeviceOrientationLandscapeRight:
		glRotatef( 90, 0, 0, 1); printOpenGLError();
		glOrthof(0, _renderBufferHeight, _renderBufferWidth, 0, 0, 1); printOpenGLError();

		*width = _renderBufferHeight;
		*height = _renderBufferWidth;
		break;

	case UIDeviceOrientationLandscapeLeft:
		glRotatef(-90, 0, 0, 1); printOpenGLError();
		glOrthof(0, _renderBufferHeight, _renderBufferWidth, 0, 0, 1); printOpenGLError();

		*width = _renderBufferHeight;
		*height = _renderBufferWidth;
		break;

	case UIDeviceOrientationPortrait:
	default:
		// We must force the portrait orientation here, since we might not know
		// the real orientation.
		_orientation = UIDeviceOrientationPortrait;

		glOrthof(0, _renderBufferWidth, _renderBufferHeight, 0, 0, 1); printOpenGLError();

		*width = _renderBufferWidth;
		*height = _renderBufferHeight;
		break;
	}
}

- (void)initSurface {
	_gameScreenTextureWidth = getSizeNextPOT(_videoContext.screenWidth);
	_gameScreenTextureHeight = getSizeNextPOT(_videoContext.screenHeight);

	_gameScreenTexCoords[2] = _gameScreenTexCoords[6] = _videoContext.screenWidth / (GLfloat)_gameScreenTextureWidth;
	_gameScreenTexCoords[5] = _gameScreenTexCoords[7] = _videoContext.screenHeight / (GLfloat)_gameScreenTextureHeight;

	int screenWidth, screenHeight;
	[self setUpOrientation:[[UIDevice currentDevice] orientation] width:&screenWidth height:&screenHeight];

	//printf("Window: (%d, %d), Surface: (%d, %d), Texture(%d, %d)\n", _fullWidth, _fullHeight, _videoContext.screenWidth, _videoContext.screenHeight, _gameScreenTextureWidth, _gameScreenTextureHeight);

	if (_screenTexture > 0) {
		glDeleteTextures(1, &_screenTexture); printOpenGLError();
	}

	glGenTextures(1, &_screenTexture); printOpenGLError();
	[self setFilterModeForTexture:_screenTexture];

	if (_overlayTexture > 0) {
		glDeleteTextures(1, &_overlayTexture); printOpenGLError();
	}

	glGenTextures(1, &_overlayTexture); printOpenGLError();
	[self setFilterModeForTexture:_overlayTexture];

	free(_gameScreenTextureBuffer);
	int textureSize = _gameScreenTextureWidth * _gameScreenTextureHeight * 2;
	_gameScreenTextureBuffer = (char *)malloc(textureSize);
	memset(_gameScreenTextureBuffer, 0, textureSize);

	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer); printOpenGLError();

	[self clearColorBuffer];

	if (_keyboardView != nil) {
		[_keyboardView removeFromSuperview];
		[[_keyboardView inputView] removeFromSuperview];
	}

	float overlayPortraitRatio;

	if (_orientation == UIDeviceOrientationLandscapeLeft || _orientation ==  UIDeviceOrientationLandscapeRight) {
		GLfloat gameScreenRatio = (GLfloat)_videoContext.screenWidth / (GLfloat)_videoContext.screenHeight;
		GLfloat screenRatio = (GLfloat)screenWidth / (GLfloat)screenHeight;

		// These are the width/height according to the portrait layout!
		int rectWidth, rectHeight;
		int xOffset, yOffset;

		if (gameScreenRatio < screenRatio) {
			// When the game screen ratio is less than the screen ratio
			// we need to scale the width, since the game screen was higher
			// compared to the width than our output screen is.
			rectWidth = (int)(screenHeight * gameScreenRatio);
			rectHeight = screenHeight;
			xOffset = (screenWidth - rectWidth) / 2;
			yOffset = 0;
		} else {
			// When the game screen ratio is bigger than the screen ratio
			// we need to scale the height, since the game screen was wider
			// compared to the height than our output screen is.
			rectWidth = screenWidth;
			rectHeight = (int)(screenWidth / gameScreenRatio);
			xOffset = 0;
			yOffset = (screenHeight - rectHeight) / 2;
		}

		//printf("Rect: %i, %i, %i, %i\n", xOffset, yOffset, rectWidth, rectHeight);
		_gameScreenRect = CGRectMake(xOffset, yOffset, rectWidth, rectHeight);
		overlayPortraitRatio = 1.0f;
	} else {
		float ratio = (float)_videoContext.screenHeight / (float)_videoContext.screenWidth;
		int height = (int)(screenWidth * ratio);
		//printf("Making rect (%u, %u)\n", screenWidth, height);
		_gameScreenRect = CGRectMake(0, 0, screenWidth, height);

		CGRect keyFrame = CGRectMake(0.0f, 0.0f, 0.0f, 0.0f);
		if (_keyboardView == nil) {
			_keyboardView = [[SoftKeyboard alloc] initWithFrame:keyFrame];
			[_keyboardView setInputDelegate:self];
		}

		[self addSubview:[_keyboardView inputView]];
		[self addSubview: _keyboardView];
		[[_keyboardView inputView] becomeFirstResponder];
		overlayPortraitRatio = (_videoContext.overlayHeight * ratio) / _videoContext.overlayWidth;
	}

	_overlayRect = CGRectMake(0, 0, screenWidth, screenHeight * overlayPortraitRatio);

	_gameScreenVertCoords[0] = _gameScreenVertCoords[4] = CGRectGetMinX(_gameScreenRect);
	_gameScreenVertCoords[1] = _gameScreenVertCoords[3] = CGRectGetMinY(_gameScreenRect);
	_gameScreenVertCoords[2] = _gameScreenVertCoords[6] = CGRectGetMaxX(_gameScreenRect);
	_gameScreenVertCoords[5] = _gameScreenVertCoords[7] = CGRectGetMaxY(_gameScreenRect);

	_overlayVertCoords[2] = _overlayVertCoords[6] = CGRectGetMaxX(_overlayRect);
	_overlayVertCoords[5] = _overlayVertCoords[7] = CGRectGetMaxY(_overlayRect);

	[self setViewTransformation];
}

- (void)setViewTransformation {
	// Set the modelview matrix. This matrix will be used for the shake offset
	// support.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Scale the shake offset according to the overlay size. We need this to
	// adjust the overlay mouse click coordinates when an offset is set.
	_scaledShakeOffsetY = (int)(_videoContext.shakeOffsetY / (GLfloat)_videoContext.screenHeight * CGRectGetHeight(_overlayRect));

	// Apply the shakeing to the output screen.
	glTranslatef(0, -_scaledShakeOffsetY, 0);
}

- (void)clearColorBuffer {
	// The color buffer is triple-buffered, so we clear it multiple times right away to avid doing any glClears later.
	int clearCount = 5;
	while (clearCount-- > 0) {
		glClear(GL_COLOR_BUFFER_BIT); printOpenGLError();
		[_context presentRenderbuffer:GL_RENDERBUFFER_OES];
	}
}

- (id)getEvent {
	if (_events == nil || [_events count] == 0) {
		return nil;
	}

	id event = [_events objectAtIndex: 0];

	[_events removeObjectAtIndex: 0];

	return event;
}

- (void)addEvent:(NSDictionary *)event {
	if (_events == nil)
		_events = [[NSMutableArray alloc] init];

	[_events addObject: event];
}

- (void)deviceOrientationChanged:(UIDeviceOrientation)orientation {
	switch (orientation) {
	case UIDeviceOrientationLandscapeLeft:
	case UIDeviceOrientationLandscapeRight:
	case UIDeviceOrientationPortrait:
		_orientation = orientation;
		break;

	default:
		return;
	}

	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputOrientationChanged], @"type",
		 [NSNumber numberWithInt:orientation], @"x",
		 [NSNumber numberWithInt:0], @"y",
		 nil
		]
	];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	NSSet *allTouches = [event allTouches];
	int x, y;

	switch ([allTouches count]) {
	case 1: {
		UITouch *touch = [touches anyObject];
		CGPoint point = [touch locationInView:self];
		if (!getMouseCoords(_orientation, point, &x, &y))
			return;

		_firstTouch = touch;
		[self addEvent:
		 [[NSDictionary alloc] initWithObjectsAndKeys:
		  [NSNumber numberWithInt:kInputMouseDown], @"type",
		  [NSNumber numberWithInt:x], @"x",
		  [NSNumber numberWithInt:y], @"y",
		  nil
		  ]
		 ];
		break;
		}

	case 2: {
		UITouch *touch = [touches anyObject];
		CGPoint point = [touch locationInView:self];
		if (!getMouseCoords(_orientation, point, &x, &y))
			return;

		_secondTouch = touch;
		[self addEvent:
		 [[NSDictionary alloc] initWithObjectsAndKeys:
		  [NSNumber numberWithInt:kInputMouseSecondDown], @"type",
		  [NSNumber numberWithInt:x], @"x",
		  [NSNumber numberWithInt:y], @"y",
		  nil
		  ]
		 ];
		break;
		}
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	//NSSet *allTouches = [event allTouches];
	int x, y;

	for (UITouch *touch in touches) {
		if (touch == _firstTouch) {
			CGPoint point = [touch locationInView:self];
			if (!getMouseCoords(_orientation, point, &x, &y))
				return;

			[self addEvent:
			 [[NSDictionary alloc] initWithObjectsAndKeys:
			  [NSNumber numberWithInt:kInputMouseDragged], @"type",
			  [NSNumber numberWithInt:x], @"x",
			  [NSNumber numberWithInt:y], @"y",
			  nil
			  ]
			 ];
		} else if (touch == _secondTouch) {
			CGPoint point = [touch locationInView:self];
			if (!getMouseCoords(_orientation, point, &x, &y))
				return;

			[self addEvent:
			 [[NSDictionary alloc] initWithObjectsAndKeys:
			  [NSNumber numberWithInt:kInputMouseSecondDragged], @"type",
			  [NSNumber numberWithInt:x], @"x",
			  [NSNumber numberWithInt:y], @"y",
			  nil
			  ]
			 ];
		}
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	NSSet *allTouches = [event allTouches];
	int x, y;

	switch ([allTouches count]) {
	case 1: {
		UITouch *touch = [[allTouches allObjects] objectAtIndex:0];
		CGPoint point = [touch locationInView:self];
		if (!getMouseCoords(_orientation, point, &x, &y))
			return;

		[self addEvent:
		 [[NSDictionary alloc] initWithObjectsAndKeys:
		  [NSNumber numberWithInt:kInputMouseUp], @"type",
		  [NSNumber numberWithInt:x], @"x",
		  [NSNumber numberWithInt:y], @"y",
		  nil
		  ]
		 ];
		break;
		}

	case 2: {
		UITouch *touch = [[allTouches allObjects] objectAtIndex:1];
		CGPoint point = [touch locationInView:self];
		if (!getMouseCoords(_orientation, point, &x, &y))
			return;

		[self addEvent:
		 [[NSDictionary alloc] initWithObjectsAndKeys:
		  [NSNumber numberWithInt:kInputMouseSecondUp], @"type",
		  [NSNumber numberWithInt:x], @"x",
		  [NSNumber numberWithInt:y], @"y",
		  nil
		  ]
		 ];
		break;
		}
	}
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
}

- (void)handleKeyPress:(unichar)c {
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputKeyPressed], @"type",
		 [NSNumber numberWithInt:c], @"x",
		 [NSNumber numberWithInt:0], @"y",
		 nil
		]
	];
}

- (BOOL)canHandleSwipes {
	return TRUE;
}

- (int)swipe:(int)num withEvent:(struct __GSEvent *)event {
	//printf("swipe: %i\n", num);

	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputSwipe], @"type",
		 [NSNumber numberWithInt:num], @"x",
		 [NSNumber numberWithInt:0], @"y",
		 nil
		]
	];

	return 0;
}

- (void)applicationSuspend {
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputApplicationSuspended], @"type",
		 [NSNumber numberWithInt:0], @"x",
		 [NSNumber numberWithInt:0], @"y",
		 nil
		]
	];
}

- (void)applicationResume {
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputApplicationResumed], @"type",
		 [NSNumber numberWithInt:0], @"x",
		 [NSNumber numberWithInt:0], @"y",
		 nil
		]
	];
}

@end
