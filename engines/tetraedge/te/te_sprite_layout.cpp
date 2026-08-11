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

#include "common/file.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_matrix4x4.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_sprite_layout.h"

namespace Tetraedge {

TeSpriteLayout::TeSpriteLayout() : _tiledSurfacePtr(new TeTiledSurface()), _sizeSet(false) {
	_tiledSurfacePtr->setColor(TeColor(255, 255, 255, 255));
	//_tiledSurfacePtr->_shouldDraw = true // should already be true..

	updateMesh();
}

int TeSpriteLayout::bufferSize() {
	return _tiledSurfacePtr->bufferSize();
}

void TeSpriteLayout::cont() {
	_tiledSurfacePtr->cont();
}

void TeSpriteLayout::draw() {
	if (!worldVisible())
		return;

	/*
	if (name() == "DEPLIANT")
		debug("Draw SpriteLayout %p (%s, surf %s, size %.01fx%.01f, surf %.01fx%.01f, %s)", this,
			  name().empty() ? "no name" : name().c_str(), _tiledSurfacePtr->getAccessName().toString().c_str(),
			  size().x(), size().y(),
			  _tiledSurfacePtr->size().x(), _tiledSurfacePtr->size().y(), color().dump().c_str());*/
	TeMatrix4x4 matrix = worldTransformationMatrix();

	if (sizeType() == ABSOLUTE) {
		matrix(0, 3) = (int)matrix(0, 3);
		matrix(1, 3) = (int)matrix(1, 3);
	}

	TeRenderer *renderer = g_engine->getRenderer();
	renderer->pushMatrix();
	renderer->loadMatrix(matrix);
	_tiledSurfacePtr->draw();
	renderer->popMatrix();
	TeLayout::draw();
}

bool TeSpriteLayout::onParentWorldColorChanged() {
	Te3DObject2::onParentWorldColorChanged();
	setColor(color());
	return false;
}

bool TeSpriteLayout::load(const Common::Path &path) {
	if (path.empty()) {
		_tiledSurfacePtr->unload();
		return true;
	}

	TeCore *core = g_engine->getCore();
	TetraedgeFSNode spriteNode = core->findFile(path);

	// The path can point to a single file, or a folder with files
	if (!spriteNode.exists()) {
		_tiledSurfacePtr->unload();
		return false;
	}

	stop();
	unload();

	_tiledSurfacePtr->setLoadedPath(path);
	if (_tiledSurfacePtr->load(spriteNode)) {
		const TeVector2s32 texSize = _tiledSurfacePtr->tiledTexture()->totalSize();
		if (texSize._y <= 0) {
			setRatio(1.0);
		} else {
			setRatio(texSize._x / texSize._y);
		}
		if (sizeType() == CoordinatesType::ABSOLUTE && !_sizeSet) {
			setSize(TeVector3f32(texSize._x, texSize._y, 1.0));
		}
		updateMesh();
	} else {
		debug("Failed to load TeSpriteLayout %s", spriteNode.toString().c_str());
		_tiledSurfacePtr->setLoadedPath("");
	}
	return true;
}

bool TeSpriteLayout::load(TeIntrusivePtr<Te3DTexture> &texture) {
	unload();

	if (_tiledSurfacePtr->load(texture)) {
		const TeVector2s32 tiledTexSize = _tiledSurfacePtr->tiledTexture()->totalSize();
		if (tiledTexSize._y <= 0) {
			setRatio(1.0);
		} else {
			setRatio((float)tiledTexSize._x / tiledTexSize._y);
		}

		if (sizeType() == CoordinatesType::ABSOLUTE && !_sizeSet) {
			setSize(TeVector3f32(tiledTexSize._x, tiledTexSize._y, 1.0));
		}
		updateMesh();
		return true;
	} else {
		debug("Failed to load TeSpriteLayout from texture %s", texture->getAccessName().toString(Common::Path::kNativeSeparator).c_str());
	}
	return false;
}

bool TeSpriteLayout::load(TeImage &img) {
	unload();

	if (_tiledSurfacePtr->load(img)) {
		const TeVector2s32 tiledTexSize = _tiledSurfacePtr->tiledTexture()->totalSize();
		if (tiledTexSize._y <= 0) {
			setRatio(1.0);
		} else {
			setRatio((float)tiledTexSize._x / tiledTexSize._y);
		}

		if (sizeType() == CoordinatesType::ABSOLUTE && !_sizeSet) {
			setSize(TeVector3f32(tiledTexSize._x, tiledTexSize._y, 1.0));
		}
		updateMesh();
		return true;
	} else {
		debug("Failed to load TeSpriteLayout from texture %s", img.getAccessName().toString(Common::Path::kNativeSeparator).c_str());
	}
	return false;
}

void TeSpriteLayout::play() {
	_tiledSurfacePtr->play();
}

void TeSpriteLayout::unload() {
	_tiledSurfacePtr->unload();
}

void TeSpriteLayout::pause() {
	_tiledSurfacePtr->pause();
}

void TeSpriteLayout::setBufferSize(int bufsize) {
	_tiledSurfacePtr->setBufferSize(bufsize);
}

void TeSpriteLayout::setColor(const TeColor &col) {
	Te3DObject2::setColor(col);
	_tiledSurfacePtr->setColor(color());
}

void TeSpriteLayout::setColorKey(const TeColor &col) {
	_tiledSurfacePtr->setColorKey(col);
}

void TeSpriteLayout::setColorKeyActivated(bool activated) {
	_tiledSurfacePtr->setColorKeyActivated(activated);
}

void TeSpriteLayout::setColorKeyTolerence(float val) {
	_tiledSurfacePtr->setColorKeyTolerence(val);
}

bool TeSpriteLayout::setName(const Common::String &newName) {
	TeLayout::setName(newName);
	_tiledSurfacePtr->setName(newName);
	return true;
}

void TeSpriteLayout::setSize(const TeVector3f32 &newSize) {
	TeLayout::setSize(newSize);
	_sizeSet = true;
}

void TeSpriteLayout::stop() {
	_tiledSurfacePtr->stop();
}

void TeSpriteLayout::updateMesh() {
	TeLayout::updateMesh();
	// Surface always renders to a 1x1 mesh.
	const TeVector3f32 surfaceScale(xSize(), -ySize(), 1.0);
	_tiledSurfacePtr->setScale(surfaceScale);
}

void TeSpriteLayout::updateSize() {
	//if (!name().empty()) {
		TeLayout::updateSize();
		updatePosition();
	//}
}

} // end namespace Tetraedge
