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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

#ifdef IPHONE_SANDBOXED
	// Create the directory for savegames
	NSFileManager *fm = [NSFileManager defaultManager];
	NSString *documentPath = [NSString stringWithUTF8String:iOS7_getDocumentsDir()];
	NSString *savePath = [documentPath stringByAppendingPathComponent:@"Savegames"];
	if (![fm fileExistsAtPath:savePath]) {
		[fm createDirectoryAtPath:savePath withIntermediateDirectories:YES attributes:nil error:nil];
	}
#endif

	_window = [[UIWindow alloc] initWithFrame:rect];
	[_window retain];

	_controller = [[iOS7ScummVMViewController alloc] init];

	_view = [[iPhoneView alloc] initWithFrame:rect];
	_view.multipleTouchEnabled = YES;
	_controller.view = _view;

	[_window setRootViewController:_controller];
	[_window makeKeyAndVisible];

	[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	[[NSNotificationCenter defaultCenter] addObserver:self
	                                         selector:@selector(didRotate:)
	                                             name:@"UIDeviceOrientationDidChangeNotification"
	                                           object:nil];

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

- (void)application:(UIApplication *)application didDecodeRestorableStateWithCoder:(NSCoder *)coder {
	_restoreState = YES;
}

- (void)didRotate:(NSNotification *)notification {
	UIDeviceOrientation screenOrientation = [[UIDevice currentDevice] orientation];
	[_view deviceOrientationChanged:screenOrientation];
}

+ (iOS7AppDelegate *)iOS7AppDelegate {
	UIApplication *app = [UIApplication sharedApplication];
	return (iOS7AppDelegate *) app.delegate;
}

+ (iPhoneView *)iPhoneView {
	iOS7AppDelegate *appDelegate = [self iOS7AppDelegate];
	return appDelegate->_view;
}

@end

const char *iOS7_getDocumentsDir() {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	return [documentsDirectory UTF8String];
}
