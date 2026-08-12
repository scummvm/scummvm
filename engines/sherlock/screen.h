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

#ifndef SHERLOCK_SCREEN_H
#define SHERLOCK_SCREEN_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/list.h"
#include "common/path.h"
#include "common/rect.h"
#include "graphics/managed_surface.h"
#include "sherlock/image_file.h"
#include "sherlock/surface.h"
#include "sherlock/resources.h"
#include "sherlock/saveload.h"

#ifdef USE_FREETYPE2
namespace Graphics {
class Font;
}
#endif

namespace Sherlock {

#define BG_GREYSCALE_RANGE_END 229
#define BLACK 0

class SherlockEngine;

class Screen : public BaseSurface {
private:
	uint32 _transitionSeed;

	/**
	 * Shared implementation behind loadRoseTattooHiresBackgroundOverride()
	 * and loadRoseTattooHiresMapOverride(): decodes a hires override PNG at
	 * the given path into _roseTattooHiresBackground/_roseTattooHiresComposite,
	 * validated against _backBuffer1's current dimensions * hiresScale.
	 */
	bool loadRoseTattooHiresBackgroundFromPath(const Common::Path &overridePath);

#ifdef USE_FREETYPE2
	/**
	 * Lazily loads (and caches, keyed by pixel size) a TrueType font used to
	 * render hires, non-blocky in-game text - see queueRoseTattooHiresText().
	 * Looks for "$SCUMMVM_SHERLOCK_TATTOO_ASSET_OVERRIDES/fonts/hires_font.ttf",
	 * the same asset-override convention used by the background/cursor hires
	 * paths. Returns nullptr if no override dir/font file is configured, or
	 * if the font fails to load.
	 */
	Graphics::Font *getRoseTattooHiresFont(int pixelHeight);
	void blendRoseTattooHiresTextLayer();

	bool _roseTattooHiresFontMissing;
	Common::HashMap<int, Graphics::Font *> _roseTattooHiresFonts;
#endif

	// Rose Tattoo fields
	int _fadeBytesRead, _fadeBytesToRead;
	int _oldFadePercent;
	int _roseTattooHiresScale;
	int _roseTattooHiresDebugMode;
	Graphics::PixelFormat _roseTattooHiresFormat;
	byte _roseTattooPalette[Graphics::PALETTE_SIZE];
	Graphics::ManagedSurface _roseTattooHiresBackground;
	Graphics::ManagedSurface _roseTattooHiresComposite;

	/**
	 * True-color RGBA layer that hires text (see queueRoseTattooHiresText())
	 * is accumulated into, then alpha-blended on top of
	 * _roseTattooHiresComposite every frame in update(). Unlike
	 * _roseTattooHiresComposite (rebuilt from scratch every frame from the
	 * background + native-diff), this layer is intentionally NOT cleared
	 * automatically each frame: queueRoseTattooHiresText() is only called
	 * from Fonts::writeString() when a widget's text is actually written
	 * (e.g. once when a tooltip's text is set), not on every subsequent
	 * frame it remains visible, so the queued glyphs need to persist across
	 * frames on their own. Widgets are responsible for calling
	 * clearRoseTattooHiresTextRect() when they erase or move their text, to
	 * avoid leaving stale glyphs behind.
	 */
	Graphics::ManagedSurface _roseTattooHiresTextLayer;

	/**
	 * Bounding box, in the same scaled output-pixel space as
	 * _roseTattooHiresComposite/_roseTattooHiresTextLayer, of all text
	 * queued into _roseTattooHiresTextLayer so far this frame, accumulated
	 * by queueRoseTattooHiresText() and consumed/reset by
	 * blendRoseTattooHiresTextLayer(). Needed because the composite's
	 * "foreground" pass (see update()) marks any native pixel where a
	 * widget's low-res bitmap glyphs were drawn as changed-from-background
	 * and upscales it blockily; without this, those blocky bitmap glyph
	 * pixels would keep peeking out from underneath/around the crisp TTF
	 * glyphs (which don't exactly match the bitmap font's per-glyph
	 * coverage/kerning) - blendRoseTattooHiresTextLayer() uses this rect to
	 * first repaint the smooth hires background across the whole text
	 * area before blending the crisp text on top, hiding the bitmap glyphs
	 * completely.
	 */
	Common::Rect _roseTattooHiresTextNativeRect;

