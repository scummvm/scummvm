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

#include "common/events.h"
#include "backends/platform/ios7/ios7_video.h"
#include "backends/platform/ios7/ios7_touch_controller.h"
#include "backends/platform/ios7/ios7_mouse_controller.h"
#include "backends/platform/ios7/ios7_gamepad_controller.h"

#include "backends/platform/ios7/ios7_app_delegate.h"

static int g_needsScreenUpdate = 0;

#if 0
static long g_lastTick = 0;
static int g_frames = 0;
#endif

void printError(const char *error_message) {
	NSString *messageString = [NSString stringWithUTF8String:error_message];
	NSLog(@"%@", messageString);
	fprintf(stderr, "%s\n", error_message);
}

#define printOpenGLError() printOglError(__FILE__, __LINE__)

void printOglError(const char *file, int line) {
	GLenum glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		Common::String error = Common::String::format("glError: %u (%s: %d)", glErr, file, line);
		printError(error.c_str());
		glErr = glGetError();
	}
}

bool iOS7_isBigDevice() {
#if TARGET_OS_IOS
	return UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad;
#elif TARGET_OS_TV
	return true;
#endif
}

static inline void execute_on_main_thread(void (^block)(void)) {
	if ([NSThread currentThread] == [NSThread mainThread]) {
		block();
	}
	else {
		dispatch_sync(dispatch_get_main_queue(), block);
	}
}

void iOS7_updateScreen() {
	//printf("Mouse: (%i, %i)\n", mouseX, mouseY);
	if (!g_needsScreenUpdate) {
		g_needsScreenUpdate = 1;
		execute_on_main_thread(^{
			[[iOS7AppDelegate iPhoneView] updateSurface];
		});
	}
}

bool iOS7_fetchEvent(InternalEvent *event) {
	__block bool fetched;
	execute_on_main_thread(^{
		fetched = [[iOS7AppDelegate iPhoneView] fetchEvent:event];
	});
	return fetched;
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

@implementation iPhoneView

@synthesize pointerPosition;

+ (Class)layerClass {
	return [CAEAGLLayer class];
}

- (VideoContext *)getVideoContext {
	return &_videoContext;
}

- (void)createContext {
	CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

	eaglLayer.opaque = YES;
	eaglLayer.drawableProperties = @{
	                                 kEAGLDrawablePropertyRetainedBacking: @NO,
	                                 kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8,
	                                };

	_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

	// In case creating the OpenGL ES context failed, we will error out here.
	if (_context == nil) {
		printError("Could not create OpenGL ES context.");
		abort();
	}

	if ([EAGLContext setCurrentContext:_context]) {
		// glEnableClientState(GL_TEXTURE_COORD_ARRAY); printOpenGLError();
		// glEnableClientState(GL_VERTEX_ARRAY); printOpenGLError();
		[self setupOpenGL];
	}
}

- (void)setupOpenGL {
	[self setupFramebuffer];
	[self createOverlaySurface];
	[self compileShaders];
	[self setupVBOs];
	[self setupTextures];

	[self finishGLSetup];
}

- (void)finishGLSetup {
	glViewport(0, 0, _renderBufferWidth, _renderBufferHeight); printOpenGLError();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); printOpenGLError();

	glUniform2f(_screenSizeSlot, _renderBufferWidth, _renderBufferHeight);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

- (void)freeOpenGL {
	[self deleteTextures];
	[self deleteVBOs];
	[self deleteShaders];
	[self deleteFramebuffer];
}

- (void)rebuildFrameBuffer {
	[self deleteFramebuffer];
	[self setupFramebuffer];
	[self finishGLSetup];
}

- (void)setupFramebuffer {
	glGenRenderbuffers(1, &_viewRenderbuffer);
	printOpenGLError();
	glBindRenderbuffer(GL_RENDERBUFFER, _viewRenderbuffer);
	printOpenGLError();
	[_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(id <EAGLDrawable>) self.layer];

	glGenFramebuffers(1, &_viewFramebuffer);
	printOpenGLError();
	glBindFramebuffer(GL_FRAMEBUFFER, _viewFramebuffer);
	printOpenGLError();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _viewRenderbuffer);
	printOpenGLError();

	// Retrieve the render buffer size. This *should* match the frame size,
	// i.e. g_fullWidth and g_fullHeight.
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_renderBufferWidth);
	printOpenGLError();
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_renderBufferHeight);
	printOpenGLError();

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		NSLog(@"Failed to make complete framebuffer object %x.", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		return;
	}
}

- (void)createOverlaySurface {
	uint overlayWidth = (uint) MAX(_renderBufferWidth, _renderBufferHeight);
	uint overlayHeight = (uint) MIN(_renderBufferWidth, _renderBufferHeight);

	_videoContext.overlayWidth = overlayWidth;
	_videoContext.overlayHeight = overlayHeight;

	uint overlayTextureWidthPOT  = getSizeNextPOT(overlayWidth);
	uint overlayTextureHeightPOT = getSizeNextPOT(overlayHeight);

	// Since the overlay size won't change the whole run, we can
	// precalculate the texture coordinates for the overlay texture here
	// and just use it later on.
	GLfloat u = _videoContext.overlayWidth / (GLfloat) overlayTextureWidthPOT;
	GLfloat v = _videoContext.overlayHeight / (GLfloat) overlayTextureHeightPOT;
	_overlayCoords[0].x = 0; _overlayCoords[0].y = 0; _overlayCoords[0].u = 0; _overlayCoords[0].v = 0;
	_overlayCoords[1].x = 0; _overlayCoords[1].y = 0; _overlayCoords[1].u = u; _overlayCoords[1].v = 0;
	_overlayCoords[2].x = 0; _overlayCoords[2].y = 0; _overlayCoords[2].u = 0; _overlayCoords[2].v = v;
	_overlayCoords[3].x = 0; _overlayCoords[3].y = 0; _overlayCoords[3].u = u; _overlayCoords[3].v = v;

	_videoContext.overlayTexture.create((uint16) overlayTextureWidthPOT, (uint16) overlayTextureHeightPOT, Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0));
}

