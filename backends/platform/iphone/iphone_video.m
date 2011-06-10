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
#include "iphone_common.h"

static iPhoneView *sharedInstance = nil;
static int _width = 0;
static int _height = 0;
static int _fullWidth;
static int _fullHeight;
static CGRect _screenRect;

static char* _textureBuffer = 0;
static int _textureWidth = 0;
static int _textureHeight = 0;

static char* _overlayTexBuffer = 0;
static int _overlayTexWidth = 0;
static int _overlayTexHeight = 0;
static int _overlayWidth = 0;
static int _overlayHeight = 0;
static float _overlayPortraitRatio = 1.0f;

NSLock* _lock = nil;
static int _needsScreenUpdate = 0;
static int _overlayIsEnabled = 0;

static UITouch* _firstTouch = NULL;
static UITouch* _secondTouch = NULL;

static short* _mouseCursor = NULL;
static int _mouseCursorHeight = 0;
static int _mouseCursorWidth = 0;
static int _mouseX = 0;
static int _mouseY = 0;

// static long lastTick = 0;
// static int frames = 0;

#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(const char *file, int line)
{
	int     retCode = 0;

	// returns 1 if an OpenGL error occurred, 0 otherwise.
	GLenum glErr = glGetError();
	while( glErr != GL_NO_ERROR)
	{
		fprintf(stderr, "glError: %u (%s: %d)\n", glErr, file, line );
		retCode = 1;
		glErr = glGetError();
	}
	return retCode;
}

void iPhone_setMouseCursor(short* buffer, int width, int height) {
	_mouseCursor = buffer;

	_mouseCursorWidth = width;
	_mouseCursorHeight = height;

	[sharedInstance performSelectorOnMainThread:@selector(updateMouseCursor) withObject:nil waitUntilDone: YES];
}

void iPhone_enableOverlay(int state) {
	_overlayIsEnabled = state;

	[sharedInstance performSelectorOnMainThread:@selector(clearColorBuffer) withObject:nil waitUntilDone: YES];
}

int iPhone_getScreenHeight() {
	return _overlayHeight;
}

int iPhone_getScreenWidth() {
	return _overlayWidth;
}

bool iPhone_isHighResDevice() {
	return _fullHeight > 480;
}

void iPhone_updateScreen(int mouseX, int mouseY) {
	//printf("Mouse: (%i, %i)\n", mouseX, mouseY);

	//_mouseX = _overlayHeight - (float)mouseX / _width * _overlayHeight;
	//_mouseY = (float)mouseY / _height * _overlayWidth;

	//_mouseX = _overlayHeight - mouseX;
	//_mouseY = mouseY;

	_mouseX = (_overlayWidth - mouseX) / (float)_overlayWidth * _overlayHeight;
	_mouseY = mouseY / (float)_overlayHeight * _overlayWidth;

	if (!_needsScreenUpdate) {
		_needsScreenUpdate = 1;
		[sharedInstance performSelectorOnMainThread:@selector(updateSurface) withObject:nil waitUntilDone: NO];
	}
}

void iPhone_updateScreenRect(unsigned short* screen, int x1, int y1, int x2, int y2) {
	int y;
	for (y = y1; y < y2; ++y)
		memcpy(&_textureBuffer[(y * _textureWidth + x1 )* 2], &screen[y * _width + x1], (x2 - x1) * 2);
}

void iPhone_updateOverlayRect(unsigned short* screen, int x1, int y1, int x2, int y2) {
	int y;
	//printf("Overlaywidth: %u, fullwidth %u\n", _overlayWidth, _fullWidth);
	for (y = y1; y < y2; ++y)
		memcpy(&_overlayTexBuffer[(y * _overlayTexWidth + x1 )* 2], &screen[y * _overlayWidth + x1], (x2 - x1) * 2);
}

void iPhone_initSurface(int width, int height) {
	_width = width;
	_height = height;
	[sharedInstance performSelectorOnMainThread:@selector(initSurface) withObject:nil waitUntilDone: YES];
}

