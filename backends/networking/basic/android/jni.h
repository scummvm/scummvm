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

#ifndef BACKENDS_NETWORKING_BASIC_ANDROID_JNI_H
#define BACKENDS_NETWORKING_BASIC_ANDROID_JNI_H

#include <jni.h>

namespace Networking {

class NetJNI {
#ifdef USE_HTTP
	friend class ConnectionManagerAndroid;
	friend class NetworkReadStreamAndroid;
#endif
	friend class AndroidSocket;
	friend class AndroidURL;

public:
	static void init(JNIEnv *env);

private:
	static jclass    _CLS_URL;

	static jmethodID _MID_url_init;
	static jmethodID _MID_url_getProtocol;
	static jmethodID _MID_url_getHost;
	static jmethodID _MID_url_getPort;
	static jmethodID _MID_url_getDefaultPort;

	static jclass    _CLS_Socket;

	static jmethodID _MID_socket_init;
	static jmethodID _MID_socket_ready;
	static jmethodID _MID_socket_send;
	static jmethodID _MID_socket_recv;
	static jmethodID _MID_socket_close;

#ifdef USE_HTTP
	static jclass    _CLS_HTTPManager;

	static jmethodID _MID_manager_init;
	static jmethodID _MID_manager_startRequest;
	static jmethodID _MID_manager_poll;
	static jfieldID  _FID_manager__empty;

	static const JNINativeMethod _natives_request[];

	static jclass    _CLS_HTTPRequest;

	static jmethodID _MID_request_bufinit;
	static jmethodID _MID_request_forminit;
	static jmethodID _MID_request_cancel;
	static jmethodID _MID_request_getURL;
#endif

	static bool _init;
};

} // End of namespace Networking

#endif

