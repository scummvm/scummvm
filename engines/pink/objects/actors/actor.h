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
#include <common/rect.h>
#include "engines/pink/objects/object.h"

namespace Pink {

class GamePage;
class Action;
class Sequencer;
class Director;
class CursorMgr;
class InventoryItem;
class InventoryMgr;

class Actor : public NamedObject {
public:
    Actor()
     : _page(nullptr), _action(nullptr),
        _isActionEnded(1)
    {};
    ~Actor();
    virtual void deserialize(Archive &archive);

    virtual void toConsole();

    Sequencer *getSequencer() const;
    GamePage *getPage() const;
    Action *getAction() const;

    bool isPlaying();
    virtual void init(bool unk);
    void hide();
    void endAction();

    Action *findAction(const Common::String &name);
    void setAction(const Common::String &name);
    void setAction(Action *newAction);
    void setAction(Action *newAction, bool unk);

    void loadState(Archive &archive);
    void saveState(Archive &archive);

    bool initPallete(Director *director);

    virtual void update() {};

    virtual void onMouseOver(Common::Point point, CursorMgr *mgr);
    virtual void onHover(Common::Point point, const Common::String &itemName, CursorMgr *cursorMgr);

    virtual bool isClickable() { return 0;}

    virtual void pause();
    virtual void unpause();

protected:
    GamePage *_page;
    Action *_action;
    Common::Array<Action*> _actions;
    bool _isActionEnded;
};

} // End of namespace Pink

#endif
