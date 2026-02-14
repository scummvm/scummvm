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

#include "common/system.h"
#include "graphics/managed_surface.h"
#include "colony/renderer.h"

namespace Colony {

class SoftwareRenderer : public Renderer {
public:
	SoftwareRenderer(OSystem *system, int width, int height);
	~SoftwareRenderer();

	void clear(uint32 color) override;
	void drawLine(int x1, int y1, int x2, int y2, uint32 color) override;
	void drawRect(const Common::Rect &rect, uint32 color) override;
	void fillRect(const Common::Rect &rect, uint32 color) override;
	void drawString(const Graphics::Font *font, const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align) override;
	void scroll(int dx, int dy, uint32 background) override;
	bool isAccelerated() const override { return false; }
	void drawEllipse(int x, int y, int rx, int ry, uint32 color) override;
	void fillEllipse(int x, int y, int rx, int ry, uint32 color) override;
	void fillDitherRect(const Common::Rect &rect, uint32 color1, uint32 color2) override;
	void setPixel(int x, int y, uint32 color) override;
	void setPalette(const byte *palette, uint start, uint count) override;

	void begin3D(int camX, int camY, int camZ, int angle, int angleY, const Common::Rect &viewport) override {}
	void draw3DWall(int x1, int y1, int x2, int y2, uint32 color) override {}
	void draw3DQuad(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4, uint32 color) override {}
	void draw3DPolygon(const float *x, const float *y, const float *z, int count, uint32 color) override {}
	void end3D() override {}

	void drawQuad(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, uint32 color) override;
	void drawPolygon(const int *x, const int *y, int count, uint32 color) override;
	void copyToScreen() override;
	void setWireframe(bool enable) override {}

private:
	OSystem *_system;
	Graphics::ManagedSurface _surface;
	int _width;
	int _height;
};

SoftwareRenderer::SoftwareRenderer(OSystem *system, int width, int height) : _system(system), _width(width), _height(height) {
	_surface.create(width, height, _system->getScreenFormat());
}

SoftwareRenderer::~SoftwareRenderer() {
	_surface.free();
}

void SoftwareRenderer::clear(uint32 color) {
	_surface.clear(color);
}

void SoftwareRenderer::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	_surface.drawLine(x1, y1, x2, y2, color);
}

void SoftwareRenderer::drawRect(const Common::Rect &rect, uint32 color) {
	_surface.frameRect(rect, color);
}

void SoftwareRenderer::fillRect(const Common::Rect &rect, uint32 color) {
	_surface.fillRect(rect, color);
}

void SoftwareRenderer::drawString(const Graphics::Font *font, const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align) {
	if (!font) return;
	font->drawString(&_surface, str, x, y, (align == Graphics::kTextAlignCenter && x == 0) ? _width : (_width - x), color, align);
}

void SoftwareRenderer::scroll(int dx, int dy, uint32 background) {
	if (abs(dx) >= _width || abs(dy) >= _height) {
		clear(background);
		return;
	}

	Graphics::ManagedSurface tmp;
	tmp.create(_width, _height, _surface.format);
	tmp.blitFrom(_surface);

	clear(background);
	_surface.blitFrom(tmp, Common::Rect(0, 0, _width, _height), Common::Rect(dx, dy, dx + _width, dy + _height));
	tmp.free();
}

void SoftwareRenderer::drawEllipse(int x, int y, int rx, int ry, uint32 color) {
	_surface.drawEllipse(x - rx, y - ry, x + rx, y + ry, color, false);
}

void SoftwareRenderer::fillEllipse(int x, int y, int rx, int ry, uint32 color) {
	_surface.drawEllipse(x - rx, y - ry, x + rx, y + ry, color, true);
}

void SoftwareRenderer::fillDitherRect(const Common::Rect &rect, uint32 color1, uint32 color2) {
	for (int y = rect.top; y < rect.bottom; y++) {
		for (int x = rect.left; x < rect.right; x++) {
			if ((x + y) % 2 == 0)
				_surface.setPixel(x, y, color1);
			else
				_surface.setPixel(x, y, color2);
		}
	}
}

void SoftwareRenderer::setPixel(int x, int y, uint32 color) {
	if (x >= 0 && x < _width && y >= 0 && y < _height)
		_surface.setPixel(x, y, color);
}

void SoftwareRenderer::setPalette(const byte *palette, uint start, uint count) {
	// Software renderer doesn't need to cache palette, ManagedSurface uses it from system if 8bpp
}

void SoftwareRenderer::drawQuad(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, uint32 color) {
	_surface.drawLine(x1, y1, x2, y2, color);
	_surface.drawLine(x2, y2, x3, y3, color);
	_surface.drawLine(x3, y3, x4, y4, color);
	_surface.drawLine(x4, y4, x1, y1, color);
}

void SoftwareRenderer::drawPolygon(const int *x, const int *y, int count, uint32 color) {
	if (count < 2) return;
	for (int i = 0; i < count; i++) {
		int next = (i + 1) % count;
		_surface.drawLine(x[i], y[i], x[next], y[next], color);
	}
}

void SoftwareRenderer::copyToScreen() {
	_system->copyRectToScreen(_surface.getPixels(), _surface.pitch, 0, 0, _surface.w, _surface.h);
	_system->updateScreen();
}

Renderer *createSoftwareRenderer(OSystem *system, int width, int height) {
	return new SoftwareRenderer(system, width, height);
}

} // End of namespace Colony
