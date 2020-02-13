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

#include "pink/archive.h"
#include "pink/pink.h"
#include "pink/objects/condition.h"
#include "pink/objects/actors/lead_actor.h"
#include "pink/objects/pages/game_page.h"

namespace Pink {

void ConditionVariable::deserialize(Archive &archive) {
	_name = archive.readString();
	_value = archive.readString();
}

bool ConditionGameVariable::evaluate(const Actor *actor) const {
	return actor->getPage()->getModule()->getGame()->checkValueOfVariable(_name, _value);
}

void ConditionGameVariable::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tConditionGameVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

bool ConditionModuleVariable::evaluate(const Actor *actor) const {
	return actor->getPage()->getModule()->checkValueOfVariable(_name, _value);
}

void ConditionModuleVariable::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tConditionModuleVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

bool ConditionNotModuleVariable::evaluate(const Actor *actor) const {
	return !ConditionModuleVariable::evaluate(actor);
}

void ConditionNotModuleVariable::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tConditionNotModuleVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

bool ConditionPageVariable::evaluate(const Actor *actor) const {
	return actor->getPage()->checkValueOfVariable(_name, _value);
}

void ConditionPageVariable::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tConditionPageVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

bool ConditionNotPageVariable::evaluate(const Actor *actor) const {
	return !ConditionPageVariable::evaluate(actor);
}

void ConditionNotPageVariable::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tConditionNotPageVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

void ConditionInventoryItemOwner::deserialize(Archive &archive) {
	_item = archive.readString();
	_owner = archive.readString();
}

bool ConditionInventoryItemOwner::evaluate(const Actor *actor) const {
	InventoryMgr *mgr = actor->getInventoryMgr();
	InventoryItem *item = mgr->findInventoryItem(_item);
	if (item)
		return item->getCurrentOwner() == _owner;
	return false;
}

void ConditionInventoryItemOwner::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tConditionInventoryItemOwner: _item=%s, _owner=%s", _item.c_str(), _owner.c_str());
}

bool ConditionNotInventoryItemOwner::evaluate(const Actor *actor) const {
	return !ConditionInventoryItemOwner::evaluate(actor);
}

void ConditionNotInventoryItemOwner::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tConditionNotInventoryItemOwner: _item=%s, _owner=%s", _item.c_str(), _owner.c_str());
}

} // End of namespace Pink
