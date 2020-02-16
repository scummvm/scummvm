/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "common/debug.h"
#include "common/file.h"
#include "graphics/surface.h"

#include "cryomni3d/sprites.h"

// #define SPRTIES_DEBUG

namespace CryOmni3D {

#define MAP_ID(id) \
    do { \
        if (_map) { \
            id = (*_map)[id]; \
        } \
    } while(false)

Sprites::Sprites() : _map(nullptr) {
	_surface = new Graphics::Surface();
}

Sprites::~Sprites() {
	for (Common::Array<CryoCursor *>::iterator it = _cursors.begin(); it != _cursors.end(); it++) {
		if ((*it)->refCnt > 1) {
			(*it)->refCnt--;
		} else {
			delete *it;
		}
	}
	delete _map;
	delete _surface;
}

void Sprites::loadSprites(Common::ReadStream &spr_fl) {
	while (true) {
		uint32 magic = spr_fl.readUint32BE();
		if (spr_fl.eos()) {
			// We are EOS so last read likely failed
			break;
		}
		if (magic != MKTAG('S', 'P', 'R', 'I')) {
			error("Invalid sprite magic");
		}

		// 2 unknown uint32
		(void) spr_fl.readUint32BE();
		(void) spr_fl.readUint32BE();

		CryoCursor *cursor = new CryoCursor();

		uint16 w = spr_fl.readUint16BE();
		uint16 h = spr_fl.readUint16BE();
		uint sz = cursor->setup(w, h);
		cursor->_offX = spr_fl.readUint32BE();
		cursor->_offY = spr_fl.readUint32BE();

		spr_fl.read(cursor->_data, sz);
		_cursors.push_back(cursor);
	}
}

void Sprites::setupMapTable(const uint *table, uint size) {
	delete _map;
	_map = nullptr;
	// Reset the reverse mapping
	for (Common::Array<CryoCursor *>::iterator it = _cursors.begin(); it != _cursors.end(); it++) {
		(*it)->_constantId = uint(-1);
	}
	if (table) {
		_map = new Common::Array<uint>(table, size);

		// Sweep all the mapping and set its reverse values
		uint i = 0;
		for (Common::Array<uint>::const_iterator it = _map->begin(); it != _map->end(); it++, i++) {
			_cursors[*it]->_constantId = i;
		}

#ifdef SPRITES_DEBUG
		// Normally we don't have any unreachable sprties from constants,
		// as it could be time consuming, this should be fixed in the static map
		// Count unswept values
		uint unswept = 0;
		for (Common::Array<CryoCursor *>::iterator it = _cursors.begin(); it != _cursors.end(); it++) {
			if ((*it)->_constantId == -1u) {
				unswept++;
			}
		}

		if (unswept) {
			warning("We got %d unreachable sprites from map table. This should not happen."
			        " Fixing it for now", unswept);
			// Enlarge the map to hold new indexes
			_map->reserve(_map->size() + unswept);

			// Set new indexes to unswept sprites
			i = 0;
			for (Common::Array<CryoCursor *>::iterator it = _cursors.begin(); it != _cursors.end(); it++, i++) {
				if ((*it)->_constantId == -1u) {
					warning("Fixing sprite the %d sprite", i);
					(*it)->_constantId = _map->size();
					_map->push_back(i);
				}
			}
		}
#endif
	}
}

void Sprites::setSpriteHotspot(uint spriteId, uint x, uint y) {
	MAP_ID(spriteId);
	_cursors[spriteId]->_offX = x;
	_cursors[spriteId]->_offY = y;
}

void Sprites::replaceSprite(uint oldSpriteId, uint newSpriteId) {
	MAP_ID(oldSpriteId);
	MAP_ID(newSpriteId);
	if (_cursors[oldSpriteId]->refCnt > 1) {
		_cursors[oldSpriteId]->refCnt--;
	} else {
		delete _cursors[oldSpriteId];
	}
	_cursors[oldSpriteId] = _cursors[newSpriteId];
	_cursors[oldSpriteId]->refCnt++;
}

void Sprites::replaceSpriteColor(uint spriteId, byte currentColor, byte newColor) {
	MAP_ID(spriteId);

	byte *data = _cursors[spriteId]->_data;
	uint size = _cursors[spriteId]->_width * _cursors[spriteId]->_height;
	for (; size > 0; size--, data++) {
		if (*data == currentColor) {
			*data = newColor;
		}
	}
}

uint Sprites::getSpritesCount() const {
	if (_map) {
		return _map->size();
	} else {
		return _cursors.size();
	}
}

uint Sprites::revMapSpriteId(uint id) const {
	if (_map) {
		if (id >= _cursors.size()) {
			error("revMapSpriteId is out of bounds: %d/%d", id, _cursors.size());
		}
		id = _cursors[id]->_constantId;
	}

	return id;
}

uint Sprites::calculateSpriteId(uint baseId, uint offset) const {
	if (_map) {
		MAP_ID(baseId);
		baseId += offset;
		if (baseId >= _cursors.size()) {
			error("Calculate sprite is out of bounds: %d/%d", baseId, _cursors.size());
		}
		uint spriteId = _cursors[baseId]->_constantId;
		if (spriteId == uint(-1)) {
			error("Sprite %d is unreachable", baseId);
		}
		return spriteId;
	} else {
		return baseId + offset;
	}
}

const Graphics::Surface &Sprites::getSurface(uint spriteId) const {
	MAP_ID(spriteId);

	CryoCursor *cursor = _cursors[spriteId];

	_surface->init(cursor->_width, cursor->_height, cursor->_width, cursor->_data,
	               Graphics::PixelFormat::createFormatCLUT8());
	return *_surface;
}

const Graphics::Cursor &Sprites::getCursor(uint spriteId) const {
	MAP_ID(spriteId);

	return *_cursors[spriteId];
}

Sprites::CryoCursor::CryoCursor() : _width(0), _height(0), _offX(0), _offY(0), _data(nullptr),
	refCnt(1), _constantId(uint(-1)) {
}

Sprites::CryoCursor::~CryoCursor() {
	assert(refCnt == 1);
	delete[] _data;
}

uint Sprites::CryoCursor::setup(uint16 width, uint16 height) {
	_width = width;
	_height = height;
	uint sz = _width * _height;
	_data = new byte[sz];
	return sz;
}

} // End of namespace CryOmni3D
