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

#ifndef PINK_SEQUENCE_H
#define PINK_SEQUENCE_H

#include <engines/pink/objects/object.h>
#include <common/array.h>

namespace Pink {

class Sequencer;
class SequenceItem;
class SequenceContext;

class Sequence : public NamedObject {
public:
    Sequence();
    virtual ~Sequence();
    virtual void deserialize(Archive &archive);

    virtual void toConsole();

    Common::Array<SequenceItem*> &getItems();

    void setContext(SequenceContext *context);
    void init(int unk);
    void start(int unk);

public:
    SequenceContext *_context;
    Sequencer *_sequencer;
    Common::Array<SequenceItem*> _items;
    int _unk;

};

class SequenceActorState {
public:
    SequenceActorState(const Common::String &name);

    const Common::String &getActor() const;

public:
    Common::String _actorName;
    Common::String _actionName;
    int _unk;
};

class Actor;

class SequenceContext {
public:
    SequenceContext(Sequence *sequence, Sequencer* sequencer);
    ~SequenceContext();

public:
    Sequence *_sequence;
    Sequencer *_sequencer;
    int _nextItemIndex;
    Actor *_actor;
    Common::Array<SequenceActorState> _states;
    int _unk;
};

} // End of namespace Pink

#endif
