/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "backends/graphics/sdl/sdl-graphics.h"
#include "backends/graphics3d/sdl/sdl-graphics3d.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/platform/sdl/sdl-sys.h"
#include "backends/platform/sdl/sdl.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "common/config-manager.h"
#include "common/fs.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/file.h"

SdlGraphics3dManager::SdlGraphics3dManager(SdlEventSource *source, SdlWindow *window) :
		_eventSource(source), _window(window) {
	ConfMan.registerDefault("fullscreen_res", "desktop");
}

void SdlGraphics3dManager::activateManager() {
	_eventSource->setGraphicsManager(this);

	// Register the graphics manager as a event observer
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

void SdlGraphics3dManager::deactivateManager() {
	// Unregister the event observer
	if (g_system->getEventManager()->getEventDispatcher()) {
		g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
	}

	_eventSource->setGraphicsManager(0);
}

void SdlGraphics3dManager::saveScreenshot() {
	Common::String filename;

	Common::String screenshotsPath;
	OSystem_SDL *sdl_g_system = dynamic_cast<OSystem_SDL*>(g_system);
	if (sdl_g_system)
		screenshotsPath = sdl_g_system->getScreenshotsPath();

	// Use the name of the running target as a base for screenshot file names
	Common::String currentTarget = ConfMan.getActiveDomainName();

#ifdef USE_PNG
	const char *extension = "png";
#else
	const char *extension = "bmp";
#endif

	for (int n = 0;; n++) {
		filename = Common::String::format("scummvm%s%s-%05d.%s", currentTarget.empty() ? "" : "-",
		                                  currentTarget.c_str(), n, extension);

		Common::FSNode file = Common::FSNode(screenshotsPath + filename);
		if (!file.exists()) {
			break;
		}
	}

	if (saveScreenshot(screenshotsPath + filename)) {
		if (screenshotsPath.empty())
			debug("Saved screenshot '%s' in current directory", filename.c_str());
		else
			debug("Saved screenshot '%s' in directory '%s'", filename.c_str(), screenshotsPath.c_str());
	} else {
		if (screenshotsPath.empty())
			warning("Could not save screenshot in current directory");
		else
			warning("Could not save screenshot in directory '%s'", screenshotsPath.c_str());
	}
}

bool SdlGraphics3dManager::notifyEvent(const Common::Event &event) {
	if (event.type != Common::EVENT_CUSTOM_BACKEND_ACTION_START) {
		return false;
	}

	switch ((CustomEventAction) event.customType) {
	case kActionToggleMouseCapture:
		getWindow()->grabMouse(!getWindow()->mouseIsGrabbed());
		return true;

	case kActionToggleFullscreen:
		toggleFullScreen();
		return true;

	case kActionSaveScreenshot:
		saveScreenshot();
		return true;

	default:
		return false;
	}
}

void SdlGraphics3dManager::toggleFullScreen() {
	if (!g_system->hasFeature(OSystem::kFeatureFullscreenMode) ||
	   (!g_system->hasFeature(OSystem::kFeatureFullscreenToggleKeepsContext) && g_system->hasFeature(OSystem::kFeatureOpenGLForGame))) {
		return;
	}

	setFeatureState(OSystem::kFeatureFullscreenMode, !getFeatureState(OSystem::kFeatureFullscreenMode));
}

Common::Keymap *SdlGraphics3dManager::getKeymap() {
	using namespace Common;

	Keymap *keymap = new Keymap(Keymap::kKeymapTypeGlobal, "sdl-graphics", _("Graphics"));
	Action *act;

	if (g_system->hasFeature(OSystem::kFeatureFullscreenMode)) {
		act = new Action("FULS", _("Toggle fullscreen"));
		act->addDefaultInputMapping("A+RETURN");
		act->addDefaultInputMapping("A+KP_ENTER");
		act->setCustomBackendActionEvent(kActionToggleFullscreen);
		keymap->addAction(act);
	}

	act = new Action("CAPT", _("Toggle mouse capture"));
	act->addDefaultInputMapping("C+m");
	act->setCustomBackendActionEvent(kActionToggleMouseCapture);
	keymap->addAction(act);

	act = new Action("SCRS", _("Save screenshot"));
	act->addDefaultInputMapping("A+s");
	act->setCustomBackendActionEvent(kActionSaveScreenshot);
	keymap->addAction(act);

	if (hasFeature(OSystem::kFeatureAspectRatioCorrection)) {
		act = new Action("ASPT", _("Toggle aspect ratio correction"));
		act->addDefaultInputMapping("C+A+a");
		act->setCustomBackendActionEvent(kActionToggleAspectRatioCorrection);
		keymap->addAction(act);
	}

	return keymap;
}

Common::Rect SdlGraphics3dManager::getPreferredFullscreenResolution() {
	// Default to the desktop resolution, unless the user has set a
	// resolution in the configuration file
	const Common::String &fsres = ConfMan.get("fullscreen_res");
	if (fsres != "desktop") {
		uint newW, newH;
		int converted = sscanf(fsres.c_str(), "%ux%u", &newW, &newH);
		if (converted == 2) {
			return Common::Rect(newW, newH);
		} else {
			warning("Could not parse 'fullscreen_res' option: expected WWWxHHH, got %s", fsres.c_str());
		}
	}

	return _window->getDesktopResolution();
}

#pragma mark -
#pragma mark --- Mouse ---
#pragma mark -

bool SdlGraphics3dManager::showMouse(bool visible) {
	SDL_ShowCursor(visible);
	return true;
}

bool SdlGraphics3dManager::lockMouse(bool lock) {
	return _window->lockMouse(lock);
}

bool SdlGraphics3dManager::notifyMousePosition(Common::Point &mouse) {
	transformMouseCoordinates(mouse);

	return true;
}