bool iPhone_fetchEvent(int *outEvent, float *outX, float *outY) {
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
	*outX = [[event objectForKey:@"x"] floatValue];
	*outY = [[event objectForKey:@"y"] floatValue];
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

const char* iPhone_getDocumentsDir() {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	return [documentsDirectory UTF8String];
}

bool getLocalMouseCoords(CGPoint *point) {
	if (_overlayIsEnabled) {
		point->x = point->x / _overlayHeight;
		point->y = point->y / _overlayWidth;
	} else {
		if (point->x < _screenRect.origin.x || point->x >= _screenRect.origin.x + _screenRect.size.width ||
			point->y < _screenRect.origin.y || point->y >= _screenRect.origin.y + _screenRect.size.height) {
				return false;
		}

		point->x = (point->x - _screenRect.origin.x) / _screenRect.size.width;
		point->y = (point->y - _screenRect.origin.y) / _screenRect.size.height;
	}

	return true;
}


@implementation iPhoneView

+ (Class) layerClass
{
	return [CAEAGLLayer class];
}

- (id)initWithFrame:(struct CGRect)frame {
	self = [super initWithFrame: frame];

	if([[UIScreen mainScreen] respondsToSelector: NSSelectorFromString(@"scale")])
	{
		if([self respondsToSelector: NSSelectorFromString(@"contentScaleFactor")])
		{
			//self.contentScaleFactor = [[UIScreen mainScreen] scale];
		}
	}

	_fullWidth = frame.size.width;
	_fullHeight = frame.size.height;
	_screenLayer = nil;

	sharedInstance = self;

	_lock = [NSLock new];
	_keyboardView = nil;
	_context = nil;
	_screenTexture = 0;
	_overlayTexture = 0;
	_mouseCursorTexture = 0;

	return self;
}

-(void) dealloc {
	[super dealloc];

	if (_keyboardView != nil) {
		[_keyboardView dealloc];
	}

	if (_screenTexture)
		free(_textureBuffer);

	free(_overlayTexBuffer);
}

- (void *)getSurface {
	return _screenSurface;
}

- (void)drawRect:(CGRect)frame {
	// if (lastTick == 0) {
	//	lastTick = time(0);
	// }
	//
	// frames++;
	// if (time(0) > lastTick) {
	//	lastTick = time(0);
	//	printf("FPS: %i\n", frames);
	//	frames = 0;
	// }
}

- (void)updateSurface {
	if (!_needsScreenUpdate) {
		return;
	}
	_needsScreenUpdate = 0;

	if (_overlayIsEnabled) {
		glClear(GL_COLOR_BUFFER_BIT); printOpenGLError();
	}

	[self updateMainSurface];

	if (_overlayIsEnabled) {
		[self updateOverlaySurface];
		[self updateMouseSurface];
	}

	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer); printOpenGLError();
	[_context presentRenderbuffer:GL_RENDERBUFFER_OES];

}

-(void)updateMouseCursor {
	if (_mouseCursorTexture == 0) {
		glGenTextures(1, &_mouseCursorTexture); printOpenGLError();
		glBindTexture(GL_TEXTURE_2D, _mouseCursorTexture); printOpenGLError();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); printOpenGLError();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); printOpenGLError();
	}

	glBindTexture(GL_TEXTURE_2D, _mouseCursorTexture); printOpenGLError();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getSizeNextPOT(_mouseCursorWidth), getSizeNextPOT(_mouseCursorHeight), 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, _mouseCursor); printOpenGLError();

	free(_mouseCursor);
	_mouseCursor = NULL;
}

- (void)updateMainSurface {
	GLfloat vertices[] = {
		0.0f + _heightOffset, 0.0f + _widthOffset,
		_visibleWidth - _heightOffset, 0.0f + _widthOffset,
		0.0f + _heightOffset,  _visibleHeight - _widthOffset,
		_visibleWidth - _heightOffset,  _visibleHeight - _widthOffset
	};

	float texWidth = _width / (float)_textureWidth;
	float texHeight = _height / (float)_textureHeight;

	const GLfloat texCoords[] = {
		texWidth, 0.0f,
		0.0f, 0.0f,
		texWidth, texHeight,
		0.0f, texHeight
	};

	glVertexPointer(2, GL_FLOAT, 0, vertices); printOpenGLError();
	glTexCoordPointer(2, GL_FLOAT, 0, texCoords); printOpenGLError();

	glBindTexture(GL_TEXTURE_2D, _screenTexture); printOpenGLError();

	// Unfortunately we have to update the whole texture every frame, since glTexSubImage2D is actually slower in all cases
	// due to the iPhone internals having to convert the whole texture back from its internal format when used.
	// In the future we could use several tiled textures instead.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _textureWidth, _textureHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, _textureBuffer); printOpenGLError();
	glDisable(GL_BLEND);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); printOpenGLError();
}

