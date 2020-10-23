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

#include <Cocoa/Cocoa.h>

@interface ScummVMDockTilePlugIn : NSObject <NSDockTilePlugIn> {
	NSMenu *recentGamesMenu;
}
@end

@interface StartGameMenuItem : NSMenuItem {
	NSString *game;
}
- (IBAction) startGame;
- (NSMenuItem*)initWithGame:(NSString *)gameId description:(NSString*)desc icon:(NSString*)iconFile;
@end

@implementation ScummVMDockTilePlugIn

- (id)init {
	self = [super init];
	if (self) {
		recentGamesMenu = nil;
	}
	return self;
}

- (void)dealloc {
	[recentGamesMenu release];
	[super dealloc];
}


- (void)setDockTile:(NSDockTile *)dockTile {
}

- (NSMenu*)dockMenu {
	// Get the list or recent games
	CFPreferencesAppSynchronize(CFSTR("org.scummvm.scummvm"));
	NSArray *array = CFPreferencesCopyAppValue(CFSTR("recentGames"), CFSTR("org.scummvm.scummvm"));
	if (array == nil)
		return nil;

	// Create the menu
	if (recentGamesMenu == nil)
		recentGamesMenu = [[NSMenu alloc] init];
	else
		[recentGamesMenu removeAllItems];

	NSEnumerator *enumerator = [array objectEnumerator];
	NSDictionary *recentGame;
	while (recentGame = [enumerator nextObject]) {
		NSString *gameId = [recentGame valueForKey:@"game"];
		NSString *desc = [recentGame valueForKey:@"description"];
		NSString *iconFile = [recentGame valueForKey:@"icon"];

		StartGameMenuItem *menuItem = [[StartGameMenuItem alloc] initWithGame:gameId description:desc icon:iconFile];
		[recentGamesMenu addItem:menuItem];
		[menuItem release];
	}

	CFRelease(array);
	return recentGamesMenu;
}

@end

@implementation StartGameMenuItem

- (NSMenuItem*)initWithGame:(NSString *)gameId description:(NSString*)desc icon:(NSString*)iconFile {
	self = [super initWithTitle:(desc == nil ? gameId : desc) action:@selector(startGame) keyEquivalent:@""];
	[self setTarget:self];

	if (iconFile != nil) {
		NSImage *image = [[NSImage alloc] initWithContentsOfFile:iconFile];
		[self setImage:image];
		[image release];
	}

	game = gameId;
	[game retain];

	return self;
}

- (void)dealloc {
	[game release];
	[super dealloc];
}

- (IBAction) startGame {
	NSLog(@"Starting Game %@...", game);

	NSString *scummVMPath = [[NSWorkspace sharedWorkspace] absolutePathForAppBundleWithIdentifier:@"org.scummvm.scummvm"];
	if (scummVMPath == nil) {
		NSLog(@"Cannot find ScummVM.app!");
		return;
	}
	// Start ScummVM.app with the game ID as argument
	NSURL *url = [NSURL fileURLWithPath:scummVMPath];
	NSMutableDictionary *args = [[NSMutableDictionary alloc] init];
	[args setObject:[NSArray arrayWithObject:game] forKey:NSWorkspaceLaunchConfigurationArguments];
	[[NSWorkspace sharedWorkspace] launchApplicationAtURL:url options:NSWorkspaceLaunchDefault configuration:args error:nil];
	[args release];
}

@end
