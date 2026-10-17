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

/*
 * This translation unit is built into an indepent shared object
 *
 * It acts as a proxy between Java and the real ScummVM code to let us unload
 * ScummVM and clean up global variables when restarting.
 * As JNI doesn't support unloading libraries, we need to resort to such stunts
 */

#include <jni.h>
#include <dlfcn.h>

typedef jint JNICALL (*onLoadFunction)(JavaVM *vm, void *);

static JavaVM *vm;
static void *reserved;

static void *scummvm_so;
static onLoadFunction scummvm_onLoad;
static bool dirty;

static jboolean scummvm_reload(JNIEnv *env, jclass clazz);

static const JNINativeMethod reloader_natives[] = {
	{ "reload", "()Z", (void *)scummvm_reload },
};

__attribute__ ((visibility("default")))
jint JNICALL JNI_OnLoad(JavaVM *vm_, void *reserved_) {
	vm = vm_;
	reserved = reserved_;

	scummvm_so = dlopen("libscummvm.so", RTLD_NOW | RTLD_LOCAL);
	if (!scummvm_so) {
		return JNI_ERR;
	}
	scummvm_onLoad = (onLoadFunction)dlsym(scummvm_so, "JNI_OnLoad");
	if (!scummvm_onLoad) {
		dlclose(scummvm_so);
		return JNI_ERR;
	}

	dirty = false;

	JNIEnv *env;

	if (vm->GetEnv((void **)&env, JNI_VERSION_1_2))
		return JNI_ERR;

	jclass cls = env->FindClass("org/scummvm/scummvm/ScummVM$Reloader");
	if (cls == 0)
		return JNI_ERR;

	if (env->RegisterNatives(cls, reloader_natives, sizeof(reloader_natives) / sizeof(*reloader_natives)) < 0)
		return JNI_ERR;

	env->DeleteLocalRef(cls);

	return scummvm_onLoad(vm, reserved);
}

static jboolean scummvm_reload(JNIEnv *env, jclass clazz) {
	if (!dirty) {
		// Ignore first reload request: we loaded a fresh instance at startup
		dirty = true;
		return true;
	}

	if (scummvm_so) {
		dlclose(scummvm_so);
	}

	scummvm_so = dlopen("libscummvm.so", RTLD_NOW | RTLD_LOCAL);
	if (!scummvm_so) {
		return false;
	}

	scummvm_onLoad = (onLoadFunction)dlsym(scummvm_so, "JNI_OnLoad");
	if (!scummvm_onLoad) {
		dlclose(scummvm_so);
		return false;
	}

	return scummvm_onLoad(vm, reserved) != JNI_ERR;
}
