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

#ifndef MOHAWK_ZOOMBINI_GRAPHICS_H
#define MOHAWK_ZOOMBINI_GRAPHICS_H

#include "common/stack.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "mohawk/graphics.h"
#include "mohawk/zoombini_resource.h"

namespace Mohawk {

class MohawkEngine_Zoombini;
class MohawkSurface;
class ZoombiniText;
class ZmbHotspot;
class ZmbDrawRecord;

/**
 * Owns the Zoombini drawing surfaces, palette state, image caches, and dirty-rect
 * handoff to the ScummVM system screen.
 *
 * The renderer keeps two 640x480 surfaces. @ref kBackScreen is the persistent
 * page background. @ref kShapeScreen is the persistent composite containing the
 * background and the currently materialized feature shapes. A page or dialog
 * writes the background surface, the render pass restores changed areas from it,
 * and feature runners draw the composite surface in Z order. @ref flushScreens()
 * copies the normalized exact dirty region to the host screen.
 *
 * Shape resources have two index conventions.
 * (1) subImageId: 0-based index, native to ScummVM mohawk engine bitmap implementation.
 * (2) shapeIdx: 1-based index, native to Mohawk Feature Hotspot.
 * Use the matching @ref findSubImage() or @ref findShape() family so the two
 * conventions are not mixed.
 */
class ZoombiniGraphics : public GraphicsManager {
public:
	/** Create the Zoombini graphics manager and its bitmap decoder. */
	explicit ZoombiniGraphics(MohawkEngine_Zoombini *vm);
	/** Release screen surfaces, palette state, and cached images. */
	~ZoombiniGraphics() override;

	// [*] Screen Kind
	enum ScreenKind {
		kBackScreen,
		kShapeScreen,
	};

	/**
	 * Return the raw surface for the requested screen.
	 *
	 * @warning Directly modifying the returned surface bypasses the renderer's dirty-rect tracking and active render clip.
	 * Use the @ref ZoombiniGraphics drawing and copy helpers for writes;
	 * this accessor is intended for read-only access and operations which perform their own required bookkeeping.
	 */
	Graphics::Surface *getScreen(ScreenKind screenKind);
	Graphics::Surface *getBackScreen() { return _backScreen; }
	Graphics::Surface *getShapeScreen() { return _shapeScreen; }

	// [*] Pixel format
	Graphics::PixelFormat getPixelFormat() { return _pixelFormat; }

	// [*] Screen captures
	/** Create a host-compatible surface with the game screen format. */
	void createScreen(Graphics::Surface &screen);
	/** Copy one internal screen into a destination surface. */
	void captureScreen(ScreenKind srcScreenKind, Graphics::Surface *destScreen);
	/** Copy a source surface into one internal screen. */
	void copyToScreen(ScreenKind destScreenKind, const Graphics::Surface &srcScreen);
	/** Capture the composed game image from an internal screen. */
	void captureComposedScreen(ScreenKind destScreenKind);
	/** Capture the composed game image into a destination surface. */
	void captureComposedScreen(Graphics::Surface *destScreen);

