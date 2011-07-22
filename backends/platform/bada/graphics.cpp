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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "form.h"
#include "system.h"
#include "graphics.h"

//
// BadaGraphicsManager
//
BadaGraphicsManager::BadaGraphicsManager(BadaAppForm* appForm) :
  appForm(appForm),
  eglDisplay(EGL_DEFAULT_DISPLAY),
  eglSurface(EGL_NO_SURFACE),
  eglConfig(0),
  eglContext(EGL_NO_CONTEXT),
  initState(true) {
  assert(appForm != null);
  _videoMode.fullscreen = true;
  _videoMode.antialiasing = true;
}

Common::List<Graphics::PixelFormat> BadaGraphicsManager::getSupportedFormats() const {
  logEntered();

  Common::List<Graphics::PixelFormat> res;
  res.push_back(Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0));
  res.push_back(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
  res.push_back(Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0));
  res.push_back(Graphics::PixelFormat::createFormatCLUT8());
  return res;
}

bool BadaGraphicsManager::hasFeature(OSystem::Feature f) {
  bool result = (f == OSystem::kFeatureFullscreenMode || 
                 f == OSystem::kFeatureVirtualKeyboard ||
                 OpenGLGraphicsManager::hasFeature(f));
  return result;
}

void BadaGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
  OpenGLGraphicsManager::setFeatureState(f, enable);
}

void BadaGraphicsManager::setInternalMousePosition(int x, int y) {
}

void BadaGraphicsManager::setReady() {
  if (initState) {
    initEventObserver();
    initState = false;
  }
}

void BadaGraphicsManager::updateScreen() {
  if (_transactionMode == kTransactionNone) {
    internUpdateScreen();
  }
}

// see: http://forums.badadev.com/viewtopic.php?f=7&t=208
bool BadaGraphicsManager::loadEgl() {
  logEntered();

  EGLint numConfigs = 1;
  EGLint eglConfigList[] = {
    EGL_RED_SIZE,   5,
    EGL_GREEN_SIZE, 6,
    EGL_BLUE_SIZE,  5,
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

  if (eglDisplay) {
    unloadGFXMode();
  }

  eglDisplay = eglGetDisplay((EGLNativeDisplayType) EGL_DEFAULT_DISPLAY);
  if (EGL_NO_DISPLAY == eglDisplay) {
    systemError("eglGetDisplay() failed");
    return false;
  }
  
  if (EGL_FALSE == eglInitialize(eglDisplay, null, null) || 
      EGL_SUCCESS != eglGetError()) {
    systemError("eglInitialize() failed");
    return false;
  }

  if (EGL_FALSE == eglChooseConfig(eglDisplay, eglConfigList, 
                                   &eglConfig, 1, &numConfigs) ||
      EGL_SUCCESS != eglGetError()) {
    systemError("eglChooseConfig() failed");
    return false;
  }

  if (!numConfigs) {
    systemError("eglChooseConfig() failed. Matching config does not exist \n");
    return false;
  }

  eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, 
                                      (EGLNativeWindowType)appForm, null);
  if (EGL_NO_SURFACE == eglSurface || EGL_SUCCESS != eglGetError()) {
    systemError("eglCreateWindowSurface() failed. EGL_NO_SURFACE");
    return false;
  }
  
  eglContext = eglCreateContext(eglDisplay, eglConfig, 
                                EGL_NO_CONTEXT, eglContextList);
  if (EGL_NO_CONTEXT == eglContext ||
      EGL_SUCCESS != eglGetError()) {
    systemError("eglCreateContext() failed");
    return false;
  }

  if (false == eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) ||
      EGL_SUCCESS != eglGetError()) {
    systemError("eglMakeCurrent() failed");
    return false;
  }

  logLeaving();
  return true;
}

bool BadaGraphicsManager::loadGFXMode() {
  logEntered();

  if (!loadEgl()) {
    unloadGFXMode();
    return false;
  }

  int x, y, width, height;
  appForm->GetBounds(x, y, width, height);
  _videoMode.overlayWidth = _videoMode.hardwareWidth = width;
  _videoMode.overlayHeight = _videoMode.hardwareHeight = height;
  _videoMode.scaleFactor = 3; // for proportional sized cursor in the launcher

  AppLog("screen size: %dx%d", _videoMode.hardwareWidth, _videoMode.hardwareHeight);
  return OpenGLGraphicsManager::loadGFXMode();
}

