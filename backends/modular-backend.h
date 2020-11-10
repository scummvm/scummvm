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
class MixerManager;
class MutexManager;

/**
 * Base classes for modular backends.
 *
 * They wrap most functions to their manager equivalent, but not
 * all OSystem functions are implemented here.
 *
 * A backend derivated from these classes, will need to implement
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
class ModularGraphicsBackend : virtual public BaseBackend {
public:
	ModularGraphicsBackend();
	virtual ~ModularGraphicsBackend();

	/** @name Features */
	//@{

	virtual bool hasFeature(Feature f) override;
	virtual void setFeatureState(Feature f, bool enable) override;
	virtual bool getFeatureState(Feature f) override;

	//@}

	/** @name Graphics */
	//@{

	GraphicsManager *getGraphicsManager();
	virtual const GraphicsMode *getSupportedGraphicsModes() const override;
	virtual int getDefaultGraphicsMode() const override;
	virtual bool setGraphicsMode(int mode, uint flags = kGfxModeNoFlags) override;
	virtual int getGraphicsMode() const override;
	virtual const GraphicsMode *getSupportedShaders() const override final;
	virtual int getDefaultShader() const override final;
	virtual int getShader() const override final;
	virtual bool setShader(int id) override final;
	virtual const GraphicsMode *getSupportedStretchModes() const override final;
	virtual int getDefaultStretchMode() const override final;
	virtual bool setStretchMode(int mode) override final;
	virtual int getStretchMode() const override final;
#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const override final;
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const override final;
#endif
	virtual void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL) override final;
	virtual void initSizeHint(const Graphics::ModeList &modes) override final;
	virtual int getScreenChangeID() const override final;

	virtual void beginGFXTransaction() override final;
	virtual OSystem::TransactionError endGFXTransaction() override final;

	virtual int16 getHeight() override final;
	virtual int16 getWidth() override final;
	virtual PaletteManager *getPaletteManager() override final;
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override final;
	virtual Graphics::Surface *lockScreen() override final;
	virtual void unlockScreen() override final;
	virtual void fillScreen(uint32 col) override final;
	virtual void updateScreen() override final;
	virtual void setShakePos(int shakeXOffset, int shakeYOffset) override final;
	virtual void setFocusRectangle(const Common::Rect& rect) override final;
	virtual void clearFocusRectangle() override final;

	virtual void showOverlay() override final;
	virtual void hideOverlay() override final;
	virtual bool isOverlayVisible() const override final;
	virtual Graphics::PixelFormat getOverlayFormat() const override final;
	virtual void clearOverlay() override final;
	virtual void grabOverlay(void *buf, int pitch) override final;
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override final;
	virtual int16 getOverlayHeight() override final;
	virtual int16 getOverlayWidth() override final;

	virtual bool showMouse(bool visible) override final;
	virtual void warpMouse(int x, int y) override final;
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL) override final;
	virtual void setCursorPalette(const byte *colors, uint start, uint num) override final;
	virtual bool lockMouse(bool lock) override final;

	//@}

	/** @name Miscellaneous */
	//@{

	virtual void displayMessageOnOSD(const Common::U32String &msg) override final;
	virtual void displayActivityIconOnOSD(const Graphics::Surface *icon) override final;

	virtual void saveScreenshot() override final;

	//@}

protected:
	/** @name Managers variables */
	//@{

	GraphicsManager *_graphicsManager;

	//@}
};

class ModularMixerBackend : virtual public BaseBackend {
public:
	ModularMixerBackend();
	virtual ~ModularMixerBackend();

	/** @name Sound */
	//@{

	virtual MixerManager *getMixerManager();
	virtual Audio::Mixer *getMixer() override final;

	//@}

protected:
	/** @name Managers variables */
	//@{

	MixerManager *_mixerManager;

	//@}
};

class ModularMutexBackend : virtual public BaseBackend {
public:
	ModularMutexBackend();
	virtual ~ModularMutexBackend();

	/** @name Mutex handling */
	//@{

	virtual MutexRef createMutex() override final;
	virtual void lockMutex(MutexRef mutex) override final;
	virtual void unlockMutex(MutexRef mutex) override final;
	virtual void deleteMutex(MutexRef mutex) override final;

	//@}

protected:
	/** @name Managers variables */
	//@{

	MutexManager *_mutexManager;

	//@}
};

#endif
