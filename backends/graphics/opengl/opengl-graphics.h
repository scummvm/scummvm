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

#ifndef BACKENDS_GRAPHICS_OPENGL_OPENGL_GRAPHICS_H
#define BACKENDS_GRAPHICS_OPENGL_OPENGL_GRAPHICS_H

#include "backends/graphics/opengl/opengl-sys.h"
#include "backends/graphics/opengl/framebuffer.h"
#include "backends/graphics/graphics.h"

#include "common/frac.h"
#include "common/mutex.h"

#include "graphics/surface.h"

namespace Graphics {
class Font;
} // End of namespace Graphics

namespace OpenGL {

// HACK: We use glColor in the OSD code. This might not be working on GL ES but
// we still enable it because Tizen already shipped with it. Also, the
// SurfaceSDL backend enables it and disabling it can cause issues in sdl.cpp.
#define USE_OSD 1

class Surface;
class Pipeline;
#if !USE_FORCED_GLES
class Shader;
#endif

enum {
	GFX_OPENGL = 0
};

class OpenGLGraphicsManager : virtual public GraphicsManager {
public:
	OpenGLGraphicsManager();
	virtual ~OpenGLGraphicsManager();

	// GraphicsManager API
	virtual bool hasFeature(OSystem::Feature f);
	virtual void setFeatureState(OSystem::Feature f, bool enable);
	virtual bool getFeatureState(OSystem::Feature f);

	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;

	virtual void resetGraphicsScale() {}

#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const;
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const = 0;
#endif

	virtual void beginGFXTransaction();
	virtual OSystem::TransactionError endGFXTransaction();

	virtual int getScreenChangeID() const;

	virtual void initSize(uint width, uint height, const Graphics::PixelFormat *format);

	virtual int16 getWidth();
	virtual int16 getHeight();

	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h);
	virtual void fillScreen(uint32 col);

	virtual void setShakePos(int shakeOffset);

	virtual void updateScreen();

	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();

	virtual void setFocusRectangle(const Common::Rect& rect);
	virtual void clearFocusRectangle();

	virtual int16 getOverlayWidth();
	virtual int16 getOverlayHeight();

	virtual void showOverlay();
	virtual void hideOverlay();

	virtual Graphics::PixelFormat getOverlayFormat() const;

	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h);
	virtual void clearOverlay();
	virtual void grabOverlay(void *buf, int pitch);

	virtual bool showMouse(bool visible);
	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format);
	virtual void setCursorPalette(const byte *colors, uint start, uint num);

	virtual void displayMessageOnOSD(const char *msg);
	virtual void displayActivityIconOnOSD(const Graphics::Surface *icon);

	// PaletteManager interface
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void grabPalette(byte *colors, uint start, uint num);

protected:
	/**
	 * Whether an GLES or GLES2 context is active.
	 */
	bool isGLESContext() const { return g_context.type == kContextGLES || g_context.type == kContextGLES2; }

	/**
	 * Set up the actual screen size available for the OpenGL code to do any
	 * drawing.
	 *
	 * @param width  The width of the screen.
	 * @param height The height of the screen.
	 */
	void setActualScreenSize(uint width, uint height);

	/**
	 * Sets the OpenGL (ES) type the graphics manager shall work with.
	 *
	 * This needs to be called at least once (and before ever calling
	 * notifyContextCreate).
	 *
	 * @param type Type of the OpenGL (ES) contexts to be created.
	 */
	void setContextType(ContextType type);

	/**
	 * Notify the manager of a OpenGL context change. This should be the first
	 * thing to call after you created an OpenGL (ES) context!
	 *
	 * @param defaultFormat      The new default format for the game screen
	 *                           (this is used for the CLUT8 game screens).
	 * @param defaultFormatAlpha The new default format with an alpha channel
	 *                           (this is used for the overlay and cursor).
	 */
	void notifyContextCreate(const Graphics::PixelFormat &defaultFormat, const Graphics::PixelFormat &defaultFormatAlpha);

	/**
	 * Notify the manager that the OpenGL context is about to be destroyed.
	 * This will free up/reset internal OpenGL related state and *must* be
	 * called whenever a context might be created again after destroying a
	 * context.
	 */
	void notifyContextDestroy();

	/**
	 * Adjust the physical mouse coordinates according to the currently visible screen.
	 */
	void adjustMousePosition(int16 &x, int16 &y);

	/**
	 * Set up the mouse position for graphics output.
	 *
	 * @param x X coordinate in physical coordinates.
	 * @param y Y coordinate in physical coordinates.
	 */
	void setMousePosition(int x, int y);

	/**
	 * Query the mouse position in physical coordinates.
	 */
	void getMousePosition(int16 &x, int16 &y) const { x = _cursorX; y = _cursorY; }

	/**
	 * Set up the mouse position for the (event) system.
	 *
	 * @param x X coordinate in physical coordinates.
	 * @param y Y coordinate in physical coordinates.
	 */
	virtual void setInternalMousePosition(int x, int y) = 0;

