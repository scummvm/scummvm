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

#ifndef COLONY_GFX_H
#define COLONY_GFX_H

#include "colony/renderer.h"

namespace Colony {

class Gfx {
public:
	Gfx(OSystem *system, int width, int height);
	~Gfx();

	void clear(uint32 color) { _renderer->clear(color); }
	void drawLine(int x1, int y1, int x2, int y2, uint32 color) { _renderer->drawLine(x1, y1, x2, y2, color); }
	void drawRect(const Common::Rect &rect, uint32 color) { _renderer->drawRect(rect, color); }
	void fillRect(const Common::Rect &rect, uint32 color) { _renderer->fillRect(rect, color); }
	
	void drawString(const Graphics::Font *font, const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align = Graphics::kTextAlignLeft) {
		_renderer->drawString(font, str, x, y, color, align);
	}
	void scroll(int dx, int dy, uint32 background) { _renderer->scroll(dx, dy, background); }

	void drawEllipse(int x, int y, int rx, int ry, uint32 color) { _renderer->drawEllipse(x, y, rx, ry, color); }
	void fillEllipse(int x, int y, int rx, int ry, uint32 color) { _renderer->fillEllipse(x, y, rx, ry, color); }
	void fillDitherRect(const Common::Rect &rect, uint32 color1, uint32 color2) { _renderer->fillDitherRect(rect, color1, color2); }
	void setPixel(int x, int y, uint32 color) { _renderer->setPixel(x, y, color); }
	void setPalette(const byte *palette, uint start, uint count) { _renderer->setPalette(palette, start, count); }
 
	void begin3D(int camX, int camY, int camZ, int angle, int angleY, const Common::Rect &viewport) { _renderer->begin3D(camX, camY, camZ, angle, angleY, viewport); }
	void draw3DWall(int x1, int y1, int x2, int y2, uint32 color) { _renderer->draw3DWall(x1, y1, x2, y2, color); }
	void draw3DQuad(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4, uint32 color) {
		_renderer->draw3DQuad(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, color);
	}
	void draw3DPolygon(const float *x, const float *y, const float *z, int count, uint32 color) {
		_renderer->draw3DPolygon(x, y, z, count, color);
	}
	void end3D() { _renderer->end3D(); }
	bool isAccelerated() const { return _renderer->isAccelerated(); }

	void drawQuad(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, uint32 color) {
		_renderer->drawQuad(x1, y1, x2, y2, x3, y3, x4, y4, color);
	}
	void drawPolygon(const int *x, const int *y, int count, uint32 color) {
		_renderer->drawPolygon(x, y, count, color);
	}

	void copyToScreen() { _renderer->copyToScreen(); }
	void setWireframe(bool enable) { _renderer->setWireframe(enable); }

	uint32 white() { return 255; }
	uint32 black() { return 0; }

private:
	Renderer *_renderer;
};

} // End of namespace Colony

#endif
