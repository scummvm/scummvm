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

#ifndef COLONY_RENDERER_H
#define COLONY_RENDERER_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "graphics/managed_surface.h"
#include "graphics/font.h"

namespace Colony {

class Renderer {
public:
	virtual ~Renderer() {}

	virtual void clear(uint32 color) = 0;
	virtual void drawLine(int x1, int y1, int x2, int y2, uint32 color) = 0;
	virtual void drawRect(const Common::Rect &rect, uint32 color) = 0;
	virtual void fillRect(const Common::Rect &rect, uint32 color) = 0;
	
	virtual void drawString(const Graphics::Font *font, const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align) = 0;
	virtual void scroll(int dx, int dy, uint32 background) = 0;
	virtual bool isAccelerated() const = 0;
	virtual void drawEllipse(int x, int y, int rx, int ry, uint32 color) = 0;
	virtual void fillEllipse(int x, int y, int rx, int ry, uint32 color) = 0;
	virtual void fillDitherRect(const Common::Rect &rect, uint32 color1, uint32 color2) = 0;
	virtual void setPixel(int x, int y, uint32 color) = 0;

	virtual void setPalette(const byte *palette, uint start, uint count) = 0;

	virtual void begin3D(int camX, int camY, int camZ, int angle, int angleY, const Common::Rect &viewport) = 0;
	virtual void draw3DWall(int x1, int y1, int x2, int y2, uint32 color) = 0;
	virtual void draw3DQuad(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4, uint32 color) = 0;
	virtual void draw3DPolygon(const float *x, const float *y, const float *z, int count, uint32 color) = 0;
	virtual void end3D() = 0;

	virtual void drawQuad(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, uint32 color) = 0;
	virtual void drawPolygon(const int *x, const int *y, int count, uint32 color) = 0;

	virtual void copyToScreen() = 0;
	virtual void setWireframe(bool enable) = 0;
};

Renderer *createSoftwareRenderer(OSystem *system, int width, int height);
Renderer *createOpenGLRenderer(OSystem *system, int width, int height);

} // End of namespace Colony

#endif