- (void)updateOverlaySurface {
	GLfloat vertices[] = {
		0.0f, 0.0f,
		_overlayHeight, 0.0f,
		0.0f,  _overlayWidth * _overlayPortraitRatio,
		_overlayHeight,  _overlayWidth * _overlayPortraitRatio
	};

	float texWidth = _overlayWidth / (float)_overlayTexWidth;
	float texHeight = _overlayHeight / (float)_overlayTexHeight;

	const GLfloat texCoords[] = {
		texWidth, 0.0f,
		0.0f, 0.0f,
		texWidth, texHeight,
		0.0f, texHeight
	};

	glVertexPointer(2, GL_FLOAT, 0, vertices); printOpenGLError();
	glTexCoordPointer(2, GL_FLOAT, 0, texCoords); printOpenGLError();

	glBindTexture(GL_TEXTURE_2D, _overlayTexture); printOpenGLError();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _overlayTexWidth, _overlayTexHeight, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, _overlayTexBuffer); printOpenGLError();
	glEnable(GL_BLEND);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); printOpenGLError();
}

- (void)updateMouseSurface {

	int width = _mouseCursorWidth / (float)_backingWidth * _backingHeight;
	int height = _mouseCursorHeight / (float)_backingHeight * _backingWidth;

	GLfloat vertices[] = {
		_mouseX, _mouseY,
		_mouseX + height, _mouseY,
		_mouseX, _mouseY + width,
		_mouseX + height,  _mouseY + width
	};

	//printf("Cursor: width %u height %u\n", _mouseCursorWidth, _mouseCursorHeight);

	float texWidth = _mouseCursorWidth / (float)getSizeNextPOT(_mouseCursorWidth);
	float texHeight = _mouseCursorHeight / (float)getSizeNextPOT(_mouseCursorHeight);

	const GLfloat texCoords[] = {
		texWidth, 0.0f,
		0.0f, 0.0f,
		texWidth, texHeight,
		0.0f, texHeight
	};

	glVertexPointer(2, GL_FLOAT, 0, vertices); printOpenGLError();
	glTexCoordPointer(2, GL_FLOAT, 0, texCoords); printOpenGLError();

	glBindTexture(GL_TEXTURE_2D, _mouseCursorTexture); printOpenGLError();
	glEnable(GL_BLEND);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); printOpenGLError();
}

