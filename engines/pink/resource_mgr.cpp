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

#include "resource_mgr.h"
#include "file.h"
#include "pink.h"
#include "objects/page.h"

namespace Pink {

ResourceMgr::ResourceMgr()
        : _orb(nullptr), _bro(nullptr),
          _resDescTable(nullptr), _resCount(0)
{}

ResourceMgr::~ResourceMgr() {
    delete[] _resDescTable;
}

void ResourceMgr::init(PinkEngine *game, GamePage *page) {
    _orb = game->getOrb();
    _bro = game->getBro();

    ObjectDescription *objDesc = _orb->getObjDesc(page->getName().c_str());
    _resCount = objDesc->resourcesCount;
    _orb->loadObject(page, objDesc);
    _resDescTable = _orb->getResDescTable(objDesc);
}

} // End of namespace Pink
