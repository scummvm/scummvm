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

#include "common/scummsys.h"

#ifdef __amigaos4__

#include "backends/platform/sdl/amigaos/amigaos.h"
#include "backends/fs/amigaos/amigaos-fs-factory.h"
#include "backends/dialogs/amigaos/amigaos-dialogs.h"

void OSystem_AmigaOS::init() {
	// Initialze File System Factory
	_fsFactory = new AmigaOSFilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();

#if defined(USE_SYSDIALOGS)
	_dialogManager = new AmigaOSDialogManager();
#endif
}

bool OSystem_AmigaOS::hasFeature(Feature f) {
#if defined(USE_SYSDIALOGS)
	if (f == kFeatureSystemBrowserDialog)
		return true;
#endif

	return OSystem_SDL::hasFeature(f);
}

void OSystem_AmigaOS::initBackend() {
	// AmigaOS4 SDL provides two OpenGL implementations
	// (OpenGL 1.3 with miniGL and OpenGL ES with OGLES2)
	// This is chosen by setting the profile mask attribute
	// before the first window creation but after init
	int force = 0;
	if (ConfMan.hasKey("opengl_implementation")) {
		Common::String implem = ConfMan.get("opengl_implementation");
		if (implem == "gl") {
			force = 1;;
		} else if (implem == "gles2") {
			force = 2;
		}
	}

	// If not forcing, try OGLES2 first
	if (!force || force == 2) {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		if (SDL_GL_LoadLibrary(NULL) < 0) {
			if (force) {
				warning("OpenGL implementation chosen is unsupported, falling back");
				force = 0;
			}
			// SDL doesn't seem to be clean when loading fail
			SDL_GL_UnloadLibrary();
			SDL_GL_ResetAttributes();
		} else {
			// Loading succeeded, don't try anything more
			force = 2;
		}
	}
	// If not forcing, next try miniGL
	if (!force || force == 1) {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		if (SDL_GL_LoadLibrary(NULL) < 0) {
			if (force) {
				warning("OpenGL implementation chosen is unsupported, falling back");
				force = 0;
			}
			// SDL doesn't seem to be clean when loading fail
			SDL_GL_UnloadLibrary();
			SDL_GL_ResetAttributes();
		} else {
			// Loading succeeded, don't try anything more
			force = 1;
		}
	}
	// First time user defaults
	ConfMan.registerDefault("audio_buffer_size", "2048");
	ConfMan.registerDefault("aspect_ratio", true);
	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("gfx_mode", "opengl");
	ConfMan.registerDefault("stretch_mode", "stretch");
	ConfMan.registerDefault("gui_mode", "antialias");
	ConfMan.registerDefault("gui_theme", "scummremastered");
	ConfMan.registerDefault("gui_scale", "125");
	ConfMan.registerDefault("extrapath", "extras/");
	ConfMan.registerDefault("themepath", "themes/");
	// First time .ini defaults
	if (!ConfMan.hasKey("audio_buffer_size")) {
		ConfMan.set("audio_buffer_size", "2048");
	}
	if (!ConfMan.hasKey("aspect_ratio")) {
		ConfMan.setBool("aspect_ratio", true);
	}
	if (!ConfMan.hasKey("fullscreen")) {
		ConfMan.setBool("fullscreen", true);
	}
	if (!ConfMan.hasKey("gfx_mode")) {
		ConfMan.set("gfx_mode", "opengl");
	}
	if (!ConfMan.hasKey("stretch_mode")) {
		ConfMan.set("stretch_mode", "stretch");
	}
	if (!ConfMan.hasKey("gui_mode")) {
		ConfMan.set("gui_mode", "antialias");
	}
	if (!ConfMan.hasKey("gui_theme")) {
		ConfMan.set("gui_theme", "scummremastered");
	}
	if (!ConfMan.hasKey("gui_scale")) {
		ConfMan.set("gui_scale", "125");
	}
	if (!ConfMan.hasKey("extrapath")) {
		ConfMan.set("extrapath", "extras/");
	}
	if (!ConfMan.hasKey("themepath")) {
		ConfMan.set("themepath", "themes/");
	}
	OSystem_SDL::initBackend();
}

#endif
