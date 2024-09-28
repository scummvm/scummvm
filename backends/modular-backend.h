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

#ifndef BACKENDS_MODULAR_BACKEND_H
#define BACKENDS_MODULAR_BACKEND_H

#include "backends/base-backend.h"

class GraphicsManager;
class MixerManager;

/**
 * Base classes for modular backends.
 *
 * They wrap most functions to their manager equivalent, but not
 * all OSystem functions are implemented here.
 *
 * A backend derivated from these classes, will need to implement
 * these functions on its own:
 *   OSystem::pollEvent()
 *   OSystem::createMutex()
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

	bool hasFeature(Feature f) override;
	void setFeatureState(Feature f, bool enable) override;
	bool getFeatureState(Feature f) override;

	//@}

	/** @name Graphics */
	//@{

	GraphicsManager *getGraphicsManager();
	const GraphicsMode *getSupportedGraphicsModes() const override;
	int getDefaultGraphicsMode() const override;
	bool setGraphicsMode(int mode, uint flags = kGfxModeNoFlags) override;
	int getGraphicsMode() const override;
#if defined(USE_IMGUI)
	void setImGuiCallbacks(const ImGuiCallbacks &callbacks) override final;
	void *getImGuiTexture(const Graphics::Surface &image, const byte *palette, int palCount) override final;
	void freeImGuiTexture(void *texture) override final;
#endif
	bool setShader(const Common::Path &name) override final;
	const GraphicsMode *getSupportedStretchModes() const override final;
	int getDefaultStretchMode() const override final;
	bool setStretchMode(int mode) override final;
	int getStretchMode() const override final;
	uint getDefaultScaler() const override final;
	uint getDefaultScaleFactor() const override final;
	using BaseBackend::setScaler;
	bool setScaler(uint mode, int factor) override final;
	uint getScaler() const override final;
	uint getScaleFactor() const override final;
#ifdef USE_RGB_COLOR
	Graphics::PixelFormat getScreenFormat() const override final;
	Common::List<Graphics::PixelFormat> getSupportedFormats() const override final;
#endif
	void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL) override final;
	void initSizeHint(const Graphics::ModeList &modes) override final;
	int getScreenChangeID() const override final;

	void beginGFXTransaction() override final;
	OSystem::TransactionError endGFXTransaction() override final;

	int16 getHeight() override final;
	int16 getWidth() override final;
	PaletteManager *getPaletteManager() override final;
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override final;
	Graphics::Surface *lockScreen() override final;
	void unlockScreen() override final;
	void fillScreen(uint32 col) override final;
	void fillScreen(const Common::Rect &r, uint32 col) override final;
	void updateScreen() override final;
	void setShakePos(int shakeXOffset, int shakeYOffset) override final;
	void setFocusRectangle(const Common::Rect& rect) override final;
	void clearFocusRectangle() override final;

	void showOverlay(bool inGUI) override final;
	void hideOverlay() override final;
	bool isOverlayVisible() const override final;
	Graphics::PixelFormat getOverlayFormat() const override final;
	void clearOverlay() override final;
	void grabOverlay(Graphics::Surface &surface) override final;
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override final;
	int16 getOverlayHeight() override final;
	int16 getOverlayWidth() override final;

	float getHiDPIScreenFactor() const override final;

	bool showMouse(bool visible) override final;
	void warpMouse(int x, int y) override final;
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL, const byte *mask = NULL) override final;
	void setCursorPalette(const byte *colors, uint start, uint num) override final;
	bool lockMouse(bool lock) override final;

	//@}

	/** @name Miscellaneous */
	//@{

	void displayMessageOnOSD(const Common::U32String &msg) override final;
	void displayActivityIconOnOSD(const Graphics::Surface *icon) override final;

	void saveScreenshot() override final;

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
	Audio::Mixer *getMixer() override final;

	//@}

protected:
	/** @name Managers variables */
	//@{

	MixerManager *_mixerManager;

	//@}
};

#endif
