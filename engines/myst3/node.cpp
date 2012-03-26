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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/myst3/node.h"
#include "engines/myst3/menu.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/state.h"
#include "engines/myst3/subtitles.h"

#include "common/debug.h"
#include "common/rect.h"

#include "graphics/conversion.h"

namespace Myst3 {

void Face::setTextureFromJPEG(Graphics::JPEGDecoder *jpeg) {
	_bitmap = new Graphics::Surface();
	_bitmap->create(jpeg->getComponent(1)->w, jpeg->getComponent(1)->h, Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0));

	byte *y = (byte *)jpeg->getComponent(1)->getBasePtr(0, 0);
	byte *u = (byte *)jpeg->getComponent(2)->getBasePtr(0, 0);
	byte *v = (byte *)jpeg->getComponent(3)->getBasePtr(0, 0);
	
	byte *ptr = (byte *)_bitmap->getBasePtr(0, 0);
	for (int i = 0; i < _bitmap->w * _bitmap->h; i++) {
		byte r, g, b;
		Graphics::YUV2RGB(*y++, *u++, *v++, r, g, b);
		*ptr++ = r;
		*ptr++ = g;
		*ptr++ = b;
	}

	_texture = _vm->_gfx->createTexture(_bitmap);
}

Face::Face(Myst3Engine *vm) :
	_vm(vm),
	_textureDirty(true),
	_texture(0) {
}

void Face::uploadTexture() {
	if (_textureDirty) {
		_texture->update(_bitmap);
		_textureDirty = false;
	}
}

Face::~Face() {
	_bitmap->free();
	delete _bitmap;
	_bitmap = 0;

	if (_texture) {
		_vm->_gfx->freeTexture(_texture);
	}
}

Node::Node(Myst3Engine *vm, uint16 id) :
	_vm(vm),
	_subtitles(0) {
	for (uint i = 0; i < 6; i++)
		_faces[i] = 0;
}

void Node::dumpFaceMask(uint16 index, int face) {
	static const int32 kMaskSize = 640 * 640;

	byte *mask = new byte[kMaskSize];
	memset(mask, 0, kMaskSize);
	uint32 headerOffset = 0;
	uint32 dataOffset = 0;

	const DirectorySubEntry *maskDesc = _vm->getFileDescription(0, index, face, DirectorySubEntry::kFaceMask);

	if (!maskDesc)
		return;

	Common::MemoryReadStream *maskStream = maskDesc->getData();

	while (headerOffset < 400) {
		int blockX = (headerOffset / sizeof(dataOffset)) % 10;
		int blockY = (headerOffset / sizeof(dataOffset)) / 10;

		maskStream->seek(headerOffset, SEEK_SET);
		dataOffset = maskStream->readUint32LE();
		headerOffset = maskStream->pos();

		if (dataOffset != 0) {
			maskStream->seek(dataOffset, SEEK_SET);

			for(int i = 63; i >= 0; i--) {
				int x = 0;
				byte numValues = maskStream->readByte();
				for (int j = 0; j < numValues; j++) {
					byte repeat = maskStream->readByte();
					byte value = maskStream->readByte();
					for (int k = 0; k < repeat; k++) {
						mask[((blockY * 64) + i) * 640 + blockX * 64 + x] = value;
						x++;
					}
				}
			}
		}
	}

	delete maskStream;

	Common::DumpFile outFile;
	outFile.open("dump/1-1.masku");
	outFile.write(mask, kMaskSize);
	outFile.close();
	delete[] mask;
}

Node::~Node() {
	for (uint i = 0; i < _spotItems.size(); i++) {
		delete _spotItems[i];
	}
	_spotItems.clear();

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
		const DirectorySubEntry *jpegDesc = _vm->getFileDescription(0, id, i + 1, DirectorySubEntry::kSpotItem);

		if (!jpegDesc)
			jpegDesc = _vm->getFileDescription(0, id, i + 1, DirectorySubEntry::kMenuSpotItem);

		if (!jpegDesc) continue;

		SpotItemFace *spotItemFace = new SpotItemFace(
				_faces[i],
				jpegDesc->getSpotItemData().u,
				jpegDesc->getSpotItemData().v);

		Common::MemoryReadStream *jpegStream = jpegDesc->getData();

		Graphics::JPEGDecoder jpeg;
		if (!jpeg.loadStream(*jpegStream))
			error("Could not decode Myst III JPEG");
		
		spotItemFace->loadData(&jpeg);

		delete jpegStream;

		spotItem->addFace(spotItemFace);
	}

	_spotItems.push_back(spotItem);
}

