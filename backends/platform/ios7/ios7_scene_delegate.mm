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

#include "backends/platform/ios7/ios7_scene_delegate.h"
#include "backends/platform/ios7/ios7_app_delegate.h"
#include "backends/platform/ios7/ios7_video.h"

API_AVAILABLE(ios(13.0))
@implementation iOS7SceneDelegate

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions {

	UIWindowScene *windowScene = (UIWindowScene *)scene;
	[iOS7AppDelegate setKeyWindow:[[UIWindow alloc] initWithWindowScene:windowScene]];
}

- (void)sceneDidDisconnect:(UIScene *)scene {
}

- (void)sceneDidBecomeActive:(UIScene *)scene {
	[[iOS7AppDelegate iPhoneView] applicationResume];
}

- (void)sceneWillResignActive:(UIScene *)scene {
	[[iOS7AppDelegate iPhoneView] applicationSuspend];
}

- (void)sceneDidEnterBackground:(UIScene *)scene {
	// Start the background task before sending the application entered background event.
	// This is because this event will be handled in a separate thread and it will likely
	// no be started before we return from this function.
	[[iOS7AppDelegate iPhoneView] beginBackgroundSaveStateTask];

	[[iOS7AppDelegate iPhoneView] saveApplicationState];
}

- (void)sceneWillEnterForeground:(UIScene *)scene {
}

@end
