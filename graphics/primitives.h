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

#ifndef GRAPHICS_PRIMITIVES_H
#define GRAPHICS_PRIMITIVES_H

#include "common/rect.h"

namespace Graphics {

void drawLine(int x0, int y0, int x1, int y1, int color, void (*plotProc)(int, int, int, void *), void *data);
void drawHLine(int x1, int x2, int y, int color, void (*plotProc)(int, int, int, void *), void *data);
void drawVLine(int x, int y1, int y2, int color, void (*plotProc)(int, int, int, void *), void *data);
void drawThickLine(int x0, int y0, int x1, int y1, int penX, int penY, int color, void (*plotProc)(int, int, int, void *), void *data);
void drawThickLine2(int x1, int y1, int x2, int y2, int thick, int color,
								void (*plotProc)(int, int, int, void *), void *data);
void drawFilledRect(Common::Rect &rect, int color, void (*plotProc)(int, int, int, void *), void *data);
void drawRect(Common::Rect &rect, int color, void (*plotProc)(int, int, int, void *), void *data);
void drawRoundRect(Common::Rect &rect, int arc, int color, bool filled, void (*plotProc)(int, int, int, void *), void *data);
void drawPolygonScan(int *polyX, int *polyY, int npoints, Common::Rect &bbox, int color,
								void (*plotProc)(int, int, int, void *), void *data);
void drawEllipse(int x0, int y0, int x1, int y1, int color, bool filled, void (*plotProc)(int, int, int, void *), void *data);

} // End of namespace Graphics

#endif
