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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "graphics/fontman.h"

#include "backends/platform/tizen/form.h"
#include "backends/platform/tizen/system.h"
#include "backends/platform/tizen/graphics.h"

//
// TizenGraphicsManager
//
TizenGraphicsManager::TizenGraphicsManager(TizenAppForm *appForm) :
	_appForm(appForm),
	_eglDisplay(EGL_DEFAULT_DISPLAY),
	_eglSurface(EGL_NO_SURFACE),
	_eglConfig(NULL),
	_eglContext(EGL_NO_CONTEXT),
	_initState(true) {
	assert(appForm != NULL);
	_videoMode.fullscreen = true;
}

TizenGraphicsManager::~TizenGraphicsManager() {
	logEntered();

	if (_eglDisplay != EGL_NO_DISPLAY) {
		eglMakeCurrent(_eglDisplay, NULL, NULL, NULL);
		if (_eglContext != EGL_NO_CONTEXT) {
			eglDestroyContext(_eglDisplay, _eglContext);
		}
	}
}

const Graphics::Font *TizenGraphicsManager::getFontOSD() {
	return FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
}

bool TizenGraphicsManager::moveMouse(int16 &x, int16 &y) {
	int16 currentX = _cursorState.x;
	int16 currentY = _cursorState.y;

	// save the current hardware coordinates
	_cursorState.x = x;
	_cursorState.y = y;

	// return x/y as game coordinates
	adjustMousePosition(x, y);

	// convert current x/y to game coordinates
	adjustMousePosition(currentX, currentY);

	// return whether game coordinates have changed
	return (currentX != x || currentY != y);
}

Common::List<Graphics::PixelFormat> TizenGraphicsManager::getSupportedFormats() const {
	logEntered();

	Common::List<Graphics::PixelFormat> res;
	res.push_back(Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0));
	res.push_back(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	res.push_back(Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0));
	res.push_back(Graphics::PixelFormat::createFormatCLUT8());
	return res;
}

bool TizenGraphicsManager::hasFeature(OSystem::Feature f) {
	bool result = (f == OSystem::kFeatureFullscreenMode ||
			f == OSystem::kFeatureVirtualKeyboard ||
			OpenGLGraphicsManager::hasFeature(f));
	return result;
}

void TizenGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	if (f == OSystem::kFeatureVirtualKeyboard && enable) {
		_appForm->showKeypad();
	} else {
		OpenGLGraphicsManager::setFeatureState(f, enable);
	}
}

void TizenGraphicsManager::setReady() {
	_initState = false;
}

void TizenGraphicsManager::updateScreen() {
	if (_transactionMode == kTransactionNone) {
		internUpdateScreen();
	}
}

bool TizenGraphicsManager::loadEgl() {
	logEntered();

	EGLint numConfigs = 1;
	EGLint eglConfigList[] = {
		EGL_RED_SIZE,	5,
		EGL_GREEN_SIZE, 6,
		EGL_BLUE_SIZE,	5,
		EGL_ALPHA_SIZE, 0,
		EGL_DEPTH_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
		EGL_NONE
	};

	EGLint eglContextList[] = {
		EGL_CONTEXT_CLIENT_VERSION, 1,
		EGL_NONE
	};

	eglBindAPI(EGL_OPENGL_ES_API);

	if (_eglDisplay) {
		unloadGFXMode();
	}

	_eglDisplay = eglGetDisplay((EGLNativeDisplayType) EGL_DEFAULT_DISPLAY);
	if (EGL_NO_DISPLAY == _eglDisplay) {
		systemError("eglGetDisplay() failed");
		return false;
	}

	if (EGL_FALSE == eglInitialize(_eglDisplay, NULL, NULL) ||
			EGL_SUCCESS != eglGetError()) {
		systemError("eglInitialize() failed");
		return false;
	}

	if (EGL_FALSE == eglChooseConfig(_eglDisplay, eglConfigList, &_eglConfig, 1, &numConfigs) ||
			EGL_SUCCESS != eglGetError()) {
		systemError("eglChooseConfig() failed");
		return false;
	}

	if (!numConfigs) {
		systemError("eglChooseConfig() failed. Matching config does not exist \n");
		return false;
	}

	_eglSurface = eglCreateWindowSurface(_eglDisplay, _eglConfig, (EGLNativeWindowType)_appForm, NULL);
	if (EGL_NO_SURFACE == _eglSurface || EGL_SUCCESS != eglGetError()) {
		systemError("eglCreateWindowSurface() failed. EGL_NO_SURFACE");
		return false;
	}

	_eglContext = eglCreateContext(_eglDisplay, _eglConfig, EGL_NO_CONTEXT, eglContextList);
	if (EGL_NO_CONTEXT == _eglContext ||
			EGL_SUCCESS != eglGetError()) {
		systemError("eglCreateContext() failed");
		return false;
	}

	if (false == eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _eglContext) ||
			EGL_SUCCESS != eglGetError()) {
		systemError("eglMakeCurrent() failed");
		return false;
	}

	logLeaving();
	return true;
}

bool TizenGraphicsManager::loadGFXMode() {
	logEntered();

	if (!loadEgl()) {
		unloadGFXMode();
		return false;
	}

	int x, y, width, height;
	_appForm->GetBounds(x, y, width, height);
	_videoMode.overlayWidth = _videoMode.hardwareWidth = width;
	_videoMode.overlayHeight = _videoMode.hardwareHeight = height;
	_videoMode.scaleFactor = 4; // for proportional sized cursor in the launcher

	AppLog("screen size: %dx%d", _videoMode.hardwareWidth, _videoMode.hardwareHeight);
	return OpenGLGraphicsManager::loadGFXMode();
}

void TizenGraphicsManager::loadTextures() {
	logEntered();
	OpenGLGraphicsManager::loadTextures();
}

void TizenGraphicsManager::internUpdateScreen() {
	if (!_initState) {
		OpenGLGraphicsManager::internUpdateScreen();
		eglSwapBuffers(_eglDisplay, _eglSurface);
	}
}

void TizenGraphicsManager::unloadGFXMode() {
	logEntered();

	if (_eglDisplay != EGL_NO_DISPLAY) {
		eglMakeCurrent(_eglDisplay, NULL, NULL, NULL);

		if (_eglContext != EGL_NO_CONTEXT) {
			eglDestroyContext(_eglDisplay, _eglContext);
			_eglContext = EGL_NO_CONTEXT;
		}

		if (_eglSurface != EGL_NO_SURFACE) {
			eglDestroySurface(_eglDisplay, _eglSurface);
			_eglSurface = EGL_NO_SURFACE;
		}

		eglTerminate(_eglDisplay);
		_eglDisplay = EGL_NO_DISPLAY;
	}

	_eglConfig = NULL;
	OpenGLGraphicsManager::unloadGFXMode();
	logLeaving();
}