- (void)deleteFramebuffer {
	glDeleteRenderbuffers(1, &_viewRenderbuffer);
	glDeleteFramebuffers(1, &_viewFramebuffer);
}

- (void)setupVBOs {
	glGenBuffers(1, &_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
}

- (void)deleteVBOs {
	glDeleteBuffers(1, &_vertexBuffer);
}

- (GLuint)compileShader:(const char*)shaderPrg withType:(GLenum)shaderType {
	GLuint shaderHandle = glCreateShader(shaderType);

	int shaderPrgLength = strlen(shaderPrg);
	glShaderSource(shaderHandle, 1, &shaderPrg, &shaderPrgLength);

	glCompileShader(shaderHandle);

	GLint compileSuccess;
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);
	if (compileSuccess == GL_FALSE) {
		GLchar messages[256];
		glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
		printError(messages);
		abort();
	}

	return shaderHandle;
}

- (void)compileShaders {
	const char *vertexPrg =
			"uniform vec2 ScreenSize;"
			"uniform float ShakeX;"
			"uniform float ShakeY;"
			""
			"attribute vec2 Position;"
			"attribute vec2 TexCoord;"
			""
			"varying vec4 DestColor;"
			"varying vec2 o_TexCoord;"
			""
			"void main(void) {"
			"	DestColor = vec4(Position.x, Position.y, 0, 1);"
			"	o_TexCoord = TexCoord;"
			"	gl_Position = vec4(((Position.x + ShakeX) / ScreenSize.x) * 2.0 - 1.0, (1.0 - (Position.y + ShakeY) / ScreenSize.y) * 2.0 - 1.0, 0, 1);"
			"}";

	const char *fragmentPrg =
			"uniform sampler2D Texture;"
			""
			"varying lowp vec4 DestColor;"
			"varying lowp vec2 o_TexCoord;"
			""
			"void main(void) {"
			"	gl_FragColor = texture2D(Texture, o_TexCoord);"
			"}";

	_vertexShader = [self compileShader:vertexPrg withType:GL_VERTEX_SHADER];
	_fragmentShader = [self compileShader:fragmentPrg withType:GL_FRAGMENT_SHADER];

	GLuint programHandle = glCreateProgram();
	glAttachShader(programHandle, _vertexShader);
	glAttachShader(programHandle, _fragmentShader);
	glLinkProgram(programHandle);

	GLint linkSuccess;
	glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
	if (linkSuccess == GL_FALSE) {
		printOpenGLError();
		abort();
	}

	glUseProgram(programHandle);

	_screenSizeSlot = (GLuint) glGetUniformLocation(programHandle, "ScreenSize");
	_textureSlot = (GLuint) glGetUniformLocation(programHandle, "Texture");
	_shakeXSlot = (GLuint) glGetUniformLocation(programHandle, "ShakeX");
	_shakeYSlot = (GLuint) glGetUniformLocation(programHandle, "ShakeY");

	_positionSlot = (GLuint) glGetAttribLocation(programHandle, "Position");
	_textureCoordSlot = (GLuint) glGetAttribLocation(programHandle, "TexCoord");

	glEnableVertexAttribArray(_positionSlot);
	glEnableVertexAttribArray(_textureCoordSlot);

	glUniform1i(_textureSlot, 0); printOpenGLError();
}

- (void)deleteShaders {
	glDeleteShader(_vertexShader);
	glDeleteShader(_fragmentShader);
}

- (void)setupTextures {
	glGenTextures(1, &_screenTexture); printOpenGLError();
	glGenTextures(1, &_overlayTexture); printOpenGLError();
	glGenTextures(1, &_mouseCursorTexture); printOpenGLError();

	[self setGraphicsMode];
}

- (void)deleteTextures {
	if (_screenTexture) {
		glDeleteTextures(1, &_screenTexture); printOpenGLError();
		_screenTexture = 0;
	}
	if (_overlayTexture) {
		glDeleteTextures(1, &_overlayTexture); printOpenGLError();
		_overlayTexture = 0;
	}
	if (_mouseCursorTexture) {
		glDeleteTextures(1, &_mouseCursorTexture); printOpenGLError();
		_mouseCursorTexture = 0;
	}
}

