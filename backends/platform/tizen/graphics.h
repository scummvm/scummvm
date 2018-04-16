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

#ifndef TIZEN_GRAPHICS_H
#define TIZEN_GRAPHICS_H

#include <FBase.h>
#include <FGraphics.h>
#include <FApp.h>
#include <FGraphicsOpengl.h>
#include <FSystem.h>
#include <FUi.h>

#include "config.h"
#include "backends/graphics/opengl/opengl-graphics.h"
#include "graphics/font.h"
#include "backends/platform/tizen/form.h"

using namespace Tizen::Graphics;
using namespace Tizen::Graphics::Opengl;
using namespace Tizen::App;

class TizenGraphicsManager : public OpenGL::OpenGLGraphicsManager {
public:
	TizenGraphicsManager(TizenAppForm *appForm);
	virtual ~TizenGraphicsManager();

	result Construct();

	Common::List<Graphics::PixelFormat> getSupportedFormats() const;
	bool hasFeature(OSystem::Feature f);
	void setFeatureState(OSystem::Feature f, bool enable);
	void updateScreen();

	void setReady();
	bool isReady() { return !_initState; }

	bool moveMouse(int16 &x, int16 &y);

protected:
	void setInternalMousePosition(int x, int y) {}

	bool loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format);

	void refreshScreen();

	void *getProcAddress(const char *name) const;

	const Graphics::Font *getFontOSD();

private:
	bool loadEgl();
	TizenAppForm *_appForm;
	EGLDisplay _eglDisplay;
	EGLSurface _eglSurface;
	EGLConfig	 _eglConfig;
	EGLContext _eglContext;
	bool _initState;
};

#endif
