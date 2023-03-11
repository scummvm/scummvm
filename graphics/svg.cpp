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

#include "graphics/svg.h"

#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#define NANOSVG_IMPLEMENTATION
#include "graphics/nanosvg/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "graphics/nanosvg/nanosvgrast.h"

#ifdef SCUMM_BIG_ENDIAN
#define PIXELFORMAT Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0)
#else
#define PIXELFORMAT Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24)
#endif

namespace Graphics {

SVGBitmap::SVGBitmap(Common::SeekableReadStream *in, int dw, int dh)
	: ManagedSurface(dw, dh, PIXELFORMAT) {
	if (dw == 0 || dh == 0)
		return;

	int64 size = in->size();
	char *data = new char[size + 1];

	in->read(data, size);
	data[size] = '\0';

	NSVGimage *svg = nsvgParse(data, "px", 96);
	if (svg == NULL)
		error("Cannot parse SVG image");

	delete[] data;
	data = nullptr;

	// Maintain aspect ratio
	float xRatio = 1.0f * dw / svg->width;
	float yRatio = 1.0f * dh / svg->height;
	float ratio = xRatio < yRatio ? xRatio : yRatio;

	NSVGrasterizer *rasterizer = nsvgCreateRasterizer();

	nsvgRasterize(rasterizer, svg, 0, 0, ratio, (byte *)getPixels(), dw, dh, pitch);

	nsvgDeleteRasterizer(rasterizer);
	nsvgDelete(svg);
}

} // end of namespace Graphics
