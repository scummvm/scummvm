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
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/sdl/macosx/appmenu_osx.h"
#include "common/translation.h"

#include <Cocoa/Cocoa.h>

// Apple removed setAppleMenu from the header files in 10.4,
// but as the method still exists we declare it ourselves here.
// Yes, this works :)
@interface NSApplication(MissingFunction)
- (void)setAppleMenu:(NSMenu *)menu;
@end

void replaceApplicationMenuItems() {

	// Code mainly copied and adapted from SDLmain.m
	NSMenu *appleMenu;
	NSMenu *windowMenu;
	NSMenuItem *menuItem;

	// For some reason [[NSApp mainMenu] removeAllItems] doesn't work and crashes, so we need
	// to remove the SDL generated menus one by one
	[[NSApp mainMenu] removeItemAtIndex:0];		// Remove application menu
	[[NSApp mainMenu] removeItemAtIndex:0];		// Remove "Windows" menu

	// Create new application menu
	appleMenu = [[NSMenu alloc] initWithTitle:@""];

	// Get current encoding
#ifdef USE_TRANSLATION
	NSStringEncoding stringEncoding = CFStringConvertEncodingToNSStringEncoding(CFStringConvertIANACharSetNameToEncoding((CFStringRef)[NSString stringWithCString:(TransMan.getCurrentCharset()).c_str() encoding:NSASCIIStringEncoding]));
#else
	NSStringEncoding stringEncoding = NSASCIIStringEncoding;
#endif
	
	// Add "About ScummVM" menu item
	[appleMenu addItemWithTitle:[NSString stringWithCString:_("About ScummVM") encoding:stringEncoding] action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];

	// Add separator
	[appleMenu addItem:[NSMenuItem separatorItem]];

	// Add "Hide ScummVM" menu item
	[appleMenu addItemWithTitle:[NSString stringWithCString:_("Hide ScummVM") encoding:stringEncoding] action:@selector(hide:) keyEquivalent:@"h"];

	// Add "Hide Others" menu item
	menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:[NSString stringWithCString:_("Hide Others") encoding:stringEncoding] action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
	[menuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];

	// Add "Show All" menu item
	[appleMenu addItemWithTitle:[NSString stringWithCString:_("Show All") encoding:stringEncoding] action:@selector(unhideAllApplications:) keyEquivalent:@""];

	// Add separator
	[appleMenu addItem:[NSMenuItem separatorItem]];

	// Add "Quit ScummVM" menu item
	[appleMenu addItemWithTitle:[NSString stringWithCString:_("Quit ScummVM") encoding:stringEncoding] action:@selector(terminate:) keyEquivalent:@"q"];

	// Put application menu into the menubar
	menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
	[menuItem setSubmenu:appleMenu];
	[[NSApp mainMenu] addItem:menuItem];

	// Tell the application object that this is now the application menu
	[NSApp setAppleMenu:appleMenu];


	// Create new "Window" menu
	windowMenu = [[NSMenu alloc] initWithTitle:[NSString stringWithCString:_("Window") encoding:stringEncoding]];

	// Add "Minimize" menu item
	menuItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithCString:_("Minimize") encoding:stringEncoding] action:@selector(performMiniaturize:) keyEquivalent:@"m"];
	[windowMenu addItem:menuItem];

	// Put menu into the menubar
	menuItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithCString:_("Window") encoding:stringEncoding] action:nil keyEquivalent:@""];
	[menuItem setSubmenu:windowMenu];
	[[NSApp mainMenu] addItem:menuItem];

	// Tell the application object that this is now the window menu.
	[NSApp setWindowsMenu:windowMenu];

	// Finally give up our references to the objects
	[appleMenu release];
	[windowMenu release];
	[menuItem release];
}
