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

#include "backends/networking/http/android/networkreadstream-android.h"
#include "backends/networking/http/android/connectionmanager-android.h"
#include "common/debug.h"

namespace Networking {

NetworkReadStream *NetworkReadStream::make(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading, bool usingPatch, bool keepAlive, long keepAliveIdle, long keepAliveInterval) {
	return new NetworkReadStreamAndroid(url, headersList, postFields, uploading, usingPatch, keepAlive, keepAliveIdle, keepAliveInterval);
}

NetworkReadStream *NetworkReadStream::make(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles, bool keepAlive, long keepAliveIdle, long keepAliveInterval) {
	return new NetworkReadStreamAndroid(url, headersList, formFields, formFiles, keepAlive, keepAliveIdle, keepAliveInterval);
}

NetworkReadStream *NetworkReadStream::make(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post, bool keepAlive, long keepAliveIdle, long keepAliveInterval) {
	return new NetworkReadStreamAndroid(url, headersList, buffer, bufferSize, uploading, usingPatch, post, keepAlive, keepAliveIdle, keepAliveInterval);
}

void NetworkReadStreamAndroid::gotHeaders(JNIEnv *env, jobject obj, jlong nativePointer, jobjectArray headers) {
	NetworkReadStreamAndroid *stream = (NetworkReadStreamAndroid *)nativePointer;
	if (!stream) {
		return;
	}

	jsize size = env->GetArrayLength(headers);
	assert((size & 1) == 0);

	stream->_responseHeadersMap.clear();
	for (jsize i = 0; i < size; i += 2) {
		jstring key_obj   = (jstring)env->GetObjectArrayElement(headers, i);
		jstring value_obj = (jstring)env->GetObjectArrayElement(headers, i + 1);
		const char *key = env->GetStringUTFChars(key_obj, 0);
		const char *value = env->GetStringUTFChars(value_obj, 0);

		if (key != nullptr && value != nullptr) {
			stream->_responseHeadersMap[key] = value;
		}
		env->ReleaseStringUTFChars(key_obj, key);
		env->ReleaseStringUTFChars(value_obj, value);

		env->DeleteLocalRef(key_obj);
		env->DeleteLocalRef(value_obj);
	}
}

void NetworkReadStreamAndroid::gotData(JNIEnv *env, jobject obj, jlong nativePointer, jbyteArray data, jint size, jint totalSize) {
	NetworkReadStreamAndroid *stream = (NetworkReadStreamAndroid *)nativePointer;
	if (!stream) {
		return;
	}

	jsize arrSize = env->GetArrayLength(data);
	assert(size >= 0 && (jsize)size <= arrSize);

	if (size > 0) {
		jbyte *dataP = (jbyte *)env->GetPrimitiveArrayCritical(data, 0);
		assert(dataP);

		stream->_backingStream.write(dataP, size);
		stream->_downloaded += size;

		env->ReleasePrimitiveArrayCritical(data, dataP, JNI_ABORT);
	}

	stream->setProgress(stream->_downloaded, totalSize);
}

void NetworkReadStreamAndroid::finished_(JNIEnv *env, jobject obj, jlong nativePointer, jint errorCode, jstring errorMsg) {
	NetworkReadStreamAndroid *stream = (NetworkReadStreamAndroid *)nativePointer;
	if (!stream) {
		return;
	}

	Common::String errorMsgStr;
	if (errorMsg) {
		const char *errorMsgP = env->GetStringUTFChars(errorMsg, 0);
		if (errorMsgP != 0) {
			errorMsgStr = Common::String(errorMsgP);
			env->ReleaseStringUTFChars(errorMsg, errorMsgP);
		}
	}

	stream->finished(errorCode, errorMsgStr);
}

static jobjectArray getHeaders(JNIEnv *env, RequestHeaders *headersList) {
	if (!headersList) {
		return nullptr;
	}

	jclass stringClass = env->FindClass("java/lang/String");
	jobjectArray array = env->NewObjectArray(headersList->size() * 2, stringClass, nullptr);
	env->DeleteLocalRef(stringClass);

	int i = 0;
	for (const Common::String &header : *headersList) {
		// Find the colon separator
		uint colonPos = header.findFirstOf(':');
		if (colonPos == Common::String::npos) {
			warning("NetworkReadStreamAndroid: Malformed header (no colon): %s", header.c_str());
			continue;
		}

		// Split into key and value parts
		Common::String key = header.substr(0, colonPos);
		Common::String value = header.substr(colonPos + 1);

		// Trim whitespace from key and value
		key.trim();
		value.trim();

		jobject key_obj   = env->NewStringUTF(key.c_str());
		jobject value_obj = env->NewStringUTF(value.c_str());

		// Store key and value as separate strings
		env->SetObjectArrayElement(array, i,     key_obj);
		env->SetObjectArrayElement(array, i + 1, value_obj);

		env->DeleteLocalRef(key_obj);
		env->DeleteLocalRef(value_obj);

		i += 2;
	}

	return array;
}

static jobjectArray getFormFields(JNIEnv *env, const Common::HashMap<Common::String, Common::String> &map) {
	jclass stringClass = env->FindClass("java/lang/String");
	jobjectArray array = env->NewObjectArray(map.size() * 2, stringClass, nullptr);
	env->DeleteLocalRef(stringClass);

	int i = 0;
	for (const Common::HashMap<Common::String, Common::String>::Node &entry : map) {
		jobject key_obj   = env->NewStringUTF(entry._key.c_str());
		jobject value_obj = env->NewStringUTF(entry._value.c_str());

		// Store key and value as separate strings
		env->SetObjectArrayElement(array, i,     key_obj);
		env->SetObjectArrayElement(array, i + 1, value_obj);

		env->DeleteLocalRef(key_obj);
		env->DeleteLocalRef(value_obj);
	}

	return array;
}

static jobjectArray getFormFiles(JNIEnv *env, const Common::HashMap<Common::String, Common::Path> &map) {
	jclass stringClass = env->FindClass("java/lang/String");
	jobjectArray array = env->NewObjectArray(map.size() * 2, stringClass, nullptr);
	env->DeleteLocalRef(stringClass);

	int i = 0;
	for (const Common::HashMap<Common::String, Common::Path>::Node &entry : map) {
		jobject key_obj   = env->NewStringUTF(entry._key.c_str());
		jobject value_obj = env->NewStringUTF(entry._value.toString('/').c_str());

		// Store key and value as separate strings
		env->SetObjectArrayElement(array, i,     key_obj);
		env->SetObjectArrayElement(array, i + 1, value_obj);

		env->DeleteLocalRef(key_obj);
		env->DeleteLocalRef(value_obj);
	}

	return array;
}

NetworkReadStreamAndroid::NetworkReadStreamAndroid(const char *url, RequestHeaders *headersList, const Common::String &postFields,
	bool uploading, bool usingPatch, bool keepAlive, long keepAliveIdle, long keepAliveInterval) :
	NetworkReadStream(keepAlive, keepAliveIdle, keepAliveInterval), _request(nullptr) {
	if (!reuse(url, headersList, postFields, uploading, usingPatch)) {
		abort();
	}
}

NetworkReadStreamAndroid::NetworkReadStreamAndroid(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields,
	const Common::HashMap<Common::String, Common::Path> &formFiles, bool keepAlive, long keepAliveIdle, long keepAliveInterval) :
	NetworkReadStream(keepAlive, keepAliveIdle, keepAliveInterval), _request(nullptr) {
	if (!reuse(url, headersList, formFields, formFiles)) {
		abort();
	}
}

NetworkReadStreamAndroid::NetworkReadStreamAndroid(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize,
	bool uploading, bool usingPatch, bool post, bool keepAlive, long keepAliveIdle, long keepAliveInterval) :
	NetworkReadStream(keepAlive, keepAliveIdle, keepAliveInterval), _request(nullptr) {
	if (!reuse(url, headersList, buffer, bufferSize, uploading, usingPatch, post)) {
		abort();
	}
}

bool NetworkReadStreamAndroid::reuse(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading, bool usingPatch) {
	JNIEnv *env = JNI::getEnv();

	resetStream(env);

	jstring url_obj = env->NewStringUTF(url);
	jobjectArray headers_obj = getHeaders(env, headersList);
	jbyteArray uploadBuffer_obj = env->NewByteArray(postFields.size());
	env->SetByteArrayRegion(uploadBuffer_obj, 0, postFields.size(), (const jbyte *)postFields.c_str());

	jobject obj = env->NewObject(NetJNI::_CLS_HTTPRequest, NetJNI::_MID_request_bufinit, (jlong)this, url_obj, headers_obj, uploadBuffer_obj, uploading, usingPatch, false);

	env->DeleteLocalRef(uploadBuffer_obj);
	env->DeleteLocalRef(headers_obj);
	env->DeleteLocalRef(url_obj);

	if (env->ExceptionCheck()) {
		LOGE("HTTPRequest::<init> failed");
		env->ExceptionDescribe();
		env->ExceptionClear();

		return false;
	}
	_request = env->NewGlobalRef(obj);
	env->DeleteLocalRef(obj);

	dynamic_cast<ConnectionManagerAndroid &>(ConnMan).registerRequest(env, _request);
	return true;
}

bool NetworkReadStreamAndroid::reuse(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles) {
	JNIEnv *env = JNI::getEnv();

	resetStream(env);

	jstring url_obj = env->NewStringUTF(url);
	jobjectArray headers_obj = getHeaders(env, headersList);
	jobjectArray formFields_obj = getFormFields(env, formFields);
	jobjectArray formFiles_obj = getFormFiles(env, formFiles);

	jobject obj = env->NewObject(NetJNI::_CLS_HTTPRequest, NetJNI::_MID_request_forminit, (jlong)this, url_obj, headers_obj, formFields_obj, formFiles_obj);

	env->DeleteLocalRef(formFiles_obj);
	env->DeleteLocalRef(formFields_obj);
	env->DeleteLocalRef(headers_obj);
	env->DeleteLocalRef(url_obj);

	if (env->ExceptionCheck()) {
		LOGE("HTTPRequest::<init> failed");
		env->ExceptionDescribe();
		env->ExceptionClear();
		return false;
	}
	_request = env->NewGlobalRef(obj);
	env->DeleteLocalRef(obj);

	dynamic_cast<ConnectionManagerAndroid &>(ConnMan).registerRequest(env, _request);
	return true;
}

bool NetworkReadStreamAndroid::reuse(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post) {
	JNIEnv *env = JNI::getEnv();

	resetStream(env);

	jstring url_obj = env->NewStringUTF(url);
	jobjectArray headers_obj = getHeaders(env, headersList);
	jbyteArray uploadBuffer_obj = env->NewByteArray(bufferSize);
	env->SetByteArrayRegion(uploadBuffer_obj, 0, bufferSize, (const jbyte *)buffer);

	jobject obj = env->NewObject(NetJNI::_CLS_HTTPRequest, NetJNI::_MID_request_bufinit, (jlong)this, url_obj, headers_obj, uploadBuffer_obj, uploading, usingPatch, false);

	env->DeleteLocalRef(uploadBuffer_obj);
	env->DeleteLocalRef(headers_obj);
	env->DeleteLocalRef(url_obj);

	if (env->ExceptionCheck()) {
		LOGE("HTTPRequest::<init> failed");
		env->ExceptionDescribe();
		env->ExceptionClear();
		return false;
	}
	_request = env->NewGlobalRef(obj);
	env->DeleteLocalRef(obj);

	dynamic_cast<ConnectionManagerAndroid &>(ConnMan).registerRequest(env, _request);
	return true;
}

NetworkReadStreamAndroid::~NetworkReadStreamAndroid() {
	JNIEnv *env = JNI::getEnv();

	env->CallVoidMethod(_request, NetJNI::_MID_request_cancel);
	if (env->ExceptionCheck()) {
		LOGE("HTTPRequest::cancel failed");
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->DeleteGlobalRef(_request);
}

void NetworkReadStreamAndroid::finished(int errorCode, const Common::String &errorMsg) {
	_requestComplete = true;
	_errorCode = errorCode;
	_errorMsg = errorMsg;

	if (_errorCode >= 200 && _errorCode < 300) {
		debug(9, "NetworkReadStreamAndroid: %s - Request succeeded", currentLocation().c_str());
	} else {
		warning("NetworkReadStreamAndroid: %s - Request failed (%d - %s)", currentLocation().c_str(), _errorCode, _errorMsg.c_str());
	}
}

void NetworkReadStreamAndroid::resetStream(JNIEnv *env) {
	_eos = _requestComplete = false;
	_progressDownloaded = _progressTotal = 0;

	if (_request) {
		env->CallVoidMethod(_request, NetJNI::_MID_request_cancel);
		if (env->ExceptionCheck()) {
			LOGE("HTTPRequest::cancel failed");
			env->ExceptionDescribe();
			env->ExceptionClear();
		}
		env->DeleteGlobalRef(_request);
		_request = nullptr;
	}

	_responseHeadersMap.clear();
	_downloaded = 0;
	_errorCode = 0;
	_errorMsg.clear();
}

Common::String NetworkReadStreamAndroid::currentLocation() const {
	if (!_request) {
		return Common::String();
	}

	JNIEnv *env = JNI::getEnv();

	jstring location_obj = (jstring)env->CallObjectMethod(_request, NetJNI::_MID_request_getURL);
	if (env->ExceptionCheck()) {
		LOGE("HTTPRequest::getURL failed");
		env->ExceptionDescribe();
		env->ExceptionClear();

		return Common::String();
	}

	uint length = env->GetStringLength(location_obj);
	if (!length) {
		env->DeleteLocalRef(location_obj);
		return Common::String();
	}

	const char *location_ptr = env->GetStringUTFChars(location_obj, 0);
	if (!location_ptr) {
		env->DeleteLocalRef(location_obj);
		return Common::String();
	}

	Common::String result(location_ptr, length);

	env->ReleaseStringUTFChars(location_obj, location_ptr);
	env->DeleteLocalRef(location_obj);

	return result;
}

} // End of namespace Networking
