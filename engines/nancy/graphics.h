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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_GRAPHICS_H
#define NANCY_GRAPHICS_H

#include "common/path.h"

#include "graphics/screen.h"

#include "engines/nancy/font.h"

namespace Nancy {

class IFF;
class RenderObject;
struct ScriptBootData;

// Packed 0xAARRGGBB colors, independent of the byte order in memory
extern const Graphics::PixelFormat kARGBFormat;

// An ARGB color, as a pixel of the given format
uint32 argbToColor(const Graphics::PixelFormat &format, uint32 argb);

// Draws a pixel with straight alpha over what the surface already holds
void blendPixel(Graphics::ManagedSurface &surface, int x, int y, byte a, byte r, byte g, byte b);

// Nancy16 blends its colors as ARGB channels from 0 to 1, in that order. These convert
// between a channel and the byte it gets drawn with, and between channels and colors.
byte colorChannelToByte(float channel);
void argbToChannels(uint32 argb, float *channels);
uint32 channelsToArgb(const float *channels);

// How much of the way toward a new color gets covered in the given time. Each 1/30
// of a second, the color keeps the given rate of its distance to the target.
float getBlendAmount(float seconds, float rate);

// Moves an ARGB color toward the target; returns true when its bytes changed
bool blendColor(float *current, uint32 target, float amount);

// Nancy16+ font colors have one color for each state of a UI control
enum FontColorState {
	kFontColorNormal		= 0,
	kFontColorDisabled		= 1,
	kFontColorHidden		= 2,
	kFontColorFocus			= 3,
	kFontColorMouseOver		= 4,
	kFontColorPressed		= 5,
	kNumFontColorStates		= 6
};

// Graphics class that handles multilayered surface rendering with minimal redraw
class GraphicsManager {
public:
	GraphicsManager();

	void init();
	void draw(bool updateScreen = true);

	void loadFonts(Common::SeekableReadStream *chunkStream);
	void loadSystemFonts(const IFF &fontIFF);
	void loadSystemFonts(const ScriptBootData &boot);	// Nancy19+

	void addObject(RenderObject *object);
	void removeObject(RenderObject *object);
	// Move an already-added object to its new place in the draw order
	void reorderObject(RenderObject *object);
	void clearObjects();

	void redrawAll();
	void suppressNextDraw();

	const Font *getFont(uint id) const { return id < _fonts.size() ? &_fonts[id] : nullptr; }
	const Font *getFont(const Common::String &name) const;
	int getFontID(const Common::String &name) const;
	uint32 getFontColor(uint id, const Graphics::PixelFormat &format, FontColorState state = kFontColorNormal) const;
	const Graphics::Screen *getScreen() { return &_screen; }
	static Common::Rect getScreenBounds();

	// Nancy16 draws the game at a lower resolution than the screen's, and scales it up.
	// Objects and the mouse are in game coordinates, unless they are at the screen's
	// resolution. For the other games the two are the same.
	bool isGameScaled() const { return !_gameSurface.empty(); }

	// What the game drew last, at its own resolution
	const Graphics::ManagedSurface &getGameImage() const { return isGameScaled() ? _gameSurface : _screen; }
	Common::Rect getGameBounds() const;
	Common::Point screenToGame(const Common::Point &pos) const;
	Common::Point gameToScreen(const Common::Point &pos) const;
	Common::Rect gameToScreen(const Common::Rect &rect) const;

	const Graphics::PixelFormat &getInputPixelFormat(uint bpp = 0);
	const Graphics::PixelFormat &getScreenPixelFormat();
	const Graphics::PixelFormat &getTransparentPixelFormat();
	uint32 getTransColor() { return _transColor; }

	Graphics::ManagedSurface &getAutotextSurface(uint16 id) { return _autotextSurfaces.getOrCreateVal(id); }
	Common::Rect &getAutotextSurfaceBounds(uint16 id) { return _autotextSurfaceBounds.getOrCreateVal(id); }

	void grabViewportObjects(Common::Array<RenderObject *> &inArray);
	void screenshotScreen(Graphics::ManagedSurface &inSurf);

	static void loadSurfacePalette(Graphics::ManagedSurface &inSurf, const Common::Path &paletteFilename, uint paletteStart = 0, uint paletteSize = 256);
	static void copyToManaged(const Graphics::Surface &src, Graphics::ManagedSurface &dst, bool verticalFlip = false, bool doubleSize = false);
	static void copyToManaged(void *src, Graphics::ManagedSurface &dst, uint srcW, uint srcH, const Graphics::PixelFormat &format, bool verticalFlip = false, bool doubleSize = false);

	static void rotateBlit(const Graphics::ManagedSurface &src, Graphics::ManagedSurface &dest, byte rotation);
	static void crossDissolve(const Graphics::ManagedSurface &from, const Graphics::ManagedSurface &to, byte alpha, const Common::Rect &rect, Graphics::ManagedSurface &inResult);

	bool getIsSuppressed() const { return _isSuppressed; }

	// Debug
	void debugDrawToScreen(const Graphics::ManagedSurface &surf);

	Graphics::ManagedSurface _object0;


private:
	enum Layer {
		kLayerAll,		// the game isn't scaled
		kLayerGame,
		kLayerNative
	};

	void addDirtyRect(const RenderObject &object, const Common::Rect &rect);
	bool isInLayer(const RenderObject &object, Layer layer) const;
	static void filterDirtyRects(Common::List<Common::Rect> &rects);
	void drawObjects(Graphics::ManagedSurface &target, const Common::List<Common::Rect> &rects, Layer layer);
	void blitObject(Graphics::ManagedSurface &target, const RenderObject &src, Common::Rect dest);

	// Where an object gets drawn, which is somewhere else for one that is zoomed into
	Common::Rect getDrawnPosition(const RenderObject &object) const;
	void blitGameToScreen(Common::Rect screenRect);

	static int objectComparator(const void *a, const void *b);

	Common::SortedArray<RenderObject *> _objects;

	Graphics::PixelFormat _inputPixelFormat16, _inputPixelFormat24, _inputPixelFormat32;
	Graphics::PixelFormat _screenPixelFormat16, _screenPixelFormat32;
	Graphics::PixelFormat _clut8Format;
	Graphics::PixelFormat _transparentPixelFormat;

	Graphics::Screen _screen;
	Common::Array<Font> _fonts;
	struct FontColor {
		uint32 colors[kNumFontColorStates] = {};
	};

	Common::Array<FontColor> _fontColors;

	Graphics::ManagedSurface _gameSurface;		// empty unless the game is scaled

	Common::List<Common::Rect> _dirtyRects;			// in game coordinates
	Common::List<Common::Rect> _nativeDirtyRects;	// in screen coordinates

	Common::HashMap<uint16, Graphics::ManagedSurface> _autotextSurfaces;
	Common::HashMap<uint16, Common::Rect> _autotextSurfaceBounds;

	uint32 _transColor = 0;

	bool _isSuppressed;
	bool _wasZoomed = false;	// whether something was zoomed into the last time
};

} // End of namespace Nancy

#endif // NANCY_GRAPHICS_H
