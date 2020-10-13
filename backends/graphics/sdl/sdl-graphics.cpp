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

#include "backends/graphics/sdl/sdl-graphics.h"
#include "backends/platform/sdl/sdl-sys.h"
#include "backends/platform/sdl/sdl.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "common/config-manager.h"
#include "common/fs.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "graphics/scaler/aspect.h"
#ifdef USE_OSD
#include "common/translation.h"
#endif

SdlGraphicsManager::SdlGraphicsManager(SdlEventSource *source, SdlWindow *window)
	: _eventSource(source), _window(window), _hwScreen(nullptr)
#if SDL_VERSION_ATLEAST(2, 0, 0)
	, _allowWindowSizeReset(false), _hintedWidth(0), _hintedHeight(0), _lastFlags(0)
#endif
{
	SDL_GetMouseState(&_cursorX, &_cursorY);
}

void SdlGraphicsManager::activateManager() {
	_eventSource->setGraphicsManager(this);

	// Register the graphics manager as a event observer
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

void SdlGraphicsManager::deactivateManager() {
	// Unregister the event observer
	if (g_system->getEventManager()->getEventDispatcher()) {
		g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
	}

	_eventSource->setGraphicsManager(0);
}

SdlGraphicsManager::State SdlGraphicsManager::getState() const {
	State state;

	state.screenWidth   = getWidth();
	state.screenHeight  = getHeight();
	state.aspectRatio   = getFeatureState(OSystem::kFeatureAspectRatioCorrection);
	state.fullscreen    = getFeatureState(OSystem::kFeatureFullscreenMode);
	state.cursorPalette = getFeatureState(OSystem::kFeatureCursorPalette);
#ifdef USE_RGB_COLOR
	state.pixelFormat   = getScreenFormat();
#endif
	return state;
}

bool SdlGraphicsManager::setState(const State &state) {
	beginGFXTransaction();
#ifdef USE_RGB_COLOR
		initSize(state.screenWidth, state.screenHeight, &state.pixelFormat);
#else
		initSize(state.screenWidth, state.screenHeight, nullptr);
#endif
		setFeatureState(OSystem::kFeatureAspectRatioCorrection, state.aspectRatio);
		setFeatureState(OSystem::kFeatureFullscreenMode, state.fullscreen);
		setFeatureState(OSystem::kFeatureCursorPalette, state.cursorPalette);

	if (endGFXTransaction() != OSystem::kTransactionSuccess) {
		return false;
	} else {
		return true;
	}
}

bool SdlGraphicsManager::defaultGraphicsModeConfig() const {
	const Common::ConfigManager::Domain *transientDomain = ConfMan.getDomain(Common::ConfigManager::kTransientDomain);
	if (transientDomain && transientDomain->contains("gfx_mode")) {
		const Common::String &mode = transientDomain->getVal("gfx_mode");
		if (!mode.equalsIgnoreCase("normal") && !mode.equalsIgnoreCase("default")) {
			return false;
		}
	}

	const Common::ConfigManager::Domain *gameDomain = ConfMan.getActiveDomain();
	if (gameDomain && gameDomain->contains("gfx_mode")) {
		const Common::String &mode = gameDomain->getVal("gfx_mode");
		if (!mode.equalsIgnoreCase("normal") && !mode.equalsIgnoreCase("default")) {
			return false;
		}
	}

	return true;
}

int SdlGraphicsManager::getGraphicsModeIdByName(const Common::String &name) const {
	if (name == "normal" || name == "default") {
		return getDefaultGraphicsMode();
	}

	const OSystem::GraphicsMode *mode = getSupportedGraphicsModes();
	while (mode && mode->name != nullptr) {
		if (name.equalsIgnoreCase(mode->name)) {
			return mode->id;
		}
		++mode;
	}
	return -1;
}

void SdlGraphicsManager::initSizeHint(const Graphics::ModeList &modes) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	const bool useDefault = defaultGraphicsModeConfig();

	int scale = getGraphicsModeScale(getGraphicsModeIdByName(ConfMan.get("gfx_mode")));
	if (scale == -1) {
		warning("Unknown scaler; defaulting to 1");
		scale = 1;
	}

	int16 bestWidth = 0, bestHeight = 0;
	const Graphics::ModeList::const_iterator end = modes.end();
	for (Graphics::ModeList::const_iterator it = modes.begin(); it != end; ++it) {
		int16 width = it->width, height = it->height;

		// TODO: Normalize AR correction by passing a PAR in the mode list
		// instead of checking the dimensions here like this, since not all
		// 320x200/640x400 uses are with non-square pixels (e.g. DreamWeb).
		if (ConfMan.getBool("aspect_ratio")) {
			if ((width == 320 && height == 200) || (width == 640 && height == 400)) {
				height = real2Aspect(height);
			}
		}

		if (!useDefault || width <= 320) {
			width *= scale;
			height *= scale;
		}

		if (bestWidth < width) {
			bestWidth = width;
		}

		if (bestHeight < height) {
			bestHeight = height;
		}
	}

	_hintedWidth = bestWidth;
	_hintedHeight = bestHeight;
#endif
}

