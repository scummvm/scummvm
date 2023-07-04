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

#include "backends/platform/ios7/ios7_osys_main.h"
#include "backends/platform/ios7/ios7_video.h"

#include "backends/platform/ios7/ios7_app_delegate.h"

#define UIViewParentController(__view) ({ \
	UIResponder *__responder = __view; \
	while ([__responder isKindOfClass:[UIView class]]) \
		__responder = [__responder nextResponder]; \
	(UIViewController *)__responder; \
})

static void displayAlert(void *ctx) {
	UIAlertController* alert = [UIAlertController alertControllerWithTitle:@"Fatal Error"
								message:[NSString stringWithCString:(const char *)ctx 	encoding:NSUTF8StringEncoding]
								preferredStyle:UIAlertControllerStyleAlert];

	UIAlertAction* defaultAction = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault
	   handler:^(UIAlertAction * action) {
		OSystem_iOS7::sharedInstance()->quit();
		abort();
	}];

	[alert addAction:defaultAction];
	[UIViewParentController([iOS7AppDelegate iPhoneView]) presentViewController:alert animated:YES completion:nil];
}

void OSystem_iOS7::fatalError() {
	if (_lastErrorMessage.size()) {
		dispatch_async_f(dispatch_get_main_queue(), (void *)_lastErrorMessage.c_str(), displayAlert);
		for(;;);
	}
	else {
		OSystem::fatalError();
	}
}

void OSystem_iOS7::logMessage(LogMessageType::Type type, const char *message) {
	FILE *output = 0;

	if (type == LogMessageType::kInfo || type == LogMessageType::kDebug)
		output = stdout;
	else
		output = stderr;

	if (type == LogMessageType::kError) {
		_lastErrorMessage = message;
		NSString *messageString = [NSString stringWithUTF8String:message];
		NSLog(@"%@", messageString);
	}

	fputs(message, output);
	fflush(output);
}

static inline void execute_on_main_thread(void (^block)(void)) {
	if ([NSThread currentThread] == [NSThread mainThread]) {
		block();
	}
	else {
		dispatch_sync(dispatch_get_main_queue(), block);
	}
}

void OSystem_iOS7::engineInit() {
	EventsBaseBackend::engineInit();
	// Prevent the device going to sleep during game play (and in particular cut scenes)
	dispatch_async(dispatch_get_main_queue(), ^{
		[[UIApplication sharedApplication] setIdleTimerDisabled:YES];
	});
	[[iOS7AppDelegate iPhoneView] setIsInGame:YES];

#if TARGET_OS_IOS
	// Automatically open the keyboard when starting a game and in portrait mode.
	// This is preferred for text input games and there's a lot of screen space to
	// utilize for the keyboard anyway.
	if (_screenOrientation == kScreenOrientationPortrait ||
		_screenOrientation == kScreenOrientationFlippedPortrait) {
		execute_on_main_thread(^ {
			[[iOS7AppDelegate iPhoneView] showKeyboard];
		});
	}
#endif
}

void OSystem_iOS7::engineDone() {
	EventsBaseBackend::engineDone();
	// Allow the device going to sleep if idle while in the Launcher
	dispatch_async(dispatch_get_main_queue(), ^{
		[[UIApplication sharedApplication] setIdleTimerDisabled:NO];
	});
	[[iOS7AppDelegate iPhoneView] setIsInGame:NO];

#if TARGET_OS_IOS
	// Hide keyboard when going back to launcher
	execute_on_main_thread(^ {
		[[iOS7AppDelegate iPhoneView] hideKeyboard];
	});
#endif
}

void OSystem_iOS7::updateOutputSurface() {
	execute_on_main_thread(^ {
		[[iOS7AppDelegate iPhoneView] initSurface];
	});
}

void OSystem_iOS7::virtualController(bool connect) {
	execute_on_main_thread(^ {
		[[iOS7AppDelegate iPhoneView] virtualController:connect];
	});
}

void OSystem_iOS7::setShowKeyboard(bool show) {
	if (show) {
#if TARGET_OS_IOS
		execute_on_main_thread(^ {
			[[iOS7AppDelegate iPhoneView] showKeyboard];
		});
#elif TARGET_OS_TV
		// Delay the showing of keyboard 1 second so the user
		// is able to see the message
		dispatch_time_t delay = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC));
		dispatch_after(delay, dispatch_get_main_queue(), ^(void){
			[[iOS7AppDelegate iPhoneView] showKeyboard];
		});
#endif
	} else {
		// If in game, do not hide the keyboard in portrait mode as it is shown automatically and not
		// just when asked with the kFeatureVirtualKeyboard.
		if (_screenOrientation == kScreenOrientationLandscape || _screenOrientation == kScreenOrientationFlippedLandscape || ![[iOS7AppDelegate iPhoneView] isInGame]) {
			execute_on_main_thread(^ {
				[[iOS7AppDelegate iPhoneView] hideKeyboard];
			});
		}
	}
}

bool OSystem_iOS7::isKeyboardShown() const {
	__block bool isShown = false;
	execute_on_main_thread(^{
		isShown = [[iOS7AppDelegate iPhoneView] isKeyboardShown];
	});
	return isShown;
}

uint OSystem_iOS7::createOpenGLContext() {
	return [[iOS7AppDelegate iPhoneView] createOpenGLContext];
}

void OSystem_iOS7::destroyOpenGLContext() {
	[[iOS7AppDelegate iPhoneView] destroyOpenGLContext];
}

void OSystem_iOS7::refreshScreen() const {
	[[iOS7AppDelegate iPhoneView] refreshScreen];
}

int OSystem_iOS7::getScreenWidth() const {
	return [[iOS7AppDelegate iPhoneView] getScreenWidth];
}

int OSystem_iOS7::getScreenHeight() const {
	return [[iOS7AppDelegate iPhoneView] getScreenHeight];
}

float OSystem_iOS7::getSystemHiDPIScreenFactor() const {
	return [[UIScreen mainScreen] scale];
}