	/**
	 * True-color RGBA layer for AI-upscaled sprite overrides (see
	 * queueRoseTattooHiresSprite()) - e.g. inventory item icons - blended
	 * on top of _roseTattooHiresComposite every frame in update(), the same
	 * way _roseTattooHiresTextLayer is. Persists across frames like the
	 * text layer for the same reason: callers only queue a sprite when it's
	 * actually (re)drawn, not every subsequent frame it stays visible, so
	 * widgets must call clearRoseTattooHiresSpriteRect() (or
	 * clearRoseTattooHiresSpriteLayer() for a full wipe) when they erase or
	 * move the sprite.
	 */
	Graphics::ManagedSurface _roseTattooHiresSpriteLayer;

	/**
	 * Bounding box (same scaled output-pixel space as the composite/layer)
	 * of all sprites queued into _roseTattooHiresSpriteLayer so far this
	 * frame - same purpose as _roseTattooHiresTextNativeRect: lets
	 * blendRoseTattooHiresSpriteLayer() repaint the smooth hires background
	 * under the queued area first, so the blocky nearest-neighbor-upscaled
	 * native sprite pixels the composite's normal foreground pass already
	 * drew there don't peek out around the crisp override's edges.
	 */
	Common::Rect _roseTattooHiresSpriteNativeRect;

	/**
	 * True-color RGBA layer for AI-upscaled overrides of the live scene's
	 * walking characters and bg-shape objects (see
	 * queueRoseTattooHiresSceneSprite(), called from
	 * TattooScene::drawAllShapes()) - blended on top of the composite in
	 * update() just like _roseTattooHiresSpriteLayer, but kept as a
	 * separate layer/lifecycle from it deliberately: unlike UI widgets
	 * (inventory icons), which only re-queue a sprite when it's actually
	 * redrawn and rely on callers to erase stale entries, every character/
	 * object in the scene moves or can appear/disappear on essentially
	 * every frame, and the scene's draw pass has no notion of a modal
	 * widget's "erase on hide" bookkeeping. So this layer is instead fully
	 * cleared and freshly re-populated every single time drawAllShapes()
	 * runs (see clearRoseTattooHiresSceneSpriteLayer()), rather than
	 * persisting/accumulating across frames - keeping it as its own layer
	 * means that full-clear policy can't ever race with or accidentally
	 * wipe out a UI widget's independently-managed sprite overrides (which
	 * may still be visible/queued while the scene keeps animating
	 * underneath, e.g. background animation continuing behind an open
	 * modal).
	 */
	Graphics::ManagedSurface _roseTattooHiresSceneSpriteLayer;

	/**
	 * Per-native-pixel (i.e. unscaled SHERLOCK_SCREEN_WIDTH x
	 * SHERLOCK_SCREEN_HEIGHT) companion to _roseTattooHiresSceneSpriteLayer:
	 * for every native pixel a queued scene sprite/object covers this
	 * frame, records the native pixel value _backBuffer1 held immediately
	 * after that shape's own SHtransBlitFrom() call (see
	 * queueRoseTattooHiresSceneSprite()). _roseTattooHiresSceneSpriteExpectedValid
	 * marks which entries were actually written this frame (both arrays are
	 * reset - the valid one to all-zero - by clearRoseTattooHiresSceneSpriteLayer()).
	 *
	 * This exists to fix AI-upscaled characters/objects visibly "floating"
	 * on top of things that should occlude them: most bg-shape scenery
	 * (ordinary furniture, doorframes, etc.) has no AI sprite override of
	 * its own, so when such an un-overridden shape is drawn *after* a
	 * character in native z-order (correctly occluding it in the plain
	 * nearest-neighbor-upscaled composite - see the diff loop in
	 * Screen::update()), nothing ever tells the scene-sprite layer that
	 * its previously-queued character pixels there are now supposed to be
	 * hidden again. The same blind spot also affects the right-click verb
	 * menu, tooltips, and any other UI drawn on top of the scene later in
	 * the same frame (all native-only content with no AI override).
	 * blendRoseTattooHiresSceneSpriteLayer() compares this recorded
	 * "expected" value against the real final native pixel (after every
	 * later shape/widget in the frame has had its chance to draw) and
	 * skips blending the AI pixel wherever they differ, letting the
	 * correctly-occluding native content show through instead.
	 */
	Common::Array<byte> _roseTattooHiresSceneSpriteExpectedNative;
	Common::Array<byte> _roseTattooHiresSceneSpriteExpectedValid;

