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

#ifndef BACKENDS_NETWORKING_HTTP_ANDROID_NETWORKREADSTREAM_ANDROID_H
#define BACKENDS_NETWORKING_HTTP_ANDROID_NETWORKREADSTREAM_ANDROID_H

#include <jni.h>

#include "backends/networking/http/networkreadstream.h"

namespace Networking {

class NetworkReadStreamAndroid : public NetworkReadStream {
	friend class NetJNI;
private:
	static void gotHeaders(JNIEnv *env, jobject obj, jlong nativePointer, jobjectArray headers);
	static void gotData(JNIEnv *env, jobject obj, jlong nativePointer, jbyteArray data, jint size, jint totalSize);
	static void finished_(JNIEnv *env, jobject obj, jlong nativePointer, jint errorCode, jstring errorMsg);

	void resetStream(JNIEnv *env);
	void finished(int errorCode, const Common::String &errorMsg);

	jobject _request;

	Common::HashMap<Common::String, Common::String> _responseHeadersMap;
	uint64 _downloaded;
	int _errorCode;
	Common::String _errorMsg;
public:
	NetworkReadStreamAndroid(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading, bool usingPatch, bool keepAlive, long keepAliveIdle, long keepAliveInterval);

	NetworkReadStreamAndroid(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles, bool keepAlive, long keepAliveIdle, long keepAliveInterval);

	NetworkReadStreamAndroid(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post, bool keepAlive, long keepAliveIdle, long keepAliveInterval);

	~NetworkReadStreamAndroid() override;

	/** Send <postFields>, using POST by default. */
	bool reuse(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading = false, bool usingPatch = false) override;
	/** Send <formFields>, <formFiles>, using POST multipart/form. */
	bool reuse(
		const char *url, RequestHeaders *headersList,
		const Common::HashMap<Common::String, Common::String> &formFields,
		const Common::HashMap<Common::String, Common::Path> &formFiles) override;
	/** Send <buffer>, using POST by default. */
	bool reuse(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading = false, bool usingPatch = false, bool post = true) override;

	long httpResponseCode() const override { return _errorCode; }
	Common::String currentLocation() const override;
	/**
	 * Return response headers as HashMap. All header names in
	 * it are lowercase.
	 *
	 * @note This method should be called when eos() == true.
	 */
	Common::HashMap<Common::String, Common::String> responseHeadersMap() const override { return _responseHeadersMap; }

	bool hasError() const override { return _errorCode < 200 || _errorCode >= 300; }
	const char *getError() const override { return _errorMsg.c_str(); }
};

} // End of namespace Networking

#endif