	// [*] Screen updates
	bool isDirty() { return _isScreenDirty; }
	/** Mark the host screen as needing presentation. */
	void setDirty();
	/** Present the normalized exact dirty region and report whether host pixels changed. */
	bool flushScreens();
	/** Clear both internal screen surfaces and dirty-region state. */
	void clearScreens();
	/**
	 * Present a localized startup loading notice before game resources are available.
	 *
	 * When ScummVM's music-device scan reaches the Windows plugin, the first MIDI query after boot
	 * can pause for about ten seconds before the logo video's first frame.
	 * Constructing @ref ZoombiniMidiPlayer invokes @ref MidiDriver::detectDevice().
	 * The Windows plugin enumerates WinMM outputs through midiOutGetNumDevs().
	 * On systems with Windows MIDI Services enabled, wdmaud2.drv forwards that legacy request
	 * to the demand-started MidiSrv service.
	 * If MidiSrv is still stopped after boot, this request starts it and enumerates its endpoints.
	 * This can take several seconds, and more with larger device sets,
	 * even when the caller uses WinMM.
	 * ScummVM performs interactive Windows enumeration on a worker and runs a progress dialog
	 * on the main thread when the query exceeds the short warm-path threshold.
	 * The engine still waits for the complete device list, but event handling and cursor updates
	 * continue through the GUI modal loop.
	 *
	 * This notice submits a visible frame immediately before that initialization.
	 * It uses ScummVM's already initialized localized GUI font and does not depend
	 * on a game font or archive that may itself still be loading. Every Zoombini
	 * release uses the same startup feedback path; a slow Windows MIDI query replaces
	 * the notice with the shared progress dialog, while fast queries proceed to the
	 * logo video almost immediately.
	 */
	void showDemoStartupLoadingScreen();
	/**
	 * Switch the OSystem graphics mode between true-color and CLUT8.
	 * v2.0TLC only; Bink video playback requires true-color mode, but the rest of the game is palette-based.
	 *
	 * Clears all surface caches and recreates the internal screen buffers.
	 */
	void reinitGraphics(bool trueColor);
	/** Clear one internal screen surface. */
	void clearScreen(ScreenKind screenKind);
	/**
	 * Copy background port -> composite buffer (blitter port).
	 * Called at the start of each render frame, before shapes are drawn.
	 */
	void copyBackToShapeScreen();
	/**
	 * Copy background port -> composite buffer, clipped to the given rect.
	 * Only pixels within clipRect are overwritten on the shapeScreen.
	 * Used by the dirty-rect render pipeline to restore background in changed areas only.
	 */
	void copyBackToShapeScreen(const Common::Rect &clipRect);

	// [*] Render clip region
	/**
	 * Confine all drawing to the exact union of the accumulated dirty rectangles.
	 * Each draw operation is clipped to each intersecting rectangle.
	 */
	/** Replace the active render clip with an exact rectangle decomposition. */
	void setRenderClipRects(const Common::Array<Common::Rect> &rects);
	/** Add one rectangle to the active render clip. */
	void addRenderClipRect(const Common::Rect &rect);
	/** Disable and clear the active render clip. */
	void clearRenderClipRect();
	const Common::Array<Common::Rect> &getRenderClipRects() const { return _renderClipRects; }
	/** Begin tracking dirty rectangles for a render pass. */
	void beginDirtyRectTracking(bool expandRenderClip);
	/** Finish dirty tracking and return the exact accumulated bounds. */
	Common::Rect endDirtyRectTracking();

	// [*] Dirty Rect Merge
	/**
	 * Merge a rectangle into an exact, non-overlapping rectangular region.
	 *
	 * @return True if @p rect adds new coverage to @p region.
	 */
	static bool mergeRectIntoRegion(Common::Array<Common::Rect> &region, const Common::Rect &rect);

	// [*] Resource Management Extensions
	/** Find the decoded tBMP image identified by @p imgResource. */
	MohawkSurface *findImage(ZmbResource imgResource);
	/**
	 * Preload a single tBMP into the image cache.
	 * Compound tBMP resources must use @ref ZoombiniGraphics::preloadShapes() instead.
	 */
	void preloadImage(ZmbResource imgResource);
	/** Find shape bitmap using 1-based shapIdx. */
	MohawkSurface *findShape(ZmbResource imgResource, uint16 shapeIdx);
	/** Find shape bitmap using 0-based subImage id. */
	MohawkSurface *findSubImage(ZmbResource imgResource, uint16 subImage);
	/** Preload every sub-image of a compound tBMP into the shape cache. */
	void preloadShapes(ZmbResource imgResource);
	/** Get width and height of shape bitmap using 1-based shapeIdx. */
	Common::Rect getShapeSize(ZmbResource imgResource, uint16 shapeIdx);
	/** Get width and height of shape bitmap using 0-based subImage id. */
	Common::Rect getSubImageSize(ZmbResource imgResource, uint16 subImage);
	/** Return the number of 1-based shapes in an image resource. */
	uint32 getShapeCount(ZmbResource imgResource);
	/** Clear page-independent image and shape caches. */
	void clearCommonCache();