	/**
	 * Shared implementation behind queueRoseTattooHiresSprite() and
	 * queueRoseTattooHiresSceneSprite() - loads/scales/flips the override
	 * asset and alpha-blends it into whichever layer the caller passes
	 * (letting the two stay on independent lifecycles - see
	 * _roseTattooHiresSceneSpriteLayer's declaration for why). Returns the
	 * queued/output-pixel-space rect actually written to (may be empty if
	 * hires mode isn't active or no override asset was found).
	 *
	 * When trackNativeProvenance is true (only passed from
	 * queueRoseTattooHiresSceneSprite()), also records each written pixel's
	 * corresponding native _backBuffer1 value into
	 * _roseTattooHiresSceneSpriteExpectedNative/Valid - see their
	 * declarations for why.
	 */
	Common::Rect blitRoseTattooHiresSpriteOverride(Graphics::ManagedSurface &layer,
		const Common::String &resourceName, int frameIndex, const ImageFrame &nativeFrame,
		const Common::Point &pt, bool horizFlip, int scaleVal, bool trackNativeProvenance = false);
protected: 
	SherlockEngine *_vm;
	Surface _backBuffer;

public:
	Surface _backBuffer1, _backBuffer2;
	bool _fadeStyle;
	byte _cMap[Graphics::PALETTE_SIZE];
	byte _sMap[Graphics::PALETTE_SIZE];
	byte _tMap[Graphics::PALETTE_SIZE];
	bool _flushScreen;
	Common::Point _currentScroll;
public:
	static Screen *init(SherlockEngine *vm);
	Screen(SherlockEngine *vm);
	~Screen() override;

	void update() override;
	void getPalette(byte palette[Graphics::PALETTE_SIZE]);
	void getPalette(byte *palette, uint start, uint num);
	void setPalette(const byte palette[Graphics::PALETTE_SIZE]);
	void setPalette(const byte *palette, uint start, uint num);
	void setPalette(const Graphics::Palette &pal, uint start = 0);

	/**
	 * Obtain the currently active back buffer.
	 */
	Surface *getBackBuffer() { return &_backBuffer; }

	/**
	 * Makes first back buffer active.
	 */
	void activateBackBuffer1();

	/**
	 * Makes second back buffer active.
	 */
	void activateBackBuffer2();

	/**
	 * Fades from the currently active palette to the passed palette
	 */
	int equalizePalette(const byte palette[Graphics::PALETTE_SIZE]);

	/**
	 * Fade out the palette to black
	 */
	void fadeToBlack(int speed = 2);

	/**
	 * Fade in a given palette
	 */
	void fadeIn(const byte palette[Graphics::PALETTE_SIZE], int speed = 2);

	/**
	 * Do a random pixel transition in from _backBuffer surface to the screen
	 */
	void randomTransition();

	/**
	 * Transition to the surface from _backBuffer using a vertical transition
	 */
	void verticalTransition();

	/**
	 * Prints the text passed onto the back buffer at the given position and color.
	 * The string is then blitted to the screen
	 */
	void print(const Common::Point &pt, uint color, MSVC_PRINTF const char *formatStr, ...) GCC_PRINTF(4, 5);

	/**
	 * Print a strings onto the back buffer without blitting it to the screen
	 */
	void gPrint(const Common::Point &pt, uint color, MSVC_PRINTF const char *formatStr, ...) GCC_PRINTF(4, 5);

	/**
	 * Copies a section of the second back buffer into the main back buffer
	 */
	void restoreBackground(const Common::Rect &r);

	/**
	 * Copies a given area to the screen
	 */
	void slamArea(int16 xp, int16 yp, int16 width, int16 height);

	/**
	 * Copies a given area to the screen
	 */
	void slamRect(const Common::Rect &r);

	/**
	 * Copy an image from the back buffer to the screen, taking care of both the
	 * new area covered by the shape as well as the old area, which must be restored
	 */
	void flushImage(ImageFrame *frame, const Common::Point &pt, int16 *xp, int16 *yp,
		int16 *width, int16 *height);

	/**
	 * Similar to flushImage, this method takes in an extra parameter for the scale proporation,
	 * which affects the calculated bounds accordingly
	 */
	void flushScaleImage(ImageFrame *frame, const Common::Point &pt, int16 *xp, int16 *yp,
		int16 *width, int16 *height, int scaleVal);

