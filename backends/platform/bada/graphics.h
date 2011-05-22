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

#include "backends/graphics/opengl/opengl-graphics.h"

#include <FGraphicsOpengl.h>
#include <FAppApplication.h>
#include <FUiCtrlForm.h>

using namespace Osp::Graphics::Opengl;
using namespace Osp::App;

#ifndef BADA_GRAPHICS_H
#define BADA_GRAPHICS_H

struct BadaAppForm : public Osp::Ui::Controls::Form {
  BadaAppForm(Osp::App::Application* app) {
    this->app = app;
  }
  
  ~BadaAppForm(void) {}

  result OnDraw(void) {
    if (app) {
      // app->Draw();
    }
    return E_SUCCESS;
  }

private:
  Osp::App::Application* app;
};

struct BadaGraphicsManager : public OpenGLGraphicsManager {
  bool construct(Osp::App::Application* app);

  bool hasFeature(OSystem::Feature f);
  bool isHotkey(const Common::Event &event);
  bool loadGFXMode();
  bool notifyEvent(const Common::Event &event);
  void internUpdateScreen();
  void setFeatureState(OSystem::Feature f, bool enable);
  void setInternalMousePosition(int x, int y);
  void unloadGFXMode();
  void updateScreen();

 private:
  bool loadEgl();
  BadaAppForm* appForm;
  EGLDisplay eglDisplay;
  EGLSurface eglSurface;
  EGLConfig  eglConfig;
  EGLContext eglContext;
	EGLSurface pBufferSurface;
	GLuint     pBufferTexture;
};

#endif
