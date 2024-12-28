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

#ifndef GRAPHICS_PRIMITIVES_H
#define GRAPHICS_PRIMITIVES_H

#include "common/rect.h"

namespace Graphics {

class Primitives {
public:
	virtual ~Primitives() {}

	virtual void drawPoint(int x, int y, uint32 color, void *data) = 0;

	virtual void drawLine(int x0, int y0, int x1, int y1, uint32 color, void *data);
	virtual void drawHLine(int x1, int x2, int y, uint32 color, void *data);
	virtual void drawVLine(int x, int y1, int y2, uint32 color, void *data);
	virtual void drawThickLine(int x0, int y0, int x1, int y1, int penX, int penY, uint32 color, void *data);
	virtual void drawThickLine2(int x1, int y1, int x2, int y2, int thick, uint32 color, void *data);
	virtual void drawFilledRect(const Common::Rect &rect, uint32 color, void *data);
	virtual void drawFilledRect1(const Common::Rect &rect, uint32 color, void *data);
	virtual void drawRect(const Common::Rect &rect, uint32 color, void *data);
	virtual void drawRect1(const Common::Rect &rect, uint32 color, void *data);
	virtual void drawRoundRect(const Common::Rect &rect, int arc, uint32 color, bool filled, void *data);
	virtual void drawRoundRect1(const Common::Rect &rect, int arc, uint32 color, bool filled, void *data);
	virtual void drawPolygonScan(const int *polyX, const int *polyY, int npoints, const Common::Rect &bbox, uint32 color, void *data);
	virtual void drawEllipse(int x0, int y0, int x1, int y1, uint32 color, bool filled, void *data);
};

WARN_DEPRECATED("Use a subclass of Graphics::Primitives")
void drawLine(int x0, int y0, int x1, int y1, uint32 color, void (*plotProc)(int, int, int, void *), void *data);
WARN_DEPRECATED("Use a subclass of Graphics::Primitives")
void drawHLine(int x1, int x2, int y, uint32 color, void (*plotProc)(int, int, int, void *), void *data);
WARN_DEPRECATED("Use a subclass of Graphics::Primitives")
void drawVLine(int x, int y1, int y2, uint32 color, void (*plotProc)(int, int, int, void *), void *data);
WARN_DEPRECATED("Use a subclass of Graphics::Primitives")
void drawThickLine(int x0, int y0, int x1, int y1, int penX, int penY, uint32 color, void (*plotProc)(int, int, int, void *), void *data);
WARN_DEPRECATED("Use a subclass of Graphics::Primitives")
void drawThickLine2(int x1, int y1, int x2, int y2, int thick, uint32 color,
								void (*plotProc)(int, int, int, void *), void *data);
WARN_DEPRECATED("Use a subclass of Graphics::Primitives")
void drawFilledRect(const Common::Rect &rect, uint32 color, void (*plotProc)(int, int, int, void *), void *data);
WARN_DEPRECATED("Use a subclass of Graphics::Primitives")
void drawFilledRect1(const Common::Rect &rect, uint32 color, void (*plotProc)(int, int, int, void *), void *data);
WARN_DEPRECATED("Use a subclass of Graphics::Primitives")
void drawRect(const Common::Rect &rect, uint32 color, void (*plotProc)(int, int, int, void *), void *data);
WARN_DEPRECATED("Use a subclass of Graphics::Primitives")
void drawRect1(const Common::Rect &rect, uint32 color, void (*plotProc)(int, int, int, void *), void *data);
WARN_DEPRECATED("Use a subclass of Graphics::Primitives")
void drawRoundRect(const Common::Rect &rect, int arc, uint32 color, bool filled, void (*plotProc)(int, int, int, void *), void *data);
WARN_DEPRECATED("Use a subclass of Graphics::Primitives")
void drawRoundRect1(const Common::Rect &rect, int arc, uint32 color, bool filled, void (*plotProc)(int, int, int, void *), void *data);
WARN_DEPRECATED("Use a subclass of Graphics::Primitives")
void drawPolygonScan(const int *polyX, const int *polyY, int npoints, const Common::Rect &bbox, uint32 color,
								void (*plotProc)(int, int, int, void *), void *data);
WARN_DEPRECATED("Use a subclass of Graphics::Primitives")
void drawEllipse(int x0, int y0, int x1, int y1, uint32 color, bool filled, void (*plotProc)(int, int, int, void *), void *data);

} // End of namespace Graphics

#endif
