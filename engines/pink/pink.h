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

#ifndef PINK_PINK_H
#define PINK_PINK_H

#include "common/random.h"
#include "engines/engine.h"
#include "gui/EventRecorder.h"
#include "gui/debugger.h"
#include "file.h"
#include "director.h"


/*
 *  This is the namespace of the Pink engine.
 *
 *  Status of this engine: In Development
 *
 *  Internal name of original engine: OxCart Runtime
 *
 *  Games using this engine:
 *  - The Pink Panther: Passport to Peril
 *  - The Pink Panther: Hokus Pokus Pink
 */

namespace Pink {

class Console;
class Archive;
class NamedObject;
class Module;
class GamePage;
class LeadActor;

enum {
    kPinkDebugGeneral = 1 << 0,
    kPinkDebugLoadingResources = 1 << 1,
    kPinkDebugLoadingObjects = 1 << 2,
    kPinkDebugGraphics = 1 << 3,
    kPinkDebugSound = 1 << 4
};

enum {
    kLoadingSave = 1,
    kLoadingNewGame = 0
};

class PinkEngine : public Engine {
public:
    PinkEngine(OSystem *system, const ADGameDescription *desc);
    ~PinkEngine();

    virtual Common::Error run();

    void load(Archive &archive);
    void initModule(const Common::String &moduleName, bool isLoadingFromSave, const Common::String &pageName);
    void changeScene(GamePage *page);

    OrbFile *getOrb()  { return &_orb; }
    BroFile *getBro()  { return _bro; }
    Common::RandomSource &getRnd() { return _rnd; };
    Director *getDirector() { return &_director; }

    void setNextExecutors(const Common::String &nextModule, const Common::String &nextPage);
    void setLeadActor(LeadActor *actor) { _actor = actor; };

    void setVariable(Common::String &variable, Common::String &value);
    bool checkValueOfVariable(Common::String &variable, Common::String &value);

private:
    Common::Error init();
    void loadModule(int index);



    Console *_console;
    Common::RandomSource _rnd;

    Common::String _nextModule;
    Common::String _nextPage;

    OrbFile  _orb;
    BroFile *_bro;

    Director _director;
    LeadActor *_actor;

    Module *_module;
    Common::Array<NamedObject*> _modules;

    Common::StringMap _variables;

    const ADGameDescription _desc;
};

} // End of namespace Pink

#endif