- (void)setupGestureRecognizers {
#if TARGET_OS_IOS
	UIPinchGestureRecognizer *pinchKeyboard = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(keyboardPinch:)];

	UISwipeGestureRecognizer *swipeRight = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(twoFingersSwipeRight:)];
	swipeRight.direction = UISwipeGestureRecognizerDirectionRight;
	swipeRight.numberOfTouchesRequired = 2;
	swipeRight.delaysTouchesBegan = NO;
	swipeRight.delaysTouchesEnded = NO;

	UISwipeGestureRecognizer *swipeLeft = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(twoFingersSwipeLeft:)];
	swipeLeft.direction = UISwipeGestureRecognizerDirectionLeft;
	swipeLeft.numberOfTouchesRequired = 2;
	swipeLeft.delaysTouchesBegan = NO;
	swipeLeft.delaysTouchesEnded = NO;

	UISwipeGestureRecognizer *swipeUp = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(twoFingersSwipeUp:)];
	swipeUp.direction = UISwipeGestureRecognizerDirectionUp;
	swipeUp.numberOfTouchesRequired = 2;
	swipeUp.delaysTouchesBegan = NO;
	swipeUp.delaysTouchesEnded = NO;

	UISwipeGestureRecognizer *swipeDown = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(twoFingersSwipeDown:)];
	swipeDown.direction = UISwipeGestureRecognizerDirectionDown;
	swipeDown.numberOfTouchesRequired = 2;
	swipeDown.delaysTouchesBegan = NO;
	swipeDown.delaysTouchesEnded = NO;

	UISwipeGestureRecognizer *swipeRight3 = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(threeFingersSwipeRight:)];
	swipeRight3.direction = UISwipeGestureRecognizerDirectionRight;
	swipeRight3.numberOfTouchesRequired = 3;
	swipeRight3.delaysTouchesBegan = NO;
	swipeRight3.delaysTouchesEnded = NO;

	UISwipeGestureRecognizer *swipeLeft3 = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(threeFingersSwipeLeft:)];
	swipeLeft3.direction = UISwipeGestureRecognizerDirectionLeft;
	swipeLeft3.numberOfTouchesRequired = 3;
	swipeLeft3.delaysTouchesBegan = NO;
	swipeLeft3.delaysTouchesEnded = NO;

	UISwipeGestureRecognizer *swipeUp3 = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(threeFingersSwipeUp:)];
	swipeUp3.direction = UISwipeGestureRecognizerDirectionUp;
	swipeUp3.numberOfTouchesRequired = 3;
	swipeUp3.delaysTouchesBegan = NO;
	swipeUp3.delaysTouchesEnded = NO;

	UISwipeGestureRecognizer *swipeDown3 = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(threeFingersSwipeDown:)];
	swipeDown3.direction = UISwipeGestureRecognizerDirectionDown;
	swipeDown3.numberOfTouchesRequired = 3;
	swipeDown3.delaysTouchesBegan = NO;
	swipeDown3.delaysTouchesEnded = NO;

	UITapGestureRecognizer *doubleTapTwoFingers = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(twoFingersDoubleTap:)];
	doubleTapTwoFingers.numberOfTapsRequired = 2;
	doubleTapTwoFingers.numberOfTouchesRequired = 2;
	doubleTapTwoFingers.delaysTouchesBegan = NO;
	doubleTapTwoFingers.delaysTouchesEnded = NO;

	[self addGestureRecognizer:pinchKeyboard];
	[self addGestureRecognizer:swipeRight];
	[self addGestureRecognizer:swipeLeft];
	[self addGestureRecognizer:swipeUp];
	[self addGestureRecognizer:swipeDown];
	[self addGestureRecognizer:swipeRight3];
	[self addGestureRecognizer:swipeLeft3];
	[self addGestureRecognizer:swipeUp3];
	[self addGestureRecognizer:swipeDown3];
	[self addGestureRecognizer:doubleTapTwoFingers];

	[pinchKeyboard release];
	[swipeRight release];
	[swipeLeft release];
	[swipeUp release];
	[swipeDown release];
	[swipeRight3 release];
	[swipeLeft3 release];
	[swipeUp3 release];
	[swipeDown3 release];
	[doubleTapTwoFingers release];
#elif TARGET_OS_TV
	UITapGestureRecognizer *tapUpGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(threeFingersSwipeUp:)];
	[tapUpGestureRecognizer setAllowedPressTypes:@[@(UIPressTypeUpArrow)]];

	UITapGestureRecognizer *tapDownGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(threeFingersSwipeDown:)];
	[tapDownGestureRecognizer setAllowedPressTypes:@[@(UIPressTypeDownArrow)]];

	UITapGestureRecognizer *tapLeftGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(threeFingersSwipeLeft:)];
	[tapLeftGestureRecognizer setAllowedPressTypes:@[@(UIPressTypeLeftArrow)]];

	UITapGestureRecognizer *tapRightGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(threeFingersSwipeRight:)];
	[tapRightGestureRecognizer setAllowedPressTypes:@[@(UIPressTypeRightArrow)]];

	UILongPressGestureRecognizer *longPressGestureRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(showKeyboard)];
	[longPressGestureRecognizer setAllowedPressTypes:@[[NSNumber numberWithInteger:UIPressTypePlayPause]]];
	[longPressGestureRecognizer setMinimumPressDuration:1.0];

	[self addGestureRecognizer:tapUpGestureRecognizer];
	[self addGestureRecognizer:tapDownGestureRecognizer];
	[self addGestureRecognizer:tapLeftGestureRecognizer];
	[self addGestureRecognizer:tapRightGestureRecognizer];
	[self addGestureRecognizer:longPressGestureRecognizer];

	[tapUpGestureRecognizer release];
	[tapDownGestureRecognizer release];
	[tapLeftGestureRecognizer release];
	[tapRightGestureRecognizer release];
	[longPressGestureRecognizer release];
