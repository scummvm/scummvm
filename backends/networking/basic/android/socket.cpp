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

#include "backends/networking/basic/android/socket.h"

#include "common/debug.h"
#include "common/textconsole.h"

namespace Networking {

Socket *Socket::connect(const Common::String &url) {
	return AndroidSocket::connect(url);
}

Socket *AndroidSocket::connect(const Common::String &url) {
	JNIEnv *env = JNI::getEnv();

	jstring url_obj = env->NewStringUTF(url.c_str());
	jobject socket_obj = env->NewObject(NetJNI::_CLS_Socket, NetJNI::_MID_socket_init, url_obj);

	env->DeleteLocalRef(url_obj);

	if (env->ExceptionCheck()) {
		LOGE("Socket::<init> failed");
		env->ExceptionDescribe();
		env->ExceptionClear();

		return nullptr;
	}

	Socket *socket = new AndroidSocket(env, socket_obj);
	env->DeleteLocalRef(socket_obj);

	return socket;
}

AndroidSocket::AndroidSocket(JNIEnv *env, jobject socket) {
	_socket = env->NewGlobalRef(socket);
}

AndroidSocket::~AndroidSocket() {
	JNIEnv *env = JNI::getEnv();

	env->CallVoidMethod(_socket, NetJNI::_MID_socket_close);
	if (env->ExceptionCheck()) {
		LOGE("Socket::close failed");
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	env->DeleteGlobalRef(_socket);
}

int AndroidSocket::ready() {
	JNIEnv *env = JNI::getEnv();

	jint ret = env->CallIntMethod(_socket, NetJNI::_MID_socket_ready);
	if (env->ExceptionCheck()) {
		LOGE("Socket::ready failed");
		env->ExceptionDescribe();
		env->ExceptionClear();

		// In doubt make it ready
		return 1;
	}

	return ret;
}

size_t AndroidSocket::send(const char *data, int len) {
	JNIEnv *env = JNI::getEnv();

	jbyteArray buffer_obj = env->NewByteArray(len);
	env->SetByteArrayRegion(buffer_obj, 0, len, (const jbyte *)data);

	jint sent = env->CallIntMethod(_socket, NetJNI::_MID_socket_send, buffer_obj);

	env->DeleteLocalRef(buffer_obj);

	if (env->ExceptionCheck()) {
		LOGE("Socket::send failed");
		env->ExceptionDescribe();
		env->ExceptionClear();

		return 0;
	}

	return sent;
}

size_t AndroidSocket::recv(void *data, int maxLen) {
	JNIEnv *env = JNI::getEnv();

	jbyteArray buffer_obj = env->NewByteArray(maxLen);

	jint recvd = env->CallIntMethod(_socket, NetJNI::_MID_socket_recv, buffer_obj);
	assert(recvd <= maxLen);

	if (env->ExceptionCheck()) {
		LOGE("Socket::send failed");
		env->ExceptionDescribe();
		env->ExceptionClear();

		env->DeleteLocalRef(buffer_obj);
		return 0;
	}

	env->GetByteArrayRegion(buffer_obj, 0, recvd, (jbyte *)data);

	return recvd;
}

} // End of namespace Networking

