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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/world/get_object.h"

#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/world/world.h"

namespace Ultima {
namespace Ultima8 {

// utility functions for getting objects by ObjId in various forms

Object *getObject(ObjId id) {
	return ObjectManager::get_instance()->getObject(id);
}

Item *getItem(ObjId id) {
	return dynamic_cast<Item *>(ObjectManager::get_instance()->getObject(id));
}

Container *getContainer(ObjId id) {
	return dynamic_cast<Container *>(ObjectManager::get_instance()->getObject(id));
}

Actor *getActor(ObjId id) {
	return dynamic_cast<Actor *>(ObjectManager::get_instance()->getObject(id));
}

MainActor *getMainActor() {
	return dynamic_cast<MainActor *>(ObjectManager::get_instance()->getObject(1));
}

Actor *getControlledActor() {
	uint16 num = World::get_instance()->getControlledNPCNum();
	return dynamic_cast<Actor *>(ObjectManager::get_instance()->getObject(num));
}

Gump *getGump(ObjId id) {
	return dynamic_cast<Gump *>(ObjectManager::get_instance()->getObject(id));
}

} // End of namespace Ultima8
} // End of namespace Ultima
