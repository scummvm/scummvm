/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA8_GRAPHICS_SHAPE_H
#define ULTIMA8_GRAPHICS_SHAPE_H

#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class ShapeFrame;
struct Palette;
struct Rect;
struct ConvertShapeFormat;
class IDataSource;

class Shape {
public:
	// Parse data, create frames.
	// NB: Shape uses data without copying it. It is deleted on destruction
	// If format is not specified it will be autodetected
	Shape(const uint8 *data, uint32 size, const ConvertShapeFormat *format,
	      const uint16 flexId, const uint32 shapenum);
	Shape(IDataSource *src, const ConvertShapeFormat *format);
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

	ShapeFrame *getFrame(unsigned int frame);

	void getShapeId(uint16 &flexId, uint32 &shapenum);

	// This will detect the format of a shape
	static const ConvertShapeFormat *DetectShapeFormat(const uint8 *data, uint32 size);
	static const ConvertShapeFormat *DetectShapeFormat(IDataSource *ds, uint32 size);

	ENABLE_RUNTIME_CLASSTYPE_BASE()

	ENABLE_CUSTOM_MEMORY_ALLOCATION()

protected:

	// This will load a u8 style shape 'optimized'.
	void LoadU8Format(const uint8 *data, uint32 size, const ConvertShapeFormat *format);

	// This will load a pentagram style shape 'optimized'.
	void LoadPentagramFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format);

	// This will load any sort of shape via a ConvertShapeFormat struct
	// Crusader shapes must be loaded this way
	void LoadGenericFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format);

	Std::vector<ShapeFrame *> _frames;

	const Palette *_palette;

	const uint8 *_data;
	uint32 _size;
	const uint16 _flexId;
	const uint32 _shapeNum;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
