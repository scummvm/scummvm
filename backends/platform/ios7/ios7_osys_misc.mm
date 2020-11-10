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

#include "backends/platform/ios7/ios7_osys_main.h"

#include <UIKit/UIKit.h>
#include <SystemConfiguration/SCNetworkReachability.h>
#include "backends/platform/ios7/ios7_app_delegate.h"
#include "backends/platform/ios7/ios7_video.h"

static inline void execute_on_main_thread_async(void (^block)(void)) {
	if ([NSThread currentThread] == [NSThread mainThread]) {
		block();
	} else {
		dispatch_async(dispatch_get_main_queue(), block);
	}
}

Common::String OSystem_iOS7::getSystemLanguage() const {
	NSString *locale = [[NSLocale currentLocale] localeIdentifier];
	if (locale == nil)
		return Common::String();
	return Common::String([locale cStringUsingEncoding:NSISOLatin1StringEncoding]);
}

bool OSystem_iOS7::hasTextInClipboard() {
	return [[UIPasteboard generalPasteboard] containsPasteboardTypes:UIPasteboardTypeListString];
}

Common::U32String OSystem_iOS7::getTextFromClipboard() {
	if (!hasTextInClipboard())
		return Common::U32String();

	UIPasteboard *pb = [UIPasteboard generalPasteboard];
	NSString *str = pb.string;
	if (str == nil)
		return Common::U32String();

	// If translations are supported, use the current TranslationManager charset and otherwise
	// use ASCII. If the string cannot be represented using the requested encoding we get a null
	// pointer below, which is fine as ScummVM would not know what to do with the string anyway.
#ifdef SCUMM_LITTLE_ENDIAN
	NSStringEncoding stringEncoding = NSUTF32LittleEndianStringEncoding;
#else
	NSStringEncoding stringEncoding = NSUTF32BigEndianStringEncoding;
#endif
	NSUInteger textLength = [str length];
	uint32 *text = new uint32[textLength];

	if (![str getBytes:text maxLength:4*textLength usedLength:NULL encoding: stringEncoding options:0 range:NSMakeRange(0, textLength) remainingRange:NULL]) {
		delete[] text;
		return Common::U32String();
	}

	Common::U32String u32String(text, textLength);
	delete[] text;

	return u32String;
}

bool OSystem_iOS7::setTextInClipboard(const Common::U32String &text) {
#ifdef SCUMM_LITTLE_ENDIAN
	NSStringEncoding stringEncoding = NSUTF32LittleEndianStringEncoding;
#else
	NSStringEncoding stringEncoding = NSUTF32BigEndianStringEncoding;
#endif
	UIPasteboard *pb = [UIPasteboard generalPasteboard];
	NSString *nsstring = [[NSString alloc] initWithBytes:text.c_str() length:4*text.size() encoding: stringEncoding];
	[pb setString:nsstring];
	[nsstring release];
	return true;
}

bool OSystem_iOS7::openUrl(const Common::String &url) {
	UIApplication *application = [UIApplication sharedApplication];
	NSURL *nsurl = [NSURL URLWithString:[NSString stringWithCString:url.c_str() encoding:NSISOLatin1StringEncoding]];
	// The way to oipen a URL has changed in iOS 10. Check if the iOS 10 method is recognized
	// and otherwise use the old method.
	if ([application respondsToSelector:@selector(openURL:options:completionHandler:)]) {
		execute_on_main_thread_async(^ {
			[application openURL:nsurl options:@{} completionHandler:nil];
		});
	} else {
		execute_on_main_thread_async(^ {
			[application openURL:nsurl];
		});
	}
	return true;
}

bool OSystem_iOS7::isConnectionLimited() {
	// If we are connected to the internet through a cellular network, return true
	SCNetworkReachabilityRef ref = SCNetworkReachabilityCreateWithName(CFAllocatorGetDefault(), [@"www.google.com" UTF8String]);
	if (!ref)
		return false;
	SCNetworkReachabilityFlags flags = 0;
	SCNetworkReachabilityGetFlags(ref, &flags);
	CFRelease(ref);
	return (flags & kSCNetworkReachabilityFlagsIsWWAN);
}

void OSystem_iOS7::handleEvent_applicationSaveState() {
	[[iOS7AppDelegate iPhoneView] beginBackgroundSaveStateTask];
	saveState();
	[[iOS7AppDelegate iPhoneView] endBackgroundSaveStateTask];
}

void OSystem_iOS7::handleEvent_applicationRestoreState() {
	restoreState();
}

void OSystem_iOS7::handleEvent_applicationClearState() {
	clearState();
}
