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

#include "backends/platform/sdl/macosx/appmenu_osx.h"
#include "common/translation.h"

#include "backends/platform/sdl/macosx/macosx-compat.h"
#include <Cocoa/Cocoa.h>

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_12
#define NSEventModifierFlagCommand NSCommandKeyMask
#define NSEventModifierFlagOption  NSAlternateKeyMask
#endif

// Apple added setAppleMenu in 10.5 and removed it in 10.6.
// But as the method still exists we declare it ourselves here.
// Yes, this works :)
@interface NSApplication(MissingFunction)
- (void)setAppleMenu:(NSMenu *)menu;
@end
// However maybe we should conditionally use it depending on the system on which we run ScummVM (and not
// the one on which we compile) to only do it on OS X 10.5.
// Here is the relevant bit from the release notes for 10.6:
// In Leopard and earlier, apps that tried to construct a menu bar without a nib would get an undesirable
// stubby application menu that could not be removed. To work around this problem on Leopard, you can call
// the undocumented setAppleMenu: method and pass it the application menu, like so:
//   [NSApp setAppleMenu:[[[NSApp mainMenu] itemAtIndex:0] submenu]];
// In SnowLeopard, this workaround is unnecessary and should not be used. Under SnowLeopard, the first menu
// is always identified as the application menu.


NSString *constructNSStringFromCString(const char *rawCString, CFStringEncoding stringEncoding) {
	return (NSString *)CFStringCreateWithCString(NULL, rawCString, stringEncoding);
}

void replaceApplicationMenuItems() {

	// Code mainly copied and adapted from SDLmain.m
	NSMenu *appleMenu;
	NSMenu *windowMenu;
	NSMenuItem *menuItem;

	// We cannot use [[NSApp mainMenu] removeAllItems] as removeAllItems was added in OS X 10.6
	// So remove the SDL generated menus one by one instead.
	while ([[NSApp mainMenu] numberOfItems] > 0) {
		[[NSApp mainMenu] removeItemAtIndex:0];
	}

	// Create new application menu
	appleMenu = [[NSMenu alloc] initWithTitle:@"ResidualVM"];

	NSString *nsString = NULL;

	// Get current encoding
#ifdef USE_TRANSLATION
	nsString = constructNSStringFromCString(TransMan.getCurrentCharset().c_str(), NSASCIIStringEncoding);
	CFStringEncoding stringEncoding = CFStringConvertIANACharSetNameToEncoding((CFStringRef)nsString);
	[nsString release];
#else
	CFStringEncoding stringEncoding = kCFStringEncodingASCII;
#endif

	// Add "About ScummVM" menu item
	nsString = constructNSStringFromCString(_("About ResidualVM"), stringEncoding);
	[appleMenu addItemWithTitle:nsString action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
	[nsString release];

	// Add separator
	[appleMenu addItem:[NSMenuItem separatorItem]];

	// Add "Hide ScummVM" menu item
	nsString = constructNSStringFromCString(_("Hide ResidualVM"), stringEncoding);
	[appleMenu addItemWithTitle:nsString action:@selector(hide:) keyEquivalent:@"h"];
	[nsString release];

	// Add "Hide Others" menu item
	nsString = constructNSStringFromCString(_("Hide Others"), stringEncoding);
	menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:nsString action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
	[menuItem setKeyEquivalentModifierMask:(NSEventModifierFlagOption|NSEventModifierFlagCommand)];

	// Add "Show All" menu item
	nsString = constructNSStringFromCString(_("Show All"), stringEncoding);
	[appleMenu addItemWithTitle:nsString action:@selector(unhideAllApplications:) keyEquivalent:@""];
	[nsString release];

	// Add separator
	[appleMenu addItem:[NSMenuItem separatorItem]];

	// Add "Quit ScummVM" menu item
	nsString = constructNSStringFromCString(_("Quit ResidualVM"), stringEncoding);
	[appleMenu addItemWithTitle:nsString action:@selector(terminate:) keyEquivalent:@"q"];
	[nsString release];

	// Put application menu into the menubar
	menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
	[menuItem setSubmenu:appleMenu];
	[[NSApp mainMenu] addItem:menuItem];

	// Tell the application object that this is now the application menu
	[NSApp setAppleMenu:appleMenu];


	// Create new "Window" menu
	nsString = constructNSStringFromCString(_("Window"), stringEncoding);
	windowMenu = [[NSMenu alloc] initWithTitle:nsString];
	[nsString release];

	// Add "Minimize" menu item
	nsString = constructNSStringFromCString(_("Minimize"), stringEncoding);
	menuItem = [[NSMenuItem alloc] initWithTitle:nsString action:@selector(performMiniaturize:) keyEquivalent:@"m"];
	[windowMenu addItem:menuItem];
	[nsString release];

	// Put menu into the menubar
	nsString = constructNSStringFromCString(_("Window"), stringEncoding);
	menuItem = [[NSMenuItem alloc] initWithTitle:nsString action:nil keyEquivalent:@""];
	[menuItem setSubmenu:windowMenu];
	[[NSApp mainMenu] addItem:menuItem];
	[nsString release];

	// Tell the application object that this is now the window menu.
	[NSApp setWindowsMenu:windowMenu];

	// Finally give up our references to the objects
	[appleMenu release];
	[windowMenu release];
	[menuItem release];
}
