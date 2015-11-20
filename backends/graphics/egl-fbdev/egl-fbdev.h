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

#ifndef BACKENDS_GRAPHICS_EGL_FBDEV_GRAPHICS_H
#define BACKENDS_GRAPHICS_EGL_FBDEV_GRAPHICS_H

#include "backends/graphics/opengl/opengl-graphics.h"
#include "backends/graphics/sdl/sdl-graphics.h"
#include "backends/platform/sdl/sdl-sys.h"

#include "common/array.h"
#include "common/events.h"

#include <GLES/gl.h>
#include <EGL/egl.h>

class EGLFBDEVGraphicsManager : public OpenGL::OpenGLGraphicsManager, public SdlGraphicsManager, public Common::EventObserver {
public:
	EGLFBDEVGraphicsManager(SdlEventSource *eventSource);
	virtual ~EGLFBDEVGraphicsManager();

	// GraphicsManager API
	virtual void activateManager();
	virtual void deactivateManager();

	virtual void initEGL();
	virtual void deinitEGL();

	virtual bool hasFeature(OSystem::Feature f);
	virtual void setFeatureState(OSystem::Feature f, bool enable);
	virtual bool getFeatureState(OSystem::Feature f);

	virtual bool setGraphicsMode(int mode);
	virtual void resetGraphicsScale();

#ifdef USE_RGB_COLOR
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const;
#endif

	virtual void updateScreen();

	// EventObserver API
	virtual bool notifyEvent(const Common::Event &event);

	// SdlGraphicsManager API
	virtual void notifyVideoExpose();
	virtual void notifyResize(const uint width, const uint height);
	virtual void transformMouseCoordinates(Common::Point &point);
	virtual void notifyMousePos(Common::Point mouse);

protected:
	virtual void setInternalMousePosition(int x, int y);

	virtual bool loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format);
private:
	bool setupMode(uint width, uint height);

	SDL_Surface *_hwScreen;

	bool _gotResize;

	struct VideoMode {
		VideoMode() : width(0), height(0) {}
		VideoMode(uint w, uint h) : width(w), height(h) {}

		bool operator<(const VideoMode &right) const {
			if (width < right.width) {
				return true;
			} else if (width == right.width && height < right.height) {
				return true;
			} else {
				return false;
			}
		}

		bool operator==(const VideoMode &right) const {
			return width == right.width && height == right.height;
		}

		bool operator!=(const VideoMode &right) const {
			return !(*this == right);
		}

		uint width, height;
	};
	typedef Common::Array<VideoMode> VideoModeArray;

	virtual bool isHotkey(const Common::Event &event);

	struct {
		EGLDisplay display;
		EGLConfig config;
		EGLContext context;
		EGLSurface surface;
		uint width, height, refresh;
	} _eglInfo;

	fbdev_window _nativeWindow;
};

#endif
