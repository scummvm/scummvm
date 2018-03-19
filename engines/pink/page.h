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

#ifndef PINK_PAGE_H
#define PINK_PAGE_H

#include "engines/pink/object.h"
#include "engines/pink/module.h"
#include "resource_mgr.h"

namespace Pink {

class Archive;
class Actor;
class LeadActor;


class Page : public NamedObject {
public:

    void load(Archive &archive);
protected:
    ResourceMgr _resMgr;
    LeadActor *_leadActor;
    Common::Array<Actor*> _actors;

    /*
        int unk_1;
        CString _str;
     */
};


class CursorMgr;
class WalkMgr;
class Sequencer;
class Handler;

class GamePage : public Page  {
public:
    virtual void deserialize(Archive &archive);
    virtual void load(Archive &archive);
    void loadFields();;

    void init(bool isLoadingSave);

    PinkEngine *getGame();

private:
    int perhapsIsLoaded;
    Module *_module;
    CursorMgr *_cursorMgr;
    WalkMgr *_walkMgr;
    Sequencer *_sequencer;
    Common::Array<Handler*> _handlers;

    /*
    int perhaps_notLoaded;
    int cunk_1;
    int memfile;
    CMapStringToString map;
    int unk;
    */
};

} // End of namespace Pink

#endif