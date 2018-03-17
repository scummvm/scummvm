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

#include "page.h"

namespace Pink {

void Page::load(Archive &archive) {
    archive.mapObject(this);
    NamedObject::deserialize(archive);
    archive.readString(); //skip directory
    // deserialize actors
}

void GamePage::deserialize(Archive &archive) {
    Page::deserialize(archive);
    _module = static_cast<Module*>(archive.readObject());
    assert(dynamic_cast<Module*>(_module) != 0);
}

void GamePage::load(Archive &archive) {
    //archive.mapObject(_cursorMgr);
    //archive.mapObject(_walkMgr);
    //archive.mapObject(_sequencer);
    //_leadActor = archive.readObject()
    //serialize ccursormgr NullSub
    //serialize walkmgr
    //serialize sequencer
    //serialize handlers
}

void GamePage::init(bool isLoadingSave) {
    if (isLoadingSave){
        assert(perhapsIsLoaded == 0);
        // loadSerialize
    }
}

} // End of namespace Pink
