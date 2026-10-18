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
#include <stdlib.h>
#include <string.h>

#include <jni.h>
#include <dlfcn.h>

#include <android/log.h>

static const char *android_log_tag = "ScummVM";

#define _ANDROID_LOG(prio, fmt, args...) __android_log_print(prio, android_log_tag, fmt, ## args)
#define LOGD(fmt, args...) _ANDROID_LOG(ANDROID_LOG_DEBUG, fmt, ##args)
#define LOGI(fmt, args...) _ANDROID_LOG(ANDROID_LOG_INFO, fmt, ##args)
#define LOGW(fmt, args...) _ANDROID_LOG(ANDROID_LOG_WARN, fmt, ##args)
#define LOGE(fmt, args...) _ANDROID_LOG(ANDROID_LOG_ERROR, fmt, ##args)

typedef jint JNICALL (*onLoadFunction)(JavaVM *vm, void *);
typedef void JNICALL (*onUnloadFunction)(JavaVM *vm, void *);

static JavaVM *vm;
static void *reserved;

static void *scummvm_so;
static onLoadFunction scummvm_onLoad;
static onUnloadFunction scummvm_onUnload;
static bool dirty;

static jboolean scummvm_reload(JNIEnv *env, jclass clazz);

static const JNINativeMethod reloader_natives[] = {
	{ "reload", "()Z", (void *)scummvm_reload },
};

static const char *scummvm_so_path = "libscummvm.so";
static char *full_scummvm_so_path;

char *parse_maps_line(char *line, size_t sz) {
	char *end, *p;

	// Start address
	p = (char *)memchr(line, '-', sz);
	if (p == nullptr) {
		return nullptr;
	}
	*p = '\0';
	unsigned long int start_addr = strtoul(line, &end, 16);
	if (end != p) {
		return nullptr;
	}
	p++;

	sz -= p - line;
	line = p;

	// End address
	p = (char *)memchr(line, ' ', sz);
	if (p == nullptr) {
		return nullptr;
	}
	*p = '\0';
	unsigned long int end_addr = strtoul(line, &end, 16);
	if (end != p) {
		return nullptr;
	}
	p++;

	if (!((intptr_t)start_addr <= (intptr_t)parse_maps_line && (intptr_t)parse_maps_line < (intptr_t)end_addr)) {
		// Not our library
		return nullptr;
	}

	sz -= p - line;
	line = p;

	// Flags, pgoff, dev, inode
	for(int count = 4; count > 0; count--) {
		sz -= p - line;
		line = p;

		p = (char *)memchr(line, ' ', sz);
		if (p == nullptr) {
			return nullptr;
		}
		p++;
	}
	if (sz == 0) {
		return nullptr;
	}

	// padding
	for(; sz > 0 && *p == ' '; p++, sz--)
		;
	if (sz == 0) {
		return nullptr;
	}

	line = p;

	// file path
	for (p = line + sz - 1; p > line && *p != '/'; p--)
		;
	if (p == line) {
		return nullptr;
	}

	*p = '\0';

	int len = snprintf(nullptr, 0, "%s/%s", line, scummvm_so_path);
	if (len < 0) {
		return nullptr;
	}
	len++;

	char *path = (char *)malloc(len);
	if (!path) {
		return nullptr;
	}

	len = snprintf(path, len, "%s/%s", line, scummvm_so_path);
	if (len < 0) {
		free(path);
		return nullptr;
	}

	return path;
}

char *find_lib_path() {
	FILE *maps = fopen("/proc/self/maps", "r");
	if (!maps) {
		return nullptr;
	}

	size_t lineBufferSize = 256;
	char *lineBuffer = (char *)malloc(lineBufferSize * sizeof(*lineBuffer));
	if (!lineBuffer) {
		fclose(maps);
		return nullptr;
	}
	char *lineWPtr = lineBuffer;

	while (1) {
		size_t readLength = fread(lineWPtr, sizeof(*lineWPtr), lineBufferSize - (lineWPtr - lineBuffer), maps);
		lineWPtr += readLength;
		char *nlPtr = (char *)memchr(lineBuffer, '\n', lineWPtr - lineBuffer);

		if (nlPtr == nullptr) {
			if (lineBufferSize - (lineWPtr - lineBuffer) != 0) {
				break; // We still have room: we went till end of stream
			}

			char *newlineBuffer = (char *)realloc(lineBuffer, lineBufferSize * 2);
			if (!newlineBuffer) {
				free(lineBuffer);
				fclose(maps);
				return nullptr;
			}
			lineBuffer = newlineBuffer;
			lineWPtr = lineBuffer + lineBufferSize;
			lineBufferSize *= 2;

			// Let's read new content in our larger buffer
			continue;
		}

		char *lib_path;
		if ((lib_path = parse_maps_line(lineBuffer, nlPtr - lineBuffer))) {
			free(lineBuffer);
			fclose(maps);
			return lib_path;
		}

		nlPtr++;
		memmove(lineBuffer, nlPtr, lineWPtr - nlPtr);
		lineWPtr -= nlPtr - lineBuffer;
	}

	free(lineBuffer);
	fclose(maps);
	return nullptr;
}