private:
	/**
	 * Create a surface with the specified pixel format.
	 *
	 * @param format    The pixel format the Surface object should accept as
	 *                  input.
	 * @param wantAlpha For CLUT8 surfaces this marks whether an alpha
	 *                  channel should be used.
	 * @return A pointer to the surface or nullptr on failure.
	 */
	Surface *createSurface(const Graphics::PixelFormat &format, bool wantAlpha = false);

	//
	// Transaction support
	//
	struct VideoState {
		VideoState() : valid(false), gameWidth(0), gameHeight(0),
#ifdef USE_RGB_COLOR
		    gameFormat(),
#endif
		    aspectRatioCorrection(false), graphicsMode(GFX_OPENGL), filtering(true) {
		}

		bool valid;

		uint gameWidth, gameHeight;
#ifdef USE_RGB_COLOR
		Graphics::PixelFormat gameFormat;
#endif
		bool aspectRatioCorrection;
		int graphicsMode;
		bool filtering;

		bool operator==(const VideoState &right) {
			return gameWidth == right.gameWidth && gameHeight == right.gameHeight
#ifdef USE_RGB_COLOR
			    && gameFormat == right.gameFormat
#endif
			    && aspectRatioCorrection == right.aspectRatioCorrection
			    && graphicsMode == right.graphicsMode
				&& filtering == right.filtering;
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

protected:
	/**
	 * Set up the requested video mode. This takes parameters which describe
	 * what resolution the game screen requests (this is possibly aspect ratio
	 * corrected!).
	 *
	 * A sub-class should take these parameters as hints. It might very well
	 * set up a mode which it thinks suites the situation best.
	 *
	 * @parma requestedWidth  This is the requested actual game screen width.
	 * @param requestedHeight This is the requested actual game screen height.
	 * @param format          This is the requested pixel format of the virtual game screen.
	 * @return true on success, false otherwise
	 */
	virtual bool loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format) = 0;

	/**
	 * Refresh the screen contents.
	 */
	virtual void refreshScreen() = 0;

	/**
	 * Save a screenshot of the full display as BMP to the given file. This
	 * uses Common::DumpFile for writing the screenshot.
	 *
	 * @param filename The output filename.
	 * @return true on success, false otherwise
	 */
	bool saveScreenshot(const Common::String &filename) const;

private:
	//
	// OpenGL utilities
	//

	/**
	 * Initialize the active context for use.
	 */
	void initializeGLContext();

	/**
	 * Render back buffer.
	 */
	Backbuffer _backBuffer;

	/**
	 * OpenGL pipeline used for rendering.
	 */
	Pipeline *_pipeline;

protected:
	/**
	 * Query the address of an OpenGL function by name.
	 *
	 * This can only be used after a context has been created.
	 * Please note that this function can return valid addresses even if the
	 * OpenGL context does not support the function.
	 *
	 * @param name The name of the OpenGL function.
	 * @return An function pointer for the requested OpenGL function or
	 *         nullptr in case of failure.
	 */
	virtual void *getProcAddress(const char *name) const = 0;

private:
	/**
	 * Try to determine the internal parameters for a given pixel format.
	 *
	 * @return true when the format can be used, false otherwise.
	 */
	bool getGLPixelFormat(const Graphics::PixelFormat &pixelFormat, GLenum &glIntFormat, GLenum &glFormat, GLenum &glType) const;

	//
	// Actual hardware screen
	//

	/**
	 * The width of the physical output.
	 */
	uint _outputScreenWidth;

	/**
	 * The height of the physical output.
	 */
	uint _outputScreenHeight;

	/**
	 * @return The desired aspect of the game screen.
	 */
	frac_t getDesiredGameScreenAspect() const;

	/**
	 * Recalculates the area used to display the game screen.
	 */
	void recalculateDisplayArea();

	/**
	 * The X coordinate of the game screen.
	 */
	uint _displayX;

	/**
	 * The Y coordinate of the game screen.
	 */
	uint _displayY;

	/**
	 * The width of the game screen in physical coordinates.
	 */
	uint _displayWidth;

	/**
	 * The height of the game screen in physical coordinates.
	 */
	uint _displayHeight;

	/**
	 * The default pixel format of the backend.
	 */
	Graphics::PixelFormat _defaultFormat;

	/**
	 * The default pixel format with an alpha channel.
	 */
	Graphics::PixelFormat _defaultFormatAlpha;

	//
	// Game screen
	//

	/**
	 * The virtual game screen.
	 */
	Surface *_gameScreen;

	/**
	 * The game palette if in CLUT8 mode.
	 */
	byte _gamePalette[3 * 256];

	/**
	 * The offset by which the screen is moved vertically.
	 */
	int _gameScreenShakeOffset;

	//
	// Overlay
	//

	/**
	 * The overlay screen.
	 */
	Surface *_overlay;

	/**
	 * Whether the overlay is visible or not.
	 */
	bool _overlayVisible;

	//
	// Cursor
	//

	/**
	 * Set up the correct cursor palette.
	 */
	void updateCursorPalette();

	/**
	 * The cursor image.
	 */
	Surface *_cursor;

	/**
	 * X coordinate of the cursor in physical coordinates.
	 */
	int _cursorX;

	/**
	 * Y coordinate of the cursor in physical coordinates.
	 */
	int _cursorY;

	/**
	 * X coordinate used for drawing the cursor.
	 */
	int _cursorDisplayX;

	/**
	 * Y coordinate used for drawing the cursor.
	 */
	int _cursorDisplayY;

	/**
	 * The X offset for the cursor hotspot in unscaled coordinates.
	 */
	int _cursorHotspotX;

	/**
	 * The Y offset for the cursor hotspot in unscaled coordinates.
	 */
	int _cursorHotspotY;

	/**
	 * Recalculate the cursor scaling. Scaling is always done according to
	 * the game screen.
	 */
	void recalculateCursorScaling();

	/**
	 * The X offset for the cursor hotspot in scaled coordinates.
	 */
	int _cursorHotspotXScaled;

	/**
	 * The Y offset for the cursor hotspot in scaled coordinates.
	 */
	int _cursorHotspotYScaled;

	/**
	 * The width of the cursor scaled coordinates.
	 */
	uint _cursorWidthScaled;

	/**
	 * The height of the cursor scaled coordinates.
	 */
	uint _cursorHeightScaled;

	/**
	 * The key color.
	 */
	uint32 _cursorKeyColor;

	/**
	 * Whether the cursor is actually visible.
	 */
	bool _cursorVisible;

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

	//
	// Misc
	//

	/**
	 * Whether the screen contents shall be forced to redrawn.
	 */
	bool _forceRedraw;

#ifdef USE_OSD
	//
	// OSD
	//
protected:
	/**
	 * Returns the font used for on screen display
	 */
	virtual const Graphics::Font *getFontOSD();

private:
	/**
	 * Request for the OSD icon surface to be updated.
	 */
	bool _osdMessageChangeRequest;

	/**
	 * The next OSD message.
	 *
	 * If this value is not empty, the OSD message will be set
	 * to it on the next frame.
	 */
	Common::String _osdMessageNextData;

	/**
	 * Set the OSD message surface with the value of the next OSD message.
	 */
	void osdMessageUpdateSurface();

	/**
	 * The OSD message's contents.
	 */
	Surface *_osdMessageSurface;

	/**
	 * Current opacity level of the OSD message.
	 */
	uint8 _osdMessageAlpha;

	/**
	 * When fading the OSD message has started.
	 */
	uint32 _osdMessageFadeStartTime;

	enum {
		kOSDMessageFadeOutDelay = 2 * 1000,
		kOSDMessageFadeOutDuration = 500,
		kOSDMessageInitialAlpha = 80
	};

	/**
	 * The OSD background activity icon's contents.
	 */
	Surface *_osdIconSurface;

	enum {
		kOSDIconTopMargin = 10,
		kOSDIconRightMargin = 10
	};
#endif
};

} // End of namespace OpenGL

#endif
