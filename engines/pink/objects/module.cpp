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

#include "module.h"
#include "page.h"

namespace Pink {

ModuleProxy::ModuleProxy(const Common::String &name)
        : NamedObject(name)
{}

ModuleProxy::ModuleProxy() {}

Module::Module(PinkEngine *game, const Common::String &name)
        : NamedObject(name), _game(game), _page(nullptr)
{}

void Module::load(Archive &archive){
    archive.mapObject(this);
    NamedObject::deserialize(archive);

    archive.readString(); // skip directory

    _invMgr.deserialize(archive);
    archive >> _pages;
}

void Module::init(bool isLoadingSave, const Common::String *pageName) {
    // debugging original
    // 0 0  - new game
    // 0 1 - module changed
    // 1 0 - from save

    // 1 1 - haven't seen those values

    //this func will be rewrited after testing

    if (_page) {
        debug("loading from save");
    }
    if (pageName){
        debug("module changed");
    }
    assert(_pages.size() != 0);

    if (pageName) {
        uint i;
        for (i = 0; i < _pages.size(); ++i) {
            if(*pageName == _pages[i]->getName()) {
                _page = _pages[i];
            }
        }
        assert(i < _pages.size());
    }

    if (_page) {
        _page->init(isLoadingSave); // module changed or from save
        return;
    }

    if (_page != _pages[0]) {
        if (_page) {
            assert(0); // in original code there is call to page func but I've never seen it
            return;
        }
        _page = _pages[0];
        _page->init(isLoadingSave); // new game
        return;
    }

    assert(0);
}

} // End of namespace Pink



