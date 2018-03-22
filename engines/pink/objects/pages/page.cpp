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
#include "engines/pink/objects/actors/lead_actor.h"


namespace Pink {

void Page::load(Archive &archive) {
    archive.mapObject(this);
    NamedObject::deserialize(archive);
    archive.readString(); //skip directory
    archive >> _actors;
}

Actor *Page::findActor(Common::String &name) {
    return *Common::find_if(_actors.begin(), _actors.end(), [&name]
            (Actor *actor) {
        return name == actor->getName();
    });;
}

Sound *Page::loadSound(Common::String &fileName) {
    return _resMgr.loadSound(fileName);
}

void Page::toConsole() {
    for (int i = 0; i < _actors.size(); ++i) {
        _actors[i]->toConsole();
    }
}

} // End of namespace Pink
