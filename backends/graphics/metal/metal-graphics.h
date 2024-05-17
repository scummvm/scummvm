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

#ifndef BACKENDS_GRAPHICS_METAL_METAL_GRAPHICS_H
#define BACKENDS_GRAPHICS_METAL_METAL_GRAPHICS_H

#include "backends/graphics/windowed.h"
#include "backends/graphics/metal/framebuffer.h"

namespace MTL {
class CommandQueue;
class Device;
}

enum {
	GFX_METAL = 0
};

namespace Metal {

class Surface;
class Renderer;

class MetalGraphicsManager : virtual public WindowedGraphicsManager {
public:
	MetalGraphicsManager();
	virtual ~MetalGraphicsManager();

protected:
	// Windowed
	bool gameNeedsAspectRatioCorrection() const override;
	void handleResizeImpl(const int width, const int height) override;

	// GraphicsManager
	bool hasFeature(OSystem::Feature f) const override;
	void setFeatureState(OSystem::Feature f, bool enable) override;
	bool getFeatureState(OSystem::Feature f) const override;

	const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;

#ifdef USE_RGB_COLOR
	Graphics::PixelFormat getScreenFormat() const override;
	Common::List<Graphics::PixelFormat> getSupportedFormats() const override;
#endif

	const OSystem::GraphicsMode *getSupportedStretchModes() const override;
	int getDefaultStretchMode() const override;
	bool setStretchMode(int mode) override;
	int getStretchMode() const override;

#ifdef USE_SCALERS
	uint getDefaultScaler() const override;
	uint getDefaultScaleFactor() const override;
	bool setScaler(uint mode, int factor) override;
	uint getScaler() const override;
	uint getScaleFactor() const override;
#endif

	void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL) override;
	int getScreenChangeID() const override;

	void beginGFXTransaction() override;
	OSystem::TransactionError endGFXTransaction() override;

	int16 getHeight() const override;
	int16 getWidth() const override;

	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override;
	Graphics::Surface *lockScreen() override;
	void unlockScreen() override;
	void fillScreen(uint32 col) override;
	void fillScreen(const Common::Rect &r, uint32 col) override;
	void updateScreen() override;
	void setFocusRectangle(const Common::Rect& rect) override;
	void clearFocusRectangle() override;

	Graphics::PixelFormat getOverlayFormat() const override;
	void clearOverlay() override;
	void grabOverlay(Graphics::Surface &surface) const override;
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;
	int16 getOverlayHeight() const override;
	int16 getOverlayWidth() const override;

	bool showMouse(bool visible) override;
	void warpMouse(int x, int y) override;
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = nullptr, const byte *mask = nullptr) override;
	void setCursorPalette(const byte *colors, uint start, uint num) override;

	// PaletteManager
	void setPalette(const byte *colors, uint start, uint num) override;
	void grabPalette(byte *colors, uint start, uint num) const override;

protected:
	void renderCursor();

	/**
	 * Notify the manager of a Metal context change. This should be the first
	 * thing to call after you created a Metal context!
	 *
	 * @param commandQueue The Metal command queue
	 * @param target The framebuffer target
	 * @param defaultFormat The new default format for the game screen
	 *                     (this is used for the CLUT8 game screens).
	 * @param defaultFormatAlpha The new default format with an alpha channel
	 *                           (this is used for the overlay and cursor).
	 */
	void notifyContextCreate(
			MTL::CommandQueue *commandQueue,
			Framebuffer *target,
			const Graphics::PixelFormat &defaultFormat,
			const Graphics::PixelFormat &defaultFormatAlpha);

	/**
	 * Notify the manager that the Metal context is about to be destroyed.
	 * This will free up/reset internal Metal related state and *must* be
	 * called whenever a context might be created again after destroying a
	 * context.
	 */
	void notifyContextDestroy();

	/**
	 * Create a surface with the specified pixel format.
	 *
	 * @param format The pixel format the Surface object should accept as
	 *                   input.
	 * @param wantAlpha  For CLUT8 surfaces this marks whether an alpha
	 *                   channel should be used.
	 * @param wantScaler Whether or not a software scaler should be used.
	 * @return A pointer to the surface or nullptr on failure.
	 */
	Surface *createSurface(const Graphics::PixelFormat &format, bool wantAlpha = false, bool wantScaler = false, bool wantMask = false);

	//
	// Transaction support
	//
	struct VideoState {
		VideoState() : valid(false), gameWidth(0), gameHeight(0),
#ifdef USE_RGB_COLOR
			gameFormat(),
#endif
			aspectRatioCorrection(false), graphicsMode(GFX_METAL), filtering(true),
			scalerIndex(0), scaleFactor(1), shader() {
		}

		bool valid;

		uint gameWidth, gameHeight;
#ifdef USE_RGB_COLOR
		Graphics::PixelFormat gameFormat;
#endif
		bool aspectRatioCorrection;
		int graphicsMode;
		bool filtering;

		uint scalerIndex;
		int scaleFactor;

		Common::Path shader;

		bool operator==(const VideoState &right) {
			return gameWidth == right.gameWidth && gameHeight == right.gameHeight
#ifdef USE_RGB_COLOR
				&& gameFormat == right.gameFormat
#endif
				&& aspectRatioCorrection == right.aspectRatioCorrection
				&& graphicsMode == right.graphicsMode
				&& filtering == right.filtering
				&& shader == right.shader;
		}

		bool operator!=(const VideoState &right) {
			return !(*this == right);
		}
	};

	/**
	 * The currently set up video state.
	 */
	VideoState _currentState;

	/**
	 * The old video state used when doing a transaction rollback.
	 */
	VideoState _oldState;

