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
 */

#ifndef GRAPHICS_SVG_H
#define GRAPHICS_SVG_H

namespace Common {
class SeekableReadStream;
}

struct NSVGimage;
struct NSVGrasterizer;

namespace Graphics {

class ManagedSurface;
struct Surface;
struct PixelFormat;

class SVGBitmap {
public:
	SVGBitmap(Common::SeekableReadStream *in);
	~SVGBitmap();

	Graphics::PixelFormat *getPixelFormat() { return _pixelformat; }

	void render(Graphics::ManagedSurface &target, int dw, int dh);

private:
	NSVGimage *_svg;
	NSVGrasterizer *_rasterizer;

	Graphics::ManagedSurface *_render;
	int _cachedW, _cachedH;
	byte *_cache;

	Graphics::PixelFormat *_pixelformat;
};

} // end of namespace Graphics

#endif // GRAPHICS_SVG_H
