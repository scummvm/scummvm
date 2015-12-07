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


#import "iOS7MainApplication.h"
#import "iOS7ScummVMViewController.h"

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#import "ios7_common.h"
#import "ios7_video.h"


@implementation iOS7MainApplication

-(id)init {
	[super init];
	_window = nil;
	_view = nil;
	return self;
}

- (void)mainLoop:(id)param {
	@autoreleasepool {
		iOS7_main(iOS7_argc, iOS7_argv);
	}

	exit(0);
}

- (iPhoneView *)getView {
	return _view;
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
	CGRect  rect = [[UIScreen mainScreen] bounds];

	// Create the directory for savegames
#ifdef IPHONE_OFFICIAL
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


const char *iOS7_getDocumentsDir() {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	return [documentsDirectory UTF8String];
}