protected:
	enum TransactionMode {
		kTransactionNone = 0,
		kTransactionActive = 1,
		kTransactionRollback = 2
	};

	TransactionMode getTransactionMode() const { return _transactionMode; }

private:
	/**
	 * The current transaction mode.
	 */
	TransactionMode _transactionMode;

	/**
	 * The current screen change ID.
	 */
	int _screenChangeID;

	/**
	 * The current stretch mode.
	 */
	int _stretchMode;

	/**
	 * Scaled version of _gameScreenShakeXOffset and _gameScreenShakeYOffset (as a Common::Point)
	 */
	Common::Point _shakeOffsetScaled;

protected:
	/**
	 * Refresh the screen contents.
	 */
	virtual void refreshScreen() = 0;

	/**
	 * Saves a screenshot of the entire window, excluding window decorations.
	 *
	 * @param filename The output filename.
	 * @return true on success, false otherwise
	 */
	bool saveScreenshot(const Common::Path &filename) const;

	// Do not hide the argument-less saveScreenshot from the base class
	using WindowedGraphicsManager::saveScreenshot;

private:
	/**
	 * Metal pipeline used for rendering.
	 */
	Pipeline *_pipeline;

	/**
	 * Metal renderer doing all the drawings
	 */
	Renderer *_renderer;

	/**
	 * The system Metal device
	 */
	MTL::Device *_device;

	/**
	 * The command queue to put GPU tasks on
	 */
	MTL::CommandQueue *_commandQueue;

protected:
	/**
	 * Try to determine the internal parameters for a given pixel format.
	 *
	 * @return true when the format can be used, false otherwise.
	 */
	bool getMetalPixelFormat(const Graphics::PixelFormat &pixelFormat, uint &metalPixelFormat) const;

	int getGameRenderScale() const override;
	void recalculateDisplayAreas() override;

	void updateLinearFiltering();

	Pipeline *getPipeline() const { return _pipeline; }

	/**
	 * The default pixel format of the backend.
	 */
	Graphics::PixelFormat _defaultFormat;

	/**
	 * The default pixel format with an alpha channel.
	 */
	Graphics::PixelFormat _defaultFormatAlpha;

	/**
	 * Render target.
	 */
	Framebuffer *_targetBuffer;

	/**
	 * The rendering surface for the virtual game screen.
	 */
	Surface *_gameScreen;

	/**
	 * The game palette if in CLUT8 mode.
	 */
	byte _gamePalette[3 * 256];

	//
	// Overlay
	//

	/**
	 * The rendering surface for the overlay.
	 */
	Surface *_overlay;

	//
	// Cursor
	//

	/**
	 * Set up the correct cursor palette.
	 */
	void updateCursorPalette();

	/**
	 * The rendering surface for the mouse cursor.
	 */
	Surface *_cursor;

	/**
	 * The rendering surface for the opacity and inversion mask (if any)
	 */
	Surface *_cursorMask;

	/**
	 * The X offset for the cursor hotspot in unscaled game coordinates.
	 */
	int _cursorHotspotX;

	/**
	 * The Y offset for the cursor hotspot in unscaled game coordinates.
	 */
	int _cursorHotspotY;

	/**
	 * Recalculate the cursor scaling. Scaling is always done according to
	 * the game screen.
	 */
	void recalculateCursorScaling();

	/**
	 * The X offset for the cursor hotspot in scaled game display area
	 * coordinates.
	 */
	int _cursorHotspotXScaled;

	/**
	 * The Y offset for the cursor hotspot in scaled game display area
	 * coordinates.
	 */
	int _cursorHotspotYScaled;

	/**
	 * The width of the cursor in scaled game display area coordinates.
	 */
	float _cursorWidthScaled;

	/**
	 * The height of the cursor in scaled game display area coordinates.
	 */
	float _cursorHeightScaled;

	/**
	 * The key color.
	 */
	uint32 _cursorKeyColor;

	/**
	 * If true, use key color.
	 */
	bool _cursorUseKey;

	/**
	 * Whether no cursor scaling should be applied.
	 */
	bool _cursorDontScale;

	/**
	 * Whether the special cursor palette is enabled.
	 */
	bool _cursorPaletteEnabled;

	/**
	 * The special cursor palette in case enabled.
	 */
	byte _cursorPalette[3 * 256];

#ifdef USE_SCALERS
	/**
	 * The list of scaler plugins
	 */
	const PluginList &_scalerPlugins;
#endif
};

} // End of namespace Metal

#endif
