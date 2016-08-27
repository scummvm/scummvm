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

#ifndef TITANIC_SURFACE_OBJ_H
#define TITANIC_SURFACE_OBJ_H

#include "titanic/support/rect.h"
#include "titanic/support/video_surface.h"

namespace Titanic {

class CSurfaceArea {
private:
	/**
	 * Initialize data for the class
	 */
	void initialize();
public:
	int _field0;
	int _width;
	int _height;
	int _pitch;
	int _bpp;
	uint16 *_pixelsPtr;
	Point _centroid;
	byte _field20;
	byte _field21;
	byte _field22;
	byte _field23;
	byte _field24;
	byte _field25;
	byte _field26;
	byte _field27;
	int _field28;
	int _field2C;
	int _field30;
	int _field34;
	int _field38;
	Rect _bounds;
public:
	CSurfaceArea(CVideoSurface *surface);
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB16_H */
