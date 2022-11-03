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

#include "graphics/screen.h"

#include "engines/nancy/font.h"

namespace Nancy {

class NancyEngine;
class RenderObject;

// Graphics class that handles multilayered surface rendering with minimal redraw
class GraphicsManager {
public:
	GraphicsManager();

	void init();
	void draw();

	void addObject(RenderObject *object);
	void removeObject(RenderObject *object);
	void clearObjects();

	void redrawAll();

	const Font *getFont(uint id) const { return id < _fonts.size() ? &_fonts[id] : nullptr; }
	const Graphics::Screen *getScreen() { return &_screen; }

	const Graphics::PixelFormat &getInputPixelFormat();
	const Graphics::PixelFormat &getScreenPixelFormat();
	uint getTransColor();

	static void loadSurfacePalette(Graphics::ManagedSurface &inSurf, const Common::String paletteFilename);
	static void loadSurfacePalette(Graphics::ManagedSurface &inSurf, const Common::String paletteFilename, uint paletteStart, uint paletteSize);
	static void copyToManaged(const Graphics::Surface &src, Graphics::ManagedSurface &dst, bool verticalFlip = false, bool doubleSize = false);
	static void copyToManaged(void *src, Graphics::ManagedSurface &dst, uint srcW, uint srcH, const Graphics::PixelFormat &format, bool verticalFlip = false, bool doubleSize = false);

	// Debug
	void debugDrawToScreen(const Graphics::Surface &surf);

	Graphics::ManagedSurface _object0;

	Graphics::PixelFormat _screenPixelFormat;

private:
	void loadFonts();
	void blitToScreen(const RenderObject &src, Common::Rect dest);

	static int objectComparator(const void *a, const void *b);

	Common::SortedArray<RenderObject *> _objects;

	Graphics::PixelFormat _inputPixelFormat;
	Graphics::PixelFormat _clut8Format;

	Graphics::Screen _screen;
	Common::Array<Font> _fonts;
};

} // End of namespace Nancy

#endif // NANCY_GRAPHICS_H
