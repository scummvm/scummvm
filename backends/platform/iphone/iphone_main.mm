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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <UIKit/UIKit.h>
#include <Foundation/NSThread.h>

#include "iphone_video.h"

void iphone_main(int argc, char *argv[]);

@interface ScummVMViewController : UIViewController

@end

@implementation ScummVMViewController

- (BOOL)prefersStatusBarHidden {
    return YES;
}

@end

@interface iPhoneMain : UIApplication {
	UIWindow *_window;
	ScummVMViewController *_controller;
	iPhoneView *_view;
}

- (void)mainLoop:(id)param;
- (iPhoneView *)getView;
- (UIWindow *)getWindow;
- (void)didRotate:(NSNotification *)notification;
@end

static int g_argc;
static char **g_argv;

int main(int argc, char **argv) {
	g_argc = argc;
	g_argv = argv;

	NSAutoreleasePool *autoreleasePool = [
		[NSAutoreleasePool alloc] init
	];

	int returnCode = UIApplicationMain(argc, argv, @"iPhoneMain", @"iPhoneMain");
	[autoreleasePool release];
	return returnCode;
}

@implementation iPhoneMain

-(id) init {
	[super init];
	_window = nil;
	_view = nil;
	return self;
}

- (void)mainLoop:(id)param {
	[[NSAutoreleasePool alloc] init];

	iphone_main(g_argc, g_argv);
	exit(0);
}

- (iPhoneView *)getView {
	return _view;
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
	CGRect  rect = [[UIScreen mainScreen] bounds];

	// Create the directory for savegames
	NSFileManager *fm = [NSFileManager defaultManager];
	NSString *documentPath = [NSString stringWithUTF8String:iPhone_getDocumentsDir()];
	NSString *savePath = [documentPath stringByAppendingPathComponent:@"savegames"];
	if (![fm fileExistsAtPath:savePath]) {
		[fm createDirectoryAtPath:savePath withIntermediateDirectories:YES attributes:nil error:nil];
	}

	_window = [[UIWindow alloc] initWithFrame:rect];
	[_window retain];

	_controller = [[ScummVMViewController alloc] init];

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

	[NSThread detachNewThreadSelector:@selector(mainLoop:) toTarget:self withObject:nil];
}

- (void)applicationDidResume {
}

- (void)applicationWillSuspend {
}

- (void)applicationWillTerminate {
}

- (void)applicationSuspend:(struct __GSEvent *)event {
	//[self setApplicationBadge:NSLocalizedString(@"ON", nil)];
	[_view applicationSuspend];
}

- (void)applicationResume:(struct __GSEvent *)event {
	[_view applicationResume];
}

- (void)didRotate:(NSNotification *)notification {
	UIDeviceOrientation screenOrientation = [[UIDevice currentDevice] orientation];
	[_view deviceOrientationChanged:screenOrientation];
}

- (UIWindow*) getWindow {
	return _window;
}

@end
