//
// Created by Vincent Bénony on 07/12/2015.
//

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#import "iOS7AppDelegate.h"
#import "iOS7ScummVMViewController.h"
#import "ios7_video.h"

@implementation iOS7AppDelegate {
	UIWindow *_window;
	iOS7ScummVMViewController *_controller;
	iPhoneView *_view;
}

- (id)init {
	if (self = [super init]) {
		_window = nil;
		_view = nil;
	}
	return self;
}

- (void)mainLoop:(id)param {
	@autoreleasepool {
		iOS7_main(iOS7_argc, iOS7_argv);
	}

	exit(0);
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

- (void)applicationDidEnterBackground:(UIApplication *)application {
	[_view applicationSuspend];
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
	[_view applicationResume];
}

- (void)didRotate:(NSNotification *)notification {
	UIDeviceOrientation screenOrientation = [[UIDevice currentDevice] orientation];
	[_view deviceOrientationChanged:screenOrientation];
}

@end

const char *iOS7_getDocumentsDir() {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	return [documentsDirectory UTF8String];
}
