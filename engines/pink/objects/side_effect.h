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

#include <engines/pink/objects/object.h>
#include <common/str-array.h>

namespace Pink {

class LeadActor;

class SideEffect : public Object {
public:
    virtual void deserialize(Archive &archive) = 0;
    virtual void execute(LeadActor *actor) = 0;
};

class SideEffectExit : public SideEffect {
public:
    virtual void deserialize(Archive &archive);
    virtual void toConsole();
    virtual void execute(LeadActor *actor);

private:
    Common::String _nextModule;
    Common::String _nextPage;
};

class SideEffectLocation : public SideEffect {
public:
    virtual void deserialize(Archive &archive);
    virtual void execute(LeadActor *actor);
    virtual void toConsole();

private:
    Common::String _location;
};

class SideEffectInventoryItemOwner : public SideEffect {
public:
    virtual void deserialize(Archive &archive);
    virtual void execute(LeadActor *actor);
    virtual void toConsole();

private:
    Common::String _item;
    Common::String _owner;
};

class SideEffectVariable : public SideEffect {
public:
    virtual void deserialize(Archive &archive);
    virtual void execute(LeadActor *actor) = 0;

protected:
    Common::String _name;
    Common::String _value;
};

class SideEffectGameVariable : public SideEffectVariable {
public:
    virtual void toConsole();
    virtual void execute(LeadActor *actor);
};

class SideEffectModuleVariable : public SideEffectVariable {
public:
    virtual void toConsole();
    virtual void execute(LeadActor *actor);
};

class SideEffectPageVariable : public SideEffectVariable {
public:
    virtual void toConsole();
    virtual void execute(LeadActor *actor);
};

class SideEffectRandomPageVariable : public SideEffect
{
public:
    virtual void deserialize(Archive &archive);
    virtual void toConsole();
    virtual void execute(LeadActor *actor);

private:
    Common::String _name;
    Common::StringArray _values;
};

}

#endif