	// [*] CURS
	enum MouseCursorResourceId : int16 {
		kResCursor00_Default = 0,
		kResCursor01_Watch,
		kResCursor02_EyeMiddle,
		kResCursor03_EyeRight,
		kResCursor04_EyeLeft,
		kResCursor05_EyeBlink,
	};
	/** Persistent animation deadline for the next eye-cursor pose. */
	uint32 _nextMouseCursorEyeAnimationFrame = 0;

	/**
	 * Authored cursor sequence used by the animated eye cursor. ScummVM replaces
	 * each pose atomically to avoid the transition flicker visible in the original
	 * Win32 renderer, while preserving the blink poses and 12-tick timing.
	 */
	const ZoombiniGraphics::MouseCursorResourceId _mouseCursorEyeAnimationFrames[12] = {
		ZoombiniGraphics::kResCursor02_EyeMiddle,
		ZoombiniGraphics::kResCursor04_EyeLeft,
		ZoombiniGraphics::kResCursor02_EyeMiddle,
		ZoombiniGraphics::kResCursor03_EyeRight,
		ZoombiniGraphics::kResCursor02_EyeMiddle,
		ZoombiniGraphics::kResCursor05_EyeBlink,
		ZoombiniGraphics::kResCursor02_EyeMiddle,
		ZoombiniGraphics::kResCursor05_EyeBlink,
		ZoombiniGraphics::kResCursor02_EyeMiddle,
		ZoombiniGraphics::kResCursor04_EyeLeft,
		ZoombiniGraphics::kResCursor03_EyeRight,
		ZoombiniGraphics::kResCursor05_EyeBlink,
	};
	/** Persistent index in @ref _mouseCursorEyeAnimationFrames. */
	uint32 _mouseCursorEyeAnimationFrameIdx = 0;

	/** Select and display a cursor resource. */
	void setMouseCursor(MouseCursorResourceId cursorId);
	/** Return the currently selected cursor resource. */
	MouseCursorResourceId getMouseCursor() const { return _activeCursorId; }
	/** Start the page-loading cursor without restarting its persistent pose deadline. */
	void startMouseCursorEyeAnimation();
	/** Stop the animated eye cursor and restore its base cursor. */
	void stopMouseCursorEyeAnimation();
	/** Advance the animated eye cursor when its authored tick interval elapses. */
	void runMouseCursorEyeAnimationFrame(uint32 currentFrame);
	/** Return whether the animated eye cursor is active. */
	bool isMouseCursorEyeAnimationActive() const;

