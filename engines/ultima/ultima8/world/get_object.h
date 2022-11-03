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

#ifndef ULTIMA8_WORLD_GETOBJECT_H
#define ULTIMA8_WORLD_GETOBJECT_H

namespace Ultima {
namespace Ultima8 {

// utility functions for getting objects by ObjId in various forms

class Object;
class Item;
class Container;
class Actor;
class MainActor;
class Gump;

Object *getObject(ObjId id);
Item *getItem(ObjId id);
Container *getContainer(ObjId id);
Actor *getActor(ObjId id);
MainActor *getMainActor();
Actor *getControlledActor();

Gump *getGump(ObjId id);

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
