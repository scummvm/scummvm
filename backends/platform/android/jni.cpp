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

#if defined(__ANDROID__)

#include "base/main.h"
#include "common/config-manager.h"

#include "backends/platform/android/android.h"

// Fix JNIEXPORT declaration to actually do something useful
#undef JNIEXPORT
#define JNIEXPORT __attribute__ ((visibility("default")))

static JavaVM *cached_jvm;
static jfieldID FID_Event_type;
static jfieldID FID_Event_synthetic;
static jfieldID FID_Event_kbd_keycode;
static jfieldID FID_Event_kbd_ascii;
static jfieldID FID_Event_kbd_flags;
static jfieldID FID_Event_mouse_x;
static jfieldID FID_Event_mouse_y;
static jfieldID FID_Event_mouse_relative;
static jfieldID FID_ScummVM_nativeScummVM;
static jmethodID MID_Object_wait;

JNIEnv *JNU_GetEnv() {
	JNIEnv *env = 0;

	jint res = cached_jvm->GetEnv((void **)&env, JNI_VERSION_1_2);

	if (res != JNI_OK) {
		LOGE("GetEnv() failed: %d", res);
		abort();
	}

	return env;
}

void JNU_AttachThread() {
	JNIEnv *env = 0;
	jint res = cached_jvm->AttachCurrentThread(&env, 0);

	if (res != JNI_OK) {
		LOGE("AttachCurrentThread() failed: %d", res);
		abort();
	}
}

void JNU_DetachThread() {
	jint res = cached_jvm->DetachCurrentThread();

	if (res != JNI_OK) {
		LOGE("DetachCurrentThread() failed: %d", res);
		abort();
	}
}

static void JNU_ThrowByName(JNIEnv *env, const char *name, const char *msg) {
	jclass cls = env->FindClass(name);

	// if cls is 0, an exception has already been thrown
	if (cls != 0)
		env->ThrowNew(cls, msg);

	env->DeleteLocalRef(cls);
}

static void ScummVM_create(JNIEnv *env, jobject self, jobject am) {
	assert(!g_system);

	g_sys = new OSystem_Android(am);
	assert(g_sys);

	// Exception already thrown by initJavaHooks?
	if (!g_sys->initJavaHooks(env, self))
		return;

	env->SetLongField(self, FID_ScummVM_nativeScummVM, (jlong)g_sys);

	g_system = g_sys;
}

static void ScummVM_nativeDestroy(JNIEnv *env, jobject self) {
	assert(g_sys);

	OSystem_Android *tmp = g_sys;
	g_system = 0;
	g_sys = 0;
	delete tmp;
}

static jint ScummVM_scummVMMain(JNIEnv *env, jobject self, jobjectArray args) {
	assert(g_sys);

	const int MAX_NARGS = 32;
	int res = -1;

	int argc = env->GetArrayLength(args);
	if (argc > MAX_NARGS) {
		JNU_ThrowByName(env, "java/lang/IllegalArgumentException",
						"too many arguments");
		return 0;
	}

	char *argv[MAX_NARGS];

	// note use in cleanup loop below
	int nargs;

	for (nargs = 0; nargs < argc; ++nargs) {
		jstring arg = (jstring)env->GetObjectArrayElement(args, nargs);

		if (arg == 0) {
			argv[nargs] = 0;
		} else {
			const char *cstr = env->GetStringUTFChars(arg, 0);

			argv[nargs] = const_cast<char *>(cstr);

			// exception already thrown?
			if (cstr == 0)
				goto cleanup;
		}

		env->DeleteLocalRef(arg);
	}

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new AndroidPluginProvider());
#endif

	LOGI("Entering scummvm_main with %d args", argc);

	res = scummvm_main(argc, argv);

	LOGI("Exiting scummvm_main");

	g_sys->quit();

cleanup:
	nargs--;

	for (int i = 0; i < nargs; ++i) {
		if (argv[i] == 0)
			continue;

		jstring arg = (jstring)env->GetObjectArrayElement(args, nargs);

		// Exception already thrown?
		if (arg == 0)
			return res;

		env->ReleaseStringUTFChars(arg, argv[i]);
		env->DeleteLocalRef(arg);
	}

	return res;
}

static void ScummVM_pushEvent(JNIEnv *env, jobject self, jobject java_event) {
	assert(g_sys);

	Common::Event event;
	event.type = (Common::EventType)env->GetIntField(java_event,
														FID_Event_type);

	event.synthetic =
		env->GetBooleanField(java_event, FID_Event_synthetic);

	switch (event.type) {
	case Common::EVENT_KEYDOWN:
	case Common::EVENT_KEYUP:
		event.kbd.keycode = (Common::KeyCode)env->GetIntField(
			java_event, FID_Event_kbd_keycode);
		event.kbd.ascii = static_cast<int>(env->GetIntField(
			java_event, FID_Event_kbd_ascii));
		event.kbd.flags = static_cast<int>(env->GetIntField(
			java_event, FID_Event_kbd_flags));
		break;
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
	case Common::EVENT_MBUTTONDOWN:
	case Common::EVENT_MBUTTONUP:
		event.mouse.x =
			env->GetIntField(java_event, FID_Event_mouse_x);
		event.mouse.y =
			env->GetIntField(java_event, FID_Event_mouse_y);
		// This is a terrible hack.	 We stash "relativeness"
		// in the kbd.flags field until pollEvent() can work
		// it out.
		event.kbd.flags = env->GetBooleanField(
			java_event, FID_Event_mouse_relative) ? 1 : 0;
		break;
	default:
		break;
	}

	g_sys->pushEvent(event);
}

