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

#include "backends/fs/posix/posix-iostream.h"

#include <sys/stat.h>

#if defined(ANDROID_PLAIN_PORT)
#include "backends/platform/android/jni-android.h"
#include <unistd.h>
#endif


PosixIoStream *PosixIoStream::makeFromPath(const Common::String &path, bool writeMode) {
	FILE *handle = fopen(path.c_str(), writeMode ? "wb" : "rb");

	if (handle)
		return new PosixIoStream(handle);

#if defined(ANDROID_PLAIN_PORT)
	else {
		// TODO also address case for writeMode false

		// TODO eventually android specific stuff should be moved to an Android backend for fs
		//      peterkohaut already has some work on that in his fork (moving the port to more native code)
		//      However, I have not found a way to do this Storage Access Framework stuff natively yet.

		// if we are here we are only interested in hackyFilenames -- which mean we went through SAF. Otherwise we ignore the case
		if (writeMode) {
			Common::String hackyFilename = JNI::createFileWithSAF(path);
			// https://stackoverflow.com/questions/59000390/android-accessing-files-in-native-c-c-code-with-google-scoped-storage-api
			//warning ("PosixIoStream::makeFromPath() JNI::createFileWithSAF returned: %s", hackyFilename.c_str() );
			if (strstr(hackyFilename.c_str(), "/proc/self/fd/") == hackyFilename.c_str()) {
				//warning ("PosixIoStream::makeFromPath() match with hacky prefix!" );
				int fd = atoi(hackyFilename.c_str() + 14);
				if (fd != 0) {
					//warning ("PosixIoStream::makeFromPath() got fd int: %d!", fd );
					// Why dup(fd) below: if we called fdopen() on the
					// original fd value, and the native code closes
					// and tries to re-open that file, the second fdopen(fd)
					// would fail, return NULL - after closing the
					// original fd received from Android, it's no longer valid.
					FILE *safHandle = fdopen(dup(fd), "wb");
					// Why rewind(fp): if the native code closes and
					// opens again the file, the file read/write position
					// would not change, because with dup(fd) it's still
					// the same file...
					rewind(safHandle);
					if (safHandle) {
						return new PosixIoStream(safHandle, true, hackyFilename);
					}
				}
			}
	   }
	}
#endif // ANDROID_PLAIN_PORT

	return nullptr;
}


#if defined(ANDROID_PLAIN_PORT)
PosixIoStream::PosixIoStream(void *handle, bool bCreatedWithSAF, Common::String sHackyFilename) :
		StdioStream(handle) {
	createdWithSAF = bCreatedWithSAF;
	hackyfilename = sHackyFilename;
}

PosixIoStream::~PosixIoStream() {
	//warning("PosixIoStream::~PosixIoStream() closing file");
	if (createdWithSAF && !hackyfilename.empty() ) {
		JNI::closeFileWithSAF(hackyfilename);
	}
	// we'leave the base class destructor to close the FILE
	// it does not seem to matter that the operation is done
	// after the JNI call to close the descriptor on the Java side
}
#endif // ANDROID_PLAIN_PORT

PosixIoStream::PosixIoStream(void *handle) :
		StdioStream(handle) {
#if defined(ANDROID_PLAIN_PORT)
	createdWithSAF = false;
	hackyfilename = "";
#endif // ANDROID_PLAIN_PORT
}

int32 PosixIoStream::size() const {
	int fd = fileno((FILE *)_handle);
	if (fd == -1) {
		return StdioStream::size();
	}

	// Using fstat to obtain the file size is generally faster than fseek / ftell
	// because it does not affect the IO buffer.
	struct stat st;
	if (fstat(fd, &st) == -1) {
		return StdioStream::size();
	}

	return st.st_size;
}
