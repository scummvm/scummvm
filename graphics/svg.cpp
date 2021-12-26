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

#include "common/scummsys.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "graphics/managed_surface.h"
#include "graphics/pixelformat.h"

#include "graphics/svg.h"


#define NANOSVG_IMPLEMENTATION
#include "graphics/nanosvg/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "graphics/nanosvg/nanosvgrast.h"

namespace Graphics {

SVGBitmap::SVGBitmap(Common::SeekableReadStream *in) {
	int32 size = in->size();
	char *data = (char *)malloc(size + 1);

	in->read(data, size);
	data[size] = '\0';

	_svg = nsvgParse(data, "px", 96);
	free(data);

	if (_svg == NULL)
		error("Cannot parse SVG image");

	_rasterizer = NULL;
	_cachedW = _cachedH = -1;
	_cache = NULL;
	_render = NULL;

#ifdef SCUMM_BIG_ENDIAN
	_pixelformat = new Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
	_pixelformat = new Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif
}

SVGBitmap::~SVGBitmap() {
	if (_rasterizer)
		nsvgDeleteRasterizer(_rasterizer);

	nsvgDelete(_svg);

	delete _render;
	delete _pixelformat;
}

void SVGBitmap::render(Graphics::ManagedSurface &target, int dw, int dh) {
	if (dw == 0 || dh == 0)
		return;

	if (_rasterizer == NULL)
		_rasterizer = nsvgCreateRasterizer();

	if (_cachedW != dw || _cachedH != dh) {
		if (_cache)
			free(_cache);

		_cache = (byte *)malloc(dw * dh * 4);

		// Maintain aspect ratio
		float xRatio = 1.0f * dw / _svg->width;
		float yRatio = 1.0f * dh / _svg->height;
		float ratio = xRatio < yRatio ? xRatio : yRatio;

		nsvgRasterize(_rasterizer, _svg, 0, 0, ratio, _cache, dw, dh, dw * 4);

		_cachedW = dw;
		_cachedH = dh;

		if (_render)
			delete _render;

		Graphics::Surface tmp;;
		tmp.init(dw, dh, dw * 4, _cache, *_pixelformat);

		_render = new ManagedSurface(dw, dh, *_pixelformat);

		_render->transBlitFrom(tmp);

		tmp.free();
	}

	target.blitFrom(_render->rawSurface());
}

} // end of namespace Graphics
