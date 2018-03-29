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

#include <common/hash-str.h>
#include "side_effect.h"
#include <engines/pink/archive.h>
#include <engines/pink/objects/actors/lead_actor.h>
#include <engines/pink/objects/pages/game_page.h>
#include <engines/pink/pink.h>
#include <engines/pink/objects/walk/walk_location.h>
#include <engines/pink/objects/walk/walk_mgr.h>


namespace Pink {

void SideEffectExit::deserialize(Archive &archive) {
    archive >> _nextModule >> _nextPage;
}

void SideEffectExit::execute(LeadActor *actor) {
    actor->setNextExecutors(_nextModule, _nextPage);
}

void SideEffectExit::toConsole() {
    debug("\t\tSideEffectExit: _nextModule=%s, _nextPage=%s", _nextModule.c_str(), _nextPage.c_str());
}

void SideEffectLocation::deserialize(Archive &archive) {
    archive >> _location;
}

void SideEffectLocation::execute(LeadActor *actor) {
    WalkMgr *mgr = actor->getPage()->getWalkMgr();
    WalkLocation *location = mgr->findLocation(_location);
    //TODO end this method
}

void SideEffectLocation::toConsole() {
    debug("\t\tSideEffectLocation: _location=%s", _location.c_str());
}

void SideEffectInventoryItemOwner::deserialize(Archive &archive) {
    archive >> _item >> _owner;
}

void SideEffectInventoryItemOwner::execute(LeadActor *actor) {
    //TODO
}

void SideEffectInventoryItemOwner::toConsole() {
    debug("\t\tSideEffectInventoryItemOwner: _item=%s, _owner=%s", _item.c_str(), _owner.c_str());
}

void SideEffectVariable::deserialize(Pink::Archive &archive) {
    archive >> _name >> _value;
}

void SideEffectGameVariable::execute(LeadActor *actor) {
    actor->getPage()->getGame()->setVariable(_name, _value);
}

void SideEffectGameVariable::toConsole() {
    debug("\t\tSideEffectGameVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

void SideEffectModuleVariable::execute(LeadActor *actor) {
   actor->getPage()->getModule()->setVariable(_name, _value);
}

void SideEffectModuleVariable::toConsole() {
    debug("\t\tSideEffectModuleVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

void SideEffectPageVariable::execute(LeadActor *actor) {
    actor->getPage()->setVariable(_name, _value);
}

void SideEffectPageVariable::toConsole() {
    debug("\t\tSideEffectPageVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

void SideEffectRandomPageVariable::deserialize(Archive &archive) {
    archive >> _name >> _values;
}

void SideEffectRandomPageVariable::execute(LeadActor *actor) {
    assert(!_values.empty());

    Common::RandomSource &rnd = actor->getPage()->getGame()->getRnd();
    uint index = rnd.getRandomNumber(_values.size() - 1);

    actor->getPage()->setVariable(_name, _values[index]);
}

void SideEffectRandomPageVariable::toConsole() {
    Common::String values("{");
    for (int i = 0; i < _values.size(); ++i) {
        values += _values[i];
        values += ',';
    }
    values += '}';
    debug("\t\tSideEffectRandomPageVariable: _name=%s, _values=%s", _name.c_str(), values.c_str());
}

} // End of namespace Pink