	// [*] Bitmap - Image and Shapes
	/** Palette remap for color blind mode */
	enum PaletteRemapMode {
		/** Do not remap - use the palette as-is. */
		kPaletteRemapNone,
		/**
		 * Palette swap for snoid nose / net dirt color.
		 * - Yellow/Orange -> White
		 * - Purple -> Azure/Cyan
		 */
		kPaletteRemapNoseNet,
		/**
		 * Palette swap for maze purple cells.
		 * - Purple -> Azure/Cyan
		 */
		kPaletteRemapMazePurple,
	};
	/** Draw a background image to the default back screen. */
	void drawBackground(int16 image);
	/** Draw a background image to a selected internal screen. */
	void drawBackground(ScreenKind screenKind, int16 image);
	/** Draw an image resource at an authored destination point. */
	void drawImage(ScreenKind screenKind, int16 image, const Common::Point &destPos);
	/**
	 * Draw shape bitmap to screen using 1-based shapeIdx.
	 *
	 * @param screenKind Target screen buffer.
	 * @param res Resource containing the shapes.
	 * @param shapeIdx 1-based index of the shape within the resource.
	 * @param destPos Destination position on the screen.
	 * @param clearBeforeRender Whether to clear the destination area before rendering.
	 * @param paletteRemap Palette remapping mode for color blind support.
	 * @return The rectangle occupied by the drawn shape.
	 */
	Common::Rect drawShape(ScreenKind screenKind, ZmbResource res, uint16 shapeIdx, const Common::Point &destPos, bool clearBeforeRender = false, PaletteRemapMode paletteRemap = kPaletteRemapNone);
	/**
	 * Draw shape bitmap to screen using 1-based shapeIdx.
	 *
	 * @param screenKind Target screen buffer.
	 * @param res Resource containing the shapes.
	 * @param shapeIdx 1-based index of the shape within the resource.
	 * @param destRect Destination rectangle on the screen.
	 * @param clearBeforeRender Whether to clear the destination area before rendering.
	 * @param paletteRemap Palette remapping mode for color blind support.
	 * @return The rectangle occupied by the drawn shape.
	 */
	Common::Rect drawShape(ScreenKind screenKind, ZmbResource res, uint16 shapeIdx, const Common::Rect &destRect, bool clearBeforeRender = false, PaletteRemapMode paletteRemap = kPaletteRemapNone);
	/**
	 * Draw shape bitmap to screen using a feature hotspot.
	 *
	 * @param screenKind Target screen buffer.
	 * @param res Resource containing the shapes.
	 * @param hotspot Hotspot defining the shape and position.
	 * @param clearBeforeRender Whether to clear the destination area before rendering.
	 * @param paletteRemap Palette remapping mode for color blind support.
	 * @return The rectangle occupied by the drawn shape.
	 */
	Common::Rect drawShape(ScreenKind screenKind, ZmbResource res, const ZmbHotspot *hotspot, bool clearBeforeRender = false, PaletteRemapMode paletteRemap = kPaletteRemapNone);
	/**
	 * Draw shape bitmap to screen using 0-based subImage id.
	 *
	 * @param screenKind Target screen buffer.
	 * @param res Resource containing the sub-images.
	 * @param subImage 0-based index of the sub-image within the resource.
	 * @param destPos Destination position on the screen.
	 * @param clearBeforeRender Whether to clear the destination area before rendering.
	 * @param paletteRemap Palette remapping mode for color blind support.
	 * @return The rectangle occupied by the drawn sub-image.
	 */
	Common::Rect drawSubImage(ScreenKind screenKind, ZmbResource res, uint16 subImage, const Common::Point &destPos, bool clearBeforeRender = false, PaletteRemapMode paletteRemap = kPaletteRemapNone);
	/**
	 * Draw shape bitmap to screen using 0-based subImage id.
	 *
	 * @param screenKind Target screen buffer.
	 * @param res Resource containing the sub-images.
	 * @param subImage 0-based index of the sub-image within the resource.
	 * @param destRect Destination rectangle on the screen.
	 * @param clearBeforeRender Whether to clear the destination area before rendering.
	 * @param paletteRemap Palette remapping mode for color blind support.
	 * @return The rectangle occupied by the drawn sub-image.
	 */
	Common::Rect drawSubImage(ScreenKind screenKind, ZmbResource res, uint16 subImage, const Common::Rect &destRect, bool clearBeforeRender = false, PaletteRemapMode paletteRemap = kPaletteRemapNone);
	/** Copy a source image rectangle to a destination screen rectangle. */
	Common::Rect drawImageSectionToScreen(ScreenKind screenKind, MohawkSurface *imgSurface, const Common::Rect &srcRect, const Common::Rect &dstRect, bool clearBeforeRender = false, PaletteRemapMode paletteRemap = kPaletteRemapNone);

	// [*] DrawLine
	/** Draw a one-pixel diagnostic line on an internal screen. */
	void drawLine(ScreenKind screenKind, const Common::Point &start, const Common::Point &end, uint32 color);
	/** Draw a line with an authored rectangular pen size. */
	void drawThickLine(ScreenKind screenKind, const Common::Point &start, const Common::Point &end, int penX, int penY, uint32 color);

