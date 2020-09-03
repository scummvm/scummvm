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

#include "common/system.h"
#include "backends/updates/macosx/macosx-updates.h"

#ifdef USE_SPARKLE
#include "common/translation.h"
#include "common/config-manager.h"

#include <Cocoa/Cocoa.h>
#include <Sparkle/Sparkle.h>

#include <AvailabilityMacros.h>

SUUpdater *sparkleUpdater;

/**
 * Sparkle is a software update framework for Mac OS X which uses appcasts for
 * release information. Appcasts are RSS-like XML feeds which contain information
 * about the most current version at the time. If a new version is available, the
 * user is presented the release-notes/changes/fixes and is asked if he wants to
 * update, and if yes the Sparkle framework downloads a signed update package
 * from the server and automatically installs and restarts the software.
 * More detailed information is available at the following address:
 * http://sparkle.andymatuschak.org/
 *
 */
MacOSXUpdateManager::MacOSXUpdateManager() {
	NSBundle* mainBundle = [NSBundle mainBundle];

	NSString *version = [mainBundle objectForInfoDictionaryKey:(__bridge NSString *)kCFBundleVersionKey];
	if (!version || [version isEqualToString:@""]) {
		warning("Running not in bundle, skipping Sparkle initialization");

		sparkleUpdater = nullptr;
		return;
	}

	NSMenuItem *menuItem = [[NSApp mainMenu] itemAtIndex:0];
	NSMenu *applicationMenu = [menuItem submenu];

	// Init Sparkle
	sparkleUpdater = [SUUpdater sharedUpdater];

	NSString* feedbackURL = [mainBundle objectForInfoDictionaryKey:@"SUFeedURL"];

	// Set appcast URL
	[sparkleUpdater setFeedURL:[NSURL URLWithString:feedbackURL]];

	// Add "Check for Updates..." menu item
	CFStringRef title = CFStringCreateWithCString(NULL, _("Check for Updates...").encode().c_str(), kCFStringEncodingUTF8);
	NSMenuItem *updateMenuItem = [applicationMenu insertItemWithTitle:(NSString *)title action:@selector(checkForUpdates:) keyEquivalent:@"" atIndex:1];
	CFRelease(title);

	// Set the target of the new menu item
	[updateMenuItem setTarget:sparkleUpdater];

	if (!ConfMan.hasKey("updates_check")
			|| ConfMan.getInt("updates_check") == Common::UpdateManager::kUpdateIntervalNotSupported) {
		setAutomaticallyChecksForUpdates(kUpdateStateDisabled);
	} else {
		setAutomaticallyChecksForUpdates(kUpdateStateEnabled);
		setUpdateCheckInterval(normalizeInterval(ConfMan.getInt("updates_check")));
	}
}

MacOSXUpdateManager::~MacOSXUpdateManager() {
	[sparkleUpdater release];
}

void MacOSXUpdateManager::checkForUpdates() {
	if (sparkleUpdater == nullptr)
		return;

	[sparkleUpdater checkForUpdates:nil];
}

void MacOSXUpdateManager::setAutomaticallyChecksForUpdates(UpdateManager::UpdateState state) {
	if (state == kUpdateStateNotSupported)
		return;

	if (sparkleUpdater == nullptr)
		return;

	[sparkleUpdater setAutomaticallyChecksForUpdates:(state == kUpdateStateEnabled ? YES : NO)];
}

Common::UpdateManager::UpdateState MacOSXUpdateManager::getAutomaticallyChecksForUpdates() {
	if (sparkleUpdater == nullptr)
		return kUpdateStateDisabled;

	if ([sparkleUpdater automaticallyChecksForUpdates])
		return kUpdateStateEnabled;
	else
		return kUpdateStateDisabled;
}

void MacOSXUpdateManager::setUpdateCheckInterval(int interval) {
	if (sparkleUpdater == nullptr)
		return;

	if (interval == kUpdateIntervalNotSupported)
		return;

	interval = normalizeInterval(interval);

	[sparkleUpdater setUpdateCheckInterval:(NSTimeInterval)interval];
}

int MacOSXUpdateManager::getUpdateCheckInterval() {
	if (sparkleUpdater == nullptr)
		return kUpdateIntervalOneDay;

	// This is kind of a hack but necessary, as the value stored by Sparkle
	// might have been changed outside of ScummVM (in which case we return the
	// default interval of one day)

	UpdateInterval updateInterval = (UpdateInterval)[sparkleUpdater updateCheckInterval];
	switch (updateInterval) {
	case kUpdateIntervalOneDay:
	case kUpdateIntervalOneWeek:
	case kUpdateIntervalOneMonth:
		return updateInterval;

	default:
		// Return the default value (one day)
		return kUpdateIntervalOneDay;
	}
}

bool MacOSXUpdateManager::getLastUpdateCheckTimeAndDate(TimeDate &t) {
	if (sparkleUpdater == nullptr)
		return false;

	NSDate *date = [sparkleUpdater lastUpdateCheckDate];
#ifdef MAC_OS_X_VERSION_10_10
	NSCalendar *gregorian = [[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
	NSDateComponents *components = [gregorian components:(NSCalendarUnitDay | NSCalendarUnitWeekday) fromDate:date];
#else
	NSCalendar *gregorian = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
	NSDateComponents *components = [gregorian components:(NSDayCalendarUnit | NSWeekdayCalendarUnit) fromDate:date];
#endif

	t.tm_wday = [components weekday];
	t.tm_year = [components year];
	t.tm_mon = [components month];
	t.tm_mday = [components day];
	t.tm_hour = [components hour];
	t.tm_min = [components minute];
	t.tm_sec = [components second];

	[gregorian release];

	return true;
}

#endif
