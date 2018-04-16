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

#ifndef PRINCE_OBJECT_H
#define PRINCE_OBJECT_H

#include "image/image_decoder.h"
#include "graphics/surface.h"

namespace Prince {

class Object {
public:
	Object();
	~Object();

	int32 _x;
	int32 _y;
	int32 _z;
	uint16 _width;
	uint16 _height;
	int32 _flags;
	int32 _zoomTime;
	Graphics::Surface *_zoomSurface;

	// Used instead of offset in setData and getData
	enum AttrId {
		kObjectAddr = 0,
		kObjectX = 4,
		kObjectY = 6,
		kObjectZ = 8,
		kObjectFlags = 10,
		kObjectZoomInSource = 12,
		kObjectZoomInLen = 16,
		kObjectZoomInAddr = 20,
		kObjectZoomInTime = 24
	};

	bool loadFromStream(Common::SeekableReadStream &stream);
	Graphics::Surface *getSurface() const { return _surface; }
	int32 getData(AttrId dataId);
	void setData(AttrId dataId, int32 value);

private:
	void loadSurface(Common::SeekableReadStream &stream);
	Graphics::Surface *_surface;
};

} // End of namespace Prince

#endif