	// [*] Clear / Fill Area
	/** Clear the rectangle recorded by one draw record. */
	void clearArea(ScreenKind screenKind, ZmbDrawRecord *record);
	/** Clear the area occupied by a hotspot shape. */
	void clearArea(ScreenKind screenKind, ZmbResource res, const ZmbHotspot *hotspot);
	/** Clear one authored screen rectangle. */
	void clearArea(ScreenKind screenKind, const Common::Rect &rect);
	/** Fill the rectangle recorded by one draw record. */
	void fillArea(ScreenKind screenKind, ZmbDrawRecord *record, uint32 color = kTransparentKey);
	/** Fill the area occupied by a hotspot shape. */
	void fillArea(ScreenKind screenKind, ZmbResource res, const ZmbHotspot *hotspot, uint32 color = kTransparentKey);
	/** Fill one authored screen rectangle. */
	void fillArea(ScreenKind screenKind, const Common::Rect &rect, uint32 color = kTransparentKey);
	/** Fill the entire selected screen with one palette color. */
	void fillArea(ScreenKind screenKind, uint32 color = kTransparentKey);

	// [*] Palette (256 color)
	/** Apply palette from a SHPL resource. */
	void setPalette(uint16 id) override;
	/**
	 * Read palette from a SHPL resource.
	 * @param destBuf The buffer to read the palette data into. 768 bytes or larger buffer is recommended.
	 * @param destBufSize Size of the destBuf.
	 * @return True if successfully read the palette data. False if the buffer size is not enough, or failed to read a resource.
	 */
	bool readPalette(int16 id, byte *destBuf, size_t destBufSize);
	/** Rebuild and apply the current palette using the active brightness setting. */
	void refreshPalette();
	/** Clear the active palette and reset palette bookkeeping. */
	void clearPalette();
	/**
	 * Rotate a contiguous palette span one entry to the right and apply it.
	 * Updates the stored palette state so repeated calls continue the animation.
	 */
	void rotatePaletteRight(uint16 startEntry, uint16 count);

	enum PredefinedColor : uint32 {
		/** Treated as transparent in the palette. (0x00) */
		kTransparentKey = 0x00,
		/** Always treated as black in the palette. (0xFF) */
		kBlackKey = 0xFF,
		/** #FEFEFE */
		kColor0A_White = 0x0A,
		/** #202020 */
		kColor0B_VeryDarkGray = 0x0B,
		/** #414141 */
		kColor0C_DarkGray = 0x0C,
		/** #828282 */
		kColor0D_LightGray = 0x0D,
		/** #C0C0C0 */
		kColor0E_VeryLightGray = 0x0E,
		/** #112135 */
		kColor0F_VeryDarkCyan = 0x0F,
		/** #2E4F7F */
		kColor10_DarkCyan = 0x10,
		/** #4677AF */
		kColor11_Cyan = 0x11,
		/** #5E9EFF */
		kColor12_SkyBlue = 0x12,
		/** #7DAEFF */
		kColor13_LightBlue = 0x13,
		/** #9EBFFF */
		kColor14_PastelBlue = 0x14,
		/** #16002E */
		kColor15_VeryDarkPurple = 0x15,
		/** #280A5C */
		kColor16_DarkPurple = 0x16,
		/** #3A148A */
		kColor17_Purple = 0x17,
		/** #5E1268 */
		kColor18_DarkMagenta = 0x18,
		/** #B415C2 */
		kColor19_Magenta = 0x19,
		/** #D443E1 */
		kColor1A_DarkPink = 0x1A,
		/** #F470FF */
		kColor1B_Pink = 0x1B,
		/** #1316A9 */
		kColor1C_DarkAzure = 0x1C,
		/** #3D3DFF */
		kColor1D_Azure = 0x1D,
		/** #9496FF */
		kColor1E_LightAzure = 0x1E,
		/** #0C2701 */
		kColor1F_VeryDarkGreen = 0x1F,
		/** #005500 */
		kColor20_DarkGreen = 0x20,
		/** #009900 */
		kColor21_Green = 0x21,
		/** #4ED24B */
		kColor22_LimeGreen = 0x22,
		/** #A34400 */
		kColor23_DarkOrange = 0x23,
		/** #FF7B00 */
		kColor24_Orange = 0x24,
		/** #FFB87A */
		kColor25_LightOrange = 0x25,
		/** #961221 */
		kColor26_DarkRed = 0x26,
		/** #FC2C44 */
		kColor27_Red = 0x27,
		/** #FE8B9A */
		kColor28_LightRed = 0x28,
		/** #7A3505 */
		kColor29_Brown = 0x29,
		/** #B58F21 */
		kColor2A_LightBrown = 0x2A,
		/** #EDF50A */
		kColor2B_Yellow = 0x2B,
		/** #7F7F7F */
		kColor2C_Gray = 0x2C,
		/** #000000 */
		kColor2D_Black = 0x2D,
	};

