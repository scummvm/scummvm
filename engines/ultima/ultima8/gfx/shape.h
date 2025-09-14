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

#ifndef ULTIMA8_GFX_SHAPE_H
#define ULTIMA8_GFX_SHAPE_H

#include "ultima/shared/std/containers.h"

#include "common/stream.h"

namespace Ultima {
namespace Ultima8 {

class ShapeFrame;
class RawShapeFrame;
class Palette;
struct ConvertShapeFormat;

class Shape {
public:
	// Parse data, create frames.
	// NB: Shape uses data without copying it. It is deleted on destruction
	// If format is not specified it will be autodetected
	Shape(const uint8 *data, uint32 size, const ConvertShapeFormat *format,
	      const uint16 flexId, const uint32 shapenum);
	Shape(Common::SeekableReadStream *src, const ConvertShapeFormat *format);
	virtual ~Shape();
	void setPalette(const Palette *pal) {
		_palette = pal;
	}
	const Palette *getPalette() const {
		return _palette;
	}

	uint32 frameCount() const {
		return static_cast<uint32>(_frames.size());
	}

	//! Returns the dimensions of all frames combined
	//! (w,h) = size of smallest rectangle covering all frames
	//! (x,y) = coordinates of origin relative to top-left point of rectangle
	void getTotalDimensions(int32 &w, int32 &h, int32 &x, int32 &y) const;

	const ShapeFrame *getFrame(unsigned int frame) const;

	void getShapeId(uint16 &flexId, uint32 &shapenum) const;

	// This will detect the format of a shape
	static const ConvertShapeFormat *DetectShapeFormat(const uint8 *data, uint32 size);
	static const ConvertShapeFormat *DetectShapeFormat(Common::SeekableReadStream &ds, uint32 size);

private:
	void loadFrames(const uint8 *data, uint32 size, const ConvertShapeFormat *format);

	// This will load a u8 style shape 'optimized'.
	static Common::Array<RawShapeFrame *> loadU8Format(const uint8 *data, uint32 size, const ConvertShapeFormat *format);

	// This will load a pentagram style shape 'optimized'.
	static Common::Array<RawShapeFrame *> loadPentagramFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format);

	// This will load any sort of shape via a ConvertShapeFormat struct
	// Crusader shapes must be loaded this way
	static Common::Array<RawShapeFrame *> loadGenericFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format);

	Common::Array<ShapeFrame *> _frames;

	const Palette *_palette;

	const uint16 _flexId;
	const uint32 _shapeNum;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
