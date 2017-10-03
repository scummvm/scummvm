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

#ifndef BACKENDS_GRAPHICS_OPENGLSDL_OPENGLSDL_GRAPHICS_H
#define BACKENDS_GRAPHICS_OPENGLSDL_OPENGLSDL_GRAPHICS_H

#include "backends/graphics/opengl/opengl-graphics.h"
#include "backends/graphics/sdl/sdl-graphics.h"
#include "backends/platform/sdl/sdl-sys.h"

#include "common/array.h"
#include "common/events.h"

class OpenGLSdlGraphicsManager : public OpenGL::OpenGLGraphicsManager, public SdlGraphicsManager, public Common::EventObserver {
public:
	OpenGLSdlGraphicsManager(uint desktopWidth, uint desktopHeight, SdlEventSource *eventSource, SdlWindow *window);
	virtual ~OpenGLSdlGraphicsManager();

	// GraphicsManager API
	virtual void activateManager();
	virtual void deactivateManager();

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

	virtual void refreshScreen();

	virtual void *getProcAddress(const char *name) const;
private:
	bool setupMode(uint width, uint height);

#if SDL_VERSION_ATLEAST(2, 0, 0)
	int _glContextProfileMask, _glContextMajor, _glContextMinor;
	SDL_GLContext _glContext;
#else
	uint32 _lastVideoModeLoad;
	SDL_Surface *_hwScreen;
#endif

	void getWindowDimensions(int *width, int *height);

	uint _forceFrameUpdate;
	uint _lastRequestedWidth;
	uint _lastRequestedHeight;
	uint _graphicsScale;
	bool _ignoreLoadVideoMode;
	bool _gotResize;

	bool _wantsFullScreen;
	uint _ignoreResizeEvents;

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
	VideoModeArray _fullscreenVideoModes;

	uint _desiredFullscreenWidth;
	uint _desiredFullscreenHeight;

	virtual bool isHotkey(const Common::Event &event);
};

#endif
