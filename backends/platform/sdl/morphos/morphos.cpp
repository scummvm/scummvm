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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#include "common/scummsys.h"

#ifdef __MORPHOS__

#include "backends/platform/sdl/morphos/morphos.h"
#include "backends/fs/morphos/morphos-fs-factory.h"
#include "backends/dialogs/morphos/morphos-dialogs.h"

void OSystem_MorphOS::init() {
	// Initialze File System Factory
	_fsFactory = new MorphOSFilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();

#if defined(USE_SYSDIALOGS)
	_dialogManager = new MorphosDialogManager();
#endif
}

bool OSystem_MorphOS::hasFeature(Feature f) {
	if (f == kFeatureOpenUrl)
		return true;

#if defined(USE_SYSDIALOGS)
	if (f == kFeatureSystemBrowserDialog)
		return true;
#endif

	return OSystem_SDL::hasFeature(f);
}

void OSystem_MorphOS::initBackend() {

	// First time user defaults
	ConfMan.registerDefault("audio_buffer_size", "2048");
	ConfMan.registerDefault("fullscreen", false);
	ConfMan.registerDefault("gfx_mode", "surfacesdl");
	ConfMan.registerDefault("stretch_mode", "stretch");
	ConfMan.registerDefault("gui_mode", "antialias");
	ConfMan.registerDefault("gui_theme", "scummremastered");
	ConfMan.registerDefault("gui_scale", "125");
	ConfMan.registerDefault("extrapath", Common::Path("extras/"));
	ConfMan.registerDefault("iconspath", Common::Path("icons/"));
	ConfMan.registerDefault("pluginspath", Common::Path("plugins/"));
	ConfMan.registerDefault("savepath", Common::Path("saves/"));
	ConfMan.registerDefault("themepath", Common::Path("themes/"));
	// First time .ini defaults
	if (!ConfMan.hasKey("audio_buffer_size")) {
		ConfMan.set("audio_buffer_size", "2048");
	}
	if (!ConfMan.hasKey("fullscreen")) {
		ConfMan.setBool("fullscreen", false);
	}
	if (!ConfMan.hasKey("gfx_mode")) {
		ConfMan.set("gfx_mode", "surfacesdl");
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
		ConfMan.setPath("extrapath", "extras/");
	}
	if (!ConfMan.hasKey("iconspath")) {
		ConfMan.setPath("iconspath", "icons/");
	}
	if (!ConfMan.hasKey("pluginspath")) {
		ConfMan.setPath("pluginspath", "plugins/");
	}
	if (!ConfMan.hasKey("savepath")) {
		ConfMan.setPath("savepath", "saves/");
	}
	if (!ConfMan.hasKey("themepath")) {
		ConfMan.setPath("themepath", "themes/");
	}
	OSystem_SDL::initBackend();
}

Common::String OSystem_MorphOS::getSystemLanguage() const {
#if defined(USE_DETECTLANG) && defined(USE_TRANSLATION)
	SDL_Locale *locales = SDL_GetPreferredLocales(); // USE SDL TO CHECK SYSTEM LANGUAGE
    if (locales) {
		if (locales[0].language != NULL) {
			return Common::String::format("%s_%s", locales[0].country,	locales[0].language);
		}
		SDL_free(locales);
    } 
#endif // USE_DETECTLANG
	return OSystem_SDL::getSystemLanguage();
}

void OSystem_MorphOS::logMessage(LogMessageType::Type type, const char * message) {
#ifdef DEBUG_BUILD
	printf("%s\n", message);
#endif
}
#endif