	/**
	 * Variation of flushImage/flushScaleImage that takes in and updates a rect
	 */
	void flushImage(ImageFrame *frame, const Common::Point &pt, Common::Rect &newBounds, int scaleVal);

	/**
	 * Copies data from the back buffer to the screen
	 */
	void blockMove(const Common::Rect &r);

	/**
	 * Copies the entire screen from the back buffer
	 */
	void blockMove();

	/**
	 * Fills an area on the back buffer, and then copies it to the screen
	 */
	void vgaBar(const Common::Rect &r, int color);

	/**
	 * Sets the active back buffer pointer to a restricted sub-area of the first back buffer
	 */
	void setDisplayBounds(const Common::Rect &r);

	/**
	 * Resets the active buffer pointer to point back to the full first back buffer
	 */
	void resetDisplayBounds();

	/**
	 * Return the size of the current display window
	 */
	Common::Rect getDisplayBounds();

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Serializer &s);

	/**
	 * Draws the given string into the back buffer using the images stored in _font
	 */
	void writeString(const Common::String &str, const Common::Point &pt, uint overrideColor);


	// Rose Tattoo specific methods
	void initPaletteFade(int bytesToRead);

	int fadeRead(Common::SeekableReadStream &stream, byte *buf, int totalSize);

	/**
	 * Translate a palette from 6-bit RGB values to full 8-bit values suitable for passing
	 * to the underlying palette manager
	 */
	static void translatePalette(byte palette[Graphics::PALETTE_SIZE]);

	int roseTattooHiresScale() const { return _roseTattooHiresScale; }

	/**
	 * True when Screen::queueRoseTattooHiresText() will actually queue a
	 * crisp TrueType rendering for any string handed to it right now (hires
	 * mode active, with a true-color composite format to render/blend the
	 * anti-aliased glyphs into - see that method's own early-return check,
	 * which this mirrors). Widgets that draw their own blocky bitmap text
	 * directly onto the native screen (e.g. WidgetTooltipBase::draw()'s
	 * SHtransBlitFrom() call) can use this to skip that redundant bitmap
	 * draw entirely when the crisp hires text will fully replace it, rather
	 * than drawing it and then trying to paint over/hide it afterwards -
	 * the latter proved unreliable in practice (see the "blocky bitmap text
	 * bleed-through" bug this was introduced to fix once and for all).
	 */
	bool usesRoseTattooHiresText() const {
		return _roseTattooHiresScale > 1 && !_roseTattooHiresFormat.isCLUT8();
	}

	bool loadRoseTattooHiresBackgroundOverride(int sceneNumber);

	/**
	 * Like loadRoseTattooHiresBackgroundOverride(), but for the overhead/
	 * travel map screen (TattooMap::show()), which bypasses Scene::loadScene()
	 * entirely and uses its own larger, double-size scrollable canvas
	 * instead of a per-scene one. Looks for a fixed
	 * "sprites/map_vgs/frame_000@<scale>x.png" override (matching the
	 * extract_rosetattoo_sprites.py/upscale_rosetattoo_sprites.py naming
	 * convention for the MAP.VGS resource) instead of a per-scene path.
	 * Caller must size _backBuffer1 to the map's real canvas dimensions
	 * before calling this, since the expected override size is derived
	 * from it.
	 */
	bool loadRoseTattooHiresMapOverride();
	void clearRoseTattooHiresBackground();

	/**
	 * Attempts to load a true-color, AI-upscaled cursor frame override for
	 * the given VGS resource ("rmouse_vgs"/"omouse_vgs") and frame index
	 * (which lines up 1:1 with CursorId - see tools/extract_rosetattoo_sprites.py).
	 * On success, fills in outSurface (owned by the caller) at native
	 * (hiresScale-multiplied) resolution and returns true; the pixel format
	 * is always kRGBA8888 with real per-pixel alpha (transparent pixels are
	 * fully transparent, never a color-keyed sentinel).
	 */
	bool loadRoseTattooHiresCursorOverride(const Common::String &resourceName, int frameIndex,
		Graphics::Surface &outSurface);

