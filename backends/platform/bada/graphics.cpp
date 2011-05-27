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
  pixmapSurface(EGL_NO_SURFACE),
  pBitmap(null) {
  assert(appForm != null);
  _videoMode.fullscreen = true;
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
  logEntered();
  switch (f) {
  case OSystem::kFeatureVirtualKeyboard:
    // TODO
    // http://forums.badadev.com/viewtopic.php?f=6&t=258
    break;
  default:
    OpenGLGraphicsManager::setFeatureState(f, enable);
    break;
  }
}

void BadaGraphicsManager::setInternalMousePosition(int x, int y) {
  logEntered();

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
    systemError("eglGetDisplay() failed. [0x%x]\n", eglGetError());
    return false;
  }
  
  if (EGL_FALSE == eglInitialize(eglDisplay, null, null) || 
      EGL_SUCCESS != eglGetError()) {
    systemError("eglInitialize() failed. [0x%x]\n", eglGetError());
    return false;
  }

  if (EGL_FALSE == eglChooseConfig(eglDisplay, eglConfigList, 
                                   &eglConfig, 1, &numConfigs) ||
      EGL_SUCCESS != eglGetError()) {
    systemError("eglChooseConfig() failed. [0x%x]\n", eglGetError());
    return false;
  }

  if (!numConfigs) {
    systemError("eglChooseConfig() failed. Matching config does not exist \n");
    return false;
  }

  eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, 
                                      (EGLNativeWindowType)appForm, null);
  if (EGL_NO_SURFACE == eglSurface || EGL_SUCCESS != eglGetError()) {
    systemError("eglCreateWindowSurface() failed. EGL_NO_SURFACE [0x%x]\n", 
                eglGetError());
    return false;
  }
  
  eglContext = eglCreateContext(eglDisplay, eglConfig, 
                                EGL_NO_CONTEXT, eglContextList);
  if (EGL_NO_CONTEXT == eglContext ||
      EGL_SUCCESS != eglGetError()) {
    systemError("eglCreateContext() failed. [0x%x]\n", eglGetError());
    return false;
  }

  if (false == eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) ||
      EGL_SUCCESS != eglGetError()) {
    systemError("eglMakeCurrent() failed. [0x%x]\n", eglGetError());
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

  EGLint surfaceType;
  eglGetConfigAttrib(eglDisplay, eglConfig, EGL_SURFACE_TYPE, &surfaceType);

  if ((surfaceType & EGL_PIXMAP_BIT) > 0) {
    // can also just draw directly to the eglSurface so not 
    // sure what the advantage of using this is
    pBitmap = new Bitmap();
    result r = pBitmap->Construct(Osp::Graphics::Rectangle(0, 0, width, height));
    if (!IsFailed(r)) {
      pixmapSurface = eglCreatePixmapSurface(eglDisplay, eglConfig, (NativePixmapType)pBitmap, null);
      if (pixmapSurface == EGL_NO_SURFACE) {
        delete pBitmap;
        pBitmap = null;
      }
    }
  }

  assert(pixmapSurface != EGL_NO_SURFACE);

  _videoMode.screenWidth = _videoMode.overlayWidth = _videoMode.hardwareWidth = width;
  _videoMode.screenHeight = _videoMode.overlayHeight = _videoMode.hardwareHeight = height;
  AppLog("screen size: %dx%d", _videoMode.screenWidth, _videoMode.screenHeight);

  _screenFormat = Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0); 
  _videoMode.format = _screenFormat; // RGBA444

  return OpenGLGraphicsManager::loadGFXMode();
}

void BadaGraphicsManager::internUpdateScreen() {
  logEntered();
  eglMakeCurrent(eglDisplay, pixmapSurface, pixmapSurface, eglContext);
  OpenGLGraphicsManager::internUpdateScreen();

  // these might not be needed
  glFlush();
  glFinish();
  eglSwapBuffers(eglDisplay, eglSurface);

  eglUpdateBufferOSP(eglDisplay, pixmapSurface);
  if (pBitmap) {
    Rectangle dstRect(0, 0, _videoMode.screenWidth, _videoMode.screenHeight);
    Canvas canvas;
    canvas.Construct();
    canvas.DrawBitmap(dstRect, *pBitmap);
    canvas.Show();
  }
}

void BadaGraphicsManager::unloadGFXMode() {
  logEntered();

  if (pBitmap) {
    delete pBitmap;
    pBitmap = null;
  }

  if (EGL_NO_DISPLAY != eglDisplay) {
    eglMakeCurrent(eglDisplay, null, null, null);

    if (pixmapSurface != EGL_NO_SURFACE) {
      eglDestroySurface(eglDisplay, pixmapSurface);
      pixmapSurface = EGL_NO_SURFACE;
    }

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

//
// end of graphics.cpp 
//