#endif
}

- (id)initWithFrame:(struct CGRect)frame {
	self = [super initWithFrame: frame];

	_backgroundSaveStateTask = UIBackgroundTaskInvalid;

	[self setupGestureRecognizers];

	if (@available(iOS 14.0, tvOS 14.0, *)) {
		_controllers.push_back([[MouseController alloc] initWithView:self]);
		_controllers.push_back([[GamepadController alloc] initWithView:self]);
	}
	_controllers.push_back([[TouchController alloc] initWithView:self]);

	[self setContentScaleFactor:[[UIScreen mainScreen] scale]];

	_keyboardView = nil;
	_screenTexture = 0;
	_overlayTexture = 0;
	_mouseCursorTexture = 0;

	_scaledShakeXOffset = 0;
	_scaledShakeYOffset = 0;

	_eventLock = [[NSLock alloc] init];

	memset(_gameScreenCoords, 0, sizeof(GLVertex) * 4);
	memset(_overlayCoords, 0, sizeof(GLVertex) * 4);
	memset(_mouseCoords, 0, sizeof(GLVertex) * 4);

	// Initialize the OpenGL ES context
	[self createContext];

	return self;
}

- (void)dealloc {
	[_keyboardView release];

	_videoContext.screenTexture.free();
	_videoContext.overlayTexture.free();
	_videoContext.mouseTexture.free();

	[_eventLock release];
	[super dealloc];
}

- (void)setFilterModeForTexture:(GLuint)tex {
	if (!tex)
		return;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex); printOpenGLError();

	GLint filter = _videoContext.filtering ? GL_LINEAR : GL_NEAREST;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter); printOpenGLError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter); printOpenGLError();
	// We use GL_CLAMP_TO_EDGE here to avoid artifacts when linear filtering
	// is used. If we would not use this for example the cursor in Loom would
	// have a line/border artifact on the right side of the covered rect.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); printOpenGLError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); printOpenGLError();
}

- (void)setGraphicsMode {
	[self setFilterModeForTexture:_screenTexture];
	[self setFilterModeForTexture:_overlayTexture];
	[self setFilterModeForTexture:_mouseCursorTexture];
}

- (void)updateSurface {
	if (!g_needsScreenUpdate) {
		return;
	}
	g_needsScreenUpdate = 0;

	glClear(GL_COLOR_BUFFER_BIT); printOpenGLError();

	[self updateMainSurface];

	if (_videoContext.overlayVisible)
		[self updateOverlaySurface];

	if (_videoContext.mouseIsVisible)
		[self updateMouseSurface];

	[_context presentRenderbuffer:GL_RENDERBUFFER];
	glFinish();
}

- (void)notifyMouseMove {
	const GLint mouseX = (GLint)(_videoContext.mouseX * _mouseScaleX) - _mouseHotspotX;
	const GLint mouseY = (GLint)(_videoContext.mouseY * _mouseScaleY) - _mouseHotspotY;

	_mouseCoords[0].x = _mouseCoords[2].x = mouseX;
	_mouseCoords[0].y = _mouseCoords[1].y = mouseY;
	_mouseCoords[1].x = _mouseCoords[3].x = mouseX + _mouseWidth;
	_mouseCoords[2].y = _mouseCoords[3].y = mouseY + _mouseHeight;
}

- (void)updateMouseCursorScaling {
	CGRect *rect;
	int maxWidth, maxHeight;

	if (!_videoContext.overlayInGUI) {
		rect = &_gameScreenRect;
		maxWidth = _videoContext.screenWidth;
		maxHeight = _videoContext.screenHeight;
	} else {
		rect = &_overlayRect;
		maxWidth = _videoContext.overlayWidth;
		maxHeight = _videoContext.overlayHeight;
	}

	if (!maxWidth || !maxHeight) {
		printf("WARNING: updateMouseCursorScaling called when screen was not ready (%d)!\n", _videoContext.overlayInGUI);
		return;
	}

	_mouseScaleX = CGRectGetWidth(*rect) / (GLfloat)maxWidth;
	_mouseScaleY = CGRectGetHeight(*rect) / (GLfloat)maxHeight;

	_mouseWidth = (GLint)(_videoContext.mouseWidth * _mouseScaleX);
	_mouseHeight = (GLint)(_videoContext.mouseHeight * _mouseScaleY);

	_mouseHotspotX = (GLint)(_videoContext.mouseHotspotX * _mouseScaleX);
	_mouseHotspotY = (GLint)(_videoContext.mouseHotspotY * _mouseScaleY);

	// We subtract the screen offset to the hotspot here to simplify the
	// screen offset handling in the mouse code. Note the subtraction here
	// makes sure that the offset actually gets added to the mouse position,
	// since the hotspot offset is substracted from the position.
	_mouseHotspotX -= (GLint)CGRectGetMinX(*rect);
	_mouseHotspotY -= (GLint)CGRectGetMinY(*rect);

	// FIXME: For now we also adapt the mouse position here. In reality we
	// would be better off to also adjust the event position when switching
	// from overlay to game screen or vica versa.
	[self notifyMouseMove];
}

