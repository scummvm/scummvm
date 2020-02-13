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

#ifndef PINK_SIDE_EFFECT_H
#define PINK_SIDE_EFFECT_H

#include "pink/utils.h"
#include "pink/objects/object.h"

namespace Pink {

class Actor;

class SideEffect : public Object {
public:
	void deserialize(Archive &archive) override = 0;
	virtual void execute(Actor *actor) = 0;
};

class SideEffectExit : public SideEffect {
public:
	void deserialize(Archive &archive) override;
	void toConsole() const override;
	void execute(Actor *actor) override;

private:
	Common::String _nextModule;
	Common::String _nextPage;
};

class SideEffectLocation : public SideEffect {
public:
	void deserialize(Archive &archive) override;
	void execute(Actor *actor) override;
	void toConsole() const override;

private:
	Common::String _location;
};

class SideEffectInventoryItemOwner : public SideEffect {
public:
	void deserialize(Archive &archive) override;
	void execute(Actor *actor) override;
	void toConsole() const override;

private:
	Common::String _item;
	Common::String _owner;
};

class SideEffectVariable : public SideEffect {
public:
	void deserialize(Archive &archive) override;
	void execute(Actor *actor) override = 0;

protected:
	Common::String _name;
	Common::String _value;
};

class SideEffectGameVariable : public SideEffectVariable {
public:
	void toConsole() const override;
	void execute(Actor *actor) override;
};

class SideEffectModuleVariable : public SideEffectVariable {
public:
	void toConsole() const override;
	void execute(Actor *actor) override;
};

class SideEffectPageVariable : public SideEffectVariable {
public:
	void toConsole() const override;
	void execute(Actor *actor) override;
};

class SideEffectRandomPageVariable : public SideEffect {
public:
	void deserialize(Archive &archive) override;
	void toConsole() const override;
	void execute(Actor *actor) override;

private:
	Common::String _name;
	StringArray _values;
};

} // End of namespace Pink

#endif
