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
#include "saga2/rect.h"

namespace Saga2 {

void Point16::load(Common::SeekableReadStream *stream) {
	x = stream->readSint16LE();
	y = stream->readSint16LE();
}

void Point16::write(Common::MemoryWriteStreamDynamic *out) {
	out->writeSint16LE(x);
	out->writeSint16LE(y);
}

void Rect16::read(Common::InSaveFile *in) {
	x = in->readSint16LE();
	y = in->readSint16LE();
	width = in->readSint16LE();
	height = in->readSint16LE();
}

void Rect16::write(Common::MemoryWriteStreamDynamic *out) {
	out->writeSint16LE(x);
	out->writeSint16LE(y);
	out->writeSint16LE(width);
	out->writeSint16LE(height);
}

Rect16 bound(const Rect16 a, const Rect16 b) {
	int16               x1, x2, y1, y2;

	x1 = MIN(a.x, b.x);
	x2 = MAX(a.x + a.width, b.x + b.width) - x1;
	y1 = MIN(a.y, b.y);
	y2 = MAX(a.y + a.height, b.y + b.height) - y1;

	return Rect16(x1, y1, x2, y2);
}

void Rect16::expand(int16 dx, int16 dy) {
	x -= dx;
	width += dx + dx;
	y -= dy;
	height += dy + dy;
}

void Rect16::expand(int16 left, int16 top, int16 right, int16 bottom) {
	x -= left;
	width  += left + right;
	y -= top;
	height += top + bottom;
}

Rect16 intersect(const Rect16 a, const Rect16 b) {
	int16               x1, width, y1, height;

	x1      = MAX(a.x, b.x);
	width   = MIN(a.x + a.width, b.x + b.width) - x1;
	y1      = MAX(a.y, b.y);
	height  = MIN(a.y + a.height, b.y + b.height) - y1;

	if ((width <= 0) || (height <= 0))
		return Rect16(0, 0, 0, 0);
	else
		return Rect16(x1, y1, width, height);
}

void Rect16::normalize() {
	if (width < 0) {
		x += width;
		width = -width;
	}
	if (height < 0) {
		y += height;
		height = - height;
	}
}

bool Rect16::overlap(const Rect16 &r) const {
	return (r.x < x + width  && x < r.x + r.width
	        && r.y < y + height && y < r.y + r.height);
}

Rect32 bound(const Rect32 a, const Rect32 b) {
	int32               x1, x2, y1, y2;

	x1 = MIN(a.x, b.x);
	x2 = MAX(a.x + a.width, b.x + b.width) - x1;
	y1 = MIN(a.y, b.y);
	y2 = MAX(a.y + a.height, b.y + b.height) - y1;

	return Rect32(x1, y1, x2, y2);
}

void Rect32::expand(int32 dx, int32 dy) {
	x -= dx;
	width += dx + dx;
	y -= dy;
	height += dy + dy;
}

void Rect32::expand(int32 left, int32 top, int32 right, int32 bottom) {
	x -= left;
	width  += left + right;
	y -= top;
	height += top + bottom;
}

Rect32 intersect(const Rect32 a, const Rect32 b) {
	int32               x1, width, y1, height;

	x1      = MAX(a.x, b.x);
	width   = MIN(a.x + a.width, b.x + b.width) - x1;
	y1      = MAX(a.y, b.y);
	height  = MIN(a.y + a.height, b.y + b.height) - y1;

//	if ( ( width <= 0 ) || ( height <= 0 ) )
//		return Rect32( 0, 0, 0, 0 );
//	else
	return Rect32(x1, y1, width, height);
}

void Rect32::normalize() {
	if (width < 0) {
		x += width;
		width = -width;
	}
	if (height < 0) {
		y += height;
		height = - height;
	}
}

bool Rect32::overlap(const Rect32 &r) const {
	return (r.x < x + width  && x < r.x + r.width
	        && r.y < y + height && y < r.y + r.height);
}

} // end if namespace Saga2
