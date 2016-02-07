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

#ifndef TITANIC_DIRECT_DRAW_H
#define TITANIC_DIRECT_DRAW_H

#include "common/scummsys.h"
#include "common/array.h"
#include "titanic/titanic.h"

namespace Titanic {

class DirectDraw: public Manager {
public:
	int _field8;
	int _fieldC;
	int _width;
	int _height;
	int _bpp;
	int _field1C;
	int _field24;
public:
	DirectDraw(TitanicEngine *vm);
};

class DirectDrawManager: public Manager {
public:
	DirectDraw _directDraw;
	void *_mainSurface;
	void *_backSurfaces[2];
public:
	DirectDrawManager(TitanicEngine *vm);
};

} // End of namespace Titanic

#endif /* TITANIC_DIRECT_DRAW_H */
