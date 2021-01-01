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
#include "backends/updates/win32/win32-updates.h"

#ifdef USE_SPARKLE
#include "backends/platform/sdl/win32/win32-window.h"
#include "common/config-manager.h"

#include <time.h>
#include <windows.h>
#include <winsparkle.h>

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
 * WinSparkle is a heavily (to the point of being its almost-port) inspired by the
 * Sparkle framework originally by Andy Matuschak that became the de facto standard
 * for software updates on OS X.
 * More detailed information is available at the following address:
 * https://winsparkle.org/
 *
 */

static SdlWindow_Win32 *_window;

Win32UpdateManager::Win32UpdateManager(SdlWindow_Win32 *window) {
	_window = window;
	const char *appcastUrl = "https://www.scummvm.org/appcasts/macosx/release.xml";
	win_sparkle_set_appcast_url(appcastUrl);
	win_sparkle_set_can_shutdown_callback(canShutdownCallback);
	win_sparkle_set_shutdown_request_callback(shutdownRequestCallback);
    win_sparkle_init();

    if (!ConfMan.hasKey("updates_check")
      || ConfMan.getInt("updates_check") == Common::UpdateManager::kUpdateIntervalNotSupported) {
        setAutomaticallyChecksForUpdates(kUpdateStateDisabled);
    } else {
        setAutomaticallyChecksForUpdates(kUpdateStateEnabled);
        setUpdateCheckInterval(normalizeInterval(ConfMan.getInt("updates_check")));
    }
}

Win32UpdateManager::~Win32UpdateManager() {
    win_sparkle_cleanup();
}

void Win32UpdateManager::checkForUpdates() {
    win_sparkle_check_update_with_ui();
}

void Win32UpdateManager::setAutomaticallyChecksForUpdates(UpdateManager::UpdateState state) {
    if (state == kUpdateStateNotSupported)
        return;

    win_sparkle_set_automatic_check_for_updates(state == kUpdateStateEnabled ? 1 : 0);
}

Common::UpdateManager::UpdateState Win32UpdateManager::getAutomaticallyChecksForUpdates() {
    if (win_sparkle_get_automatic_check_for_updates() == 1)
        return kUpdateStateEnabled;
    else
        return kUpdateStateDisabled;
}

void Win32UpdateManager::setUpdateCheckInterval(int interval) {
    if (interval == kUpdateIntervalNotSupported)
        return;

    interval = normalizeInterval(interval);

    win_sparkle_set_update_check_interval(interval);
}

int Win32UpdateManager::getUpdateCheckInterval() {
    // This is kind of a hack but necessary, as the value stored by Sparkle
    // might have been changed outside of ScummVM (in which case we return the
    // default interval of one day)

    int updateInterval = win_sparkle_get_update_check_interval();
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

bool Win32UpdateManager::getLastUpdateCheckTimeAndDate(TimeDate &t) {
    time_t updateTime = win_sparkle_get_last_check_time();
    tm *ut = localtime(&updateTime);

    t.tm_wday = ut->tm_wday;
    t.tm_year = ut->tm_year;
    t.tm_mon  = ut->tm_mon;
    t.tm_mday = ut->tm_mday;
    t.tm_hour = ut->tm_hour;
    t.tm_min  = ut->tm_min;
    t.tm_sec  = ut->tm_sec;

    return true;
}

// WinSparkle calls this to ask if we can shut down.
//  At this point the download has completed, the user has
//  selected Install Update, and the installer has started.
//  This callback runs on a non-main thread.
int Win32UpdateManager::canShutdownCallback() {
	return true;
}

// WinSparkle calls this to request that we shut down.
//  This callback runs on a non-main thread so we post
//  a WM_CLOSE message to our window so that we exit
//  cleanly, as opposed to calling g_system->quit().
void Win32UpdateManager::shutdownRequestCallback() {
	PostMessage(_window->getHwnd(), WM_CLOSE, 0, 0);
}

#endif