bool SdlGraphicsManager::showMouse(bool visible) {
	if (visible == _cursorVisible) {
		return visible;
	}

	int showCursor = SDL_DISABLE;
	if (visible) {
		// _cursorX and _cursorY are currently always clipped to the active
		// area, so we need to ask SDL where the system's mouse cursor is
		// instead
		int x, y;
		SDL_GetMouseState(&x, &y);
		if (!_activeArea.drawRect.contains(Common::Point(x, y))) {
			showCursor = SDL_ENABLE;
		}
	}
	SDL_ShowCursor(showCursor);

	return WindowedGraphicsManager::showMouse(visible);
}

bool SdlGraphicsManager::lockMouse(bool lock) {
	return _window->lockMouse(lock);
}

bool SdlGraphicsManager::notifyMousePosition(Common::Point &mouse) {
	mouse.x = CLIP<int16>(mouse.x, 0, _windowWidth - 1);
	mouse.y = CLIP<int16>(mouse.y, 0, _windowHeight - 1);

	int showCursor = SDL_DISABLE;
	bool valid = true;
	if (_activeArea.drawRect.contains(mouse)) {
		_cursorLastInActiveArea = true;
	} else {
		mouse.x = CLIP<int>(mouse.x, _activeArea.drawRect.left, _activeArea.drawRect.right - 1);
		mouse.y = CLIP<int>(mouse.y, _activeArea.drawRect.top, _activeArea.drawRect.bottom - 1);

		if (_window->mouseIsGrabbed() ||
			// Keep the mouse inside the game area during dragging to prevent an
			// event mismatch where the mouseup event gets lost because it is
			// performed outside of the game area
			(_cursorLastInActiveArea && SDL_GetMouseState(nullptr, nullptr) != 0)) {
			setSystemMousePosition(mouse.x, mouse.y);
		} else {
			// Allow the in-game mouse to get a final movement event to the edge
			// of the window if the mouse was moved out of the game area
			if (_cursorLastInActiveArea) {
				_cursorLastInActiveArea = false;
			} else if (_cursorVisible) {
				// Keep sending events to the game if the cursor is invisible,
				// since otherwise if a game lets you skip a cutscene by
				// clicking and the user moved the mouse outside the active
				// area, the clicks wouldn't do anything, which would be
				// confusing
				valid = false;
			}

			if (_cursorVisible) {
				showCursor = SDL_ENABLE;
			}
		}
	}

	SDL_ShowCursor(showCursor);
	if (valid) {
		setMousePosition(mouse.x, mouse.y);
		mouse = convertWindowToVirtual(mouse.x, mouse.y);
	}
	return valid;
}

void SdlGraphicsManager::setSystemMousePosition(const int x, const int y) {
	assert(_window);
	if (!_window->warpMouseInWindow(x, y)) {
		const Common::Point mouse = convertWindowToVirtual(x, y);
		_eventSource->fakeWarpMouse(mouse.x, mouse.y);
	}
}

