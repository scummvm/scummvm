/* Copyright (C) 2023 Giovanni Cascione <ing.cascione@gmail.com>
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
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#if defined(_WIN32)
#include "backends/fs/windows/windows-fs-factory.h"
#define FS_SYSTEM_FACTORY WindowsFilesystemFactory
#else
#include "backends/platform/libretro/include/libretro-fs-factory.h"
#define FS_SYSTEM_FACTORY LibRetroFilesystemFactory
#endif

#include <features/features_cpu.h>

#include "audio/mixer_intern.h"
#include "backends/base-backend.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "backends/saves/default/default-saves.h"
#include "backends/platform/libretro/include/libretro-defs.h"
#include "backends/platform/libretro/include/libretro-core.h"
#include "backends/platform/libretro/include/libretro-timer.h"
#include "backends/platform/libretro/include/libretro-os.h"
#include "backends/platform/libretro/include/libretro-fs.h"

OSystem_libretro::OSystem_libretro() : _mousePaletteEnabled(false), _mouseVisible(false), _mouseX(0), _mouseY(0), _mouseXAcc(0.0), _mouseYAcc(0.0), _mouseHotspotX(0), _mouseHotspotY(0), _dpadXAcc(0.0), _dpadYAcc(0.0), _dpadXVel(0.0f), _dpadYVel(0.0f), _mouseKeyColor(0), _mouseDontScale(false), _mixer(0), _startTime(0), _threadSwitchCaller(0), _cursorStatus(0) {
	_fsFactory = new FS_SYSTEM_FACTORY();

	setLibretroDir(retro_get_system_dir(), s_systemDir);
	setLibretroDir(retro_get_save_dir(), s_saveDir);
	setLibretroDir(retro_get_playlist_dir(), s_playlistDir);

	memset(_mouseButtons, 0, sizeof(_mouseButtons));

	_startTime = (uint32)(cpu_features_get_time_usec() / 1000);
}

OSystem_libretro::~OSystem_libretro() {
	_gameScreen.free();
	_overlay.free();
	_mouseImage.free();
	_screen.free();

	delete _mixer;
}

void OSystem_libretro::initBackend() {
	Common::String s_homeDir(LibRetroFilesystemNode::getHomeDir());
	Common::String s_themeDir(s_systemDir + "/" + SCUMMVM_SYSTEM_SUBDIR + "/" + SCUMMVM_THEME_SUBDIR);
	Common::String s_extraDir(s_systemDir + "/" + SCUMMVM_SYSTEM_SUBDIR + "/" + SCUMMVM_EXTRA_SUBDIR);
	Common::String s_soundfontPath(s_extraDir + "/" + DEFAULT_SOUNDFONT_FILENAME);

	if (! LibRetroFilesystemNode(s_themeDir).isDirectory())
		s_themeDir.clear();
	if (! LibRetroFilesystemNode(s_extraDir).isDirectory())
		s_extraDir.clear();
	if (! LibRetroFilesystemNode(s_soundfontPath).exists())
		s_soundfontPath.clear();
	if ((s_homeDir.empty() || ! LibRetroFilesystemNode(s_homeDir).isDirectory()) && ! s_systemDir.empty())
		s_homeDir = s_systemDir;

	//Register default paths
	if (! s_homeDir.empty()) {
		ConfMan.registerDefault("browser_lastpath", s_homeDir);
		retro_log_cb(RETRO_LOG_DEBUG, "Default browser last path set to: %s\n", s_homeDir.c_str());
	}
	if (! s_saveDir.empty()) {
		ConfMan.registerDefault("savepath", s_saveDir);
		retro_log_cb(RETRO_LOG_DEBUG, "Default save path set to: %s\n", s_saveDir.c_str());
	}

	//Check current path settings
	if (!checkPathSetting("savepath", s_saveDir))
		retro_osd_notification("ScummVM save folder not found.");
	if (!checkPathSetting("themepath", s_themeDir))
		retro_osd_notification("ScummVM theme folder not found.");
	if (!checkPathSetting("extrapath", s_extraDir))
		retro_osd_notification("ScummVM extra folder not found. Some engines/features (e.g. Virtual Keyboard) will not work without relevant datafiles.");
	checkPathSetting("soundfont", s_soundfontPath, false);
	checkPathSetting("browser_lastpath", s_homeDir);
	checkPathSetting("libretro_playlist_path", s_playlistDir.empty() ? s_homeDir : s_playlistDir);

	//Check other settings
	if (! ConfMan.hasKey("libretro_playlist_version"))
		ConfMan.set("libretro_playlist_version", 0);

	if (! ConfMan.hasKey("libretro_hooks_location"))
		ConfMan.set("libretro_hooks_location", 0);

	if (! ConfMan.hasKey("libretro_hooks_clear"))
		ConfMan.set("libretro_hooks_clear", 0);

	_savefileManager = new DefaultSaveFileManager();

#ifdef FRONTEND_SUPPORTS_RGB565
	_overlay.create(RES_W_OVERLAY, RES_H_OVERLAY, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
#else
	_overlay.create(RES_W_OVERLAY, RES_H_OVERLAY, Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 15));
#endif
	_mixer = new Audio::MixerImpl(retro_setting_get_sample_rate());
	retro_log_cb(RETRO_LOG_DEBUG, "Mixer set up at %dHz\n", retro_setting_get_sample_rate());

	_timerManager = new LibretroTimerManager(retro_setting_get_frame_rate());

	_mixer->setReady(true);

	EventsBaseBackend::initBackend();

	refreshRetroSettings();
}

void OSystem_libretro::engineInit() {
	Common::String engineId = ConfMan.get("engineid");
	if (engineId.equalsIgnoreCase("scumm") && ConfMan.getBool("original_gui")) {
		ConfMan.setBool("original_gui", false);
		retro_log_cb(RETRO_LOG_INFO, "\"original_gui\" setting forced to false\n");
	}
	_mousePalette.reset();
	_gamePalette.reset();
}

void OSystem_libretro::engineDone() {
	reset_performance_tuner();
}

bool OSystem_libretro::hasFeature(Feature f) {
	return (f == OSystem::kFeatureCursorPalette) || (f == OSystem::kFeatureCursorAlpha);
}

void OSystem_libretro::setFeatureState(Feature f, bool enable) {
	if (f == kFeatureCursorPalette)
		_mousePaletteEnabled = enable;
}

bool OSystem_libretro::getFeatureState(Feature f) {
	return (f == kFeatureCursorPalette) ? _mousePaletteEnabled : false;
}

Audio::Mixer *OSystem_libretro::getMixer() {
	return _mixer;
}

void OSystem_libretro::refreshRetroSettings() {
	_adjusted_cursor_speed = (float)BASE_CURSOR_SPEED * retro_setting_get_gamepad_cursor_speed() * (float)(_overlayInGUI ? _overlay.w : _gameScreen.w) / 320.0f; // Dpad cursor speed should always be based off a 320 wide screen, to keep speeds consistent;
	_inverse_acceleration_time = (retro_setting_get_gamepad_acceleration_time() > 0.0) ? (1.0f / (float)retro_setting_get_frame_rate()) * (1.0f / retro_setting_get_gamepad_acceleration_time()) : 1.0f;
}

void OSystem_libretro::destroy() {
	delete this;
}

bool OSystem_libretro::checkPathSetting(const char *setting, Common::String const &defaultPath, bool isDirectory) {
	Common::String setPath;
	if (ConfMan.hasKey(setting))
		setPath = Common::Path::fromConfig(ConfMan.get(setting)).toString();

	if (setPath.empty() || ! (isDirectory ? LibRetroFilesystemNode(setPath).isDirectory() : LibRetroFilesystemNode(setPath).exists()))
		ConfMan.removeKey(setting, Common::ConfigManager::kApplicationDomain);
	if (! ConfMan.hasKey(setting))
		if (defaultPath.empty())
			return false;
		else
			ConfMan.set(setting, defaultPath);
	return true;
}

void OSystem_libretro::setLibretroDir(const char * path, Common::String &var) {
	var = Common::String(path ? path : "");
	if (! var.empty())
		if (! LibRetroFilesystemNode(var).isDirectory())
			var.clear();
	return;
}
