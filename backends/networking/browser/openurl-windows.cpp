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

#include "backends/networking/browser/openurl.h"
#include "common/textconsole.h"
#include <windows.h>
#include <shellapi.h>

namespace Networking {
namespace Browser {

bool openUrl(const Common::String &url) {
	const uint64 result = (uint64)ShellExecute(0, 0, /*(wchar_t*)nativeFilePath.utf16()*/url.c_str(), 0, 0, SW_SHOWNORMAL);
	// ShellExecute returns a value greater than 32 if successful
	if (result <= 32) {
		warning("ShellExecute failed: error = %u", result);
		return false;
	}
	return true;
}

} // End of namespace Browser
} // End of namespace Networking