	enum XferRoutePathLevelColor : uint32 {
		// Route path animation palette colors (XFER/RODMAP flood-fill overlay).
		// These form a gradient used by the route path flood-fill animation.
		// Source pixel values 1 and 2 in the shape are replaced with these palette indices during the animation.
		// RGB values extracted from XFER SHPL_1000-4000 palettes.
		/** #D6A55A ~ #E7AD5A (warm tan, varies per view) */
		kRoutePathColor2E_LevelOneBack1 = 0x2E,
		/** #D6AD9C ~ #FAB375 (peachy/salmon, varies per view) */
		kRoutePathColor2F_LevelOneBack2 = 0x2F,
		/** #005F41 (dark teal green) */
		kRoutePathColor30_LevelOneColor1 = 0x30,
		/** #579984 (medium teal) */
		kRoutePathColor31_LevelOneColor2 = 0x31,
		/** #F4A200 (bright orange) */
		kRoutePathColor32_LevelTwoColor1 = 0x32,
		/** #FFC863 (golden yellow) */
		kRoutePathColor33_LevelTwoColor2 = 0x33,
		/** #FF5711 (bright red-orange) */
		kRoutePathColor34_LevelThreeColor1 = 0x34,
		/** #FF9569 (coral/salmon) */
		kRoutePathColor35_LevelThreeColor2 = 0x35,
		/** #BF0218 (dark red) */
		kRoutePathColor36_LevelFourColor1 = 0x36,
		/** #E25161 (salmon red) */
		kRoutePathColor37_LevelFourColor2 = 0x37,
	};

	// [*] Draw TrueType Text
	struct TextConf {
		/** Font family selected for the text draw. */
		ZoombiniFontUsage _fontUsage = ZoombiniFontUsage::kFontText;
		/** Palette index used for the text glyphs. */
		uint32 _textPalette = kColor2D_Black;
		/** Whether an outline is drawn around the glyphs. */
		bool _outlineEffect = false;
		/** Palette index used for the outline. */
		uint32 _outlinePalette = kTransparentKey;
		/**
		 * If set to true, fill the background with @ref _fillBackgroundKey.
		 * @remarks ScummVM-only feature
		 */
		bool _fillBackground = false;
		/**
		 * If @ref _fillBackground is true, fill the background with this color.
		 * @remarks ScummVM-only feature
		 */
		uint32 _fillBackgroundKey = kTransparentKey;
		/** Horizontal alignment of the text. */
		Graphics::TextAlign _hAlign = Graphics::kTextAlignLeft;
		/** Vertical alignment of the text. */
		Graphics::TextAlign _vAlign = Graphics::kTextAlignStart;
		/**
		 * Controls word wrapping.
		 *
		 * @remarks Deliberate rendering adaptation: defaults to disabled.
		 * In original, all text has GDI DT_WORDBREAK applied.
		 * FreeType rendering tends to render a few pixel wider than GDI.
		 * Thus, always enabling this like original causes word wrapping more frequently.
		 * To mitigate this
		 */
		bool _wordWrap = false;
	};
	/** Draw a localized text key with default text configuration. */
	void drawText(ScreenKind screenKind, uint32 textKey, const Common::Rect &destRect);
	/** Draw a localized text key with explicit text configuration. */
	void drawText(ScreenKind screenKind, uint32 textKey, const Common::Rect &destRect, const TextConf &tc);
	/** Draw UTF-32 text with default text configuration. */
	void drawText(ScreenKind screenKind, const Common::U32String &text, const Common::Rect &destRect);
	/** Draw UTF-32 text with explicit text configuration. */
	void drawText(ScreenKind screenKind, const Common::U32String &text, const Common::Rect &destRect, const TextConf &tc);
	/** Measure text width using the selected font and text configuration. */
	int getTextWidth(const Common::U32String &text, const TextConf &tc);
	/** Return the selected font height for a text configuration. */
	int16 getFontHeight(const TextConf &tc);

