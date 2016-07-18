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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/networking/browser/openurl.h"
#include <CoreFoundation/CFBundle.h>
#include <ApplicationServices/ApplicationServices.h>

namespace Networking {
namespace Browser {

using namespace std;

bool openUrl(const Common::String &url) {
	CFURLRef urlRef = CFURLCreateWithBytes (
		NULL,
		(UInt8*)url.c_str(),
		url.size(),
		kCFStringEncodingASCII,
		NULL
	);
	int result = LSOpenCFURLRef(urlRef, 0);
	CFRelease(urlRef);
	return result == 0;
}

} // End of namespace Browser
} // End of namespace Networking