void SdlGraphicsManager::handleResizeImpl(const int width, const int height, const int xdpi, const int ydpi) {
	_forceRedraw = true;
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
bool SdlGraphicsManager::createOrUpdateWindow(int width, int height, const Uint32 flags) {
	if (!_window) {
		return false;
	}

	// We only update the actual window when flags change (which usually means
	// fullscreen mode is entered/exited), when updates are forced so that we
	// do not reset the window size whenever a game makes a call to change the
	// size or pixel format of the internal game surface (since a user may have
	// resized the game window), or when the launcher is visible (since a user
	// may change the scaler, which should reset the window size)
	if (!_window->getSDLWindow() || _lastFlags != flags || _overlayVisible || _allowWindowSizeReset) {
		const bool fullscreen = (flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
		if (!fullscreen) {
			if (_hintedWidth) {
				width = _hintedWidth;
			}
			if (_hintedHeight) {
				height = _hintedHeight;
			}
		}

		if (!_window->createOrUpdateWindow(width, height, flags)) {
			return false;
		}

		_lastFlags = flags;
		_allowWindowSizeReset = false;
	}

	return true;
}
#endif

void SdlGraphicsManager::saveScreenshot() {
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

bool SdlGraphicsManager::notifyEvent(const Common::Event &event) {
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

void SdlGraphicsManager::toggleFullScreen() {
	if (!g_system->hasFeature(OSystem::kFeatureFullscreenMode) ||
	   (!g_system->hasFeature(OSystem::kFeatureFullscreenToggleKeepsContext) && g_system->hasFeature(OSystem::kFeatureOpenGLForGame))) {
		return;
	}

	beginGFXTransaction();
	setFeatureState(OSystem::kFeatureFullscreenMode, !getFeatureState(OSystem::kFeatureFullscreenMode));
	endGFXTransaction();
#ifdef USE_OSD
	if (getFeatureState(OSystem::kFeatureFullscreenMode))
		displayMessageOnOSD(_("Fullscreen mode"));
	else
		displayMessageOnOSD(_("Windowed mode"));
#endif
}

Common::Keymap *SdlGraphicsManager::getKeymap() {
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

	if (hasFeature(OSystem::kFeatureFilteringMode)) {
		act = new Action("FILT", _("Toggle linear filtered scaling"));
		act->addDefaultInputMapping("C+A+f");
		act->setCustomBackendActionEvent(kActionToggleFilteredScaling);
		keymap->addAction(act);
	}

	if (hasFeature(OSystem::kFeatureStretchMode)) {
		act = new Action("STCH", _("Cycle through stretch modes"));
		act->addDefaultInputMapping("C+A+s");
		act->setCustomBackendActionEvent(kActionCycleStretchMode);
		keymap->addAction(act);
	}

	act = new Action("SCL+", _("Increase the scale factor"));
	act->addDefaultInputMapping("C+A+PLUS");
	act->addDefaultInputMapping("C+A+KP_PLUS");
	act->setCustomBackendActionEvent(kActionIncreaseScaleFactor);
	keymap->addAction(act);

	act = new Action("SCL-", _("Decrease the scale factor"));
	act->addDefaultInputMapping("C+A+MINUS");
	act->addDefaultInputMapping("C+A+KP_MINUS");
	act->setCustomBackendActionEvent(kActionDecreaseScaleFactor);
	keymap->addAction(act);

#ifdef USE_SCALERS
	struct ActionEntry {
		const char *id;
		const char *description;
	};
	static const ActionEntry filters[] = {
			{ "FLT1", _s("Switch to nearest neighbour scaling") },
			{ "FLT2", _s("Switch to AdvMame 2x/3x scaling")     },
#ifdef USE_HQ_SCALERS
			{ "FLT3", _s("Switch to HQ 2x/3x scaling")          },
#endif
			{ "FLT4", _s("Switch to 2xSai scaling")             },
			{ "FLT5", _s("Switch to Super2xSai scaling")        },
			{ "FLT6", _s("Switch to SuperEagle scaling")        },
			{ "FLT7", _s("Switch to TV 2x scaling")             },
			{ "FLT8", _s("Switch to DotMatrix scaling")         }
	};

	for (uint i = 0; i < ARRAYSIZE(filters); i++) {
		act = new Action(filters[i].id, _(filters[i].description));
		act->addDefaultInputMapping(String::format("C+A+%d", i + 1));
		act->addDefaultInputMapping(String::format("C+A+KP%d", i + 1));
		act->setCustomBackendActionEvent(kActionSetScaleFilter1 + i);
		keymap->addAction(act);
	}
#endif

	return keymap;
}
