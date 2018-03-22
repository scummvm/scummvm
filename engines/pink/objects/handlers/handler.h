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

#ifndef PINK_HANDLER_H
#define PINK_HANDLER_H

#include <common/array.h>
#include <common/str-array.h>
#include <engines/pink/objects/object.h>


namespace Pink {

class Condition;
class SideEffect;
class LeadActor;

class Handler : public Object {
public:
    virtual void deserialize(Archive &archive);
    bool isSuitable(LeadActor *actor);

protected:
    void prepareForNextHandler(LeadActor *actor);

    Common::Array<Condition*> _conditions;
    Common::Array<SideEffect*> _sideEffects;
};

class Sequence;

class HandlerSequences : public Handler {
public:
    virtual void deserialize(Archive &archive);
    void init(LeadActor *actor);
    virtual void handle(Sequence *sequence) = 0;

protected:
    Common::StringArray _sequences;
};

class HandlerStartPage : public HandlerSequences {
public:
    ~HandlerStartPage() {};

    virtual void toConsole();

    virtual void handle(Sequence *sequence);
};

} // End of namespace Pink

#endif
