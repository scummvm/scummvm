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

#include "backends/platform/ios7/ios7_scummvm_view_controller.h"
#include "backends/platform/ios7/ios7_app_delegate.h"
#include "backends/platform/ios7/ios7_video.h"

@implementation iOS7ScummVMViewController

- (id)init {
	self = [super init];
#if TARGET_OS_IOS
	self->currentOrientation = UIInterfaceOrientationUnknown;
#endif
	return self;
}

- (BOOL)prefersStatusBarHidden {
	return YES;
}

- (BOOL)prefersHomeIndicatorAutoHidden {
	return YES;
}

- (BOOL)prefersPointerLocked {
	/* This hides the OS cursor so ScummVM has to draw one */
	return YES;
}

#if TARGET_OS_IOS
-(void) setCurrentOrientation:(UIInterfaceOrientation)orientation {
	if (orientation != currentOrientation) {
		currentOrientation = orientation;
		[[iOS7AppDelegate iPhoneView] interfaceOrientationChanged:currentOrientation];
	}
}

- (void)viewDidAppear:(BOOL)animated {
	[super viewDidAppear:animated];

	UIInterfaceOrientation orientationBefore = UIInterfaceOrientationUnknown;
	if (@available(iOS 13.0, *)) {
		orientationBefore = [[[[self view] window] windowScene] interfaceOrientation];
	} else {
		orientationBefore = [[UIApplication sharedApplication] statusBarOrientation];
	}
	if (currentOrientation != UIInterfaceOrientationUnknown)
		[[iOS7AppDelegate iPhoneView] interfaceOrientationChanged:currentOrientation];
}

- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator {
	[super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];

	[coordinator animateAlongsideTransition:nil completion:^(id<UIViewControllerTransitionCoordinatorContext> context) {
		UIInterfaceOrientation orientationAfter = UIInterfaceOrientationUnknown;
		if (@available(iOS 13.0, *)) {
			orientationAfter = [[[[self view] window] windowScene] interfaceOrientation];
		} else {
			orientationAfter = [[UIApplication sharedApplication] statusBarOrientation];
		}
		if (orientationAfter != UIInterfaceOrientationUnknown && orientationAfter != currentOrientation) {
			currentOrientation = orientationAfter;
			[[iOS7AppDelegate iPhoneView] interfaceOrientationChanged:currentOrientation];
		}
	}];
}
#endif

@end
