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

#ifndef PINK_ACTOR_H
#define PINK_ACTOR_H

#include <common/array.h>
#include "../object.h"

namespace Pink {

class GamePage;
class Action;
class Sequencer;

class Actor : public NamedObject {
public:
    Actor()
     : _page(nullptr), _action(nullptr),
        _isActionEnd(1)
    {};
    virtual void deserialize(Archive &archive);

    Action *findAction(Common::String &name);

    Sequencer *getSequencer();
    GamePage *getPage() const;

    void setIdleAction(bool unk);

protected:
    GamePage *_page;
    Action *_action;
    Common::Array<Action*> _actions;
    bool _isActionEnd;
};

} // End of namespace Pink

#endif
