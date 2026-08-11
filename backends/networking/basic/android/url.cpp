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

#include "backends/networking/basic/android/url.h"

#include "common/debug.h"
#include "common/textconsole.h"

namespace Networking {

URL *URL::parseURL(const Common::String &url) {
	return AndroidURL::parseURL(url);
}

URL *AndroidURL::parseURL(const Common::String &url) {
	JNIEnv *env = JNI::getEnv();

	jstring url_sobj = env->NewStringUTF(url.c_str());
	jobject url_obj = env->NewObject(NetJNI::_CLS_URL, NetJNI::_MID_url_init, url_sobj);

	env->DeleteLocalRef(url_sobj);

	if (env->ExceptionCheck()) {
		LOGE("URL::<init> failed");
		env->ExceptionDescribe();
		env->ExceptionClear();

		return nullptr;
	}

	URL *url_ = new AndroidURL(env, url_obj);
	env->DeleteLocalRef(url_obj);

	return url_;
}

AndroidURL::AndroidURL(JNIEnv *env, jobject url) {
	_url = env->NewGlobalRef(url);
}

AndroidURL::~AndroidURL() {
	JNIEnv *env = JNI::getEnv();
	env->DeleteGlobalRef(_url);
}

Common::String AndroidURL::getScheme() const {
	JNIEnv *env = JNI::getEnv();

	jstring protocol_obj = (jstring)env->CallObjectMethod(_url, NetJNI::_MID_url_getProtocol);
	if (env->ExceptionCheck()) {
		LOGE("URL::getProtocol failed");
		env->ExceptionDescribe();
		env->ExceptionClear();

		return Common::String();
	}

	uint length = env->GetStringLength(protocol_obj);
	if (!length) {
		env->DeleteLocalRef(protocol_obj);
		return Common::String();
	}

	const char *protocol_ptr = env->GetStringUTFChars(protocol_obj, 0);
	if (!protocol_ptr) {
		env->DeleteLocalRef(protocol_obj);
		return Common::String();
	}

	Common::String result(protocol_ptr, length);

	env->ReleaseStringUTFChars(protocol_obj, protocol_ptr);
	env->DeleteLocalRef(protocol_obj);

	return result;
}

Common::String AndroidURL::getHost() const {
	JNIEnv *env = JNI::getEnv();

	jstring protocol_obj = (jstring)env->CallObjectMethod(_url, NetJNI::_MID_url_getHost);
	if (env->ExceptionCheck()) {
		LOGE("URL::getHost failed");
		env->ExceptionDescribe();
		env->ExceptionClear();

		return Common::String();
	}

	uint length = env->GetStringLength(protocol_obj);
	if (!length) {
		env->DeleteLocalRef(protocol_obj);
		return Common::String();
	}

	const char *protocol_ptr = env->GetStringUTFChars(protocol_obj, 0);
	if (!protocol_ptr) {
		env->DeleteLocalRef(protocol_obj);
		return Common::String();
	}

	Common::String result(protocol_ptr, length);

	env->ReleaseStringUTFChars(protocol_obj, protocol_ptr);
	env->DeleteLocalRef(protocol_obj);

	return result;
}

int AndroidURL::getPort(bool defaultPort) const {
	JNIEnv *env = JNI::getEnv();

	jint port = env->CallIntMethod(_url, NetJNI::_MID_url_getPort);
	if (env->ExceptionCheck()) {
		LOGE("URL::getPort failed");
		env->ExceptionDescribe();
		env->ExceptionClear();

		return -1;
	}
	if (port == -1 && defaultPort) {
		port = env->CallIntMethod(_url, NetJNI::_MID_url_getDefaultPort);
		if (env->ExceptionCheck()) {
			LOGE("URL::getDefaultPort failed");
			env->ExceptionDescribe();
			env->ExceptionClear();

			return -1;
		}
	}
	if (port == -1) {
		port = 0;
	}

	return port;
}

} // End of namespace Networking
