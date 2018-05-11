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

#include <engines/pink/objects/handlers/handler.h>
#include "game_page.h"
#include <engines/pink/objects/walk/walk_mgr.h>
#include "engines/pink/cursor_mgr.h"
#include "engines/pink/objects/actors/lead_actor.h"
#include "engines/pink/objects/sequences/sequencer.h"
#include "pink/pink.h"

namespace Pink {

GamePage::GamePage()
        : _cursorMgr(nullptr), _walkMgr(nullptr), _sequencer(nullptr),
          _isLoaded(false), _memFile(nullptr)
{}

GamePage::~GamePage() {
    clear();
}

void GamePage::toConsole() {
    Page::toConsole();
    _walkMgr->toConsole();
    _sequencer->toConsole();
    for (int i = 0; i < _handlers.size(); ++i) {
        _handlers[i]->toConsole();
    }
}

void GamePage::deserialize(Archive &archive) {
    Page::deserialize(archive);
    _module = static_cast<Module*>(archive.readObject());
    assert(dynamic_cast<Module*>(_module) != 0);
}

void GamePage::load(Archive &archive) {
    archive.mapObject(_cursorMgr);
    archive.mapObject(_walkMgr);
    archive.mapObject(_sequencer);

    Page::load(archive);

    _leadActor = static_cast<LeadActor*>(archive.readObject());

    _walkMgr->deserialize(archive);
    _sequencer->deserialize(archive);
    archive >> _handlers;
}

void GamePage::init(bool isLoadingSave) {
    if (!_isLoaded)
        loadManagers();

    toConsole();

    for (int j = 0; j < _actors.size(); ++j) {
        if (_actors[j]->initPallete(_module->getGame()->getDirector()))
            break;
    }

    LeadActor::State state = _leadActor->getState();
    bool startNow = !(state == LeadActor::kInventory || state == LeadActor::kPDA);

    for (int i = 0; i < _actors.size(); ++i) {
        _actors[i]->init(startNow);
    }

    bool isHandler = false;
    if (!isLoadingSave) {
        isHandler = initHandler();
    }

    _leadActor->start(isHandler);
}

bool GamePage::initHandler() {
    for (uint i = 0; i < _handlers.size(); ++i) {
        if (_handlers[i]->isSuitable(_leadActor)){
            _handlers[i]->handle(_leadActor);
            return true;
        }
    }
    return false;
}

void GamePage::loadManagers() {
    _isLoaded = true;

    _cursorMgr = new CursorMgr(_module->getGame(), this);
    _walkMgr = new WalkMgr;
    _sequencer = new Sequencer(this);

    _resMgr.init(_module->getGame(), this);

    if (_memFile != nullptr) {
        loadState();

        delete _memFile;
        _memFile = nullptr;
    }
}

PinkEngine *GamePage::getGame() {
    return _resMgr.getGame();
}

Sequencer *GamePage::getSequencer() {
    return _sequencer;
}

Module *GamePage::getModule() const {
    return _module;
}

bool GamePage::checkValueOfVariable(const Common::String &variable, const Common::String &value) {
    if (!_variables.contains(variable))
        return value == kUndefined;
    return _variables[variable] == value;
}

void GamePage::setVariable(Common::String &variable, Common::String &value) {
    _variables[variable] = value;
    _leadActor->onVariableSet();
}

WalkMgr *GamePage::getWalkMgr() {
    return _walkMgr;
}

void GamePage::loadState() {
    Archive archive(static_cast<Common::SeekableReadStream*>(_memFile));
    //_variables.clear(1);
    Common::StringMap mapTest; // HACK. Without it isn't working
    //archive >> _variables;
    archive >> mapTest;
    _variables = mapTest;

    uint16 actorCount;
    archive >> actorCount;

    Common::String actorName;
    for (int i = 0; i < actorCount; ++i) {
        actorName = archive.readString();
        findActor(actorName)->loadState(archive);
    }
}

void GamePage::saveState() {
    _memFile = new Common::MemoryReadWriteStream(DisposeAfterUse::YES);
    Archive archive(static_cast<Common::WriteStream*>(_memFile));

    archive << _variables;

    archive.writeWORD(_actors.size());
    for (int i = 0; i < _actors.size(); ++i) {
        archive.writeString(_actors[i]->getName());
        _actors[i]->saveState(archive);
    }

}

void GamePage::unload() {
    _leadActor->setAction(_leadActor->findAction(kIdleAction));

    saveState();
    clear();

    _isLoaded = false;
}

void GamePage::clear() {
    Page::clear();
    //_variables.clear(1);

    for (int i = 0; i < _handlers.size(); ++i) {
        delete _handlers[i];
    }

    _handlers.clear();

    delete _cursorMgr; _cursorMgr = nullptr;
    delete _sequencer; _sequencer = nullptr;
    delete _walkMgr; _walkMgr = nullptr;
}


} // End of namespace Pink