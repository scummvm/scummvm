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

#ifndef BACKENDS_MODULAR_BACKEND_H
#define BACKENDS_MODULAR_BACKEND_H

#include "backends/base-backend.h"

class GraphicsManager;
class MutexManager;

/**
 * Base class for modular backends.
 *
 * It wraps most functions to their manager equivalent, but not
 * all OSystem functions are implemented here.
 *
 * A backend derivated from this class, will need to implement
 * these functions on its own:
 *   OSystem::pollEvent()
 *   OSystem::getMillis()
 *   OSystem::delayMillis()
 *   OSystem::getTimeAndDate()
 *   OSystem::quit()
 *
 * And, it should also initialize all the managers variables
 * declared in this class, or override their related functions.
 */
class ModularBackend : public BaseBackend {
public:
	ModularBackend();
	virtual ~ModularBackend();

	/** @name Features */
	//@{

	virtual bool hasFeature(Feature f) override;
	virtual void setFeatureState(Feature f, bool enable) override;
	virtual bool getFeatureState(Feature f) override;

	//@}

	/** @name Graphics */
	//@{

	virtual GraphicsManager *getGraphicsManager();
	virtual const GraphicsMode *getSupportedGraphicsModes() const override;
	virtual int getDefaultGraphicsMode() const override;
	virtual bool setGraphicsMode(int mode) override;
	virtual int getGraphicsMode() const override;
	virtual const GraphicsMode *getSupportedShaders() const override;
	virtual int getShader() const override;
	virtual bool setShader(int id) override;
	virtual const GraphicsMode *getSupportedStretchModes() const override;
	virtual int getDefaultStretchMode() const override;
	virtual bool setStretchMode(int mode) override;
	virtual int getStretchMode() const override;
	virtual void resetGraphicsScale() override;
#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const override;
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const override;
#endif
	virtual void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL) override;
	virtual void initSizeHint(const Graphics::ModeList &modes) override;
	virtual int getScreenChangeID() const override;

	virtual void beginGFXTransaction() override;
	virtual OSystem::TransactionError endGFXTransaction() override;

	virtual int16 getHeight() override;
	virtual int16 getWidth() override;
	virtual PaletteManager *getPaletteManager() override;
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override;
	virtual Graphics::Surface *lockScreen() override;
	virtual void unlockScreen() override;
	virtual void fillScreen(uint32 col) override;
	virtual void updateScreen() override;
	virtual void setShakePos(int shakeXOffset, int shakeYOffset) override;
	virtual void setFocusRectangle(const Common::Rect& rect) override;
	virtual void clearFocusRectangle() override;

	virtual void showOverlay() override;
	virtual void hideOverlay() override;
	virtual Graphics::PixelFormat getOverlayFormat() const override;
	virtual void clearOverlay() override;
	virtual void grabOverlay(void *buf, int pitch) override;
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;
	virtual int16 getOverlayHeight() override;
	virtual int16 getOverlayWidth() override;

	virtual bool showMouse(bool visible) override;
	virtual void warpMouse(int x, int y) override;
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL) override;
	virtual void setCursorPalette(const byte *colors, uint start, uint num) override;

	//@}

	/** @name Mutex handling */
	//@{

	virtual MutexRef createMutex() override;
	virtual void lockMutex(MutexRef mutex) override;
	virtual void unlockMutex(MutexRef mutex) override;
	virtual void deleteMutex(MutexRef mutex) override;

	//@}

	/** @name Sound */
	//@{

	virtual Audio::Mixer *getMixer() override;

	//@}

	/** @name Miscellaneous */
	//@{

	virtual void displayMessageOnOSD(const char *msg) override;
	virtual void displayActivityIconOnOSD(const Graphics::Surface *icon) override;

	//@}

protected:
	/** @name Managers variables */
	//@{

	MutexManager *_mutexManager;
	GraphicsManager *_graphicsManager;
	Audio::Mixer *_mixer;

	//@}
};

#endif
