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

#include <video/flic_decoder.h>
#include <common/substream.h>
#include <graphics/surface.h>
#include "resource_mgr.h"
#include "file.h"
#include "pink.h"
#include "sound.h"
#include "engines/pink/objects/pages/game_page.h"
#include "cel_decoder.h"

namespace Pink {

ResourceMgr::ResourceMgr()
        : _game(nullptr), _resDescTable(nullptr),
          _resCount(0) {}

ResourceMgr::~ResourceMgr() {
    clear();
}

void ResourceMgr::init(PinkEngine *game, GamePage *page) {
    OrbFile *orb = game->getOrb();
    _game = game;

    ObjectDescription *objDesc = orb->getObjDesc(page->getName().c_str());
    _resCount = objDesc->resourcesCount;
    orb->loadObject(page, objDesc);
    _resDescTable = orb->getResDescTable(objDesc);
}

Sound *ResourceMgr::loadSound(Common::String &name) {
    return new Sound(_game->_mixer, getResourceStream(name));
}

Common::SafeSeekableSubReadStream *ResourceMgr::getResourceStream(Common::String &name) {
    Common::SeekableReadStream *stream;
    uint i;
    for (i = 0; i < _resCount; ++i) {
        if (name.compareToIgnoreCase(_resDescTable[i].name) == 0){
            break;
        }
    }
    assert(i < _resDescTable[i].size);

    if (_resDescTable[i].inBro)
        stream = _game->getBro();
    else stream = _game->getOrb();

    stream->seek(_resDescTable[i].offset);

    return new Common::SafeSeekableSubReadStream(stream, _resDescTable[i].offset,
                                             _resDescTable[i].offset + _resDescTable[i].size);
}

PinkEngine *ResourceMgr::getGame() const {
    return _game;
}

CelDecoder *ResourceMgr::loadCEL(Common::String &name) {
    CelDecoder *decoder = new CelDecoder();
    decoder->loadStream(getResourceStream(name));
    return decoder;
}

void ResourceMgr::clear() {
    delete[] _resDescTable;
    _resDescTable = nullptr;
}

} // End of namespace Pink
