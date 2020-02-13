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

#ifndef PINK_CONDITION_H
#define PINK_CONDITION_H

#include "pink/objects/object.h"

namespace Pink {

class Actor;

class Condition : public Object {
public:
	void deserialize(Archive &archive) override = 0;
	virtual bool evaluate(Actor *actor) = 0;
};

class ConditionVariable : public Condition {
public:

	void deserialize(Archive &archive) override;
	bool evaluate(Actor *actor) override = 0;

protected:
	Common::String _name;
	Common::String _value;
};

class ConditionGameVariable : public ConditionVariable {
public:
	void toConsole() override;
	bool evaluate(Actor *actor) override;
};

/*
 * It is not used in games and has evaluate method with infinity recursion
class ConditionNotGameVariable : public ConditionGameVariable {
	virtual bool evaluate(LeadActor *leadActor);
};
 */

class ConditionModuleVariable : public ConditionVariable {
public:
	void toConsole() override;
	bool evaluate(Actor *actor) override;
};

class ConditionNotModuleVariable : public ConditionModuleVariable {
public:
	void toConsole() override;
	bool evaluate(Actor *actor) override;
};

class ConditionPageVariable : public ConditionVariable {
public:
	void toConsole() override;
	bool evaluate(Actor *actor) override;
};

class ConditionNotPageVariable : public ConditionPageVariable {
public:
	void toConsole() override;
	bool evaluate(Actor *actor) override;
};

class ConditionInventoryItemOwner : public Condition {
public:
	void toConsole() override;
	void deserialize(Archive &archive) override;
	bool evaluate(Actor *actor) override;

protected:
	Common::String _item;
	Common::String _owner;
};

class ConditionNotInventoryItemOwner : public ConditionInventoryItemOwner {
public:
	void toConsole() override;
	bool evaluate(Actor *actor) override;
};

} // End of namespace Pink

#endif