- (void)updateMouseCursor {
	[self updateMouseCursorScaling];

	_mouseCoords[1].u = _mouseCoords[3].u = (_videoContext.mouseWidth - 1) / (GLfloat)_videoContext.mouseTexture.w;
	_mouseCoords[2].v = _mouseCoords[3].v = (_videoContext.mouseHeight - 1) / (GLfloat)_videoContext.mouseTexture.h;

	[self setFilterModeForTexture:_mouseCursorTexture];
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _videoContext.mouseTexture.w, _videoContext.mouseTexture.h, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, _videoContext.mouseTexture.getPixels()); printOpenGLError();
}

- (void *)getTextureInRGBA8888BE_AsRGBA8888LE {
	// Allocate a pixel buffer with 32 bits per pixel
	void *pixelBuffer = malloc(_videoContext.screenTexture.h * _videoContext.screenTexture.w * sizeof(uint32_t));
	// Copy the texture pixels as we don't want to operate on the
	memcpy(pixelBuffer, _videoContext.screenTexture.getPixels(), _videoContext.screenTexture.h * _videoContext.screenTexture.w * sizeof(uint32_t));

	// Utilize the Accelerator Framwork to do some byte swapping
	vImage_Buffer src;
	src.height = _videoContext.screenTexture.h;
	src.width = _videoContext.screenTexture.w;
	src.rowBytes = _videoContext.screenTexture.pitch;
	src.data = _videoContext.screenTexture.getPixels();

	// Initialise dst with src, change data pointer to pixelBuffer
	vImage_Buffer dst = src;
	dst.data = pixelBuffer;

	// Swap pixel channels from RGBA BE to RGBA LE (ABGR)
	const uint8_t map[4] = { 3, 2, 1, 0 };
	vImagePermuteChannels_ARGB8888(&src, &dst, map, kvImageNoFlags);

	return pixelBuffer;
}

- (void)updateMainSurface {
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLVertex) * 4, _gameScreenCoords, GL_STATIC_DRAW);
	glVertexAttribPointer(_positionSlot, 2, GL_FLOAT, GL_FALSE, sizeof(GLVertex), 0);
	glVertexAttribPointer(_textureCoordSlot, 2, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *) (sizeof(GLfloat) * 2));

	[self setFilterModeForTexture:_screenTexture];

	// Unfortunately we have to update the whole texture every frame, since glTexSubImage2D is actually slower in all cases
	// due to the iPhone internals having to convert the whole texture back from its internal format when used.
	// In the future we could use several tiled textures instead.
	if (_videoContext.screenTexture.format == Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24)) {
		// ABGR8888 in big endian which in little endian is RBGA8888 -> no convertion needed
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _videoContext.screenTexture.w, _videoContext.screenTexture.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, _videoContext.screenTexture.getPixels()); printOpenGLError();
	} else if (_videoContext.screenTexture.format == Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0)) {
		// RGBA8888 (big endian) = ABGR8888 (little endian) -> needs convertion
		void* pixelBuffer = [self getTextureInRGBA8888BE_AsRGBA8888LE];
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _videoContext.screenTexture.w, _videoContext.screenTexture.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer); printOpenGLError();
		free(pixelBuffer);
	} else {
		// Assuming RGB565
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _videoContext.screenTexture.w, _videoContext.screenTexture.h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, _videoContext.screenTexture.getPixels()); printOpenGLError();
	}
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); printOpenGLError();
}

- (void)updateOverlaySurface {
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLVertex) * 4, _overlayCoords, GL_STATIC_DRAW);
	glVertexAttribPointer(_positionSlot, 2, GL_FLOAT, GL_FALSE, sizeof(GLVertex), 0);
	glVertexAttribPointer(_textureCoordSlot, 2, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *) (sizeof(GLfloat) * 2));

	[self setFilterModeForTexture:_overlayTexture];

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _videoContext.overlayTexture.w, _videoContext.overlayTexture.h, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, _videoContext.overlayTexture.getPixels()); printOpenGLError();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); printOpenGLError();
}

- (void)updateMouseSurface {
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLVertex) * 4, _mouseCoords, GL_STATIC_DRAW);
	glVertexAttribPointer(_positionSlot, 2, GL_FLOAT, GL_FALSE, sizeof(GLVertex), 0);
	glVertexAttribPointer(_textureCoordSlot, 2, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *) (sizeof(GLfloat) * 2));

	glBindTexture(GL_TEXTURE_2D, _mouseCursorTexture); printOpenGLError();

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); printOpenGLError();
}

- (void)setGameScreenCoords{
	const uint screenTexWidth = getSizeNextPOT(_videoContext.screenWidth);
	const uint screenTexHeight = getSizeNextPOT(_videoContext.screenHeight);

	_gameScreenCoords[1].u = _gameScreenCoords[3].u = _videoContext.screenWidth / (GLfloat)screenTexWidth;
	_gameScreenCoords[2].v = _gameScreenCoords[3].v = _videoContext.screenHeight / (GLfloat)screenTexHeight;
}