- (void)initSurface {
	_textureWidth = getSizeNextPOT(_width);
	_textureHeight = getSizeNextPOT(_height);

	UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];

	//printf("Window: (%d, %d), Surface: (%d, %d), Texture(%d, %d)\n", _fullWidth, _fullHeight, _width, _height, _textureWidth, _textureHeight);

	if (_context == nil) {
		orientation = UIDeviceOrientationLandscapeRight;
		CAEAGLLayer *eaglLayer = (CAEAGLLayer*) self.layer;

		eaglLayer.opaque = YES;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
										[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGB565, kEAGLDrawablePropertyColorFormat, nil];

		_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
		if (!_context || [EAGLContext setCurrentContext:_context]) {
			glGenFramebuffersOES(1, &_viewFramebuffer); printOpenGLError();
			glGenRenderbuffersOES(1, &_viewRenderbuffer); printOpenGLError();

			glBindFramebufferOES(GL_FRAMEBUFFER_OES, _viewFramebuffer); printOpenGLError();
			glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer); printOpenGLError();
			[_context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(id<EAGLDrawable>)self.layer];
			glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, _viewRenderbuffer); printOpenGLError();

			glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &_backingWidth); printOpenGLError();
			glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &_backingHeight); printOpenGLError();

			if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
				NSLog(@"Failed to make complete framebuffer object %x.", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
				return;
			}

			_overlayHeight = _backingWidth;
			_overlayWidth = _backingHeight;
			_overlayTexWidth = getSizeNextPOT(_overlayHeight);
			_overlayTexHeight = getSizeNextPOT(_overlayWidth);

			int textureSize = _overlayTexWidth * _overlayTexHeight * 2;
			_overlayTexBuffer = (char *)malloc(textureSize);
			memset(_overlayTexBuffer, 0, textureSize);

			glViewport(0, 0, _backingWidth, _backingHeight); printOpenGLError();
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f); printOpenGLError();

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_TEXTURE_2D); printOpenGLError();
			glEnableClientState(GL_TEXTURE_COORD_ARRAY); printOpenGLError();
			glEnableClientState(GL_VERTEX_ARRAY); printOpenGLError();
		}
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (orientation ==  UIDeviceOrientationLandscapeRight) {
		glRotatef(-90, 0, 0, 1); printOpenGLError();
	} else if (orientation == UIDeviceOrientationLandscapeLeft) {
		glRotatef(90, 0, 0, 1); printOpenGLError();
	} else {
		glRotatef(180, 0, 0, 1); printOpenGLError();
	}

	glOrthof(0, _backingWidth, 0, _backingHeight, 0, 1); printOpenGLError();

	if (_screenTexture > 0) {
		glDeleteTextures(1, &_screenTexture); printOpenGLError();
	}

	glGenTextures(1, &_screenTexture); printOpenGLError();
	glBindTexture(GL_TEXTURE_2D, _screenTexture); printOpenGLError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); printOpenGLError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); printOpenGLError();

	if (_overlayTexture > 0) {
		glDeleteTextures(1, &_overlayTexture); printOpenGLError();
	}

	glGenTextures(1, &_overlayTexture); printOpenGLError();
	glBindTexture(GL_TEXTURE_2D, _overlayTexture); printOpenGLError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); printOpenGLError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); printOpenGLError();

	if (_textureBuffer) {
		free(_textureBuffer);
	}

	int textureSize = _textureWidth * _textureHeight * 2;
	_textureBuffer = (char*)malloc(textureSize);
	memset(_textureBuffer, 0, textureSize);

	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer); printOpenGLError();

	[self clearColorBuffer];

	if (_keyboardView != nil) {
		[_keyboardView removeFromSuperview];
		[[_keyboardView inputView] removeFromSuperview];
	}

	if (orientation == UIDeviceOrientationLandscapeLeft || orientation ==  UIDeviceOrientationLandscapeRight) {
		_visibleHeight = _backingHeight;
		_visibleWidth = _backingWidth;

		float ratioDifference = ((float)_height / (float)_width) / ((float)_fullWidth / (float)_fullHeight);
		int rectWidth, rectHeight;
		if (ratioDifference < 1.0f) {
			rectWidth = _fullWidth * ratioDifference;
			rectHeight = _fullHeight;
			_widthOffset = (_fullWidth - rectWidth) / 2;
			_heightOffset = 0;
		} else {
			rectWidth = _fullWidth;
			rectHeight = _fullHeight / ratioDifference;
			_heightOffset = (_fullHeight - rectHeight) / 2;
			_widthOffset = 0;
		}

		//printf("Rect: %i, %i, %i, %i\n", _widthOffset, _heightOffset, rectWidth, rectHeight);
		_screenRect = CGRectMake(_widthOffset, _heightOffset, rectWidth, rectHeight);
		_overlayPortraitRatio = 1.0f;
	} else {
		float ratio = (float)_height / (float)_width;
		int height = _fullWidth * ratio;
		//printf("Making rect (%u, %u)\n", _fullWidth, height);
		_screenRect = CGRectMake(0, 0, _fullWidth - 1, height - 1);

		_visibleHeight = height;
		_visibleWidth = _backingWidth;
		_heightOffset = 0.0f;
		_widthOffset = 0.0f;

		CGRect keyFrame = CGRectMake(0.0f, 0.0f, 0.0f, 0.0f);
		if (_keyboardView == nil) {
			_keyboardView = [[SoftKeyboard alloc] initWithFrame:keyFrame];
			[_keyboardView setInputDelegate:self];
		}

		[self addSubview:[_keyboardView inputView]];
		[self addSubview: _keyboardView];
		[[_keyboardView inputView] becomeFirstResponder];
		_overlayPortraitRatio = (_overlayHeight * ratio) / _overlayWidth;
	}
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

- (void)addEvent:(NSDictionary*)event {

	if (_events == nil)
		_events = [[NSMutableArray alloc] init];

	[_events addObject: event];
}

