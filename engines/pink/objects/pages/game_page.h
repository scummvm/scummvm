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

#ifndef PINK_GAME_PAGE_H
#define PINK_GAME_PAGE_H

#include "page.h"

namespace Pink {

class CursorMgr;
class WalkMgr;
class Sequencer;
class Handler;

class GamePage : public Page {
public:
    virtual void deserialize(Archive &archive);

    virtual void load(Archive &archive);

    void loadManagers();

    void init(bool isLoadingSave);

    PinkEngine *getGame();
    Sequencer *getSequencer();
    WalkMgr *getWalkMgr();

    Module *getModule() const;
    bool checkValueOfVariable(Common::String &variable, Common::String &value);
    void setVariable(Common::String &variable, Common::String &value);

    virtual void toConsole();

private:
    void prepareHandler();


    int perhapsIsLoaded;
    Module *_module;
    CursorMgr *_cursorMgr;
    WalkMgr *_walkMgr;
    Sequencer *_sequencer;
    Common::Array<HandlerStartPage*> _handlers;
    Common::StringMap _variables;

    /*
    int cunk_1;
    int memfile;
    int unk;
    */
};

}

#endif //SCUMMVM_GAME_PAGE_H