- (void)initSurface {
	if (_context) {
		[self rebuildFrameBuffer];
	}

#if TARGET_OS_IOS
	UIInterfaceOrientation interfaceOrientation = UIInterfaceOrientationUnknown;
	if (@available(iOS 13.0, *)) {
		interfaceOrientation = [[[self window] windowScene] interfaceOrientation];
	} else {
		interfaceOrientation = [[UIApplication sharedApplication] statusBarOrientation];
	}
	BOOL isLandscape = UIInterfaceOrientationIsLandscape(interfaceOrientation);
#else // TVOS
	BOOL isLandscape = YES;
#endif

	int screenWidth, screenHeight;
	if (isLandscape) {
		screenWidth = MAX(_renderBufferWidth, _renderBufferHeight);
		screenHeight = MIN(_renderBufferWidth, _renderBufferHeight);
	} else {
		screenWidth = MIN(_renderBufferWidth, _renderBufferHeight);
		screenHeight = MAX(_renderBufferWidth, _renderBufferHeight);
	}

	if (_keyboardView == nil) {
		_keyboardView = [[SoftKeyboard alloc] initWithFrame:CGRectZero];
		[_keyboardView setInputDelegate:self];
		[self addSubview:[_keyboardView inputView]];
		[self addSubview: _keyboardView];
		[self showKeyboard];
	}

	glBindRenderbuffer(GL_RENDERBUFFER, _viewRenderbuffer); printOpenGLError();

	[self clearColorBuffer];

	GLfloat adjustedWidth = _videoContext.screenWidth;
	GLfloat adjustedHeight = _videoContext.screenHeight;
	if (_videoContext.asprectRatioCorrection) {
		if (_videoContext.screenWidth == 320 && _videoContext.screenHeight == 200)
			adjustedHeight = 240;
		else if (_videoContext.screenWidth == 640 && _videoContext.screenHeight == 400)
			adjustedHeight = 480;
	}

	float overlayPortraitRatio;

	if (isLandscape) {
		GLfloat gameScreenRatio = adjustedWidth / adjustedHeight;
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
		GLfloat ratio = adjustedHeight / adjustedWidth;
		int height = (int)(screenWidth * ratio);
		//printf("Making rect (%u, %u)\n", screenWidth, height);

		_gameScreenRect = CGRectMake(0, 0, screenWidth, height);

		overlayPortraitRatio = (_videoContext.overlayHeight * ratio) / _videoContext.overlayWidth;
	}
	_overlayRect = CGRectMake(0, 0, screenWidth, screenHeight * overlayPortraitRatio);

	_gameScreenCoords[0].x = _gameScreenCoords[2].x = CGRectGetMinX(_gameScreenRect);
	_gameScreenCoords[0].y = _gameScreenCoords[1].y = CGRectGetMinY(_gameScreenRect);
	_gameScreenCoords[1].x = _gameScreenCoords[3].x = CGRectGetMaxX(_gameScreenRect);
	_gameScreenCoords[2].y = _gameScreenCoords[3].y = CGRectGetMaxY(_gameScreenRect);

	_overlayCoords[1].x = _overlayCoords[3].x = CGRectGetMaxX(_overlayRect);
	_overlayCoords[2].y = _overlayCoords[3].y = CGRectGetMaxY(_overlayRect);

	[self setViewTransformation];
	[self updateMouseCursorScaling];
	[self adjustViewFrameForSafeArea];
}

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

-(void)adjustViewFrameForSafeArea {
	// The code below does not quite compile with SDKs older than 11.0.
	// warning: instance method '-safeAreaInsets' not found (return type defaults to 'id')
	// error: no viable conversion from 'id' to 'UIEdgeInsets'
	// So for now disable this code when compiled with an older SDK, which means it is only
	// available when running on iOS 11+ if it has been compiled on iOS 11+
#ifdef __IPHONE_11_0
	if ( @available(iOS 11, tvOS 11, *) ) {
		CGRect screenSize = [[UIScreen mainScreen] bounds];
		CGRect newFrame = screenSize;
#if TARGET_OS_IOS
		UIEdgeInsets inset = [[[UIApplication sharedApplication] keyWindow] safeAreaInsets];
		UIInterfaceOrientation orientation = UIInterfaceOrientationUnknown;
		if (@available(iOS 13.0, *)) {
			orientation = [[[self window] windowScene] interfaceOrientation];
		} else {
			orientation = [[UIApplication sharedApplication] statusBarOrientation];
		}
		if ( orientation == UIInterfaceOrientationPortrait ) {
			newFrame = CGRectMake(screenSize.origin.x, screenSize.origin.y + inset.top, screenSize.size.width, screenSize.size.height - inset.top);
		} else if ( orientation == UIInterfaceOrientationPortraitUpsideDown ) {
			newFrame = CGRectMake(screenSize.origin.x, screenSize.origin.y, screenSize.size.width, screenSize.size.height - inset.top);
		} else if ( orientation == UIInterfaceOrientationLandscapeLeft ) {
			newFrame = CGRectMake(screenSize.origin.x, screenSize.origin.y, screenSize.size.width - inset.right, screenSize.size.height);
		} else if ( orientation == UIInterfaceOrientationLandscapeRight ) {
			newFrame = CGRectMake(screenSize.origin.x + inset.left, screenSize.origin.y, screenSize.size.width - inset.left, screenSize.size.height);
		}
#endif
		self.frame = newFrame;
	}
#endif
}

#ifdef __IPHONE_11_0
// This delegate method is called when the safe area of the view changes
-(void)safeAreaInsetsDidChange {
	[self adjustViewFrameForSafeArea];
}
#endif

- (void)setViewTransformation {
	// Scale the shake offset according to the overlay size. We need this to
	// adjust the overlay mouse click coordinates when an offset is set.
	_scaledShakeXOffset = (int)(_videoContext.shakeXOffset / (GLfloat)_videoContext.screenWidth * CGRectGetWidth(_overlayRect));
	_scaledShakeYOffset = (int)(_videoContext.shakeYOffset / (GLfloat)_videoContext.screenHeight * CGRectGetHeight(_overlayRect));

	glUniform1f(_shakeXSlot, _scaledShakeXOffset);
	glUniform1f(_shakeYSlot, _scaledShakeYOffset);
}

