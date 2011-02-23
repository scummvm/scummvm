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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef _ANDROID_JNI_H_
#define _ANDROID_JNI_H_

#if defined(__ANDROID__)

#include <jni.h>

extern jobject back_ptr;

class OSystem_Android;

class JNI {
public:
	static jint onLoad(JavaVM *vm);

	static JNIEnv *getEnv();

	static void attachThread();
	static void detachThread();

private:
	JNI();
	virtual ~JNI();

private:
	static JavaVM *_vm;
	// back pointer to (java) peer instance
	static jobject _jobj;
	static OSystem_Android *_system;

	static jfieldID _FID_Event_type;
	static jfieldID _FID_Event_synthetic;
	static jfieldID _FID_Event_kbd_keycode;
	static jfieldID _FID_Event_kbd_ascii;
	static jfieldID _FID_Event_kbd_flags;
	static jfieldID _FID_Event_mouse_x;
	static jfieldID _FID_Event_mouse_y;
	static jfieldID _FID_Event_mouse_relative;
	static jfieldID _FID_ScummVM_nativeScummVM;

	static const JNINativeMethod _natives[];

	static void throwByName(JNIEnv *env, const char *name, const char *msg);

	// natives for the dark side
	static void create(JNIEnv *env, jobject self, jobject am);
	static void destroy(JNIEnv *env, jobject self);
	static jint main(JNIEnv *env, jobject self, jobjectArray args);
	static void pushEvent(JNIEnv *env, jobject self, jobject java_event);
	static void audioMixCallback(JNIEnv *env, jobject self, jbyteArray jbuf);
	static void setConfManInt(JNIEnv *env, jclass cls, jstring key_obj,
								jint value);
	static void setConfManString(JNIEnv *env, jclass cls, jstring key_obj,
							   		jstring value_obj);
	static void enableZoning(JNIEnv *env, jobject self, jboolean enable);
	static void setSurfaceSize(JNIEnv *env, jobject self, jint width,
								jint height);
};

#endif
#endif

