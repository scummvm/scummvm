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
#include "backends/platform/libretro/include/libretro-graphics-surface.h"
#ifdef USE_OPENGL
#include "backends/platform/libretro/include/libretro-graphics-opengl.h"
#endif

OSystem_libretro::OSystem_libretro() : _mouseX(0), _mouseY(0), _mouseXAcc(0.0), _mouseYAcc(0.0), _dpadXAcc(0.0), _dpadYAcc(0.0), _dpadXVel(0.0f), _dpadYVel(0.0f), _mixer(0), _startTime(0), _cursorStatus(0) {
	_fsFactory = new FS_SYSTEM_FACTORY();

	setLibretroDir(retro_get_system_dir(), s_systemDir);
	setLibretroDir(retro_get_save_dir(), s_saveDir);
	setLibretroDir(retro_get_playlist_dir(), s_playlistDir);

	memset(_mouseButtons, 0, sizeof(_mouseButtons));

	_startTime = (uint32)(cpu_features_get_time_usec() / 1000);
}

OSystem_libretro::~OSystem_libretro() {
	delete _mixer;
	_mixer = nullptr;
}

void OSystem_libretro::initBackend() {
	/* ScummVM paths checks are triggered by applyBackendSettings on setupGraphics() */

	/* Initialize other settings */
	if (! ConfMan.hasKey("libretro_playlist_version"))
		ConfMan.set("libretro_playlist_version", 0);

	if (! ConfMan.hasKey("libretro_hooks_location"))
		ConfMan.set("libretro_hooks_location", 0);

	if (! ConfMan.hasKey("libretro_hooks_clear"))
		ConfMan.set("libretro_hooks_clear", 0);

	_savefileManager = new DefaultSaveFileManager();

	_mixer = new Audio::MixerImpl(retro_setting_get_sample_rate(), true, retro_setting_get_audio_samples_buffer_size());
	retro_log_cb(RETRO_LOG_DEBUG, "Mixer set up at %dHz\n", retro_setting_get_sample_rate());

	_timerManager = new LibretroTimerManager(retro_setting_get_frame_rate());

	_mixer->setReady(true);

	resetGraphicsManager();

	EventsBaseBackend::initBackend();
	refreshRetroSettings();
}

void OSystem_libretro::engineInit() {
	Common::String engineId = ConfMan.get("engineid");
	if (engineId.equalsIgnoreCase("scumm") && ConfMan.getBool("original_gui")) {
		ConfMan.setBool("original_gui", false);
		retro_log_cb(RETRO_LOG_INFO, "\"original_gui\" setting forced to false\n");
	}

	/* See LibretroPalette::set workaround */
	/*if (retro_get_video_hw_mode() & VIDEO_GRAPHIC_MODE_REQUEST_SW){
		dynamic_cast<LibretroGraphics *>(_graphicsManager)->_mousePalette.reset();
		dynamic_cast<LibretroGraphics *>(_graphicsManager)->_gamePalette.reset();
	}*/
}

Audio::Mixer *OSystem_libretro::getMixer() {
	return _mixer;
}

void OSystem_libretro::refreshRetroSettings() {
	_adjusted_cursor_speed = (float)BASE_CURSOR_SPEED * retro_setting_get_gamepad_cursor_speed() * (float)getScreenWidth() / 320.0f; // Dpad cursor speed should always be based off a 320 wide screen, to keep speeds consistent;
	_inverse_acceleration_time = (retro_setting_get_gamepad_acceleration_time() > 0.0) ? (1.0f / (float)retro_setting_get_frame_rate()) * (1.0f / retro_setting_get_gamepad_acceleration_time()) : 1.0f;
}

void OSystem_libretro::destroy() {
	delete this;
}

void OSystem_libretro::getScreen(const Graphics::ManagedSurface *&screen) {
	if (retro_get_video_hw_mode() & VIDEO_GRAPHIC_MODE_REQUEST_SW)
		screen = dynamic_cast<LibretroGraphics *>(_graphicsManager)->getScreen();
}

void OSystem_libretro::refreshScreen(void) {
	if (retro_get_video_hw_mode() & VIDEO_GRAPHIC_MODE_REQUEST_SW)
		dynamic_cast<LibretroGraphics *>(_graphicsManager)->realUpdateScreen();
}

#ifdef USE_OPENGL
#ifdef USE_GLAD
void *OSystem_libretro::getOpenGLProcAddress(const char *name) const {
	return retro_get_proc_address(name);
}
#endif
void OSystem_libretro::resetGraphicsContext(void) {
	if ((retro_get_video_hw_mode() & VIDEO_GRAPHIC_MODE_REQUEST_HW) && (retro_get_video_hw_mode() & VIDEO_GRAPHIC_MODE_HAVE_OPENGL))
		dynamic_cast<LibretroOpenGLGraphics *>(_graphicsManager)->resetContext(OpenGL::kContextGL);
	else if ((retro_get_video_hw_mode() & VIDEO_GRAPHIC_MODE_REQUEST_HW) && (retro_get_video_hw_mode() & VIDEO_GRAPHIC_MODE_HAVE_OPENGLES2))
		dynamic_cast<LibretroOpenGLGraphics *>(_graphicsManager)->resetContext(OpenGL::kContextGLES2);
}
#endif

int16 OSystem_libretro::getScreenWidth(void){
	return dynamic_cast<WindowedGraphicsManager *>(_graphicsManager)->getWindowWidth();
}

int16 OSystem_libretro::getScreenHeight(void){
	return dynamic_cast<WindowedGraphicsManager *>(_graphicsManager)->getWindowHeight();
}

bool OSystem_libretro::isOverlayInGUI(void) {
#ifdef USE_OPENGL
	if (retro_get_video_hw_mode() & VIDEO_GRAPHIC_MODE_REQUEST_HW)
		return dynamic_cast<LibretroOpenGLGraphics *>(_graphicsManager)->isOverlayInGUI();
	else
#endif
		return dynamic_cast<LibretroGraphics *>(_graphicsManager)->isOverlayInGUI();
}

void OSystem_libretro::resetGraphicsManager(void) {

	if (_graphicsManager) {
		delete _graphicsManager;
		_graphicsManager = nullptr;
	}

#ifdef USE_OPENGL
	if ((retro_get_video_hw_mode() & VIDEO_GRAPHIC_MODE_REQUEST_HW) && (retro_get_video_hw_mode() & VIDEO_GRAPHIC_MODE_HAVE_OPENGL))
		_graphicsManager = new LibretroOpenGLGraphics(OpenGL::kContextGL);
	else if ((retro_get_video_hw_mode() & VIDEO_GRAPHIC_MODE_REQUEST_HW) && (retro_get_video_hw_mode() & VIDEO_GRAPHIC_MODE_HAVE_OPENGLES2))
		_graphicsManager = new LibretroOpenGLGraphics(OpenGL::kContextGLES2);
	else
#endif
		_graphicsManager = new LibretroGraphics();
}

bool OSystem_libretro::inLauncher() {
	return (nullptr == ConfMan.getActiveDomain());
}
