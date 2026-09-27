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
	void blitToScreen(const RenderObject &src, Common::Rect dest);

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

	Common::List<Common::Rect> _dirtyRects;

	Common::HashMap<uint16, Graphics::ManagedSurface> _autotextSurfaces;
	Common::HashMap<uint16, Common::Rect> _autotextSurfaceBounds;

	uint32 _transColor = 0;

	bool _isSuppressed;
};

} // End of namespace Nancy

#endif // NANCY_GRAPHICS_H