void Node::loadMenuSpotItem(uint16 id, uint16 condition, const Common::Rect &rect) {
	SpotItem *spotItem = new SpotItem(_vm);

	spotItem->setCondition(condition);
	spotItem->setFade(false);
	spotItem->setFadeVar(abs(condition));

	SpotItemFace *spotItemFace = new SpotItemFace(_faces[0], rect.left, rect.top);
	spotItemFace->initBlack(rect.width(), rect.height());

	if (id == 1)
		_vm->_menu->setSaveLoadSpotItem(spotItemFace);

	spotItem->addFace(spotItemFace);

	_spotItems.push_back(spotItem);
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
	_fadeValue(0)
{
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
	_bitmap->create(width, height, Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0));

	initNotDrawn(width, height);
}

void SpotItemFace::loadData(Graphics::JPEGDecoder *jpeg) {
	// Convert active SpotItem image to raw data
	_bitmap = new Graphics::Surface();
	_bitmap->create(jpeg->getComponent(1)->w, jpeg->getComponent(1)->h, Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0));

	for (int i = 0; i < _bitmap->h; i++) {
		byte *y = (byte *)jpeg->getComponent(1)->getBasePtr(0, i);
		byte *u = (byte *)jpeg->getComponent(2)->getBasePtr(0, i);
		byte *v = (byte *)jpeg->getComponent(3)->getBasePtr(0, i);

		byte *ptr = (byte *)_bitmap->getBasePtr(0, i);

		for (int j = 0; j < _bitmap->w; j++) {
			byte r, g, b;
			Graphics::YUV2RGB(*y++, *u++, *v++, r, g, b);
			*ptr++ = r;
			*ptr++ = g;
			*ptr++ = b;
		}
	}

	initNotDrawn(_bitmap->w, _bitmap->h);
}

void SpotItemFace::updateData(const uint8 *data) {
	assert(_bitmap && data);
	memcpy(_bitmap->pixels, data, _bitmap->pitch * _bitmap->h);

	_drawn = false;
}

void SpotItemFace::clear() {
	memset(_bitmap->pixels, 0, _bitmap->pitch * _bitmap->h);

	_drawn = false;
}

void SpotItemFace::initNotDrawn(uint16 width, uint16 height) {
	// Copy not drawn SpotItem image from face
	_notDrawnBitmap = new Graphics::Surface();
	_notDrawnBitmap->create(width, height, Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0));

	for (uint i = 0; i < height; i++) {
		memcpy(_notDrawnBitmap->getBasePtr(0, i),
				_face->_bitmap->getBasePtr(_posX, _posY + i), width * 3);
	}
}

void SpotItemFace::draw() {
	for (uint i = 0; i < _bitmap->h; i++) {
		memcpy(_face->_bitmap->getBasePtr(_posX, _posY + i),
				_bitmap->getBasePtr(0, i),
				_bitmap->w * 3);
	}

	_drawn = true;
	_face->markTextureDirty();
}

void SpotItemFace::undraw() {
	for (uint i = 0; i < _notDrawnBitmap->h; i++) {
		memcpy(_face->_bitmap->getBasePtr(_posX, _posY + i),
				_notDrawnBitmap->getBasePtr(0, i),
				_notDrawnBitmap->w * 3);
	}

	_drawn = false;
	_face->markTextureDirty();
}

void SpotItemFace::fadeDraw() {
	for (int i = 0; i < _bitmap->h; i++) {
		byte *ptrND = (byte *)_notDrawnBitmap->getBasePtr(0, i);
		byte *ptrD = (byte *)_bitmap->getBasePtr(0, i);
		byte *ptrDest = (byte *)_face->_bitmap->getBasePtr(_posX, _posY + i);

		for (int j = 0; j < _bitmap->w; j++) {
			byte rND = *ptrND++;
			byte gND = *ptrND++;
			byte bND = *ptrND++;
			byte rD = *ptrD++;
			byte gD = *ptrD++;
			byte bD = *ptrD++;

			// TODO: optimize ?
			*ptrDest++ = rND * (100 - _fadeValue) / 100 + rD * _fadeValue / 100;
			*ptrDest++ = gND * (100 - _fadeValue) / 100 + gD * _fadeValue / 100;
			*ptrDest++ = bND * (100 - _fadeValue) / 100 + bD * _fadeValue / 100;
		}
	}

	_drawn = true;
	_face->markTextureDirty();
}

} // end of namespace Myst3