__attribute__ ((visibility("default")))
jint JNICALL JNI_OnLoad(JavaVM *vm_, void *reserved_) {
	vm = vm_;
	reserved = reserved_;

	scummvm_so = dlopen(scummvm_so_path, RTLD_NOW | RTLD_LOCAL);
	if (!scummvm_so) {
		LOGW("Failed to load libscummvm.so: %s", dlerror());
		full_scummvm_so_path = find_lib_path();
		if (full_scummvm_so_path) {
			scummvm_so = dlopen(full_scummvm_so_path, RTLD_NOW | RTLD_LOCAL);
		}

	}
	if (!scummvm_so) {
		__android_log_assert(nullptr, android_log_tag, "Failed to load libscummvm.so or %s: %s", full_scummvm_so_path, dlerror());
		return JNI_ERR;
	}
	scummvm_onLoad = (onLoadFunction)dlsym(scummvm_so, "JNI_OnLoad");
	if (!scummvm_onLoad) {
		LOGD("Didn't find JNI_OnLoad");
	}
	scummvm_onUnload = (onUnloadFunction)dlsym(scummvm_so, "JNI_OnUnload");
	if (!scummvm_onUnload) {
		LOGD("Didn't find JNI_OnUnload");
	}

	dirty = false;

	JNIEnv *env;

	if (vm->GetEnv((void **)&env, JNI_VERSION_1_2)) {
		LOGE("Failed to GetEnv");
		return JNI_ERR;
	}

	jclass cls = env->FindClass("org/scummvm/scummvm/ScummVM$Reloader");
	if (cls == 0) {
		return JNI_ERR;
	}

	if (env->RegisterNatives(cls, reloader_natives, sizeof(reloader_natives) / sizeof(*reloader_natives)) < 0) {
		return JNI_ERR;
	}

	env->DeleteLocalRef(cls);

	if (scummvm_onLoad) {
		LOGD("Running ScummVM onLoad");
		return scummvm_onLoad(vm, reserved);
	} else {
		// JNI_OnLoad is optional
		return JNI_VERSION_1_2;
	}
}

__attribute__ ((visibility("default")))
void JNICALL JNI_OnUnload(JavaVM *vm_, void *reserved_) {
	if (!scummvm_so) {
		return;
	}
	if (scummvm_onUnload) {
		LOGD("Running ScummVM onUnload");
		scummvm_onUnload(vm_, reserved_);
	}
	dlclose(scummvm_so);
	free(full_scummvm_so_path);
}

static jboolean scummvm_reload(JNIEnv *env, jclass clazz) {
	if (!dirty) {
		// Ignore first reload request: we loaded a fresh instance at startup
		dirty = true;
		return true;
	}

	LOGI("Reloading ScummVM");
	if (scummvm_so) {
		if (scummvm_onUnload) {
			LOGD("Running ScummVM onUnload");
			scummvm_onUnload(vm, reserved);
		}
		dlclose(scummvm_so);
	}

	scummvm_so = dlopen(full_scummvm_so_path ? full_scummvm_so_path : scummvm_so_path, RTLD_NOW | RTLD_LOCAL);
	if (!scummvm_so) {
		__android_log_assert(nullptr, android_log_tag, "Failed to load libscummvm.so or %s: %s", full_scummvm_so_path, dlerror());
		return false;
	}

	scummvm_onLoad = (onLoadFunction)dlsym(scummvm_so, "JNI_OnLoad");
	scummvm_onUnload = (onUnloadFunction)dlsym(scummvm_so, "JNI_OnUnload");

	if (!scummvm_onLoad) {
		return true;
	}
	LOGD("Running ScummVM onLoad");
	return scummvm_onLoad(vm, reserved) != JNI_ERR;
}
