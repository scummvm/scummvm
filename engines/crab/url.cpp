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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "crab/url.h"

namespace Crab {

#ifdef __WIN32__
#include <ShellAPI.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CFBundle.h>
#endif

void OpenURL(const std::string &url_str) {
#ifdef __WIN32__
	ShellExecuteA(NULL, "open", url_str.c_str(), NULL, NULL, SW_SHOW);
#endif

#ifdef __APPLE__
	CFURLRef url = CFURLCreateWithBytes(
		NULL,                     // allocator
		(UInt8 *)url_str.c_str(), // URLBytes
		url_str.length(),         // length
		kCFStringEncodingASCII,   // encoding
		NULL                      // baseURL
	);
	LSOpenCFURLRef(url, 0);
	CFRelease(url);
#endif

#ifdef __GNUC__
	std::string command = "xdg-open " + url_str;
	system(command.c_str());
#endif
}

} // End of namespace Crab