void BadaGraphicsManager::internUpdateScreen() {
  if (!initState) {
    OpenGLGraphicsManager::internUpdateScreen();
    eglSwapBuffers(eglDisplay, eglSurface);
  }
  else {
    showSplash();
  }
}

void BadaGraphicsManager::unloadGFXMode() {
  logEntered();

  if (EGL_NO_DISPLAY != eglDisplay) {
    eglMakeCurrent(eglDisplay, null, null, null);

    if (eglContext != EGL_NO_CONTEXT) {
      eglDestroyContext(eglDisplay, eglContext);
      eglContext = EGL_NO_CONTEXT;
    }

    if (eglSurface != EGL_NO_SURFACE) {
      eglDestroySurface(eglDisplay, eglSurface);
      eglSurface = EGL_NO_SURFACE;
    }

    eglTerminate(eglDisplay);
    eglDisplay = EGL_NO_DISPLAY;
  }
  
  eglConfig = null;

  OpenGLGraphicsManager::unloadGFXMode();
  logLeaving();
}

void BadaGraphicsManager::refreshGameScreen() {
	if (_screenNeedsRedraw)
		_screenDirtyRect = Common::Rect(0, 0, _screenData.w, _screenData.h);

	int x = _screenDirtyRect.left;
	int y = _screenDirtyRect.top;
	int w = _screenDirtyRect.width();
	int h = _screenDirtyRect.height();

	if (_screenData.format.bytesPerPixel == 1) {
		// Create a temporary RGB888 surface
    int sw = w;
    int sh = h;

    if (_videoMode.screenWidth == w && _videoMode.screenHeight == h) {
      // The extra border prevents random pixels from appearing in the right and bottom
      // screen column/row. Not sure whether this should be applied to opengl-graphics.cpp
      sw = w + 1;
      sh = h + 1;
    }

    byte *surface = new byte[sw * sh * 3];
    memset(surface, 0, sw * sh * 3);

		// Convert the paletted buffer to RGB888
		const byte *src = (byte *)_screenData.pixels + y * _screenData.pitch;
		src += x * _screenData.format.bytesPerPixel;
		byte *dst = surface;
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				dst[0] = _gamePalette[src[j] * 3];
				dst[1] = _gamePalette[src[j] * 3 + 1];
				dst[2] = _gamePalette[src[j] * 3 + 2];
				dst += 3;
			}
			src += _screenData.pitch;
		}

		// Update the texture
		_gameTexture->updateBuffer(surface, w * 3, x, y, sw, sh);

		// Free the temp surface
		delete[] surface;
	} else {
		// Update the texture
		_gameTexture->updateBuffer((byte *)_screenData.pixels + y * _screenData.pitch +
			x * _screenData.format.bytesPerPixel, _screenData.pitch, x, y, w, h);
	}

	_screenNeedsRedraw = false;
	_screenDirtyRect = Common::Rect();
}

// display a simple splash screen until launcher is ready
void BadaGraphicsManager::showSplash() {
  Canvas canvas;
  canvas.Construct();
  canvas.SetBackgroundColor(Color::COLOR_BLACK);
  canvas.Clear();
  
  int x = _videoMode.hardwareWidth / 3;
  int y = _videoMode.hardwareHeight / 3;

  Font* pFont = new Font();
  pFont->Construct(FONT_STYLE_ITALIC | FONT_STYLE_BOLD, 55);
  canvas.SetFont(*pFont);
  canvas.SetForegroundColor(Color::COLOR_GREEN);
  canvas.DrawText(Point(x, y), L"ScummVM");
  delete pFont;

  pFont = new Font();
  pFont->Construct(FONT_STYLE_ITALIC | FONT_STYLE_BOLD, 35);
  canvas.SetFont(*pFont);
  canvas.SetForegroundColor(Color::COLOR_WHITE);
  canvas.DrawText(Point(x + 70, y + 50), L"Loading ...");
  delete pFont;
  
  canvas.Show();

}

//
// end of graphics.cpp 
//
