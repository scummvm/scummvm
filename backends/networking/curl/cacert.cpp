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

#if defined(ANDROID_BACKEND)
#include "backends/platform/android/jni-android.h"
#endif

#include "backends/networking/curl/cacert.h"

#include "common/fs.h"

namespace Networking {

Common::String getCaCertPath() {
#if defined(ANDROID_BACKEND)
	// cacert path must exist on filesystem and be reachable by standard open syscall
	// Lets use ScummVM internal directory
	Common::String assetsPath = JNI::getScummVMAssetsPath();
	return assetsPath + "/cacert.pem";
#elif defined(DATA_PATH)
	static enum {
		kNotInitialized,
		kFileNotFound,
		kFileExists
	} state = kNotInitialized;

	if (state == kNotInitialized) {
		Common::FSNode node(DATA_PATH "/cacert.pem");
		state = node.exists() ? kFileExists : kFileNotFound;
	}

	if (state == kFileExists) {
		return DATA_PATH "/cacert.pem";
	} else {
		return "";
	}
#else
	return "";
#endif
}


} // End of namespace Networking
