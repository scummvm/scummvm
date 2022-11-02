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

#include "saga2/saga2.h"
#include "saga2/tile.h"
#include "saga2/tileline.h"

namespace Saga2 {

void TPTriangle(const TilePoint &tp1, const TilePoint &tp2, const TilePoint &tp3, int16 color) {
	TPLine(tp1, tp2, color);
	TPLine(tp2, tp3, color);
	TPLine(tp3, tp1, color);
}

void TPRectangle(const TilePoint &tp1, const TilePoint &tp2, const TilePoint &tp3, const TilePoint &tp4, int16 color) {
	TPLine(tp1, tp2, color);
	TPLine(tp2, tp3, color);
	TPLine(tp3, tp4, color);
	TPLine(tp4, tp1, color);
}

void TPCircle(const TilePoint &tp, const int radius, int16 color) {
	TPLine(tp + TilePoint(radius / 1, radius / 2, 0), tp + TilePoint(radius / 2, radius / 1, 0), color);
	TPLine(tp + TilePoint(radius / 2, radius / 1, 0), tp + TilePoint(radius / -2, radius / 1, 0), color);
	TPLine(tp + TilePoint(radius / -2, radius / 1, 0), tp + TilePoint(radius / -1, radius / 2, 0), color);
	TPLine(tp + TilePoint(radius / -1, radius / 2, 0), tp + TilePoint(radius / -1, radius / -2, 0), color);
	TPLine(tp + TilePoint(radius / -1, radius / -2, 0), tp + TilePoint(radius / -2, radius / -1, 0), color);
	TPLine(tp + TilePoint(radius / -2, radius / -1, 0), tp + TilePoint(radius / 2, radius / -1, 0), color);
	TPLine(tp + TilePoint(radius / 2, radius / -1, 0), tp + TilePoint(radius / 1, radius / -2, 0), color);
	TPLine(tp + TilePoint(radius / 1, radius / -2, 0), tp + TilePoint(radius / 1, radius / 2, 0), color);
}

//  Draw a line in TP space

void TPLine(const TilePoint &start, const TilePoint &stop) {
	Point16         startPt,
	                stopPt;

	TileToScreenCoords(start, startPt);
	TileToScreenCoords(stop, stopPt);

	startPt.x   += kTileRectX;
	stopPt.x    += kTileRectX;
	startPt.y   += kTileRectY;
	stopPt.y    += kTileRectY;

	g_vm->_mainPort.setColor(1);
	g_vm->_mainPort.moveTo(startPt);
	g_vm->_mainPort.drawTo(stopPt);
}

void TPLine(const TilePoint &start, const TilePoint &stop, int16 color) {
	Point16         startPt,
	                stopPt;

	TileToScreenCoords(start, startPt);
	TileToScreenCoords(stop, stopPt);

	startPt.x   += kTileRectX;
	stopPt.x    += kTileRectX;
	startPt.y   += kTileRectY;
	stopPt.y    += kTileRectY;

	g_vm->_mainPort.setColor(color);
	g_vm->_mainPort.moveTo(startPt);
	g_vm->_mainPort.drawTo(stopPt);
}

} // end of namespace Saga2
