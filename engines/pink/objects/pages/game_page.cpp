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
    if (!isLoadingSave){
        //assert(perhapsIsLoaded == 0);
        loadManagers();
    }
    getGame()->getDirector()->clear();

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

    bool isHandler = 0;
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
    perhapsIsLoaded = true;
    _cursorMgr = new CursorMgr(_module->getGame(), this);
    _walkMgr = new WalkMgr;
    _sequencer = new Sequencer(this);

    _resMgr.init(_module->getGame(), this);

    // memfile manipulations if from save or page changing

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

bool GamePage::checkValueOfVariable(Common::String &variable, Common::String &value) {
    if (!_variables.contains(variable))
        return value == "UNDEFINED";
    return _variables[variable] == value;
}

void GamePage::setVariable(Common::String &variable, Common::String &value) {
    _variables[variable] = value;
}

WalkMgr *GamePage::getWalkMgr() {
    return _walkMgr;
}

void GamePage::toConsole() {
    Page::toConsole();
    _walkMgr->toConsole();
    _sequencer->toConsole();
    for (int i = 0; i < _handlers.size(); ++i) {
        _handlers[i]->toConsole();
    }
}

GamePage::~GamePage() {
    delete _cursorMgr;
    delete _walkMgr;
    delete _sequencer;
    for (int i = 0; i < _handlers.size(); ++i) {
        delete _handlers[i];
    }
}

GamePage::GamePage()
    : _cursorMgr(nullptr), _walkMgr(nullptr), _sequencer(nullptr)
{

}

} // End of namespace Pink