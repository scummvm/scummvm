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

void Pink::ConditionVariable::deserialize(Archive &archive) {
	_name = archive.readString();
	_value = archive.readString();
}

bool Pink::ConditionGameVariable::evaluate(Actor *actor) {
	return actor->getPage()->getModule()->getGame()->checkValueOfVariable(_name, _value);
}

void ConditionGameVariable::toConsole() {
	debug("\t\tConditionGameVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

bool Pink::ConditionModuleVariable::evaluate(Actor *actor) {
	return actor->getPage()->getModule()->checkValueOfVariable(_name, _value);
}

void ConditionModuleVariable::toConsole() {
	debug("\t\tConditionModuleVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

bool Pink::ConditionNotModuleVariable::evaluate(Actor *actor) {
	return !ConditionModuleVariable::evaluate(actor);
}

void ConditionNotModuleVariable::toConsole() {
	debug("\t\tConditionNotModuleVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

bool ConditionPageVariable::evaluate(Actor *actor) {
	return actor->getPage()->checkValueOfVariable(_name, _value);
}

void ConditionPageVariable::toConsole() {
	debug("\t\tConditionPageVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

bool ConditionNotPageVariable::evaluate(Actor *actor) {
	return !ConditionPageVariable::evaluate(actor);
}

void ConditionNotPageVariable::toConsole() {
	debug("\t\tConditionNotPageVariable: _name=%s, _value=%s", _name.c_str(), _value.c_str());
}

void ConditionInventoryItemOwner::deserialize(Archive &archive) {
	_item = archive.readString();
	_owner = archive.readString();
}

bool ConditionInventoryItemOwner::evaluate(Actor *actor) {
	InventoryMgr *mgr = actor->getPage()->getModule()->getInventoryMgr();
	InventoryItem *item = mgr->findInventoryItem(_item);
	return item->getCurrentOwner() == _owner;
}

void ConditionInventoryItemOwner::toConsole() {
	debug("\t\tConditionInventoryItemOwner: _item=%s, _owner=%s", _item.c_str(), _owner.c_str());
}

bool ConditionNotInventoryItemOwner::evaluate(Actor *actor) {
	return !ConditionInventoryItemOwner::evaluate(actor);
}

void ConditionNotInventoryItemOwner::toConsole() {
	debug("\t\tConditionNotInventoryItemOwner: _item=%s, _owner=%s", _item.c_str(), _owner.c_str());
}

} // End of namespace Pink
