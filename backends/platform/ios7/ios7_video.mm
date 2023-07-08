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
	return false;
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

bool iOS7_fetchEvent(InternalEvent *event) {
	__block bool fetched;
	execute_on_main_thread(^{
		fetched = [[iOS7AppDelegate iPhoneView] fetchEvent:event];
	});
	return fetched;
}

@implementation iPhoneView

+ (Class)layerClass {
	return [CAEAGLLayer class];
}

// According to Apple doc layoutSublayersOfLayer: is supported from iOS 10.0.
// This doesn't seem to be correct since the instance method layoutSublayers,
// supported from iOS 2.0, default calls the layoutSublayersOfLayer: method
// of the layer’s delegate object. It's been verified that this function is
// called in at least iOS 9.3.5.
- (void)layoutSublayersOfLayer:(CAEAGLLayer *)layer {
	if (layer == self.layer) {
		[self addEvent:InternalEvent(kInputScreenChanged, 0, 0)];
	}
	[super layoutSublayersOfLayer:layer];
}

- (void)createContext {
	CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

	eaglLayer.opaque = YES;
	eaglLayer.drawableProperties = @{
	                                 kEAGLDrawablePropertyRetainedBacking: @NO,
	                                 kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8,
	                                };

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
	return _viewRenderbuffer;
}

- (void)destroyOpenGLContext {
	[_openGLContext release];
	_openGLContext = nil;
}

- (void)refreshScreen {
	glBindRenderbuffer(GL_RENDERBUFFER, _viewRenderbuffer);
	[_openGLContext presentRenderbuffer:GL_RENDERBUFFER];
}

- (int)getScreenWidth {
	return _renderBufferWidth;
}

- (int)getScreenHeight {
	return _renderBufferHeight;
}

- (void)setupRenderBuffer {
	execute_on_main_thread(^{
		if (!_viewRenderbuffer) {
			glGenRenderbuffers(1, &_viewRenderbuffer);
			printOpenGLError();
		}
		glBindRenderbuffer(GL_RENDERBUFFER, _viewRenderbuffer);
		printOpenGLError();
		if (![_mainContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(id <EAGLDrawable>) self.layer]) {
			printError("Failed renderbufferStorage");
		}
		// Retrieve the render buffer size. This *should* match the frame size,
		// i.e. g_fullWidth and g_fullHeight.
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_renderBufferWidth);
		printOpenGLError();
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_renderBufferHeight);
		printOpenGLError();
	});
}

- (void)deleteRenderbuffer {
	glDeleteRenderbuffers(1, &_viewRenderbuffer);
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
#if TARGET_OS_IOS
	_currentOrientation = UIInterfaceOrientationUnknown;
#endif

	[self setupGestureRecognizers];

	if (@available(iOS 14.0, tvOS 14.0, *)) {
		_controllers.push_back([[MouseController alloc] initWithView:self]);
		_controllers.push_back([[GamepadController alloc] initWithView:self]);
	}
	_controllers.push_back([[TouchController alloc] initWithView:self]);

	[self setContentScaleFactor:[[UIScreen mainScreen] scale]];

	_keyboardView = nil;
	_eventLock = [[NSLock alloc] init];

	// Initialize the OpenGL ES context
	[self createContext];

	return self;
}

- (void)dealloc {
	[_keyboardView release];

	[_eventLock release];
	[super dealloc];
}

- (void)initSurface {
	[self setupRenderBuffer];

	if (_keyboardView == nil) {
		_keyboardView = [[SoftKeyboard alloc] initWithFrame:CGRectZero];
		[_keyboardView setInputDelegate:self];
		[self addSubview:[_keyboardView inputView]];
		[self addSubview: _keyboardView];
	}

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
		CGRect newFrame = self.frame;
#if TARGET_OS_IOS
		UIEdgeInsets inset = [[[UIApplication sharedApplication] keyWindow] safeAreaInsets];
		UIInterfaceOrientation orientation = [iOS7AppDelegate currentOrientation];

		// The code below adjust the screen size according to what Apple calls
		// the "safe area". It also cover the cases when the software keyboard
		// is visible and has changed the frame height so the keyboard doesn't
		// cover any part of the game screen.
		if (orientation != _currentOrientation) {
			// If the orientation is changed the keyboard will hide or show
			// depending on the current orientation. The frame size must be
			// "reset" again to "full" screen size dimension. The keyboard
			// will then calculate the approriate height when becoming visible.
			newFrame = screenSize;
			_currentOrientation = orientation;
		}
		// Make sure the frame height (either full screen or resized due to
		// visible keyboard) is within the safe area.
		CGFloat safeAreaHeight = screenSize.size.height - inset.top;
		CGFloat height = newFrame.size.height < safeAreaHeight ? newFrame.size.height : safeAreaHeight;

		if ( orientation == UIInterfaceOrientationPortrait ) {
			newFrame = CGRectMake(screenSize.origin.x, screenSize.origin.y + inset.top, screenSize.size.width, height);
		} else if ( orientation == UIInterfaceOrientationPortraitUpsideDown ) {
			newFrame = CGRectMake(screenSize.origin.x, screenSize.origin.y, screenSize.size.width, height);
		} else if ( orientation == UIInterfaceOrientationLandscapeLeft ) {
			newFrame = CGRectMake(screenSize.origin.x, screenSize.origin.y, screenSize.size.width - inset.right, height);
		} else if ( orientation == UIInterfaceOrientationLandscapeRight ) {
			newFrame = CGRectMake(screenSize.origin.x + inset.left, screenSize.origin.y, screenSize.size.width - inset.left, height);
		}
#endif
		self.frame = newFrame;
	}
#endif
}

#ifdef __IPHONE_11_0
// This delegate method is called when the safe area of the view changes
-(void)safeAreaInsetsDidChange {
	[super safeAreaInsetsDidChange];

	[self adjustViewFrameForSafeArea];
}
#endif

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
	ScreenOrientation screenOrientation;
	if (orientation == UIInterfaceOrientationPortrait) {
		screenOrientation = kScreenOrientationPortrait;
	} else if (orientation == UIInterfaceOrientationPortraitUpsideDown) {
		screenOrientation = kScreenOrientationFlippedPortrait;
	} else if (orientation == UIInterfaceOrientationLandscapeRight) {
		screenOrientation = kScreenOrientationLandscape;
	} else { // UIInterfaceOrientationLandscapeLeft
		screenOrientation = kScreenOrientationFlippedLandscape;
	}

	[self addEvent:InternalEvent(kInputOrientationChanged, screenOrientation, 0)];
	if (UIInterfaceOrientationIsLandscape(orientation)) {
		[self hideKeyboard];
	} else {
		// Automatically open the keyboard if changing orientation in a game
		if ([self isInGame])
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
	// Make sure to hide the keyboard when suspended. Else the frame
	// sizing might become incorrect because the NotificationCenter
	// sends keyboard notifications on resume.
	[self hideKeyboard];
	[self addEvent:InternalEvent(kInputApplicationSuspended, 0, 0)];
}

- (void)applicationResume {
	[self addEvent:InternalEvent(kInputApplicationResumed, 0, 0)];
	// The device may have changed orientation. Make sure to update
	// the screen size to the graphic manager.
	[self addEvent:InternalEvent(kInputScreenChanged, 0, 0)];
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
