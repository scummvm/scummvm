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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_TILELINE_H
#define SAGA2_TILELINE_H

#include "saga2/tcoords.h"

namespace Saga2 {

void TPLine(const TilePoint &start, const TilePoint &stop, int16 color);
void TPLine(const TilePoint &start, const TilePoint &stop);
void TPTriangle(const TilePoint &tp1, const TilePoint &tp2, const TilePoint &tp3, int16 color);
void TPRectangle(const TilePoint &tp1, const TilePoint &tp2, const TilePoint &tp3, const TilePoint &tp4, int16 color);
void TPCircle(const TilePoint &tp1, const int radius, int16 color);

} // end of namespace Saga2

#endif
