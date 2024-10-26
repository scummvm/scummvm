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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_RENDERER_H
#define WINTERMUTE_BASE_RENDERER_H

#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/base/base.h"

#include "common/rect.h"
#include "common/array.h"

namespace Wintermute {

class BaseImage;
class BaseActiveRect;
class BaseObject;
class BaseSurface;
class BasePersistenceManager;
#ifdef ENABLE_WME3D
class Camera3D;
#endif

/**
 * @class BaseRenderer a common interface for the rendering portion of WME
 * this interface is mainly intended to wrap away any differencies between
 * software-rendering/hardware-rendering.
 */
class BaseRenderer: public BaseClass {
public:
	int _realWidth;
	int _realHeight;
	int _drawOffsetX;
	int _drawOffsetY;

	virtual void dumpData(const char *filename) {}
	/**
	 * Take a screenshot of the current screenstate
	 *
	 * @return a BaseImage containing the current screen-buffer.
	 */
	virtual BaseImage *takeScreenshot() = 0;
	virtual bool saveScreenShot(const Common::String &filename, int sizeX = 0, int sizeY = 0);
	virtual bool setViewport(int left, int top, int right, int bottom);
	virtual bool setViewport(Rect32 *rect);
	virtual bool setScreenViewport();
	virtual void setWindowed(bool windowed) = 0;

	virtual Graphics::PixelFormat getPixelFormat() const = 0;
	/**
	 * Fade the screen to black
	 *
	 * @param alpha amount to fade by (alpha value of black)
	 */
	virtual void fade(uint16 alpha) = 0;
	/**
	 * Fade a portion of the screen to a specific color
	 *
	 * @param r the red component to fade too.
	 * @param g the green component to fade too.
	 * @param b the blue component to fade too.
	 * @param a the alpha component to fade too.
	 * @param rect the portion of the screen to fade (if nullptr, the entire screen will be faded).
	 */
	virtual void fadeToColor(byte r, byte g, byte b, byte a) = 0;

	virtual bool drawLine(int x1, int y1, int x2, int y2, uint32 color); // Unused outside indicator-display
	virtual bool drawRect(int x1, int y1, int x2, int y2, uint32 color, int width = 1); // Unused outside indicator-display
	BaseRenderer(BaseGame *inGame = nullptr);
	~BaseRenderer() override;
	virtual bool setProjection() {
		return STATUS_OK;
	};

	virtual bool windowedBlt();
	/**
	 * Fill a portion of the screen with a specified color
	 *
	 * @param r the red component to fill with.
	 * @param g the green component to fill with.
	 * @param b the blue component to fill with.
	 */
	virtual bool fill(byte r, byte g, byte b, Common::Rect *rect = nullptr) = 0;
	virtual void onWindowChange();
	virtual bool initRenderer(int width, int height, bool windowed);
	/**
	 * Flip the backbuffer onto the screen-buffer
	 * The screen will NOT be updated before calling this function.
	 *
	 * @return true if successful, false on error.
	 */
	virtual bool flip() = 0;
	/**
	 * Special flip for the indicator drawn during save/load
	 * essentially, just copies the region defined by the _indicator-variables.
	 */
	virtual bool indicatorFlip() = 0;
	virtual bool forcedFlip() = 0;
	virtual void initLoop();
	virtual bool setup2D(bool force = false);
#ifdef ENABLE_WME3D
	virtual bool setup3D(Camera3D *camera = nullptr, bool force = false);
#endif
	virtual bool setupLines();

	/**
	 * Get the name of the current renderer
	 *
	 * @return the name of the renderer.
	 */
	virtual Common::String getName() const = 0;
	virtual bool displayDebugInfo() {
		return STATUS_FAILED;
	};
	virtual bool drawShaderQuad() {
		return STATUS_FAILED;
	}

	virtual float getScaleRatioX() const {
		return 1.0f;
	}
	virtual float getScaleRatioY() const {
		return 1.0f;
	}

	/**
	 * Create a Surface fit for use with the renderer.
	 * As diverse implementations of BaseRenderer might have different solutions for storing surfaces
	 * this allows for a common interface for creating surface-handles. (Mostly useful to ease future
	 * implementation of hw-accelerated rendering, or readding 3D-support at some point).
	 *
	 * @return a surface that can be used with this renderer
	 */
	virtual BaseSurface *createSurface() = 0;

	bool clipCursor();
	bool unclipCursor();

	BaseObject *getObjectAt(int x, int y);
	void deleteRectList();

	virtual bool startSpriteBatch() {
		return STATUS_OK;
	};
	virtual bool endSpriteBatch() {
		return STATUS_OK;
	};
	bool pointInViewport(Point32 *P);
	bool _active;
	bool _ready;

	bool isReady() const { return _ready; }
	bool isWindowed() const { return _windowed; }
	int32 getBPP() const { return _bPP; }

	uint32 _window;
	uint32 _forceAlphaColor;

	void addRectToList(BaseActiveRect *rect);

	// Indicator & Save/Load-related functions
	void initIndicator();
	void setIndicatorVal(int value);
	void setIndicator(int width, int height, int x, int y, uint32 color);
	void persistSaveLoadImages(BasePersistenceManager *persistMgr);
	void initSaveLoad(bool isSaving, bool quickSave = false);
	virtual void endSaveLoad();
	void setLoadingScreen(const char *filename, int x, int y);
	void setSaveImage(const char *filename, int x, int y);

	bool displayIndicator();

	int32 getWidth() const { return _width; }
	int32 getHeight() const { return _height; }
protected:
	int32 _height;
	int32 _width;

	bool _windowed;
	int32 _bPP;

	Common::String _loadImageName;
	Common::String _saveImageName;
	int32 _saveImageX;
	int32 _saveImageY;
	int32 _loadImageX;
	int32 _loadImageY;
	BaseSurface *_saveLoadImage;
	bool _hasDrawnSaveLoadImage;

	int32 _indicatorWidthDrawn;
	uint32 _indicatorColor;
	int32 _indicatorX;
	int32 _indicatorY;
	int32 _indicatorWidth;
	int32 _indicatorHeight;
	bool _loadInProgress;
	bool _indicatorDisplay;
	int32 _indicatorProgress;

	uint32 _clipperWindow;

	Rect32 _windowRect;
	Rect32 _viewportRect;
	Rect32 _screenRect;
	Rect32 _monitorRect;
private:
	Common::Array<BaseActiveRect *> _rectList;
	bool displaySaveloadImage();
	bool displaySaveloadLines();
};

BaseRenderer *makeOSystemRenderer(BaseGame *inGame); // Implemented in BRenderSDL.cpp
#ifdef ENABLE_WME3D
class BaseRenderer3D;

BaseRenderer3D *makeOpenGL3DRenderer(BaseGame *inGame);
BaseRenderer3D *makeOpenGL3DShaderRenderer(BaseGame *inGame);
#endif

} // End of namespace Wintermute

#endif
