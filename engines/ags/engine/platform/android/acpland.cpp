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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/shared/core/platform.h"

#if AGS_PLATFORM_OS_ANDROID

#include "ags/lib/allegro.h"
#include "ags/shared/platform/base/agsplatformdriver.h"
#include "ags/shared/ac/runtime_defines.h"
#include "ags/shared/main/config.h"
#include "ags/shared/plugin/agsplugin.h"
//include <stdio.h>
//include <dirent.h>
//include <sys/stat.h>
//include <ctype.h>
//include <unistd.h>
#include "ags/shared/util/string_compat.h"



//include <jni.h>
//include <android/log.h>

namespace AGS3 {

using namespace AGS::Shared;

#define ANDROID_CONFIG_FILENAME "android.cfg"

bool ReadConfiguration(char *filename, bool read_everything);
void ResetConfiguration();

struct AGSAndroid : AGSPlatformDriver {

	virtual int  CDPlayerCommand(int cmdd, int datt);
	virtual void Delay(int millis);
	virtual void DisplayAlert(const char *, ...);
	virtual const char *GetAppOutputDirectory();
	virtual unsigned long GetDiskFreeSpaceMB();
	virtual const char *GetNoMouseErrorString();
	virtual bool IsBackendResponsibleForMouseScaling() {
		return true;
	}
	virtual eScriptSystemOSID GetSystemOSID();
	virtual int  InitializeCDPlayer();
	virtual void PostAllegroExit();
	virtual void SetGameWindowIcon();
	virtual void ShutdownCDPlayer();
	virtual void WriteStdOut(const char *fmt, ...);
	virtual void WriteStdErr(const char *fmt, ...);
};


//int psp_return_to_menu = 1;
int psp_ignore_acsetup_cfg_file = 1;
int psp_clear_cache_on_room_change = 0;
int psp_rotation = 0;
int psp_config_enabled = 0;
char psp_translation[100];
char *psp_translations[100];

// Mouse option from Allegro.
extern int config_mouse_control_mode;


// Graphic options from the Allegro library.
extern int psp_gfx_scaling;
extern int psp_gfx_smoothing;


// Audio options from the Allegro library.
unsigned int psp_audio_samplerate = 44100;
int psp_audio_enabled = 1;
volatile int psp_audio_multithreaded = 1;
int psp_audio_cachesize = 10;
int psp_midi_enabled = 1;
int psp_midi_preload_patches = 0;

int psp_video_framedrop = 0;

int psp_gfx_renderer = 0;
int psp_gfx_super_sampling = 0;
int psp_gfx_smooth_sprites = 0;

int psp_debug_write_to_logcat = 0;

int config_mouse_longclick = 0;

extern int display_fps;
extern int want_exit;
extern void PauseGame();
extern void UnPauseGame();
extern int main(int argc, char *argv[]);

char android_base_directory[256];
char android_app_directory[256];
char psp_game_file_name[256];
char *psp_game_file_name_pointer = psp_game_file_name;

bool psp_load_latest_savegame = false;
extern char saveGameDirectory[260];
extern const char *loadSaveGameOnStartup;
char lastSaveGameName[200];


extern JavaVM *android_jni_vm;
JNIEnv *java_environment;
jobject java_object;
jclass java_class;
jmethodID java_messageCallback;
jmethodID java_blockExecution;
jmethodID java_swapBuffers;
jmethodID java_setRotation;
jmethodID java_enableLongclick;

bool reset_configuration = false;

extern "C"
{

	const int CONFIG_IGNORE_ACSETUP = 0;
	const int CONFIG_CLEAR_CACHE = 1;
	const int CONFIG_AUDIO_RATE = 2;
	const int CONFIG_AUDIO_ENABLED = 3;
	const int CONFIG_AUDIO_THREADED = 4;
	const int CONFIG_AUDIO_CACHESIZE = 5;
	const int CONFIG_MIDI_ENABLED = 6;
	const int CONFIG_MIDI_PRELOAD = 7;
	const int CONFIG_VIDEO_FRAMEDROP = 8;
	const int CONFIG_GFX_RENDERER = 9;
	const int CONFIG_GFX_SMOOTHING = 10;
	const int CONFIG_GFX_SCALING = 11;
	const int CONFIG_GFX_SS = 12;
	const int CONFIG_ROTATION = 13;
	const int CONFIG_ENABLED = 14;
	const int CONFIG_DEBUG_FPS = 15;
	const int CONFIG_GFX_SMOOTH_SPRITES = 16;
	const int CONFIG_TRANSLATION = 17;
	const int CONFIG_DEBUG_LOGCAT = 18;
	const int CONFIG_MOUSE_METHOD = 19;
	const int CONFIG_MOUSE_LONGCLICK = 20;

	extern void android_debug_printf(const char *format, ...);

	JNIEXPORT jboolean JNICALL
	Java_com_bigbluecup_android_PreferencesActivity_readConfigFile(JNIEnv *env, jobject object, jstring directory) {
		const char *cdirectory = env->GetStringUTFChars(directory, NULL);
		chdir(cdirectory);
		env->ReleaseStringUTFChars(directory, cdirectory);

		ResetConfiguration();

		return ReadConfiguration(ANDROID_CONFIG_FILENAME, true);
	}


	JNIEXPORT jboolean JNICALL
	Java_com_bigbluecup_android_PreferencesActivity_writeConfigFile(JNIEnv *env, jobject object) {
		FILE *config = fopen(ANDROID_CONFIG_FILENAME, "wb");
		if (config) {
			fprintf(config, "[misc]\n");
			fprintf(config, "config_enabled = %d\n", psp_config_enabled);
			fprintf(config, "rotation = %d\n", psp_rotation);
			fprintf(config, "translation = %s\n", psp_translation);

			fprintf(config, "[controls]\n");
			fprintf(config, "mouse_method = %d\n", config_mouse_control_mode);
			fprintf(config, "mouse_longclick = %d\n", config_mouse_longclick);

			fprintf(config, "[compatibility]\n");
//    fprintf(config, "ignore_acsetup_cfg_file = %d\n", psp_ignore_acsetup_cfg_file);
			fprintf(config, "clear_cache_on_room_change = %d\n", psp_clear_cache_on_room_change);

			fprintf(config, "[sound]\n");
			fprintf(config, "samplerate = %d\n", psp_audio_samplerate);
			fprintf(config, "enabled = %d\n", psp_audio_enabled);
			fprintf(config, "threaded = %d\n", psp_audio_multithreaded);
			fprintf(config, "cache_size = %d\n", psp_audio_cachesize);

			fprintf(config, "[midi]\n");
			fprintf(config, "enabled = %d\n", psp_midi_enabled);
			fprintf(config, "preload_patches = %d\n", psp_midi_preload_patches);

			fprintf(config, "[video]\n");
			fprintf(config, "framedrop = %d\n", psp_video_framedrop);

			fprintf(config, "[graphics]\n");
			fprintf(config, "renderer = %d\n", psp_gfx_renderer);
			fprintf(config, "smoothing = %d\n", psp_gfx_smoothing);
			fprintf(config, "scaling = %d\n", psp_gfx_scaling);
			fprintf(config, "super_sampling = %d\n", psp_gfx_super_sampling);
			fprintf(config, "smooth_sprites = %d\n", psp_gfx_smooth_sprites);

			fprintf(config, "[debug]\n");
			fprintf(config, "show_fps = %d\n", (display_fps == 2) ? 1 : 0);
			fprintf(config, "logging = %d\n", psp_debug_write_to_logcat);

			fclose(config);

			return true;
		}

		return false;
	}


	JNIEXPORT jint JNICALL
	Java_com_bigbluecup_android_PreferencesActivity_readIntConfigValue(JNIEnv *env, jobject object, jint id) {
		switch (id) {
		case CONFIG_IGNORE_ACSETUP:
			return psp_ignore_acsetup_cfg_file;
			break;
		case CONFIG_CLEAR_CACHE:
			return psp_clear_cache_on_room_change;
			break;
		case CONFIG_AUDIO_RATE:
			return psp_audio_samplerate;
			break;
		case CONFIG_AUDIO_ENABLED:
			return psp_audio_enabled;
			break;
		case CONFIG_AUDIO_THREADED:
			return psp_audio_multithreaded;
			break;
		case CONFIG_AUDIO_CACHESIZE:
			return psp_audio_cachesize;
			break;
		case CONFIG_MIDI_ENABLED:
			return psp_midi_enabled;
			break;
		case CONFIG_MIDI_PRELOAD:
			return psp_midi_preload_patches;
			break;
		case CONFIG_VIDEO_FRAMEDROP:
			return psp_video_framedrop;
			break;
		case CONFIG_GFX_RENDERER:
			return psp_gfx_renderer;
			break;
		case CONFIG_GFX_SMOOTHING:
			return psp_gfx_smoothing;
			break;
		case CONFIG_GFX_SCALING:
			return psp_gfx_scaling;
			break;
		case CONFIG_GFX_SS:
			return psp_gfx_super_sampling;
			break;
		case CONFIG_GFX_SMOOTH_SPRITES:
			return psp_gfx_smooth_sprites;
			break;
		case CONFIG_ROTATION:
			return psp_rotation;
			break;
		case CONFIG_ENABLED:
			return psp_config_enabled;
			break;
		case CONFIG_DEBUG_FPS:
			return (display_fps == 2) ? 1 : 0;
			break;
		case CONFIG_DEBUG_LOGCAT:
			return psp_debug_write_to_logcat;
			break;
		case CONFIG_MOUSE_METHOD:
			return config_mouse_control_mode;
			break;
		case CONFIG_MOUSE_LONGCLICK:
			return config_mouse_longclick;
			break;
		default:
			return 0;
			break;
		}
	}


	JNIEXPORT jstring JNICALL
	Java_com_bigbluecup_android_PreferencesActivity_readStringConfigValue(JNIEnv *env, jobject object, jint id, jstring value) {
		switch (id) {
		case CONFIG_TRANSLATION:
			return env->NewStringUTF(&psp_translation[0]);
			break;
		}
	}


	JNIEXPORT void JNICALL
	Java_com_bigbluecup_android_PreferencesActivity_setIntConfigValue(JNIEnv *env, jobject object, jint id, jint value) {
		switch (id) {
		case CONFIG_IGNORE_ACSETUP:
			psp_ignore_acsetup_cfg_file = value;
			break;
		case CONFIG_CLEAR_CACHE:
			psp_clear_cache_on_room_change = value;
			break;
		case CONFIG_AUDIO_RATE:
			psp_audio_samplerate = value;
			break;
		case CONFIG_AUDIO_ENABLED:
			psp_audio_enabled = value;
			break;
		case CONFIG_AUDIO_THREADED:
			psp_audio_multithreaded = value;
			break;
		case CONFIG_AUDIO_CACHESIZE:
			psp_audio_cachesize = value;
			break;
		case CONFIG_MIDI_ENABLED:
			psp_midi_enabled = value;
			break;
		case CONFIG_MIDI_PRELOAD:
			psp_midi_preload_patches = value;
			break;
		case CONFIG_VIDEO_FRAMEDROP:
			psp_video_framedrop = value;
			break;
		case CONFIG_GFX_RENDERER:
			psp_gfx_renderer = value;
			break;
		case CONFIG_GFX_SMOOTHING:
			psp_gfx_smoothing = value;
			break;
		case CONFIG_GFX_SCALING:
			psp_gfx_scaling = value;
			break;
		case CONFIG_GFX_SS:
			psp_gfx_super_sampling = value;
			break;
		case CONFIG_GFX_SMOOTH_SPRITES:
			psp_gfx_smooth_sprites = value;
			break;
		case CONFIG_ROTATION:
			psp_rotation = value;
			break;
		case CONFIG_ENABLED:
			psp_config_enabled = value;
			break;
		case CONFIG_DEBUG_FPS:
			display_fps = (value == 1) ? 2 : 0;
			break;
		case CONFIG_DEBUG_LOGCAT:
			psp_debug_write_to_logcat = value;
			break;
		case CONFIG_MOUSE_METHOD:
			config_mouse_control_mode = value;
			break;
		case CONFIG_MOUSE_LONGCLICK:
			config_mouse_longclick = value;
			break;
		default:
			break;
		}
	}


	JNIEXPORT void JNICALL
	Java_com_bigbluecup_android_PreferencesActivity_setStringConfigValue(JNIEnv *env, jobject object, jint id, jstring value) {
		const char *cstring = env->GetStringUTFChars(value, NULL);

		switch (id) {
		case CONFIG_TRANSLATION:
			strcpy(psp_translation, cstring);
			break;
		default:
			break;
		}

		env->ReleaseStringUTFChars(value, cstring);
	}


	JNIEXPORT jint JNICALL
	Java_com_bigbluecup_android_PreferencesActivity_getAvailableTranslations(JNIEnv *env, jobject object, jobjectArray translations) {
		int i = 0;
		int length;
		DIR *dir;
		struct dirent *entry;
		char buffer[200];

		dir = opendir(".");
		if (dir) {
			while ((entry = readdir(dir)) != 0) {
				length = strlen(entry->d_name);
				if (length > 4) {
					if (ags_stricmp(&entry->d_name[length - 4], ".tra") == 0) {
						memset(buffer, 0, 200);
						strncpy(buffer, entry->d_name, length - 4);
						psp_translations[i] = (char *)malloc(strlen(buffer) + 1);
						strcpy(psp_translations[i], buffer);
						env->SetObjectArrayElement(translations, i, env->NewStringUTF(&buffer[0]));
						i++;
					}
				}
			}
			closedir(dir);
		}

		return i;
	}


	JNIEXPORT void JNICALL
	Java_com_bigbluecup_android_EngineGlue_pauseEngine(JNIEnv *env, jobject object) {
		PauseGame();
	}

	JNIEXPORT void JNICALL
	Java_com_bigbluecup_android_EngineGlue_resumeEngine(JNIEnv *env, jobject object) {
		UnPauseGame();
	}


	JNIEXPORT void JNICALL
	Java_com_bigbluecup_android_EngineGlue_shutdownEngine(JNIEnv *env, jobject object) {
		want_exit = 1;
	}


	JNIEXPORT jboolean JNICALL
	Java_com_bigbluecup_android_EngineGlue_startEngine(JNIEnv *env, jobject object, jclass stringclass, jstring filename, jstring directory, jstring appDirectory, jboolean loadLastSave) {
		// Get JNI interfaces.
		java_object = env->NewGlobalRef(object);
		java_environment = env;
		java_class = (jclass)java_environment->NewGlobalRef(java_environment->GetObjectClass(object));
		java_messageCallback = java_environment->GetMethodID(java_class, "showMessage", "(Ljava/lang/String;)V");
		java_blockExecution = java_environment->GetMethodID(java_class, "blockExecution", "()V");
		java_setRotation = java_environment->GetMethodID(java_class, "setRotation", "(I)V");
		java_enableLongclick = java_environment->GetMethodID(java_class, "enableLongclick", "()V");

		// Initialize JNI for Allegro.
		android_allegro_initialize_jni(java_environment, java_class, java_object);

		// Get the file to run from Java.
		const char *cpath = java_environment->GetStringUTFChars(filename, NULL);
		strcpy(psp_game_file_name, cpath);
		java_environment->ReleaseStringUTFChars(filename, cpath);

		// Get the base directory (usually "/sdcard/ags").
		const char *cdirectory = java_environment->GetStringUTFChars(directory, NULL);
		chdir(cdirectory);
		strcpy(android_base_directory, cdirectory);
		java_environment->ReleaseStringUTFChars(directory, cdirectory);

		// Get the app directory (something like "/data/data/com.bigbluecup.android.launcher")
		const char *cappDirectory = java_environment->GetStringUTFChars(appDirectory, NULL);
		strcpy(android_app_directory, cappDirectory);
		java_environment->ReleaseStringUTFChars(appDirectory, cappDirectory);

		// Reset configuration.
		ResetConfiguration();

		// Read general configuration.
		ReadConfiguration(ANDROID_CONFIG_FILENAME, true);

		// Get the games path.
		char path[256];
		strcpy(path, psp_game_file_name);
		int lastindex = strlen(path) - 1;
		while (path[lastindex] != '/') {
			path[lastindex] = 0;
			lastindex--;
		}
		chdir(path);

		setenv("ULTRADIR", "..", 1);

		// Read game specific configuration.
		ReadConfiguration(ANDROID_CONFIG_FILENAME, false);

		// Set the screen rotation.
		if (psp_rotation > 0)
			java_environment->CallVoidMethod(java_object, java_setRotation, psp_rotation);

		if (config_mouse_longclick > 0)
			java_environment->CallVoidMethod(java_object, java_enableLongclick);

		psp_load_latest_savegame = loadLastSave;

		// Start the engine main function.
		main(1, &psp_game_file_name_pointer);

		// Explicitly quit here, otherwise the app will hang forever.
		exit(0);

		return true;
	}


	void selectLatestSavegame() {
		DIR *dir;
		struct dirent *entry;
		struct stat statBuffer;
		char buffer[200];
		time_t lastTime = 0;

		dir = opendir(saveGameDirectory);

		if (dir) {
			while ((entry = readdir(dir)) != 0) {
				if (ags_strnicmp(entry->d_name, "agssave", 7) == 0) {
					if (ags_stricmp(entry->d_name, "agssave.999") != 0) {
						strcpy(buffer, saveGameDirectory);
						strcat(buffer, entry->d_name);
						stat(buffer, &statBuffer);
						if (statBuffer.st_mtime > lastTime) {
							strcpy(lastSaveGameName, buffer);
							loadSaveGameOnStartup = lastSaveGameName;
							lastTime = statBuffer.st_mtime;
						}
					}
				}
			}
			closedir(dir);
		}
	}

}


int ReadInteger(int *variable, const ConfigTree &cfg, char *section, char *name, int minimum, int maximum, int default_value) {
	if (reset_configuration) {
		*variable = default_value;
		return 0;
	}

	int temp = INIreadint(cfg, section, name);

	if (temp == -1)
		return 0;

	if ((temp < minimum) || (temp > maximum))
		temp = default_value;

	*variable = temp;

	return 1;
}



int ReadString(char *variable, const ConfigTree &cfg, char *section, char *name, char *default_value) {
	if (reset_configuration) {
		strcpy(variable, default_value);
		return 0;
	}

	String temp;
	if (!INIreaditem(cfg, section, name, temp))
		temp = default_value;

	strcpy(variable, temp);

	return 1;
}



void ResetConfiguration() {
	reset_configuration = true;

	ReadConfiguration(ANDROID_CONFIG_FILENAME, true);

	reset_configuration = false;
}



bool ReadConfiguration(char *filename, bool read_everything) {
	ConfigTree cfg;
	if (IniUtil::Read(filename, cfg) || reset_configuration) {
//    ReadInteger((int*)&psp_disable_powersaving, "misc", "disable_power_saving", 0, 1, 1);

//    ReadInteger((int*)&psp_return_to_menu, "misc", "return_to_menu", 0, 1, 1);

		ReadString(&psp_translation[0], cfg, "misc", "translation", "default");

		ReadInteger((int *)&psp_config_enabled, cfg, "misc", "config_enabled", 0, 1, 0);
		if (!psp_config_enabled && !read_everything)
			return true;

		ReadInteger(&psp_debug_write_to_logcat, cfg, "debug", "logging", 0, 1, 0);
		ReadInteger(&display_fps, cfg, "debug", "show_fps", 0, 1, 0);
		if (display_fps == 1)
			display_fps = 2;

		ReadInteger((int *)&psp_rotation, cfg, "misc", "rotation", 0, 2, 0);

//    ReadInteger((int*)&psp_ignore_acsetup_cfg_file, "compatibility", "ignore_acsetup_cfg_file", 0, 1, 0);
		ReadInteger((int *)&psp_clear_cache_on_room_change, cfg, "compatibility", "clear_cache_on_room_change", 0, 1, 0);

		ReadInteger((int *)&psp_audio_samplerate, cfg, "sound", "samplerate", 0, 44100, 44100);
		ReadInteger((int *)&psp_audio_enabled, cfg, "sound", "enabled", 0, 1, 1);
		ReadInteger((int *)&psp_audio_multithreaded, cfg, "sound", "threaded", 0, 1, 1);
		ReadInteger((int *)&psp_audio_cachesize, cfg, "sound", "cache_size", 1, 50, 10);

		ReadInteger((int *)&psp_midi_enabled, cfg, "midi", "enabled", 0, 1, 1);
		ReadInteger((int *)&psp_midi_preload_patches, cfg, "midi", "preload_patches", 0, 1, 0);

		ReadInteger((int *)&psp_video_framedrop, cfg, "video", "framedrop", 0, 1, 0);

		ReadInteger((int *)&psp_gfx_renderer, cfg, "graphics", "renderer", 0, 2, 0);
		ReadInteger((int *)&psp_gfx_smoothing, cfg, "graphics", "smoothing", 0, 1, 1);
		ReadInteger((int *)&psp_gfx_scaling, cfg, "graphics", "scaling", 0, 2, 1);
		ReadInteger((int *)&psp_gfx_super_sampling, cfg, "graphics", "super_sampling", 0, 1, 0);
		ReadInteger((int *)&psp_gfx_smooth_sprites, cfg, "graphics", "smooth_sprites", 0, 1, 0);

		ReadInteger((int *)&config_mouse_control_mode, cfg, "controls", "mouse_method", 0, 1, 0);
		ReadInteger((int *)&config_mouse_longclick, cfg, "controls", "mouse_longclick", 0, 1, 1);

		return true;
	}

	return false;
}



int AGSAndroid::CDPlayerCommand(int cmdd, int datt) {
	return 1;//cd_player_control(cmdd, datt);
}

void AGSAndroid::DisplayAlert(const char *text, ...) {
	char displbuf[2000];
	va_list ap;
	va_start(ap, text);
	vsprintf(displbuf, text, ap);
	va_end(ap);

	// It is possible that this is called from a thread that is not yet known
	// to the Java VM. So attach it first before displaying the message.
	JNIEnv *thread_env;
	android_jni_vm->AttachCurrentThread(&thread_env, NULL);

	__android_log_print(ANDROID_LOG_DEBUG, "AGSNative", "%s", displbuf);

	jstring java_string = thread_env->NewStringUTF(displbuf);
	thread_env->CallVoidMethod(java_object, java_messageCallback, java_string);
	usleep(1000 * 1000);
	thread_env->CallVoidMethod(java_object, java_blockExecution);

//  android_jni_vm->DetachCurrentThread();
}

void AGSAndroid::Delay(int millis) {
	usleep(millis * 1000);
}

unsigned long AGSAndroid::GetDiskFreeSpaceMB() {
	// placeholder
	return 100;
}

const char *AGSAndroid::GetNoMouseErrorString() {
	return "This game requires a mouse. You need to configure and setup your mouse to play this game.\n";
}

eScriptSystemOSID AGSAndroid::GetSystemOSID() {
	return eOS_Android;
}

int AGSAndroid::InitializeCDPlayer() {
	return 1;//cd_player_init();
}

void AGSAndroid::PostAllegroExit() {
	java_environment->DeleteGlobalRef(java_class);
}

void AGSAndroid::SetGameWindowIcon() {
	// do nothing
}

void AGSAndroid::WriteStdOut(const char *fmt, ...) {
	// TODO: this check should probably be done once when setting up output targets for logging
	if (psp_debug_write_to_logcat) {
		va_list args;
		va_start(args, fmt);
		__android_log_vprint(ANDROID_LOG_DEBUG, "AGSNative", fmt, args);
		// NOTE: __android_log_* functions add trailing '\n'
		va_end(args);
	}
}

void AGSAndroid::WriteStdErr(const char *fmt, ...) {
	// TODO: find out if Android needs separate implementation for stderr
	if (psp_debug_write_to_logcat) {
		va_list args;
		va_start(args, fmt);
		__android_log_vprint(ANDROID_LOG_DEBUG, "AGSNative", fmt, args);
		// NOTE: __android_log_* functions add trailing '\n'
		va_end(args);
	}
}

void AGSAndroid::ShutdownCDPlayer() {
	//cd_exit();
}

const char *AGSAndroid::GetAppOutputDirectory() {
	return android_base_directory;
}

AGSPlatformDriver *AGSPlatformDriver::GetDriver() {
	if (instance == NULL)
		instance = new AGSAndroid();

	return instance;
}

} // namespace AGS3

#endif
