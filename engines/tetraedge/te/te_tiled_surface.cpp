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

#include "tetraedge/te/te_tiled_surface.h"
#include "tetraedge/te/te_frame_anim.h"
#include "tetraedge/te/te_resource_manager.h"

//#define TETRAEDGE_DUMP_LOADED_IMAGES

#ifdef TETRAEDGE_DUMP_LOADED_IMAGES
#include "image/png.h"
#endif

namespace Tetraedge {

static void getRangeIntersection(float start1, float end1, float start2, float end2, float *pstart, float *pend) {
	*pstart = MAX(start1, start2);
	*pend = MIN(end1, end2);
}

TeTiledSurface::TeTiledSurface() : _shouldDraw(true), _codec(nullptr), _colorKeyActive(false), _colorKeyTolerence(0),
_bottomCrop(0), _topCrop(0), _leftCrop(0), _rightCrop(0), _imgFormat(TeImage::INVALID) {
	_frameAnim.frameChangedSignal().add(this, &TeTiledSurface::onFrameAnimCurrentFrameChanged);
}

void TeTiledSurface::cont() {
	_frameAnim.cont();
}

TeTiledSurface::~TeTiledSurface() {
	unload();
}

void TeTiledSurface::draw() {
	if (_tiledTexture && _tiledTexture->isLoaded())
		TeModel::draw();
}

byte TeTiledSurface::isLoaded() {
	return _tiledTexture && _tiledTexture->isLoaded();
}

bool TeTiledSurface::load(const TetraedgeFSNode &node) {
	unload();

	TeResourceManager *resmgr = g_engine->getResourceManager();
	if (_loadedPath.empty())
		_loadedPath = node.getPath();

	Common::Path ttPath(_loadedPath.append(".tt"));
	TeIntrusivePtr<TeTiledTexture> texture;
	if (resmgr->exists(ttPath)) {
		texture = resmgr->getResourceByName<TeTiledTexture>(ttPath);
		// we don't own this one..
	}

	if (!texture) {
		TeCore *core = g_engine->getCore();
		_codec = core->createVideoCodec(node);
		if (!_codec)
			return false;

		texture = new TeTiledTexture();

		if (_codec->load(node)) {
			texture->setAccessName(ttPath);
			resmgr->addResource(texture.get());
			_imgFormat = _codec->imageFormat();

			if (_imgFormat == TeImage::INVALID) {
				warning("TeTiledSurface::load: Wrong image format on file %s", _loadedPath.toString(Common::Path::kNativeSeparator).c_str());
				delete _codec;
				_codec = nullptr;
				return false;
			}
			TeImage img;
			TeVector2s32 newSize = Te3DTexture::optimisedSize(TeVector2s32(_codec->width(), _codec->height()));
			int bufy = _codec->height() + 4;
			if (newSize._y < (int)_codec->height() + 4) {
				bufy = newSize._y;
			}
			int bufx = _codec->width() + 4;
			if (newSize._x < (int)_codec->width() + 4) {
				bufx = newSize._x;
			}

			Common::SharedPtr<TePalette> nullpal;
			img.createImg(_codec->width(), _codec->height(), nullpal, _imgFormat, bufx, bufy);

			if (_codec->update(0, img)) {
#ifdef TETRAEDGE_DUMP_LOADED_IMAGES
				static int dumpCount = 0;
				Common::DumpFile dumpFile;
				dumpFile.open(Common::String::format("/tmp/dump-tiledsurf-%s-%04d.png", name().c_str(), dumpCount));
				dumpCount++;
				Image::writePNG(dumpFile, img);
#endif
				texture->load(img);
			}
		} else {
			warning("TeTiledSurface::load: failed to load %s", _loadedPath.toString(Common::Path::kNativeSeparator).c_str());
			delete _codec;
			_codec = nullptr;
		}
	}

	setTiledTexture(texture);
	return true;
}

bool TeTiledSurface::load(const TeImage &image) {
	error("TODO: Implement TeTiledSurface::load(image)");
}

bool TeTiledSurface::load(const TeIntrusivePtr<Te3DTexture> &texture) {
	unload();

	TeResourceManager *resmgr = g_engine->getResourceManager();
	TeIntrusivePtr<TeTiledTexture> tiledTexture;

	const Common::Path ttPath = texture->getAccessName().append(".tt");

	if (resmgr->exists(ttPath)) {
		tiledTexture = resmgr->getResourceByName<TeTiledTexture>(ttPath);
	}

	if (!tiledTexture) {
		tiledTexture = new TeTiledTexture();
		tiledTexture->load(texture);
		tiledTexture->setAccessName(ttPath);
		resmgr->addResource(tiledTexture.get());
	}

	setTiledTexture(tiledTexture);
	return true;
}

bool TeTiledSurface::onFrameAnimCurrentFrameChanged() {
	if (!_codec)
		return false;

	if (_imgFormat == TeImage::INVALID) {
		warning("TeTiledSurface::load: Wrong image format on file %s", _loadedPath.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	TeImage img;
	TeVector2s32 vidSize(_codec->width(), _codec->height());
	TeVector2s32 optimisedSize = Te3DTexture::optimisedSize(vidSize);

	int bufysize = MIN(vidSize._y + 4, optimisedSize._y);
	int bufxsize = MIN(vidSize._x + 4, optimisedSize._x);

	Common::SharedPtr<TePalette> nullPal;
	img.createImg(vidSize._x, vidSize._y, nullPal, _imgFormat, bufxsize, bufysize);

	if (_codec->update(_frameAnim.lastFrameShown(), img))
		update(img);
	return _codec->isAtEnd();
}

void TeTiledSurface::pause() {
	_frameAnim.pause();
}

void TeTiledSurface::play() {
	if (_codec) {
		_frameAnim.setNbFrames(_codec->nbFrames());
		_frameAnim.setFrameRate(_codec->frameRate());
	}
	_frameAnim.play();
}

void TeTiledSurface::setColorKey(const TeColor &col) {
	_colorKey = col;
	if (_codec)
		_codec->setColorKey(col);
}

void TeTiledSurface::setColorKeyActivated(bool val) {
	_colorKeyActive = true;
	if (_codec)
		_codec->setColorKeyActivated(val);
}

void TeTiledSurface::setColorKeyTolerence(float val) {
	_colorKeyTolerence = val;
	if (_codec)
		_codec->setColorKeyTolerence(val);
}

void TeTiledSurface::setTiledTexture(const TeIntrusivePtr<TeTiledTexture> &texture) {
	_tiledTexture = texture;
	if (texture) {
		_meshes.clear();
		for (uint i = 0; i < texture->numberOfColumns() * texture->numberOfRow(); i++)
			_meshes.push_back(Common::SharedPtr<TeMesh>(TeMesh::makeInstance()));

		setAccessName(texture->getAccessName().append(".surface"));
		updateSurface();
	} else {
		_meshes.clear();
	}
}

void TeTiledSurface::stop() {
	_frameAnim.stop();
}

void TeTiledSurface::unload() {
	// Force stop
	_frameAnim.reset();

	if (_codec) {
		delete _codec;
		_codec = nullptr;
	}
	setTiledTexture(TeIntrusivePtr<TeTiledTexture>());
}

void TeTiledSurface::update(const TeImage &image) {
	_tiledTexture->update(image);
	setTiledTexture(_tiledTexture);
}

void TeTiledSurface::updateSurface() {
	if (!_tiledTexture)
		return;

	const long cols = _tiledTexture->numberOfColumns();
	const long rows = _tiledTexture->numberOfRow();
	int meshno = 0;
	for (long row = 0; row < rows; row++) {
		for (long col = 0; col < cols; col++) {
			TeMesh &mesh = *_meshes[meshno];
			mesh.setConf(4, 4, TeMesh::MeshMode_TriangleStrip, 0, 0);

			mesh.setShouldDraw(_shouldDraw);

			TeTiledTexture::Tile *tile = _tiledTexture->tile(TeVector2s32(col, row));
			mesh.defaultMaterial(tile->_texture);

			TeColor meshcol = color();

			float left, right, top, bottom;
			getRangeIntersection(_leftCrop, 1.0 - _rightCrop, tile->_vec1.x(), tile->_vec2.x() + tile->_vec1.x(), &left, &right);
			getRangeIntersection(_bottomCrop, 1.0 - _topCrop, tile->_vec1.y(), tile->_vec2.y() + tile->_vec1.y(), &top, &bottom);
			if (right < left)
				right = left;
			if (bottom < top)
				bottom = top;

			const float scaled_l = (left - tile->_vec1.x()) / tile->_vec2.x();
			const float scaled_r = (right - tile->_vec1.x()) / tile->_vec2.x();
			const float scaled_t = (top - tile->_vec1.y()) / tile->_vec2.y();
			const float scaled_b = (bottom - tile->_vec1.y()) / tile->_vec2.y();

			mesh.setVertex(0, TeVector3f32(left - 0.5f, top - 0.5f, 0.0f));
			mesh.setTextureUV(0, TeVector2f32(scaled_l, scaled_t));
			mesh.setNormal(0, TeVector3f32(0.0f, 0.0f, 1.0f));
			mesh.setColor(0, meshcol);
			mesh.setVertex(1, TeVector3f32(right - 0.5f, top - 0.5f, 0.0f));
			mesh.setTextureUV(1, TeVector2f32(scaled_r, scaled_t));
			mesh.setNormal(1, TeVector3f32(0.0f, 0.0f, 1.0f));
			mesh.setColor(1, meshcol);
			mesh.setVertex(2, TeVector3f32(right - 0.5f, bottom - 0.5f, 0.0f));
			mesh.setTextureUV(2, TeVector2f32(scaled_r, scaled_b));
			mesh.setNormal(2, TeVector3f32(0.0f, 0.0f, 1.0f));
			mesh.setColor(2, meshcol);
			mesh.setVertex(3, TeVector3f32(left - 0.5f, bottom - 0.5f, 0.0f));
			mesh.setTextureUV(3, TeVector2f32(scaled_l, scaled_b));
			mesh.setNormal(3, TeVector3f32(0.0f, 0.0f, 1.0f));
			mesh.setColor(3, meshcol);
			mesh.setIndex(0, 0);
			mesh.setIndex(1, 1);
			mesh.setIndex(2, 3);
			mesh.setIndex(3, 2);
			meshno++;
		}
	}
}

void TeTiledSurface::updateVideoProperties() {
	if (_codec) {
		_codec->setColorKeyActivated(_colorKeyActive);
		_codec->setColorKey(_colorKey);
		_codec->setColorKeyTolerence(_colorKeyTolerence);
	}
}

} // end namespace Tetraedge
