/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BACKENDS_GRAPHICS_OPENGLSDL_OPENGLSDL_GRAPHICS_H
#define BACKENDS_GRAPHICS_OPENGLSDL_OPENGLSDL_GRAPHICS_H

#include "backends/graphics/opengl/opengl-graphics.h"
#include "backends/graphics/sdl/sdl-graphics.h"
#include "backends/platform/sdl/sdl-sys.h"

#include "common/array.h"
#include "common/events.h"

class OpenGLSdlGraphicsManager : public OpenGL::OpenGLGraphicsManager, public SdlGraphicsManager {
public:
	OpenGLSdlGraphicsManager(SdlEventSource *eventSource, SdlWindow *window);
	virtual ~OpenGLSdlGraphicsManager();

	bool hasFeature(OSystem::Feature f) const override;
	void setFeatureState(OSystem::Feature f, bool enable) override;
	bool getFeatureState(OSystem::Feature f) const override;

	void initSize(uint w, uint h, const Graphics::PixelFormat *format) override;
	void updateScreen() override;

	float getHiDPIScreenFactor() const override;

	// EventObserver API
	bool notifyEvent(const Common::Event &event) override;

	// SdlGraphicsManager API
	void notifyVideoExpose() override;
	void notifyResize(const int width, const int height) override;

#if defined(USE_IMGUI) && SDL_VERSION_ATLEAST(2, 0, 0)
	void *getImGuiTexture(const Graphics::Surface &image, const byte *palette, int palCount) override;
	void freeImGuiTexture(void *texture) override;
#endif

protected:
	bool loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format) override;

	void refreshScreen() override;

	void handleResizeImpl(const int width, const int height) override;

	bool saveScreenshot(const Common::Path &filename) const override;

private:
	bool setupMode(uint width, uint height);

#if SDL_VERSION_ATLEAST(2, 0, 0)
	int _glContextProfileMask, _glContextMajor, _glContextMinor;

	SDL_GLContext _glContext;
#else
	uint32 _lastVideoModeLoad;
#endif

#ifdef EMSCRIPTEN
	/** 
	 * See https://registry.khronos.org/webgl/specs/latest/1.0/#2 :
	 * " By default, after compositing the contents of the drawing buffer shall be cleared to their default values [...]
	 *   Techniques like synchronous drawing buffer access (e.g., calling readPixels or toDataURL in the same function
	 *   that renders to the drawing buffer) can be used to get the contents of the drawing buffer "
	 * 
	 * This means we need to take the screenshot at the correct time, which we do by queueing taking the screenshot
	 * for the next frame instead of taking it right away.
	 */
	bool _queuedScreenshot = false;
	void saveScreenshot() override;
#endif

	OpenGL::ContextType _glContextType;

	uint _forceFrameUpdate = 0;
	uint _lastRequestedWidth;
	uint _lastRequestedHeight;
	uint _graphicsScale;
	bool _gotResize;

	bool _vsync;
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
};

#endif