	/**
	 * Queues an AI-upscaled sprite override (see
	 * tools/extract_rosetattoo_sprites.py / upscale_rosetattoo_sprites.py,
	 * same "sprites/<resourceName>/frame_NNN@Sx.png" naming convention the
	 * cursor override uses) to be alpha-blended into the hires composite in
	 * update(), instead of the blocky nearest-neighbor upscale the native
	 * sprite draw would otherwise get. resourceName/frameIndex select the
	 * override asset (via loadRoseTattooHiresCursorOverride() - the loader
	 * itself is resource-agnostic despite its cursor-era name).
	 *
	 * nativeFrame/pt/horizFlip/scaleVal mirror exactly the same-named
	 * arguments BaseSurface::SHtransBlitFrom(const ImageFrame &, pt,
	 * flipped, scaleVal) takes for its native-resolution draw - callers
	 * (see TattooScene::drawAllShapes(), the only caller that passes
	 * perspective-scaled/flipped sprites) should pass the identical
	 * nativeFrame/pt/flip/scaleVal values they already pass to the adjacent
	 * SHtransBlitFrom() call, unadjusted for the frame's own offset:
	 * nativeFrame.sDrawXOffset(scaleVal)/sDrawYOffset(scaleVal) is added
	 * internally, exactly like SHtransBlitFrom() does, so the override
	 * lines up pixel-for-pixel with where the native blit would have drawn.
	 * When scaleVal isn't SCALE_THRESHOLD (i.e. the object is perspective-
	 * scaled), the override bitmap (already at hiresScale native
	 * resolution) is further resized by the same
	 * nativeFrame.sDrawXSize()/sDrawYSize() ratio the native draw uses, so
	 * the crisp override shrinks/grows in lockstep with the blocky native
	 * fallback rather than staying a fixed size. When horizFlip is true the
	 * override is mirrored horizontally first, to match SHtransBlitFrom()'s
	 * own flip argument (used when a character walks left instead of
	 * right).
	 *
	 * No-ops quietly (falls back to the plain native draw + generic
	 * upscale) if hires mode isn't active, the format is CLUT8, or no
	 * override asset is found for this resource/frame - callers don't need
	 * to guard first, and should keep doing their normal native-resolution
	 * draw call either way (it's this override layer, not the native draw,
	 * that actually determines what's visible in hires mode).
	 */
	void queueRoseTattooHiresSprite(const Common::String &resourceName, int frameIndex,
		const ImageFrame &nativeFrame, const Common::Point &pt, bool horizFlip = false,
		int scaleVal = SCALE_THRESHOLD);

	/**
	 * Same override lookup/scale/flip semantics as queueRoseTattooHiresSprite()
	 * (see its comment), but targets the separate
	 * _roseTattooHiresSceneSpriteLayer used for the live scene's walking
	 * characters and bg-shape objects (see TattooScene::drawAllShapes(),
	 * its only caller) instead of the UI-widget sprite layer. Callers
	 * should call clearRoseTattooHiresSceneSpriteLayer() once per frame
	 * before re-queuing every currently-visible sprite - this layer is
	 * fully rebuilt each frame, not incrementally erased/re-queued like the
	 * UI layer.
	 */
	void queueRoseTattooHiresSceneSprite(const Common::String &resourceName, int frameIndex,
		const ImageFrame &nativeFrame, const Common::Point &pt, bool horizFlip = false,
		int scaleVal = SCALE_THRESHOLD);

	/**
	 * Blends _roseTattooHiresSceneSpriteLayer on top of the composite -
	 * called from update(), alongside blendRoseTattooHiresSpriteLayer().
	 */
	void blendRoseTattooHiresSceneSpriteLayer();

	/**
	 * Fully wipes _roseTattooHiresSceneSpriteLayer - called once per frame
	 * by TattooScene::drawAllShapes() before re-queuing this frame's scene
	 * sprites (see the layer's declaration for why a full wipe, rather than
	 * incremental per-rect erasing, is used here).
	 */
	void clearRoseTattooHiresSceneSpriteLayer();

	/**
	 * Like queueRoseTattooHiresSprite(), but bakes the override directly
	 * into the persistent, full-world _roseTattooHiresBackground buffer at
	 * world/native coordinates instead of the screen-space sprite overlay
	 * layer - for content that's drawn once into a larger-than-screen
	 * scrollable canvas (e.g. the overhead map's location icons, see
	 * TattooMap::drawMapIcons()) rather than redrawn as a screen-space
	 * overlay every frame. No-ops quietly if hires mode isn't active, the
	 * format is CLUT8, no hires background is currently loaded, or no
	 * override asset is found for this resource/frame.
	 */
	void paintRoseTattooHiresWorldSprite(const Common::String &resourceName, int frameIndex,
		const Common::Point &worldPt);

