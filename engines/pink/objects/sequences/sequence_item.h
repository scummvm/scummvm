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

#ifndef PINK_SEQUENCE_ITEM_H
#define PINK_SEQUENCE_ITEM_H

#include <engines/pink/objects/object.h>

namespace Pink {

class Sequence;

class SequenceItem : public Object {
public:
    virtual void deserialize(Archive &archive);

    virtual void toConsole();

    const Common::String &getActor() const;
    const Common::String &getAction() const;

    virtual bool execute(int index, Sequence *sequence, bool unk2);
    virtual bool isLeader();

protected:
    Common::String _actor;
    Common::String _action;
};

class SequenceItemLeader : public SequenceItem {
public:
    virtual void toConsole();
    virtual bool isLeader();
};

// behaviour is identical to SequenceItemLeader
class SequenceItemLeaderAudio : public SequenceItemLeader {
public:
    virtual void deserialize(Archive &archive);
    virtual void toConsole();

private:
    //uint32 _sample; // zero in data files and not used;
};

class SequenceItemDefaultAction : public SequenceItem {
public:
    virtual bool execute(int index, Sequence *sequence, bool unk2);

    virtual void toConsole();
};

/* not used in games but is implemented in engine
class SequenceItemSideEffects : public SequenceItemDefaultAction {
public:
    virtual void deserialize(Archive &archive);
    virtual bool execute(int unk, Sequence *sequence, bool unk2);
private
    Common::Array<SideEffect*> _sideEffects
};
 */

}

#endif