static void ScummVM_audioMixCallback(JNIEnv *env, jobject self,
										jbyteArray jbuf) {
	assert(g_sys);

	jsize len = env->GetArrayLength(jbuf);
	jbyte *buf = env->GetByteArrayElements(jbuf, 0);

	if (buf == 0) {
		warning("Unable to get Java audio byte array. Skipping");
		return;
	}

	Audio::MixerImpl *mixer =
		static_cast<Audio::MixerImpl *>(g_sys->getMixer());
	assert(mixer);
	mixer->mixCallback(reinterpret_cast<byte *>(buf), len);

	env->ReleaseByteArrayElements(jbuf, buf, 0);
}

static void ScummVM_setConfManInt(JNIEnv *env, jclass cls,
									jstring key_obj, jint value) {
	ENTER("%p, %d", key_obj, (int)value);

	const char *key = env->GetStringUTFChars(key_obj, 0);

	if (key == 0)
		return;

	ConfMan.setInt(key, value);

	env->ReleaseStringUTFChars(key_obj, key);
}

static void ScummVM_setConfManString(JNIEnv *env, jclass cls, jstring key_obj,
										jstring value_obj) {
	ENTER("%p, %p", key_obj, value_obj);

	const char *key = env->GetStringUTFChars(key_obj, 0);

	if (key == 0)
		return;

	const char *value = env->GetStringUTFChars(value_obj, 0);

	if (value == 0) {
		env->ReleaseStringUTFChars(key_obj, key);
		return;
	}

	ConfMan.set(key, value);

	env->ReleaseStringUTFChars(value_obj, value);
	env->ReleaseStringUTFChars(key_obj, key);
}

static void ScummVM_enableZoning(JNIEnv *env, jobject self, jboolean enable) {
	assert(g_sys);

	g_sys->enableZoning(enable);
}

static void ScummVM_setSurfaceSize(JNIEnv *env, jobject self,
									jint width, jint height) {
	assert(g_sys);

	g_sys->setSurfaceSize(width, height);
}

const static JNINativeMethod gMethods[] = {
	{ "create", "(Landroid/content/res/AssetManager;)V",
		(void *)ScummVM_create },
	{ "nativeDestroy", "()V",
		(void *)ScummVM_nativeDestroy },
	{ "scummVMMain", "([Ljava/lang/String;)I",
	 	(void *)ScummVM_scummVMMain },
	{ "pushEvent", "(Lorg/inodes/gus/scummvm/Event;)V",
		(void *)ScummVM_pushEvent },
	{ "audioMixCallback", "([B)V",
		(void *)ScummVM_audioMixCallback },
	{ "setConfMan", "(Ljava/lang/String;I)V",
		(void *)ScummVM_setConfManInt },
	{ "setConfMan", "(Ljava/lang/String;Ljava/lang/String;)V",
		(void *)ScummVM_setConfManString },
	{ "enableZoning", "(Z)V",
		(void *)ScummVM_enableZoning },
	{ "setSurfaceSize", "(II)V",
		(void *)ScummVM_setSurfaceSize },
};

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *jvm, void *reserved) {
	cached_jvm = jvm;

	JNIEnv *env;

	if (jvm->GetEnv((void **)&env, JNI_VERSION_1_2))
		return JNI_ERR;

	jclass cls = env->FindClass("org/inodes/gus/scummvm/ScummVM");
	if (cls == 0)
		return JNI_ERR;

	if (env->RegisterNatives(cls, gMethods, ARRAYSIZE(gMethods)) < 0)
		return JNI_ERR;

	FID_ScummVM_nativeScummVM = env->GetFieldID(cls, "nativeScummVM", "J");
	if (FID_ScummVM_nativeScummVM == 0)
		return JNI_ERR;

	jclass event = env->FindClass("org/inodes/gus/scummvm/Event");
	if (event == 0)
		return JNI_ERR;

	FID_Event_type = env->GetFieldID(event, "type", "I");
	if (FID_Event_type == 0)
		return JNI_ERR;

	FID_Event_synthetic = env->GetFieldID(event, "synthetic", "Z");
	if (FID_Event_synthetic == 0)
		return JNI_ERR;

	FID_Event_kbd_keycode = env->GetFieldID(event, "kbd_keycode", "I");
	if (FID_Event_kbd_keycode == 0)
		return JNI_ERR;

	FID_Event_kbd_ascii = env->GetFieldID(event, "kbd_ascii", "I");
	if (FID_Event_kbd_ascii == 0)
		return JNI_ERR;

	FID_Event_kbd_flags = env->GetFieldID(event, "kbd_flags", "I");
	if (FID_Event_kbd_flags == 0)
		return JNI_ERR;

	FID_Event_mouse_x = env->GetFieldID(event, "mouse_x", "I");
	if (FID_Event_mouse_x == 0)
		return JNI_ERR;

	FID_Event_mouse_y = env->GetFieldID(event, "mouse_y", "I");
	if (FID_Event_mouse_y == 0)
		return JNI_ERR;

	FID_Event_mouse_relative = env->GetFieldID(event, "mouse_relative", "Z");
	if (FID_Event_mouse_relative == 0)
		return JNI_ERR;

	cls = env->FindClass("java/lang/Object");
	if (cls == 0)
		return JNI_ERR;

	MID_Object_wait = env->GetMethodID(cls, "wait", "()V");
	if (MID_Object_wait == 0)
		return JNI_ERR;

	return JNI_VERSION_1_2;
}

#endif

