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

#include "backends/networking/basic/android/jni.h"
#ifdef USE_HTTP
#include "backends/networking/http/android/networkreadstream-android.h"
#endif

namespace Networking {

jclass NetJNI::_CLS_URL = nullptr;

jmethodID NetJNI::_MID_url_init = 0;
jmethodID NetJNI::_MID_url_getProtocol = 0;
jmethodID NetJNI::_MID_url_getHost = 0;
jmethodID NetJNI::_MID_url_getPort = 0;
jmethodID NetJNI::_MID_url_getDefaultPort = 0;

jclass NetJNI::_CLS_Socket = nullptr;

jmethodID NetJNI::_MID_socket_init = 0;
jmethodID NetJNI::_MID_socket_ready = 0;
jmethodID NetJNI::_MID_socket_send = 0;
jmethodID NetJNI::_MID_socket_recv = 0;
jmethodID NetJNI::_MID_socket_close = 0;

#ifdef USE_HTTP
jclass NetJNI::_CLS_HTTPManager = nullptr;

jmethodID NetJNI::_MID_manager_init = 0;
jmethodID NetJNI::_MID_manager_startRequest = 0;
jmethodID NetJNI::_MID_manager_poll = 0;
jfieldID  NetJNI::_FID_manager__empty = 0;

jclass NetJNI::_CLS_HTTPRequest = nullptr;

jmethodID NetJNI::_MID_request_bufinit = 0;
jmethodID NetJNI::_MID_request_forminit = 0;
jmethodID NetJNI::_MID_request_cancel = 0;
jmethodID NetJNI::_MID_request_getURL = 0;

const JNINativeMethod NetJNI::_natives_request[] = {
	{ "gotHeaders", "(J[Ljava/lang/String;)V",
		(void *)NetworkReadStreamAndroid::gotHeaders },
	{ "gotData", "(J[BII)V",
		(void *)NetworkReadStreamAndroid::gotData },
	{ "finished", "(JILjava/lang/String;)V",
		(void *)NetworkReadStreamAndroid::finished_ },
};
#endif

bool NetJNI::_init = false;

void NetJNI::init(JNIEnv *env) {
	if (_init) {
		return;
	}

	// We can't call error here as the backend is not built yet
#define FIND_CONSTRUCTOR(prefix, signature) do {                             \
    _MID_ ## prefix ## init = env->GetMethodID(cls, "<init>", signature "V");\
        if (_MID_ ## prefix ## init == 0) {                                  \
            LOGE("Can't find method ID <init>");                             \
            abort();                                                         \
        }                                                                    \
    } while (0)
#define FIND_METHOD(prefix, name, signature) do {                            \
    _MID_ ## prefix ## name = env->GetMethodID(cls, #name, signature);       \
        if (_MID_ ## prefix ## name == 0) {                                  \
            LOGE("Can't find method ID " #name);                             \
            abort();                                                         \
        }                                                                    \
    } while (0)
#define FIND_FIELD(prefix, name, signature) do {                             \
    _FID_ ## prefix ## name = env->GetFieldID(cls, #name, signature);        \
        if (_FID_ ## prefix ## name == 0) {                                  \
            LOGE("Can't find field ID " #name);                              \
            abort();                                                         \
        }                                                                    \
    } while (0)

	jclass cls = env->FindClass("java/net/URL");
	_CLS_URL = (jclass)env->NewGlobalRef(cls);

	FIND_CONSTRUCTOR(url_, "(Ljava/lang/String;)");
	FIND_METHOD(url_, getProtocol, "()Ljava/lang/String;");
	FIND_METHOD(url_, getHost, "()Ljava/lang/String;");
	FIND_METHOD(url_, getPort, "()I");
	FIND_METHOD(url_, getDefaultPort, "()I");

	env->DeleteLocalRef(cls);

	cls = env->FindClass("org/scummvm/scummvm/net/SSocket");
	_CLS_Socket = (jclass)env->NewGlobalRef(cls);

	FIND_CONSTRUCTOR(socket_, "(Ljava/lang/String;)");
	FIND_METHOD(socket_, ready, "()I");
	FIND_METHOD(socket_, send, "([B)I");
	FIND_METHOD(socket_, recv, "([B)I");
	FIND_METHOD(socket_, close, "()V");

	env->DeleteLocalRef(cls);

#ifdef USE_HTTP
	cls = env->FindClass("org/scummvm/scummvm/net/HTTPManager");
	_CLS_HTTPManager = (jclass)env->NewGlobalRef(cls);

	FIND_CONSTRUCTOR(manager_, "()");
	FIND_METHOD(manager_, startRequest, "(Lorg/scummvm/scummvm/net/HTTPRequest;)V");
	FIND_METHOD(manager_, poll, "()V");
	FIND_FIELD(manager_, _empty, "Z");

	env->DeleteLocalRef(cls);

	cls = env->FindClass("org/scummvm/scummvm/net/HTTPRequest");
	_CLS_HTTPRequest = (jclass)env->NewGlobalRef(cls);

	if (env->RegisterNatives(cls, _natives_request, ARRAYSIZE(_natives_request)) < 0) {
		LOGE("Can't register natives for org/scummvm/scummvm/net/HTTPRequest");
		abort();
	}

	FIND_CONSTRUCTOR(request_buf, "(JLjava/lang/String;[Ljava/lang/String;[BZZZ)");
	FIND_CONSTRUCTOR(request_form, "(JLjava/lang/String;[Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)");
	FIND_METHOD(request_, cancel, "()V");
	FIND_METHOD(request_, getURL, "()Ljava/lang/String;");

	env->DeleteLocalRef(cls);
#endif

#undef FIND_FIELD
#undef FIND_METHOD
#undef FIND_CONSTRUCTOR

	_init = true;
}

} // End of namespace Networking
