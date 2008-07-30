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
 * $URL$
 * $Id$
 *
 */

#import <UIKit/UIKit.h>
#import <Foundation/NSThread.h>
#include "iphone_video.h"

void iphone_main(int argc, char *argv[]);

@interface iPhoneMain : UIApplication {
	UIWindow* _window;
	iPhoneView* _view;
}

- (void) mainLoop: (id)param;
- (iPhoneView*) getView;
- (UIWindow*) getWindow;
@end

static int gArgc;
static char** gArgv;

int main(int argc, char** argv) {
	gArgc = argc;
	gArgv = argv;

    NSAutoreleasePool *autoreleasePool = [ 
        [ NSAutoreleasePool alloc ] init
    ];

    UIApplicationUseLegacyEvents(1);
    int returnCode = UIApplicationMain(argc, argv, [iPhoneMain class]);
    [ autoreleasePool release ];
    return returnCode;
}

@implementation iPhoneMain

-(id) init {
	[super init];
	_window = nil;
	_view = nil;
	return self;
}

- (void) mainLoop: (id)param {
	[[NSAutoreleasePool alloc] init];

	iphone_main(gArgc, gArgv);
	[UIApp terminate];
}

- (iPhoneView*) getView {
	return _view;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// hide the status bar
	[UIHardware _setStatusBarHeight:0.0f];
	//[self setStatusBarMode:2 orientation:0 duration:0.0f fenceID:0];

	//[self setStatusBarStyle:UIStatusBarStyleBlackTranslucent animated:NO];
	[self setStatusBarHidden:YES animated:YES];

	_window = [[UIWindow alloc] initWithContentRect:  [UIHardware fullScreenApplicationContentRect]];
	[_window retain];

	_view = [[iPhoneView alloc] initWithFrame:  [UIHardware fullScreenApplicationContentRect]];
	[_window setContentView: _view];

	[_window orderFront: self];
	[_window makeKey: self];

	[NSThread detachNewThreadSelector:@selector(mainLoop:) toTarget:self withObject:nil];
}

- (void)applicationSuspend:(GSEventRef)event {
	[self setApplicationBadge:NSLocalizedString(@"ON", nil)];
	[_view applicationSuspend];
}

- (void)applicationResume:(GSEventRef)event {
	[self removeApplicationBadge];
	[UIHardware _setStatusBarHeight:0.0f];
	[self setStatusBarHidden:YES animated:YES];
	[_view applicationResume];
}

- (void)deviceOrientationChanged:(GSEvent *)event {
	int screenOrientation = GSEventDeviceOrientation(event);
	[_view deviceOrientationChanged: screenOrientation];
}

- (UIWindow*) getWindow {
	return _window;
}

@end