	// [*] Transitions and effects
	enum FadeType {
		kFadeIn,
		kFadeOut,
		kFadePartialPalette
	};

	struct FadeEffect {
		/** Kind of palette or screen fade. */
		FadeType _type;
		/** Whether this effect has begun applying. */
		bool _isFading = false;
		/** Start timestamp in milliseconds. */
		uint32 _startTime; // In milliseconds
		/** Duration in milliseconds. */
		uint32 _duration; // In milliseconds
		/** First palette entry affected by a partial fade. */
		uint16 _startEntry = 0;
		/** Number of palette entries affected by a partial fade. */
		uint16 _entryCount = 0;
		/** Palette scale percentage for a partial fade. */
		uint8 _scalePercent = 100;
		/** Source palette captured before a partial fade begins. */
		byte _sourcePalette[3 * 256] = {};

		FadeEffect(FadeType type, uint32 duration)
			: _type(type), _duration(duration) {}
	};

	/** Queue a full-screen fade effect. */
	void queueFadeEffect(FadeType type, uint32 duration);
	/** Queue a scale effect for a contiguous palette range. */
	void queuePaletteScaleEffect(uint16 startEntry, uint16 count, uint8 scalePercent, uint32 duration);
	/**
	 * Apply the fade effect if one was queued.
	 * @param currentTime Current time in milliseconds.
	 * @return True while any fade effect remains queued or in progress, false otherwise.
	 */
	bool applyFadeEffect(uint32 currentTime);
	/**
	 * Check whether a fade effect is queued or currently in progress.
	 * @return True if a fade effect is queued or in progress, false otherwise.
	 */
	bool isFading() const;
	/** Apply one stepped palette dimming operation. */
	void dimPalette(uint16 fadeStepIdx, uint16 steps);

	/** Width of the fixed-resolution Zoombini game surface. */
	static constexpr uint16 kScreenWidth = 640;
	/** Height of the fixed-resolution Zoombini game surface. */
	static constexpr uint16 kScreenHeight = 480;

protected:
	/** Decode all sub-images belonging to a numeric system image ID. */
	MohawkSurface *decodeImage(uint16 id) override;
	/** Decode one image from an explicit archive-qualified resource. */
	MohawkSurface *decodeImage(ZmbResource imgResource);
	/** Decode all sub-images belonging to a numeric system image ID. */
	Common::Array<MohawkSurface *> decodeImages(uint16 id) override;
	/** Decode all sub-images from an explicit archive-qualified resource. */
	Common::Array<MohawkSurface *> decodeImages(ZmbResource imgResource);
	MohawkEngine *getVM() override { return reinterpret_cast<MohawkEngine *>(_vm); }

private:
	/** Apply one partial palette fade step. */
	void applyPartialPaletteFade(const FadeEffect &fadeEffect, uint16 blendIndex);
	/** Expand an unwrapped text rectangle according to horizontal alignment. */
	static void expandUnwrappedTextRect(Common::Rect &rect, int textWidth, Graphics::TextAlign hAlign);
	/** Split text into renderable lines with optional word wrapping. */
	Common::Array<Common::U32String> prepareTextLines(const Common::U32String &text, const Graphics::Font *font, bool wordWrap, int16 targetWidth);
	/** Calculate total draw height and max draw width */
	Common::Point getTextLinesBounds(const Graphics::Font *font, bool outlineEffect, const Common::Array<Common::U32String> &lines);
	/** Draw prepared text lines and optionally fill their background. */
	void drawTextLines(ScreenKind screenKind, const Graphics::Font *font, const Common::Array<Common::U32String> &lines, const Common::Rect &destRect, uint32 palette, Graphics::TextAlign hAlign, bool useAntialiasing, uint32 fillBackgroundColor = kTransparentKey);
	/** Copy text pixels while preserving the game transparency convention. */
	static void copyTextPixels(Graphics::Surface *textSurface, Graphics::Surface *screen, const Common::Rect &destRect, const Common::Rect &copyRect);
	/** Blend grayscale text coverage into a CLUT8 screen using its active palette. */
	void blendTextPixels(Graphics::Surface *textSurface, Graphics::Surface *screen, const Common::Rect &destRect, const Common::Rect &copyRect, uint32 palette);
	/** Record one text draw rectangle in the selected screen's dirty set. */
	void recordDirtyRect(ScreenKind screenKind, const Common::Rect &rect);
	/** Copy pixels using the requested Color Assist palette remap. */
	void copyRectToSurfaceWithColorAssistPaletteRemap(Graphics::Surface *screen, Graphics::Surface *source, int destX, int destY, const Common::Rect &sourceRect, PaletteRemapMode remapMode);
	/** Remap one palette index for Color Assist rendering. */
	byte remapColorAssistPaletteIndex(byte paletteIndex, PaletteRemapMode remapMode) const;
	/** Read and optionally brighten one SHPL palette into a caller buffer. */
	bool readPaletteInternal(int16 id, byte *destBuf, size_t destBufSize, bool applyBrightness);