- (void)clearColorBuffer {
	// The color buffer is triple-buffered, so we clear it multiple times right away to avid doing any glClears later.
	int clearCount = 5;
	while (clearCount-- > 0) {
		glClear(GL_COLOR_BUFFER_BIT); printOpenGLError();
		[_context presentRenderbuffer:GL_RENDERBUFFER];
		glFinish();
	}
}

- (void)addEvent:(InternalEvent)event {
	[_eventLock lock];
	_events.push_back(event);
	[_eventLock unlock];
}

- (bool)fetchEvent:(InternalEvent *)event {
	[_eventLock lock];
	if (_events.empty()) {
		[_eventLock unlock];
		return false;
	}

	*event = *_events.begin();
	_events.pop_front();
	[_eventLock unlock];
	return true;
}

- (bool)getMouseCoords:(CGPoint)point eventX:(int *)x eventY:(int *)y {
	// We scale the input according to our scale factor to get actual screen
	// coordinates.
	point.x *= self.contentScaleFactor;
	point.y *= self.contentScaleFactor;

	CGRect *area;
	int width, height, offsetX, offsetY;
	if (_videoContext.overlayInGUI) {
		area = &_overlayRect;
		width = _videoContext.overlayWidth;
		height = _videoContext.overlayHeight;
		offsetX = _scaledShakeXOffset;
		offsetY = _scaledShakeYOffset;
	} else {
		area = &_gameScreenRect;
		width = _videoContext.screenWidth;
		height = _videoContext.screenHeight;
		offsetX = _videoContext.shakeXOffset;
		offsetY = _videoContext.shakeYOffset;
	}

	point.x = (point.x - CGRectGetMinX(*area)) / CGRectGetWidth(*area);
	point.y = (point.y - CGRectGetMinY(*area)) / CGRectGetHeight(*area);

	*x = (int)(point.x * width + offsetX);
	// offsetY describes the translation of the screen in the upward direction,
	// thus we need to add it here.
	*y = (int)(point.y * height + offsetY);

	if (!iOS7_touchpadModeEnabled()) {
		// Clip coordinates
		if (*x < 0 || *x > width || *y < 0 || *y > height)
			return false;
	}

	return true;
}

- (BOOL)isControllerTypeConnected:(Class)controller {
	for (GameController *c : _controllers) {
		if ([c isConnected]) {
			if ([c isKindOfClass:controller]) {
				return YES;
			}
		}
	}
	return NO;
}

- (BOOL)isTouchControllerConnected {
	return [self isControllerTypeConnected:TouchController.class];
}

- (BOOL)isMouseControllerConnected {
	if (@available(iOS 14.0, tvOS 14.0, *)) {
		return [self isControllerTypeConnected:MouseController.class];
	} else {
		// Fallback on earlier versions
		return NO;
	}
}

- (BOOL)isGamepadControllerConnected {
	if (@available(iOS 14.0, tvOS 14.0, *)) {
		return [self isControllerTypeConnected:GamepadController.class];
	} else {
		// Fallback on earlier versions
		return NO;
	}
}

- (void)virtualController:(bool)connect {
	if (@available(iOS 15.0, *)) {
		for (GameController *c : _controllers) {
			if ([c isKindOfClass:GamepadController.class]) {
				[(GamepadController*)c virtualController:connect];
			}
		}
	}
}

#if TARGET_OS_IOS
- (void)interfaceOrientationChanged:(UIInterfaceOrientation)orientation {
	[self addEvent:InternalEvent(kInputOrientationChanged, orientation, 0)];
	if (UIInterfaceOrientationIsLandscape(orientation)) {
		[self hideKeyboard];
	} else {
		[self showKeyboard];
	}
}
#endif

- (void)showKeyboard {
	[_keyboardView showKeyboard];
}

- (void)hideKeyboard {
	[_keyboardView hideKeyboard];
}

