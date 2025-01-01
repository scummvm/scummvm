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

#ifndef TETRAEDGE_TE_TE_IMAGE_H
#define TETRAEDGE_TE_TE_IMAGE_H

#include "common/fs.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/types.h"
#include "common/path.h"

#include "graphics/managed_surface.h"

#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_palette.h"
#include "tetraedge/te/te_vector2s32.h"
#include "tetraedge/te/te_resource.h"

#include "tetraedge/tetraedge.h"

namespace Tetraedge {

class TeImage : public TeResource, public Graphics::ManagedSurface {
public:
	TeImage();
	TeImage(const TeImage &other);

	virtual ~TeImage() {
		destroy();
	};

	enum Format {
		RGB8 = 5,
		RGBA8 = 6,
		// GREY8 = 0xd,
		INVALID = 0xe
	};
	enum SaveType {
		PNG
	};

	void copy(TeImage &dest, const TeVector2s32 &vec1, const TeVector2s32 &vec2,
			  const TeVector2s32 &vec3) const;
	uint64 countPixelsOfColor(const TeColor &col) const;
	// void create(); // never used?
	void createImg(uint xsize, uint ysize, Common::SharedPtr<TePalette> &palette, Format newformat) {
		createImg(xsize, ysize, palette, newformat, xsize, ysize);
	}
	void createImg(uint xsize, uint ysize, Common::SharedPtr<TePalette> &pal,
				Format format, uint bufxsize, uint bufysize);
	void deserialize(Common::ReadStream &stream);
	void destroy();
	void drawPlot(int x, int y, const TeColor &col) {
		drawPlot(getPixels(), x, y, bufSize(), col);
	}
	void drawPlot(void *outbuf, int x, int y, const TeVector2s32 &bufsize, const TeColor &col);
	void fill(byte val);
	void fill(byte r, byte g, byte b, byte a);
	void getBuff(uint x, uint y, byte *pout, uint w, uint h);
	bool isExtensionSupported(const Common::Path &path);
	bool load(const TetraedgeFSNode &node);
	bool load(Common::SeekableReadStream &stream, const Common::String &type);
	bool save(const Common::Path &path, enum SaveType type);
	int serialize(Common::WriteStream &stream);
	TeVector2s32 bufSize() const {
		return TeVector2s32(pitch / format.bytesPerPixel, h);
	}
	Format teFormat() const { return _teFormat; }

private:
	Format _teFormat;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_IMAGE_H