- (void)deviceOrientationChanged:(int)orientation {
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputOrientationChanged], @"type",
		 [NSNumber numberWithFloat:(float)orientation], @"x",
		 [NSNumber numberWithFloat:0], @"y",
		 nil
		]
	];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	NSSet *allTouches = [event allTouches];

	switch ([allTouches count]) {
		case 1:
		{
			UITouch *touch = [touches anyObject];
			CGPoint point = [touch locationInView:self];
			if (!getLocalMouseCoords(&point))
				return;

			_firstTouch = touch;
			[self addEvent:
			 [[NSDictionary alloc] initWithObjectsAndKeys:
			  [NSNumber numberWithInt:kInputMouseDown], @"type",
			  [NSNumber numberWithFloat:point.x], @"x",
			  [NSNumber numberWithFloat:point.y], @"y",
			  nil
			  ]
			 ];
			break;
		}
		case 2:
		{
			UITouch *touch = [touches anyObject];
			CGPoint point = [touch locationInView:self];
			if (!getLocalMouseCoords(&point))
				return;

			_secondTouch = touch;
			[self addEvent:
			 [[NSDictionary alloc] initWithObjectsAndKeys:
			  [NSNumber numberWithInt:kInputMouseSecondDown], @"type",
			  [NSNumber numberWithFloat:point.x], @"x",
			  [NSNumber numberWithFloat:point.y], @"y",
			  nil
			  ]
			 ];
			break;
		}
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	//NSSet *allTouches = [event allTouches];

	for (UITouch* touch in touches) {
		if (touch == _firstTouch) {

			CGPoint point = [touch locationInView:self];
			if (!getLocalMouseCoords(&point))
				return;

			[self addEvent:
			 [[NSDictionary alloc] initWithObjectsAndKeys:
			  [NSNumber numberWithInt:kInputMouseDragged], @"type",
			  [NSNumber numberWithFloat:point.x], @"x",
			  [NSNumber numberWithFloat:point.y], @"y",
			  nil
			  ]
			 ];

		} else if (touch == _secondTouch) {

			CGPoint point = [touch locationInView:self];
			if (!getLocalMouseCoords(&point))
				return;

			[self addEvent:
			 [[NSDictionary alloc] initWithObjectsAndKeys:
			  [NSNumber numberWithInt:kInputMouseSecondDragged], @"type",
			  [NSNumber numberWithFloat:point.x], @"x",
			  [NSNumber numberWithFloat:point.y], @"y",
			  nil
			  ]
			 ];

		}
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	NSSet *allTouches = [event allTouches];

	switch ([allTouches count]) {
		case 1:
		{
			UITouch *touch = [[allTouches allObjects] objectAtIndex:0];
			CGPoint point = [touch locationInView:self];
			if (!getLocalMouseCoords(&point))
				return;

			[self addEvent:
			 [[NSDictionary alloc] initWithObjectsAndKeys:
			  [NSNumber numberWithInt:kInputMouseUp], @"type",
			  [NSNumber numberWithFloat:point.x], @"x",
			  [NSNumber numberWithFloat:point.y], @"y",
			  nil
			  ]
			 ];
			break;
		}
		case 2:
		{
			UITouch *touch = [[allTouches allObjects] objectAtIndex:1];
			CGPoint point = [touch locationInView:self];
			if (!getLocalMouseCoords(&point))
				return;

			[self addEvent:
			 [[NSDictionary alloc] initWithObjectsAndKeys:
			  [NSNumber numberWithInt:kInputMouseSecondUp], @"type",
			  [NSNumber numberWithFloat:point.x], @"x",
			  [NSNumber numberWithFloat:point.y], @"y",
			  nil
			  ]
			 ];
			break;
		}
	}
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{

}

- (void)handleKeyPress:(unichar)c {
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputKeyPressed], @"type",
		 [NSNumber numberWithFloat:(float)c], @"x",
		 [NSNumber numberWithFloat:0], @"y",
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
		 [NSNumber numberWithFloat:(float)num], @"x",
		 [NSNumber numberWithFloat:0], @"y",
		 nil
		]
	];
}

- (void)applicationSuspend {
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputApplicationSuspended], @"type",
		 [NSNumber numberWithFloat:0], @"x",
		 [NSNumber numberWithFloat:0], @"y",
		 nil
		]
	];
}

- (void)applicationResume {
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputApplicationResumed], @"type",
		 [NSNumber numberWithFloat:0], @"x",
		 [NSNumber numberWithFloat:0], @"y",
		 nil
		]
	];
}

@end

