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

#ifndef BACKENDS_GRAPHICS_NULL_H
#define BACKENDS_GRAPHICS_NULL_H

#include "backends/graphics/graphics.h"

static const OSystem::GraphicsMode s_noGraphicsModes[] = { {0, 0, 0} };

class NullGraphicsManager : public GraphicsManager {
public:
	virtual ~NullGraphicsManager() {}

	bool hasFeature(OSystem::Feature f) const override { return false; }
	void setFeatureState(OSystem::Feature f, bool enable) override {}
	bool getFeatureState(OSystem::Feature f) const override { return false; }

	const OSystem::GraphicsMode *getSupportedGraphicsModes() const override { return s_noGraphicsModes; }
	int getDefaultGraphicsMode() const override { return 0; }
	bool setGraphicsMode(int mode) override { return true; }
	void resetGraphicsScale() override {}
	int getGraphicsMode() const override { return 0; }
	inline Graphics::PixelFormat getScreenFormat() const override {
		return Graphics::PixelFormat::createFormatCLUT8();
	}
	inline Common::List<Graphics::PixelFormat> getSupportedFormats() const override {
		Common::List<Graphics::PixelFormat> list;
		list.push_back(Graphics::PixelFormat::createFormatCLUT8());
		return list;
	}
	void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL) override {}
	virtual int getScreenChangeID() const override { return 0; }

	void beginGFXTransaction() override {}
	OSystem::TransactionError endGFXTransaction() override { return OSystem::kTransactionSuccess; }

	int16 getHeight() const override { return 0; }
	int16 getWidth() const override { return 0; }
	void setPalette(const byte *colors, uint start, uint num) override {}
	void grabPalette(byte *colors, uint start, uint num) const override {}
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override {}
	Graphics::Surface *lockScreen() override { return NULL; }
	void unlockScreen() override {}
	void fillScreen(uint32 col) override {}
	void updateScreen() override {}
	void setShakePos(int shakeXOffset, int shakeYOffset) override {}
	void setFocusRectangle(const Common::Rect& rect) override {}
	void clearFocusRectangle() override {}

	void showOverlay() override {}
	void hideOverlay() override {}
	Graphics::PixelFormat getOverlayFormat() const override { return Graphics::PixelFormat(); }
	void clearOverlay() override {}
	void grabOverlay(void *buf, int pitch) const override {}
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override {}
	int16 getOverlayHeight() const override { return 0; }
	int16 getOverlayWidth() const override { return 0; }

	bool showMouse(bool visible) override { return !visible; }
	void warpMouse(int x, int y) override {}
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL) override {}
	void setCursorPalette(const byte *colors, uint start, uint num) override {}
};

#endif
