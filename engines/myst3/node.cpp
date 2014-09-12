/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/myst3/effects.h"
#include "engines/myst3/node.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/state.h"
#include "engines/myst3/subtitles.h"

#include "common/debug.h"
#include "common/rect.h"

namespace Myst3 {

void Face::setTextureFromJPEG(const DirectorySubEntry *jpegDesc) {
	_bitmap = Myst3Engine::decodeJpeg(jpegDesc);
	_texture = _vm->_gfx->createTexture(_bitmap);
}

Face::Face(Myst3Engine *vm) :
		_vm(vm),
		_textureDirty(true),
		_texture(0),
		_bitmap(0),
		_finalBitmap(0) {
}

void Face::uploadTexture() {
	if (_textureDirty) {
		if (_finalBitmap)
			_texture->update(_finalBitmap);
		else
			_texture->update(_bitmap);

		_textureDirty = false;
	}
}

Face::~Face() {
	_bitmap->free();
	delete _bitmap;
	_bitmap = 0;

	if (_finalBitmap) {
		_finalBitmap->free();
		delete _finalBitmap;
	}

	if (_texture) {
		_vm->_gfx->freeTexture(_texture);
	}
}

Node::Node(Myst3Engine *vm, uint16 id) :
		_vm(vm),
		_subtitles(0) {
	for (uint i = 0; i < 6; i++)
		_faces[i] = 0;

	if (_vm->_state->getWaterEffects()) {
		Effect *effect = WaterEffect::create(vm, id);
		if (effect) {
			_effects.push_back(effect);
			_vm->_state->setWaterEffectActive(true);
		}
	}

	Effect *effect = MagnetEffect::create(vm, id);
	if (effect) {
		_effects.push_back(effect);
		_vm->_state->setMagnetEffectActive(true);
	}

	effect = LavaEffect::create(vm, id);
	if (effect) {
		_effects.push_back(effect);
		_vm->_state->setLavaEffectActive(true);
	}

	effect = ShieldEffect::create(vm, id);
	if (effect) {
		_effects.push_back(effect);
		_vm->_state->setShieldEffectActive(true);
	}
}

Node::~Node() {
	for (uint i = 0; i < _spotItems.size(); i++) {
		delete _spotItems[i];
	}
	_spotItems.clear();

	for (uint i = 0; i < _effects.size(); i++) {
		delete _effects[i];
	}
	_effects.clear();
	_vm->_state->setWaterEffectActive(false);
	_vm->_state->setMagnetEffectActive(false);
	_vm->_state->setLavaEffectActive(false);
	_vm->_state->setShieldEffectActive(false);

	for (int i = 0; i < 6; i++) {
		delete _faces[i];
	}

	delete _subtitles;
}

void Node::loadSpotItem(uint16 id, uint16 condition, bool fade) {
	SpotItem *spotItem = new SpotItem(_vm);

	spotItem->setCondition(condition);
	spotItem->setFade(fade);
	spotItem->setFadeVar(abs(condition));

	for (int i = 0; i < 6; i++) {
		const DirectorySubEntry *jpegDesc = _vm->getFileDescription(0, id, i + 1, DirectorySubEntry::kLocalizedSpotItem);

		if (!jpegDesc)
			jpegDesc = _vm->getFileDescription(0, id, i + 1, DirectorySubEntry::kSpotItem);

		if (!jpegDesc) continue;

		SpotItemFace *spotItemFace = new SpotItemFace(
				_faces[i],
				jpegDesc->getSpotItemData().u,
				jpegDesc->getSpotItemData().v);

		spotItemFace->loadData(jpegDesc);

		// SpotItems with an always true conditions cannot be undrawn.
		// Draw them now to make sure the "non drawn backups" for other, potentially
		// overlapping SpotItems have them drawn.
		if (condition == 1) {
			spotItemFace->draw();
		}

		spotItem->addFace(spotItemFace);
	}

	_spotItems.push_back(spotItem);
}

SpotItemFace *Node::loadMenuSpotItem(uint16 condition, const Common::Rect &rect) {
	SpotItem *spotItem = new SpotItem(_vm);

	spotItem->setCondition(condition);
	spotItem->setFade(false);
	spotItem->setFadeVar(abs(condition));

	SpotItemFace *spotItemFace = new SpotItemFace(_faces[0], rect.left, rect.top);
	spotItemFace->initBlack(rect.width(), rect.height());

	spotItem->addFace(spotItemFace);

	_spotItems.push_back(spotItem);

	return spotItemFace;
}

void Node::loadSubtitles(uint32 id) {
	_subtitles = Subtitles::create(_vm, id);
}

bool Node::hasSubtitlesToDraw() {
	if (!_subtitles)
		return false;

	return _vm->_state->getSpotSubtitle() > 0;
}

void Node::drawOverlay() {
	if (hasSubtitlesToDraw()) {
		uint subId = _vm->_state->getSpotSubtitle();
		_subtitles->setFrame(15 * subId + 1);
		_subtitles->drawOverlay();
	}
}

void Node::update() {
	// First undraw ...
	for (uint i = 0; i < _spotItems.size(); i++) {
		_spotItems[i]->updateUndraw();
	}

	// ... then redraw
	for (uint i = 0; i < _spotItems.size(); i++) {
		_spotItems[i]->updateDraw();
	}

	bool needsUpdate = false;
	for (uint i = 0; i < _effects.size(); i++) {
		needsUpdate |= _effects[i]->update();
	}

	// Apply the effects for all the faces
	for (uint faceId = 0; faceId < 6; faceId++) {
		Face *face = _faces[faceId];

		if (face == 0)
			continue;

		uint effectsForFace = 0;
		for (uint i = 0; i < _effects.size(); i++) {
			if (_effects[i]->hasFace(faceId))
				effectsForFace++;
		}

		if (effectsForFace == 0)
			continue;
		if (!needsUpdate && !face->isTextureDirty())
			continue;

		// Alloc the target surface if necessary
		if (!face->_finalBitmap) {
			face->_finalBitmap = new Graphics::Surface();
		}
		face->_finalBitmap->copyFrom(*face->_bitmap);

		if (effectsForFace == 1) {
			_effects[0]->applyForFace(faceId, face->_bitmap, face->_finalBitmap);

			face->markTextureDirty();
		} else if (effectsForFace == 2) {
			// TODO: Keep the same temp surface to avoid heap fragmentation ?
			Graphics::Surface *tmp = new Graphics::Surface();
			tmp->copyFrom(*face->_bitmap);

			_effects[0]->applyForFace(faceId, face->_bitmap, tmp);
			_effects[1]->applyForFace(faceId, tmp, face->_finalBitmap);

			tmp->free();
			delete tmp;

			face->markTextureDirty();
		} else {
			error("Unable to render more than 2 effects per faceId (%d)", effectsForFace);
		}
	}
}

SpotItem::SpotItem(Myst3Engine *vm) :
	_vm(vm) {
}

SpotItem::~SpotItem() {
	for (uint i = 0; i < _faces.size(); i++) {
		delete _faces[i];
	}
}

void SpotItem::updateUndraw() {
	for (uint i = 0; i < _faces.size(); i++) {
		if (!_vm->_state->evaluate(_condition) && _faces[i]->isDrawn()) {
			_faces[i]->undraw();
		}
	}
}

void SpotItem::updateDraw() {
	for (uint i = 0; i < _faces.size(); i++) {
		if (_enableFade) {
			uint16 newFadeValue = _vm->_state->getVar(_fadeVar);

			if (_faces[i]->getFadeValue() != newFadeValue) {
				_faces[i]->setFadeValue(newFadeValue);
				_faces[i]->setDrawn(false);
			}
		}

		if (_vm->_state->evaluate(_condition) && !_faces[i]->isDrawn()) {
			if (_enableFade)
				_faces[i]->fadeDraw();
			else
				_faces[i]->draw();
		}
	}
}

SpotItemFace::SpotItemFace(Face *face, uint16 posX, uint16 posY):
		_face(face),
		_posX(posX),
		_posY(posY),
		_drawn(false),
		_bitmap(0),
		_notDrawnBitmap(0),
		_fadeValue(0) {
}

SpotItemFace::~SpotItemFace() {
	if (_bitmap) {
		_bitmap->free();
		delete _bitmap;
		_bitmap = 0;
	}

	if (_notDrawnBitmap) {
		_notDrawnBitmap->free();
		delete _notDrawnBitmap;
		_notDrawnBitmap = 0;
	}
}

void SpotItemFace::initBlack(uint16 width, uint16 height) {
	_bitmap = new Graphics::Surface();
	_bitmap->create(width, height, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	initNotDrawn(width, height);
}

void SpotItemFace::loadData(const DirectorySubEntry *jpegDesc) {
	// Convert active SpotItem image to raw data
	_bitmap = Myst3Engine::decodeJpeg(jpegDesc);

	initNotDrawn(_bitmap->w, _bitmap->h);
}

void SpotItemFace::updateData(const Graphics::Surface *surface) {
	assert(_bitmap && surface);

	_bitmap->free();
	_bitmap->copyFrom(*surface);

	// Ensure the pixel format is correct
	_bitmap->convertToInPlace(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	_drawn = false;
}

void SpotItemFace::clear() {
	memset(_bitmap->getPixels(), 0, _bitmap->pitch * _bitmap->h);

	_drawn = false;
}

void SpotItemFace::initNotDrawn(uint16 width, uint16 height) {
	// Copy not drawn SpotItem image from face
	_notDrawnBitmap = new Graphics::Surface();
	_notDrawnBitmap->create(width, height, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	for (uint i = 0; i < height; i++) {
		memcpy(_notDrawnBitmap->getBasePtr(0, i),
				_face->_bitmap->getBasePtr(_posX, _posY + i), width * 4);
	}
}

void SpotItemFace::draw() {
	for (uint i = 0; i < _bitmap->h; i++) {
		memcpy(_face->_bitmap->getBasePtr(_posX, _posY + i),
				_bitmap->getBasePtr(0, i),
				_bitmap->w * 4);
	}

	_drawn = true;
	_face->markTextureDirty();
}

void SpotItemFace::undraw() {
	for (uint i = 0; i < _notDrawnBitmap->h; i++) {
		memcpy(_face->_bitmap->getBasePtr(_posX, _posY + i),
				_notDrawnBitmap->getBasePtr(0, i),
				_notDrawnBitmap->w * 4);
	}

	_drawn = false;
	_face->markTextureDirty();
}

void SpotItemFace::fadeDraw() {
	uint16 fadeValue = CLIP<uint16>(_fadeValue, 0, 100);

	for (int i = 0; i < _bitmap->h; i++) {
		byte *ptrND = (byte *)_notDrawnBitmap->getBasePtr(0, i);
		byte *ptrD = (byte *)_bitmap->getBasePtr(0, i);
		byte *ptrDest = (byte *)_face->_bitmap->getBasePtr(_posX, _posY + i);

		for (int j = 0; j < _bitmap->w; j++) {
			byte rND = *ptrND++;
			byte gND = *ptrND++;
			byte bND = *ptrND++;
			ptrND++; // Alpha
			byte rD = *ptrD++;
			byte gD = *ptrD++;
			byte bD = *ptrD++;
			ptrD++; // Alpha

			// TODO: optimize ?
			*ptrDest++ = rND * (100 - fadeValue) / 100 + rD * fadeValue / 100;
			*ptrDest++ = gND * (100 - fadeValue) / 100 + gD * fadeValue / 100;
			*ptrDest++ = bND * (100 - fadeValue) / 100 + bD * fadeValue / 100;
			ptrDest++; // Alpha
		}
	}

	_drawn = true;
	_face->markTextureDirty();
}

} // end of namespace Myst3
