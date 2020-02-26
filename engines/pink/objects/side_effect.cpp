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

#include "common/hash-str.h"

#include "pink/archive.h"
#include "pink/pink.h"
#include "pink/objects/side_effect.h"
#include "pink/objects/actors/lead_actor.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/walk/walk_location.h"
#include "pink/objects/walk/walk_mgr.h"

namespace Pink {

void SideEffectExit::deserialize(Archive &archive) {
	_nextModule = archive.readString();
	_nextPage = archive.readString();
}

void SideEffectExit::execute(Actor *actor) {
	actor->getPage()->getLeadActor()->setNextExecutors(_nextModule, _nextPage);
}

void SideEffectExit::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tSideEffectExit: _nextModule=%s, _nextPage=%s", _nextModule.c_str(), _nextPage.c_str());
}

void SideEffectLocation::deserialize(Archive &archive) {
	_location = archive.readString();
}

void SideEffectLocation::execute(Actor *actor) {
	WalkMgr *mgr = actor->getPage()->getWalkMgr();
	WalkLocation *location = mgr->findLocation(_location);
	if (location)
		mgr->setCurrentWayPoint(location);
}

void SideEffectLocation::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tSideEffectLocation: _location=%s", _location.c_str());
}

void SideEffectInventoryItemOwner::deserialize(Archive &archive) {
	_item = archive.readString();
	_owner = archive.readString();
}

void SideEffectInventoryItemOwner::execute(Actor *actor) {
	InventoryMgr *mgr = actor->getInventoryMgr();
	InventoryItem *item = mgr->findInventoryItem(_item);
	mgr->setItemOwner(_owner, item);
}

void SideEffectInventoryItemOwner::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tSideEffectInventoryItemOwner: _item=%s, _owner=%s", _item.c_str(), _owner.c_str());
}

void SideEffectVariable::deserialize(Pink::Archive &archive) {
	_name = archive.readString();
	_value = archive.readString();
}

void SideEffectGameVariable::execute(Actor *actor) {
	actor->getPage()->getGame()->setVariable(_name, _value);
}

void SideEffectGameVariable::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tSideEffectGameVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

void SideEffectModuleVariable::execute(Actor *actor) {
	actor->getPage()->getModule()->setVariable(_name, _value);
}

void SideEffectModuleVariable::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tSideEffectModuleVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

void SideEffectPageVariable::execute(Actor *actor) {
	actor->getPage()->setVariable(_name, _value);
}

void SideEffectPageVariable::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tSideEffectPageVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

void SideEffectRandomPageVariable::deserialize(Archive &archive) {
	_name = archive.readString();
	_values.deserialize(archive);
}

void SideEffectRandomPageVariable::execute(Actor *actor) {
	assert(!_values.empty());

	Common::RandomSource &rnd = actor->getPage()->getGame()->getRnd();
	uint index = rnd.getRandomNumber(_values.size() - 1);

	actor->getPage()->setVariable(_name, _values[index]);
}

void SideEffectRandomPageVariable::toConsole() const {
	Common::String values("{");
	for (uint i = 0; i < _values.size(); ++i) {
		values += _values[i];
		values += ',';
	}
	values += '}';
	debugC(6, kPinkDebugLoadingObjects, "\t\tSideEffectRandomPageVariable: _name=%s, _values=%s", _name.c_str(), values.c_str());
}

} // End of namespace Pink
