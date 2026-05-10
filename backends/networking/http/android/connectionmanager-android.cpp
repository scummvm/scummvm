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

// Allow use of stuff in <time.h> and abort()
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_abort

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(__printf__, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "backends/platform/android/android.h"
#include "backends/platform/android/jni-android.h"

#include "backends/networking/basic/android/jni.h"

#include "backends/networking/http/android/connectionmanager-android.h"
#include "backends/networking/http/android/networkreadstream-android.h"
#include "common/debug.h"

namespace Common {

template<>
Networking::ConnectionManager *Singleton<Networking::ConnectionManager>::makeInstance() {
	return new Networking::ConnectionManagerAndroid();
}

} // namespace Common

namespace Networking {

ConnectionManagerAndroid::ConnectionManagerAndroid() : ConnectionManager(), _manager(0) {
	JNIEnv *env = JNI::getEnv();

	jobject obj = env->NewObject(NetJNI::_CLS_HTTPManager, NetJNI::_MID_manager_init);
	if (env->ExceptionCheck()) {
		LOGE("HTTPManager::<init> failed");
		env->ExceptionDescribe();
		env->ExceptionClear();
		abort();
	}
	_manager = env->NewGlobalRef(obj);
	env->DeleteLocalRef(obj);
}

ConnectionManagerAndroid::~ConnectionManagerAndroid() {
	JNIEnv *env = JNI::getEnv();
	env->DeleteGlobalRef(_manager);
}

void ConnectionManagerAndroid::registerRequest(JNIEnv *env, jobject request) const {
	env->CallVoidMethod(_manager, NetJNI::_MID_manager_startRequest, request);
	if (env->ExceptionCheck()) {
		LOGE("HTTPManager::startRequest failed");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

// private goes here:
void ConnectionManagerAndroid::processTransfers() {
	JNIEnv *env = JNI::getEnv();

	// Don't call Java is there is no need
	// This is not perfect as the worker threads can update it and we are not
	// synchronized but that should do the job
	if (env->GetBooleanField(_manager, NetJNI::_FID_manager__empty)) {
		return;
	}

	env->CallVoidMethod(_manager, NetJNI::_MID_manager_poll);
	if (env->ExceptionCheck()) {
		LOGE("HTTPManager::poll failed");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

} // End of namespace Networking