- (BOOL)isKeyboardShown {
	return [_keyboardView isKeyboardShown];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	for (GameController *c : _controllers) {
		if ([c isKindOfClass:TouchController.class]) {
			[(TouchController *)c touchesBegan:touches withEvent:event];
		}
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	for (GameController *c : _controllers) {
		if ([c isKindOfClass:TouchController.class]) {
			[(TouchController *)c touchesMoved:touches withEvent:event];
		}
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	for (GameController *c : _controllers) {
		if ([c isKindOfClass:TouchController.class]) {
			[(TouchController *)c touchesEnded:touches withEvent:event];
		}
	}
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	for (GameController *c : _controllers) {
		if ([c isKindOfClass:TouchController.class]) {
			[(TouchController *)c touchesCancelled:touches withEvent:event];
		}
	}
}

#if TARGET_OS_TV
// UIKit calls these methods when a button is pressed by the user.
// These methods are used to determine which button was pressed and
// to take any needed actions. The default implementation of these
// methods forwardsm the message up the responder chain.
// Button presses are already handled by the GameController class for
// connected game controllers (including the Apple TV remote).
// The Apple TV remote is not registered as a micro game controller
// when running the application in tvOS simulator, hence these methods
// only needs to be implemented for the tvOS simulator to handle presses
// on the Apple TV remote.
-(void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
#if TARGET_OS_SIMULATOR
	UIPress *press = [presses anyObject];
	if (press.type == UIPressTypeMenu) {
		// Trigger on pressesEnded
	} else if (press.type == UIPressTypeSelect || press.type == UIPressTypePlayPause) {
		[self addEvent:InternalEvent(kInputJoystickButtonDown, press.type == UIPressTypeSelect ? Common::JOYSTICK_BUTTON_A : Common::JOYSTICK_BUTTON_B, 0)];
	}
	else {
		[super pressesBegan:presses withEvent:event];
	}
#endif
}

-(void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
#if TARGET_OS_SIMULATOR
	UIPress *press = [presses anyObject];
	if (press.type == UIPressTypeMenu) {
		[self handleMainMenuKey];
	} else if (press.type == UIPressTypeSelect || press.type == UIPressTypePlayPause) {
		[self addEvent:InternalEvent(kInputJoystickButtonUp, press.type == UIPressTypeSelect ? Common::JOYSTICK_BUTTON_A : Common::JOYSTICK_BUTTON_B, 0)];
	}
	else {
		[super pressesEnded:presses withEvent:event];
	}
#endif
}

-(void)pressesChanged:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
#if TARGET_OS_SIMULATOR
	[super pressesChanged:presses withEvent:event];
#endif
}

-(void)pressesCancelled:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
#if TARGET_OS_SIMULATOR
	[super pressesCancelled:presses withEvent:event];
#endif
}
#endif

#if TARGET_OS_IOS
- (void)keyboardPinch:(UIPinchGestureRecognizer *)recognizer {
	if ([recognizer scale] < 0.8)
		[self showKeyboard];
	else if ([recognizer scale] > 1.25)
		[self hideKeyboard];
}
#endif

- (void)twoFingersSwipeRight:(UISwipeGestureRecognizer *)recognizer {
	[self addEvent:InternalEvent(kInputSwipe, kUIViewSwipeRight, 2)];
}

- (void)twoFingersSwipeLeft:(UISwipeGestureRecognizer *)recognizer {
	[self addEvent:InternalEvent(kInputSwipe, kUIViewSwipeLeft, 2)];
}

- (void)twoFingersSwipeUp:(UISwipeGestureRecognizer *)recognizer {
	[self addEvent:InternalEvent(kInputSwipe, kUIViewSwipeUp, 2)];
}

- (void)twoFingersSwipeDown:(UISwipeGestureRecognizer *)recognizer {
	[self addEvent:InternalEvent(kInputSwipe, kUIViewSwipeDown, 2)];
}

- (void)threeFingersSwipeRight:(UISwipeGestureRecognizer *)recognizer {
	[self addEvent:InternalEvent(kInputSwipe, kUIViewSwipeRight, 3)];
}

- (void)threeFingersSwipeLeft:(UISwipeGestureRecognizer *)recognizer {
	[self addEvent:InternalEvent(kInputSwipe, kUIViewSwipeLeft, 3)];
}

- (void)threeFingersSwipeUp:(UISwipeGestureRecognizer *)recognizer {
	[self addEvent:InternalEvent(kInputSwipe, kUIViewSwipeUp, 3)];
}

- (void)threeFingersSwipeDown:(UISwipeGestureRecognizer *)recognizer {
	[self addEvent:InternalEvent(kInputSwipe, kUIViewSwipeDown, 3)];
}

- (void)twoFingersDoubleTap:(UITapGestureRecognizer *)recognizer {
	[self addEvent:InternalEvent(kInputTap, kUIViewTapDouble, 2)];
}

- (void)handleKeyPress:(unichar)c {
	if (c == '`') {
		[self addEvent:InternalEvent(kInputKeyPressed, '\033', 0)];
	} else {
		[self addEvent:InternalEvent(kInputKeyPressed, c, 0)];
	}
}

- (void)handleMainMenuKey {
	if ([self isInGame]) {
		[self addEvent:InternalEvent(kInputMainMenu, 0, 0)];
	} else {
		[[UIApplication sharedApplication] performSelector:@selector(suspend)];
	}
}

- (void)applicationSuspend {
	[self addEvent:InternalEvent(kInputApplicationSuspended, 0, 0)];
}

- (void)applicationResume {
	[self addEvent:InternalEvent(kInputApplicationResumed, 0, 0)];
}

- (void)saveApplicationState {
	[self addEvent:InternalEvent(kInputApplicationSaveState, 0, 0)];
}

- (void)clearApplicationState {
	[self addEvent:InternalEvent(kInputApplicationClearState, 0, 0)];
}

- (void)restoreApplicationState {
	[self addEvent:InternalEvent(kInputApplicationRestoreState, 0, 0)];
}

- (void) beginBackgroundSaveStateTask {
	if (_backgroundSaveStateTask == UIBackgroundTaskInvalid) {
		_backgroundSaveStateTask = [[UIApplication sharedApplication] beginBackgroundTaskWithExpirationHandler:^{
			[self endBackgroundSaveStateTask];
		}];
	}
}

- (void) endBackgroundSaveStateTask {
	if (_backgroundSaveStateTask != UIBackgroundTaskInvalid) {
		[[UIApplication sharedApplication] endBackgroundTask: _backgroundSaveStateTask];
		_backgroundSaveStateTask = UIBackgroundTaskInvalid;
	}
}

@end
