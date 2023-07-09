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

#if defined(_WIN32)
#include "backends/fs/windows/windows-fs-factory.h"
#define FS_SYSTEM_FACTORY WindowsFilesystemFactory
#else
#include "backends/platform/libretro/include/libretro-fs-factory.h"
#define FS_SYSTEM_FACTORY LibRetroFilesystemFactory
#endif

#include "audio/mixer_intern.h"
#include "backends/base-backend.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "backends/saves/default/default-saves.h"
#include "backends/platform/libretro/include/libretro-timer.h"
#include "backends/platform/libretro/include/libretro-os.h"
#include "backends/platform/libretro/include/libretro-defs.h"

OSystem_libretro::OSystem_libretro() : _mousePaletteEnabled(false), _mouseVisible(false), _mouseX(0), _mouseY(0), _mouseXAcc(0.0), _mouseYAcc(0.0), _mouseHotspotX(0), _mouseHotspotY(0), _dpadXAcc(0.0), _dpadYAcc(0.0), _dpadXVel(0.0f), _dpadYVel(0.0f), _mouseKeyColor(0), _mouseDontScale(false), _joypadnumpadLast(8), _joypadnumpadActive(false), _mixer(0), _startTime(0), _threadSwitchCaller(0) {
	_fsFactory = new FS_SYSTEM_FACTORY();
	memset(_mouseButtons, 0, sizeof(_mouseButtons));
	memset(_joypadmouseButtons, 0, sizeof(_joypadmouseButtons));
	memset(_joypadkeyboardButtons, 0, sizeof(_joypadkeyboardButtons));

	s_systemDir = Common::String(retro_get_system_dir());
	s_saveDir = Common::String(retro_get_save_dir());
	s_themeDir = s_systemDir + "/" + SCUMMVM_SYSTEM_SUBDIR + "/" + SCUMMVM_THEME_SUBDIR;
	s_extraDir = s_systemDir + "/" + SCUMMVM_SYSTEM_SUBDIR + "/" + SCUMMVM_EXTRA_SUBDIR;
	s_lastDir = s_systemDir;

	_startTime = getMillis();
}

OSystem_libretro::~OSystem_libretro() {
	_gameScreen.free();
	_overlay.free();
	_mouseImage.free();
	_screen.free();

	delete _mixer;
}

void OSystem_libretro::initBackend() {

	_savefileManager = new DefaultSaveFileManager(s_saveDir);

	if (! ConfMan.hasKey("themepath")) {
		if (! Common::FSNode(s_themeDir).exists())
			retro_osd_notification("ScummVM theme folder not found.");
		else
			ConfMan.set("themepath", s_themeDir);
	}

	if (! ConfMan.hasKey("extrapath")) {
		if (! Common::FSNode(s_extraDir).exists())
			retro_osd_notification("ScummVM datafiles folder not found. Some engines/features will not work.");
		else
			ConfMan.set("extrapath", s_extraDir);
	}

	if (! ConfMan.hasKey("browser_lastpath"))
		ConfMan.set("browser_lastpath", s_lastDir);

#ifdef FRONTEND_SUPPORTS_RGB565
	_overlay.create(RES_W_OVERLAY, RES_H_OVERLAY, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
#else
	_overlay.create(RES_W_OVERLAY, RES_H_OVERLAY, Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 15));
#endif
	_mixer = new Audio::MixerImpl(sample_rate);
	log_cb(RETRO_LOG_DEBUG,"Mixer set up at %dHz\n", sample_rate);

	_timerManager = new LibretroTimerManager(frame_rate);

	_mixer->setReady(true);

	EventsBaseBackend::initBackend();
}

void OSystem_libretro::engineInit() {
	Common::String engineId = ConfMan.get("engineid");
	if (engineId.equalsIgnoreCase("scumm") && ConfMan.getBool("original_gui")) {
		ConfMan.setBool("original_gui", false);
		log_cb(RETRO_LOG_INFO, "\"original_gui\" setting forced to false\n");
	}
}

void OSystem_libretro::engineDone() {
	reset_performance_tuner();
}

bool OSystem_libretro::hasFeature(Feature f) {
	return (f == OSystem::kFeatureCursorPalette);
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

void OSystem_libretro::destroy() {
	delete this;
}