	/** Engine instance that owns this graphics manager. */
	MohawkEngine_Zoombini *_vm;
	/** Bitmap decoder used for page and system images. */
	MohawkBitmap *_bmpDecoder;

	/** Palette bytes as loaded before brightness and Color Assist changes. */
	byte _unmodifiedPaletteBytes[3 * 256];
	/** Palette bytes currently applied to the renderer. */
	byte _paletteBytes[3 * 256];
	/** Cached nearest-color lookups used by antialiased CLUT8 text blending. */
	Graphics::PaletteLookup _textPaletteLookup;

	/** Pixel format of the active game screen. */
	Graphics::PixelFormat _pixelFormat;
	/** 640x480 rectangle of the active game screen. */
	Common::Rect _screenRect;

	/** Persistent back buffer containing the composed page image. */
	Graphics::Surface *_backScreen = nullptr;
	/** Intermediate shape buffer used while rendering page features. */
	Graphics::Surface *_shapeScreen = nullptr;
	/** Whether either internal screen has pending changes. */
	bool _isScreenDirty = false;
	// Normalize contained, overlapping, or aligned adjacent damage and retain disjoint pieces.
	/** Normalized exact screen regions pending for the next presentation. */
	Common::Array<Common::Rect> _screenDirtyRects;

	// Render clip region as individual rectangles plus a bounding box for inexpensive rejection.
	/** Exact rectangles that constrain current drawing. */
	Common::Array<Common::Rect> _renderClipRects;
	/** Bounding box used for quick render-clip rejection. */
	Common::Rect _renderClipBounds; // bounding box for quick early-out
	// An active GDI region can contain zero rectangles. That clips every draw;
	// it is distinct from having no selected region, which permits all drawing.
	/** Whether a render clip region is currently selected. */
	bool _isRenderClipActive = false;
	/** Whether the selected render clip contains at least one rectangle. */
	bool _hasRenderClipRect = false;
	/** Bounding box accumulated by the current dirty-tracking pass. */
	Common::Rect _trackedDirtyBounds;
	/** Whether draw operations are currently accumulating dirty bounds. */
	bool _isDirtyRectTracking = false;
	/** Whether the current dirty-tracking pass has recorded any bounds. */
	bool _hasTrackedDirtyBounds = false;
	/** Whether tracked dirty bounds must expand the active render clip. */
	bool _expandTrackedDirtyClip = false;

	/** Cached decoded images from the system archive. */
	Common::HashMap<int16, MohawkSurface *> _sysImageCache;
	/** Cached decoded sub-images from the system archive. */
	Common::HashMap<int16, Common::Array<MohawkSurface *>> _sysSubImageCache;

	/** Fade effects waiting to be applied in frame order. */
	Common::Queue<FadeEffect> _fadeQueue;

	/** Cursor resource currently selected for rendering. */
	MouseCursorResourceId _activeCursorId = kResCursor00_Default;
};

} // End of namespace Mohawk

#endif
