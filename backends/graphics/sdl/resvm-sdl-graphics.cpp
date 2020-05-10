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

#include "backends/graphics/sdl/resvm-sdl-graphics.h"

#include "backends/platform/sdl/sdl-sys.h"
#include "backends/events/sdl/resvm-sdl-events.h"
#include "backends/platform/sdl/sdl.h"

#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/file.h"

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
		{0, 0, 0}
};

ResVmSdlGraphicsManager::ResVmSdlGraphicsManager(SdlEventSource *source, SdlWindow *window, const Capabilities &capabilities) :
		SdlGraphicsManager(source, window),
		_fullscreen(false),
		_lockAspectRatio(true),
		_overlayVisible(false),
		_screenChangeCount(0),
		_capabilities(capabilities),
		_engineRequestedWidth(0),
		_engineRequestedHeight(0)  {
	ConfMan.registerDefault("fullscreen_res", "desktop");
	ConfMan.registerDefault("aspect_ratio", true);
	ConfMan.registerDefault("vsync", true);
}

ResVmSdlGraphicsManager::~ResVmSdlGraphicsManager() {
}

void ResVmSdlGraphicsManager::activateManager() {
	SdlGraphicsManager::activateManager();

	// Register the graphics manager as a event observer
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

void ResVmSdlGraphicsManager::deactivateManager() {
	// Unregister the event observer
	if (g_system->getEventManager()->getEventDispatcher()) {
		g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
	}

	SdlGraphicsManager::deactivateManager();
}

Common::Rect ResVmSdlGraphicsManager::getPreferredFullscreenResolution() {
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

void ResVmSdlGraphicsManager::resetGraphicsScale() {
	setGraphicsMode(0);
}

void ResVmSdlGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
		case OSystem::kFeatureAspectRatioCorrection:
			_lockAspectRatio = enable;
			break;
		default:
			break;
	}
}

bool ResVmSdlGraphicsManager::getFeatureState(OSystem::Feature f) const {
	switch (f) {
		case OSystem::kFeatureFullscreenMode:
			return _fullscreen;
		case OSystem::kFeatureAspectRatioCorrection:
			return _lockAspectRatio;
		default:
			return false;
	}
}

const OSystem::GraphicsMode *ResVmSdlGraphicsManager::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int ResVmSdlGraphicsManager::getDefaultGraphicsMode() const {
	return 0;// ResidualVM: not use it
}

void ResVmSdlGraphicsManager::beginGFXTransaction() {
	// ResidualVM: not use it
}

OSystem::TransactionError ResVmSdlGraphicsManager::endGFXTransaction() {
	// ResidualVM: not use it
	return OSystem::kTransactionSuccess;
}

#ifdef USE_RGB_COLOR
Common::List<Graphics::PixelFormat> ResVmSdlGraphicsManager::getSupportedFormats() const {
	// ResidualVM: not use it
	return _supportedFormats;
}
#endif

bool ResVmSdlGraphicsManager::setGraphicsMode(int mode) {
	// ResidualVM: not use it
	return true;
}

int ResVmSdlGraphicsManager::getGraphicsMode() const {
	// ResidualVM: not use it
	return 0;
}

void ResVmSdlGraphicsManager::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
	// ResidualVM: not use it
}

void ResVmSdlGraphicsManager::copyRectToScreen(const void *src, int pitch, int x, int y, int w, int h) {
	// ResidualVM: not use it
}

Graphics::Surface *ResVmSdlGraphicsManager::lockScreen() {
	return NULL; // ResidualVM: not use it
}

void ResVmSdlGraphicsManager::unlockScreen() {
	// ResidualVM: not use it
}

void ResVmSdlGraphicsManager::fillScreen(uint32 col) {
	// ResidualVM: not use it
}

void ResVmSdlGraphicsManager::setPalette(const byte *colors, uint start, uint num) {
	// ResidualVM: not use it
}

void ResVmSdlGraphicsManager::grabPalette(byte *colors, uint start, uint num) const {
	// ResidualVM: not use it
}

void ResVmSdlGraphicsManager::setCursorPalette(const byte *colors, uint start, uint num) {
	// ResidualVM: not use it
}

void ResVmSdlGraphicsManager::setShakePos(int shakeXOffset, int shakeYOffset) {
	// ResidualVM: not use it
}

void ResVmSdlGraphicsManager::setFocusRectangle(const Common::Rect &rect) {
	// ResidualVM: not use it
}

void ResVmSdlGraphicsManager::clearFocusRectangle() {
	// ResidualVM: not use it
}

#pragma mark -
#pragma mark --- Mouse ---
#pragma mark -

bool ResVmSdlGraphicsManager::showMouse(bool visible) {
	SDL_ShowCursor(visible);
	return true;
}

bool ResVmSdlGraphicsManager::lockMouse(bool lock) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (lock)
		SDL_SetRelativeMouseMode(SDL_TRUE);
	else
		SDL_SetRelativeMouseMode(SDL_FALSE);
#else
	if (lock)
		SDL_WM_GrabInput(SDL_GRAB_ON);
	else
		SDL_WM_GrabInput(SDL_GRAB_OFF);
#endif
	return true;
}

bool ResVmSdlGraphicsManager::isMouseLocked() const {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	return SDL_GetRelativeMouseMode() == SDL_TRUE;
#else
	return SDL_GrabMode() == SDL_GRAB_ON;
#endif
}

void ResVmSdlGraphicsManager::setMouseCursor(const void *buf, uint w, uint h, int hotspot_x, int hotspot_y, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	// ResidualVM: not use it
}

#pragma mark -
#pragma mark --- On Screen Display ---
#pragma mark -

#ifdef USE_OSD
void OpenGLResVmSdlGraphicsManager::displayMessageOnOSD(const char *msg) {
	// ResidualVM: not use it
}
#endif

bool ResVmSdlGraphicsManager::notifyEvent(const Common::Event &event) {
	//ResidualVM specific:
	switch ((int)event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.hasFlags(Common::KBD_ALT) && event.kbd.keycode == Common::KEYCODE_s) {
				saveScreenshot();
				return true;
			}
			break;
		case Common::EVENT_KEYUP:
			break;
		default:
			break;
	}

	return false;
}

void ResVmSdlGraphicsManager::notifyVideoExpose() {
	//ResidualVM specific:
	//updateScreen();
}

bool ResVmSdlGraphicsManager::notifyMousePosition(Common::Point &mouse) {
	transformMouseCoordinates(mouse);
	// ResidualVM: not use that:
	//setMousePos(mouse.x, mouse.y);
	return true;
}

void ResVmSdlGraphicsManager::saveScreenshot() {
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
		filename = Common::String::format("resiudalvm%s%s-%05d.%s", currentTarget.empty() ? "" : "-",
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

int ResVmSdlGraphicsManager::getGraphicsModeScale(int mode) const {
	return -1;
}
