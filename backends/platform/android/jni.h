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

#include "common/fs.h"
#include "common/archive.h"

class OSystem_Android;

class JNI {
private:
	JNI();
	virtual ~JNI();

public:
	static int surface_changeid;
	static int egl_surface_width;
	static int egl_surface_height;

	static jint onLoad(JavaVM *vm);

	static JNIEnv *getEnv();

	static void attachThread();
	static void detachThread();

	static void setReadyForEvents(bool ready);

	static void getPluginDirectories(Common::FSList &dirs);
	static void setWindowCaption(const char *caption);
	static void displayMessageOnOSD(const char *msg);
	static void showVirtualKeyboard(bool enable);
	static void addSysArchivesToSearchSet(Common::SearchSet &s, int priority);

	static inline int swapBuffers();
	static void initSurface();
	static void deinitSurface();

	static void setAudioPause();
	static void setAudioPlay();
	static void setAudioStop();

	static inline int writeAudio(JNIEnv *env, jbyteArray &data, int offset,
									int size);

private:
	static JavaVM *_vm;
	// back pointer to (java) peer instance
	static jobject _jobj;
	static jobject _jobj_audio_track;

	static Common::Archive *_asset_archive;
	static OSystem_Android *_system;

	static bool _ready_for_events;

	static jfieldID _FID_Event_type;
	static jfieldID _FID_Event_synthetic;
	static jfieldID _FID_Event_kbd_keycode;
	static jfieldID _FID_Event_kbd_ascii;
	static jfieldID _FID_Event_kbd_flags;
	static jfieldID _FID_Event_mouse_x;
	static jfieldID _FID_Event_mouse_y;
	static jfieldID _FID_Event_mouse_relative;
	static jfieldID _FID_ScummVM_nativeScummVM;

	static jmethodID _MID_displayMessageOnOSD;
	static jmethodID _MID_setWindowCaption;
	static jmethodID _MID_showVirtualKeyboard;
	static jmethodID _MID_getSysArchives;
	static jmethodID _MID_getPluginDirectories;
	static jmethodID _MID_swapBuffers;
	static jmethodID _MID_initSurface;
	static jmethodID _MID_deinitSurface;

	static jmethodID _MID_AudioTrack_flush;
	static jmethodID _MID_AudioTrack_pause;
	static jmethodID _MID_AudioTrack_play;
	static jmethodID _MID_AudioTrack_stop;
	static jmethodID _MID_AudioTrack_write;

	static const JNINativeMethod _natives[];

	static void throwByName(JNIEnv *env, const char *name, const char *msg);
	static void throwRuntimeException(JNIEnv *env, const char *msg);

	// natives for the dark side
	static void create(JNIEnv *env, jobject self, jobject am, jobject at,
						jint sample_rate, jint buffer_size);
	static void destroy(JNIEnv *env, jobject self);

	static void setSurface(JNIEnv *env, jobject self, jint width, jint height);
	static jint main(JNIEnv *env, jobject self, jobjectArray args);

	static void pushEvent(JNIEnv *env, jobject self, jobject java_event);
	static void enableZoning(JNIEnv *env, jobject self, jboolean enable);
};

inline int JNI::swapBuffers() {
	JNIEnv *env = JNI::getEnv();

	return env->CallIntMethod(_jobj, _MID_swapBuffers);
}

inline int JNI::writeAudio(JNIEnv *env, jbyteArray &data, int offset, int size) {
	return env->CallIntMethod(_jobj_audio_track, _MID_AudioTrack_write, data, offset, size);
}

#endif
#endif