	/**
	 * Blends the accumulated hires sprite layer (see
	 * queueRoseTattooHiresSprite()) on top of the composite - called from
	 * update(), mirrors blendRoseTattooHiresTextLayer()'s approach
	 * (including the same background-repaint-first step, and the same
	 * getPixel()/setPixel() blend loop instead of blendBlitFrom() for the
	 * same RGBA32-vs-BlendBlit channel-order mismatch reason).
	 */
	void blendRoseTattooHiresSpriteLayer();

	/**
	 * Clears previously-queued sprite override pixels within nativeRect
	 * (native/unscaled coordinates) from the persistent sprite layer -
	 * callers must call this when they erase or reposition an overridden
	 * sprite, since the layer isn't auto-cleared every frame (see its
	 * declaration for why).
	 */
	void clearRoseTattooHiresSpriteRect(const Common::Rect &nativeRect);

	/**
	 * Clears the entire sprite layer at once - convenient for widgets (like
	 * the inventory window) that queue several sprites together and want a
	 * single call to erase all of them when closing/hiding.
	 */
	void clearRoseTattooHiresSpriteLayer();

#ifdef USE_FREETYPE2
	/**
	 * Queues a string to be rendered with a real (anti-aliased, properly
	 * hinted) TrueType font into the hires composite, instead of the
	 * blocky nearest-neighbor upscale that the native bitmap font glyphs
	 * (drawn by Fonts::writeString() into _backBuffer1/2 at native
	 * resolution) would otherwise get in Screen::update(). Called from
	 * Fonts::writeString() once per call (i.e. once per drawn string, not
	 * per character), so multi-line/wrapped text results in several calls.
	 *
	 * nativePt/fontHeightPx are in native (unscaled) Rose Tattoo
	 * coordinates/pixels, matching what Fonts::writeString() already
	 * receives - this method takes care of multiplying by
	 * _roseTattooHiresScale itself. rgbColor is a true 0xRRGGBB color
	 * (already resolved from the game's palette by the caller).
	 *
	 * No-ops (does nothing) if hires mode isn't active or no hires font
	 * override is configured/loadable - callers don't need to guard first.
	 */
	void queueRoseTattooHiresText(const Common::String &str, const Common::Point &nativePt,
		uint32 rgbColor, int fontHeightPx);

	/**
	 * Clears the hires text layer over the screen-absolute native-resolution
	 * rectangle @p nativeRect (in the same coordinate space as the pt passed
	 * to queueRoseTattooHiresText()/Fonts::writeString()). The text layer is
	 * no longer auto-cleared every frame (see _roseTattooHiresTextLayer's
	 * comment) since queueRoseTattooHiresText() is only called when a
	 * widget's text is first set/changed, not every frame it stays on
	 * screen - so widgets must call this when erasing or repositioning
	 * their text (i.e. whenever they'd otherwise leave stale glyphs
	 * behind), mirroring the native bitmap erase behavior.
	 */
	void clearRoseTattooHiresTextRect(const Common::Rect &nativeRect);

	/**
	 * Clears the entire hires text layer at once - for full-screen
	 * overlays (like Watson's Journal) that replace the whole native
	 * framebuffer with their own content and don't use hires TTF text
	 * themselves: any crisp text glyphs queued by widgets from before the
	 * overlay opened would otherwise keep being blended on top every
	 * frame, since the layer is only ever cleared rect-by-rect by the
	 * widget that owns it (see _roseTattooHiresTextLayer's comment).
	 */
	void clearRoseTattooHiresTextLayer();

	/**
	 * Registers @p nativeRect (screen-absolute, native/unscaled - i.e. a
	 * widget's own on-screen bounds) as an area that will be covered by
	 * hires TrueType text this frame, so blendRoseTattooHiresTextLayer()
	 * knows to repaint the smooth hires background across that whole area
	 * before blending the crisp glyphs on top - otherwise, blocky upscaled
	 * bitmap glyph pixels (drawn by the composite's normal foreground
	 * pass; see update()) would keep peeking out around/between the TTF
	 * glyphs, since the TTF font's per-glyph metrics don't exactly match
	 * the original bitmap font's. Widgets that queue hires text (directly
	 * or via Fonts::writeString()) should call this with their exact
	 * drawn bounds every frame they do so - it accumulates until the next
	 * blendRoseTattooHiresTextLayer() call, which consumes and resets it.
	 */
	void registerRoseTattooHiresTextRect(const Common::Rect &nativeRect);
#endif
};

} // End of namespace Sherlock

#endif
