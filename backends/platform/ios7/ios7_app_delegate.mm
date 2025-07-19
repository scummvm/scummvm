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
#include "backends/platform/ios7/ios7_app_delegate.h"
#include "backends/platform/ios7/ios7_scummvm_view_controller.h"
#include "backends/platform/ios7/ios7_video.h"

@implementation iOS7AppDelegate {
	UIWindow *_window;
	iOS7ScummVMViewController *_controller;
	iPhoneView *_view;
	BOOL _restoreState;
}

- (id)init {
	if (self = [super init]) {
		_window = nil;
		_view = nil;
		_restoreState = NO;
	}
	return self;
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
	CGRect rect = [[UIScreen mainScreen] bounds];

	// Create the directory for savegames
	NSFileManager *fm = [NSFileManager defaultManager];
	NSString *documentPath = [NSString stringWithUTF8String:iOS7_getDocumentsDir().c_str()];
	NSString *savePath = [documentPath stringByAppendingPathComponent:@"Savegames"];
	if (![fm fileExistsAtPath:savePath]) {
		[fm createDirectoryAtPath:savePath withIntermediateDirectories:YES attributes:nil error:nil];
	}

	_controller = [[iOS7ScummVMViewController alloc] init];

	_view = [[iPhoneView alloc] initWithFrame:rect];
#if TARGET_OS_IOS
	// This property does not affect the gesture recognizers attached to the view.
	// Gesture recognizers receive all touches that occur in the view.
	_view.multipleTouchEnabled = NO;
#endif
	_controller.view = _view;

	if (@available(iOS 13.0, *)) {
		// iOS13 and later uses of UIScene.
		// The keyWindow is setup by iOS7SceneDelegate
	} else {
		[iOS7AppDelegate setKeyWindow:[[UIWindow alloc] initWithFrame:rect]];
	}

	// Force creation of the shared instance on the main thread
	iOS7_buildSharedOSystemInstance();

	dispatch_async(dispatch_get_global_queue(0, 0), ^{
		iOS7_main(iOS7_argc, iOS7_argv);
	});

	if (_restoreState)
		[_view restoreApplicationState];
	else
		[_view clearApplicationState];
}

- (void)applicationWillResignActive:(UIApplication *)application {
	[_view applicationSuspend];
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	[_view applicationResume];

#if TARGET_OS_IOS
	// Make sure we have the correct orientation in case the orientation was changed while
	// the app was inactive.
	[_controller updateCurrentOrientation];
#endif
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
	// Start the background task before sending the application entered background event.
	// This is because this event will be handled in a separate thread and it will likely
	// no be started before we return from this function.
	[[iOS7AppDelegate iPhoneView] beginBackgroundSaveStateTask];

	[_view saveApplicationState];
}

- (BOOL)application:(UIApplication *)application shouldSaveApplicationState:(NSCoder *)coder {
	return YES;
}

- (BOOL)application:(UIApplication *)application shouldRestoreApplicationState:(NSCoder *)coder {
	return YES;
}

#ifdef __IPHONE_13_2
- (BOOL)application:(UIApplication *)application shouldSaveSecureApplicationState:(NSCoder *)coder {
	return YES;
}

- (BOOL)application:(UIApplication *)application shouldRestoreSecureApplicationState:(NSCoder *)coder {
	return YES;
}
#endif

#ifdef __IPHONE_13_0
- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options API_AVAILABLE(ios(13.0)) {
	// Called when a new scene session is being created.
	UISceneConfiguration *config = [[UISceneConfiguration alloc] initWithName:@"ScummVM Scene Configuration" sessionRole:connectingSceneSession.role];
	[config setDelegateClass:NSClassFromString(@"iOS7SceneDelegate")];
	return config;
}

- (void)application:(UIApplication *)application didDiscardSceneSessions:(NSSet<UISceneSession *> *)sceneSessions API_AVAILABLE(ios(13.0)) {
	// Called when the user discards a scene session.
	// Use this method to release any resources that were
	// specific to the discarded scenes, as they will not return.
}
#endif

- (void)application:(UIApplication *)application didDecodeRestorableStateWithCoder:(NSCoder *)coder {
	_restoreState = YES;
}

+ (iOS7AppDelegate *)iOS7AppDelegate {
	UIApplication *app = [UIApplication sharedApplication];
	// [UIApplication delegate] must be used from the main thread only
	if ([NSThread currentThread] == [NSThread mainThread]) {
		return (iOS7AppDelegate *) app.delegate;
	} else {
		__block iOS7AppDelegate *delegate = nil;
		dispatch_sync(dispatch_get_main_queue(), ^{
			delegate = (iOS7AppDelegate *) app.delegate;
		});
		return delegate;
	}
}

+ (iPhoneView *)iPhoneView {
	iOS7AppDelegate *appDelegate = [self iOS7AppDelegate];
	return appDelegate->_view;
}

#if TARGET_OS_IOS
+ (UIInterfaceOrientation)currentOrientation {
	iOS7AppDelegate *appDelegate = [self iOS7AppDelegate];
	return [appDelegate->_controller currentOrientation];
}

+ (void)setKeyWindow:(UIWindow *)window {
	iOS7AppDelegate *appDelegate = [self iOS7AppDelegate];
	appDelegate->_window = window;
	[appDelegate->_window retain];
	[appDelegate->_window setRootViewController:appDelegate->_controller];
	[appDelegate->_window makeKeyAndVisible];
}
#endif

@end
