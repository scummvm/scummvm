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

#include "common/textconsole.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

#include "backends/graphics3d/opengl/tiledsurface.h"
#include "backends/graphics3d/opengl/surfacerenderer.h"
#include "graphics/opengl/texture.h"

namespace OpenGL {

TiledSurface::TiledSurface(uint width, uint height, const Graphics::PixelFormat &pixelFormat) {
	_backingSurface.create(width, height, pixelFormat);

	for (uint y = 0; y < height; y += maxTextureSize) {
		for (uint x = 0; x < width; x += maxTextureSize) {
			uint textureWidth = (x + maxTextureSize >= width) ? (width - x) : maxTextureSize;
			uint textureHeight = (y + maxTextureSize >= height) ? (height - y) : maxTextureSize;

			_tiles.push_back(Tile());

			Tile &tile = _tiles.back();
			tile.rect = Common::Rect(textureWidth, textureHeight);
			tile.rect.translate(x, y);
			tile.texture = new Texture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
			tile.texture->setSize(textureWidth, textureHeight);
			tile.texture->enableLinearFiltering(true);
			tile.dirty = true;
		}
	}
}

TiledSurface::~TiledSurface() {
	for (uint i = 0; i < _tiles.size(); i++) {
		delete _tiles[i].texture;
	}
	_backingSurface.free();
}

void TiledSurface::copyRectToSurface(const void *src, int srcPitch, int x, int y, int w, int h) {
	_backingSurface.copyRectToSurface(src, srcPitch, x, y, w, h);

	Common::Rect destRect = Common::Rect(w, h);
	destRect.translate(x, y);

	for (uint i = 0; i < _tiles.size(); i++) {
		if (_tiles[i].rect.intersects(destRect) || _tiles[i].rect.contains(destRect)) {
			_tiles[i].dirty = true;
		}
	}
}

void TiledSurface::update() {
	for (uint i = 0; i < _tiles.size(); i++) {
		Tile &tile = _tiles[i];
		if (tile.dirty) {
			const Common::Rect rect(tile.rect.width(), tile.rect.height());

			// TODO: Restore support for GL_UNPACK_ROW_LENGTH?
			Graphics::Surface subSurface = _backingSurface.getSubArea(tile.rect);
			Graphics::Surface *conv = subSurface.convertTo(OpenGL::Texture::getRGBAPixelFormat());

			tile.texture->updateArea(rect, *conv);

			conv->free();
			delete conv;

			tile.dirty = false;
		}
	}
}

void TiledSurface::draw(SurfaceRenderer *surfaceRenderer) const {
	for (uint i = 0; i < _tiles.size(); i++) {
		const Tile &tile = _tiles[i];

		assert(tile.texture);
		assert(!tile.dirty);

		Math::Vector2d topLeft = Math::Vector2d(tile.rect.left / (float)_backingSurface.w, tile.rect.top / (float)_backingSurface.h);
		Math::Vector2d bottomRight = Math::Vector2d(tile.rect.right / (float)_backingSurface.w, tile.rect.bottom / (float)_backingSurface.h);

		surfaceRenderer->render(tile.texture, Math::Rect2d(topLeft, bottomRight));
	}
}

void TiledSurface::fill(uint32 color) {
	Common::Rect rect = Common::Rect(_backingSurface.w, _backingSurface.h);
	_backingSurface.fillRect(rect, color);

	invalidateAllTiles();
}

void TiledSurface::invalidateAllTiles() {
	for (uint i = 0; i < _tiles.size(); i++) {
		_tiles[i].dirty = true;
	}
}

Graphics::Surface *TiledSurface::getBackingSurface() {
	invalidateAllTiles();
	return &_backingSurface;
}

const Graphics::Surface *TiledSurface::getBackingSurface() const {
	return &_backingSurface;
}

} // End of namespace OpenGL

#endif
