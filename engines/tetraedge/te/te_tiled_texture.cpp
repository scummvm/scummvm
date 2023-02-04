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

#include "tetraedge/tetraedge.h"

#include "tetraedge/te/te_resource_manager.h"
#include "tetraedge/te/te_tiled_texture.h"
#include "tetraedge/te/te_tiled_surface.h"

namespace Tetraedge {

TeTiledTexture::TeTiledTexture() : _tileSize(0x800, 0x800), _skipBlank(false) {
}

uint TeTiledTexture::imageFormat() {
	if (!_tileArray.empty())
		return _tileArray[0]._texture->getFormat();
	return TeImage::INVALID;
}

bool TeTiledTexture::isLoaded() {
	return !_tileArray.empty();
}


bool TeTiledTexture::load(const Common::String &path) {
	release();
	TeIntrusivePtr<TeImage> img;
	TeResourceManager *resmgr = g_engine->getResourceManager();
	if (resmgr->exists(path)) {
		img = resmgr->getResourceByName<TeImage>(path);
	} else {
		img = new TeImage();
		TeCore *core = g_engine->getCore();
		if (!img->load(core->findFile(path)))
			return false;
	}
	load(*img);
	return true;
}

bool TeTiledTexture::load(const TeImage &img) {
	release();

	Common::Array<TeImage> imgArray;
	imgArray.reserve(4);

	_totalSize._x = img.w;
	_totalSize._y = img.h;
	_somethingSize = TeVector2s32(0, 0);
	const int rows = (int)ceilf((float)_totalSize._x / _tileSize._x);
	const int cols = (int)ceilf((float)_totalSize._y / _tileSize._y);
	_tileArray.resize(cols * rows);

	for (int col = 0; col < cols; col++) {
		for (int row = 0; row < rows; row++) {
			const TeVector2s32 remainingSize(img.w - _tileSize._x * row, img.h - _tileSize._y * col);
			TeVector2s32 newTileSize = remainingSize;

			if (_tileSize._x < remainingSize._x) {
				newTileSize = TeVector2s32(_tileSize._x, remainingSize._y);
			}
			if (_tileSize._y < remainingSize._y) {
				newTileSize = TeVector2s32(remainingSize._x, _tileSize._y);
			}

			const TeImage *tileimage;
			if (newTileSize != _totalSize) {
				TeImage *optimizedimg = optimisedTileImage(imgArray, newTileSize, Common::SharedPtr<TePalette>(), img.teFormat());
				img.copy(*optimizedimg, TeVector2s32(0, 0), TeVector2s32(_tileSize._x * row, _tileSize._y * col), newTileSize);
				//optimizedimg->_flipY = img._flipY;
				Common::String accessName = Common::String::format("%s.Tile%dx%d", img.getAccessName().c_str(), row, col);
				optimizedimg->setAccessName(accessName);
				tileimage = optimizedimg;
			} else {
				tileimage = &img;
			}

			Tile *tiledata = tile(TeVector2s32(row, col));
			if (!_skipBlank || (int)tileimage->countPixelsOfColor(TeColor(0, 0, 0, 0)) != (tileimage->h * tileimage->w)) {
				tiledata->_texture = Te3DTexture::makeInstance();
				tiledata->_texture->load(*tileimage);
				tiledata->_vec2 = TeVector3f32
						((float)tiledata->_texture->width() / (float)_totalSize._x,
						 (float)tiledata->_texture->height() / (float)_totalSize._y, 0.0);
				_somethingSize += TeVector2s32(tiledata->_texture->width(), tiledata->_texture->height());
			} else {
				tiledata->_texture.release();
				tiledata->_vec2 = TeVector3f32(0, 0, 0);
				_somethingSize = Te3DTexture::optimisedSize(TeVector2s32(tileimage->w, tileimage->h));
			}
			tiledata->_vec1 = TeVector3f32(row * ((float)_tileSize._x / _totalSize._x),
										   col * ((float)_tileSize._y / _totalSize._y), 0.0);
		}
	}

	if (cols)
		_somethingSize._x = _somethingSize._x / cols;
	if (rows)
		_somethingSize._y = _somethingSize._y / rows;
	setAccessName(img.getAccessName() + ".tt");
	return true;
}

bool TeTiledTexture::load(const TeIntrusivePtr<Te3DTexture> &texture) {
	release();
	_tileSize._x = texture->width();
	_tileSize._y = texture->height();
	_totalSize = _tileSize;
	_tileArray.resize(1);
	Tile *tileData = tile(TeVector2s32(0, 0));
	tileData->_texture = texture;
	tileData->_vec2 = TeVector3f32(1, 1, 0);
	tileData->_vec1 = TeVector3f32(0, 0, 0);
	setAccessName(texture->getAccessName() + ".tt");
	return true;
}

uint32 TeTiledTexture::numberOfColumns() const {
	return ceilf((float)_totalSize._x / (float)_tileSize._x);
}

uint32 TeTiledTexture::numberOfRow() const {
	return ceilf((float)_totalSize._y / (float)_tileSize._y);
}

/*static*/
TeImage *TeTiledTexture::optimisedTileImage(Common::Array<TeImage> &images, const TeVector2s32 &size,
							const Common::SharedPtr<TePalette> &pal, enum TeImage::Format format) {
	for (TeImage &image : images) {
		if (image.w == size._x && image.h == size._y && image.teFormat() == format) {
			return &image;
		}
	}
	images.resize(images.size() + 1);
	TeImage &newImg = images.back();
	Common::SharedPtr<TePalette> nullPal;
	newImg.createImg((uint)size._x, (uint)size._y, nullPal, format);
	return &newImg;
}

void TeTiledTexture::release() {
	_tileArray.clear();
	_totalSize = TeVector2s32();
	_somethingSize = TeVector2s32();
}

TeTiledTexture::Tile *TeTiledTexture::tile(const TeVector2s32 &loc) {
	int columns = ceilf((float)_totalSize._x / (float)_tileSize._x);
	uint tileno = columns * loc._y + loc._x;
	if (tileno >= _tileArray.size())
		error("Invalid tile %d/%d requested in TeTiledTexture::tile", tileno + 1, _tileArray.size());
	return &_tileArray[tileno];
}

void TeTiledTexture::update(const TeImage &image) {
// TODO: Implement fast path of TeTiledTexture::update
#if 0
	if (image.w == _totalSize._x && image.h == _totalSize._y) {
		// fast path code goes here
	} else {
		load(image);
	}
#else
	load(image);
#endif
}

} // end namespace Tetraedge
